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

   /* Structure which is used to track Callback Information related to  */
   /* this module.                                                      */
typedef struct _tagPBAM_Entry_Info_t
{
   unsigned int                  ConnectionStatus;
   Event_t                       ConnectionEvent;
   unsigned long                 Flags;
   BD_ADDR_t                     BluetoothAddress;
   PBAM_Event_Callback_t         EventCallback;
   void                         *CallbackParameter;
   struct _tagPBAM_Entry_Info_t *NextPBAMEntryInfoPtr;
} PBAM_Entry_Info_t;

   /* The following functions are used with the Flags member of the     */
   /* PBAM_Entry_Info_t structure to denote various state information.  */
#define PBAM_ENTRY_INFO_FLAGS_SYNCHRONOUS_CONNECT_REQUEST   0x20000000
#define PBAM_ENTRY_INFO_FLAGS_LOCALLY_HANDLED               0x40000000
#define PBAM_ENTRY_INFO_FLAGS_EVENT_CLOSING                 0x80000000

   /* Structure which is used to track Callback Information related to  */
   /* this module's server instances.                                   */
typedef struct _tagPBAM_Server_Entry_t
{
   unsigned int                    ServerID;
   PBAM_Event_Callback_t           EventCallback;
   void                           *CallbackParameter;
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

   /* Variables which hold a pointer to the first element in the Phone  */
   /* Book Access entry information list (which holds all callbacks     */
   /* tracked by this module).                                          */
static PBAM_Entry_Info_t *PBAMEntryInfoList;
static PBAM_Server_Entry_t *PBAMServerList;

   /* Custom list routines to add/remove/free the specific type of      */
   /* structure PBAM_Entry_Info_t.                                      */
static PBAM_Entry_Info_t *AddPBAMEntryInfoEntry(PBAM_Entry_Info_t **ListHead, PBAM_Entry_Info_t *EntryToAdd);
static PBAM_Entry_Info_t *SearchPBAMEntryInfoEntry(PBAM_Entry_Info_t **ListHead, BD_ADDR_t *BluetoothAddress);
static PBAM_Entry_Info_t *DeletePBAMEntryInfoEntry(PBAM_Entry_Info_t **ListHead, BD_ADDR_t *BluetoothAddress);
static void FreePBAMEntryInfoEntryMemory(PBAM_Entry_Info_t *EntryToFree);
static void FreePBAMEntryInfoList(PBAM_Entry_Info_t **ListHead);

   /* Custom list routines to add/remove/free the specific type of      */
   /* structure PBAM_Server_Entry_t.                                    */
static PBAM_Server_Entry_t *AddPBAMServerEntry(PBAM_Server_Entry_t **ListHead, PBAM_Server_Entry_t *EntryToAdd);
static PBAM_Server_Entry_t *SearchPBAMServerEntry(PBAM_Server_Entry_t **ListHead, unsigned int ServerID);
static PBAM_Server_Entry_t *DeletePBAMServerEntry(PBAM_Server_Entry_t **ListHead, unsigned int ServerID);
static void FreePBAMServerEntryMemory(PBAM_Server_Entry_t *EntryToFree);
static void FreePBAMServerList(PBAM_Server_Entry_t **ListHead);

   /* BluetopiaPM server IPC events process handling                    */
static void ProcessDeviceConnectionStatusEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int ConnectionStatus);
static void ProcessDeviceDisconnectionEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int DisconnectReason, unsigned int ServerID, unsigned int ConnectionID);
static void ProcessPhoneBookSizeEvent(BD_ADDR_t RemoteDeviceAddress, int Status, unsigned int PhoneBookSize);
static void ProcessPhoneBookSetEvent(BD_ADDR_t RemoteDeviceAddress, int Status, unsigned int CurrentPathSize, Byte_t *CurrentPath);
static void ProcessVCardDataEvent(BD_ADDR_t RemoteDeviceAddress, int Status, Boolean_t Final, unsigned int NewMissedCalls, PBAM_VCard_Format_t Format, unsigned int BufferSize, Byte_t *Buffer);
static void ProcessVCardListingDataEvent(BD_ADDR_t RemoteDeviceAddress, int Status, Boolean_t Final, unsigned int NewMissedCalls, unsigned int BufferSize, Byte_t *Buffer);

   /* BluetopiaPM server IPC events process handling (PSE).             */
static void ProcessConnectionRequestEvent(PBAM_Connection_Request_Message_t *Message);
static void ProcessConnectedEvent(PBAM_Connected_Message_t *Message);
static void ProcessPullPhoneBookEvent(PBAM_Pull_Phone_Book_Message_t *Message);
static void ProcessPullPhoneBookSizeEvent(PBAM_Pull_Phone_Book_Size_Message_t *Message);
static void ProcessSetPhoneBookEvent(PBAM_Set_Phone_Book_Message_t *Message);
static void ProcessPullvCardListingEvent(PBAM_Pull_vCard_Listing_Message_t *Message);
static void ProcessPullvCardListingSizeEvent(PBAM_Pull_vCard_Listing_Size_Message_t *Message);
static void ProcessPullvCardEvent(PBAM_Pull_vCard_Message_t *Message);
static void ProcessAbortedEvent(PBAM_Aborted_Message_t *Message);

static void ProcessReceivedMessage(BTPM_Message_t *Message);

static void BTPSAPI BTPMDispatchCallback_PBAM(void *CallbackParameter);
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter);

static void BTPSAPI PhoneBookAccessGroupHandler(BTPM_Message_t *Message, void *CallbackParameter);

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
                  /* OK, we need to see if we are at the last element   */
                  /* of the List.  If we are, we simply break out of    */
                  /* the list traversal because we know there are NO    */
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
   /* specified Bluetooth Address. This function returns NULL if either */
   /* the parameters are invalid, or the specified callback ID was NOT  */
   /* found.                                                            */
