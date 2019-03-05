/*****< btpmpbam.c >***********************************************************/
/*      Copyright 2011 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMPBAM - Phone Book Access Manager for Stonestreet One Bluetooth        */
/*             Protocol Stack Platform Manager.                               */
/*                                                                            */
/*  Author:  Greg Hensley                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   09/28/11  G. Hensley     Initial creation.                               */
/******************************************************************************/
#include <string.h>              /* Used for path manipulation.               */

#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMPBAM.h"            /* BTPM PBAM Manager Prototypes/Constants.   */
#include "PBAMAPI.h"             /* PBAM Manager Prototypes/Constants.        */
#include "PBAMMSG.h"             /* BTPM PBAM Manager Message Formats.        */
#include "PBAMGR.h"              /* PBAM Manager Impl. Prototypes/Constants.  */

#include "SS1BTPM.h"             /* BTPM Main Prototypes and Constants.       */
#include "BTPMERR.h"             /* BTPM Error Prototypes/Constants.          */
#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

   /* The following enumerated type is used to denote the various states*/
   /* that are used when tracking connections.                          */
typedef enum
{
   csIdle,
   csAuthorizing,
   csAuthenticating,
   csEncrypting,
   csConnecting,
   csConnected
} Connection_State_t;

   /* Structure which is used to track Callback Information related to  */
   /* this module.                                                      */
typedef struct _tagPBAM_Entry_Info_t
{
   unsigned int                  ConnectionStatus;
   unsigned int                  RemoteServerPort;
   unsigned int                  ClientID;
   Connection_State_t            ConnectionState;
   PBAM_VCard_Format_t           LastVCardRequestFormt;
   Event_t                       ConnectionEvent;
   unsigned long                 Flags;
   BD_ADDR_t                     BluetoothAddress;
   unsigned int                  PBAPID;
   char                         *PhonebookPath;
   char                         *PendingPath;
   unsigned int                  PendingPathOffset;
   PBAM_Event_Callback_t         EventCallback;
   void                         *CallbackParameter;
   struct _tagPBAM_Entry_Info_t *NextPBAMEntryInfoPtr;
} PBAM_Entry_Info_t;

   /* The following functions are used with the Flags member of the     */
   /* PBAM_Entry_Info_t structure to denote various state information.  */
#define PBAM_ENTRY_INFO_FLAGS_PENDING_PHONE_BOOK_SIZE       0x04000000
#define PBAM_ENTRY_INFO_FLAGS_PENDING_ABORT                 0x08000000
#define PBAM_ENTRY_INFO_FLAGS_SYNCHRONOUS_CONNECT_REQUEST   0x10000000
#define PBAM_ENTRY_INFO_FLAGS_LOCALLY_HANDLED               0x20000000
#define PBAM_ENTRY_INFO_FLAGS_EVENT_CLOSING                 0x40000000
#define PBAM_ENTRY_INFO_FLAGS_PENDING_ABSOLUTE_PATH_CHANGE  0x80000000

typedef enum
{
   coNone,
   coPullPhonebook,
   coPullPhonebookSize,
   coSetPhonebook,
   coPullvCardListing,
   coPullvCardListingSize,
   coPullvCard
} Current_Operation_t;

   /* Structure which holds information pertaining to an open server    */
   /* instance.                                                         */
typedef struct _tagPBAM_Server_Entry_t
{
   unsigned int ServerID;
   unsigned int PBAPID;
   unsigned int ConnectionID;

   BD_ADDR_t RemoteDeviceAddress;
   Connection_State_t ConnectionState;
   Current_Operation_t CurrentOperation;

   unsigned int PortNumber;
   DWord_t ServiceRecordHandle;
   Byte_t SupportedRepositories;

   unsigned int                  DataBufferSize;
   unsigned int                  DataBufferSent;
   Byte_t                       *DataBuffer;
   Boolean_t                     DataFinal;

   unsigned long Flags;
   unsigned long ConnectionFlags;

   unsigned int ClientID;
   PBAM_Event_Callback_t CallbackFunction;
   void *CallbackParameter;

   struct _tagPBAM_Server_Entry_t *NextServerEntryPtr;
} PBAM_Server_Entry_t;

   /* Internal Variables to this Module (remember that all variables    */
   /* declared static are initialized to 0 automatically by the compiler*/
   /* as part of standard C/C++).                                       */

   /* Variable which serves as a global flag about whether this module  */
   /* is initialized or not.                                            */
static Boolean_t Initialized;

   /* Variable which holds the current power state of the device.       */
static Boolean_t CurrentPowerState;

   /* Variables which hold the next IDs for the different type tracking */
   /* structures.                                                       */
static unsigned int NextServerID;
static unsigned int NextConnectionID;

   /* Variables which hold a pointer to the first element in the Phone  */
   /* Book Access entry information list (which holds all callbacks     */
   /* tracked by this module).                                          */
static PBAM_Entry_Info_t *PBAMEntryInfoList;
static PBAM_Server_Entry_t *PBAMServerEntryList;

   /* Internal Function Prototypes.                                     */
static unsigned int GetNextServerID(void);
static unsigned int GetNextConnectionID(void);

   /* Custom list routines to add/remove/free the specific type of      */
   /* structure PBAM_Entry_Info_t.                                      */
static PBAM_Entry_Info_t *AddPBAMEntryInfoEntry(PBAM_Entry_Info_t **ListHead, PBAM_Entry_Info_t *EntryToAdd);
static PBAM_Entry_Info_t *SearchPBAMEntryInfoEntry(PBAM_Entry_Info_t **ListHead, BD_ADDR_t *BluetoothAddress, unsigned int PBAPID);
static PBAM_Entry_Info_t *DeletePBAMEntryInfoEntry(PBAM_Entry_Info_t **ListHead, BD_ADDR_t *BluetoothAddress, unsigned int PBAPID);
static void FreePBAMEntryInfoEntryMemory(PBAM_Entry_Info_t *EntryToFree);
static void FreePBAMEntryInfoList(PBAM_Entry_Info_t **ListHead);

static PBAM_Server_Entry_t *AddPBAMServerEntry(PBAM_Server_Entry_t **ListHead, PBAM_Server_Entry_t *EntryToAdd);
static PBAM_Server_Entry_t *SearchPBAMServerEntryServerID(PBAM_Server_Entry_t **ListHead, unsigned int ServerID);
static PBAM_Server_Entry_t *SearchPBAMServerEntryPBAPID(PBAM_Server_Entry_t **ListHead, unsigned int PBAPID);
static PBAM_Server_Entry_t *SearchPBAMServerEntryConnectionID(PBAM_Server_Entry_t **ListHead, unsigned int ConnectionID);
static PBAM_Server_Entry_t *DeletePBAMServerEntry(PBAM_Server_Entry_t **ListHead, unsigned int ServerID);
static void FreePBAMServerEntryMemory(PBAM_Server_Entry_t *EntryToFree);
static void FreePBAMServerEntryList(PBAM_Server_Entry_t **ListHead);

static void CleanupPBAMServerEntry(PBAM_Server_Entry_t *EntryToCleanup);

static Boolean_t MapResponseStatusCodeToResponseCode(unsigned int ResponseStatusCode, Byte_t *ResponseCode);

   /* BluetopiaPM server IPC events process handling                    */
static void ProcessDeviceConnectionStatusEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int ConnectionStatus);
static void ProcessDeviceDisconnectionEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int DisconnectReason);
static void ProcessPhoneBookSizeEvent(BD_ADDR_t RemoteDeviceAddress, int Status, unsigned int PhoneBookSize);
static void ProcessPhoneBookSetEvent(BD_ADDR_t RemoteDeviceAddress, int Status, unsigned int CurrentPathSize, Byte_t *CurrentPath);
static void ProcessVCardDataEvent(BD_ADDR_t RemoteDeviceAddress, int Status, Boolean_t Final, unsigned int NewMissedCalls, PBAM_VCard_Format_t Format, unsigned int BufferSize, Byte_t *Buffer);
static void ProcessVCardListingDataEvent(BD_ADDR_t RemoteDeviceAddress, int Status, Boolean_t Final, unsigned int NewMissedCalls, unsigned int BufferSize, Byte_t *Buffer);
static void ProcessReceivedMessage(BTPM_Message_t *Message);

   /* BluetopiaPM client IPC events process handling                    */
static void ProcessConnectRemoteDevice(PBAM_Connect_Remote_Device_Request_t *Message);
static void ProcessDisconnectDeviceRequest(PBAM_Disconnect_Device_Request_t *Message);
static void ProcessAbortRequest(PBAM_Abort_Request_t *Message);
static void ProcessPullPhoneBookRequest(PBAM_Pull_Phone_Book_Request_t *Message);
static void ProcessPullPhoneBookSizeRequest(PBAM_Pull_Phone_Book_Size_Request_t *Message);
static void ProcessSetPhoneBookRequest(PBAM_Set_Phone_Book_Request_t *Message);
static void ProcessPullvCardListingRequest(PBAM_Pull_vCard_Listing_Request_t *Message);
static void ProcessPullvCardRequest(PBAM_Pull_vCard_Request_t *Message);
static void ProcessSetPhoneBookAbsoluteRequest(PBAM_Set_Phone_Book_Absolute_Request_t *Message);

   /* BluetopiaPM - PSE Client IPC process handling.                    */
static void ProcessRegisterServerMessage(PBAM_Register_Server_Request_t *Message);
static void ProcessUnRegisterServerMessage(PBAM_Un_Register_Server_Request_t *Message);
static void ProcessConnectionRequestResponseMessage(PBAM_Connection_Request_Response_Request_t *Message);
static void ProcessCloseServerConnectionMessage(PBAM_Close_Server_Connection_Request_t *Message);
static void ProcessSendPhoneBookMessage(PBAM_Send_Phone_Book_Request_t *Message);
static void ProcessSendPhoneBookSizeMessage(PBAM_Send_Phone_Book_Size_Request_t *Message);
static void ProcessSetPhoneBookResponseMessage(PBAM_Set_Phone_Book_Response_Request_t *Message);
static void ProcessSendvCardListingMessage(PBAM_Send_vCard_Listing_Request_t *Message);
static void ProcessSendvCardListingSizeMessage(PBAM_Send_vCard_Listing_Size_Request_t *Message);
static void ProcessSendvCardMessage(PBAM_Send_vCard_Request_t *Message);

static void ProcessReceivedRequestMessage(BTPM_Message_t *Message);

static void BTPSAPI BTPMDispatchCallback_PBAM(void *CallbackParameter);
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter);

static void DispatchPBAMEvent(PBAM_Entry_Info_t *EntryInfo, BTPM_Message_t *Message);
static void DispatchPBAMServerEvent(PBAM_Server_Entry_t *EntryInfo, PBAM_Event_Data_t *EventData, BTPM_Message_t *Message);

   /* Bluetopia PBAP-PCE Events                                         */
static void ProcessOpenConfirmationEvent(PBAP_Open_Port_Confirmation_Data_t *OpenConfirmationData);
static void ProcessCloseIndicationEvent(PBAP_Close_Port_Indication_Data_t *CloseIndicationData);
static void ProcessAbortConfirmationEvent(PBAP_Abort_Confirmation_Data_t *AbortConfirmationData);
static void ProcessPullPhoneBookConfirmationEvent(PBAP_Pull_Phonebook_Confirmation_Data_t *PullPhoneBookConfirmationData);
static void ProcessSetPhoneBookConfirmationEvent(PBAP_Set_Phonebook_Confirmation_Data_t *SetPhoneBookConfirmationData);
static void ProcessPullvCardListingConfirmationEvent(PBAP_Pull_vCard_Listing_Confirmation_Data_t *PullvCardListingConfirmationData);
static void ProcessPullvCardEntryConfirmationEvent(PBAP_Pull_vCard_Entry_Confirmation_Data_t *PullvCardEntryConfirmationData);

   /* Bluetopia PBAP-PSE Events                                         */
static void ProcessOpenRequestIndication(PBAP_Open_Port_Request_Indication_Data_t *OpenRequestIndicationData);
static void ProcessOpenIndication(PBAP_Open_Port_Indication_Data_t *OpenIndicationData);
static void ProcessAbortIndication(PBAP_Abort_Indication_Data_t *AbortIndicationData);
static void ProcessPullPhonebookIndication(PBAP_Pull_Phonebook_Indication_Data_t *PullPhonebookIndicationData);
static void ProcessSetPhonebookIndication(PBAP_Set_Phonebook_Indication_Data_t *SetPhonebookIndicationData);
static void ProcessPullvCardListingIndication(PBAP_Pull_vCard_Listing_Indication_Data_t *PullvCardListingIndicationData);
static void ProcessPullvCardEntryIndication(PBAP_Pull_vCard_Entry_Indication_Data_t *PullvCardEntryIndicationData);

static void ProcessPBAPEvent(PBAM_Phone_Book_Access_Event_Data_t *PBAPEventData);
static void BTPSAPI BTPMDispatchCallback_PBAP(void *CallbackParameter);

static void ProcessDEVMStatusEvent(BD_ADDR_t BluetoothAddress, int Status);
static void BTPSAPI PhoneBookAccessManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter);

static int _Disconnect_Device(BD_ADDR_t RemoteDeviceAddress, unsigned int ClientID);
static int _Abort(BD_ADDR_t RemoteDeviceAddress, unsigned int ClientID);
static int _Pull_Phone_Book(BD_ADDR_t RemoteDeviceAddress, char *PhoneBookNamePath, DWord_t FilterLow, DWord_t FilterHigh, PBAM_VCard_Format_t VCardFormat, Word_t MaxListCount, Word_t ListStartOffset, unsigned int ClientID);
static int _Pull_Phone_Book_Size(BD_ADDR_t RemoteDeviceAddress, unsigned int ClientID);
static int _Set_Phone_Book(BD_ADDR_t RemoteDeviceAddress, PBAM_Set_Path_Option_t PathOption, char *FolderName, unsigned int ClientID);
static int _Pull_vCard_Listing(BD_ADDR_t RemoteDeviceAddress, char *PhonebookPath, PBAM_List_Order_t ListOrder, PBAM_Search_Attribute_t SearchAttribute, char *SearchValue, Word_t MaxListCount, Word_t ListStartOffset, unsigned int ClientID);
static int _Pull_vCard(BD_ADDR_t RemoteDeviceAddress, char *VCardName, DWord_t FilterLow, DWord_t FilterHigh, PBAM_VCard_Format_t VCardFormat, unsigned int ClientID);
static int _Set_Phone_Book_Absolute(BD_ADDR_t RemoteDeviceAddress, char *AbsolutePath, unsigned int ClientID);

   /* PBAP-PSE Command Handler Helper functions.                        */
static int ProcessRegisterServer(unsigned int ClientID, unsigned int ServerPort, unsigned int SupportedRepositories, unsigned long IncomingConnectionFlags, char *ServiceName, PBAM_Event_Callback_t EventCallback, void *CallbackParameter);
static int ProcessUnRegisterServer(unsigned int ClientID, unsigned int ServerID);
static int ProcessConnectionRequestResponse(unsigned int ClientID, unsigned int ConnectionID, Boolean_t Accept);
static int ProcessCloseServerConnection(unsigned int ClientID, unsigned int ConnectionID);
static int ProcessSendPhoneBook(unsigned int ClientID, unsigned int ConnectionID, unsigned int ResponseStatusCode, Byte_t *NewMissedCalls, unsigned int BufferSize, Byte_t *Buffer, Boolean_t Final);
static int ProcessSendPhoneBookSize(unsigned int ClientID, unsigned int ConnectionID, unsigned int ResponseStatusCode, Word_t PhoneBookSize);
static int ProcessSetPhoneBookResponse(unsigned int ClientID, unsigned int ConnectionID, unsigned int ResponseStatusCode);
static int ProcessSendvCardListing(unsigned int ClientID, unsigned int ConnectionID, unsigned int ResponseStatusCode, Byte_t *NewMissedCalls, unsigned int BufferSize, Byte_t *Buffer, Boolean_t Final);
static int ProcessSendvCardListingSize(unsigned int ClientID, unsigned int ConnectionID, unsigned int ResponseStatusCode, Word_t vCardListingSize);
static int ProcessSendvCard(unsigned int ClientID, unsigned int ConnectionID, unsigned int ResponseStatusCode, unsigned int BufferSize, Byte_t *Buffer, Boolean_t Final);

   /* The following function is a utility function that exists to       */
   /* retrieve a unique Server ID that can be used to track a PBAP      */
   /* server instance.                                                  */
static unsigned int GetNextServerID(void)
{
   unsigned int ret_val;

   ret_val = ++NextServerID;

   if(NextServerID & 0x80000000)
      NextServerID = 1;

   return(ret_val);
}

   /* The following function is a utility function that exists to       */
   /* retrieve a unique Connection ID that can be used to track a PBAP  */
   /* server instance connection.                                       */
static unsigned int GetNextConnectionID(void)
{
   unsigned int ret_val;

   ret_val = ++NextConnectionID;

   if(NextConnectionID & 0x80000000)
      NextConnectionID = 1;

   return(ret_val);
}

   /* The following function adds the specified entry to the specified  */
   /* list.  This function allocates and adds an entry to the list that */
   /* has the same attributes as the entry passed into this function.   */
   /* This function will return NULL if NO entry was added.  This can   */
   /* occur if the element passed in was deemed invalid or the actual   */
   /* list head was invalid.                                            */
   /* ** NOTE ** This function does not insert duplicate entries into   */
   /*            the list.  An element is considered a duplicate if the */
   /*            BluetoothAddress field is the same as an entry already */
   /*            int the list.  When this occurs, this function returns */
   /*            NULL.                                                  */
