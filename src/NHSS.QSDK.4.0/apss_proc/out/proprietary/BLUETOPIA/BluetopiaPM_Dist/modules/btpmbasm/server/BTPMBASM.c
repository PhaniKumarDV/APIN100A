/*****< btpmbasm.c >***********************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMBASM - BAS Manager for Stonestreet One Bluetooth Protocol Stack       */
/*             Platform Manager.                                              */
/*                                                                            */
/*  Author:  Ryan Byrne                                                       */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/12/12  R. Byrne       Initial creation.                               */
/******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMBASM.h"            /* BTPM BAS Manager Prototypes/Constants.    */
#include "BASMAPI.h"             /* BAS Manager Prototypes/Constants.         */
#include "BASMMSG.h"             /* BTPM BAS Manager Message Formats.         */
#include "BASMGR.h"              /* BAS Manager Impl. Prototypes/Constants.   */

#include "SS1BTPM.h"             /* BTPM Main Prototypes and Constants.       */
#include "BTPMERR.h"             /* BTPM Error Prototypes/Constants.          */
#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

   /* The following defines the BASM LE Configuration File Section Name.*/
#define BASM_LE_CONFIGURATION_FILE_SECTION_NAME                   "BASM-Client"

   /* The following defines the Maximum Key Size that is used in the    */
   /* BASM LE Configuration File.                                       */
#define BASM_LE_CONFIGURATION_FILE_MAXIMUM_KEY_LENGTH             (10+(BD_ADDR_SIZE*2))

   /* The following defines the BASM LE Configuration File Maximum Line */
   /* Length.                                                           */
#define BASM_LE_CONFIGURATION_FILE_MAXIMUM_LINE_LENGTH            ((BTPM_CONFIGURATION_SETTINGS_MAXIMUM_FILE_LINE_LENGTH/2)-BASM_LE_CONFIGURATION_FILE_MAXIMUM_KEY_LENGTH)

   /* The following define the Key Names that are used with the BASM LE */
   /* Configuration File.                                               */
#define BASM_LE_KEY_NAME_BATTERY_LEVEL_CCCD                       "BL-%04u-%02X%02X%02X%02X%02X%02X"
#define BASM_LE_KEY_NAME_BATTERY_LEVEL_INSTANCE_COUNT             "IC-%02X%02X%02X%02X%02X%02X"

   /* Structure which is used to track what Battery instances a client  */
   /* wishes to be notified about.                                      */
typedef struct _tagNotifications_Enabled_Entry_t
{
   BD_ADDR_t                                 BluetoothAddress;
   unsigned int                              InstanceID;
   struct _tagNotifications_Enabled_Entry_t *NextNotificationsEnabledEntry;
} Notifications_Enabled_Entry_t;

   /* Structure which is used to track Callback Information related to  */
   /* this module.                                                      */
typedef struct _tagCallback_Entry_t
{
   unsigned int                   CallbackID;
   unsigned int                   AddressID;
   BASM_Event_Callback_t          EventCallback;
   void                          *CallbackParameter;
   Notifications_Enabled_Entry_t *NotificationsEnabledEntryList;
   struct _tagCallback_Entry_t   *NextCallbackEntry;
} Callback_Entry_t;

   /* Structure which is used to track information pertaining to known  */
   /* Battery Service instances.                                        */
typedef struct _tagInstance_Entry_t
{
   unsigned int                 Flags;
   unsigned int                 InstanceID;
   unsigned int                 NotificationCount;
   Boolean_t                    NotificationsEnabled;
   BAS_Client_Information_t     ServerInformationHandles;
   struct _tagInstance_Entry_t *NextInstanceEntry;
} Instance_Entry_t;

#define INSTANCE_ENTRY_FLAG_BATTERY_LEVEL_NOTIFY      0x00000001

   /* Structure which is used to track information pertaining to known  */
   /* connected LE devices supporting at least one Battery Service.     */
typedef struct _tagDevice_Entry_t
{
   BD_ADDR_t                  BluetoothAddress;
   Instance_Entry_t          *InstanceEntryList;
   struct _tagDevice_Entry_t *NextDeviceEntry;
} Device_Entry_t;

   /* The following enumeration is used to identify the type of         */
   /* transaction being processed.                                      */
typedef enum _tagTransaction_Type_t
{
   ttBatteryLevel = 0,
   ttBatteryIdentification,
   ttBatteryLevelCCCD
} Transaction_Type_t;

   /* The following structure is used to store transaction and response */
   /* information for the duration of the outstanding transaction.      */
typedef struct _tagTransaction_Entry_t
{
   Transaction_Type_t              TransactionType;
   unsigned int                    TransactionID;
   unsigned int                    GATTTransactionID;
   BD_ADDR_t                       BluetoothAddress;
   unsigned int                    InstanceID;
   unsigned int                    CallbackID;
   Word_t                          AttributeHandle;
   struct _tagTransaction_Entry_t *NextTransactionEntry;
} Transaction_Entry_t;

   /* The following structure is used to store Battery Level            */
   /* Notification data internal to this module.                        */
typedef struct _tagBattery_Level_Notification_Event_t
{
   BD_ADDR_t    RemoteServer;
   unsigned int InstanceID;
   Byte_t       BatteryLevel;
} Battery_Level_Notification_Event_t;

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
   /* Information List (which holds connected BAS LE devices).          */
static Device_Entry_t *DeviceEntryList;

   /* Variable which holds a pointer to the first transaction in the    */
   /* outstanding transaction list.                                     */
static Transaction_Entry_t *TransactionEntryList;

   /* List used to decode the Transaction Type into a Battery Instance  */
   /* Handle Offset.                                                    */
static BTPSCONST unsigned int TransactionTypeToHandleOffsetTable[] =
{
   BTPS_STRUCTURE_OFFSET(Instance_Entry_t, ServerInformationHandles) + BTPS_STRUCTURE_OFFSET(BAS_Client_Information_t, Battery_Level),
   BTPS_STRUCTURE_OFFSET(Instance_Entry_t, ServerInformationHandles) + BTPS_STRUCTURE_OFFSET(BAS_Client_Information_t, Battery_Level_Presentation_Format),
   BTPS_STRUCTURE_OFFSET(Instance_Entry_t, ServerInformationHandles) + BTPS_STRUCTURE_OFFSET(BAS_Client_Information_t, Battery_Level_Client_Configuration)
} ;

   /* Internal Function Prototypes.                                     */
static Notifications_Enabled_Entry_t *AddNotificationsEnabledEntry(Notifications_Enabled_Entry_t **EntryList, Notifications_Enabled_Entry_t *EntryToAdd);
static Notifications_Enabled_Entry_t *SearchNotificationsEnabledEntry(Notifications_Enabled_Entry_t **EntryList, BD_ADDR_t *BluetoothAddress, unsigned int InstanceID);
static Notifications_Enabled_Entry_t *DeleteNotificationsEnabledEntry(Notifications_Enabled_Entry_t **EntryList, BD_ADDR_t *BluetoothAddress, unsigned int InstanceID);
static Notifications_Enabled_Entry_t *DeleteNotificationsEnabledEntryBD_ADDR(Notifications_Enabled_Entry_t **EntryList, BD_ADDR_t *BluetoothAddress);
static void FreeNotificationsEnabledEntryMemory(Notifications_Enabled_Entry_t *EntryToFree);
static void FreeNotificationsEnabledEntryList(Notifications_Enabled_Entry_t **EntryList);

static unsigned int GetNextCallbackID(void);
static Callback_Entry_t *AddCallbackEntry(Callback_Entry_t *EntryToAdd);
static Callback_Entry_t *SearchCallbackEntry(unsigned int CallbackID);
static Callback_Entry_t *DeleteCallbackEntry(unsigned int CallbackID);
static void FreeCallbackEntryMemory(Callback_Entry_t *EntryToFree);
static void FreeCallbackEntryList(void);

static Boolean_t AddInstanceEntryActual(Instance_Entry_t **EntryList, Instance_Entry_t *EntryToAdd);
static Instance_Entry_t *SearchInstanceEntry(Instance_Entry_t **EntryList, unsigned int InstanceID);
static Instance_Entry_t *SearchDeviceInstanceEntry(BD_ADDR_t *Address, unsigned int InstanceID);
static Instance_Entry_t *SearchInstanceEntryByAttributeHandle(Instance_Entry_t **EntryList, unsigned int AttributeOffset, Word_t AttributeHandle);
static Instance_Entry_t *DeleteInstanceEntry(Instance_Entry_t **EntryList, unsigned int InstanceID);
static void FreeInstanceEntryMemory(Instance_Entry_t *EntryToFree);
static void FreeInstanceEntryList(Instance_Entry_t **EntryList);

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

static void FormatResponseMessageHeader(BASM_Response_Message_t *ResponseMessage, BTPM_Message_Header_t *MessageHeader);
static unsigned int CalculateNumberOfInstances(Instance_Entry_t *InstanceEntryList);

static void ProcessRegisterClientEventsRequestMessage(BASM_Register_Client_Events_Request_t *Message);
static void ProcessUnRegisterClientEventsRequestMessage(BASM_Un_Register_Client_Events_Request_t *Message);
static void ProcessEnableNotificationsRequestMessage(BASM_Request_Message_t *Message);
static void ProcessDisableNotificationsRequestMessage(BASM_Request_Message_t *Message);
static void ProcessGetBatteryLevelRequestMessage(BASM_Request_Message_t *Message);
static void ProcessGetBatteryIdentificationRequestMessage(BASM_Request_Message_t *Message);
static void ProcessCancelTransactionRequestMessage(BASM_Cancel_Transaction_Request_t *Message);
static void ProcessReceivedMessage(BTPM_Message_t *Message);
static void ProcessClientUnRegister(unsigned int AddressID);

static void DispatchLocalEvent(BASM_Event_Data_t *BASMEventData);
static void DispatchLocalBatteryLevelEvent(Transaction_Entry_t *TransactionEntry, int BatteryLevel);
static void DispatchRemoteBatteryLevelEvent(Transaction_Entry_t *TransactionEntry, int BatteryLevel, unsigned int AddressID);
static void DispatchBatteryLevelEvent(Transaction_Entry_t *TransactionEntry, int BatteryLevel);
static void DispatchLocalBatteryIdentificationEvent(Transaction_Entry_t *TransactionEntry, BAS_Presentation_Format_Data_t *PresentationFormatData);
static void DispatchRemoteBatteryIdentificationEvent(Transaction_Entry_t *TransactionEntry, BAS_Presentation_Format_Data_t *PresentationFormatData, unsigned int AddressID);
static void DispatchBatteryIdentificationEvent(Transaction_Entry_t *TransactionEntry, BAS_Presentation_Format_Data_t *PresentationFormatData);
static void DispatchBatteryLevelCCCDEvent(Transaction_Entry_t *TransactionEntry, int Response);
static void DispatchBatteryLevelNotificationEvent(Battery_Level_Notification_Event_t *BatteryLevelNotificationEvent);
static void DispatchConnectedEvent(Device_Entry_t *DeviceEntry);
static void DispatchDisconnectedEvent(Device_Entry_t *DeviceEntry);

static void DeleteEnabledEntriesForDevice(BD_ADDR_t BD_ADDR);

static void StoreBASMInstanceCount(BD_ADDR_t RemoteDeviceAddress, unsigned int NumberOfInstances);
static void StoreBASMInstanceInformation(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, Boolean_t NotificationsEnabled);
static void UpdateDeviceFile(Device_Entry_t *DeviceEntry);
static Boolean_t ReloadDeviceEntry(BD_ADDR_t BD_ADDR, unsigned int InstanceID, Boolean_t *NotificationsEnabled);
static void DeleteInstanceConfiguration(BD_ADDR_t BD_ADDR);

static Device_Entry_t *DiscoverBatteryServerInstances(BD_ADDR_t *BluetoothAddress);

static int FormatAddTransactionEntry(unsigned int CallbackID, BD_ADDR_t *RemoteServer, unsigned int InstanceID, Transaction_Type_t TransactionType, Transaction_Entry_t **ReturnTransactionEntry);
static int SubmitReadValue(unsigned int CallbackID, BD_ADDR_t *RemoteServer, unsigned int InstanceID, Transaction_Type_t TransactionType);
static int SubmitWriteValue(unsigned int CallbackID, BD_ADDR_t *RemoteServer, unsigned int InstanceID, Transaction_Type_t TransactionType, unsigned int DataLength, Byte_t *Data);
static int EnableNotifications(unsigned int CallbackID, BD_ADDR_t *RemoteServer, unsigned int InstanceID);
static int DisableNotifications(unsigned int CallbackID, BD_ADDR_t *RemoteServer, unsigned int InstanceID);
static int GetBatteryLevel(unsigned int CallbackID, BD_ADDR_t *RemoteServer, unsigned int InstanceID);
static int GetBatteryIdentification(unsigned int CallbackID, BD_ADDR_t *RemoteServer, unsigned int InstanceID);
static int CancelTransaction(unsigned int TransactionID);
static int WriteBatteryLevelCCCD(BD_ADDR_t *RemoteServer, unsigned int InstanceID, Boolean_t EnableNotifications);

static void ProcessBatteryLevelCCCDStateChange(Transaction_Entry_t *TransactionEntry, Boolean_t Success);

static void ProcessBASMConnection(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties);
static void ProcessBASMDisconnection(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties);
static void ProcessBASMDevicePaired(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties);
static void ProcessBASMDeviceUnPaired(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties);
static void ProcessLowEnergyAddressChangeEvent(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties);
static void ProcessRemoteDevicePropertiesChangedEvent(unsigned long ChangedMemberMask, DEVM_Remote_Device_Properties_t *RemoteDeviceProperties);
static void ProcessRemoteDeviceDeletedEvent(BD_ADDR_t RemoteDeviceAddress);

