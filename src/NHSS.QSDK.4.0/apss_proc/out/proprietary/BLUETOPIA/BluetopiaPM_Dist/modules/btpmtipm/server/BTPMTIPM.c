/*****< btpmtipm.c >***********************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMTIPM - TIP Manager for Stonestreet One Bluetooth Protocol Stack       */
/*             Platform Manager.                                              */
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

#include "BTPMTIPM.h"            /* BTPM TIP Manager Prototypes/Constants.    */
#include "TIPMAPI.h"             /* TIP Manager Prototypes/Constants.         */
#include "TIPMMSG.h"             /* BTPM TIP Manager Message Formats.         */
#include "TIPMGR.h"              /* TIP Manager Impl. Prototypes/Constants.   */

#include "SS1BTPM.h"             /* BTPM Main Prototypes and Constants.       */
#include "BTPMERR.h"             /* BTPM Error Prototypes/Constants.          */
#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

   /* The following defines the TIPM LE Configuration File Section Name.*/
#define TIPM_LE_CONFIGURATION_FILE_SECTION_NAME                   "TIPM"

   /* The following define the Key Names that are used with the TIPM    */
   /* Configuration File.                                               */
#define TIPM_KEY_NAME_CCCD_PREFIX                                 "TIPM_%02X%02X%02X%02X%02X%02X"
#define TIPM_KEY_NAME_PERSISTENT_UID                              "PU"

   /* The following defines the size of a Persistent UID that is stored */
   /* in the configuration file.                                        */
#define TIPM_PERSISTENT_UID_SIZE                                  (NON_ALIGNED_DWORD_SIZE + (NON_ALIGNED_WORD_SIZE*2))

   /* Structure which is used to track Callback Information related to  */
   /* this module.                                                      */
typedef struct _tagTIPM_Event_Callback_Info_t
{
   unsigned int                           EventCallbackID;
   unsigned int                           ClientID;
   TIPM_Event_Callback_t                  EventCallback;
   void                                  *CallbackParameter;
   struct _tagTIPM_Event_Callback_Info_t *NextTIPMEventCallbackInfoPtr;
} TIPM_Event_Callback_Info_t;

   /* Container structure used when dispatching registered callbacks.   */
typedef struct _tagCallback_Info_t
{
   unsigned int           ClientID;
   unsigned int           EventCallbackID;
   TIPM_Event_Callback_t  EventCallback;
   void                  *CallbackParameter;
} Callback_Info_t;

   /* Container which holds all of the characteristic handles for a TIP */
   /* Client connection.                                                */
typedef struct _tagClient_Connection_Info_t
{
   unsigned int SupportedServicesMask;
   Word_t       CurrentTimeHandle;
   Word_t       CurrentTimeCCD;
   Word_t       LocalTimeInformationHandle;
   Word_t       ReferenceTimeInformationHandle;
   Word_t       TimeWithDSTHandle;
   Word_t       TimeUpdateControlPointHandle;
   Word_t       TimeUpdateStateHandle;
} Client_Connection_Info_t;

   /* Structure which is used to hold all of the binary information that*/
   /* is stored to file for a paired device.                            */
typedef struct _tagConnection_Binary_Entry_t
{
   NonAlignedWord_t         CurrentTimeCCCD;
} Connection_Binary_Entry_t;

   /* Structure which is used to track information pertaining to        */
   /* incoming connection requests.                                     */
typedef struct _tagConnection_Entry_t
{
   TIPM_Connection_Type_t         ConnectionType;
   BD_ADDR_t                      BD_ADDR;
   unsigned int                   ConnectionID;
   unsigned int                   ReferenceTimeRequestTransactionID;
   Boolean_t                      ClientConnectedDispatched;
   Word_t                         CurrentTimeCCCD;
   Client_Connection_Info_t       ClientConnectionInfo;
   unsigned long                  Flags;
   struct _tagConnection_Entry_t *NextConnectionEntryPtr;
} Connection_Entry_t;

#define CONNECTION_ENTRY_FLAGS_NOTIFICATION_ENABLE_PENDING     0x00000001
#define CONNECTION_ENTRY_FLAGS_NOTIFICATION_ENABLED            0x00000002
#define CONNECTION_ENTRY_FLAGS_NOTIFICATION_DISABLE_PENDING    0x00000004

   /* Structure which is used to track clients which have registered for*/
   /* notifications of the current time.                                */
typedef struct _tagNotification_Registration_Entry_t
{
   unsigned int                                  CallbackID;
   BD_ADDR_t                                     BD_ADDR;
   struct _tagNotification_Registration_Entry_t *NextNotificationRegistrationEntryPtr;
} Notification_Registration_Entry_t;

   /* Enum which tracks the type of ongoing transaction in the          */
   /* transaction list.                                                 */
typedef enum
{
   ttGetCurrentTime,
   ttEnableNotifications,
   ttDisableNotifications,
   ttGetLocalTimeInformation,
   ttGetTimeAccuracy,
   ttGetNextDSTChangeInformation,
   ttGetReferenceTimeUpdateState,
   ttRequestReferenceTimeUpdate
} Transaction_Type_t;

typedef struct _tagTransaction_Entry_t
{
   unsigned int                    TransactionID;
   unsigned int                    GATTTransactionID;
   Transaction_Type_t              TransactionType;
   BD_ADDR_t                       BD_ADDR;
   unsigned int                    CallbackID;
   struct _tagTransaction_Entry_t *NextTransactionEntryPtr;
} Transaction_Entry_t;

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the compiler*/
   /* as part of standard C/C++).                                       */

   /* Variable which serves as a global flag about whether this module  */
   /* is initialized or not.                                            */
static Boolean_t Initialized;

   /* Varaible which holds the currently supported roles of this TIP    */
   /* modules.                                                          */
static unsigned int SupportedRoles;

   /* Variable which serves as a global flag that tells us if a         */
   /* Reference Time Request is currently outstanding.                  */
static Boolean_t ReferenceTimeRequestOutstanding;

   /* Variable which is used to hold the next (unique) Callback ID.     */
static unsigned int NextEventCallbackID;

   /* Variable which is used to hold the next (unique) Transaction ID.  */
static unsigned int NextTransactionID;

   /* Variable which holds the current Power State of the Device.       */
static Boolean_t CurrentPowerState;

   /* Variable which holds a pointer to the first element in the Generic*/
   /* Attribute Profile Callback Info List (which holds all TIPM Server */
   /* Event Callbacks registered with this module).                     */
static TIPM_Event_Callback_Info_t *EventCallbackInfoList;

   /* Variable which holds a pointer to the first element in the Generic*/
   /* Attribute Profile Callback Info List (which holds all TIPM Client */
   /* Event Callbacks registered with this module).                     */
static TIPM_Event_Callback_Info_t *ClientEventCallbackInfoList;

   /* Variable which holds a pointer to the first element of the        */
   /* Connection Information List (which holds all currently active     */
   /* connections).                                                     */
static Connection_Entry_t *ConnectionEntryList;

   /* Variable which holds a pointer to the first element of the        */
   /* Notification Registration List (which holds all clients registered*/
   /* to receive Current Time notifications.                            */
static Notification_Registration_Entry_t *NotificationRegistrationList;

   /* Varaible which holds a pointer to the first element of the        */
   /* Transaction List (which holds all outstanding GATT transactions). */
static Transaction_Entry_t *TransactionList;

   /* Internal Function Prototypes.                                     */
static unsigned int GetNextEventCallbackID(void);
static unsigned int GetNextTransactionID(void);

static TIPM_Event_Callback_Info_t *AddEventCallbackInfoEntry(TIPM_Event_Callback_Info_t **ListHead, TIPM_Event_Callback_Info_t *EntryToAdd);
static TIPM_Event_Callback_Info_t *SearchEventCallbackInfoEntry(TIPM_Event_Callback_Info_t **ListHead, unsigned int EventCallbackID);
static TIPM_Event_Callback_Info_t *DeleteEventCallbackInfoEntry(TIPM_Event_Callback_Info_t **ListHead, unsigned int EventCallbackID);
static void FreeEventCallbackInfoEntryMemory(TIPM_Event_Callback_Info_t *EntryToFree);
static void FreeEventCallbackInfoList(TIPM_Event_Callback_Info_t **ListHead);

static Connection_Entry_t *AddConnectionEntry(Connection_Entry_t **ListHead, Connection_Entry_t *EntryToAdd);
static Connection_Entry_t *SearchConnectionEntry(Connection_Entry_t **ListHead, unsigned int ConnectionID, TIPM_Connection_Type_t ConnectionType);
static Connection_Entry_t *SearchConnectionEntryByBD_ADDR(Connection_Entry_t **ListHead, BD_ADDR_t BD_ADDR, TIPM_Connection_Type_t ConnectionType);
static Connection_Entry_t *DeleteConnectionEntryByBD_ADDR(Connection_Entry_t **ListHead, BD_ADDR_t BD_ADDR, TIPM_Connection_Type_t ConnectionType);
static void FreeConnectionEntryMemory(Connection_Entry_t *EntryToFree);
static void FreeConnectionEntryList(Connection_Entry_t **ListHead);

static Connection_Entry_t *SearchAddConnectionEntry(Connection_Entry_t **ListHead, TIPM_Connection_Type_t ConnectionType, BD_ADDR_t BD_ADDR, unsigned int ConnectionID);

static Notification_Registration_Entry_t *AddNotificationRegistrationEntry(Notification_Registration_Entry_t **ListHead, Notification_Registration_Entry_t *EntryToAdd);
static Notification_Registration_Entry_t *SearchNotificationRegistrationEntry(Notification_Registration_Entry_t **ListHead, unsigned int *CallbackID, BD_ADDR_t *BD_ADDR);
static Notification_Registration_Entry_t *DeleteNotificationRegistrationEntry(Notification_Registration_Entry_t **ListHead, unsigned int *CallbackID, BD_ADDR_t *BD_ADDR);
static void FreeNotificationRegistrationEntryMemory(Notification_Registration_Entry_t *EntryToFree);
static void FreeNotificationRegistrationList(Notification_Registration_Entry_t **ListHead);

static Transaction_Entry_t *AddTransactionEntry(Transaction_Entry_t **ListHead, Transaction_Entry_t *EntryToAdd);
static Transaction_Entry_t *DeleteTransactionEntry(Transaction_Entry_t **ListHead, unsigned int GATTTransactionID);
static void FreeTransactionEntryMemory(Transaction_Entry_t *EntryToFree);
static void FreeTransactionList(Transaction_Entry_t **ListHead);

static void StoreConnectionConfiguration(Connection_Entry_t *ConnectionEntry, Boolean_t Store);
static void ReloadConnectionConfiguration(Connection_Entry_t *ConnectionEntry);

static void StorePersistentUID(DWord_t PersistentUID, GATT_Attribute_Handle_Group_t *ServiceHandleRange, Boolean_t Store);
static Boolean_t ReloadPersistentUID(DWord_t *PersistentUID, GATT_Attribute_Handle_Group_t *ServiceHandleRange);

static Boolean_t CalculateServiceHandleRange(GATT_Attribute_Handle_Group_t *ServiceHandleRange);

static int ConvertTimeSourceToCTS(TIPM_Time_Source_Type_t Source, CTS_Time_Source_Type_t *TimeSourceResult);
static int ConvertDSTOffsetToCTS(TIPM_DST_Offset_Type_t DSTOffset, CTS_DST_Offset_Type_t *DSTOffsetResult);
static int ConvertTimeZoneToCTS(TIPM_Time_Zone_Type_t TimeZone, CTS_Time_Zone_Type_t *TimeZoneResult);
static int ConvertCTSToTimeZone(CTS_Time_Zone_Type_t TimeZone, TIPM_Time_Zone_Type_t *TimeZoneResult);

static int ConvertReferenceTimeToCTS(TIPM_Reference_Time_Information_Data_t *ReferenceTime, CTS_Reference_Time_Information_Data_t *ReferenceTimeResult);

static int ConvertCTSToCurrentTime(CTS_Current_Time_Data_t *CurrentTime, TIPM_Current_Time_Data_t *CurrentTimeResult);
static int ConvertCTSToLocalTime(CTS_Local_Time_Information_Data_t *LocalTime, TIPM_Local_Time_Information_Data_t *LocalTimeResult);
static int ConvertCTSToReferenceTime(CTS_Reference_Time_Information_Data_t *ReferenceTime, TIPM_Reference_Time_Information_Data_t *ReferenceTimeResult);

static int ConvertNDCSToTimeWithDST(NDCS_Time_With_Dst_Data_t *TimeWithDST, TIPM_Time_With_DST_Data_t *TimeWithDSTResult);

static int ConvertRTUSToTimeUpdateState(RTUS_Time_Update_State_Data_t *TimeUpdateState, TIPM_Time_Update_State_Data_t *TimeUpdateStateResult);

static int ProcessSetLocalTimeInformation(TIPM_Local_Time_Information_Data_t *LocalTimeInformation);
static int ProcessReferenceTimeInformationResponse(TIPM_Reference_Time_Information_Data_t *ReferenceTimeInformation);
static int ProcessUpdateCurrentTime(unsigned int TransactionID, unsigned long AdjustMask);

static int ProcessGetCurrentTime(unsigned int ClientEventHandlerID, BD_ADDR_t BD_ADDR);
static int ProcessEnableTimeNotifications(unsigned int ClientEventHandlerID, BD_ADDR_t BD_ADDR, Boolean_t Enable);
static int ProcessGetLocalTimeInformation(unsigned int ClientEventHandlerID, BD_ADDR_t BD_ADDR);
static int ProcessGetTimeAccuracy(unsigned int ClientEventHandlerID, BD_ADDR_t BD_ADDR);
static int ProcessGetNextDSTChange(unsigned int ClientEventHandlerID, BD_ADDR_t BD_ADDR);
static int ProcessGetReferenceTimeUpdateState(unsigned int ClientEventHandlerID, BD_ADDR_t BD_ADDR);
static int ProcessRequestReferenceTimeUpdate(unsigned int ClientEventHandlerID, BD_ADDR_t BD_ADDR);
static int ProcessQueryConnectedDevices(TIPM_Connection_Type_t ConnectionType, unsigned int MaximumNumberDevices, TIPM_Remote_Device_t *RemoteDevices, unsigned int *TotalNumberDevices);

static void ConfigureCTSClient(Connection_Entry_t *ConnectionEntry, GATT_Service_Discovery_Indication_Data_t *Service);
static void ConfigureNDCSClient(Connection_Entry_t *ConnectionEntry, GATT_Service_Discovery_Indication_Data_t *Service);
static void ConfigureRTUSClient(Connection_Entry_t *ConnectionEntry, GATT_Service_Discovery_Indication_Data_t *Service);
static void ConfigureTIPClientConnection(Connection_Entry_t *ConnectionEntry);

static void DispatchTIPMEvent(TIPM_Event_Data_t *TIPMEventData, BTPM_Message_t *Message, unsigned int *CallbackHandlerID, unsigned int *MessageHandlerID, TIPM_Connection_Type_t ConnectionType);
static void DispatchTIPMEventByID(TIPM_Event_Data_t *TIPMEventData, BTPM_Message_t *Message, unsigned int CallbackID, TIPM_Connection_Type_t ConnectionType);

static void DispatchTIPConnectionEvent(Connection_Entry_t *ConnectionEntry);
static void DispatchTIPDisconnectionEvent(Connection_Entry_t *ConnectionEntry);
static void DispatchTIPReferenceTimeRequestEvent(void);

static void ProcessRegisterServerEventsRequestMessage(TIPM_Register_Server_Events_Request_t *Message);
static void ProcessUnRegisterServerEventsRequestMessage(TIPM_Un_Register_Server_Events_Request_t *Message);
static void ProcessSetLocalTimeInformationRequestMessage(TIPM_Set_Local_Time_Information_Request_t *Message);
static void ProcessUpdateCurrentTimeRequestMessage(TIPM_Update_Current_Time_Request_t *Message);
static void ProcessReferenceTimeResponseMessage(TIPM_Reference_Time_Response_Request_t *Message);

static void ProcessRegisterClientEventsRequestMessage(TIPM_Register_Client_Events_Request_t *Message);
static void ProcessUnRegisterClientEventsRequestMessage(TIPM_Un_Register_Client_Events_Request_t *Message);
static void ProcessGetCurrentTimeRequestMessage(TIPM_Get_Current_Time_Request_t *Message);
static void ProcessEnableTimeNotificationsRequestMessage(TIPM_Enable_Time_Notifications_Request_t *Message);
static void ProcessGetLocalTimeInformationRequestMessage(TIPM_Get_Local_Time_Information_Request_t *Message);
static void ProcessGetTimeAccuracyRequestMessage(TIPM_Get_Time_Accuracy_Request_t *Message);
static void ProcessGetNextDSTChangeInformationRequestMessage(TIPM_Get_Next_DST_Change_Information_Request_t *Message);
static void ProcessGetReferenceTimeUpdateStateRequestMessage(TIPM_Get_Reference_Time_Update_State_Request_t *Message);
static void ProcessRequestReferenceTimeUpdateRequestMessage(TIPM_Request_Reference_Time_Update_Request_t *Message);
static void ProcessQueryConnectedDevicesRequestMessage(TIPM_Query_Connected_Devices_Request_t *Message);

static void ProcessReceivedMessage(BTPM_Message_t *Message);
static void ProcessClientUnRegister(unsigned int ClientID);

static void ProcessCTSReadConfigurationRequestEvent(CTS_Read_Client_Configuration_Data_t *ReadClientConfigurationData);
static void ProcessCTSConfigurationUpdateEvent(CTS_Client_Configuration_Update_Data_t *ClientConfigurationUpdateData);
static void ProcessCTSReadCurrentTimeRequest(CTS_Read_Current_Time_Request_Data_t *ReadCurrentTimeRequestData);
static void ProcessCTSReadReferenceTimeRequest(CTS_Read_Reference_Time_Information_Request_Data_t *ReadReferenceTimeRequestData);

static void ProcessCTSServerEvent(CTS_Server_Event_Data_t *CTSServerEventData);

static void ProcessGetCurrentTimeResponse(GATT_Read_Response_Data_t *GATTReadResponseData, Transaction_Entry_t *TransactionEntry, unsigned int Status);
static void ProcessCurrentTimeNotification(GATT_Server_Notification_Data_t *GATTServerNotificationData, Notification_Registration_Entry_t *NotificationRegistrationEntry);
static void ProcessLocalTimeInformationResponse(GATT_Read_Response_Data_t *GATTReadResponseData, Transaction_Entry_t *TransactionEntry, unsigned int Status);
static void ProcessTimeAccuracyResponse(GATT_Read_Response_Data_t *GATTReadResponseData, Transaction_Entry_t *TransactionEntry, unsigned int Status);
static void ProcessNextDSTChangeResponse(GATT_Read_Response_Data_t *GATTReadResponseData, Transaction_Entry_t *TransactionEntry, unsigned int Status);
static void ProcessTimeUpdateStateResponse(GATT_Read_Response_Data_t *GATTReadResponseData, Transaction_Entry_t *TransactionEntry, unsigned int Status);

static void CleanupNotificationRegistration(Connection_Entry_t *ConnectionEntry);
static void CleanupNotificationRegistrationByCallbackID(unsigned int CallbackID);

static void ProcessGATTReadResponseEvent(GATT_Read_Response_Data_t *GATTReadResponseEventData);
static void ProcessGATTWriteResponseEvent(GATT_Write_Response_Data_t *GATTWriteResponseEventData);
static void ProcessGATTErrorResponseEvent(GATT_Request_Error_Data_t *GATTRequestErrorData);
static void ProcessGATTServerNotificationEvent(GATT_Server_Notification_Data_t *GATTServerNotificationEventData);

static void ProcessGATTClientEvent(TIPM_GATT_Client_Event_Data_t *GATTClientEventData);
static void ProcessGATTConnectionEvent(TIPM_GATT_Connection_Event_Data_t *GATTConenctionEventData);

static void ProcessLowEnergyConnectionEvent(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties);
static void ProcessLowEnergyDisconnectionEvent(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties);
static void ProcessLowEnergyAddressChangeEvent(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties);
static void ProcessLowEnergyPairingChangeEvent(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties);
static void ProcessLowEnergyServicesStateChangeEvent(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties);

static void ProcessRemoteDevicePropertiesChangedEvent(unsigned long ChangedMemberMask, DEVM_Remote_Device_Properties_t *RemoteDeviceProperties);

static void BTPSAPI BTPMDispatchCallback_TIPM(void *CallbackParameter);
static void BTPSAPI BTPMDispatchCallback_CTS(void *CallbackParameter);
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter);
static void BTPSAPI BTPMDispatchCallback_GATTClient(void *CallbackParameter);
static void BTPSAPI BTPMDispatchCallback_GATTConnection(void *CallbackParameter);

static void BTPSAPI TIPManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter);

   /* The following function is a utility function that exists to       */
   /* retrieve a unique Callback ID that can be used to add an entry    */
   /* into the TIP Event Callback List.                                 */
static unsigned int GetNextEventCallbackID(void)
{
   ++NextEventCallbackID;

   if(NextEventCallbackID & 0x80000000)
      NextEventCallbackID = 1;

   return(NextEventCallbackID);
}

   /* The following function is a utility function that exists to       */
   /* retrieve a unique Transaction ID that can be used to add an entry */
   /* into the TIP Transaction List.                                    */
static unsigned int GetNextTransactionID(void)
{
   ++NextTransactionID;

   if(NextTransactionID & 0x80000000)
      NextTransactionID = 1;

   return(NextTransactionID);
}

   /* The following function adds the specified Entry to the specified  */
   /* List.  This function allocates and adds an entry to the list that */
   /* has the same attributes as the Entry passed into this function.   */
   /* This function will return NULL if NO Entry was added.  This can   */
   /* occur if the element passed in was deemed invalid or the actual   */
   /* List Head was invalid.                                            */
   /* ** NOTE ** This function does not insert duplicate entries into   */
   /*            the list.  An element is considered a duplicate if the */
   /*            EventCallbackID field is the same as an entry already  */
   /*            in the list.  When this occurs, this function returns  */
   /*            NULL.                                                  */
