/*****< btpmanpm.c >***********************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMANPM - ANP Manager for Stonestreet One Bluetooth Protocol Stack       */
/*             Platform Manager.                                              */
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

#include "BTPMANPM.h"            /* BTPM ANP Manager Prototypes/Constants.    */
#include "ANPMAPI.h"             /* ANP Manager Prototypes/Constants.         */
#include "ANPMMSG.h"             /* BTPM ANP Manager Message Formats.         */
#include "ANPMGR.h"              /* ANP Manager Impl. Prototypes/Constants.   */

#include "SS1BTPM.h"             /* BTPM Main Prototypes and Constants.       */
#include "BTPMERR.h"             /* BTPM Error Prototypes/Constants.          */
#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

   /* The following defines the ANPM LE Configuration File Section Name.*/
#define ANPM_LE_CONFIGURATION_FILE_SECTION_NAME                   "ANPM"

   /* The following define the Key Names that are used with the ANPM    */
   /* Configuration File.                                               */
#define ANPM_KEY_NAME_CCCD_PREFIX                                 "ANPM_%02X%02X%02X%02X%02X%02X"
#define ANPM_KEY_NAME_PERSISTENT_UID                              "PU"

   /* The following defines the size of a Persistent UID that is stored */
   /* in the configuration file.                                        */
#define ANPM_PERSISTENT_UID_SIZE                                  (NON_ALIGNED_DWORD_SIZE + (NON_ALIGNED_WORD_SIZE*2))

   /* Structure which is used to track Callback Information related to  */
   /* this module.                                                      */
typedef struct _tagANPM_Event_Callback_Info_t
{
   unsigned int                           EventCallbackID;
   unsigned int                           ClientID;
   ANPM_Event_Callback_t                  EventCallback;
   void                                  *CallbackParameter;
   struct _tagANPM_Event_Callback_Info_t *NextANPMEventCallbackInfoPtr;
} ANPM_Event_Callback_Info_t;

   /* Container structure used when dispatching registered callbacks.   */
typedef struct _tagCallback_Info_t
{
   unsigned int           ClientID;
   ANPM_Event_Callback_t  EventCallback;
   void                  *CallbackParameter;
} Callback_Info_t;

   /* Structure which is used to hold all of the binary information that*/
   /* is stored to file for a paired device.                            */
typedef struct _tagConnection_Binary_Entry_t
{
   Byte_t NewAlertCCCD;
   Byte_t UnReadAlertCCCD;
} Connection_Binary_Entry_t;

   /* Structure which is used to track information pertaining to        */
   /* incoming connection requests.                                     */
typedef struct _tagConnection_Entry_t
{
   BD_ADDR_t                      BD_ADDR;
   unsigned int                   ConnectionID;
   Boolean_t                      ClientConnectedDispatched;
   Boolean_t                      New_Alert_Client_Configuration;
   Boolean_t                      Unread_Alert_Status_Client_Configuration;
   Word_t                         Enabled_New_Alert_Categories;
   Word_t                         Enabled_Unread_Alert_Categories;
   struct _tagConnection_Entry_t *NextConnectionEntryPtr;
} Connection_Entry_t;

   /* The following structure is used to track information related to   */
   /* New and Un-Read Alert Counts.                                     */
typedef struct _tagAlert_Entry_t
{
   unsigned int                    ClientID;
   ANPM_Category_Identification_t  Category;
   unsigned int                    Count;
   char                           *LastAlertText;
   struct _tagAlert_Entry_t       *NextAlertEntryPtr;
} Alert_Entry_t;

#define ALERT_ENTRY_DATA_SIZE(_x)      ((sizeof(Alert_Entry_t)) + ((unsigned int)(_x)))

   /* Structure which tracks clients which have enabled notifications   */
   /* for the a connection.                                             */
typedef struct _tagNotification_Entry_t
{
   unsigned int                     CallbackID;
   struct _tagNotification_Entry_t *NextNotificationEntryPtr;
} Notification_Entry_t;

   /* Structure which tracks clients which have enabled specific        */
   /* categories for a connection.                                      */
typedef struct _tagCategory_Entry_t
{
   unsigned int                    CallbackID;
   unsigned long                   CategoryMask;
   struct _tagCategory_Entry_t    *NextCategoryEntryPtr;
} Category_Entry_t;

   /* Structure which tracks Client Connections.                        */
typedef struct _tagClient_Connection_Entry_t
{
   BD_ADDR_t                             BD_ADDR;
   unsigned long                         Flags;
   ANS_Client_Information_t              ClientInformation;
   Notification_Entry_t                 *NewAlertNotificationList;
   Notification_Entry_t                 *UnreadStatusNotificationList;
   Category_Entry_t                     *NewAlertCategoryList;
   Category_Entry_t                     *UnreadStatusCategoryList;
   unsigned long                         EnabledNewAlertCategoryMask;
   unsigned long                         EnabledUnreadStatusCategoryMask;
   struct _tagClient_Connection_Entry_t *NextClientConnectionEntryPtr;
} Client_Connection_Entry_t;

#define CLIENT_CONNECTION_FLAGS_NEW_ALERT_NOTIFICATIONS_ENABLED   0x00000001
#define CLIENT_CONNECTION_FLAGS_UNREAD_NOTIFICATIONS_ENABLED      0x00000002

   /* Defines the types of transactions that could be outstanding.      */
typedef enum
{
   ttReadSupportedNewAlertCategories,
   ttReadSupportedUnreadStatusCategories,
   ttEnableNewAlertCCCD,
   ttDisableNewAlertCCCD,
   ttEnableUnreadStatusCCCD,
   ttDisableUnreadStatusCCCD,
   ttWriteControlPoint
} TransactionType_t;

   /* Defines the types of control commands that could be outstanding.  */
typedef enum
{
   cpoEnableNewAlertCategory,
   cpoEnableUnreadStatusCategory,
   cpoDisableNewAlertCategory,
   cpoDisableUnreadStatusCategory,
   cpoRequestNewAlert,
   cpoRequestUnreadStatus
} ControlPointOperation_t;

   /* Structure which tracks outstanding GATT transactions.             */
typedef struct _tagTransaction_Entry_t
{
   unsigned int                    TransactionID;
   unsigned int                    GATMTransactionID;
   unsigned long                   Flags;
   TransactionType_t               TransactionType;
   BD_ADDR_t                       BD_ADDR;
   unsigned int                    SingleCallbackID;
   Notification_Entry_t           *PendingResultCallbacks;
   ControlPointOperation_t         ControlPointOperation;
   ANPM_Category_Identification_t  PendingCategoryID;
   struct _tagTransaction_Entry_t *NextTransactionEntryPtr;
} Transaction_Entry_t;

#define TRANSACTION_ENTRY_FLAGS_SINGLE_CALLBACK                0x00000001

   /* Enumerated type which represents the identifiers for the          */
   /* characteristics in the ANS service.                               */
typedef enum
{
   auSupportedNewAlertCategory,
   auNewAlert,
   auSupportedUnreadAlertCategory,
   auUnreadAlertStatus,
   auControlPoint,
   auUnknown
} ANS_UUID_t;

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the compiler*/
   /* as part of standard C/C++).                                       */

   /* Variable which serves as a global flag about whether this module  */
   /* is initialized or not.                                            */
static Boolean_t Initialized;

   /* Variable which is used to hold the next (unique) Callback ID.     */
static unsigned int NextEventCallbackID;
static unsigned int NextTransactionID;

   /* Variable which holds the current Power State of the Device.       */
static Boolean_t CurrentPowerState;

   /* Variable which holds a pointer to the first element in the Generic*/
   /* Attribute Profile Callback Info List (which holds all ANPM Event  */
   /* Callbacks registered with this module).                           */
static ANPM_Event_Callback_Info_t *EventCallbackInfoList;
static ANPM_Event_Callback_Info_t *ClientEventCallbackInfoList;

   /* Variable which holds the supported New Alert Categories.          */
static Word_t SupportedNewAlertCategories;

   /* Variable which holds the supported Un-Read Alert Categories.      */
static Word_t SupportedUnReadAlertCategories;

   /* Variable which holds a pointer to the first element in the Alert  */
   /* List (which holds all currently active alerts).                   */
static Alert_Entry_t *NewAlertEntryList;
static Alert_Entry_t *UnReadAlertEntryList;

   /* Variable which holds a pointer to the first element of the        */
   /* Connection Information List (which holds all currently active     */
   /* connections).                                                     */
static Connection_Entry_t *ConnectionEntryList;

   /* Variable which holds a pointer to the first element in the Client */
   /* connection list.                                                  */
static Client_Connection_Entry_t *ClientConnectionEntryList;

   /* Variable which holds a pointer to the first element in the GATT   */
   /* Transaction List.                                                 */
static Transaction_Entry_t *TransactionEntryList;

   /* Variable which holds the ID for the registered GATM Callback ID.  */
static unsigned int GATMCallbackID;

   /* Internal Function Prototypes.                                     */
static unsigned int GetNextEventCallbackID(void);
static unsigned int GetNextTransactionID(void);

static ANPM_Event_Callback_Info_t *AddEventCallbackInfoEntry(ANPM_Event_Callback_Info_t **ListHead, ANPM_Event_Callback_Info_t *EntryToAdd);
static ANPM_Event_Callback_Info_t *SearchEventCallbackInfoEntry(ANPM_Event_Callback_Info_t **ListHead, unsigned int EventCallbackID);
static ANPM_Event_Callback_Info_t *SearchEventCallbackInfoEntryByClientID(ANPM_Event_Callback_Info_t **ListHead, unsigned int ClientID);
static ANPM_Event_Callback_Info_t *DeleteEventCallbackInfoEntry(ANPM_Event_Callback_Info_t **ListHead, unsigned int EventCallbackID);
static ANPM_Event_Callback_Info_t *DeleteEventCallbackInfoEntryByClientID(ANPM_Event_Callback_Info_t **ListHead, unsigned int ClientID);
static void FreeEventCallbackInfoEntryMemory(ANPM_Event_Callback_Info_t *EntryToFree);
static void FreeEventCallbackInfoList(ANPM_Event_Callback_Info_t **ListHead);

static Connection_Entry_t *AddConnectionEntry(Connection_Entry_t **ListHead, Connection_Entry_t *EntryToAdd);
static Connection_Entry_t *SearchConnectionEntry(Connection_Entry_t **ListHead, unsigned int ConnectionID);
static Connection_Entry_t *DeleteConnectionEntry(Connection_Entry_t **ListHead, unsigned int ConnectionID);
static Connection_Entry_t *DeleteConnectionEntryByBD_ADDR(Connection_Entry_t **ListHead, BD_ADDR_t BD_ADDR);
static void FreeConnectionEntryMemory(Connection_Entry_t *EntryToFree);
static void FreeConnectionEntryList(Connection_Entry_t **ListHead);

static Alert_Entry_t *AddAlertEntry(Alert_Entry_t **ListHead, Alert_Entry_t *EntryToAdd, unsigned int StringLength, char *String);
static Alert_Entry_t *DeleteAlertEntry(Alert_Entry_t **ListHead, unsigned int ClientID, ANPM_Category_Identification_t Category);
static void FreeAlertEntryMemory(Alert_Entry_t *EntryToFree);
static void FreeAlertEntryList(Alert_Entry_t **ListHead);

static Client_Connection_Entry_t *AddClientConnectionEntry(Client_Connection_Entry_t **ListHead, Client_Connection_Entry_t *EntryToAdd);
static Client_Connection_Entry_t *SearchClientConnectionEntry(Client_Connection_Entry_t **ListHead, BD_ADDR_t BD_ADDR);
static Client_Connection_Entry_t *DeleteClientConnectionEntry(Client_Connection_Entry_t **ListHead, BD_ADDR_t BD_ADDR);
static void FreeClientConnectionEntryMemory(Client_Connection_Entry_t *EntryToFree);
static void FreeClientConnectionEntryList(Client_Connection_Entry_t **ListHead);
static void CleanupUnregisteredClient(Client_Connection_Entry_t **ListHead, unsigned int CallbackID);

static Notification_Entry_t *AddNotificationEntry(Notification_Entry_t **ListHead, Notification_Entry_t *EntryToAdd);
static Notification_Entry_t *DeleteNotificationEntry(Notification_Entry_t **ListHead, unsigned int CallbackID);
static void FreeNotificationEntryMemory(Notification_Entry_t *EntryToFree);
static void FreeNotificationEntryList(Notification_Entry_t **ListHead);

static Category_Entry_t *SearchAddCategoryEntry(Category_Entry_t **ListHead, Category_Entry_t *EntryToSearchAdd);
static Category_Entry_t *SearchCategoryEntry(Category_Entry_t **ListHead, unsigned int CallbackID);
static Category_Entry_t *DeleteCategoryEntry(Category_Entry_t **ListHead, unsigned int CallbackID);
static void FreeCategoryEntryMemory(Category_Entry_t *EntryToFree);
static void FreeCategoryEntryList(Category_Entry_t **ListHead);
static Boolean_t IsCategoryEnabledForClient(Category_Entry_t **ListHead, ANPM_Category_Identification_t Category, unsigned int CallbackID);

static Transaction_Entry_t *AddTransactionEntry(Transaction_Entry_t **ListHead, Transaction_Entry_t *EntryToAdd);
static Transaction_Entry_t *SearchTransactionEntryByType(Transaction_Entry_t **ListHead, BD_ADDR_t BD_ADDR, TransactionType_t TransactionType);
static Transaction_Entry_t *SearchTransactionEntryByPendingCategory(Transaction_Entry_t **ListHead, BD_ADDR_t BD_ADDR, ANPM_Category_Identification_t Category, ANPM_Notification_Type_t Type, Boolean_t Enabling);
static Transaction_Entry_t *DeleteTransactionEntry(Transaction_Entry_t **ListHead, unsigned int TransactionID);
static Transaction_Entry_t *DeleteTransactionEntryByGATMID(Transaction_Entry_t **ListHead, unsigned int GATMTransactionID);
static void FreeTransactionEntryMemory(Transaction_Entry_t *EntryToFree);
static void FreeTransactionEntryList(Transaction_Entry_t **ListHead);

static void DispatchANPMEvent(Boolean_t Server, ANPM_Event_Data_t *ANPMEventData, BTPM_Message_t *Message);
static void DispatchANPMNotificationEvent(Notification_Entry_t *NotificationClientList, Category_Entry_t *CategoryList, ANPM_Event_Data_t *ANPMEventData, BTPM_Message_t *Message, ANPM_Category_Identification_t Category);
static void DispatchANPMEventByID(unsigned int CallbackID, ANPM_Event_Data_t *ANPMEventData, BTPM_Message_t *Message);

static void StoreConnectionConfiguration(Connection_Entry_t *ConnectionEntry, Boolean_t Store);
static void ReloadConnectionConfiguration(Connection_Entry_t *ConnectionEntry);

static void StorePersistentUID(DWord_t PersistentUID, GATT_Attribute_Handle_Group_t *ServiceHandleRange, Boolean_t Store);
static Boolean_t ReloadPersistentUID(DWord_t *PersistentUID, GATT_Attribute_Handle_Group_t *ServiceHandleRange);

static Boolean_t CalculateServiceHandleRange(GATT_Attribute_Handle_Group_t *ServiceHandleRange);

static unsigned int CategoryAlertCount(Boolean_t NewAlerts, ANPM_Category_Identification_t Category, char **LastAlertText);

static void CleanupOldAlerts(ANPM_Category_Identification_t Category, Boolean_t NewAlerts, unsigned int NumberToClean);

static void NotifyCategory(Connection_Entry_t *SpecificConnectionEntry, Boolean_t NewAlerts, ANPM_Category_Identification_t Category);
int ConfigureAlert(unsigned int ClientID, Boolean_t NewAlert, ANPM_Category_Identification_t CategoryID, unsigned int AlertCount, char *LastAlertText);

static void DispatchANPClientConnectionEvent(Connection_Entry_t *ConnectionEntry);
static void DispatchANPClientDisconnectionEvent(Connection_Entry_t *ConnectionEntry);
static void DispatchANPNewAlertCategoryEvent(Connection_Entry_t *ConnectionEntry, Boolean_t CategoryEnabled, ANPM_Category_Identification_t Category, Word_t CategoryMask);
static void DispatchANPUnReadAlertCategoryEvent(Connection_Entry_t *ConnectionEntry, Boolean_t CategoryEnabled, ANPM_Category_Identification_t Category, Word_t CategoryMask);

static void DispatchANPServerConnectionEvent(Client_Connection_Entry_t *ConnectionEntry);
static void DispatchANPServerDisconnectionEvent(Client_Connection_Entry_t *ConnectionEntry);
static void DispatchGetSupportedNewAlertCategoriesResultEvent(Client_Connection_Entry_t *ConnectionEntry, unsigned int CallbackID, unsigned int TransactionID, unsigned int Status, unsigned int AttProtocolErrorCode, unsigned long SupportedCategories);
static void DispatchGetSupportedUnreadStatusCategoriesResultEvent(Client_Connection_Entry_t *ConnectionEntry, unsigned int CallbackID, unsigned int TransactionID, unsigned int Status, unsigned int AttProtocolErrorCode, unsigned long SupportedCategories);
static void DispatchNewAlertNotificationEvent(Client_Connection_Entry_t *ConnectionEntry, ANPM_Category_Identification_t CategoryID, unsigned int NumberNewAlerts, char *LastAlertText);
static void DispatchUnreadStatusNotificationEvent(Client_Connection_Entry_t *ConnectionEntry, ANPM_Category_Identification_t CategoryID, unsigned int NumberUnreadAlerts);
static void DispatchCommandResultEvent(Client_Connection_Entry_t *ConnectionEntry, Transaction_Entry_t *TransactionEntry, unsigned int Status, unsigned int AttProtocolErrorCode);

static int ProcessRegisterClientEvents(unsigned int ClientID, ANPM_Event_Callback_t EventCallback, void *CallbackFunction);
static int ProcessUnRegisterClientEvents(unsigned int ClientID, unsigned int CallbackID);
static int ProcessGetSupportedCategories(unsigned int ClientID, unsigned int CallbackID, BD_ADDR_t RemoteDeviceAddress, ANPM_Notification_Type_t Type);
static int ProcessEnableNotifications(unsigned int ClientID, unsigned int CallbackID, BD_ADDR_t RemoteDeviceAddress, ANPM_Notification_Type_t Type);
static int ProcessDisableNotifications(unsigned int ClientID, unsigned int CallbackID, BD_ADDR_t RemoteDeviceAddress, ANPM_Notification_Type_t Type);
static int ProcessEnableCategory(unsigned int ClientID, unsigned int CallbackID, BD_ADDR_t RemoteDeviceAddress, ANPM_Category_Identification_t CategoryID, ANPM_Notification_Type_t Type);
static int ProcessDisableCategory(unsigned int ClientID, unsigned int CallbackID, BD_ADDR_t RemoteDeviceAddress, ANPM_Category_Identification_t CategoryID, ANPM_Notification_Type_t Type);
static int ProcessRequestNotification(unsigned int ClientID, unsigned int CallbackID, BD_ADDR_t RemoteDeviceAddress, ANPM_Category_Identification_t CategoryID, ANPM_Notification_Type_t Type);

static void ProcessSetNewAlertRequestMessage(ANPM_Set_New_Alert_Request_t *Message);
static void ProcessSetUnReadAlertRequestMessage(ANPM_Set_Un_Read_Alert_Request_t *Message);
static void ProcessRegisterANPEventsRequestMessage(ANPM_Register_ANP_Events_Request_t *Message);
static void ProcessUnRegisterANPEventsRequestMessage(ANPM_Un_Register_ANP_Events_Request_t *Message);

static void ProcessRegisterClientEventsMessage(ANPM_Register_ANP_Client_Events_Request_t *Message);
static void ProcessUnRegisterClientEventsMessage(ANPM_Un_Register_ANP_Client_Events_Request_t *Message);
static void ProcessGetSupportedCategoriesMessage(ANPM_Get_Supported_Categories_Request_t *Message);
static void ProcessEnableDisableNotificationsMessage(ANPM_Enable_Disable_Notifications_Request_t *Message);
static void ProcessEnableDisableCategoryMessage(ANPM_Enable_Disable_Category_Request_t *Message);
static void ProcessRequestNotificationMessage(ANPM_Request_Notification_Request_t *Message);

static void ProcessReceivedMessage(BTPM_Message_t *Message);
static void ProcessClientUnRegister(unsigned int ClientID);

static Boolean_t ConvertCategoryPMType(ANS_Category_Identification_t ANSCategory, ANPM_Category_Identification_t *PMCategoryResult);
static Boolean_t ConvertPMCategoryType(ANPM_Category_Identification_t ANPMCategory, ANS_Category_Identification_t *CategoryResult);
static Word_t CategoryToMask(ANPM_Category_Identification_t Category, Boolean_t NewAlertCategory, Boolean_t Server);

static void ProcessANSEnableCategoriesCommand(ANS_Control_Point_Command_Data_t *ControlPointCommand, Word_t SupportedCategories, Word_t *EnabledCategoriesResult);
static void ProcessANSDisableCategoriesCommand(ANS_Control_Point_Command_Data_t *ControlPointCommand, Word_t *EnabledCategoriesResult);
static void ProcessNotifyNewAlertsImmediatelyCommand(Connection_Entry_t *ConnectionEntry, ANS_Control_Point_Command_Data_t *ControlPointCommand);
static void ProcessNotifyUnReadAlertsImmediatelyCommand(Connection_Entry_t *ConnectionEntry, ANS_Control_Point_Command_Data_t *ControlPointCommand);
static void ProcessANSReadConfigurationRequest(ANS_Read_Client_Configuration_Data_t *ReadConfigData);
static void ProcessANSClientConfigurationUpdate(ANS_Client_Configuration_Update_Data_t *ClientConfigUpdateData);
static void ProcessANSControlPointCommand(ANS_Control_Point_Command_Data_t *ControlPointCommandData);

static void ProcessGATTConnectEvent(GATT_Device_Connection_Data_t *ConnectionData);
static void ProcessGATTDisconnectEvent(GATT_Device_Disconnection_Data_t *DisconnectionData);

static void ProcessANSEvent(ANPM_Server_Event_Data_t *ANSEventData);
static void ProcessGATTConnectionEvent(GATM_Connection_Event_Data_t *GATTConnectionEventData);

static ANS_UUID_t DetermineANSUUID(GATT_UUID_t UUID);
static Word_t FindCCCD(GATT_Characteristic_Information_t *CharacteristicInformation);
static Boolean_t DiscoverANS(BD_ADDR_t BD_ADDR, ANS_Client_Information_t *ClientInformation);

static void ProcessANSClientConnection(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties);
static void ProcessANSClientDisconnection(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties);

static void ProcessLowEnergyPairingChangeEvent(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties);
static void ProcessLowEnergyAddressChangeEvent(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties);

static void ProcessRemoteDevicePropertiesChangedEvent(unsigned long ChangedMemberMask, DEVM_Remote_Device_Properties_t *RemoteDeviceProperties);

static void ProcessSupportedNewAlertCategoriesResponse(GATM_Read_Response_Event_Data_t *EventData, Transaction_Entry_t *TransactionEntry);
static void ProcessSupportedUnreadAlertCategoriesResponse(GATM_Read_Response_Event_Data_t *EventData, Transaction_Entry_t *TransactionEntry);
static void ProcessControlPointWriteResponse(GATM_Write_Response_Event_Data_t *EventData, Transaction_Entry_t *TransactionEntry);
static void ProcessHandleValueNotification(GATM_Handle_Value_Data_Event_Data_t *EventData);
static void ProcessGATMErrorResponse(GATM_Error_Response_Event_Data_t *EventData, Transaction_Entry_t *TransactionEntry);

static void BTPSAPI GATM_EventCallback(GATM_Event_Data_t *GATMEventData, void *CallbackParameter);

static void BTPSAPI BTPMDispatchCallback_ANPM(void *CallbackParameter);
static void BTPSAPI BTPMDispatchCallback_ANS(void *CallbackParameter);
static void BTPSAPI BTPMDistpatchCallback_GATT(void *CallbackParameter);
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter);

static void BTPSAPI ANPManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter);

   /* The following function is a utility function that exists to       */
   /* retrieve a unique Callback ID that can be used to add an entry    */
   /* into the ANP Event Callback List.                                 */
static unsigned int GetNextEventCallbackID(void)
{
   ++NextEventCallbackID;

   if(NextEventCallbackID & 0x80000000)
      NextEventCallbackID = 1;

   return(NextEventCallbackID);
}

   /* The following function is a utility function that exists to       */
   /* retrieve a unique Transaction ID that can be used to add an entry */
   /* into the ANP Transaction List.                                    */
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
static ANPM_Event_Callback_Info_t *AddEventCallbackInfoEntry(ANPM_Event_Callback_Info_t **ListHead, ANPM_Event_Callback_Info_t *EntryToAdd)
{
   ANPM_Event_Callback_Info_t *AddedEntry = NULL;
   ANPM_Event_Callback_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First let's verify that values passed in are semi-valid.          */
   if((ListHead) && (EntryToAdd))
   {
      /* Make sure that the element that we are adding seems semi-valid.*/
      if(EntryToAdd->EventCallbackID)
      {
         /* OK, data seems semi-valid, let's allocate a new data        */
         /* structure to add to the list.                               */
         AddedEntry = (ANPM_Event_Callback_Info_t *)BTPS_AllocateMemory(sizeof(ANPM_Event_Callback_Info_t));

         if(AddedEntry)
         {
            /* Copy All Data over.                                      */
            *AddedEntry                              = *EntryToAdd;

            /* Now Add it to the end of the list.                       */
            AddedEntry->NextANPMEventCallbackInfoPtr = NULL;

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
                     if(tmpEntry->NextANPMEventCallbackInfoPtr)
                        tmpEntry = tmpEntry->NextANPMEventCallbackInfoPtr;
                     else
                        break;
                  }
               }

               if(AddedEntry)
               {
                  /* Last element found, simply Add the entry.          */
                  tmpEntry->NextANPMEventCallbackInfoPtr = AddedEntry;
               }
            }
            else
               *ListHead = AddedEntry;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", AddedEntry));

   return(AddedEntry);
}

   /* The following function searches the specified List for the        */
   /* specified Event Callback ID.  This function returns NULL if either*/
   /* the List Head is invalid, the Event Callback ID is invalid, or the*/
   /* specified Event Callback ID was NOT found.                        */
