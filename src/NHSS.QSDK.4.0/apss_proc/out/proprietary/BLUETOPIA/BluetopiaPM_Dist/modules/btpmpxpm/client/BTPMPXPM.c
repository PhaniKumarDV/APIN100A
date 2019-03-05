/*****< btpmanpm.c >***********************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMPXPM - PXP Manager for Stonestreet One Bluetooth Protocol Stack       */
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

#include "BTPMPXPM.h"            /* BTPM PXP Manager Prototypes/Constants.    */
#include "PXPMAPI.h"             /* PXP Manager Prototypes/Constants.         */
#include "PXPMMSG.h"             /* BTPM PXP Manager Message Formats.         */
#include "PXPMGR.h"              /* PXP Manager Impl. Prototypes/Constants.   */

#include "SS1BTPM.h"             /* BTPM Main Prototypes and Constants.       */
#include "BTPMERR.h"             /* BTPM Error Prototypes/Constants.          */
#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

   /* Structure which is used to track Callback Information related to  */
   /* this module.                                                      */
typedef struct _tagPXP_Entry_Info_t
{
   unsigned int                 CallbackID;
   unsigned int                 EventHandlerID;
   BD_ADDR_t                    BD_ADDR;
   PXPM_Event_Callback_t        EventCallback;
   void                        *CallbackParameter;
   struct _tagPXP_Entry_Info_t *NextPXPEntryInfoPtr;
} PXP_Entry_Info_t;

   /* Container structure used when dispatching registered callbacks.   */
typedef struct _tagCallbackInfo_t
{
   PXPM_Event_Callback_t  EventCallback;
   void                  *CallbackParameter;
} CallbackInfo_t;

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the compiler*/
   /* as part of standard C/C++).                                       */

   /* Variable which serves as a global flag about whether this module  */
   /* is initialized or not.                                            */
static Boolean_t Initialized;

   /* Variable which is used to protect access to the state information */
   /* in this module.                                                   */
static Mutex_t PXPManagerMutex;

   /* Variable which is used to hold the next (unique) Callback ID.     */
static unsigned int NextCallbackID;

   /* Variable which holds the current Power State of the Device.       */
static Boolean_t CurrentPowerState;

   /* Variable which holds a pointer to the first element in the PXP    */
   /* Entry Information List (which holds all Callbacks tracked by this */
   /* module).                                                          */
static PXP_Entry_Info_t *PXPEntryInfoList;

   /* Internal Function Prototypes.                                     */
static unsigned int GetNextCallbackID(void);

static PXP_Entry_Info_t *AddPXPEntryInfoEntry(PXP_Entry_Info_t **ListHead, PXP_Entry_Info_t *EntryToAdd);
static PXP_Entry_Info_t *SearchPXPEntryInfoEntry(PXP_Entry_Info_t **ListHead, unsigned int CallbackID);
static PXP_Entry_Info_t *SearchPXPEntryInfoEntryByHandlerID(PXP_Entry_Info_t **ListHead, unsigned int HandlerID);
static PXP_Entry_Info_t *DeletePXPEntryInfoEntry(PXP_Entry_Info_t **ListHead, unsigned int CallbackID);
static void FreePXPEntryInfoEntryMemory(PXP_Entry_Info_t *EntryToFree);
static void FreePXPEntryInfoList(PXP_Entry_Info_t **ListHead);

static void PXPConnectedEvent(PXPM_Connected_Message_t *Message);
static void PXPDisconnectedEvent(PXPM_Disconnected_Message_t *Message);
static void PXPPathLossEvent(PXPM_Path_Loss_Alert_Message_t *Message);
static void PXPLinkLossEvent(PXPM_Link_Loss_Alert_Message_t *Message);

static void ProcessReceivedMessage(BTPM_Message_t *Message);

static void BTPSAPI BTPMDispatchCallback_PXPM(void *CallbackParameter);
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter);

static void BTPSAPI PXPManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter);

   /* The following function is a utility function that exists to       */
   /* retrieve a unique Callback ID that can be used to add an entry    */
   /* into the PXP Entry Information List.                              */