static void BTPSAPI BTPMDispatchCallback_BASM(void *CallbackParameter);
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter);
static void BTPSAPI BTPSDispatchCallback_GATT(void *CallbackParameter);
static void BTPSAPI BTPSDispatchCallback_BAS_Client(void *CallbackParameter);

static void BTPSAPI BASManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter);

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
static Notifications_Enabled_Entry_t *AddNotificationsEnabledEntry(Notifications_Enabled_Entry_t **EntryList, Notifications_Enabled_Entry_t *EntryToAdd)
{
   Notifications_Enabled_Entry_t *ret_val;

   /* Iterate the list to check for any duplicate entries.              */
   for(ret_val = *EntryList; ret_val;)
   {
      if(COMPARE_BD_ADDR(ret_val->BluetoothAddress, EntryToAdd->BluetoothAddress) && (ret_val->InstanceID == EntryToAdd->InstanceID))
      {
         /* A duplicate has been found.                                 */
         break;
      }
      else
         ret_val = ret_val->NextNotificationsEnabledEntry;
   }

   /* If a duplicate was not found, then add the entry to the list.     */
   if(!ret_val)
      ret_val = (Notifications_Enabled_Entry_t *)BSC_AddGenericListEntry(sizeof(Notifications_Enabled_Entry_t), ekNone, 0, sizeof(Notifications_Enabled_Entry_t), BTPS_STRUCTURE_OFFSET(Notifications_Enabled_Entry_t, NextNotificationsEnabledEntry), (void **)EntryList, ((void *)EntryToAdd));
   else
      ret_val = NULL;

   return(ret_val);
}

   /* The following function searches the module's Notifications Enabled*/
   /* Entry List for a Notifications Enabled Entry based on the         */
   /* specified Bluetooth Address and Instance ID pair.  This function  */
   /* returns NULL if either the Bluetooth Address is invalid, or the   */
   /* specified Entry was NOT present in the list.                      */
static Notifications_Enabled_Entry_t *SearchNotificationsEnabledEntry(Notifications_Enabled_Entry_t **EntryList, BD_ADDR_t *BluetoothAddress, unsigned int InstanceID)
{
   Notifications_Enabled_Entry_t *ret_val;

   /* Semi-validate the parameters.                                     */
   if((EntryList) && (BluetoothAddress))
   {
      /* Iterate the list to search for the specified entry.            */
      for(ret_val = *EntryList; ret_val;)
      {
         if(COMPARE_BD_ADDR(ret_val->BluetoothAddress, *BluetoothAddress) && (ret_val->InstanceID == InstanceID))
         {
            /* The entry has been found.                                */
            break;
         }
         else
            ret_val = ret_val->NextNotificationsEnabledEntry;
      }
   }
   else
      ret_val = NULL;

   return(ret_val);
}

   /* The following function searches the module's Notifications Enabled*/
   /* Entry List for the Notifications Enabled Entry with the specified */
   /* Bluetooth Address and Instance ID pair and removes it from the    */
   /* List.  This function returns NULL if either the Bluetooth Address */
   /* is invalid, or the specified Entry was NOT present in the list.   */
   /* The entry returned will have the Next Entry field set to NULL, and*/
   /* the caller is responsible for deleting the memory associated with */
   /* this entry by calling FreeCallbackEntryMemory().                  */
static Notifications_Enabled_Entry_t *DeleteNotificationsEnabledEntry(Notifications_Enabled_Entry_t **EntryList, BD_ADDR_t *BluetoothAddress, unsigned int InstanceID)
{
   Notifications_Enabled_Entry_t *ret_val;

   /* First, find the transaction entry.                                */
   if((ret_val = SearchNotificationsEnabledEntry(EntryList, BluetoothAddress, InstanceID)) != NULL)
   {
      /* A transaction entry was found, just delete it.                 */
      ret_val = BSC_DeleteGenericListEntry(ekEntryPointer, (void *)ret_val, 0, BTPS_STRUCTURE_OFFSET(Notifications_Enabled_Entry_t, NextNotificationsEnabledEntry), (void **)EntryList);
   }

   return(ret_val);
}

   /* The following function searches the module's Notifications Enabled*/
   /* Entry List for the Notifications Enabled Entry with the specified */
   /* Bluetooth Address and Instance ID pair and removes it from the    */
   /* List.  This function returns NULL if either the Bluetooth Address */
   /* is invalid, or the specified Entry was NOT present in the list.   */
   /* The entry returned will have the Next Entry field set to NULL, and*/
   /* the caller is responsible for deleting the memory associated with */
   /* this entry by calling FreeCallbackEntryMemory().                  */
static Notifications_Enabled_Entry_t *DeleteNotificationsEnabledEntryBD_ADDR(Notifications_Enabled_Entry_t **EntryList, BD_ADDR_t *BluetoothAddress)
{
   Notifications_Enabled_Entry_t *ret_val;

   /* A transaction entry was found, just delete it.                    */
   ret_val = (Notifications_Enabled_Entry_t *)BSC_DeleteGenericListEntry(ekBD_ADDR_t, (void *)BluetoothAddress, BTPS_STRUCTURE_OFFSET(Notifications_Enabled_Entry_t, BluetoothAddress), BTPS_STRUCTURE_OFFSET(Notifications_Enabled_Entry_t, NextNotificationsEnabledEntry), (void **)EntryList);

   return(ret_val);
}

   /* This function frees the specified Notifications Enabled Entry     */
   /* member.  No check is done on this entry other than making sure it */
   /* NOT NULL.                                                         */