static ANPM_Event_Callback_Info_t *SearchEventCallbackInfoEntry(ANPM_Event_Callback_Info_t **ListHead, unsigned int EventCallbackID)
{
   ANPM_Event_Callback_Info_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", EventCallbackID));

   /* Let's make sure the list and Event Callback ID to search for      */
   /* appear to be valid.                                               */
   if((ListHead) && (EventCallbackID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->EventCallbackID != EventCallbackID))
         FoundEntry = FoundEntry->NextANPMEventCallbackInfoPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* The following function searches the specified List for the        */
   /* specified Client ID.  This function returns NULL if either the    */
   /* List Head is invalid, the Client ID is invalid, or the specified  */
   /* Client ID was NOT found.                                          */
static ANPM_Event_Callback_Info_t *SearchEventCallbackInfoEntryByClientID(ANPM_Event_Callback_Info_t **ListHead, unsigned int ClientID)
{
   ANPM_Event_Callback_Info_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", ClientID));

   /* Let's make sure the list and Event Callback ID to search for      */
   /* appear to be valid.                                               */
   if((ListHead) && (ClientID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->ClientID != ClientID))
         FoundEntry = FoundEntry->NextANPMEventCallbackInfoPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

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
static ANPM_Event_Callback_Info_t *DeleteEventCallbackInfoEntry(ANPM_Event_Callback_Info_t **ListHead, unsigned int EventCallbackID)
{
   ANPM_Event_Callback_Info_t *FoundEntry = NULL;
   ANPM_Event_Callback_Info_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", EventCallbackID));

   /* Let's make sure the List and Event Callback ID to search for      */
   /* appear to be semi-valid.                                          */
   if((ListHead) && (EventCallbackID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->EventCallbackID != EventCallbackID))
      {
         LastEntry  = FoundEntry;
         FoundEntry = FoundEntry->NextANPMEventCallbackInfoPtr;
      }

      /* Check to see if we found the specified entry.                  */
      if(FoundEntry)
      {
         /* OK, now let's remove the entry from the list.  We have to   */
         /* check to see if the entry was the first entry in the list.  */
         if(LastEntry)
         {
            /* Entry was NOT the first entry in the list.               */
            LastEntry->NextANPMEventCallbackInfoPtr = FoundEntry->NextANPMEventCallbackInfoPtr;
         }
         else
            *ListHead = FoundEntry->NextANPMEventCallbackInfoPtr;

         FoundEntry->NextANPMEventCallbackInfoPtr = NULL;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* The following function searches the specified List for the        */
   /* specified Client ID and removes it from the List.  This function  */
   /* returns NULL if either the List Head is invalid, the Callback ID  */
   /* is invalid, or the specified Callback ID was NOT present in the   */
   /* list.  The entry returned will have the Next Entry field set to   */
   /* NULL, and the caller is responsible for deleting the memory       */
   /* associated with this entry by calling                             */
   /* FreeEventCallbackInfoEntryMemory().                               */
static ANPM_Event_Callback_Info_t *DeleteEventCallbackInfoEntryByClientID(ANPM_Event_Callback_Info_t **ListHead, unsigned int ClientID)
{
   ANPM_Event_Callback_Info_t *FoundEntry = NULL;
   ANPM_Event_Callback_Info_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", ClientID));

   /* Let's make sure the List and Event Callback ID to search for      */
   /* appear to be semi-valid.                                          */
   if((ListHead) && (ClientID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->ClientID != ClientID))
      {
         LastEntry  = FoundEntry;
         FoundEntry = FoundEntry->NextANPMEventCallbackInfoPtr;
      }

      /* Check to see if we found the specified entry.                  */
      if(FoundEntry)
      {
         /* OK, now let's remove the entry from the list.  We have to   */
         /* check to see if the entry was the first entry in the list.  */
         if(LastEntry)
         {
            /* Entry was NOT the first entry in the list.               */
            LastEntry->NextANPMEventCallbackInfoPtr = FoundEntry->NextANPMEventCallbackInfoPtr;
         }
         else
            *ListHead = FoundEntry->NextANPMEventCallbackInfoPtr;

         FoundEntry->NextANPMEventCallbackInfoPtr = NULL;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* This function frees the specified Event Callback member.  No check*/
   /* is done on this entry other than making sure it NOT NULL.         */
static void FreeEventCallbackInfoEntryMemory(ANPM_Event_Callback_Info_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToFree)
      BTPS_FreeMemory(EntryToFree);

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified Event Callback List.  Upon return of this*/
   /* function, the Head Pointer is set to NULL.                        */
static void FreeEventCallbackInfoList(ANPM_Event_Callback_Info_t **ListHead)
{
   ANPM_Event_Callback_Info_t *EntryToFree;
   ANPM_Event_Callback_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ListHead)
   {
      /* Simply traverse the list and free every element present.       */
      EntryToFree = *ListHead;

      while(EntryToFree)
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Deleting Event Callback ID: 0x%08X\n", EntryToFree->EventCallbackID));

         tmpEntry    = EntryToFree;
         EntryToFree = EntryToFree->NextANPMEventCallbackInfoPtr;

         FreeEventCallbackInfoEntryMemory(tmpEntry);
      }

      /* Make sure the List appears to be empty.                        */
      *ListHead = NULL;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function adds the specified Entry to the specified  */
   /* List.  This function allocates and adds an entry to the list that */
   /* has the same attributes as the Entry passed into this function.   */
   /* This function will return NULL if NO Entry was added.  This can   */
   /* occur if the element passed in was deemed invalid or the actual   */
   /* List Head was invalid.                                            */
   /* ** NOTE ** This function does not insert duplicate entries into   */
   /*            the list.  An element is considered a duplicate if the */
   /*            the BD_ADDR field is the same as an entry already in   */
   /*            the list.  When this occurs, this function returns     */
   /*            NULL.                                                  */
static Connection_Entry_t *AddConnectionEntry(Connection_Entry_t **ListHead, Connection_Entry_t *EntryToAdd)
{
   BD_ADDR_t           NULL_BD_ADDR;
   Connection_Entry_t *AddedEntry = NULL;
   Connection_Entry_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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
                  if(COMPARE_BD_ADDR(tmpEntry->BD_ADDR, AddedEntry->BD_ADDR))
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

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", AddedEntry));

   return(AddedEntry);
}

   /* The following function searches the specified Connection Entry    */
   /* List for the specified Connection Entry based on the specified    */
   /* Bluetooth Device Address.  This function returns NULL if either   */
   /* the Connection Entry List Head is invalid, the Bluetooth Device   */
   /* Address is invalid, or the specified Entry was NOT present in the */
   /* list.                                                             */
static Connection_Entry_t *SearchConnectionEntry(Connection_Entry_t **ListHead, unsigned int ConnectionID)
{
   Connection_Entry_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's make sure the list and Bluetooth Device Address to search   */
   /* for appear to be valid.                                           */
   if((ListHead) && (ConnectionID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->ConnectionID != ConnectionID))
         FoundEntry = FoundEntry->NextConnectionEntryPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

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
static Connection_Entry_t *DeleteConnectionEntry(Connection_Entry_t **ListHead, unsigned int ConnectionID)
{
   Connection_Entry_t *FoundEntry = NULL;
   Connection_Entry_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's make sure the list and Bluetooth Device Address to search   */
   /* for appear to be valid.                                           */
   if((ListHead) && (ConnectionID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->ConnectionID != ConnectionID))
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

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

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
static Connection_Entry_t *DeleteConnectionEntryByBD_ADDR(Connection_Entry_t **ListHead, BD_ADDR_t BD_ADDR)
{
   Connection_Entry_t *FoundEntry = NULL;
   Connection_Entry_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's make sure the list and Bluetooth Device Address to search   */
   /* for appear to be valid.                                           */
   if((ListHead) && (!COMPARE_NULL_BD_ADDR(BD_ADDR)))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (!COMPARE_BD_ADDR(FoundEntry->BD_ADDR, BD_ADDR)))
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

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* This function frees the specified Connection Information member.  */
   /* No check is done on this entry other than making sure it NOT NULL.*/
static void FreeConnectionEntryMemory(Connection_Entry_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToFree)
      BTPS_FreeMemory(EntryToFree);

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified Connection Information List.  Upon return*/
   /* of this function, the Head Pointer is set to NULL.                */
static void FreeConnectionEntryList(Connection_Entry_t **ListHead)
{
   Connection_Entry_t *EntryToFree;
   Connection_Entry_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function adds the specified Entry to the specified  */
   /* List.  This function allocates and adds an entry to the list that */
   /* has the same attributes as the Entry passed into this function.   */
   /* This function will return NULL if NO Entry was added.  This can   */
   /* occur if the element passed in was deemed invalid or the actual   */
   /* List Head was invalid.                                            */
   /* ** NOTE ** This function does not insert duplicate entries into   */
   /*            the list.  An element is considered a duplicate if the */
   /*            the BD_ADDR field is the same as an entry already in   */
   /*            the list.  When this occurs, this function returns     */
   /*            NULL.                                                  */
static Alert_Entry_t *AddAlertEntry(Alert_Entry_t **ListHead, Alert_Entry_t *EntryToAdd, unsigned int StringLength, char *String)
{
   Alert_Entry_t *AddedEntry = NULL;
   Alert_Entry_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First let's verify that values passed in are semi-valid.          */
   if((ListHead) && (EntryToAdd))
   {
      /* Make sure that the element that we are adding seems semi-valid.*/
      if((EntryToAdd->ClientID) && (EntryToAdd->Count))
      {
         /* OK, data seems semi-valid, let's allocate a new data        */
         /* structure to add to the list.                               */
         AddedEntry = (Alert_Entry_t *)BTPS_AllocateMemory(ALERT_ENTRY_DATA_SIZE(StringLength+1));

         if(AddedEntry)
         {
            /* Copy All Data over.                                      */
            *AddedEntry                   = *EntryToAdd;

            /* Now Add it to the end of the list.                       */
            AddedEntry->NextAlertEntryPtr = NULL;

            /* Add the string if needed.                                */
            if((StringLength) && (String))
            {
               AddedEntry->LastAlertText = (char *)(((Byte_t *)AddedEntry) + ((unsigned int)ALERT_ENTRY_DATA_SIZE(0)));

               BTPS_MemCopy(AddedEntry->LastAlertText, String, StringLength);

               AddedEntry->LastAlertText[StringLength] = '\0';
            }
            else
               AddedEntry->LastAlertText = NULL;

            /* First, let's check to see if there are any elements      */
            /* already present in the List that was passed in.          */
            if((tmpEntry = *ListHead) != NULL)
            {
               /* Head Pointer was not NULL, so we will traverse the    */
               /* list until we reach the last element.                 */
               while(tmpEntry)
               {
                  if((tmpEntry->ClientID == AddedEntry->ClientID) && (tmpEntry->Category == AddedEntry->Category))
                  {
                     /* Entry was already added, so free the memory and */
                     /* flag an error to the caller.                    */
                     FreeAlertEntryMemory(AddedEntry);
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
                     if(tmpEntry->NextAlertEntryPtr)
                        tmpEntry = tmpEntry->NextAlertEntryPtr;
                     else
                        break;
                  }
               }

               if(AddedEntry)
               {
                  /* Last element found, simply Add the entry.          */
                  tmpEntry->NextAlertEntryPtr = AddedEntry;
               }
            }
            else
               *ListHead = AddedEntry;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", AddedEntry));

   return(AddedEntry);
}

   /* The following function searches the specified Connection Entry    */
   /* List for the Connection Entry with the specified Bluetooth Device */
   /* Address and removes it from the List.  This function returns NULL */
   /* if either the Connection Entry List Head is invalid, the Bluetooth*/
   /* Device Address is invalid, or the specified Entry was NOT present */
   /* in the list.  The entry returned will have the Next Entry field   */
   /* set to NULL, and the caller is responsible for deleting the memory*/
   /* associated with this entry by calling FreeConnectionEntryMemory().*/
static Alert_Entry_t *DeleteAlertEntry(Alert_Entry_t **ListHead, unsigned int ClientID, ANPM_Category_Identification_t Category)
{
   Alert_Entry_t *FoundEntry = NULL;
   Alert_Entry_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's make sure the list and Bluetooth Device Address to search   */
   /* for appear to be valid.                                           */
   if((ListHead) && (ClientID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && ((FoundEntry->ClientID != ClientID) || (FoundEntry->Category != Category)))
      {
         LastEntry  = FoundEntry;
         FoundEntry = FoundEntry->NextAlertEntryPtr;
      }

      /* Check to see if we found the specified entry.                  */
      if(FoundEntry)
      {
         /* OK, now let's remove the entry from the list.  We have to   */
         /* check to see if the entry was the first entry in the list.  */
         if(LastEntry)
         {
            /* Entry was NOT the first entry in the list.               */
            LastEntry->NextAlertEntryPtr = FoundEntry->NextAlertEntryPtr;
         }
         else
            *ListHead = FoundEntry->NextAlertEntryPtr;

         FoundEntry->NextAlertEntryPtr = NULL;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* This function frees the specified Connection Information member.  */
   /* No check is done on this entry other than making sure it NOT NULL.*/
static void FreeAlertEntryMemory(Alert_Entry_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToFree)
      BTPS_FreeMemory(EntryToFree);

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified Connection Information List.  Upon return*/
   /* of this function, the Head Pointer is set to NULL.                */
static void FreeAlertEntryList(Alert_Entry_t **ListHead)
{
   Alert_Entry_t *EntryToFree;
   Alert_Entry_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ListHead)
   {
      /* Simply traverse the list and free every element present.       */
      EntryToFree = *ListHead;

      while(EntryToFree)
      {
         tmpEntry    = EntryToFree;
         EntryToFree = EntryToFree->NextAlertEntryPtr;

         FreeAlertEntryMemory(tmpEntry);
      }

      /* Make sure the List appears to be empty.                        */
      *ListHead = NULL;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function adds the specified Entry to the specified  */
   /* List.  This function allocates and adds an entry to the list that */
   /* has the same attributes as the Entry passed into this function.   */
   /* This function will return NULL if NO Entry was added.  This can   */
   /* occur if the element passed in was deemed invalid or the actual   */
   /* List Head was invalid.                                            */
   /* ** NOTE ** This function does not insert duplicate entries into   */
   /*            the list.  An element is considered a duplicate if the */
   /*            the BD_ADDR field is the same as an entry already in   */
   /*            the list.  When this occurs, this function returns     */
   /*            NULL.                                                  */
static Client_Connection_Entry_t *AddClientConnectionEntry(Client_Connection_Entry_t **ListHead, Client_Connection_Entry_t *EntryToAdd)
{
   return((Client_Connection_Entry_t *)BSC_AddGenericListEntry(sizeof(Client_Connection_Entry_t), ekBD_ADDR_t, BTPS_STRUCTURE_OFFSET(Client_Connection_Entry_t, BD_ADDR), sizeof(Client_Connection_Entry_t), BTPS_STRUCTURE_OFFSET(Client_Connection_Entry_t, NextClientConnectionEntryPtr), (void **)ListHead, ((void *)EntryToAdd)));
}

   /* The following function searches the specified Connection Entry    */
   /* List for the specified Connection Entry based on the specified    */
   /* Bluetooth Device Address.  This function returns NULL if either   */
   /* the Connection Entry List Head is invalid, the Bluetooth Device   */
   /* Address is invalid, or the specified Entry was NOT present in the */
   /* list.                                                             */
static Client_Connection_Entry_t *SearchClientConnectionEntry(Client_Connection_Entry_t **ListHead, BD_ADDR_t BD_ADDR)
{
   return((Client_Connection_Entry_t *)BSC_SearchGenericListEntry(ekBD_ADDR_t, (void *)&BD_ADDR, BTPS_STRUCTURE_OFFSET(Client_Connection_Entry_t, BD_ADDR), BTPS_STRUCTURE_OFFSET(Client_Connection_Entry_t, NextClientConnectionEntryPtr), (void **)ListHead));
}

   /* The following function searches the specified Connection Entry    */
   /* List for the Connection Entry with the specified Bluetooth Device */
   /* Address and removes it from the List.  This function returns      */
   /* NULL if either the Connection Entry List Head is invalid, the     */
   /* Bluetooth Device Address is invalid, or the specified Entry       */
   /* was NOT present in the list.  The entry returned will have the    */
   /* Next Entry field set to NULL, and the caller is responsible       */
   /* for deleting the memory associated with this entry by calling     */
   /* FreeClientConnectionEntryMemory().                                */
static Client_Connection_Entry_t *DeleteClientConnectionEntry(Client_Connection_Entry_t **ListHead, BD_ADDR_t BD_ADDR)
{
   return((Client_Connection_Entry_t *)BSC_DeleteGenericListEntry(ekBD_ADDR_t, (void *)&BD_ADDR, BTPS_STRUCTURE_OFFSET(Client_Connection_Entry_t, BD_ADDR), BTPS_STRUCTURE_OFFSET(Client_Connection_Entry_t, NextClientConnectionEntryPtr), (void **)ListHead));
}

   /* This function frees the specified Connection Information member.  */
   /* No check is done on this entry other than making sure it NOT NULL.*/
static void FreeClientConnectionEntryMemory(Client_Connection_Entry_t *EntryToFree)
{
   if(EntryToFree)
   {
      FreeNotificationEntryList(&EntryToFree->NewAlertNotificationList);
      FreeNotificationEntryList(&EntryToFree->UnreadStatusNotificationList);
      FreeCategoryEntryList(&EntryToFree->NewAlertCategoryList);
      FreeCategoryEntryList(&EntryToFree->UnreadStatusCategoryList);

      BSC_FreeGenericListEntryMemory((void *)EntryToFree);
   }
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified Connection Information List.  Upon return*/
   /* of this function, the Head Pointer is set to NULL.                */
static void FreeClientConnectionEntryList(Client_Connection_Entry_t **ListHead)
{
   Client_Connection_Entry_t *ConnectionEntry;
   Client_Connection_Entry_t *tmpConnectionEntry;

   if(ListHead)
   {
      ConnectionEntry = *ListHead;

      while(ConnectionEntry)
      {
         tmpConnectionEntry = ConnectionEntry;
         ConnectionEntry = ConnectionEntry->NextClientConnectionEntryPtr;

         FreeClientConnectionEntryMemory(tmpConnectionEntry);
      }
   }
}

   /* The following function will cleanup any registration entries for a*/
   /* specified Callback ID if the callback has been un-registered. This*/
   /* function will remove all entries from the list, then remove any   */
   /* registration with a remote device if the specified callback was   */
   /* the last registered listener.                                     */
static void CleanupUnregisteredClient(Client_Connection_Entry_t **ListHead, unsigned int CallbackID)
{
   Word_t                             Buffer;
   Category_Entry_t                  *CategoryEntry;
   Notification_Entry_t              *NotificationEntry;
   Client_Connection_Entry_t         *ConnectionEntry;
   ANS_Control_Point_Command_Value_t  CommandValue;

   if(ListHead)
   {
      for(ConnectionEntry=*ListHead;ConnectionEntry;ConnectionEntry=ConnectionEntry->NextClientConnectionEntryPtr)
      {
         /* First remove any entries for the client, then clean up any  */
         /* remote registration if the removed entry was the last in the*/
         /* list.                                                       */
         /* * NOTE * Just send out any disable requests, we don't care  */
         /*          about responses since no one is listening.         */
         Buffer = 0;

         if((CategoryEntry = DeleteCategoryEntry(&ConnectionEntry->NewAlertCategoryList, CallbackID)) != NULL)
         {
            if((!ConnectionEntry->NewAlertCategoryList) && (!ANS_Format_Control_Point_Command(&CommandValue, pcDisable_New_Alert_Notifications, ciAllCategories)))
            {
               GATM_WriteValue(GATMCallbackID, ConnectionEntry->BD_ADDR, ConnectionEntry->ClientInformation.Control_Point, ANS_CONTROL_POINT_COMMAND_VALUE_DATA_SIZE, (Byte_t *)&CommandValue);
               ConnectionEntry->Flags &= ~((unsigned long)CLIENT_CONNECTION_FLAGS_NEW_ALERT_NOTIFICATIONS_ENABLED);
            }

            FreeCategoryEntryMemory(CategoryEntry);
         }

         if((CategoryEntry = DeleteCategoryEntry(&ConnectionEntry->UnreadStatusCategoryList, CallbackID)) != NULL)
         {
            if((!ConnectionEntry->UnreadStatusCategoryList) && (!ANS_Format_Control_Point_Command(&CommandValue, pcDisable_Unread_Category_Notifications, ciAllCategories)))
            {
               GATM_WriteValue(GATMCallbackID, ConnectionEntry->BD_ADDR, ConnectionEntry->ClientInformation.Control_Point, ANS_CONTROL_POINT_COMMAND_VALUE_DATA_SIZE, (Byte_t *)&CommandValue);
               ConnectionEntry->Flags &= ~((unsigned long)CLIENT_CONNECTION_FLAGS_UNREAD_NOTIFICATIONS_ENABLED);
            }

            FreeCategoryEntryMemory(CategoryEntry);
         }

         if((NotificationEntry = DeleteNotificationEntry(&ConnectionEntry->NewAlertNotificationList, CallbackID)) != NULL)
         {
            if(!ConnectionEntry->NewAlertNotificationList)
            {
               GATM_WriteValue(GATMCallbackID, ConnectionEntry->BD_ADDR, ConnectionEntry->ClientInformation.New_Alert_Client_Configuration, NON_ALIGNED_WORD_SIZE, (Byte_t *)&Buffer);
               ConnectionEntry->EnabledNewAlertCategoryMask = 0;
            }

            FreeNotificationEntryMemory(NotificationEntry);
         }

         if((NotificationEntry = DeleteNotificationEntry(&ConnectionEntry->UnreadStatusNotificationList, CallbackID)) != NULL)
         {
            if(!ConnectionEntry->UnreadStatusNotificationList)
            {
               GATM_WriteValue(GATMCallbackID, ConnectionEntry->BD_ADDR, ConnectionEntry->ClientInformation.Unread_Alert_Status_Client_Configuration, NON_ALIGNED_WORD_SIZE, (Byte_t *)&Buffer);
               ConnectionEntry->EnabledUnreadStatusCategoryMask = 0;
            }

            FreeNotificationEntryMemory(NotificationEntry);
         }
      }
   }
}

   /* The following function adds the specified Entry to the specified  */
   /* List.  This function allocates and adds an entry to the list that */
   /* has the same attributes as the Entry passed into this function.   */
   /* This function will return NULL if NO Entry was added.  This can   */
   /* occur if the element passed in was deemed invalid or the actual   */
   /* List Head was invalid.                                            */
   /* ** NOTE ** This function does not insert duplicate entries into   */
   /*            the list.  An element is considered a duplicate if the */
   /*            the CallbackID field is the same as an entry already in*/
   /*            the list.  When this occurs, this function returns     */
   /*            NULL.                                                  */
static Notification_Entry_t *AddNotificationEntry(Notification_Entry_t **ListHead, Notification_Entry_t *EntryToAdd)
{
   return((Notification_Entry_t *)BSC_AddGenericListEntry(sizeof(Notification_Entry_t), ekUnsignedInteger, BTPS_STRUCTURE_OFFSET(Notification_Entry_t, CallbackID), sizeof(Notification_Entry_t), BTPS_STRUCTURE_OFFSET(Notification_Entry_t, NextNotificationEntryPtr), (void **)ListHead, ((void *)EntryToAdd)));
}

   /* The following function searches the specified Notification List   */
   /* for the Connection Entry with the specified Callback ID and       */
   /* removes it from the List.  This function returns NULL if either   */
   /* the Notification List Head is invalid, the CallbackID is invalid, */
   /* or the specified Entry was NOT present in the list.  The entry    */
   /* returned will have the Next Entry field set to NULL, and the      */
   /* caller is responsible for deleting the memory associated with this*/
   /* entry by calling FreeNotificationEntryMemory().                   */
static Notification_Entry_t *DeleteNotificationEntry(Notification_Entry_t **ListHead, unsigned int CallbackID)
{
   return((Notification_Entry_t *)BSC_DeleteGenericListEntry(ekUnsignedInteger, (void *)&CallbackID, BTPS_STRUCTURE_OFFSET(Notification_Entry_t, CallbackID), BTPS_STRUCTURE_OFFSET(Notification_Entry_t, NextNotificationEntryPtr), (void **)ListHead));
}

   /* This function frees the specified Notification Entry member.  No  */
   /* check is done on this entry other than making sure it NOT NULL.   */
static void FreeNotificationEntryMemory(Notification_Entry_t *EntryToFree)
{
   BSC_FreeGenericListEntryMemory((void *)EntryToFree);
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified Notification List.  Upon return of this  */
   /* function, the Head Pointer is set to NULL.                        */
static void FreeNotificationEntryList(Notification_Entry_t **ListHead)
{
   BSC_FreeGenericListEntryList((void **)ListHead, BTPS_STRUCTURE_OFFSET(Notification_Entry_t, NextNotificationEntryPtr));
}

   /* The following function searches a Category List for the specified */
   /* CallbackID and adds the specified Entry to the specified List if  */
   /* the CallbackID does not currently exist.  This function allocates */
   /* and adds an entry to the list that has the same attributes as the */
   /* Entry passed into this function.  This function will return NULL  */
   /* if NO Entry was found and a new Entry could not be added.  This   */
   /* can occur if the element passed in was deemed invalid or memory   */
   /* could not be allocated.                                           */
static Category_Entry_t *SearchAddCategoryEntry(Category_Entry_t **ListHead, Category_Entry_t *EntryToSearchAdd)
{
   Category_Entry_t *CategoryEntry = NULL;

   if(EntryToSearchAdd)
   {
      CategoryEntry = BSC_SearchGenericListEntry(ekUnsignedInteger, (void *)&EntryToSearchAdd->CallbackID, BTPS_STRUCTURE_OFFSET(Category_Entry_t, CallbackID), BTPS_STRUCTURE_OFFSET(Category_Entry_t, NextCategoryEntryPtr), (void **)ListHead);

      if(!CategoryEntry)
         CategoryEntry = BSC_AddGenericListEntry(sizeof(Category_Entry_t), ekUnsignedInteger, BTPS_STRUCTURE_OFFSET(Category_Entry_t, CallbackID), sizeof(Category_Entry_t), BTPS_STRUCTURE_OFFSET(Category_Entry_t, NextCategoryEntryPtr), (void **)ListHead, ((void *)EntryToSearchAdd));
   }

   return(CategoryEntry);
}

   /* The following function searches the specified Category Entry  */
   /* List for the specified Connection Entry based on the specified    */
   /* CallbackID.  This function returns NULL if either the Category*/
   /* List Head is invalid, the CallbackID is invalid, or the specified */
   /* Entry was NOT present in the list.                                */
static Category_Entry_t *SearchCategoryEntry(Category_Entry_t **ListHead, unsigned int CallbackID)
{
   return((Category_Entry_t *)BSC_SearchGenericListEntry(ekUnsignedInteger, (void *)&CallbackID, BTPS_STRUCTURE_OFFSET(Category_Entry_t, CallbackID), BTPS_STRUCTURE_OFFSET(Category_Entry_t, NextCategoryEntryPtr), (void **)ListHead));
}

   /* The following function searches the specified Category List for   */
   /* the Connection Entry with the specified Callback ID and removes it*/
   /* from the List.  This function returns NULL if either the Category */
   /* List Head is invalid, the CallbackID is invalid, or the specified */
   /* Entry was NOT present in the list.  The entry returned will have  */
   /* the Next Entry field set to NULL, and the caller is responsible   */
   /* for deleting the memory associated with this entry by calling     */
   /* FreeCategoryEntryMemory().                                        */
static Category_Entry_t *DeleteCategoryEntry(Category_Entry_t **ListHead, unsigned int CallbackID)
{
   return((Category_Entry_t *)BSC_DeleteGenericListEntry(ekUnsignedInteger, (void *)&CallbackID, BTPS_STRUCTURE_OFFSET(Category_Entry_t, CallbackID), BTPS_STRUCTURE_OFFSET(Category_Entry_t, NextCategoryEntryPtr), (void **)ListHead));
}

   /* This function frees the specified Category member.  No check is   */
   /* done on this entry other than making sure it NOT NULL.            */
static void FreeCategoryEntryMemory(Category_Entry_t *EntryToFree)
{
   BSC_FreeGenericListEntryMemory((void *)EntryToFree);
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified Category List.  Upon return of this      */
   /* function, the Head Pointer is set to NULL.                        */
static void FreeCategoryEntryList(Category_Entry_t **ListHead)
{
   BSC_FreeGenericListEntryList((void **)ListHead, BTPS_STRUCTURE_OFFSET(Category_Entry_t, NextCategoryEntryPtr));
}

   /* The following function will determine if a registered callback    */
   /* has enabled a specified category. This function will return TRUE  */
   /* if the callback has enabled ALL categories or if the callback has */
   /* enabled the specified category explicitly.                        */
static Boolean_t IsCategoryEnabledForClient(Category_Entry_t **ListHead, ANPM_Category_Identification_t Category, unsigned int CallbackID)
{
   Boolean_t         ret_val       = FALSE;
   unsigned long     Mask;
   Category_Entry_t *CategoryEntry;

   if((ListHead) && (CallbackID))
   {
      CategoryEntry = *ListHead;
      Mask = CategoryToMask(Category, TRUE, FALSE);

      while((CategoryEntry) && (!ret_val))
      {
         if((CategoryEntry->CallbackID == CallbackID) && ((CategoryEntry->CategoryMask & Mask) == Mask))
            ret_val = TRUE;

         CategoryEntry = CategoryEntry->NextCategoryEntryPtr;
      }
   }

   return(ret_val);
}

   /* This function will traverse a list of enabled categories for      */
   /* callbacks and reset them base on the EnabledMask provided. The    */
   /* EnabledMask should be the mask of categories currently enabled    */
   /* successfully on the remote ANP server. Each entry in the last will*/
   /* have all bits removed that are not currently enabled on the remote*/
   /* server.                                                           */
   /* * NOTE * This is used when a write attempt to enabled categories  */
   /*          fails. This allows us to reset back to a state where we  */
   /*          are clients are still enabled for categories that had    */
   /*          been previously enabled but the failed categories are    */
   /*          removied.                                                */
   /* * NOTE * If the mask for an entry in the list is empty after being*/
   /*          reset, the entry will be deleted.                        */
static void ResetCategoriesToEnabledMask(Category_Entry_t **ListHead, unsigned long EnabledMask)
{
   Category_Entry_t *CategoryEntry;
   Category_Entry_t *tmpCategoryEntry;

   if(ListHead)
   {
      CategoryEntry = *ListHead;

      while(CategoryEntry)
      {
         CategoryEntry->CategoryMask &= EnabledMask;

         if(!CategoryEntry->CategoryMask)
         {
            tmpCategoryEntry = CategoryEntry->NextCategoryEntryPtr;

            if((CategoryEntry = DeleteCategoryEntry(ListHead, CategoryEntry->CallbackID)) != NULL)
               FreeCategoryEntryMemory(CategoryEntry);

            CategoryEntry = tmpCategoryEntry;
         }
         else
            CategoryEntry = CategoryEntry->NextCategoryEntryPtr;
      }
   }
}

   /* The following function adds the specified Entry to the specified  */
   /* List.  This function allocates and adds an entry to the list that */
   /* has the same attributes as the Entry passed into this function.   */
   /* This function will return NULL if NO Entry was added.  This can   */
   /* occur if the element passed in was deemed invalid or the actual   */
   /* List Head was invalid.                                            */
   /* ** NOTE ** This function does not insert duplicate entries into   */
   /*            the list.  An element is considered a duplicate if the */
   /*            the TransactionID field is the same as an entry already*/
   /*            in the list.  When this occurs, this function returns  */
   /*            NULL.                                                  */
static Transaction_Entry_t *AddTransactionEntry(Transaction_Entry_t **ListHead, Transaction_Entry_t *EntryToAdd)
{
   return((Transaction_Entry_t *)BSC_AddGenericListEntry(sizeof(Transaction_Entry_t), ekUnsignedInteger, BTPS_STRUCTURE_OFFSET(Transaction_Entry_t, TransactionID), sizeof(Transaction_Entry_t), BTPS_STRUCTURE_OFFSET(Transaction_Entry_t, NextTransactionEntryPtr), (void **)ListHead, ((void *)EntryToAdd)));
}

   /* The following function searches the specified Transaction Entry   */
   /* List for the specified Transaction Entry based on the specified   */
   /* Remote Device Address and Transaction Type.  This function        */
   /* returns NULL if either the Transaction List Head is invalid, the  */
   /* RemoteDeviceAddress is invalid, or the specified Entry was NOT    */
   /* present in the list.                                              */
   /* * NOTE * It is possible to have multiple results in a list for    */
   /*          this function if the TransactionType allows queued       */
   /*          transactions from separate clients. This functions       */
   /*          is mainly used for transactions which only allow one     */
   /*          outstanding transaction and multiplex the response.      */
static Transaction_Entry_t *SearchTransactionEntryByType(Transaction_Entry_t **ListHead, BD_ADDR_t BD_ADDR, TransactionType_t TransactionType)
{
   Transaction_Entry_t *ret_val = NULL;

   if(ListHead)
   {
      if(!COMPARE_NULL_BD_ADDR(BD_ADDR))
      {
         for(ret_val = *ListHead; ret_val; ret_val = ret_val->NextTransactionEntryPtr)
         {
            if(COMPARE_BD_ADDR(ret_val->BD_ADDR, BD_ADDR) && (ret_val->TransactionType == TransactionType))
               break;
         }
      }
   }

   return(ret_val);
}

   /* The following function searches the specified Transaction Entry   */
   /* List for the specified Transaction Entry based on the specified   */
   /* Address and whether it has a pending enable or disable category   */
   /* transaction for the given category.  This function returns NULL   */
   /* if either the Transaction List Head is invalid, the Address is    */
   /* invalid, or the specified Entry was NOT present in the list.      */
static Transaction_Entry_t *SearchTransactionEntryByPendingCategory(Transaction_Entry_t **ListHead, BD_ADDR_t BD_ADDR, ANPM_Category_Identification_t Category, ANPM_Notification_Type_t Type, Boolean_t Enabling)
{
   Transaction_Entry_t     *ret_val   = NULL;
   ControlPointOperation_t  Operation;

   if(ListHead)
   {
      if(!COMPARE_NULL_BD_ADDR(BD_ADDR))
      {
         if(Type == ntNewAlert)
            Operation = Enabling?cpoEnableNewAlertCategory:cpoDisableNewAlertCategory;
         else
            Operation = Enabling?cpoEnableUnreadStatusCategory:cpoDisableUnreadStatusCategory;

         for(ret_val = *ListHead; ret_val; ret_val = ret_val->NextTransactionEntryPtr)
         {
            if(COMPARE_BD_ADDR(ret_val->BD_ADDR, BD_ADDR) && (ret_val->TransactionType == ttWriteControlPoint) && (ret_val->ControlPointOperation == Operation) && (ret_val->PendingCategoryID == Category))
               break;
         }
      }
   }

   return(ret_val);
}

   /* The following function searches the specified Transaction List    */
   /* for the Transaction Entry with the specified Transaction ID       */
   /* and removes it from the List.  This function returns NULL if      */
   /* either the Transaction List Head is invalid, the TransactionID is */
   /* invalid, or the specified Entry was NOT present in the list.  The */
   /* entry returned will have the Next Entry field set to NULL, and the*/
   /* caller is responsible for deleting the memory associated with this*/
   /* entry by calling FreeTransactionEntryMemory().                    */
static Transaction_Entry_t *DeleteTransactionEntry(Transaction_Entry_t **ListHead, unsigned int TransactionID)
{
   return((Transaction_Entry_t *)BSC_DeleteGenericListEntry(ekUnsignedInteger, (void *)&TransactionID, BTPS_STRUCTURE_OFFSET(Transaction_Entry_t, TransactionID), BTPS_STRUCTURE_OFFSET(Transaction_Entry_t, NextTransactionEntryPtr), (void **)ListHead));
}

   /* The following function searches the specified Transaction List for*/
   /* the Transaction Entry with the specified GATM Transaction ID and  */
   /* removes it from the List.  This function returns NULL if either   */
   /* the Transaction List Head is invalid, the GATM TransactionID is   */
   /* invalid, or the specified Entry was NOT present in the list.  The */
   /* entry returned will have the Next Entry field set to NULL, and the*/
   /* caller is responsible for deleting the memory associated with this*/
   /* entry by calling FreeTransactionEntryMemory().                    */
static Transaction_Entry_t *DeleteTransactionEntryByGATMID(Transaction_Entry_t **ListHead, unsigned int GATMTransactionID)
{
   return((Transaction_Entry_t *)BSC_DeleteGenericListEntry(ekUnsignedInteger, (void *)&GATMTransactionID, BTPS_STRUCTURE_OFFSET(Transaction_Entry_t, GATMTransactionID), BTPS_STRUCTURE_OFFSET(Transaction_Entry_t, NextTransactionEntryPtr), (void **)ListHead));
}

   /* This function frees the specified Transaction member.  No check is*/
   /* done on this entry other than making sure it NOT NULL.            */
static void FreeTransactionEntryMemory(Transaction_Entry_t *EntryToFree)
{
   if(EntryToFree)
   {
      FreeNotificationEntryList(&EntryToFree->PendingResultCallbacks);
      BSC_FreeGenericListEntryMemory((void *)EntryToFree);
   }
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified Transaction List.  Upon return of this   */
   /* function, the Head Pointer is set to NULL.                        */
static void FreeTransactionEntryList(Transaction_Entry_t **ListHead)
{
   BSC_FreeGenericListEntryList((void **)ListHead, BTPS_STRUCTURE_OFFSET(Transaction_Entry_t, TransactionID));
}

   /* The following function is a utility function that exists to       */
   /* dispatch the specified ANP event to every registered ANP Event    */
   /* Callback.                                                         */
   /* * NOTE * This function should be called with the ANP Manager Lock */
   /*          held.  Upon exit from this function it will free the ANP */
   /*          Manager Lock.                                            */
static void DispatchANPMEvent(Boolean_t Server, ANPM_Event_Data_t *ANPMEventData, BTPM_Message_t *Message)
{
   unsigned int                Index;
   unsigned int                Index1;
   unsigned int                ServerID;
   unsigned int                NumberCallbacks;
   Callback_Info_t             CallbackInfoArray[16];
   Callback_Info_t            *CallbackInfoArrayPtr;
   ANPM_Event_Callback_Info_t *CallbackInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   CallbackInfoPtr = Server?EventCallbackInfoList:ClientEventCallbackInfoList;

   /* Make sure that the input parameters appear to be semi-valid.      */
   if((CallbackInfoPtr) && (ANPMEventData) && (Message))
   {
      /* Next, let's determine how many callbacks are registered.       */
      ServerID        = MSG_GetServerAddressID();
      NumberCallbacks = 0;

      /* First, add the default event handlers.                         */
      while(CallbackInfoPtr)
      {
         if((CallbackInfoPtr->EventCallback) || (CallbackInfoPtr->ClientID != ServerID))
            NumberCallbacks++;

         CallbackInfoPtr = CallbackInfoPtr->NextANPMEventCallbackInfoPtr;
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
            CallbackInfoPtr = Server?EventCallbackInfoList:ClientEventCallbackInfoList;
            NumberCallbacks = 0;

            /* Next add the default event handlers.                     */
            while(CallbackInfoPtr)
            {
               if((CallbackInfoPtr->EventCallback) || (CallbackInfoPtr->ClientID != ServerID))
               {
                  CallbackInfoArrayPtr[NumberCallbacks].ClientID          = CallbackInfoPtr->ClientID;
                  CallbackInfoArrayPtr[NumberCallbacks].EventCallback     = CallbackInfoPtr->EventCallback;
                  CallbackInfoArrayPtr[NumberCallbacks].CallbackParameter = CallbackInfoPtr->CallbackParameter;

                  NumberCallbacks++;
               }

               CallbackInfoPtr = CallbackInfoPtr->NextANPMEventCallbackInfoPtr;
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
               /*          for ANP events and Data Events.              */
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
                        (*CallbackInfoArrayPtr[Index].EventCallback)(ANPMEventData, CallbackInfoArrayPtr[Index].CallbackParameter);
                     }
                  }
                  __BTPSEXCEPT(1)
                  {
                     /* Do Nothing.                                     */
                  }
               }
               else
               {
                  /* Walk the proceding list and see if we have already */
                  /* dispatched this event to this client.              */
                  for(Index1=0;Index1<Index;Index1++)
                  {
                     if((CallbackInfoArrayPtr[Index1].ClientID != ServerID) && (CallbackInfoArrayPtr[Index1].ClientID == CallbackInfoArrayPtr[Index].ClientID))
                        break;
                  }

                  if(Index1 == Index)
                  {
                     /* Dispatch the Message.                           */
                     Message->MessageHeader.AddressID = CallbackInfoArrayPtr[Index].ClientID;

                     MSG_SendMessage(Message);
                  }
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

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch the specified ANP event to every ANP Event Callback which*/
   /* is registered for the given category.                             */
   /* * NOTE * This function should be called with the ANP Manager Lock */
   /*          held.                                                    */
static void DispatchANPMNotificationEvent(Notification_Entry_t *NotificationClientList, Category_Entry_t *CategoryList, ANPM_Event_Data_t *ANPMEventData, BTPM_Message_t *Message, ANPM_Category_Identification_t Category)
{
   unsigned int                Index;
   unsigned int                Index1;
   unsigned int                ServerID;
   unsigned int                NumberCallbacks;
   Callback_Info_t             CallbackInfoArray[16];
   Callback_Info_t            *CallbackInfoArrayPtr;
   Notification_Entry_t       *NotificationEntry;
   ANPM_Event_Callback_Info_t *CallbackInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure that the input parameters appear to be semi-valid.      */
   if((NotificationClientList) && (ANPMEventData) && (Message))
   {
      /* Next, let's determine how many callbacks are registered.       */
      NotificationEntry = NotificationClientList;
      ServerID          = MSG_GetServerAddressID();
      NumberCallbacks   = 0;

      /* First, add the default event handlers.                         */
      while(NotificationEntry)
      {
         if(((CallbackInfoPtr = SearchEventCallbackInfoEntry(&ClientEventCallbackInfoList, NotificationEntry->CallbackID)) != NULL) && ((CallbackInfoPtr->EventCallback) || (CallbackInfoPtr->ClientID != ServerID)))
         {
            if((Category == cmAllSupportedCategories) || (IsCategoryEnabledForClient(&CategoryList, Category, NotificationEntry->CallbackID)))
               NumberCallbacks++;
         }

         NotificationEntry = NotificationEntry->NextNotificationEntryPtr;
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
            NumberCallbacks   = 0;
            NotificationEntry = NotificationClientList;

            /* Next add the default event handlers.                     */
            while(NotificationEntry)
            {
               if(((CallbackInfoPtr = SearchEventCallbackInfoEntry(&ClientEventCallbackInfoList, NotificationEntry->CallbackID)) != NULL) && ((CallbackInfoPtr->EventCallback) || (CallbackInfoPtr->ClientID != ServerID)))
               {
                  if((Category == cmAllSupportedCategories) || (IsCategoryEnabledForClient(&CategoryList, Category, NotificationEntry->CallbackID)))
                  {
                     CallbackInfoArrayPtr[NumberCallbacks].ClientID          = CallbackInfoPtr->ClientID;
                     CallbackInfoArrayPtr[NumberCallbacks].EventCallback     = CallbackInfoPtr->EventCallback;
                     CallbackInfoArrayPtr[NumberCallbacks].CallbackParameter = CallbackInfoPtr->CallbackParameter;

                     NumberCallbacks++;
                  }
               }

               NotificationEntry = NotificationEntry->NextNotificationEntryPtr;
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
               /*          for ANP events and Data Events.              */
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
                        (*CallbackInfoArrayPtr[Index].EventCallback)(ANPMEventData, CallbackInfoArrayPtr[Index].CallbackParameter);
                     }
                  }
                  __BTPSEXCEPT(1)
                  {
                     /* Do Nothing.                                     */
                  }
               }
               else
               {
                  /* Walk the proceding list and see if we have already */
                  /* dispatched this event to this client.              */
                  for(Index1=0;Index1<Index;Index1++)
                  {
                     if((CallbackInfoArrayPtr[Index1].ClientID != ServerID) && (CallbackInfoArrayPtr[Index1].ClientID == CallbackInfoArrayPtr[Index].ClientID))
                        break;
                  }

                  if(Index1 == Index)
                  {
                     /* Dispatch the Message.                           */
                     Message->MessageHeader.AddressID = CallbackInfoArrayPtr[Index].ClientID;

                     MSG_SendMessage(Message);
                  }
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

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch the specified ANP event to specified ANP Event Callback  */
   /* * NOTE * This function should be called with the ANP Manager Lock */
   /*          held.                                                    */
static void DispatchANPMEventByID(unsigned int CallbackID, ANPM_Event_Data_t *ANPMEventData, BTPM_Message_t *Message)
{
   void                       *CallbackParameter;
   unsigned int                ClientID;
   ANPM_Event_Callback_t       EventCallback;
   ANPM_Event_Callback_Info_t *CallbackInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the parameters seem valid.                              */
   if((CallbackID) && (ANPMEventData) && (Message))
   {
      /* Attempt to find the callback entry.                            */
      if((CallbackInfo = SearchEventCallbackInfoEntry(&ClientEventCallbackInfoList, CallbackID)) != NULL)
      {
         /* Note the callback information before we release the lock.   */
         ClientID          = CallbackInfo->ClientID;
         EventCallback     = CallbackInfo->EventCallback;
         CallbackParameter = CallbackInfo->CallbackParameter;

         /* Now release the lock.                                       */
         DEVM_ReleaseLock();

         /* Determine if this is a local or remote callback.            */
         if(ClientID == MSG_GetServerAddressID())
         {
            /* Dispatch to the local callback.                          */
            __BTPSTRY
            {
               if(EventCallback)
               {
                  (*EventCallback)(ANPMEventData, CallbackParameter);
               }
            }
            __BTPSEXCEPT(1)
            {
               /* Do Nothing.                                           */
            }
         }
         else
         {
            /* Send the IPC message.                                    */
            Message->MessageHeader.AddressID = ClientID;

            MSG_SendMessage(Message);
         }

         /* Re-acquire the lock.                                        */
         DEVM_AcquireLock();
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to store*/
   /* a Devices' stored configuration for the specified connection entry*/
   /* from the Low Energy Configuration File.                           */
static void StoreConnectionConfiguration(Connection_Entry_t *ConnectionEntry, Boolean_t Store)
{
   char                            TempString[64];
   Connection_Binary_Entry_t       BinaryEntry;
   DEVM_Remote_Device_Properties_t RemoteDeviceProperties;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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
            sprintf(TempString, ANPM_KEY_NAME_CCCD_PREFIX, ConnectionEntry->BD_ADDR.BD_ADDR5, ConnectionEntry->BD_ADDR.BD_ADDR4, ConnectionEntry->BD_ADDR.BD_ADDR3, ConnectionEntry->BD_ADDR.BD_ADDR2, ConnectionEntry->BD_ADDR.BD_ADDR1, ConnectionEntry->BD_ADDR.BD_ADDR0);

            /* Check to see if we are storing something to flash or     */
            /* deleting something.                                      */
            if(Store)
            {
               /* Device is paired so go ahead and store the            */
               /* configuration to flash (if the device has registered  */
               /* for ANP notifications).                               */
               if((ConnectionEntry->New_Alert_Client_Configuration) || (ConnectionEntry->Unread_Alert_Status_Client_Configuration))
               {
                  /* Format the Binary Entry.                           */
                  ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&(BinaryEntry.NewAlertCCCD), (Byte_t)ConnectionEntry->New_Alert_Client_Configuration);
                  ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&(BinaryEntry.UnReadAlertCCCD), (Byte_t)ConnectionEntry->Unread_Alert_Status_Client_Configuration);

                  /* Now write out the new Key-Value Pair.              */
                  SET_WriteBinaryData(ANPM_LE_CONFIGURATION_FILE_SECTION_NAME, TempString, (unsigned char *)&BinaryEntry, sizeof(BinaryEntry), BTPM_CONFIGURATION_LOW_ENERGY_CONFIGURATION_FILE_NAME);
               }
               else
               {
                  DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Deleting Configuration for Device due to no CCCD being configured, Key = %s\n", TempString));

                  /* Delete the configuration stored for this device.   */
                  SET_WriteBinaryData(ANPM_LE_CONFIGURATION_FILE_SECTION_NAME, TempString, NULL, 0, BTPM_CONFIGURATION_LOW_ENERGY_CONFIGURATION_FILE_NAME);
               }
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Deleting Configuration for Device, Key = %s\n", TempString));

               /* Delete the configuration stored for this device.      */
               SET_WriteBinaryData(ANPM_LE_CONFIGURATION_FILE_SECTION_NAME, TempString, NULL, 0, BTPM_CONFIGURATION_LOW_ENERGY_CONFIGURATION_FILE_NAME);
            }
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* reload a Devices' stored configuration for the specified          */
   /* connection entry from the Low Energy Configuration File.          */
static void ReloadConnectionConfiguration(Connection_Entry_t *ConnectionEntry)
{
   char                      TempString[64];
   Connection_Binary_Entry_t BinaryEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if(ConnectionEntry)
   {
      /* Format the Key Name.                                           */
      sprintf(TempString, ANPM_KEY_NAME_CCCD_PREFIX, ConnectionEntry->BD_ADDR.BD_ADDR5, ConnectionEntry->BD_ADDR.BD_ADDR4, ConnectionEntry->BD_ADDR.BD_ADDR3, ConnectionEntry->BD_ADDR.BD_ADDR2, ConnectionEntry->BD_ADDR.BD_ADDR1, ConnectionEntry->BD_ADDR.BD_ADDR0);

      /* Attempt to reload the configuration for this device.           */
      if(SET_ReadBinaryData(ANPM_LE_CONFIGURATION_FILE_SECTION_NAME, TempString, (unsigned char *)&BinaryEntry, sizeof(BinaryEntry), BTPM_CONFIGURATION_LOW_ENERGY_CONFIGURATION_FILE_NAME) == sizeof(Connection_Binary_Entry_t))
      {
         /* Reload the configuration into the connection entry.         */
         ConnectionEntry->New_Alert_Client_Configuration           = READ_UNALIGNED_BYTE_LITTLE_ENDIAN(&(BinaryEntry.NewAlertCCCD));
         ConnectionEntry->Unread_Alert_Status_Client_Configuration = READ_UNALIGNED_BYTE_LITTLE_ENDIAN(&(BinaryEntry.UnReadAlertCCCD));

         DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Reloaded New Alert CCCD:    %u\n", (unsigned int)ConnectionEntry->New_Alert_Client_Configuration));
         DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Reloaded UnRead Alert CCCD: %u\n", (unsigned int)ConnectionEntry->Unread_Alert_Status_Client_Configuration));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to store*/
   /* Persistent UID for the ANS service registered by this module from */
   /* the Low Energy Configuration File.                                */
static void StorePersistentUID(DWord_t PersistentUID, GATT_Attribute_Handle_Group_t *ServiceHandleRange, Boolean_t Store)
{
   char          TempString[64];
   unsigned int  Index;
   unsigned char TempBuffer[ANPM_PERSISTENT_UID_SIZE];

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if(((Store == FALSE) || ((PersistentUID) && (ServiceHandleRange))))
   {
      /* Format the Key Name.                                           */
      BTPS_StringCopy(TempString, ANPM_KEY_NAME_PERSISTENT_UID);

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
         SET_WriteBinaryData(ANPM_LE_CONFIGURATION_FILE_SECTION_NAME, TempString, (unsigned char *)TempBuffer, sizeof(TempBuffer), BTPM_CONFIGURATION_LOW_ENERGY_CONFIGURATION_FILE_NAME);
      }
      else
      {
         /* Delete the configuration stored.                            */
         SET_WriteBinaryData(ANPM_LE_CONFIGURATION_FILE_SECTION_NAME, TempString, NULL, 0, BTPM_CONFIGURATION_LOW_ENERGY_CONFIGURATION_FILE_NAME);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit:\n"));
}

   /* The following function is a utility function that is used to      */
   /* reload the Persistent UID for the ANS service registered by this  */
   /* module from the Low Energy Configuration File.  This function     */
   /* returns TRUE if the Persistent UID was reloaded or false          */
   /* otherwise.                                                        */
static Boolean_t ReloadPersistentUID(DWord_t *PersistentUID, GATT_Attribute_Handle_Group_t *ServiceHandleRange)
{
   char          TempString[64];
   Boolean_t     ret_val = FALSE;
   unsigned int  Index;
   unsigned char TempBuffer[ANPM_PERSISTENT_UID_SIZE];

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if((PersistentUID) && (ServiceHandleRange))
   {
      /* Format the Key Name.                                           */
      BTPS_StringCopy(TempString, ANPM_KEY_NAME_PERSISTENT_UID);

      /* Attempt to reload the configuration for this device.           */
      if(SET_ReadBinaryData(ANPM_LE_CONFIGURATION_FILE_SECTION_NAME, TempString, (unsigned char *)TempBuffer, sizeof(TempBuffer), BTPM_CONFIGURATION_LOW_ENERGY_CONFIGURATION_FILE_NAME) == sizeof(TempBuffer))
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

         DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Persistent UID: 0x%08X\n", (unsigned int)*PersistentUID));
         DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Service Range:  0x%04X - 0x%04X\n", ServiceHandleRange->Starting_Handle, ServiceHandleRange->Ending_Handle));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", (int)ret_val));

   return(ret_val);
}

   /* The following function is a utility function which is used to     */
   /* either load from the configuration file or generate a new Service */
   /* Handle Range for the ANS Service that is registered by this       */
   /* module.  This function returns TRUE if a Handle Range was         */
   /* calculated or FALSE otherwise.                                    */
static Boolean_t CalculateServiceHandleRange(GATT_Attribute_Handle_Group_t *ServiceHandleRange)
{
   DWord_t      PersistentUID;
   Boolean_t    RegisterPersistent;
   Boolean_t    ret_val = FALSE;
   unsigned int NumberOfAttributes;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if(ServiceHandleRange)
   {
      /* Query the number of attributes needed by the service.          */
      if((NumberOfAttributes = _ANPM_Query_Number_Attributes()) > 0)
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

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", (int)ret_val));

   return(ret_val);
}

   /* The following function is a utility function that is used to      */
   /* calculate the number of alerts for a specified category and return*/
   /* the alerts text if required.                                      */
static unsigned int CategoryAlertCount(Boolean_t NewAlerts, ANPM_Category_Identification_t Category, char **LastAlertText)
{
   unsigned int   Count = 0;
   Alert_Entry_t *AlertEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Walk the list and clean up old alerts.                            */
   if(NewAlerts)
      AlertEntry = NewAlertEntryList;
   else
      AlertEntry = UnReadAlertEntryList;

   if(LastAlertText)
      *LastAlertText = NULL;

   /* Walk the list and get counts.                                     */
   while(AlertEntry)
   {
      if(AlertEntry->Category == Category)
      {
         Count += AlertEntry->Count;

         if((NewAlerts) && (AlertEntry->LastAlertText) && (LastAlertText))
            *LastAlertText = AlertEntry->LastAlertText;
      }

      AlertEntry = AlertEntry->NextAlertEntryPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %u\n", Count));

   return(Count);
}

   /* The following function is a utility function which is used to     */
   /* cleanup the Alert List of all old alerts.                         */
static void CleanupOldAlerts(ANPM_Category_Identification_t Category, Boolean_t NewAlerts, unsigned int NumberToClean)
{
   Alert_Entry_t *AlertEntry;
   Alert_Entry_t *tmpAlertEntry;
   Alert_Entry_t **ListHead;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Walk the list and clean up old alerts.                            */
   if(NewAlerts)
   {
      ListHead   = &NewAlertEntryList;
      AlertEntry = NewAlertEntryList;
   }
   else
   {
      ListHead   = &UnReadAlertEntryList;
      AlertEntry = UnReadAlertEntryList;
   }

   while((AlertEntry) && (NumberToClean))
   {
      /* Check to see if this is the proper category.                   */
      if(AlertEntry->Category == Category)
      {
         /* Save a pointer to the list entry to delete.                 */
         tmpAlertEntry = AlertEntry;

         /* Advance to the next alert.                                  */
         AlertEntry    = AlertEntry->NextAlertEntryPtr;

         if(tmpAlertEntry->Count < NumberToClean)
            NumberToClean -= tmpAlertEntry->Count;
         else
            NumberToClean = 0;

         /* Delete the entry from the list.                             */
         if((tmpAlertEntry = DeleteAlertEntry(ListHead, tmpAlertEntry->ClientID, tmpAlertEntry->Category)) != NULL)
         {
            /* Free the memory allocated for this entry.                */
            FreeAlertEntryMemory(tmpAlertEntry);
         }
      }
      else
         AlertEntry = AlertEntry->NextAlertEntryPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to send */
   /* notifications of the specified type for the specified category.   */
static void NotifyCategory(Connection_Entry_t *SpecificConnectionEntry, Boolean_t NewAlerts, ANPM_Category_Identification_t Category)
{
   char                          *LastAlertText;
   Word_t                         Mask;
   unsigned int                   CleanupCount;
   unsigned int                   Count;
   Connection_Entry_t            *ConnectionEntry;
   ANS_New_Alert_Data_t           NewAlertData;
   ANS_Un_Read_Alert_Data_t       UnReadAlertData;
   ANS_Category_Identification_t  ANSCategory;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Convert the category to a mask.                                   */
   Mask  = CategoryToMask(Category, NewAlerts, TRUE);

   /* Get the number of alerts for the specified category.              */
   Count = CategoryAlertCount(NewAlerts, Category, &LastAlertText);
   if((Count) && (Mask) && (ConvertPMCategoryType(Category, &ANSCategory)))
   {
      /* Cap the alert count at the maximum.                            */
      if(Count>255)
      {
         CleanupCount = Count - 255;
         Count        = 255;
      }
      else
         CleanupCount = 0;

      /* Configure the propert alert data.                              */
      if(NewAlerts)
      {
         NewAlertData.CategoryID        = ANSCategory;
         NewAlertData.NumberOfNewAlerts = (Byte_t)Count;
         NewAlertData.LastAlertString   = LastAlertText;
      }
      else
      {
         UnReadAlertData.CategoryID           = ANSCategory;
         UnReadAlertData.NumberOfUnreadAlerts = (Byte_t)Count;
      }

      if(SpecificConnectionEntry == NULL)
      {
         /* Walk the connection list and notify any client that is      */
         /* registered for this events.                                 */
         ConnectionEntry = ConnectionEntryList;
         while(ConnectionEntry)
         {
            /* Verify that the CCCD was configured by this Client.      */
            if(((NewAlerts) && (ConnectionEntry->New_Alert_Client_Configuration)) || ((!NewAlerts) && (ConnectionEntry->Unread_Alert_Status_Client_Configuration)))
            {
               /* Check to see that this category was enabled by the    */
               /* client.                                               */
               if(((NewAlerts) && (ConnectionEntry->Enabled_New_Alert_Categories & Mask)) || ((!NewAlerts) && (ConnectionEntry->Enabled_Unread_Alert_Categories & Mask)))
               {
                  /* Send the proper notification.                      */
                  if(NewAlerts)
                     _ANS_New_Alert_Notification(ConnectionEntry->ConnectionID, &NewAlertData);
                  else
                     _ANS_UnRead_Alert_Notification(ConnectionEntry->ConnectionID, &UnReadAlertData);
               }
            }

            ConnectionEntry = ConnectionEntry->NextConnectionEntryPtr;
         }
      }
      else
      {
         /* Verify that the CCCD was configured by this Client.         */
         if(((NewAlerts) && (SpecificConnectionEntry->New_Alert_Client_Configuration)) || ((!NewAlerts) && (SpecificConnectionEntry->Unread_Alert_Status_Client_Configuration)))
         {
            /* Check to see that this category was enabled by the       */
            /* client.                                                  */
            if(((NewAlerts) && (SpecificConnectionEntry->Enabled_New_Alert_Categories & Mask)) || ((!NewAlerts) && (SpecificConnectionEntry->Enabled_Unread_Alert_Categories & Mask)))
            {
               /* Send the proper notification.                         */
               if(NewAlerts)
                  _ANS_New_Alert_Notification(SpecificConnectionEntry->ConnectionID, &NewAlertData);
               else
                  _ANS_UnRead_Alert_Notification(SpecificConnectionEntry->ConnectionID, &UnReadAlertData);
            }
         }
      }

      if(CleanupCount)
         CleanupOldAlerts(Category, NewAlerts, CleanupCount);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* configure a alert.  This function returns ZERO on success or a    */
   /* negative error code.                                              */
int ConfigureAlert(unsigned int ClientID, Boolean_t NewAlert, ANPM_Category_Identification_t CategoryID, unsigned int AlertCount, char *LastAlertText)
{
   int                            ret_val;
   Alert_Entry_t                  AlertEntry;
   Alert_Entry_t                 *AlertEntryPtr;
   ANS_Category_Identification_t  ANSCategoryID;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: Category ID = %u, Alert Count %u\n", (unsigned int)CategoryID, AlertCount));

   /* Check to see if the input parameters appear to be semi-valid.     */
   if(ConvertPMCategoryType(CategoryID, &ANSCategoryID))
   {
      /* Initialize the list entry to add.                              */
      BTPS_MemInitialize(&AlertEntry, 0, sizeof(AlertEntry));
      AlertEntry.ClientID = ClientID;
      AlertEntry.Count    = (AlertCount>255)?255:AlertCount;
      AlertEntry.Category = CategoryID;

      if(NewAlert)
      {
         /* Delete any existing entry for this client.                  */
         if((AlertEntryPtr = DeleteAlertEntry(&NewAlertEntryList, AlertEntry.ClientID, AlertEntry.Category)) != NULL)
         {
            FreeAlertEntryMemory(AlertEntryPtr);

            AlertEntryPtr = NULL;
         }

         /* Attempt to add the entry to the list.                       */
         if(AlertCount)
            AlertEntryPtr = AddAlertEntry(&NewAlertEntryList, &AlertEntry, (LastAlertText?BTPS_StringLength(LastAlertText):0), LastAlertText);
      }
      else
      {
         /* Delete any existing entry for this client.                  */
         if((AlertEntryPtr = DeleteAlertEntry(&UnReadAlertEntryList, AlertEntry.ClientID, AlertEntry.Category)) != NULL)
         {
            FreeAlertEntryMemory(AlertEntryPtr);

            AlertEntryPtr = NULL;
         }

         /* Attempt to add the entry to the list.                       */
         if(AlertCount)
            AlertEntryPtr = AddAlertEntry(&UnReadAlertEntryList, &AlertEntry, 0, NULL);
      }

      /* Add the entry to the list.                                     */
      if((!AlertCount) || (AlertEntryPtr != NULL))
      {
         /* Send new alert notifications to all connected ANP Clients   */
         /* for this category.                                          */
         if(AlertCount)
            NotifyCategory(NULL, NewAlert, CategoryID);

         /* Finally return success to the caller.                       */
         ret_val = 0;
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is a utility function that is used to      */
   /* dispatch a Client Connection Event to all registered callbacks.   */
static void DispatchANPClientConnectionEvent(Connection_Entry_t *ConnectionEntry)
{
   ANPM_Event_Data_t        EventData;
   ANPM_Connected_Message_t Message;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ConnectionEntry)
   {
      /* Flag that a Connection Event has been dispatched for this      */
      /* device.                                                        */
      ConnectionEntry->ClientConnectedDispatched = TRUE;

      /* Format the event that will be dispatched locally.              */
      EventData.EventType                                        = aetANPConnected;
      EventData.EventLength                                      = ANPM_CONNECTED_EVENT_DATA_SIZE;
      EventData.EventData.ConnectedEventData.ConnectionType      = actServer;
      EventData.EventData.ConnectedEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;

      /* Next, format up the Message to dispatch.                       */
      BTPS_MemInitialize(&Message, 0, sizeof(Message));

      Message.MessageHeader.AddressID       = 0;
      Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
      Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ALERT_NOTIFICATION_MANAGER;
      Message.MessageHeader.MessageFunction = ANPM_MESSAGE_FUNCTION_CONNECTED;
      Message.MessageHeader.MessageLength   = (ANPM_CONNECTED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

      Message.ConnectionType                = actServer;
      Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;

      /* Dispatch the event to all registered callbacks.                */
      DispatchANPMEvent(TRUE, &EventData, (BTPM_Message_t *)&Message);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* dispatch a Client Disconnection Event to all registered callbacks.*/
static void DispatchANPClientDisconnectionEvent(Connection_Entry_t *ConnectionEntry)
{
   ANPM_Event_Data_t           EventData;
   ANPM_Disconnected_Message_t Message;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ConnectionEntry)
   {
      /* Format the event that will be dispatched locally.              */
      EventData.EventType                                           = aetANPDisconnected;
      EventData.EventLength                                         = ANPM_DISCONNECTED_EVENT_DATA_SIZE;
      EventData.EventData.DisconnectedEventData.ConnectionType      = actServer;
      EventData.EventData.DisconnectedEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;

      /* Next, format up the Message to dispatch.                       */
      BTPS_MemInitialize(&Message, 0, sizeof(Message));

      Message.MessageHeader.AddressID       = 0;
      Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
      Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ALERT_NOTIFICATION_MANAGER;
      Message.MessageHeader.MessageFunction = ANPM_MESSAGE_FUNCTION_DISCONNECTED;
      Message.MessageHeader.MessageLength   = (ANPM_DISCONNECTED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

      Message.ConnectionType                = actServer;
      Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;

      /* Dispatch the event to all registered callbacks.                */
      DispatchANPMEvent(TRUE, &EventData, (BTPM_Message_t *)&Message);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* dispatch a New Alert Category (Enabled/Disabled) Event to all     */
   /* registered callbacks.                                             */
static void DispatchANPNewAlertCategoryEvent(Connection_Entry_t *ConnectionEntry, Boolean_t CategoryEnabled, ANPM_Category_Identification_t Category, Word_t CategoryMask)
{
   ANPM_Event_Data_t                          EventData;
   ANPM_New_Alert_Category_Enabled_Message_t  EnabledMessage;
   ANPM_New_Alert_Category_Disabled_Message_t DisabledMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ConnectionEntry)
   {
      /* Determine if a category is being enabled or disabled.          */
      if(CategoryEnabled)
      {
         /* Format the event that will be dispatched locally.           */
         EventData.EventType                                                      = aetANPNewAlertCategoryEnabled;
         EventData.EventLength                                                    = ANPM_NEW_ALERT_CATEGORY_ENABLED_EVENT_DATA_SIZE;
         EventData.EventData.NewAlertCategoryEnabledEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;
         EventData.EventData.NewAlertCategoryEnabledEventData.CategoryEnabled     = Category;
         EventData.EventData.NewAlertCategoryEnabledEventData.EnabledCategories   = CategoryMask;

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&EnabledMessage, 0, sizeof(EnabledMessage));

         EnabledMessage.MessageHeader.AddressID       = 0;
         EnabledMessage.MessageHeader.MessageID       = MSG_GetNextMessageID();
         EnabledMessage.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ALERT_NOTIFICATION_MANAGER;
         EnabledMessage.MessageHeader.MessageFunction = ANPM_MESSAGE_FUNCTION_NEW_ALERT_CATEGORY_ENABLED;
         EnabledMessage.MessageHeader.MessageLength   = (ANPM_NEW_ALERT_CATEGORY_ENABLED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         EnabledMessage.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;
         EnabledMessage.CategoryEnabled               = Category;
         EnabledMessage.EnabledCategories             = CategoryMask;

         /* Dispatch the New Alert Category enabled event to all        */
         /* registered callbacks.                                       */
         DispatchANPMEvent(TRUE, &EventData, (BTPM_Message_t *)&EnabledMessage);
      }
      else
      {
         /* Format the event that will be dispatched locally.           */
         EventData.EventType                                                       = aetANPNewAlertCategoryDisabled;
         EventData.EventLength                                                     = ANPM_NEW_ALERT_CATEGORY_DISABLED_EVENT_DATA_SIZE;
         EventData.EventData.NewAlertCategoryDisabledEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;
         EventData.EventData.NewAlertCategoryDisabledEventData.CategoryDisabled    = Category;
         EventData.EventData.NewAlertCategoryDisabledEventData.EnabledCategories   = CategoryMask;

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&DisabledMessage, 0, sizeof(DisabledMessage));

         DisabledMessage.MessageHeader.AddressID       = 0;
         DisabledMessage.MessageHeader.MessageID       = MSG_GetNextMessageID();
         DisabledMessage.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ALERT_NOTIFICATION_MANAGER;
         DisabledMessage.MessageHeader.MessageFunction = ANPM_MESSAGE_FUNCTION_NEW_ALERT_CATEGORY_DISABLED;
         DisabledMessage.MessageHeader.MessageLength   = (ANPM_NEW_ALERT_CATEGORY_DISABLED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         DisabledMessage.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;
         DisabledMessage.CategoryDisabled              = Category;
         DisabledMessage.EnabledCategories             = CategoryMask;

         /* Dispatch the New Alert Category enabled event to all        */
         /* registered callbacks.                                       */
         DispatchANPMEvent(TRUE, &EventData, (BTPM_Message_t *)&DisabledMessage);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* dispatch a Un-Read Alert Category (Enabled/Disabled) Event to all */
   /* registered callbacks.                                             */
static void DispatchANPUnReadAlertCategoryEvent(Connection_Entry_t *ConnectionEntry, Boolean_t CategoryEnabled, ANPM_Category_Identification_t Category, Word_t CategoryMask)
{
   ANPM_Event_Data_t                              EventData;
   ANPM_Un_Read_Alert_Category_Enabled_Message_t  EnabledMessage;
   ANPM_Un_Read_Alert_Category_Disabled_Message_t DisabledMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ConnectionEntry)
   {
      /* Determine if a category is being enabled or disabled.          */
      if(CategoryEnabled)
      {
         /* Format the event that will be dispatched locally.           */
         EventData.EventType                                                         = aetANPUnReadAlertCategoryEnabled;
         EventData.EventLength                                                       = ANPM_UN_READ_ALERT_CATEGORY_ENABLED_EVENT_DATA_SIZE;
         EventData.EventData.UnReadAlertCategoryEnabledEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;
         EventData.EventData.UnReadAlertCategoryEnabledEventData.CategoryEnabled     = Category;
         EventData.EventData.UnReadAlertCategoryEnabledEventData.EnabledCategories   = CategoryMask;

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&EnabledMessage, 0, sizeof(EnabledMessage));

         EnabledMessage.MessageHeader.AddressID       = 0;
         EnabledMessage.MessageHeader.MessageID       = MSG_GetNextMessageID();
         EnabledMessage.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ALERT_NOTIFICATION_MANAGER;
         EnabledMessage.MessageHeader.MessageFunction = ANPM_MESSAGE_FUNCTION_UN_READ_ALERT_CATEGORY_ENABLED;
         EnabledMessage.MessageHeader.MessageLength   = (ANPM_UN_READ_ALERT_CATEGORY_ENABLED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         EnabledMessage.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;
         EnabledMessage.CategoryEnabled               = Category;
         EnabledMessage.EnabledCategories             = CategoryMask;

         /* Dispatch the New Alert Category enabled event to all        */
         /* registered callbacks.                                       */
         DispatchANPMEvent(TRUE, &EventData, (BTPM_Message_t *)&EnabledMessage);
      }
      else
      {
         /* Format the event that will be dispatched locally.           */
         EventData.EventType                                                          = aetANPUnReadAlertCategoryDisabled;
         EventData.EventLength                                                        = ANPM_UN_READ_ALERT_CATEGORY_DISABLED_EVENT_DATA_SIZE;
         EventData.EventData.UnReadAlertCategoryDisabledEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;
         EventData.EventData.UnReadAlertCategoryDisabledEventData.CategoryDisabled    = Category;
         EventData.EventData.UnReadAlertCategoryDisabledEventData.EnabledCategories   = CategoryMask;

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&DisabledMessage, 0, sizeof(DisabledMessage));

         DisabledMessage.MessageHeader.AddressID       = 0;
         DisabledMessage.MessageHeader.MessageID       = MSG_GetNextMessageID();
         DisabledMessage.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ALERT_NOTIFICATION_MANAGER;
         DisabledMessage.MessageHeader.MessageFunction = ANPM_MESSAGE_FUNCTION_UN_READ_ALERT_CATEGORY_DISABLED;
         DisabledMessage.MessageHeader.MessageLength   = (ANPM_UN_READ_ALERT_CATEGORY_DISABLED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         DisabledMessage.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;
         DisabledMessage.CategoryDisabled              = Category;
         DisabledMessage.EnabledCategories             = CategoryMask;

         /* Dispatch the New Alert Category enabled event to all        */
         /* registered callbacks.                                       */
         DispatchANPMEvent(TRUE, &EventData, (BTPM_Message_t *)&DisabledMessage);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* Dispatch an event for a remote ANP Server connection to the       */
   /* appropriate registered event callbacks.                           */
static void DispatchANPServerConnectionEvent(Client_Connection_Entry_t *ConnectionEntry)
{
   ANPM_Event_Data_t        EventData;
   ANPM_Connected_Message_t Message;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ConnectionEntry)
   {
      /* Format the event that will be dispatched locally.              */
      EventData.EventType                                        = aetANPConnected;
      EventData.EventLength                                      = ANPM_CONNECTED_EVENT_DATA_SIZE;
      EventData.EventData.ConnectedEventData.ConnectionType      = actClient;
      EventData.EventData.ConnectedEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;

      /* Next, format up the Message to dispatch.                       */
      BTPS_MemInitialize(&Message, 0, sizeof(Message));

      Message.MessageHeader.AddressID       = 0;
      Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
      Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ALERT_NOTIFICATION_MANAGER;
      Message.MessageHeader.MessageFunction = ANPM_MESSAGE_FUNCTION_CONNECTED;
      Message.MessageHeader.MessageLength   = (ANPM_CONNECTED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

      Message.ConnectionType                = actClient;
      Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;

      /* Dispatch the event to all registered callbacks.                */
      DispatchANPMEvent(FALSE, &EventData, (BTPM_Message_t *)&Message);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* Dispatch an event for a remote ANP Serer disconnection to the     */
   /* appropriate registered event callbacks.                           */
static void DispatchANPServerDisconnectionEvent(Client_Connection_Entry_t *ConnectionEntry)
{
   ANPM_Event_Data_t           EventData;
   ANPM_Disconnected_Message_t Message;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ConnectionEntry)
   {
      /* Format the event that will be dispatched locally.              */
      EventData.EventType                                           = aetANPDisconnected;
      EventData.EventLength                                         = ANPM_DISCONNECTED_EVENT_DATA_SIZE;
      EventData.EventData.DisconnectedEventData.ConnectionType      = actClient;
      EventData.EventData.DisconnectedEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;

      /* Next, format up the Message to dispatch.                       */
      BTPS_MemInitialize(&Message, 0, sizeof(Message));

      Message.MessageHeader.AddressID       = 0;
      Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
      Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ALERT_NOTIFICATION_MANAGER;
      Message.MessageHeader.MessageFunction = ANPM_MESSAGE_FUNCTION_DISCONNECTED;
      Message.MessageHeader.MessageLength   = (ANPM_DISCONNECTED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

      Message.ConnectionType                = actClient;
      Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;

      /* Dispatch the event to all registered callbacks.                */
      DispatchANPMEvent(FALSE, &EventData, (BTPM_Message_t *)&Message);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* Dispatch an event for a Get Supported New Alert Categories        */
   /* response to the appropriate registered event callbacks.           */
static void DispatchGetSupportedNewAlertCategoriesResultEvent(Client_Connection_Entry_t *ConnectionEntry, unsigned int CallbackID, unsigned int TransactionID, unsigned int Status, unsigned int AttProtocolErrorCode, unsigned long SupportedCategories)
{
   ANPM_Event_Data_t                                    EventData;
   ANPM_Supported_New_Alert_Categories_Result_Message_t Message;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ConnectionEntry)
   {
      /* Format the event that will be dispatched locally.              */
      EventData.EventType                                                           = aetANPSupportedNewAlertCategoriesResult;
      EventData.EventLength                                                         = ANPM_SUPPORTED_NEW_ALERT_CATEGORIES_RESULT_EVENT_DATA_SIZE;

      EventData.EventData.SupportedNewAlertCategoriesEventData.RemoteDeviceAddress  = ConnectionEntry->BD_ADDR;
      EventData.EventData.SupportedNewAlertCategoriesEventData.TransactionID        = TransactionID;
      EventData.EventData.SupportedNewAlertCategoriesEventData.Status               = Status;
      EventData.EventData.SupportedNewAlertCategoriesEventData.AttProtocolErrorCode = AttProtocolErrorCode;
      EventData.EventData.SupportedNewAlertCategoriesEventData.SupportedCategories  = SupportedCategories;

      /* Next, format up the Message to dispatch.                       */
      BTPS_MemInitialize(&Message, 0, sizeof(Message));

      Message.MessageHeader.AddressID       = 0;
      Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
      Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ALERT_NOTIFICATION_MANAGER;
      Message.MessageHeader.MessageFunction = ANPM_MESSAGE_FUNCTION_SUPPORTED_NEW_ALERT_CATEGORIES_RESULT;
      Message.MessageHeader.MessageLength   = (ANPM_SUPPORTED_NEW_ALERT_CATEGORIES_RESULT_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

      Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;
      Message.TransactionID                 = TransactionID;
      Message.Status                        = Status;
      Message.AttProtocolErrorCode          = AttProtocolErrorCode;
      Message.SupportedCategories           = SupportedCategories;

      /* Dispatch the event to all registered callbacks.                */
      DispatchANPMEventByID(CallbackID, &EventData, (BTPM_Message_t *)&Message);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* Dispatch an event for a Get Supported Unread Status Categories    */
   /* response to the appropriate registered event callbacks.           */
static void DispatchGetSupportedUnreadStatusCategoriesResultEvent(Client_Connection_Entry_t *ConnectionEntry, unsigned int CallbackID, unsigned int TransactionID, unsigned int Status, unsigned int AttProtocolErrorCode, unsigned long SupportedCategories)
{
   ANPM_Event_Data_t                                 EventData;
   ANPM_Supported_Unread_Categories_Result_Message_t Message;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ConnectionEntry)
   {
      /* Format the event that will be dispatched locally.              */
      EventData.EventType                                                         = aetANPSupportedUnreadCategoriesResult;
      EventData.EventLength                                                       = ANPM_SUPPORTED_UNREAD_CATEGORIES_RESULT_EVENT_DATA_SIZE;
      EventData.EventData.SupportedUnreadCategoriesEventData.RemoteDeviceAddress  = ConnectionEntry->BD_ADDR;
      EventData.EventData.SupportedUnreadCategoriesEventData.TransactionID        = TransactionID;
      EventData.EventData.SupportedUnreadCategoriesEventData.Status               = Status;
      EventData.EventData.SupportedUnreadCategoriesEventData.AttProtocolErrorCode = AttProtocolErrorCode;
      EventData.EventData.SupportedUnreadCategoriesEventData.SupportedCategories  = SupportedCategories;

      /* Next, format up the Message to dispatch.                       */
      BTPS_MemInitialize(&Message, 0, sizeof(Message));

      Message.MessageHeader.AddressID       = 0;
      Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
      Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ALERT_NOTIFICATION_MANAGER;
      Message.MessageHeader.MessageFunction = ANPM_MESSAGE_FUNCTION_SUPPORTED_UNREAD_CATEGORIES_RESULT;
      Message.MessageHeader.MessageLength   = (ANPM_SUPPORTED_UNREAD_CATEGORIES_RESULT_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

      Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;
      Message.TransactionID                 = TransactionID;
      Message.Status                        = Status;
      Message.AttProtocolErrorCode          = AttProtocolErrorCode;
      Message.SupportedCategories           = SupportedCategories;

      /* Dispatch the event to all registered callbacks.                */
      DispatchANPMEventByID(CallbackID, &EventData, (BTPM_Message_t *)&Message);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* Dispatch an event for a New Alert Notification to the appropriate */
   /* registered event callbacks.                                       */
static void DispatchNewAlertNotificationEvent(Client_Connection_Entry_t *ConnectionEntry, ANPM_Category_Identification_t CategoryID, unsigned int NumberNewAlerts, char *LastAlertText)
{
   ANPM_Event_Data_t                      EventData;
   unsigned int                           StringLength;
   ANPM_New_Alert_Notification_Message_t *Message;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ConnectionEntry)
   {
      if(LastAlertText)
         StringLength = BTPS_StringLength(LastAlertText) + 1;
      else
         StringLength = 0;

      /* Format the event that will be dispatched locally.              */
      EventData.EventType                                                   = aetANPNewAlertNotification;
      EventData.EventLength                                                 = ANPM_NEW_ALERT_NOTIFICATION_EVENT_DATA_SIZE;

      EventData.EventData.NewAlertNotificationEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;
      EventData.EventData.NewAlertNotificationEventData.CategoryID          = CategoryID;
      EventData.EventData.NewAlertNotificationEventData.NumberNewAlerts     = NumberNewAlerts;
      EventData.EventData.NewAlertNotificationEventData.LastAlertText       = LastAlertText;

      /* Next, format up the Message to dispatch.                       */
      if((Message = (ANPM_New_Alert_Notification_Message_t *)BTPS_AllocateMemory(ANPM_NEW_ALERT_NOTIFICATION_MESSAGE_SIZE(StringLength))) != NULL)
      {
         BTPS_MemInitialize(Message, 0, ANPM_NEW_ALERT_NOTIFICATION_MESSAGE_SIZE(StringLength));

         Message->MessageHeader.AddressID       = 0;
         Message->MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ALERT_NOTIFICATION_MANAGER;
         Message->MessageHeader.MessageFunction = ANPM_MESSAGE_FUNCTION_NEW_ALERT_NOTIFICATION;
         Message->MessageHeader.MessageLength   = (ANPM_NEW_ALERT_NOTIFICATION_MESSAGE_SIZE(StringLength) - BTPM_MESSAGE_HEADER_SIZE);

         Message->RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;
         Message->CategoryID                    = CategoryID;
         Message->NumberNewAlerts               = NumberNewAlerts;
         Message->LastAlertTextLength           = StringLength;

         if(StringLength)
         {
            BTPS_MemCopy(Message->LastAlertText, LastAlertText, StringLength);
            Message->LastAlertText[StringLength-1] = '\0';
         }


         /* Dispatch the event to all registered callbacks.             */
         DispatchANPMNotificationEvent(ConnectionEntry->NewAlertNotificationList, ConnectionEntry->NewAlertCategoryList, &EventData, (BTPM_Message_t *)Message, CategoryID);

         BTPS_FreeMemory(Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* Dispatch an event for a Unread Status Notification to the         */
   /* appropriate registered event callbacks.                           */
static void DispatchUnreadStatusNotificationEvent(Client_Connection_Entry_t *ConnectionEntry, ANPM_Category_Identification_t CategoryID, unsigned int NumberUnreadAlerts)
{
   ANPM_Event_Data_t                         EventData;
   ANPM_Unread_Status_Notification_Message_t Message;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ConnectionEntry)
   {
      /* Format the event that will be dispatched locally.              */
      EventData.EventType                                                       = aetANPUnreadStatusNotification;
      EventData.EventLength                                                     = ANPM_UNREAD_STATUS_NOTIFICATION_EVENT_DATA_SIZE;
      EventData.EventData.UnreadStatusNotificationEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;
      EventData.EventData.UnreadStatusNotificationEventData.CategoryID          = CategoryID;
      EventData.EventData.UnreadStatusNotificationEventData.NumberUnreadAlerts  = NumberUnreadAlerts;

      /* Next, format up the Message to dispatch.                       */
      BTPS_MemInitialize(&Message, 0, sizeof(Message));

      Message.MessageHeader.AddressID       = 0;
      Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
      Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ALERT_NOTIFICATION_MANAGER;
      Message.MessageHeader.MessageFunction = ANPM_MESSAGE_FUNCTION_UNREAD_STATUS_NOTIFICATION;
      Message.MessageHeader.MessageLength   = (ANPM_UNREAD_STATUS_NOTIFICATION_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

      Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;
      Message.CategoryID                    = CategoryID;
      Message.NumberUnreadAlerts            = NumberUnreadAlerts;

      /* Dispatch the event to all registered callbacks.                */
      DispatchANPMNotificationEvent(ConnectionEntry->UnreadStatusNotificationList, ConnectionEntry->UnreadStatusCategoryList, &EventData, (BTPM_Message_t *)&Message, CategoryID);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* Dispatch an event for a response to a GATT Write command to the   */
   /* appropriate registered event callbacks.                           */
static void DispatchCommandResultEvent(Client_Connection_Entry_t *ConnectionEntry, Transaction_Entry_t *TransactionEntry, unsigned int Status, unsigned int AttProtocolErrorCode)
{
   ANPM_Event_Data_t              EventData;
   Notification_Entry_t          *NotificationEntry;
   ANPM_Command_Result_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ConnectionEntry)
   {
      /* Format the event that will be dispatched locally.              */
      EventData.EventType                                             = aetANPCommandResult;
      EventData.EventLength                                           = ANPM_COMMAND_RESULT_EVENT_DATA_SIZE;

      EventData.EventData.CommandResultEventData.RemoteDeviceAddress  = ConnectionEntry->BD_ADDR;
      EventData.EventData.CommandResultEventData.TransactionID        = TransactionEntry->TransactionID;
      EventData.EventData.CommandResultEventData.Status               = Status;
      EventData.EventData.CommandResultEventData.AttProtocolErrorCode = AttProtocolErrorCode;

      /* Next, format up the Message to dispatch.                       */
      BTPS_MemInitialize(&Message, 0, sizeof(Message));

      Message.MessageHeader.AddressID       = 0;
      Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
      Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_ALERT_NOTIFICATION_MANAGER;
      Message.MessageHeader.MessageFunction = ANPM_MESSAGE_FUNCTION_COMMAND_RESULT;
      Message.MessageHeader.MessageLength   = (ANPM_COMMAND_RESULT_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

      Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;
      Message.TransactionID                 = TransactionEntry->TransactionID;
      Message.Status                        = Status;
      Message.AttProtocolErrorCode          = AttProtocolErrorCode;

      /* Check whether the completed transaction maps to one callback or*/
      /* can have multiple.                                             */
      if(TransactionEntry->Flags & TRANSACTION_ENTRY_FLAGS_SINGLE_CALLBACK)
         DispatchANPMEventByID(TransactionEntry->SingleCallbackID, &EventData, (BTPM_Message_t *)&Message);
      else
      {
         /* Dispatch the event to all registered callbacks.             */
         for(NotificationEntry=TransactionEntry->PendingResultCallbacks; NotificationEntry; NotificationEntry = NotificationEntry->NextNotificationEntryPtr)
            DispatchANPMEventByID(NotificationEntry->CallbackID, &EventData, (BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static int ProcessRegisterClientEvents(unsigned int ClientID, ANPM_Event_Callback_t EventCallback, void *CallbackParameter)
{
   int ret_val;
   ANPM_Event_Callback_Info_t CallbackInfo;
   ANPM_Event_Callback_Info_t *CallbackInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format the callback entry.                                        */
   BTPS_MemInitialize(&CallbackInfo, 0, sizeof(CallbackInfo));

   CallbackInfo.EventCallbackID   = GetNextEventCallbackID();
   CallbackInfo.ClientID          = ClientID;
   CallbackInfo.EventCallback     = EventCallback;
   CallbackInfo.CallbackParameter = CallbackParameter;

   /* Attempt to add the entry.                                        */
   if((CallbackInfoPtr = AddEventCallbackInfoEntry(&ClientEventCallbackInfoList, &CallbackInfo)) != NULL)
      ret_val = CallbackInfoPtr->EventCallbackID;
   else
      ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

static int ProcessUnRegisterClientEvents(unsigned int ClientID, unsigned int CallbackID)
{
   int ret_val;
   ANPM_Event_Callback_Info_t *CallbackInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure this callback is registered to the client.              */
   if(((CallbackInfo = SearchEventCallbackInfoEntry(&ClientEventCallbackInfoList, CallbackID)) != NULL) && (CallbackInfo->ClientID == ClientID))
   {
      if((CallbackInfo = DeleteEventCallbackInfoEntry(&ClientEventCallbackInfoList, CallbackID)) != NULL)
         FreeEventCallbackInfoEntryMemory(CallbackInfo);

      ret_val = 0;
   }
   else
      ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_INVALID_CALLBACK_ID;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* Utility function for processing a Get Supprted Categories request */
   /* from either a local API or remote IPC call.  This function returns*/
   /* a postive Transaction ID if successful or a negative error code if*/
   /* there was an error.                                               */
static int ProcessGetSupportedCategories(unsigned int ClientID, unsigned int CallbackID, BD_ADDR_t RemoteDeviceAddress, ANPM_Notification_Type_t Type)
{
   int                         ret_val;
   Transaction_Entry_t         TransactionEntry;
   Transaction_Entry_t        *TransactionEntryPtr;
   Client_Connection_Entry_t  *ConnectionEntry;
   ANPM_Event_Callback_Info_t *CallbackInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %d\n", Type));

   /* Make sure the callback is registered.                             */
   if(((CallbackInfo = SearchEventCallbackInfoEntry(&ClientEventCallbackInfoList, CallbackID)) != NULL) && (CallbackInfo->ClientID == ClientID))
   {
      /* Make sure we are tracking the device.                          */
      if((ConnectionEntry = SearchClientConnectionEntry(&ClientConnectionEntryList, RemoteDeviceAddress)) != NULL)
      {
         /* Format and add the transaction entry.                       */
         BTPS_MemInitialize(&TransactionEntry, 0, sizeof(TransactionEntry));

         TransactionEntry.TransactionID    = GetNextTransactionID();
         TransactionEntry.BD_ADDR          = RemoteDeviceAddress;
         TransactionEntry.TransactionType  = (Type == ntNewAlert)?ttReadSupportedNewAlertCategories:ttReadSupportedUnreadStatusCategories;
         TransactionEntry.Flags           |= TRANSACTION_ENTRY_FLAGS_SINGLE_CALLBACK;
         TransactionEntry.SingleCallbackID = CallbackID;

         if((TransactionEntryPtr = AddTransactionEntry(&TransactionEntryList, &TransactionEntry)) != NULL)
         {
            if((ret_val = GATM_ReadValue(GATMCallbackID, RemoteDeviceAddress, (Type == ntNewAlert)?ConnectionEntry->ClientInformation.Supported_New_Alert_Category:ConnectionEntry->ClientInformation.Supported_Unread_Alert_Category, 0, TRUE)) > 0)
            {
               TransactionEntryPtr->GATMTransactionID = (unsigned int)ret_val;
               ret_val                                = TransactionEntryPtr->TransactionID;
            }
            else
            {
               if((TransactionEntryPtr = DeleteTransactionEntry(&TransactionEntryList, TransactionEntryPtr->TransactionID)) != NULL)
                  FreeTransactionEntryMemory(TransactionEntryPtr);
            }
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
      }
      else
         ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_NOT_CONNECTED;
   }
   else
      ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_INVALID_CALLBACK_ID;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* Utility function for processing a Enable Notifications request    */
   /* from either a local API or remote IPC call.  This function returns*/
   /* a positive Transaction ID if successful and a GATT Write was      */
   /* submitted, zero if successful and a GATT write was not necessary, */
   /* or a negative error code if there was an error.                   */
static int ProcessEnableNotifications(unsigned int ClientID, unsigned int CallbackID, BD_ADDR_t RemoteDeviceAddress, ANPM_Notification_Type_t Type)
{
   int                          ret_val = 0;
   Word_t                       Handle;
   Word_t                       Buffer;
   unsigned long                EnabledFlag;
   TransactionType_t            TransactionType;
   Transaction_Entry_t          TransactionEntry;
   Transaction_Entry_t         *TransactionEntryPtr;
   Notification_Entry_t         NotificationEntry;
   Notification_Entry_t        *NotificationEntryPtr;
   Notification_Entry_t       **NotificationListHead;
   Client_Connection_Entry_t   *ConnectionEntry;
   ANPM_Event_Callback_Info_t  *CallbackInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the callback is registered.                             */
   if(((CallbackInfo = SearchEventCallbackInfoEntry(&ClientEventCallbackInfoList, CallbackID)) != NULL) && (CallbackInfo->ClientID == ClientID))
   {
      /* Make sure we are tracking the device.                          */
      if((ConnectionEntry = SearchClientConnectionEntry(&ClientConnectionEntryList, RemoteDeviceAddress)) != NULL)
      {
         /* Note type-specific variables.                               */
         Handle               = (Type == ntNewAlert)?ConnectionEntry->ClientInformation.New_Alert_Client_Configuration:ConnectionEntry->ClientInformation.Unread_Alert_Status_Client_Configuration;
         TransactionType      = (Type == ntNewAlert)?ttEnableNewAlertCCCD:ttEnableUnreadStatusCCCD;
         NotificationListHead = (Type == ntNewAlert)?&ConnectionEntry->NewAlertNotificationList:&ConnectionEntry->UnreadStatusNotificationList;
         EnabledFlag          = (Type == ntNewAlert)?CLIENT_CONNECTION_FLAGS_NEW_ALERT_NOTIFICATIONS_ENABLED:CLIENT_CONNECTION_FLAGS_UNREAD_NOTIFICATIONS_ENABLED;

         /* Format and add the new entry into the notification list.    */
         BTPS_MemInitialize(&NotificationEntry, 0, sizeof(NotificationEntry));

         NotificationEntry.CallbackID = CallbackID;

         if(AddNotificationEntry(NotificationListHead, &NotificationEntry) != NULL)
         {
            /* Check if we have actually enabled notifications on the   */
            /* remote device.                                           */
            if(!(ConnectionEntry->Flags & EnabledFlag))
            {
               /* Check if we already have a write outstanding.         */
               if((TransactionEntryPtr = SearchTransactionEntryByType(&TransactionEntryList, RemoteDeviceAddress, TransactionType)) == NULL)
               {
                  /* No transaction, so format and add it.              */
                  BTPS_MemInitialize(&TransactionEntry, 0, sizeof(TransactionEntry));

                  TransactionEntry.TransactionID   = GetNextTransactionID();
                  TransactionEntry.BD_ADDR         = RemoteDeviceAddress;
                  TransactionEntry.TransactionType = TransactionType;

                  if((TransactionEntryPtr = AddTransactionEntry(&TransactionEntryList, &TransactionEntry)) != NULL)
                  {
                     /* Submit the write request.                       */
                     ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&Buffer, GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_NOTIFY_ENABLE);

                     if((ret_val = GATM_WriteValue(GATMCallbackID, RemoteDeviceAddress, Handle, NON_ALIGNED_WORD_SIZE, (Byte_t *)&Buffer)) > 0)
                     {
                        TransactionEntryPtr->GATMTransactionID = (unsigned int)ret_val;
                        ret_val                                = TransactionEntryPtr->TransactionID;
                     }
                     else
                     {
                        if((TransactionEntryPtr = DeleteTransactionEntry(&TransactionEntryList, TransactionEntryPtr->TransactionID)) != NULL)
                           FreeTransactionEntryMemory(TransactionEntryPtr);
                     }
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
               }
               else
                  ret_val = TransactionEntryPtr->TransactionID;

               if(ret_val >= 0)
               {
                  /* If there was no error, than add a notification     */
                  /* entry to the transaction for the completion        */
                  /* callback.                                          */
                  /* * NOTE * This should only fail if (a)there is      */
                  /*          already an entry and we just return       */
                  /*          success or (b)there is no more            */
                  /*          memory. Either way, there isn't anything  */
                  /*          to do.                                    */
                  AddNotificationEntry(&TransactionEntryPtr->PendingResultCallbacks, &NotificationEntry);
               }
            }
            else
               ret_val = 0;

            /* If there was an error, delete the notification entry we  */
            /* added.                                                   */
            if(ret_val < 0)
            {
               if((NotificationEntryPtr = DeleteNotificationEntry(NotificationListHead, CallbackID)) != NULL)
                  FreeNotificationEntryMemory(NotificationEntryPtr);
            }
         }
         else
            ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_ALREADY_ENABLED;
      }
      else
         ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_NOT_CONNECTED;
   }
   else
      ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_INVALID_CALLBACK_ID;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* Utility function for processing a Disable Notifications request   */
   /* from either a local API or remote IPC call.  This function returns*/
   /* zero if successful or a negative error code if there was an error.*/
static int ProcessDisableNotifications(unsigned int ClientID, unsigned int CallbackID, BD_ADDR_t RemoteDeviceAddress, ANPM_Notification_Type_t Type)
{
   int                          ret_val = 0;
   Word_t                       Handle;
   Word_t                       Buffer;
   unsigned long                EnabledFlag;
   TransactionType_t            TransactionType;
   Transaction_Entry_t          TransactionEntry;
   Transaction_Entry_t         *TransactionEntryPtr;
   Notification_Entry_t         NotificationEntry;
   Notification_Entry_t        *NotificationEntryPtr;
   Notification_Entry_t       **NotificationListHead;
   Client_Connection_Entry_t   *ConnectionEntry;
   ANPM_Event_Callback_Info_t  *CallbackInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the callback is registered.                             */
   if(((CallbackInfo = SearchEventCallbackInfoEntry(&ClientEventCallbackInfoList, CallbackID)) != NULL) && (CallbackInfo->ClientID == ClientID))
   {
      /* Make sure we are tracking the device.                          */
      if((ConnectionEntry = SearchClientConnectionEntry(&ClientConnectionEntryList, RemoteDeviceAddress)) != NULL)
      {
         /* Note type-specific variables.                               */
         Handle               = (Type == ntNewAlert)?ConnectionEntry->ClientInformation.New_Alert_Client_Configuration:ConnectionEntry->ClientInformation.Unread_Alert_Status_Client_Configuration;
         TransactionType      = (Type == ntNewAlert)?ttDisableNewAlertCCCD:ttDisableUnreadStatusCCCD;
         NotificationListHead = (Type == ntNewAlert)?&ConnectionEntry->NewAlertNotificationList:&ConnectionEntry->UnreadStatusNotificationList;
         EnabledFlag          = (Type == ntNewAlert)?CLIENT_CONNECTION_FLAGS_NEW_ALERT_NOTIFICATIONS_ENABLED:CLIENT_CONNECTION_FLAGS_UNREAD_NOTIFICATIONS_ENABLED;

         /* Remove the registration.                                    */
         if((NotificationEntryPtr = DeleteNotificationEntry(NotificationListHead, CallbackID)) != NULL)
         {
            /* Check if this was the last registration to go away.      */
            if((!*NotificationListHead) && (ConnectionEntry->Flags & EnabledFlag))
            {
               /* Check if we already have a write outstanding (we      */
               /* shouldn't).                                           */
               if((TransactionEntryPtr = SearchTransactionEntryByType(&TransactionEntryList, RemoteDeviceAddress, TransactionType)) == NULL)
               {
                  /* No transaction, so format and add it.              */
                  BTPS_MemInitialize(&TransactionEntry, 0, sizeof(TransactionEntry));

                  TransactionEntry.TransactionID   = GetNextTransactionID();
                  TransactionEntry.BD_ADDR         = RemoteDeviceAddress;
                  TransactionEntry.TransactionType = TransactionType;

                  if((TransactionEntryPtr = AddTransactionEntry(&TransactionEntryList, &TransactionEntry)) != NULL)
                  {
                     /* Submit the write request.                       */
                     Buffer = 0;

                     if((ret_val = GATM_WriteValue(GATMCallbackID, RemoteDeviceAddress, Handle, NON_ALIGNED_WORD_SIZE, (Byte_t *)&Buffer)) > 0)
                     {
                        TransactionEntryPtr->GATMTransactionID = (unsigned int)ret_val;
                        ret_val                                = 0;

                        /* Flag that we are not enabled.                */
                        ConnectionEntry->Flags &= ~EnabledFlag;
                     }
                     else
                     {
                        if((TransactionEntryPtr = DeleteTransactionEntry(&TransactionEntryList, TransactionEntryPtr->TransactionID)) != NULL)
                           FreeTransactionEntryMemory(TransactionEntryPtr);
                     }
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
               }

               if(ret_val >= 0)
               {
                  /* If there was no error, than add a notification     */
                  /* entry to the transaction for the completion        */
                  /* callback.                                          */
                  /* * NOTE * This should only fail if (a)there is      */
                  /*          already an entry and we just return       */
                  /*          success or (b)there is no more            */
                  /*          memory. Either way, there isn't anything  */
                  /*          to do.                                    */
                  BTPS_MemInitialize(&NotificationEntry, 0, sizeof(NotificationEntry));

                  NotificationEntry.CallbackID = CallbackID;
                  AddNotificationEntry(&TransactionEntryPtr->PendingResultCallbacks, &NotificationEntry);
               }
            }
            else
               ret_val = 0;

            FreeNotificationEntryMemory(NotificationEntryPtr);
         }
         else
            ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_NOT_ENABLED;
      }
      else
         ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_NOT_CONNECTED;
   }
   else
      ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_INVALID_CALLBACK_ID;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* Utility function for processing a Enable Category request from    */
   /* either a local API or remote IPC call.  This function returns     */
   /* a positive Transaction ID if successful and a GATT Write was      */
   /* submitted, zero if successful and a GATT write was not necessary, */
   /* or a negative error code if there was an error.                   */
static int ProcessEnableCategory(unsigned int ClientID, unsigned int CallbackID, BD_ADDR_t RemoteDeviceAddress, ANPM_Category_Identification_t CategoryID, ANPM_Notification_Type_t Type)
{
   int                                 ret_val;
   unsigned long                       EnabledMask;
   unsigned long                       CategoryMask;
   Category_Entry_t                    CategoryEntry;
   Category_Entry_t                   *CategoryEntryPtr;
   Category_Entry_t                  **CategoryListHead;
   Transaction_Entry_t                 TransactionEntry;
   Transaction_Entry_t                *TransactionEntryPtr;
   Notification_Entry_t                NotificationEntry;
   Client_Connection_Entry_t          *ConnectionEntry;
   ANPM_Event_Callback_Info_t         *CallbackInfo;
   ANS_Control_Point_Command_t         Command;
   ANS_Category_Identification_t       ANSCategory;
   ANS_Control_Point_Command_Value_t   CommandValue;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the callback is registered.                             */
   if(((CallbackInfo = SearchEventCallbackInfoEntry(&ClientEventCallbackInfoList, CallbackID)) != NULL) && (CallbackInfo->ClientID == ClientID))
   {
      /* Make sure we are tracking the device.                          */
      if((ConnectionEntry = SearchClientConnectionEntry(&ClientConnectionEntryList, RemoteDeviceAddress)) != NULL)
      {
         /* Note type-specific variables.                               */
         CategoryListHead = (Type == ntNewAlert)?&ConnectionEntry->NewAlertCategoryList:&ConnectionEntry->UnreadStatusCategoryList;
         EnabledMask      = (Type == ntNewAlert)?ConnectionEntry->EnabledNewAlertCategoryMask:ConnectionEntry->EnabledUnreadStatusCategoryMask;
         Command          = (Type == ntNewAlert)?pcEnable_New_Alert_Notifications:pcEnable_Unread_Category_Notifications;

         /* Format add add the category entry.                          */
         BTPS_MemInitialize(&CategoryEntry, 0, sizeof(CategoryEntry));

         CategoryEntry.CallbackID = CallbackID;

         CategoryMask = CategoryToMask(CategoryID, (Type == ntNewAlert), FALSE);

         if(((CategoryEntryPtr = SearchAddCategoryEntry(CategoryListHead, &CategoryEntry)) != NULL) && ((CategoryEntryPtr->CategoryMask & CategoryMask) != CategoryMask))
         {
            /* Check if we have already enabled this category remotely. */
            if((EnabledMask & CategoryMask) != CategoryMask)
            {
               /* Check if we already have a write outstanding.         */
               if((TransactionEntryPtr = SearchTransactionEntryByPendingCategory(&TransactionEntryList, RemoteDeviceAddress, CategoryID, Type, TRUE)) == NULL)
               {
                  /* We need to send the write, so format the           */
                  /* transaction entry.                                 */
                  BTPS_MemInitialize(&TransactionEntry, 0, sizeof(TransactionEntry));

                  TransactionEntry.TransactionID         = GetNextTransactionID();
                  TransactionEntry.BD_ADDR               = RemoteDeviceAddress;
                  TransactionEntry.TransactionType       = ttWriteControlPoint;
                  TransactionEntry.ControlPointOperation = (Type == ntNewAlert)?cpoEnableNewAlertCategory:cpoEnableUnreadStatusCategory;
                  TransactionEntry.PendingCategoryID     = CategoryID;

                  if((TransactionEntryPtr = AddTransactionEntry(&TransactionEntryList, &TransactionEntry)) != NULL)
                  {
                     /* Attempt to format the command.                  */
                     ConvertPMCategoryType(CategoryID, &ANSCategory);

                     if((ret_val = ANS_Format_Control_Point_Command(&CommandValue, Command, ANSCategory)) == 0)
                     {
                        if((ret_val = GATM_WriteValue(GATMCallbackID, RemoteDeviceAddress, ConnectionEntry->ClientInformation.Control_Point, ANS_CONTROL_POINT_COMMAND_VALUE_DATA_SIZE, (Byte_t *)&CommandValue)) > 0)
                        {
                           TransactionEntryPtr->GATMTransactionID = (unsigned int)ret_val;
                           ret_val                                = TransactionEntryPtr->TransactionID;
                        }
                     }

                     if(ret_val < 0)
                     {
                        if((TransactionEntryPtr = DeleteTransactionEntry(&TransactionEntryList, TransactionEntryPtr->TransactionID)) != NULL)
                           FreeTransactionEntryMemory(TransactionEntryPtr);
                     }
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
               }
               else
                  ret_val = TransactionEntryPtr->TransactionID;

               if(ret_val >= 0)
               {
                  /* If there was no error, than add a notification     */
                  /* entry to the transaction for the completion        */
                  /* callback.                                          */
                  /* * NOTE * This should only fail if (a)there is      */
                  /*          already an entry and we just return       */
                  /*          success or (b)there is no more            */
                  /*          memory. Either way, there isn't anything  */
                  /*          to do.                                    */
                  BTPS_MemInitialize(&NotificationEntry, 0, sizeof(NotificationEntry));

                  NotificationEntry.CallbackID = CallbackID;
                  AddNotificationEntry(&TransactionEntryPtr->PendingResultCallbacks, &NotificationEntry);
               }
            }
            else
               ret_val = 0;

            if(ret_val >= 0)
            {
               /* Note the categories we enabled.                       */
               CategoryEntryPtr->CategoryMask |= CategoryMask;
            }
            else
            {
               /* If there was an error and we just added the entry,    */
               /* delete the category entry we added.                   */
               if(!CategoryEntryPtr->CategoryMask)
               {
                  if((CategoryEntryPtr = DeleteCategoryEntry(CategoryListHead, CallbackID)) != NULL)
                     FreeCategoryEntryMemory(CategoryEntryPtr);
               }
            }
         }
         else
         {
            if(CategoryEntryPtr)
               ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_ALREADY_ENABLED;
            else
               ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
         }
      }
      else
         ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_NOT_CONNECTED;
   }
   else
      ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_INVALID_CALLBACK_ID;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* Utility function for processing a Disable Category request from   */
   /* either a local API or remote IPC call.  This function returns zero*/
   /* if successful or a negative error code if there was an error.     */
static int ProcessDisableCategory(unsigned int ClientID, unsigned int CallbackID, BD_ADDR_t RemoteDeviceAddress, ANPM_Category_Identification_t CategoryID, ANPM_Notification_Type_t Type)
{
   int                                 ret_val;
   unsigned long                      *EnabledMask;
   unsigned long                       CategoryMask;
   Category_Entry_t                   *CategoryEntryPtr;
   Category_Entry_t                  **CategoryListHead;
   Transaction_Entry_t                 TransactionEntry;
   Transaction_Entry_t                *TransactionEntryPtr;
   Client_Connection_Entry_t          *ConnectionEntry;
   ANPM_Event_Callback_Info_t         *CallbackInfo;
   ANS_Control_Point_Command_t         Command;
   ANS_Control_Point_Command_Value_t   CommandValue;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the callback is registered.                             */
   if(((CallbackInfo = SearchEventCallbackInfoEntry(&ClientEventCallbackInfoList, CallbackID)) != NULL) && (CallbackInfo->ClientID == ClientID))
   {
      /* Make sure we are tracking the device.                          */
      if((ConnectionEntry = SearchClientConnectionEntry(&ClientConnectionEntryList, RemoteDeviceAddress)) != NULL)
      {
         /* Note type-specific variables.                               */
         CategoryListHead = (Type == ntNewAlert)?&ConnectionEntry->NewAlertCategoryList:&ConnectionEntry->UnreadStatusCategoryList;
         EnabledMask      = (Type == ntNewAlert)?&ConnectionEntry->EnabledNewAlertCategoryMask:&ConnectionEntry->EnabledUnreadStatusCategoryMask;
         Command          = (Type == ntNewAlert)?pcDisable_New_Alert_Notifications:pcDisable_Unread_Category_Notifications;

         CategoryMask = CategoryToMask(CategoryID, (Type == ntNewAlert), FALSE);

         /* Find the category entry.                                    */
         /* * NOTE * We intentionally accept an enabled mask where any  */
         /*          of the bits match. (i.e. One category is enabled   */
         /*          and the user says disable all.)                    */
         if(((CategoryEntryPtr = SearchCategoryEntry(CategoryListHead, CallbackID)) != NULL) && (CategoryEntryPtr->CategoryMask & CategoryMask))
         {
            /* Remove the category.                                     */
            CategoryEntryPtr->CategoryMask &= ~CategoryMask;

            /* Remove the list entry if there are not more categories.  */
            if((!CategoryEntryPtr->CategoryMask) && ((CategoryEntryPtr = DeleteCategoryEntry(CategoryListHead, CallbackID)) != NULL))
               FreeCategoryEntryMemory(CategoryEntryPtr);

            /* Only disable on the remote device if this is the last    */
            /* registration entry.                                      */
            if(!(*CategoryListHead))
            {
               /* Check if we already have a write outstanding.         */
               if((TransactionEntryPtr = SearchTransactionEntryByPendingCategory(&TransactionEntryList, RemoteDeviceAddress, cmAllSupportedCategories, Type, FALSE)) == NULL)
               {
                  /* We need to send the write, so format the           */
                  /* transaction entry.                                 */
                  BTPS_MemInitialize(&TransactionEntry, 0, sizeof(TransactionEntry));

                  TransactionEntry.TransactionID         = GetNextTransactionID();
                  TransactionEntry.BD_ADDR               = RemoteDeviceAddress;
                  TransactionEntry.TransactionType       = ttWriteControlPoint;
                  TransactionEntry.ControlPointOperation = (Type == ntNewAlert)?cpoDisableNewAlertCategory:cpoDisableUnreadStatusCategory;
                  TransactionEntry.PendingCategoryID     = cmAllSupportedCategories;
                  TransactionEntry.Flags                 = TRANSACTION_ENTRY_FLAGS_SINGLE_CALLBACK;
                  TransactionEntry.SingleCallbackID      = 0;

                  if((TransactionEntryPtr = AddTransactionEntry(&TransactionEntryList, &TransactionEntry)) != NULL)
                  {
                     if((ret_val = ANS_Format_Control_Point_Command(&CommandValue, Command, ciAllCategories)) == 0)
                     {
                        if((ret_val = GATM_WriteValue(GATMCallbackID, RemoteDeviceAddress, ConnectionEntry->ClientInformation.Control_Point, ANS_CONTROL_POINT_COMMAND_VALUE_DATA_SIZE, (Byte_t *)&CommandValue)) > 0)
                        {
                           TransactionEntryPtr->GATMTransactionID = (unsigned int)ret_val;
                           ret_val                                = 0;

                           *EnabledMask                           = 0;
                        }
                     }

                     if(ret_val < 0)
                     {
                        if((TransactionEntryPtr = DeleteTransactionEntry(&TransactionEntryList, TransactionEntryPtr->TransactionID)) != NULL)
                           FreeTransactionEntryMemory(TransactionEntryPtr);
                     }
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
               }
               else
                  ret_val = 0;
            }
            else
               ret_val = 0;
         }
         else
            ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_NOT_ENABLED;
      }
      else
         ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_NOT_CONNECTED;
   }
   else
      ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_INVALID_CALLBACK_ID;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* Utility function for processing a Request Notification request    */
   /* from either a local API or remote IPC call.  This function returns*/
   /* a postive Transaction ID if successful or a negative error code if*/
   /* there was an error.                                               */
static int ProcessRequestNotification(unsigned int ClientID, unsigned int CallbackID, BD_ADDR_t RemoteDeviceAddress, ANPM_Category_Identification_t CategoryID, ANPM_Notification_Type_t Type)
{
   int                                ret_val;
   Transaction_Entry_t                TransactionEntry;
   Transaction_Entry_t               *TransactionEntryPtr;
   Client_Connection_Entry_t         *ConnectionEntry;
   ANPM_Event_Callback_Info_t        *CallbackInfo;
   ANS_Category_Identification_t      ANSCategory;
   ANS_Control_Point_Command_Value_t  CommandValue;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the callback is registered.                             */
   if(((CallbackInfo = SearchEventCallbackInfoEntry(&ClientEventCallbackInfoList, CallbackID)) != NULL) && (CallbackInfo->ClientID == ClientID))
   {
      /* Make sure we are tracking the device.                          */
      if((ConnectionEntry = SearchClientConnectionEntry(&ClientConnectionEntryList, RemoteDeviceAddress)) != NULL)
      {
         /* Attempt to format the Control Point command.                */
         if((ConvertPMCategoryType(CategoryID, &ANSCategory)) && (ANS_Format_Control_Point_Command(&CommandValue, (Type == ntNewAlert)?pcNotify_New_Alert_Immediately:pcNotify_Unread_Category_Immediately, ANSCategory) == 0))
         {
            /* Format and add the transaction entry.                    */
            BTPS_MemInitialize(&TransactionEntry, 0, sizeof(TransactionEntry));

            TransactionEntry.TransactionID         = GetNextTransactionID();
            TransactionEntry.BD_ADDR               = RemoteDeviceAddress;
            TransactionEntry.TransactionType       = ttWriteControlPoint;
            TransactionEntry.ControlPointOperation = (Type == ntNewAlert)?cpoRequestNewAlert:cpoRequestUnreadStatus;
            TransactionEntry.Flags                 = TRANSACTION_ENTRY_FLAGS_SINGLE_CALLBACK;
            TransactionEntry.SingleCallbackID      = CallbackID;

            if((TransactionEntryPtr = AddTransactionEntry(&TransactionEntryList, &TransactionEntry)) != NULL)
            {
               /* Submit the write request.                             */
               if((ret_val = GATM_WriteValue(GATMCallbackID, RemoteDeviceAddress, ConnectionEntry->ClientInformation.Control_Point, ANS_CONTROL_POINT_COMMAND_VALUE_DATA_SIZE, (Byte_t *)&CommandValue)) > 0)
               {
                  TransactionEntryPtr->GATMTransactionID = (unsigned int)ret_val;
                  ret_val                                = TransactionEntryPtr->TransactionID;
               }
               else
               {
                  if((TransactionEntryPtr = DeleteTransactionEntry(&TransactionEntryList, TransactionEntryPtr->TransactionID)) != NULL)
                     FreeTransactionEntryMemory(TransactionEntryPtr);
               }
            }
            else
               ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
         }
         else
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
      else
         ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_NOT_CONNECTED;
   }
   else
      ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_INVALID_CALLBACK_ID;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function processes the specified Set New Alert      */
   /* Request Message and responds to the message accordingly.  This    */
   /* function does not verify the integrity of the Message (i.e.  the  */
   /* length) because it is the caller's responsibility to verify the   */
   /* Message before calling this function.                             */
   /* * NOTE * This function *MUST* be called with the ANP Manager Lock */
   /*          held.                                                    */
static void ProcessSetNewAlertRequestMessage(ANPM_Set_New_Alert_Request_t *Message)
{
   int                            Result;
   char                          *AlertText;
   ANPM_Set_New_Alert_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Configure the alert text.                                      */
      if(Message->LastAlertTextLength)
         AlertText = Message->LastAlertText;
      else
         AlertText = NULL;

      /* Configure the Alert.                                           */
      Result = ConfigureAlert(Message->MessageHeader.AddressID, TRUE, Message->CategoryID, Message->NewAlertCount, AlertText);

      /* Format and send the response message.                          */
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = ANPM_SET_NEW_ALERT_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Set Un-Read Alert  */
   /* Request Message and responds to the message accordingly.  This    */
   /* function does not verify the integrity of the Message (i.e.  the  */
   /* length) because it is the caller's responsibility to verify the   */
   /* Message before calling this function.                             */
   /* * NOTE * This function *MUST* be called with the ANP Manager Lock */
   /*          held.                                                    */
static void ProcessSetUnReadAlertRequestMessage(ANPM_Set_Un_Read_Alert_Request_t *Message)
{
   int                               Result;
   ANPM_Set_Un_Read_Alert_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Configure the Alert.                                           */
      Result = ConfigureAlert(Message->MessageHeader.AddressID, FALSE, Message->CategoryID, Message->UnReadAlertCount, NULL);

      /* Format and send the response message.                          */
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = ANPM_SET_UN_READ_ALERT_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Register ANP Events*/
   /* Request Message and responds to the message accordingly.  This    */
   /* function does not verify the integrity of the Message (i.e.  the  */
   /* length) because it is the caller's responsibility to verify the   */
   /* Message before calling this function.                             */
   /* * NOTE * This function *MUST* be called with the ANP Manager Lock */
   /*          held.                                                    */
static void ProcessRegisterANPEventsRequestMessage(ANPM_Register_ANP_Events_Request_t *Message)
{
   int                                  Result;
   ANPM_Event_Callback_Info_t           EventCallbackEntry;
   ANPM_Register_ANP_Events_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Verify that the client does not have a callback already        */
      /* registered.                                                    */
      if(!SearchEventCallbackInfoEntryByClientID(&EventCallbackInfoList, Message->MessageHeader.AddressID))
      {
         /* Attempt to add an entry into the Event Callback Entry list. */
         BTPS_MemInitialize(&EventCallbackEntry, 0, sizeof(ANPM_Event_Callback_Info_t));

         EventCallbackEntry.EventCallbackID   = GetNextEventCallbackID();
         EventCallbackEntry.ClientID          = Message->MessageHeader.AddressID;
         EventCallbackEntry.EventCallback     = NULL;
         EventCallbackEntry.CallbackParameter = 0;

         if(AddEventCallbackInfoEntry(&EventCallbackInfoList, &EventCallbackEntry))
            Result = 0;
         else
            Result = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
      }
      else
         Result = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;

      /* Format and send the response message.                          */
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = ANPM_REGISTER_ANP_EVENTS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Un-Register ANP    */
   /* Events Request Message and responds to the message accordingly.   */
   /* This function does not verify the integrity of the Message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the Message before calling this function.                         */
   /* * NOTE * This function *MUST* be called with the ANP Manager Lock */
   /*          held.                                                    */
static void ProcessUnRegisterANPEventsRequestMessage(ANPM_Un_Register_ANP_Events_Request_t *Message)
{
   int                                     Result;
   ANPM_Event_Callback_Info_t             *EventCallbackEntryPtr;
   ANPM_Un_Register_ANP_Events_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Attempt to delete the callback specified for this device.      */
      if((EventCallbackEntryPtr = DeleteEventCallbackInfoEntryByClientID(&EventCallbackInfoList, Message->MessageHeader.AddressID)) != NULL)
      {
         /* Free the memory allocated for this event callback.          */
         FreeEventCallbackInfoEntryMemory(EventCallbackEntryPtr);

         /* Return success.                                             */
         Result = 0;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      /* Format and send the response message.                          */
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = ANPM_UN_REGISTER_ANP_EVENTS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Register Client    */
   /* Events Request Message and responds to the message accordingly.   */
   /* This function does not verify the integrity of the Message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the Message before calling this function.                         */
   /* * NOTE * This function *MUST* be called with the ANP Manager Lock */
   /*          held.                                                    */
static void ProcessRegisterClientEventsMessage(ANPM_Register_ANP_Client_Events_Request_t *Message)
{
   int                                        Result;
   ANPM_Register_ANP_Client_Events_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      Result = ProcessRegisterClientEvents(Message->MessageHeader.AddressID, NULL, NULL);

      /* Format and send the response message.                          */
      BTPS_MemInitialize(&ResponseMessage, 0, sizeof(ResponseMessage));

      ResponseMessage.MessageHeader                = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength  = ANPM_REGISTER_ANP_CLIENT_EVENTS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Un Register Client */
   /* Events Request Message and responds to the message accordingly.   */
   /* This function does not verify the integrity of the Message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the Message before calling this function.                         */
   /* * NOTE * This function *MUST* be called with the ANP Manager Lock */
   /*          held.                                                    */
static void ProcessUnRegisterClientEventsMessage(ANPM_Un_Register_ANP_Client_Events_Request_t *Message)
{
   int                                           Result;
   ANPM_Un_Register_ANP_Client_Events_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      Result = ProcessUnRegisterClientEvents(Message->MessageHeader.AddressID, Message->ClientCallbackID);

      /* Format and send the response message.                          */
      BTPS_MemInitialize(&ResponseMessage, 0, sizeof(ResponseMessage));

      ResponseMessage.MessageHeader                = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength  = ANPM_UN_REGISTER_ANP_CLIENT_EVENTS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Get Supported      */
   /* Categories Request Message and responds to the message            */
   /* accordingly.  This function does not verify the integrity         */
   /* of the Message (i.e. the length) because it is the caller's       */
   /* responsibility to verify the Message before calling this function.*/
   /* * NOTE * This function *MUST* be called with the ANP Manager Lock */
   /*          held.                                                    */
static void ProcessGetSupportedCategoriesMessage(ANPM_Get_Supported_Categories_Request_t *Message)
{
   int                                      Result;
   ANPM_Get_Supported_Categories_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      Result = ProcessGetSupportedCategories(Message->MessageHeader.AddressID, Message->ClientCallbackID, Message->RemoteDeviceAddress, Message->NotificationType);

      /* Format and send the response message.                          */
      BTPS_MemInitialize(&ResponseMessage, 0, sizeof(ResponseMessage));

      ResponseMessage.MessageHeader                = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength  = ANPM_GET_SUPPORTED_CATEGORIES_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Enable/Disable     */
   /* Notifications Request Message and responds to the message         */
   /* accordingly.  This function does not verify the integrity         */
   /* of the Message (i.e. the length) because it is the caller's       */
   /* responsibility to verify the Message before calling this function.*/
   /* * NOTE * This function *MUST* be called with the ANP Manager Lock */
   /*          held.                                                    */
static void ProcessEnableDisableNotificationsMessage(ANPM_Enable_Disable_Notifications_Request_t *Message)
{
   int                                          Result;
   ANPM_Enable_Disable_Notifications_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      if(Message->Enable)
         Result = ProcessEnableNotifications(Message->MessageHeader.AddressID, Message->ClientCallbackID, Message->RemoteDeviceAddress, Message->NotificationType);
      else
         Result = ProcessDisableNotifications(Message->MessageHeader.AddressID, Message->ClientCallbackID, Message->RemoteDeviceAddress, Message->NotificationType);

      /* Format and send the response message.                          */
      BTPS_MemInitialize(&ResponseMessage, 0, sizeof(ResponseMessage));

      ResponseMessage.MessageHeader                = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength  = ANPM_ENABLE_DISABLE_NOTIFICATIONS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Enabled/Disable    */
   /* Catgory Request Message and responds to the message accordingly.  */
   /* This function does not verify the integrity of the Message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the Message before calling this function.                         */
   /* * NOTE * This function *MUST* be called with the ANP Manager Lock */
   /*          held.                                                    */
static void ProcessEnableDisableCategoryMessage(ANPM_Enable_Disable_Category_Request_t *Message)
{
   int                                     Result;
   ANPM_Enable_Disable_Category_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      if(Message->Enable)
         Result = ProcessEnableCategory(Message->MessageHeader.AddressID, Message->ClientCallbackID, Message->RemoteDeviceAddress, Message->CategoryID, Message->NotificationType);
      else
         Result = ProcessDisableCategory(Message->MessageHeader.AddressID, Message->ClientCallbackID, Message->RemoteDeviceAddress, Message->CategoryID, Message->NotificationType);

      /* Format and send the response message.                          */
      BTPS_MemInitialize(&ResponseMessage, 0, sizeof(ResponseMessage));

      ResponseMessage.MessageHeader                = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength  = ANPM_ENABLE_DISABLE_CATEGORY_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Request            */
   /* Notification Request Message and responds to the message          */
   /* accordingly.  This function does not verify the integrity         */
   /* of the Message (i.e. the length) because it is the caller's       */
   /* responsibility to verify the Message before calling this function.*/
   /* * NOTE * This function *MUST* be called with the ANP Manager Lock */
   /*          held.                                                    */
static void ProcessRequestNotificationMessage(ANPM_Request_Notification_Request_t *Message)
{
   int                                  Result;
   ANPM_Request_Notification_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      Result = ProcessRequestNotification(Message->MessageHeader.AddressID, Message->ClientCallbackID, Message->RemoteDeviceAddress, Message->CategoryID, Message->NotificationType);

      /* Format and send the response message.                          */
      BTPS_MemInitialize(&ResponseMessage, 0, sizeof(ResponseMessage));

      ResponseMessage.MessageHeader                = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength  = ANPM_REQUEST_NOTIFICATION_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is called from the Main Bluetopia Dispatch */
   /* thread in response to a Process Received Message.  This function  */
   /* SHOULD ONLY be called in this capacity and NOT under any other    */
   /* circumstances.                                                    */
   /* * NOTE * This function *MUST* be called with the ANP Manager      */
   /*          Lock held.  This function will release the Lock before   */
   /*          it exits (i.e. the caller SHOULD NOT RELEASE THE LOCK).  */
static void ProcessReceivedMessage(BTPM_Message_t *Message)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", Message->MessageHeader.MessageFunction));

   /* Check to make sure this isn't some kind of spoofed response (we do*/
   /* not process responses).                                           */
   if((Message) && (!(Message->MessageHeader.MessageID & BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK)))
   {
      /* Request received, go ahead and process it.                     */
      switch(Message->MessageHeader.MessageFunction)
      {
         case ANPM_MESSAGE_FUNCTION_SET_NEW_ALERT:
            DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Set New Alert Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANPM_SET_NEW_ALERT_REQUEST_SIZE(0))
            {
               if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANPM_SET_NEW_ALERT_REQUEST_SIZE(((ANPM_Set_New_Alert_Request_t *)Message)->LastAlertTextLength))
                  ProcessSetNewAlertRequestMessage((ANPM_Set_New_Alert_Request_t *)Message);
               else
               {
                  DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length for specified data\n"));
               }
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case ANPM_MESSAGE_FUNCTION_SET_UN_READ_ALERT:
            DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Set Un-Read Alert Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANPM_SET_UN_READ_ALERT_REQUEST_SIZE)
               ProcessSetUnReadAlertRequestMessage((ANPM_Set_Un_Read_Alert_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case ANPM_MESSAGE_FUNCTION_REGISTER_ANP_EVENTS:
            DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Register ANP Events Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANPM_REGISTER_ANP_EVENTS_REQUEST_SIZE)
               ProcessRegisterANPEventsRequestMessage((ANPM_Register_ANP_Events_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case ANPM_MESSAGE_FUNCTION_UN_REGISTER_ANP_EVENTS:
            DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Un-Register ANP Events Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANPM_UN_REGISTER_ANP_EVENTS_REQUEST_SIZE)
               ProcessUnRegisterANPEventsRequestMessage((ANPM_Un_Register_ANP_Events_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case ANPM_MESSAGE_FUNCTION_REGISTER_ANP_CLIENT_EVENTS:
            DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Register ANP Client Events Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANPM_REGISTER_ANP_CLIENT_EVENTS_REQUEST_SIZE)
               ProcessRegisterClientEventsMessage((ANPM_Register_ANP_Client_Events_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case ANPM_MESSAGE_FUNCTION_UN_REGISTER_ANP_CLIENT_EVENTS:
            DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Un-Register ANP Client Events Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANPM_UN_REGISTER_ANP_CLIENT_EVENTS_REQUEST_SIZE)
               ProcessUnRegisterClientEventsMessage((ANPM_Un_Register_ANP_Client_Events_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case ANPM_MESSAGE_FUNCTION_GET_SUPPORTED_CATEGORIES:
            DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Get Supported Categories Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANPM_GET_SUPPORTED_CATEGORIES_REQUEST_SIZE)
               ProcessGetSupportedCategoriesMessage((ANPM_Get_Supported_Categories_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case ANPM_MESSAGE_FUNCTION_ENABLE_DISABLE_NOTIFICATIONS:
            DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Enable/Disable Notifications Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANPM_ENABLE_DISABLE_NOTIFICATIONS_REQUEST_SIZE)
               ProcessEnableDisableNotificationsMessage((ANPM_Enable_Disable_Notifications_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case ANPM_MESSAGE_FUNCTION_ENABLE_DISABLE_CATEGORY:
            DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Enable/Disable Category Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANPM_ENABLE_DISABLE_CATEGORY_REQUEST_SIZE)
               ProcessEnableDisableCategoryMessage((ANPM_Enable_Disable_Category_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case ANPM_MESSAGE_FUNCTION_REQUEST_NOTIFICATION:
            DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Request Notification Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANPM_REQUEST_NOTIFICATION_REQUEST_SIZE)
               ProcessRequestNotificationMessage((ANPM_Request_Notification_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         default:
            DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled Message\n"));

            break;
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid or Response Message Received\n"));
   }

   /* Release the Lock because we are finished with it.                 */
   DEVM_ReleaseLock();

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the function that is called to process a*/
   /* client Un-Registration event.  This function will perform any     */
   /* cleanup required for the specified Client.                        */
static void ProcessClientUnRegister(unsigned int ClientID)
{
   ANPM_Event_Callback_Info_t  *EventCallback;
   ANPM_Event_Callback_Info_t  *tmpEventCallback;
   ANPM_Event_Callback_Info_t **ListHead;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", ClientID));

   if((ClientID) && (ClientID != MSG_GetServerAddressID()))
   {
      /* Loop through the event callback list and delete all callbacks  */
      /* registered for this callback.                                  */
      ListHead      = &EventCallbackInfoList;
      EventCallback = EventCallbackInfoList;
      while(EventCallback)
      {
         /* Check to see if the current Client Information is the one   */
         /* that is being un-registered.                                */
         if(EventCallback->ClientID == ClientID)
         {
            /* Note the next Event Callback Entry in the list (we are   */
            /* about to delete the current entry).                      */
            tmpEventCallback = EventCallback->NextANPMEventCallbackInfoPtr;

            /* Go ahead and delete the Event Callback Entry and clean up*/
            /* the resources.                                           */
            if((EventCallback = DeleteEventCallbackInfoEntry(ListHead, EventCallback->EventCallbackID)) != NULL)
            {
               /* If this is the ANP client list, clean out any registration by this client. */
               if(ListHead == &ClientEventCallbackInfoList)
                  CleanupUnregisteredClient(&ClientConnectionEntryList, EventCallback->EventCallbackID);

               /* All finished with the memory so free the entry.       */
               FreeEventCallbackInfoEntryMemory(EventCallback);
            }

            /* Go ahead and set the next Event Callback Entry (past the */
            /* one we just deleted).                                    */
            EventCallback = tmpEventCallback;
         }
         else
            EventCallback = EventCallback->NextANPMEventCallbackInfoPtr;

         if((!EventCallback) && (ListHead == &EventCallbackInfoList))
         {
            ListHead      = &ClientEventCallbackInfoList;
            EventCallback = *ListHead;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* convert an ANS categoriy to a PM Category.  This function returns */
   /* TRUE if successful or a negative error code.                      */
static Boolean_t ConvertCategoryPMType(ANS_Category_Identification_t ANSCategory, ANPM_Category_Identification_t *PMCategoryResult)
{
   Boolean_t ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %u\n", (unsigned int)ANSCategory));

   /* Verify that the input parameter is semi-valid.                    */
   if(PMCategoryResult)
   {
      /* Initialize the return value to success.                        */
      ret_val = TRUE;

      /* Do the conversion.                                             */
      switch(ANSCategory)
      {
         case ciSimpleAlert:
            *PMCategoryResult = cmSimpleAlert;
            break;
         case ciEmail:
            *PMCategoryResult = cmEmail;
            break;
         case ciNews:
            *PMCategoryResult = cmNews;
            break;
         case ciCall:
            *PMCategoryResult = cmCall;
            break;
         case ciMissedCall:
            *PMCategoryResult = cmMissedCall;
            break;
         case ciSMS_MMS:
            *PMCategoryResult = cmSMS_MMS;
            break;
         case ciVoiceMail:
            *PMCategoryResult = cmVoiceMail;
            break;
         case ciSchedule:
            *PMCategoryResult = cmSchedule;
            break;
         case ciHighPriorityAlert:
            *PMCategoryResult = cmHighPriorityAlert;
            break;
         case ciInstantMessage:
            *PMCategoryResult = cmInstantMessage;
            break;
         case ciAllCategories:
            *PMCategoryResult = cmAllSupportedCategories;
            break;
         default:
            ret_val = FALSE;
            break;
      }
   }
   else
      ret_val = FALSE;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %u\n", (unsigned int)ret_val));

   return(ret_val);
}

   /* The following function is a utility function that is used to      */
   /* convert a PM Category Type to an ANS one.                         */
static Boolean_t ConvertPMCategoryType(ANPM_Category_Identification_t ANPMCategory, ANS_Category_Identification_t *CategoryResult)
{
   Boolean_t ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %u\n", (unsigned int)ANPMCategory));

   /* Verify that the input parameter is semi-valid.                    */
   if(CategoryResult)
   {
      /* Initialize the return value to success.                        */
      ret_val = TRUE;

      /* Do the conversion.                                             */
      switch(ANPMCategory)
      {
         case cmSimpleAlert:
            *CategoryResult = ciSimpleAlert;
            break;
         case cmEmail:
            *CategoryResult = ciEmail;
            break;
         case cmNews:
            *CategoryResult = ciNews;
            break;
         case cmCall:
            *CategoryResult = ciCall;
            break;
         case cmMissedCall:
            *CategoryResult = ciMissedCall;
            break;
         case cmSMS_MMS:
            *CategoryResult = ciSMS_MMS;
            break;
         case cmVoiceMail:
            *CategoryResult = ciVoiceMail;
            break;
         case cmSchedule:
            *CategoryResult = ciSchedule;
            break;
         case cmHighPriorityAlert:
            *CategoryResult = ciHighPriorityAlert;
            break;
         case cmInstantMessage:
            *CategoryResult = ciInstantMessage;
            break;
         case cmAllSupportedCategories:
            *CategoryResult = ciAllCategories;
            break;
         default:
            ret_val = FALSE;
            break;
      }
   }
   else
      ret_val = FALSE;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));

   return(ret_val);
}

   /* The following function is a utility function that is used to      */
   /* convert a Category to a Category Mask.                            */
static Word_t CategoryToMask(ANPM_Category_Identification_t Category, Boolean_t NewAlertCategory, Boolean_t Server)
{
   Word_t CategoryMask;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %u\n", (unsigned int)Category));

   switch(Category)
   {
      case cmSimpleAlert:
         CategoryMask = ANPM_ALERT_CATEGORY_BIT_MASK_SIMPLE_ALERT;
         break;
      case cmEmail:
         CategoryMask = ANPM_ALERT_CATEGORY_BIT_MASK_EMAIL;
         break;
      case cmNews:
         CategoryMask = ANPM_ALERT_CATEGORY_BIT_MASK_NEWS;
         break;
      case cmCall:
         CategoryMask = ANPM_ALERT_CATEGORY_BIT_MASK_CALL;
         break;
      case cmMissedCall:
         CategoryMask = ANPM_ALERT_CATEGORY_BIT_MASK_MISSED_CALL;
         break;
      case cmSMS_MMS:
         CategoryMask = ANPM_ALERT_CATEGORY_BIT_MASK_SMS_MMS;
         break;
      case cmVoiceMail:
         CategoryMask = ANPM_ALERT_CATEGORY_BIT_MASK_VOICE_MAIL;
         break;
      case cmSchedule:
         CategoryMask = ANPM_ALERT_CATEGORY_BIT_MASK_SCHEDULE;
         break;
      case cmHighPriorityAlert:
         CategoryMask = ANPM_ALERT_CATEGORY_BIT_MASK_HIGH_PRIORITY_ALERT;
         break;
      case cmInstantMessage:
         CategoryMask = ANPM_ALERT_CATEGORY_BIT_MASK_INSTANT_MESSAGE;
         break;
      case cmAllSupportedCategories:
         if(Server)
         {
            if(NewAlertCategory)
               CategoryMask = SupportedNewAlertCategories;
            else
               CategoryMask = SupportedUnReadAlertCategories;
         }
         else
            CategoryMask = ANPM_ALERT_CATEGORY_BIT_MASK_ALL_CATEGORIES;
         break;
      default:
         CategoryMask = 0;
         break;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: 0x%04X\n", CategoryMask));

   return(CategoryMask);
}

   /* The following function is a utility function that is used to      */
   /* process a Enable Categories Command received from a remote client.*/
static void ProcessANSEnableCategoriesCommand(ANS_Control_Point_Command_Data_t *ControlPointCommand, Word_t SupportedCategories, Word_t *EnabledCategoriesResult)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if((ControlPointCommand) && (EnabledCategoriesResult))
   {
      /* Check if the client wants to enable all supported categories   */
      if(ControlPointCommand->Category == ciAllCategories)
      {
         /* Verify we support at least 1 Category.                      */
         if(SupportedCategories)
         {
            /* Store the enabled categories for this device.            */
            *EnabledCategoriesResult = SupportedCategories;
         }
         else
            DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Client attempting to enable categories, Local Device doesn't support any of requested type.\n"));
      }
      else
      {
         /* Verify that Local Device supports requested category.       */
         if(SupportedCategories & (Word_t)(0x1 << ControlPointCommand->Category))
         {
            /* Enable the Requested Category.                           */
            *EnabledCategoriesResult |= (Word_t)(0x1 << ControlPointCommand->Category);
         }
         else
            DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Client attempting to enable un-supported category: %u.\n", (0x1 << ControlPointCommand->Category)));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a Disable Categories Command received from a remote       */
   /* client.                                                           */
static void ProcessANSDisableCategoriesCommand(ANS_Control_Point_Command_Data_t *ControlPointCommand, Word_t *EnabledCategoriesResult)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if((ControlPointCommand) && (EnabledCategoriesResult))
   {
      /* Check if the client wants to enable all supported categories   */
      if(ControlPointCommand->Category == ciAllCategories)
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Client disabling all categories.\n"));

         /* Disable all categories.                                     */
         *EnabledCategoriesResult  = 0;
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Client disabling category: %u.\n", (0x1 << ControlPointCommand->Category)));

         /* Disable the selected category for this client.              */
         *EnabledCategoriesResult &= (Word_t)(~(0x1 << ControlPointCommand->Category));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a Notify New Alerts Immediately command from the specified*/
   /* Client connection.                                                */
static void ProcessNotifyNewAlertsImmediatelyCommand(Connection_Entry_t *ConnectionEntry, ANS_Control_Point_Command_Data_t *ControlPointCommand)
{
   ANPM_Category_Identification_t CategoryID;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if((ConnectionEntry) && (ControlPointCommand))
   {
      /* Convert the ANS category to a PM Category.                     */
      if(ConvertCategoryPMType(ControlPointCommand->Category, &CategoryID))
      {
         /* Check to see if all categories are enabled.                 */
         if(CategoryID == cmAllSupportedCategories)
         {
            /* Notify all categories.  Note that the underlying code    */
            /* will ensure that the category is enabled by the remote   */
            /* deivce and also that it is supported by the local device.*/
            NotifyCategory(ConnectionEntry, TRUE, cmSimpleAlert);
            NotifyCategory(ConnectionEntry, TRUE, cmEmail);
            NotifyCategory(ConnectionEntry, TRUE, cmNews);
            NotifyCategory(ConnectionEntry, TRUE, cmCall);
            NotifyCategory(ConnectionEntry, TRUE, cmMissedCall);
            NotifyCategory(ConnectionEntry, TRUE, cmSMS_MMS);
            NotifyCategory(ConnectionEntry, TRUE, cmVoiceMail);
            NotifyCategory(ConnectionEntry, TRUE, cmSchedule);
            NotifyCategory(ConnectionEntry, TRUE, cmHighPriorityAlert);
            NotifyCategory(ConnectionEntry, TRUE, cmInstantMessage);
         }
         else
         {
            /* Just notify the selected category.                       */
            NotifyCategory(ConnectionEntry, TRUE, CategoryID);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a Notify Un-Read Alerts Immediately command from the      */
   /* specified Client connection.                                      */
static void ProcessNotifyUnReadAlertsImmediatelyCommand(Connection_Entry_t *ConnectionEntry, ANS_Control_Point_Command_Data_t *ControlPointCommand)
{
   ANPM_Category_Identification_t CategoryID;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if((ConnectionEntry) && (ControlPointCommand))
   {
      /* Convert the ANS category to a PM Category.                     */
      if(ConvertCategoryPMType(ControlPointCommand->Category, &CategoryID))
      {
         /* Check to see if all categories are enabled.                 */
         if(CategoryID == cmAllSupportedCategories)
         {
            /* Notify all categories.  Note that the underlying code    */
            /* will ensure that the category is enabled by the remote   */
            /* deivce and also that it is supported by the local device.*/
            NotifyCategory(ConnectionEntry, FALSE, cmSimpleAlert);
            NotifyCategory(ConnectionEntry, FALSE, cmEmail);
            NotifyCategory(ConnectionEntry, FALSE, cmNews);
            NotifyCategory(ConnectionEntry, FALSE, cmCall);
            NotifyCategory(ConnectionEntry, FALSE, cmMissedCall);
            NotifyCategory(ConnectionEntry, FALSE, cmSMS_MMS);
            NotifyCategory(ConnectionEntry, FALSE, cmVoiceMail);
            NotifyCategory(ConnectionEntry, FALSE, cmSchedule);
            NotifyCategory(ConnectionEntry, FALSE, cmHighPriorityAlert);
            NotifyCategory(ConnectionEntry, FALSE, cmInstantMessage);
         }
         else
         {
            /* Just notify the selected category.                       */
            NotifyCategory(ConnectionEntry, FALSE, CategoryID);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a Read Configuration Data request from a remote client.   */
static void ProcessANSReadConfigurationRequest(ANS_Read_Client_Configuration_Data_t *ReadConfigData)
{
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if(ReadConfigData)
   {
      /* Search for the Connection Event for this connection.           */
      if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, ReadConfigData->ConnectionID)) != NULL)
      {
         /* Determine what the client is attempting to read.            */
         switch(ReadConfigData->ClientConfigurationType)
         {
            case ctNewAlert:
               /* Respond with the stored ANS CCCD data for New Alert.  */
               _ANS_Read_Client_Configuration_Response(ReadConfigData->TransactionID, ConnectionEntry->New_Alert_Client_Configuration);
               break;
            case ctUnreadAlertStatus:
               /* Respond with the stored ANS CCCD data for Un-Read     */
               /* Alert.                                                */
               _ANS_Read_Client_Configuration_Response(ReadConfigData->TransactionID, ConnectionEntry->Unread_Alert_Status_Client_Configuration);
               break;
            default:
               DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Unknown CCCD Type: %u\n", (unsigned int)ReadConfigData->ClientConfigurationType));

               _ANS_Read_Client_Configuration_Response(ReadConfigData->TransactionID, FALSE);
               break;
         }
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Unknown Connection ID: %u\n", ReadConfigData->ConnectionID));

         _ANS_Read_Client_Configuration_Response(ReadConfigData->TransactionID, FALSE);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a Client Configuration Update event from a remote client. */
static void ProcessANSClientConfigurationUpdate(ANS_Client_Configuration_Update_Data_t *ClientConfigUpdateData)
{
   Word_t                          CategoryMask;
   Word_t                          SupportedCategoryMask;
   Connection_Entry_t             *ConnectionEntry;
   ANPM_Category_Identification_t  Category;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if(ClientConfigUpdateData)
   {
      /* Search for the Connection Event for this connection.           */
      if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, ClientConfigUpdateData->ConnectionID)) != NULL)
      {
         /* Determine what the client is attempting to read.            */
         switch(ClientConfigUpdateData->ClientConfigurationType)
         {
            case ctNewAlert:
               DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Client %s for New Alert Notifications \n", (ClientConfigUpdateData->NotificationsEnabled?"registering":"un-registering")));

               /* Save the CCCD state for New Alert.                    */
               ConnectionEntry->New_Alert_Client_Configuration = ClientConfigUpdateData->NotificationsEnabled;

               /* Dispatch a Connection Event if necessary.             */
               if((ConnectionEntry->ClientConnectedDispatched == FALSE) && (ConnectionEntry->Enabled_New_Alert_Categories) && (ConnectionEntry->New_Alert_Client_Configuration))
               {
                  /* Dispatch a connection event for this device.       */
                  DispatchANPClientConnectionEvent(ConnectionEntry);

                  /* Flag that the Client Connection Event has been     */
                  /* dispatched.                                        */
                  ConnectionEntry->ClientConnectedDispatched = TRUE;

                  /* Also dispatch a New Alert Enabled Message for this */
                  /* connection.                                        */
                  if(ConnectionEntry->Enabled_New_Alert_Categories == SupportedNewAlertCategories)
                     DispatchANPNewAlertCategoryEvent(ConnectionEntry, TRUE, cmAllSupportedCategories, ConnectionEntry->Enabled_New_Alert_Categories);
                  else
                  {
                     /* If client did not enable all supported          */
                     /* categories dispatch individual events as        */
                     /* necessary.                                      */
                     for(Category=cmSimpleAlert,SupportedCategoryMask=0,CategoryMask=ANPM_ALERT_CATEGORY_BIT_MASK_SIMPLE_ALERT;Category<cmAllSupportedCategories;Category++)
                     {
                        /* Check to see if this category is enabled.    */
                        if(CategoryMask & ConnectionEntry->Enabled_New_Alert_Categories)
                        {
                           /* Update the local supported mask.          */
                           SupportedCategoryMask |= CategoryMask;

                           /* Category is enabled so dispatch a event   */
                           /* for this.                                 */
                           DispatchANPNewAlertCategoryEvent(ConnectionEntry, TRUE, Category, SupportedCategoryMask);
                        }

                        /* Update the category mask.                    */
                        CategoryMask <<= 1;
                     }
                  }
               }
               break;
            case ctUnreadAlertStatus:
               DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Client %s for Un-Read Alert Notifications \n", (ClientConfigUpdateData->NotificationsEnabled?"registering":"un-registering")));

               /* Save the CCCD state for Un-Read Alert.                */
               ConnectionEntry->Unread_Alert_Status_Client_Configuration = ClientConfigUpdateData->NotificationsEnabled;

               /* Dispatch a Connection Event if necessary.             */
               if((ConnectionEntry->ClientConnectedDispatched == FALSE) && (ConnectionEntry->Enabled_Unread_Alert_Categories) && (ConnectionEntry->Unread_Alert_Status_Client_Configuration))
               {
                  /* Dispatch a connection event for this device.       */
                  DispatchANPClientConnectionEvent(ConnectionEntry);

                  /* Flag that the Client Connection Event has been     */
                  /* dispatched.                                        */
                  ConnectionEntry->ClientConnectedDispatched = TRUE;

                  /* Also dispatch a New Alert Enabled Message for this */
                  /* connection.                                        */
                  if(ConnectionEntry->Enabled_Unread_Alert_Categories == SupportedNewAlertCategories)
                     DispatchANPUnReadAlertCategoryEvent(ConnectionEntry, TRUE, cmAllSupportedCategories, ConnectionEntry->Enabled_Unread_Alert_Categories);
                  else
                  {
                     /* If client did not enable all supported          */
                     /* categories dispatch individual events as        */
                     /* necessary.                                      */
                     for(Category=cmSimpleAlert,SupportedCategoryMask=0,CategoryMask=ANPM_ALERT_CATEGORY_BIT_MASK_SIMPLE_ALERT;Category<cmAllSupportedCategories;Category++)
                     {
                        /* Check to see if this category is enabled.    */
                        if(CategoryMask & ConnectionEntry->Enabled_Unread_Alert_Categories)
                        {
                           /* Update the local supported mask.          */
                           SupportedCategoryMask |= CategoryMask;

                           /* Category is enabled so dispatch a event   */
                           /* for this.                                 */
                           DispatchANPUnReadAlertCategoryEvent(ConnectionEntry, TRUE, Category, SupportedCategoryMask);
                        }

                        /* Update the category mask.                    */
                        CategoryMask <<= 1;
                     }
                  }
               }
               break;
            default:
               DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Unknown CCCD Type: %u\n", (unsigned int)ClientConfigUpdateData->ClientConfigurationType));
               break;
         }

         /* Update the configuration file if necessary.                 */
         if(ConnectionEntry->ClientConnectedDispatched)
         {
            /* Check to see if we should store or delete configuration  */
            /* information.                                             */
            if((ConnectionEntry->New_Alert_Client_Configuration) || (ConnectionEntry->Unread_Alert_Status_Client_Configuration))
               StoreConnectionConfiguration(ConnectionEntry, TRUE);
            else
               StoreConnectionConfiguration(ConnectionEntry, FALSE);
         }
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Unknown Connection ID: %u\n", ClientConfigUpdateData->ConnectionID));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a Control Point Command event from a remote client.       */
static void ProcessANSControlPointCommand(ANS_Control_Point_Command_Data_t *ControlPointCommandData)
{
   Word_t                          PreviousMask;
   Connection_Entry_t             *ConnectionEntry;
   ANPM_Category_Identification_t  ANPMCategory;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if(ControlPointCommandData)
   {
      /* Search for the Connection Event for this connection.           */
      if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, ControlPointCommandData->ConnectionID)) != NULL)
      {
         /* Determine what the client is attempting to read.            */
         switch(ControlPointCommandData->Command)
         {
            case pcEnable_New_Alert_Notifications:
               DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Client enabling new alert notifications for category: %u\n", (unsigned int)ControlPointCommandData->Category));

               /* Save the previous mask to know if it has actually     */
               /* changed.                                              */
               PreviousMask = ConnectionEntry->Enabled_New_Alert_Categories;

               /* Process the Enable New Alert Command.                 */
               ProcessANSEnableCategoriesCommand(ControlPointCommandData, SupportedNewAlertCategories, &(ConnectionEntry->Enabled_New_Alert_Categories));

               /* Check to see if we need to dispatch a Client          */
               /* Connection Event for this device.                     */
               if((ConnectionEntry->ClientConnectedDispatched == FALSE) && (ConnectionEntry->Enabled_New_Alert_Categories) && (ConnectionEntry->New_Alert_Client_Configuration))
               {
                  /* Dispatch a connection event for this device.       */
                  DispatchANPClientConnectionEvent(ConnectionEntry);

                  /* Flag that the Client Connection Event has been     */
                  /* dispatched.                                        */
                  ConnectionEntry->ClientConnectedDispatched = TRUE;
               }

               /* Dispatch a New Alert Enabled Event if necessary.  We  */
               /* will not dispatch this before a Client Connection     */
               /* Event has been dispatched.                            */
               if((ConnectionEntry->ClientConnectedDispatched) && (PreviousMask != ConnectionEntry->Enabled_New_Alert_Categories))
               {
                  /* Convert the ANS Category to a ANPM Category.       */
                  if(ConvertCategoryPMType(ControlPointCommandData->Category, &ANPMCategory))
                     DispatchANPNewAlertCategoryEvent(ConnectionEntry, TRUE, ANPMCategory, ConnectionEntry->Enabled_New_Alert_Categories);
               }
               break;
            case pcDisable_New_Alert_Notifications:
               DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Client disabling new alert notifications for category: %u\n", (unsigned int)ControlPointCommandData->Category));

               /* Save the previous mask to know if it has actually     */
               /* changed.                                              */
               PreviousMask = ConnectionEntry->Enabled_New_Alert_Categories;

               /* Process the Disable New Alert Command.                */
               ProcessANSDisableCategoriesCommand(ControlPointCommandData, &(ConnectionEntry->Enabled_New_Alert_Categories));

               /* Dispatch a New Alert Disabled Event if necessary.  We */
               /* will not dispatch this before a Client Connection     */
               /* Event has been dispatched.                            */
               if((ConnectionEntry->ClientConnectedDispatched) && (PreviousMask != ConnectionEntry->Enabled_New_Alert_Categories))
               {
                  /* Convert the ANS Category to a ANPM Category.       */
                  if(ConvertCategoryPMType(ControlPointCommandData->Category, &ANPMCategory))
                     DispatchANPNewAlertCategoryEvent(ConnectionEntry, FALSE, ANPMCategory, ConnectionEntry->Enabled_New_Alert_Categories);
               }
               break;
            case pcEnable_Unread_Category_Notifications:
               DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Client enabling un-alert alert notifications for category: %u\n", (unsigned int)ControlPointCommandData->Category));

               /* Save the previous mask to know if it has actually     */
               /* changed.                                              */
               PreviousMask = ConnectionEntry->Enabled_Unread_Alert_Categories;

               /* Process the Enable Un-Read Alert Command.             */
               ProcessANSEnableCategoriesCommand(ControlPointCommandData, SupportedUnReadAlertCategories, &(ConnectionEntry->Enabled_Unread_Alert_Categories));

               /* Check to see if we need to dispatch a Client          */
               /* Connection Event for this device.                     */
               if((ConnectionEntry->ClientConnectedDispatched == FALSE) && (ConnectionEntry->Enabled_Unread_Alert_Categories) && (ConnectionEntry->Unread_Alert_Status_Client_Configuration))
               {
                  /* Dispatch a connection event for this device.       */
                  DispatchANPClientConnectionEvent(ConnectionEntry);

                  /* Flag that the Client Connection Event has been     */
                  /* dispatched.                                        */
                  ConnectionEntry->ClientConnectedDispatched = TRUE;
               }

               /* Dispatch a Un-Read Enabled Event if necessary.  We    */
               /* will not dispatch this before a Client Connection     */
               /* Event has been dispatched.                            */
               if((ConnectionEntry->ClientConnectedDispatched) && (PreviousMask != ConnectionEntry->Enabled_Unread_Alert_Categories))
               {
                  /* Convert the ANS Category to a ANPM Category.       */
                  if(ConvertCategoryPMType(ControlPointCommandData->Category, &ANPMCategory))
                     DispatchANPUnReadAlertCategoryEvent(ConnectionEntry, TRUE, ANPMCategory, ConnectionEntry->Enabled_Unread_Alert_Categories);
               }
               break;
            case pcDisable_Unread_Category_Notifications:
               DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Client disabling un-alert alert notifications for category: %u\n", (unsigned int)ControlPointCommandData->Category));

               /* Save the previous mask to know if it has actually     */
               /* changed.                                              */
               PreviousMask = ConnectionEntry->Enabled_Unread_Alert_Categories;

               /* Process the Disable Un-Read Alert Command.            */
               ProcessANSDisableCategoriesCommand(ControlPointCommandData, &(ConnectionEntry->Enabled_Unread_Alert_Categories));

               /* Dispatch a Un-Read Disabled Event if necessary.  We   */
               /* will not dispatch this before a Client Connection     */
               /* Event has been dispatched.                            */
               if((ConnectionEntry->ClientConnectedDispatched) && (PreviousMask != ConnectionEntry->Enabled_Unread_Alert_Categories))
               {
                  /* Convert the ANS Category to a ANPM Category.       */
                  if(ConvertCategoryPMType(ControlPointCommandData->Category, &ANPMCategory))
                     DispatchANPUnReadAlertCategoryEvent(ConnectionEntry, FALSE, ANPMCategory, ConnectionEntry->Enabled_Unread_Alert_Categories);
               }
               break;
            case pcNotify_New_Alert_Immediately:
               /* Simply call an internal function to process this      */
               /* command.                                              */
               ProcessNotifyNewAlertsImmediatelyCommand(ConnectionEntry, ControlPointCommandData);
               break;
            case pcNotify_Unread_Category_Immediately:
               /* Simply call an internal function to process this      */
               /* command.                                              */
               ProcessNotifyUnReadAlertsImmediatelyCommand(ConnectionEntry, ControlPointCommandData);
               break;
            default:
               DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Unknown Control Point Command: %u\n", (unsigned int)ControlPointCommandData->Command));
               break;
         }
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Unknown Connection ID: %u\n", ControlPointCommandData->ConnectionID));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process GATT Connection Events.                                   */
static void ProcessGATTConnectEvent(GATT_Device_Connection_Data_t *ConnectionData)
{
   Connection_Entry_t              ConnectionEntry;
   DEVM_Remote_Device_Properties_t RemoteDeviceProperties;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is semi-valid and this is a       */
   /* connection for an LE Device (ANS won't over BR/EDR per the        */
   /* specification).                                                   */
   if((ConnectionData) && (ConnectionData->ConnectionType == gctLE))
   {
      /* Format the entry to add to the list.                           */
      BTPS_MemInitialize(&ConnectionEntry, 0, sizeof(ConnectionEntry));

      ConnectionEntry.ConnectionID              = ConnectionData->ConnectionID;
      ConnectionEntry.BD_ADDR                   = ConnectionData->RemoteDevice;
      ConnectionEntry.ClientConnectedDispatched = FALSE;

      /* Update the address to the resolved address (may have no        */
      /* effect).                                                       */
      if(!DEVM_QueryRemoteDeviceProperties(ConnectionEntry.BD_ADDR, DEVM_QUERY_REMOTE_DEVICE_PROPERTIES_FLAGS_LOW_ENERGY, &RemoteDeviceProperties))
         ConnectionEntry.BD_ADDR = RemoteDeviceProperties.BD_ADDR;

      /* Re-load configuration from file.                               */
      ReloadConnectionConfiguration(&ConnectionEntry);

      /* Add the connection to the connection list.                     */
      AddConnectionEntry(&ConnectionEntryList, &ConnectionEntry);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process GATT Disconnection Events.                                */
static void ProcessGATTDisconnectEvent(GATT_Device_Disconnection_Data_t *DisconnectionData)
{
   Connection_Entry_t *ConnectionEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is semi-valid and this is a       */
   /* disconnection for an LE Device (ANS won't over BR/EDR per the     */
   /* specification).                                                   */
   if((DisconnectionData) && (DisconnectionData->ConnectionType == gctLE))
   {
      /* Delete the connection from the connection list.                */
      if((ConnectionEntryPtr = DeleteConnectionEntry(&ConnectionEntryList, DisconnectionData->ConnectionID)) != NULL)
      {
         /* Dispatch a Device Disconnection if we dispatched a Client   */
         /* Connection.                                                 */
         if(ConnectionEntryPtr->ClientConnectedDispatched)
            DispatchANPClientDisconnectionEvent(ConnectionEntryPtr);

         /* Free the memory that was allocated.                         */
         FreeConnectionEntryMemory(ConnectionEntryPtr);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the function that is responsible for    */
   /* processing ANS Events that have been received.  This function     */
   /* should ONLY be called with the Context locked AND ONLY in the     */
   /* context of an arbitrary processing thread.                        */
static void ProcessANSEvent(ANPM_Server_Event_Data_t *ANSEventData)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First make sure that the input parameter appears to be semi-valid.*/
   if(ANSEventData)
   {
      /* Process the event based on the event type.                     */
      switch(ANSEventData->Event_Data_Type)
      {
         case etANS_Server_Read_Client_Configuration_Request:
            /* Process the Read Client Configuration event.             */
            ProcessANSReadConfigurationRequest(&(ANSEventData->Event_Data.ANS_Read_Client_Configuration_Data));
            break;
         case etANS_Server_Client_Configuration_Update:
            /* Process the Client Configuration Update event.           */
            ProcessANSClientConfigurationUpdate(&(ANSEventData->Event_Data.ANS_Client_Configuration_Update_Data));
            break;
         case etANS_Server_Control_Point_Command_Indication:
            /* Process the Control Point Command event.                 */
            ProcessANSControlPointCommand(&(ANSEventData->Event_Data.ANS_Control_Point_Command_Data));
            break;
         default:
            /* Invalid/Unknown Event.                                   */
            DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid/Unknown ANS Event Type: %d\n", ANSEventData->Event_Data_Type));
            break;
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid ANS Event Data\n"));
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the function that is responsible for    */
   /* processing GATT Connection Events that have been received.  This  */
   /* function should ONLY be called with the Context locked AND ONLY in*/
   /* the context of an arbitrary processing thread.                    */
static void ProcessGATTConnectionEvent(GATM_Connection_Event_Data_t *GATTConnectionEventData)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First make sure that the input parameter appears to be semi-valid.*/
   if(GATTConnectionEventData)
   {
      /* Process the event based on the event type.                     */
      switch(GATTConnectionEventData->Event_Data_Type)
      {
         case etGATT_Connection_Device_Connection:
            /* Process the GATT Connection Event.                       */
            ProcessGATTConnectEvent(&(GATTConnectionEventData->Event_Data.GATT_Device_Connection_Data));
            break;
         case etGATT_Connection_Device_Disconnection:
            /* Process the GATT Disconnection Event.                    */
            ProcessGATTDisconnectEvent(&(GATTConnectionEventData->Event_Data.GATT_Device_Disconnection_Data));
            break;
         default:
            /* Invalid/Unknown Event.                                   */
            DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid/Unknown GATT Connection Event Type: %d\n", GATTConnectionEventData->Event_Data_Type));
            break;
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid GATT Connection Event Data\n"));
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* Utility function which determines which ANS Characteristic a UUID */
   /* represents.                                                       */
static ANS_UUID_t DetermineANSUUID(GATT_UUID_t UUID)
{
   ANS_UUID_t Type = auUnknown;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(UUID.UUID_Type == guUUID_16)
   {
      if(ANS_COMPARE_ANS_SUPORTED_NEW_ALERT_CATEGORY_UUID_TO_UUID_16(UUID.UUID.UUID_16))
         Type = auSupportedNewAlertCategory;
      else
      {
         if(ANS_COMPARE_ANS_NEW_ALERT_UUID_TO_UUID_16(UUID.UUID.UUID_16))
            Type = auNewAlert;
         else
         {
            if(ANS_COMPARE_ANS_SUPPORTED_UNREAD_ALERT_CATEGORY_UUID_TO_UUID_16(UUID.UUID.UUID_16))
               Type = auSupportedUnreadAlertCategory;
            else
            {
               if(ANS_COMPARE_ANS_UNREAD_ALERT_STATUS_UUID_TO_UUID_16(UUID.UUID.UUID_16))
                  Type = auUnreadAlertStatus;
               else
               {
                  if(ANS_COMPARE_ANS_CONTROL_POINT_UUID_TO_UUID_16(UUID.UUID.UUID_16))
                     Type = auControlPoint;
               }
            }
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", Type));

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

   /* Utility function which attempts to discover the ANS services and  */
   /* parse the service information. This function returns TRUE if      */
   /* the ANS service is discovered and successfully parsed into the    */
   /* supplied ClientInformation.                                       */
static Boolean_t DiscoverANS(BD_ADDR_t BD_ADDR, ANS_Client_Information_t *ClientInformation)
{
   int                                       Result;
   Boolean_t                                 ret_val                            = FALSE;
   GATT_UUID_t                               UUID;
   unsigned int                              ServiceDataSize;
   unsigned int                              Index;
   unsigned int                              CharacteristicIndex;
   unsigned char                            *ServiceData;
   ANS_Client_Information_t                  TempClientInformation;
   DEVM_Parsed_Services_Data_t               ParsedGATTData;
   GATT_Characteristic_Information_t        *CharacteristicInformation;
   GATT_Service_Discovery_Indication_Data_t *GATTServiceDiscoveryIndicationData;


   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if((!COMPARE_NULL_BD_ADDR(BD_ADDR)) && (ClientInformation))
   {
      /* Get the buffer size required to parse the service data.        */
      if((Result = DEVM_QueryRemoteDeviceServices(BD_ADDR, DEVM_QUERY_REMOTE_DEVICE_SERVICES_FLAGS_LOW_ENERGY, 0, NULL, &ServiceDataSize)) == 0)
      {
         /* Allocate the buffer required to parse the service data      */
         if((ServiceData = (unsigned char *)BTPS_AllocateMemory(ServiceDataSize)) != NULL)
         {
            /* Query the services for the remote device.                */
            if((Result = DEVM_QueryRemoteDeviceServices(BD_ADDR, DEVM_QUERY_REMOTE_DEVICE_SERVICES_FLAGS_LOW_ENERGY, ServiceDataSize, ServiceData, NULL)) > 0)
            {
               /* Convert the Raw GATT Stream to a Parsed GATT Stream.  */
               if((Result = DEVM_ConvertRawServicesStreamToParsedServicesData(ServiceDataSize, ServiceData, &ParsedGATTData)) == 0)
               {
                  /* Iterate the services on the remote device.         */
                  for(Index=0;Index<ParsedGATTData.NumberServices;Index++)
                  {
                     UUID = ParsedGATTData.GATTServiceDiscoveryIndicationData[Index].ServiceInformation.UUID;

                     /* Check for ANS.                                  */
                     if((UUID.UUID_Type == guUUID_16) && (ANS_COMPARE_ANS_SERVICE_UUID_TO_UUID_16(UUID.UUID.UUID_16)))
                     {
                        GATTServiceDiscoveryIndicationData = &(ParsedGATTData.GATTServiceDiscoveryIndicationData[Index]);
                        BTPS_MemInitialize(&TempClientInformation, 0, sizeof(TempClientInformation));

                        /* Iterate the CSC characteristics.             */
                        for(CharacteristicIndex=0;CharacteristicIndex<GATTServiceDiscoveryIndicationData->NumberOfCharacteristics;CharacteristicIndex++)
                        {
                           CharacteristicInformation = &(GATTServiceDiscoveryIndicationData->CharacteristicInformationList[CharacteristicIndex]);
                           UUID                      = CharacteristicInformation->Characteristic_UUID;

                           switch(DetermineANSUUID(UUID))
                           {
                              case auSupportedNewAlertCategory:
                                 TempClientInformation.Supported_New_Alert_Category = CharacteristicInformation->Characteristic_Handle;
                                 break;
                              case auNewAlert:
                                 TempClientInformation.New_Alert                      = CharacteristicInformation->Characteristic_Handle;
                                 TempClientInformation.New_Alert_Client_Configuration = FindCCCD(CharacteristicInformation);
                                 break;
                              case auSupportedUnreadAlertCategory:
                                 TempClientInformation.Supported_Unread_Alert_Category = CharacteristicInformation->Characteristic_Handle;
                                 break;
                              case auUnreadAlertStatus:
                                 TempClientInformation.Unread_Alert_Status                      = CharacteristicInformation->Characteristic_Handle;
                                 TempClientInformation.Unread_Alert_Status_Client_Configuration = FindCCCD(CharacteristicInformation);
                                 break;
                              case auControlPoint:
                                 TempClientInformation.Control_Point = CharacteristicInformation->Characteristic_Handle;
                                 break;
                              default:
                                 DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Unknown characteristic in ANS Service\n"));
                                 break;
                           }
                        }

                        /* Make sure we found all the characteristics.  */
                        if((TempClientInformation.Supported_New_Alert_Category) && (TempClientInformation.New_Alert) && (TempClientInformation.New_Alert_Client_Configuration) && (TempClientInformation.Supported_Unread_Alert_Category) && (TempClientInformation.Unread_Alert_Status) && (TempClientInformation.Unread_Alert_Status_Client_Configuration) && (TempClientInformation.Control_Point))
                        {
                           ret_val            = TRUE;
                           *ClientInformation = TempClientInformation;
                        }
                     }
                  }

                  /* All finished with the parsed data, so free it.     */
                  DEVM_FreeParsedServicesData(&ParsedGATTData);
               }
               else
               {
                  DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FAILURE), ("DEVM_ConvertRawServicesStreamToParsedServicesData returned %d.\n", Result));
               }
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FAILURE), ("DEVM_QueryRemoteDeviceServices returned %d.\n", Result));
            }

            /* Free the previously allocated buffer holding service data*/
            /* information.                                             */
            BTPS_FreeMemory(ServiceData);
         }
         else
         {
            DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FAILURE), ("Allocation request for Service Data failed, size = %u.\n", ServiceDataSize));
         }
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FAILURE), ("DEVM_QueryRemoteDeviceServices returned %d.\n", Result));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* Processes an LE device connection and determines if it should be  */
   /* processes as an ANS device connection.                            */
static void ProcessANSClientConnection(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties)
{
   Client_Connection_Entry_t  ConnectionEntry;
   Client_Connection_Entry_t *ConnectionEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(RemoteDeviceProperties)
   {
      /* Initialize the connection structure before we use it.          */
      BTPS_MemInitialize(&ConnectionEntry, 0, sizeof(ConnectionEntry));

      ConnectionEntry.BD_ADDR = RemoteDeviceProperties->BD_ADDR;

      /* Check if the device supports ANS.                              */
      if(DiscoverANS(RemoteDeviceProperties->BD_ADDR, &(ConnectionEntry.ClientInformation)))
      {
         /* Attempt to add the Connection Entry.                        */
         if((ConnectionEntryPtr = AddClientConnectionEntry(&ClientConnectionEntryList, &ConnectionEntry)) != NULL)
            DispatchANPServerConnectionEvent(ConnectionEntryPtr);
         else
            DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FAILURE), ("Unable to add entry to list.\n"));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* Processes an LE device disconenction and determines if we are     */
   /* tracking the device and need to process the disconnection.        */
static void ProcessANSClientDisconnection(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties)
{
   Client_Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(RemoteDeviceProperties)
   {
      if((ConnectionEntry = DeleteClientConnectionEntry(&ClientConnectionEntryList, RemoteDeviceProperties->BD_ADDR)) != NULL)
      {
         DispatchANPServerDisconnectionEvent(ConnectionEntry);
         FreeClientConnectionEntryMemory(ConnectionEntry);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the function that is called to process a*/
   /* LE Pairing Change Event.                                          */
   /* * NOTE * This function *MUST* be called with the ANPM Manager Lock*/
   /*          held.                                                    */
static void ProcessLowEnergyPairingChangeEvent(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties)
{
   Connection_Entry_t  ConnectionEntry;
   Connection_Entry_t *ConnectionEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}


   /* The following function is the function that is called to process a*/
   /* LE Address Change Event.                                          */
   /* * NOTE * This function *MUST* be called with the ANPM Manager Lock*/
   /*          held.                                                    */
static void ProcessLowEnergyAddressChangeEvent(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties)
{
   Connection_Entry_t        *ConnectionEntryPtr;
   Client_Connection_Entry_t *ClientConnectionEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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
            DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("ANPM Address Updated\n"));

            /* Save the new Base Address.                               */
            ConnectionEntryPtr->BD_ADDR = RemoteDeviceProperties->BD_ADDR;
         }

         ConnectionEntryPtr = ConnectionEntryPtr->NextConnectionEntryPtr;
      }

      /* Now walk the client connection list and perform the same check.*/
      ClientConnectionEntryPtr = ClientConnectionEntryList;
      while(ClientConnectionEntryPtr)
      {
         /* Check to see if this entry needs to be updated.             */
         if(COMPARE_BD_ADDR(ClientConnectionEntryPtr->BD_ADDR, RemoteDeviceProperties->PriorResolvableBD_ADDR))
         {
            DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("ANPM Address Updated\n"));

            /* Save the new Base Address.                               */
            ClientConnectionEntryPtr->BD_ADDR = RemoteDeviceProperties->BD_ADDR;
         }

         ClientConnectionEntryPtr = ClientConnectionEntryPtr->NextClientConnectionEntryPtr;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a Remote Device Properties Changed Event.                 */
   /* * NOTE * This function *MUST* be called with the ANPM Manager Lock*/
   /*          held.                                                    */
static void ProcessRemoteDevicePropertiesChangedEvent(unsigned long ChangedMemberMask, DEVM_Remote_Device_Properties_t *RemoteDeviceProperties)
{
   unsigned long RequiredFlags;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08lX\n", ChangedMemberMask));

   /* First make sure that the input parameter appears to be semi-valid.*/
   if((ChangedMemberMask) && (RemoteDeviceProperties) && (RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_SUPPORTS_LOW_ENERGY))
   {
      /* Check for an LE connection with services known. */
      if(ChangedMemberMask & (DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_CONNECTION_STATE | DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_SERVICES_STATE))
      {
         RequiredFlags = (DEVM_REMOTE_DEVICE_FLAGS_DEVICE_LE_SERVICES_KNOWN | DEVM_REMOTE_DEVICE_FLAGS_DEVICE_CURRENTLY_CONNECTED_OVER_LE);

         /* Process a connection if we are both connected and know      */
         /* services.                                                   */
         if((RemoteDeviceProperties->RemoteDeviceFlags & RequiredFlags) == RequiredFlags)
         {
            ProcessANSClientConnection(RemoteDeviceProperties);
         }
         else
            ProcessANSClientDisconnection(RemoteDeviceProperties);
      }

      /* Check to see what changed.  We are only interested if the LE   */
      /* Address is updated or the LE Pairing State changes.            */
      if(ChangedMemberMask & (DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_PAIRING_STATE | DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_PRIOR_RESOLVABLE_ADDRESS))
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Flags: 0x%08lX\n", RemoteDeviceProperties->RemoteDeviceFlags));

         /* Process the LE Pairing State Change Event if necessary.     */
         if(ChangedMemberMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_PAIRING_STATE)
            ProcessLowEnergyPairingChangeEvent(RemoteDeviceProperties);

         /* Process the Address Updated event if necessary.             */
         if(ChangedMemberMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_PRIOR_RESOLVABLE_ADDRESS)
            ProcessLowEnergyAddressChangeEvent(RemoteDeviceProperties);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* Process a GATM Read Response for the Supported New Alert          */
   /* Categories attribute.                                             */
static void ProcessSupportedNewAlertCategoriesResponse(GATM_Read_Response_Event_Data_t *EventData, Transaction_Entry_t *TransactionEntry)
{
   Word_t                     SupportedCategories;
   unsigned int               Status;
   Client_Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if((EventData) && (TransactionEntry))
   {
      if((ConnectionEntry = SearchClientConnectionEntry(&ClientConnectionEntryList, EventData->RemoteDeviceAddress)) != NULL)
      {
         SupportedCategories = 0;

         if(ANS_Decode_Supported_Categories(EventData->ValueLength, EventData->Value, &SupportedCategories) == 0)
            Status = ANPM_OPERATION_STATUS_SUCCESS;
         else
            Status = ANPM_OPERATION_STATUS_INVALID_RESPONSE;

         //TODO: Map the bitmask?
         DispatchGetSupportedNewAlertCategoriesResultEvent(ConnectionEntry, TransactionEntry->SingleCallbackID, TransactionEntry->TransactionID, Status, 0, (unsigned long)SupportedCategories);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* Process a GATM Read Response for the Supported Unread Alert Status*/
   /* Categories attribute.                                             */
static void ProcessSupportedUnreadAlertCategoriesResponse(GATM_Read_Response_Event_Data_t *EventData, Transaction_Entry_t *TransactionEntry)
{
   Word_t                     SupportedCategories;
   unsigned int               Status;
   Client_Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if((EventData) && (TransactionEntry))
   {
      if((ConnectionEntry = SearchClientConnectionEntry(&ClientConnectionEntryList, EventData->RemoteDeviceAddress)) != NULL)
      {
         SupportedCategories = 0;

         if(ANS_Decode_Supported_Categories(EventData->ValueLength, EventData->Value, &SupportedCategories) == 0)
            Status = ANPM_OPERATION_STATUS_SUCCESS;
         else
            Status = ANPM_OPERATION_STATUS_INVALID_RESPONSE;

         //TODO: Map the bitmask?
         DispatchGetSupportedUnreadStatusCategoriesResultEvent(ConnectionEntry, TransactionEntry->SingleCallbackID, TransactionEntry->TransactionID, Status, 0, (unsigned long)SupportedCategories);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* Process a GATM Write Response for the Control Point attribute.    */
static void ProcessControlPointWriteResponse(GATM_Write_Response_Event_Data_t *EventData, Transaction_Entry_t *TransactionEntry)
{
   Client_Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if((EventData) && (TransactionEntry))
   {
      if((ConnectionEntry = SearchClientConnectionEntry(&ClientConnectionEntryList, EventData->RemoteDeviceAddress)) != NULL)
      {
         switch(TransactionEntry->ControlPointOperation)
         {
            case cpoEnableNewAlertCategory:
               ConnectionEntry->EnabledNewAlertCategoryMask |= (unsigned long)CategoryToMask(TransactionEntry->PendingCategoryID, TRUE, FALSE);
               break;
            case cpoEnableUnreadStatusCategory:
               ConnectionEntry->EnabledUnreadStatusCategoryMask |= (unsigned long)CategoryToMask(TransactionEntry->PendingCategoryID, FALSE, FALSE);
               break;
            case cpoDisableNewAlertCategory:
               ConnectionEntry->EnabledNewAlertCategoryMask = 0;
               break;
            case cpoDisableUnreadStatusCategory:
               ConnectionEntry->EnabledUnreadStatusCategoryMask = 0;
            default:
               break;
         }

         DispatchCommandResultEvent(ConnectionEntry, TransactionEntry, ANPM_OPERATION_STATUS_SUCCESS, 0);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* Process a GATM Write Response for a Client Characteristic         */
   /* Configuration Descriptor.                                         */
static void ProcessCCCDWriteResponse(GATM_Write_Response_Event_Data_t *EventData, Transaction_Entry_t *TransactionEntry)
{
   Client_Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if((EventData) && (TransactionEntry))
   {
      if((ConnectionEntry = SearchClientConnectionEntry(&ClientConnectionEntryList, EventData->RemoteDeviceAddress)) != NULL)
      {
         switch(TransactionEntry->TransactionType)
         {
            case ttEnableNewAlertCCCD:
               ConnectionEntry->Flags |= CLIENT_CONNECTION_FLAGS_NEW_ALERT_NOTIFICATIONS_ENABLED;
               DispatchCommandResultEvent(ConnectionEntry, TransactionEntry, ANPM_OPERATION_STATUS_SUCCESS, 0);
               break;
            case ttEnableUnreadStatusCCCD:
               ConnectionEntry->Flags |= CLIENT_CONNECTION_FLAGS_UNREAD_NOTIFICATIONS_ENABLED;
               DispatchCommandResultEvent(ConnectionEntry, TransactionEntry, ANPM_OPERATION_STATUS_SUCCESS, 0);
               break;
            default:
               /* Nothing to do, even for disable CCCD because we       */
               /* already flagged disabled on write.                    */
               break;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* Process a GATM Handle Value Indication.                           */
static void ProcessHandleValueNotification(GATM_Handle_Value_Data_Event_Data_t *EventData)
{
   ANS_New_Alert_Data_t           *NewAlertData;
   ANS_Un_Read_Alert_Data_t        UnreadAlertData;
   Client_Connection_Entry_t      *ConnectionEntry;
   ANPM_Category_Identification_t  PMCatergoryID;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EventData)
   {
      if((ConnectionEntry = SearchClientConnectionEntry(&ClientConnectionEntryList, EventData->RemoteDeviceAddress)) != NULL)
      {
         /* Determine what handle the notification is for.              */
         if(EventData->AttributeHandle == ConnectionEntry->ClientInformation.New_Alert)
         {
            if((NewAlertData = ANS_Decode_New_Alert_Notification(EventData->AttributeValueLength, EventData->AttributeValue)) != NULL)
            {
               if(ConvertCategoryPMType(NewAlertData->CategoryID, &PMCatergoryID))
                  DispatchNewAlertNotificationEvent(ConnectionEntry, PMCatergoryID, NewAlertData->NumberOfNewAlerts, NewAlertData->LastAlertString);

               ANS_Free_New_Alert_Data(NewAlertData);
            }
         }
         else
         {
            if(EventData->AttributeHandle == ConnectionEntry->ClientInformation.Unread_Alert_Status)
            {
               if((ANS_Decode_Un_Read_Alert_Status_Notification(EventData->AttributeValueLength, EventData->AttributeValue, &UnreadAlertData) == 0) && (ConvertCategoryPMType(UnreadAlertData.CategoryID, &PMCatergoryID)))
                  DispatchUnreadStatusNotificationEvent(ConnectionEntry, PMCatergoryID, UnreadAlertData.NumberOfUnreadAlerts);
            }
            else
               DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Unexpected GATM Handle Value: %d\n", EventData->AttributeHandle));
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* Process a GATM Error Response.                                    */
static void ProcessGATMErrorResponse(GATM_Error_Response_Event_Data_t *EventData, Transaction_Entry_t *TransactionEntry)
{
   unsigned int               Status;
   unsigned int               ProtocolCode     = 0;
   Client_Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if((EventData) && (TransactionEntry))
   {
      if((ConnectionEntry = SearchClientConnectionEntry(&ClientConnectionEntryList, EventData->RemoteDeviceAddress)) != NULL)
      {
         switch(EventData->ErrorType)
         {
            case retErrorResponse:
               Status       = ANPM_OPERATION_STATUS_ATT_PROTOCOL_ERROR;
               ProtocolCode = EventData->AttributeProtocolErrorCode;
               break;
            case retProtocolTimeout:
               Status = ANPM_OPERATION_STATUS_TIMEOUT;
               break;
            case retPrepareWriteDataMismatch:
            default:
               /* This shouldn't happen since we are not using any      */
               /* prepared writes.                                      */
               Status = ANPM_OPERATION_STATUS_UNKNOWN_ERROR;
               break;
         }

         switch(TransactionEntry->TransactionType)
         {
            case ttReadSupportedNewAlertCategories:
               DispatchGetSupportedNewAlertCategoriesResultEvent(ConnectionEntry, TransactionEntry->TransactionID, TransactionEntry->SingleCallbackID, Status, ProtocolCode, 0);
               break;
            case ttReadSupportedUnreadStatusCategories:
               DispatchGetSupportedUnreadStatusCategoriesResultEvent(ConnectionEntry, TransactionEntry->TransactionID, TransactionEntry->SingleCallbackID, Status, ProtocolCode, 0);
               break;
            case ttWriteControlPoint:
               switch(TransactionEntry->ControlPointOperation)
               {
                  case cpoEnableNewAlertCategory:
                     ResetCategoriesToEnabledMask(&ConnectionEntry->NewAlertCategoryList, ConnectionEntry->EnabledNewAlertCategoryMask);
                     break;
                  case cpoEnableUnreadStatusCategory:
                     ResetCategoriesToEnabledMask(&ConnectionEntry->UnreadStatusCategoryList, ConnectionEntry->EnabledUnreadStatusCategoryMask);
                     break;
                  default:
                     break;
               }

               DispatchCommandResultEvent(ConnectionEntry, TransactionEntry, Status, ProtocolCode);
               break;
            case ttEnableNewAlertCCCD:
               FreeNotificationEntryList(&ConnectionEntry->NewAlertNotificationList);
               DispatchCommandResultEvent(ConnectionEntry, TransactionEntry, Status, ProtocolCode);
               break;
            case ttEnableUnreadStatusCCCD:
               FreeNotificationEntryList(&ConnectionEntry->UnreadStatusNotificationList);
               DispatchCommandResultEvent(ConnectionEntry, TransactionEntry, Status, ProtocolCode);
               break;
            case ttDisableNewAlertCCCD:
            case ttDisableUnreadStatusCCCD:
               /* We already flagged disabled and delted entries. If    */
               /* this fails there isn't anything to do.                */
               break;
            default:
               DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Unexpected Transaction: %d\n", TransactionEntry->TransactionType));
               break;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* Event callback which is registered with GATM to receive Generic   */
   /* Attribute Profile callbacks.                                      */
static void BTPSAPI GATM_EventCallback(GATM_Event_Data_t *GATMEventData, void *CallbackParameter)
{
   Transaction_Entry_t *TransactionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(GATMEventData)
   {
      /* Acquire the lock the protects the module's state.              */
      if(DEVM_AcquireLock())
      {
         switch(GATMEventData->EventType)
         {
            case getGATTReadResponse:
               if((TransactionEntry = DeleteTransactionEntryByGATMID(&TransactionEntryList, GATMEventData->EventData.ReadResponseEventData.TransactionID)) != NULL)
               {
                  switch(TransactionEntry->TransactionType)
                  {
                     case ttReadSupportedNewAlertCategories:
                        ProcessSupportedNewAlertCategoriesResponse(&GATMEventData->EventData.ReadResponseEventData, TransactionEntry);
                        break;
                     case ttReadSupportedUnreadStatusCategories:
                        ProcessSupportedUnreadAlertCategoriesResponse(&GATMEventData->EventData.ReadResponseEventData, TransactionEntry);
                        break;
                     default:
                        DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Unexpected Read Response: %d\n", TransactionEntry->TransactionType));
                        break;
                  }

                  FreeTransactionEntryMemory(TransactionEntry);
               }
               break;
            case getGATTWriteResponse:
               if((TransactionEntry = DeleteTransactionEntryByGATMID(&TransactionEntryList, GATMEventData->EventData.WriteResponseEventData.TransactionID)) != NULL)
               {
                  switch(TransactionEntry->TransactionType)
                  {
                     case ttEnableUnreadStatusCCCD:
                     case ttEnableNewAlertCCCD:
                     case ttDisableNewAlertCCCD:
                     case ttDisableUnreadStatusCCCD:
                        ProcessCCCDWriteResponse(&GATMEventData->EventData.WriteResponseEventData, TransactionEntry);
                        break;
                     case ttWriteControlPoint:
                        ProcessControlPointWriteResponse(&GATMEventData->EventData.WriteResponseEventData, TransactionEntry);
                        break;
                     default:
                        DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Unexpected Write Response: %d\n", TransactionEntry->TransactionType));
                        break;
                  }

                        DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Freeing Entry\n"));
                  FreeTransactionEntryMemory(TransactionEntry);
                        DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Freed\n"));
               }
               break;
            case getGATTHandleValueData:
               ProcessHandleValueNotification(&GATMEventData->EventData.HandleValueDataEventData);
               break;
            case getGATTErrorResponse:
               if((TransactionEntry = DeleteTransactionEntryByGATMID(&TransactionEntryList, GATMEventData->EventData.ErrorResponseEventData.TransactionID)) != NULL)
               {
                  ProcessGATMErrorResponse(&GATMEventData->EventData.ErrorResponseEventData, TransactionEntry);

                  FreeTransactionEntryMemory(TransactionEntry);
               }
               break;
            default:
                  DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled GATM Event: %u\n", GATMEventData->EventType));
                  break;
         }

         DEVM_ReleaseLock();
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process ANP Manager Asynchronous Events.            */
static void BTPSAPI BTPMDispatchCallback_ANPM(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process ANS Events.                                 */
static void BTPSAPI BTPMDispatchCallback_ANS(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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
            if(((ANPM_Update_Data_t *)CallbackParameter)->UpdateType == utANPEvent)
               ProcessANSEvent(&(((ANPM_Update_Data_t *)CallbackParameter)->UpdateData.ServerEventData));

            /* Release the lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
      }

      /* Go ahead and free the memory because we are finished with it.  */
      BTPS_FreeMemory(CallbackParameter);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process GATT Connection Events.                     */
static void BTPSAPI BTPMDistpatchCallback_GATT(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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
            /* Double check that this is an GATT Connection Event       */
            /* Update.                                                  */
            if(((ANPM_Update_Data_t *)CallbackParameter)->UpdateType == utANPConnectionEvent)
               ProcessGATTConnectionEvent(&(((ANPM_Update_Data_t *)CallbackParameter)->UpdateData.ConnectionEventData));

            /* Release the lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
      }

      /* Go ahead and free the memory because we are finished with it.  */
      BTPS_FreeMemory(CallbackParameter);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process Client Un-Register Asynchronous Events.     */
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function represents the Message Group Handler       */
   /* function that is installed to process all ANP Manager Messages.   */
static void BTPSAPI ANPManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the Message specified appears to be semi-valid.   */
   if(Message)
   {
      /* Next, check to see if this message is specified for our        */
      /* specific Message Group.                                        */
      if(Message->MessageHeader.MessageGroup == BTPM_MESSAGE_GROUP_ALERT_NOTIFICATION_MANAGER)
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("ANP Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

         /* Message is destined for us, make sure we are still          */
         /* processing messages.                                        */
         if(Initialized)
         {
            /* Check to see if this message is a ANP Manager defined    */
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
               /* ANP Manager Thread.                                   */
               if(!BTPM_QueueMailboxCallback(BTPMDispatchCallback_ANPM, (void *)Message))
               {
                  DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue ANP Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

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
                        DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue ANP Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));
                     }
                  }
               }
            }
         }
         else
         {
            /* Check to see if this message is an ANP Manager defined   */
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
         DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Non ANP Manager Message: 0x%08X, 0x%08X\n", Message->MessageHeader.MessageGroup, Message->MessageHeader.MessageFunction));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for initializing/cleaning up*/
   /* the Bluetopia Platform Manager ANP Manager Module.  This function */
   /* should be registered with the Bluetopia Platform Manager Module   */
   /* Handler and will be called when the Platform Manager is           */
   /* initialized (or shut down).                                       */
void BTPSAPI ANPM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData)
{
   int Result;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %d\n", Initialize));

   /* First, determine if the request is to initialize or shut down.    */
   if(Initialize)
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Initialization Request\n"));

      if(!Initialized)
      {
         /* Note the new initialization state.                          */
         Initialized = TRUE;

         DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing ANP Manager\n"));

         /* Next, let's attempt to register our Message Group Handler to*/
         /* process ANP Manager messages.                               */
         if(!MSG_RegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_ALERT_NOTIFICATION_MANAGER, ANPManagerGroupHandler, NULL))
         {
            /* Configure the initialization data.                       */
            if(InitializationData)
            {
               SupportedNewAlertCategories    = ((ANPM_Initialization_Info_t *)InitializationData)->SupportedNewAlertCategories;
               SupportedUnReadAlertCategories = ((ANPM_Initialization_Info_t *)InitializationData)->SupportedUnReadAlertCategories;
            }
            else
            {
               SupportedNewAlertCategories    = ANPM_ALERT_CATEGORY_BIT_MASK_ALL_CATEGORIES;
               SupportedUnReadAlertCategories = ANPM_ALERT_CATEGORY_BIT_MASK_ALL_CATEGORIES;
            }

            /* Initialize the actual ANP Manager Implementation Module  */
            /* (this is the module that is actually responsible for     */
            /* actually implementing the ANP Manager functionality -    */
            /* this module is just the framework shell).                */
            if(!(Result = _ANPM_Initialize(SupportedNewAlertCategories, SupportedUnReadAlertCategories)))
            {
               /* Attempt to register the GATM callback. */
               if((Result = GATM_RegisterEventCallback(GATM_EventCallback, NULL)) > 0)
               {
                  GATMCallbackID = (unsigned int)Result;

                  /* Determine the current Device Power State.             */
                  CurrentPowerState       = (Boolean_t)((DEVM_QueryDevicePowerState() > 0)?TRUE:FALSE);

                  /* Initialize a unique, starting ANP Callback ID.        */
                  NextEventCallbackID     = 0;
                  NextTransactionID       = 0;

                  /* Go ahead and flag that this module is initialized.    */
                  Initialized             = TRUE;

                  /* Flag success.                                         */
                  Result                  = 0;
               }
            }
         }
         else
            Result = BTPM_ERROR_CODE_UNABLE_TO_REGISTER_HANDLER;

         /* If an error occurred then we need to free all resources that*/
         /* were allocated.                                             */
         if(Result)
         {
            _ANPM_Cleanup();

            MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_ALERT_NOTIFICATION_MANAGER);
         }
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("ANP Manager Already initialized\n"));
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Shutdown Request\n"));

      if(Initialized)
      {
         /* First, let's make sure that we un-register our Message Group*/
         /* Handler (so that we do not process any more incoming        */
         /* messages).                                                  */
         MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_ALERT_NOTIFICATION_MANAGER);

         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Make sure we inform the ANP Manager Implementation that  */
            /* we are shutting down.                                    */
            _ANPM_Cleanup();

            /* Free the Connection Info list.                           */
            FreeConnectionEntryList(&ConnectionEntryList);
            FreeClientConnectionEntryList(&ClientConnectionEntryList);

            /* Free the Event Callback Info List.                       */
            FreeEventCallbackInfoList(&EventCallbackInfoList);

            /* Free the Alert Lists.                                    */
            FreeAlertEntryList(&NewAlertEntryList);
            FreeAlertEntryList(&UnReadAlertEntryList);

            /* Free the Transaction List.                               */
            FreeTransactionEntryList(&TransactionEntryList);

            /* Flag that the device is not powered on.                  */
            CurrentPowerState       = FALSE;

            /* Flag that this module is no longer initialized.          */
            Initialized             = FALSE;

            /* Free the Lock we acquired earlier.                       */
            DEVM_ReleaseLock();
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing asynchronous */
   /* Device Manager (DEVM) Events (including Power On/Off events).     */
   /* This function should be registered with the Bluetopia Platform    */
   /* Manager Module Handler and will be called when an asynchronous    */
   /* Device Manager event is dispatched.                               */
void BTPSAPI ANPM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData)
{
   int                            Result;
   Connection_Entry_t             ConnectionEntry;
   Connection_Entry_t            *ConnectionEntryPtr;
   GATT_Attribute_Handle_Group_t  ServiceHandleRange;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* First, check to make sure the ANP Manager has been initialized.   */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         switch(EventData->EventType)
         {
            case detDevicePoweredOn:
               DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power On Occurred.\n"));

               /* Power on Event.                                       */

               /* Note the new Power State.                             */
               CurrentPowerState = TRUE;

               /* Go ahead and inform the ANP Manager that it should    */
               /* initialize.                                           */
               if((Result = DEVM_QueryDeviceBluetoothStackID()) > 0)
               {
                  /* Attempt to calculate the Service Handle Range for  */
                  /* this service in the GATT database.                 */
                  if(CalculateServiceHandleRange(&ServiceHandleRange))
                     _ANPM_SetBluetoothStackID((unsigned int)Result, &ServiceHandleRange);
                  else
                     _ANPM_SetBluetoothStackID((unsigned int)Result, NULL);
               }
               break;
            case detDevicePoweringOff:
            case detDevicePoweredOff:
               DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power Off Occurred.\n"));

               /* Note the new Power State.                             */
               CurrentPowerState = FALSE;

               /* Inform the ANP Manager that the Stack has been closed.*/
               _ANPM_SetBluetoothStackID(0, NULL);

               /* Free the Connection Info list.                        */
               FreeConnectionEntryList(&ConnectionEntryList);
               break;
            case detRemoteDevicePropertiesChanged:
               DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Remote Device Properties Changed.\n"));

               /* Process the Remote Device Properties Changed Event.   */
               ProcessRemoteDevicePropertiesChangedEvent(EventData->EventData.RemoteDevicePropertiesChangedEventData.ChangedMemberMask, &(EventData->EventData.RemoteDevicePropertiesChangedEventData.RemoteDeviceProperties));
               break;
            case detRemoteDeviceDeleted:
               DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Remote Device Deleted.\n"));

               /* Delete the specified Connection Entry from the list   */
               /* (if any exists).                                      */
               if((ConnectionEntryPtr = DeleteConnectionEntryByBD_ADDR(&ConnectionEntryList, EventData->EventData.RemoteDeviceDeletedEventData.RemoteDeviceAddress)) != NULL)
               {
                  /* Dispatch a Device Disconnection if we dispatched a */
                  /* Client Connection.                                 */
                  if(ConnectionEntryPtr->ClientConnectedDispatched)
                     DispatchANPClientDisconnectionEvent(ConnectionEntryPtr);

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow the caller the ability*/
   /* to notify the ANP Manager of a specific Update Event.  The ANP    */
   /* Manager can then take the correct action to process the update.   */
Boolean_t ANPM_NotifyUpdate(ANPM_Update_Data_t *UpdateData)
{
   Boolean_t ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, make sure that the input parameters appear to be           */
   /* semi-valid.                                                       */
   if(UpdateData)
   {
      switch(UpdateData->UpdateType)
      {
         case utANPEvent:
            DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Queueing ANS Event: %d\n", UpdateData->UpdateData.ServerEventData.Event_Data_Type));

            /* Simply Queue the Dispatch Data.                          */
            /* * NOTE * To avoid copying again, we will simply submit   */
            /*          the entire Notification Message.                */
            ret_val = BTPM_QueueMailboxCallback(BTPMDispatchCallback_ANS, (void *)UpdateData);
            break;
         case utANPConnectionEvent:
            DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Queueing GATT Connection Event: %d\n", UpdateData->UpdateData.ConnectionEventData.Event_Data_Type));

            /* Simply Queue the Dispatch Data.                          */
            /* * NOTE * To avoid copying again, we will simply submit   */
            /*          the entire Notification Message.                */
            ret_val = BTPM_QueueMailboxCallback(BTPMDistpatchCallback_GATT, (void *)UpdateData);
            break;
         default:
            DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled Notification: %d\n", UpdateData->UpdateType));

            ret_val = FALSE;
            break;
      }
   }
   else
      ret_val = FALSE;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* module to set the number of New Alerts for a specific category and*/
   /* the text of the last alert for the specified category.  This      */
   /* function accepts as the Category ID of the specific category, the */
   /* number of new alerts for the specified category and a text string */
   /* that describes the last alert for the specified category (if any).*/
   /* This function returns zero if successful, or a negative return    */
   /* error code if there was an error.                                 */
   /* * NOTE * ciAllSupportedCategories is not a valid value for the    */
   /*          CategoryID parameter.                                    */
int BTPSAPI ANPM_Set_New_Alert(ANPM_Category_Identification_t CategoryID, unsigned int NewAlertCount, char *LastAlertText)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Alert Notification Manager has been */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Configure the Alert.                                        */
         ret_val = ConfigureAlert(MSG_GetServerAddressID(), TRUE, CategoryID, NewAlertCount, LastAlertText);

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* module to set the number of Un-Read Alerts for a specific         */
   /* category.  This function accepts as the Category ID of the        */
   /* specific category, and the number of un-read alerts for the       */
   /* specified category.  This function returns zero if successful, or */
   /* a negative return error code if there was an error.               */
   /* * NOTE * ciAllSupportedCategories is not a valid value for the    */
   /*          CategoryID parameter.                                    */
int BTPSAPI ANPM_Set_Un_Read_Alert(ANPM_Category_Identification_t CategoryID, unsigned int UnReadAlertCount)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Alert Notification Manager has been */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Configure the Alert.                                        */
         ret_val = ConfigureAlert(MSG_GetServerAddressID(), FALSE, CategoryID, UnReadAlertCount, NULL);

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a server callback function with the Alert     */
   /* Notification (ANP) Manager Service.  This Callback will be        */
   /* dispatched by the ANP Manager when various ANP Manager Server     */
   /* Events occur.  This function accepts the Callback Function and    */
   /* Callback Parameter (respectively) to call when a ANP Manager      */
   /* Server Event needs to be dispatched.  This function returns a     */
   /* positive (non-zero) value if successful, or a negative return     */
   /* error code if there was an error.                                 */
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          ANPM_Un_Register_Server_Event_Callback() function to     */
   /*          un-register the callback from this module.               */
int BTPSAPI ANPM_Register_Server_Event_Callback(ANPM_Event_Callback_t CallbackFunction, void *CallbackParameter)
{
   int                         ret_val;
   ANPM_Event_Callback_Info_t  EventCallbackEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Alert Notification Manager has been */
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
            /* Attempt to add an entry into the Event Callback Entry    */
            /* list.                                                    */
            BTPS_MemInitialize(&EventCallbackEntry, 0, sizeof(ANPM_Event_Callback_Info_t));

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
      ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered ANP Manager Server Event      */
   /* Callback (registered via a successful call to the                 */
   /* ANPM_Register_Server_Event_Callback() function).  This function   */
   /* accepts as input the ANP Manager Event Callback ID (return value  */
   /* from ANPM_Register_Server_Event_Callback() function).             */
void BTPSAPI ANPM_Un_Register_Server_Event_Callback(unsigned int ANPManagerCallbackID)
{
   ANPM_Event_Callback_Info_t *EventCallbackPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Alert Notification Manager has been */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check to see if the input parameters appear to be semi-valid.  */
      if(ANPManagerCallbackID)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Verify that the owner of this callback is the person who */
            /* is doing the un-registering.                             */
            if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, ANPManagerCallbackID)) != NULL)
            {
               /* Verify that this is owned by the Server Process.      */
               if(EventCallbackPtr->ClientID == MSG_GetServerAddressID())
               {
                  /* Delete the callback back from the list.            */
                  if((EventCallbackPtr = DeleteEventCallbackInfoEntry(&EventCallbackInfoList, ANPManagerCallbackID)) != NULL)
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

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit:\n"));
}

   /* Alert Notification Client API definitions. */

   /* This functions submits a request to a remote ANP Server to get the*/
   /* supported New Alert Categories.  The ClientCallbackID parameter   */
   /* should be an ID return from ANPM_Register_Client_Event_Callback().*/
   /* The RemoteDeviceAddress parameter is the Bluetooth Address of the */
   /* remote server.  This function returns a positive value indicating */
   /* the Transaction ID if successful or a negative return error code  */
   /* if there was an error.                                            */
   /* * NOTE * An aetANPSupportedNewAlertCategoriesResult event will be */
   /*          dispatched when this request completes.                  */
int BTPSAPI ANPM_Get_Supported_New_Alert_Categories(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Alert Notification Manager has been */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Make sure the parameters seem semi-valid. */
      if((ClientCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Simply call the function to process the request.         */
            ret_val = ProcessGetSupportedCategories(MSG_GetServerAddressID(), ClientCallbackID, RemoteDeviceAddress, ntNewAlert);

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
      ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* This functions will enable New Alert Notifications from a         */
   /* remote ANP server.  The ClientCallbackID parameter should be      */
   /* an ID return from ANPM_Register_Client_Event_Callback().  The     */
   /* RemoteDeviceAddress parameter is the Bluetooth Address of the     */
   /* remote server.  If successful and a GATT write was triggered to   */
   /* enabled notifications on the remote ANP server, this function     */
   /* will return a positive value representing the Transaction ID of   */
   /* the submitted write. If this function successfully registers the  */
   /* callback, but a GATT write is not necessary, it will return 0. If */
   /* an error occurs, this function will return a negative error code. */
int BTPSAPI ANPM_Enable_New_Alert_Notifications(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Alert Notification Manager has been */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Make sure the parameters seem semi-valid. */
      if((ClientCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Simply call the function to process the request.         */
            ret_val = ProcessEnableNotifications(MSG_GetServerAddressID(), ClientCallbackID, RemoteDeviceAddress, ntNewAlert);

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
      ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* This functions will disable New Alert Notifications from a        */
   /* remote ANP server.  The ClientCallbackID parameter should be      */
   /* an ID return from ANPM_Register_Client_Event_Callback().  The     */
   /* RemoteDeviceAddress parameter is the Bluetooth Address of the     */
   /* remote server.  This function returns zero if successful or a     */
   /* negative return error code if there was an error.                 */
int BTPSAPI ANPM_Disable_New_Alert_Notifications(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Alert Notification Manager has been */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Make sure the parameters seem semi-valid. */
      if((ClientCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Simply call the function to process the request.         */
            ret_val = ProcessDisableNotifications(MSG_GetServerAddressID(), ClientCallbackID, RemoteDeviceAddress, ntNewAlert);

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
      ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* This functions submits a request to a remote ANP Server           */
   /* to get the supported Unread Alery Status Categories.  The         */
   /* ClientCallbackID parameter should be an ID return from            */
   /* ANPM_Register_Client_Event_Callback().  The RemoteDeviceAddress   */
   /* parameter is the Bluetooth Address of the remote server.  This    */
   /* function returns a positive value indicating the Transaction ID if*/
   /* successful or a negative return error code if there was an error. */
   /* * NOTE * An aetANPSupportedUnreadCategoriesResult event will be   */
   /*          dispatched when this request completes.                  */
int BTPSAPI ANPM_Get_Supported_Unread_Status_Categories(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Alert Notification Manager has been */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Make sure the parameters seem semi-valid. */
      if((ClientCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Simply call the function to process the request.         */
            ret_val = ProcessGetSupportedCategories(MSG_GetServerAddressID(), ClientCallbackID, RemoteDeviceAddress, ntUnreadStatus);

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
      ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* This functions will enable Unread Alert Status Notifications from */
   /* a remote ANP server.  The ClientCallbackID parameter should be    */
   /* an ID return from ANPM_Register_Client_Event_Callback().  The     */
   /* RemoteDeviceAddress parameter is the Bluetooth Address of the     */
   /* remote server.  If successful and a GATT write was triggered to   */
   /* enabled notifications on the remote ANP server, this function     */
   /* will return a positive value representing the Transaction ID of   */
   /* the submitted write. If this function successfully registers the  */
   /* callback, but a GATT write is not necessary, it will return 0. If */
   /* an error occurs, this function will return a negative error code. */
int BTPSAPI ANPM_Enable_Unread_Status_Notifications(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Alert Notification Manager has been */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Make sure the parameters seem semi-valid. */
      if((ClientCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Simply call the function to process the request.         */
            ret_val = ProcessEnableNotifications(MSG_GetServerAddressID(), ClientCallbackID, RemoteDeviceAddress, ntUnreadStatus);

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
      ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* This functions will disable Unread Alert Status Notifications     */
   /* from a remote ANP server.  The ClientCallbackID parameter should  */
   /* be an ID return from ANPM_Register_Client_Event_Callback().  The  */
   /* RemoteDeviceAddress parameter is the Bluetooth Address of the     */
   /* remote server.  This function returns zero if successful or a     */
   /* negative return error code if there was an error.                 */
int BTPSAPI ANPM_Disable_Unread_Status_Notifications(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Alert Notification Manager has been */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Make sure the parameters seem semi-valid. */
      if((ClientCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Simply call the function to process the request.         */
            ret_val = ProcessDisableNotifications(MSG_GetServerAddressID(), ClientCallbackID, RemoteDeviceAddress, ntUnreadStatus);

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
      ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* This functions submits a request to a remote ANP Server to enable */
   /* notifications for a given category The ClientCallbackID parameter */
   /* should be an ID return from ANPM_Register_Client_Event_Callback().*/
   /* The RemoteDeviceAddress parameter is the Bluetooth Address of the */
   /* remote server.  The CategoryID paremter indicates the category to */
   /* enable.  If successful and a GATT write was triggered to enabled  */
   /* notifications on the remote ANP server, this function will return */
   /* a positive value representing the Transaction ID of the submitted */
   /* write. If this function successfully registers the callback, but a*/
   /* GATT write is not necessary, it will return 0. If an error occurs,*/
   /* this function will return a negative error code.                  */
   /* * NOTE * The status of the request will be returned in an         */
   /*          aetANPCommandResult event with Transaction ID matching   */
   /*          the return value if positive.                            */
int BTPSAPI ANPM_Enable_New_Alert_Category(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress, ANPM_Category_Identification_t CategoryID)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Alert Notification Manager has been */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Make sure the parameters seem semi-valid. */
      if((ClientCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Simply call the function to process the request.         */
            ret_val = ProcessEnableCategory(MSG_GetServerAddressID(), ClientCallbackID, RemoteDeviceAddress, CategoryID, ntNewAlert);

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
      ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* This functions submits a request to a remote ANP Server to disable*/
   /* notifications for a given category The ClientCallbackID parameter */
   /* should be an ID return from ANPM_Register_Client_Event_Callback().*/
   /* The RemoteDeviceAddress parameter is the Bluetooth Address of the */
   /* remote server.  The CategoryID paremter indicates the category to */
   /* enable.  This function returns zero if successful or a negative   */
   /* return error code if there was an error.                          */
int BTPSAPI ANPM_Disable_New_Alert_Category(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress, ANPM_Category_Identification_t CategoryID)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Alert Notification Manager has been */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Make sure the parameters seem semi-valid. */
      if((ClientCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Simply call the function to process the request.         */
            ret_val = ProcessDisableCategory(MSG_GetServerAddressID(), ClientCallbackID, RemoteDeviceAddress, CategoryID, ntNewAlert);

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
      ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* This functions submits a request to a remote ANP Server to enable */
   /* notifications for a given category The ClientCallbackID parameter */
   /* should be an ID return from ANPM_Register_Client_Event_Callback().*/
   /* The RemoteDeviceAddress parameter is the Bluetooth Address of the */
   /* remote server.  The CategoryID paremter indicates the category to */
   /* enable.  If successful and a GATT write was triggered to enabled  */
   /* notifications on the remote ANP server, this function will return */
   /* a positive value representing the Transaction ID of the submitted */
   /* write. If this function successfully registers the callback, but a*/
   /* GATT write is not necessary, it will return 0. If an error occurs,*/
   /* this function will return a negative error code.                  */
   /* * NOTE * The status of the request will be returned in an         */
   /*          aetANPCommandResult event with Transaction ID matching   */
   /*          the return value if positive.                            */
int BTPSAPI ANPM_Enable_Unread_Status_Category(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress, ANPM_Category_Identification_t CategoryID)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Alert Notification Manager has been */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Make sure the parameters seem semi-valid. */
      if((ClientCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Simply call the function to process the request.         */
            ret_val = ProcessEnableCategory(MSG_GetServerAddressID(), ClientCallbackID, RemoteDeviceAddress, CategoryID, ntUnreadStatus);

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
      ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* This functions submits a request to a remote ANP Server to disable*/
   /* notifications for a given category The ClientCallbackID parameter */
   /* should be an ID return from ANPM_Register_Client_Event_Callback().*/
   /* The RemoteDeviceAddress parameter is the Bluetooth Address of the */
   /* remote server.  The CategoryID paremter indicates the category to */
   /* enable.  This function returns zero if successful or a negative   */
   /* return error code if there was an error.                          */
int BTPSAPI ANPM_Disable_Unread_Status_Category(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress, ANPM_Category_Identification_t CategoryID)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Alert Notification Manager has been */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Make sure the parameters seem semi-valid. */
      if((ClientCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Simply call the function to process the request.         */
            ret_val = ProcessDisableCategory(MSG_GetServerAddressID(), ClientCallbackID, RemoteDeviceAddress, CategoryID, ntUnreadStatus);

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
      ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* This functions submits a request to a remote ANP Server           */
   /* to request an immediate New Alert Notification.  The              */
   /* ClientCallbackID parameter should be an ID return from            */
   /* ANPM_Register_Client_Event_Callback().  The RemoteDeviceAddress   */
   /* parameter is the Bluetooth Address of the remote server.  The     */
   /* CategoryID paremter indicates the category to enable.  This       */
   /* function returns a positive value representing the Transaction    */
   /* ID if successful or a negative return error code if there was an  */
   /* error.                                                            */
   /* * NOTE * The status of the request will be returned in an         */
   /*          aetANPCommandResult event with Transaction ID matching   */
   /*          the return value.                                        */
int BTPSAPI ANPM_Request_New_Alert_Notification(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress, ANPM_Category_Identification_t CategoryID)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Alert Notification Manager has been */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Make sure the parameters seem semi-valid. */
      if((ClientCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Simply call the function to process the request.         */
            ret_val = ProcessRequestNotification(MSG_GetServerAddressID(), ClientCallbackID, RemoteDeviceAddress, CategoryID, ntNewAlert);

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
      ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* This functions submits a request to a remote ANP Server           */
   /* to request an immediate Unread Alert Status Notification.         */
   /* The ClientCallbackID parameter should be an ID return from        */
   /* ANPM_Register_Client_Event_Callback().  The RemoteDeviceAddress   */
   /* parameter is the Bluetooth Address of the remote server.  The     */
   /* CategoryID paremter indicates the category to enable.  This       */
   /* function returns a positive value representing the Transaction    */
   /* ID if successful or a negative return error code if there was an  */
   /* error.                                                            */
   /* * NOTE * The status of the request will be returned in an         */
   /*          aetANPCommandResult event with Transaction ID matching   */
   /*          the return value.                                        */
int BTPSAPI ANPM_Request_Unread_Status_Notification(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress, ANPM_Category_Identification_t CategoryID)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Alert Notification Manager has been */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Make sure the parameters seem semi-valid. */
      if((ClientCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Simply call the function to process the request.         */
            ret_val = ProcessRequestNotification(MSG_GetServerAddressID(), ClientCallbackID, RemoteDeviceAddress, CategoryID, ntUnreadStatus);

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
      ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a server callback function with the Alert     */
   /* Notification (ANP) Manager Service.  This Callback will be        */
   /* dispatched by the ANP Manager when various ANP Manager Client     */
   /* Events occur.  This function accepts the Callback Function and    */
   /* Callback Parameter (respectively) to call when a ANP Manager      */
   /* Client Event needs to be dispatched.  This function returns a     */
   /* positive (non-zero) value if successful, or a negative return     */
   /* error code if there was an error.                                 */
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          ANPM_Un_Register_Client_Event_Callback() function to     */
   /*          un-register the callback from this module.               */
int BTPSAPI ANPM_Register_Client_Event_Callback(ANPM_Event_Callback_t CallbackFunction, void *CallbackParameter)
{
   int                         ret_val;
   ANPM_Event_Callback_Info_t  EventCallbackEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Alert Notification Manager has been */
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
            /* Attempt to add an entry into the Event Callback Entry    */
            /* list.                                                    */
            BTPS_MemInitialize(&EventCallbackEntry, 0, sizeof(ANPM_Event_Callback_Info_t));

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
      ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered ANP Manager Client Event      */
   /* Callback (registered via a successful call to the                 */
   /* ANPM_Register_Client_Event_Callback() function).  This function   */
   /* accepts as input the ANP Manager Event Callback ID (return value  */
   /* from ANPM_Register_Client_Event_Callback() function).             */
void BTPSAPI ANPM_Un_Register_Client_Event_Callback(unsigned int ANPManagerCallbackID)
{
   ANPM_Event_Callback_Info_t *EventCallbackPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Alert Notification Manager has been */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check to see if the input parameters appear to be semi-valid.  */
      if(ANPManagerCallbackID)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Verify that the owner of this callback is the person who */
            /* is doing the un-registering.                             */
            if((EventCallbackPtr = SearchEventCallbackInfoEntry(&ClientEventCallbackInfoList, ANPManagerCallbackID)) != NULL)
            {
               /* Verify that this is owned by the Server Process.      */
               if(EventCallbackPtr->ClientID == MSG_GetServerAddressID())
               {
                  /* Delete the callback back from the list.            */
                  if((EventCallbackPtr = DeleteEventCallbackInfoEntry(&ClientEventCallbackInfoList, ANPManagerCallbackID)) != NULL)
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

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit:\n"));
}