static unsigned int GetNextCallbackID(void)
{
   unsigned int ret_val;

   ret_val = NextCallbackID++;

   if((!NextCallbackID) || (NextCallbackID & 0x80000000))
      NextCallbackID = 0x00000001;

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
   /*            CallbackID field is the same as an entry already in    */
   /*            the list.  When this occurs, this function returns     */
   /*            NULL.                                                  */
static PXP_Entry_Info_t *AddPXPEntryInfoEntry(PXP_Entry_Info_t **ListHead, PXP_Entry_Info_t *EntryToAdd)
{
   PXP_Entry_Info_t *AddedEntry = NULL;
   PXP_Entry_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First let's verify that values passed in are semi-valid.          */
   if((ListHead) && (EntryToAdd))
   {
      /* Make sure that the element that we are adding seems semi-valid.*/
      if(EntryToAdd->CallbackID)
      {
         /* OK, data seems semi-valid, let's allocate a new data        */
         /* structure to add to the list.                               */
         AddedEntry = (PXP_Entry_Info_t *)BTPS_AllocateMemory(sizeof(PXP_Entry_Info_t));

         if(AddedEntry)
         {
            /* Copy All Data over.                                      */
            *AddedEntry                     = *EntryToAdd;

            /* Now Add it to the end of the list.                       */
            AddedEntry->NextPXPEntryInfoPtr = NULL;

            /* First, let's check to see if there are any elements      */
            /* already present in the List that was passed in.          */
            if((tmpEntry = *ListHead) != NULL)
            {
               /* Head Pointer was not NULL, so we will traverse the    */
               /* list until we reach the last element.                 */
               while(tmpEntry)
               {
                  if(tmpEntry->CallbackID == AddedEntry->CallbackID)
                  {
                     /* Entry was already added, so free the memory and */
                     /* flag an error to the caller.                    */
                     FreePXPEntryInfoEntryMemory(AddedEntry);
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
                     if(tmpEntry->NextPXPEntryInfoPtr)
                        tmpEntry = tmpEntry->NextPXPEntryInfoPtr;
                     else
                        break;
                  }
               }

               if(AddedEntry)
               {
                  /* Last element found, simply Add the entry.          */
                  tmpEntry->NextPXPEntryInfoPtr = AddedEntry;
               }
            }
            else
               *ListHead = AddedEntry;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", AddedEntry));

   return(AddedEntry);
}

   /* The following function searches the specified List for the        */
   /* specified Event Callback ID.  This function returns NULL if either*/
   /* the List Head is invalid, the Event Callback ID is invalid, or the*/
   /* specified Event Callback ID was NOT found.                        */
static PXP_Entry_Info_t *SearchPXPEntryInfoEntry(PXP_Entry_Info_t **ListHead, unsigned int CallbackID)
{
   PXP_Entry_Info_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", CallbackID));

   /* Let's make sure the list and Event Callback ID to search for      */
   /* appear to be valid.                                               */
   if((ListHead) && (CallbackID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->CallbackID != CallbackID))
         FoundEntry = FoundEntry->NextPXPEntryInfoPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* The following function searches the specified List for the        */
   /* specified Event Handler ID.  This function returns NULL if either */
   /* the List Head is invalid, the Event Handler ID is invalid, or the */
   /* specified Event Handler ID was NOT found.                         */
static PXP_Entry_Info_t *SearchPXPEntryInfoEntryByHandlerID(PXP_Entry_Info_t **ListHead, unsigned int HandlerID)
{
   PXP_Entry_Info_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", HandlerID));

   /* Let's make sure the list and Event Handler ID to search for appear*/
   /* to be valid.                                                      */
   if((ListHead) && (HandlerID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->EventHandlerID != HandlerID))
         FoundEntry = FoundEntry->NextPXPEntryInfoPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* The following function searches the specified PXP Entry           */
   /* Information List for the specified Callback ID and removes it from*/
   /* the List.  This function returns NULL if either the PXP Entry     */
   /* Information List Head is invalid, the Callback ID is invalid, or  */
   /* the specified Callback ID was NOT present in the list.  The entry */
   /* returned will have the Next Entry field set to NULL, and the      */
   /* caller is responsible for deleting the memory associated with this*/
   /* entry by calling FreePXPEntryInfoEntryMemory().                   */
static PXP_Entry_Info_t *DeletePXPEntryInfoEntry(PXP_Entry_Info_t **ListHead, unsigned int CallbackID)
{
   PXP_Entry_Info_t *FoundEntry = NULL;
   PXP_Entry_Info_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", CallbackID));

   /* Let's make sure the List and Callback ID to search for appear to  */
   /* be semi-valid.                                                    */
   if((ListHead) && (CallbackID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->CallbackID != CallbackID))
      {
         LastEntry  = FoundEntry;
         FoundEntry = FoundEntry->NextPXPEntryInfoPtr;
      }

      /* Check to see if we found the specified entry.                  */
      if(FoundEntry)
      {
         /* OK, now let's remove the entry from the list.  We have to   */
         /* check to see if the entry was the first entry in the list.  */
         if(LastEntry)
         {
            /* Entry was NOT the first entry in the list.               */
            LastEntry->NextPXPEntryInfoPtr = FoundEntry->NextPXPEntryInfoPtr;
         }
         else
            *ListHead = FoundEntry->NextPXPEntryInfoPtr;

         FoundEntry->NextPXPEntryInfoPtr = NULL;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* This function frees the specified PXP Entry Information member.   */
   /* No check is done on this entry other than making sure it NOT NULL.*/
static void FreePXPEntryInfoEntryMemory(PXP_Entry_Info_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToFree)
      BTPS_FreeMemory(EntryToFree);

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified PXP Entry Information List.  Upon return */
   /* of this function, the Head Pointer is set to NULL.                */
static void FreePXPEntryInfoList(PXP_Entry_Info_t **ListHead)
{
   PXP_Entry_Info_t *EntryToFree;
   PXP_Entry_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ListHead)
   {
      /* Simply traverse the list and free every element present.       */
      EntryToFree = *ListHead;

      while(EntryToFree)
      {
         tmpEntry    = EntryToFree;
         EntryToFree = EntryToFree->NextPXPEntryInfoPtr;

         FreePXPEntryInfoEntryMemory(tmpEntry);
      }

      /* Make sure the List appears to be empty.                        */
      *ListHead = NULL;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a PXP Connected Message.                                  */
static void PXPConnectedEvent(PXPM_Connected_Message_t *Message)
{
   void                  *CallbackParameter;
   PXP_Entry_Info_t      *PXPEntryInfo;
   PXPM_Event_Data_t      PXPMEventData;
   PXPM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if(Message)
   {
      /* Search for the event callback to dispatch this event to.       */
      if((PXPEntryInfo = SearchPXPEntryInfoEntryByHandlerID(&PXPEntryInfoList, Message->EventHandlerID)) != NULL)
      {
         /* Format the event to dispatch.                               */
         PXPMEventData.EventType                                         = etPXPConnected;
         PXPMEventData.EventLength                                       = PXPM_CONNECTED_EVENT_DATA_SIZE;

         PXPMEventData.EventData.ConnectedEventData.CallbackID           = PXPEntryInfo->CallbackID;
         PXPMEventData.EventData.ConnectedEventData.ConnectionType       = Message->ConnectionType;
         PXPMEventData.EventData.ConnectedEventData.RemoteDeviceAddress  = Message->RemoteDeviceAddress;
         PXPMEventData.EventData.ConnectedEventData.SupportedFeatures    = Message->SupportedFeatures;

         /* Cache the callback information locally before we release the*/
         /* PXP Manager Mutex.                                          */
         EventCallback     = PXPEntryInfo->EventCallback;
         CallbackParameter = PXPEntryInfo->CallbackParameter;

         /* Release the Mutex so we can dispatch the event.             */
         BTPS_ReleaseMutex(PXPManagerMutex);

         /* Go ahead and dispatch the event.                            */
         __BTPSTRY
         {
            if(EventCallback)
            {
               (*EventCallback)(&PXPMEventData, CallbackParameter);
            }
         }
         __BTPSEXCEPT(1)
         {
            /* Do Nothing.                                              */
         }
      }
      else
      {
         /* Error - Release the Mutex.                                  */
         BTPS_ReleaseMutex(PXPManagerMutex);

         DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to find Event Callback for Event Handler ID: %u.\n", Message->EventHandlerID));
      }
   }
   else
   {
      /* Error - Release the Mutex.                                     */
      BTPS_ReleaseMutex(PXPManagerMutex);

      DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message\n"));
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a PXP Disconnected Message.                               */
static void PXPDisconnectedEvent(PXPM_Disconnected_Message_t *Message)
{
   void                  *CallbackParameter;
   PXP_Entry_Info_t      *PXPEntryInfo;
   PXPM_Event_Data_t      PXPMEventData;
   PXPM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if(Message)
   {
      /* Search for the event callback to dispatch this event to.       */
      if((PXPEntryInfo = SearchPXPEntryInfoEntryByHandlerID(&PXPEntryInfoList, Message->EventHandlerID)) != NULL)
      {
         /* Format the event to dispatch.                               */
         PXPMEventData.EventType                                            = etPXPDisconnected;
         PXPMEventData.EventLength                                          = PXPM_DISCONNECTED_EVENT_DATA_SIZE;

         PXPMEventData.EventData.DisconnectedEventData.CallbackID           = PXPEntryInfo->CallbackID;
         PXPMEventData.EventData.DisconnectedEventData.ConnectionType       = Message->ConnectionType;
         PXPMEventData.EventData.DisconnectedEventData.RemoteDeviceAddress  = Message->RemoteDeviceAddress;

         /* Cache the callback information locally before we release the*/
         /* PXP Manager Mutex.                                          */
         EventCallback     = PXPEntryInfo->EventCallback;
         CallbackParameter = PXPEntryInfo->CallbackParameter;

         /* Release the Mutex so we can dispatch the event.             */
         BTPS_ReleaseMutex(PXPManagerMutex);

         /* Go ahead and dispatch the event.                            */
         __BTPSTRY
         {
            if(EventCallback)
            {
               (*EventCallback)(&PXPMEventData, CallbackParameter);
            }
         }
         __BTPSEXCEPT(1)
         {
            /* Do Nothing.                                              */
         }
      }
      else
      {
         /* Error - Release the Mutex.                                  */
         BTPS_ReleaseMutex(PXPManagerMutex);

         DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to find Event Callback for Event Handler ID: %u.\n", Message->EventHandlerID));
      }
   }
   else
   {
      /* Error - Release the Mutex.                                     */
      BTPS_ReleaseMutex(PXPManagerMutex);

      DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message\n"));
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a PXP Path Loss Message.                                  */
static void PXPPathLossEvent(PXPM_Path_Loss_Alert_Message_t *Message)
{
   void                  *CallbackParameter;
   PXP_Entry_Info_t      *PXPEntryInfo;
   PXPM_Event_Data_t      PXPMEventData;
   PXPM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if(Message)
   {
      /* Search for the event callback to dispatch this event to.       */
      if((PXPEntryInfo = SearchPXPEntryInfoEntryByHandlerID(&PXPEntryInfoList, Message->EventHandlerID)) != NULL)
      {
         /* Format the event to dispatch.                               */
         PXPMEventData.EventType                                             = etPXPPathLossAlert;
         PXPMEventData.EventLength                                           = PXPM_PATH_LOSS_ALERT_EVENT_DATA_SIZE;

         PXPMEventData.EventData.PathLossAlertEventData.CallbackID           = PXPEntryInfo->CallbackID;
         PXPMEventData.EventData.PathLossAlertEventData.ConnectionType       = Message->ConnectionType;
         PXPMEventData.EventData.PathLossAlertEventData.RemoteDeviceAddress  = Message->RemoteDeviceAddress;
         PXPMEventData.EventData.PathLossAlertEventData.AlertLevel           = Message->AlertLevel;

         /* Cache the callback information locally before we release the*/
         /* PXP Manager Mutex.                                          */
         EventCallback     = PXPEntryInfo->EventCallback;
         CallbackParameter = PXPEntryInfo->CallbackParameter;

         /* Release the Mutex so we can dispatch the event.             */
         BTPS_ReleaseMutex(PXPManagerMutex);

         /* Go ahead and dispatch the event.                            */
         __BTPSTRY
         {
            if(EventCallback)
            {
               (*EventCallback)(&PXPMEventData, CallbackParameter);
            }
         }
         __BTPSEXCEPT(1)
         {
            /* Do Nothing.                                              */
         }
      }
      else
      {
         /* Error - Release the Mutex.                                  */
         BTPS_ReleaseMutex(PXPManagerMutex);

         DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to find Event Callback for Event Handler ID: %u.\n", Message->EventHandlerID));
      }
   }
   else
   {
      /* Error - Release the Mutex.                                     */
      BTPS_ReleaseMutex(PXPManagerMutex);

      DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message\n"));
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a PXP Link Loss Message.                                  */
static void PXPLinkLossEvent(PXPM_Link_Loss_Alert_Message_t *Message)
{
   void                  *CallbackParameter;
   PXP_Entry_Info_t      *PXPEntryInfo;
   PXPM_Event_Data_t      PXPMEventData;
   PXPM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if(Message)
   {
      /* Search for the event callback to dispatch this event to.       */
      if((PXPEntryInfo = SearchPXPEntryInfoEntryByHandlerID(&PXPEntryInfoList, Message->EventHandlerID)) != NULL)
      {
         /* Format the event to dispatch.                               */
         PXPMEventData.EventType                                             = etPXPLinkLossAlert;
         PXPMEventData.EventLength                                           = PXPM_LINK_LOSS_ALERT_EVENT_DATA_SIZE;

         PXPMEventData.EventData.LinkLossAlertEventData.CallbackID           = PXPEntryInfo->CallbackID;
         PXPMEventData.EventData.LinkLossAlertEventData.ConnectionType       = Message->ConnectionType;
         PXPMEventData.EventData.LinkLossAlertEventData.RemoteDeviceAddress  = Message->RemoteDeviceAddress;
         PXPMEventData.EventData.LinkLossAlertEventData.AlertLevel           = Message->AlertLevel;

         /* Cache the callback information locally before we release the*/
         /* PXP Manager Mutex.                                          */
         EventCallback     = PXPEntryInfo->EventCallback;
         CallbackParameter = PXPEntryInfo->CallbackParameter;

         /* Release the Mutex so we can dispatch the event.             */
         BTPS_ReleaseMutex(PXPManagerMutex);

         /* Go ahead and dispatch the event.                            */
         __BTPSTRY
         {
            if(EventCallback)
            {
               (*EventCallback)(&PXPMEventData, CallbackParameter);
            }
         }
         __BTPSEXCEPT(1)
         {
            /* Do Nothing.                                              */
         }
      }
      else
      {
         /* Error - Release the Mutex.                                  */
         BTPS_ReleaseMutex(PXPManagerMutex);

         DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to find Event Callback for Event Handler ID: %u.\n", Message->EventHandlerID));
      }
   }
   else
   {
      /* Error - Release the Mutex.                                     */
      BTPS_ReleaseMutex(PXPManagerMutex);

      DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message\n"));
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is called from the Main Bluetopia Dispatch */
   /* thread in response to a Process Received Message.  This function  */
   /* SHOULD ONLY be called in this capacity and NOT under any other    */
   /* circumstances.                                                    */
   /* * NOTE * This function *MUST* be called with the PXP Manager Mutex*/
   /*          held.  This function will release the Mutex before it    */
   /*          exits (i.e. the caller SHOULD NOT RELEASE THE MUTEX).    */
static void ProcessReceivedMessage(BTPM_Message_t *Message)
{
   Boolean_t ReleaseMutex = TRUE;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", Message->MessageHeader.MessageFunction));

   /* Check to make sure this isn't some kind of spoofed response (we do*/
   /* not process responses).                                           */
   if((Message) && (!(Message->MessageHeader.MessageID & BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK)))
   {
      /* Request received, go ahead and process it.                     */
      switch(Message->MessageHeader.MessageFunction)
      {
         case PXPM_MESSAGE_FUNCTION_CONNECTED:
            DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("PXP Connected Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PXPM_CONNECTED_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the PXP */
               /* Connection Event.                                     */
               PXPConnectedEvent((PXPM_Connected_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case PXPM_MESSAGE_FUNCTION_DISCONNECTED:
            DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("PXP Disconnected Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PXPM_DISCONNECTED_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the PXP */
               /* Disconnection Event.                                  */
               PXPDisconnectedEvent((PXPM_Disconnected_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case PXPM_MESSAGE_FUNCTION_PATH_LOSS_ALERT:
            DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("PXP Path Loss Alert Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PXPM_PATH_LOSS_ALERT_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the PXP */
               /* Path Loss Event.                                      */
               PXPPathLossEvent((PXPM_Path_Loss_Alert_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case PXPM_MESSAGE_FUNCTION_LINK_LOSS_ALERT:
            DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("PXP Link Loss Alert Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PXPM_LINK_LOSS_ALERT_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the PXP */
               /* Link Loss Event.                                      */
               PXPLinkLossEvent((PXPM_Link_Loss_Alert_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         default:
            DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled Message\n"));
            break;
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid or Response Message Received\n"));
   }

   /* If we need to Release the Mutex then we will do it at this time.  */
   if(ReleaseMutex)
      BTPS_ReleaseMutex(PXPManagerMutex);

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process PXP Manager Asynchronous Events.            */
static void BTPSAPI BTPMDispatchCallback_PXPM(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's determine if the specified parameter appears to be          */
   /* semi-valid.                                                       */
   if(CallbackParameter)
   {
      /* Next, let's check to see if we need to process it.             */
      if(Initialized)
      {
         /* Attempt to wait for access to the PXP state information.    */
         if(BTPS_WaitMutex(PXPManagerMutex, BTPS_INFINITE_WAIT))
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

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process Server un-register asynchronous events.     */
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's determine if the specified parameter appears to be          */
   /* semi-valid.                                                       */
   if(CallbackParameter)
   {
      /* Next, let's check to see if we need to process it.             */
      if(Initialized)
      {
         /* Attempt to wait for access to the PXP state information.    */
         if(BTPS_WaitMutex(PXPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Free the Event Callback List.                            */
            FreePXPEntryInfoList(&PXPEntryInfoList);

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(PXPManagerMutex);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function represents the Message Group Handler       */
   /* function that is installed to process all PXP Manager Messages.   */
static void BTPSAPI PXPManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the Message specified appears to be semi-valid.   */
   if(Message)
   {
      /* Next, check to see if this message is specified for our        */
      /* specific Message Group.                                        */
      if(Message->MessageHeader.MessageGroup == BTPM_MESSAGE_GROUP_PROXIMITY_MANAGER)
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("PXP Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

         /* Message is destined for us, make sure we are still          */
         /* processing messages.                                        */
         if(Initialized)
         {
            /* Check to see if this message is a PXP Manager defined    */
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
               /* PXP Manager Thread.                                   */
               if(!BTPM_QueueMailboxCallback(BTPMDispatchCallback_PXPM, (void *)Message))
               {
                  DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue PXP Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

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
                        DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue PXP Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));
                     }
                  }
               }
            }
         }
         else
         {
            /* Check to see if this message is an PXP Manager defined   */
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
         DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("Non PXP Manager Message: 0x%08X, 0x%08X\n", Message->MessageHeader.MessageGroup, Message->MessageHeader.MessageFunction));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for initializing/cleaning up*/
   /* the Bluetopia Platform Manager PXP Manager Module.  This function */
   /* should be registered with the Bluetopia Platform Manager Module   */
   /* Handler and will be called when the Platform Manager is           */
   /* initialized (or shut down).                                       */
void BTPSAPI PXPM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData)
{
   int Result;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %d\n", Initialize));

   /* First, determine if the request is to initialize or shut down.    */
   if(Initialize)
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("Initialization Request\n"));

      if(!Initialized)
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing PXP Manager\n"));

         /* First, let's create a Mutex to guard access to this module. */
         if((PXPManagerMutex = BTPS_CreateMutex(FALSE)) != NULL)
         {
            /* Next, let's attempt to register our Message Group Handler*/
            /* to process PXP Manager messages.                         */
            if(!MSG_RegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_PROXIMITY_MANAGER, PXPManagerGroupHandler, NULL))
            {
               /* Initialize the actual PXP Manager Implementation      */
               /* Module (this is the module that is actually           */
               /* responsible for actually implementing the PXP Manager */
               /* functionality - this module is just the framework     */
               /* shell).                                               */
               if(!(Result = _PXPM_Initialize()))
               {
                  /* Note the current Power State.                      */
                  CurrentPowerState = (Boolean_t)((DEVM_QueryDevicePowerState() > 0)?TRUE:FALSE);

                  /* Initialize a unique starting Callback ID.          */
                  NextCallbackID      = 0x000000001;

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
            if(PXPManagerMutex)
               BTPS_CloseMutex(PXPManagerMutex);

            MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_PROXIMITY_MANAGER);

            /* Flag that none of the resources are allocated.           */
            PXPManagerMutex     = NULL;
         }
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("PXP Manager Already initialized\n"));
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("Shutdown Request\n"));

      if(Initialized)
      {
         /* First, let's make sure that we un-register our Message Group*/
         /* Handler (so that we do not process any more incoming        */
         /* messages).                                                  */
         MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_PROXIMITY_MANAGER);

         /* Wait for access to the Mutex that guards access to this     */
         /* module.                                                     */
         if(BTPS_WaitMutex(PXPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Make sure we inform the PXP Manager Implementation that  */
            /* we are shutting down.                                    */
            _PXPM_Cleanup();

            BTPS_CloseMutex(PXPManagerMutex);

            /* Make sure that the PXP Entry Information List is empty.  */
            FreePXPEntryInfoList(&PXPEntryInfoList);

            /* Flag that the resources are no longer allocated.         */
            PXPManagerMutex   = NULL;
            CurrentPowerState = FALSE;

            /* Flag that this module is no longer initialized.          */
            Initialized       = FALSE;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing asynchronous */
   /* Device Manager (DEVM) Events (including Power On/Off events).     */
   /* This function should be registered with the Bluetopia Platform    */
   /* Manager Module Handler and will be called when an asynchronous    */
   /* Device Manager event is dispatched.                               */
void BTPSAPI PXPM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the PXP Manager has been initialized.   */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(BTPS_WaitMutex(PXPManagerMutex, BTPS_INFINITE_WAIT))
      {
         switch(EventData->EventType)
         {
            case detDevicePoweredOn:
               DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power On Occurred.\n"));

               /* Note the new Power State.                             */
               CurrentPowerState = TRUE;
               break;
            case detDevicePoweringOff:
            case detDevicePoweredOff:
               DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power Off Occurred.\n"));

               /* Note the new Power State.                             */
               CurrentPowerState   = FALSE;
               break;
            default:
               /* Do nothing.                                           */
               break;
         }

         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(PXPManagerMutex);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a Monitor callback function with the Proximity*/
   /* Profile (PXP) Manager Service.  This Callback will be dispatched  */
   /* by the PXP Manager when various PXP Manager Monitor Events occur. */
   /* This function accepts the Callback Function and Callback Parameter*/
   /* (respectively) to call when a PXP Manager Monitor Event needs to  */
   /* be dispatched.  This function returns a positive (non-zero) value */
   /* if successful, or a negative return error code if there was an    */
   /* error.                                                            */
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          PXPM_Un_Register_Monitor_Event_Callback() function to    */
   /*          un-register the callback from this module.               */
   /* * NOTE * Only 1 Monitor Event Callback can be registered in the   */
   /*          system at a time.                                        */
int BTPSAPI PXPM_Register_Monitor_Event_Callback(PXPM_Event_Callback_t CallbackFunction, void *CallbackParameter)
{
   int              ret_val;
   PXP_Entry_Info_t PXPEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the PXP Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(CallbackFunction)
      {
         /* Attempt to wait for access to the PXP Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(PXPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Attempt to add an entry into the PXP Entry list.         */
            BTPS_MemInitialize(&PXPEntryInfo, 0, sizeof(PXP_Entry_Info_t));

            PXPEntryInfo.CallbackID        = GetNextCallbackID();
            PXPEntryInfo.EventCallback     = CallbackFunction;
            PXPEntryInfo.CallbackParameter = CallbackParameter;

            /* Attempt to register with the server for monitor events.  */
            if((ret_val = _PXPM_Register_Monitor_Events()) > 0)
            {
               /* Save the Monitor Event Handler ID.                    */
               PXPEntryInfo.EventHandlerID = (unsigned int)ret_val;

               /* Attempt to add the entry to the local callback list.  */
               if(AddPXPEntryInfoEntry(&PXPEntryInfoList, &PXPEntryInfo))
                  ret_val = PXPEntryInfo.CallbackID;
               else
               {
                  /* Un-Register the Event Handler with the Server.     */
                  _PXPM_Un_Register_Monitor_Events(PXPEntryInfo.EventHandlerID);

                  ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
               }
            }

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(PXPManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_PROXIMITY_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered PXP Manager Monitor Event     */
   /* Callback (registered via a successful call to the                 */
   /* PXPM_Register_Monitor_Event_Callback() function).  This function  */
   /* accepts as input the PXP Manager Event Callback ID (return value  */
   /* from PXPM_Register_Monitor_Event_Callback() function).            */
void BTPSAPI PXPM_Un_Register_Monitor_Event_Callback(unsigned int MonitorCallbackID)
{
   PXP_Entry_Info_t *PXPEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the PXP Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(MonitorCallbackID)
      {
         /* Attempt to wait for access to the PXP Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(PXPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Delete the specified callback.                           */
            if((PXPEntryInfo = DeletePXPEntryInfoEntry(&PXPEntryInfoList, MonitorCallbackID)) != NULL)
            {
               /* Un-Register the Event Handler with the Server.        */
               _PXPM_Un_Register_Monitor_Events(PXPEntryInfo->EventHandlerID);

               /* Free the memory because we are finished with it.      */
               FreePXPEntryInfoEntryMemory(PXPEntryInfo);
            }

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(PXPManagerMutex);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism of        */
   /* changing the refresh time for checking the Path Loss (the time    */
   /* between checking the path loss for a given link).  This function  */
   /* accepts as it's parameter the MonitorCallbackID that was returned */
   /* from a successful call to PXPM_Register_Monitor_Event_Callback()  */
   /* and the Refresh Time (in milliseconds).  This function returns    */
   /* zero if successful, or a negative return error code if there was  */
   /* an error.                                                         */
   /* * NOTE * Decreasing the refresh rate will increase the power      */
   /*          consumption of the local Bluetooth device as it will     */
   /*          involve reading the RSSI at a faster rate.               */
int BTPSAPI PXPM_Set_Path_Loss_Refresh_Time(unsigned int MonitorCallbackID, unsigned int RefreshTime)
{
   int               ret_val;
   PXP_Entry_Info_t *PXPEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the PXP Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(MonitorCallbackID)
      {
         /* Attempt to wait for access to the PXP Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(PXPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Search for the callback entry for the specified device.  */
            if((PXPEntryInfo = SearchPXPEntryInfoEntry(&PXPEntryInfoList, MonitorCallbackID)) != NULL)
            {
               /* Simply call the Impl.  manager to perform the work.   */
               ret_val = _PXPM_Set_Path_Loss_Refresh_Time(PXPEntryInfo->EventHandlerID, RefreshTime);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(PXPManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_PROXIMITY_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism of        */
   /* changing the Path Loss Threshold for a specified PXP Monitor      */
   /* Connection.  If the Path Loss exceeds this threshold then an event*/
   /* will be generated to inform the caller to sound an alert.  This   */
   /* function accepts as it's parameter the MonitorCallbackID that was */
   /* returned from a successful call to                                */
   /* PXPM_Register_Monitor_Event_Callback(), the BD_ADDR of the Monitor*/
   /* Connection to set the path loss for, and the Path Loss Threshold  */
   /* to set.  This function returns zero if successful, or a negative  */
   /* return error code if there was an error.                          */
   /* * NOTE * The Path Loss Threshold should be specified in units of  */
   /*          dBm.                                                     */
int BTPSAPI PXPM_Set_Path_Loss_Threshold(unsigned int MonitorCallbackID, BD_ADDR_t BD_ADDR, int PathLossThreshold)
{
   int               ret_val;
   PXP_Entry_Info_t *PXPEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the PXP Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(MonitorCallbackID)
      {
         /* Attempt to wait for access to the PXP Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(PXPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Search for the callback entry for the specified device.  */
            if((PXPEntryInfo = SearchPXPEntryInfoEntry(&PXPEntryInfoList, MonitorCallbackID)) != NULL)
            {
               /* Simply call the Impl.  manager to perform the work.   */
               ret_val = _PXPM_Set_Path_Loss_Threshold(PXPEntryInfo->EventHandlerID, BD_ADDR, PathLossThreshold);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(PXPManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_PROXIMITY_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism of        */
   /* changing the Path Loss Alert Level for a specified PXP Monitor    */
   /* Connection.  If the Path Loss exceeds this threshold then an event*/
   /* will be generated to inform the caller to sound an alert at the   */
   /* specified level.  This function accepts as it's parameter the     */
   /* MonitorCallbackID that was returned from a successful call to     */
   /* PXPM_Register_Monitor_Event_Callback(), the BD_ADDR of the Monitor*/
   /* Connection to set the path loss alert level for, and the Path Loss*/
   /* Alert Level to set.  This function returns zero if successful, or */
   /* a negative return error code if there was an error.               */
int BTPSAPI PXPM_Set_Path_Loss_Alert_Level(unsigned int MonitorCallbackID, BD_ADDR_t BD_ADDR, PXPM_Alert_Level_t AlertLevel)
{
   int               ret_val;
   PXP_Entry_Info_t *PXPEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the PXP Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(MonitorCallbackID)
      {
         /* Attempt to wait for access to the PXP Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(PXPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Search for the callback entry for the specified device.  */
            if((PXPEntryInfo = SearchPXPEntryInfoEntry(&PXPEntryInfoList, MonitorCallbackID)) != NULL)
            {
               /* Simply call the Impl.  manager to perform the work.   */
               ret_val = _PXPM_Set_Path_Loss_Alert_Level(PXPEntryInfo->EventHandlerID, BD_ADDR, AlertLevel);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(PXPManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_PROXIMITY_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism of        */
   /* querying the current Path Loss for a specified PXP Monitor        */
   /* Connection.  This function accepts as it's parameter the          */
   /* MonitorCallbackID that was returned from a successful call to     */
   /* PXPM_Register_Monitor_Event_Callback(), the BD_ADDR of the Monitor*/
   /* Connection to set the path loss for, and a pointer to a buffer to */
   /* return the current Path Loss in (if successfull).  This function  */
   /* returns zero if successful, or a negative return error code if    */
   /* there was an error.                                               */
   /* * NOTE * The Path Loss Threshold will be specified in units of    */
   /*          dBm.                                                     */
int BTPSAPI PXPM_Query_Current_Path_Loss(unsigned int MonitorCallbackID, BD_ADDR_t BD_ADDR, int *PathLossThreshold)
{
   int               ret_val;
   PXP_Entry_Info_t *PXPEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the PXP Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if((MonitorCallbackID) && (PathLossThreshold))
      {
         /* Attempt to wait for access to the PXP Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(PXPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Search for the callback entry for the specified device.  */
            if((PXPEntryInfo = SearchPXPEntryInfoEntry(&PXPEntryInfoList, MonitorCallbackID)) != NULL)
            {
               /* Simply call the Impl.  manager to perform the work.   */
               ret_val = _PXPM_Query_Current_Path_Loss(PXPEntryInfo->EventHandlerID, BD_ADDR, PathLossThreshold);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(PXPManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_PROXIMITY_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism of        */
   /* changing the Link Loss Alert Level for a specified PXP Monitor    */
   /* Connection.  If the link to the proximity device is lost then an  */
   /* event will be generated to inform the caller to sound an alert at */
   /* the specified level.  This function accepts as it's parameter the */
   /* MonitorCallbackID that was returned from a successful call to     */
   /* PXPM_Register_Monitor_Event_Callback(), the BD_ADDR of the Monitor*/
   /* Connection to set the link loss alert level for, and the Link Loss*/
   /* Alert Level to set.  This function returns zero if successful, or */
   /* a negative return error code if there was an error.               */
int BTPSAPI PXPM_Set_Link_Loss_Alert_Level(unsigned int MonitorCallbackID, BD_ADDR_t BD_ADDR, PXPM_Alert_Level_t AlertLevel)
{
   int               ret_val;
   PXP_Entry_Info_t *PXPEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the PXP Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(MonitorCallbackID)
      {
         /* Attempt to wait for access to the PXP Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(PXPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Search for the callback entry for the specified device.  */
            if((PXPEntryInfo = SearchPXPEntryInfoEntry(&PXPEntryInfoList, MonitorCallbackID)) != NULL)
            {
               /* Simply call the Impl.  manager to perform the work.   */
               ret_val = _PXPM_Set_Link_Loss_Alert_Level(PXPEntryInfo->EventHandlerID, BD_ADDR, AlertLevel);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(PXPManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_PROXIMITY_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

