/*****< btpmancm.c >***********************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMANCM - ANCS Manager for Stonestreet One Bluetooth Protocol Stack      */
/*             Platform Manager.                                              */
/*                                                                            */
/*  Author:  Matt Buckley                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   10/03/13  M. Buckley     Initial creation.                               */
/******************************************************************************/
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMANCM.h"            /* BTPM ANCS Manager Prototypes/Constants.   */
#include "ANCMAPI.h"             /* ANCS Manager Prototypes/Constants.        */

#include "SS1BTPM.h"             /* BTPM Main Prototypes and Constants.       */
#include "BTPMERR.h"             /* BTPM Error Prototypes/Constants.          */
#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

   /* The following defines the length to wait after successfully       */
   /* writing a Get App Attributes or Get Notification Attributes       */
   /* command before timing out and raising an error.                   */
#define ANCM_TRANSACTION_TIMEOUT_VALUE                   30000

   /* The following defines the initial size of the data buffer used to */
   /* store incoming response data for a Get App Attributes or Get      */
   /* Notification Attributes command.                                  */
#define ANCM_DEFAULT_ATTRIBUTE_BUFFER_DATA_SIZE          512

   /* The following is a structure which is used to track Callback      */
   /* Information related to this module.                               */
typedef struct _tagANCM_Event_Callback_Info_t
{
   unsigned int                           EventCallbackID;
   ANCM_Event_Callback_t                  EventCallback;
   void                                  *CallbackParameter;
   struct _tagANCM_Event_Callback_Info_t *NextANCMEventCallbackInfoPtr;
} ANCM_Event_Callback_Info_t;

   /* The following is a container structure that is used when          */
   /* dispatching registered callbacks.                                 */
typedef struct _tagCallback_Info_t
{
   unsigned int           ClientID;
   unsigned int           EventCallbackID;
   ANCM_Event_Callback_t  EventCallback;
   void                  *CallbackParameter;
} Callback_Info_t;

   /* The following structure is a container which holds all            */
   /* characteristic and descriptor handles for an ANCS Consumer        */
   /* connection.                                                       */
typedef struct _tagANCM_Consumer_Connection_Info_t
{
   Word_t NotificationSourceHandle;
   Word_t NotificationSourceCCCD;
   Word_t ControlPointHandle;
   Word_t DataSourceHandle;
   Word_t DataSourceCCCD;
} ANCM_Consumer_Connection_Info_t;

   /* The following enumerated type lists the types of transactions     */
   /* that can occur.  This is used for instances of                    */
   /* Transaction_Entry_t in order to track the type of request.        */
typedef enum
{
   ttGetNotificationAttributes,
   ttGetAppAttributes,
   ttConfigureNotificationSourceCCCD,
   ttConfigureDataSourceCCCD,
   ttRefreshUnregister,
   ttRefreshRegister
} Transaction_Type_t;

   /* The following struct is a container which holds all information   */
   /* relevant to a GATT transaction.                                   */
typedef struct _tagTransaction_Entry_t
{
   unsigned int                    TransactionID;
   unsigned int                    GATTTransactionID;
   Transaction_Type_t              TransactionType;
   BD_ADDR_t                       BD_ADDR;
   unsigned int                    CallbackID;
   unsigned int                    TimerID;
   struct _tagTransaction_Entry_t *NextTransactionEntryPtr;
} Transaction_Entry_t;

   /* The following structure contains the elements of an ANCS          */
   /* notification.  This structure is used when decoding an incoming   */
   /* notification on the Notification Source characteristic.           */
typedef struct _tagANCS_Notification_Data_t
{
   ANCM_Event_ID_t EventID;
   Byte_t          EventFlags;
   unsigned int    CategoryID;
   unsigned int    CategoryCount;
   DWord_t         NotificationUID;
} ANCS_Notification_Data_t;

   /* The following is a structure which is used to track incoming      */
   /* notifications that need to be queued until we verify a connection */
   /* has succeeded.                                                    */
typedef struct _tagNotification_Queue_Entry_t
{
   ANCS_Notification_Data_t               NotificationData;
   struct _tagNotification_Queue_Entry_t *NextNotificationQueueEntryPtr;
} Notification_Queue_Entry_t;

   /* The following is a structure which is used to track information   */
   /* related to incoming connections.                                  */
typedef struct _tagConnection_Entry_t
{
   BD_ADDR_t                        BD_ADDR;
   ANCM_Consumer_Connection_Info_t  ConsumerConnectionInfo;
   unsigned long                    ConnectionFlags;
   unsigned int                     NumberOfAttributesRequested;
   unsigned int                     AttributeRequestTimerID;
   Byte_t                          *AttributeData;
   ANCM_Attribute_Data_Type_t       AttributeDataType;
   unsigned int                     AttributeDataLength;
   Transaction_Entry_t             *TransactionEntryList;
   Notification_Queue_Entry_t      *NotificationQueue;
   Notification_Queue_Entry_t      *NotificationQueueTail;
   struct _tagConnection_Entry_t   *NextConnectionEntryPtr;
} Connection_Entry_t;

   /* The following defined constants represent flags that are used in  */
   /* the member ConnectionFlags of an instance of Connection_Entry_t.  */
   /* The first flag indicates that the ANCS service was found on a     */
   /* remote device, but is currently being parsed and configured.  The */
   /* second flag indicates that the Notification Source characteristic */
   /* has successfully been subscribed to.  The third flag indicates    */
   /* the same for the Data Source characteristic.  The fourth flag     */
   /* indicates that the ANCS service is ready to be used on the remote */
   /* device.  The fifth flag indicates that a Get Attribute request is */
   /* pending, and is used to prevent concurrent requests.  The sixth   */
   /* flag indicates that the remote device has disconnected.  The      */
   /* seventh flag indicates that notifications have been enabled for   */
   /* this device by the callback, and allows notification processing   */
   /* for this device in this instance of this module.                  */
#define ANCM_CONNECTION_ENTRY_FLAGS_CONFIGURING_ANCS_SERVICE            0x00000001
#define ANCM_CONNECTION_ENTRY_FLAGS_NOTIFICATION_SOURCE_CCCD_SUBSCRIBED 0x00000010
#define ANCM_CONNECTION_ENTRY_FLAGS_DATA_SOURCE_CCCD_SUBSCRIBED         0x00000100
#define ANCM_CONNECTION_ENTRY_FLAGS_ANCS_SERVICE_CONFIGURED             0x00001000
#define ANCM_CONNECTION_ENTRY_FLAGS_GET_ATTRIBUTE_REQUEST_PENDING       0x00010000
#define ANCM_CONNECTION_ENTRY_FLAGS_REFRESHING                          0x00000020

   /* The following structure contains the elements of a response to a  */
   /* Get Notification Attributes request.  This structure is used when */
   /* decoding an incoming notification on the Data Source              */
   /* characteristic.                                                   */
typedef struct _tagANCS_Get_Notification_Attributes_Response_Data_t
{
   DWord_t       NotificationUID;
   unsigned int  AttributeDataLength;
   Byte_t       *AttributeData;
} ANCS_Get_Notification_Attributes_Response_Data_t;

   /* The following structure contains the elements of a response to a  */
   /* Get App Attributes request.  This structure is used when decoding */
   /* an incoming notification on the Data Source characteristic.       */
typedef struct _tagANCS_Get_App_Attributes_Response_Data_t
{
   char         *AppIdentifier;
   unsigned int  AttributeDataLength;
   Byte_t       *AttributeData;
} ANCS_Get_App_Attributes_Response_Data_t;

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the compiler*/
   /* as part of standard C/C++).                                       */

   /* Variable which serves as a global flag about whether this module  */
   /* is initialized or not.                                            */
static Boolean_t Initialized;

   /* Variable which is used to protect access to the state information */
   /* in this module.                                                   */
static Mutex_t ANCManagerMutex;

   /* Variable which holds the GATM Event Callback ID.                  */
static unsigned int GATMEventCallbackID;

   /* Variable which is used to hold the next (unique) Callback ID.     */
static unsigned int NextEventCallbackID;

   /* Variable which is used to hold the next (unique) Transaction ID.  */
static unsigned int NextTransactionID;

   /* Variable which holds a pointer to the first element of the        */
   /* Connection Information List (which holds all currently active     */
   /* connections).                                                     */
static Connection_Entry_t *ConnectionEntryList;

   /* Variable which holds the current Power State of the Device.       */
static Boolean_t CurrentPowerState;

   /* Variable which holds a pointer to the first element in the Generic*/
   /* Generic Attribute Profile Callback Info List (which holds all     */
   /* ANCM Server Event Callbacks registered with this module).         */
static ANCM_Event_Callback_Info_t *EventCallbackInfoList;

   /* Internal Function Prototypes.                                     */
static Boolean_t MaxLengthParameterRequired(ANCM_Attribute_Data_Type_t Type, ANCM_Notification_Attribute_ID_t NotificationAttributeID, ANCM_App_Attribute_ID_t AppAttributeID);

static Boolean_t ANCM_AcquireLock(void);
static void ANCM_ReleaseLock(void);

static unsigned int GetNextEventCallbackID(void);
static unsigned int GetNextTransactionID(void);

static ANCM_Event_Callback_Info_t *AddEventCallbackInfoEntry(ANCM_Event_Callback_Info_t **ListHead, ANCM_Event_Callback_Info_t *EntryToAdd);
static ANCM_Event_Callback_Info_t *SearchEventCallbackInfoEntry(ANCM_Event_Callback_Info_t **ListHead, unsigned int EventCallbackID);
static ANCM_Event_Callback_Info_t *DeleteEventCallbackInfoEntry(ANCM_Event_Callback_Info_t **ListHead, unsigned int EventCallbackID);
static void FreeEventCallbackInfoEntryMemory(ANCM_Event_Callback_Info_t *EntryToFree);
static void FreeEventCallbackInfoList(ANCM_Event_Callback_Info_t **ListHead);

static Connection_Entry_t *AddConnectionEntry(Connection_Entry_t **ListHead, Connection_Entry_t *EntryToAdd);
static Connection_Entry_t *SearchAddConnectionEntry(Connection_Entry_t **ListHead, BD_ADDR_t BD_ADDR);
static Connection_Entry_t *SearchConnectionEntry(Connection_Entry_t **ListHead, BD_ADDR_t BD_ADDR);
static Connection_Entry_t *SearchConnectionEntryByTimerID(Connection_Entry_t **ListHead, unsigned int TimerID);
static Connection_Entry_t *DeleteConnectionEntryByBD_ADDR(Connection_Entry_t **ListHead, BD_ADDR_t BD_ADDR);
static void FreeConnectionEntryMemory(Connection_Entry_t *EntryToFree);
static void FreeConnectionEntryList(Connection_Entry_t **ListHead);

static Transaction_Entry_t *AddTransactionEntry(Transaction_Entry_t **ListHead, Transaction_Entry_t *EntryToAdd);
static Transaction_Entry_t *SearchTransactionEntryGATTID(Transaction_Entry_t **ListHead, unsigned int GATTTransactionID);
static Transaction_Entry_t *DeleteTransactionEntry(Transaction_Entry_t **ListHead, unsigned int GATTTransactionID);
static void FreeTransactionEntryMemory(Transaction_Entry_t *EntryToFree);
static void FreeTransactionEntryList(Transaction_Entry_t **ListHead);

static Boolean_t AddNotificationToQueue(Connection_Entry_t *ConnectionEntry, ANCS_Notification_Data_t *NotificationData);
static void ClearNotificationQueue(Connection_Entry_t *ConnectionEntry, Boolean_t Dispatch);

static void DispatchANCMEvent(ANCM_Event_Data_t *ANCMEventData);

static void DispatchANCConnectionEvent(Connection_Entry_t *ConnectionEntry);
static void DispatchANCDisconnectionEvent(Connection_Entry_t *ConnectionEntry);
static void DispatchANCNotificationReceivedEvent(ANCS_Notification_Data_t *NotificationData, BD_ADDR_t RemoteDeviceAddress);
static void DispatchANCGetNotificationAttributesResponseEvent(ANCS_Get_Notification_Attributes_Response_Data_t *ResponseData, BD_ADDR_t RemoteDeviceAddress, Byte_t ErrorCode);
static void DispatchANCGetAppAttributesResponseEvent(ANCS_Get_App_Attributes_Response_Data_t *ResponseData, BD_ADDR_t RemoteDeviceAddress, Byte_t ErrorCode);

static void CleanupGetAttributeOperation(Connection_Entry_t *ConnectionEntry);

static Boolean_t ConfigureCCCDescriptors(Connection_Entry_t *ConnectionEntry);
static Boolean_t ConfigureANCSClient(Connection_Entry_t *ConnectionEntry, GATT_Service_Discovery_Indication_Data_t *Service);
static Boolean_t ConfigureANCSClientConnection(Connection_Entry_t *ConnectionEntry);

static void ProcessLowEnergyConnectionEvent(BD_ADDR_t BD_ADDR);
static void ProcessLowEnergyDisconnectionEvent(BD_ADDR_t BD_ADDR);
static void ProcessLowEnergyAddressChangeEvent(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties);
static void ProcessRemoteDevicePropertiesChangedEvent(unsigned long ChangedMemberMask, DEVM_Remote_Device_Properties_t *RemoteDeviceProperties);

static void ProcessGATTHandleValueData(GATM_Handle_Value_Data_Event_Data_t *HandleValueEventData);
static void ProcessGATTWriteResponse(GATM_Write_Response_Event_Data_t *WriteResponse);
static void ProcessGATTErrorResponse(GATM_Error_Response_Event_Data_t *ErrorResponse);

static void ProcessGATTTransactionResponse(Connection_Entry_t *ConnectionEntry, Transaction_Entry_t *TransactionEntry, Boolean_t ErrorResponse, void *EventData);

static int  ProcessGetAppAttributesRequest(ANCM_Event_Callback_Info_t *EventCallbackPtr, Connection_Entry_t *ConnectionEntry, char *AppIdentifier, unsigned int NumberOfAttributes, ANCM_App_Attribute_Request_Data_t *AttributeRequestData);
static int  ProcessGetNotificationAttributesRequest(ANCM_Event_Callback_Info_t *EventCallbackPtr, Connection_Entry_t *ConnectionEntry, DWord_t NotificationUID, unsigned int NumberOfAttributes, ANCM_Notification_Attribute_Request_Data_t *AttributeRequestData);
static int  ProcessRefreshNotificationsRequest(ANCM_Event_Callback_Info_t *EventCallbackPtr, Connection_Entry_t *ConnectionEntry);
static void ProcessNotificationSourceNotification(Connection_Entry_t *ConnectionEntry, GATM_Handle_Value_Data_Event_Data_t *HandleValueEventData);
static void ProcessDataSourceNotification(Connection_Entry_t *ConnectionEntry, GATM_Handle_Value_Data_Event_Data_t *HandleValueEventData);
static void ProcessRefreshUnregisterResponse(Connection_Entry_t *ConnectionEntry, unsigned int CallbackID);

static Boolean_t BTPSAPI TimerCallback(unsigned int TimerID, void *CallbackParameter);

static int DecodeNotification(unsigned int DataLength, Byte_t *Data, ANCS_Notification_Data_t *NotificationData);
static int DecodeGetNotificationAttributesResponse(Connection_Entry_t *ConnectionEntry, Word_t BufferLength, Byte_t *Buffer, ANCS_Get_Notification_Attributes_Response_Data_t *ResponseData);
static int DecodeGetAppAttributesResponse(Connection_Entry_t *ConnectionEntry, Word_t BufferLength, Byte_t *Buffer, ANCS_Get_App_Attributes_Response_Data_t *ResponseData);

static void BTPSAPI GATM_Event_Callback(GATM_Event_Data_t *EventData, void *CallbackParameter);

   /* The following function determines whether a Max Length parameter  */
   /* is required for a given Attribute ID.  This function takes as its */
   /* parameters a Type to mark whether the caller is requesting the    */
   /* info on a Notification Attribute ID or an App Attribute ID, a     */
   /* Notification Attribute ID, and an App Attribute ID.  It returns   */
   /* TRUE if the Attribute ID does require a Max Length parameter and  */
   /* FALSE if it does not.                                             */
   /* * NOTE * This function will only pay attention to the Attribute   */
   /*          ID type for which its Type specifies.                    */
static Boolean_t MaxLengthParameterRequired(ANCM_Attribute_Data_Type_t Type, ANCM_Notification_Attribute_ID_t NotificationAttributeID, ANCM_App_Attribute_ID_t AppAttributeID)
{
   Boolean_t ret_val;

   if(Type == adtNotification)
   {
      switch(NotificationAttributeID)
      {
         case naidTitle:
         case naidSubtitle:
         case naidMessage:
            ret_val = TRUE;
            break;
         case naidAppIdentifier:
         case naidMessageSize:
         case naidDate:
         default:
            ret_val = FALSE;
            break;
      }
   }
   else
   {
      /* As of the ANCS Specification v1.0, no App Attribute IDs        */
      /* require a Max Length parameter when submitting a Get App       */
      /* Attributes request.                                            */
      ret_val = FALSE;
   }

   return(ret_val);
}

   /* The following function is provided to allow a mechanism to lock   */
   /* the ANCS Manager in order to prevent simultaneous thread access   */
   /* from corrupting internal resources (such as the Connection Entry  */
   /* list and the Callback Info list).                                 */
