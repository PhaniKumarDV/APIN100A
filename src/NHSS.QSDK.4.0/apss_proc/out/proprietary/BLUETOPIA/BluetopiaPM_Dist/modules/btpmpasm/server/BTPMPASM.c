/*****< btpmpasm.c >***********************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMPASM - Phone Alert Status (PAS) Manager for Stonestreet One Bluetooth */
/*             Protocol Stack Platform Manager.                               */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/28/12  D. Lange       Initial creation.                               */
/*   12/04/12  T. Cook        Finished Implementation.                        */
/******************************************************************************/
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "SS1BTPAS.h"            /* PAS Prototypes/Constants.                 */

#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMPASM.h"            /* BTPM PAS Manager Prototypes/Constants.    */
#include "PASMAPI.h"             /* PAS Manager Prototypes/Constants.         */
#include "PASMMSG.h"             /* BTPM PAS Manager Message Formats.         */
#include "PASMGR.h"              /* PAS Manager Impl. Prototypes/Constants.   */

#include "SS1BTPM.h"             /* BTPM Main Prototypes and Constants.       */
#include "BTPMERR.h"             /* BTPM Error Prototypes/Constants.          */
#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

   /* The following defines the PASM LE Configuration File Section Name.*/
#define PASM_LE_CONFIGURATION_FILE_SECTION_NAME                   "PASM"

   /* The following define the Key Names that are used with the PASM    */
   /* Configuration File.                                               */
#define PASM_KEY_NAME_CCCD_PREFIX                                 "PASM_%02X%02X%02X%02X%02X%02X"
#define PASM_KEY_NAME_PERSISTENT_UID                              "PU"

   /* The following defines the size of a Persistent UID that is stored */
   /* in the configuration file.                                        */
#define PASM_PERSISTENT_UID_SIZE                                  (NON_ALIGNED_DWORD_SIZE + (NON_ALIGNED_WORD_SIZE*2))

   /* Structure which is used to track Callback Information related to  */
   /* this module.                                                      */
typedef struct _tagPASM_Event_Callback_Info_t
{
   unsigned int                           EventCallbackID;
   unsigned int                           ClientID;
   PASM_Event_Callback_t                  EventCallback;
   void                                  *CallbackParameter;
   struct _tagPASM_Event_Callback_Info_t *NextPASMEventCallbackInfoPtr;
} PASM_Event_Callback_Info_t;

   /* Container structure used when dispatching registered callbacks.   */
typedef struct _tagCallback_Info_t
{
   unsigned int           CallbackID;
   unsigned int           ClientID;
   PASM_Event_Callback_t  EventCallback;
   void                  *CallbackParameter;
} Callback_Info_t;

   /* Structure which is used to hold all of the binary information that*/
   /* is stored to file for a paired device.                            */
typedef struct _tagConnection_Binary_Entry_t
{
   Byte_t RingerSettingCCCD;
   Byte_t AlertStatusCCCD;
} Connection_Binary_Entry_t;

   /* Structure which is used to track information pertaining to        */
   /* incoming connection requests.                                     */
typedef struct _tagConnection_Entry_t
{
   BD_ADDR_t                      BD_ADDR;
   Boolean_t                      ClientConnectedDispatched;
   Boolean_t                      RingerSettingNotifications;
   Boolean_t                      AlertStatusNotifications;
   PASM_Connection_Type_t         ConnectionType;
   unsigned int                   ConnectionID;
   unsigned long                  ConnectionFlags;
   struct _tagConnection_Entry_t *NextConnectionEntryPtr;
} Connection_Entry_t;

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the compiler*/
   /* as part of standard C/C++).                                       */

   /* Variable which serves as a global flag about whether this module  */
   /* is initialized or not.                                            */
static Boolean_t Initialized;

   /* Variable which is used to hold the next (unique) Callback ID.     */
static unsigned int NextEventCallbackID;

   /* Variable which holds the current Power State of the Device.       */
static Boolean_t CurrentPowerState;

   /* Variable which holds a pointer to the first element in the Generic*/
   /* Attribute Profile Callback Info List (which holds all PASM Event  */
   /* Callbacks registered with this module).                           */
static PASM_Event_Callback_Info_t *ServerEventCallbackInfoList;

   /* Variable which holds a pointer to the first element of the        */
   /* Connection Information List (which holds all currently active     */
   /* connections).                                                     */
static Connection_Entry_t *ConnectionEntryList;

   /* Variable which holds the current Ringer Setting and Alert Status. */
static PASM_Alert_Status_t PASMAlertStatus;
static PASM_Ringer_Setting_t PASMRingerSetting;

   /* Internal Function Prototypes.                                     */
static unsigned int GetNextEventCallbackID(void);

static PASM_Event_Callback_Info_t *AddEventCallbackInfoEntry(PASM_Event_Callback_Info_t **ListHead, PASM_Event_Callback_Info_t *EntryToAdd);
static PASM_Event_Callback_Info_t *SearchEventCallbackInfoEntry(PASM_Event_Callback_Info_t **ListHead, unsigned int EventCallbackID);
static PASM_Event_Callback_Info_t *DeleteEventCallbackInfoEntry(PASM_Event_Callback_Info_t **ListHead, unsigned int EventCallbackID);
static void FreeEventCallbackInfoEntryMemory(PASM_Event_Callback_Info_t *EntryToFree);
static void FreeEventCallbackInfoList(PASM_Event_Callback_Info_t **ListHead);

static Connection_Entry_t *AddConnectionEntry(Connection_Entry_t **ListHead, Connection_Entry_t *EntryToAdd);
static Connection_Entry_t *SearchConnectionEntry(Connection_Entry_t **ListHead, unsigned int ConnectionID);
static Connection_Entry_t *DeleteConnectionEntryByBD_ADDR(Connection_Entry_t **ListHead, BD_ADDR_t BD_ADDR);
static void FreeConnectionEntryMemory(Connection_Entry_t *EntryToFree);
static void FreeConnectionEntryList(Connection_Entry_t **ListHead);

static Connection_Entry_t *SearchAddConnectionEntry(Connection_Entry_t **ListHead, PASM_Connection_Type_t ConnectionType, BD_ADDR_t BD_ADDR, unsigned int ConnectionID);

static void StoreConnectionConfiguration(Connection_Entry_t *ConnectionEntry, Boolean_t Store);
static void ReloadConnectionConfiguration(Connection_Entry_t *ConnectionEntry);

static void DispatchPASMEvent(PASM_Event_Callback_Info_t **EventCallbackListHead, PASM_Event_Data_t *PASMEventData, BTPM_Message_t *Message, unsigned int *CallbackHandlerID, unsigned int *MessageHandlerID);

static void DispatchPASMConnectionEvent(Connection_Entry_t *ConnectionEntry);
static void DispatchPASMDisconnectionEvent(Connection_Entry_t *ConnectionEntry);
static void DispatchPASMRingerControlCommandEvent(Connection_Entry_t *ConnectionEntry, PASM_Ringer_Control_Command_t RingerControlCommand);

static void StorePersistentUID(DWord_t PersistentUID, GATT_Attribute_Handle_Group_t *ServiceHandleRange, Boolean_t Store);
static Boolean_t ReloadPersistentUID(DWord_t *PersistentUID, GATT_Attribute_Handle_Group_t *ServiceHandleRange);

static Boolean_t CalculateServiceHandleRange(GATT_Attribute_Handle_Group_t *ServiceHandleRange);

static int ConvertRingerSettingToPASS(PASM_Ringer_Setting_t RingerSetting, PASS_Ringer_Setting_t *RingerSettingResult);
static int ConvertAlertStatusToPASS(PASM_Alert_Status_t *AlertStatus, PASS_Alert_Status_t *AlertStatusResult);

static int ProcessSetRingerSettingRequest(PASM_Ringer_Setting_t RingerSetting);
static int ProcessSetAlertStatusRequest(PASM_Alert_Status_t *AlertStatus);

static void ProcessRegisterServerEventsMessage(PASM_Register_Server_Events_Request_t *Message);
static void ProcessUnRegisterServerEventsMessage(PASM_Un_Register_Server_Events_Request_t *Message);
static void ProcessSetAlertStatusMessage(PASM_Set_Alert_Status_Request_t *Message);
static void ProcessQueryAlertStatusMessage(PASM_Query_Alert_Status_Request_t *Message);
static void ProcessSetRingerSettingMessage(PASM_Set_Ringer_Setting_Request_t *Message);
static void ProcessQueryRingerSettingMessage(PASM_Query_Ringer_Setting_Request_t *Message);

static void ProcessReceivedMessage(BTPM_Message_t *Message);
static void ProcessClientUnRegister(unsigned int ClientID);

static void ProcessReadClientConfigurationRequestEvent(PASS_Read_Client_Configuration_Data_t *ReadClientConfigurationData);
static void ProcessUpdateClientConfigurationEvent(PASS_Client_Configuration_Update_Data_t *ClientConfigurationUpdateData);
static void ProcessRingerControlCommandIndicationEvent(PASS_Ringer_Control_Command_Data_t *RingerControlCommandData);

static void ProcessPASSEvent(PASM_PASS_Event_Data_t *PASSEventData);

static void ProcessLowEnergyConnectionEvent(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties);
static void ProcessLowEnergyDisconnectionEvent(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties);
static void ProcessLowEnergyAddressChangeEvent(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties);
static void ProcessLowEnergyPairingChangeEvent(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties);

static void ProcessRemoteDevicePropertiesChangedEvent(unsigned long ChangedMemberMask, DEVM_Remote_Device_Properties_t *RemoteDeviceProperties);

static void BTPSAPI BTPMDispatchCallback_PASM(void *CallbackParameter);
static void BTPSAPI BTPMDispatchCallback_PASS(void *CallbackParameter);
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter);

static void BTPSAPI PASManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter);

   /* The following function is a utility function that exists to       */
   /* retrieve a unique Callback ID that can be used to add an entry    */
   /* into the PAS Event Callback List.                                 */