static void FreeNotificationsEnabledEntryMemory(Notifications_Enabled_Entry_t *EntryToFree)
{
   BSC_FreeGenericListEntryMemory((void *)EntryToFree);
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the module's Notifications Enabled Entry List.  Upon   */
   /* return of this function, the Head Pointer is set to NULL.         */
static void FreeNotificationsEnabledEntryList(Notifications_Enabled_Entry_t **EntryList)
{
   BSC_FreeGenericListEntryList((void **)EntryList, BTPS_STRUCTURE_OFFSET(Notifications_Enabled_Entry_t, NextNotificationsEnabledEntry));
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
   /* Free the Notifications Enabled list before freeing the callback   */
   /* entry.                                                            */
   FreeNotificationsEnabledEntryList(&(EntryToFree->NotificationsEnabledEntryList));

   BSC_FreeGenericListEntryMemory((void *)EntryToFree);
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the module's Callback Entry List.  Upon return of this */
   /* function, the Head Pointer is set to NULL.                        */
static void FreeCallbackEntryList(void)
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

   /* The following function adds the specified Battery Instance Entry  */
   /* to the module's list.  This function simply adds the entry to the */
   /* list, it does not allocate a new buffer to store the entry.  This */
   /* function will return NULL if NO Entry was added.  This can occur  */
   /* if the element passed in was deemed invalid.                      */
   /* ** NOTE ** This function does not insert duplicate entries into   */
   /*            the list.  An element is considered a duplicate if the */
   /*            the BD_ADDR field is the same as an entry already in   */
   /*            the list.  When this occurs, this function returns     */
   /*            NULL.                                                  */
static Boolean_t AddInstanceEntryActual(Instance_Entry_t **EntryList, Instance_Entry_t *EntryToAdd)
{
   return(BSC_AddGenericListEntry_Actual(ekUnsignedInteger, BTPS_STRUCTURE_OFFSET(Instance_Entry_t, InstanceID), BTPS_STRUCTURE_OFFSET(Instance_Entry_t, NextInstanceEntry), (void **)EntryList, (void *)EntryToAdd));
}

   /* The following function searches a Battery Instance Entry List for */
   /* a Battery Instance Entry based on the specified Instance ID.  This*/
   /* function returns NULL if either the Instance ID is invalid, or the*/
   /* specified Entry was NOT present in the list.                      */
static Instance_Entry_t *SearchInstanceEntry(Instance_Entry_t **EntryList, unsigned int InstanceID)
{
   return((Instance_Entry_t *)BSC_SearchGenericListEntry(ekUnsignedInteger, (void *)&InstanceID, BTPS_STRUCTURE_OFFSET(Instance_Entry_t, InstanceID), BTPS_STRUCTURE_OFFSET(Instance_Entry_t, NextInstanceEntry), (void **)EntryList));
}

   /* The following function searches the module's Device Entry List and*/
   /* the Device's Battery Instance Entry List for a Battery Instance   */
   /* Entry based on the specified Bluetooth Address and Instance ID    */
   /* pair.  This function returns NULL if either the Bluetooth Address */
   /* is invalid or the specified Entry was NOT present in the list.    */
static Instance_Entry_t *SearchDeviceInstanceEntry(BD_ADDR_t *Address, unsigned int InstanceID)
{
   Device_Entry_t   *DeviceEntry;
   Instance_Entry_t *ret_val;

   if((DeviceEntry = SearchDeviceEntry(Address)) != NULL)
   {
      ret_val = SearchInstanceEntry(&(DeviceEntry->InstanceEntryList), InstanceID);
   }
   else
      ret_val = NULL;

   return(ret_val);
}

   /* The following function searches the a Battery Instance Entry List */
   /* for a Battery Instance Entry based on the specified Attribute     */
   /* Handle.  The offset of the attribute into the Instance Entry must */
   /* also be specified.  This function returns NULL if either the      */
   /* Instance ID is invalid, or the specified Entry was NOT present in */
   /* the list.                                                         */
static Instance_Entry_t *SearchInstanceEntryByAttributeHandle(Instance_Entry_t **EntryList, unsigned int AttributeOffset, Word_t AttributeHandle)
{
   Instance_Entry_t *ret_val;

   if((AttributeOffset >= BTPS_STRUCTURE_OFFSET(Instance_Entry_t, ServerInformationHandles)) && (AttributeOffset <= (BTPS_STRUCTURE_OFFSET(Instance_Entry_t, ServerInformationHandles) + sizeof(BAS_Client_Information_t))))
      ret_val = (Instance_Entry_t *)BSC_SearchGenericListEntry(ekWord_t, (void *)&AttributeHandle, AttributeOffset, BTPS_STRUCTURE_OFFSET(Instance_Entry_t, NextInstanceEntry), (void **)EntryList);
   else
      ret_val = NULL;

   return(ret_val);
}

   /* The following function searches the module's Battery Instance     */
   /* Entry List for the Battery Instance Entry with the specified      */
   /* Instance ID and removes it from the List.  This function returns  */
   /* NULL if either the Instance ID is invalid, or the specified Entry */
   /* was NOT present in the list.  The entry returned will have the    */
   /* Next Entry field set to NULL, and the caller is responsible for   */
   /* deleting the memory associated with this entry by calling         */
   /* FreeInstanceEntryMemory().                                        */
static Instance_Entry_t *DeleteInstanceEntry(Instance_Entry_t **EntryList, unsigned int InstanceID)
{
   return((Instance_Entry_t *)BSC_DeleteGenericListEntry(ekUnsignedInteger, (void *)&InstanceID, BTPS_STRUCTURE_OFFSET(Instance_Entry_t, InstanceID), BTPS_STRUCTURE_OFFSET(Instance_Entry_t, NextInstanceEntry), (void **)EntryList));
}

   /* This function frees the specified Battery Instance Entry member.  */
   /* No check is done on this entry other than making sure it NOT NULL.*/
static void FreeInstanceEntryMemory(Instance_Entry_t *EntryToFree)
{
   BSC_FreeGenericListEntryMemory((void *)EntryToFree);
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the Battery Instance Entry List.  Upon return of this  */
   /* function, the Head Pointer is set to NULL.                        */
static void FreeInstanceEntryList(Instance_Entry_t **EntryList)
{
  BSC_FreeGenericListEntryList((void **)EntryList, BTPS_STRUCTURE_OFFSET(Instance_Entry_t, NextInstanceEntry));
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
   /* Free the Instance list before freeing the Device entry.           */
   FreeInstanceEntryList(&(EntryToFree->InstanceEntryList));

   BSC_FreeGenericListEntryMemory((void *)EntryToFree);
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the module's Device Entry Entry List.  Upon return of  */
   /* this function, the Head Pointer is set to NULL.                   */
static void FreeDeviceEntryList(void)
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
static void FormatResponseMessageHeader(BASM_Response_Message_t *ResponseMessage, BTPM_Message_Header_t *MessageHeader)
{
   ResponseMessage->MessageHeader                = *MessageHeader;
   ResponseMessage->MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
   ResponseMessage->MessageHeader.MessageLength  = BASM_RESPONSE_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE;
}

   /* The following function will calculate and return the number of    */
   /* Battery Instance entries in the provided list.                    */
   /* * NOTE * This function *MUST* be called with the Module Manager's */
   /*          Lock held.                                               */
static unsigned int CalculateNumberOfInstances(Instance_Entry_t *InstanceEntryList)
{
   unsigned int      ret_val;
   Instance_Entry_t *InstanceEntry;

   for(InstanceEntry = InstanceEntryList, ret_val = 0; InstanceEntry; InstanceEntry = InstanceEntry->NextInstanceEntry, ret_val++);

   return(ret_val);
}

   /* The following function processes a Register Client Events Request */
   /* Message and responds to the message accordingly.  This function   */
   /* does not verify the integrity of the Message (i.e.  the length)   */
   /* because it is the caller's responsibility to verify the Message   */
   /* before calling this function.                                     */
   /* * NOTE * This function *MUST* be called with the Module Manager's */
   /*          Lock held.                                               */
static void ProcessRegisterClientEventsRequestMessage(BASM_Register_Client_Events_Request_t *Message)
{
   Callback_Entry_t        *CallbackEntryPtr;
   Callback_Entry_t         CallbackEntry;
   BASM_Response_Message_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Initialize the Callback Entry.                                 */
      BTPS_MemInitialize(&CallbackEntry, 0, sizeof(Callback_Entry_t));

      CallbackEntry.AddressID     = Message->MessageHeader.AddressID;
      CallbackEntry.EventCallback = NULL;

      /* Format the response message.                                   */
      FormatResponseMessageHeader(&ResponseMessage, &(Message->MessageHeader));

      if((CallbackEntryPtr = AddCallbackEntry(&CallbackEntry)) != NULL)
      {
         /* Give the callback an ID and indicate success.               */
         CallbackEntryPtr->CallbackID = GetNextCallbackID();
         ResponseMessage.Status       = CallbackEntryPtr->CallbackID;
      }
      else
         ResponseMessage.Status = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;

      /* Send the response message.                                     */
      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes a Un Register Client Events      */
   /* Request Message and responds to the message accordingly.  This    */
   /* function does not verify the integrity of the Message (i.e.  the  */
   /* length) because it is the caller's responsibility to verify the   */
   /* Message before calling this function.                             */
   /* * NOTE * This function *MUST* be called with the Module Manager's */
   /*          Lock held.                                               */
static void ProcessUnRegisterClientEventsRequestMessage(BASM_Un_Register_Client_Events_Request_t *Message)
{
   Callback_Entry_t        *CallbackEntryPtr;
   BASM_Response_Message_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes an Enable Notifications Request  */
   /* Message and responds to the message accordingly.  This function   */
   /* does not verify the integrity of the Message (i.e.  the length)   */
   /* because it is the caller's responsibility to verify the Message   */
   /* before calling this function.                                     */
   /* * NOTE * This function *MUST* be called with the Module Manager's */
   /*          Lock held.                                               */
static void ProcessEnableNotificationsRequestMessage(BASM_Request_Message_t *Message)
{
   BASM_Response_Message_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Format the response message.                                   */
      FormatResponseMessageHeader(&ResponseMessage, &(Message->MessageHeader));

      ResponseMessage.Status  = EnableNotifications(Message->CallbackID, &(Message->RemoteDeviceAddress), Message->InstanceID);

      /* Send the response message.                                     */
      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes a Disable Notifications Request  */
   /* Message and responds to the message accordingly.  This function   */
   /* does not verify the integrity of the Message (i.e.  the length)   */
   /* because it is the caller's responsibility to verify the Message   */
   /* before calling this function.                                     */
   /* * NOTE * This function *MUST* be called with the Module Manager's */
   /*          Lock held.                                               */
static void ProcessDisableNotificationsRequestMessage(BASM_Request_Message_t *Message)
{
   BASM_Response_Message_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Format the response message.                                   */
      FormatResponseMessageHeader(&ResponseMessage, &(Message->MessageHeader));

      ResponseMessage.Status  = DisableNotifications(Message->CallbackID, &(Message->RemoteDeviceAddress), Message->InstanceID);;

      /* Send the response message.                                     */
      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes a Get Battery Level Request      */
   /* Message and responds to the message accordingly.  This function   */
   /* does not verify the integrity of the Message (i.e.  the length)   */
   /* because it is the caller's responsibility to verify the Message   */
   /* before calling this function.                                     */
   /* * NOTE * This function *MUST* be called with the Module Manager's */
   /*          Lock held.                                               */
static void ProcessGetBatteryLevelRequestMessage(BASM_Request_Message_t *Message)
{
   BASM_Response_Message_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Format the response message.                                   */
      FormatResponseMessageHeader(&ResponseMessage, &(Message->MessageHeader));

      if(SearchCallbackEntry(Message->CallbackID))
         ResponseMessage.Status = GetBatteryLevel(Message->CallbackID, &(Message->RemoteDeviceAddress), Message->InstanceID);
      else
         ResponseMessage.Status = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      /* Send the response message.                                     */
      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes a Get Battery Identification     */
   /* Request Message and responds to the message accordingly.  This    */
   /* function does not verify the integrity of the Message (i.e.  the  */
   /* length) because it is the caller's responsibility to verify the   */
   /* Message before calling this function.                             */
   /* * NOTE * This function *MUST* be called with the Module Manager's */
   /*          Lock held.                                               */
static void ProcessGetBatteryIdentificationRequestMessage(BASM_Request_Message_t *Message)
{
   BASM_Response_Message_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Format the response message.                                   */
      FormatResponseMessageHeader(&ResponseMessage, &(Message->MessageHeader));

      if(SearchCallbackEntry(Message->CallbackID))
         ResponseMessage.Status = GetBatteryIdentification(Message->CallbackID, &(Message->RemoteDeviceAddress), Message->InstanceID);
      else
         ResponseMessage.Status = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      /* Send the response message.                                     */
      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes a Cancel Transaction Request     */
   /* Message and responds to the message accordingly.  This function   */
   /* does not verify the integrity of the Message (i.e.  the length)   */
   /* because it is the caller's responsibility to verify the Message   */
   /* before calling this function.                                     */
   /* * NOTE * This function *MUST* be called with the Module Manager's */
   /*          Lock held.                                               */
static void ProcessCancelTransactionRequestMessage(BASM_Cancel_Transaction_Request_t *Message)
{
   BASM_Response_Message_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Format the response message.                                   */
      FormatResponseMessageHeader(&ResponseMessage, &(Message->MessageHeader));

      ResponseMessage.Status = CancelTransaction(Message->TransactionID);

      /* Send the response message.                                     */
      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
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
   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", Message->MessageHeader.MessageFunction));

   /* Check to make sure this isn't some kind of spoofed response (we do*/
   /* not process responses).                                           */
   if((Message) && (!(Message->MessageHeader.MessageID & BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK)))
   {
      /* Request received, go ahead and process it.                     */
      switch(Message->MessageHeader.MessageFunction)
      {
         case BASM_MESSAGE_FUNCTION_REGISTER_CLIENT_EVENTS:
            DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_VERBOSE), ("Register BAS Events Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= BASM_REGISTER_CLIENT_EVENTS_REQUEST_SIZE)
            {
               ProcessRegisterClientEventsRequestMessage((BASM_Register_Client_Events_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case BASM_MESSAGE_FUNCTION_UN_REGISTER_CLIENT_EVENTS:
            DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_VERBOSE), ("Un-Register BAS Events Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= BASM_UN_REGISTER_CLIENT_EVENTS_REQUEST_SIZE)
            {
               ProcessUnRegisterClientEventsRequestMessage((BASM_Un_Register_Client_Events_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case BASM_MESSAGE_FUNCTION_ENABLE_NOTIFICATIONS:
            DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_VERBOSE), ("Enable Notifications Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= BASM_REQUEST_MESSAGE_SIZE)
            {
               ProcessEnableNotificationsRequestMessage((BASM_Request_Message_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case BASM_MESSAGE_FUNCTION_DISABLE_NOTIFICATIONS:
            DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_VERBOSE), ("Disable Notifications Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= BASM_REQUEST_MESSAGE_SIZE)
            {
               ProcessDisableNotificationsRequestMessage((BASM_Request_Message_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case BASM_MESSAGE_FUNCTION_GET_BATTERY_LEVEL:
            DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_VERBOSE), ("Get Battery Level Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= BASM_REQUEST_MESSAGE_SIZE)
            {
               ProcessGetBatteryLevelRequestMessage((BASM_Request_Message_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case BASM_MESSAGE_FUNCTION_GET_BATTERY_IDENTIFICATION:
            DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_VERBOSE), ("Get Battery Identification Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= BASM_REQUEST_MESSAGE_SIZE)
            {
               ProcessGetBatteryIdentificationRequestMessage((BASM_Request_Message_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case BASM_MESSAGE_FUNCTION_CANCEL_TRANSACTION:
            DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_VERBOSE), ("Cancel Transaction Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= BASM_CANCEL_TRANSACTION_REQUEST_SIZE)
            {
               ProcessCancelTransactionRequestMessage((BASM_Cancel_Transaction_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         default:
            DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled Message\n"));
            break;
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid or Response Message Received\n"));
   }

   /* Release the Lock because we are finished with it.                 */
   DEVM_ReleaseLock();

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the function that is called to process a*/
   /* client Un-Registration event.  This function will perform any     */
   /* cleanup required for the specified Client.                        */
static void ProcessClientUnRegister(unsigned int AddressID)
{
   Callback_Entry_t *CallbackEntry;
   Callback_Entry_t *tmpCallbackEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", AddressID));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a possible BASM Connection.                               */
static void ProcessBASMConnection(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* First make sure that the input parameter appears to be semi-valid.*/
   if(RemoteDeviceProperties)
   {
      /* Update Device Entry with service discovery information.        */
      DiscoverBatteryServerInstances(&(RemoteDeviceProperties->BD_ADDR));
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a possible BASM disconnection.                            */
static void ProcessBASMDisconnection(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties)
{
   Device_Entry_t *DeviceEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

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

      /* Delete all enabled entries for this device.                    */
      DeleteEnabledEntriesForDevice(RemoteDeviceProperties->BD_ADDR);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a possible BASM Device Paired Event.                      */
static void ProcessBASMDevicePaired(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties)
{
   Boolean_t         NotificationsEnabled;
   Boolean_t         UpdateFile;
   Device_Entry_t   *DeviceEntry;
   Instance_Entry_t *InstanceEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* First make sure that the input parameter appears to be semi-valid.*/
   if(RemoteDeviceProperties)
   {
      /* Make sure that we have a device entry at this stage.           */
      if((DeviceEntry = SearchDeviceEntry(&(RemoteDeviceProperties->BD_ADDR))) != NULL)
      {
         /* Walk the instance list and sync each instance with the      */
         /* device file.                                                */
         InstanceEntry = DeviceEntry->InstanceEntryList;
         UpdateFile    = FALSE;
         while(InstanceEntry)
         {
            /* Check to see if this instance supports notifications.    */
            if(InstanceEntry->Flags & INSTANCE_ENTRY_FLAG_BATTERY_LEVEL_NOTIFY)
            {
               /* Initialize that notifications are not enabled.        */
               NotificationsEnabled = FALSE;

               /* Attempt to reload the configuration for this device.  */
               if(ReloadDeviceEntry(DeviceEntry->BluetoothAddress, InstanceEntry->InstanceID, &NotificationsEnabled))
               {
                  if(InstanceEntry->NotificationsEnabled)
                  {
                     if(!NotificationsEnabled)
                        UpdateFile = TRUE;
                  }
                  else
                     InstanceEntry->NotificationsEnabled = NotificationsEnabled;
               }
               else
               {
                  if(InstanceEntry->NotificationsEnabled)
                     UpdateFile = TRUE;
               }
            }

            InstanceEntry = InstanceEntry->NextInstanceEntry;
         }

         /* Update the file if request.                                 */
         if(UpdateFile)
            UpdateDeviceFile(DeviceEntry);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a possible BASM Device Un-Paired Event.                   */
static void ProcessBASMDeviceUnPaired(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* First make sure that the input parameter appears to be semi-valid.*/
   if(RemoteDeviceProperties)
   {
      /* Make sure that any device entry stored for this device is      */
      /* deleted if we have written any information for the device to   */
      /* the file.                                                      */
      DeleteInstanceConfiguration(RemoteDeviceProperties->BD_ADDR);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the function that is called to process a*/
   /* LE Address Change Event.                                          */
   /* * NOTE * This function *MUST* be called with the HOGM Manager Lock*/
   /*          held.                                                    */
static void ProcessLowEnergyAddressChangeEvent(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties)
{
   Device_Entry_t *DeviceEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if(RemoteDeviceProperties)
   {
      /* Make sure that any device entry stored for this device is      */
      /* deleted if we have written any information for the device to   */
      /* the file.                                                      */
      DeleteInstanceConfiguration(RemoteDeviceProperties->PriorResolvableBD_ADDR);

      /* Walk the Connection List and update any BD_ADDRs as needed.    */
      DeviceEntry = DeviceEntryList;
      while(DeviceEntry)
      {
         /* Check to see if this entry needs to be updated.             */
         if(COMPARE_BD_ADDR(DeviceEntry->BluetoothAddress, RemoteDeviceProperties->PriorResolvableBD_ADDR))
         {
            DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_VERBOSE), ("BASM Address Updated\n"));

            /* Save the new Base Address.                               */
            DeviceEntry->BluetoothAddress = RemoteDeviceProperties->BD_ADDR;

            /* Update the device file if necessary.                     */
            UpdateDeviceFile(DeviceEntry);
         }

         /* Advance to the next entry in the list.                      */
         DeviceEntry = DeviceEntry->NextDeviceEntry;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a Remote Device Properties Changed Event.                 */
   /* * NOTE * This function *MUST* be called with the BASM Manager Lock*/
   /*          held.                                                    */
static void ProcessRemoteDevicePropertiesChangedEvent(unsigned long ChangedMemberMask, DEVM_Remote_Device_Properties_t *RemoteDeviceProperties)
{
   unsigned long RequiredConnectionFlags;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08lX\n", ChangedMemberMask));

   /* First make sure that the input parameter appears to be semi-valid.*/
   if((ChangedMemberMask) && (RemoteDeviceProperties) && (RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_SUPPORTS_LOW_ENERGY))
   {
      /* Check to see what changed.  We are only interested if the LE   */
      /* Address is updated or the LE Pairing State changes.            */
      if(ChangedMemberMask & (DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_PAIRING_STATE | DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_PRIOR_RESOLVABLE_ADDRESS | DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_ENCRYPTION_STATE | DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_CONNECTION_STATE | DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_SERVICES_STATE))
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Flags: 0x%08lX\n", RemoteDeviceProperties->RemoteDeviceFlags));

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
               /* Process a possible BASM Connection.                   */
               ProcessBASMConnection(RemoteDeviceProperties);
            }
            else
            {
               /* Process a possible BASM Disconnection Event.          */
               ProcessBASMDisconnection(RemoteDeviceProperties);

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
                  DeleteInstanceConfiguration(RemoteDeviceProperties->BD_ADDR);
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
               ProcessBASMDevicePaired(RemoteDeviceProperties);
            else
            {
               /* If we are no longer paired then we need to process    */
               /* this event.                                           */
               if(!(RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_CURRENTLY_PAIRED_OVER_LE))
                  ProcessBASMDeviceUnPaired(RemoteDeviceProperties);
            }
         }

         /* Process the Address Updated event if necessary.             */
         if(ChangedMemberMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_PRIOR_RESOLVABLE_ADDRESS)
            ProcessLowEnergyAddressChangeEvent(RemoteDeviceProperties);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a Remote Device Deleted Event.                            */
   /* * NOTE * This function *MUST* be called with the BASM Manager Lock*/
   /*          held.                                                    */
static void ProcessRemoteDeviceDeletedEvent(BD_ADDR_t RemoteDeviceAddress)
{
   Device_Entry_t *DeviceEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

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

      /* Delete all enabled entries for this device.                    */
      DeleteEnabledEntriesForDevice(RemoteDeviceAddress);

      /* Make sure that any device entry stored for this device is      */
      /* deleted if we have written any information for the device to   */
      /* the file.                                                      */
      DeleteInstanceConfiguration(RemoteDeviceAddress);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process the Module Manager's Asynchronous Events.   */
static void BTPSAPI BTPMDispatchCallback_BASM(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process Client Un-Register Asynchronous Events.     */
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch the specified module event to the registered local event */
   /* callbacks.  The function will verify that a callback has been     */
   /* registered.                                                       */
   /* * NOTE * This function should be called with the Module Manager's */
   /*          Lock held.                                               */
static void DispatchLocalEvent(BASM_Event_Data_t *EventData)
{
   Callback_Entry_t *CallbackEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch a Local Battery Level Response Event.  It is the caller's*/
   /* responsibility to verify the Response before calling this         */
   /* function.                                                         */
   /* * NOTE * This function should be called with the Module Manager's */
   /*          Lock held.                                               */
static void DispatchLocalBatteryLevelEvent(Transaction_Entry_t *TransactionEntry, int BatteryLevel)
{
   BASM_Event_Data_t EventData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format the event that will be dispatched locally.                 */
   EventData.EventType                                           = betBASBatteryLevel;
   EventData.EventLength                                         = BASM_BATTERY_LEVEL_EVENT_DATA_SIZE;
   EventData.EventCallbackID                                     = TransactionEntry->CallbackID;
   EventData.EventData.BatteryLevelEventData.RemoteDeviceAddress = TransactionEntry->BluetoothAddress;
   EventData.EventData.BatteryLevelEventData.InstanceID          = TransactionEntry->InstanceID;
   EventData.EventData.BatteryLevelEventData.TransactionID       = TransactionEntry->TransactionID;

   if(BatteryLevel >= 0)
   {
      EventData.EventData.BatteryLevelEventData.Status       = 0;
      EventData.EventData.BatteryLevelEventData.BatteryLevel = (Byte_t)BatteryLevel;
   }
   else
      EventData.EventData.BatteryLevelEventData.Status       = BatteryLevel;

   /* Dispatch the event to the registered callback.                    */
   DispatchLocalEvent(&EventData);

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch a Remote Battery Level Response Message.  It is the      */
   /* caller's responsibility to verify the Response before calling this*/
   /* function.                                                         */
   /* * NOTE * This function should be called with the Module Manager's */
   /*          Lock held.                                               */
static void DispatchRemoteBatteryLevelEvent(Transaction_Entry_t *TransactionEntry, int BatteryLevel, unsigned int AddressID)
{
   BASM_Battery_Level_Message_t Message;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   Message.MessageHeader.AddressID       = AddressID;
   Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
   Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_BATTERY_SERVICE_MANAGER;
   Message.MessageHeader.MessageFunction = BASM_MESSAGE_FUNCTION_BATTERY_LEVEL;
   Message.MessageHeader.MessageLength   = (BASM_BATTERY_LEVEL_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);
   Message.CallbackID                    = TransactionEntry->CallbackID;
   Message.RemoteDeviceAddress           = TransactionEntry->BluetoothAddress;
   Message.InstanceID                    = TransactionEntry->InstanceID;
   Message.TransactionID                 = TransactionEntry->TransactionID;

   if(BatteryLevel >= 0)
   {
      Message.Status       = 0;
      Message.BatteryLevel = (Byte_t)BatteryLevel;
   }
   else
      Message.Status = BatteryLevel;

   MSG_SendMessage((BTPM_Message_t *)&Message);

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch a Battery Level Response Event.  It is the caller's      */
   /* responsibility to verify the Response before calling this         */
   /* function.                                                         */
   /* * NOTE * This function should be called with the Module Manager's */
   /*          Lock held.                                               */
static void DispatchBatteryLevelEvent(Transaction_Entry_t *TransactionEntry, int BatteryLevel)
{
   Callback_Entry_t *CallbackEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that there is a callback to dispatch to.                   */
   if(((CallbackEntry = SearchCallbackEntry(TransactionEntry->CallbackID))) != NULL)
   {
      if(CallbackEntry->AddressID == MSG_GetServerAddressID())
         DispatchLocalBatteryLevelEvent(TransactionEntry, BatteryLevel);
      else
         DispatchRemoteBatteryLevelEvent(TransactionEntry, BatteryLevel, CallbackEntry->AddressID);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch a Local Battery Identification Response Event.  It is the*/
   /* caller's responsibility to verify the Response before calling this*/
   /* function.                                                         */
   /* * NOTE * This function should be called with the Module Manager's */
   /*          Lock held.                                               */
static void DispatchLocalBatteryIdentificationEvent(Transaction_Entry_t *TransactionEntry, BAS_Presentation_Format_Data_t *PresentationFormatData)
{
   BASM_Event_Data_t EventData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format the event that will be dispatched locally.                 */
   EventData.EventType                                                    = betBASBatteryIdentification;
   EventData.EventLength                                                  = BASM_BATTERY_IDENTIFICATION_EVENT_DATA_SIZE;
   EventData.EventCallbackID                                              = TransactionEntry->CallbackID;
   EventData.EventData.BatteryIdentificationEventData.RemoteDeviceAddress = TransactionEntry->BluetoothAddress;
   EventData.EventData.BatteryIdentificationEventData.InstanceID          = TransactionEntry->InstanceID;
   EventData.EventData.BatteryIdentificationEventData.TransactionID       = TransactionEntry->TransactionID;

   if(PresentationFormatData != NULL)
   {
      EventData.EventData.BatteryIdentificationEventData.Status      = 0;
      EventData.EventData.BatteryIdentificationEventData.Namespace   = PresentationFormatData->NameSpace;
      EventData.EventData.BatteryIdentificationEventData.Description = PresentationFormatData->Description;
   }
   else
      EventData.EventData.BatteryLevelEventData.Status = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

   /* Dispatch the event to the registered callback.                    */
   DispatchLocalEvent(&EventData);

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch a Remote Battery Identification Response Message.  It is */
   /* the caller's responsibility to verify the Response before calling */
   /* this function.                                                    */
   /* * NOTE * This function should be called with the Module Manager's */
   /*          Lock held.                                               */
static void DispatchRemoteBatteryIdentificationEvent(Transaction_Entry_t *TransactionEntry, BAS_Presentation_Format_Data_t *PresentationFormatData, unsigned int AddressID)
{
   BASM_Battery_Identification_Message_t Message;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   Message.MessageHeader.AddressID       = AddressID;
   Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
   Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_BATTERY_SERVICE_MANAGER;
   Message.MessageHeader.MessageFunction = BASM_MESSAGE_FUNCTION_BATTERY_IDENTIFICATION;
   Message.MessageHeader.MessageLength   = (BASM_BATTERY_IDENTIFICATION_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);
   Message.CallbackID                    = TransactionEntry->CallbackID;
   Message.RemoteDeviceAddress           = TransactionEntry->BluetoothAddress;
   Message.InstanceID                    = TransactionEntry->InstanceID;
   Message.TransactionID                 = TransactionEntry->TransactionID;

   if(PresentationFormatData != NULL)
   {
      Message.Status      = 0;
      Message.Namespace   = PresentationFormatData->NameSpace;
      Message.Description = PresentationFormatData->Description;
   }
   else
      Message.Status = BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID;

   MSG_SendMessage((BTPM_Message_t *)&Message);

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch a Battery Identification Response Event.  It is the      */
   /* caller's responsibility to verify the Response before calling this*/
   /* function.                                                         */
   /* * NOTE * This function should be called with the Module Manager's */
   /*          Lock held.                                               */
static void DispatchBatteryIdentificationEvent(Transaction_Entry_t *TransactionEntry, BAS_Presentation_Format_Data_t *PresentationFormatData)
{
   Callback_Entry_t *CallbackEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that there is a callback to dispatch to.                   */
   if(((CallbackEntry = SearchCallbackEntry(TransactionEntry->CallbackID))) != NULL)
   {
      if(CallbackEntry->AddressID == MSG_GetServerAddressID())
         DispatchLocalBatteryIdentificationEvent(TransactionEntry, PresentationFormatData);
      else
         DispatchRemoteBatteryIdentificationEvent(TransactionEntry, PresentationFormatData, CallbackEntry->AddressID);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch a Battery Level CCD Event.  It is the caller's           */
   /* responsibility to verify the Response before calling this         */
   /* function.                                                         */
   /* * NOTE * This function should be called with the Module Manager's */
   /*          Lock held.                                               */
static void DispatchBatteryLevelCCCDEvent(Transaction_Entry_t *TransactionEntry, int Response)
{
   /* There is nothing to do in this function at this time.  It is here */
   /* in case hooking the CCD response is needed in the future.         */
   if(!Response)
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_VERBOSE), ("Notifications Enabled on device %02X:%02X:%02X:%02X:%02X:%02X, instance %u\n",
                                                                              TransactionEntry->BluetoothAddress.BD_ADDR5,
                                                                              TransactionEntry->BluetoothAddress.BD_ADDR4,
                                                                              TransactionEntry->BluetoothAddress.BD_ADDR3,
                                                                              TransactionEntry->BluetoothAddress.BD_ADDR2,
                                                                              TransactionEntry->BluetoothAddress.BD_ADDR1,
                                                                              TransactionEntry->BluetoothAddress.BD_ADDR0,
                                                                              TransactionEntry->InstanceID));
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_VERBOSE), ("Notification Enable Failure on device %02X:%02X:%02X:%02X:%02X:%02X, instance %u\n",
                                                                              TransactionEntry->BluetoothAddress.BD_ADDR5,
                                                                              TransactionEntry->BluetoothAddress.BD_ADDR4,
                                                                              TransactionEntry->BluetoothAddress.BD_ADDR3,
                                                                              TransactionEntry->BluetoothAddress.BD_ADDR2,
                                                                              TransactionEntry->BluetoothAddress.BD_ADDR1,
                                                                              TransactionEntry->BluetoothAddress.BD_ADDR0,
                                                                              TransactionEntry->InstanceID));
   }
}

   /* The following function is a utility function that exists to       */
   /* dispatch a Battery Level Notification to all registered clients   */
   /* that have requested notifications.  It is the caller's            */
   /* responsibility to verify the Response before calling this         */
   /* function.                                                         */
   /* * NOTE * This function should be called with the Module Manager's */
   /*          Lock held.                                               */
static void DispatchBatteryLevelNotificationEvent(Battery_Level_Notification_Event_t *BatteryLevelNotificationEvent)
{
   Callback_Entry_t                          *CallbackEntry;
   BASM_Event_Data_t                          EventData;
   BASM_Battery_Level_Notification_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format the event that will be dispatched locally.                 */
   EventData.EventType                                                       = betBASBatteryLevelNotification;
   EventData.EventLength                                                     = BASM_BATTERY_LEVEL_NOTIFICATION_EVENT_DATA_SIZE;
   EventData.EventData.BatteryLevelNotificationEventData.RemoteDeviceAddress = BatteryLevelNotificationEvent->RemoteServer;
   EventData.EventData.BatteryLevelNotificationEventData.InstanceID          = BatteryLevelNotificationEvent->InstanceID;
   EventData.EventData.BatteryLevelNotificationEventData.BatteryLevel        = BatteryLevelNotificationEvent->BatteryLevel;

   /* Format the message that will be dispatched remotely.              */
   Message.MessageHeader.MessageGroup                                        = BTPM_MESSAGE_GROUP_BATTERY_SERVICE_MANAGER;
   Message.MessageHeader.MessageFunction                                     = BASM_MESSAGE_FUNCTION_BATTERY_LEVEL_NOTIFICATION;
   Message.MessageHeader.MessageLength                                       = (BASM_BATTERY_LEVEL_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);
   Message.RemoteDeviceAddress                                               = BatteryLevelNotificationEvent->RemoteServer;
   Message.InstanceID                                                        = BatteryLevelNotificationEvent->InstanceID;
   Message.BatteryLevel                                                      = BatteryLevelNotificationEvent->BatteryLevel;

   /* Iterate through each callback entry looking for any clients that  */
   /* have enabled notifications for this Battery Instance.             */
   for(CallbackEntry=CallbackEntryList;CallbackEntry;CallbackEntry=CallbackEntry->NextCallbackEntry)
   {
      if(SearchNotificationsEnabledEntry(&(CallbackEntry->NotificationsEnabledEntryList), &(BatteryLevelNotificationEvent->RemoteServer), BatteryLevelNotificationEvent->InstanceID) != NULL)
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

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch a Connected Event to every registered callback.  It is   */
   /* the caller's responsibility to verify the Response before calling */
   /* this function.                                                    */
   /* * NOTE * This function should be called with the Module Manager's */
   /*          Lock held.                                               */
static void DispatchConnectedEvent(Device_Entry_t *DeviceEntry)
{
   unsigned int              NumberOfInstances;
   Callback_Entry_t         *CallbackEntry;
   BASM_Event_Data_t         EventData;
   BASM_Connected_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Calculate the number of Battery Service Instances.                */
   NumberOfInstances                                          = CalculateNumberOfInstances(DeviceEntry->InstanceEntryList);

   /* Format the event that will be dispatched locally.                 */
   EventData.EventType                                        = betBASConnected;
   EventData.EventLength                                      = BASM_CONNECTED_EVENT_DATA_SIZE;
   EventData.EventData.ConnectedEventData.RemoteDeviceAddress = DeviceEntry->BluetoothAddress;
   EventData.EventData.ConnectedEventData.ConnectionType      = bctClient;
   EventData.EventData.ConnectedEventData.ConnectedFlags      = 0;
   EventData.EventData.ConnectedEventData.NumberOfInstances   = NumberOfInstances;

   /* Format the message that will be dispatched remotely.              */
   Message.MessageHeader.MessageGroup                         = BTPM_MESSAGE_GROUP_BATTERY_SERVICE_MANAGER;
   Message.MessageHeader.MessageFunction                      = BASM_MESSAGE_FUNCTION_CONNECTED;
   Message.MessageHeader.MessageLength                        = (BASM_CONNECTED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);
   Message.RemoteDeviceAddress                                = DeviceEntry->BluetoothAddress;
   Message.ConnectionType                                     = bctClient;
   Message.ConnectedFlags                                     = 0;
   Message.NumberOfInstances                                  = NumberOfInstances;

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch a Disconnected Event to every registered callback.  It is*/
   /* the caller's responsibility to verify the Response before calling */
   /* this function.                                                    */
   /* * NOTE * This function should be called with the Module Manager's */
   /*          Lock held.                                               */
static void DispatchDisconnectedEvent(Device_Entry_t *DeviceEntry)
{
   Callback_Entry_t            *CallbackEntry;
   BASM_Event_Data_t            EventData;
   BASM_Disconnected_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format the event that will be dispatched locally.                 */
   EventData.EventType                                           = betBASDisconnected;
   EventData.EventLength                                         = BASM_DISCONNECTED_EVENT_DATA_SIZE;
   EventData.EventData.DisconnectedEventData.RemoteDeviceAddress = DeviceEntry->BluetoothAddress;
   EventData.EventData.DisconnectedEventData.ConnectionType      = bctClient;

   /* Format the message that will be dispatched remotely.              */
   Message.MessageHeader.MessageGroup                            = BTPM_MESSAGE_GROUP_BATTERY_SERVICE_MANAGER;
   Message.MessageHeader.MessageFunction                         = BASM_MESSAGE_FUNCTION_DISCONNECTED;
   Message.MessageHeader.MessageLength                           = (BASM_DISCONNECTED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);
   Message.RemoteDeviceAddress                                   = DeviceEntry->BluetoothAddress;
   Message.ConnectionType                                        = bctClient;

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* delete the enabled entries for a specified device for all         */
   /* registered callbacks.                                             */
static void DeleteEnabledEntriesForDevice(BD_ADDR_t BD_ADDR)
{
   Callback_Entry_t              *CallbackEntry;
   Notifications_Enabled_Entry_t *EnabledEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if(!COMPARE_NULL_BD_ADDR(BD_ADDR))
   {
      /* Walk the list and delete all enabled entries for this device.  */
      CallbackEntry = CallbackEntryList;
      while(CallbackEntry)
      {
         /* Delete all entries for this device.                         */
         while((EnabledEntry = DeleteNotificationsEnabledEntryBD_ADDR(&(CallbackEntry->NotificationsEnabledEntryList), &BD_ADDR)) != NULL)
            FreeNotificationsEnabledEntryMemory(EnabledEntry);

         CallbackEntry = CallbackEntry->NextCallbackEntry;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* store/delete BASM Device Information for the specified Device to  */
   /* file.                                                             */
static void StoreBASMInstanceCount(BD_ADDR_t RemoteDeviceAddress, unsigned int NumberOfInstances)
{
   char KeyName[BASM_LE_CONFIGURATION_FILE_MAXIMUM_KEY_LENGTH];

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if(!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress))
   {
      /* Build the Key Name for the Battery Level CCCD.                 */
      sprintf(KeyName, BASM_LE_KEY_NAME_BATTERY_LEVEL_INSTANCE_COUNT, RemoteDeviceAddress.BD_ADDR5, RemoteDeviceAddress.BD_ADDR4, RemoteDeviceAddress.BD_ADDR3, RemoteDeviceAddress.BD_ADDR2, RemoteDeviceAddress.BD_ADDR1, RemoteDeviceAddress.BD_ADDR0);

      SET_WriteInteger(BASM_LE_CONFIGURATION_FILE_SECTION_NAME, KeyName, (unsigned int)NumberOfInstances, BTPM_CONFIGURATION_LOW_ENERGY_CONFIGURATION_FILE_NAME);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* store/delete BASM Instance Information for the specified Battery  */
   /* Service Instance to file.                                         */
static void StoreBASMInstanceInformation(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, Boolean_t NotificationsEnabled)
{
   char KeyName[BASM_LE_CONFIGURATION_FILE_MAXIMUM_KEY_LENGTH];

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if(!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress))
   {
      /* Build the Key Name for the Battery Level CCCD.                 */
      sprintf(KeyName, BASM_LE_KEY_NAME_BATTERY_LEVEL_CCCD, InstanceID, RemoteDeviceAddress.BD_ADDR5, RemoteDeviceAddress.BD_ADDR4, RemoteDeviceAddress.BD_ADDR3, RemoteDeviceAddress.BD_ADDR2, RemoteDeviceAddress.BD_ADDR1, RemoteDeviceAddress.BD_ADDR0);

      SET_WriteInteger(BASM_LE_CONFIGURATION_FILE_SECTION_NAME, KeyName, (unsigned int)NotificationsEnabled, BTPM_CONFIGURATION_LOW_ENERGY_CONFIGURATION_FILE_NAME);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* update the Configuration File store for the specified device to   */
   /* the LE Configuration file (if and only if the device is paired).  */
static void UpdateDeviceFile(Device_Entry_t *DeviceEntry)
{
   unsigned int                     InstanceCount;
   unsigned long                    RequiredFlags;
   Instance_Entry_t                *InstanceEntryPtr;
   DEVM_Remote_Device_Properties_t  DeviceProperties;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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
         {
            /* Walk the list and write all of the instances that support*/
            /* notifications to the LE Configuration file.              */
            InstanceEntryPtr = DeviceEntry->InstanceEntryList;
            InstanceCount    = 0;
            while(InstanceEntryPtr)
            {
               /* Store the information for this instance.              */
               if(InstanceEntryPtr->Flags & INSTANCE_ENTRY_FLAG_BATTERY_LEVEL_NOTIFY)
               {
                  StoreBASMInstanceInformation(DeviceEntry->BluetoothAddress, InstanceEntryPtr->InstanceID, InstanceEntryPtr->NotificationsEnabled);

                  InstanceCount++;
               }

               /* Advance to the next instance.                         */
               InstanceEntryPtr = InstanceEntryPtr->NextInstanceEntry;
            }

            /* Store the number of instances that support notifications */
            /* to the file.                                             */
            if(InstanceCount)
               StoreBASMInstanceCount(DeviceEntry->BluetoothAddress, InstanceCount);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* update the Configuration File store for the specified instance    */
   /* from the LE Configuration file.  This function returns TRUE if    */
   /* there is valid data stored for the selected device or false       */
   /* otherwise.                                                        */
static Boolean_t ReloadDeviceEntry(BD_ADDR_t BD_ADDR, unsigned int InstanceID, Boolean_t *NotificationsEnabled)
{
   char         KeyName[BASM_LE_CONFIGURATION_FILE_MAXIMUM_KEY_LENGTH];
   Boolean_t    ret_val = FALSE;
   unsigned int Value;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if(!COMPARE_NULL_BD_ADDR(BD_ADDR))
   {
      /* Build the Key Name for the Blood Pressure CCCD.                */
      sprintf(KeyName, BASM_LE_KEY_NAME_BATTERY_LEVEL_CCCD, InstanceID, BD_ADDR.BD_ADDR5, BD_ADDR.BD_ADDR4, BD_ADDR.BD_ADDR3, BD_ADDR.BD_ADDR2, BD_ADDR.BD_ADDR1, BD_ADDR.BD_ADDR0);

      /* Attempt to read the Blood Pressure CCCD.                       */
      Value = SET_ReadInteger(BASM_LE_CONFIGURATION_FILE_SECTION_NAME, KeyName, 0, BTPM_CONFIGURATION_LOW_ENERGY_CONFIGURATION_FILE_NAME);

      /* Only update the Device Entry if necessary.                     */
      if(Value)
      {
         /* Updated the Device Entry if specified.                      */
         if(NotificationsEnabled)
            *NotificationsEnabled = TRUE;

         /* Make sure we return success.                                */
         ret_val = TRUE;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %u\n", (unsigned int)ret_val));

   return(ret_val);
}

   /* The following function is a utility function that is used to      */
   /* delete the Instance Configuration for the specified device from   */
   /* the BASM Section of the LE Configuration file.                    */
static void DeleteInstanceConfiguration(BD_ADDR_t BD_ADDR)
{
   char         KeyName[BASM_LE_CONFIGURATION_FILE_MAXIMUM_KEY_LENGTH];
   unsigned int Index;
   unsigned int InstanceCount;
   unsigned int InstanceValue;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if(!COMPARE_NULL_BD_ADDR(BD_ADDR))
   {
      /* Build the Key Name for the Instance Count field.               */
      sprintf(KeyName, BASM_LE_KEY_NAME_BATTERY_LEVEL_INSTANCE_COUNT, BD_ADDR.BD_ADDR5, BD_ADDR.BD_ADDR4, BD_ADDR.BD_ADDR3, BD_ADDR.BD_ADDR2, BD_ADDR.BD_ADDR1, BD_ADDR.BD_ADDR0);

      /* Attempt to read the Instance Count.                            */
      InstanceCount = SET_ReadInteger(BASM_LE_CONFIGURATION_FILE_SECTION_NAME, KeyName, 0, BTPM_CONFIGURATION_LOW_ENERGY_CONFIGURATION_FILE_NAME);
      if(InstanceCount)
      {
         /* Reset the instance count in the file.                       */
         SET_WriteInteger(BASM_LE_CONFIGURATION_FILE_SECTION_NAME, KeyName, 0, BTPM_CONFIGURATION_LOW_ENERGY_CONFIGURATION_FILE_NAME);

         /* Read through the instances and delete all that are present. */
         for(Index=0;Index<InstanceCount;Index++)
         {
            /* Build the Key Name for the Instance entry.               */
            sprintf(KeyName, BASM_LE_KEY_NAME_BATTERY_LEVEL_CCCD, Index, BD_ADDR.BD_ADDR5, BD_ADDR.BD_ADDR4, BD_ADDR.BD_ADDR3, BD_ADDR.BD_ADDR2, BD_ADDR.BD_ADDR1, BD_ADDR.BD_ADDR0);

            /* Attempt to read the Instance CCCD.                       */
            InstanceValue = SET_ReadInteger(BASM_LE_CONFIGURATION_FILE_SECTION_NAME, KeyName, 0, BTPM_CONFIGURATION_LOW_ENERGY_CONFIGURATION_FILE_NAME);
            if(InstanceValue)
            {
               /* Reset the value for this instance.                    */
               SET_WriteInteger(BASM_LE_CONFIGURATION_FILE_SECTION_NAME, KeyName, 0, BTPM_CONFIGURATION_LOW_ENERGY_CONFIGURATION_FILE_NAME);
            }
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit:\n"));
}


   /* The following function is the Dispatch Callback function that is  */
   /* registered to process GATT Notification Events.                   */
static void BTPSAPI BTPSDispatchCallback_GATT(void *CallbackParameter)
{
   Device_Entry_t                     *DeviceEntry;
   Instance_Entry_t                   *InstanceEntry;
   GATT_Server_Notification_Data_t    *EventData;
   Battery_Level_Notification_Event_t  BatteryLevelNotificationEvent;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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
            EventData = &(((BASM_Update_Data_t *)CallbackParameter)->UpdateData.GATTServerNotificationData);

            /* Verify that the notification has occurred on the Battery */
            /* Level attribute handle.                                  */
            if((DeviceEntry = SearchDeviceEntry(&(EventData->RemoteDevice))) != NULL)
            {
               if((InstanceEntry = SearchInstanceEntryByAttributeHandle(&(DeviceEntry->InstanceEntryList), BTPS_STRUCTURE_OFFSET(Instance_Entry_t, ServerInformationHandles) + BTPS_STRUCTURE_OFFSET(BAS_Client_Information_t, Battery_Level), EventData->AttributeHandle)) != NULL)
               {
                  if(EventData->AttributeValueLength >= BAS_BATTERY_LEVEL_VALUE_LENGTH)
                  {
                     /* Notification is for Battery Level, go ahead and */
                     /* format up the event and dispatch it to everyone */
                     /* that has enabled notifications for this Battery */
                     /* Instance.                                       */
                     BatteryLevelNotificationEvent.RemoteServer = DeviceEntry->BluetoothAddress;
                     BatteryLevelNotificationEvent.InstanceID   = InstanceEntry->InstanceID;
                     BatteryLevelNotificationEvent.BatteryLevel = EventData->AttributeValue[0];

                     DispatchBatteryLevelNotificationEvent(&BatteryLevelNotificationEvent);
                  }
               }
            }
         }

         /* Release the lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }

      /* Go ahead and free the memory because we are finished with it.  */
      BTPS_FreeMemory(CallbackParameter);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process BAS Client Events.                          */
static void BTPSAPI BTPSDispatchCallback_BAS_Client(void *CallbackParameter)
{
   Transaction_Entry_t            *TransactionEntry;
   GATT_Client_Event_Data_t       *EventData;
   BAS_Presentation_Format_Data_t  PresentationFormatData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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
            EventData = &(((BASM_Update_Data_t *)CallbackParameter)->UpdateData.GATTClientEventData);

            switch(EventData->Event_Data_Type)
            {
               case etGATT_Client_Error_Response:
                  DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_VERBOSE), ("etGATT_Client_Error_Response, TransactionID = %u\n", EventData->Event_Data.GATT_Request_Error_Data->TransactionID));

                  /* Verify that there is an outstanding transaction for*/
                  /* this event.                                        */
                  if((TransactionEntry = DeleteTransactionEntryByGATTTransactionID(EventData->Event_Data.GATT_Request_Error_Data->TransactionID)) != NULL)
                  {
                     switch(TransactionEntry->TransactionType)
                     {
                        case ttBatteryLevel:
                           DispatchBatteryLevelEvent(TransactionEntry, BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID);
                           break;
                        case ttBatteryIdentification:
                           DispatchBatteryIdentificationEvent(TransactionEntry, NULL);
                           break;
                        case ttBatteryLevelCCCD:
                           ProcessBatteryLevelCCCDStateChange(TransactionEntry, FALSE);
                           break;
                        default:
                           DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_VERBOSE), ("etGATT_Client_Error_Response, Unkown Transaction Type = %u\n", (unsigned int)TransactionEntry->TransactionType));
                           break;
                     }

                     FreeTransactionEntryMemory(TransactionEntry);
                  }
                  break;
               case etGATT_Client_Write_Response:
                  DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_VERBOSE), ("etGATT_Client_Write_Response, TransactionID = %u\n", EventData->Event_Data.GATT_Write_Response_Data->TransactionID));

                  /* Verify that there is an outstanding transaction for*/
                  /* this event.                                        */
                  if((TransactionEntry = DeleteTransactionEntryByGATTTransactionID(EventData->Event_Data.GATT_Write_Response_Data->TransactionID)) != NULL)
                  {
                     switch(TransactionEntry->TransactionType)
                     {
                        case ttBatteryLevelCCCD:
                           ProcessBatteryLevelCCCDStateChange(TransactionEntry, TRUE);
                           break;
                        default:
                           DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_VERBOSE), ("etGATT_Client_Write_Response, Unkown Transaction Type = %u\n", (unsigned int)TransactionEntry->TransactionType));
                           break;
                     }

                     FreeTransactionEntryMemory(TransactionEntry);
                  }
                  break;
               case etGATT_Client_Read_Response:
                  DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_VERBOSE), ("etGATT_Client_Read_Response, TransactionID = %u\n", EventData->Event_Data.GATT_Read_Response_Data->TransactionID));

                  /* Verify that there is an outstanding transaction for*/
                  /* this event.                                        */
                  if((TransactionEntry = DeleteTransactionEntryByGATTTransactionID(EventData->Event_Data.GATT_Read_Response_Data->TransactionID)) != NULL)
                  {
                     switch(TransactionEntry->TransactionType)
                     {
                        case ttBatteryLevel:
                           if(EventData->Event_Data.GATT_Read_Response_Data->AttributeValueLength >= BAS_BATTERY_LEVEL_VALUE_LENGTH)
                              DispatchBatteryLevelEvent(TransactionEntry, EventData->Event_Data.GATT_Read_Response_Data->AttributeValue[0]);
                           else
                              DispatchBatteryLevelEvent(TransactionEntry, BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID);
                           break;
                        case ttBatteryIdentification:
                           if(EventData->Event_Data.GATT_Read_Response_Data->AttributeValueLength >= BAS_PRESENTATION_FORMAT_SIZE)
                           {
                              if(BAS_Decode_Characteristic_Presentation_Format(EventData->Event_Data.GATT_Read_Response_Data->AttributeValueLength, EventData->Event_Data.GATT_Read_Response_Data->AttributeValue, &PresentationFormatData) == 0)
                                 DispatchBatteryIdentificationEvent(TransactionEntry, &PresentationFormatData);
                              else
                                 DispatchBatteryIdentificationEvent(TransactionEntry, NULL);
                           }
                           break;
                        default:
                           DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_VERBOSE), ("etGATT_Client_Read_Response, Unkown Transaction Type = %u\n", (unsigned int)TransactionEntry->TransactionType));
                           break;
                     }

                     FreeTransactionEntryMemory(TransactionEntry);
                  }
                  break;
               default:
                  DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Unknown GATT Client event type.\n"));
                  break;
            }
         }

         /* Release the lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }

      /* Go ahead and free the memory because we are finished with it.  */
      BTPS_FreeMemory(CallbackParameter);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following is a utility function used to discover all valid    */
   /* Battery Service Instances on a remote device.  If one or more     */
   /* valid Battery Service Instances is found, then a Device Entry is  */
   /* added to the module's list.  On success, a pointer to the new     */
   /* device entry will be returned; otherwise, NULL will be returned.  */
   /* * NOTE * This function should be called with the Module Manager's */
   /*          Lock held.                                               */
static Device_Entry_t *DiscoverBatteryServerInstances(BD_ADDR_t *BluetoothAddress)
{
   int                                       Result;
   GATT_UUID_t                               UUID;
   unsigned int                              ServiceDataSize;
   unsigned int                              NextInstanceID = 0;
   unsigned int                              Index;
   unsigned int                              Index1;
   unsigned int                              Index2;
   unsigned char                            *ServiceData;
   Device_Entry_t                           *DeviceEntry    = NULL;
   Instance_Entry_t                         *InstanceEntry;
   DEVM_Parsed_Services_Data_t               ParsedGATTData;
   GATT_Characteristic_Information_t        *CharacteristicInformation;
   GATT_Service_Discovery_Indication_Data_t *GATTServiceDiscoveryIndicationData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

                  /* Check for BAS.                                     */
                  if((UUID.UUID_Type == guUUID_16) && (BAS_COMPARE_BAS_SERVICE_UUID_TO_UUID_16(UUID.UUID.UUID_16)))
                  {
                     /* Create a device entry if this is the first      */
                     /* Battery Instance found.                         */
                     if(!DeviceEntry)
                     {
                        if((DeviceEntry = (Device_Entry_t *)BTPS_AllocateMemory(sizeof(Device_Entry_t))) != NULL)
                        {
                           /* Initalize the device entry.               */
                           BTPS_MemInitialize(DeviceEntry, 0, sizeof(Device_Entry_t));
                           DeviceEntry->BluetoothAddress = *BluetoothAddress;
                        }
                     }

                     if(DeviceEntry)
                     {
                        /* Create an instance entry.                    */
                        if((InstanceEntry = (Instance_Entry_t *)BTPS_AllocateMemory(sizeof(Instance_Entry_t))) != NULL)
                        {
                           /* Initalize the instance entry.             */
                           BTPS_MemInitialize(InstanceEntry, 0, sizeof(Instance_Entry_t));

                           GATTServiceDiscoveryIndicationData = &(ParsedGATTData.GATTServiceDiscoveryIndicationData[Index]);

                           /* Iterate the BAS characteristics.          */
                           for(Index1=GATTServiceDiscoveryIndicationData->NumberOfCharacteristics;Index1--;)
                           {
                              CharacteristicInformation = &(GATTServiceDiscoveryIndicationData->CharacteristicInformationList[Index1]);
                              UUID                      = CharacteristicInformation->Characteristic_UUID;

                              if(UUID.UUID_Type == guUUID_16)
                              {
                                 /* Check for Battery Level UUID.       */
                                 if((BAS_COMPARE_BATTERY_LEVEL_UUID_TO_UUID_16(UUID.UUID.UUID_16)) && (CharacteristicInformation->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
                                 {
                                    InstanceEntry->ServerInformationHandles.Battery_Level = CharacteristicInformation->Characteristic_Handle;

                                    /* Flag if notification is          */
                                    /* supported.                       */
                                    if(CharacteristicInformation->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_NOTIFY)
                                    {
                                       InstanceEntry->Flags = INSTANCE_ENTRY_FLAG_BATTERY_LEVEL_NOTIFY;
                                    }

                                    for(Index2=CharacteristicInformation->NumberOfDescriptors;Index2--;)
                                    {
                                       UUID = CharacteristicInformation->DescriptorList[Index2].Characteristic_Descriptor_UUID;

                                       /* We only care about 16-bit     */
                                       /* descriptor UUIDs              */
                                       if(UUID.UUID_Type == guUUID_16)
                                       {
                                          /* Check for Battery Level CCD*/
                                          /* UUID.                      */
                                          if(GATT_COMPARE_CLIENT_CHARACTERISTIC_CONFIGURATION_ATTRIBUTE_TYPE_TO_BLUETOOTH_UUID_16(UUID.UUID.UUID_16))
                                          {
                                             InstanceEntry->ServerInformationHandles.Battery_Level_Client_Configuration = CharacteristicInformation->DescriptorList[Index2].Characteristic_Descriptor_Handle;
                                          }
                                          else
                                          {
                                             /* Check for Battery Level */
                                             /* Presentation Format     */
                                             /* UUID.                   */
                                             if(GATT_COMPARE_CHARACTERISTIC_PRESENTATION_FORMAT_ATTRIBUTE_TYPE_TO_UUID_16(UUID.UUID.UUID_16))
                                             {
                                                InstanceEntry->ServerInformationHandles.Battery_Level_Presentation_Format = CharacteristicInformation->DescriptorList[Index2].Characteristic_Descriptor_Handle;
                                             }
                                          }
                                       }
                                    }
                                 }
                              }
                           }

                           /* At a minimum, BAS must support the Battery*/
                           /* Level Characteristic.  If notify is       */
                           /* supported, then the CCD must be supported.*/
                           /* The presentation format is supported if   */
                           /* multiple instances exist, but this will be*/
                           /* checked after all instances are           */
                           /* discovered.                               */
                           if((InstanceEntry->ServerInformationHandles.Battery_Level) && (!(InstanceEntry->Flags & INSTANCE_ENTRY_FLAG_BATTERY_LEVEL_NOTIFY) || (InstanceEntry->ServerInformationHandles.Battery_Level_Client_Configuration)))
                           {
                              /* Set the Instance ID before adding it to*/
                              /* give the entry uniqueness.             */
                              InstanceEntry->InstanceID = NextInstanceID++;

                              if(!AddInstanceEntryActual(&(DeviceEntry->InstanceEntryList), InstanceEntry))
                              {
                                 BTPS_FreeMemory(InstanceEntry);

                                 DeviceEntry = NULL;
                              }
                           }
                           else
                           {
                              /* Battery Instance is invalid.           */
                              BTPS_FreeMemory(InstanceEntry);

                              DeviceEntry = NULL;
                           }
                        }
                        else
                        {
                           /* Could not create an instance entry.  Free */
                           /* previously created instances and device   */
                           /* and exit.                                 */
                           FreeDeviceEntryMemory(DeviceEntry);

                           DeviceEntry = NULL;
                           break;
                        }
                     }
                     else
                     {
                        break;
                     }
                  }
               }

               /* All finished with the parsed data, so free it.        */
               DEVM_FreeParsedServicesData(&ParsedGATTData);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FAILURE), ("DEVM_ConvertRawServicesStreamToParsedServicesData returned %d.\n", Result));
            }
         }
         else
         {
            DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FAILURE), ("DEVM_QueryRemoteDeviceServices returned %d.\n", Result));
         }

         /* Free the previously allocated buffer holding service data   */
         /* information.                                                */
         BTPS_FreeMemory(ServiceData);
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FAILURE), ("Allocation request for Service Data failed, size = %u.\n", ServiceDataSize));
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FAILURE), ("DEVM_QueryRemoteDeviceServices returned %d.\n", Result));
   }

   /* A device entry was created.  This means that at least one Battery */
   /* Instance has been found on the remote device.                     */
   if(DeviceEntry)
   {
      /* Validate the Presentation Format Descriptors of each instance  */
      /* before adding it to the list.                                  */
      while(TRUE)
      {
         /* Get the number of Battery Instances.                        */
         if((Index = CalculateNumberOfInstances(DeviceEntry->InstanceEntryList)) != 0)
         {
            if(Index == 1)
            {
               /* There is only one Battery Instance, Presentation      */
               /* Format is optional.                                   */
               break;
            }
            else
            {
               /* Validate each Instance.                               */
               for(InstanceEntry = DeviceEntry->InstanceEntryList; InstanceEntry; InstanceEntry = InstanceEntry->NextInstanceEntry)
               {
                  if(InstanceEntry->ServerInformationHandles.Battery_Level_Presentation_Format == 0)
                     break;
               }

               /* If a break occurred, then the Instance Entry is       */
               /* invalid.  Remove it from the list and start over.     */
               if(InstanceEntry)
               {
                  if((InstanceEntry = DeleteInstanceEntry(&(DeviceEntry->InstanceEntryList), InstanceEntry->InstanceID)) != NULL)
                  {
                     FreeInstanceEntryMemory(InstanceEntry);

                     InstanceEntry = NULL;
                  }
               }
               else
               {
                  /* Every instance was validated.  Break and add the   */
                  /* BAS device.                                        */
                  break;
               }
            }
         }
         else
         {
            /* The device does not have any Battery Instances.  Do not  */
            /* add it to the list.                                      */
            FreeDeviceEntryMemory(DeviceEntry);

            DeviceEntry = NULL;

            break;
         }
      }

      /* If the device entry still exists, then it is valid.            */
      if(DeviceEntry)
      {
         /* Instances could be deleted if invalidated.  Re-initalize the*/
         /* Instance ID's.                                              */
         NextInstanceID = 0;

         for(InstanceEntry = DeviceEntry->InstanceEntryList; InstanceEntry; InstanceEntry = InstanceEntry->NextInstanceEntry)
            InstanceEntry->InstanceID = NextInstanceID++;

         /* Add it to the list and notify all PM clients about the      */
         /* device.                                                     */
         if(AddDeviceEntryActual(DeviceEntry))
         {
            DispatchConnectedEvent(DeviceEntry);
         }
         else
         {
            FreeDeviceEntryMemory(DeviceEntry);

            DeviceEntry = NULL;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", DeviceEntry));

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
static int FormatAddTransactionEntry(unsigned int CallbackID, BD_ADDR_t *RemoteServer, unsigned int InstanceID, Transaction_Type_t TransactionType, Transaction_Entry_t **ReturnTransactionEntry)
{
   int                  ret_val;
   Instance_Entry_t    *InstanceEntry;
   Transaction_Entry_t  TransactionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Wait for access to the lock that guards access to this module.    */
   if(DEVM_AcquireLock())
   {
      /* Find the remote Battery Instance.                              */
      if((InstanceEntry = SearchDeviceInstanceEntry(RemoteServer, InstanceID)) != NULL)
      {
         /* Verify that this transaction is unique for the provided     */
         /* callback.                                                   */
         if(!SearchTransactionEntry(RemoteServer, TransactionType, CallbackID))
         {
            /* Populate the Transaction entry.                          */
            TransactionEntry.BluetoothAddress = *RemoteServer;
            TransactionEntry.TransactionID    = GetNextTransactionID();
            TransactionEntry.TransactionType  = TransactionType;
            TransactionEntry.CallbackID       = CallbackID;
            TransactionEntry.AttributeHandle  = *((Word_t *)(((Byte_t *)InstanceEntry) + TransactionTypeToHandleOffsetTable[TransactionType]));
            TransactionEntry.InstanceID       = InstanceID;

            /* Add the transaction to the list.                         */
            if((*ReturnTransactionEntry = AddTransactionEntry(&TransactionEntry)) != NULL)
               ret_val = 0;
            else
               ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
         }
         else
            ret_val = BTPM_ERROR_CODE_BATTERY_SAME_REQUEST_OUTSTANDING;
      }
      else
         ret_val = BTPM_ERROR_CODE_BATTERY_INVALID_INSTANCE;

      /* Release the Lock on failure.                                   */
      if(ret_val)
         DEVM_ReleaseLock();
   }
   else
      ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following is a utility function used to submit a read request */
   /* to the provided Battery Instance given a Transaction Type.  A     */
   /* transaction entry used to identify the response will also be      */
   /* created on success.  A positive Transaction ID will be returned on*/
   /* success; otherwise, a negative error code will be returned.       */
static int SubmitReadValue(unsigned int CallbackID, BD_ADDR_t *RemoteServer, unsigned int InstanceID, Transaction_Type_t TransactionType)
{
   int                  ret_val;
   Transaction_Entry_t *TransactionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Add a transaction entry for this request.                         */
   if(!(ret_val = FormatAddTransactionEntry(CallbackID, RemoteServer, InstanceID, TransactionType, &TransactionEntry)))
   {
      /* Submit the command request.                                    */
      if((ret_val = _BASM_Read_Value(RemoteServer, TransactionEntry->AttributeHandle)) > 0)
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_VERBOSE), ("_BASM_Read_Value(), TransactionID = %u\n", (unsigned int)ret_val));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following is a utility function used to submit a write request*/
   /* to the provided Battery Instance given a Transaction Type.  A     */
   /* transaction entry used to identify the response will also be      */
   /* created on success.  A positive Transaction ID will be returned on*/
   /* success; otherwise, a negative error code will be returned.       */
static int SubmitWriteValue(unsigned int CallbackID, BD_ADDR_t *RemoteServer, unsigned int InstanceID, Transaction_Type_t TransactionType, unsigned int DataLength, Byte_t *Data)
{
   int                  ret_val;
   Transaction_Entry_t *TransactionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Add a transaction entry for this request.                         */
   if(!(ret_val = FormatAddTransactionEntry(CallbackID, RemoteServer, InstanceID, TransactionType, &TransactionEntry)))
   {
      /* Submit the command request.                                    */
      if((ret_val = _BASM_Write_Value(RemoteServer, TransactionEntry->AttributeHandle, (Word_t)DataLength, Data)) > 0)
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_VERBOSE), ("_BASM_Write_Value(), TransactionID = %u\n", (unsigned int)ret_val));

         /* Save the GATT Transaction ID and return our Transaction ID. */
         TransactionEntry->GATTTransactionID = ret_val;
         ret_val                             = (int)TransactionEntry->TransactionID;
      }
      else
      {
         if((TransactionEntry = DeleteTransactionEntry(TransactionEntry->TransactionID)) != NULL)
            FreeTransactionEntryMemory(TransactionEntry);
      }

      DEVM_ReleaseLock();
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following is a utility function used to enable notifications  */
   /* on the provided Battery Instance given a Transaction Type.  Zero  */
   /* will be returned on success; otherwise, a negative error code will*/
   /* be returned.                                                      */
static int EnableNotifications(unsigned int CallbackID, BD_ADDR_t *RemoteServer, unsigned int InstanceID)
{
   int                            ret_val;
   Callback_Entry_t              *CallbackEntryPtr;
   Instance_Entry_t              *InstanceEntry;
   Notifications_Enabled_Entry_t  NotificationsEnabledEntry;
   Notifications_Enabled_Entry_t *NotificationsEnabledEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Wait for access to the lock for this module.                      */
   if(DEVM_AcquireLock())
   {
      /* Search for the callback associated with this request.          */
      if((CallbackEntryPtr = SearchCallbackEntry(CallbackID)) != NULL)
      {
         /* Verify that the instance entry associated with this request */
         /* exists.                                                     */
         if((InstanceEntry = SearchDeviceInstanceEntry(RemoteServer, InstanceID)) != NULL)
         {
            /* Check to see if this instance supports notifications on  */
            /* the battery level characteristic.                        */
            if(InstanceEntry->Flags & INSTANCE_ENTRY_FLAG_BATTERY_LEVEL_NOTIFY)
            {
               /* The instance exists, so add the entry into the enabled*/
               /* notifications list for the specified callback.        */
               NotificationsEnabledEntry.BluetoothAddress = *RemoteServer;
               NotificationsEnabledEntry.InstanceID       = InstanceID;

               if(AddNotificationsEnabledEntry(&(CallbackEntryPtr->NotificationsEnabledEntryList), &NotificationsEnabledEntry))
               {
                  /* Configure the CCD only when the count goes from    */
                  /* zero to positive.                                  */
                  if((!(InstanceEntry->NotificationCount)) && (InstanceEntry->NotificationsEnabled == FALSE))
                  {
                     if((ret_val = WriteBatteryLevelCCCD(RemoteServer, InstanceID, TRUE)) > 0)
                     {
                        InstanceEntry->NotificationCount++;

                        ret_val = 0;
                     }
                     else
                     {
                        /* Remove the entry in the list since           */
                        /* configuring the CCD failed.                  */
                        if((NotificationsEnabledEntryPtr = DeleteNotificationsEnabledEntry(&(CallbackEntryPtr->NotificationsEnabledEntryList), RemoteServer, InstanceID)) != NULL)
                           FreeNotificationsEnabledEntryMemory(NotificationsEnabledEntryPtr);
                     }
                  }
                  else
                  {
                     InstanceEntry->NotificationCount++;

                     ret_val = 0;
                  }
               }
               else
                  ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
            }
            else
               ret_val = BTPM_ERROR_CODE_BATTERY_NOTIFY_UNSUPPORTED;
         }
         else
            ret_val = BTPM_ERROR_CODE_BATTERY_INVALID_INSTANCE;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      /* Release the Lock because we are finished with it.              */
      DEVM_ReleaseLock();
   }
   else
      ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following is a utility function used to disable notifications */
   /* on the provided Battery Instance given a Transaction Type.  Zero  */
   /* will be returned on success; otherwise, a negative error code will*/
   /* be returned.                                                      */
static int DisableNotifications(unsigned int CallbackID, BD_ADDR_t *RemoteServer, unsigned int InstanceID)
{
   int                            ret_val;
   Callback_Entry_t              *CallbackEntryPtr;
   Instance_Entry_t              *InstanceEntry;
   Notifications_Enabled_Entry_t *NotificationsEnabledEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Wait for access to the lock for this module.                      */
   if(DEVM_AcquireLock())
   {
      /* Search for the callback associated with this request.          */
      if((CallbackEntryPtr = SearchCallbackEntry(CallbackID)) != NULL)
      {
         /* Try to delete the requested Notifications Enabled entry from*/
         /* the list.                                                   */
         if((NotificationsEnabledEntry = DeleteNotificationsEnabledEntry(&(CallbackEntryPtr->NotificationsEnabledEntryList), RemoteServer, InstanceID)) != NULL)
         {
            /* Decrement the notification count from the instance.      */
            if((InstanceEntry = SearchDeviceInstanceEntry(RemoteServer, InstanceID)) != NULL)
            {
               /* Configure the CCD only when the count goes from       */
               /* positive to zero.                                     */
               if((InstanceEntry->NotificationCount) && (!(--InstanceEntry->NotificationCount)) && (InstanceEntry->NotificationsEnabled == TRUE))
               {
                  if((ret_val = WriteBatteryLevelCCCD(RemoteServer, InstanceID, FALSE)) > 0)
                     ret_val = 0;
               }
               else
                  ret_val = 0;
            }
            else
               ret_val = BTPM_ERROR_CODE_BATTERY_INVALID_INSTANCE;

            FreeNotificationsEnabledEntryMemory(NotificationsEnabledEntry);
         }
         else
            ret_val = BTPM_ERROR_CODE_BATTERY_NOTIFICATIONS_DISABLED;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      /* Release the Lock because we are finished with it.              */
      DEVM_ReleaseLock();
   }
   else
      ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following is a utility function that submits a Get Battery    */
   /* Level request to the lower level BAS implementation.  The Callback*/
   /* should also be passed in to identify where the response should be */
   /* sent.                                                             */
static int GetBatteryLevel(unsigned int CallbackID, BD_ADDR_t *RemoteServer, unsigned int InstanceID)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   ret_val = SubmitReadValue(CallbackID, RemoteServer, InstanceID, ttBatteryLevel);

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following is a utility function that submits a Get Battery    */
   /* Identification request to the lower level BAS implementation.  The*/
   /* Callback ID should also be passed in to identify where the        */
   /* response should be sent.                                          */
static int GetBatteryIdentification(unsigned int CallbackID, BD_ADDR_t *RemoteServer, unsigned int InstanceID)
{
   int             ret_val;
   Device_Entry_t *DeviceEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Wait for access to the lock for this module.                      */
   if(DEVM_AcquireLock())
   {
      if((DeviceEntry = SearchDeviceEntry(RemoteServer)) != NULL)
      {
         if(CalculateNumberOfInstances(DeviceEntry->InstanceEntryList) > 1)
            ret_val = SubmitReadValue(CallbackID, RemoteServer, InstanceID, ttBatteryIdentification);
         else
            ret_val = BTPM_ERROR_CODE_BATTERY_IDENTIFICATION_UNSUPPORTED;
      }
      else
         ret_val = BTPM_ERROR_CODE_BATTERY_INVALID_INSTANCE;

      /* Release the Lock because we are finished with it.              */
      DEVM_ReleaseLock();
   }
   else
      ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following is a utility function that submits a Cancel         */
   /* Transaction request to the lower level BAS implementation.  Zero  */
   /* is returned on success; otherwise, a negative error value is      */
   /* returned.                                                         */
static int CancelTransaction(unsigned int TransactionID)
{
   int                  ret_val;
   Transaction_Entry_t *TransactionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First try to delete the transaction entry.                        */
   if((TransactionEntry = DeleteTransactionEntry(TransactionID)) != NULL)
   {
      FreeTransactionEntryMemory(TransactionEntry);

      ret_val = _BASM_Cancel_Transaction(TransactionID);
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following is a utility function that submits a Write Battery  */
   /* Level CCD request to the lower level BAS implementation.  A       */
   /* positive Transaction ID is returned on success; otherwise, a      */
   /* negative error value is returned.                                 */
static int WriteBatteryLevelCCCD(BD_ADDR_t *RemoteServer, unsigned int InstanceID, Boolean_t EnableNotifications)
{
   int              ret_val;
   NonAlignedWord_t CCD;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&CCD, EnableNotifications ? GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_NOTIFY_ENABLE : 0);

   ret_val = SubmitWriteValue(0, RemoteServer, InstanceID, ttBatteryLevelCCCD, sizeof(NonAlignedWord_t), (Byte_t *)&CCD);

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function that is used to      */
   /* process a Battery Level CCCD Notification State Change.           */
static void ProcessBatteryLevelCCCDStateChange(Transaction_Entry_t *TransactionEntry, Boolean_t Success)
{
   Device_Entry_t                *DeviceEntry;
   Instance_Entry_t              *InstanceEntry;
   Callback_Entry_t              *CallbackEntryPtr;
   Notifications_Enabled_Entry_t *EnabledEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if(TransactionEntry)
   {
      /* Dispatch the BPM CCCD Event.                                   */
      DispatchBatteryLevelCCCDEvent(TransactionEntry, (Success?0:BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID));

      /* Search for the device entry for the specified pointer.         */
      if((DeviceEntry = SearchDeviceEntry(&(TransactionEntry->BluetoothAddress))) != NULL)
      {
         /* Search for the instance for this event.                     */
         if((InstanceEntry = SearchInstanceEntry(&(DeviceEntry->InstanceEntryList), TransactionEntry->InstanceID)) != NULL)
         {
            /* If successfull update the enabled entry for this device. */
            if(Success)
            {
               /* Flag that indications are enabled.                    */
               InstanceEntry->NotificationsEnabled = TRUE;

               /* Update the device file if necessary.                  */
               UpdateDeviceFile(DeviceEntry);
            }
            else
            {
               /* We failed to write the CCCD so decrement the count.   */
               if(InstanceEntry->NotificationCount)
                  --(InstanceEntry->NotificationCount);

               /* Delete the enabled callback entry.                    */
               if((CallbackEntryPtr = SearchCallbackEntry(TransactionEntry->CallbackID)) != NULL)
               {
                  /* Remove the entry in the list since configuring the */
                  /* CCD failed.                                        */
                  if((EnabledEntryPtr = DeleteNotificationsEnabledEntry(&(CallbackEntryPtr->NotificationsEnabledEntryList), &(DeviceEntry->BluetoothAddress), InstanceEntry->InstanceID)) != NULL)
                     FreeNotificationsEnabledEntryMemory(EnabledEntryPtr);
               }
            }
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function represents the Message Group Handler       */
   /* function that is installed to process all Module Manager Messages.*/
static void BTPSAPI BASManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the Message specified appears to be semi-valid.   */
   if(Message)
   {
      /* Next, check to see if this message is specified for our        */
      /* specific Message Group.                                        */
      if(Message->MessageHeader.MessageGroup == BTPM_MESSAGE_GROUP_BATTERY_SERVICE_MANAGER)
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_VERBOSE), ("BAS Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

         /* Message is destined for us, make sure we are still          */
         /* processing messages.                                        */
         if(Initialized)
         {
            /* Check to see if this message is a BAS Manager defined    */
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
               /* BAS Manager Thread.                                   */
               if(!BTPM_QueueMailboxCallback(BTPMDispatchCallback_BASM, (void *)Message))
               {
                  DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

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
                        DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue Message: 0x%08X\n", Message->MessageHeader.MessageFunction));
                     }
                  }
               }
            }
         }
         else
         {
            /* Check to see if this message is an BAS Manager defined   */
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
         DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message: 0x%08X, 0x%08X\n", Message->MessageHeader.MessageGroup, Message->MessageHeader.MessageFunction));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for initializing/cleaning up*/
   /* this Bluetopia Platform Manager Module.  This function should be  */
   /* registered with the Bluetopia Platform Manager Module Handler and */
   /* will be called when the Platform Manager is initialized (or shut  */
   /* down).                                                            */
void BTPSAPI BASM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData)
{
   int Result;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %d\n", Initialize));

   /* First, determine if the request is to initialize or shut down.    */
   if(Initialize)
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_VERBOSE), ("Initialization Request\n"));

      if(!Initialized)
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing BAS Manager\n"));

         /* Next, let's attempt to register our Message Group Handler to*/
         /* process BAS Manager messages.                               */
         if(!MSG_RegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_BATTERY_SERVICE_MANAGER, BASManagerGroupHandler, NULL))
         {
            /* Initialize the BAS Manager Implementation Module (this is*/
            /* the module that is responsible for implementing the BAS  */
            /* Manager functionality - this module is just the framework*/
            /* shell).                                                  */
            if(!(Result = _BASM_Initialize()))
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
            _BASM_Cleanup();

            MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_BATTERY_SERVICE_MANAGER);
         }
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_VERBOSE), ("BAS Manager Already initialized\n"));
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_VERBOSE), ("Shutdown Request\n"));

      if(Initialized)
      {
         /* First, let's make sure that we un-register our Message Group*/
         /* Handler (so that we do not process any more incoming        */
         /* messages).                                                  */
         MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_BATTERY_SERVICE_MANAGER);

         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Make sure we inform the BAS Manager Implementation that  */
            /* we are shutting down.                                    */
            _BASM_Cleanup();

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing asynchronous */
   /* Device Manager (DEVM) Events (including Power On/Off events).     */
   /* This function should be registered with the Bluetopia Platform    */
   /* Manager Module Handler and will be called when an asynchronous    */
   /* Device Manager event is dispatched.                               */
void BTPSAPI BASM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData)
{
   int Result;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the BAS Manager has been initialized.   */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         switch(EventData->EventType)
         {
            case detDevicePoweredOn:
               DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power On Occurred.\n"));

               /* Set the Bluetooth ID in the lower level module if     */
               /* available.                                            */
               if((Result = DEVM_QueryDeviceBluetoothStackID()) > 0)
                  _BASM_SetBluetoothStackID((unsigned int)Result);
               break;
            case detDevicePoweringOff:
            case detDevicePoweredOff:
               DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power Off Occurred.\n"));

               /* Clear the Bluetooth ID in the lower level module .    */
               _BASM_SetBluetoothStackID(0);

               /* Free the Device and Transaction Entry list.           */
               FreeDeviceEntryList();
               FreeTransactionEntryList();
               break;
            case detRemoteDevicePropertiesChanged:
               DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("detRemoteDevicePropertiesChanged\n"));

               if(EventData->EventLength >= DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_EVENT_DATA_SIZE)
               {
                  /* Process the Remote Device Properties Changed Event.*/
                  ProcessRemoteDevicePropertiesChangedEvent(EventData->EventData.RemoteDevicePropertiesChangedEventData.ChangedMemberMask, &(EventData->EventData.RemoteDevicePropertiesChangedEventData.RemoteDeviceProperties));
               }
               break;
            case detRemoteDeviceDeleted:
               DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("detRemoteDeviceDeleted\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow the caller the ability*/
   /* to notify the Module Manager of a specific Update Event.  The     */
   /* Module Manager can then take the correct action to process the    */
   /* update.                                                           */
Boolean_t BASM_NotifyUpdate(BASM_Update_Data_t *UpdateData)
{
   Boolean_t ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, make sure that the input parameters appear to be           */
   /* semi-valid.                                                       */
   if(UpdateData)
   {
      switch(UpdateData->UpdateType)
      {
         case utGATTNotificationEvent:
            DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_VERBOSE), ("Queueing GATT Notification Event\n"));

            /* Simply Queue the Dispatch Data.                          */
            /* * NOTE * To avoid copying again, we will simply submit   */
            /*          the entire Notification Message.                */
            ret_val = BTPM_QueueMailboxCallback(BTPSDispatchCallback_GATT, (void *)UpdateData);
            break;
         case utGATTClientEvent:
            DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_VERBOSE), ("Queueing Client Event: %d\n", UpdateData->UpdateData.GATTClientEventData.Event_Data_Type));

            /* Simply Queue the Dispatch Data.                          */
            /* * NOTE * To avoid copying again, we will simply submit   */
            /*          the entire Notification Message.                */
            ret_val = BTPM_QueueMailboxCallback(BTPSDispatchCallback_BAS_Client, (void *)UpdateData);
            break;
         default:
            DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled Notification: %d\n", UpdateData->UpdateType));

            ret_val = FALSE;
            break;
      }
   }
   else
      ret_val = FALSE;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the Battery Service  */
   /* Manager (BASM).  This Callback will be dispatched by the BAS      */
   /* Manager when various BAS Manager Events occur.  This function     */
   /* accepts the Callback Function and Callback Parameter              */
   /* (respectively) to call when a BAS Manager Event needs to be       */
   /* dispatched.  This function returns a positive (non-zero) value if */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          BASM_Un_Register_Client_Event_Callback() function to     */
   /*          un-register the callback from this module.               */
int BTPSAPI BASM_Register_Client_Event_Callback(BASM_Event_Callback_t CallbackFunction, void *CallbackParameter)
{
   int               ret_val;
   Callback_Entry_t *CallbackEntryPtr;
   Callback_Entry_t  CallbackEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

            if((CallbackEntryPtr = AddCallbackEntry(&CallbackEntry)) != NULL)
            {
               /* Give the callback an ID and indicate success.         */
               CallbackEntryPtr->CallbackID = GetNextCallbackID();
               ret_val                      = CallbackEntryPtr->CallbackID;
            }
            else
               ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;

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
      ret_val = BTPM_ERROR_CODE_BATTERY_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered BAS Manager Event Callback    */
   /* (registered via a successful call to the                          */
   /* BASM_Register_Client_Event_Callback() function).  This function   */
   /* accepts as input the BAS Manager Event Callback ID (return value  */
   /* from BASM_Register_Client_Event_Callback() function).             */
void BTPSAPI BASM_Un_Register_Client_Event_Callback(unsigned int ClientCallbackID)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Serial Port Manager has been        */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         DeleteCallbackEntry(ClientCallbackID);

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit:\n"));
}

   /* The following function is provided to allow a mechanism to enable */
   /* notifications on a specified Battery Server instance.  This       */
   /* function accepts as input the Callback ID (return value from      */
   /* BASM_Register_Client_Event_Callback() function) as the first      */
   /* parameter.  The second parameter is the Bluetooth Address of the  */
   /* remote device to request the Battery Level from.  The third       */
   /* parameter is the Instance ID of the Battery Server.  This function*/
   /* returns zero on success; otherwise, a negative error value is     */
   /* returned.                                                         */
   /* * NOTE * Battery Level Notification support is optional for a     */
   /*          given Battery Server Instance.  This function will return*/
   /*          BTPM_ERROR_CODE_BATTERY_NOTIFY_UNSUPPORTED if the        */
   /*          specified instance does not support Battery Level        */
   /*          Notifications.                                           */
int BTPSAPI BASM_Enable_Notifications(unsigned int ClientCallbackID, BD_ADDR_t RemoteServer, unsigned int InstanceID)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY  | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the module has been initialized.        */
   if(Initialized)
      ret_val = EnableNotifications(ClientCallbackID, &RemoteServer, InstanceID);
   else
      ret_val = BTPM_ERROR_CODE_BATTERY_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to disable*/
   /* notifications on a specified Battery Server instance.  This       */
   /* function accepts as input the Callback ID (return value from      */
   /* BASM_Register_Client_Event_Callback() function) as the first      */
   /* parameter.  The second parameter is the Bluetooth Address of the  */
   /* remote device to request the Battery Level from.  The third       */
   /* parameter is the Instance ID of the Battery Server.  This function*/
   /* returns zero on success; otherwise, a negative error value is     */
   /* returned.                                                         */
int BTPSAPI BASM_Disable_Notifications(unsigned int ClientCallbackID, BD_ADDR_t RemoteServer, unsigned int InstanceID)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY  | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the module has been initialized.        */
   if(Initialized)
      ret_val = DisableNotifications(ClientCallbackID, &RemoteServer, InstanceID);
   else
      ret_val = BTPM_ERROR_CODE_BATTERY_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to submit */
   /* a Get Battery Level Request to a remote server.  This function    */
   /* accepts as input the Callback ID (return value from               */
   /* BASM_Register_Client_Event_Callback() function) as the first      */
   /* parameter.  The second parameter is the Bluetooth Address of the  */
   /* remote device to request the Battery Level from.  The third       */
   /* parameter is the Instance ID of the Battery Server.  This function*/
   /* returns a positive Transaction ID on success; otherwise, a        */
   /* negative error value is returned.                                 */
int BTPSAPI BASM_Get_Battery_Level(unsigned int ClientCallbackID, BD_ADDR_t RemoteServer, unsigned int InstanceID)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY  | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the module has been initialized.        */
   if(Initialized)
      ret_val = GetBatteryLevel(ClientCallbackID, &RemoteServer, InstanceID);
   else
      ret_val = BTPM_ERROR_CODE_BATTERY_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to submit */
   /* a Get Battery Identification Request to a remote server.  This    */
   /* will retrieve the Namespace and Description values from the       */
   /* Presentation Format Descriptor of the remote server.  This        */
   /* function accepts as input the Callback ID (return value from      */
   /* BASM_Register_Client_Event_Callback() function) as the first      */
   /* parameter.  The second parameter is the Bluetooth Address of the  */
   /* remote device to request the Identification information from.  The*/
   /* third parameter is the Instance ID of the Battery Server.  This   */
   /* function returns a positive Transaction ID on success; otherwise, */
   /* a negative error value is returned.                               */
   /* * NOTE * Identification Information is only available on a remote */
   /*          device that have multiple BAS Server Instances.  This    */
   /*          function will return                                     */
   /*          BTPM_ERROR_CODE_BATTERY_IDENTIFICATION_UNSUPPORTED if the*/
   /*          specified remote device does not have multiple BAS Server*/
   /*          Instances.                                               */
int BTPSAPI BASM_Get_Battery_Identification(unsigned int ClientCallbackID, BD_ADDR_t RemoteServer, unsigned int InstanceID)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY  | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the module has been initialized.        */
   if(Initialized)
      ret_val = GetBatteryIdentification(ClientCallbackID, &RemoteServer, InstanceID);
   else
      ret_val = BTPM_ERROR_CODE_BATTERY_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to submit */
   /* a Cancel Transaction Request to a remote server.  This function   */
   /* accepts as input the Callback ID (return value from               */
   /* BASM_Register_Client_Event_Callback() function) as the first      */
   /* parameter.  The second parameter is the Transaction ID returned by*/
   /* a previously called function in this module.  This function       */
   /* returns zero on success; otherwise, a negative error value is     */
   /* returned.                                                         */
int BTPSAPI BASM_Cancel_Transaction(unsigned int ClientCallbackID, unsigned int TransactionID)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY  | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the module has been initialized.        */
   if(Initialized)
   {
      /* Wait for access to the lock for this module.                   */
      if(DEVM_AcquireLock())
      {
         /* Check for a registered callback.                            */
         if(SearchCallbackEntry(ClientCallbackID) != NULL)
         {
            /* Cancel the transaction.                                  */
            ret_val = CancelTransaction(TransactionID);
         }
         else
            ret_val = BTPM_ERROR_CODE_BATTERY_CALLBACK_NOT_REGISTERED;

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      ret_val = BTPM_ERROR_CODE_BATTERY_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_BATTERY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}