static Boolean_t ANCM_AcquireLock(void)
{
   Boolean_t ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Attempt to gain access to the ANCS Manager mutex.                 */
   ret_val = BTPS_WaitMutex(ANCManagerMutex, BTPS_INFINITE_WAIT);

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* release a previously acquired lock (via the ANCM_AcquireLock()    */
   /* function).                                                        */
static void ANCM_ReleaseLock(void)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Simply make the call to release the lock on the mutex.            */
   BTPS_ReleaseMutex(ANCManagerMutex);

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* retrieve a unique Callback ID that can be used to add an entry    */
   /* into the ANCM Event Callback List.                                */
static unsigned int GetNextEventCallbackID(void)
{
   ++NextEventCallbackID;

   if(NextEventCallbackID & 0x80000000)
      NextEventCallbackID = 1;

   return(NextEventCallbackID);
}

   /* The following function is a utility function that exists to       */
   /* retrieve a unique Transaction ID that can be used to add an entry */
   /* into the ANCM Transaction List.                                   */
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
static ANCM_Event_Callback_Info_t *AddEventCallbackInfoEntry(ANCM_Event_Callback_Info_t **ListHead, ANCM_Event_Callback_Info_t *EntryToAdd)
{
   ANCM_Event_Callback_Info_t *AddedEntry = NULL;
   ANCM_Event_Callback_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First let's verify that values passed in are semi-valid.          */
   if((ListHead) && (EntryToAdd))
   {
      /* Make sure that the element that we are adding seems semi-valid.*/
      if(EntryToAdd->EventCallbackID)
      {
         /* OK, data seems semi-valid, let's allocate a new data        */
         /* structure to add to the list.                               */
         AddedEntry = (ANCM_Event_Callback_Info_t *)BTPS_AllocateMemory(sizeof(ANCM_Event_Callback_Info_t));

         if(AddedEntry)
         {
            /* Copy All Data over.                                      */
            *AddedEntry                              = *EntryToAdd;

            /* Now Add it to the end of the list.                       */
            AddedEntry->NextANCMEventCallbackInfoPtr = NULL;

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
                     if(tmpEntry->NextANCMEventCallbackInfoPtr)
                        tmpEntry = tmpEntry->NextANCMEventCallbackInfoPtr;
                     else
                        break;
                  }
               }

               if(AddedEntry)
               {
                  /* Last element found, simply Add the entry.          */
                  tmpEntry->NextANCMEventCallbackInfoPtr = AddedEntry;
               }
            }
            else
               *ListHead = AddedEntry;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", AddedEntry));

   return(AddedEntry);
}

   /* The following function searches the specified List for the        */
   /* specified Event Callback ID.  This function returns NULL if either*/
   /* the List Head is invalid, the Event Callback ID is invalid, or the*/
   /* specified Event Callback ID was NOT found.                        */
static ANCM_Event_Callback_Info_t *SearchEventCallbackInfoEntry(ANCM_Event_Callback_Info_t **ListHead, unsigned int EventCallbackID)
{
   ANCM_Event_Callback_Info_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", EventCallbackID));

   /* Let's make sure the list and Event Callback ID to search for      */
   /* appear to be valid.                                               */
   if((ListHead) && (EventCallbackID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->EventCallbackID != EventCallbackID))
         FoundEntry = FoundEntry->NextANCMEventCallbackInfoPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

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
static ANCM_Event_Callback_Info_t *DeleteEventCallbackInfoEntry(ANCM_Event_Callback_Info_t **ListHead, unsigned int EventCallbackID)
{
   ANCM_Event_Callback_Info_t *FoundEntry = NULL;
   ANCM_Event_Callback_Info_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", EventCallbackID));

   /* Let's make sure the List and Event Callback ID to search for      */
   /* appear to be semi-valid.                                          */
   if((ListHead) && (EventCallbackID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->EventCallbackID != EventCallbackID))
      {
         LastEntry  = FoundEntry;
         FoundEntry = FoundEntry->NextANCMEventCallbackInfoPtr;
      }

      /* Check to see if we found the specified entry.                  */
      if(FoundEntry)
      {
         /* OK, now let's remove the entry from the list.  We have to   */
         /* check to see if the entry was the first entry in the list.  */
         if(LastEntry)
         {
            /* Entry was NOT the first entry in the list.               */
            LastEntry->NextANCMEventCallbackInfoPtr = FoundEntry->NextANCMEventCallbackInfoPtr;
         }
         else
            *ListHead = FoundEntry->NextANCMEventCallbackInfoPtr;

         FoundEntry->NextANCMEventCallbackInfoPtr = NULL;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* The following function frees the specified Event Callback member. */
   /* No check is done on this entry other than making sure it NOT NULL.*/
static void FreeEventCallbackInfoEntryMemory(ANCM_Event_Callback_Info_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToFree)
      BTPS_FreeMemory(EntryToFree);

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function deletes and frees the memory of every      */
   /* element of the specified Event Callback List.  Upon return of     */
   /* this function, the Head Pointer is set to NULL.                   */
static void FreeEventCallbackInfoList(ANCM_Event_Callback_Info_t **ListHead)
{
   ANCM_Event_Callback_Info_t *EntryToFree;
   ANCM_Event_Callback_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ListHead)
   {
      /* Simply traverse the list and free every element present.       */
      EntryToFree = *ListHead;

      while(EntryToFree)
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_VERBOSE), ("Deleting Event Callback ID: 0x%08X\n", EntryToFree->EventCallbackID));

         tmpEntry    = EntryToFree;
         EntryToFree = EntryToFree->NextANCMEventCallbackInfoPtr;

         FreeEventCallbackInfoEntryMemory(tmpEntry);
      }

      /* Make sure the List appears to be empty.                        */
      *ListHead = NULL;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
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

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", AddedEntry));

   return(AddedEntry);
}

   /* The following function is a utility function that is used to      */
   /* search for an existing entry for the specified device OR to create*/
   /* a new entry and add it to the specified list.  This function      */
   /* returns the Entry on success or NULL on failure.                  */
static Connection_Entry_t *SearchAddConnectionEntry(Connection_Entry_t **ListHead, BD_ADDR_t BD_ADDR)
{
   Connection_Entry_t               EntryToAdd;
   Connection_Entry_t              *ret_val = NULL;
   DEVM_Remote_Device_Properties_t  RemoteDeviceProperties;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters appear semi-valid.               */
   if((ListHead) && (!COMPARE_NULL_BD_ADDR(BD_ADDR)))
   {
      /* First search the list for the specified Bluetooth Device       */
      /* Address.                                                       */
      if((ret_val = SearchConnectionEntry(ListHead, BD_ADDR)) == NULL)
      {
         /* Query the remote device properties to get the "Base" Address*/
         /* (which may not be the same address we get here).            */
         if(!DEVM_QueryRemoteDeviceProperties(BD_ADDR, DEVM_QUERY_REMOTE_DEVICE_PROPERTIES_FLAGS_LOW_ENERGY, &RemoteDeviceProperties))
         {
            /* Search the list for the "Base" BD_ADDR.                  */
            if((ret_val = SearchConnectionEntry(ListHead, RemoteDeviceProperties.BD_ADDR)) == NULL)
            {
               /* Entry was not found so add it to the list.            */
               BTPS_MemInitialize(&EntryToAdd, 0, sizeof(Connection_Entry_t));

               EntryToAdd.BD_ADDR         = RemoteDeviceProperties.BD_ADDR;

               /* Attempt to add the entry to the list.                 */
               ret_val = AddConnectionEntry(ListHead, &EntryToAdd);
            }
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", ret_val));

   return(ret_val);
}

   /* The following function searches the specified Connection Entry    */
   /* List for the specified Connection Entry based on the specified    */
   /* Bluetooth Device Address.  This function returns NULL if either   */
   /* the Connection Entry List Head is invalid, the Bluetooth Device   */
   /* Address is invalid, or the specified Entry was NOT present in the */
   /* list.                                                             */
static Connection_Entry_t *SearchConnectionEntry(Connection_Entry_t **ListHead, BD_ADDR_t BD_ADDR)
{
   Connection_Entry_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's make sure the list and Bluetooth Device Address to search   */
   /* for appear to be valid.                                           */
   if((ListHead) && (!COMPARE_NULL_BD_ADDR(BD_ADDR)))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (!COMPARE_BD_ADDR(FoundEntry->BD_ADDR, BD_ADDR)))
         FoundEntry = FoundEntry->NextConnectionEntryPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* The following function searches the specified Connection Entry    */
   /* List for the specified Connection Entry based on the specified    */
   /* Timer ID.  This function returns NULL if either the Connection    */
   /* Entry List Head is invalid or the specified Timer ID was NOT      */
   /* present in the list.                                              */
static Connection_Entry_t *SearchConnectionEntryByTimerID(Connection_Entry_t **ListHead, unsigned int TimerID)
{
   Connection_Entry_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's make sure the list and Bluetooth Device Address to search   */
   /* for appear to be valid.                                           */
   if((ListHead) && (TimerID > 0))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (TimerID != FoundEntry->AttributeRequestTimerID))
         FoundEntry = FoundEntry->NextConnectionEntryPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* This function frees the specified Connection Information member.  */
   /* No check is done on this entry other than making sure it NOT NULL.*/
static void FreeConnectionEntryMemory(Connection_Entry_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure that the entry actually exists.                         */
   if(EntryToFree)
   {
      /* Free any Transaction Entries that may exist.                   */
      if(EntryToFree->TransactionEntryList)
         FreeTransactionEntryList(&(EntryToFree->TransactionEntryList));

      /* Free any Attribute Data that may exist.                        */
      if(EntryToFree->AttributeData)
         BTPS_FreeMemory(EntryToFree->AttributeData);

      /* Free any queued notifications.                                 */
      if(EntryToFree->NotificationQueue)
         ClearNotificationQueue(EntryToFree, FALSE);

      BTPS_FreeMemory(EntryToFree);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function deletes and frees the memory of every      */
   /* element of the specified Connection Information List.  Upon return*/
   /* of this function, the Head Pointer is set to NULL.                */
static void FreeConnectionEntryList(Connection_Entry_t **ListHead)
{
   Connection_Entry_t *EntryToFree;
   Connection_Entry_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function adds the specified Entry to the specified  */
   /* List.  This function allocates and adds an entry to the list that */
   /* has the same attributes as the Entry passed into this function.   */
   /* This function will return NULL if NO Entry was added.  This can   */
   /* occur if the element passed in was deemed invalid or the actual   */
   /* List Head was invalid.                                            */
   /* ** NOTE ** This function does not insert duplicate entries into   */
   /*            the list.  An element is considered a duplicate if     */
   /*            the the Transaction ID field is the same as an entry   */
   /*            already in the list.  When this occurs, this function  */
   /*            returns NULL.                                          */
static Transaction_Entry_t *AddTransactionEntry(Transaction_Entry_t **ListHead, Transaction_Entry_t *EntryToAdd)
{
   Transaction_Entry_t *AddedEntry = NULL;
   Transaction_Entry_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", AddedEntry));

   return(AddedEntry);
}

   /* The following function searches the specified Transaction Entry   */
   /* List for the specified Transaction Entry based on the specified   */
   /* GATTTransactionID.  This function returns NULL if either the      */
   /* Transaction Entry List Head is invalid or the specified Entry was */
   /* NOT present in the list.                                          */
static Transaction_Entry_t *SearchTransactionEntryGATTID(Transaction_Entry_t **ListHead, unsigned int GATTTransactionID)
{
   Transaction_Entry_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's make sure the list and Bluetooth Device Address to search   */
   /* for appear to be valid.                                           */
   if((ListHead) && (GATTTransactionID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->GATTTransactionID != GATTTransactionID))
         FoundEntry = FoundEntry->NextTransactionEntryPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
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

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* This function frees the specified Transaction Information member. */
   /* No check is done on this entry other than making sure it NOT NULL.*/
static void FreeTransactionEntryMemory(Transaction_Entry_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToFree)
      BTPS_FreeMemory(EntryToFree);

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function deletes (and frees the memory of) every    */
   /* element of the specified Transaction Information List.  Upon      */
   /* return of this function, the Head Pointer is set to NULL.         */
static void FreeTransactionEntryList(Transaction_Entry_t **ListHead)
{
   Transaction_Entry_t *EntryToFree;
   Transaction_Entry_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function to add an incoming   */
   /* ANCS Notification to a the connection's notification queue. The   */
   /* ConnectionEntryParameter specifies which connection the           */
   /* notification is for and the NotificationData specifies the        */
   /* notification information to add. This function returns TRUE if    */
   /* successful and FALSE if an error occurs.                          */
static Boolean_t AddNotificationToQueue(Connection_Entry_t *ConnectionEntry, ANCS_Notification_Data_t *NotificationData)
{
   Boolean_t                   ret_val;
   Notification_Queue_Entry_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify the the parmeters are semi-valid.                          */
   if((ConnectionEntry) && (NotificationData))
   {
      /* Attempt to allocate the new queue entry.                       */
      if((tmpEntry = BTPS_AllocateMemory(sizeof(Notification_Queue_Entry_t))) != NULL)
      {
         /* Initialize the new entry.                                   */
         BTPS_MemInitialize(tmpEntry, 0, sizeof(Notification_Queue_Entry_t));
         tmpEntry->NotificationData = *NotificationData;

         /* Check whether the queue is currently empty.                 */
         if(ConnectionEntry->NotificationQueue)
         {
            /* There is already a queue, so place this at the end.      */
            ConnectionEntry->NotificationQueueTail->NextNotificationQueueEntryPtr = tmpEntry;
            ConnectionEntry->NotificationQueueTail                                = tmpEntry;
         }
         else
         {
            /* No queue exists so create a new one.                     */
            ConnectionEntry->NotificationQueue     = tmpEntry;
            ConnectionEntry->NotificationQueueTail = tmpEntry;
         }

         ret_val = TRUE;
      }
      else
         ret_val = FALSE;
   }
   else
      ret_val = FALSE;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function to clear out the     */
   /* notification queue for a given connection. The ConnectionEntry    */
   /* parameter specifies the connection and the Dispatch parameter     */
   /* specifies with the notification should be dispatched as an event  */
   /* or simply deleted.                                                */
static void ClearNotificationQueue(Connection_Entry_t *ConnectionEntry, Boolean_t Dispatch)
{
   Notification_Queue_Entry_t *tmpEntry;
   Notification_Queue_Entry_t *CurrentEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify the the parmeters are semi-valid.                          */
   if(ConnectionEntry)
   {
      /* Start at the beginning of the queue.                           */
      CurrentEntry = ConnectionEntry->NotificationQueue;

      /* Continue while the current queue entry is valid.               */
      while(CurrentEntry)
      {
         /* If the caller wants to dispatch the notifications, go ahead */
         /* and do so.                                                  */
         if(Dispatch)
            DispatchANCNotificationReceivedEvent(&CurrentEntry->NotificationData, ConnectionEntry->BD_ADDR);

         /* Note the next entry before we delete the current one.       */
         tmpEntry = CurrentEntry->NextNotificationQueueEntryPtr;

         /* Delete the current entry.                                   */
         BTPS_FreeMemory(CurrentEntry);

         /* Move on to the next entry.                                  */
         CurrentEntry = tmpEntry;
      }

      /* Note that the queue is no longer valid.                        */
      ConnectionEntry->NotificationQueue     = NULL;
      ConnectionEntry->NotificationQueueTail = NULL;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch the specified module event to any registered ANCM Event  */
   /* callbacks.                                                        */
   /* * NOTE * This function should be called with the Module Manager's */
   /*          Mutex held.  Upon exit from this function it will free   */
   /*          the Module Manager's Mutex.                              */
static void DispatchANCMEvent(ANCM_Event_Data_t *ANCMEventData)
{
   unsigned int                Index;
   unsigned int                NumberCallbacks;
   Callback_Info_t             CallbackInfoArray[16];
   Callback_Info_t            *CallbackInfoArrayPtr;
   ANCM_Event_Callback_Info_t *CallbackInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure that the input parameters appear to be semi-valid.      */
   if((EventCallbackInfoList) && (ANCMEventData))
   {
      /* Let's determine how many callbacks are registered.             */
      CallbackInfoPtr = EventCallbackInfoList;
      NumberCallbacks = 0;

      /* First, sum the default event handlers.                         */
      while(CallbackInfoPtr)
      {
         if(CallbackInfoPtr->EventCallback)
            NumberCallbacks++;

         CallbackInfoPtr = CallbackInfoPtr->NextANCMEventCallbackInfoPtr;
      }

      DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Number Callbacks: %u\n", NumberCallbacks));
      if(NumberCallbacks)
      {
         /* Make sure that we have memory to copy the Callback List     */
         /* into.                                                       */
         if(NumberCallbacks <= (sizeof(CallbackInfoArray)/sizeof(Callback_Info_t)))
            CallbackInfoArrayPtr = CallbackInfoArray;
         else
            CallbackInfoArrayPtr = BTPS_AllocateMemory((NumberCallbacks*sizeof(Callback_Info_t)));

         if(CallbackInfoArrayPtr)
         {
            CallbackInfoPtr = EventCallbackInfoList;
            NumberCallbacks = 0;

            /* Next add the default event handlers.                     */
            while(CallbackInfoPtr)
            {
               if(CallbackInfoPtr->EventCallback)
               {
                  CallbackInfoArrayPtr[NumberCallbacks].EventCallbackID   = CallbackInfoPtr->EventCallbackID;
                  CallbackInfoArrayPtr[NumberCallbacks].EventCallback     = CallbackInfoPtr->EventCallback;
                  CallbackInfoArrayPtr[NumberCallbacks].CallbackParameter = CallbackInfoPtr->CallbackParameter;

                  NumberCallbacks++;
               }

               CallbackInfoPtr = CallbackInfoPtr->NextANCMEventCallbackInfoPtr;
            }

            /* Release the Lock because we have already built the       */
            /* Callback Array.                                          */
            ANCM_ReleaseLock();

            /* Now we are ready to dispatch the callbacks.              */
            Index = 0;

            while((Index < NumberCallbacks) && (Initialized))
            {
               /* Go ahead and make the callback.                       */
               /* * NOTE * If the callback was deleted (or new ones were*/
               /*          added, they will not be caught for this      */
               /*          message dispatch).  Under normal operating   */
               /*          circumstances this case shouldn't matter     */
               /*          because these groups aren't really dynamic   */
               /*          and are only registered at initialization    */
               /*          time.                                        */
               __BTPSTRY
               {
                  if(CallbackInfoArrayPtr[Index].EventCallback)
                     (*CallbackInfoArrayPtr[Index].EventCallback)(ANCMEventData, CallbackInfoArrayPtr[Index].CallbackParameter);
               }
               __BTPSEXCEPT(1)
               {
                  /* Do Nothing.                                        */
               }

               Index++;
            }

            /* Re-acquire the Lock.                                     */
            ANCM_AcquireLock();

            /* Free any memory that was allocated.                      */
            if(CallbackInfoArrayPtr != CallbackInfoArray)
               BTPS_FreeMemory(CallbackInfoArrayPtr);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch a Connected Asynchronous Event to the local callback.  It*/
   /* is the caller's responsibility to verify the message before       */
   /* calling this function.                                            */
   /* * NOTE * This function should be called with the Module Manager's */
   /*          Mutex held.  Upon exit from this function it will free   */
   /*          the Module Manager's Mutex.                              */
static void DispatchANCConnectionEvent(Connection_Entry_t *ConnectionEntry)
{
   ANCM_Event_Data_t           EventData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure that the parameters are semi-valid.                     */
   if(ConnectionEntry)
   {
      EventData.EventType                                        = etANCMConnected;
      EventData.EventLength                                      = ANCM_CONNECTED_EVENT_DATA_SIZE;
      EventData.EventData.ConnectedEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;

      /* Dispatch the event to all registered callbacks.                */
      DispatchANCMEvent(&EventData);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch a Disconnected Asynchronous Event to the local callback. */
   /* It is the caller's responsibility to verify the message before    */
   /* calling this function.                                            */
   /* * NOTE * This function should be called with the Module Manager's */
   /*          Mutex held.  Upon exit from this function it will free   */
   /*          the Module Manager's Mutex.                              */
static void DispatchANCDisconnectionEvent(Connection_Entry_t *ConnectionEntry)
{
   ANCM_Event_Data_t              EventData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure that the parameters are semi-valid.                     */
   if(ConnectionEntry)
   {
      EventData.EventType                                           = etANCMDisconnected;
      EventData.EventLength                                         = ANCM_DISCONNECTED_EVENT_DATA_SIZE;
      EventData.EventData.DisconnectedEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;

      /* Dispatch the event to all registered callbacks.                */
      DispatchANCMEvent(&EventData);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch a Notification Received Asynchronous Event to the local  */
   /* callback.  It is the caller's responsibility to verify the        */
   /* message data before calling this function.                        */
   /* * NOTE * This function should be called with the Module Manager's */
   /*          Mutex held.  Upon exit from this function it will free   */
   /*          the Module Manager's Mutex.                              */
static void DispatchANCNotificationReceivedEvent(ANCS_Notification_Data_t *NotificationData, BD_ADDR_t RemoteDeviceAddress)
{
   ANCM_Event_Data_t                       EventData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure that the parameters are semi-valid.                     */
   if((NotificationData) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
   {
      /* Format the event.                                              */
      EventData.EventType                                                   = etANCMNotificationReceived;
      EventData.EventLength                                                 = ANCM_NOTIFICATION_RECEIVED_EVENT_DATA_SIZE;
      EventData.EventData.NotificationReceivedEventData.RemoteDeviceAddress = RemoteDeviceAddress;
      EventData.EventData.NotificationReceivedEventData.EventID             = NotificationData->EventID;
      EventData.EventData.NotificationReceivedEventData.EventFlags          = NotificationData->EventFlags;
      EventData.EventData.NotificationReceivedEventData.CategoryID          = NotificationData->CategoryID;
      EventData.EventData.NotificationReceivedEventData.CategoryCount       = NotificationData->CategoryCount;
      EventData.EventData.NotificationReceivedEventData.NotificationUID     = NotificationData->NotificationUID;

      /* Dispatch the event to all registered callbacks.                */
      DispatchANCMEvent(&EventData);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch a Get Notification Attributes Response Event to the      */
   /* local callback.  It is the caller's responsibility to verify the  */
   /* message data before calling this function.                        */
   /* * NOTE * This function should be called with the Module Manager's */
   /*          Mutex held.  Upon exit from this function it will free   */
   /*          the Module Manager's Mutex.                              */
static void DispatchANCGetNotificationAttributesResponseEvent(ANCS_Get_Notification_Attributes_Response_Data_t *ResponseData, BD_ADDR_t RemoteDeviceAddress, Byte_t ErrorCode)
{
   ANCM_Event_Data_t EventData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure that the second parameter is semi-valid.  ResponseData  */
   /* is optional.                                                      */
   if(!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress))
   {
      /* Format the event.                                              */
      EventData.EventType                                                                   = etANCMGetNotificationAttributesResponse;
      EventData.EventData.GetNotificationAttributesResponseEventData.RemoteDeviceAddress    = RemoteDeviceAddress;
      EventData.EventData.GetNotificationAttributesResponseEventData.Status                 = (unsigned int)ErrorCode;

      /* ResponseData should only be present if no error occurred.      */
      if((ResponseData) && (ErrorCode == 0))
      {
         EventData.EventData.GetNotificationAttributesResponseEventData.NotificationUID     = ResponseData->NotificationUID;
         EventData.EventData.GetNotificationAttributesResponseEventData.AttributeDataLength = ResponseData->AttributeDataLength;
         EventData.EventData.GetNotificationAttributesResponseEventData.AttributeData       = ResponseData->AttributeData;
         EventData.EventLength                                                              = ANCM_GET_NOTIFICATION_ATTRIBUTES_RESPONSE_EVENT_DATA_SIZE + ResponseData->AttributeDataLength;
      }
      else
      {
         if(ErrorCode != 0)
         {
            EventData.EventLength                                                           = ANCM_GET_NOTIFICATION_ATTRIBUTES_RESPONSE_EVENT_DATA_SIZE;
         }
      }

      /* Dispatch the event to all registered callbacks.                */
      DispatchANCMEvent(&EventData);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch a Get App Attributes Response Event to the local         */
   /* callback.  It is the caller's responsibility to verify the        */
   /* message data before calling this function.                        */
   /* * NOTE * This function should be called with the Module Manager's */
   /*          Mutex held.  Upon exit from this function it will free   */
   /*          the Module Manager's Mutex.                              */
static void DispatchANCGetAppAttributesResponseEvent(ANCS_Get_App_Attributes_Response_Data_t *ResponseData, BD_ADDR_t RemoteDeviceAddress, Byte_t ErrorCode)
{
   ANCM_Event_Data_t EventData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure that the second parameter is semi-valid.  ResponseData  */
   /* is optional.                                                      */
   if(!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress))
   {
      /* Format the event.                                              */
      EventData.EventType                                                          = etANCMGetAppAttributesResponse;
      EventData.EventData.GetAppAttributesResponseEventData.RemoteDeviceAddress    = RemoteDeviceAddress;
      EventData.EventData.GetAppAttributesResponseEventData.Status                 = (unsigned int)ErrorCode;

      /* ResponseData should only be present if no error occurred.      */
      if((ResponseData) && (ErrorCode == 0))
      {
         EventData.EventData.GetAppAttributesResponseEventData.AppIdentifier       = ResponseData->AppIdentifier;
         EventData.EventData.GetAppAttributesResponseEventData.AttributeDataLength = ResponseData->AttributeDataLength;
         EventData.EventData.GetAppAttributesResponseEventData.AttributeData       = ResponseData->AttributeData;
         EventData.EventLength                                                     = ANCM_GET_APP_ATTRIBUTES_RESPONSE_EVENT_DATA_SIZE + ResponseData->AttributeDataLength;
      }
      else
      {
         if(ErrorCode != 0)
         {
            EventData.EventLength                                                  = ANCM_GET_APP_ATTRIBUTES_RESPONSE_EVENT_DATA_SIZE;
         }
      }

      /* Dispatch the event to all registered callbacks.                */
      DispatchANCMEvent(&EventData);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to clean*/
   /* up an outstanding operation.                                      */
static void CleanupGetAttributeOperation(Connection_Entry_t *ConnectionEntry)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ConnectionEntry)
   {
      /* Free any allocated memory.                                     */
      if(ConnectionEntry->AttributeData)
      {
         BTPS_FreeMemory(ConnectionEntry->AttributeData);

         ConnectionEntry->AttributeData = NULL;
      }

      /* If there is a timer waiting on this, stop it for now.          */
      if(ConnectionEntry->AttributeRequestTimerID)
         TMR_StopTimer(ConnectionEntry->AttributeRequestTimerID);

      /* Clear information on the requested information.                */
      ConnectionEntry->NumberOfAttributesRequested  = 0;
      ConnectionEntry->AttributeRequestTimerID      = 0;
      ConnectionEntry->AttributeDataType            = 0;
      ConnectionEntry->AttributeDataLength          = 0;

      /* Flag that there is no longer a Get Attribute request pending.  */
      ConnectionEntry->ConnectionFlags             &= ~((unsigned long)ANCM_CONNECTION_ENTRY_FLAGS_GET_ATTRIBUTE_REQUEST_PENDING);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* configure the Client Characteristic Configuration Descriptors for */
   /* the Notification Source and Data Source characteristics.  This    */
   /* function takes as its parameters the Connection Entry for which   */
   /* descriptors will be configured and the Boolean value Subscribe    */
   /* which indicates whether notifications should be enabled (TRUE) or */
   /* disabled (FALSE).  This function returns 0 on success or a        */
   /* negative value if an error occurred.                              */
static Boolean_t ConfigureCCCDescriptors(Connection_Entry_t *ConnectionEntry)
{
   int                  Result;
   Boolean_t            NoError = TRUE;
   NonAlignedWord_t     CCCDValue;
   Transaction_Entry_t  TransactionEntry;
   Transaction_Entry_t *TransactionEntryPtr;
   Transaction_Entry_t  TransactionEntry1;
   Transaction_Entry_t *TransactionEntry1Ptr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if(ConnectionEntry)
   {
      /* Configure the CCD for the Notification Source characteristic.  */
      /* Configure the Transaction Information for this transaction.    */
      BTPS_MemInitialize(&TransactionEntry, 0, sizeof(TransactionEntry));

      TransactionEntry.TransactionID   = GetNextTransactionID();
      TransactionEntry.TransactionType = ttConfigureNotificationSourceCCCD;
      TransactionEntry.BD_ADDR         = ConnectionEntry->BD_ADDR;

      if((TransactionEntryPtr = AddTransactionEntry(&(ConnectionEntry->TransactionEntryList), &TransactionEntry)) != NULL)
      {
         /* Configure the CCCD for notifications.                       */
         ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&CCCDValue, GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_NOTIFY_ENABLE);

         /* Perform the write to configure the CCCD.                    */
         Result = GATM_WriteValue(GATMEventCallbackID, ConnectionEntry->BD_ADDR, ConnectionEntry->ConsumerConnectionInfo.NotificationSourceCCCD, NON_ALIGNED_WORD_SIZE, (Byte_t *)&CCCDValue);
         if(Result > 0)
         {
            /* Save the GATM Transaction ID.                            */
            TransactionEntryPtr->GATTTransactionID = (unsigned int)Result;
         }
         else
         {
            DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_VERBOSE), ("Error GATM_WriteValue() returned %d\n", Result));

            /* Delete the transaction information memory and free the   */
            /* memory.                                                  */
            if((TransactionEntryPtr = DeleteTransactionEntry(&(ConnectionEntry->TransactionEntryList), TransactionEntryPtr->TransactionID)) != NULL)
               FreeTransactionEntryMemory(TransactionEntryPtr);

            /* Flag that an error occurred.                             */
            NoError = FALSE;
         }
      }
      else
         NoError = FALSE;

      /* Continue if no error has occurred.                             */
      if(NoError)
      {
         /* Configure the CCD for the Data Source characteristic.       */
         BTPS_MemInitialize(&TransactionEntry1, 0, sizeof(TransactionEntry1));

         TransactionEntry1.TransactionID   = GetNextTransactionID();
         TransactionEntry1.TransactionType = ttConfigureDataSourceCCCD;
         TransactionEntry1.BD_ADDR         = ConnectionEntry->BD_ADDR;

         if((TransactionEntry1Ptr = AddTransactionEntry(&(ConnectionEntry->TransactionEntryList), &TransactionEntry1)) != NULL)
         {
            /* Configure the CCCD for notifications.                    */
            ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&CCCDValue, GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_NOTIFY_ENABLE);

            /* Perform the write to configure the CCCD.                 */
            Result = GATM_WriteValue(GATMEventCallbackID, ConnectionEntry->BD_ADDR, ConnectionEntry->ConsumerConnectionInfo.DataSourceCCCD, NON_ALIGNED_WORD_SIZE, (Byte_t *)&CCCDValue);

            if(Result > 0)
            {
               /* Save the GATM Transaction ID.                         */
               TransactionEntry1Ptr->GATTTransactionID = (unsigned int)Result;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_VERBOSE), ("Error GATM_WriteValue() returned %d\n", Result));

               /* Delete the transaction information memory and free the*/
               /* memory.                                               */
               if((TransactionEntry1Ptr = DeleteTransactionEntry(&(ConnectionEntry->TransactionEntryList), TransactionEntry1Ptr->TransactionID)) != NULL)
                  FreeTransactionEntryMemory(TransactionEntry1Ptr);

               /* Flag that an error occurred.                          */
               NoError = FALSE;
            }
         }
         else
            NoError = FALSE;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", (int)NoError));

   /* Finally return the result to the caller.                          */
   return(NoError);
}

   /* The following function is a utility function used to parse LE     */
   /* service data for the Apple Notification Center Service.  It is    */
   /* called by ConfigureANCSClientConnection() in order to parse and   */
   /* save this service data.  It returns a Boolean value that is TRUE  */
   /* if everything was successful.                                     */
static Boolean_t ConfigureANCSClient(Connection_Entry_t *ConnectionEntry, GATT_Service_Discovery_Indication_Data_t *Service)
{
   Word_t                                        NotificationSourceHandle = 0;
   Word_t                                        NotificationSourceCCCD   = 0;
   Word_t                                        ControlPointHandle       = 0;
   Word_t                                        DataSourceHandle         = 0;
   Word_t                                        DataSourceCCCD           = 0;
   Boolean_t                                     ret_val                  = FALSE;
   unsigned int                                  Index;
   unsigned int                                  Index1;
   GATT_Characteristic_Information_t            *Characteristic;
   GATT_Characteristic_Descriptor_Information_t *Descriptor;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure that the input parameters appear to be semi-valid.      */
   if((ConnectionEntry) && (Service))
   {
      /* Mark that we are currently configuring this service.           */
      ConnectionEntry->ConnectionFlags |= ANCM_CONNECTION_ENTRY_FLAGS_CONFIGURING_ANCS_SERVICE;

      /* Grab the first characteristic.                                 */
      Characteristic = Service->CharacteristicInformationList;

      /* Look through each characteristic.                              */
      for(Index = 0; Index < Service->NumberOfCharacteristics; Index++, Characteristic++)
      {
         /* We only care about UUID_128.                                */
         if(Characteristic->Characteristic_UUID.UUID_Type == guUUID_128)
         {
            /* Check for the Notification Source Characteristic.        */
            if(ANCS_COMPARE_NOTIFICATION_SOURCE_UUID_TO_UUID_128(Characteristic->Characteristic_UUID.UUID.UUID_128))
            {
               /* Note the handle.                                      */
               NotificationSourceHandle = Characteristic->Characteristic_Handle;

               Descriptor = Characteristic->DescriptorList;

               /* Look for the CCCD for Notification Source.            */
               for(Index1 = 0; Index1 < Characteristic->NumberOfDescriptors; Index1++, Descriptor++)
               {
                  /* Found the CCCD for Notification Source.            */
                  if((Descriptor->Characteristic_Descriptor_UUID.UUID_Type == guUUID_16) && (GATT_COMPARE_CLIENT_CHARACTERISTIC_CONFIGURATION_ATTRIBUTE_TYPE_TO_BLUETOOTH_UUID_16(Descriptor->Characteristic_Descriptor_UUID.UUID.UUID_16)))
                     NotificationSourceCCCD = Descriptor->Characteristic_Descriptor_Handle;
               }
            }
            else
            {
               /* Check for the Control Point Characteristic.           */
               if(ANCS_COMPARE_CONTROL_POINT_UUID_TO_UUID_128(Characteristic->Characteristic_UUID.UUID.UUID_128))
               {
                  /* Note the handle.                                   */
                  ControlPointHandle  = Characteristic->Characteristic_Handle;
               }
               else
               {
                  /*Check for the Data Source Characteristic.           */
                  if(ANCS_COMPARE_DATA_SOURCE_UUID_TO_UUID_128(Characteristic->Characteristic_UUID.UUID.UUID_128))
                  {
                     /* Note the handle.                                */
                     DataSourceHandle = Characteristic->Characteristic_Handle;

                     Descriptor = Characteristic->DescriptorList;

                     /* Look for the CCCD for Notification Source.      */
                     for(Index1 = 0; Index1 < Characteristic->NumberOfDescriptors; Index1++, Descriptor++)
                     {
                        /* Found the CCCD for Notification Source.      */
                        if((Descriptor->Characteristic_Descriptor_UUID.UUID_Type == guUUID_16) && (GATT_COMPARE_CLIENT_CHARACTERISTIC_CONFIGURATION_ATTRIBUTE_TYPE_TO_BLUETOOTH_UUID_16(Descriptor->Characteristic_Descriptor_UUID.UUID.UUID_16)))
                           DataSourceCCCD = Descriptor->Characteristic_Descriptor_Handle;
                     }
                  }
               }
            }
         }
      }

      /* All characteristics and CCC descriptors are required for       */
      /* ANCS support.                                                  */
      if((NotificationSourceHandle) && (NotificationSourceCCCD) && (DataSourceHandle) && (DataSourceCCCD) && (ControlPointHandle))
      {
         /* Update the Connection Entry.                                */
         ConnectionEntry->ConsumerConnectionInfo.NotificationSourceHandle = NotificationSourceHandle;
         ConnectionEntry->ConsumerConnectionInfo.NotificationSourceCCCD   = NotificationSourceCCCD;
         ConnectionEntry->ConsumerConnectionInfo.DataSourceHandle         = DataSourceHandle;
         ConnectionEntry->ConsumerConnectionInfo.DataSourceCCCD           = DataSourceCCCD;
         ConnectionEntry->ConsumerConnectionInfo.ControlPointHandle       = ControlPointHandle;

         DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_VERBOSE), ("Configured ANCS Successfully:\n"));
         DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_VERBOSE), ("Notification Source Handle: %d\n", ConnectionEntry->ConsumerConnectionInfo.NotificationSourceHandle));
         DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_VERBOSE), ("Notification Source CCCD Handle: %d\n", ConnectionEntry->ConsumerConnectionInfo.NotificationSourceCCCD));
         DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_VERBOSE), ("Data Source Handle: %d\n", ConnectionEntry->ConsumerConnectionInfo.DataSourceHandle));
         DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_VERBOSE), ("Data Source CCCD Handle: %d\n", ConnectionEntry->ConsumerConnectionInfo.DataSourceCCCD));
         DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_VERBOSE), ("Control Point Handle: %d\n", ConnectionEntry->ConsumerConnectionInfo.ControlPointHandle));

         ret_val = ConfigureCCCDescriptors(ConnectionEntry);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", (int)ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is a utility function to retrieve the      */
   /* service for an LE connection and parse it for a remote Apple      */
   /* Notification Center Service Notification Provider.  This function */
   /* takes as its sole parameter the Connection Entry on which ANCS is */
   /* being configured and returns a Boolean value indicating whether   */
   /* configuration was successful.                                     */
   /* * NOTE * Both characteristics Notification Source and Data Source */
   /*          will NOT be subscribed until the API function            */
   /*          ANCM_Enable_Notifications() is called to enable          */
   /*          notifications.                                           */
static Boolean_t ConfigureANCSClientConnection(Connection_Entry_t *ConnectionEntry)
{
   int                                       Result;
   Boolean_t                                 ret_val = FALSE;
   Byte_t                                   *ServiceData;
   unsigned int                              TotalServiceSize;
   unsigned int                              Index;
   DEVM_Parsed_Services_Data_t               ParsedGATTData;
   GATT_Service_Discovery_Indication_Data_t *Service;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure that the input parameters appear to be semi-valid.      */
   if(ConnectionEntry)
   {
      /* Make sure that we are neither configuring nor have already     */
      /* configured the ANCS Service on the remote device.              */
      if((!(ConnectionEntry->ConnectionFlags & ANCM_CONNECTION_ENTRY_FLAGS_ANCS_SERVICE_CONFIGURED)) && (!(ConnectionEntry->ConnectionFlags & ANCM_CONNECTION_ENTRY_FLAGS_CONFIGURING_ANCS_SERVICE)))
      {
         /* Determine the size of the service data.                     */
         if(!DEVM_QueryRemoteDeviceServices(ConnectionEntry->BD_ADDR, DEVM_QUERY_REMOTE_DEVICE_SERVICES_FLAGS_LOW_ENERGY, 0, NULL, &TotalServiceSize))
         {
            /* Allocate a buffer to hold the service data.              */
            if((ServiceData = BTPS_AllocateMemory(TotalServiceSize)) != NULL)
            {
               /* Get the service data.                                 */
               if((Result = DEVM_QueryRemoteDeviceServices(ConnectionEntry->BD_ADDR, DEVM_QUERY_REMOTE_DEVICE_SERVICES_FLAGS_LOW_ENERGY, TotalServiceSize, ServiceData, NULL)) >= 0)
               {
                  TotalServiceSize = (unsigned int)Result;

                  /* Convert the Raw Service data to Parsed Service     */
                  /* Data.                                              */
                  if(!DEVM_ConvertRawServicesStreamToParsedServicesData(TotalServiceSize, ServiceData, &ParsedGATTData))
                  {
                     Service = ParsedGATTData.GATTServiceDiscoveryIndicationData;

                     /* Check each service.                             */
                     for(Index=0;Index < ParsedGATTData.NumberServices;Index++,Service++)
                     {
                        /* We only care about guUUID_128 since this is  */
                        /* not a SIG defined profile.                   */
                        if(Service->ServiceInformation.UUID.UUID_Type == guUUID_128)
                        {
                           /* Check for the ANCS service and configure  */
                           /* if found.                                 */
                           if(ANCS_COMPARE_ANCS_SERVICE_UUID_TO_UUID_128(Service->ServiceInformation.UUID.UUID.UUID_128))
                           {
                              /* Attempt to configure the ANCS service. */
                              ret_val = ConfigureANCSClient(ConnectionEntry, Service);

                              /* since only 1 instance of the ANCS      */
                              /* service is allowed we can exit the loop*/
                              /* now.                                   */
                              break;
                           }
                        }
                     }

                     /* Free the parsed service data.                   */
                     DEVM_FreeParsedServicesData(&ParsedGATTData);
                  }
               }

               /* Free the allocated service memory.                    */
               BTPS_FreeMemory(ServiceData);
            }
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", (int)ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is the function that is called to process  */
   /* an LE Connection Event.                                           */
   /* * NOTE * This function *MUST* be called with the ANCM Manager     */
   /*          Lock held.                                               */
static void ProcessLowEnergyConnectionEvent(BD_ADDR_t BD_ADDR)
{
   Connection_Entry_t *ConnectionEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if(!COMPARE_NULL_BD_ADDR(BD_ADDR))
   {
      /* Attempt to add the Connection Entry.                           */
      if((ConnectionEntryPtr = SearchAddConnectionEntry(&ConnectionEntryList, BD_ADDR)) != NULL)
      {
         /* Attempt to configure an ANCS client connection if the       */
         /* service exists.                                             */
         if(!ConfigureANCSClientConnection(ConnectionEntryPtr))
         {
            /* If we failed to configure the ANCS Service go ahead and  */
            /* delete the entry that was added for this device.         */
            if((ConnectionEntryPtr = DeleteConnectionEntryByBD_ADDR(&ConnectionEntryList, BD_ADDR)) != NULL)
               FreeConnectionEntryMemory(ConnectionEntryPtr);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the function that is called to process  */
   /* an LE Disconnect Event.                                           */
   /* * NOTE * This function *MUST* be called with the ANC Manager Lock */
   /*          held.                                                    */
static void ProcessLowEnergyDisconnectionEvent(BD_ADDR_t BD_ADDR)
{
   Connection_Entry_t *ConnectionEntryPtr;
   Byte_t              ErrorCode;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if(!COMPARE_NULL_BD_ADDR(BD_ADDR))
   {
      /* Delete the specified Connection Entry from the list.           */
      if((ConnectionEntryPtr = DeleteConnectionEntryByBD_ADDR(&ConnectionEntryList, BD_ADDR)) != NULL)
      {
         /* If there are any pending Get Attribute requests, raise an   */
         /* event that they have failed.                                */
         if((ConnectionEntryPtr->ConnectionFlags & ANCM_CONNECTION_ENTRY_FLAGS_GET_ATTRIBUTE_REQUEST_PENDING) && !(ConnectionEntryPtr->ConnectionFlags & ANCM_CONNECTION_ENTRY_FLAGS_REFRESHING))
         {
            /* Mark the Error Code so that the callback knows that this */
            /* was a result of disconnection.                           */
            ErrorCode = ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR;

            if(ConnectionEntryPtr->AttributeDataType == adtNotification)
            {
               /* There is an outstanding Get Notification Attributes   */
               /* request, so let any registered callbacks know.        */
               DispatchANCGetNotificationAttributesResponseEvent(NULL, ConnectionEntryPtr->BD_ADDR, ErrorCode);
            }
            else
            {
               /* There is an outstanding Get App Attributes Request,   */
               /* so let any registered callbacks know.                 */
               DispatchANCGetAppAttributesResponseEvent(NULL, ConnectionEntryPtr->BD_ADDR, ErrorCode);
            }
         }

         /* Dispatch an ANCM Disconnection Event.                       */
         DispatchANCDisconnectionEvent(ConnectionEntryPtr);

         /* Free the memory that was allocated for this entry.          */
         FreeConnectionEntryMemory(ConnectionEntryPtr);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the function that is called to process  */
   /* an LE Address Change Event.                                       */
   /* * NOTE * This function *MUST* be called with the ANC Manager Lock */
   /*          held.                                                    */
static void ProcessLowEnergyAddressChangeEvent(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties)
{
   Connection_Entry_t *ConnectionEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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
            DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_VERBOSE), ("ANCM Address Updated\n"));

            /* Save the new Base Address.                               */
            ConnectionEntryPtr->BD_ADDR = RemoteDeviceProperties->BD_ADDR;
         }

         ConnectionEntryPtr = ConnectionEntryPtr->NextConnectionEntryPtr;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a Remote Device Properties Changed Event.                 */
   /* * NOTE * This function *MUST* be called with the ANC Manager Lock */
   /*          held.                                                    */
static void ProcessRemoteDevicePropertiesChangedEvent(unsigned long ChangedMemberMask, DEVM_Remote_Device_Properties_t *RemoteDeviceProperties)
{
   unsigned long ConnectionFlags;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08lX\n", ChangedMemberMask));

   /* First make sure that the input parameter appears to be semi-valid.*/
   if((ChangedMemberMask) && (RemoteDeviceProperties) && (RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_SUPPORTS_LOW_ENERGY))
   {
      /* Check to see what changed.  We are only interested if the LE   */
      /* Connection State or the LE Service state or the Address is     */
      /* updated.                                                       */
      if(ChangedMemberMask & (DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_CONNECTION_STATE | DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_ENCRYPTION_STATE | DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_PRIOR_RESOLVABLE_ADDRESS | DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_PAIRING_STATE | DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_SERVICES_STATE))
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Flags: 0x%08lX\n", RemoteDeviceProperties->RemoteDeviceFlags));

         /* Handle Connections/Disconnections.                          */
         if(ChangedMemberMask & (DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_CONNECTION_STATE | DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_SERVICES_STATE | DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_PAIRING_STATE | DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_ENCRYPTION_STATE))
         {
            /* Set the required flags for a logical ANCS connection to  */
            /* occur.                                                   */
            ConnectionFlags = (DEVM_REMOTE_DEVICE_FLAGS_DEVICE_CURRENTLY_CONNECTED_OVER_LE | DEVM_REMOTE_DEVICE_FLAGS_DEVICE_CURRENTLY_PAIRED_OVER_LE | DEVM_REMOTE_DEVICE_FLAGS_DEVICE_LE_LINK_CURRENTLY_ENCRYPTED | DEVM_REMOTE_DEVICE_FLAGS_DEVICE_LE_SERVICES_KNOWN);

            /* Check to see if we are currently connected or a          */
            /* connection was just disconnected.                        */
            if((RemoteDeviceProperties->RemoteDeviceFlags & ConnectionFlags) == ConnectionFlags)
            {
               /* Process the Low Energy Connection.                    */
               ProcessLowEnergyConnectionEvent(RemoteDeviceProperties->BD_ADDR);
            }
            else
            {
               /* Process the Low Energy Disconnection.                 */
               ProcessLowEnergyDisconnectionEvent(RemoteDeviceProperties->BD_ADDR);
            }
         }

         /* Process the Address Updated event if necessary.             */
         if(ChangedMemberMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_PRIOR_RESOLVABLE_ADDRESS)
            ProcessLowEnergyAddressChangeEvent(RemoteDeviceProperties);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a GATT Handle Value notification.                         */
   /* * NOTE * This function *MUST* be called with the ANC Manager Lock */
   /*          held.                                                    */
static void ProcessGATTHandleValueData(GATM_Handle_Value_Data_Event_Data_t *HandleValueEventData)
{
   Connection_Entry_t      *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if(HandleValueEventData)
   {
      /* Search for the connection entry for this device.               */
      if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, HandleValueEventData->RemoteDeviceAddress)) != NULL)
      {
         /* Next check to see if this is a notification or indication   */
         /* of a Notification Source characteristic.                    */
         if(HandleValueEventData->AttributeHandle == ConnectionEntry->ConsumerConnectionInfo.NotificationSourceHandle)
         {
            /* Verify that this is a notification.                      */
            if(HandleValueEventData->HandleValueIndication == FALSE)
            {
               /* Attempt to process the new Notification from the      */
               /* Notification Provider (iOS).                          */
               ProcessNotificationSourceNotification(ConnectionEntry, HandleValueEventData);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_VERBOSE), ("Expected notification of Notification Source Characteristic\n"));
            }
         }
         else
         {
            /* Next check to see if this is a notification or           */
            /* indication of a Data Source characteristic.              */
            if(HandleValueEventData->AttributeHandle == ConnectionEntry->ConsumerConnectionInfo.DataSourceHandle)
            {
               /* If there is a timer waiting on this, stop it for now. */
               if(ConnectionEntry->AttributeRequestTimerID)
                  TMR_StopTimer(ConnectionEntry->AttributeRequestTimerID);

               /* Verify that this is a notification.                   */
               if(HandleValueEventData->HandleValueIndication == FALSE)
               {
                  /* Attempt to process the Data Source notification.   */
                  ProcessDataSourceNotification(ConnectionEntry, HandleValueEventData);
               }
               else
               {
                  DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_VERBOSE), ("Expected notification of Data Source Characteristic\n"));
               }
            }
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a GATT Write Response.                                    */
   /* * NOTE * This function *MUST* be called with the ANC Manager Lock */
   /*          held.                                                    */
static void ProcessGATTWriteResponse(GATM_Write_Response_Event_Data_t *WriteResponse)
{
   Transaction_Entry_t *TransactionEntry;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if(WriteResponse)
   {
      /* Search for the connection entry for this device.               */
      if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, WriteResponse->RemoteDeviceAddress)) != NULL)
      {
         /* Delete the Transaction Info for this response.              */
         if((TransactionEntry = SearchTransactionEntryGATTID(&(ConnectionEntry->TransactionEntryList), WriteResponse->TransactionID)) != NULL)
         {
            /* Process the GATT Transaction Response.                   */
            ProcessGATTTransactionResponse(ConnectionEntry, TransactionEntry, FALSE, (void *)WriteResponse);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a GATT Error Response.                                    */
   /* * NOTE * This function *MUST* be called with the ANC Manager Lock */
   /*          held.                                                    */
static void ProcessGATTErrorResponse(GATM_Error_Response_Event_Data_t *ErrorResponse)
{
   Transaction_Entry_t *TransactionEntry;
   Connection_Entry_t  *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if(ErrorResponse)
   {
      /* Search for the connection entry for this device.               */
      if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, ErrorResponse->RemoteDeviceAddress)) != NULL)
      {
         /* Delete the Transaction Info for this response.              */
         if((TransactionEntry = SearchTransactionEntryGATTID(&(ConnectionEntry->TransactionEntryList), ErrorResponse->TransactionID)) != NULL)
         {
            /* Process the GATT Transaction Response.                   */
            ProcessGATTTransactionResponse(ConnectionEntry, TransactionEntry, TRUE, (void *)ErrorResponse);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a GATT Transaction Response.                              */
static void ProcessGATTTransactionResponse(Connection_Entry_t *ConnectionEntry, Transaction_Entry_t *TransactionEntry, Boolean_t ErrorResponse, void *EventData)
{
   Byte_t ErrorCode;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if((ConnectionEntry) && (TransactionEntry) && (EventData))
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_VERBOSE), ("Transaction Response: Status = 0x%02X\n", (ErrorResponse?((GATM_Error_Response_Event_Data_t *)EventData)->AttributeProtocolErrorCode:0)));

      /* Process the correct transaction.                               */
      switch(TransactionEntry->TransactionType)
      {
         case ttGetNotificationAttributes:
            DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_VERBOSE), ("Transaction Response: ttGetNotificationAttributes\n"));

            if(ErrorResponse)
            {
               /* Cleanup the Get App/Notification Attributes Request   */
               /* operation.                                            */
               CleanupGetAttributeOperation(ConnectionEntry);

               /* Get the error code.                                   */
               if(((GATM_Error_Response_Event_Data_t *)EventData)->ErrorType == retErrorResponse)
                  ErrorCode = ((GATM_Error_Response_Event_Data_t *)EventData)->AttributeProtocolErrorCode;
               else
                  ErrorCode = ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR;

               DispatchANCGetNotificationAttributesResponseEvent(NULL, ConnectionEntry->BD_ADDR, ErrorCode);
            }
            else
            {
               /* Start a timer for the operation.                      */
               ConnectionEntry->AttributeRequestTimerID = TMR_StartTimer(NULL, TimerCallback, ANCM_TRANSACTION_TIMEOUT_VALUE);
            }
            break;
         case ttGetAppAttributes:
            DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_VERBOSE), ("Transaction Response: ttGetAppAttributes\n"));

            if(ErrorResponse)
            {
               /* Cleanup the Get App/Notification Attributes Request   */
               /* operation.                                            */
               CleanupGetAttributeOperation(ConnectionEntry);

               /* Get the error code.                                   */
               if(((GATM_Error_Response_Event_Data_t *)EventData)->ErrorType == retErrorResponse)
                  ErrorCode = ((GATM_Error_Response_Event_Data_t *)EventData)->AttributeProtocolErrorCode;
               else
                  ErrorCode = ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR;

               DispatchANCGetAppAttributesResponseEvent(NULL, ConnectionEntry->BD_ADDR, ErrorCode);
            }
            else
            {
               /* Start a timer for the operation.                      */
               ConnectionEntry->AttributeRequestTimerID = TMR_StartTimer(NULL, TimerCallback, ANCM_TRANSACTION_TIMEOUT_VALUE);
            }
            break;
         case ttConfigureNotificationSourceCCCD:
            DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_VERBOSE), ("Transaction Response: ttConfigureNotificationSourceCCCD\n"));

            if(!ErrorResponse)
               ConnectionEntry->ConnectionFlags |= ANCM_CONNECTION_ENTRY_FLAGS_NOTIFICATION_SOURCE_CCCD_SUBSCRIBED;
            else
               ClearNotificationQueue(ConnectionEntry, FALSE);
            break;
         case ttConfigureDataSourceCCCD:
            DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_VERBOSE), ("Transaction Response: ttConfigureDataSourceCCCD\n"));

            if(!ErrorResponse)
               ConnectionEntry->ConnectionFlags |= ANCM_CONNECTION_ENTRY_FLAGS_DATA_SOURCE_CCCD_SUBSCRIBED;
            else
               ClearNotificationQueue(ConnectionEntry, FALSE);
            break;
         case ttRefreshUnregister:
            DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_VERBOSE), ("Transaction Response: ttRefreshUnreigster\n"));

            if(!ErrorResponse)
            {
               /* Just call the function to submit the re-register.     */
               ProcessRefreshUnregisterResponse(ConnectionEntry, TransactionEntry->CallbackID);
            }
            else
            {
               //XXX: Do nothing for now (we are still connected and registered).
            }

            break;
         case ttRefreshRegister:
            DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_VERBOSE), ("Transaction Response: ttRefreshReigster\n"));

            if(!ErrorResponse)
            {
               /* Clear out the flags noting the refresh was in         */
               /* progress.                                             */
               ConnectionEntry->ConnectionFlags &= ~(((unsigned long)ANCM_CONNECTION_ENTRY_FLAGS_REFRESHING | ANCM_CONNECTION_ENTRY_FLAGS_GET_ATTRIBUTE_REQUEST_PENDING));
            }
            else
            {
               /* Re-register failed, so we are no longer configured.   */
               ProcessLowEnergyDisconnectionEvent(ConnectionEntry->BD_ADDR);
            }
            break;
         default:
            DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled response type\n"));
            break;
      }

      /* Delete the Transaction Info.                                   */
      if((TransactionEntry = DeleteTransactionEntry(&(ConnectionEntry->TransactionEntryList), TransactionEntry->TransactionID)) != NULL)
         FreeTransactionEntryMemory(TransactionEntry);

      /* Check to see if we need to flag that.                          */
      if((ConnectionEntry->ConnectionFlags & ANCM_CONNECTION_ENTRY_FLAGS_CONFIGURING_ANCS_SERVICE) && (ConnectionEntry->ConnectionFlags & ANCM_CONNECTION_ENTRY_FLAGS_NOTIFICATION_SOURCE_CCCD_SUBSCRIBED) && (ConnectionEntry->ConnectionFlags & ANCM_CONNECTION_ENTRY_FLAGS_DATA_SOURCE_CCCD_SUBSCRIBED))
      {
         /* Mark that we are ready to use ANCS on this remote device.   */
         ConnectionEntry->ConnectionFlags |= (unsigned long)ANCM_CONNECTION_ENTRY_FLAGS_ANCS_SERVICE_CONFIGURED;

         /* Mark that we are done configuring this device.              */
         ConnectionEntry->ConnectionFlags &= ~((unsigned long)ANCM_CONNECTION_ENTRY_FLAGS_CONFIGURING_ANCS_SERVICE);

         /* We are now connected & configured, so notify any registered */
         /* callbacks of a connection.                                  */
         DispatchANCConnectionEvent(ConnectionEntry);

         /* Now dispatch any notifications that were queue while we were*/
         /* configuring.                                                */
         ClearNotificationQueue(ConnectionEntry, TRUE);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a Get Notification Attributes request and write it to the */
   /* Control Point characteristic.  It returns the Transaction ID of   */
   /* the new Write request.                                            */
static int ProcessGetNotificationAttributesRequest(ANCM_Event_Callback_Info_t *EventCallbackPtr, Connection_Entry_t *ConnectionEntry, DWord_t NotificationUID, unsigned int NumberOfAttributes, ANCM_Notification_Attribute_Request_Data_t *AttributeRequestData)
{
   int                  ret_val;
   Byte_t              *Buffer;
   Byte_t              *BufferPtr;
   unsigned int         Index;
   unsigned int         BufferLength;
   Transaction_Entry_t  TransactionEntry;
   Transaction_Entry_t *TransactionEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Initialize some default values.                                   */
   ret_val      = 0;
   BufferLength = ANCS_NOTIFICATION_ATTRIBUTE_REQUEST_PACKET_SIZE(0);

   /* Format the new transaction.                                       */
   BTPS_MemInitialize(&TransactionEntry, 0, sizeof(Transaction_Entry_t));

   TransactionEntry.TransactionID               = GetNextTransactionID();
   TransactionEntry.BD_ADDR                     = ConnectionEntry->BD_ADDR;
   TransactionEntry.CallbackID                  = EventCallbackPtr->EventCallbackID;

   ConnectionEntry->NumberOfAttributesRequested = NumberOfAttributes;
   ConnectionEntry->AttributeDataType           = adtNotification;

   /* First, we need to find out how big of a buffer we need.           */
   for(Index = 0; Index < NumberOfAttributes; Index++)
   {
      /* Increase the buffer size by one for each Attribute ID.         */
      BufferLength += NON_ALIGNED_BYTE_SIZE;

      /* Check to see if a Max Length parameter is required for this    */
      /* Notification Attribute ID.  If so, make sure the caller        */
      /* provided one.                                                  */
      if(MaxLengthParameterRequired(adtNotification, AttributeRequestData[Index].NotificationAttributeID, 0))
      {
         if((AttributeRequestData[Index].AttributeMaxLength) > 0)
         {
            /* Increase the buffer size by two for the 2-byte Max       */
            /* Length parameter.                                        */
            BufferLength += NON_ALIGNED_WORD_SIZE;
         }
         else
         {
            /* A max length parameter was not provided, so cancel the   */
            /* request.                                                 */
            ret_val = BTPM_ERROR_CODE_ANCS_INVALID_COMMAND_DATA;

            /* Break out of the for() loop since the command data is    */
            /* invalid.                                                 */
            break;
         }
      }
   }

   /* Check to see if any errors have occurred.                         */
   if(!ret_val)
   {
      /* Go ahead and build the value that needs to be written.         */
      if((Buffer = BTPS_AllocateMemory(BufferLength)) != NULL)
      {
         /* Format the data buffer.                                     */
         BTPS_MemInitialize(Buffer, 0, BufferLength);

         /* Set the Command ID to 0 for Get Notification Attributes.    */
         ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&(((ANCS_Notification_Attribute_Request_Packet_t *)Buffer)->CommandID), ANCS_COMMAND_ID_GET_NOTIFICATION_ATTRIBUTES);

         /* Copy over the Notification UID.                             */
         ASSIGN_HOST_DWORD_TO_LITTLE_ENDIAN_UNALIGNED_DWORD(&(((ANCS_Notification_Attribute_Request_Packet_t *)Buffer)->NotificationUID), NotificationUID);

         /* Assign a temporary buffer into the location in memory to    */
         /* store the requested attributes.                             */
         BufferPtr  = &(((ANCS_Notification_Attribute_Request_Packet_t *)Buffer)->AttributeRequestData[0]);

         /* Copy over the Attribute Request Data.                       */
         for(Index = 0; Index < NumberOfAttributes; Index++)
         {
            /* Copy over the Notification Attribute ID.                 */
            ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(BufferPtr, AttributeRequestData[Index].NotificationAttributeID);

            BufferPtr += NON_ALIGNED_BYTE_SIZE;

            /* If required, copy over the Max Length parameter.         */
            if(MaxLengthParameterRequired(adtNotification, AttributeRequestData[Index].NotificationAttributeID, 0))
            {
               ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(BufferPtr, AttributeRequestData[Index].AttributeMaxLength);

               BufferPtr += NON_ALIGNED_WORD_SIZE;
            }
         }

         /* Attempt to add the transaction entry for this request.      */
         if((TransactionEntryPtr = AddTransactionEntry(&(ConnectionEntry->TransactionEntryList), &TransactionEntry)) != NULL)
         {
            /* Write the value to the Control Point characteristic.     */
            if((ret_val = GATM_WriteValue(GATMEventCallbackID, ConnectionEntry->BD_ADDR, ConnectionEntry->ConsumerConnectionInfo.ControlPointHandle, BufferLength, Buffer)) > 0)
            {
               /* Mark that a Get Attribute request is pending so that  */
               /* someone doesn't try to send another request.          */
               ConnectionEntry->ConnectionFlags             |= ANCM_CONNECTION_ENTRY_FLAGS_GET_ATTRIBUTE_REQUEST_PENDING;

               /* Mark the number of attributes that we are waiting on. */
               ConnectionEntry->NumberOfAttributesRequested  = NumberOfAttributes;

               /* Save the GATM Transaction ID.                         */
               TransactionEntryPtr->GATTTransactionID        = (unsigned int)ret_val;

               /* Return the Transaction ID to the caller.              */
               ret_val                                       = TransactionEntryPtr->TransactionID;
            }
            else
            {
               /* Failed to send the request.  Clean up the transaction.*/
               if((TransactionEntryPtr = DeleteTransactionEntry(&(ConnectionEntry->TransactionEntryList), TransactionEntryPtr->TransactionID)) != NULL)
                  FreeTransactionEntryMemory(TransactionEntryPtr);
            }
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;

         /* Free the memory allocated for the request packet.  Either   */
         /* the packet has been submitted to the GATM module (in which  */
         /* case GATM has made it's own copy of the data or submitted it*/
         /* to GATT which has done so) or an error occurred.  Either way*/
         /* the memory must be freed.                                   */
         BTPS_FreeMemory(Buffer);
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function that is used to      */
   /* process a Refresh Notifications rqeuest. This function returns    */
   /* zero if successful.                                               */
static int ProcessRefreshNotificationsRequest(ANCM_Event_Callback_Info_t *EventCallbackPtr, Connection_Entry_t *ConnectionEntry)
{
   int                  ret_val;
   Word_t               CCCDValue;
   Transaction_Entry_t  TransactionEntry;
   Transaction_Entry_t *TransactionEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the parameters are semi-valid.                 */
   if((EventCallbackPtr) && (ConnectionEntry))
   {
      /* Format the new transaction.                                    */
      BTPS_MemInitialize(&TransactionEntry, 0, sizeof(Transaction_Entry_t));

      TransactionEntry.TransactionID               = GetNextTransactionID();
      TransactionEntry.BD_ADDR                     = ConnectionEntry->BD_ADDR;
      TransactionEntry.CallbackID                  = EventCallbackPtr->EventCallbackID;
      TransactionEntry.TransactionType             = ttRefreshUnregister;

      if((TransactionEntryPtr = AddTransactionEntry(&(ConnectionEntry->TransactionEntryList), &TransactionEntry)) != NULL)
      {
         /* Disable the notifications first.                            */
         CCCDValue = 0;

         if((ret_val = GATM_WriteValue(GATMEventCallbackID, ConnectionEntry->BD_ADDR, ConnectionEntry->ConsumerConnectionInfo.NotificationSourceCCCD, NON_ALIGNED_WORD_SIZE, (Byte_t *)&CCCDValue)) > 0)
         {
            /* Mark that a request is pending so that someone doesn't   */
            /* try to send another request.                             */
            ConnectionEntry->ConnectionFlags      |= ((unsigned long)(ANCM_CONNECTION_ENTRY_FLAGS_GET_ATTRIBUTE_REQUEST_PENDING | ANCM_CONNECTION_ENTRY_FLAGS_REFRESHING));

            /* Save the GATM Transaction ID.                            */
            TransactionEntryPtr->GATTTransactionID = (unsigned int)ret_val;

            /* Return success to the caller.                            */
            ret_val                                = 0;
         }
         else
         {
            /* Failed to send the request.  Clean up the transaction.   */
            if((TransactionEntryPtr = DeleteTransactionEntry(&(ConnectionEntry->TransactionEntryList), TransactionEntryPtr->TransactionID)) != NULL)
               FreeTransactionEntryMemory(TransactionEntryPtr);
         }
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function that is used to      */
   /* handle a Get App Attributes request and write it to the Control   */
   /* Point characteristic.  It returns the Transaction ID of the new   */
   /* Write request.                                                    */
static int  ProcessGetAppAttributesRequest(ANCM_Event_Callback_Info_t *EventCallbackPtr, Connection_Entry_t *ConnectionEntry, char *AppIdentifier, unsigned int NumberOfAttributes, ANCM_App_Attribute_Request_Data_t *AttributeRequestData)
{
   int                  ret_val;
   Byte_t              *Buffer;
   Byte_t              *BufferPtr;
   unsigned int         AppIDLength;
   unsigned int         BufferLength;
   unsigned int         Index;
   Transaction_Entry_t  TransactionEntry;
   Transaction_Entry_t *TransactionEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Initialize some default values.                                   */
   ret_val      = 0;
   BufferLength = ANCS_APP_ATTRIBUTE_REQUEST_PACKET_SIZE(0, 0);

   /* Format the new transaction.                                       */
   BTPS_MemInitialize(&TransactionEntry, 0, sizeof(Transaction_Entry_t));

   TransactionEntry.TransactionID               = GetNextTransactionID();
   TransactionEntry.BD_ADDR                     = ConnectionEntry->BD_ADDR;
   TransactionEntry.CallbackID                  = EventCallbackPtr->EventCallbackID;

   ConnectionEntry->NumberOfAttributesRequested = NumberOfAttributes;
   ConnectionEntry->AttributeDataType           = adtApplication;

   /* First, we need to find out how big of a buffer we need.           */
   for(Index = 0; Index < NumberOfAttributes; Index++)
   {
      /* Increase the buffer size by one for each Attribute ID.         */
      BufferLength += NON_ALIGNED_BYTE_SIZE;

      /* Check to see if a Max Length parameter is required for this    */
      /* App Attribute ID.  If so, make sure the caller provided one.   */
      if(MaxLengthParameterRequired(adtApplication, 0, AttributeRequestData[Index].AppAttributeID))
      {
         if((AttributeRequestData[Index].AttributeMaxLength) > 0)
         {
            /* Increase the buffer size by two for the 2-byte Max       */
            /* Length parameter.                                        */
            BufferLength += NON_ALIGNED_WORD_SIZE;
         }
         else
         {
            /* A max length parameter was not provided, so cancel the   */
            /* request.                                                 */
            ret_val = BTPM_ERROR_CODE_ANCS_INVALID_COMMAND_DATA;

            /* Break out of the for() loop since the command data is    */
            /* invalid.                                                 */
            break;
         }
      }
   }

   /* Calculate the size of the Application Identifier string (plus the */
   /* required NULL terminator).                                        */
   AppIDLength   = (BTPS_StringLength(AppIdentifier) + 1);

   /* Next calculate the size of the buffer that is rquired to hold the */
   /* entire request packet.                                            */
   BufferLength += AppIDLength;

   /* Check to see if any errors have occurred.                         */
   if(!ret_val)
   {
      /* Go ahead and build the value that needs to be written.         */
      if((Buffer = BTPS_AllocateMemory(BufferLength)) != NULL)
      {
         /* Format the data buffer.                                     */
         BTPS_MemInitialize(Buffer, 0, BufferLength);

         /* Set the Command ID to 1 for Get App Attributes.             */
         ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&(((ANCS_App_Attribute_Request_Packet_t *)Buffer)->CommandID), ANCS_COMMAND_ID_GET_APP_ATTRIBUTES);

         /* Assign a pointer into the location in memory to store the   */
         /* Application Identifer followed by the requested attributes. */
         BufferPtr  = &(((ANCS_App_Attribute_Request_Packet_t *)Buffer)->AppIdAndAttributeData[0]);

         /* Copy over the App Identifier.                               */
         BTPS_StringCopy((char *)BufferPtr, AppIdentifier);

         BufferPtr += AppIDLength;

         /* Copy over the Attribute Request Data.                       */
         for(Index = 0; Index < NumberOfAttributes; Index++)
         {
            /* Copy over the App Attribute ID.                          */
            ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(BufferPtr, AttributeRequestData[Index].AppAttributeID);

            BufferPtr += NON_ALIGNED_BYTE_SIZE;

            /* If required, copy over the Max Length parameter.         */
            if(MaxLengthParameterRequired(adtApplication, 0, AttributeRequestData[Index].AppAttributeID))
            {
               ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(BufferPtr, AttributeRequestData[Index].AttributeMaxLength);

               BufferPtr += NON_ALIGNED_WORD_SIZE;
            }
         }

         /* Attempt to add the transaction entry for this request.      */
         if((TransactionEntryPtr = AddTransactionEntry(&(ConnectionEntry->TransactionEntryList), &TransactionEntry)) != NULL)
         {
            /* Write the value to the Control Point characteristic.     */
            if((ret_val = GATM_WriteValue(GATMEventCallbackID, ConnectionEntry->BD_ADDR, ConnectionEntry->ConsumerConnectionInfo.ControlPointHandle, BufferLength, Buffer)) > 0)
            {
               /* Mark for now that a Get Attribute request is pending  */
               /* so that someone doesn't try to send another request.  */
               ConnectionEntry->ConnectionFlags             |= ANCM_CONNECTION_ENTRY_FLAGS_GET_ATTRIBUTE_REQUEST_PENDING;

               /* Mark the number of attributes that we are waiting on. */
               ConnectionEntry->NumberOfAttributesRequested  = NumberOfAttributes;

               /* Save the GATM Transaction ID.                         */
               TransactionEntryPtr->GATTTransactionID        = (unsigned int)ret_val;

               /* Return the Transaction ID to the caller.              */
               ret_val                                       = TransactionEntryPtr->TransactionID;
            }
            else
            {
               /* Failed to send the request.  Clean up the transaction.*/
               if((TransactionEntryPtr = DeleteTransactionEntry(&(ConnectionEntry->TransactionEntryList), TransactionEntryPtr->TransactionID)) != NULL)
                  FreeTransactionEntryMemory(TransactionEntryPtr);
            }
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;

         /* Free the memory allocated for the request packet.  Either   */
         /* the packet has been submitted to the GATM module (in which  */
         /* case GATM has made it's own copy of the data or submitted it*/
         /* to GATT which has done so) or an error occurred.  Either way*/
         /* the memory must be freed.                                   */
         BTPS_FreeMemory(Buffer);
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function to handle a Query    */
   /* Connected Devices request via ANCM_Query_Connected_Devices.  It   */
   /* copies the Bluetooth Device Addresses of all connected devices    */
   /* which have ANCS present into the list RemoteDevices, up to a      */
   /* maximum number specified by MaximumNumberDevices.  It assigns to  */
   /* the parameter TotalNumberDevices the total number of devices that */
   /* have ANCS present and are connected, regardless of                */
   /* MaximumNumberDevices.  This function returns the number of        */
   /* devices that were copied into the list RemoteDevices.             */
static int ProcessQueryConnectedDevices(unsigned int MaximumNumberDevices, BD_ADDR_t *RemoteDevices, unsigned int *TotalNumberDevices)
{
   int                 ret_val;
   unsigned int        NumberDevices;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Initialize at zero devices copied.                                */
   ret_val       = 0;
   NumberDevices = 0;

   /* Start at the front of the connection list.                        */
   ConnectionEntry = ConnectionEntryList;

   /* Walk the list of connections.                                     */
   while(ConnectionEntry)
   {
      /* Make sure we are actually connected to ANCS.                   */
      if(ConnectionEntry->ConnectionFlags & ANCM_CONNECTION_ENTRY_FLAGS_ANCS_SERVICE_CONFIGURED)
      {
         /* Copy the data if we have room in the buffer.                */
         if((RemoteDevices) && (ret_val < MaximumNumberDevices))
         {
            RemoteDevices[ret_val] = ConnectionEntry->BD_ADDR;

            /* Increment the number of devices we have copied.          */
            ret_val++;
         }

         /* Increment the total number of devices.                      */
         NumberDevices++;
      }

      /* Advance to the next entry.                                     */
      ConnectionEntry = ConnectionEntry->NextConnectionEntryPtr;
   }

   /* Set the Total Number parameter if supplied.                       */
   if(TotalNumberDevices)
      *TotalNumberDevices = NumberDevices;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d.\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is a utility function used to parse raw    */
   /* data received on the Notification Source characteristic into data */
   /* that can be returned as a part of a Notification Received Event.  */
   /* This function returns 0 if successful or a negative value if an   */
   /* error occurred.                                                   */
static int DecodeNotification(unsigned int DataLength, Byte_t *Data, ANCS_Notification_Data_t *NotificationData)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters appear semi-valid.               */
   if((DataLength) && (Data) && (NotificationData))
   {
      /* Verify that the Notification is a valid length.                */
      if(DataLength == ANCS_NOTIFICATION_PACKET_SIZE)
      {
         /* Parse the relevant parameters from the notifications.       */
         NotificationData->EventID         = READ_UNALIGNED_BYTE_LITTLE_ENDIAN(&(((ANCS_Notification_Packet_t *)Data)->EventID));
         NotificationData->EventFlags      = READ_UNALIGNED_BYTE_LITTLE_ENDIAN(&(((ANCS_Notification_Packet_t *)Data)->EventFlags));
         NotificationData->CategoryID      = READ_UNALIGNED_BYTE_LITTLE_ENDIAN(&(((ANCS_Notification_Packet_t *)Data)->CategoryID));
         NotificationData->CategoryCount   = READ_UNALIGNED_BYTE_LITTLE_ENDIAN(&(((ANCS_Notification_Packet_t *)Data)->CategoryCount));
         NotificationData->NotificationUID = READ_UNALIGNED_DWORD_LITTLE_ENDIAN(&(((ANCS_Notification_Packet_t *)Data)->NotificationUID));

         /* Initialize the return value to success.                     */
         ret_val = 0;
      }
      else
         ret_val = BTPM_ERROR_CODE_ANCS_UNABLE_TO_PARSE_DATA;
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is a utility function used to parse raw    */
   /* data received on the Data Source characteristic into data that    */
   /* represents a response to a Get Notification Attributes request.   */
   /* This function returns the number of attributes that have not yet  */
   /* been received - i.e. 0 if we have received all the data or a      */
   /* positive number if we are still waiting on packets.  This         */
   /* function returns a negative value if an error occurred.           */
static int DecodeGetNotificationAttributesResponse(Connection_Entry_t *ConnectionEntry, Word_t BufferLength, Byte_t *Buffer, ANCS_Get_Notification_Attributes_Response_Data_t *ResponseData)
{
   int           ret_val;
   Byte_t       *BufferPtr;
   Word_t        TempLength;
   Word_t        AttributeBufferLength;
   unsigned int  Index;
   unsigned int  NumberOfAttributesFound;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %u\n", (unsigned int)BufferLength));

   /* Verify that the input parameters appear semi-valid.               */
   if((ConnectionEntry) && (BufferLength > ANCS_NOTIFICATION_ATTRIBUTE_RESPONSE_PACKET_SIZE(0)) && (Buffer) && (ResponseData))
   {
      /* Initialize the return value to success.                        */
      ret_val = 0;

      /* Initialize the data that we will be passing along to be sent   */
      /* with a Callback Event to zero.                                 */
      BTPS_MemInitialize(ResponseData, 0, sizeof(ResponseData));

      /* Calculate the length of the attribute list in the notification.*/
      AttributeBufferLength   = BufferLength - ANCS_NOTIFICATION_ATTRIBUTE_RESPONSE_PACKET_SIZE(0);

      /* Now we need to iterate through the remaining attribute data    */
      /* and determine if we have received all of the data.             */
      BufferPtr               = ((ANCS_Notification_Attribute_Response_Packet_t *)Buffer)->AttributeResponseData;
      Index                   = 0;
      NumberOfAttributesFound = 0;
      while(Index < AttributeBufferLength)
      {
         /* Advance past the App Attribute ID.                          */
         Index      += NON_ALIGNED_BYTE_SIZE;

         /* Read in the length of this attribute.                       */
         TempLength  = READ_UNALIGNED_WORD_LITTLE_ENDIAN(&(BufferPtr[Index]));

         /* Add the lengths of the Attribute Length and Attribute Data  */
         /* fields to the Index.                                        */
         Index      += (NON_ALIGNED_WORD_SIZE + TempLength);

         /* Verify that all of the attribute data for this attribute is */
         /* contained in this notification.                             */
         if(Index <= AttributeBufferLength)
         {
            NumberOfAttributesFound++;

            DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_VERBOSE), ("Found %u Attributes in buffer.\n", NumberOfAttributesFound));
         }
      }

      /* Check to see if we have received everything.                   */
      if(NumberOfAttributesFound < ConnectionEntry->NumberOfAttributesRequested)
      {
         /* Return to the caller the number of attributes that we are   */
         /* still waiting on.                                           */
         ret_val = ConnectionEntry->NumberOfAttributesRequested - NumberOfAttributesFound;
      }
      else
      {
         /* Read in the Notification UID.                               */
         ResponseData->NotificationUID     = READ_UNALIGNED_DWORD_LITTLE_ENDIAN(&(((ANCS_Notification_Attribute_Response_Packet_t *)Buffer)->NotificationUID));

         /* Calculate the length of the Attribute Data.                 */
         ResponseData->AttributeDataLength = AttributeBufferLength;
         ResponseData->AttributeData       = ((ANCS_Notification_Attribute_Response_Packet_t *)Buffer)->AttributeResponseData;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is a utility function used to parse raw    */
   /* data received on the Data Source characteristic into data that    */
   /* represents a response to a Get App Attributes request.  This      */
   /* function returns the number of attributes that have not yet been  */
   /* received - i.e. 0 if we have received all the data or a positive  */
   /* number if we are still waiting on packets.  This function returns */
   /* a negative value if an error occurred.                            */
static int DecodeGetAppAttributesResponse(Connection_Entry_t *ConnectionEntry, Word_t BufferLength, Byte_t *Buffer, ANCS_Get_App_Attributes_Response_Data_t *ResponseData)
{
   int           ret_val;
   char         *AppIdentifer;
   Word_t        TempLength;
   unsigned int  Index;
   unsigned int  AppIdentifierLength;
   unsigned int  NumberOfAttributesFound;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %u\n", BufferLength));

   /* Verify that the input parameters appear semi-valid.               */
   if((ConnectionEntry) && (BufferLength > ANCS_APP_ATTRIBUTE_RESPONSE_PACKET_SIZE(0, 0)) && (Buffer))
   {
      /* Initialize some default values.                                */
      Index                   = 0;
      ret_val                 = 0;
      AppIdentifierLength     = 0;
      NumberOfAttributesFound = 0;

      /* Initialize the data that we will be passing along to be sent   */
      /* with a Callback Event to zero.                                 */
      BTPS_MemInitialize(ResponseData, 0, sizeof(ResponseData));

      /* Determine the length of the AppIdentifer.                      */
      AppIdentifer  = (char *)&(((ANCS_App_Attribute_Response_Packet_t *)Buffer)->AppIdAndAttributeResponseData[0]);
      BufferLength -= ANCS_APP_ATTRIBUTE_RESPONSE_PACKET_SIZE(0, 0);
      Index         = 0;
      while((AppIdentifer[Index] != '\0') && (Index < BufferLength))
      {
         Index++;
         AppIdentifierLength++;
      }

      /* Increment past the NULL terminator.                            */
      Index++;

      /* Continue only if there is more data left in the packet to be   */
      /* parsed after the app identifer.                                */
      if(Index < BufferLength)
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_VERBOSE), ("App Identifier: %s\n", AppIdentifer));

         /* Set the buffer pointer into the beginning of the Attribute  */
         /* ID/Attribute Length/Attribute Data tuple.                   */
         Buffer        = (Byte_t *)&(AppIdentifer[Index]);
         BufferLength -= (AppIdentifierLength + NON_ALIGNED_BYTE_SIZE);

         /* Now we need to iterate through the remaining attribute data */
         /* and determine if we have received all of the data.          */

         /* Set our Index past the Command ID and App Identifier.       */
         Index = 0;
         while(Index < BufferLength)
         {
            /* Advance past the Attribute ID.                           */
            Index      += NON_ALIGNED_BYTE_SIZE;

            /* Read in the length of this attribute.                    */
            TempLength  = READ_UNALIGNED_WORD_LITTLE_ENDIAN(&(Buffer[Index]));

            /* Add the lengths of the Attribute Length and Attribute    */
            /* Data fields to the Index.                                */
            Index      += (NON_ALIGNED_WORD_SIZE + TempLength);

            /* Verify that all of the attribute data for this attribute */
            /* is contained in this notification.                       */
            if(Index <= BufferLength)
            {
               NumberOfAttributesFound++;

               DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_VERBOSE), ("Found %u Attributes in buffer.\n", NumberOfAttributesFound));
            }
         }

         /* Check to see if we have received everything.                */
         if(NumberOfAttributesFound < ConnectionEntry->NumberOfAttributesRequested)
         {
            /* Return to the caller the number of attributes that we    */
            /* are still waiting on.                                    */
            ret_val = ConnectionEntry->NumberOfAttributesRequested - NumberOfAttributesFound;
         }
         else
         {
            /* Set the return value to 0.                               */
            ret_val = 0;

            /* Set our Buffer Pointer past the Command ID and App       */
            /* Identifier.                                              */
            ResponseData->AppIdentifier       = AppIdentifer;
            ResponseData->AttributeDataLength = BufferLength;
            ResponseData->AttributeData       = Buffer;
         }
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function represents the GATM Event Callback that is */
   /* installed to process all GATM Events.                             */
static void BTPSAPI GATM_Event_Callback(GATM_Event_Data_t *EventData, void *CallbackParameter)
{
   /* Verify that the input parameters are semi-valid.                  */
   if(EventData)
   {
      /* Next, let's check to see if we need to process it.             */
      if(Initialized)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(ANCM_AcquireLock())
         {
            switch(EventData->EventType)
            {
               case getGATTHandleValueData:
                  /* Process the GATT Handle Value Data event.          */
                  ProcessGATTHandleValueData(&(EventData->EventData.HandleValueDataEventData));
                  break;
               case getGATTWriteResponse:
                  /* Process the GATT Write Response.                   */
                  ProcessGATTWriteResponse(&(EventData->EventData.WriteResponseEventData));
                  break;
               case getGATTErrorResponse:
                  /* Process the GATT Error Response.                   */
                  ProcessGATTErrorResponse(&(EventData->EventData.ErrorResponseEventData));
                  break;
               default:
                  DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled GATM Event\n"));
                  break;
            }

            /* Release the lock because we are finished with it.        */
            ANCM_ReleaseLock();
         }
      }
   }
}

   /* The following function is responsible for handling a notification */
   /* that occurs on the Notification Source characteristic.            */
static void ProcessNotificationSourceNotification(Connection_Entry_t *ConnectionEntry, GATM_Handle_Value_Data_Event_Data_t *HandleValueEventData)
{
   ANCS_Notification_Data_t NotificationData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if((ConnectionEntry) && (HandleValueEventData))
   {
      /* Attempt to decode the data received into a parsed Notification.*/
      if(!DecodeNotification(HandleValueEventData->AttributeValueLength, HandleValueEventData->AttributeValue, &NotificationData))
      {
         /* If we are still configuring the connection, we need to      */
         /* queue any incoming notifications until after we dispatch    */
         /* connected.                                                  */
         if(ConnectionEntry->ConnectionFlags & ANCM_CONNECTION_ENTRY_FLAGS_CONFIGURING_ANCS_SERVICE)
         {
            /* Add the notification to the connection's queue.          */
            AddNotificationToQueue(ConnectionEntry, &NotificationData);
         }
         else
         {
            /* If successful, go ahead and dispatch an event with the   */
            /* data contained in the new Notification.                  */
            DispatchANCNotificationReceivedEvent(&NotificationData, ConnectionEntry->BD_ADDR);
         }
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_VERBOSE), ("DecodeNotification() failed to parse ANCS notification data.\n"));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for handling a notification */
   /* that occurs on the Data Source characteristic.                    */
static void ProcessDataSourceNotification(Connection_Entry_t *ConnectionEntry, GATM_Handle_Value_Data_Event_Data_t *HandleValueEventData)
{
   int                                               Result;
   Byte_t                                           *TempBuffer;
   unsigned int                                      TempLength;
   ANCS_Get_App_Attributes_Response_Data_t           AppResponseData;
   ANCS_Get_Notification_Attributes_Response_Data_t  NotificationResponseData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if((ConnectionEntry) && (ConnectionEntry->ConnectionFlags & ANCM_CONNECTION_ENTRY_FLAGS_GET_ATTRIBUTE_REQUEST_PENDING) && (HandleValueEventData))
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_VERBOSE), ("Value size: %u\n", (unsigned int)(HandleValueEventData->AttributeValueLength)));
      DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_VERBOSE), ("Enter:\n"));

      /* Initialize the result to success.                              */
      Result     = 0;

      /* Calculate the total amount of data that we have received so    */
      /* far for this Get Attributes response.                          */
      TempLength = ConnectionEntry->AttributeDataLength + HandleValueEventData->AttributeValueLength;
      TempLength = (TempLength < ANCM_DEFAULT_ATTRIBUTE_BUFFER_DATA_SIZE) ? ANCM_DEFAULT_ATTRIBUTE_BUFFER_DATA_SIZE : TempLength;

      /* See if we need to allocate more memory for the buffer.         */
      if((TempLength > ConnectionEntry->AttributeDataLength) || (ConnectionEntry->AttributeData == NULL))
      {
         /* Allocate the memory buffer that is needed to hold the data. */
         if((TempBuffer = BTPS_AllocateMemory(TempLength)) != NULL)
         {
            /* If we had a buffer previously allocated for responses go */
            /* ahead and copy the data and free the existing buffer.    */
            if(ConnectionEntry->AttributeData)
            {
               /* Copy over any existing data.                          */
               BTPS_MemCopy(TempBuffer, ConnectionEntry->AttributeData, ConnectionEntry->AttributeDataLength);

               /* Free the existing buffer.                             */
               BTPS_FreeMemory(ConnectionEntry->AttributeData);
            }
            else
               ConnectionEntry->AttributeDataLength = 0;

            /* Save the new memory location.                            */
            ConnectionEntry->AttributeData = TempBuffer;
         }
         else
            Result = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
      }

      /* Continue only if no error has occurred.                        */
      if(!Result)
      {
         /* Append the new data onto the current buffer.                */
         BTPS_MemCopy(&(ConnectionEntry->AttributeData[ConnectionEntry->AttributeDataLength]), HandleValueEventData->AttributeValue, HandleValueEventData->AttributeValueLength);

         /* Increment the size of the stored data.                      */
         ConnectionEntry->AttributeDataLength += HandleValueEventData->AttributeValueLength;

         /* Check to see what kind of attribute this is.                */
         if(ConnectionEntry->AttributeDataType == adtNotification)
         {
            /* Process this as a Get Notification Attributes response.  */
            Result = DecodeGetNotificationAttributesResponse(ConnectionEntry, ConnectionEntry->AttributeDataLength, ConnectionEntry->AttributeData, &NotificationResponseData);
            if(Result == 0)
            {
               /* Store a temporary buffer to the Attribute Data stored */
               /* in the connection entry.  We will also set the pointer*/
               /* in the connection entry to NULL.  We do this so that  */
               /* CleanupGetAttributeOperation() (which must be called  */
               /* before the event is dispatched to allow further       */
               /* requests to be made in the callback) does not free    */
               /* memory that must be valid for the event to be         */
               /* dispatched.                                           */
               TempBuffer                     = ConnectionEntry->AttributeData;
               ConnectionEntry->AttributeData = NULL;

               /* Cleanup the operation flags.                          */
               CleanupGetAttributeOperation(ConnectionEntry);

               /* Decoding the Get Notification Attributes response was */
               /* successful.                                           */
               DispatchANCGetNotificationAttributesResponseEvent(&NotificationResponseData, ConnectionEntry->BD_ADDR, 0);

               /* Free the memory allocated to store this operation.    */
               BTPS_FreeMemory(TempBuffer);
            }
            else
            {
               if(Result > 0)
               {
                  /* We have not received the entire response yet.      */
                  DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_VERBOSE), ("DecodeGetNotificationAttributesResponse() indicates that the received response is incomplete.\n"));

                  /* Restart the timer waiting for response data.       */
                  ConnectionEntry->AttributeRequestTimerID = TMR_StartTimer(NULL, TimerCallback, ANCM_TRANSACTION_TIMEOUT_VALUE);
               }
               else
               {
                  DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_VERBOSE), ("DecodeGetNotificationAttributesResponse(%d) failed to parse ANCS notification data.\n", Result));

                  /* Cleanup the operation flags.                       */
                  CleanupGetAttributeOperation(ConnectionEntry);

                  /* Dispatch an event with an error.                   */
                  DispatchANCGetNotificationAttributesResponseEvent(NULL, ConnectionEntry->BD_ADDR, Result);
               }
            }
         }
         else
         {
            /* Process this as a Get App Attributes response.           */
            Result = DecodeGetAppAttributesResponse(ConnectionEntry, ConnectionEntry->AttributeDataLength, ConnectionEntry->AttributeData, &AppResponseData);
            if(Result ==  0)
            {
               /* Store a temporary buffer to the Attribute Data stored */
               /* in the connection entry.  We will also set the pointer*/
               /* in the connection entry to NULL.  We do this so that  */
               /* CleanupGetAttributeOperation() (which must be called  */
               /* before the event is dispatched to allow further       */
               /* requests to be made in the callback) does not free    */
               /* memory that must be valid for the event to be         */
               /* dispatched.                                           */
               TempBuffer                     = ConnectionEntry->AttributeData;
               ConnectionEntry->AttributeData = NULL;

               /* Cleanup the operation flags.                          */
               CleanupGetAttributeOperation(ConnectionEntry);

               /* Decoding the Get App Attributes response was          */
               /* successful.                                           */
               DispatchANCGetAppAttributesResponseEvent(&AppResponseData, ConnectionEntry->BD_ADDR, 0);

               /* Free the memory allocated to store this operation.    */
               BTPS_FreeMemory(TempBuffer);
            }
            else
            {
               if(Result > 0)
               {
                  /* We have not received the entire response yet.      */
                  DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_VERBOSE), ("DispatchANCGetAppAttributesResponseEvent() indicates that the received response is incomplete.\n"));

                  /* Restart the timer waiting for response data.       */
                  ConnectionEntry->AttributeRequestTimerID = TMR_StartTimer(NULL, TimerCallback, ANCM_TRANSACTION_TIMEOUT_VALUE);
               }
               else
               {
                  DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_VERBOSE), ("DecodeGetAppAttributesResponse(%d) failed to parse ANCS notification data.\n", Result));

                  /* Cleanup the operation flags.                       */
                  CleanupGetAttributeOperation(ConnectionEntry);

                  /* Dispatch an event with an error.                   */
                  DispatchANCGetAppAttributesResponseEvent(NULL, ConnectionEntry->BD_ADDR, Result);
               }
            }
         }
      }
      else
      {
         /* Cleanup the operation flags.                                */
         CleanupGetAttributeOperation(ConnectionEntry);

         /* Check to see what kind of attribute this is.                */
         if(ConnectionEntry->AttributeDataType == adtNotification)
         {
            /* Dispatch an event with an error.                         */
            DispatchANCGetNotificationAttributesResponseEvent(NULL, ConnectionEntry->BD_ADDR, Result);
         }
         else
         {
            /* Dispatch an event with an error.                         */
            DispatchANCGetAppAttributesResponseEvent(NULL, ConnectionEntry->BD_ADDR, Result);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function to handle a write    */
   /* response when refreshing notifications.                           */
static void ProcessRefreshUnregisterResponse(Connection_Entry_t *ConnectionEntry, unsigned int CallbackID)
{
   int                  Result;
   Word_t               CCCDValue;
   Transaction_Entry_t  TransactionEntry;
   Transaction_Entry_t *TransactionEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* First make sure the parameters seem valid.                        */
   if((ConnectionEntry) && (CallbackID))
   {
      /* Fromat the new transaction.                                    */
      BTPS_MemInitialize(&TransactionEntry, 0, sizeof(Transaction_Entry_t));

      TransactionEntry.TransactionID   = GetNextTransactionID();
      TransactionEntry.CallbackID      = CallbackID;
      TransactionEntry.BD_ADDR         = ConnectionEntry->BD_ADDR;
      TransactionEntry.TransactionType = ttRefreshRegister;

      /* Attempt to add the transaction to the list.                    */
      if((TransactionEntryPtr = AddTransactionEntry(&(ConnectionEntry->TransactionEntryList), &TransactionEntry)) != NULL)
      {
         ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&CCCDValue, GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_NOTIFY_ENABLE);

         if((Result = GATM_WriteValue(GATMEventCallbackID, TransactionEntryPtr->BD_ADDR, ConnectionEntry->ConsumerConnectionInfo.NotificationSourceCCCD, NON_ALIGNED_WORD_SIZE, (Byte_t *)&CCCDValue)) > 0)
         {
            /* Note the transaction ID.                                 */
            TransactionEntryPtr->GATTTransactionID = (unsigned int)Result;
         }
         else
         {
            /* An error occurred, and we are no longer                  */
            /* configured. Dispatch a disconnect.                       */
            DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_VERBOSE), ("GATM Write failed: %d\n", Result));
            ProcessLowEnergyDisconnectionEvent(ConnectionEntry->BD_ADDR);
         }
      }
      else
      {
         /* An error occurred, and we are no longer configured. Dispatch*/
         /* a disconnect.                                               */
         DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_VERBOSE), ("Add returned null\n"));
         ProcessLowEnergyDisconnectionEvent(ConnectionEntry->BD_ADDR);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the callback function that is           */
   /* registered when calling TMR_StartTimer.  This timer is set to     */
   /* prevent an infinite wait when there is an outstanding Get         */
   /* App Attributes or Get Notification Attributes request.            */
static Boolean_t BTPSAPI TimerCallback(unsigned int TimerID, void *CallbackParameter)
{
   Byte_t              ErrorCode;
   Connection_Entry_t *ConnectionEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Next, let's check to see if we need to process it.                */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(ANCM_AcquireLock())
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_VERBOSE), ("Processing Timer Callback...\n"));

         /* Process the procedure timeout.                              */
         if((ConnectionEntryPtr = SearchConnectionEntryByTimerID(&ConnectionEntryList, TimerID)) != NULL)
         {
            /* Check to see if there is still a Get Attribute request   */
            /* pending.                                                 */
            if(ConnectionEntryPtr->ConnectionFlags & ANCM_CONNECTION_ENTRY_FLAGS_GET_ATTRIBUTE_REQUEST_PENDING)
            {
               /* Set the error code for the Event Callback.            */
               ErrorCode = (Byte_t)ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR;

               /* Cleanup the Get App/Notification Attributes Request   */
               /* operation.                                            */
               CleanupGetAttributeOperation(ConnectionEntryPtr);

               if(ConnectionEntryPtr->AttributeDataType == adtNotification)
               {
                  /* Let the callback know that a timeout occurred.     */
                  DispatchANCGetNotificationAttributesResponseEvent(NULL, ConnectionEntryPtr->BD_ADDR, ErrorCode);
               }
               else
               {
                  /* Let the callback know that a timeout occurred.     */
                  DispatchANCGetAppAttributesResponseEvent(NULL, ConnectionEntryPtr->BD_ADDR, ErrorCode);
               }
            }
         }

         /* Release the lock because we are finished with it.           */
         ANCM_ReleaseLock();
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));

   return(FALSE);
}

   /* The following function is responsible for initializing/cleaning up*/
   /* the Bluetopia Platform Manager ANCS Manager Module.  This function*/
   /* should be registered with the Bluetopia Platform Manager Module   */
   /* Handler and will be called when the Platform Manager is           */
   /* initialized (or shut down).                                       */
void BTPSAPI ANCM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData)
{
   int                              Result;
   BD_ADDR_t                       *LEDeviceList;
   unsigned int                     NumberDevices;
   unsigned int                     Index;
   unsigned long                    ConnectionFlags;
   Class_of_Device_t                EmptyCOD;
   DEVM_Remote_Device_Properties_t  RemoteProperties;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %d\n", Initialize));

   /* First, determine if the request is to initialize or shut down.    */
   if(Initialize)
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_VERBOSE), ("Initialization Request\n"));

      if(!Initialized)
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing ANCS Manager\n"));

         /* First, let's create a Mutex to guard access to this module. */
         if((ANCManagerMutex = BTPS_CreateMutex(FALSE)) != NULL)
         {
            /* Register a GATM Event Callback.                          */
            if((Result = GATM_RegisterEventCallback(GATM_Event_Callback, NULL)) > 0)
            {
               /* Save the GATM Event Callback ID.                      */
               GATMEventCallbackID     = (unsigned int)Result;

               /* Determine the current Device Power State.             */
               CurrentPowerState       = (Boolean_t)((DEVM_QueryDevicePowerState() > 0)?TRUE:FALSE);

               DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_VERBOSE), ("Initialization Success\n"));

               /* Initialize a unique, starting ANCM Callback ID.       */
               NextEventCallbackID     = 1;

               /* Initialize a unique, starting ANCM Transaction ID.    */
               NextTransactionID       = 1;

               ASSIGN_CLASS_OF_DEVICE(EmptyCOD, 0, 0, 0);

               /* If currently powered on attempt to get the list of    */
               /* currently connected LE devices.                       */
               if(CurrentPowerState)
               {
                  if(!(Result = DEVM_QueryRemoteDeviceList((DEVM_QUERY_REMOTE_DEVICE_LIST_CURRENTLY_CONNECTED | DEVM_QUERY_REMOTE_DEVICE_LIST_NO_BR_EDR_DEVICES), EmptyCOD, 0, NULL, &NumberDevices)))
                  {
                     if(NumberDevices)
                     {
                        if((LEDeviceList = (BD_ADDR_t *)BTPS_AllocateMemory(sizeof(BD_ADDR_t) * NumberDevices)) != NULL)
                        {
                           if((Result = DEVM_QueryRemoteDeviceList((DEVM_QUERY_REMOTE_DEVICE_LIST_CURRENTLY_CONNECTED | DEVM_QUERY_REMOTE_DEVICE_LIST_NO_BR_EDR_DEVICES), EmptyCOD, NumberDevices, LEDeviceList, NULL)) >= 0)
                           {
                              Result          = 0;
                              ConnectionFlags = (DEVM_REMOTE_DEVICE_FLAGS_DEVICE_CURRENTLY_CONNECTED_OVER_LE | DEVM_REMOTE_DEVICE_FLAGS_DEVICE_CURRENTLY_PAIRED_OVER_LE | DEVM_REMOTE_DEVICE_FLAGS_DEVICE_LE_SERVICES_KNOWN);

                              for(Index = 0; ((Index < NumberDevices) && (!Result)); Index++)
                              {
                                 if((Result = DEVM_QueryRemoteDeviceProperties(LEDeviceList[Index], DEVM_QUERY_REMOTE_DEVICE_PROPERTIES_FLAGS_LOW_ENERGY, &RemoteProperties)) == 0)
                                 {
                                    if((RemoteProperties.RemoteDeviceFlags & ConnectionFlags) == ConnectionFlags)
                                       ProcessLowEnergyConnectionEvent(RemoteProperties.BD_ADDR);
                                 }
                              }
                           }

                           BTPS_FreeMemory(LEDeviceList);
                        }
                     }
                  }
               }

               /* Flag that this module is initialized.                 */
               Initialized = TRUE;

               Result = 0;
            }
         }
         else
            Result = BTPM_ERROR_CODE_UNABLE_TO_CREATE_MUTEX;

         /* If an error occurred then we need to free all resources that*/
         /* were allocated.                                             */
         if(Result < 0)
         {
            DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_VERBOSE), ("Initialization Failed.\n"));

            if(ANCManagerMutex)
               BTPS_CloseMutex(ANCManagerMutex);

            /* Flag that none of the resources are allocated.           */
            ANCManagerMutex = NULL;
         }
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_VERBOSE), ("ANCS Manager Already Initialized\n"));
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_VERBOSE), ("Shutdown Request\n"));

      if(Initialized)
      {
         /* Wait for access to the Mutex that guards access to this     */
         /* module.                                                     */
         if(BTPS_WaitMutex(ANCManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Free the Connection Info List.                           */
            FreeConnectionEntryList(&ConnectionEntryList);

            /* Free the Event Callback Info List.                       */
            FreeEventCallbackInfoList(&EventCallbackInfoList);

            BTPS_CloseMutex(ANCManagerMutex);

            /* Flag that the device is not powered on.                  */
            CurrentPowerState       = FALSE;

            /* Flag that the resources are no longer allocated.         */
            ANCManagerMutex = NULL;

            /* Flag that this module is no longer initialized.          */
            Initialized     = FALSE;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", Initialized));
}

   /* The following function is responsible for processing asynchronous */
   /* Device Manager (DEVM) Events (including Power On/Off events).     */
   /* This function should be registered with the Bluetopia Platform    */
   /* Manager Module Handler and will be called when an asynchronous    */
   /* Device Manager event is dispatched.                               */
void BTPSAPI ANCM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData)
{
   Connection_Entry_t *ConnectionEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANCS Manager has been initialized.  */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(ANCM_AcquireLock())
      {
         switch(EventData->EventType)
         {
            case detDevicePoweredOn:
               DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power On Occurred.\n"));

               /* Power on Event.                                       */

               /* Note the new Power State.                             */
               CurrentPowerState = TRUE;
               break;
            case detDevicePoweringOff:
            case detDevicePoweredOff:
               DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power Off Occurred.\n"));

               /* Note the new Power State.                             */
               CurrentPowerState = FALSE;

               /* Finally free all Connection Entries (as there cannot  */
               /* be any active connections).                           */
               FreeConnectionEntryList(&ConnectionEntryList);

               /* Free the Event Callback Info List.                    */
               FreeEventCallbackInfoList(&EventCallbackInfoList);
               break;
            case detRemoteDevicePropertiesChanged:
               DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_VERBOSE), ("Remote Device Properties Changed.\n"));

               /* Process the Remote Device Properties Changed Event.   */
               ProcessRemoteDevicePropertiesChangedEvent(EventData->EventData.RemoteDevicePropertiesChangedEventData.ChangedMemberMask, &(EventData->EventData.RemoteDevicePropertiesChangedEventData.RemoteDeviceProperties));
               break;
            case detRemoteDeviceDeleted:
               DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_VERBOSE), ("Remote Device Deleted.\n"));

               /* Delete the specified Connection Entry from the list.  */
               if((ConnectionEntryPtr = DeleteConnectionEntryByBD_ADDR(&ConnectionEntryList, EventData->EventData.RemoteDeviceDeletedEventData.RemoteDeviceAddress)) != NULL)
               {
                  /* Dispatch an ANCM Disconnection Event.              */
                  DispatchANCDisconnectionEvent(ConnectionEntryPtr);

                  /* Free the memory that was allocated for this entry. */
                  if((ConnectionEntryPtr = DeleteConnectionEntryByBD_ADDR(&ConnectionEntryList, ConnectionEntryPtr->BD_ADDR)) != NULL)
                     FreeConnectionEntryMemory(ConnectionEntryPtr);
               }
               break;
            default:
               /* Do nothing.                                           */
               break;
         }

         /* Release the Lock because we are finished with it.           */
         ANCM_ReleaseLock();
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the ANCS             */
   /* Manager Service.  This Callback will be dispatched by the ANCS    */
   /* Manager when various ANCS Manager Events occur.  This function    */
   /* accepts the Callback Function and Callback Parameter              */
   /* (respectively) to call when a ANCS Manager Event needs to be      */
   /* dispatched.  This function returns a positive (non-zero) value if */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          ANCM_Un_Register_Consumer_Event_Callback() function to   */
   /*          un-register the callback from this module.               */