static PBAM_Entry_Info_t *AddPBAMEntryInfoEntry(PBAM_Entry_Info_t **ListHead, PBAM_Entry_Info_t *EntryToAdd)
{
   PBAM_Entry_Info_t *AddedEntry = NULL;
   PBAM_Entry_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First let's verify that values passed in are semi-valid.          */
   if((ListHead) && (EntryToAdd))
   {
      /* OK, data seems semi-valid, let's allocate a new data structure */
      /* to add to the list.                                            */
      AddedEntry = (PBAM_Entry_Info_t *)BTPS_AllocateMemory(sizeof(PBAM_Entry_Info_t));

      if(AddedEntry)
      {
         /* Copy All Data over.                                         */
         *AddedEntry                      = *EntryToAdd;

         /* Now Add it to the end of the list.                          */
         AddedEntry->NextPBAMEntryInfoPtr = NULL;

         /* First, let's check to see if there are any elements already */
         /* present in the List that was passed in.                     */
         if((tmpEntry = *ListHead) != NULL)
         {
            /* Head Pointer was not NULL, so we will traverse the list  */
            /* until we reach the last element.                         */
            while(tmpEntry)
            {
               if(COMPARE_BD_ADDR(tmpEntry->BluetoothAddress, AddedEntry->BluetoothAddress))
               {
                  /* Entry was already added, so free the memory and    */
                  /* flag an error to the caller.                       */
                  FreePBAMEntryInfoEntryMemory(AddedEntry);
                  AddedEntry = NULL;

                  /* Abort the Search.                                  */
                  tmpEntry   = NULL;
               }
               else
               {
                  /* OK, we need to see if we are at the last element of*/
                  /* the List.  If we are, we simply break out of the   */
                  /* list traversal because we know there are NO        */
                  /* duplicates AND we are at the end of the list.      */
                  if(tmpEntry->NextPBAMEntryInfoPtr)
                     tmpEntry = tmpEntry->NextPBAMEntryInfoPtr;
                  else
                     break;
               }
            }

            if(AddedEntry)
            {
               /* Last element found, simply Add the entry.             */
               tmpEntry->NextPBAMEntryInfoPtr = AddedEntry;
            }
         }
         else
            *ListHead = AddedEntry;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", AddedEntry));

   return(AddedEntry);
}

   /* The following function searches the specified list for the        */
   /* specified PBAPID or Bluetooth Address.  If BluetoothAddress is    */
   /* NULL then the specified PBAPID will be used for the search.  This */
   /* function returns NULL if either the the parameters are invalid, or*/
   /* the specified callback ID was NOT found.                          */
static PBAM_Entry_Info_t *SearchPBAMEntryInfoEntry(PBAM_Entry_Info_t **ListHead, BD_ADDR_t *BluetoothAddress, unsigned int PBAPID)
{
   PBAM_Entry_Info_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter %p %d\n", BluetoothAddress, PBAPID));

   /* Let's make sure the list and Bluetooth Address/PBAP ID to search  */
   /* for appear to be valid.                                           */
   if((ListHead) && ((BluetoothAddress) || (PBAPID)))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      if(BluetoothAddress)
      {
         while((FoundEntry) && (!COMPARE_BD_ADDR(FoundEntry->BluetoothAddress, *BluetoothAddress)))
            FoundEntry = FoundEntry->NextPBAMEntryInfoPtr;
      }
      else
      {
         while((FoundEntry) && (FoundEntry->PBAPID != PBAPID))
            FoundEntry = FoundEntry->NextPBAMEntryInfoPtr;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* The following function searches the specified Phone Book Access   */
   /* entry information list for the specified callback ID and removes  */
   /* it from the List. This function returns NULL if either the entry  */
   /* information list head is invalid, the BluetoothAddress is invalid,*/
   /* or the specified BluetoothAddress was NOT present in the list. The*/
   /* entry returned will have the next entry field set to NULL, and the*/
   /* caller is responsible for deleting the memory associated with this*/
   /* entry by calling FreePBAMEntryInfoEntryMemory().                  */
static PBAM_Entry_Info_t *DeletePBAMEntryInfoEntry(PBAM_Entry_Info_t **ListHead, BD_ADDR_t *BluetoothAddress, unsigned int PBAPID)
{
   PBAM_Entry_Info_t *FoundEntry = NULL;
   PBAM_Entry_Info_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter %p %d\n", BluetoothAddress, PBAPID));

   /* Let's make sure the list and Bluetooth Address/PBAP ID to search  */
   /* for appear to be valid.                                           */
   if(ListHead)
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      if(BluetoothAddress)
      {
         while((FoundEntry) && (!COMPARE_BD_ADDR(FoundEntry->BluetoothAddress, *BluetoothAddress)))
         {
            LastEntry  = FoundEntry;

            FoundEntry = FoundEntry->NextPBAMEntryInfoPtr;
         }
      }
      else
      {
         while((FoundEntry) && (FoundEntry->PBAPID != PBAPID))
         {
            LastEntry  = FoundEntry;

            FoundEntry = FoundEntry->NextPBAMEntryInfoPtr;
         }
      }

      /* Check to see if we found the specified entry.                  */
      if(FoundEntry)
      {
         /* OK, now let's remove the entry from the list.  We have to   */
         /* check to see if the entry was the first entry in the list.  */
         if(LastEntry)
         {
            /* Entry was NOT the first entry in the list.               */
            LastEntry->NextPBAMEntryInfoPtr = FoundEntry->NextPBAMEntryInfoPtr;
         }
         else
            *ListHead = FoundEntry->NextPBAMEntryInfoPtr;

         FoundEntry->NextPBAMEntryInfoPtr = NULL;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* This function frees the specified Phone Book Access entry         */
   /* information member. No check is done on this entry other than     */
   /* making sure it NOT NULL.                                          */
static void FreePBAMEntryInfoEntryMemory(PBAM_Entry_Info_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToFree)
      BTPS_FreeMemory(EntryToFree);

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified Phone Book Access entry information list.*/
   /* Upon return of this function, the head pointer is set to NULL.    */
static void FreePBAMEntryInfoList(PBAM_Entry_Info_t **ListHead)
{
   PBAM_Entry_Info_t *EntryToFree;
   PBAM_Entry_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ListHead)
   {
      /* Simply traverse the list and free every element present.       */
      EntryToFree = *ListHead;

      while(EntryToFree)
      {
         tmpEntry    = EntryToFree;
         EntryToFree = EntryToFree->NextPBAMEntryInfoPtr;

         if(tmpEntry->ConnectionEvent)
            BTPS_CloseEvent(tmpEntry->ConnectionEvent);

         FreePBAMEntryInfoEntryMemory(tmpEntry);
      }

      /* Make sure the list appears to be empty.                        */
      *ListHead = NULL;
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function adds the specified entry to the specified  */
   /* list.  This function allocates and adds an entry to the list that */
   /* has the same attributes as the entry passed into this function.   */
   /* This function will return NULL if NO entry was added.  This can   */
   /* occur if the element passed in was deemed invalid or the actual   */
   /* list head was invalid.                                            */
static PBAM_Server_Entry_t *AddPBAMServerEntry(PBAM_Server_Entry_t **ListHead, PBAM_Server_Entry_t *EntryToAdd)
{
   /* NOTE: We WILL insert duplicate ServerIDs because it is possible   */
   /*       in the future that we may manage multiple ports for multiple*/
   /*       connections to the same server.                             */
   return (PBAM_Server_Entry_t *)BSC_AddGenericListEntry(sizeof(PBAM_Server_Entry_t), ekNone, 0, sizeof(PBAM_Server_Entry_t), BTPS_STRUCTURE_OFFSET(PBAM_Server_Entry_t, NextServerEntryPtr), (void **)ListHead, (void *)EntryToAdd);
}

   /* The following function searches the specified list for the        */
   /* specified ServerID.  This function returns NULL if either the the */
   /* parameters are invalid, or the specified ServerID was NOT found.  */
static PBAM_Server_Entry_t *SearchPBAMServerEntryServerID(PBAM_Server_Entry_t **ListHead, unsigned int ServerID)
{
   return BSC_SearchGenericListEntry(ekUnsignedInteger, (void *)&ServerID, BTPS_STRUCTURE_OFFSET(PBAM_Server_Entry_t, ServerID), BTPS_STRUCTURE_OFFSET(PBAM_Server_Entry_t, NextServerEntryPtr), (void **)ListHead);
}

   /* The following function searches the specified list for the        */
   /* specified PBAPID.  This function returns NULL if either the the   */
   /* parameters are invalid, or the specified PBAPID was NOT found.    */
static PBAM_Server_Entry_t *SearchPBAMServerEntryPBAPID(PBAM_Server_Entry_t **ListHead, unsigned int PBAPID)
{
   return BSC_SearchGenericListEntry(ekUnsignedInteger, (void *)&PBAPID, BTPS_STRUCTURE_OFFSET(PBAM_Server_Entry_t, PBAPID), BTPS_STRUCTURE_OFFSET(PBAM_Server_Entry_t, NextServerEntryPtr), (void **)ListHead);
}

   /* The following function searches the specified list for the        */
   /* specified ConnectionID.  This function returns NULL if either the */
   /* the parameters are invalid, or the specified Connection ID was NOT*/
   /* found.                                                            */
static PBAM_Server_Entry_t *SearchPBAMServerEntryConnectionID(PBAM_Server_Entry_t **ListHead, unsigned int ConnectionID)
{
   return BSC_SearchGenericListEntry(ekUnsignedInteger, (void *)&ConnectionID, BTPS_STRUCTURE_OFFSET(PBAM_Server_Entry_t, ConnectionID), BTPS_STRUCTURE_OFFSET(PBAM_Server_Entry_t, NextServerEntryPtr), (void **)ListHead);
}

   /* The following function searches the specified Phone Book Access   */
   /* entry information list for the specified PBAPID and removes it    */
   /* from the List. This function returns NULL if either the entry     */
   /* information list head is invalid, the PBAP is invalid, or the     */
   /* specified PBAP was NOT present in the list. The entry returned    */
   /* will have the next entry field set to NULL, and the caller is     */
   /* responsible for deleting the memory associated with this entry by */
   /* calling FreePBAMServerEntryMemory().                              */
static PBAM_Server_Entry_t *DeletePBAMServerEntry(PBAM_Server_Entry_t **ListHead, unsigned int PBAPID)
{
   return BSC_DeleteGenericListEntry(ekUnsignedInteger, (void *)&PBAPID, BTPS_STRUCTURE_OFFSET(PBAM_Server_Entry_t, PBAPID), BTPS_STRUCTURE_OFFSET(PBAM_Server_Entry_t, NextServerEntryPtr), (void **)ListHead);
}

   /* This function frees the specified Phone Book Access Server entry  */
   /* information member. No check is done on this entry other than     */
   /* making sure it NOT NULL.                                          */
static void FreePBAMServerEntryMemory(PBAM_Server_Entry_t *EntryToFree)
{
   if(EntryToFree)
   {
      /* Free any memory that was allocate inside the Server Entry.     */
      CleanupPBAMServerEntry(EntryToFree);

      /* Now, free the entry's memory.                                  */
      BSC_FreeGenericListEntryMemory((void *)EntryToFree);
   }
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified Phone Book Access Server entry           */
   /* information list.  Upon return of this function, the head pointer */
   /* is set to NULL.                                                   */
static void FreePBAMServerEntryList(PBAM_Server_Entry_t **ListHead)
{
   PBAM_Server_Entry_t *EntryToFree;
   PBAM_Server_Entry_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ListHead)
   {
      /* Simply traverse the list and free every element present.       */
      EntryToFree = *ListHead;

      while(EntryToFree)
      {
         tmpEntry    = EntryToFree;
         EntryToFree = EntryToFree->NextServerEntryPtr;

         FreePBAMServerEntryMemory(tmpEntry);
      }

      /* Make sure the list appears to be empty.                        */
      *ListHead = NULL;
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to free  */
   /* any resources that might have been allocated for the specified    */
   /* PBAM Server Entry.  This Entry *DOES NOT* free the Entry itself,  */
   /* just any resources that have not been freed by the Entry.         */
static void CleanupPBAMServerEntry(PBAM_Server_Entry_t *EntryToCleanup)
{
   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToCleanup)
   {
      /* If there was an on-going transfer we need to clean it up.      */
      if(EntryToCleanup->DataBuffer)
         BTPS_FreeMemory(EntryToCleanup->DataBuffer);

      /* Flag the resources that were freed (above) as not being        */
      /* present.                                                       */
      EntryToCleanup->DataBuffer        = NULL;
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* convert the Bluetopia PM Response Status Codes to the correct OBEX*/
   /* Response Code.  This function returns TRUE if the mapping was able*/
   /* to made successfully or FALSE if there was invalid parameter.     */
static Boolean_t MapResponseStatusCodeToResponseCode(unsigned int ResponseStatusCode, Byte_t *ResponseCode)
{
   Boolean_t ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %d \n", ResponseStatusCode));

   /* Check to make sure that we have a buffer to store the result into.*/
   if(ResponseCode)
   {
      /* Initialize success.                                            */
      ret_val = TRUE;

      /* Next, map the Status to the correct code.                      */
      switch(ResponseStatusCode)
      {
         case PBAM_RESPONSE_STATUS_CODE_SUCCESS:
            *ResponseCode = PBAP_OBEX_RESPONSE_OK;
            break;
         case PBAM_RESPONSE_STATUS_CODE_NOT_FOUND:
            *ResponseCode = PBAP_OBEX_RESPONSE_NOT_FOUND;
            break;
         case PBAM_RESPONSE_STATUS_CODE_SERVICE_UNAVAILABLE:
            *ResponseCode = PBAP_OBEX_RESPONSE_SERVICE_UNAVAILABLE;
            break;
         case PBAM_RESPONSE_STATUS_CODE_BAD_REQUEST:
            *ResponseCode = PBAP_OBEX_RESPONSE_BAD_REQUEST;
            break;
         case PBAM_RESPONSE_STATUS_CODE_NOT_IMPLEMENTED:
            *ResponseCode = PBAP_OBEX_RESPONSE_NOT_IMPLEMENTED;
            break;
         case PBAM_RESPONSE_STATUS_CODE_UNAUTHORIZED:
            *ResponseCode = PBAP_OBEX_RESPONSE_UNAUTHORIZED;
            break;
         case PBAM_RESPONSE_STATUS_CODE_PRECONDITION_FAILED:
            *ResponseCode = PBAP_OBEX_RESPONSE_PRECONDITION_FAILED;
            break;
         case PBAM_RESPONSE_STATUS_CODE_NOT_ACCEPTABLE:
            *ResponseCode = PBAP_OBEX_RESPONSE_NOT_ACCEPTABLE;
            break;
         case PBAM_RESPONSE_STATUS_CODE_FORBIDDEN:
            *ResponseCode = PBAP_OBEX_RESPONSE_FORBIDDEN;
            break;
         case PBAM_RESPONSE_STATUS_CODE_DEVICE_POWER_OFF:
         case PBAM_RESPONSE_STATUS_CODE_UNKNOWN:
         default:
            ret_val = FALSE;
            break;
      }
   }
   else
      ret_val = FALSE;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for processing the device   */
   /* connection status asynchronous message.                           */
   /* * NOTE * This function *MUST* be called with the Phone Book Access*/
   /*          Manager Mutex held.  This function will release the Mutex*/
   /*          before it exits (i.e. the caller SHOULD NOT RELEASE THE  */
   /*          MUTEX).                                                  */
static void ProcessDeviceConnectionStatusEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int ConnectionStatus)
{
   void                  *CallbackParameter;
   PBAM_Event_Data_t      EventData;
   PBAM_Entry_Info_t     *PBAMEntryInfo;
   PBAM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, determine if there is an Event Callback waiting on this    */
   /* connection result.                                                */
   if(PBAMEntryInfoList)
   {
      if(((PBAMEntryInfo = SearchPBAMEntryInfoEntry(&PBAMEntryInfoList, &RemoteDeviceAddress, 0)) != NULL))
      {
         if((PBAMEntryInfo->Flags & PBAM_ENTRY_INFO_FLAGS_SYNCHRONOUS_CONNECT_REQUEST) && (COMPARE_BD_ADDR(PBAMEntryInfo->BluetoothAddress, RemoteDeviceAddress)))
         {
            /* Note the Status.                                         */
            PBAMEntryInfo->ConnectionStatus = ConnectionStatus;

            /* Set the Event.                                           */
            BTPS_SetEvent(PBAMEntryInfo->ConnectionEvent);

            /* Release the Mutex because we are finished with it.       */
            DEVM_ReleaseLock();
         }
         else
         {
            /* Asynchronous Entry, go ahead dispatch the result.        */

            /* Note the Callback information.                           */
            EventCallback     = PBAMEntryInfo->EventCallback;
            CallbackParameter = PBAMEntryInfo->CallbackParameter;

            /* Event needs to be dispatched. Go ahead and format the    */
            /* event.                                                   */
            BTPS_MemInitialize(&EventData, 0, sizeof(PBAM_Event_Data_t));

            /* Format up the event.                                     */
            EventData.EventType                                               = petConnectionStatus;
            EventData.EventLength                                             = PBAM_CONNECTION_STATUS_EVENT_DATA_SIZE;

            EventData.EventData.ConnectionStatusEventData.RemoteDeviceAddress = RemoteDeviceAddress;
            EventData.EventData.ConnectionStatusEventData.ConnectionStatus    = ConnectionStatus;

            /* Release the Mutex so we can dispatch the event.          */
            DEVM_ReleaseLock();

            __BTPSTRY
            {
               if(EventCallback)
                  (*EventCallback)(&EventData, CallbackParameter);
            }
            __BTPSEXCEPT(1)
            {
               /* Do Nothing.                                           */
            }
         }
      }
      else
      {
         /* Error - Release the Mutex.                                  */
         DEVM_ReleaseLock();

         DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_CRITICAL), ("Unable to locate device\n"));
      }
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      DEVM_ReleaseLock();
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the device   */
   /* disconnection asynchronous message.                               */
   /* * NOTE * This function *MUST* be called with the Phone Book Access*/
   /*          Manager Mutex held.  This function will release the Mutex*/
   /*          before it exits (i.e. the caller SHOULD NOT RELEASE THE  */
   /*          MUTEX).                                                  */
static void ProcessDeviceDisconnectionEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int DisconnectReason)
{
   PBAM_Event_Data_t      EventData;
   PBAM_Entry_Info_t     *PBAMEntryInfo;
   PBAM_Event_Callback_t  EventCallback;
   void                  *CallbackParameter;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Delete the device on disconnect                                   */
   if(((PBAMEntryInfo = DeletePBAMEntryInfoEntry(&PBAMEntryInfoList, &RemoteDeviceAddress, 0)) != NULL))
   {
      /* Note the Callback information.                                 */
      EventCallback     = PBAMEntryInfo->EventCallback;
      CallbackParameter = PBAMEntryInfo->CallbackParameter;

      /* Event needs to be dispatched.  Go ahead and format the event.  */
      BTPS_MemInitialize(&EventData, 0, sizeof(PBAM_Event_Data_t));

      /* Format up the Event.                                           */
      EventData.EventType                                           = petDisconnected;
      EventData.EventLength                                         = PBAM_DISCONNECTED_EVENT_DATA_SIZE;

      EventData.EventData.DisconnectedEventData.RemoteDeviceAddress = RemoteDeviceAddress;
      EventData.EventData.DisconnectedEventData.DisconnectReason    = DisconnectReason;

      FreePBAMEntryInfoEntryMemory(PBAMEntryInfo);

      /* Release the Mutex so we can dispatch the event.                */
      DEVM_ReleaseLock();

      __BTPSTRY
      {
         if(EventCallback)
            (*EventCallback)(&EventData, CallbackParameter);
      }
      __BTPSEXCEPT(1)
      {
         /* Do Nothing.                                                 */
      }
   }
   else
   {
      /* Error - Release the Mutex.                                     */
      DEVM_ReleaseLock();

      DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_CRITICAL), ("Unable to locate device\n"));
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the device   */
   /* connection asynchronous message.                                  */
   /* * NOTE * This function *MUST* be called with the Phone Book Access*/
   /*          Manager Mutex held.  This function will release the Mutex*/
   /*          before it exits (i.e. the caller SHOULD NOT RELEASE THE  */
   /*          MUTEX).                                                  */
static void ProcessPhoneBookSizeEvent(BD_ADDR_t RemoteDeviceAddress, int Status, unsigned int PhoneBookSize)
{
   PBAM_Event_Data_t      EventData;
   PBAM_Entry_Info_t     *PBAMEntryInfo;
   PBAM_Event_Callback_t  EventCallback;
   void                  *CallbackParameter;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(((PBAMEntryInfo = SearchPBAMEntryInfoEntry(&PBAMEntryInfoList, &RemoteDeviceAddress, 0)) != NULL))
   {
      /* Note the Callback information.                                 */
      EventCallback     = PBAMEntryInfo->EventCallback;
      CallbackParameter = PBAMEntryInfo->CallbackParameter;

      /* Event needs to be dispatched.  Go ahead and format the event.  */
      BTPS_MemInitialize(&EventData, 0, sizeof(PBAM_Event_Data_t));

      /* Format up the Event.                                           */
      EventData.EventType                                            = petPhoneBookSize;
      EventData.EventLength                                          = PBAM_PULL_PHONEBOOK_SIZE_EVENT_DATA_SIZE;

      EventData.EventData.PhoneBookSizeEventData.RemoteDeviceAddress = RemoteDeviceAddress;
      EventData.EventData.PhoneBookSizeEventData.Status              = Status;
      EventData.EventData.PhoneBookSizeEventData.PhoneBookSize       = PhoneBookSize;

      /* Release the Mutex so we can dispatch the event.                */
      DEVM_ReleaseLock();

      __BTPSTRY
      {
         if(EventCallback)
            (*EventCallback)(&EventData, CallbackParameter);
      }
      __BTPSEXCEPT(1)
      {
         /* Do Nothing.                                                 */
      }
   }
   else
   {
      /* Error - Release the Mutex.                                     */
      DEVM_ReleaseLock();

      DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_CRITICAL), ("Unable to locate device\n"));
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the device   */
   /* set phone book asynchronous message.                              */
   /* * NOTE * This function *MUST* be called with the Phone Book Access*/
   /*          Manager Mutex held.  This function will release the Mutex*/
   /*          before it exits (i.e. the caller SHOULD NOT RELEASE THE  */
   /*          MUTEX).                                                  */
static void ProcessPhoneBookSetEvent(BD_ADDR_t RemoteDeviceAddress, int Status, unsigned int CurrentPathSize, Byte_t *CurrentPath)
{
   PBAM_Event_Data_t     *EventData;
   PBAM_Entry_Info_t     *PBAMEntryInfo;
   PBAM_Event_Callback_t  EventCallback;
   void                  *CallbackParameter;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter Path Size: %d\n", CurrentPathSize));

   if(((PBAMEntryInfo = SearchPBAMEntryInfoEntry(&PBAMEntryInfoList, &RemoteDeviceAddress, 0)) != NULL))
   {
      /* Note the Callback information.                                 */
      EventCallback     = PBAMEntryInfo->EventCallback;
      CallbackParameter = PBAMEntryInfo->CallbackParameter;

      /* Event needs to be dispatched.  Go ahead and format the event.  */
      if((EventData = (PBAM_Event_Data_t *)BTPS_AllocateMemory(STRUCTURE_OFFSET(PBAM_Event_Data_t, EventData) + PBAM_PHONEBOOK_SET_EVENT_DATA_SIZE(CurrentPathSize))) != NULL)
      {
         BTPS_MemInitialize(EventData, 0, (STRUCTURE_OFFSET(PBAM_Event_Data_t, EventData) + PBAM_PHONEBOOK_SET_EVENT_DATA_SIZE(CurrentPathSize)));

         /* Format up the Event.                                        */
         EventData->EventType                                           = petPhoneBookSet;
         EventData->EventLength                                         = PBAM_PHONE_BOOK_SET_MESSAGE_SIZE(CurrentPathSize);

         EventData->EventData.PhoneBookSetEventData.RemoteDeviceAddress = RemoteDeviceAddress;
         EventData->EventData.PhoneBookSetEventData.Status              = Status;

         if(CurrentPathSize && CurrentPath)
         {
            EventData->EventData.PhoneBookSetEventData.CurrentPathSize = CurrentPathSize;

            BTPS_MemCopy(EventData->EventData.PhoneBookSetEventData.CurrentPath, CurrentPath, CurrentPathSize);
         }

         /* Release the Mutex so we can dispatch the event.             */
         DEVM_ReleaseLock();

         __BTPSTRY
         {
            if(EventCallback)
               (*EventCallback)(EventData, CallbackParameter);
         }
         __BTPSEXCEPT(1)
         {
            /* Do Nothing.                                              */
         }
      }
   }
   else
   {
      /* Error - Release the Mutex.                                     */
      DEVM_ReleaseLock();

      DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_CRITICAL), ("Unable to locate device\n"));
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the device   */
   /* vcard data asynchronous message.                                  */
   /* * NOTE * This function *MUST* be called with the Phone Book Access*/
   /*          Manager Mutex held.  This function will release the Mutex*/
   /*          before it exits (i.e. the caller SHOULD NOT RELEASE THE  */
   /*          MUTEX).                                                  */
static void ProcessVCardDataEvent(BD_ADDR_t RemoteDeviceAddress, int Status, Boolean_t Final, unsigned int NewMissedCalls, PBAM_VCard_Format_t Format, unsigned int BufferSize, Byte_t *Buffer)
{
   PBAM_Event_Data_t     *EventData;
   PBAM_Entry_Info_t     *PBAMEntryInfo;
   PBAM_Event_Callback_t  EventCallback;
   void                  *CallbackParameter;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(((PBAMEntryInfo = SearchPBAMEntryInfoEntry(&PBAMEntryInfoList, &RemoteDeviceAddress, 0)) != NULL))
   {
      /* Note the Callback information.                                 */
      EventCallback     = PBAMEntryInfo->EventCallback;
      CallbackParameter = PBAMEntryInfo->CallbackParameter;

      if((EventData = (PBAM_Event_Data_t *)BTPS_AllocateMemory(STRUCTURE_OFFSET(PBAM_Event_Data_t, EventData) + PBAM_VCARD_EVENT_DATA_SIZE(BufferSize))) != NULL)
      {
         /* Event needs to be dispatched. Go ahead and format the event.*/
         BTPS_MemInitialize(EventData, 0, (STRUCTURE_OFFSET(PBAM_Event_Data_t, EventData) + PBAM_VCARD_EVENT_DATA_SIZE(BufferSize)));

         /* Format up the Event.                                        */
         EventData->EventType                                    = petVCardData;
         EventData->EventLength                                  = PBAM_VCARD_EVENT_DATA_SIZE(BufferSize);

         EventData->EventData.VCardEventData.RemoteDeviceAddress = RemoteDeviceAddress;
         EventData->EventData.VCardEventData.Status              = Status;
         EventData->EventData.VCardEventData.Final               = Final;
         EventData->EventData.VCardEventData.NewMissedCalls      = NewMissedCalls;
         EventData->EventData.VCardEventData.VCardFormat         = Format;
         EventData->EventData.VCardEventData.BufferSize          = BufferSize;

         if(BufferSize)
            BTPS_MemCopy(EventData->EventData.VCardEventData.Buffer, Buffer, BufferSize);

         /* Release the Mutex so we can dispatch the event.             */
         DEVM_ReleaseLock();

         __BTPSTRY
         {
            if(EventCallback)
               (*EventCallback)(EventData, CallbackParameter);
         }
         __BTPSEXCEPT(1)
         {
            /* Do Nothing.                                              */
         }

         BTPS_FreeMemory(EventData);
      }
   }
   else
   {
      /* Error - Release the Mutex.                                     */
      DEVM_ReleaseLock();

      DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_CRITICAL), ("Unable to locate device\n"));
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the device   */
   /* vcard listing asynchronous message.                               */
   /* * NOTE * This function *MUST* be called with the Phone Book Access*/
   /*          Manager Mutex held.  This function will release the Mutex*/
   /*          before it exits (i.e. the caller SHOULD NOT RELEASE THE  */
   /*          MUTEX).                                                  */
static void ProcessVCardListingDataEvent(BD_ADDR_t RemoteDeviceAddress, int Status, Boolean_t Final, unsigned int NewMissedCalls, unsigned int BufferSize, Byte_t *Buffer)
{
   PBAM_Event_Data_t     *EventData;
   PBAM_Entry_Info_t     *PBAMEntryInfo;
   PBAM_Event_Callback_t  EventCallback;
   void                  *CallbackParameter;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(((PBAMEntryInfo = SearchPBAMEntryInfoEntry(&PBAMEntryInfoList, &RemoteDeviceAddress, 0)) != NULL))
   {
      /* Note the Callback information.                                 */
      EventCallback     = PBAMEntryInfo->EventCallback;
      CallbackParameter = PBAMEntryInfo->CallbackParameter;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Allocating\n"));
      if((EventData = (PBAM_Event_Data_t *)BTPS_AllocateMemory(STRUCTURE_OFFSET(PBAM_Event_Data_t, EventData) + PBAM_VCARD_LISTING_EVENT_DATA_SIZE(BufferSize))) != NULL)
      {
         /* Event needs to be dispatched. Go ahead and format the event.*/

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Inializing\n"));
         BTPS_MemInitialize(EventData, 0, (STRUCTURE_OFFSET(PBAM_Event_Data_t, EventData) + PBAM_VCARD_LISTING_EVENT_DATA_SIZE(BufferSize)));

         /* Format up the Event.                                        */
         EventData->EventType                                    = petVCardListing;
         EventData->EventLength                                  = PBAM_VCARD_EVENT_DATA_SIZE(BufferSize);

         EventData->EventData.VCardListingEventData.RemoteDeviceAddress = RemoteDeviceAddress;
         EventData->EventData.VCardListingEventData.Status              = Status;
         EventData->EventData.VCardListingEventData.Final               = Final;
         EventData->EventData.VCardListingEventData.NewMissedCalls      = NewMissedCalls;
         EventData->EventData.VCardListingEventData.BufferSize          = BufferSize;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Copying Buffer\n"));

         if(BufferSize)
            BTPS_MemCopy(EventData->EventData.VCardListingEventData.Buffer, Buffer, BufferSize);

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Releasing Lock\n"));
         /* Release the Mutex so we can dispatch the event.             */
         DEVM_ReleaseLock();

         __BTPSTRY
         {
            if(EventCallback)
               (*EventCallback)(EventData, CallbackParameter);
         }
         __BTPSEXCEPT(1)
         {
            /* Do Nothing.                                              */
         }

         BTPS_FreeMemory(EventData);
      }
   }
   else
   {
      /* Error - Release the Mutex.                                     */
      DEVM_ReleaseLock();

      DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_CRITICAL), ("Unable to locate device\n"));
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is called from the Main Bluetopia dispatch */
   /* thread in response to a process received message.  This function  */
   /* SHOULD ONLY be called in this capacity and NOT under any other    */
   /* circumstances.                                                    */
   /* * NOTE * This function *MUST* be called with the Phone Book Access*/
   /*          Manager Mutex held.  This function will release the Mutex*/
   /*          before it exits (i.e. the caller SHOULD NOT RELEASE THE  */
   /*          MUTEX).                                                  */
static void ProcessReceivedMessage(BTPM_Message_t *Message)
{
   Boolean_t ReleaseMutex = TRUE;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", Message->MessageHeader.MessageFunction));

   /* Check to make sure this isn't some kind of spoofed response (we do*/
   /* not process responses).                                           */
   if((Message) && (!(Message->MessageHeader.MessageID & BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK)))
   {
      /* Request received, go ahead and process it.                     */
      switch(Message->MessageHeader.MessageFunction)
      {
         case PBAM_MESSAGE_FUNCTION_DEVICE_CONNECTION_STATUS:
            DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Connection Status Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PBAM_DEVICE_CONNECTION_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* device connection status event.                       */
               ProcessDeviceConnectionStatusEvent(((PBAM_Device_Connection_Message_t *)Message)->RemoteDeviceAddress, ((PBAM_Device_Connection_Message_t *)Message)->ConnectionStatus);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_CRITICAL), ("Invalid Message Length\n"));
            }
            break;
         case PBAM_MESSAGE_FUNCTION_DEVICE_DISCONNECTED:
            DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Disconnection Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PBAM_DEVICE_DISCONNECTED_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* device disconnection event.                           */
               ProcessDeviceDisconnectionEvent(((PBAM_Device_Disconnected_Message_t *)Message)->RemoteDeviceAddress, ((PBAM_Device_Disconnected_Message_t *)Message)->DisconnectReason);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_CRITICAL), ("Invalid Message Length\n"));
            }
            break;
         case PBAM_MESSAGE_FUNCTION_VCARD_DATA:
            DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("vCard Data Message\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PBAM_VCARD_DATA_MESSAGE_SIZE(0)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PBAM_VCARD_DATA_MESSAGE_SIZE(((PBAM_VCard_Data_Message_t *)Message)->BufferSize)))
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* arbitrary command indication event.                   */
               ProcessVCardDataEvent(((PBAM_VCard_Data_Message_t *)Message)->RemoteDeviceAddress, ((PBAM_VCard_Data_Message_t *)Message)->Status, ((PBAM_VCard_Data_Message_t *)Message)->Final, ((PBAM_VCard_Data_Message_t *)Message)->NewMissedCalls, ((PBAM_VCard_Data_Message_t *)Message)->Format, ((PBAM_VCard_Data_Message_t *)Message)->BufferSize, ((PBAM_VCard_Data_Message_t *)Message)->Buffer);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_CRITICAL), ("Invalid Message Length\n"));
            }
            break;
         case PBAM_MESSAGE_FUNCTION_VCARD_LISTING_DATA:
            DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("vCard ListingData Message\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PBAM_VCARD_LISTING_DATA_MESSAGE_SIZE(0)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PBAM_VCARD_LISTING_DATA_MESSAGE_SIZE(((PBAM_VCard_Listing_Data_Message_t *)Message)->BufferSize)))
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* arbitrary command indication event.                   */
               ProcessVCardListingDataEvent(((PBAM_VCard_Listing_Data_Message_t *)Message)->RemoteDeviceAddress, ((PBAM_VCard_Listing_Data_Message_t *)Message)->Status, ((PBAM_VCard_Listing_Data_Message_t *)Message)->Final, ((PBAM_VCard_Listing_Data_Message_t *)Message)->NewMissedCalls, ((PBAM_VCard_Listing_Data_Message_t *)Message)->BufferSize, ((PBAM_VCard_Listing_Data_Message_t *)Message)->Buffer);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_CRITICAL), ("Invalid Message Length\n"));
            }
            break;
         case PBAM_MESSAGE_FUNCTION_PHONE_BOOK_SIZE:
            DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Phone Book Size Message\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PBAM_PHONE_BOOK_SIZE_MESSAGE_SIZE))
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* arbitrary command indication event.                   */
               ProcessPhoneBookSizeEvent(((PBAM_Phone_Book_Size_Message_t *)Message)->RemoteDeviceAddress, ((PBAM_Phone_Book_Size_Message_t *)Message)->Status, ((PBAM_Phone_Book_Size_Message_t *)Message)->PhoneBookSize);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_CRITICAL), ("Invalid Message Length\n"));
            }
            break;
         case PBAM_MESSAGE_FUNCTION_PHONE_BOOK_SET:
            DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("phone book set message\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PBAM_PHONE_BOOK_SET_MESSAGE_SIZE(0)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PBAM_PHONE_BOOK_SET_MESSAGE_SIZE(((PBAM_Phone_Book_Set_Message_t *)Message)->CurrentPathSize)))
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* arbitrary command indication event.                   */
               ProcessPhoneBookSetEvent(((PBAM_Phone_Book_Set_Message_t *)Message)->RemoteDeviceAddress, ((PBAM_Phone_Book_Set_Message_t *)Message)->Status, ((PBAM_Phone_Book_Set_Message_t *)Message)->CurrentPathSize, ((PBAM_Phone_Book_Set_Message_t *)Message)->CurrentPath);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_CRITICAL), ("Invalid Message Length\n"));
            }
            break;
         default:
            DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_CRITICAL), ("Unhandled Message %d\n", Message->MessageHeader.MessageFunction));
            break;
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_CRITICAL), ("Invalid or Response Message Received\n"));
   }

   /* If we need to Release the Mutex then we will do it at this time.  */
   if(ReleaseMutex)
      DEVM_ReleaseLock();

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is used to process a connection request    */
   /* (from a BluetopiaPM client). This function takes a valid IPC      */
   /* connect request structure.                                        */
   /* * NOTE * This function *MUST* be called with the Phone Book Access*/
   /*          Manager Mutex held.  This function will release the Mutex*/
   /*          before it exits (i.e. the caller SHOULD NOT RELEASE THE  */
   /*          MUTEX).                                                  */
static void ProcessConnectRemoteDevice(PBAM_Connect_Remote_Device_Request_t *Message)
{
   int                                    Result;
   PBAM_Entry_Info_t                      PBAMEntryInfo;
   PBAM_Entry_Info_t                     *PBAMEntryInfoPtr;
   PBAM_Connect_Remote_Device_Response_t  ResponseMessage;

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Next, verify that the input parameters appear to be semi-valid.*/
      if((!COMPARE_NULL_BD_ADDR(Message->RemoteDeviceAddress)) && (Message->RemoteServerPort))
      {
         /* Next, check to see if we are powered up.                    */
         if(CurrentPowerState)
         {
            /* Device is powered on, attempt to add an entry into the   */
            /* Phone Book Access entry list.                            */
            BTPS_MemInitialize(&PBAMEntryInfo, 0, sizeof(PBAM_Entry_Info_t));

            PBAMEntryInfo.ClientID         = Message->MessageHeader.AddressID;
            PBAMEntryInfo.BluetoothAddress = Message->RemoteDeviceAddress;
            PBAMEntryInfo.RemoteServerPort = Message->RemoteServerPort;
            PBAMEntryInfo.ConnectionState  = csIdle;

            if((PBAMEntryInfoPtr = AddPBAMEntryInfoEntry(&PBAMEntryInfoList, &PBAMEntryInfo)) != NULL)
            {
               DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to open Remote device %d 0x%08lX\n", Message->RemoteServerPort, Message->ConnectionFlags));

               /* Next, attempt to open the remote device.              */
               if(Message->ConnectionFlags & PBAM_CONNECT_REMOTE_DEVICE_FLAGS_REQUIRE_ENCRYPTION)
                  PBAMEntryInfoPtr->ConnectionState = csEncrypting;
               else
               {
                  if(Message->ConnectionFlags & PBAM_CONNECT_REMOTE_DEVICE_FLAGS_REQUIRE_AUTHENTICATION)
                     PBAMEntryInfoPtr->ConnectionState = csAuthenticating;
                  else
                     PBAMEntryInfoPtr->ConnectionState = csIdle;
               }

               Result = DEVM_ConnectWithRemoteDevice(Message->RemoteDeviceAddress, (PBAMEntryInfoPtr->ConnectionState == csIdle)?0:((PBAMEntryInfoPtr->ConnectionState == csEncrypting)?(DEVM_CONNECT_WITH_REMOTE_DEVICE_FLAGS_AUTHENTICATE | DEVM_CONNECT_WITH_REMOTE_DEVICE_FLAGS_ENCRYPT):DEVM_CONNECT_WITH_REMOTE_DEVICE_FLAGS_AUTHENTICATE));

               if((Result >= 0) || (Result == BTPM_ERROR_CODE_DEVICE_IS_CURRENTLY_CONNECTED) || (Result == BTPM_ERROR_CODE_DEVICE_ALREADY_AUTHENTICATED) || (Result == BTPM_ERROR_CODE_DEVICE_ALREADY_ENCRYPTED))
               {
                  /* Check to see if we need to actually issue the      */
                  /* Remote connection.                                 */
                  if(Result < 0)
                  {
                     /* Set the state to connecting remote device.      */
                     PBAMEntryInfoPtr->ConnectionState = csConnecting;

                     /* Next, attempt to open the remote device.        */
                     if((Result = _PBAM_Connect_Remote_Device(Message->RemoteDeviceAddress, Message->RemoteServerPort)) <= 0)
                     {
                        Result = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_UNABLE_TO_CONNECT_TO_DEVICE;

                        /* Error opening device, go ahead and delete the*/
                        /* entry that was added.                        */
                        if((PBAMEntryInfoPtr = DeletePBAMEntryInfoEntry(&PBAMEntryInfoList, &(PBAMEntryInfoPtr->BluetoothAddress), 0)) != NULL)
                           FreePBAMEntryInfoEntryMemory(PBAMEntryInfoPtr);
                     }
                     else
                     {
                        /* Note the connection ID.                      */
                        PBAMEntryInfoPtr->PBAPID = Result;

                        /* Flag success.                                */
                        Result                   = 0;

                        DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Connection open %d\n", PBAMEntryInfoPtr->PBAPID));
                     }
                  }
               }
               else
               {
                  /* If we are not tracking this connection OR there was*/
                  /* an error, go ahead and delete the entry that was   */
                  /* added.                                             */
                  if((PBAMEntryInfoPtr = DeletePBAMEntryInfoEntry(&PBAMEntryInfoList, &(PBAMEntryInfoPtr->BluetoothAddress), 0)) != NULL)
                     FreePBAMEntryInfoEntryMemory(PBAMEntryInfoPtr);
               }
            }
            else
               Result = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
         }
         else
            Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_PARAMETER;

      ResponseMessage.MessageHeader                = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength  = PBAM_CONNECT_REMOTE_DEVICE_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;
      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }
   else
      Result = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_NOT_INITIALIZED;


   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit %d\n", Result));
}

   /* The following function is used to process a disconnect request    */
   /* (from a BluetopiaPM client). This function takes a valid IPC      */
   /* disconnect request structure.                                     */
