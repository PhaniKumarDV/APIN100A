/*****< btpmoppm.c >***********************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMOPPM - Object Push Manager for Stonestreet One Bluetooth              */
/*             Protocol Stack Platform Manager.                               */
/*                                                                            */
/*  Author:  Matt Seabold                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   12/09/13  M. Seabold     Initial creation.                               */
/******************************************************************************/
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMOPPM.h"            /* BTPM OPPM Manager Prototypes/Constants.   */
#include "OPPMAPI.h"             /* OPPM Manager Prototypes/Constants.        */
#include "OPPMMSG.h"             /* BTPM OPPM Manager Message Formats.        */
#include "OPPMGR.h"              /* OPPM Manager Impl. Prototypes/Constants.  */
#include "OPPMUTIL.h"            /* OPPM Manager Utility Functions.           */

#include "SS1BTPM.h"             /* BTPM Main Prototypes and Constants.       */
#include "BTPMERR.h"             /* BTPM Error Prototypes/Constants.          */
#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

   /* Structure which is used to track Callback Information related to  */
   /* this module.                                                      */
typedef struct _tagOPPM_Entry_Info_t
{
   unsigned int                  TrackingID;
   unsigned int                  OPPTrackingID;
   unsigned int                  ServerID;
   BD_ADDR_t                     RemoteDeviceAddress;
   unsigned int                  ConnectionStatus;
   Event_t                       ConnectionEvent;
   unsigned long                 Flags;
   OPPM_Event_Callback_t         EventCallback;
   void                         *CallbackParameter;
   struct _tagOPPM_Entry_Info_t *NextOPPMEntryInfoPtr;
} OPPM_Entry_Info_t;

   /* The following functions are used with the Flags member of the     */
   /* OPPM_Entry_Info_t structure to denote various state information.  */
#define OPPM_ENTRY_INFO_FLAGS_CONNECTION_OPENING            0x80000000
#define OPPM_ENTRY_INFO_FLAGS_SERVER                        0x00000001

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the compiler*/
   /* as part of standard C/C++).                                       */

   /* Variable which serves as a global flag about whether this module  */
   /* is initialized or not.                                            */
static Boolean_t Initialized;

   /* Variable which is used to protect access to the state information */
   /* in this module.                                                   */
static Mutex_t OPPManagerMutex;

   /* Variable which holds the current Power State of the Device.       */
static Boolean_t CurrentPowerState;

   /* Variable which is used to hold the next (unique) tracking ID.     */
static unsigned int NextTrackingID;

   /* Variable which holds a pointer to the first element in the OPP    */
   /* Entry Information List (which holds all Callbacks tracked by this */
   /* module).                                                          */
static OPPM_Entry_Info_t *OPPMEntryInfoList;

   /* Internal Function Prototypes.                                     */
static unsigned int GetNextTrackingID(void);

static OPPM_Entry_Info_t *AddOPPMEntryInfoEntry(OPPM_Entry_Info_t **ListHead, OPPM_Entry_Info_t *EntryToAdd);
static OPPM_Entry_Info_t *SearchOPPMEntryInfoByTrackingID(OPPM_Entry_Info_t **ListHead, unsigned int TrackingID);
static OPPM_Entry_Info_t *SearchOPPMEntryInfoByOPPTrackingID(OPPM_Entry_Info_t **ListHead, unsigned int OPPTrackingID);
static OPPM_Entry_Info_t *DeleteOPPMEntryInfoEntry(OPPM_Entry_Info_t **ListHead, unsigned int TrackingID);
static void FreeOPPMEntryInfoEntryMemory(OPPM_Entry_Info_t *EntryToFree);
static void FreeOPPMEntryInfoList(OPPM_Entry_Info_t **ListHead);

static void ProcessConnectionRequestEvent(OPPM_Connection_Request_Message_t *Message);
static void ProcessConnectedEvent(OPPM_Connected_Message_t *Message);
static void ProcessDisconnectedEvent(OPPM_Disconnected_Message_t *Message);
static void ProcessConnectionStatusEvent(OPPM_Connection_Status_Message_t *Message);
static void ProcessPushObjectRequestEvent(OPPM_Push_Object_Request_Message_t *Message);
static void ProcessPushObjectResponseEvent(OPPM_Push_Object_Response_Message_t *Message);
static void ProcessPullBusinessCardRequestEvent(OPPM_Pull_Business_Card_Request_Message_t *Message);
static void ProcessPullBusinessCardResponseEvent(OPPM_Pull_Business_Card_Response_Message_t *Message);

static void ProcessReceivedMessage(BTPM_Message_t *Message);

static void BTPSAPI BTPMDispatchCallback_OPPM(void *CallbackParameter);
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter);

static void BTPSAPI OPPManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter);

   /* The following function is a utility function that exists to       */
   /* retrieve a unique, Tracking ID that can be used to track a OPP    */
   /* Entry (client or server).                                         */