static PBAM_Entry_Info_t *SearchPBAMEntryInfoEntry(PBAM_Entry_Info_t **ListHead, BD_ADDR_t *BluetoothAddress)
{
   PBAM_Entry_Info_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's make sure the list and device to search for appear to be    */
   /* valid.                                                            */
   if((ListHead) && (BluetoothAddress))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (!COMPARE_BD_ADDR(FoundEntry->BluetoothAddress, *BluetoothAddress)))
         FoundEntry = FoundEntry->NextPBAMEntryInfoPtr;
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
static PBAM_Entry_Info_t *DeletePBAMEntryInfoEntry(PBAM_Entry_Info_t **ListHead, BD_ADDR_t *BluetoothAddress)
{
   PBAM_Entry_Info_t *FoundEntry = NULL;
   PBAM_Entry_Info_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's make sure the list and device to search for appear to be    */
   /* semi-valid.                                                       */
   if((ListHead) && (BluetoothAddress))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (!COMPARE_BD_ADDR(FoundEntry->BluetoothAddress, *BluetoothAddress)))
      {
         LastEntry  = FoundEntry;
         FoundEntry = FoundEntry->NextPBAMEntryInfoPtr;
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

static PBAM_Server_Entry_t *AddPBAMServerEntry(PBAM_Server_Entry_t **ListHead, PBAM_Server_Entry_t *EntryToAdd)
{
   PBAM_Server_Entry_t *AddedEntry = NULL;
   PBAM_Server_Entry_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First let's verify that values passed in are semi-valid.          */
   if((ListHead) && (EntryToAdd))
   {
      /* OK, data seems semi-valid, let's allocate a new data structure */
      /* to add to the list.                                            */
      AddedEntry = (PBAM_Server_Entry_t *)BTPS_AllocateMemory(sizeof(PBAM_Server_Entry_t));

      if(AddedEntry)
      {
         /* Copy All Data over.                                         */
         *AddedEntry                    = *EntryToAdd;

         /* Now Add it to the end of the list.                          */
         AddedEntry->NextServerEntryPtr = NULL;

         /* First, let's check to see if there are any elements already */
         /* present in the List that was passed in.                     */
         if((tmpEntry = *ListHead) != NULL)
         {
            /* Head Pointer was not NULL, so we will traverse the list  */
            /* until we reach the last element.                         */
            while(tmpEntry)
            {
               if(tmpEntry->ServerID != AddedEntry->ServerID)
               {
                  /* Entry was already added, so free the memory and    */
                  /* flag an error to the caller.                       */
                  FreePBAMServerEntryMemory(AddedEntry);
                  AddedEntry = NULL;

                  /* Abort the Search.                                  */
                  tmpEntry   = NULL;
               }
               else
               {
                  /* OK, we need to see if we are at the last element   */
                  /* of the List.  If we are, we simply break out of    */
                  /* the list traversal because we know there are NO    */
                  /* duplicates AND we are at the end of the list.      */
                  if(tmpEntry->NextServerEntryPtr)
                     tmpEntry = tmpEntry->NextServerEntryPtr;
                  else
                     break;
               }
            }

            if(AddedEntry)
            {
               /* Last element found, simply Add the entry.             */
               tmpEntry->NextServerEntryPtr = AddedEntry;
            }
         }
         else
            *ListHead = AddedEntry;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", AddedEntry));

   return(AddedEntry);
}

static PBAM_Server_Entry_t *SearchPBAMServerEntry(PBAM_Server_Entry_t **ListHead, unsigned int ServerID)
{
   PBAM_Server_Entry_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's make sure the list and device to search for appear to be    */
   /* valid.                                                            */
   if((ListHead) && (ServerID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->ServerID != ServerID))
         FoundEntry = FoundEntry->NextServerEntryPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

static PBAM_Server_Entry_t *DeletePBAMServerEntry(PBAM_Server_Entry_t **ListHead, unsigned int ServerID)
{
   PBAM_Server_Entry_t *FoundEntry = NULL;
   PBAM_Server_Entry_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's make sure the list and device to search for appear to be    */
   /* semi-valid.                                                       */
   if((ListHead) && (ServerID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->ServerID != ServerID))
      {
         LastEntry  = FoundEntry;
         FoundEntry = FoundEntry->NextServerEntryPtr;
      }

      /* Check to see if we found the specified entry.                  */
      if(FoundEntry)
      {
         /* OK, now let's remove the entry from the list.  We have to   */
         /* check to see if the entry was the first entry in the list.  */
         if(LastEntry)
         {
            /* Entry was NOT the first entry in the list.               */
            LastEntry->NextServerEntryPtr = FoundEntry->NextServerEntryPtr;
         }
         else
            *ListHead = FoundEntry->NextServerEntryPtr;

         FoundEntry->NextServerEntryPtr = NULL;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

static void FreePBAMServerEntryMemory(PBAM_Server_Entry_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToFree)
      BTPS_FreeMemory(EntryToFree);

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void FreePBAMServerList(PBAM_Server_Entry_t **ListHead)
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

   /* The following function is responsible for processing the device   */
   /* connection status asynchronous message.                           */
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
      if(((PBAMEntryInfo = SearchPBAMEntryInfoEntry(&PBAMEntryInfoList, &RemoteDeviceAddress)) != NULL))
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

            /* If there was an error we need to delete the device.      */
            if(ConnectionStatus)
            {
               if((PBAMEntryInfo = DeletePBAMEntryInfoEntry(&PBAMEntryInfoList, &RemoteDeviceAddress)) != NULL)
                  FreePBAMEntryInfoEntryMemory(PBAMEntryInfo);
            }

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
static void ProcessDeviceDisconnectionEvent(BD_ADDR_t RemoteDeviceAddress, unsigned int DisconnectReason, unsigned int ServerID, unsigned int ConnectionID)
{
   PBAM_Event_Data_t      EventData;
   PBAM_Entry_Info_t     *PBAMEntryInfo;
   PBAM_Server_Entry_t   *PBAMServerEntry;
   PBAM_Event_Callback_t  EventCallback;
   void                  *CallbackParameter;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Delete the device on disconnect                                   */
   if(((PBAMEntryInfo = DeletePBAMEntryInfoEntry(&PBAMEntryInfoList, &RemoteDeviceAddress)) != NULL))
   {
      /* Note the Callback information.                                 */
      EventCallback     = PBAMEntryInfo->EventCallback;
      CallbackParameter = PBAMEntryInfo->CallbackParameter;
   }
   else
   {
      if((PBAMServerEntry = SearchPBAMServerEntry(&PBAMServerList, ServerID)) != NULL)
      {
         /* Note the Callback information.                              */
         EventCallback     = PBAMServerEntry->EventCallback;
         CallbackParameter = PBAMServerEntry->CallbackParameter;
      }
      else
         EventCallback = NULL;
   }

   if(EventCallback)
   {
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
static void ProcessPhoneBookSizeEvent(BD_ADDR_t RemoteDeviceAddress, int Status, unsigned int PhoneBookSize)
{
   PBAM_Event_Data_t      EventData;
   PBAM_Entry_Info_t     *PBAMEntryInfo;
   PBAM_Event_Callback_t  EventCallback;
   void                  *CallbackParameter;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(((PBAMEntryInfo = SearchPBAMEntryInfoEntry(&PBAMEntryInfoList, &RemoteDeviceAddress)) != NULL))
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
static void ProcessPhoneBookSetEvent(BD_ADDR_t RemoteDeviceAddress, int Status, unsigned int CurrentPathSize, Byte_t *CurrentPath)
{
   PBAM_Event_Data_t     *EventData;
   PBAM_Entry_Info_t     *PBAMEntryInfo;
   PBAM_Event_Callback_t  EventCallback;
   void                  *CallbackParameter;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter Path Size: %d\n", CurrentPathSize));

   if(((PBAMEntryInfo = SearchPBAMEntryInfoEntry(&PBAMEntryInfoList, &RemoteDeviceAddress)) != NULL))
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
   /* vcard data asynchronous message.                                  */
static void ProcessVCardDataEvent(BD_ADDR_t RemoteDeviceAddress, int Status, Boolean_t Final, unsigned int NewMissedCalls, PBAM_VCard_Format_t Format, unsigned int BufferSize, Byte_t *Buffer)
{
   PBAM_Event_Data_t     *EventData;
   PBAM_Entry_Info_t     *PBAMEntryInfo;
   PBAM_Event_Callback_t  EventCallback;
   void                  *CallbackParameter;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(((PBAMEntryInfo = SearchPBAMEntryInfoEntry(&PBAMEntryInfoList, &RemoteDeviceAddress)) != NULL))
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
   /* vcard data listing asynchronous message.                          */
static void ProcessVCardListingDataEvent(BD_ADDR_t RemoteDeviceAddress, int Status, Boolean_t Final, unsigned int NewMissedCalls, unsigned int BufferSize, Byte_t *Buffer)
{
   PBAM_Event_Data_t     *EventData;
   PBAM_Entry_Info_t     *PBAMEntryInfo;
   PBAM_Event_Callback_t  EventCallback;
   void                  *CallbackParameter;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(((PBAMEntryInfo = SearchPBAMEntryInfoEntry(&PBAMEntryInfoList, &RemoteDeviceAddress)) != NULL))
   {
      /* Note the Callback information.                                 */
      EventCallback     = PBAMEntryInfo->EventCallback;
      CallbackParameter = PBAMEntryInfo->CallbackParameter;

      if((EventData = (PBAM_Event_Data_t *)BTPS_AllocateMemory(STRUCTURE_OFFSET(PBAM_Event_Data_t, EventData) + PBAM_VCARD_LISTING_EVENT_DATA_SIZE(BufferSize))) != NULL)
      {
         /* Event needs to be dispatched. Go ahead and format the event.*/

         BTPS_MemInitialize(EventData, 0, (STRUCTURE_OFFSET(PBAM_Event_Data_t, EventData) + PBAM_VCARD_LISTING_EVENT_DATA_SIZE(BufferSize)));

         /* Format up the Event.                                        */
         EventData->EventType                                    = petVCardListing;
         EventData->EventLength                                  = PBAM_VCARD_EVENT_DATA_SIZE(BufferSize);

         EventData->EventData.VCardListingEventData.RemoteDeviceAddress = RemoteDeviceAddress;
         EventData->EventData.VCardListingEventData.Status              = Status;
         EventData->EventData.VCardListingEventData.Final               = Final;
         EventData->EventData.VCardListingEventData.NewMissedCalls      = NewMissedCalls;
         EventData->EventData.VCardListingEventData.BufferSize          = BufferSize;

         if(BufferSize)
            BTPS_MemCopy(EventData->EventData.VCardListingEventData.Buffer, Buffer, BufferSize);

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

   /* The following function is responsible for processing the          */
   /* connection request asynchronous message.                          */
static void ProcessConnectionRequestEvent(PBAM_Connection_Request_Message_t *Message)
{
   PBAM_Event_Data_t      EventData;
   PBAM_Server_Entry_t   *PBAMEntryInfo;
   PBAM_Event_Callback_t  EventCallback;
   void                  *CallbackParameter;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(((PBAMEntryInfo = SearchPBAMServerEntry(&PBAMServerList, Message->ServerID)) != NULL))
   {
      /* Note the Callback information.                                 */
      EventCallback     = PBAMEntryInfo->EventCallback;
      CallbackParameter = PBAMEntryInfo->CallbackParameter;

      /* Go ahead and format the event.                                 */

      BTPS_MemInitialize(&EventData, 0, sizeof(EventData));

      EventData.EventType                                                = petConnectionRequest;
      EventData.EventLength                                              = PBAM_CONNECTION_REQUEST_EVENT_DATA_SIZE;

      EventData.EventData.ConnectionRequestEventData.ConnectionID        = Message->ConnectionID;
      EventData.EventData.ConnectionRequestEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;
      EventData.EventData.ConnectionRequestEventData.ServerID            = Message->ServerID;

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

      DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_CRITICAL), ("Unable to server instance\n"));
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the connected*/
   /* asynchronous message.                                             */
static void ProcessConnectedEvent(PBAM_Connected_Message_t *Message)
{
   PBAM_Event_Data_t      EventData;
   PBAM_Server_Entry_t   *PBAMEntryInfo;
   PBAM_Event_Callback_t  EventCallback;
   void                  *CallbackParameter;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(((PBAMEntryInfo = SearchPBAMServerEntry(&PBAMServerList, Message->ServerID)) != NULL))
   {
      /* Note the Callback information.                                 */
      EventCallback     = PBAMEntryInfo->EventCallback;
      CallbackParameter = PBAMEntryInfo->CallbackParameter;

      /* Go ahead and format the event.                                 */

      BTPS_MemInitialize(&EventData, 0, sizeof(EventData));

      EventData.EventType                                        = petConnected;
      EventData.EventLength                                      = PBAM_CONNECTED_EVENT_DATA_SIZE;

      EventData.EventData.ConnectedEventData.ConnectionID        = Message->ConnectionID;
      EventData.EventData.ConnectedEventData.ServerID            = Message->ServerID;
      EventData.EventData.ConnectedEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;

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

      DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_CRITICAL), ("Unable to server instance\n"));
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the pull     */
   /* phone book asynchronous message.                                  */
static void ProcessPullPhoneBookEvent(PBAM_Pull_Phone_Book_Message_t *Message)
{
   PBAM_Event_Data_t      EventData;
   PBAM_Server_Entry_t   *PBAMEntryInfo;
   PBAM_Event_Callback_t  EventCallback;
   void                  *CallbackParameter;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(((PBAMEntryInfo = SearchPBAMServerEntry(&PBAMServerList, Message->ServerID)) != NULL))
   {
      /* Note the Callback information.                                 */
      EventCallback     = PBAMEntryInfo->EventCallback;
      CallbackParameter = PBAMEntryInfo->CallbackParameter;

      /* Go ahead and format the event.                                 */

      BTPS_MemInitialize(&EventData, 0, sizeof(EventData));

      EventData.EventType                                        = petPullPhoneBook;
      EventData.EventLength                                      = PBAM_PULL_PHONE_BOOK_EVENT_DATA_SIZE;

      EventData.EventData.PullPhoneBookEventData.ConnectionID    = Message->ConnectionID;
      EventData.EventData.PullPhoneBookEventData.vCardFormat     = Message->vCardFormat;
      EventData.EventData.PullPhoneBookEventData.FilterLow       = Message->FilterLow;
      EventData.EventData.PullPhoneBookEventData.FilterHigh      = Message->FilterHigh;
      EventData.EventData.PullPhoneBookEventData.MaxListCount    = Message->MaxListCount;
      EventData.EventData.PullPhoneBookEventData.ListStartOffset = Message->ListStartOffset;

      if(Message->ObjectNameLength)
         EventData.EventData.PullPhoneBookEventData.ObjectName = Message->ObjectName;

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

      DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_CRITICAL), ("Unable to server instance\n"));
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the pull     */
   /* phone book size asynchronous message.                             */
static void ProcessPullPhoneBookSizeEvent(PBAM_Pull_Phone_Book_Size_Message_t *Message)
{
   PBAM_Event_Data_t      EventData;
   PBAM_Server_Entry_t   *PBAMEntryInfo;
   PBAM_Event_Callback_t  EventCallback;
   void                  *CallbackParameter;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(((PBAMEntryInfo = SearchPBAMServerEntry(&PBAMServerList, Message->ServerID)) != NULL))
   {
      /* Note the Callback information.                                 */
      EventCallback     = PBAMEntryInfo->EventCallback;
      CallbackParameter = PBAMEntryInfo->CallbackParameter;

      /* Go ahead and format the event.                                 */

      BTPS_MemInitialize(&EventData, 0, sizeof(EventData));

      EventData.EventType                                         = petPullPhoneBookSize;
      EventData.EventLength                                       = PBAM_PULL_PHONE_BOOK_SIZE_EVENT_DATA_SIZE;

      EventData.EventData.PullPhoneBookSizeEventData.ConnectionID = Message->ConnectionID;

      if(Message->ObjectNameLength)
         EventData.EventData.PullPhoneBookSizeEventData.ObjectName   = Message->ObjectName;

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

      DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_CRITICAL), ("Unable to server instance\n"));
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the set phone*/
   /* book asynchronous message.                                        */
static void ProcessSetPhoneBookEvent(PBAM_Set_Phone_Book_Message_t *Message)
{
   PBAM_Event_Data_t      EventData;
   PBAM_Server_Entry_t   *PBAMEntryInfo;
   PBAM_Event_Callback_t  EventCallback;
   void                  *CallbackParameter;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(((PBAMEntryInfo = SearchPBAMServerEntry(&PBAMServerList, Message->ServerID)) != NULL))
   {
      /* Note the Callback information.                                 */
      EventCallback     = PBAMEntryInfo->EventCallback;
      CallbackParameter = PBAMEntryInfo->CallbackParameter;

      /* Go ahead and format the event.                                 */

      BTPS_MemInitialize(&EventData, 0, sizeof(EventData));

      EventData.EventType                                    = petSetPhoneBook;
      EventData.EventLength                                  = PBAM_SET_PHONE_BOOK_EVENT_DATA_SIZE;

      EventData.EventData.SetPhoneBookEventData.ConnectionID = Message->ConnectionID;
      EventData.EventData.SetPhoneBookEventData.PathOption   = Message->PathOption;

      if(Message->ObjectNameLength)
         EventData.EventData.SetPhoneBookEventData.ObjectName = Message->ObjectName;

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

      DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_CRITICAL), ("Unable to server instance\n"));
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the pull     */
   /* vCard listing asynchronous message.                               */
static void ProcessPullvCardListingEvent(PBAM_Pull_vCard_Listing_Message_t *Message)
{
   PBAM_Event_Data_t      EventData;
   PBAM_Server_Entry_t   *PBAMEntryInfo;
   PBAM_Event_Callback_t  EventCallback;
   void                  *CallbackParameter;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(((PBAMEntryInfo = SearchPBAMServerEntry(&PBAMServerList, Message->ServerID)) != NULL))
   {
      /* Note the Callback information.                                 */
      EventCallback     = PBAMEntryInfo->EventCallback;
      CallbackParameter = PBAMEntryInfo->CallbackParameter;

      /* Go ahead and format the event.                                 */

      BTPS_MemInitialize(&EventData, 0, sizeof(EventData));

      EventData.EventType                                    = petPullvCardListing;
      EventData.EventLength                                  = PBAM_PULL_VCARD_LISTING_EVENT_DATA_SIZE;

      EventData.EventData.PullvCardListingEventData.ConnectionID    = Message->ConnectionID;
      EventData.EventData.PullvCardListingEventData.ListOrder       = Message->ListOrder;
      EventData.EventData.PullvCardListingEventData.SearchAttribute = Message->SearchAttribute;
      EventData.EventData.PullvCardListingEventData.MaxListCount    = Message->MaxListCount;
      EventData.EventData.PullvCardListingEventData.ListStartOffset = Message->ListStartOffset;

      if(Message->ObjectNameLength)
         EventData.EventData.PullvCardListingEventData.ObjectName = (char *)Message->VariableData;

      if(Message->SearchValueLength)
         EventData.EventData.PullvCardListingEventData.SearchValue = (char *)(Message->VariableData + Message->ObjectNameLength);

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

      DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_CRITICAL), ("Unable to server instance\n"));
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the          */
   /* asynchronous message.                                             */
static void ProcessPullvCardListingSizeEvent(PBAM_Pull_vCard_Listing_Size_Message_t *Message)
{
   PBAM_Event_Data_t      EventData;
   PBAM_Server_Entry_t   *PBAMEntryInfo;
   PBAM_Event_Callback_t  EventCallback;
   void                  *CallbackParameter;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(((PBAMEntryInfo = SearchPBAMServerEntry(&PBAMServerList, Message->ServerID)) != NULL))
   {
      /* Note the Callback information.                                 */
      EventCallback     = PBAMEntryInfo->EventCallback;
      CallbackParameter = PBAMEntryInfo->CallbackParameter;

      /* Go ahead and format the event.                                 */

      BTPS_MemInitialize(&EventData, 0, sizeof(EventData));

      EventData.EventType                                            = petPullvCardListingSize;
      EventData.EventLength                                          = PBAM_PULL_VCARD_LISTING_SIZE_EVENT_DATA_SIZE;

      EventData.EventData.PullvCardListingSizeEventData.ConnectionID = Message->ConnectionID;

      if(Message->ObjectNameLength)
         EventData.EventData.PullvCardListingSizeEventData.ObjectName = Message->ObjectName;

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

      DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_CRITICAL), ("Unable to server instance\n"));
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the          */
   /* asynchronous message.                                             */
static void ProcessPullvCardEvent(PBAM_Pull_vCard_Message_t *Message)
{
   PBAM_Event_Data_t      EventData;
   PBAM_Server_Entry_t   *PBAMEntryInfo;
   PBAM_Event_Callback_t  EventCallback;
   void                  *CallbackParameter;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(((PBAMEntryInfo = SearchPBAMServerEntry(&PBAMServerList, Message->ServerID)) != NULL))
   {
      /* Note the Callback information.                                 */
      EventCallback     = PBAMEntryInfo->EventCallback;
      CallbackParameter = PBAMEntryInfo->CallbackParameter;

      /* Go ahead and format the event.                                 */

      BTPS_MemInitialize(&EventData, 0, sizeof(EventData));

      EventData.EventType                                    = petPullvCard;
      EventData.EventLength                                  = PBAM_PULL_VCARD_EVENT_DATA_SIZE;

      EventData.EventData.PullvCardEventData.ConnectionID = Message->ConnectionID;
      EventData.EventData.PullvCardEventData.Format       = Message->vCardFormat;
      EventData.EventData.PullvCardEventData.FilterLow    = Message->FilterLow;
      EventData.EventData.PullvCardEventData.FilterHigh   = Message->FilterHigh;

      if(Message->ObjectNameLength)
         EventData.EventData.PullvCardEventData.ObjectName = Message->ObjectName;

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

      DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_CRITICAL), ("Unable to server instance\n"));
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the          */
   /* asynchronous message.                                             */
static void ProcessAbortedEvent(PBAM_Aborted_Message_t *Message)
{
   PBAM_Event_Data_t      EventData;
   PBAM_Server_Entry_t   *PBAMEntryInfo;
   PBAM_Event_Callback_t  EventCallback;
   void                  *CallbackParameter;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(((PBAMEntryInfo = SearchPBAMServerEntry(&PBAMServerList, Message->ServerID)) != NULL))
   {
      /* Note the Callback information.                                 */
      EventCallback     = PBAMEntryInfo->EventCallback;
      CallbackParameter = PBAMEntryInfo->CallbackParameter;

      /* Go ahead and format the event.                                 */

      BTPS_MemInitialize(&EventData, 0, sizeof(EventData));

      EventData.EventType                               = petAborted;
      EventData.EventLength                             = PBAM_ABORTED_EVENT_DATA_SIZE;

      EventData.EventData.AbortedEventData.ConnectionID = Message->ConnectionID;

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

      DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_CRITICAL), ("Unable to server instance\n"));
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
               ProcessDeviceDisconnectionEvent(((PBAM_Device_Disconnected_Message_t *)Message)->RemoteDeviceAddress, ((PBAM_Device_Disconnected_Message_t *)Message)->DisconnectReason, ((PBAM_Device_Disconnected_Message_t *)Message)->ServerID, ((PBAM_Device_Disconnected_Message_t *)Message)->ConnectionID);

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
            DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("phone book set message Message: %u. Macro: %lu\n", Message->MessageHeader.MessageLength, PBAM_PHONE_BOOK_SET_MESSAGE_SIZE(((PBAM_Phone_Book_Set_Message_t *)Message)->CurrentPathSize)));

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
         case PBAM_MESSAGE_FUNCTION_CONNECTION_REQUEST:
            DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Connection Request\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PBAM_CONNECTION_REQUEST_MESSAGE_SIZE)
            {
               /* Call the handler function.                            */
               ProcessConnectionRequestEvent((PBAM_Connection_Request_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_CRITICAL), ("Invalid Message Length\n"));
            }
            break;
         case PBAM_MESSAGE_FUNCTION_CONNECTED:
            DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Connected\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PBAM_CONNECTED_MESSAGE_SIZE)
            {
               /* Call the handler function.                            */
               ProcessConnectedEvent((PBAM_Connected_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_CRITICAL), ("Invalid Message Length\n"));
            }
            break;
         case PBAM_MESSAGE_FUNCTION_PULL_PHONE_BOOK_EVENT:
            DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Pull Phone Book\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PBAM_PULL_PHONE_BOOK_EVENT_MESSAGE_SIZE(0)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PBAM_PULL_PHONE_BOOK_EVENT_MESSAGE_SIZE(((PBAM_Pull_Phone_Book_Message_t *)Message)->ObjectNameLength)))
            {
               /* Call the handler function.                            */
               ProcessPullPhoneBookEvent((PBAM_Pull_Phone_Book_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_CRITICAL), ("Invalid Message Length\n"));
            }
            break;
         case PBAM_MESSAGE_FUNCTION_PULL_PHONE_BOOK_SIZE_EVENT:
            DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Pull Phonebook Size\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PBAM_PULL_PHONE_BOOK_SIZE_EVENT_MESSAGE_SIZE(0)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PBAM_PULL_PHONE_BOOK_SIZE_EVENT_MESSAGE_SIZE(((PBAM_Pull_Phone_Book_Size_Message_t *)Message)->ObjectNameLength)))
            {
               /* Call the handler function.                            */
               ProcessPullPhoneBookSizeEvent((PBAM_Pull_Phone_Book_Size_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_CRITICAL), ("Invalid Message Length\n"));
            }
            break;
         case PBAM_MESSAGE_FUNCTION_SET_PHONE_BOOK_EVENT:
            DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Set Phonebook\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PBAM_SET_PHONE_BOOK_EVENT_MESSAGE_SIZE(0)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PBAM_SET_PHONE_BOOK_EVENT_MESSAGE_SIZE(((PBAM_Set_Phone_Book_Message_t *)Message)->ObjectNameLength)))
            {
               /* Call the handler function.                            */
               ProcessSetPhoneBookEvent((PBAM_Set_Phone_Book_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_CRITICAL), ("Invalid Message Length\n"));
            }
            break;
         case PBAM_MESSAGE_FUNCTION_PULL_VCARD_LISTING_EVENT:
            DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Pull vCard Listing\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PBAM_PULL_VCARD_LISTING_EVENT_MESSAGE_SIZE(0,0)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PBAM_PULL_VCARD_LISTING_EVENT_MESSAGE_SIZE(((PBAM_Pull_vCard_Listing_Message_t *)Message)->ObjectNameLength, ((PBAM_Pull_vCard_Listing_Message_t *)Message)->SearchValueLength)))
            {
               /* Call the handler function.                            */
               ProcessPullvCardListingEvent((PBAM_Pull_vCard_Listing_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_CRITICAL), ("Invalid Message Length\n"));
            }
            break;
         case PBAM_MESSAGE_FUNCTION_PULL_VCARD_LISTING_SIZE_EVENT:
            DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Pull vCard Listing Size\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PBAM_PULL_VCARD_LISTING_SIZE_EVENT_MESSAGE_SIZE(0)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PBAM_PULL_VCARD_LISTING_SIZE_EVENT_MESSAGE_SIZE(((PBAM_Pull_vCard_Listing_Size_Message_t *)Message)->ObjectNameLength)))
            {
               /* Call the handler function.                            */
               ProcessPullvCardListingSizeEvent((PBAM_Pull_vCard_Listing_Size_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_CRITICAL), ("Invalid Message Length\n"));
            }
            break;
         case PBAM_MESSAGE_FUNCTION_PULL_VCARD_EVENT:
            DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Pull vCard\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PBAM_PULL_VCARD_EVENT_MESSAGE_SIZE(0)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PBAM_PULL_VCARD_EVENT_MESSAGE_SIZE(((PBAM_Pull_vCard_Message_t *)Message)->ObjectNameLength)))
            {
               /* Call the handler function.                            */
               ProcessPullvCardEvent((PBAM_Pull_vCard_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_CRITICAL), ("Invalid Message Length\n"));
            }
            break;
         case PBAM_MESSAGE_FUNCTION_ABORTED:
            DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Aborted\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PBAM_ABORTED_MESSAGE_SIZE)
            {
               /* Call the handler function.                            */
               ProcessAbortedEvent((PBAM_Aborted_Message_t *)Message);

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

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process Phone Book Access Manager Asynchronous      */
   /* Events.                                                           */
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
            ProcessReceivedMessage((BTPM_Message_t *)CallbackParameter);

            /* Note we do not have to release the Mutex because         */
            /* ProcessReceivedMessage() is documented that it will be   */
            /* called with the Mutex being held and it will release the */
            /* Mutex when it is finished with it.                       */
         }
      }

      /* All finished with the Message, so go ahead and free it.        */
      MSG_FreeReceivedMessageGroupHandlerMessage((BTPM_Message_t *)CallbackParameter);
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process Server un-register asynchronous events.     */
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter)
{
   PBAM_Entry_Info_t *PBAMEntryInfo;

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
            /* Make sure we cancel any synchronous connections.         */
            PBAMEntryInfo = PBAMEntryInfoList;

            while(PBAMEntryInfo)
            {
               /* Check to see if there is a synchronous open operation.*/
               if((PBAMEntryInfo->Flags & PBAM_ENTRY_INFO_FLAGS_SYNCHRONOUS_CONNECT_REQUEST) && (PBAMEntryInfo->ConnectionEvent))
               {
                  PBAMEntryInfo->ConnectionStatus = PBAM_DEVICE_CONNECTION_STATUS_FAILURE_DEVICE_POWER_OFF;

                  BTPS_SetEvent(PBAMEntryInfo->ConnectionEvent);
               }

               PBAMEntryInfo = PBAMEntryInfo->NextPBAMEntryInfoPtr;
            }

            /* Release the Lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function represents the Message Group Handler       */
   /* function that is installed to process all Phone Book Access       */
   /* Manager Messages.                                                 */
static void BTPSAPI PhoneBookAccessGroupHandler(BTPM_Message_t *Message, void *CallbackParameter)
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
               /* Dispatch to the main handler that the server has      */
               /* un-registered.                                        */
               if(Message->MessageHeader.MessageFunction == BTPM_MESSAGE_FUNCTION_CLIENT_REGISTRATION)
               {
                  if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= BTPM_CLIENT_REGISTRATION_MESSAGE_SIZE) && (!(((BTPM_Client_Registration_Message_t *)Message)->Registered)))
                  {
                     if(!BTPM_QueueMailboxCallback(BTPMDispatchCallback_MSG, (void *)(((BTPM_Client_Registration_Message_t *)Message)->AddressID)))
                     {
                        DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue Phone Book Access Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));
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

         /* Attempt to register our Message Group Handler to process    */
         /* Phone Book Access Manager messages.                         */
         if(!MSG_RegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_PHONE_BOOK_ACCESS_MANAGER, PhoneBookAccessGroupHandler, NULL))
         {
            /* Initialize the actual Phone Book Access Manager          */
            /* implementation module (this is the module that is        */
            /* actually responsible for actually implementing the PBAM  */
            /* Manager functionality - this module is just the framework*/
            /* shell).                                                  */
            if(!(Result = _PBAM_Initialize()))
            {
               /* Note the current Power State.                         */
               CurrentPowerState = (Boolean_t)((DEVM_QueryDevicePowerState() > 0)?TRUE:FALSE);

               /* Go ahead and flag that this module is initialized.    */
               Initialized    = TRUE;
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
            FreePBAMServerList(&PBAMServerList);

            /* Flag that the resources are no longer allocated.         */
            CurrentPowerState       = FALSE;

            /* Flag that this module is no longer initialized.          */
            Initialized             = FALSE;

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

                  PBAMEntryInfo = PBAMEntryInfo->NextPBAMEntryInfoPtr;
               }

               FreePBAMServerList(&PBAMServerList);

               /* Note the new Power State.                             */
               CurrentPowerState   = FALSE;
               break;
            default:
               DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Event %d.\n", EventData->EventType));
               /* Do nothing.                                           */
               break;
         }

         /* Release the Mutex because we are finished with it.          */
         DEVM_ReleaseLock();
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* Phone Book Access Manager Connection Management Functions.        */

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
                     if((ret_val = _PBAM_Connect_Remote_Device(RemoteDeviceAddress, RemoteServerPort, ConnectionFlags)) != 0)
                     {
                        /* Error opening device, go ahead and delete the*/
                        /* entry that was added.                        */
                        if((PBAMEntryInfoPtr = DeletePBAMEntryInfoEntry(&PBAMEntryInfoList, &(PBAMEntryInfoPtr->BluetoothAddress))) != NULL)
                        {
                           if(PBAMEntryInfoPtr->ConnectionEvent)
                              BTPS_CloseEvent(PBAMEntryInfoPtr->ConnectionEvent);

                           FreePBAMEntryInfoEntryMemory(PBAMEntryInfoPtr);
                        }
                     }
                     else
                     {
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
                              if(((PBAMEntryInfoPtr = SearchPBAMEntryInfoEntry(&PBAMEntryInfoList, &RemoteDeviceAddress)) != NULL))
                              {
                                 /* Note the connection status.         */
                                 *ConnectionStatus = PBAMEntryInfoPtr->ConnectionStatus;

                                 PBAMEntryInfo.Flags &= ~((unsigned long)PBAM_ENTRY_INFO_FLAGS_SYNCHRONOUS_CONNECT_REQUEST);

                                 BTPS_CloseEvent(PBAMEntryInfoPtr->ConnectionEvent);

                                 if(PBAMEntryInfoPtr->ConnectionStatus)
                                 {
                                    /* Because the connection failed, we*/
                                    /* need to delete the entry.        */
                                    if((PBAMEntryInfoPtr = DeletePBAMEntryInfoEntry(&PBAMEntryInfoList, &RemoteDeviceAddress)) != NULL)
                                       FreePBAMEntryInfoEntryMemory(PBAMEntryInfoPtr);

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
   int                ret_val;
   PBAM_Entry_Info_t *PBAMEntryInfo;

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
            if((PBAMEntryInfo = SearchPBAMEntryInfoEntry(&PBAMEntryInfoList, &RemoteDeviceAddress)) != NULL)
            {
               /* Make sure that this port is not already in the process*/
               /* of closing.                                           */
               if(!(PBAMEntryInfo->Flags & PBAM_ENTRY_INFO_FLAGS_EVENT_CLOSING))
               {
                  DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Close Device\n"));

                  /* Flag that we are now in the closing state.         */
                  PBAMEntryInfo->Flags |= PBAM_ENTRY_INFO_FLAGS_EVENT_CLOSING;

                  /* Nothing to do here other than to call the actual   */
                  /* function to disconnect the remote device.          */
                  ret_val               = _PBAM_Disconnect_Device(PBAMEntryInfo->BluetoothAddress);
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
            /* Guard against disconnecting if a connection is in        */
            /* progress.                                                */
            if(((PBAMEntryInfo = SearchPBAMEntryInfoEntry(&PBAMEntryInfoList, &RemoteDeviceAddress)) != NULL))
            {
               /* Nothing to do here other than to call the actual      */
               /* function to send the abort.                           */
               ret_val = _PBAM_Abort_Request(PBAMEntryInfo->BluetoothAddress);
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
            /* Guard against disconnecting if a connection is in        */
            /* progress.                                                */
            if(((PBAMEntryInfo = SearchPBAMEntryInfoEntry(&PBAMEntryInfoList, &RemoteDeviceAddress)) != NULL))
            {
               /* Nothing to do here other than to call the actual      */
               /* function to send the abort.                           */
               ret_val = _PBAM_Pull_Phone_Book_Request(PBAMEntryInfo->BluetoothAddress, PhoneBookNamePath, FilterLow, FilterHigh, VCardFormat, MaxListCount, ListStartOffset);
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
            /* Guard against disconnecting if a connection is in        */
            /* progress.                                                */
            if(((PBAMEntryInfo = SearchPBAMEntryInfoEntry(&PBAMEntryInfoList, &RemoteDeviceAddress)) != NULL))
            {
               /* Nothing to do here other than to call the actual      */
               /* function to start the pull.                           */
               ret_val = _PBAM_Pull_Phone_Book_Size(PBAMEntryInfo->BluetoothAddress);
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
            /* Guard against disconnecting if a connection is in        */
            /* progress.                                                */
            if(((PBAMEntryInfo = SearchPBAMEntryInfoEntry(&PBAMEntryInfoList, &RemoteDeviceAddress)) != NULL))
            {
               /* Nothing to do here other than to call the actual      */
               /* function to send the request to set the phonebook.    */
               ret_val = _PBAM_Set_Phone_Book_Request(PBAMEntryInfo->BluetoothAddress, PathOption, FolderName);
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
            /* Guard against disconnecting if a connection is in        */
            /* progress.                                                */
            if(((PBAMEntryInfo = SearchPBAMEntryInfoEntry(&PBAMEntryInfoList, &RemoteDeviceAddress)) != NULL))
            {
               /* Nothing to do here other than to call the actual      */
               /* function to send the request to pull the vCard        */
               /* listing.                                              */
               ret_val = _PBAM_Pull_vCard_Listing(PBAMEntryInfo->BluetoothAddress, PhonebookPath, ListOrder, SearchAttribute, SearchValue, MaxListCount, ListStartOffset);
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
            /* Guard against disconnecting if a connection is in        */
            /* progress.                                                */
            if(((PBAMEntryInfo = SearchPBAMEntryInfoEntry(&PBAMEntryInfoList, &RemoteDeviceAddress)) != NULL))
            {
               /* Nothing to do here other than to call the actual      */
               /* function to send the request to pull the vCard.       */
               ret_val = _PBAM_Pull_vCard(PBAMEntryInfo->BluetoothAddress, VCardName, FilterLow, FilterHigh, VCardFormat);
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
            /* Guard against disconnecting if a connection is in        */
            /* progress.                                                */
            if(((PBAMEntryInfo = SearchPBAMEntryInfoEntry(&PBAMEntryInfoList, &RemoteDeviceAddress)) != NULL))
            {
               /* Nothing to do here other than to call the actual      */
               /* function to send the request to set the phone book.   */
               ret_val = _PBAM_Set_Phone_Book_Absolute(PBAMEntryInfo->BluetoothAddress, AbsolutePath);
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

   /* PSE Role API Functions.                                           */


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
   int                 ret_val;
   PBAM_Server_Entry_t PBAMEntryInfo;

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
            /* Simply submit the request to the server.                 */
            if((ret_val = _PBAM_Register_Server(ServerPort, SupportedRepositories, IncomingConnectionFlags, ServiceName)) > 0)
            {
               /* Add the new instance into the server list.            */
               BTPS_MemInitialize(&PBAMEntryInfo, 0, sizeof(PBAMEntryInfo));

               PBAMEntryInfo.ServerID          = (unsigned int)ret_val;
               PBAMEntryInfo.EventCallback     = EventCallback;
               PBAMEntryInfo.CallbackParameter = CallbackParameter;

               if(AddPBAMServerEntry(&PBAMServerList, &PBAMEntryInfo) == NULL)
               {
                  /* We failed to add the server to the list, so we need*/
                  /* to un-register it.                                 */
                  _PBAM_Un_Register_Server(PBAMEntryInfo.ServerID);

                  ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
               }
            }
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

   /* Unregisters a previously opened PBAP server port. The ServerID    */
   /* parameter is the ID of the server returned from a successful call */
   /* to PBAM_Register_Server(). This fuction returns zero if successful*/
   /* or a negative return error code if there was an error.            */
int BTPSAPI PBAM_Un_Register_Server(unsigned int ServerID)
{
   int                  ret_val;
   PBAM_Server_Entry_t *PBAMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Phone Book Access Manager has been  */
   /* initialized.                                                      */
   if(Initialized)
   {
      if(ServerID)
      {
         /* Check to see if the device is powered on.                   */
         if(CurrentPowerState)
         {
            /* Attempt to find/remove the server instance.              */
            if((PBAMEntryInfo = DeletePBAMServerEntry(&PBAMServerList, ServerID)) != NULL)
            {
               /* Simply submit the request to the server.              */
               ret_val = _PBAM_Un_Register_Server(ServerID);

               /* Free the entry's memory.                              */
               FreePBAMServerEntryMemory(PBAMEntryInfo);
            }
            else
               ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_INVALID_SERVER_ID;
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

   /* First, check to make sure the Phone Book Access Manager has been  */
   /* initialized.                                                      */
   if(Initialized)
   {
      if(ConnectionID)
      {
         /* Check to see if the device is powered on.                   */
         if(CurrentPowerState)
         {
            /* Simply submit the request to the server.                 */
            ret_val = _PBAM_Connection_Request_Response(ConnectionID, Accept);
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

   /* First, check to make sure the Phone Book Access Manager has been  */
   /* initialized.                                                      */
   if(Initialized)
   {
      if(ConnectionID)
      {
         /* Check to see if the device is powered on.                   */
         if(CurrentPowerState)
         {
            /* Simply submit the request to the server.                 */
            ret_val = _PBAM_Close_Server_Connection(ConnectionID);
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
   /* event indicated the 'mch' phonebook, the NewMissedCalls parameter */
   /* should be a pointer to the value of the number of missed calls    */
   /* since the last 'mch' pull request. If it is not an 'mch' request, */
   /* this parameter should be set to NULL. The BufferSize parameter    */
   /* indicates the amount of data in the buffer to be sent. The Buffer */
   /* parameter is a pointer to the phone book data to send. The Final  */
   /* parameter should be set to FALSE if there is more data to be sent */
   /* after this buffer or TRUE if there is no more data. This function */
   /* returns zero if successful and a negative return error code if    */
   /* there was an error.                                               */
int BTPSAPI PBAM_Send_Phone_Book(unsigned int ConnectionID, unsigned int ResponseStatusCode, Byte_t *NewMissedCalls, unsigned int BufferSize, Byte_t *Buffer, Boolean_t Final)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Phone Book Access Manager has been  */
   /* initialized.                                                      */
   if(Initialized)
   {
      if((ConnectionID) && ((!BufferSize) || (Buffer)))
      {
         /* Check to see if the device is powered on.                   */
         if(CurrentPowerState)
         {
            /* Simply submit the request to the server.                 */
            ret_val = _PBAM_Send_Phone_Book(ConnectionID, ResponseStatusCode, NewMissedCalls, BufferSize, Buffer, Final);
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

   /* First, check to make sure the Phone Book Access Manager has been  */
   /* initialized.                                                      */
   if(Initialized)
   {
      if(ConnectionID)
      {
         /* Check to see if the device is powered on.                   */
         if(CurrentPowerState)
         {
            /* Simply submit the request to the server.                 */
            ret_val = _PBAM_Send_Phone_Book_Size(ConnectionID, ResponseStatusCode, PhoneBookSize);
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

   /* First, check to make sure the Phone Book Access Manager has been  */
   /* initialized.                                                      */
   if(Initialized)
   {
      if(ConnectionID)
      {
         /* Check to see if the device is powered on.                   */
         if(CurrentPowerState)
         {
            /* Simply submit the request to the server.                 */
            ret_val = _PBAM_Set_Phone_Book_Response(ConnectionID, ResponseStatusCode);
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
   /* event indicated the 'mch' phonebook, the NewMissedCalls parameter */
   /* should be a pointer to the value of the number of missed calls    */
   /* since the last 'mch' pull request. If it is not an 'mch' request, */
   /* this parameter should be set to NULL. The BufferSize parameter    */
   /* indicates the amount of data in the buffer to be sent. The Buffer */
   /* parameter is a pointer to the vCardListing data to send. The Final*/
   /* parameter should be set to FALSE if there is more data to be sent */
   /* after this buffer or TRUE if there is no more data. This function */
   /* returns zero if successful and a negative return error code if    */
   /* there was an error.                                               */
int BTPSAPI PBAM_Send_vCard_Listing(unsigned int ConnectionID, unsigned int ResponseStatusCode, Byte_t *NewMissedCalls, unsigned int BufferSize, Byte_t *Buffer, Boolean_t Final)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Phone Book Access Manager has been  */
   /* initialized.                                                      */
   if(Initialized)
   {
      if((ConnectionID) && ((!BufferSize) || (Buffer)))
      {
         /* Check to see if the device is powered on.                   */
         if(CurrentPowerState)
         {
            /* Simply submit the request to the server.                 */
            ret_val = _PBAM_Send_vCard_Listing(ConnectionID, ResponseStatusCode, NewMissedCalls, BufferSize, Buffer, Final);
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

   /* First, check to make sure the Phone Book Access Manager has been  */
   /* initialized.                                                      */
   if(Initialized)
   {
      if(ConnectionID)
      {
         /* Check to see if the device is powered on.                   */
         if(CurrentPowerState)
         {
            /* Simply submit the request to the server.                 */
            ret_val = _PBAM_Send_vCard_Listing_Size(ConnectionID, ResponseStatusCode, vCardListingSize);
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

   /* First, check to make sure the Phone Book Access Manager has been  */
   /* initialized.                                                      */
   if(Initialized)
   {
      if((ConnectionID) && ((!BufferSize) || (Buffer)))
      {
         /* Check to see if the device is powered on.                   */
         if(CurrentPowerState)
         {
            /* Simply submit the request to the server.                 */
            ret_val = _PBAM_Send_vCard(ConnectionID, ResponseStatusCode, BufferSize, Buffer, Final);
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