static TIPM_Event_Callback_Info_t *AddEventCallbackInfoEntry(TIPM_Event_Callback_Info_t **ListHead, TIPM_Event_Callback_Info_t *EntryToAdd)
{
   TIPM_Event_Callback_Info_t *AddedEntry = NULL;
   TIPM_Event_Callback_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First let's verify that values passed in are semi-valid.          */
   if((ListHead) && (EntryToAdd))
   {
      /* Make sure that the element that we are adding seems semi-valid.*/
      if(EntryToAdd->EventCallbackID)
      {
         /* OK, data seems semi-valid, let's allocate a new data        */
         /* structure to add to the list.                               */
         AddedEntry = (TIPM_Event_Callback_Info_t *)BTPS_AllocateMemory(sizeof(TIPM_Event_Callback_Info_t));

         if(AddedEntry)
         {
            /* Copy All Data over.                                      */
            *AddedEntry                              = *EntryToAdd;

            /* Now Add it to the end of the list.                       */
            AddedEntry->NextTIPMEventCallbackInfoPtr = NULL;

            /* First, let's check to see if there are any elements      */
            /* already present in the List that was passed in.          */
            if((tmpEntry = *ListHead) != NULL)
            {
               /* Head Pointer was not NULL, so we will traverse the    */
               /* list until we reach the last element.                 */
               while(tmpEntry)
               {
                  if(tmpEntry->EventCallbackID == AddedEntry->EventCallbackID)
                  {
                     /* Entry was already added, so free the memory and */
                     /* flag an error to the caller.                    */
                     FreeEventCallbackInfoEntryMemory(AddedEntry);
                     AddedEntry = NULL;

                     /* Abort the Search.                               */
                     tmpEntry   = NULL;
                  }
                  else
                  {
                     /* OK, we need to see if we are at the last element*/
                     /* of the List.  If we are, we simply break out of */
                     /* the list traversal because we know there are NO */
                     /* duplicates AND we are at the end of the list.   */
                     if(tmpEntry->NextTIPMEventCallbackInfoPtr)
                        tmpEntry = tmpEntry->NextTIPMEventCallbackInfoPtr;
                     else
                        break;
                  }
               }

               if(AddedEntry)
               {
                  /* Last element found, simply Add the entry.          */
                  tmpEntry->NextTIPMEventCallbackInfoPtr = AddedEntry;
               }
            }
            else
               *ListHead = AddedEntry;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", AddedEntry));

   return(AddedEntry);
}

   /* The following function searches the specified List for the        */
   /* specified Event Callback ID.  This function returns NULL if either*/
   /* the List Head is invalid, the Event Callback ID is invalid, or the*/
   /* specified Event Callback ID was NOT found.                        */
static TIPM_Event_Callback_Info_t *SearchEventCallbackInfoEntry(TIPM_Event_Callback_Info_t **ListHead, unsigned int EventCallbackID)
{
   TIPM_Event_Callback_Info_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", EventCallbackID));

   /* Let's make sure the list and Event Callback ID to search for      */
   /* appear to be valid.                                               */
   if((ListHead) && (EventCallbackID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->EventCallbackID != EventCallbackID))
         FoundEntry = FoundEntry->NextTIPMEventCallbackInfoPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* The following function searches the specified List for the        */
   /* specified Callback ID and removes it from the List.  This function*/
   /* returns NULL if either the List Head is invalid, the Callback ID  */
   /* is invalid, or the specified Callback ID was NOT present in the   */
   /* list.  The entry returned will have the Next Entry field set to   */
   /* NULL, and the caller is responsible for deleting the memory       */
   /* associated with this entry by calling                             */
   /* FreeEventCallbackInfoEntryMemory().                               */
static TIPM_Event_Callback_Info_t *DeleteEventCallbackInfoEntry(TIPM_Event_Callback_Info_t **ListHead, unsigned int EventCallbackID)
{
   TIPM_Event_Callback_Info_t *FoundEntry = NULL;
   TIPM_Event_Callback_Info_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", EventCallbackID));

   /* Let's make sure the List and Event Callback ID to search for      */
   /* appear to be semi-valid.                                          */
   if((ListHead) && (EventCallbackID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->EventCallbackID != EventCallbackID))
      {
         LastEntry  = FoundEntry;
         FoundEntry = FoundEntry->NextTIPMEventCallbackInfoPtr;
      }

      /* Check to see if we found the specified entry.                  */
      if(FoundEntry)
      {
         /* OK, now let's remove the entry from the list.  We have to   */
         /* check to see if the entry was the first entry in the list.  */
         if(LastEntry)
         {
            /* Entry was NOT the first entry in the list.               */
            LastEntry->NextTIPMEventCallbackInfoPtr = FoundEntry->NextTIPMEventCallbackInfoPtr;
         }
         else
            *ListHead = FoundEntry->NextTIPMEventCallbackInfoPtr;

         FoundEntry->NextTIPMEventCallbackInfoPtr = NULL;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* This function frees the specified Event Callback member.  No check*/
   /* is done on this entry other than making sure it NOT NULL.         */
static void FreeEventCallbackInfoEntryMemory(TIPM_Event_Callback_Info_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToFree)
      BTPS_FreeMemory(EntryToFree);

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified Event Callback List.  Upon return of this*/
   /* function, the Head Pointer is set to NULL.                        */
static void FreeEventCallbackInfoList(TIPM_Event_Callback_Info_t **ListHead)
{
   TIPM_Event_Callback_Info_t *EntryToFree;
   TIPM_Event_Callback_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ListHead)
   {
      /* Simply traverse the list and free every element present.       */
      EntryToFree = *ListHead;

      while(EntryToFree)
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Deleting Event Callback ID: 0x%08X\n", EntryToFree->EventCallbackID));

         tmpEntry    = EntryToFree;
         EntryToFree = EntryToFree->NextTIPMEventCallbackInfoPtr;

         FreeEventCallbackInfoEntryMemory(tmpEntry);
      }

      /* Make sure the List appears to be empty.                        */
      *ListHead = NULL;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function adds the specified Entry to the specified  */
   /* List.  This function allocates and adds an entry to the list that */
   /* has the same attributes as the Entry passed into this function.   */
   /* This function will return NULL if NO Entry was added.  This can   */
   /* occur if the element passed in was deemed invalid or the actual   */
   /* List Head was invalid.                                            */
   /* ** NOTE ** This function does not insert duplicate entries into   */
   /*            the list.  An element is considered a duplicate if the */
   /*            the BD_ADDR AND the Connection ID field is the same as */
   /*            an entry already in the list.  When this occurs, this  */
   /*            function returns NULL.                                 */
static Connection_Entry_t *AddConnectionEntry(Connection_Entry_t **ListHead, Connection_Entry_t *EntryToAdd)
{
   BD_ADDR_t           NULL_BD_ADDR;
   Connection_Entry_t *AddedEntry = NULL;
   Connection_Entry_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First let's verify that values passed in are semi-valid.          */
   if((ListHead) && (EntryToAdd))
   {
      /* Format a NULL BD_ADDR to test against.                         */
      ASSIGN_BD_ADDR(NULL_BD_ADDR, 0, 0, 0, 0, 0, 0);

      /* Make sure that the element that we are adding seems semi-valid.*/
      if(!COMPARE_BD_ADDR(NULL_BD_ADDR, EntryToAdd->BD_ADDR))
      {
         /* OK, data seems semi-valid, let's allocate a new data        */
         /* structure to add to the list.                               */
         AddedEntry = (Connection_Entry_t *)BTPS_AllocateMemory(sizeof(Connection_Entry_t));

         if(AddedEntry)
         {
            /* Copy All Data over.                                      */
            *AddedEntry                        = *EntryToAdd;

            /* Now Add it to the end of the list.                       */
            AddedEntry->NextConnectionEntryPtr = NULL;

            /* First, let's check to see if there are any elements      */
            /* already present in the List that was passed in.          */
            if((tmpEntry = *ListHead) != NULL)
            {
               /* Head Pointer was not NULL, so we will traverse the    */
               /* list until we reach the last element.                 */
               while(tmpEntry)
               {
                  if((COMPARE_BD_ADDR(tmpEntry->BD_ADDR, AddedEntry->BD_ADDR)) && (tmpEntry->ConnectionID == AddedEntry->ConnectionID) && (tmpEntry->ConnectionType == AddedEntry->ConnectionType))
                  {
                     /* Entry was already added, so free the memory and */
                     /* flag an error to the caller.                    */
                     FreeConnectionEntryMemory(AddedEntry);
                     AddedEntry = NULL;

                     /* Abort the Search.                               */
                     tmpEntry   = NULL;
                  }
                  else
                  {
                     /* OK, we need to see if we are at the last element*/
                     /* of the List.  If we are, we simply break out of */
                     /* the list traversal because we know there are NO */
                     /* duplicates AND we are at the end of the list.   */
                     if(tmpEntry->NextConnectionEntryPtr)
                        tmpEntry = tmpEntry->NextConnectionEntryPtr;
                     else
                        break;
                  }
               }

               if(AddedEntry)
               {
                  /* Last element found, simply Add the entry.          */
                  tmpEntry->NextConnectionEntryPtr = AddedEntry;
               }
            }
            else
               *ListHead = AddedEntry;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", AddedEntry));

   return(AddedEntry);
}

   /* The following function searches the specified Connection Entry    */
   /* List for the specified Connection Entry based on the specified    */
   /* Bluetooth Device Address.  This function returns NULL if either   */
   /* the Connection Entry List Head is invalid, the Bluetooth Device   */
   /* Address is invalid, or the specified Entry was NOT present in the */
   /* list.                                                             */
static Connection_Entry_t *SearchConnectionEntry(Connection_Entry_t **ListHead, unsigned int ConnectionID, TIPM_Connection_Type_t ConnectionType)
{
   Connection_Entry_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: (%u, %u)\n", ConnectionID, ConnectionType));

   /* Let's make sure the list and Bluetooth Device Address to search   */
   /* for appear to be valid.                                           */
   if((ListHead) && (ConnectionID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && ((FoundEntry->ConnectionID != ConnectionID) || (FoundEntry->ConnectionType != ConnectionType)))
         FoundEntry = FoundEntry->NextConnectionEntryPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* The following function searches the specified Connection Entry    */
   /* List for the specified Connection Entry based on the specified    */
   /* Bluetooth Device Address.  This function returns NULL if either   */
   /* the Connection Entry List Head is invalid, the Bluetooth Device   */
   /* Address is invalid, or the specified Entry was NOT present in the */
   /* list.                                                             */
static Connection_Entry_t *SearchConnectionEntryByBD_ADDR(Connection_Entry_t **ListHead, BD_ADDR_t BD_ADDR, TIPM_Connection_Type_t ConnectionType)
{
   Connection_Entry_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's make sure the list and Bluetooth Device Address to search   */
   /* for appear to be valid.                                           */
   if((ListHead) && (!COMPARE_NULL_BD_ADDR(BD_ADDR)))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && ((!COMPARE_BD_ADDR(FoundEntry->BD_ADDR, BD_ADDR)) || (FoundEntry->ConnectionType != ConnectionType)))
         FoundEntry = FoundEntry->NextConnectionEntryPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* The following function searches the specified Connection Entry    */
   /* List for the Connection Entry with the specified Bluetooth Device */
   /* Address and removes it from the List.  This function returns NULL */
   /* if either the Connection Entry List Head is invalid, the Bluetooth*/
   /* Device Address is invalid, or the specified Entry was NOT present */
   /* in the list.  The entry returned will have the Next Entry field   */
   /* set to NULL, and the caller is responsible for deleting the memory*/
   /* associated with this entry by calling FreeConnectionEntryMemory().*/
static Connection_Entry_t *DeleteConnectionEntryByBD_ADDR(Connection_Entry_t **ListHead, BD_ADDR_t BD_ADDR, TIPM_Connection_Type_t ConnectionType)
{
   Connection_Entry_t *FoundEntry = NULL;
   Connection_Entry_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's make sure the list and Bluetooth Device Address to search   */
   /* for appear to be valid.                                           */
   if((ListHead) && (!COMPARE_NULL_BD_ADDR(BD_ADDR)))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && ((!COMPARE_BD_ADDR(FoundEntry->BD_ADDR, BD_ADDR)) || (FoundEntry->ConnectionType != ConnectionType)))
      {
         LastEntry  = FoundEntry;
         FoundEntry = FoundEntry->NextConnectionEntryPtr;
      }

      /* Check to see if we found the specified entry.                  */
      if(FoundEntry)
      {
         /* OK, now let's remove the entry from the list.  We have to   */
         /* check to see if the entry was the first entry in the list.  */
         if(LastEntry)
         {
            /* Entry was NOT the first entry in the list.               */
            LastEntry->NextConnectionEntryPtr = FoundEntry->NextConnectionEntryPtr;
         }
         else
            *ListHead = FoundEntry->NextConnectionEntryPtr;

         FoundEntry->NextConnectionEntryPtr = NULL;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* This function frees the specified Connection Information member.  */
   /* No check is done on this entry other than making sure it NOT NULL.*/
static void FreeConnectionEntryMemory(Connection_Entry_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToFree)
      BTPS_FreeMemory(EntryToFree);

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified Connection Information List.  Upon return*/
   /* of this function, the Head Pointer is set to NULL.                */
static void FreeConnectionEntryList(Connection_Entry_t **ListHead)
{
   Connection_Entry_t *EntryToFree;
   Connection_Entry_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ListHead)
   {
      /* Simply traverse the list and free every element present.       */
      EntryToFree = *ListHead;

      while(EntryToFree)
      {
         tmpEntry    = EntryToFree;
         EntryToFree = EntryToFree->NextConnectionEntryPtr;

         FreeConnectionEntryMemory(tmpEntry);
      }

      /* Make sure the List appears to be empty.                        */
      *ListHead = NULL;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* search for an existing entry for the specified device OR to create*/
   /* a new entry and add it to the specified list.  This function      */
   /* returns the Entry on success or NULL on failure.                  */
static Connection_Entry_t *SearchAddConnectionEntry(Connection_Entry_t **ListHead, TIPM_Connection_Type_t ConnectionType, BD_ADDR_t BD_ADDR, unsigned int ConnectionID)
{
   Connection_Entry_t               EntryToAdd;
   Connection_Entry_t              *ret_val = NULL;
   DEVM_Remote_Device_Properties_t  RemoteDeviceProperties;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %u\n", ConnectionType));

   /* Verify that the input parameters appear semi-valid.               */
   if((ListHead) && ((ConnectionType == tctServer) || (ConnectionType == tctClient)) && (!COMPARE_NULL_BD_ADDR(BD_ADDR)) && (ConnectionID))
   {
      /* First search the list for the specified Connection ID.         */
      if((ret_val = SearchConnectionEntry(ListHead, ConnectionID, ConnectionType)) == NULL)
      {
         /* Query the remote device properties to get the "Base" Address*/
         /* (which may not be the same address we get here).            */
         if(!DEVM_QueryRemoteDeviceProperties(BD_ADDR, DEVM_QUERY_REMOTE_DEVICE_PROPERTIES_FLAGS_LOW_ENERGY, &RemoteDeviceProperties))
         {
            /* Entry was not found so add it to the list.               */
            BTPS_MemInitialize(&EntryToAdd, 0, sizeof(Connection_Entry_t));

            EntryToAdd.ConnectionType  = ConnectionType;
            EntryToAdd.ConnectionID    = ConnectionID;
            EntryToAdd.BD_ADDR         = RemoteDeviceProperties.BD_ADDR;

            /* Attempt to add the entry to the list.                    */
            ret_val = AddConnectionEntry(ListHead, &EntryToAdd);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", ret_val));

   return(ret_val);
}

   /* The following function adds the specified Entry to the specified  */
   /* List.  This function allocates and adds an entry to the list that */
   /* has the same attributes as the Entry passed into this function.   */
   /* This function will return NULL if NO Entry was added.  This can   */
   /* occur if the element passed in was deemed invalid or the actual   */
   /* List Head was invalid.                                            */
   /* ** NOTE ** This function does not insert duplicate entries into   */
   /*            the list.  An element is considered a duplicate if the */
   /*            the BD_ADDR AND the Callback ID field is the same as   */
   /*            an entry already in the list.  When this occurs, this  */
   /*            function returns NULL.                                 */
static Notification_Registration_Entry_t *AddNotificationRegistrationEntry(Notification_Registration_Entry_t **ListHead, Notification_Registration_Entry_t *EntryToAdd)
{
   BD_ADDR_t                          NULL_BD_ADDR;
   Notification_Registration_Entry_t *AddedEntry = NULL;
   Notification_Registration_Entry_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First let's verify that values passed in are semi-valid.          */
   if((ListHead) && (EntryToAdd))
   {
      /* Format a NULL BD_ADDR to test against.                         */
      ASSIGN_BD_ADDR(NULL_BD_ADDR, 0, 0, 0, 0, 0, 0);

      /* Make sure that the element that we are adding seems semi-valid.*/
      if(!COMPARE_BD_ADDR(NULL_BD_ADDR, EntryToAdd->BD_ADDR))
      {
         /* OK, data seems semi-valid, let's allocate a new data        */
         /* structure to add to the list.                               */
         AddedEntry = (Notification_Registration_Entry_t *)BTPS_AllocateMemory(sizeof(Notification_Registration_Entry_t));

         if(AddedEntry)
         {
            /* Copy All Data over.                                      */
            *AddedEntry                                      = *EntryToAdd;

            /* Now Add it to the end of the list.                       */
            AddedEntry->NextNotificationRegistrationEntryPtr = NULL;

            /* First, let's check to see if there are any elements      */
            /* already present in the List that was passed in.          */
            if((tmpEntry = *ListHead) != NULL)
            {
               /* Head Pointer was not NULL, so we will traverse the    */
               /* list until we reach the last element.                 */
               while(tmpEntry)
               {
                  if((COMPARE_BD_ADDR(tmpEntry->BD_ADDR, AddedEntry->BD_ADDR)) && (tmpEntry->CallbackID == AddedEntry->CallbackID))
                  {
                     /* Entry was already added, so free the memory and */
                     /* flag an error to the caller.                    */
                     FreeNotificationRegistrationEntryMemory(AddedEntry);
                     AddedEntry = NULL;

                     /* Abort the Search.                               */
                     tmpEntry   = NULL;
                  }
                  else
                  {
                     /* OK, we need to see if we are at the last element*/
                     /* of the List.  If we are, we simply break out of */
                     /* the list traversal because we know there are NO */
                     /* duplicates AND we are at the end of the list.   */
                     if(tmpEntry->NextNotificationRegistrationEntryPtr)
                        tmpEntry = tmpEntry->NextNotificationRegistrationEntryPtr;
                     else
                        break;
                  }
               }

               if(AddedEntry)
               {
                  /* Last element found, simply Add the entry.          */
                  tmpEntry->NextNotificationRegistrationEntryPtr = AddedEntry;
               }
            }
            else
               *ListHead = AddedEntry;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", AddedEntry));

   return(AddedEntry);
}

   /* The following function searches the specified Notification Entry  */
   /* List for the specified Notification Entry based on the specified  */
   /* Bluetooth Device Address and/or CallbackID.  This function returns*/
   /* NULL if either the Notification Entry List Head is invalid or the */
   /* specified Entry was NOT present in the list.                      */
static Notification_Registration_Entry_t *SearchNotificationRegistrationEntry(Notification_Registration_Entry_t **ListHead, unsigned int *CallbackID, BD_ADDR_t *BD_ADDR)
{
   Notification_Registration_Entry_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's make sure the list and Bluetooth Device Address to search   */
   /* for appear to be valid.                                           */
   if((ListHead) && ((BD_ADDR) || (CallbackID)))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (((CallbackID) && (*CallbackID != FoundEntry->CallbackID)) || ((BD_ADDR) && (!COMPARE_BD_ADDR(*BD_ADDR, FoundEntry->BD_ADDR)))))
         FoundEntry = FoundEntry->NextNotificationRegistrationEntryPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* The following function deletes the specified Notification Entry in*/
   /* the specified Notification List based on the specified Bluetooth  */
   /* Device Address and/or CallbackID.  This function returns NULL     */
   /* if either the Notification Entry List Head is invalid or the      */
   /* specified Entry was NOT present in the list.                      */
static Notification_Registration_Entry_t *DeleteNotificationRegistrationEntry(Notification_Registration_Entry_t **ListHead, unsigned int *CallbackID, BD_ADDR_t *BD_ADDR)
{
   Notification_Registration_Entry_t *FoundEntry = NULL;
   Notification_Registration_Entry_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's make sure the list and Bluetooth Device Address to search   */
   /* for appear to be valid.                                           */
   if((ListHead) && ((BD_ADDR) || (CallbackID)))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (((CallbackID) && (*CallbackID != FoundEntry->CallbackID)) || ((BD_ADDR) && (!COMPARE_BD_ADDR(*BD_ADDR, FoundEntry->BD_ADDR)))))
      {
         LastEntry  = FoundEntry;
         FoundEntry = FoundEntry->NextNotificationRegistrationEntryPtr;
      }

      /* Check to see if we found the specified entry.                  */
      if(FoundEntry)
      {
         /* OK, now let's remove the entry from the list.  We have to   */
         /* check to see if the entry was the first entry in the list.  */
         if(LastEntry)
         {
            /* Entry was NOT the first entry in the list.               */
            LastEntry->NextNotificationRegistrationEntryPtr = FoundEntry->NextNotificationRegistrationEntryPtr;
         }
         else
            *ListHead = FoundEntry->NextNotificationRegistrationEntryPtr;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* This function frees the specified Notification Information member.*/
   /* No check is done on this entry other than making sure it NOT NULL.*/
static void FreeNotificationRegistrationEntryMemory(Notification_Registration_Entry_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToFree)
      BTPS_FreeMemory(EntryToFree);

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified Notification Information List.  Upon     */
   /* return of this function, the Head Pointer is set to NULL.         */
static void FreeNotificationRegistrationList(Notification_Registration_Entry_t **ListHead)
{
   Notification_Registration_Entry_t *EntryToFree;
   Notification_Registration_Entry_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ListHead)
   {
      /* Simply traverse the list and free every element present.       */
      EntryToFree = *ListHead;

      while(EntryToFree)
      {
         tmpEntry    = EntryToFree;
         EntryToFree = EntryToFree->NextNotificationRegistrationEntryPtr;

         FreeNotificationRegistrationEntryMemory(tmpEntry);
      }

      /* Make sure the List appears to be empty.                        */
      *ListHead = NULL;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function adds the specified Entry to the specified  */
   /* List.  This function allocates and adds an entry to the list that */
   /* has the same attributes as the Entry passed into this function.   */
   /* This function will return NULL if NO Entry was added.  This can   */
   /* occur if the element passed in was deemed invalid or the actual   */
   /* List Head was invalid.                                            */
   /* ** NOTE ** This function does not insert duplicate entries into   */
   /*            the list.  An element is considered a duplicate if     */
   /*            the the Transacation ID field is the same as an entry  */
   /*            already in the list.  When this occurs, this function  */
   /*            returns NULL.                                          */
static Transaction_Entry_t *AddTransactionEntry(Transaction_Entry_t **ListHead, Transaction_Entry_t *EntryToAdd)
{
   Transaction_Entry_t *AddedEntry = NULL;
   Transaction_Entry_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First let's verify that values passed in are semi-valid.          */
   if((ListHead) && (EntryToAdd))
   {
      /* Make sure that the element that we are adding seems semi-valid.*/
      if(EntryToAdd->TransactionID)
      {
         /* OK, data seems semi-valid, let's allocate a new data        */
         /* structure to add to the list.                               */
         AddedEntry = (Transaction_Entry_t *)BTPS_AllocateMemory(sizeof(Transaction_Entry_t));

         if(AddedEntry)
         {
            /* Copy All Data over.                                      */
            *AddedEntry                        = *EntryToAdd;

            /* Now Add it to the end of the list.                       */
            AddedEntry->NextTransactionEntryPtr = NULL;

            /* First, let's check to see if there are any elements      */
            /* already present in the List that was passed in.          */
            if((tmpEntry = *ListHead) != NULL)
            {
               /* Head Pointer was not NULL, so we will traverse the    */
               /* list until we reach the last element.                 */
               while(tmpEntry)
               {
                  if(tmpEntry->TransactionID == AddedEntry->TransactionID)
                  {
                     /* Entry was already added, so free the memory and */
                     /* flag an error to the caller.                    */
                     FreeTransactionEntryMemory(AddedEntry);
                     AddedEntry = NULL;

                     /* Abort the Search.                               */
                     tmpEntry   = NULL;
                  }
                  else
                  {
                     /* OK, we need to see if we are at the last element*/
                     /* of the List.  If we are, we simply break out of */
                     /* the list traversal because we know there are NO */
                     /* duplicates AND we are at the end of the list.   */
                     if(tmpEntry->NextTransactionEntryPtr)
                        tmpEntry = tmpEntry->NextTransactionEntryPtr;
                     else
                        break;
                  }
               }

               if(AddedEntry)
               {
                  /* Last element found, simply Add the entry.          */
                  tmpEntry->NextTransactionEntryPtr = AddedEntry;
               }
            }
            else
               *ListHead = AddedEntry;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", AddedEntry));

   return(AddedEntry);
}

   /* The following function deletes the specified Transaction Entry    */
   /* in the specified Transaction List based on the specified          */
   /* GATTTransactionID.  This function returns NULL if either the      */
   /* Transaction Entry List Head is invalid or the specified Entry was */
   /* NOT present in the list.                                          */
static Transaction_Entry_t *DeleteTransactionEntry(Transaction_Entry_t **ListHead, unsigned int GATTTransactionID)
{
   Transaction_Entry_t *FoundEntry = NULL;
   Transaction_Entry_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's make sure the list and Bluetooth Device Address to search   */
   /* for appear to be valid.                                           */
   if((ListHead) && (GATTTransactionID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->GATTTransactionID != GATTTransactionID))
      {
         LastEntry  = FoundEntry;
         FoundEntry = FoundEntry->NextTransactionEntryPtr;
      }

      /* Check to see if we found the specified entry.                  */
      if(FoundEntry)
      {
         /* OK, now let's remove the entry from the list.  We have to   */
         /* check to see if the entry was the first entry in the list.  */
         if(LastEntry)
         {
            /* Entry was NOT the first entry in the list.               */
            LastEntry->NextTransactionEntryPtr = FoundEntry->NextTransactionEntryPtr;
         }
         else
            *ListHead = FoundEntry->NextTransactionEntryPtr;

         FoundEntry->NextTransactionEntryPtr = NULL;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* This function frees the specified Transaction Information member. */
   /* No check is done on this entry other than making sure it NOT NULL.*/
static void FreeTransactionEntryMemory(Transaction_Entry_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToFree)
      BTPS_FreeMemory(EntryToFree);

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified Transaction Information List.  Upon      */
   /* return of this function, the Head Pointer is set to NULL.         */
static void FreeTransactionList(Transaction_Entry_t **ListHead)
{
   Transaction_Entry_t *EntryToFree;
   Transaction_Entry_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ListHead)
   {
      /* Simply traverse the list and free every element present.       */
      EntryToFree = *ListHead;

      while(EntryToFree)
      {
         tmpEntry    = EntryToFree;
         EntryToFree = EntryToFree->NextTransactionEntryPtr;

         FreeTransactionEntryMemory(tmpEntry);
      }

      /* Make sure the List appears to be empty.                        */
      *ListHead = NULL;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* reload a Devices' stored configuration for the specified          */
   /* connection entry from the Low Energy Configuration File.          */
static void StoreConnectionConfiguration(Connection_Entry_t *ConnectionEntry, Boolean_t Store)
{
   char                            TempString[64];
   Connection_Binary_Entry_t       BinaryEntry;
   DEVM_Remote_Device_Properties_t RemoteDeviceProperties;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if(ConnectionEntry)
   {
      /* Query the Remote Device Properties to determine if this        */
      /* connection is paired if we are being asked to store something  */
      /* to the file.                                                   */
      if((!Store) || (!DEVM_QueryRemoteDeviceProperties(ConnectionEntry->BD_ADDR, DEVM_QUERY_REMOTE_DEVICE_PROPERTIES_FLAGS_LOW_ENERGY, &RemoteDeviceProperties)))
      {
         /* Verify that the device is currently paired over LE if we are*/
         /* being asked to store something to the file.                 */
         if((!Store) || ((RemoteDeviceProperties.RemoteDeviceFlags & (DEVM_REMOTE_DEVICE_FLAGS_DEVICE_SUPPORTS_LOW_ENERGY | DEVM_REMOTE_DEVICE_FLAGS_DEVICE_CURRENTLY_PAIRED_OVER_LE)) == (DEVM_REMOTE_DEVICE_FLAGS_DEVICE_SUPPORTS_LOW_ENERGY | DEVM_REMOTE_DEVICE_FLAGS_DEVICE_CURRENTLY_PAIRED_OVER_LE)))
         {
            /* Format the Key Name.                                     */
            sprintf(TempString, TIPM_KEY_NAME_CCCD_PREFIX, ConnectionEntry->BD_ADDR.BD_ADDR5, ConnectionEntry->BD_ADDR.BD_ADDR4, ConnectionEntry->BD_ADDR.BD_ADDR3, ConnectionEntry->BD_ADDR.BD_ADDR2, ConnectionEntry->BD_ADDR.BD_ADDR1, ConnectionEntry->BD_ADDR.BD_ADDR0);

            /* Check to see if we are storing something to flash or     */
            /* deleting something.                                      */
            if(Store)
            {
               /* Device is paired so go ahead and store the            */
               /* configuration to flash (if the device has registered  */
               /* for TIP notifications).                               */
               if(ConnectionEntry->CurrentTimeCCCD)
               {
                  /* Format the Binary Entry.                           */
                  ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&(BinaryEntry.CurrentTimeCCCD), ConnectionEntry->CurrentTimeCCCD);

                  /* Now write out the new Key-Value Pair.              */
                  SET_WriteBinaryData(TIPM_LE_CONFIGURATION_FILE_SECTION_NAME, TempString, (unsigned char *)&BinaryEntry, sizeof(BinaryEntry), BTPM_CONFIGURATION_LOW_ENERGY_CONFIGURATION_FILE_NAME);
               }
               else
               {
                  DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Deleting Configuration for Device due to no CCCD being configured, Key = %s\n", TempString));

                  /* Delete the configuration stored for this device.   */
                  SET_WriteBinaryData(TIPM_LE_CONFIGURATION_FILE_SECTION_NAME, TempString, NULL, 0, BTPM_CONFIGURATION_LOW_ENERGY_CONFIGURATION_FILE_NAME);
               }
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Deleting Configuration for Device, Key = %s\n", TempString));

               /* Delete the configuration stored for this device.      */
               SET_WriteBinaryData(TIPM_LE_CONFIGURATION_FILE_SECTION_NAME, TempString, NULL, 0, BTPM_CONFIGURATION_LOW_ENERGY_CONFIGURATION_FILE_NAME);
            }
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* reload a Devices' stored configuration for the specified          */
   /* connection entry from the Low Energy Configuration File.          */
static void ReloadConnectionConfiguration(Connection_Entry_t *ConnectionEntry)
{
   char                      TempString[64];
   Connection_Binary_Entry_t BinaryEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if(ConnectionEntry)
   {
      /* Format the Key Name.                                           */
      sprintf(TempString, TIPM_KEY_NAME_CCCD_PREFIX, ConnectionEntry->BD_ADDR.BD_ADDR5, ConnectionEntry->BD_ADDR.BD_ADDR4, ConnectionEntry->BD_ADDR.BD_ADDR3, ConnectionEntry->BD_ADDR.BD_ADDR2, ConnectionEntry->BD_ADDR.BD_ADDR1, ConnectionEntry->BD_ADDR.BD_ADDR0);

      /* Attempt to reload the configuration for this device.           */
      if(SET_ReadBinaryData(TIPM_LE_CONFIGURATION_FILE_SECTION_NAME, TempString, (unsigned char *)&BinaryEntry, sizeof(BinaryEntry), BTPM_CONFIGURATION_LOW_ENERGY_CONFIGURATION_FILE_NAME) == sizeof(Connection_Binary_Entry_t))
      {
         /* Reload the configuration into the connection entry.         */
         ConnectionEntry->CurrentTimeCCCD = READ_UNALIGNED_WORD_LITTLE_ENDIAN(&(BinaryEntry.CurrentTimeCCCD));

         DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Reloaded Current Time CCCD: %u\n", (unsigned int)ConnectionEntry->CurrentTimeCCCD));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to store*/
   /* Persistent UID for the CTS service registered by this module from */
   /* the Low Energy Configuration File.                                */
static void StorePersistentUID(DWord_t PersistentUID, GATT_Attribute_Handle_Group_t *ServiceHandleRange, Boolean_t Store)
{
   char          TempString[64];
   unsigned int  Index;
   unsigned char TempBuffer[TIPM_PERSISTENT_UID_SIZE];

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if(((Store == FALSE) || ((PersistentUID) && (ServiceHandleRange))))
   {
      /* Format the Key Name.                                           */
      BTPS_StringCopy(TempString, TIPM_KEY_NAME_PERSISTENT_UID);

      /* Check to see if we are storing something to flash or deleting  */
      /* something.                                                     */
      if(Store)
      {
         /* Reset the Index.                                            */
         Index  = 0;

         /* Format the Persistent UID and the Service Handle Range.     */
         ASSIGN_HOST_DWORD_TO_BIG_ENDIAN_UNALIGNED_DWORD(&(TempBuffer[Index]), PersistentUID);

         Index += NON_ALIGNED_DWORD_SIZE;

         ASSIGN_HOST_WORD_TO_BIG_ENDIAN_UNALIGNED_WORD(&(TempBuffer[Index]), ServiceHandleRange->Starting_Handle);

         Index += NON_ALIGNED_WORD_SIZE;

         ASSIGN_HOST_WORD_TO_BIG_ENDIAN_UNALIGNED_WORD(&(TempBuffer[Index]), ServiceHandleRange->Ending_Handle);

         /* Now write out the new Key-Value Pair.                       */
         SET_WriteBinaryData(TIPM_LE_CONFIGURATION_FILE_SECTION_NAME, TempString, (unsigned char *)TempBuffer, sizeof(TempBuffer), BTPM_CONFIGURATION_LOW_ENERGY_CONFIGURATION_FILE_NAME);
      }
      else
      {
         /* Delete the configuration stored.                            */
         SET_WriteBinaryData(TIPM_LE_CONFIGURATION_FILE_SECTION_NAME, TempString, NULL, 0, BTPM_CONFIGURATION_LOW_ENERGY_CONFIGURATION_FILE_NAME);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit:\n"));
}

   /* The following function is a utility function that is used to      */
   /* reload the Persistent UID for the CTS service registered by this  */
   /* module from the Low Energy Configuration File.  This function     */
   /* returns TRUE if the Persistent UID was reloaded or false          */
   /* otherwise.                                                        */
static Boolean_t ReloadPersistentUID(DWord_t *PersistentUID, GATT_Attribute_Handle_Group_t *ServiceHandleRange)
{
   char          TempString[64];
   Boolean_t     ret_val = FALSE;
   unsigned int  Index;
   unsigned char TempBuffer[TIPM_PERSISTENT_UID_SIZE];

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if((PersistentUID) && (ServiceHandleRange))
   {
      /* Format the Key Name.                                           */
      BTPS_StringCopy(TempString, TIPM_KEY_NAME_PERSISTENT_UID);

      /* Attempt to reload the configuration for this device.           */
      if(SET_ReadBinaryData(TIPM_LE_CONFIGURATION_FILE_SECTION_NAME, TempString, (unsigned char *)TempBuffer, sizeof(TempBuffer), BTPM_CONFIGURATION_LOW_ENERGY_CONFIGURATION_FILE_NAME) == sizeof(TempBuffer))
      {
         /* Reset the Index.                                            */
         Index                                = 0;

         /* Reload the Persistent UID and Service Handle Range.         */
         *PersistentUID                       = READ_UNALIGNED_DWORD_BIG_ENDIAN(&(TempBuffer[Index]));

         Index                               += NON_ALIGNED_DWORD_SIZE;

         ServiceHandleRange->Starting_Handle  = READ_UNALIGNED_WORD_BIG_ENDIAN(&(TempBuffer[Index]));

         Index                               += NON_ALIGNED_WORD_SIZE;

         ServiceHandleRange->Ending_Handle    = READ_UNALIGNED_WORD_BIG_ENDIAN(&(TempBuffer[Index]));

         /* Return success to the caller.                               */
         ret_val                              = TRUE;

         DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Persistent UID: 0x%08X\n", (unsigned int)*PersistentUID));
         DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Service Range:  0x%04X - 0x%04X\n", ServiceHandleRange->Starting_Handle, ServiceHandleRange->Ending_Handle));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", (int)ret_val));

   return(ret_val);
}

   /* The following function is a utility function which is used to     */
   /* either load from the configuration file or generate a new Service */
   /* Handle Range for the CTS Service that is registered by this       */
   /* module.  This function returns TRUE if a Handle Range was         */
   /* calculated or FALSE otherwise.                                    */
static Boolean_t CalculateServiceHandleRange(GATT_Attribute_Handle_Group_t *ServiceHandleRange)
{
   DWord_t      PersistentUID;
   Boolean_t    RegisterPersistent;
   Boolean_t    ret_val = FALSE;
   unsigned int NumberOfAttributes;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if(ServiceHandleRange)
   {
      /* Query the number of attributes needed by the service.          */
      if((NumberOfAttributes = _TIPM_Query_Number_Attributes()) > 0)
      {
         /* Attempt to re-load a previously registered Persistent UID   */
         /* from the file.                                              */
         if(ReloadPersistentUID(&PersistentUID, ServiceHandleRange))
         {
            /* Verify this handle range has enough attributes to satisfy*/
            /* the request.                                             */
            if(((unsigned int)((ServiceHandleRange->Ending_Handle - ServiceHandleRange->Starting_Handle) + 1)) != NumberOfAttributes)
            {
               /* Delete the old stored Persistent UID.                 */
               StorePersistentUID(0, NULL, FALSE);

               /* We don't have enough handles for this service so      */
               /* allocate a new range after deleting what we have      */
               /* stored now.                                           */
               RegisterPersistent = TRUE;
            }
            else
            {
               /* We already have allocated the Service Handle Range so */
               /* just return success.                                  */
               ret_val            = TRUE;
               RegisterPersistent = FALSE;
            }
         }
         else
            RegisterPersistent = TRUE;

         /* If requested attempt to allocate a new Persistent UID.      */
         if(RegisterPersistent)
         {
            /* Attempt to register a new Persistent UID for the         */
            /* requested number of attributes.                          */
            /* * NOTE * We will subtract 1 attribute (for the service   */
            /*          declaration because GATM_RegisterPersistentUID()*/
            /*          already includes this in it's calculation.      */
            if(!GATM_RegisterPersistentUID((NumberOfAttributes - 1), &PersistentUID, ServiceHandleRange))
            {
               /* Store this configure into the LE Configuration file.  */
               StorePersistentUID(PersistentUID, ServiceHandleRange, TRUE);

               /* Return success to the caller.                         */
               ret_val = TRUE;
            }
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", (int)ret_val));

   return(ret_val);
}

   /* The following function is a utility function that is used to      */
   /* convert a PM Time Source into a CTS Time Source.  This functions  */
   /* returns ZERO on success or a negative error code.                 */
static int ConvertTimeSourceToCTS(TIPM_Time_Source_Type_t TimeSource, CTS_Time_Source_Type_t *TimeSourceResult)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if(TimeSourceResult)
   {
      /* Initialize the return value to success.                        */
      ret_val = 0;

      /* Do the necessary conversion.                                   */
      switch(TimeSource)
      {
         case tmsUnknown:
            *TimeSourceResult = tsUnknown;
            break;
         case tmsNetworkTimeProtocol:
            *TimeSourceResult = tsNetworkTimeProtocol;
            break;
         case tmsGps:
            *TimeSourceResult = tsGps;
            break;
         case tmsRadioTimeSignal:
            *TimeSourceResult = tsRadioTimeSignal;
            break;
         case tmsManual:
            *TimeSourceResult = tsManual;
            break;
         case tmsAtomicClock:
            *TimeSourceResult = tsAtomicClock;
            break;
         case tmsCellularNetwork:
            *TimeSourceResult = tsCellularNetwork;
            break;
         default:
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
            break;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d.\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is a utility function that is used to      */
   /* convert a PM DST Offset into a CTS DST Offset.  This functions    */
   /* returns ZERO on success or a negative error code.                 */
static int ConvertDSTOffsetToCTS(TIPM_DST_Offset_Type_t DSTOffset, CTS_DST_Offset_Type_t *DSTOffsetResult)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if(DSTOffsetResult)
   {
      /* Initialize the return value to success.                        */
      ret_val = 0;

      /* Do the necessary conversion.                                   */
      switch(DSTOffset)
      {
         case tmdStandardTime:
            *DSTOffsetResult = doStandardTime;
            break;
         case tmdHalfAnHourDaylightTime:
            *DSTOffsetResult = doHalfAnHourDaylightTime;
            break;
         case tmdDaylightTime:
            *DSTOffsetResult = doDaylightTime;
            break;
         case tmdDoubleDaylightTime:
            *DSTOffsetResult = doDoubleDaylightTime;
            break;
         case tmdUnknown:
            *DSTOffsetResult = doUnknown;
            break;
         default:
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
            break;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d.\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is a utility function that is used to      */
   /* convert a PM Time Zone into a CTS Time Zone.  This functions      */
   /* returns ZERO on success or a negative error code.                 */
static int ConvertTimeZoneToCTS(TIPM_Time_Zone_Type_t TimeZone, CTS_Time_Zone_Type_t *TimeZoneResult)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if(TimeZoneResult)
   {
      /* Initialize the return value to success.                        */
      ret_val = 0;

      /* Do the necessary conversion.                                   */
      switch(TimeZone)
      {
         case tmzUTCMinus1200:
            *TimeZoneResult = tzUTCMinus1200;
            break;
         case tmzUTCMinus1100:
            *TimeZoneResult = tzUTCMinus1100;
            break;
         case tmzUTCMinus1000:
            *TimeZoneResult = tzUTCMinus1000;
            break;
         case tmzUTCMinus930:
            *TimeZoneResult = tzUTCMinus930;
            break;
         case tmzUTCMinus900:
            *TimeZoneResult = tzUTCMinus900;
            break;
         case tmzUTCMinus800:
            *TimeZoneResult = tzUTCMinus800;
            break;
         case tmzUTCMinus700:
            *TimeZoneResult = tzUTCMinus700;
            break;
         case tmzUTCMinus600:
            *TimeZoneResult = tzUTCMinus600;
            break;
         case tmzUTCMinus500:
            *TimeZoneResult = tzUTCMinus500;
            break;
         case tmzUTCMinus430:
            *TimeZoneResult = tzUTCMinus430;
            break;
         case tmzUTCMinus400:
            *TimeZoneResult = tzUTCMinus400;
            break;
         case tmzUTCMinus330:
            *TimeZoneResult = tzUTCMinus330;
            break;
         case tmzUTCMinus300:
            *TimeZoneResult = tzUTCMinus300;
            break;
         case tmzUTCMinus200:
            *TimeZoneResult = tzUTCMinus200;
            break;
         case tmzUTCMinus100:
            *TimeZoneResult = tzUTCMinus100;
            break;
         case tmzUTCPlus000:
            *TimeZoneResult = tzUTCPlus000;
            break;
         case tmzUTCPlus100:
            *TimeZoneResult = tzUTCPlus100;
            break;
         case tmzUTCPlus200:
            *TimeZoneResult = tzUTCPlus200;
            break;
         case tmzUTCPlus300:
            *TimeZoneResult = tzUTCPlus300;
            break;
         case tmzUTCPlus330:
            *TimeZoneResult = tzUTCPlus330;
            break;
         case tmzUTCPlus400:
            *TimeZoneResult = tzUTCPlus400;
            break;
         case tmzUTCPlus430:
            *TimeZoneResult = tzUTCPlus430;
            break;
         case tmzUTCPlus500:
            *TimeZoneResult = tzUTCPlus500;
            break;
         case tmzUTCPlus530:
            *TimeZoneResult = tzUTCPlus530;
            break;
         case tmzUTCPlus545:
            *TimeZoneResult = tzUTCPlus545;
            break;
         case tmzUTCPlus600:
            *TimeZoneResult = tzUTCPlus600;
            break;
         case tmzUTCPlus630:
            *TimeZoneResult = tzUTCPlus630;
            break;
         case tmzUTCPlus700:
            *TimeZoneResult = tzUTCPlus700;
            break;
         case tmzUTCPlus800:
            *TimeZoneResult = tzUTCPlus800;
            break;
         case tmzUTCPlus845:
            *TimeZoneResult = tzUTCPlus845;
            break;
         case tmzUTCPlus900:
            *TimeZoneResult = tzUTCPlus900;
            break;
         case tmzUTCPlus930:
            *TimeZoneResult = tzUTCPlus930;
            break;
         case tmzUTCPlus1000:
            *TimeZoneResult = tzUTCPlus1000;
            break;
         case tmzUTCPlus1030:
            *TimeZoneResult = tzUTCPlus1030;
            break;
         case tmzUTCPlus1100:
            *TimeZoneResult = tzUTCPlus1100;
            break;
         case tmzUTCPlus1130:
            *TimeZoneResult = tzUTCPlus1130;
            break;
         case tmzUTCPlus1200:
            *TimeZoneResult = tzUTCPlus1200;
            break;
         case tmzUTCPlus1245:
            *TimeZoneResult = tzUTCPlus1245;
            break;
         case tmzUTCPlus1300:
            *TimeZoneResult = tzUTCPlus1300;
            break;
         case tmzUTCPlus1400:
            *TimeZoneResult = tzUTCPlus1400;
            break;
         case tmzUTCUnknown:
            *TimeZoneResult = tzUTCUnknown;
            break;
         default:
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
            break;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d.\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is a utility function that is used to      */
   /* convert a CTS Time Zone into a PM Time Zone.  This functions      */
   /* returns ZERO on success or a negative error code.                 */
static int ConvertCTSToTimeZone(CTS_Time_Zone_Type_t TimeZone, TIPM_Time_Zone_Type_t *TimeZoneResult)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if(TimeZoneResult)
   {
      /* Initialize the return value to success.                        */
      ret_val = 0;

      /* Do the necessary conversion.                                   */
      switch(TimeZone)
      {
         case tzUTCMinus1200:
            *TimeZoneResult = tmzUTCMinus1200;
            break;
         case tzUTCMinus1100:
            *TimeZoneResult = tmzUTCMinus1100;
            break;
         case tzUTCMinus1000:
            *TimeZoneResult = tmzUTCMinus1000;
            break;
         case tzUTCMinus930:
            *TimeZoneResult = tmzUTCMinus930;
            break;
         case tzUTCMinus900:
            *TimeZoneResult = tmzUTCMinus900;
            break;
         case tzUTCMinus800:
            *TimeZoneResult = tmzUTCMinus800;
            break;
         case tzUTCMinus700:
            *TimeZoneResult = tmzUTCMinus700;
            break;
         case tzUTCMinus600:
            *TimeZoneResult = tmzUTCMinus600;
            break;
         case tzUTCMinus500:
            *TimeZoneResult = tmzUTCMinus500;
            break;
         case tzUTCMinus430:
            *TimeZoneResult = tmzUTCMinus430;
            break;
         case tzUTCMinus400:
            *TimeZoneResult = tmzUTCMinus400;
            break;
         case tzUTCMinus330:
            *TimeZoneResult = tmzUTCMinus330;
            break;
         case tzUTCMinus300:
            *TimeZoneResult = tmzUTCMinus300;
            break;
         case tzUTCMinus200:
            *TimeZoneResult = tmzUTCMinus200;
            break;
         case tzUTCMinus100:
            *TimeZoneResult = tmzUTCMinus100;
            break;
         case tzUTCPlus000:
            *TimeZoneResult = tmzUTCPlus000;
            break;
         case tzUTCPlus100:
            *TimeZoneResult = tmzUTCPlus100;
            break;
         case tzUTCPlus200:
            *TimeZoneResult = tmzUTCPlus200;
            break;
         case tzUTCPlus300:
            *TimeZoneResult = tmzUTCPlus300;
            break;
         case tzUTCPlus330:
            *TimeZoneResult = tmzUTCPlus330;
            break;
         case tzUTCPlus400:
            *TimeZoneResult = tmzUTCPlus400;
            break;
         case tzUTCPlus430:
            *TimeZoneResult = tmzUTCPlus430;
            break;
         case tzUTCPlus500:
            *TimeZoneResult = tmzUTCPlus500;
            break;
         case tzUTCPlus530:
            *TimeZoneResult = tmzUTCPlus530;
            break;
         case tzUTCPlus545:
            *TimeZoneResult = tmzUTCPlus545;
            break;
         case tzUTCPlus600:
            *TimeZoneResult = tmzUTCPlus600;
            break;
         case tzUTCPlus630:
            *TimeZoneResult = tmzUTCPlus630;
            break;
         case tzUTCPlus700:
            *TimeZoneResult = tmzUTCPlus700;
            break;
         case tzUTCPlus800:
            *TimeZoneResult = tmzUTCPlus800;
            break;
         case tzUTCPlus845:
            *TimeZoneResult = tmzUTCPlus845;
            break;
         case tzUTCPlus900:
            *TimeZoneResult = tmzUTCPlus900;
            break;
         case tzUTCPlus930:
            *TimeZoneResult = tmzUTCPlus930;
            break;
         case tzUTCPlus1000:
            *TimeZoneResult = tmzUTCPlus1000;
            break;
         case tzUTCPlus1030:
            *TimeZoneResult = tmzUTCPlus1030;
            break;
         case tzUTCPlus1100:
            *TimeZoneResult = tmzUTCPlus1100;
            break;
         case tzUTCPlus1130:
            *TimeZoneResult = tmzUTCPlus1130;
            break;
         case tzUTCPlus1200:
            *TimeZoneResult = tmzUTCPlus1200;
            break;
         case tzUTCPlus1245:
            *TimeZoneResult = tmzUTCPlus1245;
            break;
         case tzUTCPlus1300:
            *TimeZoneResult = tmzUTCPlus1300;
            break;
         case tzUTCPlus1400:
            *TimeZoneResult = tmzUTCPlus1400;
            break;
         case tzUTCUnknown:
            *TimeZoneResult = tmzUTCUnknown;
            break;
         default:
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
            break;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d.\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is a utility function that is used to      */
   /* convert a PM Reference Time Information structure to a CTS        */
   /* Reference Time Information structure.  This functions returns ZERO*/
   /* on success or a negative error code.                              */
static int ConvertReferenceTimeToCTS(TIPM_Reference_Time_Information_Data_t *ReferenceTime, CTS_Reference_Time_Information_Data_t *ReferenceTimeResult)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if((ReferenceTime) && (ReferenceTimeResult))
   {
      /* Check to see if any of the fields are valid.                   */
      if(ReferenceTime->Flags & TIPM_REFERENCE_TIME_INFORMATION_FLAGS_REFERENCE_TIME_KNOWN)
      {
         /* Convert the PM Time Source to a CTS Time Source value.      */
         if(!(ret_val = ConvertTimeSourceToCTS(ReferenceTime->Source, &(ReferenceTimeResult->Source))))
         {
            /* Now check to see if the accuracy is known.               */
            if(ReferenceTime->Flags & TIPM_REFERENCE_TIME_INFORMATION_FLAGS_ACCURACY_KNOWN)
            {
               /* Check to see if the value is out of range.            */
               if(ReferenceTime->Accuracy > CTS_ACCURACY_MAXIMUM_VALUE)
                  ReferenceTimeResult->Accuracy = CTS_ACCURACY_OUT_OF_RANGE;
               else
                  ReferenceTimeResult->Accuracy = (Byte_t)ReferenceTime->Accuracy;
            }
            else
               ReferenceTimeResult->Accuracy = CTS_ACCURACY_UNKOWN;

            /* Check to see if the Days or Hours Since Update values are*/
            /* out of Range.                                            */
            if(ReferenceTime->Days_Since_Update >= 255)
            {
               ReferenceTimeResult->Days_Since_Update  = 255;
               ReferenceTimeResult->Hours_Since_Update = 255;
            }
            else
            {
               ReferenceTimeResult->Days_Since_Update  = (Byte_t)ReferenceTime->Days_Since_Update;
               ReferenceTimeResult->Hours_Since_Update = (Byte_t)ReferenceTime->Hours_Since_Update;
            }

            /* Return success to the caller.                            */
            ret_val = 0;
         }
      }
      else
      {
         /* Reference Time is not known so the fields to indicate this. */
         ReferenceTimeResult->Source             = tsUnknown;
         ReferenceTimeResult->Accuracy           = CTS_ACCURACY_UNKOWN;
         ReferenceTimeResult->Days_Since_Update  = 0;
         ReferenceTimeResult->Hours_Since_Update = 0;

         /* Return success to the caller.                               */
         ret_val                                 = 0;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d.\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function converts a CTS Current Time Data structure */
   /* to a PM Current Time Data structure. This function returns zero if*/
   /* successful or a negative error code.                              */
static int ConvertCTSToCurrentTime(CTS_Current_Time_Data_t *CurrentTime, TIPM_Current_Time_Data_t *CurrentTimeResult)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if((CurrentTime) && (CurrentTimeResult))
   {
      /* Verify the date-time information is valid.                     */
      if(CTS_DATE_TIME_VALID(CurrentTime->Exact_Time.Day_Date_Time.Date_Time))
      {
         /* Copy the Date_Time structure.                               */
         CurrentTimeResult->ExactTime.DayDateTime.DateTime.Year = (unsigned int)CurrentTime->Exact_Time.Day_Date_Time.Date_Time.Year;

         switch(CurrentTime->Exact_Time.Day_Date_Time.Date_Time.Month)
         {
            case myUnknown:
               CurrentTimeResult->ExactTime.DayDateTime.DateTime.Month = tmyUnknown;
               break;
            case myJanuary:
               CurrentTimeResult->ExactTime.DayDateTime.DateTime.Month = tmyJanuary;
               break;
            case myFebruary:
               CurrentTimeResult->ExactTime.DayDateTime.DateTime.Month = tmyFebruary;
               break;
            case myMarch:
               CurrentTimeResult->ExactTime.DayDateTime.DateTime.Month = tmyMarch;
               break;
            case myApril:
               CurrentTimeResult->ExactTime.DayDateTime.DateTime.Month = tmyApril;
               break;
            case myMay:
               CurrentTimeResult->ExactTime.DayDateTime.DateTime.Month = tmyMay;
               break;
            case myJune:
               CurrentTimeResult->ExactTime.DayDateTime.DateTime.Month = tmyJune;
               break;
            case myJuly:
               CurrentTimeResult->ExactTime.DayDateTime.DateTime.Month = tmyJuly;
               break;
            case myAugust:
               CurrentTimeResult->ExactTime.DayDateTime.DateTime.Month = tmyAugust;
               break;
            case mySeptember:
               CurrentTimeResult->ExactTime.DayDateTime.DateTime.Month = tmySeptember;
               break;
            case myOctober:
               CurrentTimeResult->ExactTime.DayDateTime.DateTime.Month = tmyOctober;
               break;
            case myNovember:
               CurrentTimeResult->ExactTime.DayDateTime.DateTime.Month = tmyNovember;
               break;
            case myDecember:
               CurrentTimeResult->ExactTime.DayDateTime.DateTime.Month = tmyDecember;
               break;
            default:
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         }

         CurrentTimeResult->ExactTime.DayDateTime.DateTime.Day     = (unsigned int)CurrentTime->Exact_Time.Day_Date_Time.Date_Time.Day;
         CurrentTimeResult->ExactTime.DayDateTime.DateTime.Hours   = (unsigned int)CurrentTime->Exact_Time.Day_Date_Time.Date_Time.Hours;
         CurrentTimeResult->ExactTime.DayDateTime.DateTime.Minutes = (unsigned int)CurrentTime->Exact_Time.Day_Date_Time.Date_Time.Minutes;
         CurrentTimeResult->ExactTime.DayDateTime.DateTime.Seconds = (unsigned int)CurrentTime->Exact_Time.Day_Date_Time.Date_Time.Seconds;

         /* Copy the day of week.                                       */
         switch(CurrentTime->Exact_Time.Day_Date_Time.Day_Of_Week)
         {
            case wdUnknown:
               CurrentTimeResult->ExactTime.DayDateTime.DayOfWeek = twdUnknown;
               break;
            case wdMonday:
               CurrentTimeResult->ExactTime.DayDateTime.DayOfWeek = twdMonday;
               break;
            case wdTuesday:
               CurrentTimeResult->ExactTime.DayDateTime.DayOfWeek = twdTuesday;
               break;
            case wdWednesday:
               CurrentTimeResult->ExactTime.DayDateTime.DayOfWeek = twdWednesday;
               break;
            case wdThursday:
               CurrentTimeResult->ExactTime.DayDateTime.DayOfWeek = twdThursday;
               break;
            case wdFriday:
               CurrentTimeResult->ExactTime.DayDateTime.DayOfWeek = twdFriday;
               break;
            case wdSaturday:
               CurrentTimeResult->ExactTime.DayDateTime.DayOfWeek = twdSaturday;
               break;
            case wdSunday:
               CurrentTimeResult->ExactTime.DayDateTime.DayOfWeek = twdSunday;
               break;
            default:
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         }

         /* Complete the Exact Time structure.                          */
         CurrentTimeResult->ExactTime.Fractions256 = CurrentTime->Exact_Time.Fractions256;

         /* Copy the adjust reason mask.                                */
         CurrentTimeResult->AdjustReasonMask = 0;

         if(CurrentTime->Adjust_Reason_Mask & CTS_CURRENT_TIME_ADJUST_REASON_MANUAL_TIME_UPDATE)
            CurrentTimeResult->AdjustReasonMask |= TIPM_CURRENT_TIME_ADJUST_REASON_MANUAL_TIME_UPDATE;

         if(CurrentTime->Adjust_Reason_Mask & CTS_CURRENT_TIME_ADJUST_REASON_EXTERNAL_REFERENCE_TIME_UPDATE)
            CurrentTimeResult->AdjustReasonMask |= TIPM_CURRENT_TIME_ADJUST_REASON_EXTERNAL_REFERENCE_TIME_UPDATE;

         if(CurrentTime->Adjust_Reason_Mask & CTS_CURRENT_TIME_ADJUST_REASON_CHANGE_OF_TIMEZONE)
            CurrentTimeResult->AdjustReasonMask |= TIPM_CURRENT_TIME_ADJUST_REASON_CHANGE_OF_TIMEZONE;

         if(CurrentTime->Adjust_Reason_Mask & CTS_CURRENT_TIME_ADJUST_REASON_CHANGE_OF_DST)
            CurrentTimeResult->AdjustReasonMask |= TIPM_CURRENT_TIME_ADJUST_REASON_CHANGE_OF_DST;

            /* Flag success.                                            */
            ret_val = 0;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d.\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function converts a CTS Local Time Data structure   */
   /* to a PM Local Time Data structure. This function returns zero if  */
   /* successful or a negative error code.                              */
static int ConvertCTSToLocalTime(CTS_Local_Time_Information_Data_t *LocalTime, TIPM_Local_Time_Information_Data_t *LocalTimeResult)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if((LocalTime) && (LocalTimeResult))
   {
      /* Attempt to convert the timezone.                               */
      if(!(ret_val = ConvertCTSToTimeZone(LocalTime->Time_Zone, &(LocalTimeResult->Time_Zone))))
      {
         /* Now map the DST Offset type.                                */
         switch(LocalTime->Daylight_Saving_Time)
         {
            case doStandardTime:
               LocalTimeResult->Daylight_Saving_Time = tmdStandardTime;
               break;
            case doHalfAnHourDaylightTime:
               LocalTimeResult->Daylight_Saving_Time = tmdHalfAnHourDaylightTime;
               break;
            case doDaylightTime:
               LocalTimeResult->Daylight_Saving_Time = tmdDaylightTime;
               break;
            case doDoubleDaylightTime:
               LocalTimeResult->Daylight_Saving_Time = tmdDoubleDaylightTime;
               break;
            case doUnknown:
               LocalTimeResult->Daylight_Saving_Time = tmdUnknown;
               break;
            default:
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         }
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d.\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function converts a CTS Reference Time Data         */
   /* structure to a PM Reference Time Data structure. This function    */
   /* returns zero if successful or a negative error code.              */
static int ConvertCTSToReferenceTime(CTS_Reference_Time_Information_Data_t *ReferenceTime, TIPM_Reference_Time_Information_Data_t *ReferenceTimeResult)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if((ReferenceTime) && (ReferenceTimeResult))
   {
      ret_val = 0;

      /* Map the Time Source.                                           */
      switch(ReferenceTime->Source)
      {
         case tsUnknown:
            ReferenceTimeResult->Source = tmsUnknown;
            break;
         case tsNetworkTimeProtocol:
            ReferenceTimeResult->Source = tmsNetworkTimeProtocol;
            break;
         case tsGps:
            ReferenceTimeResult->Source = tmsGps;
            break;
         case tsRadioTimeSignal:
            ReferenceTimeResult->Source = tmsRadioTimeSignal;
            break;
         case tsManual:
            ReferenceTimeResult->Source = tmsManual;
            break;
         case tsAtomicClock:
            ReferenceTimeResult->Source = tmsAtomicClock;
            break;
         case tsCellularNetwork:
            ReferenceTimeResult->Source = tmsCellularNetwork;
            break;
         default:
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }

      /* Map the accuracy data.                                         */
      ReferenceTimeResult->Accuracy           = (unsigned int)ReferenceTime->Accuracy;
      ReferenceTimeResult->Days_Since_Update  = (unsigned int)ReferenceTime->Days_Since_Update;
      ReferenceTimeResult->Hours_Since_Update = (unsigned int)ReferenceTime->Hours_Since_Update;
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d.\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function converts a NDCS Time With DST structure    */
   /* to a PM Time With DST structure. This function returns zero if    */
   /* successful or a negative error code.                              */
static int ConvertNDCSToTimeWithDST(NDCS_Time_With_Dst_Data_t *TimeWithDST, TIPM_Time_With_DST_Data_t *TimeWithDSTResult)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if((TimeWithDST) && (TimeWithDSTResult))
   {
      ret_val = 0;

      /* Convert the date_time.                                         */

      /* Determine the month.                                           */
      /* *NOTE* NDCS does define these values, so we will use the CTS,  */
      /*        constants since they are defined out of the same        */
      /*        characteristic specificiation.                          */
      switch(TimeWithDST->Date_Time.Month)
      {
         case CTS_MONTH_OF_YEAR_UNKNOWN:
            TimeWithDSTResult->DateTime.Month = tmyUnknown;
            break;
         case CTS_MONTH_OF_YEAR_JANUARY:
            TimeWithDSTResult->DateTime.Month = tmyJanuary;
            break;
         case CTS_MONTH_OF_YEAR_FEBRUARY:
            TimeWithDSTResult->DateTime.Month = tmyFebruary;
            break;
         case CTS_MONTH_OF_YEAR_MARCH:
            TimeWithDSTResult->DateTime.Month = tmyMarch;
            break;
         case CTS_MONTH_OF_YEAR_APRIL:
            TimeWithDSTResult->DateTime.Month = tmyApril;
            break;
         case CTS_MONTH_OF_YEAR_MAY:
            TimeWithDSTResult->DateTime.Month = tmyMay;
            break;
         case CTS_MONTH_OF_YEAR_JUNE:
            TimeWithDSTResult->DateTime.Month = tmyJune;
            break;
         case CTS_MONTH_OF_YEAR_JULY:
            TimeWithDSTResult->DateTime.Month = tmyJuly;
            break;
         case CTS_MONTH_OF_YEAR_AUGUST:
            TimeWithDSTResult->DateTime.Month = tmyAugust;
            break;
         case CTS_MONTH_OF_YEAR_SEPTEMBER:
            TimeWithDSTResult->DateTime.Month = tmySeptember;
            break;
         case CTS_MONTH_OF_YEAR_OCTOBER:
            TimeWithDSTResult->DateTime.Month = tmyOctober;
            break;
         case CTS_MONTH_OF_YEAR_NOVEMBER:
            TimeWithDSTResult->DateTime.Month = tmyNovember;
            break;
         case CTS_MONTH_OF_YEAR_DECEMBER:
            TimeWithDSTResult->DateTime.Month = tmyDecember;
            break;
         default:
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }

      /* Map the rest of the data.                                      */
      TimeWithDSTResult->DateTime.Year    = (unsigned int)TimeWithDST->Date_Time.Year;
      TimeWithDSTResult->DateTime.Day     = (unsigned int)TimeWithDST->Date_Time.Day;
      TimeWithDSTResult->DateTime.Hours   = (unsigned int)TimeWithDST->Date_Time.Hours;
      TimeWithDSTResult->DateTime.Minutes = (unsigned int)TimeWithDST->Date_Time.Minutes;
      TimeWithDSTResult->DateTime.Seconds = (unsigned int)TimeWithDST->Date_Time.Seconds;

      /* Map the DST Offset.                                            */
      /* *NOTE* NDCS does define these values, so we will use the CTS,  */
      /*        constants since they are defined out of the same        */
      /*        characteristic specificiation.                          */
      switch(TimeWithDST->Dst_Offset)
      {
         case CTS_DST_OFFSET_UNKNOWN:
            TimeWithDSTResult->DSTOffset = tmdUnknown;
            break;
         case CTS_DST_OFFSET_STANDARD_TIME:
            TimeWithDSTResult->DSTOffset = tmdStandardTime;
            break;
         case CTS_DST_OFFSET_HALF_AN_HOUR_DAYLIGHT_TIME:
            TimeWithDSTResult->DSTOffset = tmdHalfAnHourDaylightTime;
            break;
         case CTS_DST_OFFSET_DAYLIGHT_TIME:
            TimeWithDSTResult->DSTOffset = tmdDaylightTime;
            break;
         case CTS_DST_OFFSET_DOUBLE_DAYLIGHT_TIME:
            TimeWithDSTResult->DSTOffset = tmdDoubleDaylightTime;
            break;
         default:
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d.\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function converts a RTUS Time Update State structure*/
   /* to a PM Time Update State structure. This function returns zero if*/
   /* successful or a negative error code.                              */
static int ConvertRTUSToTimeUpdateState(RTUS_Time_Update_State_Data_t *TimeUpdateState, TIPM_Time_Update_State_Data_t *TimeUpdateStateResult)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if((TimeUpdateState) && (TimeUpdateStateResult))
   {
      ret_val = 0;

      /* Convert the current state.                                     */
      switch(TimeUpdateState->CurrentState)
      {
         case csIdle:
            TimeUpdateStateResult->CurrentState = tcsIdle;
            break;
         case csUpdatePending:
            TimeUpdateStateResult->CurrentState = tcsUpdatePending;
            break;
         default:
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }

      /* Map the result state.                                          */
      switch(TimeUpdateState->Result)
      {
         case reSuccessful:
            TimeUpdateStateResult->Result = treSuccessful;
            break;
         case reCanceled:
            TimeUpdateStateResult->Result = treCanceled;
            break;
         case reNoConnectionToReference:
            TimeUpdateStateResult->Result = treNoConnectionToReference;
            break;
         case reReferenceRespondedWithError:
            TimeUpdateStateResult->Result = treReferenceRespondedWithError;
            break;
         case reTimeout:
            TimeUpdateStateResult->Result = treTimeout;
            break;
         case reUpdateNotAttemptedAfterReset:
            TimeUpdateStateResult->Result = treUpdateNotAttemptedAfterReset;
            break;
         default:
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }

   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d.\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is a utility function that is used to      */
   /* convert the PM Local Time Information to CTS Local Time           */
   /* Information and set it in the CTS Service.  This functions returns*/
   /* ZERO on success or a negative error code.                         */
static int ProcessSetLocalTimeInformation(TIPM_Local_Time_Information_Data_t *LocalTimeInformation)
{
   int                               ret_val;
   CTS_Local_Time_Information_Data_t LocalTimeInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if(LocalTimeInformation)
   {
      /* Convert the PM Time Zone to a CTS Time Zone value.             */
      if(!(ret_val = ConvertTimeZoneToCTS(LocalTimeInformation->Time_Zone, &(LocalTimeInfo.Time_Zone))))
      {
         /* Convert the PM DST Offset to a CTS DST Offset value.        */
         if(!(ret_val = ConvertDSTOffsetToCTS(LocalTimeInformation->Daylight_Saving_Time, &(LocalTimeInfo.Daylight_Saving_Time))))
         {
            /* Simply set the converted Local Time Information.         */
            ret_val = _TIPM_CTS_Set_Local_Time_Information(&LocalTimeInfo);
         }
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d.\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is a utility function that is used to      */
   /* process a Reference Time Information Request Response.  This      */
   /* functions returns ZERO on success or a negative error code.       */
static int ProcessReferenceTimeInformationResponse(TIPM_Reference_Time_Information_Data_t *ReferenceTimeInformation)
{
   int                                     ret_val;
   Connection_Entry_t                     *ConnectionEntryPtr;
   CTS_Reference_Time_Information_Data_t   CTSReferenceTimeInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if((ReferenceTimeRequestOutstanding) && (ReferenceTimeInformation))
   {
      /* Convert the PM Reference Time Information to a CTS Reference   */
      /* Time Information value.                                        */
      if(!(ret_val = ConvertReferenceTimeToCTS(ReferenceTimeInformation, &CTSReferenceTimeInfo)))
      {
         /* Walk the Connection List and send the response to anybody   */
         /* that is waiting.                                            */
         ConnectionEntryPtr = ConnectionEntryList;
         ret_val            = 0;
         while((ConnectionEntryPtr) && (!ret_val))
         {
            /* Check to see if this device is awaiting a response.      */
            if(ConnectionEntryPtr->ReferenceTimeRequestTransactionID)
            {
               /* Respond to the request.                               */
               ret_val = _TIPM_CTS_Reference_Time_Information_Read_Request_Response(ConnectionEntryPtr->ReferenceTimeRequestTransactionID, 0, &CTSReferenceTimeInfo);
               if(!ret_val)
                  ConnectionEntryPtr->ReferenceTimeRequestTransactionID = 0;
            }

            ConnectionEntryPtr = ConnectionEntryPtr->NextConnectionEntryPtr;
         }

         /* If successful flag that the Reference Time Request is no    */
         /* longer outstanding.                                         */
         if(!ret_val)
            ReferenceTimeRequestOutstanding = FALSE;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d.\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is a utility function that is used to      */
   /* process a Update Current Time Request.  This functions returns    */
   /* ZERO on success or a negative error code.                         */
static int ProcessUpdateCurrentTime(unsigned int TransactionID, unsigned long AdjustMask)
{
   int                      ret_val;
   Connection_Entry_t      *ConnectionEntryPtr;
   CTS_Current_Time_Data_t  CurrentTime;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08lX\n", AdjustMask));

   /* Attempt to get the current time.                                  */
   if(!_TIPM_Get_Current_Time(&CurrentTime, AdjustMask))
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("CTS - Date: %u/%u/%u, Day of Week: %u\n", (unsigned int)CurrentTime.Exact_Time.Day_Date_Time.Date_Time.Month, (unsigned int)CurrentTime.Exact_Time.Day_Date_Time.Date_Time.Day, (unsigned int)CurrentTime.Exact_Time.Day_Date_Time.Date_Time.Year, (unsigned int)CurrentTime.Exact_Time.Day_Date_Time.Day_Of_Week));
      DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("CTS - Time: %u:%u:%u\n", (unsigned int)CurrentTime.Exact_Time.Day_Date_Time.Date_Time.Hours, (unsigned int)CurrentTime.Exact_Time.Day_Date_Time.Date_Time.Minutes, (unsigned int)CurrentTime.Exact_Time.Day_Date_Time.Date_Time.Seconds));
      DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("CTS - Adjust Mask: 0x%02X\n", CurrentTime.Adjust_Reason_Mask));

      /* Check to see if this result was initiated due to a Read        */
      /* Request.  If so respond to the read, otherwise we will walk the*/
      /* connection list and notify any currently connected TIP Clients.*/
      if(TransactionID)
      {
         /* Respond to the Read Request.                                */
         ret_val = _TIPM_CTS_Current_Time_Read_Request_Response(TransactionID, 0, &CurrentTime);
      }
      else
      {
         /* Walk the connection list and notify any connected TIP       */
         /* Clients that have registered for Current Time Notifications.*/
         ConnectionEntryPtr = ConnectionEntryList;
         ret_val            = 0;
         while((ConnectionEntryPtr) && (!ret_val))
         {
            /* Attempt to send a Current Time Notification to this      */
            /* Connection if it has registered for notifications.       */
            if(ConnectionEntryPtr->CurrentTimeCCCD & GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_NOTIFY_ENABLE)
               ret_val = _TIPM_CTS_Notify_Current_Time(ConnectionEntryPtr->ConnectionID, &CurrentTime);

            ConnectionEntryPtr = ConnectionEntryPtr->NextConnectionEntryPtr;
         }
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("gettimeofday() Error.\n"));

      /* If this occurred because the of a read request respond with an */
      /* error here.                                                    */
      if(TransactionID)
         _TIPM_CTS_Current_Time_Read_Request_Response(TransactionID, ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR, NULL);

      ret_val = BTPM_ERROR_CODE_TIME_UNABLE_TO_READ_TIME;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d.\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is a utility function to handle a Get      */
   /* Current Time request from either client or server call.           */
static int ProcessGetCurrentTime(unsigned int ClientEventHandlerID, BD_ADDR_t BD_ADDR)
{
   int                  ret_val;
   unsigned int         RequiredFlags;
   Connection_Entry_t  *ConnectionEntry;
   Transaction_Entry_t  TransactionEntry;
   Transaction_Entry_t *TransactionEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check if we are tracking this connection                          */
   if((ConnectionEntry = SearchConnectionEntryByBD_ADDR(&ConnectionEntryList, BD_ADDR, tctClient)) != NULL)
   {
      /* Note the supported services we need for this command.          */
      RequiredFlags = (TIPM_SUPPORTED_SERVICES_CTS | TIPM_CTS_SUPPORTED_CHARACTERISTICS_CURRENT_TIME);

      /* Ensure we support this transaction on this connection.         */
      if((ConnectionEntry->ClientConnectionInfo.SupportedServicesMask & RequiredFlags) == RequiredFlags)
      {
         /* Format the new transaction.                                 */
         BTPS_MemInitialize(&TransactionEntry, 0, sizeof(Transaction_Entry_t));

         TransactionEntry.TransactionID   = GetNextTransactionID();
         TransactionEntry.TransactionType = ttGetCurrentTime;
         TransactionEntry.BD_ADDR         = ConnectionEntry->BD_ADDR;
         TransactionEntry.CallbackID      = ClientEventHandlerID;

         /* Attempt to add the transaction to the list.                 */
         if((TransactionEntryPtr = AddTransactionEntry(&TransactionList, &TransactionEntry)) != NULL)
         {
            /* Go ahead and attempt to submit the request               */
            if((ret_val = _TIPM_Get_Current_Time_Request(ConnectionEntry->ConnectionID, ConnectionEntry->ClientConnectionInfo.CurrentTimeHandle)) > 0)
            {
               /* Note the GATT Transaction ID.                         */
               TransactionEntryPtr->GATTTransactionID = (unsigned int)ret_val;

               /* Return the ID to the caller.                          */
               ret_val = TransactionEntryPtr->TransactionID;
            }
            else
            {
               /* Failured sending the request. Clean up the            */
               /* transaction.                                          */
               if((TransactionEntryPtr = DeleteTransactionEntry(&TransactionList, TransactionEntryPtr->GATTTransactionID)) != NULL)
                  FreeTransactionEntryMemory(TransactionEntryPtr);
            }
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
      }
      else
         ret_val = BTPM_ERROR_CODE_TIME_REMOTE_OPERATION_NOT_SUPPORTED;
   }
   else
      ret_val = BTPM_ERROR_CODE_TIME_CLIENT_NOT_CONNECTED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d.\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is a utility function to handle a Enable   */
   /* Time Notifications request from either client or server call.     */
static int ProcessEnableTimeNotifications(unsigned int ClientEventHandlerID, BD_ADDR_t BD_ADDR, Boolean_t Enable)
{
   int                                ret_val = 0;
   Boolean_t                          ShouldIssueRemoteCommand = FALSE;
   unsigned int                       RequiredFlags;
   Connection_Entry_t                *ConnectionEntry;
   Transaction_Entry_t                TransactionEntry;
   Transaction_Entry_t               *TransactionEntryPtr;
   Notification_Registration_Entry_t  NotificationRegistrationEntry;
   Notification_Registration_Entry_t *NotificationRegistrationEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: Enable: %d\n", Enable));

   /* Check if we are tracking this connection                          */
   if((ConnectionEntry = SearchConnectionEntryByBD_ADDR(&ConnectionEntryList, BD_ADDR, tctClient)) != NULL)
   {
      /* Note the supported services we need for this command.          */
      RequiredFlags = (TIPM_SUPPORTED_SERVICES_CTS | TIPM_CTS_SUPPORTED_CHARACTERISTICS_CURRENT_TIME);

      /* Ensure we support this transaction on this connection.         */
      if((ConnectionEntry->ClientConnectionInfo.SupportedServicesMask & RequiredFlags) == RequiredFlags)
      {
         /* Check if we are enabling or disabling notifications.        */
         if(Enable)
         {
            /* Format the registration entry.                           */
            BTPS_MemInitialize(&NotificationRegistrationEntry, 0, sizeof(Notification_Registration_Entry_t));

            NotificationRegistrationEntry.CallbackID = ClientEventHandlerID;
            NotificationRegistrationEntry.BD_ADDR    = BD_ADDR;

            /* Attempt to add the the registration to the list.         */
            if(AddNotificationRegistrationEntry(&NotificationRegistrationList, &NotificationRegistrationEntry))
            {
               /* If we have not enabled notifications on the remote    */
               /* device, we need to do so.                             */
   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Flags: 0x%08lX\n", ConnectionEntry->Flags));
               if(!(ConnectionEntry->Flags & (CONNECTION_ENTRY_FLAGS_NOTIFICATION_ENABLE_PENDING | CONNECTION_ENTRY_FLAGS_NOTIFICATION_ENABLED)))
                  ShouldIssueRemoteCommand = TRUE;
            }
            else
               ret_val = BTPM_ERROR_CODE_TIME_NOTIFICATIONS_ALREADY_ENABLED;
         }
         else
         {
            /* Go ahead and attempt to delete the registration.         */
            if((NotificationRegistrationEntryPtr = DeleteNotificationRegistrationEntry(&NotificationRegistrationList, &ClientEventHandlerID, &BD_ADDR)) != NULL)
               FreeNotificationRegistrationEntryMemory(NotificationRegistrationEntryPtr);

            /* If we are the last registered for notifications, go ahead*/
            /* and un-register with the server.                         */
            if(!NotificationRegistrationList)
               ShouldIssueRemoteCommand = TRUE;
         }

         /* If we need to submit the command, do so.                    */
         if(ShouldIssueRemoteCommand)
         {
            /* Format the transaction entry.                            */
            BTPS_MemInitialize(&TransactionEntry, 0, sizeof(Transaction_Entry_t));

            TransactionEntry.TransactionID   = GetNextTransactionID();
            TransactionEntry.TransactionType = Enable?ttEnableNotifications:ttDisableNotifications;
            TransactionEntry.BD_ADDR         = ConnectionEntry->BD_ADDR;
            TransactionEntry.CallbackID      = ClientEventHandlerID;

            /* Attempt to add the transaction.                          */
            if((TransactionEntryPtr = AddTransactionEntry(&TransactionList, & TransactionEntry)) != NULL)
            {
               /* Submit the request.                                   */
               ret_val = _TIPM_Enable_Time_Notifications(ConnectionEntry->ConnectionID, ConnectionEntry->ClientConnectionInfo.CurrentTimeCCD, Enable);

               /* If we are enabling, and we fail, we need to clean up. */
               if(ret_val < 0)
               {
                  if(Enable)
                  {
                     /* Remove the transaction.                         */
                     if((TransactionEntryPtr = DeleteTransactionEntry(&TransactionList, TransactionEntryPtr->GATTTransactionID)) != NULL)
                        FreeTransactionEntryMemory(TransactionEntryPtr);

                     /* Remove the registration.                        */
                     if((NotificationRegistrationEntryPtr = DeleteNotificationRegistrationEntry(&NotificationRegistrationList, &ClientEventHandlerID, &BD_ADDR)) != NULL)
                        FreeNotificationRegistrationEntryMemory(NotificationRegistrationEntryPtr);
                  }
               }
               else
               {
                  /* Note the GATT Transaction ID.                      */
                  TransactionEntryPtr->GATTTransactionID = ret_val;

                  /* Flag success.                                      */
                  ret_val = 0;

                  if(Enable)
                  {
                     /* Flag that we are pending notifications enabling.*/
                     ConnectionEntry->Flags |= CONNECTION_ENTRY_FLAGS_NOTIFICATION_ENABLE_PENDING;
                  }
                  else
                  {
                     /* Flag that we are pending notifications          */
                     /* disabling.                                      */
                     ConnectionEntry->Flags &= ~((unsigned long)CONNECTION_ENTRY_FLAGS_NOTIFICATION_ENABLED);
                     ConnectionEntry->Flags |= (unsigned long)CONNECTION_ENTRY_FLAGS_NOTIFICATION_DISABLE_PENDING;
                  }
               }
            }
         }
      }
      else
         ret_val = BTPM_ERROR_CODE_TIME_REMOTE_OPERATION_NOT_SUPPORTED;
   }
   else
      ret_val = BTPM_ERROR_CODE_TIME_CLIENT_NOT_CONNECTED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d.\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is a utility function to handle a Get Local*/
   /* Time Information request from either client or server call.       */
static int ProcessGetLocalTimeInformation(unsigned int ClientEventHandlerID, BD_ADDR_t BD_ADDR)
{
   int                  ret_val;
   unsigned int         RequiredFlags;
   Connection_Entry_t  *ConnectionEntry;
   Transaction_Entry_t  TransactionEntry;
   Transaction_Entry_t *TransactionEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check if we are tracking this connection                          */
   if((ConnectionEntry = SearchConnectionEntryByBD_ADDR(&ConnectionEntryList, BD_ADDR, tctClient)) != NULL)
   {
      /* Note the supported services we need for this command.          */
      RequiredFlags = (TIPM_SUPPORTED_SERVICES_CTS | TIPM_CTS_SUPPORTED_CHARACTERISTICS_LOCAL_TIME_INFO);

      /* Ensure we support this transaction on this connection.         */
      if((ConnectionEntry->ClientConnectionInfo.SupportedServicesMask & RequiredFlags) == RequiredFlags)
      {
         /* Format the new transaction.                                 */
         BTPS_MemInitialize(&TransactionEntry, 0, sizeof(Transaction_Entry_t));

         TransactionEntry.TransactionID   = GetNextTransactionID();
         TransactionEntry.TransactionType = ttGetLocalTimeInformation;
         TransactionEntry.BD_ADDR         = ConnectionEntry->BD_ADDR;
         TransactionEntry.CallbackID      = ClientEventHandlerID;

         /* Attempt to add the transaction to the list.                 */
         if((TransactionEntryPtr = AddTransactionEntry(&TransactionList, &TransactionEntry)) != NULL)
         {
            /* Go ahead and attempt to submit the request               */
            if((ret_val = _TIPM_Get_Local_Time_Information(ConnectionEntry->ConnectionID, ConnectionEntry->ClientConnectionInfo.LocalTimeInformationHandle)) > 0)
            {
               /* Note the GATT Transaction ID.                         */
               TransactionEntryPtr->GATTTransactionID = (unsigned int)ret_val;

               /* Return the ID to the caller.                          */
               ret_val = TransactionEntryPtr->TransactionID;
            }
            else
            {
               /* Failured sending the request. Clean up the            */
               /* transaction.                                          */
               if((TransactionEntryPtr = DeleteTransactionEntry(&TransactionList, TransactionEntryPtr->GATTTransactionID)) != NULL)
                  FreeTransactionEntryMemory(TransactionEntryPtr);
            }
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
      }
      else
         ret_val = BTPM_ERROR_CODE_TIME_REMOTE_OPERATION_NOT_SUPPORTED;
   }
   else
      ret_val = BTPM_ERROR_CODE_TIME_CLIENT_NOT_CONNECTED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d.\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is a utility function to handle a Get Time */
   /* Accuracy request from either client or server call.               */
static int ProcessGetTimeAccuracy(unsigned int ClientEventHandlerID, BD_ADDR_t BD_ADDR)
{
   int                  ret_val;
   unsigned int         RequiredFlags;
   Connection_Entry_t  *ConnectionEntry;
   Transaction_Entry_t  TransactionEntry;
   Transaction_Entry_t *TransactionEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check if we are tracking this connection                          */
   if((ConnectionEntry = SearchConnectionEntryByBD_ADDR(&ConnectionEntryList, BD_ADDR, tctClient)) != NULL)
   {
      /* Note the supported services we need for this command.          */
      RequiredFlags = (TIPM_SUPPORTED_SERVICES_CTS | TIPM_CTS_SUPPORTED_CHARACTERISTICS_REFERENCE_TIME_INFO);

      /* Ensure we support this transaction on this connection.         */
      if((ConnectionEntry->ClientConnectionInfo.SupportedServicesMask & RequiredFlags) == RequiredFlags)
      {
         /* Format the new transaction.                                 */
         BTPS_MemInitialize(&TransactionEntry, 0, sizeof(Transaction_Entry_t));

         TransactionEntry.TransactionID   = GetNextTransactionID();
         TransactionEntry.TransactionType = ttGetTimeAccuracy;
         TransactionEntry.BD_ADDR         = ConnectionEntry->BD_ADDR;
         TransactionEntry.CallbackID      = ClientEventHandlerID;

         /* Attempt to add the transaction to the list.                 */
         if((TransactionEntryPtr = AddTransactionEntry(&TransactionList, &TransactionEntry)) != NULL)
         {
            /* Go ahead and attempt to submit the request               */
            if((ret_val = _TIPM_Get_Time_Accuracy(ConnectionEntry->ConnectionID, ConnectionEntry->ClientConnectionInfo.ReferenceTimeInformationHandle)) > 0)
            {
               /* Note the GATT Transaction ID.                         */
               TransactionEntryPtr->GATTTransactionID = (unsigned int)ret_val;

               /* Return the ID to the caller.                          */
               ret_val = TransactionEntryPtr->TransactionID;
            }
            else
            {
               /* Failured sending the request. Clean up the            */
               /* transaction.                                          */
               if((TransactionEntryPtr = DeleteTransactionEntry(&TransactionList, TransactionEntryPtr->GATTTransactionID)) != NULL)
                  FreeTransactionEntryMemory(TransactionEntryPtr);
            }
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
      }
      else
         ret_val = BTPM_ERROR_CODE_TIME_REMOTE_OPERATION_NOT_SUPPORTED;
   }
   else
      ret_val = BTPM_ERROR_CODE_TIME_CLIENT_NOT_CONNECTED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d.\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is a utility function to handle a Get Next */
   /* DST Change request from either client or server call.             */
static int ProcessGetNextDSTChange(unsigned int ClientEventHandlerID, BD_ADDR_t BD_ADDR)
{
   int                  ret_val;
   Connection_Entry_t  *ConnectionEntry;
   Transaction_Entry_t  TransactionEntry;
   Transaction_Entry_t *TransactionEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check if we are tracking this connection                          */
   if((ConnectionEntry = SearchConnectionEntryByBD_ADDR(&ConnectionEntryList, BD_ADDR, tctClient)) != NULL)
   {
      /* Ensure we support this transaction on this connection.         */
      if(ConnectionEntry->ClientConnectionInfo.SupportedServicesMask & TIPM_SUPPORTED_SERVICES_NDCS)
      {
         /* Format the new transaction.                                 */
         BTPS_MemInitialize(&TransactionEntry, 0, sizeof(Transaction_Entry_t));

         TransactionEntry.TransactionID   = GetNextTransactionID();
         TransactionEntry.TransactionType = ttGetNextDSTChangeInformation;
         TransactionEntry.BD_ADDR         = ConnectionEntry->BD_ADDR;
         TransactionEntry.CallbackID      = ClientEventHandlerID;

         /* Attempt to add the transaction to the list.                 */
         if((TransactionEntryPtr = AddTransactionEntry(&TransactionList, &TransactionEntry)) != NULL)
         {
            /* Go ahead and attempt to submit the request               */
            if((ret_val = _TIPM_Get_Next_DST_Change_Information(ConnectionEntry->ConnectionID, ConnectionEntry->ClientConnectionInfo.TimeWithDSTHandle)) > 0)
            {
               /* Note the GATT Transaction ID.                         */
               TransactionEntryPtr->GATTTransactionID = (unsigned int)ret_val;

               /* Return the ID to the caller.                          */
               ret_val = TransactionEntryPtr->TransactionID;
            }
            else
            {
               /* Failured sending the request. Clean up the            */
               /* transaction.                                          */
               if((TransactionEntryPtr = DeleteTransactionEntry(&TransactionList, TransactionEntryPtr->GATTTransactionID)) != NULL)
                  FreeTransactionEntryMemory(TransactionEntryPtr);
            }
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
      }
      else
         ret_val = BTPM_ERROR_CODE_TIME_REMOTE_OPERATION_NOT_SUPPORTED;
   }
   else
      ret_val = BTPM_ERROR_CODE_TIME_CLIENT_NOT_CONNECTED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d.\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is a utility function to handle a Reference*/
   /* Time Update State request from either client or server call.      */
static int ProcessGetReferenceTimeUpdateState(unsigned int ClientEventHandlerID, BD_ADDR_t BD_ADDR)
{
   int                  ret_val;
   Connection_Entry_t  *ConnectionEntry;
   Transaction_Entry_t  TransactionEntry;
   Transaction_Entry_t *TransactionEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check if we are tracking this connection                          */
   if((ConnectionEntry = SearchConnectionEntryByBD_ADDR(&ConnectionEntryList, BD_ADDR, tctClient)) != NULL)
   {
      /* Ensure we support this transaction on this connection.         */
      if(ConnectionEntry->ClientConnectionInfo.SupportedServicesMask & TIPM_SUPPORTED_SERVICES_RTUS)
      {
         /* Format the new transaction.                                 */
         BTPS_MemInitialize(&TransactionEntry, 0, sizeof(Transaction_Entry_t));

         TransactionEntry.TransactionID   = GetNextTransactionID();
         TransactionEntry.TransactionType = ttGetReferenceTimeUpdateState;
         TransactionEntry.BD_ADDR         = ConnectionEntry->BD_ADDR;
         TransactionEntry.CallbackID      = ClientEventHandlerID;

         /* Attempt to add the transaction to the list.                 */
         if((TransactionEntryPtr = AddTransactionEntry(&TransactionList, &TransactionEntry)) != NULL)
         {
            /* Go ahead and attempt to submit the request               */
            if((ret_val = _TIPM_Get_Reference_Time_Update_State(ConnectionEntry->ConnectionID, ConnectionEntry->ClientConnectionInfo.TimeUpdateStateHandle)) > 0)
            {
               /* Note the GATT Transaction ID.                         */
               TransactionEntryPtr->GATTTransactionID = (unsigned int)ret_val;

               /* Return the ID to the caller.                          */
               ret_val = TransactionEntryPtr->TransactionID;
            }
            else
            {
               /* Failured sending the request. Clean up the            */
               /* transaction.                                          */
               if((TransactionEntryPtr = DeleteTransactionEntry(&TransactionList, TransactionEntryPtr->GATTTransactionID)) != NULL)
                  FreeTransactionEntryMemory(TransactionEntryPtr);
            }
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
      }
      else
         ret_val = BTPM_ERROR_CODE_TIME_REMOTE_OPERATION_NOT_SUPPORTED;
   }
   else
      ret_val = BTPM_ERROR_CODE_TIME_CLIENT_NOT_CONNECTED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d.\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is a utility function to handle a Request  */
   /* Reference Time Update request from either client or server call.  */
static int ProcessRequestReferenceTimeUpdate(unsigned int ClientEventHandlerID, BD_ADDR_t BD_ADDR)
{
   int                 ret_val;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check if we are tracking this connection                          */
   if((ConnectionEntry = SearchConnectionEntryByBD_ADDR(&ConnectionEntryList, BD_ADDR, tctClient)) != NULL)
   {
      /* Ensure we support this transaction on this connection.         */
      if(ConnectionEntry->ClientConnectionInfo.SupportedServicesMask & TIPM_SUPPORTED_SERVICES_RTUS)
      {
         /* Go ahead and attempt to submit the request                  */
         /* * NOTE * There is no need to track this request as it uses  */
         /*          write_without_response.                            */
         if((ret_val = _TIPM_Request_Reference_Time_Update(ConnectionEntry->ConnectionID, ConnectionEntry->ClientConnectionInfo.TimeUpdateControlPointHandle)) > 0)
         {
            /* Return success to the caller.                            */
            ret_val = 0;
         }
      }
      else
         ret_val = BTPM_ERROR_CODE_TIME_REMOTE_OPERATION_NOT_SUPPORTED;
   }
   else
      ret_val = BTPM_ERROR_CODE_TIME_CLIENT_NOT_CONNECTED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d.\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is a utility function to handle a Query    */
   /* Connected Devices request from either client or server call.      */
static int ProcessQueryConnectedDevices(TIPM_Connection_Type_t ConnectionType, unsigned int MaximumNumberDevices, TIPM_Remote_Device_t *RemoteDevices, unsigned int *TotalNumberDevices)
{
   int                 ret_val;
   unsigned int        NumberDevices;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Initialize at zero devices copied.                                */
   ret_val       = 0;
   NumberDevices = 0;

   /* Start at the front of the connection list.                        */
   ConnectionEntry = ConnectionEntryList;

   /* Walk the list of connections.                                     */
   while(ConnectionEntry)
   {
      /* Determine if this connection matches the type we are counting. */
      if(ConnectionEntry->ConnectionType == ConnectionType)
      {
         /* If this is a client connection, make sure we are actually   */
         /* connected to TIP.                                           */
         if((ConnectionType == tctServer) || ((ConnectionType == tctClient) && (ConnectionEntry->ClientConnectionInfo.SupportedServicesMask & TIPM_SUPPORTED_SERVICES_CTS)))
         {

            /* Copy the data if we have room in the buffer.             */
            if((RemoteDevices) && ((unsigned int)ret_val < MaximumNumberDevices))
            {
               RemoteDevices[ret_val].ConnectionType        = ConnectionType;
               RemoteDevices[ret_val].RemoteDeviceAddress   = ConnectionEntry->BD_ADDR;
               RemoteDevices[ret_val].SupportedServicesMask = ConnectionEntry->ClientConnectionInfo.SupportedServicesMask;

               /* Increment the number of devices we have copied.       */
               ret_val++;
            }

            /* Increment the total number of devices.                   */
            NumberDevices++;
         }
      }

      /* Advance to the next entry.                                     */
      ConnectionEntry = ConnectionEntry->NextConnectionEntryPtr;
   }

   /* Set the Total Number parameter if supplied.                       */
   if(TotalNumberDevices)
      *TotalNumberDevices = NumberDevices;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d.\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is a utlility function to parse LE service */
   /* data for the CTS service.                                         */
static void ConfigureCTSClient(Connection_Entry_t *ConnectionEntry, GATT_Service_Discovery_Indication_Data_t *Service)
{
   Word_t                                        CurrentTimeHandle              = 0;
   Word_t                                        CurrentTimeCCD                 = 0;
   Word_t                                        LocalTimeInformationHandle     = 0;
   Word_t                                        ReferenceTimeInformationHandle = 0;
   unsigned int                                  Index;
   unsigned int                                  Index1;
   unsigned int                                  NewServicesMask = 0;
   GATT_Characteristic_Information_t            *Characteristic;
   GATT_Characteristic_Descriptor_Information_t *Descriptor;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure that the input parameters appear to be semi-valid.      */
   if((ConnectionEntry) && (Service))
   {
      /* Grab the first characteristic.                                 */
      Characteristic = Service->CharacteristicInformationList;

      /* Look through each charactertistic.                             */
      for(Index=0;Index<Service->NumberOfCharacteristics;Index++,Characteristic++)
      {
         /* We only care about UUID_16.                                 */
         if(Characteristic->Characteristic_UUID.UUID_Type == guUUID_16)
         {
            /* Check for the Current Time Characteristic.               */
            if(CTS_COMPARE_CTS_CURRENT_TIME_UUID_TO_UUID_16(Characteristic->Characteristic_UUID.UUID.UUID_16))
            {
               /* Note the handle.                                      */
               CurrentTimeHandle = Characteristic->Characteristic_Handle;

               Descriptor = Characteristic->DescriptorList;

               /* Look for the CCCD.                                    */
               for(Index1=0;Index1<Characteristic->NumberOfDescriptors;Index1++,Descriptor++)
               {
                  /* Found the CCCD.                                    */
                  if((Descriptor->Characteristic_Descriptor_UUID.UUID_Type == guUUID_16) && (GATT_COMPARE_CLIENT_CHARACTERISTIC_CONFIGURATION_ATTRIBUTE_TYPE_TO_BLUETOOTH_UUID_16(Descriptor->Characteristic_Descriptor_UUID.UUID.UUID_16)))
                     CurrentTimeCCD = Descriptor->Characteristic_Descriptor_Handle;
               }

               /* Only support this service if we find both handles.    */
               if((CurrentTimeHandle) && (CurrentTimeCCD))
                  NewServicesMask |= (TIPM_SUPPORTED_SERVICES_CTS | TIPM_CTS_SUPPORTED_CHARACTERISTICS_CURRENT_TIME);
            }
            else
            {
               /* Check for Local Time Info.                            */
               if(CTS_COMPARE_CTS_LOCAL_TIME_INFORMATION_UUID_TO_UUID_16(Characteristic->Characteristic_UUID.UUID.UUID_16))
               {
                  /* Note the handle.                                   */
                  LocalTimeInformationHandle  = Characteristic->Characteristic_Handle;

                  /* Only flag we support it if the handle is valid.    */
                  if(LocalTimeInformationHandle)
                     NewServicesMask |= TIPM_CTS_SUPPORTED_CHARACTERISTICS_LOCAL_TIME_INFO;
               }
               else
               {
                  /*Check for Reference Time Info.                      */
                  if(CTS_COMPARE_CTS_REFERENCE_TIME_INFORMATION_UUID_TO_UUID_16(Characteristic->Characteristic_UUID.UUID.UUID_16))
                  {
                     /* Note the handle.                                */
                     ReferenceTimeInformationHandle = Characteristic->Characteristic_Handle;

                     /* Only flag we support it if the handle is valid. */
                     if(ReferenceTimeInformationHandle)
                        NewServicesMask |= TIPM_CTS_SUPPORTED_CHARACTERISTICS_REFERENCE_TIME_INFO;
                  }
               }
            }
         }
      }

      /* Current Time is Mandatory, so we only support service if we    */
      /* found Current Time.                                            */
      if(NewServicesMask & TIPM_SUPPORTED_SERVICES_CTS)
      {
         /* Update the tracking structure.                              */
         ConnectionEntry->ClientConnectionInfo.SupportedServicesMask         |= NewServicesMask;
         ConnectionEntry->ClientConnectionInfo.CurrentTimeHandle              = CurrentTimeHandle;
         ConnectionEntry->ClientConnectionInfo.CurrentTimeCCD                 = CurrentTimeCCD;
         ConnectionEntry->ClientConnectionInfo.LocalTimeInformationHandle     = LocalTimeInformationHandle;
         ConnectionEntry->ClientConnectionInfo.ReferenceTimeInformationHandle = ReferenceTimeInformationHandle;

         DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Configured CTS Successfully:\n"));
         DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Mask %08X:\n", ConnectionEntry->ClientConnectionInfo.SupportedServicesMask));
         DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Time Handle %d:\n", ConnectionEntry->ClientConnectionInfo.CurrentTimeHandle));
         DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Time CCD %d:\n", ConnectionEntry->ClientConnectionInfo.CurrentTimeCCD));
         DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Local Time Handle: %d\n", ConnectionEntry->ClientConnectionInfo.LocalTimeInformationHandle));
         DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Ref Time Handle: %d\n", ConnectionEntry->ClientConnectionInfo.ReferenceTimeInformationHandle));

      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utlility function to parse LE service */
   /* data for the NDCS service.                                        */
static void ConfigureNDCSClient(Connection_Entry_t *ConnectionEntry, GATT_Service_Discovery_Indication_Data_t *Service)
{
   unsigned int                       Index;
   GATT_Characteristic_Information_t *Characteristic;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure that the input parameters appear to be semi-valid.      */
   if((ConnectionEntry) && (Service))
   {
      /* Grab the first characteristic.                                 */
      Characteristic = Service->CharacteristicInformationList;

      /* Look through each charactertistic.                             */
      for(Index=0;Index<Service->NumberOfCharacteristics;Index++,Characteristic++)
      {
         /* We only care about UUID_16.                                 */
         if(Characteristic->Characteristic_UUID.UUID_Type == guUUID_16)
         {
            /* Look for the Time With DST characteristic.               */
            if(NDCS_COMPARE_NDCS_TIME_WITH_DST_UUID_TO_UUID_16(Characteristic->Characteristic_UUID.UUID.UUID_16))
            {
               /* Only flag success if valid.                           */
               if((ConnectionEntry->ClientConnectionInfo.TimeWithDSTHandle = Characteristic->Characteristic_Handle) != 0)
                  ConnectionEntry->ClientConnectionInfo.SupportedServicesMask |= TIPM_SUPPORTED_SERVICES_NDCS;
            }
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utlility function to parse LE service */
   /* data for the RTUS service.                                        */
static void ConfigureRTUSClient(Connection_Entry_t *ConnectionEntry, GATT_Service_Discovery_Indication_Data_t *Service)
{
   Word_t                             TimeUpdateControlPointHandle = 0;
   Word_t                             TimeUpdateStateHandle        = 0;
   unsigned int                       Index;
   GATT_Characteristic_Information_t *Characteristic;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure that the input parameters appear to be semi-valid.      */
   if((ConnectionEntry) && (Service))
   {
      /* Grab the first characteristic.                                 */
      Characteristic = Service->CharacteristicInformationList;

      /* Look through each charactertistic.                             */
      for(Index=0;Index<Service->NumberOfCharacteristics;Index++,Characteristic++)
      {
         /* We only care about UUID_16.                                 */
         if(Characteristic->Characteristic_UUID.UUID_Type == guUUID_16)
         {
            /* Look for the Time Update Control Point characteristic.   */
            if(RTUS_COMPARE_RTUS_TIME_UPDATE_CONTROL_POINT_UUID_TO_UUID_16(Characteristic->Characteristic_UUID.UUID.UUID_16))
               TimeUpdateControlPointHandle = Characteristic->Characteristic_Handle;
            else
            {
               if(RTUS_COMPARE_RTUS_TIME_UPDATE_STATE_UUID_TO_UUID_16(Characteristic->Characteristic_UUID.UUID.UUID_16))
                  TimeUpdateStateHandle = Characteristic->Characteristic_Handle;
            }
         }
      }

      if((TimeUpdateControlPointHandle) && (TimeUpdateStateHandle))
      {
         ConnectionEntry->ClientConnectionInfo.SupportedServicesMask       |= TIPM_SUPPORTED_SERVICES_RTUS;
         ConnectionEntry->ClientConnectionInfo.TimeUpdateControlPointHandle = TimeUpdateControlPointHandle;
         ConnectionEntry->ClientConnectionInfo.TimeUpdateStateHandle        = TimeUpdateStateHandle;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function to retrieve the      */
   /* service for an LE connection and parse it for a remote Time       */
   /* Profile Server.                                                   */
static void ConfigureTIPClientConnection(Connection_Entry_t *ConnectionEntry)
{
   int                                       Result;
   Byte_t                                   *ServiceData;
   unsigned int                              TotalServiceSize;
   unsigned int                              Index;
   DEVM_Parsed_Services_Data_t               ParsedGATTData;
   GATT_Service_Discovery_Indication_Data_t *Service;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure that the input parameters appear to be semi-valid.      */
   if(ConnectionEntry)
   {
      /* Determine the size of the service data.                        */
      if(!DEVM_QueryRemoteDeviceServices(ConnectionEntry->BD_ADDR, DEVM_QUERY_REMOTE_DEVICE_SERVICES_FLAGS_LOW_ENERGY, 0, NULL, &TotalServiceSize))
      {
         /* Allocate a buffer to hold the service data.                 */
         if((ServiceData = BTPS_AllocateMemory(TotalServiceSize)) != NULL)
         {
            /* Get the service data.                                    */
            if((Result = DEVM_QueryRemoteDeviceServices(ConnectionEntry->BD_ADDR, DEVM_QUERY_REMOTE_DEVICE_SERVICES_FLAGS_LOW_ENERGY, TotalServiceSize, ServiceData, NULL)) >= 0)
            {
               TotalServiceSize = (unsigned int)Result;

               /* Convert the Raw Service data to Parsed Service Data.  */
               if(!DEVM_ConvertRawServicesStreamToParsedServicesData(TotalServiceSize, ServiceData, &ParsedGATTData))
               {
                  Service = ParsedGATTData.GATTServiceDiscoveryIndicationData;

                  /* Check each service.                                */
                  for(Index=0;Index < ParsedGATTData.NumberServices;Index++,Service++)
                  {
                     /* We only care about UUID_16 since this is a      */
                     /* defined profile.                                */
                     if(Service->ServiceInformation.UUID.UUID_Type == guUUID_16)
                     {
                        /* Check for each of the possible TIP services, */
                        /* and configure them if found.                 */
                        if(CTS_COMPARE_CTS_SERVICE_UUID_TO_UUID_16(Service->ServiceInformation.UUID.UUID.UUID_16))
                           ConfigureCTSClient(ConnectionEntry, Service);
                        else
                        {
                           if(NDCS_COMPARE_NDCS_SERVICE_UUID_TO_UUID_16(Service->ServiceInformation.UUID.UUID.UUID_16))
                              ConfigureNDCSClient(ConnectionEntry, Service);
                           else
                           {
                              if(RTUS_COMPARE_RTUS_SERVICE_UUID_TO_UUID_16(Service->ServiceInformation.UUID.UUID.UUID_16))
                                 ConfigureRTUSClient(ConnectionEntry, Service);
                           }
                        }
                     }
                  }

                  /* Check to see if we need to dispatch a connected    */
                  /* event.                                             */
                  if(ConnectionEntry->ClientConnectionInfo.SupportedServicesMask & TIPM_SUPPORTED_SERVICES_CTS)
                     DispatchTIPConnectionEvent(ConnectionEntry);

                  /* Free the parsed service data.                      */
                  DEVM_FreeParsedServicesData(&ParsedGATTData);
               }
            }

            /* Free the allocated service memory.                       */
            BTPS_FreeMemory(ServiceData);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch the specified TIP event to every registered TIP Event    */
   /* Callback.                                                         */
   /* * NOTE * This function should be called with the TIP Manager Lock */
   /*          held.  Upon exit from this function it will free the TIP */
   /*          Manager Lock.                                            */
static void DispatchTIPMEvent(TIPM_Event_Data_t *TIPMEventData, BTPM_Message_t *Message, unsigned int *CallbackHandlerID, unsigned int *MessageHandlerID, TIPM_Connection_Type_t ConnectionType)
{
   unsigned int                Index;
   unsigned int                ServerID;
   unsigned int                NumberCallbacks;
   Callback_Info_t             CallbackInfoArray[16];
   Callback_Info_t            *CallbackInfoArrayPtr;
   TIPM_Event_Callback_Info_t *CallbackInfoPtr;
   TIPM_Event_Callback_Info_t *CallbackInfoList;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %u\n", ConnectionType));

   /* Make sure that the input parameters appear to be semi-valid.      */
   if(((EventCallbackInfoList) || (ClientEventCallbackInfoList)) && (TIPMEventData) && (Message) && (CallbackHandlerID) && (MessageHandlerID))
   {
      /* Determine whether to submit this event to the client or server */
      /* callbacks.                                                     */
      if(ConnectionType == tctServer)
         CallbackInfoList = EventCallbackInfoList;
      else
         CallbackInfoList = ClientEventCallbackInfoList;

      /* Next, let's determine how many callbacks are registered.       */
      CallbackInfoPtr = CallbackInfoList;
      ServerID        = MSG_GetServerAddressID();
      NumberCallbacks = 0;

      /* First, add the default event handlers.                         */
      while(CallbackInfoPtr)
      {
         if((CallbackInfoPtr->EventCallback) || (CallbackInfoPtr->ClientID != ServerID))
            NumberCallbacks++;

         CallbackInfoPtr = CallbackInfoPtr->NextTIPMEventCallbackInfoPtr;
      }

      DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Number Callbacks: %u\n", NumberCallbacks));
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
            CallbackInfoPtr = CallbackInfoList;
            NumberCallbacks = 0;

            /* Next add the default event handlers.                     */
            while(CallbackInfoPtr)
            {
               if((CallbackInfoPtr->EventCallback) || (CallbackInfoPtr->ClientID != ServerID))
               {
                  CallbackInfoArrayPtr[NumberCallbacks].EventCallbackID   = CallbackInfoPtr->EventCallbackID;
                  CallbackInfoArrayPtr[NumberCallbacks].ClientID          = CallbackInfoPtr->ClientID;
                  CallbackInfoArrayPtr[NumberCallbacks].EventCallback     = CallbackInfoPtr->EventCallback;
                  CallbackInfoArrayPtr[NumberCallbacks].CallbackParameter = CallbackInfoPtr->CallbackParameter;

                  NumberCallbacks++;
               }

               CallbackInfoPtr = CallbackInfoPtr->NextTIPMEventCallbackInfoPtr;
            }

            /* Release the Lock because we have already built the       */
            /* Callback Array.                                          */
            DEVM_ReleaseLock();

            /* Now we are ready to dispatch the callbacks.              */
            Index = 0;

            while((Index < NumberCallbacks) && (Initialized))
            {
               /* * NOTE * It is possible that we have already          */
               /*          dispatched the event to the client (case     */
               /*          would occur if a single client has registered*/
               /*          for TIP events and Data Events.              */
               /*          To avoid this case we need to walk the       */
               /*          list of previously dispatched events to check*/
               /*          to see if it has already been dispatched     */
               /*          (we need to do this with Client Address ID's */
               /*          for messages - Event Callbacks are local     */
               /*          and therefore unique so we don't have to do  */
               /*          this filtering.                              */

               /* Determine the type of event that needs to be          */
               /* dispatched.                                           */
               if(CallbackInfoArrayPtr[Index].ClientID == ServerID)
               {
                  /* Go ahead and make the callback.                    */
                  /* * NOTE * If the callback was deleted (or new ones  */
                  /*          were added, they will not be caught for   */
                  /*          this message dispatch).  Under normal     */
                  /*          operating circumstances this case         */
                  /*          shouldn't matter because these groups     */
                  /*          aren't really dynamic and are only        */
                  /*          registered at initialization time.        */
                  __BTPSTRY
                  {
                     if(CallbackInfoArrayPtr[Index].EventCallback)
                     {
                        /* Set the Callback ID in the event to be the   */
                        /* Event Callback ID for the person we are      */
                        /* dispatching to.                              */
                        *CallbackHandlerID = CallbackInfoArrayPtr[Index].EventCallbackID;

                        (*CallbackInfoArrayPtr[Index].EventCallback)(TIPMEventData, CallbackInfoArrayPtr[Index].CallbackParameter);
                     }
                  }
                  __BTPSEXCEPT(1)
                  {
                     /* Do Nothing.                                     */
                  }
               }
               else
               {
                  /* Set the Message Event Handler ID to be for the     */
                  /* person we are sending the message to.              */
                  *MessageHandlerID                = CallbackInfoArrayPtr[Index].EventCallbackID;

                  /* Dispatch the Message.                              */
                  Message->MessageHeader.AddressID = CallbackInfoArrayPtr[Index].ClientID;

                  MSG_SendMessage(Message);
               }

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch the specified TIP event to the specified TIP Event       */
   /* Callback.                                                         */
   /* * NOTE * This function should be called with the TIP Manager Lock */
   /*          held.                                                    */
static void DispatchTIPMEventByID(TIPM_Event_Data_t *TIPMEventData, BTPM_Message_t *Message, unsigned int CallbackID, TIPM_Connection_Type_t ConnectionType)
{
   Callback_Info_t             CallbackInfo;
   TIPM_Event_Callback_Info_t *EventCallbackInfo;
   TIPM_Event_Callback_Info_t *EventCallbackList;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify the parameters seem semi-valid.                     */
   if((TIPMEventData) && (Message) && (CallbackID))
   {
      /* Determine what list to search.                                 */
      if(ConnectionType == tctServer)
         EventCallbackList = EventCallbackInfoList;
      else
         EventCallbackList = ClientEventCallbackInfoList;

      /* Attempt to locate the callback info.                           */
      if((EventCallbackList) && ((EventCallbackInfo = SearchEventCallbackInfoEntry(&EventCallbackList, CallbackID)) != NULL))
      {
         /* Note the callback info, since we will be releasing the lock.*/
         CallbackInfo.ClientID          = EventCallbackInfo->ClientID;
         CallbackInfo.EventCallbackID   = EventCallbackInfo->EventCallbackID;
         CallbackInfo.EventCallback     = EventCallbackInfo->EventCallback;
         CallbackInfo.CallbackParameter = EventCallbackInfo->CallbackParameter;

         /* Now go ahead and release the lock.                          */
         DEVM_ReleaseLock();

         /* Check where we need to dispatch the event.                  */
         if(CallbackInfo.ClientID == MSG_GetServerAddressID())
         {
            /* This is a local callback.                                */

            /* * NOTE * If the callback was deleted (or new ones        */
            /*          were added, they will not be caught for this    */
            /*          message dispatch).  Under normal operating      */
            /*          circumstances this case shouldn't matter because*/
            /*          these groups aren't really dynamic and are only */
            /*          registered at initialization time.              */
            __BTPSTRY
            {
               if(CallbackInfo.EventCallback)
               {
                  (*CallbackInfo.EventCallback)(TIPMEventData, CallbackInfo.CallbackParameter);
               }
            }
            __BTPSEXCEPT(1)
            {
               /* Do Nothing.                                           */
            }
         }
         else
         {
            /* This is a remote client.                                 */
            Message->MessageHeader.AddressID = CallbackInfo.ClientID;

            MSG_SendMessage(Message);
         }

         /* Re-acquire the lock.                                        */
         DEVM_AcquireLock();
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* dispatch a Client Connection Event to all registered callbacks.   */
static void DispatchTIPConnectionEvent(Connection_Entry_t *ConnectionEntry)
{
   TIPM_Event_Data_t        EventData;
   TIPM_Connected_Message_t Message;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ConnectionEntry)
   {
      /* Flag that a Connection Event has been dispatched for this      */
      /* device.                                                        */
      ConnectionEntry->ClientConnectedDispatched = TRUE;

      /* Format the event that will be dispatched locally.              */
      EventData.EventType                                          = aetTIPConnected;
      EventData.EventLength                                        = TIPM_CONNECTED_EVENT_DATA_SIZE;
      EventData.EventData.ConnectedEventData.ConnectionType        = ConnectionEntry->ConnectionType;
      EventData.EventData.ConnectedEventData.RemoteDeviceAddress   = ConnectionEntry->BD_ADDR;
      EventData.EventData.ConnectedEventData.SupportedServicesMask = ConnectionEntry->ClientConnectionInfo.SupportedServicesMask;

      /* Next, format up the Message to dispatch.                       */
      BTPS_MemInitialize(&Message, 0, sizeof(Message));

      Message.MessageHeader.AddressID       = 0;
      Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
      Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_TIME_MANAGER;
      Message.MessageHeader.MessageFunction = TIPM_MESSAGE_FUNCTION_CONNECTED;
      Message.MessageHeader.MessageLength   = (TIPM_CONNECTED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

      Message.ConnectionType                = ConnectionEntry->ConnectionType;
      Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;
      Message.SupportedServicesMask         = ConnectionEntry->ClientConnectionInfo.SupportedServicesMask;

      /* Dispatch the event to all registered callbacks.                */
      DispatchTIPMEvent(&EventData, (BTPM_Message_t *)&Message, &(EventData.EventData.ConnectedEventData.CallbackID), &(Message.EventHandlerID), ConnectionEntry->ConnectionType);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* dispatch a Client Disconnection Event to all registered callbacks.*/
static void DispatchTIPDisconnectionEvent(Connection_Entry_t *ConnectionEntry)
{
   TIPM_Event_Data_t           EventData;
   TIPM_Disconnected_Message_t Message;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ConnectionEntry)
   {
      /* Format the event that will be dispatched locally.              */
      EventData.EventType                                           = aetTIPDisconnected;
      EventData.EventLength                                         = TIPM_DISCONNECTED_EVENT_DATA_SIZE;
      EventData.EventData.DisconnectedEventData.ConnectionType      = ConnectionEntry->ConnectionType;
      EventData.EventData.DisconnectedEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;

      /* Next, format up the Message to dispatch.                       */
      BTPS_MemInitialize(&Message, 0, sizeof(Message));

      Message.MessageHeader.AddressID       = 0;
      Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
      Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_TIME_MANAGER;
      Message.MessageHeader.MessageFunction = TIPM_MESSAGE_FUNCTION_DISCONNECTED;
      Message.MessageHeader.MessageLength   = (TIPM_DISCONNECTED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

      Message.ConnectionType                = ConnectionEntry->ConnectionType;
      Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;

      /* Dispatch the event to all registered callbacks.                */
      DispatchTIPMEvent(&EventData, (BTPM_Message_t *)&Message, &(EventData.EventData.DisconnectedEventData.CallbackID), &(Message.EventHandlerID), ConnectionEntry->ConnectionType);

      /* If this is a client, we should remove any registered           */
      /* notification callbacks.                                        */
      if(ConnectionEntry->ConnectionType == tctClient)
         CleanupNotificationRegistration(ConnectionEntry);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* dispatch a Reference Time Request Event to all registered         */
   /* callbacks.                                                        */
static void DispatchTIPReferenceTimeRequestEvent(void)
{
   TIPM_Event_Data_t                     EventData;
   TIPM_Reference_Time_Request_Message_t Message;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to verify that there is not already an outstanding Reference*/
   /* Time Request Message.  If there is we will not dispatch any more  */
   /* requests until the outstanding request has been responded to.     */
   if(!ReferenceTimeRequestOutstanding)
   {
      /* Flag that a Reference Time Request is outstanding.             */
      ReferenceTimeRequestOutstanding = TRUE;

      /* Format the event that will be dispatched locally.              */
      EventData.EventType   = aetTIPGetReferenceTimeRequest;
      EventData.EventLength = TIPM_GET_REFERENCE_TIME_REQUEST_EVENT_DATA_SIZE;

      /* Next, format up the Message to dispatch.                       */
      BTPS_MemInitialize(&Message, 0, sizeof(Message));

      Message.MessageHeader.AddressID       = 0;
      Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
      Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_TIME_MANAGER;
      Message.MessageHeader.MessageFunction = TIPM_MESSAGE_FUNCTION_REFERENCE_TIME_REQUEST;
      Message.MessageHeader.MessageLength   = (TIPM_REFERENCE_TIME_REQUEST_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

      /* Dispatch the event to all registered callbacks.                */
      DispatchTIPMEvent(&EventData, (BTPM_Message_t *)&Message, &(EventData.EventData.GetReferenceTimeRequestEventData.ServerCallbackID), &(Message.ServerEventHandlerID), tctServer);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Register Server    */
   /* Events Request Message and responds to the message accordingly.   */
   /* This function does not verify the integrity of the Message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the Message before calling this function.                         */
   /* * NOTE * This function *MUST* be called with the TIP Manager Lock */
   /*          held.                                                    */
static void ProcessRegisterServerEventsRequestMessage(TIPM_Register_Server_Events_Request_t *Message)
{
   int                                     Result;
   TIPM_Event_Callback_Info_t              EventCallbackEntry;
   TIPM_Register_Server_Events_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Verify that no other callback is already registered, as for at */
      /* least now only 1 Server Callback can be registered at a time.  */
      if(EventCallbackInfoList == NULL)
      {
         /* Attempt to add an entry into the Event Callback Entry list. */
         BTPS_MemInitialize(&EventCallbackEntry, 0, sizeof(TIPM_Event_Callback_Info_t));

         EventCallbackEntry.EventCallbackID   = GetNextEventCallbackID();
         EventCallbackEntry.ClientID          = Message->MessageHeader.AddressID;
         EventCallbackEntry.EventCallback     = NULL;
         EventCallbackEntry.CallbackParameter = 0;

         if(AddEventCallbackInfoEntry(&EventCallbackInfoList, &EventCallbackEntry))
            Result = EventCallbackEntry.EventCallbackID;
         else
            Result = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
      }
      else
         Result = BTPM_ERROR_CODE_TIME_SERVER_CALLBACK_ALREADY_REGISTERED;

      /* Format and send the response message.                          */
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = TIPM_REGISTER_SERVER_EVENTS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      if(Result > 0)
      {
         ResponseMessage.Status                    = 0;

         ResponseMessage.ServerEventHandlerID      = Result;
      }
      else
      {
         ResponseMessage.Status                    = Result;

         ResponseMessage.ServerEventHandlerID      = 0;
      }

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Un-Register TIP    */
   /* Events Request Message and responds to the message accordingly.   */
   /* This function does not verify the integrity of the Message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the Message before calling this function.                         */
   /* * NOTE * This function *MUST* be called with the TIP Manager Lock */
   /*          held.                                                    */
static void ProcessUnRegisterServerEventsRequestMessage(TIPM_Un_Register_Server_Events_Request_t *Message)
{
   int                                        Result;
   TIPM_Event_Callback_Info_t                *EventCallbackEntryPtr;
   TIPM_Un_Register_Server_Events_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Search for the Event Callback Info for this message to verify  */
      /* that the Client who is un-registering the server is the same   */
      /* client who registered the callback.                            */
      if((EventCallbackEntryPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, Message->ServerEventHandlerID)) != NULL)
      {
         /* Verify that the same client is doing the un-registering.    */
         if(EventCallbackEntryPtr->ClientID == Message->MessageHeader.AddressID)
         {
            /* Attempt to delete the callback specified for this device.*/
            if((EventCallbackEntryPtr = DeleteEventCallbackInfoEntry(&EventCallbackInfoList, Message->ServerEventHandlerID)) != NULL)
            {
               /* Free the memory allocated for this event callback.    */
               FreeEventCallbackInfoEntryMemory(EventCallbackEntryPtr);

               /* Return success.                                       */
               Result = 0;
            }
            else
               Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      /* Format and send the response message.                          */
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = TIPM_UN_REGISTER_SERVER_EVENTS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Set Local Time     */
   /* Information Request Request Message and responds to the message   */
   /* accordingly.  This function does not verify the integrity of the  */
   /* Message (i.e.  the length) because it is the caller's             */
   /* responsibility to verify the Message before calling this function.*/
   /* * NOTE * This function *MUST* be called with the TIP Manager Lock */
   /*          held.                                                    */
static void ProcessSetLocalTimeInformationRequestMessage(TIPM_Set_Local_Time_Information_Request_t *Message)
{
   int                                         Result;
   TIPM_Event_Callback_Info_t                 *EventCallbackEntryPtr;
   TIPM_Set_Local_Time_Information_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Search for the specified Callback Entry.                       */
      if((EventCallbackEntryPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, Message->ServerEventHandlerID)) != NULL)
      {
         /* Verify that the same client is doing the set.               */
         if(EventCallbackEntryPtr->ClientID == Message->MessageHeader.AddressID)
         {
            /* Simply call the internal function to set the Local Time  */
            /* Information.                                             */
            Result = ProcessSetLocalTimeInformation(&(Message->LocalTimeInformation));
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      /* Format and send the response message.                          */
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = TIPM_SET_LOCAL_TIME_INFORMATION_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Update Current Time*/
   /* Request Request Message and responds to the message accordingly.  */
   /* This function does not verify the integrity of the Message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the Message before calling this function.                         */
   /* * NOTE * This function *MUST* be called with the TIP Manager Lock */
   /*          held.                                                    */
static void ProcessUpdateCurrentTimeRequestMessage(TIPM_Update_Current_Time_Request_t *Message)
{
   int                                  Result;
   TIPM_Event_Callback_Info_t          *EventCallbackEntryPtr;
   TIPM_Update_Current_Time_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Search for the specified Callback Entry.                       */
      if((EventCallbackEntryPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, Message->ServerEventHandlerID)) != NULL)
      {
         /* Verify that the same client is doing the set.               */
         if(EventCallbackEntryPtr->ClientID == Message->MessageHeader.AddressID)
         {
            /* Simply call the internal function to Update the Current  */
            /* Time.                                                    */
            Result = ProcessUpdateCurrentTime(0, Message->AdjustReasonFlags);
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      /* Format and send the response message.                          */
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = TIPM_UPDATE_CURRENT_TIME_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Reference Time     */
   /* Response Request Message and responds to the message accordingly. */
   /* This function does not verify the integrity of the Message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the Message before calling this function.                         */
   /* * NOTE * This function *MUST* be called with the TIP Manager Lock */
   /*          held.                                                    */
static void ProcessReferenceTimeResponseMessage(TIPM_Reference_Time_Response_Request_t *Message)
{
   int                                      Result;
   TIPM_Event_Callback_Info_t              *EventCallbackEntryPtr;
   TIPM_Reference_Time_Response_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Search for the specified Callback Entry.                       */
      if((EventCallbackEntryPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, Message->ServerEventHandlerID)) != NULL)
      {
         /* Verify that the same client is doing the set.               */
         if(EventCallbackEntryPtr->ClientID == Message->MessageHeader.AddressID)
         {
            /* Simply call the internal function to handle actually     */
            /* responding to the previous request for the Reference Time*/
            /* Information.                                             */
            Result = ProcessReferenceTimeInformationResponse(&(Message->ReferenceTimeInformation));
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      /* Format and send the response message.                          */
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = TIPM_REFERENCE_TIME_RESPONSE_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Register Client    */
   /* Events Request Message and responds to the message accordingly.   */
   /* This function does not verify the integrity of the Message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the Message before calling this function.                         */
   /* * NOTE * This function *MUST* be called with the TIP Manager Lock */
   /*          held.                                                    */
static void ProcessRegisterClientEventsRequestMessage(TIPM_Register_Client_Events_Request_t *Message)
{
   int                                     Result;
   TIPM_Event_Callback_Info_t              EventCallbackEntry;
   TIPM_Register_Client_Events_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Attempt to add an entry into the Event Callback Entry list.    */
      BTPS_MemInitialize(&EventCallbackEntry, 0, sizeof(TIPM_Event_Callback_Info_t));

      EventCallbackEntry.EventCallbackID   = GetNextEventCallbackID();
      EventCallbackEntry.ClientID          = Message->MessageHeader.AddressID;
      DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Client Registered: %u\n", EventCallbackEntry.ClientID));
      EventCallbackEntry.EventCallback     = NULL;
      EventCallbackEntry.CallbackParameter = 0;

      if(AddEventCallbackInfoEntry(&ClientEventCallbackInfoList, &EventCallbackEntry))
         Result = EventCallbackEntry.EventCallbackID;
      else
         Result = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;

      /* Format and send the response message.                          */
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = TIPM_REGISTER_CLIENT_EVENTS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      if(Result > 0)
      {
         ResponseMessage.Status                    = 0;

         ResponseMessage.ClientEventHandlerID      = Result;
      }
      else
      {
         ResponseMessage.Status                    = Result;

         ResponseMessage.ClientEventHandlerID      = 0;
      }

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Un-Register TIP    */
   /* Events Request Message and responds to the message accordingly.   */
   /* This function does not verify the integrity of the Message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the Message before calling this function.                         */
   /* * NOTE * This function *MUST* be called with the TIP Manager Lock */
   /*          held.                                                    */
static void ProcessUnRegisterClientEventsRequestMessage(TIPM_Un_Register_Client_Events_Request_t *Message)
{
   int                                        Result;
   TIPM_Event_Callback_Info_t                *EventCallbackEntryPtr;
   TIPM_Un_Register_Client_Events_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Search for the Event Callback Info for this message to verify  */
      /* that the Client who is un-registering the server is the same   */
      /* client who registered the callback.                            */
      if((EventCallbackEntryPtr = SearchEventCallbackInfoEntry(&ClientEventCallbackInfoList, Message->ClientEventHandlerID)) != NULL)
      {
         /* Verify that the same client is doing the un-registering.    */
         if(EventCallbackEntryPtr->ClientID == Message->MessageHeader.AddressID)
         {
            /* Attempt to delete the callback specified for this device.*/
            if((EventCallbackEntryPtr = DeleteEventCallbackInfoEntry(&ClientEventCallbackInfoList, Message->ClientEventHandlerID)) != NULL)
            {
               /* Free the memory allocated for this event callback.    */
               FreeEventCallbackInfoEntryMemory(EventCallbackEntryPtr);

               /* Return success.                                       */
               Result = 0;
            }
            else
               Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      /* Format and send the response message.                          */
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = TIPM_UN_REGISTER_CLIENT_EVENTS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Get Current Time   */
   /* Request Message and responds to the message accordingly.  This    */
   /* function does not verify the integrity of the Message (i.e. the   */
   /* length) because it is the caller's responsibility to verify the   */
   /* Message before calling this function.                             */
   /* * NOTE * This function *MUST* be called with the TIP Manager Lock */
   /*          held.                                                    */
static void ProcessGetCurrentTimeRequestMessage(TIPM_Get_Current_Time_Request_t *Message)
{
   int                               Result;
   TIPM_Event_Callback_Info_t       *EventCallbackEntryPtr;
   TIPM_Get_Current_Time_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Search for the specified Callback Entry.                       */
      if((EventCallbackEntryPtr = SearchEventCallbackInfoEntry(&ClientEventCallbackInfoList, Message->ClientEventHandlerID)) != NULL)
      {
         /* Verify that the same client is doing the set.               */
         if(EventCallbackEntryPtr->ClientID == Message->MessageHeader.AddressID)
         {
            /* Simply call the internal function to handle the Get      */
            /* Current Time request.                                    */
            Result = ProcessGetCurrentTime(Message->ClientEventHandlerID, Message->RemoteDeviceAddress);
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      /* Format and send the response.                                  */
      BTPS_MemInitialize(&ResponseMessage, 0, sizeof(TIPM_Get_Current_Time_Response_t));

      ResponseMessage.MessageHeader               = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID    |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength = TIPM_GET_CURRENT_TIME_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;
      ResponseMessage.Status                      = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Enable Time        */
   /* Notifications Request Message and responds to the message         */
   /* accordingly.  This function does not verify the integrity         */
   /* of the Message (i.e. the length) because it is the caller's       */
   /* responsibility to verify the Message before calling this function.*/
   /* * NOTE * This function *MUST* be called with the TIP Manager Lock */
   /*          held.                                                    */
static void ProcessEnableTimeNotificationsRequestMessage(TIPM_Enable_Time_Notifications_Request_t *Message)
{
   int                                        Result;
   TIPM_Event_Callback_Info_t                *EventCallbackEntryPtr;
   TIPM_Enable_Time_Notifications_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Search for the specified Callback Entry.                       */
      if((EventCallbackEntryPtr = SearchEventCallbackInfoEntry(&ClientEventCallbackInfoList, Message->ClientEventHandlerID)) != NULL)
      {
         /* Verify that the same client is doing the set.               */
         if(EventCallbackEntryPtr->ClientID == Message->MessageHeader.AddressID)
         {
            /* Simply call the internal function to handle the Enable   */
            /* Notifications request.                                   */
            Result = ProcessEnableTimeNotifications(Message->ClientEventHandlerID, Message->RemoteDeviceAddress, Message->Enable);
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      /* Format and send the response.                                  */
      BTPS_MemInitialize(&ResponseMessage, 0, sizeof(TIPM_Enable_Time_Notifications_Response_t));

      ResponseMessage.MessageHeader               = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID    |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength = TIPM_ENABLE_TIME_NOTIFICATIONS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;
      ResponseMessage.Status                      = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Get Local          */
   /* Time Information Request Message and responds to the message      */
   /* accordingly.  This function does not verify the integrity         */
   /* of the Message (i.e. the length) because it is the caller's       */
   /* responsibility to verify the Message before calling this function.*/
   /* * NOTE * This function *MUST* be called with the TIP Manager Lock */
   /*          held.                                                    */
static void ProcessGetLocalTimeInformationRequestMessage(TIPM_Get_Local_Time_Information_Request_t *Message)
{
   int                                         Result;
   TIPM_Event_Callback_Info_t                 *EventCallbackEntryPtr;
   TIPM_Get_Local_Time_Information_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Search for the specified Callback Entry.                       */
      if((EventCallbackEntryPtr = SearchEventCallbackInfoEntry(&ClientEventCallbackInfoList, Message->ClientEventHandlerID)) != NULL)
      {
         /* Verify that the same client is doing the set.               */
         if(EventCallbackEntryPtr->ClientID == Message->MessageHeader.AddressID)
         {
            /* Simply call the internal function to handle the Get Local*/
            /* Time Information request.                                */
            Result = ProcessGetLocalTimeInformation(Message->ClientEventHandlerID, Message->RemoteDeviceAddress);
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      /* Format and send the response.                                  */
      BTPS_MemInitialize(&ResponseMessage, 0, sizeof(TIPM_Get_Local_Time_Information_Response_t));

      ResponseMessage.MessageHeader               = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID    |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength = TIPM_GET_LOCAL_TIME_INFORMATION_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;
      ResponseMessage.Status                      = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Get Time Accuracy  */
   /* Request Message and responds to the message accordingly.  This    */
   /* function does not verify the integrity of the Message (i.e. the   */
   /* length) because it is the caller's responsibility to verify the   */
   /* Message before calling this function.                             */
   /* * NOTE * This function *MUST* be called with the TIP Manager Lock */
   /*          held.                                                    */
static void ProcessGetTimeAccuracyRequestMessage(TIPM_Get_Time_Accuracy_Request_t *Message)
{
   int                                Result;
   TIPM_Event_Callback_Info_t        *EventCallbackEntryPtr;
   TIPM_Get_Time_Accuracy_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Search for the specified Callback Entry.                       */
      if((EventCallbackEntryPtr = SearchEventCallbackInfoEntry(&ClientEventCallbackInfoList, Message->ClientEventHandlerID)) != NULL)
      {
         /* Verify that the same client is doing the set.               */
         if(EventCallbackEntryPtr->ClientID == Message->MessageHeader.AddressID)
         {
            /* Simply call the internal function to handle the Get Time */
            /* Accuracy request.                                        */
            Result = ProcessGetTimeAccuracy(Message->ClientEventHandlerID, Message->RemoteDeviceAddress);
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      /* Format and send the response.                                  */
      BTPS_MemInitialize(&ResponseMessage, 0, sizeof(TIPM_Get_Time_Accuracy_Response_t));

      ResponseMessage.MessageHeader               = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID    |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength = TIPM_GET_TIME_ACCURACY_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;
      ResponseMessage.Status                      = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Get Next DST       */
   /* Change Information Request Message and responds to the message    */
   /* accordingly.  This function does not verify the integrity         */
   /* of the Message (i.e. the length) because it is the caller's       */
   /* responsibility to verify the Message before calling this function.*/
   /* * NOTE * This function *MUST* be called with the TIP Manager Lock */
   /*          held.                                                    */
static void ProcessGetNextDSTChangeInformationRequestMessage(TIPM_Get_Next_DST_Change_Information_Request_t *Message)
{
   int                                              Result;
   TIPM_Event_Callback_Info_t                      *EventCallbackEntryPtr;
   TIPM_Get_Next_DST_Change_Information_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Search for the specified Callback Entry.                       */
      if((EventCallbackEntryPtr = SearchEventCallbackInfoEntry(&ClientEventCallbackInfoList, Message->ClientEventHandlerID)) != NULL)
      {
         /* Verify that the same client is doing the set.               */
         if(EventCallbackEntryPtr->ClientID == Message->MessageHeader.AddressID)
         {
            /* Simply call the internal function to handle the Get Next */
            /* DST Change request.                                      */
            Result = ProcessGetNextDSTChange(Message->ClientEventHandlerID, Message->RemoteDeviceAddress);
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      /* Format and send the response.                                  */
      BTPS_MemInitialize(&ResponseMessage, 0, sizeof(TIPM_Get_Next_DST_Change_Information_Response_t));

      ResponseMessage.MessageHeader               = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID    |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength = TIPM_GET_NEXT_DST_CHANGE_INFORMATION_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;
      ResponseMessage.Status                      = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Get Reference      */
   /* Time Update State Request Message and responds to the message     */
   /* accordingly.  This function does not verify the integrity         */
   /* of the Message (i.e. the length) because it is the caller's       */
   /* responsibility to verify the Message before calling this function.*/
   /* * NOTE * This function *MUST* be called with the TIP Manager Lock */
   /*          held.                                                    */
static void ProcessGetReferenceTimeUpdateStateRequestMessage(TIPM_Get_Reference_Time_Update_State_Request_t *Message)
{
   int                                              Result;
   TIPM_Event_Callback_Info_t                      *EventCallbackEntryPtr;
   TIPM_Get_Reference_Time_Update_State_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Search for the specified Callback Entry.                       */
      if((EventCallbackEntryPtr = SearchEventCallbackInfoEntry(&ClientEventCallbackInfoList, Message->ClientEventHandlerID)) != NULL)
      {
         /* Verify that the same client is doing the set.               */
         if(EventCallbackEntryPtr->ClientID == Message->MessageHeader.AddressID)
         {
            /* Simply call the internal function to handle the Ger      */
            /* Reference Time Update State request.                     */
            Result = ProcessGetReferenceTimeUpdateState(Message->ClientEventHandlerID, Message->RemoteDeviceAddress);
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      /* Format and send the response.                                  */
      BTPS_MemInitialize(&ResponseMessage, 0, sizeof(TIPM_Get_Reference_Time_Update_State_Response_t));

      ResponseMessage.MessageHeader               = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID    |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength = TIPM_GET_REFERENCE_TIME_UPDATE_STATE_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;
      ResponseMessage.Status                      = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Request Reference  */
   /* Time Update Request Message and responds to the message           */
   /* accordingly.  This function does not verify the integrity         */
   /* of the Message (i.e. the length) because it is the caller's       */
   /* responsibility to verify the Message before calling this function.*/
   /* * NOTE * This function *MUST* be called with the TIP Manager Lock */
   /*          held.                                                    */
static void ProcessRequestReferenceTimeUpdateRequestMessage(TIPM_Request_Reference_Time_Update_Request_t *Message)
{
   int                                            Result;
   TIPM_Event_Callback_Info_t                    *EventCallbackEntryPtr;
   TIPM_Request_Reference_Time_Update_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Search for the specified Callback Entry.                       */
      if((EventCallbackEntryPtr = SearchEventCallbackInfoEntry(&ClientEventCallbackInfoList, Message->ClientEventHandlerID)) != NULL)
      {
         /* Verify that the same client is doing the set.               */
         if(EventCallbackEntryPtr->ClientID == Message->MessageHeader.AddressID)
         {
            /* Simply call the internal function to handle the Request  */
            /* Time Update request.                                     */
            Result = ProcessRequestReferenceTimeUpdate(Message->ClientEventHandlerID, Message->RemoteDeviceAddress);
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      /* Format and send the response.                                  */
      BTPS_MemInitialize(&ResponseMessage, 0, sizeof(TIPM_Request_Reference_Time_Update_Response_t));

      ResponseMessage.MessageHeader               = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID    |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength = TIPM_REQUEST_REFERENCE_TIME_UPDATE_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;
      ResponseMessage.Status                      = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Query Connected    */
   /* Devices Request Message and responds to the message accordingly.  */
   /* This function does not verify the integrity of the Message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the Message before calling this function.                         */
   /* * NOTE * This function *MUST* be called with the TIP Manager Lock */
   /*          held.                                                    */
static void ProcessQueryConnectedDevicesRequestMessage(TIPM_Query_Connected_Devices_Request_t *Message)
{
   int                                      Result;
   unsigned int                             NumberDevices;
   TIPM_Query_Connected_Devices_Response_t  ErrorResponseMessage;
   TIPM_Query_Connected_Devices_Response_t *ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Determine the number of devices connected.                     */
      if((Result = ProcessQueryConnectedDevices(Message->ConnectionType, 0, NULL, &NumberDevices)) >= 0)
      {
         /* Now attempt to allocate the response message.               */
         if((ResponseMessage = (TIPM_Query_Connected_Devices_Response_t *)BTPS_AllocateMemory(TIPM_QUERY_CONNECTED_DEVICES_RESPONSE_SIZE(NumberDevices))) != NULL)
         {
            BTPS_MemInitialize(ResponseMessage, 0, sizeof(TIPM_QUERY_CONNECTED_DEVICES_RESPONSE_SIZE(NumberDevices)));

            /* Get the actual connected devices.                        */
            if((Result = ProcessQueryConnectedDevices(Message->ConnectionType, NumberDevices, ResponseMessage->RemoteDevices, NULL)) >= 0)
            {
               /* Format and send the response.                         */
               ResponseMessage->MessageHeader               = Message->MessageHeader;
               ResponseMessage->MessageHeader.MessageID    |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
               ResponseMessage->MessageHeader.MessageLength = TIPM_QUERY_CONNECTED_DEVICES_RESPONSE_SIZE(NumberDevices) - BTPM_MESSAGE_HEADER_SIZE;
               ResponseMessage->Status                      = 0;
               ResponseMessage->NumberDevices               = Result;

               MSG_SendMessage((BTPM_Message_t *)ResponseMessage);
            }

            BTPS_FreeMemory(ResponseMessage);
         }
         else
            Result = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
      }

      /* Check if there was an error.                                   */
      if(Result < 0)
      {
         /* We need to send the Error response.                         */
         BTPS_MemInitialize(&ErrorResponseMessage, 0, sizeof(TIPM_Query_Connected_Devices_Response_t));

         ErrorResponseMessage.MessageHeader               = Message->MessageHeader;
         ErrorResponseMessage.MessageHeader.MessageID    |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
         ErrorResponseMessage.MessageHeader.MessageLength = TIPM_QUERY_CONNECTED_DEVICES_RESPONSE_SIZE(0) - BTPM_MESSAGE_HEADER_SIZE;
         ErrorResponseMessage.Status                      = Result;
         ErrorResponseMessage.NumberDevices               = 0;

         MSG_SendMessage((BTPM_Message_t *)&ErrorResponseMessage);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is called from the Main Bluetopia Dispatch */
   /* thread in response to a Process Received Message.  This function  */
   /* SHOULD ONLY be called in this capacity and NOT under any other    */
   /* circumstances.                                                    */
   /* * NOTE * This function *MUST* be called with the TIP Manager      */
   /*          Lock held.  This function will release the Lock before   */
   /*          it exits (i.e. the caller SHOULD NOT RELEASE THE LOCK).  */
static void ProcessReceivedMessage(BTPM_Message_t *Message)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", Message->MessageHeader.MessageFunction));

   /* Check to make sure this isn't some kind of spoofed response (we do*/
   /* not process responses).                                           */
   if((Message) && (!(Message->MessageHeader.MessageID & BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK)))
   {
      /* Request received, go ahead and process it.                     */
      switch(Message->MessageHeader.MessageFunction)
      {
         case TIPM_MESSAGE_FUNCTION_REGISTER_SERVER_EVENTS:
            DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Register Server Events Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= TIPM_REGISTER_SERVER_EVENTS_REQUEST_SIZE)
               ProcessRegisterServerEventsRequestMessage((TIPM_Register_Server_Events_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case TIPM_MESSAGE_FUNCTION_UN_REGISTER_SERVER_EVENTS:
            DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Un-Register Server Events Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= TIPM_UN_REGISTER_SERVER_EVENTS_REQUEST_SIZE)
               ProcessUnRegisterServerEventsRequestMessage((TIPM_Un_Register_Server_Events_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case TIPM_MESSAGE_FUNCTION_REGISTER_CLIENT_EVENTS:
            DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Register Client Events Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= TIPM_REGISTER_CLIENT_EVENTS_REQUEST_SIZE)
               ProcessRegisterClientEventsRequestMessage((TIPM_Register_Client_Events_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case TIPM_MESSAGE_FUNCTION_UN_REGISTER_CLIENT_EVENTS:
            DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Un-Register Client Events Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= TIPM_UN_REGISTER_CLIENT_EVENTS_REQUEST_SIZE)
               ProcessUnRegisterClientEventsRequestMessage((TIPM_Un_Register_Client_Events_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case TIPM_MESSAGE_FUNCTION_SET_LOCAL_TIME_INFORMATION:
            DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Set Local Time Information Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= TIPM_SET_LOCAL_TIME_INFORMATION_REQUEST_SIZE)
               ProcessSetLocalTimeInformationRequestMessage((TIPM_Set_Local_Time_Information_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case TIPM_MESSAGE_FUNCTION_UPDATE_CURRENT_TIME:
            DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Update Current Time Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= TIPM_UPDATE_CURRENT_TIME_REQUEST_SIZE)
               ProcessUpdateCurrentTimeRequestMessage((TIPM_Update_Current_Time_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case TIPM_MESSAGE_FUNCTION_REFERENCE_TIME_RESPONSE:
            DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Reference Time Response Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= TIPM_REFERENCE_TIME_RESPONSE_REQUEST_SIZE)
               ProcessReferenceTimeResponseMessage((TIPM_Reference_Time_Response_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case TIPM_MESSAGE_FUNCTION_GET_CURRENT_TIME:
            DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Get Current Time Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= TIPM_GET_CURRENT_TIME_REQUEST_SIZE)
               ProcessGetCurrentTimeRequestMessage((TIPM_Get_Current_Time_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case TIPM_MESSAGE_FUNCTION_ENABLE_TIME_NOTIFICATIONS:
            DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Enable Time Notifications Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= TIPM_ENABLE_TIME_NOTIFICATIONS_REQUEST_SIZE)
               ProcessEnableTimeNotificationsRequestMessage((TIPM_Enable_Time_Notifications_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case TIPM_MESSAGE_FUNCTION_GET_LOCAL_TIME_INFORMATION:
            DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Get Local Time Information Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= TIPM_GET_LOCAL_TIME_INFORMATION_REQUEST_SIZE)
               ProcessGetLocalTimeInformationRequestMessage((TIPM_Get_Local_Time_Information_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case TIPM_MESSAGE_FUNCTION_GET_TIME_ACCURACY:
            DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Get Time Accuracy Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= TIPM_GET_TIME_ACCURACY_REQUEST_SIZE)
               ProcessGetTimeAccuracyRequestMessage((TIPM_Get_Time_Accuracy_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case TIPM_MESSAGE_FUNCTION_GET_NEXT_DST_CHANGE_INFORMATION:
            DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Get Next DST Change Information Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= TIPM_GET_NEXT_DST_CHANGE_INFORMATION_REQUEST_SIZE)
               ProcessGetNextDSTChangeInformationRequestMessage((TIPM_Get_Next_DST_Change_Information_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case TIPM_MESSAGE_FUNCTION_GET_REFERENCE_TIME_UPDATE_STATE:
            DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Get Reference Time Update State Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= TIPM_GET_REFERENCE_TIME_UPDATE_STATE_REQUEST_SIZE)
               ProcessGetReferenceTimeUpdateStateRequestMessage((TIPM_Get_Reference_Time_Update_State_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case TIPM_MESSAGE_FUNCTION_REQUEST_REFERENCE_TIME_UPDATE:
            DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Request Reference Time Update Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= TIPM_REQUEST_REFERENCE_TIME_UPDATE_REQUEST_SIZE)
               ProcessRequestReferenceTimeUpdateRequestMessage((TIPM_Request_Reference_Time_Update_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case TIPM_MESSAGE_FUNCTION_QUERY_CONNECTED_DEVICES:
            DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Query Connected Devices Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= TIPM_QUERY_CONNECTED_DEVICES_REQUEST_SIZE)
               ProcessQueryConnectedDevicesRequestMessage((TIPM_Query_Connected_Devices_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         default:
            DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled Message\n"));

            break;
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid or Response Message Received\n"));
   }

   /* Release the Lock because we are finished with it.                 */
   DEVM_ReleaseLock();

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the function that is called to process a*/
   /* client Un-Registration event.  This function will perform any     */
   /* cleanup required for the specified Client.                        */
static void ProcessClientUnRegister(unsigned int ClientID)
{
   TIPM_Event_Callback_Info_t  *EventCallback;
   TIPM_Event_Callback_Info_t  *tmpEventCallback;
   TIPM_Event_Callback_Info_t **EventCallbackList;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", ClientID));

   if((ClientID) && (ClientID != MSG_GetServerAddressID()))
   {
      /* Start with the server callbacks (if it is not empty).          */
      EventCallbackList = (EventCallbackInfoList)?&EventCallbackInfoList:&ClientEventCallbackInfoList;

      /* Loop through the event callback list and delete all callbacks  */
      /* registered for this callback.                                  */
      EventCallback = *EventCallbackList;

      while(EventCallback)
      {
         /* Check to see if the current Client Information is the one   */
         /* that is being un-registered.                                */
         if(EventCallback->ClientID == ClientID)
         {
            /* Note the next Event Callback Entry in the list (we are   */
            /* about to delete the current entry).                      */
            tmpEventCallback = EventCallback->NextTIPMEventCallbackInfoPtr;

            /* Free any Notification Registration the callback may have */
            /* had.                                                     */
            CleanupNotificationRegistrationByCallbackID(EventCallback->EventCallbackID);

            /* Go ahead and delete the Event Callback Entry and clean up*/
            /* the resources.                                           */
            if((EventCallback = DeleteEventCallbackInfoEntry(EventCallbackList, EventCallback->EventCallbackID)) != NULL)
            {
               /* All finished with the memory so free the entry.       */
               FreeEventCallbackInfoEntryMemory(EventCallback);
            }

            /* Go ahead and set the next Event Callback Entry (past the */
            /* one we just deleted).                                    */
            EventCallback = tmpEventCallback;
         }
         else
            EventCallback = EventCallback->NextTIPMEventCallbackInfoPtr;

         /* If we this is the last callback of the server list, move to */
         /* the client list.                                            */

         if((!EventCallback) && (*EventCallbackList == EventCallbackInfoList))
         {
            EventCallbackList = &ClientEventCallbackInfoList;
            EventCallback     = *EventCallbackList;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the function that is called to process a*/
   /* CTS Read Client Configuration Data Event.                         */
   /* * NOTE * This function *MUST* be called with the TIP Manager Lock */
   /*          held.                                                    */
static void ProcessCTSReadConfigurationRequestEvent(CTS_Read_Client_Configuration_Data_t *ReadClientConfigurationData)
{
   Connection_Entry_t *ConnectionEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* Verify that the input parameters seem semi-valid.                 */
   if(ReadClientConfigurationData)
   {
      /* Search for an existing connection entry for this device or add */
      /* one to the list.                                               */
      if((ConnectionEntryPtr = SearchAddConnectionEntry(&ConnectionEntryList, tctServer, ReadClientConfigurationData->RemoteDevice, ReadClientConfigurationData->ConnectionID)) != NULL)
      {
         /* Verify that we know what this descriptor is.                */
         if(ReadClientConfigurationData->ClientConfigurationType == ctCurrentTime)
         {
            /* Check to see if we have dispatched a connection for this */
            /* device.                                                  */
            if(ConnectionEntryPtr->ClientConnectedDispatched == FALSE)
            {
               /* We have not dispatched a Connection Event so do so    */
               /* now.                                                  */
               DispatchTIPConnectionEvent(ConnectionEntryPtr);

               /* Reload any stored Client Configuration for this       */
               /* device.                                               */
               ReloadConnectionConfiguration(ConnectionEntryPtr);
            }

            /* Respond to the Read Request.                             */
            _TIPM_CTS_Read_Client_Configuration_Response(ReadClientConfigurationData->TransactionID, ConnectionEntryPtr->CurrentTimeCCCD);
         }
         else
         {
            DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Unknown CTS Descriptor: %u\n", (unsigned int)ReadClientConfigurationData->ClientConfigurationType));

            /* Respond to the Read Request.                             */
            _TIPM_CTS_Read_Client_Configuration_Response(ReadClientConfigurationData->TransactionID, 0);
         }
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Failed to add to list\n"));

         /* Respond to the Read Request.                                */
         _TIPM_CTS_Read_Client_Configuration_Response(ReadClientConfigurationData->TransactionID, 0);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the function that is called to process a*/
   /* CTS Client Configuration Update Event.                            */
   /* * NOTE * This function *MUST* be called with the TIP Manager Lock */
   /*          held.                                                    */
static void ProcessCTSConfigurationUpdateEvent(CTS_Client_Configuration_Update_Data_t *ClientConfigurationUpdateData)
{
   Connection_Entry_t *ConnectionEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* Verify that the input parameters seem semi-valid.                 */
   if(ClientConfigurationUpdateData)
   {
      /* Search for an existing connection entry for this device or add */
      /* one to the list.                                               */
      if((ConnectionEntryPtr = SearchAddConnectionEntry(&ConnectionEntryList, tctServer, ClientConfigurationUpdateData->RemoteDevice, ClientConfigurationUpdateData->ConnectionID)) != NULL)
      {
         /* Verify that we know what this descriptor is.                */
         if(ClientConfigurationUpdateData->ClientConfigurationType == ctCurrentTime)
         {
            /* Check to see if we have dispatched a connection for this */
            /* device.                                                  */
            if(ConnectionEntryPtr->ClientConnectedDispatched == FALSE)
            {
               /* We have not dispatched a Connection Event so do so    */
               /* now.                                                  */
               DispatchTIPConnectionEvent(ConnectionEntryPtr);

               /* Reload any stored Client Configuration for this       */
               /* device.                                               */
               ReloadConnectionConfiguration(ConnectionEntryPtr);
            }

            DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("New Current Time CCCD = 0x%04X.\n", ClientConfigurationUpdateData->ClientConfiguration));

            /* Save the new Current Time Configuration for this device. */
            ConnectionEntryPtr->CurrentTimeCCCD = ClientConfigurationUpdateData->ClientConfiguration;

            /* Store the new CCCD value to the configuration file (if   */
            /* paired).                                                 */
            StoreConnectionConfiguration(ConnectionEntryPtr, TRUE);
         }
         else
         {
            DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Unknown CTS Descriptor: %u\n", (unsigned int)ClientConfigurationUpdateData->ClientConfigurationType));
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the function that is called to process a*/
   /* CTS Read Current Time Request Event.                              */
   /* * NOTE * This function *MUST* be called with the TIP Manager Lock */
   /*          held.                                                    */
static void ProcessCTSReadCurrentTimeRequest(CTS_Read_Current_Time_Request_Data_t *ReadCurrentTimeRequestData)
{
   Connection_Entry_t *ConnectionEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* Verify that the input parameters seem semi-valid.                 */
   if(ReadCurrentTimeRequestData)
   {
      /* Search for an existing connection entry for this device or add */
      /* one to the list.                                               */
      if((ConnectionEntryPtr = SearchAddConnectionEntry(&ConnectionEntryList, tctServer, ReadCurrentTimeRequestData->RemoteDevice, ReadCurrentTimeRequestData->ConnectionID)) != NULL)
      {
         /* Check to see if we have dispatched a connection for this    */
         /* device.                                                     */
         if(ConnectionEntryPtr->ClientConnectedDispatched == FALSE)
         {
            /* We have not dispatched a Connection Event so do so now.  */
            DispatchTIPConnectionEvent(ConnectionEntryPtr);

            /* Reload any stored Client Configuration for this device.  */
            ReloadConnectionConfiguration(ConnectionEntryPtr);
         }

         /* Process the request to update the time.                     */
         ProcessUpdateCurrentTime(ReadCurrentTimeRequestData->TransactionID, TIPM_CURRENT_TIME_ADJUST_REASON_FLAGS_MANUAL_TIME_UPDATE);
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Failed to add to list\n"));

         /* Respond with an error to the request.                       */
         _TIPM_CTS_Current_Time_Read_Request_Response(ReadCurrentTimeRequestData->TransactionID, ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR, NULL);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the function that is called to process a*/
   /* CTS Read Reference Time Request Event.                            */
   /* * NOTE * This function *MUST* be called with the TIP Manager Lock */
   /*          held.                                                    */
static void ProcessCTSReadReferenceTimeRequest(CTS_Read_Reference_Time_Information_Request_Data_t *ReadReferenceTimeRequestData)
{
   Connection_Entry_t                     *ConnectionEntryPtr;
   TIPM_Reference_Time_Information_Data_t  ReferenceTimeInformation;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* Verify that the input parameters seem semi-valid.                 */
   if(ReadReferenceTimeRequestData)
   {
      /* Search for an existing connection entry for this device or add */
      /* one to the list.                                               */
      if((ConnectionEntryPtr = SearchAddConnectionEntry(&ConnectionEntryList, tctServer, ReadReferenceTimeRequestData->RemoteDevice, ReadReferenceTimeRequestData->ConnectionID)) != NULL)
      {
         /* Check to see if we have dispatched a connection for this    */
         /* device.                                                     */
         if(ConnectionEntryPtr->ClientConnectedDispatched == FALSE)
         {
            /* We have not dispatched a Connection Event so do so now.  */
            DispatchTIPConnectionEvent(ConnectionEntryPtr);

            /* Reload any stored Client Configuration for this device.  */
            ReloadConnectionConfiguration(ConnectionEntryPtr);
         }

         /* Save the Transaction ID for this request in the connection  */
         /* entry.                                                      */
         ConnectionEntryPtr->ReferenceTimeRequestTransactionID = ReadReferenceTimeRequestData->TransactionID;

         /* Dispatch the Request for the Reference Time if there are any*/
         /* callbacks to dispatch the request to.  If not we will       */
         /* respond with "Unknown" Reference Time Information.  Note    */
         /* this function has been designed to only allow 1 un-answered */
         /* Reference Time Request message to be outstanding at a time. */
         if(EventCallbackInfoList != NULL)
            DispatchTIPReferenceTimeRequestEvent();
         else
         {
            /* Initialize the Reference Time to "Unknown" values.       */
            ReferenceTimeInformation.Flags              = 0;
            ReferenceTimeInformation.Source             = tmsUnknown;
            ReferenceTimeInformation.Accuracy           = 0;
            ReferenceTimeInformation.Hours_Since_Update = 0;
            ReferenceTimeInformation.Days_Since_Update  = 0;

            /* Respond to the Reference Time Information Read Request.  */
            ProcessReferenceTimeInformationResponse(&ReferenceTimeInformation);
         }
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Failed to add to list\n"));

         /* Respond with an error to the request.                       */
         _TIPM_CTS_Reference_Time_Information_Read_Request_Response(ReadReferenceTimeRequestData->TransactionID, ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR, NULL);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the function that is responsible for    */
   /* processing CTS Server Events that have been received.  This       */
   /* function should ONLY be called with the Context locked AND ONLY in*/
   /* the context of an arbitrary processing thread.                    */
static void ProcessCTSServerEvent(CTS_Server_Event_Data_t *CTSServerEventData)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First make sure that the input parameter appears to be semi-valid.*/
   if(CTSServerEventData)
   {
      /* Process the event based on the event type.                     */
      switch(CTSServerEventData->Event_Data_Type)
      {
         case etCTS_Server_Read_Client_Configuration_Request:
            DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("CTS Read Client Configuration Request\n"));

            ProcessCTSReadConfigurationRequestEvent(&(CTSServerEventData->Event_Data.CTS_Read_Client_Configuration_Data));
            break;
         case etCTS_Server_Update_Client_Configuration_Request:
            DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("CTS Client Configuration Update\n"));

            ProcessCTSConfigurationUpdateEvent(&(CTSServerEventData->Event_Data.CTS_Client_Configuration_Update_Data));
            break;
         case etCTS_Server_Read_Current_Time_Request:
            DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("CTS Read Current Time Request\n"));

            ProcessCTSReadCurrentTimeRequest(&(CTSServerEventData->Event_Data.CTS_Read_Current_Time_Request_Data));
            break;
         case etCTS_Server_Read_Reference_Time_Information_Request:
            DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("CTS Read Reference Time Request\n"));

            ProcessCTSReadReferenceTimeRequest(&(CTSServerEventData->Event_Data.CTS_Read_Reference_Time_Information_Request_Data));
            break;
         default:
            /* Invalid/Unknown Event.                                   */
            DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid/Unknown CTS Event Type: %d\n", CTSServerEventData->Event_Data_Type));
            break;
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid CTS Event Data\n"));
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is called to process a Get Current Time    */
   /* Response event.                                                   */
   /* * NOTE * This function *MUST* be called with the TIPM Manager Lock*/
   /*          held.                                                    */
static void ProcessGetCurrentTimeResponse(GATT_Read_Response_Data_t *GATTReadResponseData, Transaction_Entry_t *TransactionEntry, unsigned int Status)
{
   int                                      Result;
   TIPM_Event_Data_t                        EventData;
   CTS_Current_Time_Data_t                  CTSCurrentTime;
   TIPM_Current_Time_Data_t                 CurrentTime;
   TIPM_Get_Current_Time_Response_Message_t Message;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First make sure that the input parameter appears to be semi-valid.*/
   if((TransactionEntry) && ((Status) || ((!Status) && (GATTReadResponseData))))
   {
      BTPS_MemInitialize(&CurrentTime, 0, sizeof(TIPM_Current_Time_Data_t));

      /* Check if the response is success.                              */
      if(!Status)
      {
         /* Attempt to decode the attribute response as Current Time    */
         /* data.                                                       */
         if(!(Result = _TIPM_Decode_Current_Time(GATTReadResponseData->AttributeValueLength, GATTReadResponseData->AttributeValue, &CTSCurrentTime)))
         {
            /* Now attempt to convert the data to a PM structure.       */
            Result = ConvertCTSToCurrentTime(&CTSCurrentTime, &CurrentTime);
         }
      }
      else
         Result = 0;

      /* Flag if we failed to decode the data.                          */
      if(Result < 0)
         Status = ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR;

      /* Format the event information.                                  */
      EventData.EventType                                                     = aetTIPGetCurrentTimeResponse;
      EventData.EventLength                                                   = TIPM_GET_CURRENT_TIME_RESPONSE_EVENT_DATA_SIZE;
      EventData.EventData.GetCurrentTimeResponseEventData.ClientCallbackID    = TransactionEntry->CallbackID;
      EventData.EventData.GetCurrentTimeResponseEventData.RemoteDeviceAddress = TransactionEntry->BD_ADDR;
      EventData.EventData.GetCurrentTimeResponseEventData.Status              = Status;
      EventData.EventData.GetCurrentTimeResponseEventData.CurrentTimeData     = CurrentTime;

      BTPS_MemInitialize(&Message, 0, sizeof(Message));

      Message.MessageHeader.AddressID       = 0;
      Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
      Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_TIME_MANAGER;
      Message.MessageHeader.MessageFunction = TIPM_MESSAGE_FUNCTION_GET_CURRENT_TIME_RESPONSE;
      Message.MessageHeader.MessageLength   = (TIPM_GET_CURRENT_TIME_RESPONSE_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

      Message.ClientEventHandlerID = TransactionEntry->CallbackID;
      Message.RemoteDeviceAddress  = TransactionEntry->BD_ADDR;
      Message.Status               = Status;
      Message.CurrentTimeData      = CurrentTime;

      /* Now dispatch the event to the client which is waiting.         */
      DispatchTIPMEventByID(&EventData, (BTPM_Message_t *)&Message, TransactionEntry->CallbackID, tctClient);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is called to process a Current Time        */
   /* Notification event.                                               */
   /* * NOTE * This function *MUST* be called with the TIPM Manager Lock*/
   /*          held.                                                    */
static void ProcessCurrentTimeNotification(GATT_Server_Notification_Data_t *GATTServerNotificationData, Notification_Registration_Entry_t *NotificationRegistrationEntry)
{
   TIPM_Event_Data_t                        EventData;
   CTS_Current_Time_Data_t                  CTSCurrentTime;
   TIPM_Current_Time_Data_t                 CurrentTime;
   TIPM_Current_Time_Notification_Message_t Message;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First make sure that the input parameter appears to be semi-valid.*/
   if((GATTServerNotificationData) && (NotificationRegistrationEntry))
   {
      BTPS_MemInitialize(&CurrentTime, 0, sizeof(TIPM_Current_Time_Data_t));

      /* Attempt to decode the attribute response as Current Time data. */
      if(!_TIPM_Decode_Current_Time(GATTServerNotificationData->AttributeValueLength, GATTServerNotificationData->AttributeValue, &CTSCurrentTime))
      {
         /* Now attempt to convert the data to a PM structure.          */
         if(!ConvertCTSToCurrentTime(&CTSCurrentTime, &CurrentTime))
         {
            /* Format the event information.                            */
            EventData.EventType                                                      = aetTIPCurrentTimeNotification;
            EventData.EventLength                                                    = TIPM_CURRENT_TIME_NOTIFICATION_EVENT_DATA_SIZE;
            EventData.EventData.CurrentTimeNotificationEventData.ClientCallbackID    = NotificationRegistrationEntry->CallbackID;
            EventData.EventData.CurrentTimeNotificationEventData.RemoteDeviceAddress = NotificationRegistrationEntry->BD_ADDR;
            EventData.EventData.CurrentTimeNotificationEventData.CurrentTimeData     = CurrentTime;

            BTPS_MemInitialize(&Message, 0, sizeof(Message));

            Message.MessageHeader.AddressID       = 0;
            Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
            Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_TIME_MANAGER;
            Message.MessageHeader.MessageFunction = TIPM_MESSAGE_FUNCTION_CURRENT_TIME_NOTIFICATION;
            Message.MessageHeader.MessageLength   = (TIPM_CURRENT_TIME_NOTIFICATION_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

            Message.ClientEventHandlerID = NotificationRegistrationEntry->CallbackID;
            Message.RemoteDeviceAddress  = NotificationRegistrationEntry->BD_ADDR;
            Message.CurrentTimeData      = CurrentTime;

            /* Now dispatch the event to the client which is waiting.   */
            DispatchTIPMEventByID(&EventData, (BTPM_Message_t *)&Message, NotificationRegistrationEntry->CallbackID, tctClient);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is called to process a Local Time          */
   /* Information Response event.                                       */
   /* * NOTE * This function *MUST* be called with the TIPM Manager Lock*/
   /*          held.                                                    */
static void ProcessLocalTimeInformationResponse(GATT_Read_Response_Data_t *GATTReadResponseData, Transaction_Entry_t *TransactionEntry, unsigned int Status)
{
   int                                            Result;
   TIPM_Event_Data_t                              EventData;
   CTS_Local_Time_Information_Data_t              CTSLocalTime;
   TIPM_Local_Time_Information_Data_t             LocalTime;
   TIPM_Local_Time_Information_Response_Message_t Message;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First make sure that the input parameter appears to be semi-valid.*/
   if((TransactionEntry) && ((Status) || ((!Status) && (GATTReadResponseData))))
   {
      BTPS_MemInitialize(&LocalTime, 0, sizeof(TIPM_Local_Time_Information_Data_t));

      /* Check if the response is success.                              */
      if(!Status)
      {
         /* Attempt to decode the local time information.               */
         if(!(Result = _TIPM_Decode_Local_Time_Information(GATTReadResponseData->AttributeValueLength, GATTReadResponseData->AttributeValue, &CTSLocalTime)))
         {
            /* Now attempt to convert the the structure.                */
            Result = ConvertCTSToLocalTime(&CTSLocalTime, &LocalTime);
         }
      }
      else
         Result = 0;

      /* Flag if we failed to decode the data.                          */
      if(Result < 0)
         Status = ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR;

      /* Format the event information.                                  */
      EventData.EventType                                                            = aetTIPLocalTimeInformationResponse;
      EventData.EventLength                                                          = TIPM_LOCAL_TIME_INFORMATION_RESPONSE_EVENT_DATA_SIZE;
      EventData.EventData.LocalTimeInformationResponseEventData.ClientCallbackID     = TransactionEntry->CallbackID;
      EventData.EventData.LocalTimeInformationResponseEventData.RemoteDeviceAddress  = TransactionEntry->BD_ADDR;
      EventData.EventData.LocalTimeInformationResponseEventData.Status               = Status;
      EventData.EventData.LocalTimeInformationResponseEventData.LocalTimeInformation = LocalTime;

      BTPS_MemInitialize(&Message, 0, sizeof(Message));

      Message.MessageHeader.AddressID       = 0;
      Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
      Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_TIME_MANAGER;
      Message.MessageHeader.MessageFunction = TIPM_MESSAGE_FUNCTION_LOCAL_TIME_INFORMATION_RESPONSE;
      Message.MessageHeader.MessageLength   = (TIPM_LOCAL_TIME_INFORMATION_RESPONSE_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

      Message.ClientEventHandlerID = TransactionEntry->CallbackID;
      Message.RemoteDeviceAddress  = TransactionEntry->BD_ADDR;
      Message.Status               = Status;
      Message.LocalTimeInformation = LocalTime;

      /* Now dispatch the event to the client which is waiting.         */
      DispatchTIPMEventByID(&EventData, (BTPM_Message_t *)&Message, TransactionEntry->CallbackID, tctClient);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is called to process a Time Accuracy       */
   /* Resposne event.                                                   */
   /* * NOTE * This function *MUST* be called with the TIPM Manager Lock*/
   /*          held.                                                    */
static void ProcessTimeAccuracyResponse(GATT_Read_Response_Data_t *GATTReadResponseData, Transaction_Entry_t *TransactionEntry, unsigned int Status)
{
   int                                    Result;
   TIPM_Event_Data_t                      EventData;
   CTS_Reference_Time_Information_Data_t  CTSReferenceTime;
   TIPM_Time_Accuracy_Response_Message_t  Message;
   TIPM_Reference_Time_Information_Data_t ReferenceTime;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First make sure that the input parameter appears to be semi-valid.*/
   if((TransactionEntry) && ((Status) || ((!Status) && (GATTReadResponseData))))
   {
      BTPS_MemInitialize(&ReferenceTime, 0, sizeof(TIPM_Reference_Time_Information_Data_t));

      /* Check if the response is success.                              */
      if(!Status)
      {
         /* Attempt to decode the Reference time information.           */
         if(!(Result = _TIPM_Decode_Reference_Time_Information(GATTReadResponseData->AttributeValueLength, GATTReadResponseData->AttributeValue, &CTSReferenceTime)))
         {
            /* Now attempt to convert the the structure.                */
            Result = ConvertCTSToReferenceTime(&CTSReferenceTime, &ReferenceTime);
         }
      }
      else
         Result = 0;

      /* Flag if we failed to decode the data.                          */
      if(Result < 0)
         Status = ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR;

      /* Format the event information.                                  */
      EventData.EventType                                                        = aetTIPTimeAccuracyResponse;
      EventData.EventLength                                                      = TIPM_TIME_ACCURACY_RESPONSE_EVENT_DATA_SIZE;
      EventData.EventData.TimeAccuracyResponseEventData.ClientCallbackID         = TransactionEntry->CallbackID;
      EventData.EventData.TimeAccuracyResponseEventData.RemoteDeviceAddress      = TransactionEntry->BD_ADDR;
      EventData.EventData.TimeAccuracyResponseEventData.Status                   = Status;
      EventData.EventData.TimeAccuracyResponseEventData.ReferenceTimeInformation = ReferenceTime;

      BTPS_MemInitialize(&Message, 0, sizeof(Message));

      Message.MessageHeader.AddressID       = 0;
      Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
      Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_TIME_MANAGER;
      Message.MessageHeader.MessageFunction = TIPM_MESSAGE_FUNCTION_TIME_ACCURACY_RESPONSE;
      Message.MessageHeader.MessageLength   = (TIPM_TIME_ACCURACY_RESPONSE_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

      Message.ClientEventHandlerID     = TransactionEntry->CallbackID;
      Message.RemoteDeviceAddress      = TransactionEntry->BD_ADDR;
      Message.Status                   = Status;
      Message.ReferenceTimeInformation = ReferenceTime;

      /* Now dispatch the event to the client which is waiting.         */
      DispatchTIPMEventByID(&EventData, (BTPM_Message_t *)&Message, TransactionEntry->CallbackID, tctClient);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is called to process a Next DST Change     */
   /* Response event.                                                   */
   /* * NOTE * This function *MUST* be called with the TIPM Manager Lock*/
   /*          held.                                                    */
static void ProcessNextDSTChangeResponse(GATT_Read_Response_Data_t *GATTReadResponseData, Transaction_Entry_t *TransactionEntry, unsigned int Status)
{
   int                                     Result;
   TIPM_Event_Data_t                       EventData;
   NDCS_Time_With_Dst_Data_t               NDCSTimeWithDST;
   TIPM_Time_With_DST_Data_t               TimeWithDST;
   TIPM_Next_DST_Change_Response_Message_t Message;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First make sure that the input parameter appears to be semi-valid.*/
   if((TransactionEntry) && ((Status) || ((!Status) && (GATTReadResponseData))))
   {
      BTPS_MemInitialize(&TimeWithDST, 0, sizeof(TIPM_Time_With_DST_Data_t));

      /* Check if the response is success.                              */
      if(!Status)
      {
         /* Attempt to decode the Reference time information.           */
         if(!(Result = _TIPM_Decode_Time_With_DST(GATTReadResponseData->AttributeValueLength, GATTReadResponseData->AttributeValue, &NDCSTimeWithDST)))
         {
            /* Now attempt to convert the the structure.                */
            Result = ConvertNDCSToTimeWithDST(&NDCSTimeWithDST, &TimeWithDST);
         }
      }
      else
         Result = 0;

      /* Flag if we failed to decode the data.                          */
      if(Result < 0)
         Status = ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR;

      /* Format the event information.                                  */
      EventData.EventType                                                    = aetTIPNextDSTChangeResponse;
      EventData.EventLength                                                  = TIPM_NEXT_DST_CHANGE_RESPONSE_EVENT_DATA_SIZE;
      EventData.EventData.NextDSTChangeResponseEventData.ClientCallbackID    = TransactionEntry->CallbackID;
      EventData.EventData.NextDSTChangeResponseEventData.RemoteDeviceAddress = TransactionEntry->BD_ADDR;
      EventData.EventData.NextDSTChangeResponseEventData.Status              = Status;
      EventData.EventData.NextDSTChangeResponseEventData.TimeWithDST         = TimeWithDST;

      BTPS_MemInitialize(&Message, 0, sizeof(Message));

      Message.MessageHeader.AddressID       = 0;
      Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
      Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_TIME_MANAGER;
      Message.MessageHeader.MessageFunction = TIPM_MESSAGE_FUNCTION_NEXT_DST_CHANGE_RESPONSE;
      Message.MessageHeader.MessageLength   = (TIPM_NEXT_DST_CHANGE_RESPONSE_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

      Message.ClientEventHandlerID = TransactionEntry->CallbackID;
      Message.RemoteDeviceAddress  = TransactionEntry->BD_ADDR;
      Message.Status               = Status;
      Message.TimeWithDST          = TimeWithDST;

      /* Now dispatch the event to the client which is waiting.         */
      DispatchTIPMEventByID(&EventData, (BTPM_Message_t *)&Message, TransactionEntry->CallbackID, tctClient);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is called to process a Time Update State   */
   /* event.                                                            */
   /* * NOTE * This function *MUST* be called with the TIPM Manager Lock*/
   /*          held.                                                    */
static void ProcessTimeUpdateStateResponse(GATT_Read_Response_Data_t *GATTReadResponseData, Transaction_Entry_t *TransactionEntry, unsigned int Status)
{
   int                                       Result;
   TIPM_Event_Data_t                         EventData;
   RTUS_Time_Update_State_Data_t             RTUSTimeUpdateState;
   TIPM_Time_Update_State_Data_t             TimeUpdateState;
   TIPM_Time_Update_State_Response_Message_t Message;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First make sure that the input parameter appears to be semi-valid.*/
   if((TransactionEntry) && ((Status) || ((!Status) && (GATTReadResponseData))))
   {
      BTPS_MemInitialize(&TimeUpdateState, 0, sizeof(TIPM_Time_Update_State_Data_t));

      /* Check if the response is success.                              */
      if(!Status)
      {
         /* Attempt to decode the Time Update State.                    */
         if(!(Result = _TIPM_Decode_Time_Update_State(GATTReadResponseData->AttributeValueLength, GATTReadResponseData->AttributeValue, &RTUSTimeUpdateState)))
         {
            /* Now attempt to convert the the structure.                */
            Result = ConvertRTUSToTimeUpdateState(&RTUSTimeUpdateState, &TimeUpdateState);
         }
      }
      else
         Result = 0;

      /* Flag if we failed to decode the data.                          */
      if(Result < 0)
         Status = ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR;

      /* Format the event information.                                  */
      EventData.EventType                                                      = aetTIPTimeUpdateStateResponse;
      EventData.EventLength                                                    = TIPM_TIME_UPDATE_STATE_RESPONSE_EVENT_DATA_SIZE;
      EventData.EventData.TimeUpdateStateResponseEventData.ClientCallbackID    = TransactionEntry->CallbackID;
      EventData.EventData.TimeUpdateStateResponseEventData.RemoteDeviceAddress = TransactionEntry->BD_ADDR;
      EventData.EventData.TimeUpdateStateResponseEventData.Status              = Status;
      EventData.EventData.TimeUpdateStateResponseEventData.TimeUpdateStateData = TimeUpdateState;

      BTPS_MemInitialize(&Message, 0, sizeof(Message));

      Message.MessageHeader.AddressID       = 0;
      Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
      Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_TIME_MANAGER;
      Message.MessageHeader.MessageFunction = TIPM_MESSAGE_FUNCTION_TIME_UPDATE_STATE_RESPONSE;
      Message.MessageHeader.MessageLength   = (TIPM_TIME_UPDATE_STATE_RESPONSE_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

      Message.ClientEventHandlerID = TransactionEntry->CallbackID;
      Message.RemoteDeviceAddress  = TransactionEntry->BD_ADDR;
      Message.Status               = Status;
      Message.TimeUpdateStateData  = TimeUpdateState;


      /* Now dispatch the event to the client which is waiting.         */
      DispatchTIPMEventByID(&EventData, (BTPM_Message_t *)&Message, TransactionEntry->CallbackID, tctClient);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility which performs cleanup when   */
   /* enabling notifications on a remote server fails or the device is  */
   /* disconnected.                                                     */
static void CleanupNotificationRegistration(Connection_Entry_t *ConnectionEntry)
{
   Notification_Registration_Entry_t *NotificationRegistrationEntry;
   Notification_Registration_Entry_t *NotificationRegistrationListHead;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First make sure that the input parameter appears to be semi-valid.*/
   if(ConnectionEntry)
   {
      /* Flag we are no longer expecting notifications.                 */
      ConnectionEntry->Flags &= ~((unsigned long)CONNECTION_ENTRY_FLAGS_NOTIFICATION_ENABLE_PENDING);
      ConnectionEntry->Flags &= ~((unsigned long)CONNECTION_ENTRY_FLAGS_NOTIFICATION_ENABLED);
      ConnectionEntry->Flags &= ~((unsigned long)CONNECTION_ENTRY_FLAGS_NOTIFICATION_DISABLE_PENDING);

      /* Now remove any registered callbacks related to this connection.*/
      NotificationRegistrationListHead = NotificationRegistrationList;

      while((NotificationRegistrationListHead) && ((NotificationRegistrationEntry = DeleteNotificationRegistrationEntry(&NotificationRegistrationListHead, NULL, &ConnectionEntry->BD_ADDR)) != NULL))
      {
         /* Note the next entry to start from.                          */
         NotificationRegistrationListHead = NotificationRegistrationEntry->NextNotificationRegistrationEntryPtr;

         /* Now, Delete the memory.                                     */
         FreeNotificationRegistrationEntryMemory(NotificationRegistrationEntry);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility which performs cleanup when a */
   /* client app is disconnected.                                       */
static void CleanupNotificationRegistrationByCallbackID(unsigned int CallbackID)
{
   Notification_Registration_Entry_t *NotificationRegistrationEntry;
   Notification_Registration_Entry_t *tmpNotificationRegistrationEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %u\n", CallbackID));

   /* First make sure that the input parameter appears to be semi-valid.*/
   if(CallbackID)
   {
      /* Start at the beginning of the notification list.               */
      NotificationRegistrationEntry = NotificationRegistrationList;

      /* Walk the list to find any registered notifications.            */
      while(NotificationRegistrationEntry)
      {
         /* Note the next entry, as we may delete the current one during*/
         /* the process.                                                */
         tmpNotificationRegistrationEntry = NotificationRegistrationEntry->NextNotificationRegistrationEntryPtr;

         /* Check if this entry is for the given callback.              */
         if(NotificationRegistrationEntry->CallbackID == CallbackID)
         {
            /* Simply process this as a fake call to Disable            */
            /* notifications from the client.                           */
            /* * NOTE * This call WILL delete the current entry. It is  */
            /*          now invalid.                                    */
            ProcessEnableTimeNotifications(NotificationRegistrationEntry->CallbackID, NotificationRegistrationEntry->BD_ADDR, FALSE);
         }

         /* Advance to the next entry.                                  */
         NotificationRegistrationEntry = tmpNotificationRegistrationEntry;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is called to process a GATT Read Response  */
   /* Event.                                                            */
   /* * NOTE * This function *MUST* be called with the TIPM Manager Lock*/
   /*          held.                                                    */
static void ProcessGATTReadResponseEvent(GATT_Read_Response_Data_t *GATTReadResponseEventData)
{
   Transaction_Entry_t *TransactionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First make sure that the input parameter appears to be semi-valid.*/
   if(GATTReadResponseEventData)
   {
      /* Make sure we are tracking this conneciton.                     */
      if(SearchConnectionEntry(&ConnectionEntryList, GATTReadResponseEventData->ConnectionID, tctClient))
      {
         /* Now attempt to locate this transaction.                     */
         if((TransactionEntry = DeleteTransactionEntry(&TransactionList, GATTReadResponseEventData->TransactionID)) != NULL)
         {
            /* Determine what response we were waiting on.              */
            switch(TransactionEntry->TransactionType)
            {
               case ttGetCurrentTime:
                  ProcessGetCurrentTimeResponse(GATTReadResponseEventData, TransactionEntry, 0);
                  break;
               case ttGetLocalTimeInformation:
                  ProcessLocalTimeInformationResponse(GATTReadResponseEventData, TransactionEntry, 0);
                  break;
               case ttGetTimeAccuracy:
                  ProcessTimeAccuracyResponse(GATTReadResponseEventData, TransactionEntry, 0);
                  break;
               case ttGetNextDSTChangeInformation:
                  ProcessNextDSTChangeResponse(GATTReadResponseEventData, TransactionEntry, 0);
                  break;
               case ttGetReferenceTimeUpdateState:
                  ProcessTimeUpdateStateResponse(GATTReadResponseEventData, TransactionEntry, 0);
                  break;
               default:
                  /* No other transaction type should return with a read*/
                  /* response.                                          */
                  DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Unexpected Transaction Response\n"));
                  break;
            }

            /* Free the transaction memory.                             */
            FreeTransactionEntryMemory(TransactionEntry);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is called to process a GATT Write Response */
   /* Event.                                                            */
   /* * NOTE * This function *MUST* be called with the TIPM Manager Lock*/
   /*          held.                                                    */
static void ProcessGATTWriteResponseEvent(GATT_Write_Response_Data_t *GATTWriteResponseEventData)
{
   Connection_Entry_t  *ConnectionEntry;
   Transaction_Entry_t *TransactionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First make sure that the input parameter appears to be semi-valid.*/
   if(GATTWriteResponseEventData)
   {
      /* Make sure we are tracking this conneciton.                     */
      if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, GATTWriteResponseEventData->ConnectionID, tctClient)) != NULL)
      {
         /* Now attempt to locate this transaction.                     */
         if((TransactionEntry = DeleteTransactionEntry(&TransactionList, GATTWriteResponseEventData->TransactionID)) != NULL)
         {
            /* Determine which action we were waiting for.              */
            switch(TransactionEntry->TransactionType)
            {
               case ttEnableNotifications:
                  /* Flag that we successfully enbaled notifications.   */
                  ConnectionEntry->Flags &= ~((unsigned long)CONNECTION_ENTRY_FLAGS_NOTIFICATION_ENABLE_PENDING);
                  ConnectionEntry->Flags |= (unsigned long)CONNECTION_ENTRY_FLAGS_NOTIFICATION_ENABLED;
                  break;
               case ttDisableNotifications:
                  /* Flag the we are no longer enabled.                 */
                  ConnectionEntry->Flags &= ~((unsigned long)CONNECTION_ENTRY_FLAGS_NOTIFICATION_DISABLE_PENDING);
                  break;
               default:
                  /* Ignore anything else.                              */
                  ;
            }

            /* Free the transaction memory.                             */
            FreeTransactionEntryMemory(TransactionEntry);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following funcion provides a method of processing a GATT Error*/
   /* Response.                                                         */
   /* * NOTE * This function *MUST* be called with the TIPM Manager Lock*/
   /*          held.                                                    */
static void ProcessGATTErrorResponseEvent(GATT_Request_Error_Data_t *GATTRequestErrorData)
{
   unsigned int         ErrorCode;
   Connection_Entry_t  *ConnectionEntry;
   Transaction_Entry_t *TransactionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First make sure that the input parameter appears to be semi-valid.*/
   if(GATTRequestErrorData)
   {
      /* Make sure we are tracking this conneciton.                     */
      if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, GATTRequestErrorData->ConnectionID, tctClient)) != NULL)
      {
         /* Now attempt to locate this transaction.                     */
         if((TransactionEntry = DeleteTransactionEntry(&TransactionList, GATTRequestErrorData->TransactionID)) != NULL)
         {
            /* Determine the Error Code to dispatch.                    */
            if(GATTRequestErrorData->ErrorType == retErrorResponse)
               ErrorCode = GATTRequestErrorData->ErrorCode;
            else
               ErrorCode = ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR;

            /* Determine what type of transaction we were waiting on.   */
            switch(TransactionEntry->TransactionType)
            {
               case ttGetCurrentTime:
                  ProcessGetCurrentTimeResponse(NULL, TransactionEntry, ErrorCode);
                  break;
               case ttGetLocalTimeInformation:
                  ProcessLocalTimeInformationResponse(NULL, TransactionEntry, ErrorCode);
                  break;
               case ttGetTimeAccuracy:
                  ProcessTimeAccuracyResponse(NULL, TransactionEntry, ErrorCode);
                  break;
               case ttGetNextDSTChangeInformation:
                  ProcessNextDSTChangeResponse(NULL, TransactionEntry, ErrorCode);
                  break;
               case ttGetReferenceTimeUpdateState:
                  ProcessTimeUpdateStateResponse(NULL, TransactionEntry, ErrorCode);
                  break;
               case ttEnableNotifications:
                  CleanupNotificationRegistration(ConnectionEntry);
                  break;
               default:
                  /* No other transaction type should return with a read*/
                  /* response.                                          */
                  DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Unexpected Error Response\n"));
            }

            /* Free the transaction memory.                             */
            FreeTransactionEntryMemory(TransactionEntry);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is called to process a GATT Server         */
   /* Notification Event.                                               */
   /* * NOTE * This function *MUST* be called with the TIPM Manager Lock*/
   /*          held.                                                    */
static void ProcessGATTServerNotificationEvent(GATT_Server_Notification_Data_t *GATTServerNotificationEventData)
{
   Connection_Entry_t                *ConnectionEntry;
   Notification_Registration_Entry_t *NotificationRegistrationEntry;
   Notification_Registration_Entry_t *NotificationRegistrationListHead;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First make sure that the input parameter appears to be semi-valid.*/
   if(GATTServerNotificationEventData)
   {
      /* Make sure we are tracking this conneciton.                     */
      if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, GATTServerNotificationEventData->ConnectionID, tctClient)) != NULL)
      {
         /* Make sure we are not pending a disable transaction.         */
         if(ConnectionEntry->Flags & CONNECTION_ENTRY_FLAGS_NOTIFICATION_ENABLED)
         {
            /* Get the beginning of the registered notification list.   */
            NotificationRegistrationListHead = NotificationRegistrationList;

            /* Submit the Notification data to each registered callback.*/
            while((NotificationRegistrationListHead) && ((NotificationRegistrationEntry = SearchNotificationRegistrationEntry(&NotificationRegistrationListHead, NULL, &ConnectionEntry->BD_ADDR)) != NULL))
            {
               /* Send the notification to this callback.               */
               ProcessCurrentTimeNotification(GATTServerNotificationEventData, NotificationRegistrationEntry);

               /* Continue the search from after this callback.         */
               NotificationRegistrationListHead = NotificationRegistrationEntry->NextNotificationRegistrationEntryPtr;
            }
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is called to process a GATT Client Event.  */
   /* * NOTE * This function *MUST* be called with the TIPM Manager Lock*/
   /*          held.                                                    */
static void ProcessGATTClientEvent(TIPM_GATT_Client_Event_Data_t *GATTClientEventData)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First make sure that the input parameter appears to be semi-valid.*/
   if(GATTClientEventData)
   {
      /* Process the event based on the event type.                     */
      switch(GATTClientEventData->Event_Data_Type)
      {
         case etGATT_Client_Error_Response:
            DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("GATT Request Error Response\n"));

            ProcessGATTErrorResponseEvent(&(GATTClientEventData->Event_Data.GATT_Request_Error_Data));
            break;
         case etGATT_Client_Write_Response:
            DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("GATT Client Write Response\n"));

            ProcessGATTWriteResponseEvent(&(GATTClientEventData->Event_Data.GATT_Write_Response_Data));
            break;
         case etGATT_Client_Read_Response:
            DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("GATT Client Read Response\n"));

            ProcessGATTReadResponseEvent(&(GATTClientEventData->Event_Data.GATT_Read_Response_Data));
            break;
         default:
            DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Unknown GATT Client Event Type: %d\n", GATTClientEventData->Event_Data_Type));

      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid GATT Client Event Data\n"));
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is called to process a GATT Connection     */
   /* Event.                                                            */
   /* * NOTE * This function *MUST* be called with the TIPM Manager Lock*/
   /*          held.                                                    */
static void ProcessGATTConnectionEvent(TIPM_GATT_Connection_Event_Data_t *GATTConnectionEventData)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First make sure that the input parameter appears to be semi-valid.*/
   if(GATTConnectionEventData)
   {
      switch(GATTConnectionEventData->Event_Data_Type)
      {
         case etGATT_Connection_Server_Notification:
            DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("GATT Server Notification\n"));

            ProcessGATTServerNotificationEvent(&(GATTConnectionEventData->Event_Data.GATT_Server_Notification_Data));
            break;
         default:
            DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Unknown GATT Connection Event Type: %d\n", GATTConnectionEventData->Event_Data_Type));
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid GATT Connection Event Data\n"));
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));

}

   /* The following function is the function that is called to process a*/
   /* LE Connect Event.                                                 */
   /* * NOTE * This function *MUST* be called with the TIPM Manager Lock*/
   /*          held.                                                    */
static void ProcessLowEnergyConnectionEvent(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties)
{
   int                 Result;
   unsigned int        ConnectionID;
   Connection_Entry_t  ConnectionEntry;
   Connection_Entry_t *ConnectionEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if(RemoteDeviceProperties)
   {
      /* First attempt to get the Connection ID of the Connection.      */
      if(_TIPM_Query_Connection_ID(RemoteDeviceProperties->BD_ADDR, &ConnectionID))
      {
         /* Check to see if this device has another address that is     */
         /* actually the connection address.                            */
         if(!_TIPM_Query_Connection_ID(RemoteDeviceProperties->PriorResolvableBD_ADDR, &ConnectionID))
            Result = 0;
         else
            Result = -1;
      }
      else
         Result = 0;

      /* Continue only if no error has occurred.                        */
      if(!Result)
      {
         /* Initialize the Connection Entry.                            */
         BTPS_MemInitialize(&ConnectionEntry, 0, sizeof(Connection_Entry_t));

         ConnectionEntry.BD_ADDR      = RemoteDeviceProperties->BD_ADDR;
         ConnectionEntry.ConnectionID = ConnectionID;

         /* Attempt to reload the configuration for this device.        */
         ReloadConnectionConfiguration(&ConnectionEntry);

         /* If the client has registered for notifications we will      */
         /* dispatch a connection event and add this connection to our  */
         /* local list.                                                 */
         if(ConnectionEntry.CurrentTimeCCCD)
         {
            /* Search for an existing connection entry for this device  */
            /* or add one to the list.                                  */
            if((ConnectionEntryPtr = SearchAddConnectionEntry(&ConnectionEntryList, tctServer, ConnectionEntry.BD_ADDR, ConnectionEntry.ConnectionID)) != NULL)
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("TIPM Connection established due to stored configuration\n"));

               /* Check to see if we have dispatched a connection for   */
               /* this device.                                          */
               if(ConnectionEntryPtr->ClientConnectedDispatched == FALSE)
               {
                  /* We have not dispatched a Connection Event so do so */
                  /* now.                                               */
                  DispatchTIPConnectionEvent(ConnectionEntryPtr);
               }

               /* Save the reloaded configuration for this device.      */
               ConnectionEntryPtr->CurrentTimeCCCD   = ConnectionEntry.CurrentTimeCCCD;
            }
         }

         /* It is also possible the connected device has a time server, */
         /* so track it as a possible client connection.                */
         if(SupportedRoles & TIPM_INITIALIZATION_INFO_SUPPORTED_ROLES_CLIENT)
         {
            ConnectionEntry.ConnectionType = tctClient;

            if((ConnectionEntryPtr = AddConnectionEntry(&ConnectionEntryList, &ConnectionEntry)) != NULL)
            {
               /* Check to see if we know the device's services.        */
               if(RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_LE_SERVICES_KNOWN)
               {
                  /* Attempt to configure a TIP client connection if the*/
                  /* service exists.                                    */
                  ConfigureTIPClientConnection(ConnectionEntryPtr);
               }
            }
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the function that is called to process a*/
   /* LE Disconnect Event.                                              */
   /* * NOTE * This function *MUST* be called with the TIP Manager Lock */
   /*          held.                                                    */
static void ProcessLowEnergyDisconnectionEvent(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties)
{
   Connection_Entry_t *ConnectionEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if(RemoteDeviceProperties)
   {
      /* Delete the specified Connection Entry from the list for both   */
      /* Client and Server connections (if any exists).                 */
      if((ConnectionEntryPtr = DeleteConnectionEntryByBD_ADDR(&ConnectionEntryList, RemoteDeviceProperties->BD_ADDR, tctServer)) != NULL)
      {
         /* Dispatch a TIPM Disconnection Event.                        */
         DispatchTIPDisconnectionEvent(ConnectionEntryPtr);

         /* Free the memory that was allocated for this entry.          */
         FreeConnectionEntryMemory(ConnectionEntryPtr);
      }

      if((ConnectionEntryPtr = DeleteConnectionEntryByBD_ADDR(&ConnectionEntryList, RemoteDeviceProperties->BD_ADDR, tctClient)) != NULL)
      {
         /* Dispatch a TIPM Disconnection Event if the device was a TIP */
         /* connection.                                                 */
         if(ConnectionEntryPtr->ClientConnectionInfo.SupportedServicesMask & TIPM_SUPPORTED_SERVICES_CTS)
            DispatchTIPDisconnectionEvent(ConnectionEntryPtr);

         /* Free the memory that was allocated for this entry.          */
         FreeConnectionEntryMemory(ConnectionEntryPtr);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the function that is called to process a*/
   /* LE Address Change Event.                                          */
   /* * NOTE * This function *MUST* be called with the TIP Manager Lock */
   /*          held.                                                    */
static void ProcessLowEnergyAddressChangeEvent(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties)
{
   Connection_Entry_t *ConnectionEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if(RemoteDeviceProperties)
   {
      /* Walk the Connection List and update any BD_ADDRs as needed.    */
      ConnectionEntryPtr = ConnectionEntryList;
      while(ConnectionEntryPtr)
      {
         /* Check to see if this entry needs to be updated.             */
         if(COMPARE_BD_ADDR(ConnectionEntryPtr->BD_ADDR, RemoteDeviceProperties->PriorResolvableBD_ADDR))
         {
            DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("TIPM Address Updated\n"));

            /* Save the new Base Address.                               */
            ConnectionEntryPtr->BD_ADDR = RemoteDeviceProperties->BD_ADDR;
         }

         ConnectionEntryPtr = ConnectionEntryPtr->NextConnectionEntryPtr;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the function that is called to process a*/
   /* LE Pairing Change Event.                                          */
   /* * NOTE * This function *MUST* be called with the TIPM Manager Lock*/
   /*          held.                                                    */
static void ProcessLowEnergyPairingChangeEvent(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties)
{
   Connection_Entry_t  ConnectionEntry;
   Connection_Entry_t *ConnectionEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if(RemoteDeviceProperties)
   {
      /* Check to see if we just paired or unpaired from the device.    */
      if(RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_CURRENTLY_PAIRED_OVER_LE)
      {
         /* Walk the Connection List and find the connection entry.     */
         ConnectionEntryPtr = ConnectionEntryList;
         while(ConnectionEntryPtr)
         {
            /* Check to see if this entry needs to be updated.          */
            if(COMPARE_BD_ADDR(ConnectionEntryPtr->BD_ADDR, RemoteDeviceProperties->BD_ADDR))
            {
               /* If we are currently paired go ahead and update the    */
               /* device file.                                          */
               StoreConnectionConfiguration(ConnectionEntryPtr, TRUE);
               break;
            }

            ConnectionEntryPtr = ConnectionEntryPtr->NextConnectionEntryPtr;
         }
      }
      else
      {
         /* If not currently paired over LE, then go ahead and delete   */
         /* any stored configuration for device.                        */
         ConnectionEntry.BD_ADDR = RemoteDeviceProperties->BD_ADDR;
         StoreConnectionConfiguration(&ConnectionEntry, FALSE);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is called to process a LE Services State   */
   /* Change Event.                                                     */
   /* * NOTE * This function *MUST* be called with the TIPM Manager Lock*/
   /*          held.                                                    */
static void ProcessLowEnergyServicesStateChangeEvent(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties)
{
   int                 Result;
   Boolean_t           TIPClientConnected;
   unsigned int        ConnectionID;
   Connection_Entry_t *ConnectionEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if(RemoteDeviceProperties)
   {
      /* Get the connection ID of the connection.                       */
      if(_TIPM_Query_Connection_ID(RemoteDeviceProperties->BD_ADDR, &ConnectionID))
      {
         /* Check to see if this device has another address that is     */
         /* actually the connection address.                            */
         if(!_TIPM_Query_Connection_ID(RemoteDeviceProperties->PriorResolvableBD_ADDR, &ConnectionID))
            Result = 0;
         else
            Result = -1;
      }
      else
         Result = 0;

      /* Continue only if no error has occurred.                        */
      if(!Result)
      {
         /* Look for a client connection.                               */
         if((ConnectionEntryPtr = SearchConnectionEntry(&ConnectionEntryList, ConnectionID, tctClient)) != NULL)
         {
            /* Make sure we know the services now.                      */
            if(RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_LE_SERVICES_KNOWN)
            {
               /* Determine if this connection currently has a TIP      */
               /* Client connection.                                    */
               TIPClientConnected = (Boolean_t)((ConnectionEntryPtr->ClientConnectionInfo.SupportedServicesMask & TIPM_SUPPORTED_SERVICES_CTS)?TRUE:FALSE);

               /* The services may have changed, so dump the old        */
               /* connection state.                                     */
               BTPS_MemInitialize(&ConnectionEntryPtr->ClientConnectionInfo, 0, sizeof(Client_Connection_Info_t));

               /* Attempt to configure a TIP Client connection.         */
               ConfigureTIPClientConnection(ConnectionEntryPtr);

               /* If we previously had a connection, and we no longer   */
               /* do, we need to dispatch a disconnection.              */
               if((TIPClientConnected) && !(ConnectionEntryPtr->ClientConnectionInfo.SupportedServicesMask & TIPM_SUPPORTED_SERVICES_CTS))
                  DispatchTIPDisconnectionEvent(ConnectionEntryPtr);

            }
            else
            {
               /* We know longer no the services. Disconnect the client */
               /* connection.                                           */
               BTPS_MemInitialize(&ConnectionEntryPtr->ClientConnectionInfo, 0, sizeof(Client_Connection_Info_t));

               /* Dispatch the disconnected callback.                   */
               DispatchTIPDisconnectionEvent(ConnectionEntryPtr);
            }
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a Remote Device Properties Changed Event.                 */
   /* * NOTE * This function *MUST* be called with the TIP Manager Lock */
   /*          held.                                                    */
static void ProcessRemoteDevicePropertiesChangedEvent(unsigned long ChangedMemberMask, DEVM_Remote_Device_Properties_t *RemoteDeviceProperties)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08lX\n", ChangedMemberMask));

   /* First make sure that the input parameter appears to be semi-valid.*/
   if((ChangedMemberMask) && (RemoteDeviceProperties) && (RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_SUPPORTS_LOW_ENERGY))
   {
      /* Check to see what changed.  We are only interested if the LE   */
      /* Connection State or the LE Service state or the Address is     */
      /* updated.                                                       */
      if(ChangedMemberMask & (DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_CONNECTION_STATE | DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_PRIOR_RESOLVABLE_ADDRESS | DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_PAIRING_STATE | DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_SERVICES_STATE))
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Flags: 0x%08lX\n", RemoteDeviceProperties->RemoteDeviceFlags));

         /* Handle Connections/Disconnections.                          */
         if(ChangedMemberMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_CONNECTION_STATE)
         {
            /* Check to see if we are currently connected or a          */
            /* connection was just disconnected.                        */
            if(RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_CURRENTLY_CONNECTED_OVER_LE)
            {
               /* Process the Low Energy Connection.                    */
               ProcessLowEnergyConnectionEvent(RemoteDeviceProperties);
            }
            else
            {
               /* Process the Low Energy Disconnection.                 */
               ProcessLowEnergyDisconnectionEvent(RemoteDeviceProperties);
            }
         }

         /* Process the LE Pairing State Change Event if necessary.     */
         if(ChangedMemberMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_PAIRING_STATE)
            ProcessLowEnergyPairingChangeEvent(RemoteDeviceProperties);

         /* Process the Address Updated event if necessary.             */
         if(ChangedMemberMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_PRIOR_RESOLVABLE_ADDRESS)
            ProcessLowEnergyAddressChangeEvent(RemoteDeviceProperties);

         /* Process the Remote Device Service Known event if necessary. */
         if(ChangedMemberMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_SERVICES_STATE)
            ProcessLowEnergyServicesStateChangeEvent(RemoteDeviceProperties);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process TIP Manager Asynchronous Events.            */
static void BTPSAPI BTPMDispatchCallback_TIPM(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process ANS Events.                                 */
static void BTPSAPI BTPMDispatchCallback_CTS(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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
            /* Double check that this is an ANS Event Update.           */
            if(((TIPM_Update_Data_t *)CallbackParameter)->UpdateType == utCTSServerEvent)
               ProcessCTSServerEvent(&(((TIPM_Update_Data_t *)CallbackParameter)->UpdateData.CTSServerEventData));

            /* Release the lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
      }

      /* Go ahead and free the memory because we are finished with it.  */
      BTPS_FreeMemory(CallbackParameter);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process Client Un-Register Asynchronous Events.     */
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process GATT Client Asynchronous Events.            */
static void BTPSAPI BTPMDispatchCallback_GATTClient(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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
            /* Double check that this is an ANS Event Update.           */
            if(((TIPM_Update_Data_t *)CallbackParameter)->UpdateType == utGATTClientEvent)
               ProcessGATTClientEvent(&(((TIPM_Update_Data_t *)CallbackParameter)->UpdateData.GATTClientEventData));

            /* Release the lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
      }

      /* Go ahead and free the memory because we are finished with it.  */
      BTPS_FreeMemory(CallbackParameter);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process GATT Connection Asynchronous Events.        */
static void BTPSAPI BTPMDispatchCallback_GATTConnection(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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
            /* Double check that this is an ANS Event Update.           */
            if(((TIPM_Update_Data_t *)CallbackParameter)->UpdateType == utGATTConnectionEvent)
               ProcessGATTConnectionEvent(&(((TIPM_Update_Data_t *)CallbackParameter)->UpdateData.GATTConnectionEventData));

            /* Release the lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
      }

      /* Go ahead and free the memory because we are finished with it.  */
      BTPS_FreeMemory(CallbackParameter);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function represents the Message Group Handler       */
   /* function that is installed to process all TIP Manager Messages.   */
static void BTPSAPI TIPManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the Message specified appears to be semi-valid.   */
   if(Message)
   {
      /* Next, check to see if this message is specified for our        */
      /* specific Message Group.                                        */
      if(Message->MessageHeader.MessageGroup == BTPM_MESSAGE_GROUP_TIME_MANAGER)
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("TIP Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

         /* Message is destined for us, make sure we are still          */
         /* processing messages.                                        */
         if(Initialized)
         {
            /* Check to see if this message is a TIP Manager defined    */
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
               /* TIP Manager Thread.                                   */
               if(!BTPM_QueueMailboxCallback(BTPMDispatchCallback_TIPM, (void *)Message))
               {
                  DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue TIP Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

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
                        DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue TIP Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));
                     }
                  }
               }
            }
         }
         else
         {
            /* Check to see if this message is an TIP Manager defined   */
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
         DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Non TIP Manager Message: 0x%08X, 0x%08X\n", Message->MessageHeader.MessageGroup, Message->MessageHeader.MessageFunction));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for initializing/cleaning up*/
   /* the Bluetopia Platform Manager TIP Manager Module.  This function */
   /* should be registered with the Bluetopia Platform Manager Module   */
   /* Handler and will be called when the Platform Manager is           */
   /* initialized (or shut down).                                       */
void BTPSAPI TIPM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData)
{
   int Result;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %d\n", Initialize));

   /* First, determine if the request is to initialize or shut down.    */
   if(Initialize)
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Initialization Request\n"));

      if(!Initialized)
      {
         /* Attempt to get the supported roles.                         */
         /* * NOTE * For compatibility, we will assume un-supplied data */
         /*          supports both roles.                               */
         if(InitializationData)
            SupportedRoles = ((TIPM_Initialization_Info_t *)InitializationData)->SupportedRoles;
         else
            SupportedRoles = TIPM_INITIALIZATION_INFO_SUPPORTED_ROLES_SERVER | TIPM_INITIALIZATION_INFO_SUPPORTED_ROLES_CLIENT;

         /* Note the new initialization state.                          */
         Initialized = TRUE;

         DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing TIP Manager\n"));

         /* Next, let's attempt to register our Message Group Handler to*/
         /* process TIP Manager messages.                               */
         if(!MSG_RegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_TIME_MANAGER, TIPManagerGroupHandler, NULL))
         {
            /* Initialize the actual TIP Manager Implementation Module  */
            /* (this is the module that is actually responsible for     */
            /* actually implementing the TIP Manager functionality -    */
            /* this module is just the framework shell).                */
            if(!(Result = _TIPM_Initialize(SupportedRoles)))
            {
               /* Determine the current Device Power State.             */
               CurrentPowerState       = (Boolean_t)((DEVM_QueryDevicePowerState() > 0)?TRUE:FALSE);

               /* Initialize a unique, starting TIP Callback ID.        */
               NextEventCallbackID     = 0;

               /* Go ahead and flag that this module is initialized.    */
               Initialized             = TRUE;

               /* Flag success.                                         */
               Result                  = 0;
            }
         }
         else
            Result = BTPM_ERROR_CODE_UNABLE_TO_REGISTER_HANDLER;

         /* If an error occurred then we need to free all resources that*/
         /* were allocated.                                             */
         if(Result)
         {
            _TIPM_Cleanup();

            MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_TIME_MANAGER);
         }
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("TIP Manager Already initialized\n"));
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Shutdown Request\n"));

      if(Initialized)
      {
         /* First, let's make sure that we un-register our Message Group*/
         /* Handler (so that we do not process any more incoming        */
         /* messages).                                                  */
         MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_TIME_MANAGER);

         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Make sure we inform the TIP Manager Implementation that  */
            /* we are shutting down.                                    */
            _TIPM_Cleanup();

            /* Free the Connection Info list.                           */
            FreeConnectionEntryList(&ConnectionEntryList);

            /* Free the Event Callback Info List.                       */
            FreeEventCallbackInfoList(&EventCallbackInfoList);

            /* Free the Client Event Callbacks.                         */
            FreeEventCallbackInfoList(&ClientEventCallbackInfoList);

            /* Free the Transaction List.                               */
            FreeTransactionList(&TransactionList);

            /* Free the Notification Registration List.                 */
            FreeNotificationRegistrationList(&NotificationRegistrationList);

            /* Flag that the device is not powered on.                  */
            CurrentPowerState       = FALSE;

            /* Flag that this module is no longer initialized.          */
            Initialized             = FALSE;

            /* Free the Lock we acquired earlier.                       */
            DEVM_ReleaseLock();
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing asynchronous */
   /* Device Manager (DEVM) Events (including Power On/Off events).     */
   /* This function should be registered with the Bluetopia Platform    */
   /* Manager Module Handler and will be called when an asynchronous    */
   /* Device Manager event is dispatched.                               */
void BTPSAPI TIPM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData)
{
   int                            Result;
   Connection_Entry_t             ConnectionEntry;
   Connection_Entry_t            *ConnectionEntryPtr;
   GATT_Attribute_Handle_Group_t  ServiceHandleRange;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the TIP Manager has been initialized.   */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         switch(EventData->EventType)
         {
            case detDevicePoweredOn:
               DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power On Occurred.\n"));

               /* Power on Event.                                       */

               /* Note the new Power State.                             */
               CurrentPowerState = TRUE;

               /* Go ahead and inform the TIP Manager that it should    */
               /* initialize.                                           */
               if((Result = DEVM_QueryDeviceBluetoothStackID()) > 0)
               {
                  /* Attempt to calculate the Service Handle Range for  */
                  /* this service in the GATT database.                 */
                  if(CalculateServiceHandleRange(&ServiceHandleRange))
                     _TIPM_SetBluetoothStackID((unsigned int)Result, &ServiceHandleRange);
                  else
                     _TIPM_SetBluetoothStackID((unsigned int)Result, NULL);
               }
               break;
            case detDevicePoweringOff:
            case detDevicePoweredOff:
               DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power Off Occurred.\n"));

               /* Note the new Power State.                             */
               CurrentPowerState = FALSE;

               /* Inform the TIP Manager that the Stack has been closed.*/
               _TIPM_SetBluetoothStackID(0, NULL);

               /* Free the Connection Info list.                        */
               FreeConnectionEntryList(&ConnectionEntryList);
               break;
            case detRemoteDevicePropertiesChanged:
               DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Remote Device Properties Changed.\n"));

               /* Process the Remote Device Properties Changed Event.   */
               ProcessRemoteDevicePropertiesChangedEvent(EventData->EventData.RemoteDevicePropertiesChangedEventData.ChangedMemberMask, &(EventData->EventData.RemoteDevicePropertiesChangedEventData.RemoteDeviceProperties));
               break;
            case detRemoteDeviceDeleted:
               DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Remote Device Deleted.\n"));

               /* Delete the specified Connection Entry from the list   */
               /* (if any exists).                                      */
               if((ConnectionEntryPtr = DeleteConnectionEntryByBD_ADDR(&ConnectionEntryList, EventData->EventData.RemoteDeviceDeletedEventData.RemoteDeviceAddress, tctServer)) != NULL)
               {
                  /* Dispatch a TIPM Disconnection Event.               */
                  DispatchTIPDisconnectionEvent(ConnectionEntryPtr);

                  /* Free the memory that was allocated for this entry. */
                  FreeConnectionEntryMemory(ConnectionEntryPtr);
               }

               /* Now delete client connections.                        */
               if((ConnectionEntryPtr = DeleteConnectionEntryByBD_ADDR(&ConnectionEntryList, EventData->EventData.RemoteDeviceDeletedEventData.RemoteDeviceAddress, tctClient)) != NULL)
               {
                  /* Dispatch a TIPM Disconnection Event.               */
                  DispatchTIPDisconnectionEvent(ConnectionEntryPtr);

                  /* Free the memory that was allocated for this entry. */
                  FreeConnectionEntryMemory(ConnectionEntryPtr);
               }

               /* Delete any stored configuration for this device.      */
               ConnectionEntry.BD_ADDR = EventData->EventData.RemoteDeviceDeletedEventData.RemoteDeviceAddress;
               StoreConnectionConfiguration(&ConnectionEntry, FALSE);
               break;
            default:
               /* Do nothing.                                           */
               break;
         }

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow the caller the ability*/
   /* to notify the TIP Manager of a specific Update Event.  The TIP    */
   /* Manager can then take the correct action to process the update.   */
Boolean_t TIPM_NotifyUpdate(TIPM_Update_Data_t *UpdateData)
{
   Boolean_t ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, make sure that the input parameters appear to be           */
   /* semi-valid.                                                       */
   if(UpdateData)
   {
      switch(UpdateData->UpdateType)
      {
         case utCTSServerEvent:
            DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Queueing CTS Event: %d\n", UpdateData->UpdateData.CTSServerEventData.Event_Data_Type));

            /* Simply Queue the Dispatch Data.                          */
            /* * NOTE * To avoid copying again, we will simply submit   */
            /*          the entire Notification Message.                */
            ret_val = BTPM_QueueMailboxCallback(BTPMDispatchCallback_CTS, (void *)UpdateData);
            break;
         case utGATTClientEvent:
            DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Queueing GATT Client Event: %d\n", UpdateData->UpdateData.GATTClientEventData.Event_Data_Type));

            /* Simply Queue the Dispatch Data.                          */
            /* * NOTE * To avoid copying again, we will simply submit   */
            /*          the entire Notification Message.                */
            ret_val = BTPM_QueueMailboxCallback(BTPMDispatchCallback_GATTClient, (void *)UpdateData);
            break;
         case utGATTConnectionEvent:
            DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Queueing GATT Connection Event: %d\n", UpdateData->UpdateData.GATTConnectionEventData.Event_Data_Type));

            /* Simply Queue the Dispatch Data.                          */
            /* * NOTE * To avoid copying again, we will simply submit   */
            /*          the entire Notification Message.                */
            ret_val = BTPM_QueueMailboxCallback(BTPMDispatchCallback_GATTConnection, (void *)UpdateData);
            break;
         default:
            DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled Notification: %d\n", UpdateData->UpdateType));

            ret_val = FALSE;
            break;
      }
   }
   else
      ret_val = FALSE;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a server callback function with the Time (TIP)*/
   /* Manager Service.  This Callback will be dispatched by the TIP     */
   /* Manager when various TIP Manager Server Events occur.  This       */
   /* function accepts the Callback Function and Callback Parameter     */
   /* (respectively) to call when a TIP Manager Server Event needs to be*/
   /* dispatched.  This function returns a positive (non-zero) value if */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          TIPM_Un_Register_Server_Event_Callback() function to     */
   /*          un-register the callback from this module.               */
int BTPSAPI TIPM_Register_Server_Event_Callback(TIPM_Event_Callback_t CallbackFunction, void *CallbackParameter)
{
   int                        ret_val;
   TIPM_Event_Callback_Info_t EventCallbackEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Time Manager has been initialized.  */
   if(Initialized)
   {
      /* Check to see if the input parameters appear to be semi-valid.  */
      if(CallbackFunction)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Attempt to add an entry into the Event Callback Entry    */
            /* list.                                                    */
            BTPS_MemInitialize(&EventCallbackEntry, 0, sizeof(TIPM_Event_Callback_Info_t));

            EventCallbackEntry.EventCallbackID   = GetNextEventCallbackID();
            EventCallbackEntry.ClientID          = MSG_GetServerAddressID();
            EventCallbackEntry.EventCallback     = CallbackFunction;
            EventCallbackEntry.CallbackParameter = CallbackParameter;

            if(AddEventCallbackInfoEntry(&EventCallbackInfoList, &EventCallbackEntry))
               ret_val = EventCallbackEntry.EventCallbackID;
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
      ret_val = BTPM_ERROR_CODE_TIME_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered TIP Manager Server Event      */
   /* Callback (registered via a successful call to the                 */
   /* TIPM_Register_Server_Event_Callback() function).  This function   */
   /* accepts as input the Server Event Callback ID (return value from  */
   /* TIPM_Register_Server_Event_Callback() function).                  */
void BTPSAPI TIPM_Un_Register_Server_Event_Callback(unsigned int ServerCallbackID)
{
   TIPM_Event_Callback_Info_t *EventCallbackPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Time Manager has been initialized.  */
   if(Initialized)
   {
      /* Check to see if the input parameters appear to be semi-valid.  */
      if(ServerCallbackID)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Verify that the owner of this callback is the person who */
            /* is doing the un-registering.                             */
            if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, ServerCallbackID)) != NULL)
            {
               /* Verify that this is owned by the Server Process.      */
               if(EventCallbackPtr->ClientID == MSG_GetServerAddressID())
               {
                  /* Delete the callback back from the list.            */
                  if((EventCallbackPtr = DeleteEventCallbackInfoEntry(&EventCallbackInfoList, ServerCallbackID)) != NULL)
                  {
                     /* Free the memory because we are finished with it.*/
                     FreeEventCallbackInfoEntryMemory(EventCallbackPtr);
                  }
               }
            }

            /* Release the Lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit:\n"));
}

   /* The following function is a utility function that is used to set  */
   /* the current Local Time Information.  This function accepts the    */
   /* Server Callback ID (return value from                             */
   /* TIPM_Register_Server_Event_Callback() function) and a pointer to  */
   /* the Local Time Information to set.  This function returns ZERO if */
   /* successful, or a negative return error code if there was an error.*/
int BTPSAPI TIPM_Set_Local_Time_Information(unsigned int ServerCallbackID, TIPM_Local_Time_Information_Data_t *LocalTimeInformation)
{
   int                         ret_val;
   TIPM_Event_Callback_Info_t *EventCallbackPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Time Manager has been initialized.  */
   if(Initialized)
   {
      /* Check to see if the input parameters appear to be semi-valid.  */
      if((ServerCallbackID) && (LocalTimeInformation))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Search for the Specified Callback entry.                 */
            if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, ServerCallbackID)) != NULL)
            {
               /* Verify that this is owned by the Server Process.      */
               if(EventCallbackPtr->ClientID == MSG_GetServerAddressID())
               {
                  /* Simply call the internal function to handle        */
                  /* actually setting the local information.            */
                  ret_val = ProcessSetLocalTimeInformation(LocalTimeInformation);
               }
               else
                  ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

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
      ret_val = BTPM_ERROR_CODE_TIME_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is a utility function that is used to force*/
   /* an update of the Current Time.  This function accepts the Server  */
   /* Callback ID (return value from                                    */
   /* TIPM_Register_Server_Event_Callback() function) and a bit mask    */
   /* that contains the reason for the Current Time Update.  This       */
   /* function returns ZERO if successful, or a negative return error   */
   /* code if there was an error.                                       */
int BTPSAPI TIPM_Update_Current_Time(unsigned int ServerCallbackID, unsigned long AdjustReasonMask)
{
   int                         ret_val;
   TIPM_Event_Callback_Info_t *EventCallbackPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Time Manager has been initialized.  */
   if(Initialized)
   {
      /* Check to see if the input parameters appear to be semi-valid.  */
      if(ServerCallbackID)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Search for the Specified Callback entry.                 */
            if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, ServerCallbackID)) != NULL)
            {
               /* Verify that this is owned by the Server Process.      */
               if(EventCallbackPtr->ClientID == MSG_GetServerAddressID())
               {
                  /* Make sure that at least 1 bit is set in the Adjust */
                  /* Reason Mask.                                       */
                  if(!AdjustReasonMask)
                     AdjustReasonMask = TIPM_CURRENT_TIME_ADJUST_REASON_FLAGS_MANUAL_TIME_UPDATE;

                  /* Make sure that no invalid bits are set.            */
                  AdjustReasonMask &= ~((unsigned long)(TIPM_CURRENT_TIME_ADJUST_REASON_FLAGS_MANUAL_TIME_UPDATE | TIPM_CURRENT_TIME_ADJUST_REASON_FLAGS_EXTERNAL_REFERENCE_TIME_UPDATE | TIPM_CURRENT_TIME_ADJUST_REASON_FLAGS_CHANGE_OF_TIMEZONE | TIPM_CURRENT_TIME_ADJUST_REASON_FLAGS_CHANGE_OF_DST));

                  /* Simply call the internal function to handle        */
                  /* actually updating the current time.                */
                  ret_val = ProcessUpdateCurrentTime(0, AdjustReasonMask);
               }
               else
                  ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

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
      ret_val = BTPM_ERROR_CODE_TIME_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is a utility function that is used to      */
   /* respond to a request for the Reference Time Information.  This    */
   /* function accepts the Server Callback ID (return value from        */
   /* TIPM_Register_Server_Event_Callback() function) and a pointer to  */
   /* the Reference Time Information to respond to the request with.    */
   /* This function returns ZERO if successful, or a negative return    */
   /* error code if there was an error.                                 */
int BTPSAPI TIPM_Reference_Time_Response(unsigned int ServerCallbackID, TIPM_Reference_Time_Information_Data_t *ReferenceTimeInformation)
{
   int                         ret_val;
   TIPM_Event_Callback_Info_t *EventCallbackPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Time Manager has been initialized.  */
   if(Initialized)
   {
      /* Check to see if the input parameters appear to be semi-valid.  */
      if((ServerCallbackID) && (ReferenceTimeInformation))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Search for the Specified Callback entry.                 */
            if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, ServerCallbackID)) != NULL)
            {
               /* Verify that this is owned by the Server Process.      */
               if(EventCallbackPtr->ClientID == MSG_GetServerAddressID())
               {
                  /* Simply call the internal function to handle        */
                  /* actually responding to the previous request for the*/
                  /* Reference Time Information.                        */
                  ret_val = ProcessReferenceTimeInformationResponse(ReferenceTimeInformation);
               }
               else
                  ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

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
      ret_val = BTPM_ERROR_CODE_TIME_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a Client callback function with the Time (TIP)*/
   /* Manager Service.  This Callback will be dispatched by the TIP     */
   /* Manager when various TIP Manager Client Events occur.  This       */
   /* function accepts the Callback Function and Callback Parameter     */
   /* (respectively) to call when a TIP Manager Client Event needs to be*/
   /* dispatched.  This function returns a positive (non-zero) value if */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          TIPM_Un_Register_Client_Event_Callback() function to     */
   /*          un-register the callback from this module.               */
int BTPSAPI TIPM_Register_Client_Event_Callback(TIPM_Event_Callback_t CallbackFunction, void *CallbackParameter)
{
   int                        ret_val;
   TIPM_Event_Callback_Info_t EventCallbackEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Time Manager has been initialized.  */
   if(Initialized)
   {
      /* Check to see if the input parameters appear to be semi-valid.  */
      if(CallbackFunction)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Attempt to add an entry into the Event Callback Entry    */
            /* list.                                                    */
            BTPS_MemInitialize(&EventCallbackEntry, 0, sizeof(TIPM_Event_Callback_Info_t));

            EventCallbackEntry.EventCallbackID   = GetNextEventCallbackID();
            EventCallbackEntry.ClientID          = MSG_GetServerAddressID();
            EventCallbackEntry.EventCallback     = CallbackFunction;
            EventCallbackEntry.CallbackParameter = CallbackParameter;

            if(AddEventCallbackInfoEntry(&ClientEventCallbackInfoList, &EventCallbackEntry))
               ret_val = EventCallbackEntry.EventCallbackID;
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
      ret_val = BTPM_ERROR_CODE_TIME_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered TIP Manager Client Event      */
   /* Callback (registered via a successful call to the                 */
   /* TIPM_Register_Client_Event_Callback() function).  This function   */
   /* accepts as input the Client Event Callback ID (return value from  */
   /* TIPM_Register_Client_Event_Callback() function).                  */
void BTPSAPI TIPM_Un_Register_Client_Event_Callback(unsigned int ClientCallbackID)
{
   TIPM_Event_Callback_Info_t *EventCallbackPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Time Manager has been initialized.  */
   if(Initialized)
   {
      /* Check to see if the input parameters appear to be semi-valid.  */
      if(ClientCallbackID)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Verify that the owner of this callback is the person who */
            /* is doing the un-registering.                             */
            if((EventCallbackPtr = SearchEventCallbackInfoEntry(&ClientEventCallbackInfoList, ClientCallbackID)) != NULL)
            {
               /* Verify that this is owned by the Server Process.      */
               if(EventCallbackPtr->ClientID == MSG_GetServerAddressID())
               {
                  /* Delete the callback back from the list.            */
                  if((EventCallbackPtr = DeleteEventCallbackInfoEntry(&EventCallbackInfoList, ClientCallbackID)) != NULL)
                  {
                     /* Free the memory because we are finished with it.*/
                     FreeEventCallbackInfoEntryMemory(EventCallbackPtr);
                  }
               }
            }

            /* Release the Lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit:\n"));
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to get the current time from a remote TIP server.  The    */
   /* first parameter is the CallbackID returned from a successful call */
   /* to TIPM_Register_Client_Events.  The second parameter is the      */
   /* Bluetooth Address of the remote TIP Server.  This function returns*/
   /* a positive number representing the Transaction ID of this request */
   /* if successful and a negative return error code if there was an    */
   /* error.                                                            */
   /* * NOTE * A successul return from this function does not mean the  */
   /*          request was successfully completed. The result of the    */
   /*          request will be returned in a aetGetCurrentTimeResponse  */
   /*          event.                                                   */
int BTPSAPI TIPM_Get_Current_Time(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress)
{
   int                         ret_val;
   TIPM_Event_Callback_Info_t *EventCallbackPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Time Manager has been initialized.  */
   if(Initialized)
   {
      /* Check to see if the input parameters appear to be semi-valid.  */
      if((ClientCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Search for the Specified Callback entry.                 */
            if((EventCallbackPtr = SearchEventCallbackInfoEntry(&ClientEventCallbackInfoList, ClientCallbackID)) != NULL)
            {
               /* Verify that this is owned by the Server Process.      */
               if(EventCallbackPtr->ClientID == MSG_GetServerAddressID())
               {
                  /* Simply call the internal function to handle        */
                  /* actually send the get current time request.        */
                  ret_val = ProcessGetCurrentTime(ClientCallbackID, RemoteDeviceAddress);
               }
               else
                  ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

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
      ret_val = BTPM_ERROR_CODE_TIME_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to enable current time notifications from a remote TIP    */
   /* Server.  The first parameter is the CallbackID returned from      */
   /* a successful call to TIPM_Register_Client_Events.  The second     */
   /* parameter is the Bluetooth Address of the remote TIP Server.  This*/
   /* function returns zero if successful and a negative return error   */
   /* code if there is an error.                                        */
int BTPSAPI TIPM_Enable_Time_Notifications(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress, Boolean_t Enable)
{
   int                         ret_val;
   TIPM_Event_Callback_Info_t *EventCallbackPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Time Manager has been initialized.  */
   if(Initialized)
   {
      /* Check to see if the input parameters appear to be semi-valid.  */
      if((ClientCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Search for the Specified Callback entry.                 */
            if((EventCallbackPtr = SearchEventCallbackInfoEntry(&ClientEventCallbackInfoList, ClientCallbackID)) != NULL)
            {
               /* Verify that this is owned by the Server Process.      */
               if(EventCallbackPtr->ClientID == MSG_GetServerAddressID())
               {
                  /* Simply call the internal function to handle        */
                  /* actually enable time notifications.                */
                  ret_val = ProcessEnableTimeNotifications(ClientCallbackID, RemoteDeviceAddress, Enable);
               }
               else
                  ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

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
      ret_val = BTPM_ERROR_CODE_TIME_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to get the local time information from a remote TIP       */
   /* server.  The first parameter is the CallbackID returned from      */
   /* a successful call to TIPM_Register_Client_Events.  The second     */
   /* parameter is the Bluetooth Address of the remote TIP Server.      */
   /* * NOTE * A successul return from this function does not mean the  */
   /*          request was successfully completed. The                  */
   /*          result of the request will be returned in a              */
   /*          aetLocalTimeInformationResponse event.                   */
   /* * NOTE * Not all TIP Servers support the capability. The caller   */
   /*          should check the SupportedServicesMask returned          */
   /*          by either an aetTIPConnected event or a call to          */
   /*          TIPM_Quert_Connected_Devices() to determine if this is   */
   /*          supported.                                               */
int BTPSAPI TIPM_Get_Local_Time_Information(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress)
{
   int                         ret_val;
   TIPM_Event_Callback_Info_t *EventCallbackPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Time Manager has been initialized.  */
   if(Initialized)
   {
      /* Check to see if the input parameters appear to be semi-valid.  */
      if((ClientCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Search for the Specified Callback entry.                 */
            if((EventCallbackPtr = SearchEventCallbackInfoEntry(&ClientEventCallbackInfoList, ClientCallbackID)) != NULL)
            {
               /* Verify that this is owned by the Server Process.      */
               if(EventCallbackPtr->ClientID == MSG_GetServerAddressID())
               {
                  /* Simply call the internal function to handle        */
                  /* actually get the local time information.           */
                  ret_val = ProcessGetLocalTimeInformation(ClientCallbackID, RemoteDeviceAddress);
               }
               else
                  ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

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
      ret_val = BTPM_ERROR_CODE_TIME_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to get the time accuracy and the reference time           */
   /* information of a remote TIP Server.  The first parameter          */
   /* is the CallbackID returned from a successful call to              */
   /* TIPM_Register_Client_Events.  The second parameter is the         */
   /* Bluetooth Address of the remote TIP Server.  This function returns*/
   /* a positive number representing the Transaction ID of this request */
   /* if successful and a negative return error code if there was an    */
   /* error.                                                            */
   /* * NOTE * A successul return from this function does not mean the  */
   /*          request was successfully completed. The result of the    */
   /*          request will be returned in a aetTimeAccuracyResponse    */
   /*          event.                                                   */
   /* * NOTE * Not all TIP Servers support the capability. The caller   */
   /*          should check the SupportedServicesMask returned          */
   /*          by either an aetTIPConnected event or a call to          */
   /*          TIPM_Quert_Connected_Devices() to determine if this is   */
   /*          supported.                                               */
int BTPSAPI TIPM_Get_Time_Accuracy(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress)
{
   int                         ret_val;
   TIPM_Event_Callback_Info_t *EventCallbackPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Time Manager has been initialized.  */
   if(Initialized)
   {
      /* Check to see if the input parameters appear to be semi-valid.  */
      if((ClientCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Search for the Specified Callback entry.                 */
            if((EventCallbackPtr = SearchEventCallbackInfoEntry(&ClientEventCallbackInfoList, ClientCallbackID)) != NULL)
            {
               /* Verify that this is owned by the Server Process.      */
               if(EventCallbackPtr->ClientID == MSG_GetServerAddressID())
               {
                  /* Simply call the internal function to handle        */
                  /* actually get the time accuracy.                    */
                  ret_val = ProcessGetTimeAccuracy(ClientCallbackID, RemoteDeviceAddress);
               }
               else
                  ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

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
      ret_val = BTPM_ERROR_CODE_TIME_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to get the next Daylight Savings Time change information  */
   /* from a remote TIP Server.  The first parameter is the CallbackID  */
   /* returned from a successful call to TIPM_Register_Client_Events.   */
   /* The second parameter is the Bluetooth Address of the remote TIP   */
   /* Server.  This function returns a positive number representing the */
   /* Transaction ID of this request if successful and a negative return*/
   /* error code if there was an error.                                 */
   /* * NOTE * A successul return from this function does not mean the  */
   /*          request was successfully completed. The result of the    */
   /*          request will be returned in a aetNextDSTChangeResponse   */
   /*          event.                                                   */
   /* * NOTE * Not all TIP Servers support the capability. The caller   */
   /*          should check the SupportedServicesMask returned          */
   /*          by either an aetTIPConnected event or a call to          */
   /*          TIPM_Quert_Connected_Devices() to determine if this is   */
   /*          supported.                                               */
int BTPSAPI TIPM_Get_Next_DST_Change_Information(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress)
{
   int                         ret_val;
   TIPM_Event_Callback_Info_t *EventCallbackPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Time Manager has been initialized.  */
   if(Initialized)
   {
      /* Check to see if the input parameters appear to be semi-valid.  */
      if((ClientCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Search for the Specified Callback entry.                 */
            if((EventCallbackPtr = SearchEventCallbackInfoEntry(&ClientEventCallbackInfoList, ClientCallbackID)) != NULL)
            {
               /* Verify that this is owned by the Server Process.      */
               if(EventCallbackPtr->ClientID == MSG_GetServerAddressID())
               {
                  /* Simply call the internal function to handle        */
                  /* actually get the next DST change information.      */
                  ret_val = ProcessGetNextDSTChange(ClientCallbackID, RemoteDeviceAddress);
               }
               else
                  ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

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
      ret_val = BTPM_ERROR_CODE_TIME_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to get the time update state of a remote TIP Server.  The */
   /* first parameter is the CallbackID returned from a successful call */
   /* to TIPM_Register_Client_Events.  The second parameter is the      */
   /* Bluetooth Address of the remote TIP Server.  This function returns*/
   /* a positive number representing the Transaction ID of this request */
   /* if successful and a negative return error code if there was an    */
   /* error.                                                            */
   /* * NOTE * A successul return from this function does not mean the  */
   /*          request was successfully completed. The result of the    */
   /*          request will be returned in a aetTimeUpdateStateResponse */
   /*          event.                                                   */
   /* * NOTE * Not all TIP Servers support the capability. The caller   */
   /*          should check the SupportedServicesMask returned          */
   /*          by either an aetTIPConnected event or a call to          */
   /*          TIPM_Quert_Connected_Devices() to determine if this is   */
   /*          supported.                                               */
int BTPSAPI TIPM_Get_Reference_Time_Update_State(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress)
{
   int                         ret_val;
   TIPM_Event_Callback_Info_t *EventCallbackPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Time Manager has been initialized.  */
   if(Initialized)
   {
      /* Check to see if the input parameters appear to be semi-valid.  */
      if((ClientCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Search for the Specified Callback entry.                 */
            if((EventCallbackPtr = SearchEventCallbackInfoEntry(&ClientEventCallbackInfoList, ClientCallbackID)) != NULL)
            {
               /* Verify that this is owned by the Server Process.      */
               if(EventCallbackPtr->ClientID == MSG_GetServerAddressID())
               {
                  /* Simply call the internal function to handle        */
                  /* actually get the reference time update state.      */
                  ret_val = ProcessGetReferenceTimeUpdateState(ClientCallbackID, RemoteDeviceAddress);
               }
               else
                  ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

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
      ret_val = BTPM_ERROR_CODE_TIME_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to request a reference time update on a remote TIP Server */
   /* The first parameter is the CallbackID returned from a successful  */
   /* call to TIPM_Register_Client_Events.  The second parameter is the */
   /* Bluetooth Address of the remote TIP Server.  This function returns*/
   /* zero if successful and a negative return error code if there is an*/
   /* error.                                                            */
   /* * NOTE * Not all TIP Servers support the capability. The caller   */
   /*          should check the SupportedServicesMask returned          */
   /*          by either an aetTIPConnected event or a call to          */
   /*          TIPM_Quert_Connected_Devices() to determine if this is   */
   /*          supported.                                               */
int BTPSAPI TIPM_Request_Reference_Time_Update(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress)
{
   int                         ret_val;
   TIPM_Event_Callback_Info_t *EventCallbackPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Time Manager has been initialized.  */
   if(Initialized)
   {
      /* Check to see if the input parameters appear to be semi-valid.  */
      if((ClientCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Search for the Specified Callback entry.                 */
            if((EventCallbackPtr = SearchEventCallbackInfoEntry(&ClientEventCallbackInfoList, ClientCallbackID)) != NULL)
            {
               /* Verify that this is owned by the Server Process.      */
               if(EventCallbackPtr->ClientID == MSG_GetServerAddressID())
               {
                  /* Simply call the internal function to handle        */
                  /* actually request a reference time update.          */
                  ret_val = ProcessRequestReferenceTimeUpdate(ClientCallbackID, RemoteDeviceAddress);
               }
               else
                  ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

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
      ret_val = BTPM_ERROR_CODE_TIME_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to determine if there are currently any connected Time    */
   /* Profile devices.  This function accepts the buffer information    */
   /* to receive any currently connected devices.  The first parameter  */
   /* specifies the maximum number of BD_ADDR entries that the buffer   */
   /* will support (i.e. can be copied into the buffer).  The next      */
   /* parameter is optional and, if specified, will be populated        */
   /* with the total number of connected devices if the function is     */
   /* successful.  The final parameter can be used to retrieve the total*/
   /* number of connected devices (regardless of the size of the list   */
   /* specified by the first two parameters).  This function returns    */
   /* a non-negative value if successful which represents the number    */
   /* of connected devices that were copied into the specified input    */
   /* buffer.  This function returns a negative return error code if    */
   /* there was an error.                                               */
   /* * NOTE * It is possible to specify zero for the Maximum Number of */
   /*          Remote Device Entries, in which case the final parameter */
   /*          *MUST* be specified.                                     */
int BTPSAPI TIPM_Query_Connected_Devices(TIPM_Connection_Type_t ConnectionType, unsigned int MaximumRemoteDeviceListEntries, TIPM_Remote_Device_t *RemoteDeviceList, unsigned int *TotalNumberConnectedDevices)
{
   int                         ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Time Manager has been initialized.  */
   if(Initialized)
   {
      /* Check to see if the input parameters appear to be semi-valid.  */
      if(((MaximumRemoteDeviceListEntries) && (RemoteDeviceList)) || (TotalNumberConnectedDevices))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Simply call the internal function to handle actually     */
            /* query the connected devices.                             */
            ret_val = ProcessQueryConnectedDevices(ConnectionType, MaximumRemoteDeviceListEntries, RemoteDeviceList, TotalNumberConnectedDevices);

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
      ret_val = BTPM_ERROR_CODE_TIME_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_TIME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