static unsigned int GetNextTrackingID(void)
{
   unsigned int ret_val;

   ret_val = ++NextTrackingID;

   if(NextTrackingID & 0x80000000)
      NextTrackingID = 1;

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
   /*            callbackID field is the same as an entry already in    */
   /*            the list.  When this occurs, this function returns     */
   /*            NULL.                                                  */
static OPPM_Entry_Info_t *AddOPPMEntryInfoEntry(OPPM_Entry_Info_t **ListHead, OPPM_Entry_Info_t *EntryToAdd)
{
   OPPM_Entry_Info_t *AddedEntry = NULL;
   OPPM_Entry_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First let's verify that values passed in are semi-valid.          */
   if((ListHead) && (EntryToAdd))
   {
      /* Make sure that the element that we are adding seems semi-valid.*/
      if(EntryToAdd->TrackingID)
      {
         /* OK, data seems semi-valid, let's allocate a new data        */
         /* structure to add to the list.                               */
         AddedEntry = (OPPM_Entry_Info_t *)BTPS_AllocateMemory(sizeof(OPPM_Entry_Info_t));

         if(AddedEntry)
         {
            /* Copy All Data over.                                      */
            *AddedEntry                      = *EntryToAdd;

            /* Now Add it to the end of the list.                       */
            AddedEntry->NextOPPMEntryInfoPtr = NULL;

            /* First, let's check to see if there are any elements      */
            /* already present in the List that was passed in.          */
            if((tmpEntry = *ListHead) != NULL)
            {
               /* Head Pointer was not NULL, so we will traverse the    */
               /* list until we reach the last element.                 */
               while(tmpEntry)
               {
                  if(tmpEntry->TrackingID == AddedEntry->TrackingID)
                  {
                     /* Entry was already added, so free the memory and */
                     /* flag an error to the caller.                    */
                     FreeOPPMEntryInfoEntryMemory(AddedEntry);
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
                     if(tmpEntry->NextOPPMEntryInfoPtr)
                        tmpEntry = tmpEntry->NextOPPMEntryInfoPtr;
                     else
                        break;
                  }
               }

               if(AddedEntry)
               {
                  /* Last element found, simply Add the entry.          */
                  tmpEntry->NextOPPMEntryInfoPtr = AddedEntry;
               }
            }
            else
               *ListHead = AddedEntry;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", AddedEntry));

   return(AddedEntry);
}

   /* The following function searches the specified List for the        */
   /* specified Tracking ID.  This function returns NULL if either the  */
   /* list head is invalid, the Tracking ID is invalid, or the specified*/
   /* Tracking ID was NOT found.                                        */
static OPPM_Entry_Info_t *SearchOPPMEntryInfoByTrackingID(OPPM_Entry_Info_t **ListHead, unsigned int TrackingID)
{
   OPPM_Entry_Info_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", TrackingID));

   /* Let's make sure the list and Tracking ID to search for appear to  */
   /* be valid.                                                         */
   if((ListHead) && (TrackingID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->TrackingID != TrackingID))
         FoundEntry = FoundEntry->NextOPPMEntryInfoPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* The following function searches the specified List for the        */
   /* specified OPPTracking ID.  This function returns NULL if either   */
   /* the list head is invalid, the OPPTracking ID is invalid, or the   */
   /* specified OPPTracking ID was NOT found.                           */
static OPPM_Entry_Info_t *SearchOPPMEntryInfoByOPPTrackingID(OPPM_Entry_Info_t **ListHead, unsigned int OPPTrackingID)
{
   OPPM_Entry_Info_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", OPPTrackingID));

   /* Let's make sure the list and OPPTracking ID to search for appear  */
   /* to be valid.                                                      */
   if((ListHead) && (OPPTrackingID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->OPPTrackingID != OPPTrackingID))
         FoundEntry = FoundEntry->NextOPPMEntryInfoPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* The following function searches the specified List for the        */
   /* specified connection.  This function returns NULL if either the   */
   /* list head is invalid, the Remote Device is invalid, the Instance  */
   /* ID is invalid, or the specified entry was NOT found.              */
static OPPM_Entry_Info_t *SearchOPPMEntryInfoByConnection(OPPM_Entry_Info_t **ListHead, BD_ADDR_t RemoteDeviceAddress, Boolean_t Server)
{
   unsigned long      ServerFlags;
   OPPM_Entry_Info_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter"));

   /* Check if ListHead is not NULL and the remote address appears to be*/
   /* semi-valid.                                                       */
   if((ListHead) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
   {
      /* Note the Server Flags to test against when searching.          */
      ServerFlags = Server?OPPM_ENTRY_INFO_FLAGS_SERVER:0;

      /* Search the list for the specified entry.                       */
      FoundEntry  = *ListHead;

      while((FoundEntry) && ((!COMPARE_BD_ADDR(FoundEntry->RemoteDeviceAddress, RemoteDeviceAddress)) || ((FoundEntry->Flags & OPPM_ENTRY_INFO_FLAGS_SERVER) != ServerFlags)))
         FoundEntry = FoundEntry->NextOPPMEntryInfoPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   /* Return the specified entry or a NULL value.                       */
   return(FoundEntry);
}

   /* The following function searches the specified Object Push entry   */
   /* information list for the specified callback ID and removes it     */
   /* from the List.  This function returns NULL if either the Hands    */
   /* Free entry information list head is invalid, the callback ID is   */
   /* invalid, or the specified callback ID was NOT present in the list.*/
   /* The entry returned will have the next entry field set to NULL, and*/
   /* the caller is responsible for deleting the memory associated with */
   /* this entry by calling FreeOPPMEntryInfoEntryMemory().             */
static OPPM_Entry_Info_t *DeleteOPPMEntryInfoEntry(OPPM_Entry_Info_t **ListHead, unsigned int TrackingID)
{
   OPPM_Entry_Info_t *FoundEntry = NULL;
   OPPM_Entry_Info_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", TrackingID));

   /* Let's make sure the List and Tracking ID to search for appear to  */
   /* be semi-valid.                                                    */
   if((ListHead) && (TrackingID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->TrackingID != TrackingID))
      {
         LastEntry  = FoundEntry;
         FoundEntry = FoundEntry->NextOPPMEntryInfoPtr;
      }

      /* Check to see if we found the specified entry.                  */
      if(FoundEntry)
      {
         /* OK, now let's remove the entry from the list.  We have to   */
         /* check to see if the entry was the first entry in the list.  */
         if(LastEntry)
         {
            /* Entry was NOT the first entry in the list.               */
            LastEntry->NextOPPMEntryInfoPtr = FoundEntry->NextOPPMEntryInfoPtr;
         }
         else
            *ListHead = FoundEntry->NextOPPMEntryInfoPtr;

         FoundEntry->NextOPPMEntryInfoPtr = NULL;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* This function frees the specified Hands Free entry information    */
   /* member.  No check is done on this entry other than making sure it */
   /* NOT NULL.                                                         */
static void FreeOPPMEntryInfoEntryMemory(OPPM_Entry_Info_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToFree)
      BTPS_FreeMemory(EntryToFree);

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified Hands Free entry information list.  Upon */
   /* return of this function, the head pointer is set to NULL.         */
static void FreeOPPMEntryInfoList(OPPM_Entry_Info_t **ListHead)
{
   OPPM_Entry_Info_t *EntryToFree;
   OPPM_Entry_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ListHead)
   {
      /* Simply traverse the list and free every element present.       */
      EntryToFree = *ListHead;

      while(EntryToFree)
      {
         tmpEntry    = EntryToFree;
         EntryToFree = EntryToFree->NextOPPMEntryInfoPtr;

         if(tmpEntry->ConnectionEvent)
            BTPS_CloseEvent(tmpEntry->ConnectionEvent);

         FreeOPPMEntryInfoEntryMemory(tmpEntry);
      }

      /* Make sure the List appears to be empty.                        */
      *ListHead = NULL;
   }

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the          */
   /* Connection Request asynchronous message.                          */
static void ProcessConnectionRequestEvent(OPPM_Connection_Request_Message_t *Message)
{
   void                  *CallbackParameter;
   OPPM_Entry_Info_t     *OPPMEntryInfo;
   OPPM_Event_Data_t      EventData;
   OPPM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the message is valid.                                 */
   if(Message)
   {
      /* Attempt to find the server being connected.                    */
      if((OPPMEntryInfo = SearchOPPMEntryInfoByOPPTrackingID(&OPPMEntryInfoList, Message->ServerID)) != NULL)
      {
         EventData.EventType                                                        = oetIncomingConnectionRequest;
         EventData.EventLength                                                      = OPPM_INCOMING_CONNECTION_REQUEST_EVENT_DATA_SIZE;
         EventData.EventData.IncomingConnectionRequestEventData.ServerPortID        = OPPMEntryInfo->TrackingID;
         EventData.EventData.IncomingConnectionRequestEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;

         /* Note the callback info.                                     */
         EventCallback     = OPPMEntryInfo->EventCallback;
         CallbackParameter = OPPMEntryInfo->CallbackParameter;

         /* Release the lock to make the callback.                      */
         BTPS_ReleaseMutex(OPPManagerMutex);

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
         /* Ne need to release the lock.                                */
         BTPS_ReleaseMutex(OPPManagerMutex);
      }
   }
   else
   {
      /* Ne need to release the lock.                                   */
      BTPS_ReleaseMutex(OPPManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Connected*/
   /* asynchronous message.                                             */
static void ProcessConnectedEvent(OPPM_Connected_Message_t *Message)
{
   void                  *CallbackParameter;
   OPPM_Entry_Info_t     *OPPMEntryInfo;
   OPPM_Event_Data_t      EventData;
   OPPM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the message is valid.                                 */
   if(Message)
   {
      /* Attempt to find the server being connected.                    */
      if((OPPMEntryInfo = SearchOPPMEntryInfoByOPPTrackingID(&OPPMEntryInfoList, Message->ServerID)) != NULL)
      {
         EventData.EventType                                        = oetConnected;
         EventData.EventLength                                      = OPPM_CONNECTED_EVENT_DATA_SIZE;
         EventData.EventData.ConnectedEventData.ServerPortID        = OPPMEntryInfo->TrackingID;
         EventData.EventData.ConnectedEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;

         /* Note the callback info.                                     */
         EventCallback     = OPPMEntryInfo->EventCallback;
         CallbackParameter = OPPMEntryInfo->CallbackParameter;

         /* Release the lock to make the callback.                      */
         BTPS_ReleaseMutex(OPPManagerMutex);

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
         /* Ne need to release the lock.                                */
         BTPS_ReleaseMutex(OPPManagerMutex);
      }
   }
   else
   {
      /* Ne need to release the lock.                                   */
      BTPS_ReleaseMutex(OPPManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the          */
   /* Disconnected asynchronous message.                                */
static void ProcessDisconnectedEvent(OPPM_Disconnected_Message_t *Message)
{
   void                  *CallbackParameter;
   OPPM_Entry_Info_t     *OPPMEntryInfo;
   OPPM_Event_Data_t      EventData;
   OPPM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the message is valid.                                 */
   if(Message)
   {
      /* Attempt to find the server being connected.                    */
      if((OPPMEntryInfo = SearchOPPMEntryInfoByOPPTrackingID(&OPPMEntryInfoList, Message->PortID)) != NULL)
      {
         EventData.EventType                                           = oetDisconnected;
         EventData.EventLength                                         = OPPM_DISCONNECTED_EVENT_DATA_SIZE;
         EventData.EventData.DisconnectedEventData.PortID              = OPPMEntryInfo->TrackingID;
         EventData.EventData.DisconnectedEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;

         /* Note the callback info.                                     */
         EventCallback     = OPPMEntryInfo->EventCallback;
         CallbackParameter = OPPMEntryInfo->CallbackParameter;

         /* Go ahead and delete the entry if it was a client connection.*/
         if(!(OPPMEntryInfo->Flags & OPPM_ENTRY_INFO_FLAGS_SERVER))
         {
            if((OPPMEntryInfo = DeleteOPPMEntryInfoEntry(&OPPMEntryInfoList, OPPMEntryInfo->TrackingID)) != NULL)
            {
               /* Clear any allocated event.                            */
               if(OPPMEntryInfo->ConnectionEvent)
                  BTPS_CloseEvent(OPPMEntryInfo->ConnectionEvent);

               /* Delete the entry memory.                              */
               FreeOPPMEntryInfoEntryMemory(OPPMEntryInfo);
            }
         }

         /* Release the lock to make the callback.                      */
         BTPS_ReleaseMutex(OPPManagerMutex);

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
         /* Ne need to release the lock.                                */
         BTPS_ReleaseMutex(OPPManagerMutex);
      }
   }
   else
   {
      /* Ne need to release the lock.                                   */
      BTPS_ReleaseMutex(OPPManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the          */
   /* Connection Status asynchronous message.                           */
static void ProcessConnectionStatusEvent(OPPM_Connection_Status_Message_t *Message)
{
   void                  *CallbackParameter;
   OPPM_Entry_Info_t     *OPPMEntryInfo;
   OPPM_Event_Data_t      EventData;
   OPPM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the message is valid.                                 */
   if(Message)
   {
      /* Attempt to find the server being connected.                    */
      if((OPPMEntryInfo = SearchOPPMEntryInfoByOPPTrackingID(&OPPMEntryInfoList, Message->ClientID)) != NULL)
      {
         EventData.EventType                                               = oetConnectionStatus;
         EventData.EventLength                                             = OPPM_CONNECTION_STATUS_EVENT_DATA_SIZE;
         EventData.EventData.ConnectionStatusEventData.ClientPortID        = OPPMEntryInfo->TrackingID;
         EventData.EventData.ConnectionStatusEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;
         EventData.EventData.ConnectionStatusEventData.Status              = Message->Status;

         /* Note the callback info.                                     */
         EventCallback     = OPPMEntryInfo->EventCallback;
         CallbackParameter = OPPMEntryInfo->CallbackParameter;

         /* Release the lock to make the callback.                      */
         BTPS_ReleaseMutex(OPPManagerMutex);

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
         /* Ne need to release the lock.                                */
         BTPS_ReleaseMutex(OPPManagerMutex);
      }
   }
   else
   {
      /* Ne need to release the lock.                                   */
      BTPS_ReleaseMutex(OPPManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Push     */
   /* Object Request asynchronous message.                              */
static void ProcessPushObjectRequestEvent(OPPM_Push_Object_Request_Message_t *Message)
{
   void                  *CallbackParameter;
   OPPM_Entry_Info_t     *OPPMEntryInfo;
   OPPM_Event_Data_t      EventData;
   OPPM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the message is valid.                                 */
   if(Message)
   {
      /* Attempt to find the server being connected.                    */
      if((OPPMEntryInfo = SearchOPPMEntryInfoByOPPTrackingID(&OPPMEntryInfoList, Message->ServerID)) != NULL)
      {
         EventData.EventType                                                = oetPushObjectRequest;
         EventData.EventLength                                              = OPPM_PUSH_OBJECT_REQUEST_EVENT_DATA_SIZE;
         EventData.EventData.PushObjectRequestEventData.ServerPortID        = OPPMEntryInfo->TrackingID;
         EventData.EventData.PushObjectRequestEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;
         EventData.EventData.PushObjectRequestEventData.ObjectType          = Message->ObjectType;
         EventData.EventData.PushObjectRequestEventData.ObjectName          = (Message->ObjectNameLength)?(char *)Message->VariableData:NULL;
         EventData.EventData.PushObjectRequestEventData.ObjectTotalLength   = Message->ObjectTotalLength;
         EventData.EventData.PushObjectRequestEventData.Final               = Message->Final;
         EventData.EventData.PushObjectRequestEventData.DataLength          = Message->DataLength;
         EventData.EventData.PushObjectRequestEventData.DataBuffer          = (Message->DataLength)?&(Message->VariableData[Message->ObjectNameLength]):NULL;

         /* Note the callback info.                                     */
         EventCallback     = OPPMEntryInfo->EventCallback;
         CallbackParameter = OPPMEntryInfo->CallbackParameter;

         /* Release the lock to make the callback.                      */
         BTPS_ReleaseMutex(OPPManagerMutex);

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
         /* Ne need to release the lock.                                */
         BTPS_ReleaseMutex(OPPManagerMutex);
      }
   }
   else
   {
      /* Ne need to release the lock.                                   */
      BTPS_ReleaseMutex(OPPManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Push     */
   /* Object Response asynchronous message.                             */
static void ProcessPushObjectResponseEvent(OPPM_Push_Object_Response_Message_t *Message)
{

   void                  *CallbackParameter;
   OPPM_Entry_Info_t     *OPPMEntryInfo;
   OPPM_Event_Data_t      EventData;
   OPPM_Event_Callback_t  EventCallback;
   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the message is valid.                                 */
   if(Message)
   {
      /* Attempt to find the server being connected.                    */
      if((OPPMEntryInfo = SearchOPPMEntryInfoByOPPTrackingID(&OPPMEntryInfoList, Message->ClientID)) != NULL)
      {
         EventData.EventType                                                 = oetPushObjectResponse;
         EventData.EventLength                                               = OPPM_PUSH_OBJECT_RESPONSE_EVENT_DATA_SIZE;
         EventData.EventData.PushObjectResponseEventData.ClientPortID        = OPPMEntryInfo->TrackingID;
         EventData.EventData.PushObjectResponseEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;
         EventData.EventData.PushObjectResponseEventData.ResponseCode        = Message->ResponseCode;

         /* Note the callback info.                                     */
         EventCallback     = OPPMEntryInfo->EventCallback;
         CallbackParameter = OPPMEntryInfo->CallbackParameter;

         /* Release the lock to make the callback.                      */
         BTPS_ReleaseMutex(OPPManagerMutex);

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
         /* Ne need to release the lock.                                */
         BTPS_ReleaseMutex(OPPManagerMutex);
      }
   }
   else
   {
      /* Ne need to release the lock.                                   */
      BTPS_ReleaseMutex(OPPManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Pull     */
   /* Business Card Request asynchronous message.                       */
static void ProcessPullBusinessCardRequestEvent(OPPM_Pull_Business_Card_Request_Message_t *Message)
{
   void                  *CallbackParameter;
   OPPM_Entry_Info_t     *OPPMEntryInfo;
   OPPM_Event_Data_t      EventData;
   OPPM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the message is valid.                                 */
   if(Message)
   {
      /* Attempt to find the server being connected.                    */
      if((OPPMEntryInfo = SearchOPPMEntryInfoByOPPTrackingID(&OPPMEntryInfoList, Message->ServerID)) != NULL)
      {
         EventData.EventType                                                      = oetPullBusinessCardRequest;
         EventData.EventLength                                                    = OPPM_PULL_BUSINESS_CARD_REQUEST_EVENT_DATA_SIZE;
         EventData.EventData.PullBusinessCardRequestEventData.ServerPortID        = OPPMEntryInfo->TrackingID;
         EventData.EventData.PullBusinessCardRequestEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;

         /* Note the callback info.                                     */
         EventCallback     = OPPMEntryInfo->EventCallback;
         CallbackParameter = OPPMEntryInfo->CallbackParameter;

         /* Release the lock to make the callback.                      */
         BTPS_ReleaseMutex(OPPManagerMutex);

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
         /* Ne need to release the lock.                                */
         BTPS_ReleaseMutex(OPPManagerMutex);
      }
   }
   else
   {
      /* Ne need to release the lock.                                   */
      BTPS_ReleaseMutex(OPPManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Pull     */
   /* Business Card Response asynchronous message.                      */
static void ProcessPullBusinessCardResponseEvent(OPPM_Pull_Business_Card_Response_Message_t *Message)
{
   void                  *CallbackParameter;
   OPPM_Entry_Info_t     *OPPMEntryInfo;
   OPPM_Event_Data_t      EventData;
   OPPM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the message is valid.                                 */
   if(Message)
   {
      /* Attempt to find the server being connected.                    */
      if((OPPMEntryInfo = SearchOPPMEntryInfoByOPPTrackingID(&OPPMEntryInfoList, Message->ClientID)) != NULL)
      {
         EventData.EventType                                                       = oetPullBusinessCardResponse;
         EventData.EventLength                                                     = OPPM_PULL_BUSINESS_CARD_RESPONSE_EVENT_DATA_SIZE;
         EventData.EventData.PullBusinessCardResponseEventData.ClientPortID        = OPPMEntryInfo->TrackingID;
         EventData.EventData.PullBusinessCardResponseEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;
         EventData.EventData.PullBusinessCardResponseEventData.ResponseCode        = Message->ResponseCode;
         EventData.EventData.PullBusinessCardResponseEventData.ObjectTotalLength   = Message->ObjectTotalLength;
         EventData.EventData.PullBusinessCardResponseEventData.Final               = Message->Final;
         EventData.EventData.PullBusinessCardResponseEventData.DataLength          = Message->DataLength;
         EventData.EventData.PullBusinessCardResponseEventData.DataBuffer          = (Message->DataLength)?Message->DataBuffer:NULL;

         /* Note the callback info.                                     */
         EventCallback     = OPPMEntryInfo->EventCallback;
         CallbackParameter = OPPMEntryInfo->CallbackParameter;

         /* Release the lock to make the callback.                      */
         BTPS_ReleaseMutex(OPPManagerMutex);

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
         /* Ne need to release the lock.                                */
         BTPS_ReleaseMutex(OPPManagerMutex);
      }
   }
   else
   {
      /* Ne need to release the lock.                                   */
      BTPS_ReleaseMutex(OPPManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is called from the Main Bluetopia Dispatch */
   /* thread in response to a Process Received Message.  This function  */
   /* SHOULD ONLY be called in this capacity and NOT under any other    */
   /* circumstances.                                                    */
   /* * NOTE * This function *MUST* be called with the OPP Manager Mutex*/
   /*          held.  This function will release the Mutex before it    */
   /*          exits (i.e. the caller SHOULD NOT RELEASE THE MUTEX).    */
static void ProcessReceivedMessage(BTPM_Message_t *Message)
{
   Boolean_t ReleaseMutex = TRUE;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", Message->MessageHeader.MessageFunction));

   /* Check to make sure this isn't some kind of spoofed response (we do*/
   /* not process responses).                                           */
   if((Message) && (!(Message->MessageHeader.MessageID & BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK)))
   {
      /* Request received, go ahead and process it.                     */
      switch(Message->MessageHeader.MessageFunction)
      {
         case OPPM_MESSAGE_FUNCTION_CONNECTION_REQUEST:
            DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Connection Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= OPPM_CONNECTION_REQUEST_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Incoming Connection Request Event.                    */
               ProcessConnectionRequestEvent((OPPM_Connection_Request_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case OPPM_MESSAGE_FUNCTION_CONNECTED:
            DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Connected Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= OPPM_CONNECTED_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Connected Event.                                      */
               ProcessConnectedEvent((OPPM_Connected_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case OPPM_MESSAGE_FUNCTION_DISCONNECTED:
            DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Disconnected Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= OPPM_DISCONNECTED_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Disconnected Event.                                   */
               ProcessDisconnectedEvent((OPPM_Disconnected_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case OPPM_MESSAGE_FUNCTION_CONNECTION_STATUS:
            DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Connection Status Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= OPPM_CONNECTION_STATUS_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Connection Status Event.                              */
               ProcessConnectionStatusEvent((OPPM_Connection_Status_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case OPPM_MESSAGE_FUNCTION_PUSH_OBJECT_REQUEST:
            DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Push Object Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= OPPM_PUSH_OBJECT_REQUEST_MESSAGE_SIZE(((OPPM_Push_Object_Request_Message_t *)Message)->ObjectNameLength, ((OPPM_Push_Object_Request_Message_t *)Message)->DataLength))
            {
               /* Size seems to be valid, go ahead and dispatch the Push*/
               /* Object Request Event.                                 */
               ProcessPushObjectRequestEvent((OPPM_Push_Object_Request_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case OPPM_MESSAGE_FUNCTION_PUSH_OBJECT_RESPONSE:
            DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Push Object Response Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= OPPM_PUSH_OBJECT_RESPONSE_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the Push*/
               /* Object Response Event.                                */
               ProcessPushObjectResponseEvent((OPPM_Push_Object_Response_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case OPPM_MESSAGE_FUNCTION_PULL_BUSINESS_CARD_REQUEST:
            DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Pull Business Card Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= OPPM_PULL_BUSINESS_CARD_REQUEST_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the Pull*/
               /* Business Card Request Event.                          */
               ProcessPullBusinessCardRequestEvent((OPPM_Pull_Business_Card_Request_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case OPPM_MESSAGE_FUNCTION_PULL_BUSINESS_CARD_RESPONSE:
            DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Pull Business Card Response Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= OPPM_PULL_BUSINESS_CARD_RESPONSE_MESSAGE_SIZE(((OPPM_Pull_Business_Card_Response_Message_t *)Message)->DataLength))
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Pull Business Card Response Event.                    */
               ProcessPullBusinessCardResponseEvent((OPPM_Pull_Business_Card_Response_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         default:
            DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled Message\n"));
            break;
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid or Response Message Received\n"));
   }

   /* If we need to Release the Mutex then we will do it at this time.  */
   if(ReleaseMutex)
      BTPS_ReleaseMutex(OPPManagerMutex);

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process OPP Manager Asynchronous Events.            */
static void BTPSAPI BTPMDispatchCallback_OPPM(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's determine if the specified parameter appears to be          */
   /* semi-valid.                                                       */
   if(CallbackParameter)
   {
      /* Next, let's check to see if we need to process it.             */
      if(Initialized)
      {
         /* Attempt to wait for access to the OPP state information.    */
         if(BTPS_WaitMutex(OPPManagerMutex, BTPS_INFINITE_WAIT))
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

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process Server un-register asynchronous events.     */
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter)
{
   OPPM_Entry_Info_t *OPPEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's determine if the specified parameter appears to be          */
   /* semi-valid.                                                       */
   if(CallbackParameter)
   {
      /* Next, let's check to see if we need to process it.             */
      if(Initialized)
      {
         /* Attempt to wait for access to the OPP state information.    */
         if(BTPS_WaitMutex(OPPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Make sure we cancel any synchronous connections.         */
            OPPEntryInfo = OPPMEntryInfoList;

            while(OPPEntryInfo)
            {
               /* Check to see if there is a synchronous open operation.*/
               if((OPPEntryInfo->Flags & OPPM_ENTRY_INFO_FLAGS_CONNECTION_OPENING) && (OPPEntryInfo->ConnectionEvent))
               {
                  OPPEntryInfo->ConnectionStatus = OPPM_CONNECTION_STATUS_FAILURE_DEVICE_POWER_OFF;

                  BTPS_SetEvent(OPPEntryInfo->ConnectionEvent);
               }

               OPPEntryInfo = OPPEntryInfo->NextOPPMEntryInfoPtr;
            }

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(OPPManagerMutex);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function represents the Message Group Handler       */
   /* function that is installed to process all OPP Manager Messages.   */
static void BTPSAPI OPPManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the Message specified appears to be semi-valid.   */
   if(Message)
   {
      /* Next, check to see if this message is specified for our        */
      /* specific Message Group.                                        */
      if(Message->MessageHeader.MessageGroup == BTPM_MESSAGE_GROUP_OBJECT_PUSH_MANAGER)
      {
         DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("OPP Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

         /* Message is destined for us, make sure we are still          */
         /* processing messages.                                        */
         if(Initialized)
         {
            /* Check to see if this message is a OPP Manager defined    */
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
               /* OPP Manager Thread.                                   */
               if(!BTPM_QueueMailboxCallback(BTPMDispatchCallback_OPPM, (void *)Message))
               {
                  DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue OPP Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

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
                        DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue OPP Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));
                     }
                  }
               }
            }
         }
         else
         {
            /* Check to see if this message is an OPP Manager defined   */
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
         DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Non OPP Manager Message: 0x%08X, 0x%08X\n", Message->MessageHeader.MessageGroup, Message->MessageHeader.MessageFunction));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for initializing/cleaning up*/
   /* the Bluetopia Platform Manager OPP Manager Module.  This function */
   /* should be registered with the Bluetopia Platform Manager Module   */
   /* Handler and will be called when the Platform Manager is           */
   /* initialized (or shut down).                                       */
void BTPSAPI OPPM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData)
{
   int Result;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %d\n", Initialize));

   /* First, determine if the request is to initialize or shut down.    */
   if(Initialize)
   {
      DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Initialization Request\n"));

      if(!Initialized)
      {
         DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing OPP Manager\n"));

         /* First, let's create a Mutex to guard access to this module. */
         if((OPPManagerMutex = BTPS_CreateMutex(FALSE)) != NULL)
         {
            /* Next, let's attempt to register our Message Group Handler*/
            /* to process OPP Manager messages.                         */
            if(!MSG_RegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_OBJECT_PUSH_MANAGER, OPPManagerGroupHandler, NULL))
            {
               /* Initialize the actual OPP Manager Implementation      */
               /* Module (this is the module that is actually           */
               /* responsible for actually implementing the OPP Manager */
               /* functionality - this module is just the framework     */
               /* shell).                                               */
               if(!(Result = _OPPM_Initialize()))
               {
                  /* Note the current Power State.                      */
                  CurrentPowerState = (Boolean_t)((DEVM_QueryDevicePowerState() > 0)?TRUE:FALSE);

                  /* Go ahead and flag that this module is initialized. */
                  Initialized       = TRUE;
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
            if(OPPManagerMutex)
               BTPS_CloseMutex(OPPManagerMutex);

            MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_OBJECT_PUSH_MANAGER);

            /* Flag that none of the resources are allocated.           */
            OPPManagerMutex = NULL;
         }
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("OPP Manager Already initialized\n"));
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Shutdown Request\n"));

      if(Initialized)
      {
         /* First, let's make sure that we un-register our Message Group*/
         /* Handler (so that we do not process any more incoming        */
         /* messages).                                                  */
         MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_OBJECT_PUSH_MANAGER);

         /* Wait for access to the Mutex that guards access to this     */
         /* module.                                                     */
         if(BTPS_WaitMutex(OPPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Make sure we inform the OPP Manager Implementation that  */
            /* we are shutting down.                                    */
            _OPPM_Cleanup();

            BTPS_CloseMutex(OPPManagerMutex);

            /* Make sure that the OPP Entry Information List is empty.  */
            FreeOPPMEntryInfoList(&OPPMEntryInfoList);

            /* Flag that the resources are no longer allocated.         */
            OPPManagerMutex   = NULL;
            CurrentPowerState = FALSE;

            /* Flag that this module is no longer initialized.          */
            Initialized       = FALSE;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing asynchronous */
   /* Device Manager (DEVM) Events (including Power On/Off events).     */
   /* This function should be registered with the Bluetopia Platform    */
   /* Manager Module Handler and will be called when an asynchronous    */
   /* Device Manager event is dispatched.                               */
void BTPSAPI OPPM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData)
{
   OPPM_Entry_Info_t *OPPMEntryInfo;
   OPPM_Entry_Info_t *tmpOPPMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the OPP Manager has been initialized.   */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(BTPS_WaitMutex(OPPManagerMutex, BTPS_INFINITE_WAIT))
      {
         switch(EventData->EventType)
         {
            case detDevicePoweredOn:
               DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power On Occurred.\n"));

               /* Note the new Power State.                             */
               CurrentPowerState = TRUE;
               break;
            case detDevicePoweringOff:
            case detDevicePoweredOff:
               DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power Off Occurred.\n"));

               /* Power off event, let's loop through ALL the registered*/
               /* OPP Entries and set any events that have synchronous  */
               /* operations pending.                                   */
               OPPMEntryInfo = OPPMEntryInfoList;

               while(OPPMEntryInfo)
               {
                  /* Check to see if there is a synchronous open        */
                  /* operation.                                         */
                  if((OPPMEntryInfo->Flags & OPPM_ENTRY_INFO_FLAGS_CONNECTION_OPENING) && (OPPMEntryInfo->ConnectionEvent))
                  {
                     OPPMEntryInfo->ConnectionStatus = OPPM_CONNECTION_STATUS_FAILURE_DEVICE_POWER_OFF;

                     BTPS_SetEvent(OPPMEntryInfo->ConnectionEvent);

                     OPPMEntryInfo = OPPMEntryInfo->NextOPPMEntryInfoPtr;
                  }
                  else
                  {
                     tmpOPPMEntryInfo = OPPMEntryInfo;
                     OPPMEntryInfo    = OPPMEntryInfo->NextOPPMEntryInfoPtr;

                     /* Delete the entry since it is no longer needed.  */
                     if((tmpOPPMEntryInfo = DeleteOPPMEntryInfoEntry(&OPPMEntryInfoList, tmpOPPMEntryInfo->TrackingID)) != NULL)
                        FreeOPPMEntryInfoEntryMemory(tmpOPPMEntryInfo);
                  }
               }

               /* Note the new Power State.                             */
               CurrentPowerState   = FALSE;
               break;
            default:
               /* Do nothing.                                           */
               break;
         }

         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(OPPManagerMutex);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}


   /* The following function is provided to allow a mechanism for local */
   /* module to accept or reject (authorize) an incoming OPP connection.*/
   /* This function returns zero if successful, or a negative return    */
   /* error code if there was an error.                                 */
   /* * NOTE * If the connection is accepted, it does NOT mean that the */
   /*          connection is successfully connected.  A OPP Connected   */
   /*          event will be dispatched to signify the actual result.   */
int BTPSAPI OPPM_Connection_Request_Response(unsigned int ServerID, Boolean_t Accept)
{
   int                ret_val;
   OPPM_Entry_Info_t *OPPMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the OPP Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, check to make sure the parameters are semi-valid.        */
      if(ServerID)
      {
         /* Wait for access to the OPP State Information.               */
         if(BTPS_WaitMutex(OPPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Make sure entry exits.                                   */
            if((OPPMEntryInfo = SearchOPPMEntryInfoByTrackingID(&OPPMEntryInfoList, ServerID)) != NULL)
            {
               ret_val = _OPPM_Connection_Request_Response(OPPMEntryInfo->OPPTrackingID, Accept);
            }
            else
               ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_INVALID_SERVER_ID;

            BTPS_ReleaseMutex(OPPManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is provided to allows a mechanism for local*/
   /* modules to register an Object Push Server. This first parameter   */
   /* is the RFCOMM server port. If this parameter is zero, the server  */
   /* will be opened on an available port. The SupportedObjectTypes     */
   /* parameter is a bitmask representing the types of objects supported*/
   /* by this server. The IncomingConnectionFlags parameter is a        */
   /* bitmask which indicates whether incoming connections should       */
   /* be authorized, autenticated, or encrypted. The ServiceName        */
   /* parameter is a null-terminate string represting the name of the   */
   /* service to be placed in the Service Record. The EventCallback     */
   /* is the function which will receive events related to this         */
   /* server. The CallbackParameter will be included in each call to    */
   /* the CallbackFunction. This function returns a positive integer    */
   /* representing the ServerID of the created server if successful and */
   /* a negative error code if there was an error.                      */
int BTPSAPI OPPM_Register_Server(unsigned int ServerPort, unsigned long SupportedObjectTypes, unsigned long IncomingConnectionFlags, char *ServiceName, OPPM_Event_Callback_t CallbackFunction, void *CallbackParameter)
{
   int                ret_val;
   OPPM_Entry_Info_t  OPPMEntryInfo;
   OPPM_Entry_Info_t *OPPMEntryInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %08lX\n", SupportedObjectTypes));

   /* First, check to make sure the OPP Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, check to make sure the parameters are semi-valid.        */
      if((SupportedObjectTypes) && (CallbackFunction))
      {
         /* Make sure the local device is powered up.                   */
         if(CurrentPowerState)
         {
            /* Wait for access to the OPP State Information.            */
            if(BTPS_WaitMutex(OPPManagerMutex, BTPS_INFINITE_WAIT))
            {
               /* Initialize the list entry.                            */
               BTPS_MemInitialize(&OPPMEntryInfo, 0, sizeof(OPPM_Entry_Info_t));

               OPPMEntryInfo.TrackingID        = GetNextTrackingID();
               OPPMEntryInfo.EventCallback     = CallbackFunction;
               OPPMEntryInfo.CallbackParameter = CallbackParameter;
               OPPMEntryInfo.Flags             = OPPM_ENTRY_INFO_FLAGS_SERVER;

               /* Attempt to add the entry.                             */
               if((OPPMEntryInfoPtr = AddOPPMEntryInfoEntry(&OPPMEntryInfoList, &OPPMEntryInfo)) != NULL)
               {
                  if((ret_val = _OPPM_Register_Server(ServerPort, SupportedObjectTypes, IncomingConnectionFlags, ServiceName)) < 0)
                  {
                     /* Error opening the port. Go ahead and delete the */
                     /* entry.                                          */
                     if((OPPMEntryInfoPtr = DeleteOPPMEntryInfoEntry(&OPPMEntryInfoList, OPPMEntryInfoPtr->TrackingID)) != NULL)
                        FreeOPPMEntryInfoEntryMemory(OPPMEntryInfoPtr);
                  }
                  else
                  {
                     /* Note the returned ID.                           */
                     OPPMEntryInfoPtr->OPPTrackingID = ret_val;

                     /* Return the Tracking ID.                         */
                     ret_val = OPPMEntryInfoPtr->TrackingID;
                  }

               }
               else
                  ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;

               /* Release the mutex.                                    */
               BTPS_ReleaseMutex(OPPManagerMutex);
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
      ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is provided to allows a mechanism for      */
   /* local modules to register an Object Push Server registered by a   */
   /* successful call to OPPM_Register_Server(). This function accepts  */
   /* as a parameter the ServerID returned from a successful call to    */
   /* OPPM_Register_Server(). This function returns zero if successful  */
   /* and a negative error code if there was an error.                  */
int BTPSAPI OPPM_Un_Register_Server(unsigned int ServerID)
{
   int                ret_val;
   OPPM_Entry_Info_t *OPPMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the OPP Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, check to make sure the parameters are semi-valid.        */
      if(ServerID)
      {
         /* Wait for access to the OPP State Information.               */
         if(BTPS_WaitMutex(OPPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Make sure entry exits.                                   */
            if((OPPMEntryInfo = SearchOPPMEntryInfoByTrackingID(&OPPMEntryInfoList, ServerID)) != NULL)
            {
               /* Go ahead and un register.                             */
               ret_val = _OPPM_Un_Register_Server(OPPMEntryInfo->OPPTrackingID);

               /* Delete the entry.                                     */
               if((OPPMEntryInfo = DeleteOPPMEntryInfoEntry(&OPPMEntryInfoList, OPPMEntryInfo->TrackingID)) != NULL)
                  FreeOPPMEntryInfoEntryMemory(OPPMEntryInfo);
            }
            else
               ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_INVALID_SERVER_ID;

            BTPS_ReleaseMutex(OPPManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to query the details of OPP services offered by a remote  */
   /* Object Push Server device. This function accepts the remote device*/
   /* address of the device whose SDP records will be parsed and the    */
   /* buffer which will hold the parsed service details. This function  */
   /* returns zero if successful or a negative return error code if     */
   /* there was an error.                                               */
   /* * NOTE * This function operates on the locally cached copy of the */
   /*          remote device's Service Records and will return an error */
   /*          if the cache is empty. For information on updating the   */
   /*          local cache, see DEVM_QueryRemoteDeviceServices().       */
   /* * NOTE * When this function is successful, the provided buffer    */
   /*          will be populated with the parsed service                */
   /*          details. This buffer MUST be passed to                   */
   /*          OPPM_Free_Object_Push_Service_Info() in order to release */
   /*          any resources that were allocated during the query       */
   /*          process.                                                 */
int BTPSAPI OPPM_Parse_Remote_Object_Push_Services(BD_ADDR_t RemoteDeviceAddress, OPPM_Parsed_Service_Info_t *ServiceInfo)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Object Push Manager has been        */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, verfiy that the input parameters appear to be semi-valid */
      if(!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress) && (ServiceInfo))
      {
         /* Next check to make sure we are powered up.                  */
         if(CurrentPowerState)
         {
            /* Simply call the helper function to handle this request.  */
            ret_val = ParseObjectPushServicesFromSDP(RemoteDeviceAddress, ServiceInfo);
         }
         else
            ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for       */
   /* local modules to free all resources that were allocated to        */
   /* query the service details of a Object Push Server device. See     */
   /* the OPPM_Query_Remote_Object_Push_Services() function for more    */
   /* information.                                                      */
void BTPSAPI OPPM_Free_Parsed_Object_Push_Service_Info(OPPM_Parsed_Service_Info_t *ServiceInfo)
{
   unsigned int Index;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Next, verfiy that the input parameters appear to be semi-valid.   */
   if((ServiceInfo) && (ServiceInfo->ServiceDetails))
   {
      BTPS_FreeMemory((void *)(ServiceInfo->ServiceDetails));

      if(ServiceInfo->RESERVED)
         BTPS_FreeMemory(ServiceInfo->RESERVED);

      BTPS_MemInitialize(ServiceInfo, 0, sizeof(OPPM_Parsed_Service_Info_t));
   }

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to connect to a remote Object Push Server device.  The    */
   /* RemoteDeviceAddress and RemoteServerPort parameter specify the    */
   /* connection information for the remote server.  The ConnectionFlags*/
   /* parameter specifies whether authentication or encryption should   */
   /* be used to create this connection.  The CallbackFunction is the   */
   /* function that will be registered to receive events for this       */
   /* connection.  The CallbackParameter is a parameter which will be   */
   /* included in the status callback.  This function returns zero if   */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * The final parameter to this function, if specified,      */
   /*          instructs this function to block until the connection    */
   /*          status is received (i.e.  the connection is completed).  */
   /*          If this parameter is not specified (i.e. NULL) then the  */
   /*          connection status will be returned asynchronously in the */
   /*          Message Access Manager Event Callback supplied.          */
int BTPSAPI OPPM_Connect_Remote_Device(BD_ADDR_t RemoteDeviceAddress, unsigned int RemoteServerPort, unsigned long ConnectionFlags, OPPM_Event_Callback_t CallbackFunction, void *CallbackParameter, unsigned int *ConnectionStatus)
{
   int               ret_val;
   Event_t           ConnectionEvent;
   OPPM_Entry_Info_t  OPPMEntryInfo;
   OPPM_Entry_Info_t *OPPMEntryInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the OPP Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, verify that the input parameters appear to be semi-valid.*/
      if((!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (((CallbackFunction) && (!ConnectionStatus)) || ((!CallbackFunction) && (ConnectionStatus))))
      {
         /* Attempt to wait for access to the OPP state information.    */
         if(BTPS_WaitMutex(OPPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* Device is powered on, attempt to add an entry into the*/
               /* OPPM Entry Info list.                                 */
               BTPS_MemInitialize(&OPPMEntryInfo, 0, sizeof(OPPM_Entry_Info_t));

               OPPMEntryInfo.TrackingID          = GetNextTrackingID();
               OPPMEntryInfo.RemoteDeviceAddress = RemoteDeviceAddress;
               OPPMEntryInfo.ConnectionEvent     = BTPS_CreateEvent(FALSE);
               OPPMEntryInfo.EventCallback       = CallbackFunction;
               OPPMEntryInfo.CallbackParameter   = CallbackParameter;

               if(OPPMEntryInfo.ConnectionEvent)
               {
                  if((OPPMEntryInfoPtr = AddOPPMEntryInfoEntry(&OPPMEntryInfoList, &OPPMEntryInfo)) != NULL)
                  {
                     /* Next, attempt to open the remote port.          */
                     if((ret_val = _OPPM_Connect_Remote_Device(RemoteDeviceAddress, RemoteServerPort, ConnectionFlags)) > 0)
                     {
                        /* Note the returned ID.                        */
                        OPPMEntryInfoPtr->OPPTrackingID = ret_val;

                        /* Return the Tracking ID.                      */
                        ret_val = OPPMEntryInfoPtr->TrackingID;

                        /* Connection request submitted successfully.   */
                        /* If the caller has requested a blocking open, */
                        /* note that we are waiting for the result.     */
                        if(ConnectionStatus)
                           OPPMEntryInfoPtr->Flags |= OPPM_ENTRY_INFO_FLAGS_CONNECTION_OPENING;
                     }
                     else
                     {
                        /* Error opening port, go ahead and delete the  */
                        /* entry that was added.                        */
                        if((OPPMEntryInfoPtr = DeleteOPPMEntryInfoEntry(&OPPMEntryInfoList, OPPMEntryInfoPtr->TrackingID)) != NULL)
                        {
                           BTPS_CloseEvent(OPPMEntryInfoPtr->ConnectionEvent);

                           FreeOPPMEntryInfoEntryMemory(OPPMEntryInfoPtr);
                        }
                     }

                     /* Next, determine if the caller has requested a   */
                     /* blocking open.                                  */
                     if((ret_val == 0) && (ConnectionStatus) && (OPPMEntryInfoPtr))
                     {
                        /* Blocking open, go ahead and wait for the     */
                        /* event.                                       */

                        /* Note the Connection Event.                   */
                        ConnectionEvent = OPPMEntryInfoPtr->ConnectionEvent;

                        /* Release the Mutex because we are finished    */
                        /* with it.                                     */
                        BTPS_ReleaseMutex(OPPManagerMutex);

                        BTPS_WaitEvent(ConnectionEvent, BTPS_INFINITE_WAIT);

                        /* Re-acquire the Mutex.                        */
                        if(BTPS_WaitMutex(OPPManagerMutex, BTPS_INFINITE_WAIT))
                        {
                           if((OPPMEntryInfoPtr = SearchOPPMEntryInfoByConnection(&OPPMEntryInfoList, RemoteDeviceAddress, FALSE)) != NULL)
                           {
                              /* Flag that we are no longer opening the */
                              /* connection.                            */
                              OPPMEntryInfoPtr->Flags &= ~((unsigned long)OPPM_ENTRY_INFO_FLAGS_CONNECTION_OPENING);

                              *ConnectionStatus        = OPPMEntryInfoPtr->ConnectionStatus;

                              if(OPPMEntryInfoPtr->ConnectionStatus)
                              {
                                 if((OPPMEntryInfoPtr = DeleteOPPMEntryInfoEntry(&OPPMEntryInfoList, OPPMEntryInfoPtr->TrackingID)) != NULL)
                                 {
                                    BTPS_CloseEvent(OPPMEntryInfoPtr->ConnectionEvent);

                                    FreeOPPMEntryInfoEntryMemory(OPPMEntryInfoPtr);
                                 }

                                 ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_UNABLE_TO_CONNECT_TO_DEVICE;
                              }
                           }
                           else
                              ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_UNABLE_TO_CONNECT_TO_DEVICE;
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
               BTPS_ReleaseMutex(OPPManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function exists to close an active Object Push      */
   /* connection that was previously opened by a successful call to     */
   /* OPPM_Connect_Remote_Device() function or by a oetConnected        */
   /* event. This function accpets the either the ClientID or ServerID  */
   /* of the connection as a parameter. This function returns zero if   */
   /* successful, or a negative return value if there was an error.     */
int BTPSAPI OPPM_Disconnect(unsigned int PortID)
{
   int                ret_val;
   OPPM_Entry_Info_t *OPPMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the OPP Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, check to make sure the parameters are semi-valid.        */
      if(PortID)
      {
         /* Wait for access to the OPP State Information.               */
         if(BTPS_WaitMutex(OPPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Make sure entry exits.                                   */
            if((OPPMEntryInfo = SearchOPPMEntryInfoByTrackingID(&OPPMEntryInfoList, PortID)) != NULL)
            {
               ret_val = _OPPM_Disconnect(OPPMEntryInfo->OPPTrackingID);
            }
            else
               ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_DEVICE_NOT_CONNECTED;

            BTPS_ReleaseMutex(OPPManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for aborting ANY currently  */
   /* outstanding OPPM profile client request.  This function accepts as*/
   /* input the ClientID of the device specifying which connection is to*/
   /* have the Abort issued.  This function returns zero if successful, */
   /* or a negative return error code if there was an error.            */
   /* * NOTE * There can only be one outstanding OPPM request active at */
   /*          any one time.  Because of this, another OPPM request     */
   /*          cannot be issued until either the current request is     */
   /*          aborted (by calling the OPPM_Abort() function) or        */
   /*          the current request is completed (this is signified      */
   /*          by receiving a confirmation event in the OPPM event      */
   /*          callback).                                               */
int BTPSAPI OPPM_Abort(unsigned int ClientID)
{
   int                ret_val;
   OPPM_Entry_Info_t *OPPMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the OPP Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, check to make sure the parameters are semi-valid.        */
      if(ClientID)
      {
         /* Wait for access to the OPP State Information.               */
         if(BTPS_WaitMutex(OPPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Make sure entry exits.                                   */
            if((OPPMEntryInfo = SearchOPPMEntryInfoByTrackingID(&OPPMEntryInfoList, ClientID)) != NULL)
            {
               ret_val = _OPPM_Abort(OPPMEntryInfo->OPPTrackingID);
            }
            else
               ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_DEVICE_NOT_CONNECTED;

            BTPS_ReleaseMutex(OPPManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for sending an Object Push  */
   /* Request to the remote Object_Push Server.  The first parameter    */
   /* is the ClientID of the remote device connection. The ObjectType   */
   /* parameter specifies the type of object being pushed. The Object   */
   /* Name parameter is a UNICODE encoded string representing the name  */
   /* of the object to push. The DataLength and DataBuffer specify the  */
   /* length and contents of the object. This function returns zero if  */
   /* successful and a negative error code if there is an error.        */
   /* * NOTE * The Object Name is a pointer to a NULL Terminated        */
   /*          UNICODE String.                                          */
   /* * NOTE * There can only be one outstanding OPPM request active at */
   /*          any one time.  Because of this, another OPPM request     */
   /*          cannot be issued until either the current request is     */
   /*          aborted (by calling the OPPM_Abort() function) or        */
   /*          the current request is completed (this is signified      */
   /*          by receiving a confirmation event in the OPPM event      */
   /*          callback).                                               */
   /* * NOTE * If the Final parameter is TRUE, then an End-of-Body      */
   /*          header will be placed in the final packet.  If FALSE,    */
   /*          then this function must be called again with the Final   */
   /*          flag TRUE in order for the response to be completed.     */
int BTPSAPI OPPM_Push_Object_Request(unsigned int ClientID, OPPM_Object_Type_t ObjectType, char *ObjectName, unsigned long ObjectTotalLength, unsigned int DataLength, Byte_t *DataBuffer, Boolean_t Final)
{
   int                ret_val;
   OPPM_Entry_Info_t *OPPMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the OPP Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, check to make sure the parameters are semi-valid.        */
      if(ClientID)
      {
         /* Wait for access to the OPP State Information.               */
         if(BTPS_WaitMutex(OPPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Make sure entry exits.                                   */
            if((OPPMEntryInfo = SearchOPPMEntryInfoByTrackingID(&OPPMEntryInfoList, ClientID)) != NULL)
            {
               ret_val = _OPPM_Push_Object_Request(OPPMEntryInfo->OPPTrackingID, ObjectType, ObjectName, ObjectTotalLength, DataLength, DataBuffer, Final);
            }
            else
               ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_INVALID_SERVER_ID;

            BTPS_ReleaseMutex(OPPManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for sending an Object       */
   /* Push Response to the remote Client.  The first parameter is       */
   /* the ServerID of the local Object Push server. The ResponseCode    */
   /* parameter is the OBEX response code associated with this          */
   /* response. The function returns zero if successful and a negative  */
   /* error code if there is an error.                                  */
int BTPSAPI OPPM_Push_Object_Response(unsigned int ServerID, unsigned int ResponseCode)
{
   int                ret_val;
   OPPM_Entry_Info_t *OPPMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the OPP Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, check to make sure the parameters are semi-valid.        */
      if(ServerID)
      {
         /* Wait for access to the OPP State Information.               */
         if(BTPS_WaitMutex(OPPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Make sure entry exits.                                   */
            if((OPPMEntryInfo = SearchOPPMEntryInfoByTrackingID(&OPPMEntryInfoList, ServerID)) != NULL)
            {
               ret_val = _OPPM_Push_Object_Response(OPPMEntryInfo->OPPTrackingID, ResponseCode);
            }
            else
               ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_INVALID_SERVER_ID;

            BTPS_ReleaseMutex(OPPManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for sending a Pull Business */
   /* Card Request to the remote Object Push Server.  The Client        */
   /* parameter is the ClientID of the remote Object Push server        */
   /* connection. This function returns zero if successful and a        */
   /* negative error code if there was an error.                        */
   /* * NOTE * There can only be one outstanding OPPM request active at */
   /*          any one time.  Because of this, another OPPM request     */
   /*          cannot be issued until either the current request is     */
   /*          aborted (by calling the OPPM_Abort() function) or        */
   /*          the current request is completed (this is signified      */
   /*          by receiving a confirmation event in the OPPM event      */
   /*          callback).                                               */
int BTPSAPI OPPM_Pull_Business_Card_Request(unsigned int ClientID)
{
   int                ret_val;
   OPPM_Entry_Info_t *OPPMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the OPP Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, check to make sure the parameters are semi-valid.        */
      if(ClientID)
      {
         /* Wait for access to the OPP State Information.               */
         if(BTPS_WaitMutex(OPPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Make sure entry exits.                                   */
            if((OPPMEntryInfo = SearchOPPMEntryInfoByTrackingID(&OPPMEntryInfoList, ClientID)) != NULL)
            {
               ret_val = _OPPM_Pull_Business_Card_Request(OPPMEntryInfo->OPPTrackingID);
            }
            else
               ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_INVALID_SERVER_ID;

            BTPS_ReleaseMutex(OPPManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for sending a Pull Business */
   /* Card Response to the remote Client.  The first parameter is the   */
   /* ServerID of the remote Object Push client. The ResponseCode       */
   /* parameter is the OBEX response code associated with the           */
   /* response. The DataLength and DataBuffer parameters contain the    */
   /* business card data to be sent. This function returns zero if      */
   /* successful and a negative return error code if there is an error. */
   /* * NOTE * If the Final parameter is TRUE, then an End-of-Body      */
   /*          header will be placed in the final packet.  If FALSE,    */
   /*          then this function must be called again with the Final   */
   /*          flag TRUE in order for the response to be completed.     */
int BTPSAPI OPPM_Pull_Business_Card_Response(unsigned int ServerID, unsigned int ResponseCode, unsigned long ObjectTotalLength, unsigned int DataLength, Byte_t *DataBuffer, Boolean_t Final)
{
   int                ret_val;
   OPPM_Entry_Info_t *OPPMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the OPP Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, check to make sure the parameters are semi-valid.        */
      if(ServerID)
      {
         /* Wait for access to the OPP State Information.               */
         if(BTPS_WaitMutex(OPPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Make sure entry exits.                                   */
            if((OPPMEntryInfo = SearchOPPMEntryInfoByTrackingID(&OPPMEntryInfoList, ServerID)) != NULL)
            {
               ret_val = _OPPM_Pull_Business_Card_Response(OPPMEntryInfo->OPPTrackingID, ResponseCode, ObjectTotalLength, DataLength, DataBuffer, Final);
            }
            else
               ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_INVALID_SERVER_ID;

            BTPS_ReleaseMutex(OPPManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

