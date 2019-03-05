/*****< btpmhdpm.c >***********************************************************/
/*      Copyright 2010 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMHDPM - Health Device Profile Manager for Stonestreet One Bluetooth    */
/*             Protocol Stack Platform Manager.                               */
/*                                                                            */
/*  Author:  Greg Hensley                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   03/11/13  G. Hensley     Initial creation.                               */
/******************************************************************************/
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMHDPM.h"            /* BTPM HDP Manager Prototypes/Constants.    */
#include "HDPMAPI.h"             /* HDP Manager Prototypes/Constants.         */
#include "HDPMMSG.h"             /* BTPM HDP Manager Message Formats.         */
#include "HDPMGR.h"              /* HDP Manager Impl. Prototypes/Constants.   */

#include "SS1BTPM.h"             /* BTPM Main Prototypes and Constants.       */
#include "BTPMERR.h"             /* BTPM Error Prototypes/Constants.          */
#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

   /* Container structure used when dispatching registered callbacks.   */
typedef struct _tagCallbackInfo_t
{
   HDPM_Event_Callback_t  EventCallback;
   void                  *CallbackParameter;
} CallbackInfo_t;

typedef enum _tagEntry_Type_t
{
   etLocallyRegisteredEndpoint,
   etRemotelyInitiatedEndpointConnection,
   etRemoteInstanceConnection,
   etLocallyInitiatedEndpointConnection
} Entry_Type_t;

   /* Structure which is used to track Callback Information related to  */
   /* this module.                                                      */
typedef struct _tagHDP_Entry_Info_t
{
   Entry_Type_t                 EntryType;
   unsigned int                 Handle;
   unsigned int                 ConnectionStatus;
   Event_t                      ConnectionEvent;
   DWord_t                      Flags;
   BD_ADDR_t                    BD_ADDR;
   DWord_t                      Instance;
   CallbackInfo_t               CallbackInfo;
   struct _tagHDP_Entry_Info_t *NextEntryInfoPtr;
} HDP_Entry_Info_t;

   /* The following functions are used with the Flags member of the     */
   /* HDP_Entry_Info_t structure to denote various state information.   */
#define HDP_ENTRY_INFO_FLAGS_WAITING_ON_CONNECT             0x10000000

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the compiler*/
   /* as part of standard C/C++).                                       */

   /* Variable which serves as a global flag about whether this module  */
   /* is initialized or not.                                            */
static Boolean_t Initialized;

   /* Variable which is used to protect access to the state information */
   /* in this module.                                                   */
static Mutex_t HDPManagerMutex;

   /* Variable which is used to hold the next (unique) Entry Handle.    */
static unsigned int NextTemporaryHandle;

   /* Variable which holds the current Power State of the Device.       */
static Boolean_t CurrentPowerState;

   /* Variable which holds a pointer to the first element in the Health */
   /* Device entry information list (which holds all callbacks tracked  */
   /* by this module).                                                  */
static HDP_Entry_Info_t *HDPEntryInfoList;

   /* Internal Function Prototypes.                                     */
static unsigned int GetNextTemporaryHandle(void);

static HDP_Entry_Info_t *AddHDPEntryInfoEntry(HDP_Entry_Info_t **ListHead, HDP_Entry_Info_t *EntryToAdd);
static HDP_Entry_Info_t *SearchHDPEntryInfoEntryHandle(HDP_Entry_Info_t **ListHead, Entry_Type_t Type, unsigned int Handle);
static HDP_Entry_Info_t *SearchHDPEntryInfoEntryInstance(HDP_Entry_Info_t **ListHead, Entry_Type_t Type, BD_ADDR_t BD_ADDR, DWord_t Instance);
static HDP_Entry_Info_t *DeleteHDPEntryInfoEntryHandle(HDP_Entry_Info_t **ListHead, Entry_Type_t Type, unsigned int Handle);
static HDP_Entry_Info_t *DeleteHDPEntryInfoEntryInstance(HDP_Entry_Info_t **ListHead, Entry_Type_t Type, BD_ADDR_t BD_ADDR, DWord_t Instance);
static void FreeHDPEntryInfoEntryMemory(HDP_Entry_Info_t *EntryToFree);
static void FreeHDPEntryInfoList(HDP_Entry_Info_t **ListHead);

static void ProcessConnectionStatus(HDPM_Connection_Status_Message_t *Message);
static void ProcessDisconnected(HDPM_Disconnected_Message_t *Message);
static void ProcessIncomingDataConnectionRequest(HDPM_Incoming_Data_Connection_Message_t *Message);
static void ProcessDataConnected(HDPM_Data_Connected_Message_t *Message);
static void ProcessDataDisconnected(HDPM_Data_Disconnected_Message_t *Message);
static void ProcessDataConnectionStatus(HDPM_Data_Connection_Status_Message_t *Message);
static void ProcessDataReceived(HDPM_Data_Received_Message_t *Message);

static void ProcessReceivedMessage(BTPM_Message_t *Message);

static void BTPSAPI BTPMDispatchCallback_HDPM(void *CallbackParameter);
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter);

static void BTPSAPI HealthDeviceManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter);

   /* The following function is a utility function that exists to       */
   /* retrieve a unique handle that can be used to add an entry into the*/
   /* Health Device entry information list.                             */
static unsigned int GetNextTemporaryHandle(void)
{
   unsigned int ret_val;

   ret_val = NextTemporaryHandle++;

   if(!NextTemporaryHandle)
      NextTemporaryHandle = 0x80000000;

   return(ret_val);
}

   /* The following function adds the specified entry to the specified  */
   /* list. This function allocates and adds an entry to the list that  */
   /* has the same attributes as the entry passed into this function.   */
   /* This function will return NULL if NO entry was added. This can    */
   /* occur if the element passed in was deemed invalid or the actual   */
   /* list head was invalid.                                            */
   /* ** NOTE ** This function does not insert duplicate entries into   */
   /*            the list. An element is considered a duplicate if the  */
   /*            handle field is the same as an entry already in the    */
   /*            list. When this occurs, this function returns NULL.    */