int BTPSAPI ANCM_Register_Consumer_Event_Callback(ANCM_Event_Callback_t CallbackFunction, void *CallbackParameter)
{
   int                        ret_val;
   ANCM_Event_Callback_Info_t EventCallbackEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANCS Manager has been initialized.  */
   if(Initialized)
   {
      /* Check to see if the input parameters appear to be semi-valid.  */
      if(CallbackFunction)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(ANCM_AcquireLock())
         {
            /* Attempt to add an entry into the Event Callback Entry    */
            /* list.                                                    */
            BTPS_MemInitialize(&EventCallbackEntry, 0, sizeof(ANCM_Event_Callback_Info_t));

            EventCallbackEntry.EventCallbackID   = GetNextEventCallbackID();
            EventCallbackEntry.EventCallback     = CallbackFunction;
            EventCallbackEntry.EventCallback     = CallbackFunction;
            EventCallbackEntry.CallbackParameter = CallbackParameter;

            if(AddEventCallbackInfoEntry(&EventCallbackInfoList, &EventCallbackEntry))
            {
               /* Set the return value.                                 */
               ret_val = EventCallbackEntry.EventCallbackID;
            }
            else
               ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;

            /* Release the Lock because we are finished with it.        */
            ANCM_ReleaseLock();
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_ANCS_CALLBACK_NOT_REGISTERED;
   }
   else
      ret_val = BTPM_ERROR_CODE_ANCS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered ANCS Manager Event Callback   */
   /* (registered via a successful call to the                          */
   /* ANCM_Register_Consumer_Event_Callback() function).  This          */
   /* function accepts as input the ANCS Manager Event Callback ID      */
   /* (return value from ANCM_Register_Consumer_Event_Callback()        */
   /* function).                                                        */
void BTPSAPI ANCM_Un_Register_Consumer_Event_Callback(unsigned int ConsumerCallbackID)
{
   ANCM_Event_Callback_Info_t *EventCallbackPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANCS Manager has been initialized   */
   /* and the parameter appears correct.                                */
   if((Initialized) && (ConsumerCallbackID))
   {
      /* Wait for access to the lock that guards access to this module. */
      if(ANCM_AcquireLock())
      {
         /* Make sure that this callback is actually registered.        */
         if((SearchEventCallbackInfoEntry(&EventCallbackInfoList, ConsumerCallbackID)) != NULL)
         {
            /* Delete the callback info from the list.                  */
            if((EventCallbackPtr = DeleteEventCallbackInfoEntry(&EventCallbackInfoList, ConsumerCallbackID)) != NULL)
            {
               /* Free the memory because we are finished with it.      */
               FreeEventCallbackInfoEntryMemory(EventCallbackPtr);
            }
         }

         /* Release the Lock because we are finished with it.           */
         ANCM_ReleaseLock();
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism to submit */
   /* a Get Notification Attributes request to a Notification Provider. */
   /* This function accepts as input the Callback ID (the return value  */
   /* from the ANCM_Register_Consumer_Event_Callback() function), the   */
   /* Bluetooth address of the Notification Provider, the Notification  */
   /* UID of the Notification for which Attributes are being requested, */
   /* the number of attributes that are being requested, and an array   */
   /* of ANCM_Notification_Attribute_Request_Data_t structures          */
   /* representing each attribute that is being requested.  This        */
   /* function returns a positive (non-zero) value if successful, or a  */
   /* negative error code if an error occurred.                         */
   /* * NOTE * Only one Get Notification Attributes or Get App          */
   /*          Attributes request can be pending at a time.  This       */
   /*          function will return an error if a request is still      */
   /*          pending.                                                 */
int BTPSAPI ANCM_Get_Notification_Attributes(unsigned int ConsumerCallbackID, BD_ADDR_t RemoteDeviceAddress, DWord_t NotificationUID, unsigned int NumberOfAttributes, ANCM_Notification_Attribute_Request_Data_t *AttributeRequestData)
{
   int                         ret_val;
   Connection_Entry_t         *ConnectionEntryPtr;
   ANCM_Event_Callback_Info_t *EventCallbackPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS  | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the module has been initialized and a   */
   /* callback has been registered.                                     */
   if((Initialized))
   {
      /* Check for a registered callback.  The list head should be NULL */
      /* if there are no callbacks registered.                          */
      if(EventCallbackInfoList)
      {
         /* Next, check to see if the input parameters appear to be     */
         /* semi-valid.                                                 */
         if((ConsumerCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (NumberOfAttributes > 0) && (AttributeRequestData))
         {
            /* Wait for access to the lock that guards access to this   */
            /* module.                                                  */
            if(ANCM_AcquireLock())
            {
               /* Check to see if the device is powered on.             */
               if(CurrentPowerState)
               {
                  /* Go ahead and find the Callback Info Entry for this */
                  /* Consumer Callback ID.                              */
                  if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, ConsumerCallbackID)) != NULL)
                  {
                     /* Go ahead and find the Connection Entry          */
                     /* associated with the Bluetooth Device Address.   */
                     if((ConnectionEntryPtr = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress)) != NULL)
                     {
                        /* Make sure that there isn't already a Get App */
                        /* Attributes or Get Notification Attributes    */
                        /* request outstanding.                         */
                        if(!(ConnectionEntryPtr->ConnectionFlags & ANCM_CONNECTION_ENTRY_FLAGS_GET_ATTRIBUTE_REQUEST_PENDING))
                        {
                           /* Make sure that the remote device supports */
                           /* ANCS and is configured.                   */
                           if(ConnectionEntryPtr->ConnectionFlags & ANCM_CONNECTION_ENTRY_FLAGS_ANCS_SERVICE_CONFIGURED)
                           {
                              /* Call the internal function to send the */
                              /* Get Notification Attributes command.   */
                              ret_val = ProcessGetNotificationAttributesRequest(EventCallbackPtr, ConnectionEntryPtr, NotificationUID, NumberOfAttributes, AttributeRequestData);
                           }
                           else
                              ret_val = BTPM_ERROR_CODE_ANCS_SERVICE_NOT_PRESENT;
                        }
                        else
                           ret_val = BTPM_ERROR_CODE_ANCS_REQUEST_CURRENTLY_PENDING;
                     }
                     else
                        ret_val = BTPM_ERROR_CODE_DEVICE_IS_NOT_CURRENTLY_CONNECTED;
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
               }
               else
                  ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

               /* Release the Lock because we are finished with it.     */
               ANCM_ReleaseLock();
            }
            else
               ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
         }
         else
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
      else
         ret_val = BTPM_ERROR_CODE_ANCS_CALLBACK_NOT_REGISTERED;
   }
   else
      ret_val = BTPM_ERROR_CODE_ANCS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to submit */
   /* a Get App Attributes request to a Notification Provider. This     */
   /* function accepts as input the Callback ID (the return value from  */
   /* the ANCM_Register_Consumer_Event_Callback() function), the        */
   /* Bluetooth address of the Notification Provider, the App           */
   /* Identifier of the application for which Attributes are being      */
   /* requested, the number of attributes that are being requested, and */
   /* an array of App Attribute IDs for each attribute that is being    */
   /* requested.  This function returns a positive (non-zero) value if  */
   /* successful, or a negative error code if an error occurred.        */
   /* * NOTE * Only one Get Notification Attributes or Get App          */
   /*          Attributes request can be pending at a time.  This       */
   /*          function will return an error if a request is still      */
   /*          pending.                                                 */
int BTPSAPI ANCM_Get_App_Attributes(unsigned int ConsumerCallbackID, BD_ADDR_t RemoteDeviceAddress, char *AppIdentifier, unsigned int NumberOfAttributes, ANCM_App_Attribute_Request_Data_t *AttributeRequestData)
{
   int                         ret_val;
   Connection_Entry_t         *ConnectionEntryPtr;
   ANCM_Event_Callback_Info_t *EventCallbackPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS  | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the module has been initialized and a   */
   /* callback has been registered.                                     */
   if((Initialized))
   {
      /* Check for a registered callback.  The list head should be NULL */
      /* if there are no callbacks registered.                          */
      if(EventCallbackInfoList)
      {
         /* Next, check to see if the input parameters appear to be     */
         /* semi-valid.                                                 */
         if((ConsumerCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (AppIdentifier) && (BTPS_StringLength(AppIdentifier)) && (NumberOfAttributes > 0) && (AttributeRequestData))
         {
            /* Wait for access to the lock that guards access to this   */
            /* module.                                                  */
            if(ANCM_AcquireLock())
            {
               /* Check to see if the device is powered on.             */
               if(CurrentPowerState)
               {
                  /* Go ahead and find the Callback Info Entry for this */
                  /* Consumer Callback ID.                              */
                  if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, ConsumerCallbackID)) != NULL)
                  {
                     /* Go ahead and find the Connection Entry          */
                     /* associated with the Bluetooth Device Address.   */
                     if((ConnectionEntryPtr = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress)) != NULL)
                     {
                        /* Make sure that there isn't already a Get App */
                        /* Attributes or Get Notification Attributes    */
                        /* request outstanding.                         */
                        if(!(ConnectionEntryPtr->ConnectionFlags & ANCM_CONNECTION_ENTRY_FLAGS_GET_ATTRIBUTE_REQUEST_PENDING))
                        {
                           /* Make sure that the remote device supports */
                           /* ANCS and is configured.                   */
                           if(ConnectionEntryPtr->ConnectionFlags & ANCM_CONNECTION_ENTRY_FLAGS_ANCS_SERVICE_CONFIGURED)
                           {
                              /* Call the internal function to send the */
                              /* Get App Attributes command.            */
                              ret_val = ProcessGetAppAttributesRequest(EventCallbackPtr, ConnectionEntryPtr, AppIdentifier, NumberOfAttributes, AttributeRequestData);
                           }
                           else
                              ret_val = BTPM_ERROR_CODE_ANCS_SERVICE_NOT_PRESENT;
                        }
                        else
                           ret_val = BTPM_ERROR_CODE_ANCS_REQUEST_CURRENTLY_PENDING;
                     }
                     else
                        ret_val = BTPM_ERROR_CODE_DEVICE_IS_NOT_CURRENTLY_CONNECTED;
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
               }
               else
                  ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

               /* Release the Lock because we are finished with it.     */
               ANCM_ReleaseLock();
            }
            else
               ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
         }
         else
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
      else
         ret_val = BTPM_ERROR_CODE_ANCS_CALLBACK_NOT_REGISTERED;
   }
   else
      ret_val = BTPM_ERROR_CODE_ANCS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is a utility function that exists to parse */
   /* the specified Raw Attribute Data Stream into Parsed Attribute     */
   /* Data in fhe form of ANCM_Parsed_Attribute_Data_t.  This function  */
   /* accepts as input the type of attribute data that is being parsed, */
   /* the length of the Raw stream (must be greater than zero),         */
   /* followed by a pointer to the actual Raw Attribute Data Stream.    */
   /* The final parameter is a pointer to a buffer that will contain    */
   /* the header information for the parsed data.  This function        */
   /* returns zero if successful or a negative value if an error        */
   /* occurred.                                                         */
   /* * NOTE * If this function is successful the final parameter *MUST**/
   /*          be passed to the ANCM_Free_Parsed_Attribute_Data() to    */
   /*          free any allocated resources that were allocated to      */
   /*          track the Parsed Attribute Data Stream.                  */
   /* * NOTE * The Raw Attribute Stream Buffer (third parameter) *MUST* */
   /*          remain active while the data is being processed.         */
int BTPSAPI ANCM_Convert_Raw_Attribute_Data_To_Parsed_Attribute_Data(ANCM_Attribute_Data_Type_t DataType, unsigned int RawDataLength, unsigned char *RawAttributeData, ANCM_Parsed_Attribute_Data_t *ParsedAttributeData)
{
   int                                 Index;
   int                                 ret_val;
   Byte_t                             *IndividualAttributeData   = NULL;
   Byte_t                              IndividualAttributeID;
   Word_t                              IndividualAttributeLength;
   unsigned int                        BytesRead;
   unsigned int                        NumberOfAttributes;
   unsigned char                      *RawAttributeDataPtr       = NULL;
   ANCM_App_Attribute_Data_t          *AppAttributeData          = NULL;
   ANCM_Notification_Attribute_Data_t *NotificationAttributeData = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS  | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First make sure that the input parameters appear to be semi-valid.*/
   if((RawDataLength >= 3) && (RawAttributeData) && (ParsedAttributeData))
   {
      /* Initialize the parsed attribute information to a known state.  */
      BTPS_MemInitialize(ParsedAttributeData, 0, ANCM_PARSED_ATTRIBUTE_DATA_SIZE);

      RawAttributeDataPtr = RawAttributeData;
      NumberOfAttributes  = 0;
      BytesRead           = 0;
      ret_val             = 0;

      /* First, we need to find out how many attributes there are.      */
      while((RawDataLength - BytesRead) > 0)
      {
         /* Increase the number of attributes read.                     */
         NumberOfAttributes++;

         /* Read in the Attribute ID.                                   */
         IndividualAttributeID = READ_UNALIGNED_BYTE_LITTLE_ENDIAN(RawAttributeDataPtr);

         RawAttributeDataPtr += sizeof(Byte_t);
         BytesRead           += sizeof(Byte_t);

         /* Read in the length of the attribute.                        */
         IndividualAttributeLength = READ_UNALIGNED_WORD_LITTLE_ENDIAN(RawAttributeDataPtr);

         RawAttributeDataPtr += sizeof(Word_t);
         BytesRead           += sizeof(Word_t);

         RawAttributeDataPtr += IndividualAttributeLength;
         BytesRead           += IndividualAttributeLength;
      }

      /* Mark the number of attributes that we found.                   */
      ParsedAttributeData->NumberOfAttributes = NumberOfAttributes;

      if(DataType == adtNotification)
      {
         /* Mark that this is a Get Notification Attributes response.   */
         ParsedAttributeData->Type = adtNotification;

         /* Allocate enough memory for each of the individual Attribute */
         /* Data structures.                                            */
         if((NotificationAttributeData = (ANCM_Notification_Attribute_Data_t *)BTPS_AllocateMemory((sizeof(ANCM_Notification_Attribute_Data_t)*NumberOfAttributes))) == NULL)
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;

         ParsedAttributeData->AttributeData.NotificationAttributeData = NotificationAttributeData;
      }
      else
      {
         /* Mark that this is a Get App Attributes response.            */
         ParsedAttributeData->Type = adtApplication;

         /* Allocate enough memory for each of the individual Attribute */
         /* Data structures.                                            */
         if((AppAttributeData = (ANCM_App_Attribute_Data_t *)BTPS_AllocateMemory((sizeof(ANCM_App_Attribute_Data_t)*NumberOfAttributes))) == NULL)
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;

         ParsedAttributeData->AttributeData.AppAttributeData = AppAttributeData;
      }

      RawAttributeDataPtr = RawAttributeData;
      NumberOfAttributes  = 0;
      BytesRead           = 0;
      Index               = 0;

      /* Loop through and read in all of the attributes.                */
      while(((RawDataLength - BytesRead) > 0) && (!ret_val))
      {
         /* Increase the number of attributes read.                     */
         NumberOfAttributes++;

         /* Read in the Attribute ID.                                   */
         IndividualAttributeID = READ_UNALIGNED_BYTE_LITTLE_ENDIAN(RawAttributeDataPtr);

         RawAttributeDataPtr += sizeof(Byte_t);
         BytesRead           += sizeof(Byte_t);

         /* Read in the length of the attribute.                        */
         IndividualAttributeLength = READ_UNALIGNED_WORD_LITTLE_ENDIAN(RawAttributeDataPtr);

         RawAttributeDataPtr += sizeof(Word_t);
         BytesRead           += sizeof(Word_t);

         /* Allocate enough space to copy over the data for this        */
         /* attribute.                                                  */
         if((IndividualAttributeData = (Byte_t *)BTPS_AllocateMemory(IndividualAttributeLength)) != NULL)
         {
            /* Go ahead and copy the data over.                         */
            BTPS_MemCopy(IndividualAttributeData, RawAttributeDataPtr, IndividualAttributeLength);

            RawAttributeDataPtr += IndividualAttributeLength;
            BytesRead           += IndividualAttributeLength;
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;

         if(DataType == adtNotification)
         {
            NotificationAttributeData[Index].AttributeID     = (ANCM_Notification_Attribute_ID_t)IndividualAttributeID;
            NotificationAttributeData[Index].AttributeLength = (unsigned int)IndividualAttributeLength;
            NotificationAttributeData[Index].AttributeData   = IndividualAttributeData;
         }
         else
         {
            AppAttributeData[Index].AttributeID     = (ANCM_App_Attribute_ID_t)IndividualAttributeID;
            AppAttributeData[Index].AttributeLength = (unsigned int)IndividualAttributeLength;
            AppAttributeData[Index].AttributeData   = IndividualAttributeData;
         }

         Index++;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   /* If there was an error, erase everything that we have created.     */
   if(ret_val)
   {
      /* Free any memory that was allocated.                            */
      if(AppAttributeData)
      {
         for(Index = 0; Index < NumberOfAttributes; Index++)
         {
            if((AppAttributeData[Index].AttributeData) != NULL)
               BTPS_FreeMemory(AppAttributeData[Index].AttributeData);
         }

         BTPS_FreeMemory(AppAttributeData);
      }

      if(NotificationAttributeData)
      {
         for(Index = 0; Index < NumberOfAttributes; Index++)
         {
            if((NotificationAttributeData[Index].AttributeData) != NULL)
               BTPS_FreeMemory(NotificationAttributeData[Index].AttributeData);
         }

         BTPS_FreeMemory(NotificationAttributeData);
      }

      BTPS_MemInitialize(ParsedAttributeData, 0, ANCM_PARSED_ATTRIBUTE_DATA_SIZE);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS  | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));

   return(ret_val);
}

   /* The following function is provided to allow a mechanism to free   */
   /* all resources that were allocated to parse a Raw Attribute Data   */
   /* Stream into Parsed Attribute Data.  See the                       */
   /* ANCM_ConvertRawAttributeDataToParsedAttributeData() function for  */
   /* more information.                                                 */
void BTPSAPI ANCM_Free_Parsed_Attribute_Data(ANCM_Parsed_Attribute_Data_t *ParsedAttributeData)
{
   unsigned int Index;
   unsigned int NumberOfAttributes;

   /* Verify that the parameter is not NULL.                            */
   if(ParsedAttributeData)
   {
      /* Get the number of Attributes for which we will have to free    */
      /* AttributeData memory.                                          */
      NumberOfAttributes = ParsedAttributeData->NumberOfAttributes;

      switch(ParsedAttributeData->Type)
      {
         case adtNotification:
            /* Loop through the Parsed Attribute Data and free the      */
            /* memory of the AttributeData buffer for each attribute.   */
            for(Index = 0; Index < NumberOfAttributes; Index++)
            {
               BTPS_FreeMemory(ParsedAttributeData->AttributeData.NotificationAttributeData[Index].AttributeData);
            }

            /* Free the remaining memory of the Notification Attribute  */
            /* Data.                                                    */
            BTPS_FreeMemory(ParsedAttributeData->AttributeData.NotificationAttributeData);

            /* Initialize the memory of the Parsed Attribute Data       */
            /* container to zero.                                       */
            BTPS_MemInitialize(ParsedAttributeData, 0, ANCM_PARSED_ATTRIBUTE_DATA_SIZE);
            break;
         case adtApplication:
            /* Loop through the Parsed Attribute Data and free the      */
            /* memory of the AttributeData buffer for each attribute.   */
            for(Index = 0; Index < NumberOfAttributes; Index++)
            {
               BTPS_FreeMemory(ParsedAttributeData->AttributeData.AppAttributeData[Index].AttributeData);
            }

            /* Free the remaining memory of the App Attribute Data.     */
            BTPS_FreeMemory(ParsedAttributeData->AttributeData.AppAttributeData);

            /* Initialize the memory of the Parsed Attribute Data       */
            /* container to zero.                                       */
            BTPS_MemInitialize(ParsedAttributeData, 0, ANCM_PARSED_ATTRIBUTE_DATA_SIZE);
            break;
         default:
            break;
      }
   }
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to determine if there are currently any connected ANCS    */
   /* devices.  This function accepts the buffer information to receive */
   /* any currently connected devices.  The first parameter specifies   */
   /* the maximum number of BD_ADDR entries that the buffer will        */
   /* support (i.e. can be copied into the buffer).  The next parameter */
   /* is optional and, if specified, will be populated with the         */
   /* Bluetooth addresses of any connected devices if the function is   */
   /* successful.  The final parameter can be used to retrieve the      */
   /* total number of connected devices (regardless of the size of the  */
   /* list specified by the first two parameters).  This function       */
   /* returns a non-negative value if successful which represents the   */
   /* number of connected devices that were copied into the specified   */
   /* input buffer.  This function returns a negative return error code */
   /* if there was an error.                                            */
   /* * NOTE * It is possible to specify zero for the Maximum Number of */
   /*          Remote Device Entries, in which case the final parameter */
   /*          *MUST* be specified.                                     */
int BTPSAPI ANCM_Query_Connected_Devices(unsigned int MaximumRemoteDeviceListEntries, BD_ADDR_t *RemoteDeviceList, unsigned int *TotalNumberConnectedDevices)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANCS Manager has been initialized.  */
   if(Initialized)
   {
      /* Check to see if the input parameters appear to be semi-valid.  */
      if(((MaximumRemoteDeviceListEntries) && (RemoteDeviceList)) || (TotalNumberConnectedDevices))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(ANCM_AcquireLock())
         {
            /* Simply call the internal function to handle actually     */
            /* query the connected devices.                             */
            ret_val = ProcessQueryConnectedDevices(MaximumRemoteDeviceListEntries, RemoteDeviceList, TotalNumberConnectedDevices);

            /* Release the Lock because we are finished with it.        */
            ANCM_ReleaseLock();
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_ANCS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to query the current notifications available on the remote*/
   /* ANCS device.  This function accepts as input the Callback ID (the */
   /* return value from the ANCM_Register_Consumer_Event_Callback()     */
   /* function) and the Bluetooth address of the Notification           */
   /* Provider. This functions returns zero if successful and a negative*/
   /* return error code if there was an error.                          */
   /* * NOTE * The results of this function will be returned in an      */
   /*          etANCMNotificationReceived event for each current        */
   /*          notification.                                            */
int BTPSAPI ANCM_Query_Current_Notifications(unsigned int ConsumerCallbackID, BD_ADDR_t RemoteDeviceAddress)
{
   int                         ret_val;
   Connection_Entry_t         *ConnectionEntryPtr;
   ANCM_Event_Callback_Info_t *EventCallbackPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS  | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the module has been initialized and a   */
   /* callback has been registered.                                     */
   if((Initialized))
   {
      /* Check for a registered callback.  The list head should be NULL */
      /* if there are no callbacks registered.                          */
      if(EventCallbackInfoList)
      {
         /* Next, check to see if the input parameters appear to be     */
         /* semi-valid.                                                 */
         if((ConsumerCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
         {
            /* Wait for access to the lock that guards access to this   */
            /* module.                                                  */
            if(ANCM_AcquireLock())
            {
               /* Check to see if the device is powered on.             */
               if(CurrentPowerState)
               {
                  /* Go ahead and find the Callback Info Entry for this */
                  /* Consumer Callback ID.                              */
                  if((EventCallbackPtr = SearchEventCallbackInfoEntry(&EventCallbackInfoList, ConsumerCallbackID)) != NULL)
                  {
                     /* Go ahead and find the Connection Entry          */
                     /* associated with the Bluetooth Device Address.   */
                     if((ConnectionEntryPtr = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress)) != NULL)
                     {
                        /* Make sure that there isn't already a Get App */
                        /* Attributes or Get Notification Attributes    */
                        /* request outstanding.                         */
                        if(!(ConnectionEntryPtr->ConnectionFlags & ANCM_CONNECTION_ENTRY_FLAGS_GET_ATTRIBUTE_REQUEST_PENDING))
                        {
                           /* Make sure that the remote device supports */
                           /* ANCS and is configured.                   */
                           if(ConnectionEntryPtr->ConnectionFlags & ANCM_CONNECTION_ENTRY_FLAGS_ANCS_SERVICE_CONFIGURED)
                           {
                              /* Now call the function to process the   */
                              /* request.                               */
                              ret_val = ProcessRefreshNotificationsRequest(EventCallbackPtr, ConnectionEntryPtr);
                           }
                           else
                              ret_val = BTPM_ERROR_CODE_ANCS_SERVICE_NOT_PRESENT;
                        }
                        else
                           ret_val = BTPM_ERROR_CODE_ANCS_REQUEST_CURRENTLY_PENDING;
                     }
                     else
                        ret_val = BTPM_ERROR_CODE_DEVICE_IS_NOT_CURRENTLY_CONNECTED;
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
               }
               else
                  ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

               /* Release the Lock because we are finished with it.     */
               ANCM_ReleaseLock();
            }
            else
               ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
         }
         else
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
      else
         ret_val = BTPM_ERROR_CODE_ANCS_CALLBACK_NOT_REGISTERED;
   }
   else
      ret_val = BTPM_ERROR_CODE_ANCS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ANCS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}