static void ProcessDisconnectDeviceRequest(PBAM_Disconnect_Device_Request_t *Message)
{
   int                                Result;
   PBAM_Entry_Info_t                 *ConnectionEntryPtr;
   PBAM_Disconnect_Device_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      Result = _Disconnect_Device(Message->RemoteDeviceAddress, Message->MessageHeader.AddressID);

      if((!Result) && ((ConnectionEntryPtr = DeletePBAMEntryInfoEntry(&PBAMEntryInfoList, &(Message->RemoteDeviceAddress), 0)) != NULL))
         FreePBAMEntryInfoEntryMemory(ConnectionEntryPtr);

      ResponseMessage.MessageHeader                = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength  = PBAM_DISCONNECT_DEVICE_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;
      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is used to process an abort request (from a*/
   /* BluetopiaPM client). This function takes a valid IPC abort request*/
   /* structure.                                                        */
static void ProcessAbortRequest(PBAM_Abort_Request_t *Message)
{
   int                   Result;
   PBAM_Abort_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      Result                                       = _Abort(Message->RemoteDeviceAddress, Message->MessageHeader.AddressID);

      ResponseMessage.MessageHeader                = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength  = PBAM_ABORT_REQUEST_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;
      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is used to process a pull phone book       */
   /* request (from a BluetopiaPM client). This function takes a valid  */
   /* IPC pull phone book request structure.                            */
static void ProcessPullPhoneBookRequest(PBAM_Pull_Phone_Book_Request_t *Message)
{
   int                             Result;
   PBAM_Pull_Phone_Book_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      Result                                       = _Pull_Phone_Book(Message->RemoteDeviceAddress, Message->PhoneBookNamePath, Message->FilterLow, Message->FilterHigh, Message->VCardFormat, Message->MaxListCount, Message->ListStartOffset, Message->MessageHeader.AddressID);

      ResponseMessage.MessageHeader                = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength  = PBAM_PULL_PHONE_BOOK_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;
      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is used to process a pull phone book size  */
   /* request (from a BluetopiaPM client). This function takes a valid  */
   /* IPC pull phone book size request structure.                       */
static void ProcessPullPhoneBookSizeRequest(PBAM_Pull_Phone_Book_Size_Request_t *Message)
{
   int                                  Result;
   PBAM_Pull_Phone_Book_Size_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      Result                                       = _Pull_Phone_Book_Size(Message->RemoteDeviceAddress, Message->MessageHeader.AddressID);

      ResponseMessage.MessageHeader                = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength  = PBAM_PULL_PHONE_BOOK_SIZE_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;
      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is used to process a set phone book request*/
   /* (from a BluetopiaPM client). This function takes a valid IPC set  */
   /* hone book request structure.                                      */
static void ProcessSetPhoneBookRequest(PBAM_Set_Phone_Book_Request_t *Message)
{
   int                            Result;
   PBAM_Set_Phone_Book_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      Result                                       = _Set_Phone_Book(Message->RemoteDeviceAddress, Message->PathOption, Message->FolderName, Message->MessageHeader.AddressID);

      ResponseMessage.MessageHeader                = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength  = PBAM_SET_PHONE_BOOK_SIZE_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;
      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is used to process a pull vCard listing    */
   /* request (from a BluetopiaPM client). This function takes a valid  */
   /* IPC pull vCard listing request structure.                         */
static void ProcessPullvCardListingRequest(PBAM_Pull_vCard_Listing_Request_t *Message)
{
   int                                Result;
   char                               PhonebookPath[64];
   char                               SearchValue[64];
   PBAM_Pull_vCard_Listing_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      if(Message->PhonebookPathSize)
         BTPS_MemCopy(PhonebookPath, Message->VariableData, Message->PhonebookPathSize);

      if(Message->SearchValueSize)
         BTPS_MemCopy(SearchValue, Message->VariableData + Message->PhonebookPathSize, Message->SearchValueSize);

      Result                                       = _Pull_vCard_Listing(Message->RemoteDeviceAddress, (Message->PhonebookPathSize)?PhonebookPath:NULL, Message->ListOrder, Message->SearchAttribute, (Message->SearchValueSize)?SearchValue:NULL, Message->MaxListCount, Message->ListStartOffset, Message->MessageHeader.AddressID);

      ResponseMessage.MessageHeader                = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength  = PBAM_PULL_VCARD_LISTING_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;
      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is used to process a pull vCard request    */
   /* (from a BluetopiaPM client). This function takes a valid IPC pull */
   /* vCard request structure.                                          */
static void ProcessPullvCardRequest(PBAM_Pull_vCard_Request_t *Message)
{
   int                        Result;
   PBAM_Pull_vCard_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      Result                                       = _Pull_vCard(Message->RemoteDeviceAddress, (Message->VCardNameSize)?Message->VCardName:NULL, Message->FilterLow, Message->FilterHigh, Message->VCardFormat, Message->MessageHeader.AddressID);

      ResponseMessage.MessageHeader                = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength  = PBAM_PULL_VCARD_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;
      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is used to process a set phone book        */
   /* absolute request (from a BluetopiaPM client). This function takes */
   /* a valid IPC set phone book absolute request structure.            */
static void ProcessSetPhoneBookAbsoluteRequest(PBAM_Set_Phone_Book_Absolute_Request_t *Message)
{
   int                                     Result;
   PBAM_Set_Phone_Book_Absolute_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      Result                                       = _Set_Phone_Book_Absolute(Message->RemoteDeviceAddress, Message->Path, Message->MessageHeader.AddressID);

      ResponseMessage.MessageHeader                = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength  = PBAM_SET_PHONE_BOOK_ABSOLUTE_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;
      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is used to process a register server       */
   /* request (from a BluetopiaPM client). This function takes a valid  */
   /* IPC register server structure.                                    */
static void ProcessRegisterServerMessage(PBAM_Register_Server_Request_t *Message)
{
   int                             Result;
   PBAM_Register_Server_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Check if we are powered up.                                    */
      if(CurrentPowerState)
      {
         Result = ProcessRegisterServer(Message->MessageHeader.AddressID, Message->ServerPort, Message->SupportedRepositories, Message->IncomingConnectionFlags, (char *)Message->ServiceName, NULL, NULL);
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength  = PBAM_REGISTER_SERVER_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;
      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is used to process a un-register server    */
   /* request (from a BluetopiaPM client). This function takes a valid  */
   /* IPC un-register server structure.                                 */
static void ProcessUnRegisterServerMessage(PBAM_Un_Register_Server_Request_t *Message)
{
   int                                Result;
   PBAM_Un_Register_Server_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Check if we are powered up.                                    */
      if(CurrentPowerState)
      {
         Result = ProcessUnRegisterServer(Message->MessageHeader.AddressID, Message->ServerID);
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength  = PBAM_UN_REGISTER_SERVER_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;
      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is used to process a connection request    */
   /* response request (from a BluetopiaPM client). This function takes */
   /* a valid IPC connection request response. structure.               */
static void ProcessConnectionRequestResponseMessage(PBAM_Connection_Request_Response_Request_t *Message)
{
   int                                         Result;
   PBAM_Connection_Request_Response_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Check if we are powered up.                                    */
      if(CurrentPowerState)
      {
         Result = ProcessConnectionRequestResponse(Message->MessageHeader.AddressID, Message->ConnectionID, Message->Accept);
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength  = PBAM_CONNECTION_REQUEST_RESPONSE_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;
      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is used to process a close server          */
   /* connection request (from a BluetopiaPM client). This function     */
   /* takes a valid IPC close server connection structure.              */
static void ProcessCloseServerConnectionMessage(PBAM_Close_Server_Connection_Request_t *Message)
{
   int                                     Result;
   PBAM_Close_Server_Connection_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Check if we are powered up.                                    */
      if(CurrentPowerState)
      {
         Result = ProcessCloseServerConnection(Message->MessageHeader.AddressID, Message->ConnectionID);
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength  = PBAM_CLOSE_SERVER_CONNECTION_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;
      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is used to process a send phone book       */
   /* request (from a BluetopiaPM client). This function takes a valid  */
   /* IPC send phonebook structure.                                     */
static void ProcessSendPhoneBookMessage(PBAM_Send_Phone_Book_Request_t *Message)
{
   int                             Result;
   PBAM_Send_Phone_Book_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Check if we are powered up.                                    */
      if(CurrentPowerState)
      {
         Result = ProcessSendPhoneBook(Message->MessageHeader.AddressID, Message->ConnectionID, Message->ResponseStatusCode, Message->IncludeMissedCalls?&(Message->NewMissedCalls):NULL, Message->BufferSize, Message->Buffer, Message->Final);
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength  = PBAM_SEND_PHONE_BOOK_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;
      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is used to process a send phone book size  */
   /* request (from a BluetopiaPM client). This function takes a valid  */
   /* IPC send phone book size structure.                               */
static void ProcessSendPhoneBookSizeMessage(PBAM_Send_Phone_Book_Size_Request_t *Message)
{
   int                                  Result;
   PBAM_Send_Phone_Book_Size_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Check if we are powered up.                                    */
      if(CurrentPowerState)
      {
         Result = ProcessSendPhoneBookSize(Message->MessageHeader.AddressID, Message->ConnectionID, Message->ResponseStatusCode, Message->PhoneBookSize);
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength  = PBAM_SEND_PHONE_BOOK_SIZE_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;
      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is used to process a set phone book        */
   /* response request (from a BluetopiaPM client). This function takes */
   /* a valid IPC set phone book response structure.                    */
static void ProcessSetPhoneBookResponseMessage(PBAM_Set_Phone_Book_Response_Request_t *Message)
{
   int                                     Result;
   PBAM_Set_Phone_Book_Response_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Check if we are powered up.                                    */
      if(CurrentPowerState)
      {
         Result = ProcessSetPhoneBookResponse(Message->MessageHeader.AddressID, Message->ConnectionID, Message->ResponseStatusCode);
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength  = PBAM_SET_PHONE_BOOK_RESPONSE_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;
      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is used to process a send vCard listing    */
   /* request (from a BluetopiaPM client). This function takes a valid  */
   /* IPC send vCard listing structure.                                 */
static void ProcessSendvCardListingMessage(PBAM_Send_vCard_Listing_Request_t *Message)
{
   int                                Result;
   PBAM_Send_vCard_Listing_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Check if we are powered up.                                    */
      if(CurrentPowerState)
      {
         Result = ProcessSendvCardListing(Message->MessageHeader.AddressID, Message->ConnectionID, Message->ResponseStatusCode, Message->IncludeMissedCalls?&(Message->NewMissedCalls):NULL, Message->BufferSize, Message->Buffer, Message->Final);
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength  = PBAM_SEND_VCARD_LISTING_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;
      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is used to process a send vCard listing    */
   /* size request (from a BluetopiaPM client). This function takes a   */
   /* valid IPC send vCard listing size structure.                      */
static void ProcessSendvCardListingSizeMessage(PBAM_Send_vCard_Listing_Size_Request_t *Message)
{
   int                                     Result;
   PBAM_Send_vCard_Listing_Size_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Check if we are powered up.                                    */
      if(CurrentPowerState)
      {
         Result = ProcessSendvCardListingSize(Message->MessageHeader.AddressID, Message->ConnectionID, Message->ResponseStatusCode, Message->vCardListingSize);
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength  = PBAM_SEND_VCARD_LISTING_SIZE_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;
      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is used to process a send vCard request    */
   /* (from a BluetopiaPM client). This function takes a valid IPC send */
   /* vCard structure.                                                  */
static void ProcessSendvCardMessage(PBAM_Send_vCard_Request_t *Message)
{
   int                        Result;
   PBAM_Send_vCard_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Check if we are powered up.                                    */
      if(CurrentPowerState)
      {
         Result = ProcessSendvCard(Message->MessageHeader.AddressID, Message->ConnectionID, Message->ResponseStatusCode, Message->BufferSize, Message->Buffer, Message->Final);
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength  = PBAM_SEND_VCARD_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;
      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is used to process all BluetopiaPM IPC     */
   /* request messages. The function accepts a valid BTPM_Message_t.    */
   /* * NOTE * This function *MUST* be called with the Phone Book Access*/
   /*          Manager Mutex held.  This function will release the Mutex*/
   /*          before it exits (i.e. the caller SHOULD NOT RELEASE THE  */
   /*          MUTEX).                                                  */
static void ProcessReceivedRequestMessage(BTPM_Message_t *Message)
{
   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", Message->MessageHeader.MessageFunction));

   /* Check to make sure this isn't some kind of spoofed response (we do*/
   /* not process responses).                                           */
   if((Message) && (!(Message->MessageHeader.MessageID & BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK)))
   {
      /* Request received, go ahead and process it.                     */
      switch(Message->MessageHeader.MessageFunction)
      {
         case PBAM_MESSAGE_FUNCTION_CONNECT_REMOTE_DEVICE:
            DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Connect Remote Device\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PBAM_CONNECT_REMOTE_DEVICE_REQUEST_SIZE)
               ProcessConnectRemoteDevice((PBAM_Connect_Remote_Device_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_CRITICAL), ("Invalid Message Length\n"));
            }
            break;
         case PBAM_MESSAGE_FUNCTION_DISCONNECT_DEVICE:
            DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Disconnect Device\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PBAM_DISCONNECT_DEVICE_REQUEST_SIZE)
               ProcessDisconnectDeviceRequest((PBAM_Disconnect_Device_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_CRITICAL), ("Invalid Message Length\n"));
            }
            break;
         case PBAM_MESSAGE_FUNCTION_ABORT:
            DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Abort\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PBAM_ABORT_REQUEST_SIZE)
               ProcessAbortRequest((PBAM_Abort_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_CRITICAL), ("Invalid Message Length\n"));
            }
            break;
         case PBAM_MESSAGE_FUNCTION_PULL_PHONE_BOOK:
            DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Pull Phone Book\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PBAM_PULL_PHONE_BOOK_REQUEST_SIZE(0)) && (BTPM_MESSAGE_SIZE(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PBAM_PULL_PHONE_BOOK_REQUEST_SIZE(((PBAM_Pull_Phone_Book_Request_t *)Message)->PhoneBookNamePathSize))))
               ProcessPullPhoneBookRequest((PBAM_Pull_Phone_Book_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_CRITICAL), ("Invalid Message Length\n"));
            }
            break;
         case PBAM_MESSAGE_FUNCTION_PULL_PHONE_BOOK_SIZE:
            DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Pull phone book size\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PBAM_PULL_PHONE_BOOK_SIZE_REQUEST_SIZE)
               ProcessPullPhoneBookSizeRequest((PBAM_Pull_Phone_Book_Size_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_CRITICAL), ("Invalid Message Length\n"));
            }
            break;
         case PBAM_MESSAGE_FUNCTION_SET_PHONE_BOOK:
            DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Pull phone book size\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PBAM_SET_PHONE_BOOK_REQUEST_SIZE(0)) && (BTPM_MESSAGE_SIZE(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PBAM_SET_PHONE_BOOK_REQUEST_SIZE(((PBAM_Set_Phone_Book_Request_t *)Message)->FolderNameSize))))
               ProcessSetPhoneBookRequest((PBAM_Set_Phone_Book_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_CRITICAL), ("Invalid Message Length\n"));
            }
            break;
         case PBAM_MESSAGE_FUNCTION_PULL_VCARD_LISTING:
            DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Pull vCard Listing\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PBAM_PULL_VCARD_LISTING_REQUEST_SIZE(0,0)) && (BTPM_MESSAGE_SIZE(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PBAM_PULL_VCARD_LISTING_REQUEST_SIZE(((PBAM_Pull_vCard_Listing_Request_t *)Message)->PhonebookPathSize,((PBAM_Pull_vCard_Listing_Request_t *)Message)->SearchValueSize))))
               ProcessPullvCardListingRequest((PBAM_Pull_vCard_Listing_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_CRITICAL), ("Invalid Message Length\n"));
            }
            break;
         case PBAM_MESSAGE_FUNCTION_PULL_VCARD:
            DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Pull vCard\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PBAM_PULL_VCARD_REQUEST_SIZE(0)) && (BTPM_MESSAGE_SIZE(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PBAM_PULL_VCARD_REQUEST_SIZE(((PBAM_Pull_vCard_Request_t *)Message)->VCardNameSize))))
               ProcessPullvCardRequest((PBAM_Pull_vCard_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_CRITICAL), ("Invalid Message Length\n"));
            }
            break;
         case PBAM_MESSAGE_FUNCTION_SET_PHONE_BOOK_ABSOLUTE:
            DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Set Phone Book Absolute\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PBAM_SET_PHONE_BOOK_ABSOLUTE_REQUEST_SIZE(0)) && (BTPM_MESSAGE_SIZE(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PBAM_SET_PHONE_BOOK_ABSOLUTE_REQUEST_SIZE(((PBAM_Set_Phone_Book_Absolute_Request_t *)Message)->PathSize))))
               ProcessSetPhoneBookAbsoluteRequest((PBAM_Set_Phone_Book_Absolute_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_CRITICAL), ("Invalid Message Length\n"));
            }
            break;
         case PBAM_MESSAGE_FUNCTION_REGISTER_SERVER:
            DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Register Server\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PBAM_REGISTER_SERVER_REQUEST_SIZE(0)) && (BTPM_MESSAGE_SIZE(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PBAM_REGISTER_SERVER_REQUEST_SIZE(((PBAM_Register_Server_Request_t *)Message)->ServiceNameLength))))
               ProcessRegisterServerMessage((PBAM_Register_Server_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_CRITICAL), ("Invalid Message Length\n"));
            }
            break;
         case PBAM_MESSAGE_FUNCTION_UN_REGISTER_SERVER:
            DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Un Register Server\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PBAM_UN_REGISTER_SERVER_REQUEST_SIZE)
               ProcessUnRegisterServerMessage((PBAM_Un_Register_Server_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_CRITICAL), ("Invalid Message Length\n"));
            }
            break;
         case PBAM_MESSAGE_FUNCTION_CONNECTION_REQUEST_RESPONSE:
            DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Connection Request Response\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PBAM_CONNECTION_REQUEST_RESPONSE_REQUEST_SIZE)
               ProcessConnectionRequestResponseMessage((PBAM_Connection_Request_Response_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_CRITICAL), ("Invalid Message Length\n"));
            }
            break;
         case PBAM_MESSAGE_FUNCTION_CLOSE_SERVER_CONNECTION:
            DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Close Server Connection\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PBAM_CLOSE_SERVER_CONNECTION_REQUEST_SIZE)
               ProcessCloseServerConnectionMessage((PBAM_Close_Server_Connection_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_CRITICAL), ("Invalid Message Length\n"));
            }
            break;
         case PBAM_MESSAGE_FUNCTION_SEND_PHONE_BOOK:
            DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Send Phone Book\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PBAM_SEND_PHONE_BOOK_REQUEST_SIZE(0)) && (BTPM_MESSAGE_SIZE(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PBAM_SEND_PHONE_BOOK_REQUEST_SIZE(((PBAM_Send_Phone_Book_Request_t *)Message)->BufferSize))))
               ProcessSendPhoneBookMessage((PBAM_Send_Phone_Book_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_CRITICAL), ("Invalid Message Length\n"));
            }
            break;
         case PBAM_MESSAGE_FUNCTION_SEND_PHONE_BOOK_SIZE:
            DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Send Phone Book Size\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PBAM_SEND_PHONE_BOOK_SIZE_REQUEST_SIZE)
               ProcessSendPhoneBookSizeMessage((PBAM_Send_Phone_Book_Size_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_CRITICAL), ("Invalid Message Length\n"));
            }
            break;
         case PBAM_MESSAGE_FUNCTION_SET_PHONE_BOOK_RESPONSE:
            DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Set Phone Book Response\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PBAM_SET_PHONE_BOOK_RESPONSE_REQUEST_SIZE)
               ProcessSetPhoneBookResponseMessage((PBAM_Set_Phone_Book_Response_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_CRITICAL), ("Invalid Message Length\n"));
            }
            break;
         case PBAM_MESSAGE_FUNCTION_SEND_VCARD_LISTING:
            DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Send vCard Listing\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PBAM_SEND_VCARD_LISTING_REQUEST_SIZE(0)) && (BTPM_MESSAGE_SIZE(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PBAM_SEND_VCARD_LISTING_REQUEST_SIZE(((PBAM_Send_vCard_Listing_Request_t *)Message)->BufferSize))))
               ProcessSendvCardListingMessage((PBAM_Send_vCard_Listing_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_CRITICAL), ("Invalid Message Length\n"));
            }
            break;
         case PBAM_MESSAGE_FUNCTION_SEND_VCARD_LISTING_SIZE:
            DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Send vCard Listing Size\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PBAM_SEND_VCARD_LISTING_SIZE_REQUEST_SIZE)
               ProcessSendvCardListingSizeMessage((PBAM_Send_vCard_Listing_Size_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_CRITICAL), ("Invalid Message Length\n"));
            }
            break;
         case PBAM_MESSAGE_FUNCTION_SEND_VCARD:
            DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Send vCard\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PBAM_SEND_VCARD_REQUEST_SIZE(0)) && (BTPM_MESSAGE_SIZE(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PBAM_SEND_VCARD_REQUEST_SIZE(((PBAM_Send_vCard_Request_t *)Message)->BufferSize))))
               ProcessSendvCardMessage((PBAM_Send_vCard_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_CRITICAL), ("Invalid Message Length\n"));
            }
            break;
         default:
            DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_CRITICAL), ("Unhandled Message\n"));
            break;
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_CRITICAL), ("Invalid or Response Message Received\n"));
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that     */
   /* is registered to process Phone Book Access Manager asynchronous   */
   /* events.                                                           */