static HDP_Entry_Info_t *AddHDPEntryInfoEntry(HDP_Entry_Info_t **ListHead, HDP_Entry_Info_t *EntryToAdd)
{
   HDP_Entry_Info_t *AddedEntry = NULL;
   HDP_Entry_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First let's verify that values passed in are semi-valid.          */
   if((ListHead) && (EntryToAdd))
   {
      /* Make sure that the element that we are adding seems semi-valid.*/
      if(EntryToAdd->Handle)
      {
         /* OK, data seems semi-valid, let's allocate a new data        */
         /* structure to add to the list.                               */
         AddedEntry = (HDP_Entry_Info_t *)BTPS_AllocateMemory(sizeof(HDP_Entry_Info_t));

         if(AddedEntry)
         {
            /* Copy All Data over.                                      */
            *AddedEntry                  = *EntryToAdd;

            /* Now Add it to the end of the list.                       */
            AddedEntry->NextEntryInfoPtr = NULL;

            /* First, let's check to see if there are any elements      */
            /* already present in the List that was passed in.          */
            if((tmpEntry = *ListHead) != NULL)
            {
               /* Head Pointer was not NULL, so we will traverse the    */
               /* list until we reach the last element.                 */
               while(tmpEntry)
               {
                  if((tmpEntry->Handle == AddedEntry->Handle) && (tmpEntry->EntryType == AddedEntry->EntryType))
                  {
                     /* Entry was already added, so free the memory and */
                     /* flag an error to the caller.                    */
                     BTPS_FreeMemory(AddedEntry);
                     AddedEntry = NULL;

                     /* Abort the Search.                               */
                     tmpEntry   = NULL;
                  }
                  else
                  {
                     /* OK, we need to see if we are at the last element*/
                     /* of the List. If we are, we simply break out of  */
                     /* the list traversal because we know there are NO */
                     /* duplicates AND we are at the end of the list.   */
                     if(tmpEntry->NextEntryInfoPtr)
                        tmpEntry = tmpEntry->NextEntryInfoPtr;
                     else
                        break;
                  }
               }

               if(AddedEntry)
               {
                  /* Last element found, simply Add the entry.          */
                  tmpEntry->NextEntryInfoPtr = AddedEntry;
               }
            }
            else
               *ListHead = AddedEntry;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", AddedEntry));

   return(AddedEntry);
}

   /* The following function searches the specified List for the        */
   /* specified callback ID.  This function returns NULL if either the  */
   /* list head is invalid, the callback ID is invalid, or the specified*/
   /* callback ID was NOT found.                                        */
static HDP_Entry_Info_t *SearchHDPEntryInfoEntryHandle(HDP_Entry_Info_t **ListHead, Entry_Type_t Type, unsigned int Handle)
{
   HDP_Entry_Info_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %d, 0x%08X\n", Type, Handle));

   /* Let's make sure the list and Callback ID to search for appear to  */
   /* be valid.                                                         */
   if((ListHead) && (Handle))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && ((FoundEntry->EntryType != Type) || (FoundEntry->Handle != Handle)))
         FoundEntry = FoundEntry->NextEntryInfoPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

static HDP_Entry_Info_t *SearchHDPEntryInfoEntryInstance(HDP_Entry_Info_t **ListHead, Entry_Type_t Type, BD_ADDR_t BD_ADDR, DWord_t Instance)
{
   HDP_Entry_Info_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %02X:%02X:%02X:%02X:%02X:%02X, 0x%08X\n", BD_ADDR.BD_ADDR5, BD_ADDR.BD_ADDR4, BD_ADDR.BD_ADDR3, BD_ADDR.BD_ADDR2, BD_ADDR.BD_ADDR1, BD_ADDR.BD_ADDR0, Instance));

   /* Let's make sure the list and Callback ID to search for appear to  */
   /* be valid.                                                         */
   if((ListHead) && ((!COMPARE_NULL_BD_ADDR(BD_ADDR)) && (Instance)))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && ((!COMPARE_BD_ADDR(FoundEntry->BD_ADDR, BD_ADDR)) || (FoundEntry->Instance != Instance)))
         FoundEntry = FoundEntry->NextEntryInfoPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* The following function searches the specified Health Device entry */
   /* information list for the specified callback ID and removes it     */
   /* from the List. This function returns NULL if either the Health    */
   /* Device entry information list head is invalid, the callback ID is */
   /* invalid, or the specified callback ID was NOT present in the list.*/
   /* The entry returned will have the next entry field set to NULL, and*/
   /* the caller is responsible for deleting the memory associated with */
   /* this entry by calling FreeHDPEntryInfoEntryMemory().              */
static HDP_Entry_Info_t *DeleteHDPEntryInfoEntryHandle(HDP_Entry_Info_t **ListHead, Entry_Type_t Type, unsigned int Handle)
{
   HDP_Entry_Info_t *FoundEntry = NULL;
   HDP_Entry_Info_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %d, 0x%08X\n", Type, Handle));

   /* Let's make sure the List and Callback ID to search for appear to  */
   /* be semi-valid.                                                    */
   if((ListHead) && (Handle))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && ((FoundEntry->EntryType != Type) || (FoundEntry->Handle != Handle)))
      {
         LastEntry  = FoundEntry;
         FoundEntry = FoundEntry->NextEntryInfoPtr;
      }

      /* Check to see if we found the specified entry.                  */
      if(FoundEntry)
      {
         /* OK, now let's remove the entry from the list.  We have to   */
         /* check to see if the entry was the first entry in the list.  */
         if(LastEntry)
         {
            /* Entry was NOT the first entry in the list.               */
            LastEntry->NextEntryInfoPtr = FoundEntry->NextEntryInfoPtr;
         }
         else
            *ListHead = FoundEntry->NextEntryInfoPtr;

         FoundEntry->NextEntryInfoPtr = NULL;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

static HDP_Entry_Info_t *DeleteHDPEntryInfoEntryInstance(HDP_Entry_Info_t **ListHead, Entry_Type_t Type, BD_ADDR_t BD_ADDR, DWord_t Instance)
{
   HDP_Entry_Info_t *FoundEntry = NULL;
   HDP_Entry_Info_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %02X:%02X:%02X:%02X:%02X:%02X, 0x%08X\n", BD_ADDR.BD_ADDR5, BD_ADDR.BD_ADDR4, BD_ADDR.BD_ADDR3, BD_ADDR.BD_ADDR2, BD_ADDR.BD_ADDR1, BD_ADDR.BD_ADDR0, Instance));

   /* Let's make sure the List and Callback ID to search for appear to  */
   /* be semi-valid.                                                    */
   if((ListHead) && (!COMPARE_NULL_BD_ADDR(BD_ADDR)) && (Instance))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && ((!COMPARE_BD_ADDR(FoundEntry->BD_ADDR, BD_ADDR)) || (FoundEntry->Instance != Instance)))
      {
         LastEntry  = FoundEntry;
         FoundEntry = FoundEntry->NextEntryInfoPtr;
      }

      /* Check to see if we found the specified entry.                  */
      if(FoundEntry)
      {
         /* OK, now let's remove the entry from the list.  We have to   */
         /* check to see if the entry was the first entry in the list.  */
         if(LastEntry)
         {
            /* Entry was NOT the first entry in the list.               */
            LastEntry->NextEntryInfoPtr = FoundEntry->NextEntryInfoPtr;
         }
         else
            *ListHead = FoundEntry->NextEntryInfoPtr;

         FoundEntry->NextEntryInfoPtr = NULL;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* This function frees the specified Health Device entry information */
   /* member. No check is done on this entry other than making sure it  */
   /* NOT NULL.                                                         */
static void FreeHDPEntryInfoEntryMemory(HDP_Entry_Info_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToFree)
   {
      if(EntryToFree->ConnectionEvent)
         BTPS_CloseEvent(EntryToFree->ConnectionEvent);

      BTPS_FreeMemory(EntryToFree);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified Hands Free entry information list.  Upon */
   /* return of this function, the head pointer is set to NULL.         */
static void FreeHDPEntryInfoList(HDP_Entry_Info_t **ListHead)
{
   HDP_Entry_Info_t *EntryToFree;
   HDP_Entry_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ListHead)
   {
      /* Simply traverse the list and free every element present.       */
      EntryToFree = *ListHead;

      while(EntryToFree)
      {
         tmpEntry    = EntryToFree;
         EntryToFree = EntryToFree->NextEntryInfoPtr;

         FreeHDPEntryInfoEntryMemory(tmpEntry);
      }

      /* Make sure the List appears to be empty.                        */
      *ListHead = NULL;
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the remote   */
   /* instance connection status asynchronous message.                  */
static void ProcessConnectionStatus(HDPM_Connection_Status_Message_t *Message)
{
   void                  *CallbackParameter;
   HDP_Entry_Info_t      *HDPEntryInfoPtr;
   HDPM_Event_Data_t      EventData;
   HDPM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(Message)
   {
      /* First, attempt to find the specified HDP Entry.                */
      if((HDPEntryInfoPtr = SearchHDPEntryInfoEntryInstance(&HDPEntryInfoList, etRemoteInstanceConnection, Message->RemoteDeviceAddress, Message->Instance)) != NULL)
      {
         DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Instance Connection Status: %02X:%02X:%02X:%02X:%02X:%02X, 0x%08X, %d\n", Message->RemoteDeviceAddress.BD_ADDR5, Message->RemoteDeviceAddress.BD_ADDR4, Message->RemoteDeviceAddress.BD_ADDR3, Message->RemoteDeviceAddress.BD_ADDR2, Message->RemoteDeviceAddress.BD_ADDR1, Message->RemoteDeviceAddress.BD_ADDR0, Message->Instance, Message->Status));

         /* Check to see if we need to dispatch the event or set an     */
         /* internal event.                                             */
         if(HDPEntryInfoPtr->Flags & HDP_ENTRY_INFO_FLAGS_WAITING_ON_CONNECT)
         {
            /* Note the Status.                                         */
            HDPEntryInfoPtr->ConnectionStatus = Message->Status;

            BTPS_SetEvent(HDPEntryInfoPtr->ConnectionEvent);

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HDPManagerMutex);
         }
         else
         {
            /* Event needs to be dispatched. Go ahead and format the    */
            /* event.                                                   */
            BTPS_MemInitialize(&EventData, 0, sizeof(HDPM_Event_Data_t));

            EventData.EventType                                               = hetHDPConnectionStatus;
            EventData.EventLength                                             = HDPM_CONNECTION_STATUS_EVENT_DATA_SIZE;

            EventData.EventData.ConnectionStatusEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;
            EventData.EventData.ConnectionStatusEventData.Instance            = Message->Instance;
            EventData.EventData.ConnectionStatusEventData.Status              = Message->Status;

            /* Note the Callback information.                           */
            EventCallback                                                     = HDPEntryInfoPtr->CallbackInfo.EventCallback;
            CallbackParameter                                                 = HDPEntryInfoPtr->CallbackInfo.CallbackParameter;

            /* Update any information in the HDP Entry structure.       */
            if(Message->Status)
            {
               /* Error, go ahead and delete the HDP Entry from the     */
               /* list.                                                 */
               if((HDPEntryInfoPtr = DeleteHDPEntryInfoEntryInstance(&HDPEntryInfoList, etRemoteInstanceConnection, Message->RemoteDeviceAddress, Message->Instance)) != NULL)
                  FreeHDPEntryInfoEntryMemory(HDPEntryInfoPtr);
            }

            /* Release the Mutex so we can dispatch the event.          */
            BTPS_ReleaseMutex(HDPManagerMutex);

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
         BTPS_ReleaseMutex(HDPManagerMutex);

         DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to locate Instance connection: %02X:%02X:%02X:%02X:%02X:%02X, 0x%08X\n", Message->RemoteDeviceAddress.BD_ADDR5, Message->RemoteDeviceAddress.BD_ADDR4, Message->RemoteDeviceAddress.BD_ADDR3, Message->RemoteDeviceAddress.BD_ADDR2, Message->RemoteDeviceAddress.BD_ADDR1, Message->RemoteDeviceAddress.BD_ADDR0, Message->Instance));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the remote   */
   /* instance disconnection asynchronous message.                      */
static void ProcessDisconnected(HDPM_Disconnected_Message_t *Message)
{
   void                  *CallbackParameter;
   HDP_Entry_Info_t      *HDPEntryInfoPtr;
   HDPM_Event_Data_t      EventData;
   HDPM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(Message)
   {
      /* First, attempt to find the specified HDP Entry.                */
      if((HDPEntryInfoPtr = DeleteHDPEntryInfoEntryInstance(&HDPEntryInfoList, etRemoteInstanceConnection, Message->RemoteDeviceAddress, Message->Instance)) != NULL)
      {
         DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Instance Disconnected: %02X:%02X:%02X:%02X:%02X:%02X, 0x%08X\n", Message->RemoteDeviceAddress.BD_ADDR5, Message->RemoteDeviceAddress.BD_ADDR4, Message->RemoteDeviceAddress.BD_ADDR3, Message->RemoteDeviceAddress.BD_ADDR2, Message->RemoteDeviceAddress.BD_ADDR1, Message->RemoteDeviceAddress.BD_ADDR0, Message->Instance));

         /* Event needs to be dispatched. Go ahead and format the event.*/
         BTPS_MemInitialize(&EventData, 0, sizeof(HDPM_Event_Data_t));

         EventData.EventType                                           = hetHDPDisconnected;
         EventData.EventLength                                         = HDPM_DISCONNECTED_EVENT_DATA_SIZE;

         EventData.EventData.DisconnectedEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;
         EventData.EventData.DisconnectedEventData.Instance            = Message->Instance;

         /* Note the Callback information.                              */
         EventCallback                                                 = HDPEntryInfoPtr->CallbackInfo.EventCallback;
         CallbackParameter                                             = HDPEntryInfoPtr->CallbackInfo.CallbackParameter;

         /* The HDP Entry is no longer needed.                          */
         FreeHDPEntryInfoEntryMemory(HDPEntryInfoPtr);

         /* Release the Mutex so we can dispatch the event.             */
         BTPS_ReleaseMutex(HDPManagerMutex);

         __BTPSTRY
         {
            if(EventCallback)
               (*EventCallback)(&EventData, CallbackParameter);
         }
         __BTPSEXCEPT(1)
         {
            /* Do Nothing.                                              */
         }
      }
      else
      {
         /* Error - Release the Mutex.                                  */
         BTPS_ReleaseMutex(HDPManagerMutex);

         DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to locate Instance connection: %02X:%02X:%02X:%02X:%02X:%02X, 0x%08X\n", Message->RemoteDeviceAddress.BD_ADDR5, Message->RemoteDeviceAddress.BD_ADDR4, Message->RemoteDeviceAddress.BD_ADDR3, Message->RemoteDeviceAddress.BD_ADDR2, Message->RemoteDeviceAddress.BD_ADDR1, Message->RemoteDeviceAddress.BD_ADDR0, Message->Instance));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the incoming */
   /* data connection request asynchronous message.                     */
static void ProcessIncomingDataConnectionRequest(HDPM_Incoming_Data_Connection_Message_t *Message)
{
   void                  *CallbackParameter;
   HDP_Entry_Info_t       HDPEntryInfo;
   HDP_Entry_Info_t      *HDPEntryInfoPtr;
   HDPM_Event_Data_t      EventData;
   HDPM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(Message)
   {
      DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Data Connection Requested: %d, %d\n", Message->EndpointID, Message->DataLinkID));

      /* First, attempt to find the specified HDP Entry for the locally */
      /* registered endpoint being requested.                           */
      if((HDPEntryInfoPtr = SearchHDPEntryInfoEntryHandle(&HDPEntryInfoList, etLocallyRegisteredEndpoint, Message->EndpointID)) != NULL)
      {
         /* Build an HDP Entry to represent this connection.            */
         BTPS_MemInitialize(&HDPEntryInfo, 0, sizeof(HDPEntryInfo));

         HDPEntryInfo.EntryType    = etRemotelyInitiatedEndpointConnection;
         HDPEntryInfo.Handle       = Message->DataLinkID;
         HDPEntryInfo.BD_ADDR      = Message->RemoteDeviceAddress;
         HDPEntryInfo.CallbackInfo = HDPEntryInfoPtr->CallbackInfo;

         /* Attempt to add the new HDP Entry.                           */
         if((HDPEntryInfoPtr = AddHDPEntryInfoEntry(&HDPEntryInfoList, &HDPEntryInfo)) != NULL)
         {
            /* Event needs to be dispatched. Go ahead and format the    */
            /* event.                                                   */
            BTPS_MemInitialize(&EventData, 0, sizeof(HDPM_Event_Data_t));

            EventData.EventType                                                            = hetHDPIncomingDataConnectionRequest;
            EventData.EventLength                                                          = HDPM_INCOMING_DATA_CONNECTION_REQUEST_EVENT_DATA_SIZE;

            EventData.EventData.IncomingDataConnectionRequestEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;
            EventData.EventData.IncomingDataConnectionRequestEventData.EndpointID          = Message->EndpointID;
            EventData.EventData.IncomingDataConnectionRequestEventData.ChannelMode         = Message->ChannelMode;
            EventData.EventData.IncomingDataConnectionRequestEventData.DataLinkID          = Message->DataLinkID;

            /* Note the Callback information.                           */
            EventCallback                                                                  = HDPEntryInfoPtr->CallbackInfo.EventCallback;
            CallbackParameter                                                              = HDPEntryInfoPtr->CallbackInfo.CallbackParameter;

            /* Release the Mutex so we can dispatch the event.          */
            BTPS_ReleaseMutex(HDPManagerMutex);

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
         else
         {
            /* Error - Release the Mutex.                               */
            BTPS_ReleaseMutex(HDPManagerMutex);

            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to add entry\n"));
         }
      }
      else
      {
         /* Error - Release the Mutex.                                  */
         BTPS_ReleaseMutex(HDPManagerMutex);

         DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to locate local Endpoint: %d\n", Message->DataLinkID));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the data     */
   /* connected asynchronous message.                                   */
static void ProcessDataConnected(HDPM_Data_Connected_Message_t *Message)
{
   void                  *CallbackParameter;
   HDP_Entry_Info_t      *HDPEntryInfoPtr;
   HDPM_Event_Data_t      EventData;
   HDPM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(Message)
   {
      DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Data Channel Connected: %d, %d\n", Message->EndpointID, Message->DataLinkID));

      /* First, attempt to find the specified HDP Entry for the locally */
      /* registered endpoint being requested.                           */
      if((HDPEntryInfoPtr = SearchHDPEntryInfoEntryHandle(&HDPEntryInfoList, etRemotelyInitiatedEndpointConnection, Message->DataLinkID)) != NULL)
      {
         /* Event needs to be dispatched.  Go ahead and format the      */
         /* event.                                                      */
         BTPS_MemInitialize(&EventData, 0, sizeof(HDPM_Event_Data_t));

         EventData.EventType                                            = hetHDPDataConnected;
         EventData.EventLength                                          = HDPM_DATA_CONNECTED_EVENT_DATA_SIZE;

         EventData.EventData.DataConnectedEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;
         EventData.EventData.DataConnectedEventData.EndpointID          = Message->EndpointID;
         EventData.EventData.DataConnectedEventData.DataLinkID          = Message->DataLinkID;

         /* Note the Callback information.                              */
         EventCallback                                                  = HDPEntryInfoPtr->CallbackInfo.EventCallback;
         CallbackParameter                                              = HDPEntryInfoPtr->CallbackInfo.CallbackParameter;

         /* Release the Mutex so we can dispatch the event.             */
         BTPS_ReleaseMutex(HDPManagerMutex);

         __BTPSTRY
         {
            if(EventCallback)
               (*EventCallback)(&EventData, CallbackParameter);
         }
         __BTPSEXCEPT(1)
         {
            /* Do Nothing.                                              */
         }
      }
      else
      {
         /* Error - Release the Mutex.                                  */
         BTPS_ReleaseMutex(HDPManagerMutex);

         DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to locate Endpoint connection: %d\n", Message->DataLinkID));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the data     */
   /* disconnected asynchronous message.                                */
static void ProcessDataDisconnected(HDPM_Data_Disconnected_Message_t *Message)
{
   void                  *CallbackParameter;
   HDP_Entry_Info_t      *HDPEntryInfoPtr;
   HDPM_Event_Data_t      EventData;
   HDPM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(Message)
   {
      DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Data Channel Disconnected: %d, %d\n", Message->DataLinkID, Message->Reason));

      /* First, attempt to find the specified HDP Entry for the         */
      /* associated endpoint connection.                                */
      if((HDPEntryInfoPtr = DeleteHDPEntryInfoEntryHandle(&HDPEntryInfoList, etRemotelyInitiatedEndpointConnection, Message->DataLinkID)) == NULL)
         HDPEntryInfoPtr = DeleteHDPEntryInfoEntryHandle(&HDPEntryInfoList, etLocallyInitiatedEndpointConnection, Message->DataLinkID);

      if(HDPEntryInfoPtr)
      {
         /* Event needs to be dispatched. Go ahead and format the event.*/
         BTPS_MemInitialize(&EventData, 0, sizeof(HDPM_Event_Data_t));

         EventData.EventType                                               = hetHDPDataDisconnected;
         EventData.EventLength                                             = HDPM_DATA_DISCONNECTED_EVENT_DATA_SIZE;

         EventData.EventData.DataDisconnectedEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;
         EventData.EventData.DataDisconnectedEventData.DataLinkID          = Message->DataLinkID;
         EventData.EventData.DataDisconnectedEventData.Reason              = Message->Reason;

         /* Note the Callback information.                              */
         EventCallback                                                     = HDPEntryInfoPtr->CallbackInfo.EventCallback;
         CallbackParameter                                                 = HDPEntryInfoPtr->CallbackInfo.CallbackParameter;

         /* The HDP Entry is no longer needed.                          */
         FreeHDPEntryInfoEntryMemory(HDPEntryInfoPtr);

         /* Release the Mutex so we can dispatch the event.             */
         BTPS_ReleaseMutex(HDPManagerMutex);

         __BTPSTRY
         {
            if(EventCallback)
               (*EventCallback)(&EventData, CallbackParameter);
         }
         __BTPSEXCEPT(1)
         {
            /* Do Nothing.                                              */
         }
      }
      else
      {
         /* Error - Release the Mutex.                                  */
         BTPS_ReleaseMutex(HDPManagerMutex);

         DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to locate Endpoint connection: %d\n", Message->DataLinkID));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the data     */
   /* connection status asynchronous message.                           */
static void ProcessDataConnectionStatus(HDPM_Data_Connection_Status_Message_t *Message)
{
   void                  *CallbackParameter;
   HDP_Entry_Info_t      *HDPEntryInfoPtr;
   HDPM_Event_Data_t      EventData;
   HDPM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(Message)
   {
      DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Data Channel Connection Status: %02X:%02X:%02X:%02X:%02X:%02X, 0x%08X, %d, %d, %d\n", Message->RemoteDeviceAddress.BD_ADDR5, Message->RemoteDeviceAddress.BD_ADDR4, Message->RemoteDeviceAddress.BD_ADDR3, Message->RemoteDeviceAddress.BD_ADDR2, Message->RemoteDeviceAddress.BD_ADDR1, Message->RemoteDeviceAddress.BD_ADDR0, Message->Instance, Message->EndpointID, Message->DataLinkID, Message->Status));

      /* First, attempt to find the specified HDP Entry for the         */
      /* associated endpoint connection.                                */
      if((HDPEntryInfoPtr = SearchHDPEntryInfoEntryHandle(&HDPEntryInfoList, etLocallyInitiatedEndpointConnection, Message->DataLinkID)) != NULL)
      {
         /* Check to see if we need to dispatch the event or set an     */
         /* internal event.                                             */
         if(HDPEntryInfoPtr->Flags & HDP_ENTRY_INFO_FLAGS_WAITING_ON_CONNECT)
         {
            /* Note the Status.                                         */
            HDPEntryInfoPtr->ConnectionStatus = Message->Status;

            BTPS_SetEvent(HDPEntryInfoPtr->ConnectionEvent);

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HDPManagerMutex);
         }
         else
         {
            /* Event needs to be dispatched. Go ahead and format the    */
            /* event.                                                   */
            BTPS_MemInitialize(&EventData, 0, sizeof(HDPM_Event_Data_t));

            EventData.EventType                                                   = hetHDPDataConnectionStatus;
            EventData.EventLength                                                 = HDPM_DATA_CONNECTION_STATUS_EVENT_DATA_SIZE;

            EventData.EventData.DataConnectionStatusEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;
            EventData.EventData.DataConnectionStatusEventData.Instance            = Message->Instance;
            EventData.EventData.DataConnectionStatusEventData.EndpointID          = Message->EndpointID;
            EventData.EventData.DataConnectionStatusEventData.DataLinkID          = Message->DataLinkID;
            EventData.EventData.DataConnectionStatusEventData.Status              = Message->Status;

            /* Note the Callback information.                           */
            EventCallback                                                         = HDPEntryInfoPtr->CallbackInfo.EventCallback;
            CallbackParameter                                                     = HDPEntryInfoPtr->CallbackInfo.CallbackParameter;

            /* Update any information in the HDP Entry structure.       */
            if(Message->Status)
            {
               /* Error, go ahead and delete the HDP Entry from the     */
               /* list.                                                 */
               if((HDPEntryInfoPtr = DeleteHDPEntryInfoEntryHandle(&HDPEntryInfoList, etLocallyInitiatedEndpointConnection, Message->DataLinkID)) != NULL)
                  FreeHDPEntryInfoEntryMemory(HDPEntryInfoPtr);
            }

            /* Release the Mutex so we can dispatch the event.          */
            BTPS_ReleaseMutex(HDPManagerMutex);

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
         BTPS_ReleaseMutex(HDPManagerMutex);

         DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to locate Endpoint connection: %d\n", Message->DataLinkID));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the data     */
   /* received asynchronous message.                                    */
static void ProcessDataReceived(HDPM_Data_Received_Message_t *Message)
{
   void                  *CallbackParameter;
   HDP_Entry_Info_t      *HDPEntryInfoPtr;
   HDPM_Event_Data_t      EventData;
   HDPM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(Message)
   {
      DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Data Received: %d, %d\n", Message->DataLinkID, Message->DataLength));

      /* First, attempt to find the specified HDP Entry for the         */
      /* associated endpoint connection.                                */
      if((HDPEntryInfoPtr = SearchHDPEntryInfoEntryHandle(&HDPEntryInfoList, etRemotelyInitiatedEndpointConnection, Message->DataLinkID)) == NULL)
         HDPEntryInfoPtr = SearchHDPEntryInfoEntryHandle(&HDPEntryInfoList, etLocallyInitiatedEndpointConnection, Message->DataLinkID);

      if(HDPEntryInfoPtr)
      {
         /* Event needs to be dispatched.  Go ahead and format the      */
         /* event.                                                      */
         BTPS_MemInitialize(&EventData, 0, sizeof(HDPM_Event_Data_t));

         EventData.EventType                                           = hetHDPDataReceived;
         EventData.EventLength                                         = HDPM_DATA_RECEIVED_EVENT_DATA_SIZE;

         EventData.EventData.DataReceivedEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;
         EventData.EventData.DataReceivedEventData.DataLinkID          = Message->DataLinkID;
         EventData.EventData.DataReceivedEventData.DataLength          = Message->DataLength;
         EventData.EventData.DataReceivedEventData.Data                = Message->Data;

         /* Note the Callback information.                              */
         EventCallback                                                 = HDPEntryInfoPtr->CallbackInfo.EventCallback;
         CallbackParameter                                             = HDPEntryInfoPtr->CallbackInfo.CallbackParameter;

         /* Release the Mutex so we can dispatch the event.             */
         BTPS_ReleaseMutex(HDPManagerMutex);

         __BTPSTRY
         {
            if(EventCallback)
               (*EventCallback)(&EventData, CallbackParameter);
         }
         __BTPSEXCEPT(1)
         {
            /* Do Nothing.                                              */
         }
      }
      else
      {
         /* Error - Release the Mutex.                                  */
         BTPS_ReleaseMutex(HDPManagerMutex);

         DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to locate Endpoint connection: %d\n", Message->DataLinkID));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is called from the Main Bluetopia Dispatch */
   /* thread in response to a Process Received Message.  This function  */
   /* SHOULD ONLY be called in this capacity and NOT under any other    */
   /* circumstances.                                                    */
   /* * NOTE * This function *MUST* be called with the Health Device    */
   /*          Manager Mutex held.  This function will release the Mutex*/
   /*          before it exits (i.e. the caller SHOULD NOT RELEASE THE  */
   /*          MUTEX).                                                  */
static void ProcessReceivedMessage(BTPM_Message_t *Message)
{
   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", Message->MessageHeader.MessageFunction));

   /* Check to make sure this isn't some kind of spoofed response (we do*/
   /* not process responses).                                           */
   if((Message) && (!(Message->MessageHeader.MessageID & BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK)))
   {
      /* Request received, go ahead and process it.                     */
      switch(Message->MessageHeader.MessageFunction)
      {
         case HDPM_MESSAGE_FUNCTION_CONNECTION_STATUS:
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Connection Status Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDPM_CONNECTION_STATUS_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* incoming connection request event.                    */
               ProcessConnectionStatus((HDPM_Connection_Status_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HDPM_MESSAGE_FUNCTION_DISCONNECTED:
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Disconnected Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDPM_DISCONNECTED_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* event.                                                */
               ProcessDisconnected((HDPM_Disconnected_Message_t *)Message);

               /* We do not need to release the Mutex as it was already */
               /* released when the event was processed.                */
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HDPM_MESSAGE_FUNCTION_INCOMING_DATA_CONNECTION_REQUEST:
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Incoming Data Connnection Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDPM_INCOMING_DATA_CONNECTION_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* event.                                                */
               ProcessIncomingDataConnectionRequest((HDPM_Incoming_Data_Connection_Message_t *)Message);

               /* We do not need to release the Mutex as it was already */
               /* released when the event was processed.                */
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HDPM_MESSAGE_FUNCTION_DATA_CONNECTED:
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Data Connected Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDPM_DATA_CONNECTED_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* event.                                                */
               ProcessDataConnected((HDPM_Data_Connected_Message_t *)Message);

               /* We do not need to release the Mutex as it was already */
               /* released when the event was processed.                */
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HDPM_MESSAGE_FUNCTION_DATA_DISCONNECTED:
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Data Disconnected Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDPM_DATA_DISCONNECTED_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* event.                                                */
               ProcessDataDisconnected((HDPM_Data_Disconnected_Message_t *)Message);

               /* We do not need to release the Mutex as it was already */
               /* released when the event was processed.                */
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HDPM_MESSAGE_FUNCTION_DATA_CONNECTION_STATUS:
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Data Connection Status Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDPM_DATA_CONNECTION_STATUS_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* event.                                                */
               ProcessDataConnectionStatus((HDPM_Data_Connection_Status_Message_t *)Message);

               /* We do not need to release the Mutex as it was already */
               /* released when the event was processed.                */
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HDPM_MESSAGE_FUNCTION_DATA_RECEIVED:
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Data Received Message\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDPM_DATA_RECEIVED_MESSAGE_SIZE(0)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDPM_DATA_RECEIVED_MESSAGE_SIZE(((HDPM_Data_Received_Message_t *)Message)->DataLength)))
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* event.                                                */
               ProcessDataReceived((HDPM_Data_Received_Message_t *)Message);

               /* We do not need to release the Mutex as it was already */
               /* released when the event was processed.                */
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         default:
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled Message: %d\n", Message->MessageHeader.MessageFunction));

            /* No message was processed, so we need to Release the Mutex*/
            /* at this time.                                            */
            BTPS_ReleaseMutex(HDPManagerMutex);
            break;
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid or Response Message Received\n"));

      /* No message was processed, so we need to Release the Mutex at   */
      /* this time.                                                     */
      BTPS_ReleaseMutex(HDPManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process Health Device Manager Asynchronous Events.  */
static void BTPSAPI BTPMDispatchCallback_HDPM(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's determine if the specified parameter appears to be          */
   /* semi-valid.                                                       */
   if(CallbackParameter)
   {
      /* Next, let's check to see if we need to process it.             */
      if(Initialized)
      {
         /* Attempt to wait for access to the Health Device state       */
         /* information.                                                */
         if(BTPS_WaitMutex(HDPManagerMutex, BTPS_INFINITE_WAIT))
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

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process Server un-register asynchronous events.     */
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter)
{
   HDP_Entry_Info_t *HDPEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's determine if the specified parameter appears to be          */
   /* semi-valid.                                                       */
   if(CallbackParameter)
   {
      /* Next, let's check to see if we need to process it.             */
      if(Initialized)
      {
         /* Attempt to wait for access to the Hands Free state          */
         /* information.                                                */
         if(BTPS_WaitMutex(HDPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Make sure we cancel any synchronous connections.         */
            HDPEntryInfo = HDPEntryInfoList;

            while(HDPEntryInfo)
            {
               /* Check to see if there is a synchronous open operation.*/
               if((!(HDPEntryInfo->Flags & HDP_ENTRY_INFO_FLAGS_WAITING_ON_CONNECT)) && (HDPEntryInfo->ConnectionEvent))
               {
                  HDPEntryInfo->ConnectionStatus = HDPM_CONNECTION_STATUS_FAILURE_DEVICE_POWER_OFF;

                  BTPS_SetEvent(HDPEntryInfo->ConnectionEvent);
               }

               HDPEntryInfo = HDPEntryInfo->NextEntryInfoPtr;
            }

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HDPManagerMutex);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function represents the Message Group Handler       */
   /* function that is installed to process all Health Device Manager   */
   /* Messages.                                                         */
static void BTPSAPI HealthDeviceManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the Message specified appears to be semi-valid.   */
   if(Message)
   {
      /* Next, check to see if this message is specified for our        */
      /* specific Message Group.                                        */
      if(Message->MessageHeader.MessageGroup == BTPM_MESSAGE_GROUP_HEALTH_DEVICE_MANAGER)
      {
         DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Health Device Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

         /* Message is destined for us, make sure we are still          */
         /* processing messages.                                        */
         if(Initialized)
         {
            /* Check to see if this message is a Health Device Manager  */
            /* defined Message. If it is it will be within the range:   */
            /*                                                          */
            /*    - BTPM_MESSAGE_FUNCTION_MINIMUM                       */
            /*    - BTPM_MESSAGE_FUNCTION_MAXIMUM                       */
            /*                                                          */
            /* See BTPMMSGT.h for more information on message functions */
            /* that are defined outside of this range.                  */
            if((Message->MessageHeader.MessageFunction >= BTPM_MESSAGE_FUNCTION_MINIMUM) && (Message->MessageHeader.MessageFunction <= BTPM_MESSAGE_FUNCTION_MAXIMUM))
            {
               /* Still processing, go ahead and post the message to the*/
               /* Health Device Manager thread.                         */
               if(!BTPM_QueueMailboxCallback(BTPMDispatchCallback_HDPM, (void *)Message))
               {
                  DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue Health Device Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

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
                        DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue Health Device Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));
                     }
                  }
               }
            }
         }
         else
         {
            /* Check to see if this message is an Health Device Manager */
            /* defined Message. If it is it will be within the range:   */
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
         DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Non Health Device Manager Message: 0x%08X, 0x%08X\n", Message->MessageHeader.MessageGroup, Message->MessageHeader.MessageFunction));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for initializing/cleaning up*/
   /* the Bluetopia Platform Manager Health Device Manager module. This */
   /* function should be registered with the Bluetopia Platform Manager */
   /* module handler and will be called when the Platform Manager is    */
   /* initialized (or shut down).                                       */
void BTPSAPI HDPM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData)
{
   int               Result;
   HDP_Entry_Info_t *HDPEntryInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %d\n", Initialize));

   /* First, determine if the request is to initialize or shut down.    */
   if(Initialize)
   {
      DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Initialization Request\n"));

      if(!Initialized)
      {
         DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing Health Device Manager\n"));

         /* First, let's create a Mutex to guard access to this module. */
         if((HDPManagerMutex = BTPS_CreateMutex(FALSE)) != NULL)
         {
            /* Next, let's attempt to register our Message Group Handler*/
            /* to process Health Device Manager messages.               */
            if(!MSG_RegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_HEALTH_DEVICE_MANAGER, HealthDeviceManagerGroupHandler, NULL))
            {
               /* Initialize the actual Health Device Manager           */
               /* Implementation Module (this is the module that is     */
               /* actually responsible for actually implementing the    */
               /* HDP Manager functionality - this module is just the   */
               /* framework shell).                                     */
               if(!(Result = _HDPM_Initialize()))
               {
                  /* Note the current Power State.                      */
                  CurrentPowerState = (Boolean_t)((DEVM_QueryDevicePowerState() > 0)?TRUE:FALSE);

                  /* Initialize a unique, starting HDPM Callback ID.    */
                  NextTemporaryHandle = 0x80000000;

                  /* Go ahead and flag that this module is initialized. */
                  Initialized         = TRUE;
               }
            }
            else
               Result = BTPM_ERROR_CODE_UNABLE_TO_REGISTER_HANDLER;
         }
         else
            Result = BTPM_ERROR_CODE_UNABLE_TO_CREATE_MUTEX;

         /* If an error occurred then we need to free all resources that*/
         /* were allocated.                                             */
         if(Result < 0)
         {
            if(HDPManagerMutex)
               BTPS_CloseMutex(HDPManagerMutex);

            MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_HEALTH_DEVICE_MANAGER);

            /* Flag that none of the resources are allocated.           */
            HDPManagerMutex     = NULL;
         }
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("HDP Manager Already initialized\n"));
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Shutdown Request\n"));

      if(Initialized)
      {
         /* First, let's make sure that we un-register our Message Group*/
         /* Handler (so that we do not process any more incoming        */
         /* messages).                                                  */
         MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_HEALTH_DEVICE_MANAGER);

         /* Wait for access to the Mutex that guards access to this     */
         /* module.                                                     */
         if(BTPS_WaitMutex(HDPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Next, Un-Register for any Endpoints. First, close all    */
            /* Endpoint connections.                                    */
            HDPEntryInfoPtr = HDPEntryInfoList;

            while(HDPEntryInfoPtr)
            {
               if((HDPEntryInfoPtr->EntryType == etRemotelyInitiatedEndpointConnection) || (HDPEntryInfoPtr->EntryType == etLocallyInitiatedEndpointConnection))
                  _HDPM_Disconnect_Remote_Device_Endpoint(HDPEntryInfoPtr->Handle);

               HDPEntryInfoPtr = HDPEntryInfoPtr->NextEntryInfoPtr;
            }

            /* Now, shut down all endpoint servers and remote Instance  */
            /* connections.                                             */
            HDPEntryInfoPtr = HDPEntryInfoList;

            while(HDPEntryInfoPtr)
            {
               if(HDPEntryInfoPtr->EntryType == etLocallyRegisteredEndpoint)
                  _HDPM_Un_Register_Endpoint(HDPEntryInfoPtr->Handle);

               if(HDPEntryInfoPtr->EntryType == etRemoteInstanceConnection)
                  _HDPM_Disconnect_Remote_Device(HDPEntryInfoPtr->BD_ADDR, HDPEntryInfoPtr->Instance);

               HDPEntryInfoPtr = HDPEntryInfoPtr->NextEntryInfoPtr;
            }

            /* Make sure we inform the Health Device Manager            */
            /* Implementation that we are shutting down.                */
            _HDPM_Cleanup();

            /* Make sure that the Health Device entry information list  */
            /* is empty.                                                */
            FreeHDPEntryInfoList(&HDPEntryInfoList);

            BTPS_CloseMutex(HDPManagerMutex);

            /* Flag that the resources are no longer allocated.         */
            HDPManagerMutex   = NULL;
            CurrentPowerState = FALSE;

            /* Flag that this module is no longer initialized.          */
            Initialized       = FALSE;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing asynchronous */
   /* Device Manager (DEVM) Events (including Power On/Off events).     */
   /* This function should be registered with the Bluetopia Platform    */
   /* Manager Module Handler and will be called when an asynchronous    */
   /* Device Manager event is dispatched.                               */
void BTPSAPI HDPM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData)
{
   HDP_Entry_Info_t *HDPEntryInfo;
   HDP_Entry_Info_t *tmpHDPEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Hands Free Manager has been         */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(BTPS_WaitMutex(HDPManagerMutex, BTPS_INFINITE_WAIT))
      {
         switch(EventData->EventType)
         {
            case detDevicePoweredOn:
               DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power On Occurred.\n"));

               /* Note the new Power State.                             */
               CurrentPowerState = TRUE;
               break;
            case detDevicePoweringOff:
            case detDevicePoweredOff:
               DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power Off Occurred.\n"));

               /* Power off event, let's loop through ALL the registered*/
               /* Hands Free entries and set any events that have       */
               /* synchronous operations pending.                       */
               HDPEntryInfo = HDPEntryInfoList;

               while(HDPEntryInfo)
               {
                  /* Check to see if there is a synchronous open        */
                  /* operation.                                         */
                  if((!(HDPEntryInfo->Flags & HDP_ENTRY_INFO_FLAGS_WAITING_ON_CONNECT)) && (HDPEntryInfo->ConnectionEvent))
                  {
                     HDPEntryInfo->ConnectionStatus = HDPM_CONNECTION_STATUS_FAILURE_DEVICE_POWER_OFF;

                     BTPS_SetEvent(HDPEntryInfo->ConnectionEvent);
                  }
                  else
                  {
                     /* Clear out the entry since the endpoint is going */
                     /* away.                                           */
                     if((tmpHDPEntryInfo = DeleteHDPEntryInfoEntryHandle(&HDPEntryInfoList, HDPEntryInfo->EntryType, HDPEntryInfo->Handle)) != NULL)
                        FreeHDPEntryInfoEntryMemory(tmpHDPEntryInfo);
                  }

                  HDPEntryInfo = HDPEntryInfo->NextEntryInfoPtr;
               }

               /* Note the new Power State.                             */
               CurrentPowerState   = FALSE;
               break;
            default:
               /* Do nothing.                                           */
               break;
         }

         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(HDPManagerMutex);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to register an Endpoint on the Local HDP Server. The first*/
   /* parameter defines the Data Type that will be supported by this    */
   /* endpoint. The second parameter specifies whether the Endpoint     */
   /* will be a data source or sink. The third parameter is optional    */
   /* and can be used to specify a short, human-readable description of */
   /* the Endpoint. The final parameters specify the Event Callback and */
   /* Callback parameter (to receive events related to the registered   */
   /* endpoint). This function returns a positive, non-zero, value if   */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * A successful return value represents the Endpoint ID that*/
   /*          can be used with various functions in this module to     */
   /*          refer to this endpoint.                                  */
int BTPSAPI HDPM_Register_Endpoint(Word_t DataType, HDP_Device_Role_t LocalRole, char *Description, HDPM_Event_Callback_t EventCallback, void *CallbackParameter)
{
   int               ret_val;
   HDP_Entry_Info_t  HDPEntryInfo;
   HDP_Entry_Info_t *HDPEntryInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Health Device Manager has been      */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, verify that the input parameters appear to be semi-valid.*/
      if(EventCallback)
      {
         /* Attempt to wait for access to the Health Device Manager     */
         /* state information.                                          */
         if(BTPS_WaitMutex(HDPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* Device Powered up, go ahead and store the information */
               /* into our list.                                        */
               BTPS_MemInitialize(&HDPEntryInfo, 0, sizeof(HDP_Entry_Info_t));

               HDPEntryInfo.EntryType                      = etLocallyRegisteredEndpoint;
               HDPEntryInfo.Handle                         = GetNextTemporaryHandle();
               HDPEntryInfo.ConnectionStatus               = 0;
               HDPEntryInfo.ConnectionEvent                = BTPS_CreateEvent(FALSE);
               HDPEntryInfo.CallbackInfo.EventCallback     = EventCallback;
               HDPEntryInfo.CallbackInfo.CallbackParameter = CallbackParameter;

               /* Make sure that all events were able to be created.    */
               if(HDPEntryInfo.ConnectionEvent)
               {
                  if((HDPEntryInfoPtr = AddHDPEntryInfoEntry(&HDPEntryInfoList, &HDPEntryInfo)) != NULL)
                  {
                     DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Register Endpoint: 0x%04x\n", DataType));

                     /* Next, attempt to register the Server.           */
                     if((ret_val = _HDPM_Register_Endpoint(DataType, LocalRole, Description)) > 0)
                     {
                        /* Now, save the real registration handle.      */
                        HDPEntryInfoPtr->Handle = (unsigned int)ret_val;
                     }
                     else
                     {
                        /* Error registering endpoint, go ahead and     */
                        /* delete the entry that was added.             */
                        if((HDPEntryInfoPtr = DeleteHDPEntryInfoEntryHandle(&HDPEntryInfoList, etLocallyRegisteredEndpoint, HDPEntryInfoPtr->Handle)) != NULL)
                           FreeHDPEntryInfoEntryMemory(HDPEntryInfoPtr);
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
            BTPS_ReleaseMutex(HDPManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
   }
   else
      ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to Un-Register a previously registered Endpoint. This     */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error.                                       */
int BTPSAPI HDPM_Un_Register_Endpoint(unsigned int EndpointID)
{
   int               ret_val;
   HDP_Entry_Info_t *HDPEntryInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Health Device Manager has been      */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Attempt to wait for access to the Health Device state          */
      /* information.                                                   */
      if(BTPS_WaitMutex(HDPManagerMutex, BTPS_INFINITE_WAIT))
      {
         /* Next, check to see if we are powered up.                    */
         if(CurrentPowerState)
         {
            /* Next, attempt to delete the Health Device Entry from the */
            /* Entry List.                                              */
            if((HDPEntryInfoPtr = DeleteHDPEntryInfoEntryHandle(&HDPEntryInfoList, etLocallyRegisteredEndpoint, EndpointID)) != NULL)
            {
               DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Unregister Endpoint: 0x%08X (%d)\n", EndpointID, EndpointID));

               /* Release the Mutex because we are finished with it.    */
               BTPS_ReleaseMutex(HDPManagerMutex);

               /* Next, go ahead and Un-Register the Endpoint.          */
               ret_val = _HDPM_Un_Register_Endpoint(EndpointID);

               /* All finished, free any memory that was allocated for  */
               /* the port.                                             */
               FreeHDPEntryInfoEntryMemory(HDPEntryInfoPtr);
            }
            else
            {
               ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_INVALID_ENDPOINT_ID;

               /* Release the Mutex because we are finished with it.    */
               BTPS_ReleaseMutex(HDPManagerMutex);
            }
         }
         else
         {
            ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HDPManagerMutex);
         }
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* module to respond to a request to establish a data connection to a*/
   /* local endpoint. The first parameter is the DataLinkID associated  */
   /* with the connection request. The second parameter is one of       */
   /* the MCAP_RESPONSE_CODE_* constants which indicates either that    */
   /* the request should be accepted (MCAP_RESPONSE_CODE_SUCCESS) or    */
   /* provides a reason for rejecting the request. If the request is to */
   /* be accepted, and the request is for a local Data Source, the final*/
   /* parameter indicates whether the connection shall use the Reliable */
   /* or Streaming communication mode. This function returns zero if    */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * If the connection is accepted, it does NOT mean that the */
   /*          connection is successfully connected.  A Data Connected  */
   /*          event will be dispatched to signify the actual result.   */
   /* * NOTE * If the connection is accepted, and the connection request*/
   /*          is for a local Data Sink, then ChannelMode must be set to*/
   /*          the Mode indicated in the request.  If the connection is */
   /*          accepted for a local Data Source, ChannelMode must be set*/
   /*          to either cmReliable or cmStreaming. If the connection   */
   /*          request is rejected, ChannelMode is ignored.             */
int BTPSAPI HDPM_Data_Connection_Request_Response(unsigned int DataLinkID, Byte_t ResponseCode, HDP_Channel_Mode_t ChannelMode)
{
   int               ret_val;
   HDP_Entry_Info_t *HDPEntryInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Health Device Manager has been      */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Attempt to wait for access to the Health Device state          */
      /* information.                                                   */
      if(BTPS_WaitMutex(HDPManagerMutex, BTPS_INFINITE_WAIT))
      {
         /* Next, attempt to find the Endpoint's Entry in the list.     */
         if((HDPEntryInfoPtr = SearchHDPEntryInfoEntryHandle(&HDPEntryInfoList, etRemotelyInitiatedEndpointConnection, DataLinkID)) != NULL)
         {
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Respond to Connection Request: 0x%08X (%d), 0x%02X\n", DataLinkID, DataLinkID, ResponseCode));

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HDPManagerMutex);

            /* Next, go ahead and send the connection request response. */
            ret_val = _HDPM_Data_Connection_Request_Response(DataLinkID, ResponseCode, ChannelMode);

            /* Check if a failure occurred above or the user responded  */
            /* with a negative response code.                           */
            if((ret_val) || (ResponseCode != MCAP_RESPONSE_CODE_SUCCESS))
            {
               /* A failure occurred above or the user responded with a */
               /* negative response code, remove the connection entry   */
               /* from the list.                                        */
               if((HDPEntryInfoPtr = DeleteHDPEntryInfoEntryHandle(&HDPEntryInfoList, etRemotelyInitiatedEndpointConnection, DataLinkID)) != NULL)
                  FreeHDPEntryInfoEntryMemory(HDPEntryInfoPtr);
            }
         }
         else
         {
            ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_INVALID_DATALINK_ID;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HDPManagerMutex);
         }
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for       */
   /* local modules to query the available HDP Instances on a remote    */
   /* device. The first parameter specifies the Address of the Remote   */
   /* Device to query. The second parameter specifies the maximum       */
   /* number of Instances that the buffer will support (i.e. can be     */
   /* copied into the buffer). The next parameter is optional and,      */
   /* if specified, will be populated with up to the total number of    */
   /* Instances advertised by the remote device, if the function is     */
   /* successful. The final parameter is optional and can be used to    */
   /* retrieve the total number of available Instances (regardless of   */
   /* the size of the list specified by the first two parameters).      */
   /* This function returns a non-negative value if successful which    */
   /* represents the number of Instances that were copied into the      */
   /* specified buffer. This function returns a negative return error   */
   /* code if there was an error.                                       */
   /* * NOTE * It is possible to specify zero for the Maximum Instance  */
   /*          List Entries, in which case the final parameter *MUST* be*/
   /*          specified.                                               */
int BTPSAPI HDPM_Query_Remote_Device_Instances(BD_ADDR_t RemoteDeviceAddress, unsigned int MaximumInstanceListEntries, DWord_t *InstanceList, unsigned int *TotalNumberInstances)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Health Device Manager has been      */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check that the parameters appears semi-valid.                  */
      if((!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (((MaximumInstanceListEntries) && (InstanceList)) || ((!MaximumInstanceListEntries) && (!InstanceList) && (TotalNumberInstances))))
      {
         /* Simply issue the query.                                     */
         ret_val = _HDPM_Query_Remote_Device_Instances(RemoteDeviceAddress, MaximumInstanceListEntries, InstanceList, TotalNumberInstances);
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to query the available Endpoints published for a specific */
   /* HDP Instances on a remote device. The first parameter specifies   */
   /* the Address of the Remote Device to query. The second parameter   */
   /* specifies Instance on the Remote Device. The third parameter      */
   /* specifies the maximum number of Endpoints that the buffer will    */
   /* support (i.e. can be copied into the buffer). The next parameter  */
   /* is optional and, if specified, will be populated with up to the   */
   /* total number of Endpoints published by the remote device, if the  */
   /* function is successful. The final parameter is optional and can   */
   /* be used to retrieve the total number of Endpoints (regardless     */
   /* of the size of the list specified by the first two parameters).   */
   /* This function returns a non-negative value if successful which    */
   /* represents the number of Endpoints that were copied into the      */
   /* specified buffer. This function returns a negative return error   */
   /* code if there was an error.                                       */
   /* * NOTE * It is possible to specify zero for the Maximum Endpoint  */
   /*          List Entries, in which case the final parameter *MUST* be*/
   /*          specified.                                               */
int BTPSAPI HDPM_Query_Remote_Device_Instance_Endpoints(BD_ADDR_t RemoteDeviceAddress, DWord_t Instance, unsigned int MaximumEndpointListEntries, HDPM_Endpoint_Info_t *EndpointInfoList, unsigned int *TotalNumberEndpoints)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Health Device Manager has been      */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check that the parameters appears semi-valid.                  */
      if((!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (Instance) && (((MaximumEndpointListEntries) && (EndpointInfoList)) || ((!MaximumEndpointListEntries) && (!EndpointInfoList) && (TotalNumberEndpoints))))
      {
         /* Simply issue the query.                                     */
         ret_val = _HDPM_Query_Remote_Device_Instance_Endpoints(RemoteDeviceAddress, Instance, MaximumEndpointListEntries, EndpointInfoList, TotalNumberEndpoints);
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to query the description of a known Endpoint published in */
   /* a specific HDP Instance by a remote device. The first parameter   */
   /* specifies the Address of the Remote Device to query. The second   */
   /* parameter specifies Instance on the Remote Device. The third      */
   /* parameter identifies the Endpoint to query. The fourth and fifth  */
   /* parameters specific the size of the buffer and the buffer to hold */
   /* the description string, respectively. The final parameter is      */
   /* optional and, if specified, will be set to the total size of the  */
   /* description string for the given Endpoint, if the function is     */
   /* successful (regardless of the size of the list specified by the   */
   /* first two parameters). This function returns a non-negative value */
   /* if successful which represents the number of bytes copied into the*/
   /* specified buffer. This function returns a negative return error   */
   /* code if there was an error.                                       */
   /* * NOTE * It is possible to specify zero for the Maximum           */
   /*          Description Length, in which case the final parameter    */
   /*          *MUST* be specified.                                     */
int BTPSAPI HDPM_Query_Remote_Device_Endpoint_Description(BD_ADDR_t RemoteDeviceAddress, DWord_t Instance, HDPM_Endpoint_Info_t *EndpointInfo, unsigned int MaximumDescriptionLength, char *DescriptionBuffer, unsigned int *TotalDescriptionLength)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Health Device Manager has been      */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check that the parameters appears semi-valid.                  */
      if((!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (Instance) && (EndpointInfo) && (((MaximumDescriptionLength) && (DescriptionBuffer)) || ((!MaximumDescriptionLength) && (!DescriptionBuffer) && (TotalDescriptionLength))))
      {
         /* Simply issue the query.                                     */
         ret_val = _HDPM_Query_Remote_Device_Endpoint_Description(RemoteDeviceAddress, Instance, EndpointInfo, MaximumDescriptionLength, DescriptionBuffer, TotalDescriptionLength);
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to establish a connection to a specific HDP Instance on a */
   /* Remote Device. The first parameter specifies the Remote Device to */
   /* connect to. The second parameter specifies the HDP Instance on the*/
   /* remote device. The next two parameters specify the (optional)     */
   /* Event Callback and Callback parameter (to receive events related  */
   /* to the connection attempt). This function returns zero if         */
   /* successful, or a negative return value if there was an error.     */
   /* * NOTE * The final parameter to this function, if specified,      */
   /*          instructs this function to block until the connection    */
   /*          status is received (i.e. the connection is completed).   */
   /*          If this parameter is not specified (i.e. NULL) then the  */
   /*          connection status will be returned asynchronously in the */
   /*          Open Remote Port Result event.                           */
int BTPSAPI HDPM_Connect_Remote_Device(BD_ADDR_t RemoteDeviceAddress, DWord_t Instance, HDPM_Event_Callback_t EventCallback, void *CallbackParameter, unsigned int *ConnectionStatus)
{
   int               ret_val;
   Event_t           ConnectionEvent;
   HDP_Entry_Info_t  HDPEntryInfo;
   HDP_Entry_Info_t *HDPEntryInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Health Device Manager has been      */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check that the parameters appear semi-valid.                   */
      if((!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (Instance) && (EventCallback))
      {
         /* Attempt to wait for access to the Health Device state       */
         /* information.                                                */
         if(BTPS_WaitMutex(HDPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* Device is powered on, attempt to add an entry into the*/
               /* health device entry list.                             */
               BTPS_MemInitialize(&HDPEntryInfo, 0, sizeof(HDPEntryInfo));

               HDPEntryInfo.EntryType                      = etRemoteInstanceConnection;
               HDPEntryInfo.Handle                         = GetNextTemporaryHandle();
               HDPEntryInfo.ConnectionStatus               = 0;
               HDPEntryInfo.ConnectionEvent                = BTPS_CreateEvent(FALSE);
               HDPEntryInfo.BD_ADDR                        = RemoteDeviceAddress;
               HDPEntryInfo.Instance                       = Instance;
               HDPEntryInfo.CallbackInfo.EventCallback     = EventCallback;
               HDPEntryInfo.CallbackInfo.CallbackParameter = CallbackParameter;

               if(HDPEntryInfo.ConnectionEvent)
               {
                  if((HDPEntryInfoPtr = AddHDPEntryInfoEntry(&HDPEntryInfoList, &HDPEntryInfo)) != NULL)
                  {
                     DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to open Remote Instance %02X:%02X:%02X:%02X:%02X:%02X, 0x%08X\n", RemoteDeviceAddress.BD_ADDR5, RemoteDeviceAddress.BD_ADDR4, RemoteDeviceAddress.BD_ADDR3, RemoteDeviceAddress.BD_ADDR2, RemoteDeviceAddress.BD_ADDR1, RemoteDeviceAddress.BD_ADDR0, Instance));

                     /* Next, attempt to open the remote port.          */
                     if((ret_val = _HDPM_Connect_Remote_Device(RemoteDeviceAddress, Instance)) == 0)
                     {
                        /* Remote Instance connection submitted. If we  */
                        /* need to wait on the connection to complete,  */
                        /* note this.                                   */
                        if(ConnectionStatus)
                           HDPEntryInfoPtr->Flags |= HDP_ENTRY_INFO_FLAGS_WAITING_ON_CONNECT;
                     }
                     else
                     {
                        /* Error opening port, go ahead and delete the  */
                        /* entry that was added.                        */
                        if((HDPEntryInfoPtr = DeleteHDPEntryInfoEntryInstance(&HDPEntryInfoList, etRemoteInstanceConnection, HDPEntryInfoPtr->BD_ADDR, HDPEntryInfoPtr->Instance)) != NULL)
                           FreeHDPEntryInfoEntryMemory(HDPEntryInfoPtr);
                     }

                     /* Next, determine if the caller has requested a   */
                     /* blocking open.                                  */
                     if((ret_val == 0) && (ConnectionStatus))
                     {
                        /* Blocking open, go ahead and wait for the     */
                        /* event.                                       */

                        /* Note the Event.                              */
                        ConnectionEvent = HDPEntryInfoPtr->ConnectionEvent;

                        /* Release the Mutex because we are finished    */
                        /* with it.                                     */
                        BTPS_ReleaseMutex(HDPManagerMutex);

                        BTPS_WaitEvent(ConnectionEvent, BTPS_INFINITE_WAIT);

                        /* Re-acquire the Mutex.                        */
                        if(BTPS_WaitMutex(HDPManagerMutex, BTPS_INFINITE_WAIT))
                        {
                           if((HDPEntryInfoPtr = SearchHDPEntryInfoEntryInstance(&HDPEntryInfoList, etRemoteInstanceConnection, RemoteDeviceAddress, Instance)) != NULL)
                           {
                              /* Flag that we are no longer opening the */
                              /* port.                                  */
                              HDPEntryInfoPtr->Flags &= ~HDP_ENTRY_INFO_FLAGS_WAITING_ON_CONNECT;

                              *ConnectionStatus       = HDPEntryInfoPtr->ConnectionStatus;

                              if(HDPEntryInfoPtr->ConnectionStatus)
                              {
                                 /* Open failed, so note the status and */
                                 /* remove the Health Device entry from */
                                 /* the list.                           */
                                 ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_UNABLE_TO_CONNECT_TO_DEVICE;

                                 if((HDPEntryInfoPtr = DeleteHDPEntryInfoEntryInstance(&HDPEntryInfoList, etRemoteInstanceConnection, HDPEntryInfoPtr->BD_ADDR, HDPEntryInfoPtr->Instance)) != NULL)
                                    FreeHDPEntryInfoEntryMemory(HDPEntryInfoPtr);
                              }
                           }
                           else
                           {
                              ret_val           = BTPM_ERROR_CODE_HEALTH_DEVICE_UNABLE_TO_CONNECT_TO_DEVICE;
                              *ConnectionStatus = HDPM_CONNECTION_STATUS_FAILURE_UNKNOWN;
                           }
                        }
                        else
                           ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
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
               BTPS_ReleaseMutex(HDPManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to close an existing connection to a specific HDP Instance*/
   /* on a Remote Device. The first parameter specifies the Remote      */
   /* Device. The second parameter specifies the HDP Instance on the    */
   /* remote device from which to disconnect. This function returns zero*/
   /* if successful, or a negative return value if there was an error.  */
int BTPSAPI HDPM_Disconnect_Remote_Device(BD_ADDR_t RemoteDeviceAddress, DWord_t Instance)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Health Device Manager has been      */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check that the parameters appear semi-valid.                   */
      if((!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (Instance))
      {
         /* Attempt to wait for access to the Health Device state       */
         /* information.                                                */
         if(BTPS_WaitMutex(HDPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* Release the Mutex because we are finished with it.    */
               BTPS_ReleaseMutex(HDPManagerMutex);

               /* Simply submit the request.                            */
               ret_val = _HDPM_Disconnect_Remote_Device(RemoteDeviceAddress, Instance);
            }
            else
            {
               /* Release the Mutex because we are finished with it.    */
               BTPS_ReleaseMutex(HDPManagerMutex);

               ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;
            }
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for       */
   /* local modules to establish an HDP connection to an Endpoint of    */
   /* a specific HDP Instance on a Remote Device. The first parameter   */
   /* specifies the Remote Device to connect to. The second parameter   */
   /* specifies the HDP Instance on the remote device. The third        */
   /* parameter specifies the Endpoint of that Instance to which the    */
   /* connection will be attempted. The fourth parameter specifies      */
   /* the type of connection that will be established. The next two     */
   /* parameters specify the Event Callback and Callback parameter (to  */
   /* receive events related to the connection). This function returns a*/
   /* positive value if successful, or a negative return value if there */
   /* was an error.                                                     */
   /* * NOTE * A successful return value represents the Data Link ID    */
   /*          shall be used with various functions and by various      */
   /*          events in this module to reference this data connection. */
   /* * NOTE * The final parameter to this function, if specified,      */
   /*          instructs this function to block until the connection    */
   /*          status is received (i.e. the connection is completed).   */
   /*          If this parameter is not specified (i.e. NULL) then the  */
   /*          connection status will be returned asynchronously in the */
   /*          Open Remote Port Result event.                           */
int BTPSAPI HDPM_Connect_Remote_Device_Endpoint(BD_ADDR_t RemoteDeviceAddress, DWord_t Instance, Byte_t EndpointID, HDP_Channel_Mode_t ChannelMode, HDPM_Event_Callback_t EventCallback, void *CallbackParameter, unsigned int *ConnectionStatus)
{
   int               ret_val;
   Event_t           ConnectionEvent;
   unsigned int      DataLinkID;
   HDP_Entry_Info_t  HDPEntryInfo;
   HDP_Entry_Info_t *HDPEntryInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Health Device Manager has been      */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check that the parameters appear semi-valid.                   */
      if((!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (Instance) && (EventCallback))
      {
         /* Attempt to wait for access to the Health Device state       */
         /* information.                                                */
         if(BTPS_WaitMutex(HDPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* Device is powered on, attempt to add an entry into the*/
               /* health device entry list.                             */
               BTPS_MemInitialize(&HDPEntryInfo, 0, sizeof(HDPEntryInfo));

               HDPEntryInfo.EntryType                      = etLocallyInitiatedEndpointConnection;
               HDPEntryInfo.Handle                         = GetNextTemporaryHandle();
               HDPEntryInfo.ConnectionStatus               = 0;
               HDPEntryInfo.ConnectionEvent                = BTPS_CreateEvent(FALSE);
               HDPEntryInfo.BD_ADDR                        = RemoteDeviceAddress;
               HDPEntryInfo.Instance                       = Instance;
               HDPEntryInfo.CallbackInfo.EventCallback     = EventCallback;
               HDPEntryInfo.CallbackInfo.CallbackParameter = CallbackParameter;

               if(HDPEntryInfo.ConnectionEvent)
               {
                  if((HDPEntryInfoPtr = AddHDPEntryInfoEntry(&HDPEntryInfoList, &HDPEntryInfo)) != NULL)
                  {
                     DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to open Remote Instance Endpoint %02X:%02X:%02X:%02X:%02X:%02X, 0x%08X, 0x%02X\n", RemoteDeviceAddress.BD_ADDR5, RemoteDeviceAddress.BD_ADDR4, RemoteDeviceAddress.BD_ADDR3, RemoteDeviceAddress.BD_ADDR2, RemoteDeviceAddress.BD_ADDR1, RemoteDeviceAddress.BD_ADDR0, Instance, EndpointID));

                     /* Next, attempt to open the remote endpoint.      */
                     if((ret_val = _HDPM_Connect_Remote_Device_Endpoint(RemoteDeviceAddress, Instance, EndpointID, ChannelMode)) >= 0)
                     {
                        /* Remote Instance Endpoint connection          */
                        /* submitted. Record the Data Link ID.          */
                        HDPEntryInfoPtr->Handle = (unsigned int)ret_val;

                        /* If we need to wait on the connection to      */
                        /* complete, note this.                         */
                        if(ConnectionStatus)
                           HDPEntryInfoPtr->Flags |= HDP_ENTRY_INFO_FLAGS_WAITING_ON_CONNECT;
                     }
                     else
                     {
                        /* Error opening endpoint, go ahead and delete  */
                        /* the entry that was added.                    */
                        if((HDPEntryInfoPtr = DeleteHDPEntryInfoEntryHandle(&HDPEntryInfoList, etLocallyInitiatedEndpointConnection, HDPEntryInfoPtr->Handle)) != NULL)
                           FreeHDPEntryInfoEntryMemory(HDPEntryInfoPtr);
                     }

                     /* Next, determine if the caller has requested a   */
                     /* blocking open.                                  */
                     if((ret_val >= 0) && (ConnectionStatus))
                     {
                        /* Blocking open, go ahead and wait for the     */
                        /* event.                                       */

                        /* Note the connection handle.                  */
                        DataLinkID      = HDPEntryInfoPtr->Handle;

                        /* Note the Event.                              */
                        ConnectionEvent = HDPEntryInfoPtr->ConnectionEvent;

                        /* Release the Mutex because we are finished    */
                        /* with it.                                     */
                        BTPS_ReleaseMutex(HDPManagerMutex);

                        BTPS_WaitEvent(ConnectionEvent, BTPS_INFINITE_WAIT);

                        /* Re-acquire the Mutex.                        */
                        if(BTPS_WaitMutex(HDPManagerMutex, BTPS_INFINITE_WAIT))
                        {
                           if((HDPEntryInfoPtr = SearchHDPEntryInfoEntryHandle(&HDPEntryInfoList, etLocallyInitiatedEndpointConnection, DataLinkID)) != NULL)
                           {
                              /* Flag that we are no longer opening the */
                              /* port.                                  */
                              HDPEntryInfoPtr->Flags &= ~HDP_ENTRY_INFO_FLAGS_WAITING_ON_CONNECT;

                              *ConnectionStatus       = HDPEntryInfoPtr->ConnectionStatus;

                              if(HDPEntryInfoPtr->ConnectionStatus)
                              {
                                 /* Open failed, so note the status and */
                                 /* remove the Health Device entry from */
                                 /* the list.                           */
                                 ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_UNABLE_TO_CONNECT_TO_ENDPOINT;

                                 if((HDPEntryInfoPtr = DeleteHDPEntryInfoEntryHandle(&HDPEntryInfoList, etLocallyInitiatedEndpointConnection, HDPEntryInfoPtr->Handle)) != NULL)
                                    FreeHDPEntryInfoEntryMemory(HDPEntryInfoPtr);
                              }
                           }
                           else
                           {
                              ret_val           = BTPM_ERROR_CODE_HEALTH_DEVICE_UNABLE_TO_CONNECT_TO_ENDPOINT;
                              *ConnectionStatus = HDPM_CONNECTION_STATUS_FAILURE_UNKNOWN;
                           }
                        }
                        else
                           ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
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
               BTPS_ReleaseMutex(HDPManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for       */
   /* local modules to disconnect an established HDP data connection.   */
   /* This function accepts the Data Link ID of the data connection     */
   /* to disconnect. This function returns zero if successful, or a     */
   /* negative return value if there was an error.                      */
int BTPSAPI HDPM_Disconnect_Remote_Device_Endpoint(unsigned int DataLinkID)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Health Device Manager has been      */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Attempt to wait for access to the Health Device state          */
      /* information.                                                   */
      if(BTPS_WaitMutex(HDPManagerMutex, BTPS_INFINITE_WAIT))
      {
         /* Next, check to see if we are powered up.                    */
         if(CurrentPowerState)
         {
            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HDPManagerMutex);

            /* Simply submit the request.                               */
            ret_val = _HDPM_Disconnect_Remote_Device_Endpoint(DataLinkID);
         }
         else
         {
            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HDPManagerMutex);

            ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;
         }
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to send data over an established HDP data connection. The */
   /* first parameter is the Data Link ID which represents the data     */
   /* connection to use. The final parameters specify the data (and     */
   /* amount) to be sent. This function returns zero if successful, or a*/
   /* negative return error code if there was an error.                 */
   /* * NOTE * This function will either send all of the data or none of*/
   /*          the data.                                                */
int BTPSAPI HDPM_Write_Data(unsigned int DataLinkID, unsigned int DataLength, Byte_t *DataBuffer)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Health Device Manager has been      */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check that the parameters appear semi-valid.                   */
      if((DataLength) && (DataBuffer))
      {
         /* Attempt to wait for access to the Health Device state       */
         /* information.                                                */
         if(BTPS_WaitMutex(HDPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* Release the Mutex because we are finished with it.    */
               BTPS_ReleaseMutex(HDPManagerMutex);

               /* Simply submit the request.                            */
               ret_val = _HDPM_Write_Data(DataLinkID, DataLength, DataBuffer);
            }
            else
            {
               /* Release the Mutex because we are finished with it.    */
               BTPS_ReleaseMutex(HDPManagerMutex);

               ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;
            }
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