static unsigned int GetNextEventCallbackID(void)
{
   NextEventCallbackID++;

   if(NextEventCallbackID & 0x80000000)
      NextEventCallbackID = 1;

   return(NextEventCallbackID);
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
static PASM_Event_Callback_Info_t *AddEventCallbackInfoEntry(PASM_Event_Callback_Info_t **ListHead, PASM_Event_Callback_Info_t *EntryToAdd)
{
   PASM_Event_Callback_Info_t *AddedEntry = NULL;
   PASM_Event_Callback_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First let's verify that values passed in are semi-valid.          */
   if((ListHead) && (EntryToAdd))
   {
      /* Make sure that the element that we are adding seems semi-valid.*/
      if(EntryToAdd->EventCallbackID)
      {
         /* OK, data seems semi-valid, let's allocate a new data        */
         /* structure to add to the list.                               */
         AddedEntry = (PASM_Event_Callback_Info_t *)BTPS_AllocateMemory(sizeof(PASM_Event_Callback_Info_t));

         if(AddedEntry)
         {
            /* Copy All Data over.                                      */
            *AddedEntry                              = *EntryToAdd;

            /* Now Add it to the end of the list.                       */
            AddedEntry->NextPASMEventCallbackInfoPtr = NULL;

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
                     if(tmpEntry->NextPASMEventCallbackInfoPtr)
                        tmpEntry = tmpEntry->NextPASMEventCallbackInfoPtr;
                     else
                        break;
                  }
               }

               if(AddedEntry)
               {
                  /* Last element found, simply Add the entry.          */
                  tmpEntry->NextPASMEventCallbackInfoPtr = AddedEntry;
               }
            }
            else
               *ListHead = AddedEntry;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", AddedEntry));

   return(AddedEntry);
}

   /* The following function searches the specified List for the        */
   /* specified Event Callback ID.  This function returns NULL if either*/
   /* the List Head is invalid, the Event Callback ID is invalid, or the*/
   /* specified Event Callback ID was NOT found.                        */
static PASM_Event_Callback_Info_t *SearchEventCallbackInfoEntry(PASM_Event_Callback_Info_t **ListHead, unsigned int EventCallbackID)
{
   PASM_Event_Callback_Info_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", EventCallbackID));

   /* Let's make sure the list and Event Callback ID to search for      */
   /* appear to be valid.                                               */
   if((ListHead) && (EventCallbackID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->EventCallbackID != EventCallbackID))
         FoundEntry = FoundEntry->NextPASMEventCallbackInfoPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

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
static PASM_Event_Callback_Info_t *DeleteEventCallbackInfoEntry(PASM_Event_Callback_Info_t **ListHead, unsigned int EventCallbackID)
{
   PASM_Event_Callback_Info_t *FoundEntry = NULL;
   PASM_Event_Callback_Info_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", EventCallbackID));

   /* Let's make sure the List and Event Callback ID to search for      */
   /* appear to be semi-valid.                                          */
   if((ListHead) && (EventCallbackID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->EventCallbackID != EventCallbackID))
      {
         LastEntry  = FoundEntry;
         FoundEntry = FoundEntry->NextPASMEventCallbackInfoPtr;
      }

      /* Check to see if we found the specified entry.                  */
      if(FoundEntry)
      {
         /* OK, now let's remove the entry from the list.  We have to   */
         /* check to see if the entry was the first entry in the list.  */
         if(LastEntry)
         {
            /* Entry was NOT the first entry in the list.               */
            LastEntry->NextPASMEventCallbackInfoPtr = FoundEntry->NextPASMEventCallbackInfoPtr;
         }
         else
            *ListHead = FoundEntry->NextPASMEventCallbackInfoPtr;

         FoundEntry->NextPASMEventCallbackInfoPtr = NULL;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* This function frees the specified Event Callback member.  No check*/
   /* is done on this entry other than making sure it NOT NULL.         */
static void FreeEventCallbackInfoEntryMemory(PASM_Event_Callback_Info_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToFree)
      BTPS_FreeMemory(EntryToFree);

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified Event Callback List.  Upon return of this*/
   /* function, the Head Pointer is set to NULL.                        */
static void FreeEventCallbackInfoList(PASM_Event_Callback_Info_t **ListHead)
{
   PASM_Event_Callback_Info_t *EntryToFree;
   PASM_Event_Callback_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ListHead)
   {
      /* Simply traverse the list and free every element present.       */
      EntryToFree = *ListHead;

      while(EntryToFree)
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("Deleting Event Callback ID: 0x%08X\n", EntryToFree->EventCallbackID));

         tmpEntry    = EntryToFree;
         EntryToFree = EntryToFree->NextPASMEventCallbackInfoPtr;

         FreeEventCallbackInfoEntryMemory(tmpEntry);
      }

      /* Make sure the List appears to be empty.                        */
      *ListHead = NULL;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
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

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", AddedEntry));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's make sure the list and Bluetooth Device Address to search   */
   /* for appear to be valid.                                           */
   if((ListHead) && (ConnectionID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->ConnectionID != ConnectionID))
         FoundEntry = FoundEntry->NextConnectionEntryPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* This function frees the specified Connection Information member.  */
   /* No check is done on this entry other than making sure it NOT NULL.*/
static void FreeConnectionEntryMemory(Connection_Entry_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToFree)
      BTPS_FreeMemory(EntryToFree);

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified Connection Information List.  Upon return*/
   /* of this function, the Head Pointer is set to NULL.                */
static void FreeConnectionEntryList(Connection_Entry_t **ListHead)
{
   Connection_Entry_t *EntryToFree;
   Connection_Entry_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* search for an existing entry for the specified device OR to create*/
   /* a new entry and add it to the specified list.  This function      */
   /* returns the Entry on success or NULL on failure.                  */
static Connection_Entry_t *SearchAddConnectionEntry(Connection_Entry_t **ListHead, PASM_Connection_Type_t ConnectionType, BD_ADDR_t BD_ADDR, unsigned int ConnectionID)
{
   Connection_Entry_t               EntryToAdd;
   Connection_Entry_t              *ret_val = NULL;
   DEVM_Remote_Device_Properties_t  RemoteDeviceProperties;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters appear semi-valid.               */
   if((ListHead) && ((ConnectionType == pctPASServer) || (ConnectionType == pctPASClient)) && (!COMPARE_NULL_BD_ADDR(BD_ADDR)) && (ConnectionID))
   {
      /* First search the list for the specified Connection ID.         */
      if((ret_val = SearchConnectionEntry(ListHead, ConnectionID)) == NULL)
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

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function that is used to      */
   /* reload a Devices' stored configuration for the specified          */
   /* connection entry from the Low Energy Configuration File.          */
static void StoreConnectionConfiguration(Connection_Entry_t *ConnectionEntry, Boolean_t Store)
{
   char                            TempString[64];
   Connection_Binary_Entry_t       BinaryEntry;
   DEVM_Remote_Device_Properties_t RemoteDeviceProperties;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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
            sprintf(TempString, PASM_KEY_NAME_CCCD_PREFIX, ConnectionEntry->BD_ADDR.BD_ADDR5, ConnectionEntry->BD_ADDR.BD_ADDR4, ConnectionEntry->BD_ADDR.BD_ADDR3, ConnectionEntry->BD_ADDR.BD_ADDR2, ConnectionEntry->BD_ADDR.BD_ADDR1, ConnectionEntry->BD_ADDR.BD_ADDR0);

            /* Check to see if we are storing something to flash or     */
            /* deleting something.                                      */
            if(Store)
            {
               /* Device is paired so go ahead and store the            */
               /* configuration to flash (if the device has registered  */
               /* for PASP notifications).                              */
               if((ConnectionEntry->RingerSettingNotifications) || (ConnectionEntry->AlertStatusNotifications))
               {
                  /* Format the Binary Entry.                           */
                  ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&(BinaryEntry.RingerSettingCCCD), (Byte_t)ConnectionEntry->RingerSettingNotifications);
                  ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&(BinaryEntry.AlertStatusCCCD), (Byte_t)ConnectionEntry->AlertStatusNotifications);

                  /* Now write out the new Key-Value Pair.              */
                  SET_WriteBinaryData(PASM_LE_CONFIGURATION_FILE_SECTION_NAME, TempString, (unsigned char *)&BinaryEntry, sizeof(BinaryEntry), BTPM_CONFIGURATION_LOW_ENERGY_CONFIGURATION_FILE_NAME);
               }
               else
               {
                  DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("Deleting Configuration for Device due to no CCCD being configured, Key = %s\n", TempString));

                  /* Delete the configuration stored for this device.   */
                  SET_WriteBinaryData(PASM_LE_CONFIGURATION_FILE_SECTION_NAME, TempString, NULL, 0, BTPM_CONFIGURATION_LOW_ENERGY_CONFIGURATION_FILE_NAME);
               }
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("Deleting Configuration for Device, Key = %s\n", TempString));

               /* Delete the configuration stored for this device.      */
               SET_WriteBinaryData(PASM_LE_CONFIGURATION_FILE_SECTION_NAME, TempString, NULL, 0, BTPM_CONFIGURATION_LOW_ENERGY_CONFIGURATION_FILE_NAME);
            }
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* reload a Devices' stored configuration for the specified          */
   /* connection entry from the Low Energy Configuration File.          */
static void ReloadConnectionConfiguration(Connection_Entry_t *ConnectionEntry)
{
   char                      TempString[64];
   Connection_Binary_Entry_t BinaryEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if(ConnectionEntry)
   {
      /* Format the Key Name.                                           */
      sprintf(TempString, PASM_KEY_NAME_CCCD_PREFIX, ConnectionEntry->BD_ADDR.BD_ADDR5, ConnectionEntry->BD_ADDR.BD_ADDR4, ConnectionEntry->BD_ADDR.BD_ADDR3, ConnectionEntry->BD_ADDR.BD_ADDR2, ConnectionEntry->BD_ADDR.BD_ADDR1, ConnectionEntry->BD_ADDR.BD_ADDR0);

      /* Attempt to reload the configuration for this device.           */
      if(SET_ReadBinaryData(PASM_LE_CONFIGURATION_FILE_SECTION_NAME, TempString, (unsigned char *)&BinaryEntry, sizeof(BinaryEntry), BTPM_CONFIGURATION_LOW_ENERGY_CONFIGURATION_FILE_NAME) == sizeof(Connection_Binary_Entry_t))
      {
         /* Reload the configuration into the connection entry.         */
         ConnectionEntry->RingerSettingNotifications = (Boolean_t)READ_UNALIGNED_BYTE_LITTLE_ENDIAN(&(BinaryEntry.RingerSettingCCCD));
         ConnectionEntry->AlertStatusNotifications   = (Boolean_t)READ_UNALIGNED_BYTE_LITTLE_ENDIAN(&(BinaryEntry.AlertStatusCCCD));

         DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("Reloaded Ringer Setting CCCD: %u\n", (unsigned int)ConnectionEntry->RingerSettingNotifications));
         DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("Reloaded Alert Status CCCD:   %u\n", (unsigned int)ConnectionEntry->AlertStatusNotifications));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch the specified PASM event to every registered PASM Event  */
   /* Callback.                                                         */
   /* * NOTE * This function should be called with the PASM Manager Lock*/
   /*          held.  Upon exit from this function it will free the PASM*/
   /*          Manager Lock.                                            */
static void DispatchPASMEvent(PASM_Event_Callback_Info_t **EventCallbackListHead, PASM_Event_Data_t *PASMEventData, BTPM_Message_t *Message, unsigned int *CallbackHandlerID, unsigned int *MessageHandlerID)
{
   unsigned int                Index;
   unsigned int                ServerID;
   unsigned int                NumberCallbacks;
   Callback_Info_t             CallbackInfoArray[16];
   Callback_Info_t            *CallbackInfoArrayPtr;
   PASM_Event_Callback_Info_t *CallbackInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure that the input parameters appear to be semi-valid.      */
   if((EventCallbackListHead) && (PASMEventData) && (Message) && (CallbackHandlerID) && (MessageHandlerID))
   {
      /* Next, let's determine how many callbacks are registered.       */
      CallbackInfoPtr = *EventCallbackListHead;
      ServerID        = MSG_GetServerAddressID();
      NumberCallbacks = 0;

      /* First, add the default event handlers.                         */
      while(CallbackInfoPtr)
      {
         if((CallbackInfoPtr->EventCallback) || (CallbackInfoPtr->ClientID != ServerID))
            NumberCallbacks++;

         CallbackInfoPtr = CallbackInfoPtr->NextPASMEventCallbackInfoPtr;
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
            CallbackInfoPtr = *EventCallbackListHead;
            NumberCallbacks = 0;

            /* Next add the default event handlers.                     */
            while(CallbackInfoPtr)
            {
               if((CallbackInfoPtr->EventCallback) || (CallbackInfoPtr->ClientID != ServerID))
               {
                  CallbackInfoArrayPtr[NumberCallbacks].CallbackID        = CallbackInfoPtr->EventCallbackID;
                  CallbackInfoArrayPtr[NumberCallbacks].ClientID          = CallbackInfoPtr->ClientID;
                  CallbackInfoArrayPtr[NumberCallbacks].EventCallback     = CallbackInfoPtr->EventCallback;
                  CallbackInfoArrayPtr[NumberCallbacks].CallbackParameter = CallbackInfoPtr->CallbackParameter;

                  NumberCallbacks++;
               }

               CallbackInfoPtr = CallbackInfoPtr->NextPASMEventCallbackInfoPtr;
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
               /*          for PASM events and Data Events.  To avoid   */
               /*          this case we need to walk the list of        */
               /*          previously dispatched events to check to see */
               /*          if it has already been dispatched (we need to*/
               /*          do this with Client Address ID's for messages*/
               /*          - Event Callbacks are local and therefore    */
               /*          unique so we don't have to do this filtering.*/

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
                        *CallbackHandlerID = CallbackInfoArrayPtr[Index].CallbackID;

                        (*CallbackInfoArrayPtr[Index].EventCallback)(PASMEventData, CallbackInfoArrayPtr[Index].CallbackParameter);
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
                  *MessageHandlerID                = CallbackInfoArrayPtr[Index].CallbackID;

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* dispatch a Connection Event to all registered callbacks.          */
static void DispatchPASMConnectionEvent(Connection_Entry_t *ConnectionEntry)
{
   PASM_Event_Data_t        EventData;
   PASM_Connected_Message_t Message;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ConnectionEntry)
   {
      /* Flag that a Connection Event has been dispatched for this      */
      /* device.                                                        */
      ConnectionEntry->ClientConnectedDispatched = TRUE;

      /* Format the event that will be dispatched locally.              */
      EventData.EventType                                        = etPASConnected;
      EventData.EventLength                                      = PASM_CONNECTED_EVENT_DATA_SIZE;
      EventData.EventData.ConnectedEventData.ConnectionType      = ConnectionEntry->ConnectionType;
      EventData.EventData.ConnectedEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;

      /* Next, format up the Message to dispatch.                       */
      BTPS_MemInitialize(&Message, 0, sizeof(Message));

      Message.MessageHeader.AddressID       = 0;
      Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
      Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PHONE_ALERT_STATUS_MANAGER;
      Message.MessageHeader.MessageFunction = PASM_MESSAGE_FUNCTION_CONNECTED;
      Message.MessageHeader.MessageLength   = (PASM_CONNECTED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

      Message.ConnectionType                = ConnectionEntry->ConnectionType;
      Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;

      /* Dispatch the event to all registered callbacks.                */
      DispatchPASMEvent(&ServerEventCallbackInfoList, &EventData, (BTPM_Message_t *)&Message, &(EventData.EventData.ConnectedEventData.CallbackID), &(Message.EventHandlerID));
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* dispatch a Disconnection Event to all registered callbacks.       */
static void DispatchPASMDisconnectionEvent(Connection_Entry_t *ConnectionEntry)
{
   PASM_Event_Data_t           EventData;
   PASM_Disconnected_Message_t Message;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ConnectionEntry)
   {
      /* Format the event that will be dispatched locally.              */
      EventData.EventType                                           = etPASDisconnected;
      EventData.EventLength                                         = PASM_DISCONNECTED_EVENT_DATA_SIZE;
      EventData.EventData.DisconnectedEventData.ConnectionType      = ConnectionEntry->ConnectionType;
      EventData.EventData.DisconnectedEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;

      /* Next, format up the Message to dispatch.                       */
      BTPS_MemInitialize(&Message, 0, sizeof(Message));

      Message.MessageHeader.AddressID       = 0;
      Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
      Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PHONE_ALERT_STATUS_MANAGER;
      Message.MessageHeader.MessageFunction = PASM_MESSAGE_FUNCTION_DISCONNECTED;
      Message.MessageHeader.MessageLength   = (PASM_DISCONNECTED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

      Message.ConnectionType                = ConnectionEntry->ConnectionType;
      Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;

      /* Dispatch the event to all registered callbacks.                */
      DispatchPASMEvent(&ServerEventCallbackInfoList, &EventData, (BTPM_Message_t *)&Message, &(EventData.EventData.DisconnectedEventData.CallbackID), &(Message.EventHandlerID));
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* dispatch a Ringer Control Command event to all registered         */
   /* callbacks.                                                        */
static void DispatchPASMRingerControlCommandEvent(Connection_Entry_t *ConnectionEntry, PASM_Ringer_Control_Command_t RingerControlCommand)
{
   PASM_Event_Data_t                           EventData;
   PASM_Ringer_Control_Point_Command_Message_t Message;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ConnectionEntry)
   {
      /* Format the event that will be dispatched locally.              */
      EventData.EventType                                                         = etPASRingerControlPointCommand;
      EventData.EventLength                                                       = PASM_RINGER_CONTROL_POINT_COMMAND_EVENT_DATA_SIZE;
      EventData.EventData.RingerControlPointCommandEventData.RingerControlCommand = RingerControlCommand;
      EventData.EventData.RingerControlPointCommandEventData.RemoteDeviceAddress  = ConnectionEntry->BD_ADDR;

      /* Next, format up the Message to dispatch.                       */
      BTPS_MemInitialize(&Message, 0, sizeof(Message));

      Message.MessageHeader.AddressID       = 0;
      Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
      Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PHONE_ALERT_STATUS_MANAGER;
      Message.MessageHeader.MessageFunction = PASM_MESSAGE_FUNCTION_RINGER_CONTROL_POINT_COMMAND;
      Message.MessageHeader.MessageLength   = (PASM_RINGER_CONTROL_POINT_COMMAND_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

      Message.RingerControlCommand          = RingerControlCommand;
      Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;

      /* Dispatch the event to all registered callbacks.                */
      DispatchPASMEvent(&ServerEventCallbackInfoList, &EventData, (BTPM_Message_t *)&Message, &(EventData.EventData.RingerControlPointCommandEventData.ServerCallbackID), &(Message.ServerEventHandlerID));
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to store*/
   /* Persistent UID for the PASS service registered by this module from*/
   /* the Low Energy Configuration File.                                */
static void StorePersistentUID(DWord_t PersistentUID, GATT_Attribute_Handle_Group_t *ServiceHandleRange, Boolean_t Store)
{
   char          TempString[64];
   unsigned int  Index;
   unsigned char TempBuffer[PASM_PERSISTENT_UID_SIZE];

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if(((Store == FALSE) || ((PersistentUID) && (ServiceHandleRange))))
   {
      /* Format the Key Name.                                           */
      BTPS_StringCopy(TempString, PASM_KEY_NAME_PERSISTENT_UID);

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
         SET_WriteBinaryData(PASM_LE_CONFIGURATION_FILE_SECTION_NAME, TempString, (unsigned char *)TempBuffer, sizeof(TempBuffer), BTPM_CONFIGURATION_LOW_ENERGY_CONFIGURATION_FILE_NAME);
      }
      else
      {
         /* Delete the configuration stored.                            */
         SET_WriteBinaryData(PASM_LE_CONFIGURATION_FILE_SECTION_NAME, TempString, NULL, 0, BTPM_CONFIGURATION_LOW_ENERGY_CONFIGURATION_FILE_NAME);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit:\n"));
}

   /* The following function is a utility function that is used to      */
   /* reload the Persistent UID for the PASS service registered by this */
   /* module from the Low Energy Configuration File.  This function     */
   /* returns TRUE if the Persistent UID was reloaded or false          */
   /* otherwise.                                                        */
static Boolean_t ReloadPersistentUID(DWord_t *PersistentUID, GATT_Attribute_Handle_Group_t *ServiceHandleRange)
{
   char          TempString[64];
   Boolean_t     ret_val = FALSE;
   unsigned int  Index;
   unsigned char TempBuffer[PASM_PERSISTENT_UID_SIZE];

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if((PersistentUID) && (ServiceHandleRange))
   {
      /* Format the Key Name.                                           */
      BTPS_StringCopy(TempString, PASM_KEY_NAME_PERSISTENT_UID);

      /* Attempt to reload the configuration for this device.           */
      if(SET_ReadBinaryData(PASM_LE_CONFIGURATION_FILE_SECTION_NAME, TempString, (unsigned char *)TempBuffer, sizeof(TempBuffer), BTPM_CONFIGURATION_LOW_ENERGY_CONFIGURATION_FILE_NAME) == sizeof(TempBuffer))
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

         DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("Persistent UID: 0x%08X\n", (unsigned int)*PersistentUID));
         DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("Service Range:  0x%04X - 0x%04X\n", ServiceHandleRange->Starting_Handle, ServiceHandleRange->Ending_Handle));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", (int)ret_val));

   return(ret_val);
}

   /* The following function is a utility function which is used to     */
   /* either load from the configuration file or generate a new Service */
   /* Handle Range for the PASS Service that is registered by this      */
   /* module.  This function returns TRUE if a Handle Range was         */
   /* calculated or FALSE otherwise.                                    */
static Boolean_t CalculateServiceHandleRange(GATT_Attribute_Handle_Group_t *ServiceHandleRange)
{
   DWord_t      PersistentUID;
   Boolean_t    RegisterPersistent;
   Boolean_t    ret_val = FALSE;
   unsigned int NumberOfAttributes;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if(ServiceHandleRange)
   {
      /* Query the number of attributes needed by the service.          */
      if((NumberOfAttributes = _PASM_Query_Number_Attributes()) > 0)
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

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", (int)ret_val));

   return(ret_val);
}

   /* The following function is a utility function that is used to      */
   /* convert a PASM Ringer Setting to a PASS Ringer Setting.  This     */
   /* function returns ZERO on success or a negative error code.        */
static int ConvertRingerSettingToPASS(PASM_Ringer_Setting_t RingerSetting, PASS_Ringer_Setting_t *RingerSettingResult)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if(((RingerSetting == prsSilent) || (RingerSetting == prsNormal)) && (RingerSettingResult))
   {
      /* Do the necessary conversion.                                   */
      if(RingerSetting == prsSilent)
         *RingerSettingResult = rsSilent;
      else
         *RingerSettingResult = rsNormal;

      /* Return success to the caller.                                  */
      ret_val = 0;
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function that is used to      */
   /* convert a PASM Alert Status to a PASS Alert Status.  This function*/
   /* returns ZERO on success or a negative error code.                 */
static int ConvertAlertStatusToPASS(PASM_Alert_Status_t *AlertStatus, PASS_Alert_Status_t *AlertStatusResult)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if((AlertStatus) && (AlertStatusResult))
   {
      /* Do the necessary conversion.                                   */
      AlertStatusResult->DisplayStateActive = AlertStatus->DisplayStateActive;
      AlertStatusResult->RingerStateActive  = AlertStatus->RingerStateActive;
      AlertStatusResult->VibrateStateActive = AlertStatus->VibrateStateActive;

      /* Return success to the caller.                                  */
      ret_val                               = 0;
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function that is used to      */
   /* process a request to set the Ringer Setting.  This function       */
   /* returns ZERO if successfull or a negative error code.             */
static int ProcessSetRingerSettingRequest(PASM_Ringer_Setting_t RingerSetting)
{
   int                    ret_val;
   Connection_Entry_t    *ConnectionEntry;
   PASS_Ringer_Setting_t  PASSRingerSetting;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %u\n", (unsigned int)RingerSetting));

   /* First convert the PASM Ringer Setting to a PASS Ringer Setting.   */
   if(!(ret_val = ConvertRingerSettingToPASS(RingerSetting, &PASSRingerSetting)))
   {
      /* Now attempt to set the value.                                  */
      if(!(ret_val = _PASM_Set_Ringer_Setting(PASSRingerSetting)))
      {
         /* Store the PASM Ringer Setting.                              */
         PASMRingerSetting = RingerSetting;

         /* Now walk the Connection List and notify anybody who has     */
         /* registered for PASS Ringer Setting Notifications.           */
         ConnectionEntry = ConnectionEntryList;
         while((ConnectionEntry) && (!ret_val))
         {
            if((ConnectionEntry->ConnectionType == pctPASServer) && (ConnectionEntry->RingerSettingNotifications))
               ret_val = _PASM_Send_Notification(ConnectionEntry->ConnectionID, rrRingerSetting);

            ConnectionEntry = ConnectionEntry->NextConnectionEntryPtr;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function that is used to      */
   /* process a request to set the Alert Status.  This function returns */
   /* ZERO if successfull or a negative error code.                     */
static int ProcessSetAlertStatusRequest(PASM_Alert_Status_t *AlertStatus)
{
   int                  ret_val;
   Connection_Entry_t  *ConnectionEntry;
   PASS_Alert_Status_t  PASSAlertStatus;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if(AlertStatus)
   {
      /* First convert the PASM Alert Status to a PASS Alert Status.    */
      if(!(ret_val = ConvertAlertStatusToPASS(AlertStatus, &PASSAlertStatus)))
      {
         /* Now attempt to set the value.                               */
         if(!(ret_val = _PASM_Set_Alert_Status(&PASSAlertStatus)))
         {
            /* Store the PASM Alert Status.                             */
            PASMAlertStatus = *AlertStatus;

            /* Now walk the Connection List and notify anybody who has  */
            /* registered for PASS Alert Status Notifications.          */
            ConnectionEntry = ConnectionEntryList;
            while((ConnectionEntry) && (!ret_val))
            {
               if((ConnectionEntry->ConnectionType == pctPASServer) && (ConnectionEntry->AlertStatusNotifications))
                  ret_val = _PASM_Send_Notification(ConnectionEntry->ConnectionID, rrAlertStatus);

               ConnectionEntry = ConnectionEntry->NextConnectionEntryPtr;
            }
         }
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function processes the specified Register PAS Server*/
   /* Events Request Message and responds to the message accordingly.   */
   /* This function does not verify the integrity of the Message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the Message before calling this function.                         */
   /* * NOTE * This function *MUST* be called with the PAS Manager Lock */
   /*          held.                                                    */
static void ProcessRegisterServerEventsMessage(PASM_Register_Server_Events_Request_t *Message)
{
   int                                     Result;
   PASM_Event_Callback_Info_t              EventCallbackEntry;
   PASM_Register_Server_Events_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Verify that the client does not have a callback already        */
      /* registered.                                                    */
      if(ServerEventCallbackInfoList == NULL)
      {
         /* Attempt to add an entry into the Event Callback Entry list. */
         BTPS_MemInitialize(&EventCallbackEntry, 0, sizeof(PASM_Event_Callback_Info_t));

         EventCallbackEntry.EventCallbackID   = GetNextEventCallbackID();
         EventCallbackEntry.ClientID          = Message->MessageHeader.AddressID;
         EventCallbackEntry.EventCallback     = NULL;
         EventCallbackEntry.CallbackParameter = 0;

         if(AddEventCallbackInfoEntry(&ServerEventCallbackInfoList, &EventCallbackEntry))
            Result = EventCallbackEntry.EventCallbackID;
         else
            Result = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
      }
      else
         Result = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;

      /* Format and send the response message.                          */
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = PASM_REGISTER_SERVER_EVENTS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Un-Register PAS    */
   /* Server Events Request Message and responds to the message         */
   /* accordingly.  This function does not verify the integrity of the  */
   /* Message (i.e. the length) because it is the caller's              */
   /* responsibility to verify the Message before calling this function.*/
   /* * NOTE * This function *MUST* be called with the PAS Manager Lock */
   /*          held.                                                    */
static void ProcessUnRegisterServerEventsMessage(PASM_Un_Register_Server_Events_Request_t *Message)
{
   int                                        Result;
   PASM_Event_Callback_Info_t                *EventCallbackEntry;
   PASM_Un_Register_Server_Events_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Attempt to delete the callback specified for this client.      */
      if((EventCallbackEntry = SearchEventCallbackInfoEntry(&ServerEventCallbackInfoList, Message->ServerEventHandlerID)) != NULL)
      {
         /* Verify that the same client is doing the un-registering.    */
         if(EventCallbackEntry->ClientID == Message->MessageHeader.AddressID)
         {
            /* Attempt to delete the callback specified for this device.*/
            if((EventCallbackEntry = DeleteEventCallbackInfoEntry(&ServerEventCallbackInfoList, Message->ServerEventHandlerID)) != NULL)
            {
               /* Free the memory allocated for this event callback.    */
               FreeEventCallbackInfoEntryMemory(EventCallbackEntry);

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

      ResponseMessage.MessageHeader.MessageLength  = PASM_UN_REGISTER_SERVER_EVENTS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified PAS Set Alert      */
   /* Status Request Message and responds to the message accordingly.   */
   /* This function does not verify the integrity of the Message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the Message before calling this function.                         */
   /* * NOTE * This function *MUST* be called with the PAS Manager Lock */
   /*          held.                                                    */
static void ProcessSetAlertStatusMessage(PASM_Set_Alert_Status_Request_t *Message)
{
   int                               Result;
   PASM_Event_Callback_Info_t       *EventCallbackEntry;
   PASM_Set_Alert_Status_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Attempt to determine if the server event callback that is   */
         /* specified in the message is registered with this module.    */
         if((EventCallbackEntry = SearchEventCallbackInfoEntry(&ServerEventCallbackInfoList, Message->ServerEventHandlerID)) != NULL)
         {
            /* Verify that this is coming from the process that         */
            /* registered the callback.                                 */
            if(EventCallbackEntry->ClientID == Message->MessageHeader.AddressID)
            {
               /* Process the request to set the alert status.          */
               Result = ProcessSetAlertStatusRequest(&(Message->AlertStatus));
            }
            else
               Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      /* Format and send the response message.                          */
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = PASM_SET_ALERT_STATUS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified PAS Query Alert    */
   /* Status Request Message and responds to the message accordingly.   */
   /* This function does not verify the integrity of the Message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the Message before calling this function.                         */
   /* * NOTE * This function *MUST* be called with the PAS Manager Lock */
   /*          held.                                                    */
static void ProcessQueryAlertStatusMessage(PASM_Query_Alert_Status_Request_t *Message)
{
   int                                 Result;
   PASM_Alert_Status_t                 AlertStatus;
   PASM_Event_Callback_Info_t         *EventCallbackEntry;
   PASM_Query_Alert_Status_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Attempt to determine if the server event callback that is   */
         /* specified in the message is registered with this module.    */
         if((EventCallbackEntry = SearchEventCallbackInfoEntry(&ServerEventCallbackInfoList, Message->ServerEventHandlerID)) != NULL)
         {
            /* Verify that this is coming from the process that         */
            /* registered the callback.                                 */
            if(EventCallbackEntry->ClientID == Message->MessageHeader.AddressID)
            {
               /* Return the current Alert Status.                      */
               AlertStatus = PASMAlertStatus;

               /* Return success.                                       */
               Result      = 0;
            }
            else
               Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      /* Format and send the response message.                          */
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = PASM_QUERY_ALERT_STATUS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      if(!Result)
         ResponseMessage.AlertStatus = AlertStatus;
      else
         BTPS_MemInitialize(&(ResponseMessage.AlertStatus), 0, sizeof(PASM_Alert_Status_t));

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified PAS Set Ringer     */
   /* Setting Request Message and responds to the message accordingly.  */
   /* This function does not verify the integrity of the Message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the Message before calling this function.                         */
   /* * NOTE * This function *MUST* be called with the PAS Manager Lock */
   /*          held.                                                    */
static void ProcessSetRingerSettingMessage(PASM_Set_Ringer_Setting_Request_t *Message)
{
   int                                 Result;
   PASM_Event_Callback_Info_t         *EventCallbackEntry;
   PASM_Set_Ringer_Setting_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Attempt to determine if the server event callback that is   */
         /* specified in the message is registered with this module.    */
         if((EventCallbackEntry = SearchEventCallbackInfoEntry(&ServerEventCallbackInfoList, Message->ServerEventHandlerID)) != NULL)
         {
            /* Verify that this is coming from the process that         */
            /* registered the callback.                                 */
            if(EventCallbackEntry->ClientID == Message->MessageHeader.AddressID)
            {
               /* Process the request to set the Ringer Setting.        */
               Result = ProcessSetRingerSettingRequest(Message->RingerSetting);
            }
            else
               Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      /* Format and send the response message.                          */
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = PASM_SET_RINGER_SETTING_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified PAS Query Ringer   */
   /* Setting Request Message and responds to the message accordingly.  */
   /* This function does not verify the integrity of the Message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the Message before calling this function.                         */
   /* * NOTE * This function *MUST* be called with the PAS Manager Lock */
   /*          held.                                                    */
static void ProcessQueryRingerSettingMessage(PASM_Query_Ringer_Setting_Request_t *Message)
{
   int                                   Result;
   PASM_Ringer_Setting_t                 RingerSetting;
   PASM_Event_Callback_Info_t           *EventCallbackEntry;
   PASM_Query_Ringer_Setting_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Attempt to determine if the server event callback that is   */
         /* specified in the message is registered with this module.    */
         if((EventCallbackEntry = SearchEventCallbackInfoEntry(&ServerEventCallbackInfoList, Message->ServerEventHandlerID)) != NULL)
         {
            /* Verify that this is coming from the process that         */
            /* registered the callback.                                 */
            if(EventCallbackEntry->ClientID == Message->MessageHeader.AddressID)
            {
               /* Return the current Ringer Setting.                    */
               RingerSetting = PASMRingerSetting;

               /* Return success.                                       */
               Result        = 0;
            }
            else
               Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      /* Format and send the response message.                          */
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = PASM_QUERY_RINGER_SETTING_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      if(!Result)
         ResponseMessage.RingerSetting = RingerSetting;
      else
         ResponseMessage.RingerSetting = prsNormal;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is called from the Main Bluetopia PM       */
   /* Dispatch thread in response to a Process Received Message.  This  */
   /* function SHOULD ONLY be called in this capacity and NOT under any */
   /* other circumstances.                                              */
   /* * NOTE * This function *MUST* be called with the PAS Manager Lock */
   /*          held.  This function will release the Lock before it     */
   /*          exits (i.e.  the caller SHOULD NOT RELEASE THE LOCK).    */
static void ProcessReceivedMessage(BTPM_Message_t *Message)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", Message->MessageHeader.MessageFunction));

   /* Check to make sure this isn't some kind of spoofed response (we do*/
   /* not process responses).                                           */
   if((Message) && (!(Message->MessageHeader.MessageID & BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK)))
   {
      /* Request received, go ahead and process it.                     */
      switch(Message->MessageHeader.MessageFunction)
      {
         case PASM_MESSAGE_FUNCTION_REGISTER_SERVER_EVENTS:
            DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("Register Server Events Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PASM_REGISTER_SERVER_EVENTS_REQUEST_SIZE)
               ProcessRegisterServerEventsMessage((PASM_Register_Server_Events_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case PASM_MESSAGE_FUNCTION_UN_REGISTER_SERVER_EVENTS:
            DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("Un-Register Server Events Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PASM_UN_REGISTER_SERVER_EVENTS_REQUEST_SIZE)
               ProcessUnRegisterServerEventsMessage((PASM_Un_Register_Server_Events_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case PASM_MESSAGE_FUNCTION_SET_ALERT_STATUS:
            DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("Set Alert Status Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PASM_SET_ALERT_STATUS_REQUEST_SIZE)
               ProcessSetAlertStatusMessage((PASM_Set_Alert_Status_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case PASM_MESSAGE_FUNCTION_QUERY_ALERT_STATUS:
            DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("Query Alert Status Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PASM_QUERY_ALERT_STATUS_REQUEST_SIZE)
               ProcessQueryAlertStatusMessage((PASM_Query_Alert_Status_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case PASM_MESSAGE_FUNCTION_SET_RINGER_SETTING:
            DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("Set Ringer Setting Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PASM_SET_RINGER_SETTING_REQUEST_SIZE)
               ProcessSetRingerSettingMessage((PASM_Set_Ringer_Setting_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case PASM_MESSAGE_FUNCTION_QUERY_RINGER_SETTING:
            DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("Query Ringer Setting Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PASM_QUERY_RINGER_SETTING_REQUEST_SIZE)
               ProcessQueryRingerSettingMessage((PASM_Query_Ringer_Setting_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         default:
            DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled Message\n"));

            break;
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid or Response Message Received\n"));
   }

   /* Release the Lock because we are finished with it.                 */
   DEVM_ReleaseLock();

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the function that is called to process a*/
   /* client Un-Registration event.  This function will perform any     */
   /* cleanup required for the specified Client.                        */
static void ProcessClientUnRegister(unsigned int ClientID)
{
   PASM_Event_Callback_Info_t *EventCallback;
   PASM_Event_Callback_Info_t *tmpEventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", ClientID));

   if((ClientID) && (ClientID != MSG_GetServerAddressID()))
   {
      /* Loop through the event callback list and delete all callbacks  */
      /* registered for this callback.                                  */
      EventCallback = ServerEventCallbackInfoList;
      while(EventCallback)
      {
         /* Check to see if the current Client Information is the one   */
         /* that is being un-registered.                                */
         if(EventCallback->ClientID == ClientID)
         {
            /* Note the next Event Callback Entry in the list (we are   */
            /* about to delete the current entry).                      */
            tmpEventCallback = EventCallback->NextPASMEventCallbackInfoPtr;

            /* Go ahead and delete the Event Callback Entry and clean up*/
            /* the resources.                                           */
            if((EventCallback = DeleteEventCallbackInfoEntry(&ServerEventCallbackInfoList, EventCallback->EventCallbackID)) != NULL)
            {
               /* All finished with the memory so free the entry.       */
               FreeEventCallbackInfoEntryMemory(EventCallback);
            }

            /* Go ahead and set the next Event Callback Entry (past the */
            /* one we just deleted).                                    */
            EventCallback = tmpEventCallback;
         }
         else
            EventCallback = EventCallback->NextPASMEventCallbackInfoPtr;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Phone    */
   /* Alert Status Read Client Configuration Data asynchronous event.   */
static void ProcessReadClientConfigurationRequestEvent(PASS_Read_Client_Configuration_Data_t *ReadClientConfigurationData)
{
   Connection_Entry_t *ConnectionEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* Verify that the input parameters seem semi-valid.                 */
   if(ReadClientConfigurationData)
   {
      /* Search for an existing connection entry for this device or add */
      /* one to the list.                                               */
      if((ConnectionEntryPtr = SearchAddConnectionEntry(&ConnectionEntryList, pctPASServer, ReadClientConfigurationData->RemoteDevice, ReadClientConfigurationData->ConnectionID)) != NULL)
      {
         /* Verify that we know what this descriptor is.                */
         if((ReadClientConfigurationData->ClientConfigurationType == rrAlertStatus) || (ReadClientConfigurationData->ClientConfigurationType == rrRingerSetting))
         {
            /* Check to see if we have dispatched a connection for this */
            /* device.                                                  */
            if(ConnectionEntryPtr->ClientConnectedDispatched == FALSE)
            {
               /* We have not dispatched a Connection Event so do so    */
               /* now.                                                  */
               DispatchPASMConnectionEvent(ConnectionEntryPtr);

               /* Update Client Configuration from File.                */
               ReloadConnectionConfiguration(ConnectionEntryPtr);
            }

            /* Respond to the Read Request.                             */
            if(ReadClientConfigurationData->ClientConfigurationType == rrAlertStatus)
               _PASM_Read_Client_Configuration_Response(ReadClientConfigurationData->TransactionID, ConnectionEntryPtr->AlertStatusNotifications);
            else
               _PASM_Read_Client_Configuration_Response(ReadClientConfigurationData->TransactionID, ConnectionEntryPtr->RingerSettingNotifications);
         }
         else
         {
            DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Unknown PASS Descriptor: %u\n", (unsigned int)ReadClientConfigurationData->ClientConfigurationType));

            /* Respond to the Read Request.                             */
            _PASM_Read_Client_Configuration_Response(ReadClientConfigurationData->TransactionID, FALSE);
         }
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Failed to add to list\n"));

         /* Respond to the Read Request.                                */
         _PASM_Read_Client_Configuration_Response(ReadClientConfigurationData->TransactionID, FALSE);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Phone    */
   /* Alert Status Client Configuration Update Data asynchronous event. */
static void ProcessUpdateClientConfigurationEvent(PASS_Client_Configuration_Update_Data_t *ClientConfigurationUpdateData)
{
   Connection_Entry_t *ConnectionEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* Verify that the input parameters seem semi-valid.                 */
   if(ClientConfigurationUpdateData)
   {
      /* Search for an existing connection entry for this device or add */
      /* one to the list.                                               */
      if((ConnectionEntryPtr = SearchAddConnectionEntry(&ConnectionEntryList, pctPASServer, ClientConfigurationUpdateData->RemoteDevice, ClientConfigurationUpdateData->ConnectionID)) != NULL)
      {
         /* Verify that we know what this descriptor is.                */
         if((ClientConfigurationUpdateData->ClientConfigurationType == rrAlertStatus) || (ClientConfigurationUpdateData->ClientConfigurationType == rrRingerSetting))
         {
            /* Check to see if we have dispatched a connection for this */
            /* device.                                                  */
            if(ConnectionEntryPtr->ClientConnectedDispatched == FALSE)
            {
               /* We have not dispatched a Connection Event so do so    */
               /* now.                                                  */
               DispatchPASMConnectionEvent(ConnectionEntryPtr);

               /* Update Client Configuration from File.                */
               ReloadConnectionConfiguration(ConnectionEntryPtr);
            }

            DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("%s notifications %s.\n", ((ClientConfigurationUpdateData->ClientConfigurationType == rrAlertStatus)?"Alert Status":"Ringer Setting"), (ClientConfigurationUpdateData->NotificationsEnabled?"enabled":"disabled")));

            /* Save the new configuration for this device.              */
            if(ClientConfigurationUpdateData->ClientConfigurationType == rrAlertStatus)
               ConnectionEntryPtr->AlertStatusNotifications   = ClientConfigurationUpdateData->NotificationsEnabled;
            else
               ConnectionEntryPtr->RingerSettingNotifications = ClientConfigurationUpdateData->NotificationsEnabled;

            /* Store Updated Client Configuration to File.              */
            StoreConnectionConfiguration(ConnectionEntryPtr, TRUE);
         }
         else
         {
            DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Unknown PASS Descriptor: %u\n", (unsigned int)ClientConfigurationUpdateData->ClientConfigurationType));
         }
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Failed to add to list\n"));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Phone    */
   /* Alert Status Ringer Control Command Indication asynchronous event.*/
static void ProcessRingerControlCommandIndicationEvent(PASS_Ringer_Control_Command_Data_t *RingerControlCommandData)
{
   Connection_Entry_t            *ConnectionEntryPtr;
   PASM_Ringer_Control_Command_t  RingerControlCommand;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* Verify that the input parameters seem semi-valid.                 */
   if(RingerControlCommandData)
   {
      /* Search for an existing connection entry for this device or add */
      /* one to the list.                                               */
      if((ConnectionEntryPtr = SearchAddConnectionEntry(&ConnectionEntryList, pctPASServer, RingerControlCommandData->RemoteDevice, RingerControlCommandData->ConnectionID)) != NULL)
      {
         /* Verify that we know what Ringer Control Command this is.    */
         if((RingerControlCommandData->Command == rcSilent) || (RingerControlCommandData->Command == rcMuteOnce) || (RingerControlCommandData->Command == rcCancelSilent))
         {
            /* Check to see if we have dispatched a connection for this */
            /* device.                                                  */
            if(ConnectionEntryPtr->ClientConnectedDispatched == FALSE)
            {
               /* We have not dispatched a Connection Event so do so    */
               /* now.                                                  */
               DispatchPASMConnectionEvent(ConnectionEntryPtr);

               /* Update Client Configuration from File.                */
               ReloadConnectionConfiguration(ConnectionEntryPtr);
            }

            /* Convert the PASS Control Command to a PASM Ringer Control*/
            /* Command.                                                 */
            if(RingerControlCommandData->Command == rcSilent)
               RingerControlCommand = prcSilent;
            else
            {
               if(RingerControlCommandData->Command == rcMuteOnce)
                  RingerControlCommand = prcMuteOnce;
               else
                  RingerControlCommand = prcCancelSilent;
            }

            /* Dispatch the Ringer Control Command Event.               */
            DispatchPASMRingerControlCommandEvent(ConnectionEntryPtr, RingerControlCommand);
         }
         else
         {
            DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Unknown PASS Ringer Control Command: %u\n", (unsigned int)RingerControlCommandData->Command));
         }
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Failed to add to list\n"));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the function that is responsible for    */
   /* processing PAS Events that have been received.  This function     */
   /* should ONLY be called with the Context locked AND ONLY in the     */
   /* context of an arbitrary processing thread.                        */
static void ProcessPASSEvent(PASM_PASS_Event_Data_t *PASSEventData)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First make sure that the input parameter appears to be semi-valid.*/
   if(PASSEventData)
   {
      /* Process the event based on the event type.                     */
      switch(PASSEventData->Event_Type)
      {
         case etPASS_Server_Read_Client_Configuration_Request:
            /* Process the Read Client Configuration Request event      */
            /* request.                                                 */
            ProcessReadClientConfigurationRequestEvent(&(PASSEventData->Event_Data.PASS_Read_Client_Configuration_Data));
            break;
         case etPASS_Server_Client_Configuration_Update:
            /* Process the Client Configuration Update event request.   */
            ProcessUpdateClientConfigurationEvent(&(PASSEventData->Event_Data.PASS_Client_Configuration_Update_Data));
            break;
         case etPASS_Server_Ringer_Control_Command_Indication:
            /* Process the Server Ringer Control Command Indication     */
            /* event.                                                   */
            ProcessRingerControlCommandIndicationEvent(&(PASSEventData->Event_Data.PASS_Ringer_Control_Command_Data));
            break;
         default:
            /* Invalid/Unknown Event.                                   */
            DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid/Unknown PAS Event Type: %d\n", PASSEventData->Event_Type));
            break;
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid PAS Event Data\n"));
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}


   /* The following function is the function that is called to process a*/
   /* LE Connect Event.                                                 */
   /* * NOTE * This function *MUST* be called with the PASM Manager Lock*/
   /*          held.                                                    */
static void ProcessLowEnergyConnectionEvent(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties)
{
   int                 Result;
   unsigned int        ConnectionID;
   Connection_Entry_t  ConnectionEntry;
   Connection_Entry_t *ConnectionEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if(RemoteDeviceProperties)
   {
      /* First attempt to get the Connection ID of the Connection.      */
      if(_PASM_Query_Connection_ID(RemoteDeviceProperties->BD_ADDR, &ConnectionID))
      {
         /* Check to see if this device has another address that is     */
         /* actually the connection address.                            */
         if(!_PASM_Query_Connection_ID(RemoteDeviceProperties->PriorResolvableBD_ADDR, &ConnectionID))
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
         if((ConnectionEntry.AlertStatusNotifications) || (ConnectionEntry.RingerSettingNotifications))
         {
            /* Search for an existing connection entry for this device  */
            /* or add one to the list.                                  */
            if((ConnectionEntryPtr = SearchAddConnectionEntry(&ConnectionEntryList, pctPASServer, ConnectionEntry.BD_ADDR, ConnectionEntry.ConnectionID)) != NULL)
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("PASM Connection established due to stored configuration\n"));

               /* Check to see if we have dispatched a connection for   */
               /* this device.                                          */
               if(ConnectionEntryPtr->ClientConnectedDispatched == FALSE)
               {
                  /* We have not dispatched a Connection Event so do so */
                  /* now.                                               */
                  DispatchPASMConnectionEvent(ConnectionEntryPtr);
               }

               /* Save the reloaded configuration for this device.      */
               ConnectionEntryPtr->AlertStatusNotifications   = ConnectionEntry.AlertStatusNotifications;
               ConnectionEntryPtr->RingerSettingNotifications = ConnectionEntry.RingerSettingNotifications;
            }
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the function that is called to process a*/
   /* LE Disconnect Event.                                              */
   /* * NOTE * This function *MUST* be called with the PASM Manager Lock*/
   /*          held.                                                    */
static void ProcessLowEnergyDisconnectionEvent(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties)
{
   Connection_Entry_t *ConnectionEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if(RemoteDeviceProperties)
   {
      /* Delete the specified Connection Entry from the list (if any    */
      /* exists).                                                       */
      if((ConnectionEntryPtr = DeleteConnectionEntryByBD_ADDR(&ConnectionEntryList, RemoteDeviceProperties->BD_ADDR)) != NULL)
      {
         /* Dispatch a PASM Disconnection Event.                        */
         DispatchPASMDisconnectionEvent(ConnectionEntryPtr);

         /* Free the memory that was allocated for this entry.          */
         FreeConnectionEntryMemory(ConnectionEntryPtr);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the function that is called to process a*/
   /* LE Address Change Event.                                          */
   /* * NOTE * This function *MUST* be called with the PASM Manager Lock*/
   /*          held.                                                    */
static void ProcessLowEnergyAddressChangeEvent(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties)
{
   Connection_Entry_t *ConnectionEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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
            DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("PASM Address Updated\n"));

            /* Save the new Base Address.                               */
            ConnectionEntryPtr->BD_ADDR = RemoteDeviceProperties->BD_ADDR;
         }

         ConnectionEntryPtr = ConnectionEntryPtr->NextConnectionEntryPtr;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the function that is called to process a*/
   /* LE Pairing Change Event.                                          */
   /* * NOTE * This function *MUST* be called with the PASM Manager Lock*/
   /*          held.                                                    */
static void ProcessLowEnergyPairingChangeEvent(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties)
{
   Connection_Entry_t  ConnectionEntry;
   Connection_Entry_t *ConnectionEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a Remote Device Properties Changed Event.                 */
   /* * NOTE * This function *MUST* be called with the PASM Manager Lock*/
   /*          held.                                                    */
static void ProcessRemoteDevicePropertiesChangedEvent(unsigned long ChangedMemberMask, DEVM_Remote_Device_Properties_t *RemoteDeviceProperties)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08lX\n", ChangedMemberMask));

   /* First make sure that the input parameter appears to be semi-valid.*/
   if((ChangedMemberMask) && (RemoteDeviceProperties) && (RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_SUPPORTS_LOW_ENERGY))
   {
      /* Check to see what changed.  We are only interested if the LE   */
      /* Connection State or the LE Service state or the Address is     */
      /* updated.                                                       */
      if(ChangedMemberMask & (DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_CONNECTION_STATE | DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_PRIOR_RESOLVABLE_ADDRESS | DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_PAIRING_STATE))
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Flags: 0x%08lX\n", RemoteDeviceProperties->RemoteDeviceFlags));

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
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process PAS Manager Asynchronous Events.            */
static void BTPSAPI BTPMDispatchCallback_PASM(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process PAS Events.                                 */
static void BTPSAPI BTPMDispatchCallback_PASS(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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
            /* Double check that this is an PAS Event Update.           */
            if(((PASM_Update_Data_t *)CallbackParameter)->UpdateType == utPASSServerEvent)
               ProcessPASSEvent(&(((PASM_Update_Data_t *)CallbackParameter)->UpdateData.PASSEventData));

            /* Release the lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
      }

      /* Go ahead and free the memory because we are finished with it.  */
      BTPS_FreeMemory(CallbackParameter);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process Client Un-Register Asynchronous Events.     */
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function represents the Message Group Handler       */
   /* function that is installed to process all PAS Manager Messages.   */
static void BTPSAPI PASManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the Message specified appears to be semi-valid.   */
   if(Message)
   {
      /* Next, check to see if this message is specified for our        */
      /* specific Message Group.                                        */
      if(Message->MessageHeader.MessageGroup == BTPM_MESSAGE_GROUP_PHONE_ALERT_STATUS_MANAGER)
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("PAS Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

         /* Message is destined for us, make sure we are still          */
         /* processing messages.                                        */
         if(Initialized)
         {
            /* Check to see if this message is a PAS Manager defined    */
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
               /* PAS Manager Thread.                                   */
               if(!BTPM_QueueMailboxCallback(BTPMDispatchCallback_PASM, (void *)Message))
               {
                  DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue PAS Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

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
                        DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue PAS Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));
                     }
                  }
               }
            }
         }
         else
         {
            /* Check to see if this message is an PAS Manager defined   */
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
         DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("Non PAS Manager Message: 0x%08X, 0x%08X\n", Message->MessageHeader.MessageGroup, Message->MessageHeader.MessageFunction));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for initializing/cleaning up*/
   /* the Bluetopia Platform Manager Phone Alert Status (PAS) Manager   */
   /* Module.  This function should be registered with the Bluetopia    */
   /* Platform Manager Module Handler and will be called when the       */
   /* Platform Manager is initialized (or shut down).                   */
void BTPSAPI PASM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData)
{
   int                   Result;
   PASS_Alert_Status_t   AlertStatus;
   PASS_Ringer_Setting_t RingerSetting;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %d\n", Initialize));

   /* First, determine if the request is to initialize or shut down.    */
   if(Initialize)
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("Initialization Request\n"));

      if(!Initialized)
      {
         /* Note the new initialization state.                          */
         Initialized = TRUE;

         DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing PAS Manager\n"));

         /* Next, let's attempt to register our Message Group Handler to*/
         /* process PAS Manager messages.                               */
         if(!MSG_RegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_PHONE_ALERT_STATUS_MANAGER, PASManagerGroupHandler, NULL))
         {
            /* Configure the initialization data.                       */
            if(InitializationData)
            {
               PASMAlertStatus   = ((PASM_Initialization_Info_t *)InitializationData)->DefaultAlertStatus;
               PASMRingerSetting = ((PASM_Initialization_Info_t *)InitializationData)->DefaultRingerSetting;
            }
            else
            {
               PASMAlertStatus.RingerStateActive  = PASM_CONFIGURATION_DEFAULT_RINGER_STATE;
               PASMAlertStatus.VibrateStateActive = PASM_CONFIGURATION_DEFAULT_VIBRATE_STATE;
               PASMAlertStatus.DisplayStateActive = PASM_CONFIGURATION_DEFAULT_DISPLAY_STATE;

               PASMRingerSetting                  = PASM_CONFIGURATION_DEFAULT_RINGER_SETTING;
            }

            /* Conver the PASM Values to PASS Values.                   */
            if((!ConvertRingerSettingToPASS(PASMRingerSetting, &RingerSetting)) && (!ConvertAlertStatusToPASS(&PASMAlertStatus, &AlertStatus)))
            {
               /* Initialize the actual PAS Manager Implementation      */
               /* Module (this is the module that is actually           */
               /* responsible for actually implementing the PAS Manager */
               /* functionality - this module is just the framework     */
               /* shell).                                               */
               if(!(Result = _PASM_Initialize(RingerSetting, &AlertStatus)))
               {
                  /* Determine the current Device Power State.          */
                  CurrentPowerState   = (Boolean_t)((DEVM_QueryDevicePowerState() > 0)?TRUE:FALSE);

                  /* Initialize a unique, starting PAS Callback ID.     */
                  NextEventCallbackID = 0;

                  /* Go ahead and flag that this module is initialized. */
                  Initialized         = TRUE;

                  /* Flag success.                                      */
                  Result              = 0;
               }
            }
            else
               Result = BTPM_ERROR_CODE_INVALID_PARAMETER;
         }
         else
            Result = BTPM_ERROR_CODE_UNABLE_TO_REGISTER_HANDLER;

         /* If an error occurred then we need to free all resources that*/
         /* were allocated.                                             */
         if(Result)
         {
            _PASM_Cleanup();

            MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_PHONE_ALERT_STATUS_MANAGER);
         }
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("PAS Manager Already initialized\n"));
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("Shutdown Request\n"));

      if(Initialized)
      {
         /* First, let's make sure that we un-register our Message Group*/
         /* Handler (so that we do not process any more incoming        */
         /* messages).                                                  */
         MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_PHONE_ALERT_STATUS_MANAGER);

         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Make sure we inform the PAS Manager Implementation that  */
            /* we are shutting down.                                    */
            _PASM_Cleanup();

            /* Free the Connection Info list.                           */
            FreeConnectionEntryList(&ConnectionEntryList);

            /* Free the Server Event Callback Info List.                */
            FreeEventCallbackInfoList(&ServerEventCallbackInfoList);

            /* Flag that the device is not powered on.                  */
            CurrentPowerState = FALSE;

            /* Flag that this module is no longer initialized.          */
            Initialized       = FALSE;

            /* Free the Lock we acquired earlier.                       */
            DEVM_ReleaseLock();
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing asynchronous */
   /* Device Manager (DEVM) Events (including Power On/Off events).     */
   /* This function should be registered with the Bluetopia Platform    */
   /* Manager Module Handler and will be called when an asynchronous    */
   /* Device Manager event is dispatched.                               */
void BTPSAPI PASM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData)
{
   int                            Result;
   Connection_Entry_t             ConnectionEntry;
   Connection_Entry_t            *ConnectionEntryPtr;
   GATT_Attribute_Handle_Group_t  ServiceHandleRange;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the PAS Manager has been initialized.   */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         switch(EventData->EventType)
         {
            case detDevicePoweredOn:
               DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power On Occurred.\n"));

               /* Power on Event.                                       */

               /* Note the new Power State.                             */
               CurrentPowerState = TRUE;

               /* Go ahead and inform the PAS Manager that it should    */
               /* initialize.                                           */
               if((Result = DEVM_QueryDeviceBluetoothStackID()) > 0)
               {
                  /* Attempt to calculate the Service Handle Range for  */
                  /* this service in the GATT database.                 */
                  if(CalculateServiceHandleRange(&ServiceHandleRange))
                     _PASM_SetBluetoothStackID((unsigned int)Result, &ServiceHandleRange);
                  else
                     _PASM_SetBluetoothStackID((unsigned int)Result, NULL);
               }
               break;
            case detDevicePoweringOff:
            case detDevicePoweredOff:
               DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power Off Occurred.\n"));

               /* Note the new Power State.                             */
               CurrentPowerState = FALSE;

               /* Inform the PAS Manager that the Stack has been closed.*/
               _PASM_SetBluetoothStackID(0, NULL);

               /* Free the Connection Info list.                        */
               FreeConnectionEntryList(&ConnectionEntryList);
               break;
            case detRemoteDevicePropertiesChanged:
               DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("Remote Device Properties Changed.\n"));

               /* Process the Remote Device Properties Changed Event.   */
               ProcessRemoteDevicePropertiesChangedEvent(EventData->EventData.RemoteDevicePropertiesChangedEventData.ChangedMemberMask, &(EventData->EventData.RemoteDevicePropertiesChangedEventData.RemoteDeviceProperties));
               break;
            case detRemoteDeviceDeleted:
               DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("Remote Device Deleted.\n"));

               /* Delete the specified Connection Entry from the list   */
               /* (if any exists).                                      */
               if((ConnectionEntryPtr = DeleteConnectionEntryByBD_ADDR(&ConnectionEntryList, EventData->EventData.RemoteDeviceDeletedEventData.RemoteDeviceAddress)) != NULL)
               {
                  /* Dispatch a PASM Disconnection Event.               */
                  DispatchPASMDisconnectionEvent(ConnectionEntryPtr);

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow the caller the ability*/
   /* to notify the PAS Manager of a specific Update Event.  The PAS    */
   /* Manager can then take the correct action to process the update.   */
Boolean_t PASM_NotifyUpdate(PASM_Update_Data_t *UpdateData)
{
   Boolean_t ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, make sure that the input parameters appear to be           */
   /* semi-valid.                                                       */
   if(UpdateData)
   {
      switch(UpdateData->UpdateType)
      {
         case utPASSServerEvent:
            DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("Queueing PAS Event: %d\n", UpdateData->UpdateData.PASSEventData.Event_Type));

            /* Simply Queue the Dispatch Data.                          */
            /* * NOTE * To avoid copying again, we will simply submit   */
            /*          the entire Notification Message.                */
            ret_val = BTPM_QueueMailboxCallback(BTPMDispatchCallback_PASS, (void *)UpdateData);
            break;
         default:
            DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled Notification: %d\n", UpdateData->UpdateType));

            ret_val = FALSE;
            break;
      }
   }
   else
      ret_val = FALSE;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a Phone Alert Status Server callback function */
   /* with the Phone Alert Status (PAS) Manager Service.  This Callback */
   /* will be dispatched by the PAS Manager when various PAS Manager    */
   /* Server Events occur.  This function accepts the Callback Function */
   /* and Callback Parameter (respectively) to call when a PAS Manager  */
   /* Server Event needs to be dispatched.  This function returns a     */
   /* positive (non-zero) value if successful, or a negative return     */
   /* error code if there was an error.                                 */
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          PASM_Un_Register_Server_Event_Callback() function to     */
   /*          un-register the callback from this module.               */
   /* * NOTE * Only 1 Server Event Callback can be registered in the    */
   /*          system at a time.                                        */
int BTPSAPI PASM_Register_Server_Event_Callback(PASM_Event_Callback_t CallbackFunction, void *CallbackParameter)
{
   int                         ret_val;
   PASM_Event_Callback_Info_t  EventCallbackEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Phone Alert Status Manager has been */
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
            /* Verify that we do not already have a registered Server   */
            /* Callback, as (FOR NOW) PASM is a singluton.              */
            if(ServerEventCallbackInfoList == NULL)
            {
               /* Attempt to add an entry into the Event Callback Entry */
               /* list.                                                 */
               BTPS_MemInitialize(&EventCallbackEntry, 0, sizeof(PASM_Event_Callback_Info_t));

               EventCallbackEntry.EventCallbackID   = GetNextEventCallbackID();
               EventCallbackEntry.ClientID          = MSG_GetServerAddressID();
               EventCallbackEntry.EventCallback     = CallbackFunction;
               EventCallbackEntry.CallbackParameter = CallbackParameter;

               if(AddEventCallbackInfoEntry(&ServerEventCallbackInfoList, &EventCallbackEntry))
                  ret_val = EventCallbackEntry.EventCallbackID;
               else
                  ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
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
      ret_val = BTPM_ERROR_CODE_PHONE_ALERT_STATUS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered Phone Alert Status (PAS)      */
   /* Manager Server Event Callback (registered via a successful call to*/
   /* the PASM_Register_Server_Event_Callback() function).  This        */
   /* function accepts as input the PAS Manager Event Callback ID       */
   /* (return value from PASM_Register_Server_Event_Callback()          */
   /* function).                                                        */
void BTPSAPI PASM_Un_Register_Server_Event_Callback(unsigned int ServerCallbackID)
{
   PASM_Event_Callback_Info_t *EventCallbackPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Phone Alert Status Manager has been */
   /* initialized.                                                      */
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
            if((EventCallbackPtr = SearchEventCallbackInfoEntry(&ServerEventCallbackInfoList, ServerCallbackID)) != NULL)
            {
               /* Verify that this is owned by the Server Process.      */
               if(EventCallbackPtr->ClientID == MSG_GetServerAddressID())
               {
                  /* Delete the callback back from the list.            */
                  if((EventCallbackPtr = DeleteEventCallbackInfoEntry(&ServerEventCallbackInfoList, ServerCallbackID)) != NULL)
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

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit:\n"));
}

   /* The following function is provided to allow a mechanism of        */
   /* changing the current Phone Alert Status (PAS).  This function is  */
   /* responsible for updating the Alert Status internally, as well as  */
   /* dispatching any Alert Notifications that have been registered by  */
   /* Phone Alert Status (PAS) clients.  This function accepts as it's  */
   /* parameter the Server callback ID that was returned from a         */
   /* successful call to PASM_Register_Server_Event_Callback() followed */
   /* by the Alert Status value to set.  This function returns zero if  */
   /* successful, or a negative return error code if there was an error.*/
int BTPSAPI PASM_Set_Alert_Status(unsigned int ServerCallbackID, PASM_Alert_Status_t *AlertStatus)
{
   int                         ret_val;
   PASM_Event_Callback_Info_t *EventCallbackPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Phone Alert Status Manager has been */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check to see if the input parameters appear to be semi-valid.  */
      if((ServerCallbackID) && (AlertStatus))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Search for the Specified Callback entry.                 */
            if((EventCallbackPtr = SearchEventCallbackInfoEntry(&ServerEventCallbackInfoList, ServerCallbackID)) != NULL)
            {
               /* Verify that this is owned by the Server Process.      */
               if(EventCallbackPtr->ClientID == MSG_GetServerAddressID())
               {
                  /* Simply call the internal function to handle the    */
                  /* request to set the Alert Status.                   */
                  ret_val = ProcessSetAlertStatusRequest(AlertStatus);
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
      ret_val = BTPM_ERROR_CODE_PHONE_ALERT_STATUS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism of        */
   /* querying the last configured Phone Alert Status (PAS) alert       */
   /* status.  This function accepts as it's parameter the Server       */
   /* callback ID that was returned from a successful call to           */
   /* PASM_Register_Server_Event_Callback() followed by a buffer that   */
   /* will hold the currently configured alert status upon successful   */
   /* execution of this function.  This function returns zero if        */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * If this function returns success then the AlertStatus    */
   /*          buffer will contain the currently configured alert       */
   /*          status.  If this function returns an error then the      */
   /*          contents of the AlertStatus buffer will be undefined.    */
int BTPSAPI PASM_Query_Alert_Status(unsigned int ServerCallbackID, PASM_Alert_Status_t *AlertStatus)
{
   int                         ret_val;
   PASM_Event_Callback_Info_t *EventCallbackPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Phone Alert Status Manager has been */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check to see if the input parameters appear to be semi-valid.  */
      if((ServerCallbackID) && (AlertStatus))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Search for the Specified Callback entry.                 */
            if((EventCallbackPtr = SearchEventCallbackInfoEntry(&ServerEventCallbackInfoList, ServerCallbackID)) != NULL)
            {
               /* Verify that this is owned by the Server Process.      */
               if(EventCallbackPtr->ClientID == MSG_GetServerAddressID())
               {
                  /* Return the current Alert Status.                   */
                  *AlertStatus = PASMAlertStatus;

                  /* Return success.                                    */
                  ret_val      = 0;
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
      ret_val = BTPM_ERROR_CODE_PHONE_ALERT_STATUS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism of        */
   /* changing the current Phone Alert Status (PAS) ringer setting as   */
   /* well as dispatching any Alert Notifications that have been        */
   /* registered by PAS clients.  This function accepts as it's         */
   /* parameter the PAS Server callback ID that was returned from a     */
   /* successful call to PASM_Register_Server_Event_Callback() and the  */
   /* ringer value to configure.  This function returns zero if         */
   /* successful, or a negative return error code if there was an error.*/
int BTPSAPI PASM_Set_Ringer_Setting(unsigned int ServerCallbackID, PASM_Ringer_Setting_t RingerSetting)
{
   int                         ret_val;
   PASM_Event_Callback_Info_t *EventCallbackPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Phone Alert Status Manager has been */
   /* initialized.                                                      */
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
            if((EventCallbackPtr = SearchEventCallbackInfoEntry(&ServerEventCallbackInfoList, ServerCallbackID)) != NULL)
            {
               /* Verify that this is owned by the Server Process.      */
               if(EventCallbackPtr->ClientID == MSG_GetServerAddressID())
               {
                  /* Simply call the internal function to handle the    */
                  /* request to set the Ringer Setting.                 */
                  ret_val = ProcessSetRingerSettingRequest(RingerSetting);
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
      ret_val = BTPM_ERROR_CODE_PHONE_ALERT_STATUS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism of        */
   /* querying the last configured Phone Alert Status (PAS) ringer      */
   /* setting.  This function accepts as it's parameter the Server      */
   /* callback ID that was returned from a successful call to           */
   /* PASM_Register_Server_Event_Callback() followed by a buffer that   */
   /* will hold the currently configured ringer setting upon successful */
   /* execution of this function.  This function returns zero if        */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * If this function returns success then the RingerSetting  */
   /*          buffer will contain the currently configured ringer      */
   /*          setting.  If this function returns an error then the     */
   /*          contents of the RingerSetting buffer will be undefined.  */
int BTPSAPI PASM_Query_Ringer_Setting(unsigned int ServerCallbackID, PASM_Ringer_Setting_t *RingerSetting)
{
   int                         ret_val;
   PASM_Event_Callback_Info_t *EventCallbackPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Phone Alert Status Manager has been */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check to see if the input parameters appear to be semi-valid.  */
      if((ServerCallbackID) && (RingerSetting))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Search for the Specified Callback entry.                 */
            if((EventCallbackPtr = SearchEventCallbackInfoEntry(&ServerEventCallbackInfoList, ServerCallbackID)) != NULL)
            {
               /* Verify that this is owned by the Server Process.      */
               if(EventCallbackPtr->ClientID == MSG_GetServerAddressID())
               {
                  /* Return the current Ringer Setting.                 */
                  *RingerSetting = PASMRingerSetting;

                  /* Return success.                                    */
                  ret_val        = 0;
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
      ret_val = BTPM_ERROR_CODE_PHONE_ALERT_STATUS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PHONE_ALERT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