static void BTPSAPI BTPMDispatchCallback_PBAM(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's determine if the specified parameter appears to be          */
   /* semi-valid.                                                       */
   if(CallbackParameter)
   {
      /* Next, let's check to see if we need to process it.             */
      if(Initialized)
      {
         /* Attempt to wait for access to the Phone Book Access state   */
         /* information.                                                */
         if(DEVM_AcquireLock())
         {
            /* Process the Message.                                     */
            ProcessReceivedRequestMessage((BTPM_Message_t *)CallbackParameter);

            /* Release the lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
      }

      /* All finished with the Message, so go ahead and free it.        */
      MSG_FreeReceivedMessageGroupHandlerMessage((BTPM_Message_t *)CallbackParameter);
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the dispatch callback function that is  */
   /* registered to process client un-register asynchronous events.     */
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter)
{
   int                  Result;
   PBAM_Entry_Info_t   *FoundEntry     = NULL;
   PBAM_Entry_Info_t   *NextEntry      = NULL;
   PBAM_Server_Entry_t *tmpServerEntry;
   PBAM_Server_Entry_t *ServerEntry;
   unsigned int         ClientID       = (unsigned int)CallbackParameter;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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
            if((ClientID) && (ClientID != MSG_GetServerAddressID()))
            {
               DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Client %d registration\n", ClientID));

               /* Search for existing connections owned by this Client. */
               FoundEntry = PBAMEntryInfoList;

               while(FoundEntry)
               {
                  if(ClientID == FoundEntry->ClientID)
                  {
                     DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Client %d matched\n", ClientID));

                     /* Note the next entry in the list because         */
                     /* the current entry will be removed upon          */
                     /* disconnection.                                  */
                     NextEntry = FoundEntry->NextPBAMEntryInfoPtr;

                     /* Disconnect the client connection.               */
                     if((Result = _Disconnect_Device(FoundEntry->BluetoothAddress, FoundEntry->ClientID)) != 0)
                        DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_WARNING), ("Unable to disconnnect device %02X:%02X:%02X:%02X:%02X:%02X (Result: %d)\n", FoundEntry->BluetoothAddress.BD_ADDR5, FoundEntry->BluetoothAddress.BD_ADDR4, FoundEntry->BluetoothAddress.BD_ADDR3, FoundEntry->BluetoothAddress.BD_ADDR2, FoundEntry->BluetoothAddress.BD_ADDR1, FoundEntry->BluetoothAddress.BD_ADDR0, Result));

                     FoundEntry = NextEntry;
                  }
                  else
                     FoundEntry = FoundEntry->NextPBAMEntryInfoPtr;
               }

               /* Search the server list for any entries by the client. */
               ServerEntry = PBAMServerEntryList;

               while(ServerEntry)
               {
                  /* Check whether this entry belongs to the client.    */
                  if(ServerEntry->ClientID == ClientID)
                  {
                     /* Note the next Entry in the list, since we are   */
                     /* about to delete the current entry.              */
                     tmpServerEntry = ServerEntry->NextServerEntryPtr;

                     if((ServerEntry = DeletePBAMServerEntry(&PBAMServerEntryList, ServerEntry->PBAPID)) != NULL)
                     {
                        /* Un-register the server's SDP record.         */
                        _PBAM_Un_Register_Service_Record(ServerEntry->PBAPID, ServerEntry->ServiceRecordHandle);

                        /* Close the server port.                       */
                        _PBAM_Close_Server(ServerEntry->PBAPID);

                        /* Free the entry.                              */
                        FreePBAMServerEntryMemory(ServerEntry);
                     }

                     /* Move on to the next entry.                      */
                     ServerEntry = tmpServerEntry;
                  }
                  else
                     ServerEntry = ServerEntry->NextServerEntryPtr;
               }

            }

            /* Release the lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is used to dispatch PM events. The function*/
   /* accepts a valid BTPM_Message_t to dispatch.                       */
   /* * NOTE      This function should be called with the Phone Book    */
   /*             Access Manager Mutex held. Upon exit from this        */
   /*             function it will free the Phone Book Access Manager   */
   /*             Mutex.                                                */
static void DispatchPBAMEvent(PBAM_Entry_Info_t *EntryInfo, BTPM_Message_t *Message)
{
   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure that the input parameters appear to be semi-valid.      */
   if((EntryInfo) && (Message))
   {
      if(EntryInfo->Flags & PBAM_ENTRY_INFO_FLAGS_LOCALLY_HANDLED)
      {
         /* Local Entry                                                 */

         /* This function expects the lock to be held                   */
         ProcessReceivedMessage(Message);
      }
      else
      {
         /* Remote Entry                                                */

         /* Remote Entry, just send to IPC.                             */
         MSG_SendMessage(Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch the specified Phonebook Access event to the specified    */
   /* client.                                                           */
   /* * NOTE * This function should be called with the Phonebook Access */
   /*          Manager Lock held.  Upon exit from this function it will */
   /*          free the Phonebook Access Lock.                          */
static void DispatchPBAMServerEvent(PBAM_Server_Entry_t *EntryInfo, PBAM_Event_Data_t *EventData, BTPM_Message_t *Message)
{
   void                  *CallbackParameter;
   unsigned int           ClientID;
   unsigned int           ServerID;
   PBAM_Event_Callback_t  CallbackFunction;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify the the parameters are semi-valid.                         */
   if((EntryInfo) && (EventData) && (Message))
   {
      /* Note the callback information.                                 */
      ServerID          = MSG_GetServerAddressID();
      ClientID          = EntryInfo->ClientID;
      CallbackFunction  = EntryInfo->CallbackFunction;
      CallbackParameter = EntryInfo->CallbackParameter;

      /* Release the lock to make the callback.                         */
      DEVM_ReleaseLock();

      /* Check whether this is a local callback.                        */
      if(ClientID == ServerID)
      {
         /* Callback is local. Go ahead and make the callback.          */
         __BTPSTRY
         {
            if(CallbackFunction)
            {
               (*CallbackFunction)(EventData, CallbackParameter);
            }
         }
         __BTPSEXCEPT(1)
         {
            /* Do nothing.                                              */
         }
      }
      else
      {
         /* Callback is a remote client.                                */

         /* Sent the ClientID of the Event Message.                     */
         Message->MessageHeader.AddressID = ClientID;

         /* Send the message to the appropriate client.                 */
         MSG_SendMessage(Message);
      }
   }
   else
      DEVM_ReleaseLock();

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is used to process Bluetopia PBAP callback */
   /* open port event. The function accepts a valid open port structure.*/
static void ProcessOpenConfirmationEvent(PBAP_Open_Port_Confirmation_Data_t *OpenConfirmationData)
{
   PBAM_Entry_Info_t                *ConnectionEntryPtr;
   PBAM_Device_Connection_Message_t  Message;
   Boolean_t                         FreeEntry = FALSE;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(OpenConfirmationData)
   {
      if((ConnectionEntryPtr = SearchPBAMEntryInfoEntry(&PBAMEntryInfoList, &(OpenConfirmationData->BD_ADDR), 0)) != NULL)
      {
         if(OpenConfirmationData->PBAPConnectStatus)
         {
            /* For the error case, we need to handle the PM             */
            /* client/server differently. If client is local, hold on to*/
            /* the context, and let the                                 */
            /* ProcessDeviceConnectionStatusEvent() send the API        */
            /* callback and free the context. Otherwise, we are done    */
            /* with context so go ahead a remove.                       */
            if(!(ConnectionEntryPtr->Flags & PBAM_ENTRY_INFO_FLAGS_LOCALLY_HANDLED))
            {
               if((ConnectionEntryPtr = DeletePBAMEntryInfoEntry(&PBAMEntryInfoList, &(OpenConfirmationData->BD_ADDR), 0)) != NULL)
                  FreeEntry = TRUE;
            }
         }

         /* Format up the Message to dispatch.                          */
         BTPS_MemInitialize(&Message, 0, sizeof(Message));

         Message.MessageHeader.AddressID       = 0;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PHONE_BOOK_ACCESS_MANAGER;
         Message.MessageHeader.MessageFunction = PBAM_MESSAGE_FUNCTION_DEVICE_CONNECTION_STATUS;
         Message.MessageHeader.MessageLength   = (PBAM_DEVICE_CONNECTION_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.RemoteDeviceAddress           = OpenConfirmationData->BD_ADDR;
         Message.ConnectionStatus              = OpenConfirmationData->PBAPConnectStatus;

         /* Finally dispatch the formatted Event and Message.           */
         DispatchPBAMEvent(ConnectionEntryPtr, (BTPM_Message_t *)&Message);

         if(FreeEntry)
            FreePBAMEntryInfoEntryMemory(ConnectionEntryPtr);
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_CRITICAL), ("Device not found\n"));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is used to process Bluetopia PBAP callback */
   /* close indication event. The function accepts a valid close        */
   /* indication structure.                                             */
static void ProcessCloseIndicationEvent(PBAP_Close_Port_Indication_Data_t *CloseIndicationData)
{
   PBAM_Entry_Info_t                  *ConnectionEntryPtr;
   PBAM_Event_Data_t                   PBAMEventData;
   PBAM_Server_Entry_t                *ServerEntry;
   PBAM_Device_Disconnected_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(CloseIndicationData)
   {
      if((ConnectionEntryPtr = DeletePBAMEntryInfoEntry(&PBAMEntryInfoList, NULL, CloseIndicationData->PBAPID)) != NULL)
      {
         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&Message, 0, sizeof(Message));

         Message.MessageHeader.AddressID       = 0;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PHONE_BOOK_ACCESS_MANAGER;
         Message.MessageHeader.MessageFunction = PBAM_MESSAGE_FUNCTION_DEVICE_DISCONNECTED;
         Message.MessageHeader.MessageLength   = (PBAM_DEVICE_DISCONNECTED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.RemoteDeviceAddress           = ConnectionEntryPtr->BluetoothAddress;

         /* Finally dispatch the formatted Event and Message.           */
         DispatchPBAMEvent(ConnectionEntryPtr, (BTPM_Message_t *)&Message);

         FreePBAMEntryInfoEntryMemory(ConnectionEntryPtr);
      }
      else
      {
         if((ServerEntry = SearchPBAMServerEntryPBAPID(&PBAMServerEntryList, CloseIndicationData->PBAPID)) != NULL)
         {
            /* Format the disconnected event for the server connection. */
            BTPS_MemInitialize(&PBAMEventData, 0, sizeof(PBAMEventData));

            PBAMEventData.EventType   = petDisconnected;
            PBAMEventData.EventLength = PBAM_DISCONNECTED_EVENT_DATA_SIZE;

            PBAMEventData.EventData.DisconnectedEventData.RemoteDeviceAddress = ServerEntry->RemoteDeviceAddress;
            PBAMEventData.EventData.DisconnectedEventData.ConnectionID        = ServerEntry->ConnectionID;

            BTPS_MemInitialize(&Message, 0, sizeof(Message));

            Message.MessageHeader.AddressID       = 0;
            Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
            Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PHONE_BOOK_ACCESS_MANAGER;
            Message.MessageHeader.MessageFunction = PBAM_MESSAGE_FUNCTION_DEVICE_DISCONNECTED;
            Message.MessageHeader.MessageLength   = (PBAM_DEVICE_DISCONNECTED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

            Message.RemoteDeviceAddress           = ServerEntry->RemoteDeviceAddress;
            Message.ConnectionID                  = ServerEntry->ConnectionID;
            Message.ServerID                      = ServerEntry->ServerID;

            /* Note that we no longer have a connection to the server   */
            /* instance.                                                */
            CleanupPBAMServerEntry(ServerEntry);

            ServerEntry->ConnectionID     = 0;
            ServerEntry->CurrentOperation = coNone;
            ServerEntry->ConnectionState  = csIdle;

            /* Now dispatch the event.                                  */
            DispatchPBAMServerEvent(ServerEntry, &PBAMEventData, (BTPM_Message_t *)&Message);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is used to process Bluetopia PBAP callback */
   /* abort confirmation event. The function accepts a valid abort      */
   /* confirmation structure.                                           */
static void ProcessAbortConfirmationEvent(PBAP_Abort_Confirmation_Data_t *AbortConfirmationData)
{
   PBAM_Entry_Info_t         *ConnectionEntryPtr;
   PBAM_VCard_Data_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(AbortConfirmationData)
   {
      /* First, flag that we are now disconnected.                      */
      if((ConnectionEntryPtr = SearchPBAMEntryInfoEntry(&PBAMEntryInfoList, NULL, AbortConfirmationData->PBAPID)) != NULL)
      {
         /* Clear the state of aborted                                  */
         ConnectionEntryPtr->Flags &= ~((unsigned long)PBAM_ENTRY_INFO_FLAGS_PENDING_ABORT);

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&Message, 0, sizeof(Message));

         Message.MessageHeader.AddressID       = 0;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PHONE_BOOK_ACCESS_MANAGER;
         Message.MessageHeader.MessageFunction = PBAM_MESSAGE_FUNCTION_VCARD_DATA;
         Message.MessageHeader.MessageLength   = (PBAM_VCARD_DATA_MESSAGE_SIZE(0) - BTPM_MESSAGE_HEADER_SIZE);

         Message.RemoteDeviceAddress           = ConnectionEntryPtr->BluetoothAddress;
         Message.Status                        = PBAM_VCARD_DATA_STATUS_ABORT;
         Message.Final                         = TRUE;

         /* Finally dispatch the formatted Event and Message.           */
         DispatchPBAMEvent(ConnectionEntryPtr, (BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is used to process Bluetopia PBAP callback */
   /* pull phone book confirmation event. The function accepts a valid  */
   /* pull phone book confirmation structure.                           */
static void ProcessPullPhoneBookConfirmationEvent(PBAP_Pull_Phonebook_Confirmation_Data_t *PullPhoneBookConfirmationData)
{
   PBAM_Entry_Info_t              *ConnectionEntryPtr;
   PBAM_VCard_Data_Message_t      *VCardDataMessage;
   unsigned int                    BufferSize;
   int                             Result;
   PBAM_Phone_Book_Size_Message_t  PhoneBookSizeMessage;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(PullPhoneBookConfirmationData)
   {
      if((ConnectionEntryPtr = SearchPBAMEntryInfoEntry(&PBAMEntryInfoList, NULL, PullPhoneBookConfirmationData->PBAPID)) != NULL)
      {
         DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Response Code %d, Final %d, Missed %d, BufferSize %d \n", PullPhoneBookConfirmationData->ResponseCode, PullPhoneBookConfirmationData->Final, PullPhoneBookConfirmationData->NewMissedCalls, PullPhoneBookConfirmationData->BufferSize));

         /* The API include query for getting the phone book size. This */
         /* is received from the remote device by sending a 'special'   */
         /* pull request. If we are in the state of querying the size.  */
         /* then return the correct event (not a pull, but the phone    */
         /* book size event.                                            */
         if(ConnectionEntryPtr->Flags & (unsigned long)PBAM_ENTRY_INFO_FLAGS_PENDING_PHONE_BOOK_SIZE)
         {
            ConnectionEntryPtr->Flags &= ~((unsigned long)PBAM_ENTRY_INFO_FLAGS_PENDING_PHONE_BOOK_SIZE);

            PhoneBookSizeMessage.MessageHeader.AddressID       = 0;
            PhoneBookSizeMessage.MessageHeader.MessageID       = MSG_GetNextMessageID();
            PhoneBookSizeMessage.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PHONE_BOOK_ACCESS_MANAGER;
            PhoneBookSizeMessage.MessageHeader.MessageFunction = PBAM_MESSAGE_FUNCTION_PHONE_BOOK_SIZE;
            PhoneBookSizeMessage.MessageHeader.MessageLength   = PBAM_PHONE_BOOK_SIZE_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

            PhoneBookSizeMessage.RemoteDeviceAddress           = ConnectionEntryPtr->BluetoothAddress;
            PhoneBookSizeMessage.PhoneBookSize                 = PullPhoneBookConfirmationData->PhonebookSize;

            /* Map the response codes, to PBAM codes.                   */
            switch(PullPhoneBookConfirmationData->ResponseCode)
            {
               case PBAP_OBEX_RESPONSE_OK:
               case PBAP_OBEX_RESPONSE_CONTINUE:
                  PhoneBookSizeMessage.Status = PBAM_PHONE_BOOK_SIZE_STATUS_SUCCESS;
                  break;
               default:
                  DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_WARNING), ("Response error : %02X\n", PullPhoneBookConfirmationData->ResponseCode));

                  PhoneBookSizeMessage.Status = PBAM_PHONE_BOOK_SIZE_STATUS_FAILURE_UNKNOWN;
                  break;
            }

            /* Dispatch the event.                                      */
            DispatchPBAMEvent(ConnectionEntryPtr, (BTPM_Message_t *)&PhoneBookSizeMessage);
         }
         else
         {
            if(ConnectionEntryPtr->Flags & PBAM_ENTRY_INFO_FLAGS_PENDING_ABORT)
            {
               /* Do nothing if we have have a pending abort. There     */
               /* could be lost data; however, the abort was requested  */
               /* and the assumption is the caller does not want the    */
               /* data.                                                 */
            }
            else
            {
               /* If there was an error, make sure we clear the buffer. */
               if((PullPhoneBookConfirmationData->ResponseCode != PBAP_OBEX_RESPONSE_OK) || (PullPhoneBookConfirmationData->ResponseCode != PBAP_OBEX_RESPONSE_CONTINUE))
                  BufferSize = PullPhoneBookConfirmationData->BufferSize;
               else
                  BufferSize = 0;

               if((VCardDataMessage = (PBAM_VCard_Data_Message_t *)BTPS_AllocateMemory(PBAM_VCARD_DATA_MESSAGE_SIZE(BufferSize))) != NULL)
               {
                  /* Next, format up the Message to dispatch.           */
                  BTPS_MemInitialize(VCardDataMessage, 0, PBAM_VCARD_DATA_MESSAGE_SIZE(BufferSize));

                  VCardDataMessage->MessageHeader.AddressID       = 0;
                  VCardDataMessage->MessageHeader.MessageID       = MSG_GetNextMessageID();
                  VCardDataMessage->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PHONE_BOOK_ACCESS_MANAGER;
                  VCardDataMessage->MessageHeader.MessageFunction = PBAM_MESSAGE_FUNCTION_VCARD_DATA;
                  VCardDataMessage->MessageHeader.MessageLength   = PBAM_VCARD_DATA_MESSAGE_SIZE(BufferSize) - BTPM_MESSAGE_HEADER_SIZE;

                  VCardDataMessage->RemoteDeviceAddress           = ConnectionEntryPtr->BluetoothAddress;

                  /* Map the response codes, to PBAM codes.             */
                  switch(PullPhoneBookConfirmationData->ResponseCode)
                  {
                     case PBAP_OBEX_RESPONSE_OK:
                     case PBAP_OBEX_RESPONSE_CONTINUE:
                        VCardDataMessage->Status = PBAM_VCARD_DATA_STATUS_SUCCESS;
                        break;
                     default:
                        DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_WARNING), ("Response error : %02X\n", PullPhoneBookConfirmationData->ResponseCode));
                        VCardDataMessage->Status = PBAM_VCARD_DATA_STATUS_FAILURE_UNKNOWN;
                  }

                  VCardDataMessage->Final          = PullPhoneBookConfirmationData->Final;
                  VCardDataMessage->NewMissedCalls = PullPhoneBookConfirmationData->NewMissedCalls;
                  VCardDataMessage->Format         = ConnectionEntryPtr->LastVCardRequestFormt;

                  if((BufferSize) && (PullPhoneBookConfirmationData->Buffer))
                  {
                     VCardDataMessage->BufferSize = BufferSize;

                     BTPS_MemCopy(VCardDataMessage->Buffer, PullPhoneBookConfirmationData->Buffer, PullPhoneBookConfirmationData->BufferSize);
                  }

                  DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Response Code %d, Final %d, Missed %d, BufferSize %d \n", VCardDataMessage->Status, VCardDataMessage->Final, VCardDataMessage->NewMissedCalls, VCardDataMessage->BufferSize));

                  /* Finally dispatch the formatted Event and Message.  */
                  DispatchPBAMEvent(ConnectionEntryPtr, (BTPM_Message_t *)VCardDataMessage);

                  if(!VCardDataMessage->Final)
                  {
                     /* Make next the next request for the caller       */
                     if((Result = _PBAM_Pull_Phone_Book_Request(ConnectionEntryPtr->PBAPID, NULL, 0, 0, pfDefault, PBAP_MAX_LIST_COUNT_NOT_RESTRICTED, 0)) != 0)
                     {
                        DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FAILURE), ("Failed to send next request %d \n", Result));

                        VCardDataMessage->MessageHeader.MessageID       = MSG_GetNextMessageID();
                        VCardDataMessage->MessageHeader.MessageLength   = PBAM_VCARD_DATA_MESSAGE_SIZE(0) - BTPM_MESSAGE_HEADER_SIZE;
                        VCardDataMessage->BufferSize                    = 0;
                        VCardDataMessage->Final                         = TRUE;
                        VCardDataMessage->Status                        = PBAM_VCARD_DATA_STATUS_FAILURE_UNKNOWN;

                        DispatchPBAMEvent(ConnectionEntryPtr, (BTPM_Message_t *)VCardDataMessage);
                     }
                  }

                  /* Finished with the Message, so go ahead and delete  */
                  /* the memory.                                        */
                  BTPS_FreeMemory(VCardDataMessage);
               }
            }
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is used to process Bluetopia PBAP callback */
   /* set phone book confirmation event. The function accepts a valid   */
   /* set phone book confirmation structure.                            */
static void ProcessSetPhoneBookConfirmationEvent(PBAP_Set_Phonebook_Confirmation_Data_t *SetPhoneBookConfirmationData)
{
   PBAM_Entry_Info_t              *ConnectionEntryPtr;
   PBAM_Phone_Book_Set_Message_t  *Message;
   char                            Buffer[64];
   char                           *Slash;
   unsigned int                    StringLength;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(SetPhoneBookConfirmationData)
   {
      if((ConnectionEntryPtr = SearchPBAMEntryInfoEntry(&PBAMEntryInfoList, NULL, SetPhoneBookConfirmationData->PBAPID)) != NULL)
      {
         /* If we are pending an absolute change, we need to catch this.*/
         if((ConnectionEntryPtr->Flags & PBAM_ENTRY_INFO_FLAGS_PENDING_ABSOLUTE_PATH_CHANGE) && (ConnectionEntryPtr->PendingPath != NULL))
         {
            /* If we succeeded, we need to update the path and make     */
            /* the next call.                                           */
            if((SetPhoneBookConfirmationData->ResponseCode == PBAP_OBEX_RESPONSE_OK) || (SetPhoneBookConfirmationData->ResponseCode == PBAP_OBEX_RESPONSE_CONTINUE))
            {
               /* First, we need to update the current path.            */
               StringLength = BTPS_StringLength(ConnectionEntryPtr->PhonebookPath);

               /* Get the folder that was pending.                      */
               if((Slash = strchr(ConnectionEntryPtr->PendingPath + ConnectionEntryPtr->PendingPathOffset, PBAM_PATH_DELIMETER_CHARACTER)) != NULL)
               {
                  /* If this is the root request, we just need to cut it*/
                  /* out of the pending path. Also, if there is no data */
                  /* to add to the path (i.e. consecutive slashes), we  */
                  /* can skip.                                          */
                  if(ConnectionEntryPtr->PendingPathOffset && (Slash - (ConnectionEntryPtr->PendingPath+ConnectionEntryPtr->PendingPathOffset)))
                  {
                     /* If we have previous path information, append a  */
                     /* delimeter.                                      */
                     if(StringLength)
                     {
                        BTPS_StringCopy(ConnectionEntryPtr->PhonebookPath + StringLength, PBAM_PATH_DELIMETER);
                        StringLength++;
                     }

                     BTPS_MemCopy(ConnectionEntryPtr->PhonebookPath + StringLength, ConnectionEntryPtr->PendingPath + ConnectionEntryPtr->PendingPathOffset, Slash - (ConnectionEntryPtr->PendingPath + ConnectionEntryPtr->PendingPathOffset));
                     ConnectionEntryPtr->PhonebookPath[StringLength + (Slash - (ConnectionEntryPtr->PendingPath + ConnectionEntryPtr->PendingPathOffset))] = '\0';
                  }

                  ConnectionEntryPtr->PendingPathOffset += Slash - (ConnectionEntryPtr->PendingPath + ConnectionEntryPtr->PendingPathOffset) + 1;


                  if((Slash = strchr(ConnectionEntryPtr->PendingPath + ConnectionEntryPtr->PendingPathOffset, PBAM_PATH_DELIMETER_CHARACTER)) != NULL)
                  {
                     BTPS_MemCopy(Buffer, ConnectionEntryPtr->PendingPath + ConnectionEntryPtr->PendingPathOffset, Slash - (ConnectionEntryPtr->PendingPath + ConnectionEntryPtr->PendingPathOffset));
                     Buffer[Slash - (ConnectionEntryPtr->PendingPath + ConnectionEntryPtr->PendingPathOffset)] = '\0';

                     /* If the buffer is empty, skip ahead.             */
                     if(!(BTPS_StringLength(Buffer)))
                        ProcessSetPhoneBookConfirmationEvent(SetPhoneBookConfirmationData);

                     else
                     {
                        if((Slash) && (_PBAM_Set_Phone_Book_Request(ConnectionEntryPtr->PBAPID, pspDown, Buffer) != 0))
                        {
                           /* If we have a null path, we need to denote */
                           /* root.                                     */
                           StringLength = ((ConnectionEntryPtr->PhonebookPath)?BTPS_StringLength(ConnectionEntryPtr->PhonebookPath):BTPS_StringLength(PBAM_PATH_DELIMETER)) + 1;

                           if((Message = (PBAM_Phone_Book_Set_Message_t *)BTPS_AllocateMemory(PBAM_PHONE_BOOK_SET_MESSAGE_SIZE(StringLength))) != NULL)
                           {
                              BTPS_MemInitialize(Message, 0, PBAM_PHONE_BOOK_SET_MESSAGE_SIZE(StringLength));

                              Message->MessageHeader.AddressID       = 0;
                              Message->MessageHeader.MessageID       = MSG_GetNextMessageID();
                              Message->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PHONE_BOOK_ACCESS_MANAGER;
                              Message->MessageHeader.MessageFunction = PBAM_MESSAGE_FUNCTION_PHONE_BOOK_SET;
                              Message->MessageHeader.MessageLength   = (PBAM_PHONE_BOOK_SET_MESSAGE_SIZE(StringLength) - BTPM_MESSAGE_HEADER_SIZE);

                              Message->RemoteDeviceAddress           = ConnectionEntryPtr->BluetoothAddress;
                              Message->Status                        = PBAM_PHONE_BOOK_SET_STATUS_FAILURE_UNKNOWN;
                              Message->CurrentPathSize               = StringLength;

                              BTPS_MemCopy(Message->CurrentPath, (ConnectionEntryPtr->PhonebookPath)?ConnectionEntryPtr->PhonebookPath:PBAM_PATH_DELIMETER, StringLength);

                              DispatchPBAMEvent(ConnectionEntryPtr, (BTPM_Message_t *)Message);
                           }
                        }
                     }
                  }
                  else
                  {

                     /* We are on the last folder, so flag done with    */
                     /* absolute.                                       */
                     ConnectionEntryPtr->Flags &= ~((unsigned long)PBAM_ENTRY_INFO_FLAGS_PENDING_ABSOLUTE_PATH_CHANGE);

                     BTPS_StringCopy(Buffer, ConnectionEntryPtr->PendingPath + ConnectionEntryPtr->PendingPathOffset);

                     /* We are done with the pending path, so free it.  */
                     BTPS_FreeMemory(ConnectionEntryPtr->PendingPath);
                     ConnectionEntryPtr->PendingPath = NULL;

                     /* There's nothing left to send, so return success.*/
                     if(!BTPS_StringLength(Buffer))
                     {
                        /* If we have a null path, we need to denote    */
                        /* root.                                        */
                        StringLength = ((ConnectionEntryPtr->PhonebookPath)?BTPS_StringLength(ConnectionEntryPtr->PhonebookPath):BTPS_StringLength(PBAM_PATH_DELIMETER)) + 1;

                        if((Message = (PBAM_Phone_Book_Set_Message_t *)BTPS_AllocateMemory(PBAM_PHONE_BOOK_SET_MESSAGE_SIZE(StringLength))) != NULL)
                        {
                           BTPS_MemInitialize(Message, 0, PBAM_PHONE_BOOK_SET_MESSAGE_SIZE(StringLength));

                           Message->MessageHeader.AddressID       = 0;
                           Message->MessageHeader.MessageID       = MSG_GetNextMessageID();
                           Message->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PHONE_BOOK_ACCESS_MANAGER;
                           Message->MessageHeader.MessageFunction = PBAM_MESSAGE_FUNCTION_PHONE_BOOK_SET;
                           Message->MessageHeader.MessageLength   = (PBAM_PHONE_BOOK_SET_MESSAGE_SIZE(StringLength) - BTPM_MESSAGE_HEADER_SIZE);

                           Message->RemoteDeviceAddress           = ConnectionEntryPtr->BluetoothAddress;
                           Message->Status                        = PBAM_PHONE_BOOK_SET_STATUS_SUCCESS;
                           Message->CurrentPathSize               = StringLength;

                           BTPS_MemCopy(Message->CurrentPath, (ConnectionEntryPtr->PhonebookPath)?ConnectionEntryPtr->PhonebookPath:PBAM_PATH_DELIMETER, StringLength);

                           DispatchPBAMEvent(ConnectionEntryPtr, (BTPM_Message_t *)Message);
                        }
                     }
                     else
                     {
                        /* Make the the call to the API to go down to   */
                        /* the last folder in the path.                 */
                        if(_Set_Phone_Book(ConnectionEntryPtr->BluetoothAddress, pspDown, Buffer, ConnectionEntryPtr->ClientID) != 0)
                        {
                           /* If we have a null path, we need to denote */
                           /* root.                                     */
                           StringLength = ((ConnectionEntryPtr->PhonebookPath)?BTPS_StringLength(ConnectionEntryPtr->PhonebookPath):BTPS_StringLength(PBAM_PATH_DELIMETER)) + 1;

                           if((Message = (PBAM_Phone_Book_Set_Message_t *)BTPS_AllocateMemory(PBAM_PHONE_BOOK_SET_MESSAGE_SIZE(StringLength))) != NULL)
                           {
                              BTPS_MemInitialize(Message, 0, PBAM_PHONE_BOOK_SET_MESSAGE_SIZE(StringLength));

                              Message->MessageHeader.AddressID       = 0;
                              Message->MessageHeader.MessageID       = MSG_GetNextMessageID();
                              Message->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PHONE_BOOK_ACCESS_MANAGER;
                              Message->MessageHeader.MessageFunction = PBAM_MESSAGE_FUNCTION_PHONE_BOOK_SET;
                              Message->MessageHeader.MessageLength   = (PBAM_PHONE_BOOK_SET_MESSAGE_SIZE(StringLength) - BTPM_MESSAGE_HEADER_SIZE);

                              Message->RemoteDeviceAddress           = ConnectionEntryPtr->BluetoothAddress;
                              Message->Status                        = PBAM_PHONE_BOOK_SET_STATUS_FAILURE_UNKNOWN;
                              Message->CurrentPathSize               = StringLength;

                              BTPS_MemCopy(Message->CurrentPath, (ConnectionEntryPtr->PhonebookPath)?ConnectionEntryPtr->PhonebookPath:PBAM_PATH_DELIMETER, StringLength);

                              DispatchPBAMEvent(ConnectionEntryPtr, (BTPM_Message_t *)Message);
                           }
                        }
                     }
                  }
               }
            }
            else
            {
               /* Something failed, notify the client, and clean up.    */
               ConnectionEntryPtr->Flags &= ~((unsigned long)PBAM_ENTRY_INFO_FLAGS_PENDING_ABSOLUTE_PATH_CHANGE);

               BTPS_FreeMemory(ConnectionEntryPtr->PendingPath);

               ConnectionEntryPtr->PendingPath       = NULL;
               ConnectionEntryPtr->PendingPathOffset = 0;

               /* If we have a null path, we need to denote root.       */
               StringLength = ((ConnectionEntryPtr->PhonebookPath)?BTPS_StringLength(ConnectionEntryPtr->PhonebookPath):BTPS_StringLength(PBAM_PATH_DELIMETER)) + 1;

               if((Message = (PBAM_Phone_Book_Set_Message_t *)BTPS_AllocateMemory(PBAM_PHONE_BOOK_SET_MESSAGE_SIZE(StringLength))) != NULL)
               {
                  BTPS_MemInitialize(Message, 0, PBAM_PHONE_BOOK_SET_MESSAGE_SIZE(StringLength));

                  Message->MessageHeader.AddressID       = 0;
                  Message->MessageHeader.MessageID       = MSG_GetNextMessageID();
                  Message->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PHONE_BOOK_ACCESS_MANAGER;
                  Message->MessageHeader.MessageFunction = PBAM_MESSAGE_FUNCTION_PHONE_BOOK_SET;
                  Message->MessageHeader.MessageLength   = (PBAM_PHONE_BOOK_SET_MESSAGE_SIZE(StringLength) - BTPM_MESSAGE_HEADER_SIZE);

                  Message->RemoteDeviceAddress           = ConnectionEntryPtr->BluetoothAddress;
                  Message->Status                        = PBAM_PHONE_BOOK_SET_STATUS_FAILURE_UNKNOWN;
                  Message->CurrentPathSize               = StringLength;

                  BTPS_MemCopy(Message->CurrentPath, (ConnectionEntryPtr->PhonebookPath)?ConnectionEntryPtr->PhonebookPath:PBAM_PATH_DELIMETER, StringLength);

                  DispatchPBAMEvent(ConnectionEntryPtr, (BTPM_Message_t *)Message);
               }
            }

         }
         else
         {
            /* If we were successful we need to update the path.        */
            if((SetPhoneBookConfirmationData->ResponseCode == PBAP_OBEX_RESPONSE_OK) || (SetPhoneBookConfirmationData->ResponseCode == PBAP_OBEX_RESPONSE_CONTINUE))
            {
               BTPS_FreeMemory(ConnectionEntryPtr->PhonebookPath);

               ConnectionEntryPtr->PhonebookPath = ConnectionEntryPtr->PendingPath;
               ConnectionEntryPtr->PendingPath   = NULL;
            }

            /* Next, format up the Message to dispatch.                 */

            /* If we have a null path, we need to denote root.          */
            StringLength = ((ConnectionEntryPtr->PhonebookPath)?BTPS_StringLength(ConnectionEntryPtr->PhonebookPath):BTPS_StringLength(PBAM_PATH_DELIMETER)) + 1;

            if((Message = (PBAM_Phone_Book_Set_Message_t *)BTPS_AllocateMemory(PBAM_PHONE_BOOK_SET_MESSAGE_SIZE(StringLength))) != NULL)
            {
               BTPS_MemInitialize(Message, 0, PBAM_PHONE_BOOK_SET_MESSAGE_SIZE(StringLength));

               Message->MessageHeader.AddressID       = 0;
               Message->MessageHeader.MessageID       = MSG_GetNextMessageID();
               Message->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PHONE_BOOK_ACCESS_MANAGER;
               Message->MessageHeader.MessageFunction = PBAM_MESSAGE_FUNCTION_PHONE_BOOK_SET;
               Message->MessageHeader.MessageLength   = PBAM_PHONE_BOOK_SET_MESSAGE_SIZE(StringLength) - BTPM_MESSAGE_HEADER_SIZE;

               Message->RemoteDeviceAddress           = ConnectionEntryPtr->BluetoothAddress;
               Message->CurrentPathSize               = StringLength;

               BTPS_MemCopy(Message->CurrentPath, (ConnectionEntryPtr->PhonebookPath)?ConnectionEntryPtr->PhonebookPath:PBAM_PATH_DELIMETER, StringLength);

               /* Map the response codes, to PBAM codes.                */
               switch(SetPhoneBookConfirmationData->ResponseCode)
               {
                  case PBAP_OBEX_RESPONSE_OK:
                  case PBAP_OBEX_RESPONSE_CONTINUE:
                     Message->Status = PBAM_PHONE_BOOK_SET_STATUS_SUCCESS;
                     break;
                  default:
                     DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_WARNING), ("Response error : %02X\n", SetPhoneBookConfirmationData->ResponseCode));

                     Message->Status = PBAM_PHONE_BOOK_SET_STATUS_FAILURE_UNKNOWN;
                     break;
               }

               /* Finally dispatch the formatted Event and Message.     */
               DispatchPBAMEvent(ConnectionEntryPtr, (BTPM_Message_t *)Message);
            }
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is used to process Bluetopia PBAP callback */
   /* pull vCard listing confirmation event. The function accepts a     */
   /* valid pull vCard listing confirmation structure.                  */
static void ProcessPullvCardListingConfirmationEvent(PBAP_Pull_vCard_Listing_Confirmation_Data_t *PullvCardListingConfirmationData)
{
   unsigned int                       BufferSize;
   PBAM_Entry_Info_t                 *ConnectionEntryPtr;
   PBAM_VCard_Listing_Data_Message_t *VCardListingDataMessage;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(PullvCardListingConfirmationData)
   {
      if((ConnectionEntryPtr = SearchPBAMEntryInfoEntry(&PBAMEntryInfoList, NULL, PullvCardListingConfirmationData->PBAPID)) != NULL)
      {
         DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Response Code %d, Final %d, Missed %d, BufferSize %d \n", PullvCardListingConfirmationData->ResponseCode, PullvCardListingConfirmationData->Final, PullvCardListingConfirmationData->NewMissedCalls, PullvCardListingConfirmationData->BufferSize));


         if(ConnectionEntryPtr->Flags & PBAM_ENTRY_INFO_FLAGS_PENDING_ABORT)
         {
            /* Do nothing if we have have a pending abort. There        */
            /* could be lost data; however, the abort was requested     */
            /* and the assumption is the caller does not want the       */
            /* data.                                                    */
         }
         else
         {
            /* If there was an error, make sure we clear the buffer.    */
            if((PullvCardListingConfirmationData->ResponseCode != PBAP_OBEX_RESPONSE_OK) || (PullvCardListingConfirmationData->ResponseCode != PBAP_OBEX_RESPONSE_CONTINUE))
               BufferSize = PullvCardListingConfirmationData->BufferSize;
            else
               BufferSize = 0;

            if((VCardListingDataMessage = (PBAM_VCard_Listing_Data_Message_t *)BTPS_AllocateMemory(PBAM_VCARD_LISTING_DATA_MESSAGE_SIZE(BufferSize))) != 0 )
            {
               /* Next, format up the Message to dispatch.              */
               BTPS_MemInitialize(VCardListingDataMessage, 0, PBAM_VCARD_LISTING_DATA_MESSAGE_SIZE(BufferSize));

               VCardListingDataMessage->MessageHeader.AddressID       = 0;
               VCardListingDataMessage->MessageHeader.MessageID       = MSG_GetNextMessageID();
               VCardListingDataMessage->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PHONE_BOOK_ACCESS_MANAGER;
               VCardListingDataMessage->MessageHeader.MessageFunction = PBAM_MESSAGE_FUNCTION_VCARD_LISTING_DATA;
               VCardListingDataMessage->MessageHeader.MessageLength   = PBAM_VCARD_LISTING_DATA_MESSAGE_SIZE(BufferSize) - BTPM_MESSAGE_HEADER_SIZE;

               VCardListingDataMessage->RemoteDeviceAddress           = ConnectionEntryPtr->BluetoothAddress;

               /* Map the response codes to PBAM codes.                 */
               switch(PullvCardListingConfirmationData->ResponseCode)
               {
                  case PBAP_OBEX_RESPONSE_OK:
                  case PBAP_OBEX_RESPONSE_CONTINUE:
                     VCardListingDataMessage->Status = PBAM_VCARD_LISTING_STATUS_SUCCESS;
                     break;
                  default:
                     DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_WARNING), ("Response error : %02X\n", PullvCardListingConfirmationData->ResponseCode));
                     VCardListingDataMessage->Status = PBAM_VCARD_LISTING_STATUS_FAILURE_UNKNOWN;
               }

               VCardListingDataMessage->Final          = PullvCardListingConfirmationData->Final;
               VCardListingDataMessage->NewMissedCalls = PullvCardListingConfirmationData->NewMissedCalls;

               if((BufferSize) && (PullvCardListingConfirmationData->Buffer))
               {
                  VCardListingDataMessage->BufferSize = BufferSize;

                  BTPS_MemCopy(VCardListingDataMessage->Buffer, PullvCardListingConfirmationData->Buffer, BufferSize);
               }

               /* Finally dispatch the formatted Event and Message.     */
               DispatchPBAMEvent(ConnectionEntryPtr, (BTPM_Message_t *)VCardListingDataMessage);

               if(!VCardListingDataMessage->Final)
               {
                  /* Make the next request for the caller.              */
                  if(_PBAM_Pull_vCard_Listing_Request(ConnectionEntryPtr->PBAPID, NULL, ploDefault, psaDefault, NULL, PBAP_MAX_LIST_COUNT_NOT_RESTRICTED, 0) != 0)
                  {
                     /* Request Failed.                                 */
                     VCardListingDataMessage->MessageHeader.MessageID       = MSG_GetNextMessageID();
                     VCardListingDataMessage->MessageHeader.MessageLength   = PBAM_VCARD_LISTING_DATA_MESSAGE_SIZE(0) - BTPM_MESSAGE_HEADER_SIZE;
                     VCardListingDataMessage->BufferSize                    = 0;
                     VCardListingDataMessage->Final                         = TRUE;
                     VCardListingDataMessage->Status                        = PBAM_VCARD_LISTING_STATUS_FAILURE_UNKNOWN;

                     DispatchPBAMEvent(ConnectionEntryPtr, (BTPM_Message_t *)VCardListingDataMessage);
                  }
               }

               /* Go ahead and delete the message.                      */
               BTPS_FreeMemory(VCardListingDataMessage);
            }
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is used to process Bluetopia PBAP callback */
   /* pull vCard entry confirmation event. The function accepts a valid */
   /* pull vCard entry confirmation structure.                          */
static void ProcessPullvCardEntryConfirmationEvent(PBAP_Pull_vCard_Entry_Confirmation_Data_t *PullvCardEntryConfirmationData)
{
   unsigned int               BufferSize;
   PBAM_Entry_Info_t         *ConnectionEntryPtr;
   PBAM_VCard_Data_Message_t *VCardDataMessage;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(PullvCardEntryConfirmationData)
   {
      if((ConnectionEntryPtr = SearchPBAMEntryInfoEntry(&PBAMEntryInfoList, NULL, PullvCardEntryConfirmationData->PBAPID)) != NULL)
      {

         if(ConnectionEntryPtr->Flags & PBAM_ENTRY_INFO_FLAGS_PENDING_ABORT)
         {
            /* Do nothing if we have have a pending abort. There        */
            /* could be lost data; however, the abort was requested     */
            /* and the assumption is the caller does not want the       */
            /* data.                                                    */
         }
         else
         {
            /* If there was an error, make sure we clear the buffer.    */
            if((PullvCardEntryConfirmationData->ResponseCode != PBAP_OBEX_RESPONSE_OK) || (PullvCardEntryConfirmationData->ResponseCode != PBAP_OBEX_RESPONSE_CONTINUE))
               BufferSize = PullvCardEntryConfirmationData->BufferSize;
            else
               BufferSize = 0;

            if((VCardDataMessage = (PBAM_VCard_Data_Message_t *)BTPS_AllocateMemory(PBAM_VCARD_DATA_MESSAGE_SIZE(BufferSize))) != 0 )
            {
               /* Next, format up the Message to dispatch.              */
               BTPS_MemInitialize(VCardDataMessage, 0, PBAM_VCARD_DATA_MESSAGE_SIZE(BufferSize));

               VCardDataMessage->MessageHeader.AddressID       = 0;
               VCardDataMessage->MessageHeader.MessageID       = MSG_GetNextMessageID();
               VCardDataMessage->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PHONE_BOOK_ACCESS_MANAGER;
               VCardDataMessage->MessageHeader.MessageFunction = PBAM_MESSAGE_FUNCTION_VCARD_DATA;
               VCardDataMessage->MessageHeader.MessageLength   = PBAM_VCARD_DATA_MESSAGE_SIZE(BufferSize) - BTPM_MESSAGE_HEADER_SIZE;

               VCardDataMessage->RemoteDeviceAddress           = ConnectionEntryPtr->BluetoothAddress;

               /* Map the response codes to PBAM codes.                 */
               switch(PullvCardEntryConfirmationData->ResponseCode)
               {
                  case PBAP_OBEX_RESPONSE_OK:
                  case PBAP_OBEX_RESPONSE_CONTINUE:
                     VCardDataMessage->Status = PBAM_VCARD_DATA_STATUS_SUCCESS;
                     break;
                  default:
                     DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_WARNING), ("Response error : %02X\n", PullvCardEntryConfirmationData->ResponseCode));
                     VCardDataMessage->Status = PBAM_VCARD_DATA_STATUS_FAILURE_UNKNOWN;
               }

               VCardDataMessage->Final          = PullvCardEntryConfirmationData->Final;
               VCardDataMessage->NewMissedCalls = 0;
               VCardDataMessage->Format         = ConnectionEntryPtr->LastVCardRequestFormt;

               if((BufferSize) && (PullvCardEntryConfirmationData->Buffer))
               {
                  VCardDataMessage->BufferSize = BufferSize;

                  BTPS_MemCopy(VCardDataMessage->Buffer, PullvCardEntryConfirmationData->Buffer, BufferSize);
               }

               /* Finally dispatch the formatted Event and Message.     */
               DispatchPBAMEvent(ConnectionEntryPtr, (BTPM_Message_t *)VCardDataMessage);

               if(!VCardDataMessage->Final)
               {
                  /* Make the next request for the caller.              */
                  if(_PBAM_Pull_vCard_Entry_Request(ConnectionEntryPtr->PBAPID, NULL, 0, 0, pfDefault) != 0)
                  {
                     /* Request Failed.                                 */
                     VCardDataMessage->MessageHeader.MessageID       = MSG_GetNextMessageID();
                     VCardDataMessage->MessageHeader.MessageLength   = PBAM_VCARD_DATA_MESSAGE_SIZE(0) - BTPM_MESSAGE_HEADER_SIZE;
                     VCardDataMessage->BufferSize                    = 0;
                     VCardDataMessage->Final                         = TRUE;
                     VCardDataMessage->Status                        = PBAM_VCARD_DATA_STATUS_FAILURE_UNKNOWN;

                     DispatchPBAMEvent(ConnectionEntryPtr, (BTPM_Message_t *)VCardDataMessage);
                  }
               }

               /* Go ahead and delete the message.                      */
               BTPS_FreeMemory(VCardDataMessage);
            }
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is used to process Bluetopia PBAP callback */
   /* Open Request Indication event. The function accepts a valid Open  */
   /* Request Indication structure.                                     */
static void ProcessOpenRequestIndication(PBAP_Open_Port_Request_Indication_Data_t *OpenRequestIndicationData)
{
   int                                Result;
   void                              *CallbackParameter;
   Boolean_t                          Authenticate;
   Boolean_t                          Encrypt;
   PBAM_Event_Data_t                  PBAMEventData;
   PBAM_Server_Entry_t                 *PBAMEntryInfo;
   PBAM_Event_Callback_t              EventCallback;
   PBAM_Connection_Request_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check that the pointer to the event data is not NULL.             */
   if(OpenRequestIndicationData)
   {
      /* Check to see if we can find the indicated server instance.     */
      if((PBAMEntryInfo = SearchPBAMServerEntryPBAPID(&PBAMServerEntryList, OpenRequestIndicationData->PBAPID)) != NULL)
      {
         /* Record the address of the remote device.                    */
         PBAMEntryInfo->RemoteDeviceAddress = OpenRequestIndicationData->BD_ADDR;
         PBAMEntryInfo->ConnectionID        = GetNextConnectionID();

         /* Check whether any connection flags are set.                 */
         if(!PBAMEntryInfo->ConnectionFlags)
         {
            /* Simply Accept the connection.                            */
            _PBAM_Open_Request_Response(OpenRequestIndicationData->PBAPID, TRUE);
         }
         else
         {
            /* Check if authorization is required.                      */
            if(PBAMEntryInfo->ConnectionFlags & PBAM_INCOMING_CONNECTION_FLAGS_REQUIRE_AUTHORIZATION)
            {
               PBAMEntryInfo->ConnectionState = csAuthorizing;

               /* Dispatch the event based upon the client registration */
               /* type.                                                 */
               if(PBAMEntryInfo->ClientID == MSG_GetServerAddressID())
               {
                  /* Dispatch the event locally.                        */

                  /* Event needs to be dispatched.  Go ahead and format */
                  /* the event.                                         */
                  BTPS_MemInitialize(&PBAMEventData, 0, sizeof(PBAM_Event_Data_t));

                  /* Format the event data.                             */
                  PBAMEventData.EventType                                                = petConnectionRequest;
                  PBAMEventData.EventLength                                              = PBAM_CONNECTION_REQUEST_EVENT_DATA_SIZE;

                  PBAMEventData.EventData.ConnectionRequestEventData.RemoteDeviceAddress = OpenRequestIndicationData->BD_ADDR;
                  PBAMEventData.EventData.ConnectionRequestEventData.ServerID            = PBAMEntryInfo->ServerID;
                  PBAMEventData.EventData.ConnectionRequestEventData.ConnectionID        = PBAMEntryInfo->ConnectionID;

                  /* Note the Callback information.                     */
                  EventCallback                                                          = PBAMEntryInfo->CallbackFunction;
                  CallbackParameter                                                      = PBAMEntryInfo->CallbackParameter;

                  /* Release the Lock so we can dispatch the event.     */
                  DEVM_ReleaseLock();

                  __BTPSTRY
                  {
                     if(EventCallback)
                        (*EventCallback)(&PBAMEventData, CallbackParameter);
                  }
                  __BTPSEXCEPT(1)
                  {
                     /* Do Nothing.                                     */
                  }

                  /* Re-acquire the Lock.                               */
                  DEVM_AcquireLock();
               }
               else
               {
                  /* Dispatch the event remotely.                       */

                  /* Format the message.                                */
                  BTPS_MemInitialize(&Message, 0, sizeof(Message));

                  Message.MessageHeader.AddressID       = PBAMEntryInfo->ClientID;
                  Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
                  Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PHONE_BOOK_ACCESS_MANAGER;
                  Message.MessageHeader.MessageFunction = PBAM_MESSAGE_FUNCTION_CONNECTION_REQUEST;
                  Message.MessageHeader.MessageLength   = (PBAM_CONNECTION_REQUEST_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

                  Message.RemoteDeviceAddress           = OpenRequestIndicationData->BD_ADDR;
                  Message.ServerID                      = PBAMEntryInfo->ServerID;
                  Message.ConnectionID                  = PBAMEntryInfo->ConnectionID;

                  /* Message has been formatted, go ahead and dispatch  */
                  /* it.                                                */
                  MSG_SendMessage((BTPM_Message_t *)&Message);
               }
            }
            else
            {
               /* Determine if authentication or encryption is required.*/
               if(PBAMEntryInfo->ConnectionFlags & PBAM_INCOMING_CONNECTION_FLAGS_REQUIRE_AUTHENTICATION)
                  Authenticate = TRUE;
               else
                  Authenticate = FALSE;

               if(PBAMEntryInfo->ConnectionFlags & PBAM_INCOMING_CONNECTION_FLAGS_REQUIRE_ENCRYPTION)
                  Encrypt = TRUE;
               else
                  Encrypt = FALSE;

               if((Authenticate) || (Encrypt))
               {
                  if(Encrypt)
                     Result = DEVM_EncryptRemoteDevice(PBAMEntryInfo->RemoteDeviceAddress, 0);
                  else
                     Result = DEVM_AuthenticateRemoteDevice(PBAMEntryInfo->RemoteDeviceAddress, 0);
               }
               else
                  Result = BTPM_ERROR_CODE_DEVICE_ALREADY_AUTHENTICATED;

               if((Result == BTPM_ERROR_CODE_DEVICE_ALREADY_AUTHENTICATED) || (Result == BTPM_ERROR_CODE_DEVICE_ALREADY_ENCRYPTED))
               {
                  /* Accept the connection.                             */
                  Result = _PBAM_Open_Request_Response(PBAMEntryInfo->PBAPID, TRUE);

                  if(Result)
                     _PBAM_Open_Request_Response(PBAMEntryInfo->PBAPID, FALSE);
                  else
                  {
                     /* Update the current connection state.            */
                     PBAMEntryInfo->ConnectionState = csConnecting;
                  }
               }
               else
               {
                  /* Set the connection state.                          */
                  if(!Result)
                  {
                     if(Encrypt)
                        PBAMEntryInfo->ConnectionState = csEncrypting;
                     else
                        PBAMEntryInfo->ConnectionState = csAuthenticating;

                     /* Flag success.                                   */
                     Result = 0;
                  }
                  else
                  {
                     /* Reject the request.                             */
                     _PBAM_Open_Request_Response(PBAMEntryInfo->PBAPID, FALSE);
                  }
               }
            }
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is used to process Bluetopia PBAP callback */
   /* Open Indication event. The function accepts a valid Open          */
   /* Indication structure.                                             */
static void ProcessOpenIndication(PBAP_Open_Port_Indication_Data_t *OpenIndicationData)
{
   PBAM_Event_Data_t         PBAMEventData;
   PBAM_Server_Entry_t      *PBAMEntryInfo;
   PBAM_Connected_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check if the event data parameter is not NULL.                    */
   if(OpenIndicationData)
   {
      /* Find the list entry by the PBAPID.                             */
      if((PBAMEntryInfo = SearchPBAMServerEntryPBAPID(&PBAMServerEntryList, OpenIndicationData->PBAPID)) != NULL)
      {
         /* Flag that this Server Entry is now connected.               */
         PBAMEntryInfo->ConnectionState = csConnected;

         /* Go ahead and format the event.                              */
         BTPS_MemInitialize(&PBAMEventData, 0, sizeof(PBAM_Event_Data_t));

         /* Format the event data.                                      */
         PBAMEventData.EventType                                        = petConnected;
         PBAMEventData.EventLength                                      = PBAM_CONNECTED_EVENT_DATA_SIZE;

         PBAMEventData.EventData.ConnectedEventData.ServerID            = PBAMEntryInfo->ServerID;
         PBAMEventData.EventData.ConnectedEventData.RemoteDeviceAddress = PBAMEntryInfo->RemoteDeviceAddress;
         PBAMEventData.EventData.ConnectedEventData.ConnectionID        = PBAMEntryInfo->ConnectionID;

         /* Format the message.                                         */
         BTPS_MemInitialize(&Message, 0, sizeof(Message));

         Message.MessageHeader.AddressID       = PBAMEntryInfo->ClientID;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PHONE_BOOK_ACCESS_MANAGER;
         Message.MessageHeader.MessageFunction = PBAM_MESSAGE_FUNCTION_CONNECTED;
         Message.MessageHeader.MessageLength   = (PBAM_CONNECTED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.RemoteDeviceAddress           = PBAMEntryInfo->RemoteDeviceAddress;
         Message.ServerID                      = PBAMEntryInfo->ServerID;
         Message.ConnectionID                  = PBAMEntryInfo->ConnectionID;

         /* Dispatch the event.                                         */
         DispatchPBAMServerEvent(PBAMEntryInfo, &PBAMEventData, (BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is used to process Bluetopia PBAP callback */
   /* Abort Indication event. The function accepts a valid Abort        */
   /* Indication structure.                                             */
static void ProcessAbortIndication(PBAP_Abort_Indication_Data_t *AbortIndicationData)
{
   PBAM_Event_Data_t       PBAMEventData;
   PBAM_Server_Entry_t    *PBAMEntryInfo;
   PBAM_Aborted_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check if the event data parameter is not NULL.                    */
   if(AbortIndicationData)
   {
      /* Find the list entry by the PBAPID.                             */
      if((PBAMEntryInfo = SearchPBAMServerEntryPBAPID(&PBAMServerEntryList, AbortIndicationData->PBAPID)) != NULL)
      {
         /* Flag we no longer have in operation in progress.            */
         PBAMEntryInfo->CurrentOperation = coNone;

         CleanupPBAMServerEntry(PBAMEntryInfo);

         /* Go ahead and format the event.                              */
         BTPS_MemInitialize(&PBAMEventData, 0, sizeof(PBAM_Event_Data_t));

         /* Format the event data.                                      */
         PBAMEventData.EventType                                        = petAborted;
         PBAMEventData.EventLength                                      = PBAM_ABORTED_EVENT_DATA_SIZE;

         PBAMEventData.EventData.AbortedEventData.ConnectionID          = PBAMEntryInfo->ConnectionID;

         /* Format the message.                                         */
         BTPS_MemInitialize(&Message, 0, sizeof(Message));

         Message.MessageHeader.AddressID       = PBAMEntryInfo->ClientID;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PHONE_BOOK_ACCESS_MANAGER;
         Message.MessageHeader.MessageFunction = PBAM_MESSAGE_FUNCTION_ABORTED;
         Message.MessageHeader.MessageLength   = (PBAM_ABORTED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.ConnectionID                  = PBAMEntryInfo->ConnectionID;
         Message.ServerID                      = PBAMEntryInfo->ServerID;

         /* Dispatch the event.                                         */
         DispatchPBAMServerEvent(PBAMEntryInfo, &PBAMEventData, (BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is used to process Bluetopia PBAP callback */
   /* Pull Phonebook Indication event. The function accepts a valid Pull*/
   /* Phonebook Indication structure.                                   */
static void ProcessPullPhonebookIndication(PBAP_Pull_Phonebook_Indication_Data_t *PullPhonebookIndicationData)
{
   unsigned int                         StringLength;
   unsigned int                         DataLength;
   PBAM_Event_Data_t                    PBAMEventData;
   PBAM_VCard_Format_t                  vCardFormat;
   PBAM_Server_Entry_t                 *PBAMEntryInfo;
   PBAM_Pull_Phone_Book_Message_t      *Message;
   PBAM_Pull_Phone_Book_Size_Message_t *SizeMessage;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check if the event data parameter is not NULL.                    */
   if(PullPhonebookIndicationData)
   {
      /* Find the list entry by the PBAPID.                             */
      if(((PBAMEntryInfo = SearchPBAMServerEntryPBAPID(&PBAMServerEntryList, PullPhonebookIndicationData->PBAPID)) != NULL) && ((PBAMEntryInfo->CurrentOperation == coNone) || PBAMEntryInfo->CurrentOperation == coPullPhonebook))
      {
         if(PullPhonebookIndicationData->ObjectName)
            StringLength = BTPS_StringLength(PullPhonebookIndicationData->ObjectName) + 1;
         else
            StringLength = 0;

         /* Convert data types.                                         */
         switch(PullPhonebookIndicationData->Format)
         {
            case pfvCard21:
               vCardFormat = pmvCard21;
               break;
            case pfvCard30:
               vCardFormat = pmvCard30;
               break;
            case pfDefault:
            default:
               vCardFormat = pmDefault;
               break;
         }

         /* Determine whether this is a data or size event.             */
         if(PullPhonebookIndicationData->MaxListCount)
         {
            /* Determine if we need to send more response data.         */
            if((PBAMEntryInfo->CurrentOperation == coPullPhonebook) && (PBAMEntryInfo->DataBufferSize) && (PBAMEntryInfo->DataBuffer))
            {
               /* Calculate the remaining data to send.                 */
               DataLength = PBAMEntryInfo->DataBufferSize - PBAMEntryInfo->DataBufferSent;

               if(_PBAM_Pull_Phonebook_Response(PBAMEntryInfo->PBAPID, (Byte_t)(PBAMEntryInfo->DataFinal?PBAP_OBEX_RESPONSE_OK:PBAP_OBEX_RESPONSE_CONTINUE), NULL, NULL, DataLength, &(PBAMEntryInfo->DataBuffer[PBAMEntryInfo->DataBufferSent]), &DataLength) == 0)
               {
                  PBAMEntryInfo->DataBufferSent += DataLength;

                  DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Submitting continuation data: %d, %d\n", (int)PBAMEntryInfo->PBAPID, PBAMEntryInfo->DataBufferSent));
               }
               else
               {
                  DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Error submitting continuation data: %d, %d\n", (int)PBAMEntryInfo->PBAPID, PBAMEntryInfo->DataBufferSent));

                  /* Error submitting response.  Not sure what we can do*/
                  /* here.                                              */
                  _PBAM_Pull_Phonebook_Response(PBAMEntryInfo->PBAPID, (Byte_t)(PBAP_OBEX_RESPONSE_SERVICE_UNAVAILABLE), NULL, NULL, 0, NULL, NULL);

                  /* Flag that we have sent all the data (so it can be  */
                  /* freed below).                                      */
                  PBAMEntryInfo->DataBufferSent   = PBAMEntryInfo->DataBufferSize;

                  /* Flag that there is no longer an operation in       */
                  /* progress.                                          */
                  PBAMEntryInfo->CurrentOperation = coNone;
               }

               /* Free any memory that was allocated (if we have sent   */
               /* all the data).                                        */
               if(PBAMEntryInfo->DataBufferSent == PBAMEntryInfo->DataBufferSize)
               {
                  DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Deleting data\n"));
                  if(PBAMEntryInfo->DataBuffer)
                  {
                     BTPS_FreeMemory(PBAMEntryInfo->DataBuffer);

                     PBAMEntryInfo->DataBuffer = NULL;
                  }

                  PBAMEntryInfo->CurrentOperation = coNone;
               }
            }
            else
            {
               /* Flag the new state we are entering.                   */
               PBAMEntryInfo->CurrentOperation = coPullPhonebook;

               /* Free any left-over data (just to be safe).            */
               if(PBAMEntryInfo->DataBuffer)
               {
                  BTPS_FreeMemory(PBAMEntryInfo->DataBuffer);

                  PBAMEntryInfo->DataBuffer = NULL;
               }

               /* Event needs to be dispatched.  Go ahead and format the*/
               /* event.                                                */
               if((Message = (PBAM_Pull_Phone_Book_Message_t *)BTPS_AllocateMemory(PBAM_PULL_PHONE_BOOK_EVENT_MESSAGE_SIZE(StringLength))) != NULL)
               {
                  BTPS_MemInitialize(&PBAMEventData, 0, sizeof(PBAM_Event_Data_t));

                  /* Format the event data.                             */
                  PBAMEventData.EventType                                        = petPullPhoneBook;
                  PBAMEventData.EventLength                                      = PBAM_PULL_PHONE_BOOK_EVENT_DATA_SIZE;

                  PBAMEventData.EventData.PullPhoneBookEventData.ConnectionID    = PBAMEntryInfo->ConnectionID;
                  PBAMEventData.EventData.PullPhoneBookEventData.ObjectName      = PullPhonebookIndicationData->ObjectName;
                  PBAMEventData.EventData.PullPhoneBookEventData.vCardFormat     = vCardFormat;
                  PBAMEventData.EventData.PullPhoneBookEventData.FilterLow       = PullPhonebookIndicationData->FilterLow;
                  PBAMEventData.EventData.PullPhoneBookEventData.FilterHigh      = PullPhonebookIndicationData->FilterHigh;
                  PBAMEventData.EventData.PullPhoneBookEventData.MaxListCount    = PullPhonebookIndicationData->MaxListCount;
                  PBAMEventData.EventData.PullPhoneBookEventData.ListStartOffset = PullPhonebookIndicationData->ListStartOffset;

                  /* Format the message.                                */
                  BTPS_MemInitialize(Message, 0, PBAM_PULL_PHONE_BOOK_EVENT_MESSAGE_SIZE(StringLength));

                  Message->MessageHeader.AddressID       = PBAMEntryInfo->ClientID;
                  Message->MessageHeader.MessageID       = MSG_GetNextMessageID();
                  Message->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PHONE_BOOK_ACCESS_MANAGER;
                  Message->MessageHeader.MessageFunction = PBAM_MESSAGE_FUNCTION_PULL_PHONE_BOOK_EVENT;
                  Message->MessageHeader.MessageLength   = (PBAM_PULL_PHONE_BOOK_EVENT_MESSAGE_SIZE(StringLength) - BTPM_MESSAGE_HEADER_SIZE);

                  Message->ServerID                      = PBAMEntryInfo->ServerID;
                  Message->ConnectionID                  = PBAMEntryInfo->ConnectionID;
                  Message->ObjectNameLength              = StringLength;
                  Message->vCardFormat                   = vCardFormat;
                  Message->FilterLow                     = PullPhonebookIndicationData->FilterLow;
                  Message->FilterHigh                    = PullPhonebookIndicationData->FilterHigh;
                  Message->MaxListCount                  = PullPhonebookIndicationData->MaxListCount;
                  Message->ListStartOffset               = PullPhonebookIndicationData->ListStartOffset;

                  if(StringLength)
                     BTPS_StringCopy(Message->ObjectName, PullPhonebookIndicationData->ObjectName);

                  /* Dispatch the event.                                */
                  DispatchPBAMServerEvent(PBAMEntryInfo, &PBAMEventData, (BTPM_Message_t *)Message);

                  BTPS_FreeMemory(Message);
               }
            }
         }
         else
         {
            /* Allocate the message.                                    */
            if((SizeMessage = (PBAM_Pull_Phone_Book_Size_Message_t *)BTPS_AllocateMemory(PBAM_PULL_PHONE_BOOK_SIZE_EVENT_MESSAGE_SIZE(StringLength))) != NULL)
            {
               /* Note the new state.                                   */
               PBAMEntryInfo->CurrentOperation = coPullPhonebookSize;

               /* Go ahead and format the event.                        */
               BTPS_MemInitialize(&PBAMEventData, 0, sizeof(PBAM_Event_Data_t));

               /* Format the event data.                                */
               PBAMEventData.EventType                                        = petPullPhoneBook;
               PBAMEventData.EventLength                                      = PBAM_PULL_PHONE_BOOK_SIZE_EVENT_DATA_SIZE;

               PBAMEventData.EventData.PullPhoneBookEventData.ConnectionID    = PBAMEntryInfo->ConnectionID;
               PBAMEventData.EventData.PullPhoneBookEventData.ObjectName      = PullPhonebookIndicationData->ObjectName;

               /* Format the message.                                   */
               BTPS_MemInitialize(SizeMessage, 0, PBAM_PULL_PHONE_BOOK_SIZE_EVENT_MESSAGE_SIZE(StringLength));

               SizeMessage->MessageHeader.AddressID       = PBAMEntryInfo->ClientID;
               SizeMessage->MessageHeader.MessageID       = MSG_GetNextMessageID();
               SizeMessage->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PHONE_BOOK_ACCESS_MANAGER;
               SizeMessage->MessageHeader.MessageFunction = PBAM_MESSAGE_FUNCTION_PULL_PHONE_BOOK_SIZE_EVENT;
               SizeMessage->MessageHeader.MessageLength   = (PBAM_PULL_PHONE_BOOK_SIZE_EVENT_MESSAGE_SIZE(StringLength) - BTPM_MESSAGE_HEADER_SIZE);

               SizeMessage->ServerID                      = PBAMEntryInfo->ServerID;
               SizeMessage->ConnectionID                  = PBAMEntryInfo->ConnectionID;
               SizeMessage->ObjectNameLength              = StringLength;

               BTPS_StringCopy(SizeMessage->ObjectName, PullPhonebookIndicationData->ObjectName);

               /* Dispatch the event.                                   */
               DispatchPBAMServerEvent(PBAMEntryInfo, &PBAMEventData, (BTPM_Message_t *)SizeMessage);

               BTPS_FreeMemory(SizeMessage);
            }
         }
      }
      else
      {
         if(!PBAMEntryInfo)
            DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to find PBAPID: %u\n", PullPhonebookIndicationData->PBAPID));
         else
            DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Prior Operation in Progress: %d\n", PBAMEntryInfo->CurrentOperation));

         _PBAM_Pull_Phonebook_Response(PullPhonebookIndicationData->PBAPID, (Byte_t)(PBAMEntryInfo?PBAP_OBEX_RESPONSE_NOT_ACCEPTABLE:PBAP_OBEX_RESPONSE_BAD_REQUEST), NULL, NULL, 0, NULL, NULL);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is used to process Bluetopia PBAP callback */
   /* Set Phonebook Indication event. The function accepts a valid Set  */
   /* Phonebook Indication structure.                                   */
static void ProcessSetPhonebookIndication(PBAP_Set_Phonebook_Indication_Data_t *SetPhonebookIndicationData)
{
   unsigned int                   StringLength;
   PBAM_Event_Data_t              PBAMEventData;
   PBAM_Server_Entry_t           *PBAMEntryInfo;
   PBAM_Set_Path_Option_t         PathOption;
   PBAM_Set_Phone_Book_Message_t *Message;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check if the event data parameter is not NULL.                    */
   if(SetPhonebookIndicationData)
   {
      switch(SetPhonebookIndicationData->PathOption)
      {
         case spUp:
            PathOption = pspUp;
            break;
         case spDown:
            PathOption = pspDown;
            break;
         default:
         case spRoot:
            PathOption = pspRoot;
            break;
      }

      /* Find the list entry by the PBAPID.                             */
      if(((PBAMEntryInfo = SearchPBAMServerEntryPBAPID(&PBAMServerEntryList, SetPhonebookIndicationData->PBAPID)) != NULL) && (PBAMEntryInfo->CurrentOperation == coNone))
      {
         if(SetPhonebookIndicationData->ObjectName)
            StringLength = BTPS_StringLength(SetPhonebookIndicationData->ObjectName) + 1;
         else
            StringLength = 0;

         /* Allocate the message.                                       */
         if((Message = (PBAM_Set_Phone_Book_Message_t *)BTPS_AllocateMemory(PBAM_SET_PHONE_BOOK_EVENT_MESSAGE_SIZE(StringLength))) != NULL)
         {
            /* Note the new state.                                      */
            PBAMEntryInfo->CurrentOperation = coSetPhonebook;

            /* Go ahead and format the event.                           */
            BTPS_MemInitialize(&PBAMEventData, 0, sizeof(PBAM_Event_Data_t));

            /* Format the event data.                                   */
            PBAMEventData.EventType                                        = petSetPhoneBook;
            PBAMEventData.EventLength                                      = PBAM_SET_PHONE_BOOK_EVENT_DATA_SIZE;

            /* Format the message.                                      */
            BTPS_MemInitialize(Message, 0, PBAM_SET_PHONE_BOOK_EVENT_MESSAGE_SIZE(StringLength));

            Message->MessageHeader.AddressID       = PBAMEntryInfo->ClientID;
            Message->MessageHeader.MessageID       = MSG_GetNextMessageID();
            Message->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PHONE_BOOK_ACCESS_MANAGER;
            Message->MessageHeader.MessageFunction = PBAM_MESSAGE_FUNCTION_SET_PHONE_BOOK_EVENT;
            Message->MessageHeader.MessageLength   = (PBAM_SET_PHONE_BOOK_EVENT_MESSAGE_SIZE(StringLength) - BTPM_MESSAGE_HEADER_SIZE);

            Message->ServerID                      = PBAMEntryInfo->ServerID;
            Message->ConnectionID                  = PBAMEntryInfo->ConnectionID;
            Message->PathOption                    = PathOption;
            Message->ObjectNameLength              = StringLength;

            if(StringLength)
               BTPS_StringCopy(Message->ObjectName, SetPhonebookIndicationData->ObjectName);

            /* Dispatch the event.                                      */
            DispatchPBAMServerEvent(PBAMEntryInfo, &PBAMEventData, (BTPM_Message_t *)Message);

            BTPS_FreeMemory(Message);
         }
      }
      else
      {
         if(!PBAMEntryInfo)
            DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to find PBAPID: %u\n", SetPhonebookIndicationData->PBAPID));
         else
            DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Prior Operation in Progress: %d\n", PBAMEntryInfo->CurrentOperation));

         _PBAM_Set_Phonebook_Response(SetPhonebookIndicationData->PBAPID, (Byte_t)(PBAMEntryInfo?PBAP_OBEX_RESPONSE_NOT_ACCEPTABLE:PBAP_OBEX_RESPONSE_BAD_REQUEST));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is used to process Bluetopia PBAP callback */
   /* Pull vCard Listing Indication event. The function accepts a valid */
   /* Pull vCard Listing Indication structure.                          */
static void ProcessPullvCardListingIndication(PBAP_Pull_vCard_Listing_Indication_Data_t *PullvCardListingIndicationData)
{
   unsigned int                            DataLength;
   unsigned int                            ObjectNameLength;
   unsigned int                            SearchValueLength;
   PBAM_List_Order_t                       ListOrder;
   PBAM_Event_Data_t                       PBAMEventData;
   PBAM_Server_Entry_t                    *PBAMEntryInfo;
   PBAM_Search_Attribute_t                 SearchAttribute;
   PBAM_Pull_vCard_Listing_Message_t      *Message;
   PBAM_Pull_vCard_Listing_Size_Message_t *SizeMessage;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check if the event data parameter is not NULL.                    */
   if(PullvCardListingIndicationData)
   {
      /* Find the list entry by the PBAPID.                             */
      if(((PBAMEntryInfo = SearchPBAMServerEntryPBAPID(&PBAMServerEntryList, PullvCardListingIndicationData->PBAPID)) != NULL) && (PBAMEntryInfo->CurrentOperation == coNone || PBAMEntryInfo->CurrentOperation == coPullvCardListing))
      {
         if(PullvCardListingIndicationData->ObjectName)
            ObjectNameLength = BTPS_StringLength(PullvCardListingIndicationData->ObjectName) + 1;
         else
            ObjectNameLength = 0;

         if(PullvCardListingIndicationData->SearchValue)
            SearchValueLength = BTPS_StringLength(PullvCardListingIndicationData->SearchValue) + 1;
         else
            SearchValueLength = 0;

         /* Convert data types.                                         */
         switch(PullvCardListingIndicationData->ListOrder)
         {
            case loIndexed:
               ListOrder = ploIndexed;
               break;
            case loAlphabetical:
               ListOrder = ploAlphabetical;
               break;
            case loPhonetical:
               ListOrder = ploPhonetical;
               break;
            case loDefault:
            default:
               ListOrder = ploDefault;
               break;
         }

         switch(PullvCardListingIndicationData->SearchAttribute)
         {
            case saName:
               SearchAttribute = psaName;
               break;
            case saNumber:
               SearchAttribute = psaNumber;
               break;
            case saSound:
               SearchAttribute = psaSound;
               break;
            case saDefault:
            default:
               SearchAttribute = psaDefault;
               break;
         }

         /* Determine whether this is a data or size event.             */
         if(PullvCardListingIndicationData->MaxListCount)
         {
            /* Determine if we need to send more response data.         */
            if((PBAMEntryInfo->CurrentOperation == coPullvCardListing) && (PBAMEntryInfo->DataBufferSize) && (PBAMEntryInfo->DataBuffer))
            {
               /* Calculate the remaining data to send.                 */
               DataLength = PBAMEntryInfo->DataBufferSize - PBAMEntryInfo->DataBufferSent;

               if(_PBAM_Pull_vCard_Listing_Response(PBAMEntryInfo->PBAPID, (Byte_t)(PBAMEntryInfo->DataFinal?PBAP_OBEX_RESPONSE_OK:PBAP_OBEX_RESPONSE_CONTINUE), NULL, NULL, DataLength, &(PBAMEntryInfo->DataBuffer[PBAMEntryInfo->DataBufferSent]), &DataLength) == 0)
               {
                  PBAMEntryInfo->DataBufferSent += DataLength;

                  DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Submitting continuation data: %d, %d\n", (int)PBAMEntryInfo->PBAPID, PBAMEntryInfo->DataBufferSent));
               }
               else
               {
                  DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Error submitting continuation data: %d, %d\n", (int)PBAMEntryInfo->PBAPID, PBAMEntryInfo->DataBufferSent));

                  /* Error submitting response.  Not sure what we can do*/
                  /* here.                                              */
                  _PBAM_Pull_vCard_Listing_Response(PBAMEntryInfo->PBAPID, (Byte_t)(PBAP_OBEX_RESPONSE_SERVICE_UNAVAILABLE), NULL, NULL, 0, NULL, NULL);

                  /* Flag that we have sent all the data (so it can be  */
                  /* freed below).                                      */
                  PBAMEntryInfo->DataBufferSent   = PBAMEntryInfo->DataBufferSize;

                  /* Flag that there is no longer an operation in       */
                  /* progress.                                          */
                  PBAMEntryInfo->CurrentOperation = coNone;
               }

               /* Free any memory that was allocated (if we have sent   */
               /* all the data).                                        */
               if(PBAMEntryInfo->DataBufferSent == PBAMEntryInfo->DataBufferSize)
               {
                  if(PBAMEntryInfo->DataBuffer)
                  {
                     BTPS_FreeMemory(PBAMEntryInfo->DataBuffer);

                     PBAMEntryInfo->DataBuffer = NULL;
                  }

                  PBAMEntryInfo->CurrentOperation = coNone;
               }
            }
            else
            {
               /* Flag the new state we are entering.                   */
               PBAMEntryInfo->CurrentOperation = coPullvCardListing;

               /* Free any left-over data (just to be safe).            */
               if(PBAMEntryInfo->DataBuffer)
               {
                  BTPS_FreeMemory(PBAMEntryInfo->DataBuffer);

                  PBAMEntryInfo->DataBuffer = NULL;
               }

               /* Event needs to be dispatched.  Go ahead and format the*/
               /* event.                                                */
               if((Message = (PBAM_Pull_vCard_Listing_Message_t *)BTPS_AllocateMemory(PBAM_PULL_VCARD_LISTING_EVENT_MESSAGE_SIZE(ObjectNameLength, SearchValueLength))) != NULL)
               {
                  BTPS_MemInitialize(&PBAMEventData, 0, sizeof(PBAM_Event_Data_t));

                  /* Format the event data.                             */
                  PBAMEventData.EventType                                           = petPullvCardListing;
                  PBAMEventData.EventLength                                         = PBAM_PULL_VCARD_LISTING_EVENT_DATA_SIZE;

                  PBAMEventData.EventData.PullvCardListingEventData.ConnectionID    = PBAMEntryInfo->ConnectionID;
                  PBAMEventData.EventData.PullvCardListingEventData.ObjectName      = PullvCardListingIndicationData->ObjectName;
                  PBAMEventData.EventData.PullvCardListingEventData.ListOrder       = ListOrder;
                  PBAMEventData.EventData.PullvCardListingEventData.SearchAttribute = SearchAttribute;
                  PBAMEventData.EventData.PullvCardListingEventData.SearchValue     = PullvCardListingIndicationData->SearchValue;
                  PBAMEventData.EventData.PullvCardListingEventData.MaxListCount    = PullvCardListingIndicationData->MaxListCount;
                  PBAMEventData.EventData.PullvCardListingEventData.ListStartOffset = PullvCardListingIndicationData->ListStartOffset;

                  /* Format the message.                                */
                  BTPS_MemInitialize(Message, 0, PBAM_PULL_VCARD_LISTING_EVENT_MESSAGE_SIZE(ObjectNameLength, SearchValueLength));

                  Message->MessageHeader.AddressID       = PBAMEntryInfo->ClientID;
                  Message->MessageHeader.MessageID       = MSG_GetNextMessageID();
                  Message->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PHONE_BOOK_ACCESS_MANAGER;
                  Message->MessageHeader.MessageFunction = PBAM_MESSAGE_FUNCTION_PULL_VCARD_LISTING_EVENT;
                  Message->MessageHeader.MessageLength   = (PBAM_PULL_VCARD_LISTING_EVENT_MESSAGE_SIZE(ObjectNameLength, SearchValueLength) - BTPM_MESSAGE_HEADER_SIZE);

                  Message->ServerID                      = PBAMEntryInfo->ServerID;
                  Message->ConnectionID                  = PBAMEntryInfo->ConnectionID;
                  Message->ObjectNameLength              = ObjectNameLength;
                  Message->ListOrder                     = ListOrder;
                  Message->SearchAttribute               = SearchAttribute;
                  Message->SearchValueLength             = SearchValueLength;
                  Message->MaxListCount                  = PullvCardListingIndicationData->MaxListCount;
                  Message->ListStartOffset               = PullvCardListingIndicationData->ListStartOffset;

                  if(ObjectNameLength)
                     BTPS_MemCopy(Message->VariableData, PullvCardListingIndicationData->ObjectName, ObjectNameLength);

                  if(SearchValueLength)
                     BTPS_MemCopy(Message->VariableData + ObjectNameLength, PullvCardListingIndicationData->SearchValue, SearchValueLength);

                  /* Dispatch the event.                                */
                  DispatchPBAMServerEvent(PBAMEntryInfo, &PBAMEventData, (BTPM_Message_t *)Message);

                  BTPS_FreeMemory(Message);
               }
            }
         }
         else
         {
            /* Allocate the message.                                    */
            if((SizeMessage = (PBAM_Pull_vCard_Listing_Size_Message_t *)BTPS_AllocateMemory(PBAM_PULL_VCARD_LISTING_SIZE_EVENT_MESSAGE_SIZE(ObjectNameLength))) != NULL)
            {
               /* Note the new state.                                   */
               PBAMEntryInfo->CurrentOperation = coPullvCardListingSize;

               /* Go ahead and format the event.                        */
               BTPS_MemInitialize(&PBAMEventData, 0, sizeof(PBAM_Event_Data_t));

               /* Format the event data.                                */
               PBAMEventData.EventType                                        = petPullvCardListing;
               PBAMEventData.EventLength                                      = PBAM_PULL_VCARD_LISTING_SIZE_EVENT_DATA_SIZE;

               PBAMEventData.EventData.PullvCardListingEventData.ConnectionID    = PBAMEntryInfo->ConnectionID;
               PBAMEventData.EventData.PullvCardListingEventData.ObjectName      = PullvCardListingIndicationData->ObjectName;

               /* Format the message.                                   */
               BTPS_MemInitialize(SizeMessage, 0, PBAM_PULL_VCARD_LISTING_SIZE_EVENT_MESSAGE_SIZE(ObjectNameLength));

               SizeMessage->MessageHeader.AddressID       = PBAMEntryInfo->ClientID;
               SizeMessage->MessageHeader.MessageID       = MSG_GetNextMessageID();
               SizeMessage->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PHONE_BOOK_ACCESS_MANAGER;
               SizeMessage->MessageHeader.MessageFunction = PBAM_MESSAGE_FUNCTION_PULL_VCARD_LISTING_SIZE_EVENT;
               SizeMessage->MessageHeader.MessageLength   = (PBAM_PULL_VCARD_LISTING_SIZE_EVENT_MESSAGE_SIZE(ObjectNameLength) - BTPM_MESSAGE_HEADER_SIZE);

               SizeMessage->ServerID                      = PBAMEntryInfo->ServerID;
               SizeMessage->ConnectionID                  = PBAMEntryInfo->ConnectionID;
               SizeMessage->ObjectNameLength              = ObjectNameLength;

               if(ObjectNameLength)
                  BTPS_StringCopy(SizeMessage->ObjectName, PullvCardListingIndicationData->ObjectName);

               /* Dispatch the event.                                   */
               DispatchPBAMServerEvent(PBAMEntryInfo, &PBAMEventData, (BTPM_Message_t *)SizeMessage);

               BTPS_FreeMemory(SizeMessage);
            }
         }
      }
      else
      {
         if(!PBAMEntryInfo)
            DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to find PBAPID: %u\n", PullvCardListingIndicationData->PBAPID));
         else
            DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Prior Operation in Progress: %d\n", PBAMEntryInfo->CurrentOperation));

         _PBAM_Pull_vCard_Listing_Response(PullvCardListingIndicationData->PBAPID, (Byte_t)(PBAMEntryInfo?PBAP_OBEX_RESPONSE_NOT_ACCEPTABLE:PBAP_OBEX_RESPONSE_BAD_REQUEST), NULL, NULL, 0, NULL, NULL);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is used to process Bluetopia PBAP callback */
   /* Pull vCard Entry Indication event. The function accepts a valid   */
   /* Pull vCard Entry Indication structure.                            */
static void ProcessPullvCardEntryIndication(PBAP_Pull_vCard_Entry_Indication_Data_t *PullvCardEntryIndicationData)
{
   unsigned int               DataLength;
   unsigned int               StringLength;
   PBAM_Event_Data_t          PBAMEventData;
   PBAM_VCard_Format_t        vCardFormat;
   PBAM_Server_Entry_t       *PBAMEntryInfo;
   PBAM_Pull_vCard_Message_t *Message;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check if the event data parameter is not NULL.                    */
   if(PullvCardEntryIndicationData)
   {
      /* Find the list entry by the PBAPID.                             */
      if(((PBAMEntryInfo = SearchPBAMServerEntryPBAPID(&PBAMServerEntryList, PullvCardEntryIndicationData->PBAPID)) != NULL) && (PBAMEntryInfo->CurrentOperation == coNone || PBAMEntryInfo->CurrentOperation == coPullvCard))
      {
         if(PullvCardEntryIndicationData->ObjectName)
            StringLength = BTPS_StringLength(PullvCardEntryIndicationData->ObjectName) + 1;
         else
            StringLength = 0;

         /* Convert data types.                                         */
         switch(PullvCardEntryIndicationData->Format)
         {
            case pfvCard21:
               vCardFormat = pmvCard21;
               break;
            case pfvCard30:
               vCardFormat = pmvCard30;
               break;
            case pfDefault:
            default:
               vCardFormat = pmDefault;
               break;
         }

         /* Determine if we need to send more response data.            */
         if((PBAMEntryInfo->CurrentOperation == coPullvCard) && (PBAMEntryInfo->DataBufferSize) && (PBAMEntryInfo->DataBuffer))
         {
            /* Calculate the remaining data to send.                    */
            DataLength = PBAMEntryInfo->DataBufferSize - PBAMEntryInfo->DataBufferSent;

            if(_PBAM_Pull_vCard_Entry_Response(PBAMEntryInfo->PBAPID, (Byte_t)(PBAMEntryInfo->DataFinal?PBAP_OBEX_RESPONSE_OK:PBAP_OBEX_RESPONSE_CONTINUE), DataLength, &(PBAMEntryInfo->DataBuffer[PBAMEntryInfo->DataBufferSent]), &DataLength) == 0)
            {
               PBAMEntryInfo->DataBufferSent += DataLength;

               DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Submitting continuation data: %d, %d\n", (int)PBAMEntryInfo->PBAPID, PBAMEntryInfo->DataBufferSent));
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Error submitting continuation data: %d, %d\n", (int)PBAMEntryInfo->PBAPID, PBAMEntryInfo->DataBufferSent));

               /* Error submitting response.  Not sure what we can do   */
               /* here.                                                 */
               _PBAM_Pull_vCard_Entry_Response(PBAMEntryInfo->PBAPID, (Byte_t)(PBAP_OBEX_RESPONSE_SERVICE_UNAVAILABLE), 0, NULL, NULL);

               /* Flag that we have sent all the data (so it can be     */
               /* freed below).                                         */
               PBAMEntryInfo->DataBufferSent   = PBAMEntryInfo->DataBufferSize;

               /* Flag that there is no longer an operation in progress.*/
               PBAMEntryInfo->CurrentOperation = coNone;
            }

            /* Free any memory that was allocated (if we have sent all  */
            /* the data).                                               */
            if(PBAMEntryInfo->DataBufferSent == PBAMEntryInfo->DataBufferSize)
            {
               if(PBAMEntryInfo->DataBuffer)
               {
                  BTPS_FreeMemory(PBAMEntryInfo->DataBuffer);

                  PBAMEntryInfo->DataBuffer = NULL;
               }


               PBAMEntryInfo->CurrentOperation = coNone;
            }
         }
         else
         {
            /* Flag the new state we are entering.                      */
            PBAMEntryInfo->CurrentOperation = coPullvCard;

            /* Free any left-over data (just to be safe).               */
            if(PBAMEntryInfo->DataBuffer)
            {
               BTPS_FreeMemory(PBAMEntryInfo->DataBuffer);

               PBAMEntryInfo->DataBuffer = NULL;
            }

            /* Allocate the message.                                    */
            if((Message = (PBAM_Pull_vCard_Message_t *)BTPS_AllocateMemory(PBAM_PULL_VCARD_EVENT_MESSAGE_SIZE(StringLength))) != NULL)
            {
               /* Go ahead and format the event.                        */
               BTPS_MemInitialize(&PBAMEventData, 0, sizeof(PBAM_Event_Data_t));

               /* Format the event data.                                */
               PBAMEventData.EventType                                 = petPullvCard;
               PBAMEventData.EventLength                               = PBAM_PULL_VCARD_EVENT_DATA_SIZE;

               PBAMEventData.EventData.PullvCardEventData.ConnectionID = PBAMEntryInfo->ConnectionID;
               PBAMEventData.EventData.PullvCardEventData.FilterLow    = PullvCardEntryIndicationData->FilterLow;
               PBAMEventData.EventData.PullvCardEventData.FilterHigh   = PullvCardEntryIndicationData->FilterHigh;
               PBAMEventData.EventData.PullvCardEventData.Format       = vCardFormat;
               PBAMEventData.EventData.PullvCardEventData.ObjectName   = PullvCardEntryIndicationData->ObjectName;

               /* Format the message.                                   */
               BTPS_MemInitialize(Message, 0, PBAM_PULL_VCARD_EVENT_MESSAGE_SIZE(StringLength));

               Message->MessageHeader.AddressID       = PBAMEntryInfo->ClientID;
               Message->MessageHeader.MessageID       = MSG_GetNextMessageID();
               Message->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PHONE_BOOK_ACCESS_MANAGER;
               Message->MessageHeader.MessageFunction = PBAM_MESSAGE_FUNCTION_PULL_VCARD_EVENT;
               Message->MessageHeader.MessageLength   = (PBAM_PULL_VCARD_EVENT_MESSAGE_SIZE(StringLength) - BTPM_MESSAGE_HEADER_SIZE);

               Message->ServerID                      = PBAMEntryInfo->ServerID;
               Message->ConnectionID                  = PBAMEntryInfo->ConnectionID;
               Message->vCardFormat                   = vCardFormat;
               Message->FilterLow                     = PullvCardEntryIndicationData->FilterLow;
               Message->FilterHigh                    = PullvCardEntryIndicationData->FilterHigh;
               Message->ObjectNameLength              = StringLength;

               if(StringLength)
                  BTPS_StringCopy(Message->ObjectName, PullvCardEntryIndicationData->ObjectName);

               /* Dispatch the event.                                   */
               DispatchPBAMServerEvent(PBAMEntryInfo, &PBAMEventData, (BTPM_Message_t *)Message);

               BTPS_FreeMemory(Message);
            }
         }
      }
      else
      {
         if(!PBAMEntryInfo)
            DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to find PBAPID: %u\n", PullvCardEntryIndicationData->PBAPID));
         else
            DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Prior Operation in Progress: %d\n", PBAMEntryInfo->CurrentOperation));

         _PBAM_Pull_vCard_Entry_Response(PullvCardEntryIndicationData->PBAPID, (Byte_t)(PBAMEntryInfo?PBAP_OBEX_RESPONSE_NOT_ACCEPTABLE:PBAP_OBEX_RESPONSE_BAD_REQUEST), 0, NULL, NULL);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is used to process all Bluetopia event     */
   /* callbacks. The function takes a valid event data structure.       */
static void ProcessPBAPEvent(PBAM_Phone_Book_Access_Event_Data_t *PBAPEventData)
{
   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First make sure that the input parameter appears to be semi-valid.*/
   if(PBAPEventData)
   {
      /* Process the event based on the event type.                     */
      switch(PBAPEventData->EventType)
      {
         case etPBAP_Open_Port_Confirmation:
            DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Open Confirmation\n"));

            ProcessOpenConfirmationEvent(&(PBAPEventData->EventData.OpenPortConfirmationData));
            break;
         case etPBAP_Close_Port_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Close Indication\n"));

            ProcessCloseIndicationEvent(&(PBAPEventData->EventData.ClosePortIndicationData));
            break;
         case etPBAP_Abort_Confirmation:
            DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Abort Confirmation\n"));

            ProcessAbortConfirmationEvent(&(PBAPEventData->EventData.AbortConfirmationData));
            break;
         case etPBAP_Pull_Phonebook_Confirmation:
            DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Pull Phone Book Confirmation\n"));

            ProcessPullPhoneBookConfirmationEvent(&(PBAPEventData->EventData.PullPhonebookConfirmationData));
            break;
         case etPBAP_Set_Phonebook_Confirmation:
            DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Set Phone Book Confirmation\n"));

            ProcessSetPhoneBookConfirmationEvent(&(PBAPEventData->EventData.SetPhonebookConfirmationData));
            break;
         case etPBAP_Pull_vCard_Listing_Confirmation:
            DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Pull vCard Listing Confirmation\n"));

            ProcessPullvCardListingConfirmationEvent(&(PBAPEventData->EventData.PullvCardListingConfirmationData));
            break;
         case etPBAP_Pull_vCard_Entry_Confirmation:
            DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Pull vCard Entry Confirmation\n"));

            ProcessPullvCardEntryConfirmationEvent(&(PBAPEventData->EventData.PullvCardEntryConfirmationData));
            break;
         case etPBAP_Open_Port_Request_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Open Port Request Indication\n"));

            ProcessOpenRequestIndication(&(PBAPEventData->EventData.OpenPortRequestIndicationData));
            break;
         case etPBAP_Open_Port_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Open Port Indication\n"));

            ProcessOpenIndication(&(PBAPEventData->EventData.OpenPortIndicationData));
            break;
         case etPBAP_Abort_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Abort Indication\n"));

            ProcessAbortIndication(&(PBAPEventData->EventData.AbortIndicationData));
            break;
         case etPBAP_Pull_Phonebook_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Pull Phonebook Indication\n"));

            ProcessPullPhonebookIndication(&(PBAPEventData->EventData.PullPhonebookIndicationData));
            break;
         case etPBAP_Set_Phonebook_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Set Phonebook Indication\n"));

            ProcessSetPhonebookIndication(&(PBAPEventData->EventData.SetPhonebookIndicationData));
            break;
         case etPBAP_Pull_vCard_Listing_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Pull vCard Listing Indication\n"));

            ProcessPullvCardListingIndication(&(PBAPEventData->EventData.PullvCardListingIndicationData));
            break;
         case etPBAP_Pull_vCard_Entry_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Pull vCard Entry Indication\n"));

            ProcessPullvCardEntryIndication(&(PBAPEventData->EventData.PullvCardEntryIndicationData));
            break;
         default:
            /* Invalid/Unknown Event.                                   */
            DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_CRITICAL), ("Invalid/Unknown Event Type: %d\n", PBAPEventData->EventType));
            break;
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_CRITICAL), ("Invalid Phone Book Access Manager Event Data\n"));
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the dispatch callback function that is  */
   /* registered to process PBAP notification events.                   */
static void BTPSAPI BTPMDispatchCallback_PBAP(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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
            /* Double check that this is an SPP Event Update.           */
            if(((PBAM_Update_Data_t *)CallbackParameter)->UpdateType == utPhoneBookAccessEvent)
            {
               /* Process the Notification.                             */
               ProcessPBAPEvent(&(((PBAM_Update_Data_t *)CallbackParameter)->UpdateData.PhoneBookAccessEventData));
            }

            /* Release the lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
      }

      /* Go ahead and free the memory because we are finished with it.  */
      BTPS_FreeMemory(CallbackParameter);
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* process Device Manager (DEVM) Status Events (for out-going        */
   /* connection management).                                           */
static void ProcessDEVMStatusEvent(BD_ADDR_t BluetoothAddress, int Status)
{
   int                                 Result;
   PBAM_Entry_Info_t                  *PBAMEntryInfo;
   PBAP_Open_Port_Confirmation_Data_t  FailedOpenConfirmationData;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter %d\n", Status));

   /* First, determine if we are tracking a connection to this device.  */
   if((((PBAMEntryInfo = SearchPBAMEntryInfoEntry(&PBAMEntryInfoList, &BluetoothAddress, 0)) != NULL)) && (PBAMEntryInfo->ConnectionState != csConnected))
   {
      /* Process the status event.                                      */

      /* Initialize common connection event members.                    */
      BTPS_MemInitialize(&FailedOpenConfirmationData, 0, sizeof(PBAP_Open_Port_Confirmation_Data_t));

      FailedOpenConfirmationData.BD_ADDR = BluetoothAddress;

      if(Status)
      {
         /* Disconnect the device.                                      */
         DEVM_DisconnectRemoteDevice(BluetoothAddress, 0);

         /* Connection Failed.                                          */

         /* Map the status to a known status.                           */
         switch(Status)
         {
            case BTPM_ERROR_CODE_DEVICE_AUTHENTICATION_FAILED:
            case BTPM_ERROR_CODE_DEVICE_ENCRYPTION_FAILED:
               FailedOpenConfirmationData.PBAPConnectStatus = PBAP_OPEN_STATUS_CONNECTION_REFUSED;
               break;
            case BTPM_ERROR_CODE_DEVICE_CONNECTION_FAILED:
            case BTPM_ERROR_CODE_DEVICE_CONNECTION_RETRIES_EXCEEDED:
               FailedOpenConfirmationData.PBAPConnectStatus = PBAP_OPEN_STATUS_CONNECTION_TIMEOUT;
               break;
            default:
               FailedOpenConfirmationData.PBAPConnectStatus = PBAP_OPEN_STATUS_UNKNOWN_ERROR;
               break;
         }

         /* This function will delete the PBAM Info entry from the list.*/
         ProcessOpenConfirmationEvent(&FailedOpenConfirmationData);

         /* Flag that the connection has been deleted.                  */
         PBAMEntryInfo = NULL;
      }
      else
      {
         /* Connection succeeded.                                       */

         /* Move the state to the connecting state.                     */
         PBAMEntryInfo->ConnectionState = csConnecting;

         if(((Result = _PBAM_Connect_Remote_Device(BluetoothAddress, PBAMEntryInfo->RemoteServerPort)) <= 0))
         {
            /* Error opening device.                                    */

            DEVM_DisconnectRemoteDevice(BluetoothAddress, 0);

            FailedOpenConfirmationData.PBAPConnectStatus = PBAP_OPEN_STATUS_UNKNOWN_ERROR;

            /* This function will delete the PBAM Info entry from the   */
            /* list.                                                    */
            ProcessOpenConfirmationEvent(&FailedOpenConfirmationData);

            /* Flag that the connection has been deleted.               */
            PBAMEntryInfo = NULL;
         }
         else
         {
            /* Note the connection handle                               */
            PBAMEntryInfo->PBAPID = Result;

            DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Connection open %d\n", PBAMEntryInfo->PBAPID));
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function represents the Message Group Handler       */
   /* function that is installed to process all Phone Book Access       */
   /* Manager Messages.                                                 */
static void BTPSAPI PhoneBookAccessManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the Message specified appears to be semi-valid.   */
   if(Message)
   {
      /* Next, check to see if this message is specified for our        */
      /* specific Message Group.                                        */
      if(Message->MessageHeader.MessageGroup == BTPM_MESSAGE_GROUP_PHONE_BOOK_ACCESS_MANAGER)
      {
         DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Phone Book Access Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

         /* Message is destined for us, make sure we are still          */
         /* processing messages.                                        */
         if(Initialized)
         {
            /* Check to see if this message is a Phone Book Access      */
            /* Manager defined Message. If it is it will be within the  */
            /* range:                                                   */
            /*                                                          */
            /*    - BTPM_MESSAGE_FUNCTION_MINIMUM                       */
            /*    - BTPM_MESSAGE_FUNCTION_MAXIMUM                       */
            /*                                                          */
            /* See BTPMMSGT.h for more information on message functions */
            /* that are defined outside of this range.                  */
            if((Message->MessageHeader.MessageFunction >= BTPM_MESSAGE_FUNCTION_MINIMUM) && (Message->MessageHeader.MessageFunction <= BTPM_MESSAGE_FUNCTION_MAXIMUM))
            {
               /* Still processing, go ahead and post the message to the*/
               /* Phone Book Access Manager thread.                     */
               if(!BTPM_QueueMailboxCallback(BTPMDispatchCallback_PBAM, (void *)Message))
               {
                  DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue Phone Book Access Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

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
                        DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue Hands Free Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));
                     }
                  }
               }
            }
         }
         else
         {
            /* Check to see if this message is an Phone Book Access     */
            /* Manager defined Message. If it is it will be within the  */
            /* range:                                                   */
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
         DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Non Phone Book Access Manager Message: 0x%08X, 0x%08X\n", Message->MessageHeader.MessageGroup, Message->MessageHeader.MessageFunction));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for initializing/cleaning up*/
   /* the Bluetopia Platform Manager Phone Book Access Manager module.  */
   /* This function should be registered with the Bluetopia Platform    */
   /* Manager module handler and will be called when the Platform       */
   /* Manager is initialized (or shut down).                            */
void BTPSAPI PBAM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData)
{
   int Result;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %d\n", Initialize));

   /* First, determine if the request is to initialize or shut down.    */
   if(Initialize)
   {
      DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Initialization Request\n"));

      if(!Initialized)
      {
         DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing Phone Book Access Manager\n"));

         /* Next, let's attempt to register our Message Group Handler to*/
         /* process Phone Book Access Manager messages.                 */
         if(!MSG_RegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_PHONE_BOOK_ACCESS_MANAGER, PhoneBookAccessManagerGroupHandler, NULL))
         {
            /* Initialize the actual Phone Book Access Manager          */
            /* implementation module (this is the module that is        */
            /* actually responsible for actually implementing the PBAM  */
            /* Manager functionality - this module is just the framework*/
            /* shell).                                                  */
            if(!(Result = _PBAM_Initialize((PBAM_Initialization_Info_t *)InitializationData)))
            {
               /* Note the current Power State.                         */
               CurrentPowerState = (Boolean_t)((DEVM_QueryDevicePowerState() > 0)?TRUE:FALSE);

               /* Go ahead and flag that this module is initialized.    */
               Initialized = TRUE;
            }
         }
         else
            Result = BTPM_ERROR_CODE_UNABLE_TO_REGISTER_HANDLER;

         /* If an error occurred then we need to free all resources that*/
         /* were allocated.                                             */
         if(Result < 0)
         {
            MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_PHONE_BOOK_ACCESS_MANAGER);
         }
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("PBAM Manager Already initialized\n"));
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Shutdown Request\n"));

      if(Initialized)
      {
         /* First, let's make sure that we un-register our Message      */
         /* Group Handler (so that we do not process any more incoming  */
         /* messages).                                                  */
         MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_PHONE_BOOK_ACCESS_MANAGER);

         /* Wait for access to the Mutex that guards access to this     */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Make sure we inform the Phone Book Access Manager        */
            /* Implementation that we are shutting down.                */
            _PBAM_Cleanup();

            /* Make sure that the Phone Book Access entry information   */
            /* list is empty.                                           */
            FreePBAMEntryInfoList(&PBAMEntryInfoList);

            /* Flag that the resources are no longer allocated.         */
            CurrentPowerState = FALSE;

            /* Flag that this module is no longer initialized.          */
            Initialized       = FALSE;

            DEVM_ReleaseLock();
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing asynchronous */
   /* Device Manager (DEVM) Events (including Power On/Off events).     */
   /* This function should be registered with the Bluetopia Platform    */
   /* Manager module handler and will be called when an asynchronous    */
   /* Device Manager event is dispatched.                               */
void BTPSAPI PBAM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData)
{
   int                Result;
   PBAM_Entry_Info_t *PBAMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Phone Book Access Manager has been  */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         switch(EventData->EventType)
         {
            case detDevicePoweredOn:
               DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power On Occurred.\n"));

               /* Note the new Power State.                             */
               CurrentPowerState = TRUE;

               if((Result = DEVM_QueryDeviceBluetoothStackID()) > 0)
                  _PBAM_SetBluetoothStackID((unsigned int)Result);
               break;
            case detDevicePoweringOff:
            case detDevicePoweredOff:
               DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power Off Occurred.\n"));

               /* Power off event, let's loop through ALL the registered*/
               /* Phone Book Access entries and set any events that have*/
               /* synchronous operations pending.                       */
               PBAMEntryInfo = PBAMEntryInfoList;

               while(PBAMEntryInfo)
               {
                  /* Check to see if there is a synchronous open        */
                  /* operation.                                         */
                  if((PBAMEntryInfo->Flags & PBAM_ENTRY_INFO_FLAGS_SYNCHRONOUS_CONNECT_REQUEST) && (PBAMEntryInfo->ConnectionEvent))
                  {
                     PBAMEntryInfo->ConnectionStatus = PBAM_DEVICE_CONNECTION_STATUS_FAILURE_DEVICE_POWER_OFF;

                     BTPS_SetEvent(PBAMEntryInfo->ConnectionEvent);
                  }

                  /* Make sure we inform the PBAP Profile that we are   */
                  /* disconnecting a device (so it can clean up).       */
                  if(PBAMEntryInfo->PBAPID)
                  {
                     if(PBAMEntryInfo->RemoteServerPort)
                        _PBAM_Disconnect_Device(PBAMEntryInfo->PBAPID);
                  }

                  PBAMEntryInfo = PBAMEntryInfo->NextPBAMEntryInfoPtr;
               }

               /* Note the new Power State.                             */
               CurrentPowerState   = FALSE;

               /* Free the PBAM Entry Information List.                 */
               FreePBAMEntryInfoList(&PBAMEntryInfoList);

               /* Free the Server Entry List.                           */
               FreePBAMServerEntryList(&PBAMServerEntryList);

               /* Flag that the stack is no longer active.              */
               _PBAM_SetBluetoothStackID(0);
               break;
            case detRemoteDeviceAuthenticationStatus:
               /* Authentication Status, process the Status Event.      */
               ProcessDEVMStatusEvent(EventData->EventData.RemoteDeviceAuthenticationStatusEventData.RemoteDeviceAddress, EventData->EventData.RemoteDeviceAuthenticationStatusEventData.Status);
               break;
            case detRemoteDeviceEncryptionStatus:
               /* Encryption Status, process the Status Event.          */
               ProcessDEVMStatusEvent(EventData->EventData.RemoteDeviceEncryptionStatusEventData.RemoteDeviceAddress, EventData->EventData.RemoteDeviceEncryptionStatusEventData.Status);
               break;
            case detRemoteDeviceConnectionStatus:
               /* Connection Status, process the Status Event.          */
               ProcessDEVMStatusEvent(EventData->EventData.RemoteDeviceConnectionStatusEventData.RemoteDeviceAddress, EventData->EventData.RemoteDeviceConnectionStatusEventData.Status);
               break;
            default:
               /* Do nothing.                                           */
               DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("DEVM event %d\n", EventData->EventType));
               break;
         }

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow the caller the ability*/
   /* to notify the Phone Book Access Manager of a specific Update      */
   /* Event. The Phone Book Access Manager can then take the correct    */
   /* action to process the update.                                     */
Boolean_t PBAM_NotifyUpdate(PBAM_Update_Data_t *UpdateData)
{
   Boolean_t ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, make sure that the input parameters appear to be           */
   /* semi-valid.                                                       */
   if(UpdateData)
   {
      switch(UpdateData->UpdateType)
      {
         case utPhoneBookAccessEvent:
            DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Queueing Phone Book Access Event: %d\n", UpdateData->UpdateData.PhoneBookAccessEventData.EventType));

            /* Simply Queue the Dispatch Data.                          */
            /* * NOTE * To avoid copying again, we will simply submit   */
            /*          the entire Notification Message.                */
            ret_val = BTPM_QueueMailboxCallback(BTPMDispatchCallback_PBAP, (void *)UpdateData);
            break;
         default:
            DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled Notification: %d\n", UpdateData->UpdateType));

            ret_val = FALSE;
            break;
      }
   }
   else
      ret_val = FALSE;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is simply a foward for local and remote    */
   /* to the PBAM_Disconnect_Device API. It checks whether the calling  */
   /* client matches the client on record and performs the operation.   */
static int _Disconnect_Device(BD_ADDR_t RemoteDeviceAddress, unsigned int ClientID)
{
   int                                ret_val;
   PBAM_Entry_Info_t                 *PBAMEntryInfo;
   PBAP_Close_Port_Indication_Data_t  FakeClosePortIndication;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Phone Book Access Manager has been  */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, verify that the input parameters appear to be semi-valid.*/
      if(!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress))
      {
         /* Attempt to wait for access to the Serial Port state         */
         /* information.                                                */
         if(DEVM_AcquireLock())
         {
            /* Verify we have a connection and that the caller is       */
            /* authorize to access it.                                  */
            if(((PBAMEntryInfo = SearchPBAMEntryInfoEntry(&PBAMEntryInfoList, &RemoteDeviceAddress, 0)) != NULL) && (PBAMEntryInfo->ClientID == ClientID))
            {
               /* Make sure that this port is not already in the process*/
               /* of closing.                                           */
               if(!(PBAMEntryInfo->Flags & PBAM_ENTRY_INFO_FLAGS_EVENT_CLOSING))
               {
                  DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Close Device %d\n", PBAMEntryInfo->PBAPID));

                  /* Flag that we are now in the closing state.         */
                  PBAMEntryInfo->Flags |= PBAM_ENTRY_INFO_FLAGS_EVENT_CLOSING;

                  /* Nothing to do here other than to call the actual   */
                  /* function to disconnect the remote device.          */
                  ret_val = _PBAM_Disconnect_Device(PBAMEntryInfo->PBAPID);

                  /* If the result was successful, we need to make sure */
                  /* we clean up everything and dispatch the event to   */
                  /* all registered clients.                            */
                  if(!ret_val)
                  {
                     /* Fake a PBAP Close Event to dispatch to all      */
                     /* registered clients that the device is no longer */
                     /* connected.                                      */
                     FakeClosePortIndication.PBAPID = PBAMEntryInfo->PBAPID;

                     ProcessCloseIndicationEvent(&FakeClosePortIndication);
                  }
               }
               else
                  ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_NOT_CONNECTED;
            }
            else
               ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_NOT_CONNECTED;

            /* Release the Mutex because we are finished with it.       */
            DEVM_ReleaseLock();
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is simply a foward for local and remote to */
   /* the PBAM_Abort API. It checks whether the calling client matches  */
   /* the client on record and performs the operation.                  */
static int _Abort(BD_ADDR_t RemoteDeviceAddress, unsigned int ClientID)
{
   int                ret_val;
   PBAM_Entry_Info_t *PBAMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Phone Book Access Manager has been  */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Check to see if the device is powered on.                   */
         if(CurrentPowerState)
         {
            /* Verify we have a connection and that the caller is       */
            /* authorize to access it.                                  */
            if(((PBAMEntryInfo = SearchPBAMEntryInfoEntry(&PBAMEntryInfoList, &RemoteDeviceAddress, 0)) != NULL) && (PBAMEntryInfo->ClientID == ClientID))
            {
               /* Note that we have a pending abort. We need to track   */
               /* this Because vcard data can be returned before the    */
               /* abort is processed                                    */
               PBAMEntryInfo->Flags |= PBAM_ENTRY_INFO_FLAGS_PENDING_ABORT;

               if((ret_val = _PBAM_Abort_Request(PBAMEntryInfo->PBAPID)) != 0)
                  PBAMEntryInfo->Flags &= ~((unsigned long)PBAM_ENTRY_INFO_FLAGS_PENDING_ABORT);
            }
            else
               ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_NOT_CONNECTED;
         }
         else
            ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is simply a foward for local and remote to */
   /* the PBAM_Pull_Phone_Book API. It checks whether the calling client*/
   /* matches the client on record and performs the operation.          */
static int _Pull_Phone_Book(BD_ADDR_t RemoteDeviceAddress, char *PhoneBookNamePath, DWord_t FilterLow, DWord_t FilterHigh, PBAM_VCard_Format_t VCardFormat, Word_t MaxListCount, Word_t ListStartOffset, unsigned int ClientID)
{
   int                  ret_val;
   PBAM_Entry_Info_t   *PBAMEntryInfo;
   PBAM_VCard_Format_t  PreviousVCardFormat;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Phone Book Access Manager has been  */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Check to see if the device is powered on.                   */
         if(CurrentPowerState)
         {
            /* Verify we have a connection and that the caller is       */
            /* authorize to access it.                                  */
            if(((PBAMEntryInfo = SearchPBAMEntryInfoEntry(&PBAMEntryInfoList, &RemoteDeviceAddress, 0)) != NULL) && (PBAMEntryInfo->ClientID == ClientID))
            {
               /* Save the last format in case the function fails.      */
               PreviousVCardFormat                  = PBAMEntryInfo->LastVCardRequestFormt;
               PBAMEntryInfo->LastVCardRequestFormt = VCardFormat;

               if((ret_val = _PBAM_Pull_Phone_Book_Request(PBAMEntryInfo->PBAPID, PhoneBookNamePath, FilterLow, FilterHigh, VCardFormat, MaxListCount, ListStartOffset)) != 0)
               {
                  PBAMEntryInfo->LastVCardRequestFormt = PreviousVCardFormat;
               }
            }
            else
               ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_NOT_CONNECTED;
         }
         else
            ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is simply a foward for local and remote to */
   /* the PBAM_Pull_Phone_Book_Size API. It checks whether the calling  */
   /* client matches the client on record and performs the operation.   */
static int _Pull_Phone_Book_Size(BD_ADDR_t RemoteDeviceAddress, unsigned int ClientID)
{
   int                ret_val;
   char               Buffer[64];
   char              *FinalPathSegment;
   PBAM_Entry_Info_t *PBAMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Phone Book Access Manager has been  */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Check to see if the device is powered on.                   */
         if(CurrentPowerState)
         {
            /* Verify we have a connection and that the caller is       */
            /* authorize to access it.                                  */
            if(((PBAMEntryInfo = SearchPBAMEntryInfoEntry(&PBAMEntryInfoList, &RemoteDeviceAddress, 0)) != NULL) && (PBAMEntryInfo->ClientID == ClientID))
            {
               /* The pull phone book size is not an Bluetopia API,     */
               /* instead a special call to pull phone book. Note the   */
               /* state and make the call.                              */
               PBAMEntryInfo->Flags |= PBAM_ENTRY_INFO_FLAGS_PENDING_PHONE_BOOK_SIZE;

               /* We need to append the proper ending to the path.      */
               if(PBAMEntryInfo->PhonebookPath)
               {
                  BTPS_StringCopy(Buffer, PBAMEntryInfo->PhonebookPath);

                  /* Locate the start of the last directory in the path.*/
                  if((FinalPathSegment = strrchr(PBAMEntryInfo->PhonebookPath, PBAM_PATH_DELIMETER_CHARACTER)) != NULL)
                     FinalPathSegment++;
                  else
                     FinalPathSegment = PBAMEntryInfo->PhonebookPath;

                  if(!BTPS_MemCompare(PBAM_TELECOM_PATH_NAME, FinalPathSegment, ((BTPS_StringLength(PBAM_TELECOM_PATH_NAME) < BTPS_StringLength(FinalPathSegment)) ? BTPS_StringLength(PBAM_TELECOM_PATH_NAME) : BTPS_StringLength(FinalPathSegment))))
                     BTPS_StringCopy((Buffer + BTPS_StringLength(Buffer)), (PBAM_PATH_DELIMETER PBAM_PHONEBOOK_PATH_NAME PBAM_OBJECT_NAME_SUFFIX));
                  else
                  {
                     if(!BTPS_MemCompare(PBAM_SIM_PATH_NAME, FinalPathSegment, ((BTPS_StringLength(PBAM_SIM_PATH_NAME) < BTPS_StringLength(FinalPathSegment)) ? BTPS_StringLength(PBAM_SIM_PATH_NAME) : BTPS_StringLength(FinalPathSegment))))
                        BTPS_StringCopy((Buffer + BTPS_StringLength(Buffer)), (PBAM_PATH_DELIMETER PBAM_TELECOM_PATH_NAME PBAM_PATH_DELIMETER PBAM_PHONEBOOK_PATH_NAME PBAM_OBJECT_NAME_SUFFIX));
                     else
                        BTPS_StringCopy((Buffer + BTPS_StringLength(Buffer)), PBAM_OBJECT_NAME_SUFFIX);
                  }
               }
               else
                  BTPS_StringCopy(Buffer, ( PBAM_TELECOM_PATH_NAME PBAM_PATH_DELIMETER PBAM_PHONEBOOK_PATH_NAME PBAM_OBJECT_NAME_SUFFIX ));

               if((ret_val = _PBAM_Pull_Phone_Book_Request(PBAMEntryInfo->PBAPID, Buffer, 0, 0, 0, 0, 0)) != 0)
                  PBAMEntryInfo->Flags &= ~((unsigned long)PBAM_ENTRY_INFO_FLAGS_PENDING_PHONE_BOOK_SIZE);
            }
            else
               ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_NOT_CONNECTED;
         }
         else
            ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is simply a foward for local and remote to */
   /* the PBAM_Set_Phone_Book API. It checks whether the calling client */
   /* matches the client on record and performs the operation.          */
static int _Set_Phone_Book(BD_ADDR_t RemoteDeviceAddress, PBAM_Set_Path_Option_t PathOption, char *FolderName, unsigned int ClientID)
{
   int                ret_val;
   char              *SlashLocation;
   PBAM_Entry_Info_t *PBAMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Phone Book Access Manager has been  */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Check to see if the device is powered on.                   */
         if(CurrentPowerState)
         {
            /* Verify we have a connection and that the caller is       */
            /* authorize to access it.                                  */
            if(((PBAMEntryInfo = SearchPBAMEntryInfoEntry(&PBAMEntryInfoList, &RemoteDeviceAddress, 0)) != NULL) && (PBAMEntryInfo->ClientID == ClientID))
            {
               /* Nothing to do here other than to call the actual      */
               /* function to send the request to set the phonebook.    */
               ret_val = _PBAM_Set_Phone_Book_Request(PBAMEntryInfo->PBAPID, PathOption, FolderName);

               /* If we were successful, update our local path.         */
               if(!ret_val)
               {
                  switch(PathOption)
                  {
                     case pspRoot:
                        if(PBAMEntryInfo->PendingPath)
                           BTPS_FreeMemory(PBAMEntryInfo->PendingPath);

                        PBAMEntryInfo->PendingPath = NULL;
                        break;
                     case pspDown:
                        if((PBAMEntryInfo->PendingPath = (char *)BTPS_AllocateMemory(((PBAMEntryInfo->PhonebookPath)?BTPS_StringLength(PBAMEntryInfo->PhonebookPath):0) + BTPS_StringLength(FolderName) + 2)) != NULL)
                        {
                           if(FolderName[BTPS_StringLength(FolderName)-1] == '/')
                              FolderName[BTPS_StringLength(FolderName)-1] = '\0';

                           if(PBAMEntryInfo->PhonebookPath)
                           {
                              BTPS_StringCopy(PBAMEntryInfo->PendingPath, PBAMEntryInfo->PhonebookPath);
                              BTPS_StringCopy((PBAMEntryInfo->PendingPath + BTPS_StringLength(PBAMEntryInfo->PendingPath)), PBAM_PATH_DELIMETER);
                              BTPS_StringCopy((PBAMEntryInfo->PendingPath + BTPS_StringLength(PBAMEntryInfo->PendingPath)), FolderName);
                           }
                           else
                           {
                              BTPS_StringCopy(PBAMEntryInfo->PendingPath, FolderName);
                           }
                        }
                        break;
                     case pspUp:
                        if(PBAMEntryInfo->PhonebookPath)
                        {
                           if((SlashLocation = strrchr(PBAMEntryInfo->PhonebookPath, '/')) != NULL)
                           {
                              if((PBAMEntryInfo->PendingPath = (char *)BTPS_AllocateMemory(BTPS_StringLength(PBAMEntryInfo->PhonebookPath))) != NULL)
                              {
                                 BTPS_MemCopy(PBAMEntryInfo->PendingPath, PBAMEntryInfo->PhonebookPath, (SlashLocation - PBAMEntryInfo->PhonebookPath));
                                 PBAMEntryInfo->PendingPath[SlashLocation - PBAMEntryInfo->PhonebookPath] = '\0';
                              }
                           }
                        }
                        break;
                  }
               }
            }
            else
               ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_NOT_CONNECTED;
         }
         else
            ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is simply a foward for local and remote to */
   /* the PBAM_Pull_vCard_Listing API. It checks whether the calling    */
   /* client matches the client on record and performs the operation.   */
static int _Pull_vCard_Listing(BD_ADDR_t RemoteDeviceAddress, char *PhonebookPath, PBAM_List_Order_t ListOrder, PBAM_Search_Attribute_t SearchAttribute, char *SearchValue, Word_t MaxListCount, Word_t ListStartOffset, unsigned int ClientID)
{
   int                  ret_val;
   PBAM_Entry_Info_t   *PBAMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Phone Book Access Manager has been  */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Check to see if the device is powered on.                   */
         if(CurrentPowerState)
         {
            /* Verify we have a connection and that the caller is       */
            /* authorize to access it.                                  */
            if(((PBAMEntryInfo = SearchPBAMEntryInfoEntry(&PBAMEntryInfoList, &RemoteDeviceAddress, 0)) != NULL) && (PBAMEntryInfo->ClientID == ClientID))
            {
               ret_val = _PBAM_Pull_vCard_Listing_Request(PBAMEntryInfo->PBAPID, PhonebookPath, ListOrder, SearchAttribute, SearchValue, MaxListCount, ListStartOffset);
            }
            else
               ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_NOT_CONNECTED;
         }
         else
            ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is simply a foward for local and remote    */
   /* to the PBAM_Pull_vCard API. It checks whether the calling client  */
   /* matches the client on record and performs the operation.          */
static int _Pull_vCard(BD_ADDR_t RemoteDeviceAddress, char *VCardName, DWord_t FilterLow, DWord_t FilterHigh, PBAM_VCard_Format_t VCardFormat, unsigned int ClientID)
{
   int                  ret_val;
   PBAM_Entry_Info_t   *PBAMEntryInfo;
   PBAM_VCard_Format_t  PreviousVCardFormat;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Phone Book Access Manager has been  */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Check to see if the device is powered on.                   */
         if(CurrentPowerState)
         {
            /* Verify we have a connection and that the caller is       */
            /* authorize to access it.                                  */
            if(((PBAMEntryInfo = SearchPBAMEntryInfoEntry(&PBAMEntryInfoList, &RemoteDeviceAddress, 0)) != NULL) && (PBAMEntryInfo->ClientID == ClientID))
            {
               /* Save the last format in case the function fails.      */
               PreviousVCardFormat                  = PBAMEntryInfo->LastVCardRequestFormt;
               PBAMEntryInfo->LastVCardRequestFormt = VCardFormat;

               if((ret_val = _PBAM_Pull_vCard_Entry_Request(PBAMEntryInfo->PBAPID, VCardName, FilterLow, FilterHigh, VCardFormat)) != 0)
               {
                  PBAMEntryInfo->LastVCardRequestFormt = PreviousVCardFormat;
               }
            }
            else
               ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_NOT_CONNECTED;
         }
         else
            ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is simply a foward for local and remote    */
   /* to the PBAM_Set_Phone_Book_Absolute API. It checks whether the    */
   /* calling client matches the client on record and performs the      */
   /* operation.                                                        */
static int _Set_Phone_Book_Absolute(BD_ADDR_t RemoteDeviceAddress, char *AbsolutePath, unsigned int ClientID)
{
   int                  ret_val;
   PBAM_Entry_Info_t   *PBAMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Phone Book Access Manager has been  */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Check to see if the device is powered on.                   */
         if(CurrentPowerState)
         {
            /* Verify we have a connection and that the caller is       */
            /* authorize to access it.                                  */
            if(((PBAMEntryInfo = SearchPBAMEntryInfoEntry(&PBAMEntryInfoList, &RemoteDeviceAddress, 0)) != NULL) && (PBAMEntryInfo->ClientID == ClientID))
            {
               /* Flag that we are pending an absolte set.              */
               PBAMEntryInfo->Flags |= PBAM_ENTRY_INFO_FLAGS_PENDING_ABSOLUTE_PATH_CHANGE;

               /* First we need to set to root.                         */
               ret_val = _PBAM_Set_Phone_Book_Request(PBAMEntryInfo->PBAPID, pspRoot, NULL);

               /* If we were successful we need to update the pending   */
               /* path.                                                 */
               if(!ret_val)
               {
                  /* Allocate the string to store the pending path. We  */
                  /* add the SIM to the beginning to note root is the   */
                  /* first pending operation.                           */
                  if((PBAMEntryInfo->PendingPath = (char *)BTPS_AllocateMemory(BTPS_StringLength(PBAM_SIM_PATH_NAME) + BTPS_StringLength(AbsolutePath) + 2)) != NULL)
                  {
                     BTPS_StringCopy(PBAMEntryInfo->PendingPath, PBAM_SIM_PATH_NAME);
                     BTPS_StringCopy((PBAMEntryInfo->PendingPath + BTPS_StringLength(PBAMEntryInfo->PendingPath)), PBAM_PATH_DELIMETER);
                     BTPS_StringCopy((PBAMEntryInfo->PendingPath + BTPS_StringLength(PBAMEntryInfo->PendingPath)), AbsolutePath);

                     /* We know the path will eventually match the      */
                     /* length of the pending path, so let's go ahead   */
                     /* and allocate it.                                */
                     BTPS_FreeMemory(PBAMEntryInfo->PhonebookPath);
                     PBAMEntryInfo->PhonebookPath = (char *)BTPS_AllocateMemory(BTPS_StringLength(PBAMEntryInfo->PendingPath)+1);
                     PBAMEntryInfo->PhonebookPath[0] = '\0';

                     /* Note that we are currently at the beginning of  */
                     /* the pending path.                               */
                     PBAMEntryInfo->PendingPathOffset = 0;
                  }
               }
            }
            else
               ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_NOT_CONNECTED;
         }
         else
            ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

static int ProcessRegisterServer(unsigned int ClientID, unsigned int ServerPort, unsigned int SupportedRepositories, unsigned long IncomingConnectionFlags, char *ServiceName, PBAM_Event_Callback_t EventCallback, void *CallbackParameter)
{
   int                  ret_val             = 0;
   DWord_t              ServiceRecordHandle;
   Boolean_t            ServerPresent;
   PBAM_Server_Entry_t  PBAMEntryInfo;
   PBAM_Server_Entry_t *PBAMEntryInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check whether a server port is supplied or if we need to assign   */
   /* one.                                                              */
   if(!ServerPort)
   {
      /* Assign an open server port.                                    */
      if((ret_val = SPPM_FindFreeServerPort()) > 0)
      {
         ServerPort = ret_val;
         ret_val    = 0;
      }
   }

   if(!ret_val)
   {
      /* Verify that the supplied port is valid.                        */
      if((ServerPort >= PBAP_PORT_NUMBER_MINIMUM) && (ServerPort <= PBAP_PORT_NUMBER_MAXIMUM) && (!SPPM_QueryServerPresent(ServerPort, &ServerPresent)) && (!ServerPresent))
      {
         BTPS_MemInitialize(&PBAMEntryInfo, 0, sizeof(PBAM_Server_Entry_t));

         PBAMEntryInfo.ServerID              = GetNextServerID();
         PBAMEntryInfo.ClientID              = ClientID;
         PBAMEntryInfo.ConnectionState       = csIdle;
         PBAMEntryInfo.ConnectionFlags       = IncomingConnectionFlags;
         PBAMEntryInfo.CurrentOperation      = coNone;
         PBAMEntryInfo.PortNumber            = ServerPort;
         PBAMEntryInfo.SupportedRepositories = SupportedRepositories;
         PBAMEntryInfo.CallbackFunction      = EventCallback;
         PBAMEntryInfo.CallbackParameter     = CallbackParameter;

         if((PBAMEntryInfoPtr = AddPBAMServerEntry(&PBAMServerEntryList, &PBAMEntryInfo)) != NULL)
         {
            if((ret_val = _PBAM_Open_Server(ServerPort, SupportedRepositories)) > 0)
            {
               /* Note the returned PBAP ID.                            */
               PBAMEntryInfoPtr->PBAPID = (unsigned int)ret_val;

               /* Now try to register the service record for this port. */
               if(!(ret_val = _PBAM_Register_Service_Record(PBAMEntryInfoPtr->PBAPID, ServiceName, &ServiceRecordHandle)))
               {
                  /* Note the record handle.                            */
                  PBAMEntryInfoPtr->ServiceRecordHandle = ServiceRecordHandle;

                  /* Return the server Server ID to the caller.         */
                  ret_val = PBAMEntryInfoPtr->ServerID;
               }
               else
               {
                  /* Service record failed to register. We need to clean*/
                  /* up the port.                                       */
                  _PBAM_Close_Server(PBAMEntryInfoPtr->PBAPID);
               }
            }

            if(ret_val < 0)
            {
               /* An error occurred, go ahead and delete the entry that */
               /* was added.                                            */
               if((PBAMEntryInfoPtr = DeletePBAMServerEntry(&PBAMServerEntryList, PBAMEntryInfoPtr->PBAPID)) != NULL)
                  FreePBAMServerEntryMemory(PBAMEntryInfoPtr);
            }
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

static int ProcessUnRegisterServer(unsigned int ClientID, unsigned int ServerID)
{
   int                ret_val;
   PBAM_Server_Entry_t *PBAMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if((PBAMEntryInfo = SearchPBAMServerEntryServerID(&PBAMServerEntryList, ServerID)) != NULL)
   {
      if(ClientID == PBAMEntryInfo->ClientID)
      {
         /* Delete the PBA Server Entry from the PBA Entry List.        */
         if((PBAMEntryInfo = DeletePBAMServerEntry(&PBAMServerEntryList, PBAMEntryInfo->PBAPID)) != NULL)
         {
            DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Delete Server: %d\n", ServerID));

            /* Reject any incoming connection that might be in progress.*/
            if((PBAMEntryInfo->ConnectionState == csAuthenticating) || (PBAMEntryInfo->ConnectionState == csAuthorizing) || (PBAMEntryInfo->ConnectionState == csEncrypting))
               _PBAM_Open_Request_Response(PBAMEntryInfo->PBAPID, FALSE);

            /* If there was a Service Record Registered, go ahead and   */
            /* make sure it is freed.                                   */
            if(PBAMEntryInfo->ServiceRecordHandle)
               _PBAM_Un_Register_Service_Record(PBAMEntryInfo->PBAPID, PBAMEntryInfo->ServiceRecordHandle);

            /* Next, go ahead and Un-Register the Server.               */
            _PBAM_Close_Server(PBAMEntryInfo->PBAPID);

            /* All finished, free any memory that was allocated for the */
            /* server.                                                  */
            FreePBAMServerEntryMemory(PBAMEntryInfo);

            /* Flag success.                                            */
            ret_val = 0;
         }
         else
            ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_INVALID_SERVER_ID;
      }
      else
         ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_INVALID_CLIENT;
   }
   else
      ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_INVALID_SERVER_ID;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

static int ProcessConnectionRequestResponse(unsigned int ClientID, unsigned int ConnectionID, Boolean_t Accept)
{
   int                  ret_val;
   Boolean_t            Encrypt;
   Boolean_t            Authenticate;
   PBAM_Server_Entry_t *PBAMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if((PBAMEntryInfo = SearchPBAMServerEntryConnectionID(&PBAMServerEntryList, ConnectionID)) != NULL)
   {
      if(ClientID == PBAMEntryInfo->ClientID)
      {
         if(PBAMEntryInfo->ConnectionState == csAuthorizing)
         {
            DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Respond to Request: %d\n", Accept));

            /* If the client has accepted the request then we need to   */
            /* process it differently.                                  */
            if(Accept)
            {
               /* Determine if Authentication and/or Encryption is      */
               /* required for this link.                               */
               if(PBAMEntryInfo->ConnectionFlags & PBAM_INCOMING_CONNECTION_FLAGS_REQUIRE_AUTHENTICATION)
                  Authenticate = TRUE;
               else
                  Authenticate = FALSE;

               if(PBAMEntryInfo->ConnectionFlags & PBAM_INCOMING_CONNECTION_FLAGS_REQUIRE_ENCRYPTION)
                  Encrypt = TRUE;
               else
                  Encrypt = FALSE;

               if((Authenticate) || (Encrypt))
               {
                  if(Encrypt)
                     ret_val = DEVM_EncryptRemoteDevice(PBAMEntryInfo->RemoteDeviceAddress, 0);
                  else
                     ret_val = DEVM_AuthenticateRemoteDevice(PBAMEntryInfo->RemoteDeviceAddress, 0);
               }
               else
                  ret_val = BTPM_ERROR_CODE_DEVICE_ALREADY_AUTHENTICATED;

               if((ret_val == BTPM_ERROR_CODE_DEVICE_ALREADY_AUTHENTICATED) || (ret_val == BTPM_ERROR_CODE_DEVICE_ALREADY_ENCRYPTED))
               {
                  /* Authorization not required, and we are already in  */
                  /* the correct state.                                 */
                  if((ret_val = _PBAM_Open_Request_Response(PBAMEntryInfo->PBAPID, TRUE)) != 0)
                  {
                     /* Failure, go ahead and try to disconnect it (will*/
                     /* probably fail as well).                         */
                     PBAMEntryInfo->ConnectionState = csIdle;

                     _PBAM_Disconnect_Device(PBAMEntryInfo->PBAPID);
                  }
               }
               else
               {
                  /* If we were successfully able to Authenticate and/or*/
                  /* Encrypt, then we need to set the correct state.    */
                  if(!ret_val)
                  {
                     if(Encrypt)
                        PBAMEntryInfo->ConnectionState = csEncrypting;
                     else
                        PBAMEntryInfo->ConnectionState = csAuthenticating;

                     /* Flag success.                                   */
                     ret_val = 0;
                  }
                  else
                  {
                     /* Error, reject the request.                      */
                     if(_PBAM_Open_Request_Response(PBAMEntryInfo->PBAPID, FALSE))
                     {
                        /* Failure, go ahead and try to disconnect it   */
                        /* (will probably fail as well).                */
                        PBAMEntryInfo->ConnectionState = csIdle;

                        _PBAM_Disconnect_Device(PBAMEntryInfo->PBAPID);
                     }
                  }
               }
            }
            else
            {
               /* Rejection - Simply respond to the request.            */
               _PBAM_Open_Request_Response(PBAMEntryInfo->PBAPID, FALSE);

               PBAMEntryInfo->ConnectionState = csIdle;

               _PBAM_Disconnect_Device(PBAMEntryInfo->PBAPID);

               /* Flag success.                                         */
               ret_val = 0;
            }
         }
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CONNECTION_STATE;
      }
      else
         ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_INVALID_CLIENT;
   }
   else
      ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_NOT_CONNECTED;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

static int ProcessCloseServerConnection(unsigned int ClientID, unsigned int ConnectionID)
{
   int                  ret_val;
   PBAM_Server_Entry_t *PBAMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %u\n", ConnectionID));

   if((PBAMEntryInfo = SearchPBAMServerEntryConnectionID(&PBAMServerEntryList, ConnectionID)) != NULL)
   {
      if(ClientID == PBAMEntryInfo->ClientID)
      {
         /* Check whether we have an active connection.                 */
         if(PBAMEntryInfo->ConnectionState != csIdle)
         {
            _PBAM_Disconnect_Device(PBAMEntryInfo->PBAPID);

            CleanupPBAMServerEntry(PBAMEntryInfo);

            PBAMEntryInfo->ConnectionState = csIdle;
         }

         /* Return success.                                             */
         ret_val = 0;
      }
      else
         ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_INVALID_CLIENT;
   }
   else
      ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_NOT_CONNECTED;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

static int ProcessSendPhoneBook(unsigned int ClientID, unsigned int ConnectionID, unsigned int ResponseStatusCode, Byte_t *NewMissedCalls, unsigned int DataLength, Byte_t *DataBuffer, Boolean_t Final)
{
   int                  ret_val;
   Byte_t               OBEXResponseCode;
   PBAM_Server_Entry_t *PBAMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if((PBAMEntryInfo = SearchPBAMServerEntryConnectionID(&PBAMServerEntryList, ConnectionID)) != NULL)
   {
      if(ClientID == PBAMEntryInfo->ClientID)
      {
         /* Entry found, make sure that the correct on-going operation  */
         /* is in progress.                                             */
         if(PBAMEntryInfo->CurrentOperation == coPullPhonebook)
         {
            if(MapResponseStatusCodeToResponseCode(ResponseStatusCode, &OBEXResponseCode))
            {
               /* Determine if we need to back up the data we are       */
               /* sending.                                              */
               /* * NOTE * There is no reason to worry about            */
               /*          sending any data (or backing any data up if  */
               /*          this is not a successful response).          */
               if(ResponseStatusCode == PBAM_RESPONSE_STATUS_CODE_SUCCESS)
               {
                  /* Check to see if we need to PBA the final bit into a*/
                  /* continue (the lack of it).                         */
                  /* * NOTE * This is required because there is         */
                  /*          No Final flag for responses (it is        */
                  /*          inherant with either an OK or or CONTINUE */
                  /*          being sent as the code).                  */
                  if(!Final)
                     OBEXResponseCode = PBAP_OBEX_RESPONSE_CONTINUE;

                  /* Note whether this response needs the final bit.    */
                  PBAMEntryInfo->DataFinal = Final;

                  if((PBAMEntryInfo->DataBufferSize = DataLength) != 0)
                  {
                     /* Free any current data we have buffered (should  */
                     /* be none).                                       */
                     if(PBAMEntryInfo->DataBuffer)
                     {
                        BTPS_FreeMemory(PBAMEntryInfo->DataBuffer);

                        PBAMEntryInfo->DataBuffer = NULL;
                     }

                     /* Go ahead and allocate the buffer (we will not   */
                     /* copy it yet, but we will allocate it so that we */
                     /* don't get an error *AFTER* we send the first    */
                     /* part of the data.                               */
                     if((PBAMEntryInfo->DataBuffer = (Byte_t *)BTPS_AllocateMemory(DataLength)) == NULL)
                        ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
                     else
                        ret_val = 0;
                  }
                  else
                     ret_val = 0;

                  /* Flag that we have not sent any data at this point. */
                  PBAMEntryInfo->DataBufferSent = 0;
               }
               else
               {
                  /* There is no reason to send any data because this is*/
                  /* an error response.                                 */
                  DataLength = 0;

                  ret_val    = 0;
               }

               if(!ret_val)
               {
                  if((ret_val = _PBAM_Pull_Phonebook_Response(PBAMEntryInfo->PBAPID, OBEXResponseCode, NULL, NewMissedCalls, DataLength, DataLength?DataBuffer:NULL, &(PBAMEntryInfo->DataBufferSent))) == 0)
                  {
                     /* Copy any remaining data into the buffer for     */
                     /* future operations.                              */
                     if((ResponseStatusCode == PBAM_RESPONSE_STATUS_CODE_SUCCESS) && (PBAMEntryInfo->DataBufferSent != DataLength))
                        BTPS_MemCopy(PBAMEntryInfo->DataBuffer, DataBuffer, DataLength);
                     else
                        PBAMEntryInfo->CurrentOperation = coNone;
                  }

                  /* If there was an error or we sent all of the data,  */
                  /* then we need to free any buffer that was allocated.*/
                  if((ret_val) || ((ResponseStatusCode == PBAM_RESPONSE_STATUS_CODE_SUCCESS) && (PBAMEntryInfo->DataBufferSent == DataLength)))
                  {
                     if(PBAMEntryInfo->DataBuffer)
                     {
                        BTPS_FreeMemory(PBAMEntryInfo->DataBuffer);

                        PBAMEntryInfo->DataBuffer = NULL;
                     }
                  }
               }
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         }
         else
            ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_INVALID_OPERATION;
      }
      else
         ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_INVALID_CLIENT;
   }
   else
      ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_NOT_CONNECTED;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

static int ProcessSendPhoneBookSize(unsigned int ClientID, unsigned int ConnectionID, unsigned int ResponseStatusCode, Word_t PhoneBookSize)
{
   int                  ret_val;
   Byte_t               OBEXResponseCode;
   PBAM_Server_Entry_t *PBAMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if((PBAMEntryInfo = SearchPBAMServerEntryConnectionID(&PBAMServerEntryList, ConnectionID)) != NULL)
   {
      if(ClientID == PBAMEntryInfo->ClientID)
      {
         /* Entry found, make sure that the correct on-going operation  */
         /* is in progress.                                             */
         if(PBAMEntryInfo->CurrentOperation == coPullPhonebookSize)
         {
            if(MapResponseStatusCodeToResponseCode(ResponseStatusCode, &OBEXResponseCode))
            {
               /* Simply submit the Phonebook Size.                     */
               if((ret_val = _PBAM_Pull_Phonebook_Response(PBAMEntryInfo->PBAPID, OBEXResponseCode, &PhoneBookSize, NULL, 0, NULL, NULL)) == 0)
               {
                  /* Note that the current opeation is finished.        */
                  PBAMEntryInfo->CurrentOperation = coNone;
               }
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         }
         else
            ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_INVALID_OPERATION;
      }
      else
         ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_INVALID_CLIENT;
   }
   else
      ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_NOT_CONNECTED;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

static int ProcessSetPhoneBookResponse(unsigned int ClientID, unsigned int ConnectionID, unsigned int ResponseStatusCode)
{
   int                  ret_val;
   Byte_t               OBEXResponseCode;
   PBAM_Server_Entry_t *PBAMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if((PBAMEntryInfo = SearchPBAMServerEntryConnectionID(&PBAMServerEntryList, ConnectionID)) != NULL)
   {
      if(ClientID == PBAMEntryInfo->ClientID)
      {
         /* Entry found, make sure that the correct on-going operation  */
         /* is in progress.                                             */
         if(PBAMEntryInfo->CurrentOperation == coSetPhonebook)
         {
            if(MapResponseStatusCodeToResponseCode(ResponseStatusCode, &OBEXResponseCode))
            {
               /* Simply submit the response .                          */
               if((ret_val = _PBAM_Set_Phonebook_Response(PBAMEntryInfo->PBAPID, OBEXResponseCode)) == 0)
               {
                  /* Note that the current opeation is finished.        */
                  PBAMEntryInfo->CurrentOperation = coNone;
               }
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         }
         else
            ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_INVALID_OPERATION;
      }
      else
         ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_INVALID_CLIENT;
   }
   else
      ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_NOT_CONNECTED;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

static int ProcessSendvCardListing(unsigned int ClientID, unsigned int ConnectionID, unsigned int ResponseStatusCode, Byte_t *NewMissedCalls, unsigned int DataLength, Byte_t *DataBuffer, Boolean_t Final)
{
   int                  ret_val;
   Byte_t               OBEXResponseCode;
   PBAM_Server_Entry_t *PBAMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if((PBAMEntryInfo = SearchPBAMServerEntryConnectionID(&PBAMServerEntryList, ConnectionID)) != NULL)
   {
      if(ClientID == PBAMEntryInfo->ClientID)
      {
         /* Entry found, make sure that the correct on-going operation  */
         /* is in progress.                                             */
         if(PBAMEntryInfo->CurrentOperation == coPullvCardListing)
         {
            if(MapResponseStatusCodeToResponseCode(ResponseStatusCode, &OBEXResponseCode))
            {
               /* Determine if we need to back up the data we are       */
               /* sending.                                              */
               /* * NOTE * There is no reason to worry about sending any*/
               /*          data (or backing any data up if this is not a*/
               /*          successful response).                        */
               if(ResponseStatusCode == PBAM_RESPONSE_STATUS_CODE_SUCCESS)
               {
                  /* Check to see if we need to PBA the final bit into a*/
                  /* continue (the lack of it).                         */
                  /* * NOTE * This is required because there is No Final*/
                  /*          flag for responses (it is inherant with   */
                  /*          either an OK or or CONTINUE being sent as */
                  /*          the code).                                */
                  if(!Final)
                     OBEXResponseCode = PBAP_OBEX_RESPONSE_CONTINUE;

                  /* Note whether this response needs the final bit.    */
                  PBAMEntryInfo->DataFinal = Final;

                  if((PBAMEntryInfo->DataBufferSize = DataLength) != 0)
                  {
                     /* Free any current data we have buffered (should  */
                     /* be none).                                       */
                     if(PBAMEntryInfo->DataBuffer)
                     {
                        BTPS_FreeMemory(PBAMEntryInfo->DataBuffer);

                        PBAMEntryInfo->DataBuffer = NULL;
                     }

                     /* Go ahead and allocate the buffer (we will not   */
                     /* copy it yet, but we will allocate it so that we */
                     /* don't get an error *AFTER* we send the first    */
                     /* part of the data.                               */
                     if((PBAMEntryInfo->DataBuffer = (Byte_t *)BTPS_AllocateMemory(DataLength)) == NULL)
                        ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
                     else
                        ret_val = 0;
                  }
                  else
                     ret_val = 0;

                  /* Flag that we have not sent any data at this point. */
                  PBAMEntryInfo->DataBufferSent = 0;
               }
               else
               {
                  /* There is no reason to send any data because this is*/
                  /* an error response.                                 */
                  DataLength = 0;

                  ret_val    = 0;
               }

               if(!ret_val)
               {
                  if((ret_val = _PBAM_Pull_vCard_Listing_Response(PBAMEntryInfo->PBAPID, OBEXResponseCode, NULL, NewMissedCalls, DataLength, DataLength?DataBuffer:NULL, &(PBAMEntryInfo->DataBufferSent))) == 0)
                  {
                     /* Copy any remaining data into the buffer for     */
                     /* future operations.                              */
                     if((ResponseStatusCode == PBAM_RESPONSE_STATUS_CODE_SUCCESS) && (PBAMEntryInfo->DataBufferSent != DataLength))
                        BTPS_MemCopy(PBAMEntryInfo->DataBuffer, DataBuffer, DataLength);
                     else
                        PBAMEntryInfo->CurrentOperation = coNone;
                  }

                  /* If there was an error or we sent all of the data,  */
                  /* then we need to free any buffer that was allocated.*/
                  if((ret_val) || ((ResponseStatusCode == PBAM_RESPONSE_STATUS_CODE_SUCCESS) && (PBAMEntryInfo->DataBufferSent == DataLength)))
                  {
                     if(PBAMEntryInfo->DataBuffer)
                     {
                        BTPS_FreeMemory(PBAMEntryInfo->DataBuffer);

                        PBAMEntryInfo->DataBuffer = NULL;
                     }
                  }
               }
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         }
         else
            ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_INVALID_OPERATION;
      }
      else
         ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_INVALID_CLIENT;
   }
   else
      ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_NOT_CONNECTED;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

static int ProcessSendvCardListingSize(unsigned int ClientID, unsigned int ConnectionID, unsigned int ResponseStatusCode, Word_t vCardListingSize)
{
   int                  ret_val;
   Byte_t               OBEXResponseCode;
   PBAM_Server_Entry_t *PBAMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if((PBAMEntryInfo = SearchPBAMServerEntryConnectionID(&PBAMServerEntryList, ConnectionID)) != NULL)
   {
      if(ClientID == PBAMEntryInfo->ClientID)
      {
         /* Entry found, make sure that the correct on-going operation  */
         /* is in progress.                                             */
         if(PBAMEntryInfo->CurrentOperation == coPullvCardListingSize)
         {
            if(MapResponseStatusCodeToResponseCode(ResponseStatusCode, &OBEXResponseCode))
            {
               /* Simply submit the Phonebook Size.                     */
               if((ret_val = _PBAM_Pull_vCard_Listing_Response(PBAMEntryInfo->PBAPID, OBEXResponseCode, &vCardListingSize, NULL, 0, NULL, NULL)) == 0)
               {
                  /* Note that the current opeation is finished.        */
                  PBAMEntryInfo->CurrentOperation = coNone;
               }
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         }
         else
            ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_INVALID_OPERATION;
      }
      else
         ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_INVALID_CLIENT;
   }
   else
      ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_NOT_CONNECTED;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

static int ProcessSendvCard(unsigned int ClientID, unsigned int ConnectionID, unsigned int ResponseStatusCode, unsigned int DataLength, Byte_t *DataBuffer, Boolean_t Final)
{
   int                  ret_val;
   Byte_t               OBEXResponseCode;
   PBAM_Server_Entry_t *PBAMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if((PBAMEntryInfo = SearchPBAMServerEntryConnectionID(&PBAMServerEntryList, ConnectionID)) != NULL)
   {
      if(ClientID == PBAMEntryInfo->ClientID)
      {
         /* Entry found, make sure that the correct on-going operation  */
         /* is in progress.                                             */
         if(PBAMEntryInfo->CurrentOperation == coPullvCard)
         {
            if(MapResponseStatusCodeToResponseCode(ResponseStatusCode, &OBEXResponseCode))
            {
               /* Determine if we need to back up the data we are       */
               /* sending.                                              */
               /* * NOTE * There is no reason to worry about sending any*/
               /*          data (or backing any data up if this is not a*/
               /*          successful response).                        */
               if(ResponseStatusCode == PBAM_RESPONSE_STATUS_CODE_SUCCESS)
               {
                  /* Check to see if we need to PBA the final bit into a*/
                  /* continue (the lack of it).                         */
                  /* * NOTE * This is required because there is No Final*/
                  /*          flag for responses (it is inherant with   */
                  /*          either an OK or or CONTINUE being sent as */
                  /*          the code).                                */
                  if(!Final)
                     OBEXResponseCode = PBAP_OBEX_RESPONSE_CONTINUE;

                  /* Note whether this response needs the final bit.    */
                  PBAMEntryInfo->DataFinal = Final;

                  if((PBAMEntryInfo->DataBufferSize = DataLength) != 0)
                  {
                     /* Free any current data we have buffered (should  */
                     /* be none).                                       */
                     if(PBAMEntryInfo->DataBuffer)
                     {
                        BTPS_FreeMemory(PBAMEntryInfo->DataBuffer);

                        PBAMEntryInfo->DataBuffer = NULL;
                     }

                     /* Go ahead and allocate the buffer (we will not   */
                     /* copy it yet, but we will allocate it so that we */
                     /* don't get an error *AFTER* we send the first    */
                     /* part of the data.                               */
                     if((PBAMEntryInfo->DataBuffer = (Byte_t *)BTPS_AllocateMemory(DataLength)) == NULL)
                        ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
                     else
                        ret_val = 0;
                  }
                  else
                     ret_val = 0;

                  /* Flag that we have not sent any data at this point. */
                  PBAMEntryInfo->DataBufferSent = 0;
               }
               else
               {
                  /* There is no reason to send any data because this is*/
                  /* an error response.                                 */
                  DataLength = 0;

                  ret_val    = 0;
               }

               if(!ret_val)
               {
                  if((ret_val = _PBAM_Pull_vCard_Entry_Response(PBAMEntryInfo->PBAPID, OBEXResponseCode, DataLength, DataLength?DataBuffer:NULL, &(PBAMEntryInfo->DataBufferSent))) == 0)
                  {
                     /* Copy any remaining data into the buffer for     */
                     /* future operations.                              */
                     if((ResponseStatusCode == PBAM_RESPONSE_STATUS_CODE_SUCCESS) && (PBAMEntryInfo->DataBufferSent != DataLength))
                        BTPS_MemCopy(PBAMEntryInfo->DataBuffer, DataBuffer, DataLength);
                     else
                        PBAMEntryInfo->CurrentOperation = coNone;
                  }

                  /* If there was an error or we sent all of the data,  */
                  /* then we need to free any buffer that was allocated.*/
                  if((ret_val) || ((ResponseStatusCode == PBAM_RESPONSE_STATUS_CODE_SUCCESS) && (PBAMEntryInfo->DataBufferSent == DataLength)))
                  {
                     if(PBAMEntryInfo->DataBuffer)
                     {
                        BTPS_FreeMemory(PBAMEntryInfo->DataBuffer);

                        PBAMEntryInfo->DataBuffer = NULL;
                     }
                  }
               }
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         }
         else
            ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_INVALID_OPERATION;
      }
      else
         ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_INVALID_CLIENT;
   }
   else
      ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_NOT_CONNECTED;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}


   /* The following function is provided to allow a mechanism for local */
   /* modules to connect to a remote Phone Book Access device.  This    */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error.  This function also accepts the       */
   /* connection information for the remote device (address and server  */
   /* port).  This function accepts the connection flags to apply to    */
   /* control how the connection is made regarding encryption and/or    */
   /* authentication.                                                   */
   /* * NOTE * The final parameter to this function, if specified,      */
   /*          instructs this function to block until the connection    */
   /*          status is received (i.e. the connection is completed).   */
   /*          If this parameter is not specified (i.e. NULL) then the  */
   /*          connection status will be returned asynchronously in the */
   /*          Phone Book Access Manager connection status Event (if    */
   /*          specified).                                              */
int BTPSAPI PBAM_Connect_Remote_Device(BD_ADDR_t RemoteDeviceAddress, unsigned int RemoteServerPort, unsigned long ConnectionFlags, PBAM_Event_Callback_t CallbackFunction, void *CallbackParameter, unsigned int *ConnectionStatus)
{
   int                ret_val;
   Event_t            ConnectionEvent;
   PBAM_Entry_Info_t  PBAMEntryInfo;
   PBAM_Entry_Info_t *PBAMEntryInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Phone Book Access Manager has been  */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, verify that the input parameters appear to be semi-valid.*/
      if((!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (RemoteServerPort) && (CallbackFunction))
      {
         /* Attempt to wait for access to the Phone Book Access Manager */
         /* state information.                                          */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* Device is powered on, attempt to add an entry into the*/
               /* Phone Book Access entry list.                         */
               BTPS_MemInitialize(&PBAMEntryInfo, 0, sizeof(PBAM_Entry_Info_t));

               PBAMEntryInfo.ClientID          = MSG_GetServerAddressID();
               PBAMEntryInfo.EventCallback     = CallbackFunction;
               PBAMEntryInfo.CallbackParameter = CallbackParameter;
               PBAMEntryInfo.BluetoothAddress  = RemoteDeviceAddress;

               if(ConnectionStatus)
               {
                  PBAMEntryInfo.ConnectionEvent = BTPS_CreateEvent(FALSE);
                  PBAMEntryInfo.Flags           = PBAM_ENTRY_INFO_FLAGS_SYNCHRONOUS_CONNECT_REQUEST;
               }

               /* Note the connection was created from a direct call to */
               /* the API.                                              */
               PBAMEntryInfo.Flags |= PBAM_ENTRY_INFO_FLAGS_LOCALLY_HANDLED;

               /* Confirm that we did not fail the event creation (if it*/
               /* was created).                                         */
               if(((ConnectionStatus) && (PBAMEntryInfo.ConnectionEvent)) || (ConnectionStatus == NULL))
               {
                  if((PBAMEntryInfoPtr = AddPBAMEntryInfoEntry(&PBAMEntryInfoList, &PBAMEntryInfo)) != NULL)
                  {
                     DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to open Remote device %d 0x%08lX\n", RemoteServerPort, ConnectionFlags));

                     /* Next, attempt to open the remote device.        */
                     if((ret_val = _PBAM_Connect_Remote_Device(RemoteDeviceAddress, RemoteServerPort)) <= 0)
                     {
                        /* Error opening device, go ahead and delete the*/
                        /* entry that was added.                        */
                        if((PBAMEntryInfoPtr = DeletePBAMEntryInfoEntry(&PBAMEntryInfoList, &(PBAMEntryInfoPtr->BluetoothAddress), 0)) != NULL)
                        {
                           if(PBAMEntryInfoPtr->ConnectionEvent)
                              BTPS_CloseEvent(PBAMEntryInfoPtr->ConnectionEvent);

                           FreePBAMEntryInfoEntryMemory(PBAMEntryInfoPtr);
                        }
                     }
                     else
                     {
                        /* Save the reference to PBAPID                 */
                        PBAMEntryInfoPtr->PBAPID = ret_val;

                        /* Next, determine if the caller has requested a*/
                        /* blocking open.                               */
                        if(ConnectionStatus)
                        {
                           /* Blocking open, go ahead and wait for the  */
                           /* event.                                    */

                           /* Note the open event.                      */
                           ConnectionEvent = PBAMEntryInfoPtr->ConnectionEvent;

                           /* Release the Mutex because we are finished */
                           /* with it.                                  */
                           DEVM_ReleaseLock();

                           BTPS_WaitEvent(ConnectionEvent, BTPS_INFINITE_WAIT);

                           /* Re-acquire the Mutex.                     */
                           if(DEVM_AcquireLock())
                           {
                              if(((PBAMEntryInfoPtr = SearchPBAMEntryInfoEntry(&PBAMEntryInfoList, &RemoteDeviceAddress, 0)) != NULL))
                              {
                                 /* Note the connection status.         */
                                 *ConnectionStatus = PBAMEntryInfoPtr->ConnectionStatus;

                                 PBAMEntryInfo.Flags &= ~((unsigned long)PBAM_ENTRY_INFO_FLAGS_SYNCHRONOUS_CONNECT_REQUEST);

                                 BTPS_CloseEvent(PBAMEntryInfoPtr->ConnectionEvent);

                                 if(PBAMEntryInfoPtr->ConnectionStatus)
                                 {
                                    /* Because the connection failed, we*/
                                    /* need to delete the entry.        */
                                    if((PBAMEntryInfoPtr = DeletePBAMEntryInfoEntry(&PBAMEntryInfoList, &RemoteDeviceAddress, 0)) != NULL)
                                    {
                                       FreePBAMEntryInfoEntryMemory(PBAMEntryInfoPtr);
                                    }

                                    ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_UNABLE_TO_CONNECT_TO_DEVICE;
                                 }
                                 else
                                 {
                                    /* Flag success to the caller.      */
                                    ret_val = 0;
                                 }
                              }
                              else
                                 ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_UNABLE_TO_CONNECT_TO_DEVICE;
                           }
                           else
                              ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
                        }
                        else
                        {

                           /* The result of the connection will be      */
                           /* returned with callback                    */
                           ret_val = 0;
                        }
                     }
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
               }
               else
                  ret_val = BTPM_ERROR_CODE_UNABLE_TO_CREATE_EVENT;
            }
            else
               ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

            /* Release the Mutex because we are finished with it.       */
            if(ret_val != BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT)
               DEVM_ReleaseLock();
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function exists to close an active Phone Book Access*/
   /* connection that was previously opened by a successful call to     */
   /* PBAM_Connect_Remote_Device() function.  This function accepts the */
   /* RemoteDeviceAddress.  This function returns zero if successful, or*/
   /* a negative return value if there was an error.                    */
int BTPSAPI PBAM_Disconnect_Device(BD_ADDR_t RemoteDeviceAddress)
{
   /* Simply foward the call to the handler with the Server ID.         */
   return(_Disconnect_Device(RemoteDeviceAddress, MSG_GetServerAddressID()));
}

   /* The following function is responsible for aborting ANY currently  */
   /* outstanding PBAM profile client request.  This function accepts as*/
   /* input the remote device address of the device to abort the current*/
   /* operation.  This function returns zero if successful, or a        */
   /* negative return error code if there was an error.                 */
   /* * NOTE * There can only be one outstanding PBAM request active at */
   /*          any one time.  Because of this, another PBAM request     */
   /*          cannot be issued until either the current request is     */
   /*          aborted (by calling the PBAM_Abort() function) or the    */
   /*          current request is completed (this is signified by       */
   /*          receiving a confirmation event in the PBAM event callback*/
   /*          that was registered when the PBAM port was opened).      */
int BTPSAPI PBAM_Abort(BD_ADDR_t RemoteDeviceAddress)
{
   /* Simply foward the call to the handler with the Server ID.         */
   return(_Abort(RemoteDeviceAddress, MSG_GetServerAddressID()));
}

   /* The following function generates a PBAP Pull PhoneBook Request to */
   /* the specified remote PBAP server.  The RemoteDeviceAddress        */
   /* parameter specifies the device connection to perform the pull     */
   /* request The PhoneBookNamePath parameter contains the name/path of */
   /* the phonebook being requested by this pull phone book operation.  */
   /* The FilterLow parameter contains the lower 32 bits of the 64-bit  */
   /* filter attribute.  The FilterHigh parameter contains the higher 32*/
   /* bits of the 64-bit filter attribute.  The VCardFormat parameter is*/
   /* an enumeration which specifies the VCard format requested in this */
   /* Pull PhoneBook request.  If pfDefault is specified then the format*/
   /* will not be included in the request.  The MaxListCount parameter  */
   /* is an unsigned integer that specifies the maximum number of       */
   /* entries the client can handle.  A value of 65535 means that the   */
   /* number of entries is not restricted.  A MaxListCount of ZERO (0)  */
   /* indicates that this is a request for the number of used indexes in*/
   /* the PhoneBook specified by the PhoneBookNamePath parameter.  The  */
   /* ListStartOffset parameter specifies the index requested by the    */
   /* Client in this PullPhoneBook.  This function returns zero if      */
   /* successful or a negative return error code if there was an error. */
   /* * NOTE * A successful return code does not mean that the remote   */
   /*          PBAP Profile Server successfully processed the command.  */
   /*          The caller needs to check the confirmation result to     */
   /*          determine if the remote PBAP profile server successfully */
   /*          executed the request.                                    */
   /* * NOTE * There can only be one outstanding PBAP profile request   */
   /*          active at any one time.  Because of this, another PBAM   */
   /*          request cannot be issued until either the current request*/
   /*          is aborted (by calling the PBAM_Abort() function) or the */
   /*          current request is completed (this is signified by       */
   /*          receiving an petPBAData event, with Final TRUE, in the   */
   /*          callback that was registered when the PBAM port was      */
   /*          opened).                                                 */
int BTPSAPI PBAM_Pull_Phone_Book(BD_ADDR_t RemoteDeviceAddress, char *PhoneBookNamePath, DWord_t FilterLow, DWord_t FilterHigh, PBAM_VCard_Format_t VCardFormat, Word_t MaxListCount, Word_t ListStartOffset)
{
   /* Simply foward the call to the handler with the Server ID.         */
   return(_Pull_Phone_Book(RemoteDeviceAddress, PhoneBookNamePath, FilterLow, FilterHigh, VCardFormat, MaxListCount, ListStartOffset, MSG_GetServerAddressID()));
}

   /* The following function generates a PBAP Pull PhoneBook Size       */
   /* Request to the specified remote PBAP server requesting the size of*/
   /* the phonebook.  The RemoteDeviceAddress parameter specifies the   */
   /* connect for the local PBAP Client (returned from a successful call*/
   /* to the PBAM_Connect_Remote_Device() This size returned in the     */
   /* event, petPhoneBookSize, may change from the time of the response */
   /* to querying the device for data.                                  */
   /* * NOTE * A successful return code does not mean that the remote   */
   /*          PBAP server successfully processed the command.  The     */
   /*          caller needs to check the confirmation result to         */
   /*          determine if the remote PBAP Profile Server successfully */
   /*          executed the Request.                                    */
   /* * NOTE * There can only be one outstanding PBAP Profile Request   */
   /*          active at any one time.  Because of this, another PBAP   */
   /*          profile request cannot be issued until either the current*/
   /*          request is aborted (by calling the PBAM_Abort() function)*/
   /*          or the current request is completed (this is signified by*/
   /*          receiving a petPhoneBookSize event in the PBAM event     */
   /*          callback that was registered when the PBAM port was      */
   /*          opened).                                                 */
int BTPSAPI PBAM_Pull_Phone_Book_Size(BD_ADDR_t RemoteDeviceAddress)
{
   /* Simply foward the call to the handler with the Server ID.         */
   return(_Pull_Phone_Book_Size(RemoteDeviceAddress, MSG_GetServerAddressID()));
}

   /* The following function generates a PBAP Set Phone Book Request to */
   /* the specified remote PBAP Server.  The RemoteDeviceAddress        */
   /* parameter specifies the connected device for the request.  The    */
   /* PathOption parameter contains an enumerated value that indicates  */
   /* the type of path change to request.  The FolderName parameter     */
   /* contains the folder name to include with this Set PhoneBook       */
   /* request.  This value can be NULL if no name is required for the   */
   /* selected PathOption.  See the PBAP specification for more         */
   /* information.  This function returns zero if successful or a       */
   /* negative return error code if there was an error.                 */
   /* * NOTE * A successful return code does not mean that the remote   */
   /*          PBAP server successfully processed the command.  The     */
   /*          caller needs to check the confirmation result to         */
   /*          determine if the remote PBAP Profile Server successfully */
   /*          executed the Request.                                    */
   /* * NOTE * There can only be one outstanding PBAP Profile Request   */
   /*          active at any one time.  Because of this, another PBAP   */
   /*          Profile Request cannot be issued until either the current*/
   /*          request is aborted (by calling the PBAM_Abort() function)*/
   /*          or the current request is completed (this is signified by*/
   /*          receiving a petPBASetPhoneBookStatus event in the PBAM   */
   /*          event callback that was registered when the PBAM port was*/
   /*          opened).                                                 */
int BTPSAPI PBAM_Set_Phone_Book(BD_ADDR_t RemoteDeviceAddress, PBAM_Set_Path_Option_t PathOption, char *FolderName)
{
   /* Simply foward the call to the handler with the Server ID.         */
   return(_Set_Phone_Book(RemoteDeviceAddress, PathOption, FolderName, MSG_GetServerAddressID()));
}

   /* The following function generates a PBAP Pull vCard Listing Request*/
   /* to the specified remote PBAP Server. The RemoteDeviceAddress      */
   /* parameter specifies the connected device for the request. The     */
   /* PhonebookPath Parameter specifies the name of the phonebook to    */
   /* pull the listing from. The ListOrder parameter is an enumerated   */
   /* type that determines the optionally requested order of listing.   */
   /* Using 'loDefault' will prevent the field from being added. The    */
   /* SearchAttribute parameter is an enumerated type that specifies    */
   /* the requested attribute to be used as a search filter. The        */
   /* SearchValue contains an optional ASCII string that contains the   */
   /* requested search value. If this is NULL, it will be excluded. The */
   /* MaxListCount is an unsigned integer that represents the maximum   */
   /* number of list entries to be returned. The ListStartOffset        */
   /* parameter specifies the index requested. This function returns    */
   /* zero if successful or a negative return error code if there was an*/
   /* error.                                                            */
   /* * NOTE * A successful return code does not mean that the remote   */
   /*          PBAP server successfully processed the command.  The     */
   /*          caller needs to check the confirmation result to         */
   /*          determine if the remote PBAP Profile Server successfully */
   /*          executed the Request.                                    */
   /* * NOTE * There can only be one outstanding PBAP Profile Request   */
   /*          active at any one time.  Because of this, another PBAP   */
   /*          Profile Request cannot be issued until either the current*/
   /*          request is aborted (by calling the PBAM_Abort() function)*/
   /*          or the current request is completed (this is signified by*/
   /*          receiving a petPBASetPhoneBookStatus event in the PBAM   */
   /*          event callback that was registered when the PBAM port was*/
   /*          opened).                                                 */
int BTPSAPI PBAM_Pull_vCard_Listing(BD_ADDR_t RemoteDeviceAddress, char *PhonebookPath, PBAM_List_Order_t ListOrder, PBAM_Search_Attribute_t SearchAttribute, char *SearchValue, Word_t MaxListCount, Word_t ListStartOffset)
{
   /* Simply foward the call to the handler with the Server ID.         */
   return(_Pull_vCard_Listing(RemoteDeviceAddress, PhonebookPath, ListOrder, SearchAttribute, SearchValue, MaxListCount, ListStartOffset, MSG_GetServerAddressID()));
}

   /* The following function generates a PBAP Pull vCard Entry Request  */
   /* to the specified remote PBAP Server. The RemoteDeviceAddress      */
   /* Parameter specifies the connected device for the request. The     */
   /* vCardName parameter is an ASCII string representing the name of   */
   /* the vCard to be pulled in the request. The FilterLow parameter    */
   /* contains the lower 32 bits of the 64-bit filter attribute. The    */
   /* FilterHigh parameter contains the higher 32 bits of the 64-bit    */
   /* filter attribute. The Format parameter is an enumeration which    */
   /* specifies the format of the vCard requested. This function returns*/
   /* zero if successful and a negative return error code if there was  */
   /* an error.                                                         */
   /* * NOTE * A successful return code does not mean that the remote   */
   /*          PBAP server successfully processed the command.  The     */
   /*          caller needs to check the confirmation result to         */
   /*          determine if the remote PBAP Profile Server successfully */
   /*          executed the Request.                                    */
   /* * NOTE * There can only be one outstanding PBAP Profile Request   */
   /*          active at any one time.  Because of this, another PBAP   */
   /*          Profile Request cannot be issued until either the current*/
   /*          request is aborted (by calling the PBAM_Abort() function)*/
   /*          or the current request is completed (this is signified by*/
   /*          receiving a petPBASetPhoneBookStatus event in the PBAM   */
   /*          event callback that was registered when the PBAM port was*/
   /*          opened).                                                 */
int BTPSAPI PBAM_Pull_vCard(BD_ADDR_t RemoteDeviceAddress, char *VCardName, DWord_t FilterLow, DWord_t FilterHigh, PBAM_VCard_Format_t VCardFormat)
{
   /* Simply foward the call to the handler with the Server ID.         */
   return(_Pull_vCard(RemoteDeviceAddress, VCardName, FilterLow, FilterHigh, VCardFormat, MSG_GetServerAddressID()));
}

   /* The following function wraps PBAP Set Phone Book Requests in order*/
   /* to supply an absolute path to change to. The RemoteDeviceAddress  */
   /* parameter specifies the connected device for the request. The     */
   /* AbsolutePath parameter is an ASCII string containing the path to  */
   /* set the phone book to. This function returns zero if successful   */
   /* and a negative return error code if there was and error.          */
   /* * NOTE * A successful return code does not mean that the remote   */
   /*          PBAP server successfully processed the command.  The     */
   /*          caller needs to check the confirmation result to         */
   /*          determine if the remote PBAP Profile Server successfully */
   /*          executed the Request.                                    */
   /* * NOTE * If there is an error while processing the series of      */
   /*          requests, a petPhoneBookSetEvent will be sent containing */
   /*          the path before the failure occurred. This will can be   */
   /*          assumed to be the current path.                          */
   /* * NOTE * There can only be one outstanding PBAP Profile Request   */
   /*          active at any one time.  Because of this, another PBAP   */
   /*          Profile Request cannot be issued until either the current*/
   /*          request is aborted (by calling the PBAM_Abort() function)*/
   /*          or the current request is completed (this is signified by*/
   /*          receiving a petPBASetPhoneBookStatus event in the PBAM   */
   /*          event callback that was registered when the PBAM port was*/
   /*          opened).                                                 */
int BTPSAPI PBAM_Set_Phone_Book_Absolute(BD_ADDR_t RemoteDeviceAddress, char *AbsolutePath)
{
   /* Simply foward the call to the handler with the Server ID.         */
  return(_Set_Phone_Book_Absolute(RemoteDeviceAddress, AbsolutePath, MSG_GetServerAddressID()));
}

   /* Register a PBAP Server Port. The ServerPort parameter specifies   */
   /* the RFCOMM port number on which to open the server. The           */
   /* SupportedRepositories parameter is a bit mask of the supported    */
   /* local contact database types. The IncomingConnectionFlags         */
   /* parameter is a bitmask that determine how to handle incoming      */
   /* connection requests to the server port. The ServiceName parameter */
   /* is the service name to insert into the SDP record for the         */
   /* server. The EventCallback parameter is the callback function      */
   /* that will receive asynchronous events for this server. The        */
   /* CallbackParameter will be passed to the EventCallback when events */
   /* are dispatched. On success, this function returns a positive,     */
   /* non-zero value representing the ServerID for the newly opened     */
   /* server. On failure, this function returns a negative error code.  */
   /* * NOTE * Supplying a ServerPort of 0 will cause this function to  */
   /*          automatically pick an available port number.             */
int BTPSAPI PBAM_Register_Server(unsigned int ServerPort, unsigned int SupportedRepositories, unsigned long IncomingConnectionFlags, char *ServiceName, PBAM_Event_Callback_t EventCallback, void *CallbackParameter)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Object Push Manager has been        */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, verfiy that the input parameters appear to be semi-valid */
      if(((!ServerPort) || ((ServerPort >= PBAP_PORT_NUMBER_MINIMUM) && (ServerPort <= PBAP_PORT_NUMBER_MAXIMUM))) && (EventCallback))
      {
         /* Next check to make sure we are powered up.                  */
         if(CurrentPowerState)
         {
            /* Wait for access to the lock that guards access to this   */
            /* module.                                                  */
            if(DEVM_AcquireLock())
            {
               /* Simply call the helper function to handle this        */
               /* request.                                              */
               ret_val = ProcessRegisterServer(MSG_GetServerAddressID(), ServerPort, SupportedRepositories, IncomingConnectionFlags, ServiceName, EventCallback, CallbackParameter);

               DEVM_ReleaseLock();
            }
            else
               ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
         }
         else
            ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* Unregisters a previously opened PBAP server port. The ServerID    */
   /* parameter is the ID of the server returned from a successful call */
   /* to PBAM_Register_Server(). This fuction returns zero if successful*/
   /* or a negative return error code if there was an error.            */
int BTPSAPI PBAM_Un_Register_Server(unsigned int ServerID)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Object Push Manager has been        */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, verfiy that the input parameters appear to be semi-valid */
      if(ServerID)
      {
         /* Next check to make sure we are powered up.                  */
         if(CurrentPowerState)
         {
            /* Wait for access to the lock that guards access to this   */
            /* module.                                                  */
            if(DEVM_AcquireLock())
            {
               /* Simply call the helper function to handle this        */
               /* request.                                              */
               ret_val = ProcessUnRegisterServer(MSG_GetServerAddressID(), ServerID);

               DEVM_ReleaseLock();
            }
            else
               ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
         }
         else
            ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* Respond to an outstanding connection request to the local         */
   /* server. The ConnectionID is the indentifier of the connection     */
   /* request returned in a petConnectionRequest event. The Accept      */
   /* parameter indicates whether the connection should be accepted or  */
   /* rejected. This function returns zero if successful or a negative  */
   /* return error code if there was an error.                          */
int BTPSAPI PBAM_Connection_Request_Response(unsigned int ConnectionID, Boolean_t Accept)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Object Push Manager has been        */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, verfiy that the input parameters appear to be semi-valid */
      if(ConnectionID)
      {
         /* Next check to make sure we are powered up.                  */
         if(CurrentPowerState)
         {
            /* Wait for access to the lock that guards access to this   */
            /* module.                                                  */
            if(DEVM_AcquireLock())
            {
               /* Simply call the helper function to handle this        */
               /* request.                                              */
               ret_val = ProcessConnectionRequestResponse(MSG_GetServerAddressID(), ConnectionID, Accept);

               DEVM_ReleaseLock();
            }
            else
               ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
         }
         else
            ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* Close an active connection to a local PBAP Server instance. The   */
   /* ConnectionID parameter is the identifier of the connection        */
   /* returned in a petConnected event. This function returns zero if   */
   /* successful or a negative return error code if there was an error. */
int BTPSAPI PBAM_Close_Server_Connection(unsigned int ConnectionID)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Object Push Manager has been        */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, verfiy that the input parameters appear to be semi-valid */
      if(ConnectionID)
      {
         /* Next check to make sure we are powered up.                  */
         if(CurrentPowerState)
         {
            /* Wait for access to the lock that guards access to this   */
            /* module.                                                  */
            if(DEVM_AcquireLock())
            {
               /* Simply call the helper function to handle this        */
               /* request.                                              */
               ret_val = ProcessCloseServerConnection(MSG_GetServerAddressID(), ConnectionID);

               DEVM_ReleaseLock();
            }
            else
               ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
         }
         else
            ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* Submits a response to a received etPullPhoneBook event. The       */
   /* ConnectionID parameter is the identifier of the active connection */
   /* returned in a petConnected event. The ResponseStatusCode parameter*/
   /* is one of the defined PBAM Response Status codes. If the request  */
   /* event indicated the 'mch' phonebook, the NumberMissedCalls        */
   /* parameter should be a pointer to the value of the number of missed*/
   /* calls since the last 'mch' pull request. If it is not an 'mch'    */
   /* request, this parameter should be set to NULL. The BufferSize     */
   /* parameter indicates the amount of data in the buffer to be        */
   /* sent. The Buffer parameter is a pointer to the phone book data    */
   /* to send. The Final parameter should be set to FALSE if there is   */
   /* more data to be sent after this buffer or TRUE if there is no more*/
   /* data. This function returns zero if successful and a negative     */
   /* return error code if there was an error.                          */
int BTPSAPI PBAM_Send_Phone_Book(unsigned int ConnectionID, unsigned int ResponseStatusCode, Byte_t *NewMissedCalls, unsigned int BufferSize, Byte_t *Buffer, Boolean_t Final)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Object Push Manager has been        */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, verfiy that the input parameters appear to be semi-valid */
      if((ConnectionID) && (!BufferSize || (Buffer)))
      {
         /* Next check to make sure we are powered up.                  */
         if(CurrentPowerState)
         {
            /* Wait for access to the lock that guards access to this   */
            /* module.                                                  */
            if(DEVM_AcquireLock())
            {
               /* Simply call the helper function to handle this        */
               /* request.                                              */
               ret_val = ProcessSendPhoneBook(MSG_GetServerAddressID(), ConnectionID, ResponseStatusCode, NewMissedCalls, BufferSize, Buffer, Final);

               DEVM_ReleaseLock();
            }
            else
               ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
         }
         else
            ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* Submits a response to a received etPullPhoneBookSize event. The   */
   /* ConnectionID parameter is the identifier of the activer connection*/
   /* returned in a petConnected event. The ResponseStatusCode          */
   /* parameter is one of the defined PBAM Response Status codes. The   */
   /* PhonebookSize parameter indicates the number of entries in the    */
   /* requested phone book. This function returns zero if successful or */
   /* a negative return error code if there was an error.               */
int BTPSAPI PBAM_Send_Phone_Book_Size(unsigned int ConnectionID, unsigned int ResponseStatusCode, unsigned int PhoneBookSize)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Object Push Manager has been        */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, verfiy that the input parameters appear to be semi-valid */
      if(ConnectionID)
      {
         /* Next check to make sure we are powered up.                  */
         if(CurrentPowerState)
         {
            /* Wait for access to the lock that guards access to this   */
            /* module.                                                  */
            if(DEVM_AcquireLock())
            {
               /* Simply call the helper function to handle this        */
               /* request.                                              */
               ret_val = ProcessSendPhoneBookSize(MSG_GetServerAddressID(), ConnectionID, ResponseStatusCode, PhoneBookSize);

               DEVM_ReleaseLock();
            }
            else
               ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
         }
         else
            ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* Submits a response to a received petSetPhoneBook event. The       */
   /* ConnectionID parameter is the identifier of the activer connection*/
   /* returned in a petConnected event. The ResponseStatusCode parameter*/
   /* is one of the defined PBAM Response Status codes. This function   */
   /* returns zero if successful or a negative return error code if     */
   /* there was an error.                                               */
int BTPSAPI PBAM_Set_Phone_Book_Response(unsigned int ConnectionID, unsigned int ResponseStatusCode)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Object Push Manager has been        */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, verfiy that the input parameters appear to be semi-valid */
      if(ConnectionID)
      {
         /* Next check to make sure we are powered up.                  */
         if(CurrentPowerState)
         {
            /* Wait for access to the lock that guards access to this   */
            /* module.                                                  */
            if(DEVM_AcquireLock())
            {
               /* Simply call the helper function to handle this        */
               /* request.                                              */
               ret_val = ProcessSetPhoneBookResponse(MSG_GetServerAddressID(), ConnectionID, ResponseStatusCode);

               DEVM_ReleaseLock();
            }
            else
               ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
         }
         else
            ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* Submits a response to a received petPullvCardListing event. The   */
   /* ConnectionID parameter is the identifier of the activer connection*/
   /* returned in a petConnected event. The ResponseStatusCode parameter*/
   /* is one of the defined PBAM Response Status codes.  If the request */
   /* event indicated the 'mch' phonebook, the NumberMissedCalls        */
   /* parameter should be a pointer to the value of the number of missed*/
   /* calls since the last 'mch' pull request. If it is not an 'mch'    */
   /* request, this parameter should be set to NULL. The BufferSize     */
   /* parameter indicates the amount of data in the buffer to be        */
   /* sent. The Buffer parameter is a pointer to the vCardListing data  */
   /* to send. The Final parameter should be set to FALSE if there is   */
   /* more data to be sent after this buffer or TRUE if there is no more*/
   /* data. This function returns zero if successful and a negative     */
   /* return error code if there was an error.                          */
int BTPSAPI PBAM_Send_vCard_Listing(unsigned int ConnectionID, unsigned int ResponseStatusCode, Byte_t *NewMissedCalls, unsigned int BufferSize, Byte_t *Buffer, Boolean_t Final)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Object Push Manager has been        */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, verfiy that the input parameters appear to be semi-valid */
      if((ConnectionID) && ((!BufferSize) || (Buffer)))
      {
         /* Next check to make sure we are powered up.                  */
         if(CurrentPowerState)
         {
            /* Wait for access to the lock that guards access to this   */
            /* module.                                                  */
            if(DEVM_AcquireLock())
            {
               /* Simply call the helper function to handle this        */
               /* request.                                              */
               ret_val = ProcessSendvCardListing(MSG_GetServerAddressID(), ConnectionID, ResponseStatusCode, NewMissedCalls, BufferSize, Buffer, Final);

               DEVM_ReleaseLock();
            }
            else
               ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
         }
         else
            ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* Submits a response to a received petPullvCardListingSize          */
   /* event. The ConnectionID parameter is the identifier of the        */
   /* activer connection returned in a petConnected event. The          */
   /* ResponseStatusCode parameter is one of the defined PBAM Response  */
   /* Status codes. The vCardListingSize parameter indicates the number */
   /* of vCard entries in the current/specfied folder. This function    */
   /* returns zero if successful or a negative return error code if     */
   /* there was an error.                                               */
int BTPSAPI PBAM_Send_vCard_Listing_Size(unsigned int ConnectionID, unsigned int ResponseStatusCode, unsigned int vCardListingSize)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Object Push Manager has been        */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, verfiy that the input parameters appear to be semi-valid */
      if(ConnectionID)
      {
         /* Next check to make sure we are powered up.                  */
         if(CurrentPowerState)
         {
            /* Wait for access to the lock that guards access to this   */
            /* module.                                                  */
            if(DEVM_AcquireLock())
            {
               /* Simply call the helper function to handle this        */
               /* request.                                              */
               ret_val = ProcessSendvCardListingSize(MSG_GetServerAddressID(), ConnectionID, ResponseStatusCode, vCardListingSize);

               DEVM_ReleaseLock();
            }
            else
               ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
         }
         else
            ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* Submits a response to a received petPullvCard event. The          */
   /* ConnectionID parameter is the identifier of the activer connection*/
   /* returned in a petConnected event. The ResponseStatusCode parameter*/
   /* is one of the defined PBAM Response Status codes.  The BufferSize */
   /* parameter indicates the amount of data in the buffer to be        */
   /* sent. The Buffer parameter is a pointer to the vCard data to      */
   /* send. The Final parameter should be set to FALSE if there is more */
   /* data to be sent after this buffer or TRUE if there is no more     */
   /* data. This function returns zero if successful and a negative     */
   /* return error code if there was an error.                          */
int BTPSAPI PBAM_Send_vCard(unsigned int ConnectionID, unsigned int ResponseStatusCode, unsigned int BufferSize, Byte_t *Buffer, Boolean_t Final)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Object Push Manager has been        */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, verfiy that the input parameters appear to be semi-valid */
      if((ConnectionID) && ((!BufferSize) || (Buffer)))
      {
         /* Next check to make sure we are powered up.                  */
         if(CurrentPowerState)
         {
            /* Wait for access to the lock that guards access to this   */
            /* module.                                                  */
            if(DEVM_AcquireLock())
            {
               /* Simply call the helper function to handle this        */
               /* request.                                              */
               ret_val = ProcessSendvCard(MSG_GetServerAddressID(), ConnectionID, ResponseStatusCode, BufferSize, Buffer, Final);

               DEVM_ReleaseLock();
            }
            else
               ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
         }
         else
            ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

