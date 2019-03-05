/*****< btpmpanm.c >***********************************************************/
/*      Copyright 2011 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMPANM - PAN Manager for Stonestreet One Bluetooth Protocol Stack       */
/*             Platform Manager.                                              */
/*                                                                            */
/*  Author:  Matt Seabold                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   07/28/11  M. Seabold     Initial creation.                               */
/******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMPANM.h"            /* BTPM PAN Manager Prototypes/Constants.    */
#include "PANMAPI.h"             /* PAN Manager Prototypes/Constants.         */
#include "PANMMSG.h"             /* BTPM PAN Manager Message Formats.         */
#include "PANMGR.h"              /* PAN Manager Impl. Prototypes/Constants.   */

#include "SS1BTPM.h"             /* BTPM Main Prototypes and Constants.       */
#include "BTPMERR.h"             /* BTPM Error Prototypes/Constants.          */
#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

   /* The following constatnt is used to set a PANID to a connection    */
   /* that failed device connection before a PAN connection could be    */
   /* attempted, and thus a PANID received.                             */
#define DEVICE_CONNECTION_FAILURE_PANID                     (0xFFFFFFFF)

   /* Structure which is used to track Callback Information related to  */
   /* this module.                                                      */
typedef struct _tagPAN_Entry_Info_t
{
   unsigned int                 CallbackID;
   unsigned int                 ClientID;
   PANM_Event_Callback_t        EventCallback;
   void                        *CallbackParameter;
   struct _tagPAN_Entry_Info_t *NextPANEntryInfoPtr;
} PAN_Entry_Info_t;

   /* The following enumerated type is used to denote the various states*/
   /* that are used when tracking incoming connections.                 */
typedef enum
{
   csIdle,
   csAuthorizing,
   csAuthenticating,
   csEncrypting,
   csConnecting,
   csConnected
} Connection_State_t;

   /* Container structure used when dispatching registered callbacks.   */
typedef struct _tagCallback_Info_t
{
   unsigned int           ClientID;
   PANM_Event_Callback_t  EventCallback;
   void                  *CallbackParameter;
} Callback_Info_t;

   /* Structure which is used to track current connections.             */
typedef struct _tagConnection_Entry_t
{
   unsigned int                   PANID;
   BD_ADDR_t                      BD_ADDR;
   Connection_State_t             ConnectionState;
   unsigned int                   ConnectionStatus;
   Boolean_t                      RemoteConnection;
   Callback_Info_t                CallbackInfo;
   Event_t                        ConnectionEvent;
   PAN_Service_Type_t             LocalServiceType;
   PAN_Service_Type_t             RemoteServiceType;
   struct _tagConnection_Entry_t *NextConnectionEntryPtr;
} Connection_Entry_t;

   /* The following enumerated type is used with the DEVM_Status_t      */
   /* structure to denote actual type of the status type information.   */
typedef enum
{
   dstAuthentication,
   dstEncryption,
   dstConnection
} DEVM_Status_Type_t;

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the compiler*/
   /* as part of standard C/C++).                                       */

   /* Variable which serves as a global flag about whether this module  */
   /* is initialized or not.                                            */
static Boolean_t Initialized;

   /* Variable which is used to hold the next (unique) Callback ID.     */
static unsigned int NextCallbackID;

   /* Variable which holds a pointer to the first element in the PAN    */
   /* Entry Information List (which holds all Callbacks tracked by this */
   /* module).                                                          */
static PAN_Entry_Info_t *PANEntryInfoList;

   /* Variable which holds a pointer to the first element in the        */
   /* Connection Information list.                                      */
static Connection_Entry_t *ConnectionEntryList;

   /* Variable which holds the current Power State of the Device.       */
static Boolean_t CurrentPowerState;

   /* Variable which holds the currently configured profile roles.      */
static unsigned long ServiceTypeFlags;

   /* Variables which control incoming connection requests              */
   /* (Authorization/Authentication/Encryption).                        */
static unsigned long IncomingConnectionFlags;

static unsigned int GetNextCallbackID(void);

static PAN_Entry_Info_t *AddPANEntryInfoEntry(PAN_Entry_Info_t **ListHead, PAN_Entry_Info_t *EntryToAdd);
static PAN_Entry_Info_t *SearchPANEntryInfoEntry(PAN_Entry_Info_t **ListHead, unsigned int CallbackID);
static PAN_Entry_Info_t *DeletePANEntryInfoEntry(PAN_Entry_Info_t **ListHead, unsigned int CallbackID);
static void FreePANEntryInfoEntryMemory(PAN_Entry_Info_t *EntryToFree);
static void FreePANEntryInfoList(PAN_Entry_Info_t **ListHead);

static Connection_Entry_t *AddConnectionEntry(Connection_Entry_t **ListHead, Connection_Entry_t *EntryToAdd);
static Connection_Entry_t *SearchConnectionEntry(Connection_Entry_t **ListHead, BD_ADDR_t BD_ADDR);
static Connection_Entry_t *SearchConnectionEntryID(Connection_Entry_t **ListHead, unsigned int PANID);
static Connection_Entry_t *DeleteConnectionEntry(Connection_Entry_t **ListHead, BD_ADDR_t BD_ADDR);
static void FreeConnectionEntryMemory(Connection_Entry_t *EntryToFree);
static void FreeConnectionEntryList(Connection_Entry_t **ListHead);

static void DispatchPANMEvent(PANM_Event_Data_t *PANMEventData, BTPM_Message_t *Message);

static void ProcessConnectionRequestResponseMessage(PANM_Connection_Request_Response_Request_t *Message);
static void ProcessConnectRemoteDeviceMessage(PANM_Connect_Remote_Device_Request_t *Message);
static void ProcessCloseConnectionMessage(PANM_Close_Connection_Request_t *Message);
static void ProcessQueryConnectedDevicesMessage(PANM_Query_Connected_Devices_Request_t *Message);
static void ProcessQueryCurrentConfigurationMessage(PANM_Query_Current_Configuration_Request_t *Message);
static void ProcessChangeIncomingConnectionFlagsMessage(PANM_Change_Incoming_Connection_Flags_Request_t *Message);
static void ProcessRegisterEventsMessage(PANM_Register_Events_Request_t *Message);
static void ProcessUnRegisterEventsMessage(PANM_Un_Register_Events_Request_t *Message);

static void ProcessReceivedMessage(BTPM_Message_t *Message);
static void ProcessClientUnRegister(unsigned int ClientID);

static void ProcessOpenRequestIndicationEvent(PAN_Open_Request_Indication_Data_t *OpenRequestIndicationData);
static void ProcessOpenIndicationEvent(PAN_Open_Indication_Data_t *OpenIndicationData);
static void ProcessOpenConfirmationEvent(Boolean_t DispatchOpen, PAN_Open_Confirmation_Data_t *OpenConfirmationData);
static void ProcessCloseIndicationEvent(PAN_Close_Indication_Data_t *CloseIndicationData);

static void ProcessPANEvent(PANM_PAN_Event_Data_t *PANEventData);

static void ProcessDEVMStatusEvent(DEVM_Status_Type_t StatusType, BD_ADDR_t BD_ADDR, int Status);

static void BTPSAPI BTPMDispatchCallback_PANM(void *CallbackParameter);
static void BTPSAPI BTPMDispatchCallback_PAN(void *CallbackParameter);
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter);

static void BTPSAPI PANManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter);

   /* The following function is a utility function that exists to       */
   /* retrieve a unique Callback ID that can be used to add an entry    */
   /* into the PAN Entry Information List.                              */
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
   /* * NOTE * This function does not insert duplicate entries into     */
   /*            the list.  An element is considered a duplicate if the */
   /*            CallbackID field is the same as an entry already in    */
   /*            the list.  When this occurs, this function returns     */
   /*            NULL.                                                  */
static PAN_Entry_Info_t *AddPANEntryInfoEntry(PAN_Entry_Info_t **ListHead, PAN_Entry_Info_t *EntryToAdd)
{
   PAN_Entry_Info_t *AddedEntry = NULL;
   PAN_Entry_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First let's verify that values passed in are semi-valid.          */
   if((ListHead) && (EntryToAdd))
   {
      /* Make sure that the element that we are adding seems semi-valid.*/
      if(EntryToAdd->CallbackID)
      {
         /* OK, data seems semi-valid, let's allocate a new data        */
         /* structure to add to the list.                               */
         AddedEntry = (PAN_Entry_Info_t *)BTPS_AllocateMemory(sizeof(PAN_Entry_Info_t));

         if(AddedEntry)
         {
            /* Copy All Data over.                                      */
            *AddedEntry                     = *EntryToAdd;

            /* Now Add it to the end of the list.                       */
            AddedEntry->NextPANEntryInfoPtr = NULL;

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
                     FreePANEntryInfoEntryMemory(AddedEntry);
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
                     if(tmpEntry->NextPANEntryInfoPtr)
                        tmpEntry = tmpEntry->NextPANEntryInfoPtr;
                     else
                        break;
                  }
               }

               if(AddedEntry)
               {
                  /* Last element found, simply Add the entry.          */
                  tmpEntry->NextPANEntryInfoPtr = AddedEntry;
               }
            }
            else
               *ListHead = AddedEntry;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", AddedEntry));

   return(AddedEntry);
}

   /* The following function searches the specified PAN Entry           */
   /* Information List for the specified Callback ID.  This function    */
   /* returns NULL if either the PAN Entry Information List Head is     */
   /* invalid, the Callback ID is invalid, or the specified Callback ID */
   /* was NOT present in the list.                                      */
static PAN_Entry_Info_t *SearchPANEntryInfoEntry(PAN_Entry_Info_t **ListHead, unsigned int CallbackID)
{
   PAN_Entry_Info_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", CallbackID));

   /* Let's make sure the List and Callback ID to search for appear to  */
   /* be semi-valid.                                                    */
   if((ListHead) && (CallbackID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->CallbackID != CallbackID))
         FoundEntry = FoundEntry->NextPANEntryInfoPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* The following function searches the specified PAN Entry           */
   /* Information List for the specified Callback ID and removes it from*/
   /* the List.  This function returns NULL if either the PAN Entry     */
   /* Information List Head is invalid, the Callback ID is invalid, or  */
   /* the specified Callback ID was NOT present in the list.  The entry */
   /* returned will have the Next Entry field set to NULL, and the      */
   /* caller is responsible for deleting the memory associated with this*/
   /* entry by calling FreePANEntryInfoEntryMemory().                   */
static PAN_Entry_Info_t *DeletePANEntryInfoEntry(PAN_Entry_Info_t **ListHead, unsigned int CallbackID)
{
   PAN_Entry_Info_t *FoundEntry = NULL;
   PAN_Entry_Info_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", CallbackID));

   /* Let's make sure the List and Callback ID to search for appear to  */
   /* be semi-valid.                                                    */
   if((ListHead) && (CallbackID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->CallbackID != CallbackID))
      {
         LastEntry  = FoundEntry;
         FoundEntry = FoundEntry->NextPANEntryInfoPtr;
      }

      /* Check to see if we found the specified entry.                  */
      if(FoundEntry)
      {
         /* OK, now let's remove the entry from the list.  We have to   */
         /* check to see if the entry was the first entry in the list.  */
         if(LastEntry)
         {
            /* Entry was NOT the first entry in the list.               */
            LastEntry->NextPANEntryInfoPtr = FoundEntry->NextPANEntryInfoPtr;
         }
         else
            *ListHead = FoundEntry->NextPANEntryInfoPtr;

         FoundEntry->NextPANEntryInfoPtr = NULL;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* This function frees the specified PAN Entry Information member. No*/
   /* check is done on this entry other than making sure it NOT NULL.   */
static void FreePANEntryInfoEntryMemory(PAN_Entry_Info_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToFree)
      BTPS_FreeMemory(EntryToFree);

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified PAN Entry Information List. Upon return  */
   /* of this function, the Head Pointer is set to NULL.                */
static void FreePANEntryInfoList(PAN_Entry_Info_t **ListHead)
{
   PAN_Entry_Info_t *tmpEntry;
   PAN_Entry_Info_t *EntryToFree;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ListHead)
   {
      /* Simply traverse the list and free every element present.       */
      EntryToFree = *ListHead;

      while(EntryToFree)
      {
         tmpEntry    = EntryToFree;
         EntryToFree = EntryToFree->NextPANEntryInfoPtr;

         FreePANEntryInfoEntryMemory(tmpEntry);
      }

      /* Make sure the List appears to be empty.                        */
      *ListHead = NULL;
   }

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
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

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", AddedEntry));

   return(AddedEntry);
}

   /* The following function searches the specified Connection Entry    */
   /* List for the specified Connection Entry based on the specified    */
   /* Bluetooth Device Address.  This function returns NULL if either   */
   /* the Connection Entry List Head is invalid, the Bluetooth Device   */
   /* Address is invalid, or the specified Entry was NOT present in the */
   /* list.                                                             */
static Connection_Entry_t *SearchConnectionEntry(Connection_Entry_t **ListHead, BD_ADDR_t BD_ADDR)
{
   BD_ADDR_t           NULL_BD_ADDR;
   Connection_Entry_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format a NULL BD_ADDR to test against.                            */
   ASSIGN_BD_ADDR(NULL_BD_ADDR, 0, 0, 0, 0, 0, 0);

   /* Let's make sure the list and Bluetooth Device Address to search   */
   /* for appear to be valid.                                           */
   if((ListHead) && (!COMPARE_BD_ADDR(NULL_BD_ADDR, BD_ADDR)))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && ((!COMPARE_BD_ADDR(FoundEntry->BD_ADDR, BD_ADDR))))
         FoundEntry = FoundEntry->NextConnectionEntryPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* The following function searches the specified Connection Entry    */
   /* List for the specified Connection Entry based on the specified    */
   /* PAN. This function returns NULL if either the Connection Entry    */
   /* List Head is invalid, the PANID is invalid, or the specified Entry*/
   /* was NOT present in the list.                                      */
static Connection_Entry_t *SearchConnectionEntryID(Connection_Entry_t **ListHead, unsigned int PANID)
{
   Connection_Entry_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's make sure the list and Bluetooth Device Address to search   */
   /* for appear to be valid.                                           */
   if((ListHead) && (PANID > 0))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (PANID != FoundEntry->PANID))
         FoundEntry = FoundEntry->NextConnectionEntryPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

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
static Connection_Entry_t *DeleteConnectionEntry(Connection_Entry_t **ListHead, BD_ADDR_t BD_ADDR)
{
   BD_ADDR_t           NULL_BD_ADDR;
   Connection_Entry_t *FoundEntry = NULL;
   Connection_Entry_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format a NULL BD_ADDR to test against.                            */
   ASSIGN_BD_ADDR(NULL_BD_ADDR, 0, 0, 0, 0, 0, 0);

   /* Let's make sure the list and Bluetooth Device Address to search   */
   /* for appear to be valid.                                           */
   if((ListHead) && (!COMPARE_BD_ADDR(NULL_BD_ADDR, BD_ADDR)))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && ((!COMPARE_BD_ADDR(FoundEntry->BD_ADDR, BD_ADDR))))
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

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* This function frees the specified Connection Information member.  */
   /* No check is done on this entry other than making sure it NOT NULL.*/
static void FreeConnectionEntryMemory(Connection_Entry_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToFree)
      BTPS_FreeMemory(EntryToFree);

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified Connection Information List.  Upon return*/
   /* of this function, the Head Pointer is set to NULL.                */
static void FreeConnectionEntryList(Connection_Entry_t **ListHead)
{
   Connection_Entry_t *EntryToFree;
   Connection_Entry_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ListHead)
   {
      /* Simply traverse the list and free every element present.       */
      EntryToFree = *ListHead;

      while(EntryToFree)
      {
         tmpEntry    = EntryToFree;
         EntryToFree = EntryToFree->NextConnectionEntryPtr;

         if(tmpEntry->ConnectionEvent)
            BTPS_CloseEvent(tmpEntry->ConnectionEvent);

         FreeConnectionEntryMemory(tmpEntry);
      }

      /* Make sure the List appears to be empty.                        */
      *ListHead = NULL;
   }

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch the specified PAN event to every registered PAN Event    */
   /* Callback.                                                         */
   /* * NOTE * This function should be called with the PAN Manager      */
   /*          Lock held.  Upon exit from this function it will free    */
   /*          the PAN Manager Lock.                                    */
static void DispatchPANMEvent(PANM_Event_Data_t *PANMEventData, BTPM_Message_t *Message)
{
   unsigned int      Index;
   unsigned int      Index1;
   unsigned int      ServerID;
   unsigned int      NumberCallbacks;
   Callback_Info_t  *CallbackInfoArrayPtr;
   Callback_Info_t   CallbackInfoArray[16];
   PAN_Entry_Info_t *PANEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure that the input paramters appear to be semi-valid        */
   if((PANEntryInfoList) && (PANMEventData) && (Message))
   {
      /* Next, let's determine how many callbacks are registered.       */
      PANEntryInfo    = PANEntryInfoList;
      ServerID        = MSG_GetServerAddressID();
      NumberCallbacks = 0;

      /* First, add the default event handlers.                         */
      while(PANEntryInfo)
      {
         if((PANEntryInfo->EventCallback) || (PANEntryInfo->ClientID != ServerID))
            NumberCallbacks++;

         PANEntryInfo = PANEntryInfo->NextPANEntryInfoPtr;
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
            PANEntryInfo  = PANEntryInfoList;
            NumberCallbacks = 0;

            /* Next add the default event handlers.                     */
            while(PANEntryInfo)
            {
               if((PANEntryInfo->EventCallback) || (PANEntryInfo->ClientID != ServerID))
               {
                  CallbackInfoArrayPtr[NumberCallbacks].ClientID          = PANEntryInfo->ClientID;
                  CallbackInfoArrayPtr[NumberCallbacks].EventCallback     = PANEntryInfo->EventCallback;
                  CallbackInfoArrayPtr[NumberCallbacks].CallbackParameter = PANEntryInfo->CallbackParameter;

                  NumberCallbacks++;
               }

               PANEntryInfo = PANEntryInfo->NextPANEntryInfoPtr;
            }

            /* Release the Lock because we have already built the       */
            /* Callback Array.                                          */
            DEVM_ReleaseLock();

            /* Now we are ready to dispatch the callbacks.              */
            Index = 0;

            while((Index < NumberCallbacks) && (Initialized))
            {
               /* * NOTE * It is possible that we have already          */
               /*          dispatched the event to the client. To       */
               /*          avoid this case we need to walk the list of  */
               /*          previously dispatched events to check to     */
               /*          see if it has already been dispatched (we    */
               /*          need to do this with Client Address ID's for */
               /*          messages - Event Callbacks are local and     */
               /*          therefore unique so we don't have to do this */
               /*          filtering).                                  */

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
                        (*CallbackInfoArrayPtr[Index].EventCallback)(PANMEventData, CallbackInfoArrayPtr[Index].CallbackParameter);
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

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Connection Request */
   /* Response Message and responds to the message accordingly.  This   */
   /* function does not verify the integrity of the Message (i.e. the   */
   /* length) because it is the caller's responsibility to verify the   */
   /* Message before calling this function.                             */
static void ProcessConnectionRequestResponseMessage(PANM_Connection_Request_Response_Request_t *Message)
{
   int                                          Result;
   Boolean_t                                    Authenticate;
   Boolean_t                                    Encrypt;
   Connection_Entry_t                          *ConnectionEntry;
   PANM_Connection_Request_Response_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered on, next, verify that we are already      */
         /* tracking a connection for the specified connection type.    */
         if(((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress)) != NULL) && (ConnectionEntry->ConnectionState == csAuthorizing))
         {
            DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Respond to Request: %d\n", Message->Accept));

            /* If the caller has accepted the request then we need to   */
            /* process it differently.                                  */
            if(Message->Accept)
            {
               /* Determine if Authentication and/or Encryption is      */
               /* required for this link.                               */
               if(IncomingConnectionFlags & PANM_INCOMING_CONNECTION_FLAGS_REQUIRE_AUTHENTICATION)
                  Authenticate = TRUE;
               else
                  Authenticate = FALSE;

               if(IncomingConnectionFlags & PANM_INCOMING_CONNECTION_FLAGS_REQUIRE_ENCRYPTION)
                  Encrypt = TRUE;
               else
                  Encrypt = FALSE;

               if((Authenticate) || (Encrypt))
               {
                  if(Encrypt)
                     Result = DEVM_EncryptRemoteDevice(ConnectionEntry->BD_ADDR, 0);
                  else
                     Result = DEVM_AuthenticateRemoteDevice(ConnectionEntry->BD_ADDR, 0);
               }
               else
                  Result = BTPM_ERROR_CODE_DEVICE_ALREADY_AUTHENTICATED;

               if((Result == BTPM_ERROR_CODE_DEVICE_ALREADY_AUTHENTICATED) || (Result == BTPM_ERROR_CODE_DEVICE_ALREADY_ENCRYPTED))
               {
                  /* Authorization not required, and we are already in  */
                  /* the correct state.                                 */
                  Result = _PANM_Open_Request_Response(ConnectionEntry->PANID, TRUE);

                  if(Result)
                  {
                     _PANM_Open_Request_Response(ConnectionEntry->PANID, FALSE);

                     /* Go ahead and delete the entry because we are    */
                     /* finished with tracking it.                      */
                     if((ConnectionEntry = DeleteConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress)) != NULL)
                        FreeConnectionEntryMemory(ConnectionEntry);
                  }
                  else
                  {
                     /* Update the current connection state.            */
                     ConnectionEntry->ConnectionState = csConnecting;
                  }
               }
               else
               {
                  /* If we were successfully able to Authenticate and/or*/
                  /* Encrypt, then we need to set the correct state.    */
                  if(!Result)
                  {
                     if(Encrypt)
                        ConnectionEntry->ConnectionState = csEncrypting;
                     else
                        ConnectionEntry->ConnectionState = csAuthenticating;

                     /* Flag success.                                   */
                     Result = 0;
                  }
                  else
                  {
                     /* Error, reject the request.                      */
                     _PANM_Open_Request_Response(ConnectionEntry->PANID, FALSE);

                     /* Go ahead and delete the entry because we are    */
                     /* finished with tracking it.                      */
                     if((ConnectionEntry = DeleteConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress)) != NULL)
                        FreeConnectionEntryMemory(ConnectionEntry);
                  }
               }
            }
            else
            {
               /* Rejection - Simply respond to the request.            */
               Result = _PANM_Open_Request_Response(ConnectionEntry->PANID, FALSE);

               /* Go ahead and delete the entry because we are finished */
               /* with tracking it.                                     */
               if((ConnectionEntry = DeleteConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress)) != NULL)
                  FreeConnectionEntryMemory(ConnectionEntry);
            }
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_CONNECTION_STATE;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength  = (PANM_CONNECTION_REQUEST_RESPONSE_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE);
      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Connect Remote     */
   /* Device Message and responds to the message accordingly.  This     */
   /* function does not verify the integrity of the Message (i.e. the   */
   /* length) because it is the caller's responsibility to verify the   */
   /* Message before calling this function.                             */
static void ProcessConnectRemoteDeviceMessage(PANM_Connect_Remote_Device_Request_t *Message)
{
   int                                    Result;
   Connection_Entry_t                     ConnectionEntry;
   Connection_Entry_t                    *ConnectionEntryPtr;
   PANM_Connect_Remote_Device_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered on, make sure the parameters are          */
         /* semi-valid.                                                 */
         if(!COMPARE_NULL_BD_ADDR(Message->RemoteDeviceAddress))
         {
            /* Now make sure we are not already tracking a connection to*/
            /* this device.                                             */
            if((ConnectionEntryPtr = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress)) == NULL)
            {
               /* Entry is not present, go ahead and create a new one.  */
               BTPS_MemInitialize(&ConnectionEntry, 0, sizeof(Connection_Entry_t));

               ConnectionEntry.BD_ADDR               = Message->RemoteDeviceAddress;
               ConnectionEntry.PANID                 = GetNextCallbackID() | 0x80000000;
               ConnectionEntry.RemoteConnection      = TRUE;
               ConnectionEntry.LocalServiceType      = Message->LocalServiceType;
               ConnectionEntry.RemoteServiceType     = Message->RemoteServiceType;
               ConnectionEntry.CallbackInfo.ClientID = Message->MessageHeader.AddressID;

               if((ConnectionEntryPtr = AddConnectionEntry(&ConnectionEntryList, &ConnectionEntry)) != NULL)
               {
                  /* Determine how we need to connect.                  */
                  if(Message->ConnectionFlags & PANM_CONNECT_REMOTE_DEVICE_FLAGS_REQUIRE_ENCRYPTION)
                     ConnectionEntryPtr->ConnectionState = csEncrypting;
                  else
                  {
                     if(Message->ConnectionFlags & PANM_CONNECT_REMOTE_DEVICE_FLAGS_REQUIRE_AUTHENTICATION)
                        ConnectionEntryPtr->ConnectionState = csAuthenticating;
                     else
                        ConnectionEntryPtr->ConnectionState = csIdle;
                  }

                  /* Go ahead and attempt to establish a connection.    */
                  Result = DEVM_ConnectWithRemoteDevice(Message->RemoteDeviceAddress, (ConnectionEntryPtr->ConnectionState == csIdle)?0:((ConnectionEntryPtr->ConnectionState == csEncrypting)?(DEVM_CONNECT_WITH_REMOTE_DEVICE_FLAGS_AUTHENTICATE | DEVM_CONNECT_WITH_REMOTE_DEVICE_FLAGS_ENCRYPT):DEVM_CONNECT_WITH_REMOTE_DEVICE_FLAGS_AUTHENTICATE));

                  if((Result == BTPM_ERROR_CODE_DEVICE_IS_CURRENTLY_CONNECTED) || (Result == BTPM_ERROR_CODE_DEVICE_ALREADY_AUTHENTICATED) || (Result == BTPM_ERROR_CODE_DEVICE_ALREADY_ENCRYPTED))
                  {
                     /* We are already connected, so attempt to open a  */
                     /* PAN Connection.                                 */
                     if((Result = _PANM_Open_Remote_Server(Message->RemoteDeviceAddress, Message->LocalServiceType, Message->RemoteServiceType)) > 0)
                     {
                        /* Open attempt successful, so note the new     */
                        /* state.                                       */
                        ConnectionEntryPtr->PANID           = Result;
                        ConnectionEntryPtr->ConnectionState = csConnecting;

                        /* Flag success to the caller.                  */
                        Result                              = 0;
                     }
                  }

                  /* If an error occurred, go ahead and delete the      */
                  /* Connection Information that was added.             */
                  if(Result)
                  {
                     if((ConnectionEntryPtr = DeleteConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress)) != NULL)
                        FreeConnectionEntryMemory(ConnectionEntryPtr);
                  }
               }
               else
                  Result = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
            }
            else
            {
               if(ConnectionEntryPtr->ConnectionState == csConnected)
                  Result = BTPM_ERROR_CODE_PAN_DEVICE_ALREADY_CONNECTED;
               else
                  Result = BTPM_ERROR_CODE_PAN_CONNECTION_IN_PROGRESS;
            }
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      /* Format the response message to send.                           */
      ResponseMessage.MessageHeader                = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength  = (PANM_CONNECT_REMOTE_DEVICE_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE);
      ResponseMessage.Status                       = Result;

      /* Message is fomatted, go ahead and send it.                     */
      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Close Connection   */
   /* Message and responds to the message accordingly.  This function   */
   /* does not verify the integrity of the Message (i.e. the length)    */
   /* because it is the caller's responsibility to verify the Message   */
   /* before calling this function.                                     */
static void ProcessCloseConnectionMessage(PANM_Close_Connection_Request_t *Message)
{
   int                               Result;
   Connection_Entry_t               *ConnectionEntry;
   PAN_Close_Indication_Data_t       CloseIndicationData;
   PANM_Close_Connection_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter specified.                          */
   if(Message)
   {
      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Make sure we have are tracking a connection.                */
         if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress)) != NULL)
         {
            /* Make sure the connection isn't still in progress.        */
            if(ConnectionEntry->ConnectionState == csConnected)
            {
               /* Go ahead and attempt to disconnect.                   */
               Result = _PANM_Close_Connection(ConnectionEntry->PANID);
            }
            else
               Result = BTPM_ERROR_CODE_PAN_CONNECTION_IN_PROGRESS;
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_PARAMETER;

      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength  = (PANM_CLOSE_CONNECTION_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE);
      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);

      if(!Result)
      {
         /* Format a fake PAN Close Event to notify registered clients  */
         /* of the disconnection.                                       */
         CloseIndicationData.PANID = ConnectionEntry->PANID;

         ProcessCloseIndicationEvent(&CloseIndicationData);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Query Connected    */
   /* Devices Message and responds to the message accordingly.  This    */
   /* function does not verify the integrity of the Message (i.e.  the  */
   /* length) because it is the caller's responsibility to verify the   */
   /* Message before calling this function.                             */
static void ProcessQueryConnectedDevicesMessage(PANM_Query_Connected_Devices_Request_t *Message)
{
   unsigned int                             NumberConnected;
   Connection_Entry_t                      *ConnectionEntry;
   PANM_Query_Connected_Devices_Response_t  ErrorResponseMessage;
   PANM_Query_Connected_Devices_Response_t *ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Initialize the Error Response Message.                         */
      ErrorResponseMessage.MessageHeader                = Message->MessageHeader;
      ErrorResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ErrorResponseMessage.MessageHeader.MessageLength  = PANM_QUERY_CONNECTED_DEVICES_RESPONSE_SIZE(0) - BTPM_MESSAGE_HEADER_SIZE;
      ErrorResponseMessage.Status                       = 0;
      ErrorResponseMessage.NumberDevicesConnected       = 0;

      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered up, so now let's determine how many       */
         /* devices are connected.                                      */
         NumberConnected = 0;
         ConnectionEntry = ConnectionEntryList;

         while(ConnectionEntry)
         {
            /* Note that we are only counting devices that are either in*/
            /* the connected state or the connecting state (i.e. have   */
            /* been authorized OR passed authentication).               */
            if((ConnectionEntry->ConnectionState == csConnected) || (ConnectionEntry->ConnectionState == csConnecting))
               NumberConnected++;

            ConnectionEntry = ConnectionEntry->NextConnectionEntryPtr;
         }

         /* Now let's attempt to allocate memory to hold the entire     */
         /* list.                                                       */
         if((ResponseMessage = (PANM_Query_Connected_Devices_Response_t *)BTPS_AllocateMemory(PANM_QUERY_CONNECTED_DEVICES_RESPONSE_SIZE(NumberConnected))) != NULL)
         {
            /* Memory allocated, now let's build the response message.  */
            /* * NOTE * Error Response has initialized all values to    */
            /*          known values (i.e. zero devices and success).   */
            BTPS_MemCopy(ResponseMessage, &ErrorResponseMessage, PANM_QUERY_CONNECTED_DEVICES_RESPONSE_SIZE(0));

            ConnectionEntry = ConnectionEntryList;

            while((ConnectionEntry) && (ResponseMessage->NumberDevicesConnected < NumberConnected))
            {
               /* Note that we are only counting devices that are either*/
               /* in the connected state or the connecting state (i.e.  */
               /* have been authorized OR passed authentication).       */
               if((ConnectionEntry->ConnectionState == csConnected) || (ConnectionEntry->ConnectionState == csConnecting))
                  ResponseMessage->DeviceConnectedList[ResponseMessage->NumberDevicesConnected++] = ConnectionEntry->BD_ADDR;

               ConnectionEntry = ConnectionEntry->NextConnectionEntryPtr;
            }

            /* Now that we are finsished we need to update the length.  */
            ResponseMessage->MessageHeader.MessageLength  = PANM_QUERY_CONNECTED_DEVICES_RESPONSE_SIZE(ResponseMessage->NumberDevicesConnected) - BTPM_MESSAGE_HEADER_SIZE;

            /* Response Message built, go ahead and send it.            */
            MSG_SendMessage((BTPM_Message_t *)ResponseMessage);

            /* Free the memory that was allocated because are finished  */
            /* with it.                                                 */
            BTPS_FreeMemory(ResponseMessage);
         }
         else
         {
            ErrorResponseMessage.Status = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;

            MSG_SendMessage((BTPM_Message_t *)&ErrorResponseMessage);
         }
      }
      else
      {
         ErrorResponseMessage.Status = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

         MSG_SendMessage((BTPM_Message_t *)&ErrorResponseMessage);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Query Current      */
   /* Configuration Message and responds to the message accordingly.    */
   /* This function does not verify the integrity of the Message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the Message before calling this function.                         */
static void ProcessQueryCurrentConfigurationMessage(PANM_Query_Current_Configuration_Request_t *Message)
{
   PANM_Query_Current_Configuration_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */
      ResponseMessage.MessageHeader                = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength  = (PANM_QUERY_CURRENT_CONFIGURATION_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE);
      ResponseMessage.Status                       = 0;

      ResponseMessage.ServiceTypeFlags             = ServiceTypeFlags;
      ResponseMessage.IncomingConnectionFlags      = IncomingConnectionFlags;

      /* Response Message built, go ahead and send it.                  */
      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Change Incoming    */
   /* Connection Flags Message and responds to the message accordingly. */
   /* This function does not verify the integrity of the Message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the Message before calling this function.                         */
static void ProcessChangeIncomingConnectionFlagsMessage(PANM_Change_Incoming_Connection_Flags_Request_t *Message)
{
   PANM_Change_Incoming_Connection_Flags_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input appears semi-valid.                           */
   if(Message)
   {
      /* Simply update to the new connection flags.                     */
      IncomingConnectionFlags                      = Message->ConnectionFlags;

      /* Format the response message to inform the user we received the */
      /* flags.                                                         */
      ResponseMessage.MessageHeader                = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength  = (PANM_CHANGE_INCOMING_CONNECTION_FLAGS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE);
      ResponseMessage.Status                       = 0;

      /* Go ahead and send the message.                                 */
      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Register Events    */
   /* Message and responds to the message accordingly. This function    */
   /* does not verify the integrity of the Message (i.e. the length)    */
   /* because it is the caller's responsibility to verify the Message   */
   /* before calling this function.                                     */
static void ProcessRegisterEventsMessage(PANM_Register_Events_Request_t *Message)
{
   int                             Result;
   PAN_Entry_Info_t                PANEntryInfo;
   PANM_Register_Events_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Attempt to add an entry into the PAN Entry list.               */
      BTPS_MemInitialize(&PANEntryInfo, 0, sizeof(PAN_Entry_Info_t));

      PANEntryInfo.CallbackID = GetNextCallbackID();
      PANEntryInfo.ClientID   = Message->MessageHeader.AddressID;

      if(AddPANEntryInfoEntry(&PANEntryInfoList, &PANEntryInfo))
         Result = PANEntryInfo.CallbackID;
      else
         Result = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;

      ResponseMessage.MessageHeader                = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength  = (PANM_REGISTER_EVENTS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

      if(Result > 0)
      {
         ResponseMessage.EventsHandlerID = (unsigned int)Result;
         ResponseMessage.Status          = 0;
      }
      else
      {
         ResponseMessage.EventsHandlerID = 0;
         ResponseMessage.Status          = Result;
      }

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Un-register Events */
   /* Message and responds to the message accordingly. This function    */
   /* does not verify the integrity of the Message (i.e. the length)    */
   /* because it is the caller's responsibility to verify the Message   */
   /* before calling this function.                                     */
static void ProcessUnRegisterEventsMessage(PANM_Un_Register_Events_Request_t *Message)
{
   int                                 Result;
   PAN_Entry_Info_t                   *PANEntryInfo;
   PANM_Un_Register_Events_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* First, check to make sure the client is the client that        */
      /* actually registered for the events.                            */
      if(((PANEntryInfo = SearchPANEntryInfoEntry(&PANEntryInfoList, Message->EventsHandlerID)) != NULL) && (PANEntryInfo->ClientID == Message->MessageHeader.AddressID))
      {
         /* Go ahead and process the message request.                   */
         if((PANEntryInfo = DeletePANEntryInfoEntry(&PANEntryInfoList, Message->EventsHandlerID)) != NULL)
         {
            /* Free the memory because we are finished with it.         */
            FreePANEntryInfoEntryMemory(PANEntryInfo);

            /* Flag success.                                            */
            Result = 0;
         }
         else
            Result = BTPM_ERROR_CODE_PAN_EVENT_HANDLER_NOT_REGISTERED;
      }
      else
         Result = BTPM_ERROR_CODE_PAN_EVENT_HANDLER_NOT_REGISTERED;

      ResponseMessage.MessageHeader                = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength  = (PANM_UN_REGISTER_EVENTS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE);
      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is called from the Main Bluetopia Dispatch */
   /* thread in response to a Process Received Message.  This function  */
   /* SHOULD ONLY be called in this capacity and NOT under any other    */
   /* circumstances.                                                    */
   /* * NOTE * This function *MUST* be called with the PAN Manager      */
   /*          Lock held.  This function will release the Lock before   */
   /*          it exits (i.e. the caller SHOULD NOT RELEASE THE LOCK).  */
static void ProcessReceivedMessage(BTPM_Message_t *Message)
{
   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to make sure this isn't some kind of spoofed response (we do*/
   /* not process responses).                                           */
   if(Message && (!(Message->MessageHeader.MessageID & BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK)))
   {
      /* Request received, go ahead and process it.                     */
      switch(Message->MessageHeader.MessageFunction)
      {
         case PANM_MESSAGE_FUNCTION_CONNECTION_REQUEST_RESPONSE:
            DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("Open Request Resposne Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PANM_CONNECTION_REQUEST_RESPONSE_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Connection Request Response.                          */
               ProcessConnectionRequestResponseMessage((PANM_Connection_Request_Response_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case PANM_MESSAGE_FUNCTION_CONNECT_REMOTE_DEVICE:
            DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("Open Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PANM_CONNECT_REMOTE_DEVICE_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Connect Remote Device Message.                        */
               ProcessConnectRemoteDeviceMessage((PANM_Connect_Remote_Device_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case PANM_MESSAGE_FUNCTION_CLOSE_CONNECTION:
            DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("Close Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PANM_CLOSE_CONNECTION_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Close Connection Message.                             */
               ProcessCloseConnectionMessage((PANM_Close_Connection_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case PANM_MESSAGE_FUNCTION_QUERY_CONNECTED_DEVICES:
            DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("Query Connected Devices Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PANM_QUERY_CONNECTED_DEVICES_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Query Connected Devices Message.                      */
               ProcessQueryConnectedDevicesMessage((PANM_Query_Connected_Devices_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case PANM_MESSAGE_FUNCTION_QUERY_CURRENT_CONFIGURATION:
            DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("Query Current Configuration Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PANM_QUERY_CURRENT_CONFIGURATION_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Query Current Configuration Message.                  */
               ProcessQueryCurrentConfigurationMessage((PANM_Query_Current_Configuration_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case PANM_MESSAGE_FUNCTION_CHANGE_INCOMING_CONNECTION_FLAGS:
            DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("Change Incoming Connection Flags Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PANM_CHANGE_INCOMING_CONNECTION_FLAGS_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Change Incoming Connection Flags Message.             */
               ProcessChangeIncomingConnectionFlagsMessage((PANM_Change_Incoming_Connection_Flags_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case PANM_MESSAGE_FUNCTION_REGISTER_EVENTS:
            DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("Register Events Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PANM_REGISTER_EVENTS_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Register Events Message.                              */
               ProcessRegisterEventsMessage((PANM_Register_Events_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case PANM_MESSAGE_FUNCTION_UN_REGISTER_EVENTS:
            DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("UnRegister Events Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PANM_UN_REGISTER_EVENTS_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Un-Register Events Message.                           */
               ProcessUnRegisterEventsMessage((PANM_Un_Register_Events_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         default:
            DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled Message\n"));
            break;
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid or Response Message Received\n"));
   }

   /* Release the Lock because we are finished with it.                 */
   DEVM_ReleaseLock();

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the function that is called to process a*/
   /* client Un-Registration event.  This function will perform any     */
   /* cleanup required for the specified Client.                        */
static void ProcessClientUnRegister(unsigned int ClientID)
{
   PAN_Entry_Info_t *PANEntryInfo;
   PAN_Entry_Info_t *tmpPANEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if((ClientID) && (ClientID != MSG_GetServerAddressID()))
   {
      PANEntryInfo = PANEntryInfoList;

      /* We need to loop through the list to remove any callbacks       */
      /* registered by the client.                                      */
      while(PANEntryInfo)
      {
         /* Check to see if the Client Information is the one that is   */
         /* being un-registered.                                        */
         if(PANEntryInfo->ClientID == ClientID)
         {
            /* Note the next PAN Entry in the list (we are about to     */
            /* delete the current entry.                                */
            tmpPANEntryInfo = PANEntryInfo->NextPANEntryInfoPtr;

            /* Go ahead and delete the PAN Information Entry and clean  */
            /* up the resources.                                        */
            if((PANEntryInfo = DeletePANEntryInfoEntry(&PANEntryInfoList, PANEntryInfo->CallbackID)) != NULL)
            {
               /* All finished with the memory so free the entry.       */
               FreePANEntryInfoEntryMemory(PANEntryInfo);
            }

            PANEntryInfo = tmpPANEntryInfo;
         }
         else
            PANEntryInfo = PANEntryInfo->NextPANEntryInfoPtr;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a PAN        */
   /* Open Request Indication Event that was been received with the     */
   /* specified information. This function should be called with the    */
   /* Lock protecting the PAN Manager Information held.                 */
static void ProcessOpenRequestIndicationEvent(PAN_Open_Request_Indication_Data_t *OpenRequestIndicationData)
{
   int                                Result;
   Boolean_t                          Authenticate;
   Boolean_t                          Encrypt;
   PANM_Event_Data_t                  PANMEventData;
   Connection_Entry_t                 ConnectionEntry;
   Connection_Entry_t                *ConnectionEntryPtr;
   PANM_Connection_Request_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(OpenRequestIndicationData)
   {
      /* First, let's see if we actually need to do anything, other than*/
      /* simply accept the connection.                                  */
      if(!IncomingConnectionFlags)
      {
         /* Simply Accept the connection.                               */
         _PANM_Open_Request_Response(OpenRequestIndicationData->PANID, TRUE);
      }
      else
      {
         /* Before proceding any further, let's make sure that there    */
         /* doesn't already exist an entry for this device.             */
         if((ConnectionEntryPtr = SearchConnectionEntry(&ConnectionEntryList, OpenRequestIndicationData->BD_ADDR)) == NULL)
         {
            BTPS_MemInitialize(&ConnectionEntry, 0, sizeof(Connection_Entry_t));

            /* Entry does not exist, go ahead and format a new entry.   */
            ConnectionEntry.PANID                  = OpenRequestIndicationData->PANID;
            ConnectionEntry.BD_ADDR                = OpenRequestIndicationData->BD_ADDR;
            ConnectionEntry.ConnectionState        = csAuthorizing;

            ConnectionEntryPtr                     = AddConnectionEntry(&ConnectionEntryList, &ConnectionEntry);
         }

         /* Check to see if we are tracking this connection.            */
         if(ConnectionEntryPtr)
         {
            if(IncomingConnectionFlags & PANM_INCOMING_CONNECTION_FLAGS_REQUIRE_AUTHORIZATION)
            {
               /* Authorization (at least) required, go ahead and       */
               /* dispatch the request.                                 */
               ConnectionEntryPtr->ConnectionState                                            = csAuthorizing;

               /* Next, format up the Event to dispatch.                */
               PANMEventData.EventType                                                        = petPANMIncomingConnectionRequest;
               PANMEventData.EventLength                                                      = PANM_INCOMING_CONNECTION_REQUEST_EVENT_DATA_SIZE;

               PANMEventData.EventData.IncomingConnectionReqeustEventData.RemoteDeviceAddress = OpenRequestIndicationData->BD_ADDR;

               /* Next, format up the Message to dispatch.              */
               BTPS_MemInitialize(&Message, 0, sizeof(Message));

               Message.MessageHeader.AddressID       = 0;
               Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
               Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PAN_MANAGER;
               Message.MessageHeader.MessageFunction = PANM_MESSAGE_FUNCTION_CONNECTION_REQUEST;
               Message.MessageHeader.MessageLength   = (PANM_CONNECTION_REQUEST_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

               Message.RemoteDeviceAddress           = OpenRequestIndicationData->BD_ADDR;

               /* Finally dispatch the formatted Event and Message.     */
               DispatchPANMEvent(&PANMEventData, (BTPM_Message_t *)&Message);
            }
            else
            {
               /* Determine if Authentication and/or Encryption is      */
               /* required for this link.                               */
               if(IncomingConnectionFlags & PANM_INCOMING_CONNECTION_FLAGS_REQUIRE_AUTHENTICATION)
                  Authenticate = TRUE;
               else
                  Authenticate = FALSE;

               if(IncomingConnectionFlags & PANM_INCOMING_CONNECTION_FLAGS_REQUIRE_ENCRYPTION)
                  Encrypt = TRUE;
               else
                  Encrypt = FALSE;

               if((Authenticate) || (Encrypt))
               {
                  if(Encrypt)
                     Result = DEVM_EncryptRemoteDevice(ConnectionEntryPtr->BD_ADDR, 0);
                  else
                     Result = DEVM_AuthenticateRemoteDevice(ConnectionEntryPtr->BD_ADDR, 0);
               }
               else
                  Result = BTPM_ERROR_CODE_DEVICE_ALREADY_AUTHENTICATED;

               if((Result == BTPM_ERROR_CODE_DEVICE_ALREADY_AUTHENTICATED) || (Result == BTPM_ERROR_CODE_DEVICE_ALREADY_ENCRYPTED))
               {
                  Result = _PANM_Open_Request_Response(ConnectionEntryPtr->PANID, TRUE);

                  if(Result)
                  {
                     _PANM_Open_Request_Response(ConnectionEntryPtr->PANID, FALSE);

                     /* Go ahead and delete the entry because we are    */
                     /* finished with tracking it.                      */
                     if((ConnectionEntryPtr = DeleteConnectionEntry(&ConnectionEntryList, ConnectionEntryPtr->BD_ADDR)) != NULL)
                        FreeConnectionEntryMemory(ConnectionEntryPtr);
                  }
                  else
                  {
                     /* Update the current connection state.            */
                     ConnectionEntryPtr->ConnectionState = csConnecting;
                  }
               }
               else
               {
                  /* If we were successfully able to Authenticate and/or*/
                  /* Encrypt, then we need to set the correct state.    */
                  if(!Result)
                  {
                     if(Encrypt)
                        ConnectionEntryPtr->ConnectionState = csEncrypting;
                     else
                        ConnectionEntryPtr->ConnectionState = csAuthenticating;

                     /* Flag success.                                   */
                     Result = 0;
                  }
                  else
                  {
                     /* Error, reject the request.                      */
                     _PANM_Open_Request_Response(ConnectionEntryPtr->PANID, FALSE);

                     /* Go ahead and delete the entry because we are    */
                     /* finished with tracking it.                      */
                     if((ConnectionEntryPtr = DeleteConnectionEntry(&ConnectionEntryList, ConnectionEntryPtr->BD_ADDR)) != NULL)
                        FreeConnectionEntryMemory(ConnectionEntryPtr);
                  }
               }
            }
         }
         else
         {
            /* Unable to add entry, go ahead and reject the request.    */
            _PANM_Open_Request_Response(OpenRequestIndicationData->PANID, FALSE);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a PAN Open   */
   /* Indication Event that was been received with the specified        */
   /* information. This function should be called with the Lock         */
   /* protecting the PAN Manager Information held.                      */
static void ProcessOpenIndicationEvent(PAN_Open_Indication_Data_t *OpenIndicationData)
{
   PANM_Event_Data_t                PANMEventData;
   Connection_Entry_t               ConnectionEntry;
   Connection_Entry_t              *ConnectionEntryPtr;
   PANM_Device_Connected_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(OpenIndicationData)
   {
      if((ConnectionEntryPtr = SearchConnectionEntry(&ConnectionEntryList, OpenIndicationData->BD_ADDR)) == NULL)
      {
         /* Entry does not exist, go ahead and format a new entry.      */
         BTPS_MemInitialize(&ConnectionEntry, 0, sizeof(Connection_Entry_t));

         ConnectionEntry.PANID           = OpenIndicationData->PANID;
         ConnectionEntry.BD_ADDR         = OpenIndicationData->BD_ADDR;
         ConnectionEntry.ConnectionState = csConnected;

         ConnectionEntryPtr              = AddConnectionEntry(&ConnectionEntryList, &ConnectionEntry);
      }
      else
         ConnectionEntryPtr->ConnectionState = csConnected;

      if(ConnectionEntryPtr)
      {
         /* Next, format up the Event to dispatch.                      */
         PANMEventData.EventType                                        = petPANMConnected;
         PANMEventData.EventLength                                      = PANM_CONNECTED_EVENT_DATA_SIZE;

         PANMEventData.EventData.ConnectedEventData.RemoteDeviceAddress = OpenIndicationData->BD_ADDR;
         PANMEventData.EventData.ConnectedEventData.ServiceType         = OpenIndicationData->ServiceType;

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&Message, 0, sizeof(Message));

         Message.MessageHeader.AddressID       = 0;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PAN_MANAGER;
         Message.MessageHeader.MessageFunction = PANM_MESSAGE_FUNCTION_DEVICE_CONNECTED;
         Message.MessageHeader.MessageLength   = (PANM_DEVICE_CONNECTED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.RemoteDeviceAddress           = OpenIndicationData->BD_ADDR;
         Message.ServiceType                   = OpenIndicationData->ServiceType;

         /* Finally, dispatch the formatted Event and Message.          */
         DispatchPANMEvent(&PANMEventData, (BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a PAN Open   */
   /* Confirmation Event that was been received with the specified      */
   /* information. This function should be called with the Lock         */
   /* protecting the PAN Manager Information held.                      */
static void ProcessOpenConfirmationEvent(Boolean_t DispatchOpen, PAN_Open_Confirmation_Data_t *OpenConfirmationData)
{
   void                                    *CallbackParameter;
   Boolean_t                                Delete;
   PANM_Event_Data_t                        PANMEventData;
   Connection_Entry_t                      *ConnectionEntryPtr;
   PANM_Event_Callback_t                    EventCallback;
   PANM_Device_Connected_Message_t          ConnectedMessage;
   PANM_Device_Connection_Status_Message_t  ConnectionStatusMessage;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(OpenConfirmationData)
   {
      /* Make sure we are tracking this connection.                     */
      if((ConnectionEntryPtr = SearchConnectionEntryID(&ConnectionEntryList, OpenConfirmationData->PANID)) != NULL)
      {
         /* Flag that we need to delete the entry.                      */
         Delete = TRUE;

         /* Determine where we need to dispatch the Connection Status.  */
         if(ConnectionEntryPtr->CallbackInfo.ClientID == MSG_GetServerAddressID())
         {
            /* We need to dispatch the Status Event locally. Go ahead   */
            /* and format the event.                                    */
            BTPS_MemInitialize(&PANMEventData, 0, sizeof(PANM_Event_Data_t));

            PANMEventData.EventType                                               = petPANMConnectionStatus;
            PANMEventData.EventLength                                             = PANM_CONNECTION_STATUS_EVENT_DATA_SIZE;

            PANMEventData.EventData.ConnectionStatusEventData.RemoteDeviceAddress = ConnectionEntryPtr->BD_ADDR;
            PANMEventData.EventData.ConnectionStatusEventData.ServiceType         = ConnectionEntryPtr->RemoteServiceType;

            /* We need to map the PAN status to a known status.         */
            switch(OpenConfirmationData->OpenStatus)
            {
               case PAN_OPEN_STATUS_SUCCESS:
                  PANMEventData.EventData.ConnectionStatusEventData.Status = PANM_DEVICE_CONNECTION_STATUS_SUCCESS;
                  break;
               case PAN_OPEN_STATUS_CONNECTION_TIMEOUT:
                  PANMEventData.EventData.ConnectionStatusEventData.Status = PANM_DEVICE_CONNECTION_STATUS_FAILURE_TIMEOUT;
                  break;
               case PAN_OPEN_STATUS_CONNECTION_REFUSED:
                  PANMEventData.EventData.ConnectionStatusEventData.Status = PANM_DEVICE_CONNECTION_STATUS_FAILURE_REFUSED;
                  break;
               default:
                  PANMEventData.EventData.ConnectionStatusEventData.Status = PANM_DEVICE_CONNECTION_STATUS_FAILURE_UNKNOWN;
                  break;
            }

            /* Now, determine if we are waiting on a synchronous event. */
            if(ConnectionEntryPtr->ConnectionEvent)
            {
               /* The Open function that is waiting needs access to this*/
               /* entry, so we won't delete it.                         */
               Delete = FALSE;

               /* Waiting for synchronous event, so note the status and */
               /* set the event.                                        */
               ConnectionEntryPtr->ConnectionStatus = PANMEventData.EventData.ConnectionStatusEventData.Status;

               BTPS_SetEvent(ConnectionEntryPtr->ConnectionEvent);
            }
            else
            {
               /* Dispatch asynchronously, note the callback            */
               /* information.                                          */
               EventCallback     = ConnectionEntryPtr->CallbackInfo.EventCallback;
               CallbackParameter = ConnectionEntryPtr->CallbackInfo.CallbackParameter;

               /* Release the Lock so we can make the callback.         */
               DEVM_ReleaseLock();

               __BTPSTRY
               {
                  if(EventCallback)
                     (*EventCallback)(&PANMEventData, CallbackParameter);
               }
               __BTPSEXCEPT(1)
               {
                  /* Do Nothing.                                        */
               }

               /* Re-acquire the Lock.                                  */
               DEVM_AcquireLock();
            }
         }
         else
         {
            /* Remote Event.                                            */

            /* Format the Message to dispatch.                          */
            BTPS_MemInitialize(&ConnectionStatusMessage, 0, sizeof(ConnectionStatusMessage));

            ConnectionStatusMessage.MessageHeader.AddressID       = ConnectionEntryPtr->CallbackInfo.ClientID;
            ConnectionStatusMessage.MessageHeader.MessageID       = MSG_GetNextMessageID();
            ConnectionStatusMessage.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PAN_MANAGER;
            ConnectionStatusMessage.MessageHeader.MessageFunction = PANM_MESSAGE_FUNCTION_DEVICE_CONNECTION_STATUS;
            ConnectionStatusMessage.MessageHeader.MessageLength   = (PANM_DEVICE_CONNECTION_STATUS_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

            ConnectionStatusMessage.RemoteDeviceAddress           = ConnectionEntryPtr->BD_ADDR;
            ConnectionStatusMessage.ServiceType                   = ConnectionEntryPtr->RemoteServiceType;

            /* Map the PAN status to a known status.                    */
            switch(OpenConfirmationData->OpenStatus)
            {
               case PAN_OPEN_STATUS_SUCCESS:
                  ConnectionStatusMessage.ConnectionStatus = PANM_DEVICE_CONNECTION_STATUS_SUCCESS;
                  break;
               case PAN_OPEN_STATUS_CONNECTION_TIMEOUT:
                  ConnectionStatusMessage.ConnectionStatus = PANM_DEVICE_CONNECTION_STATUS_FAILURE_TIMEOUT;
                  break;
               case PAN_OPEN_STATUS_CONNECTION_REFUSED:
                  ConnectionStatusMessage.ConnectionStatus = PANM_DEVICE_CONNECTION_STATUS_FAILURE_REFUSED;
                  break;
               default:
                  ConnectionStatusMessage.ConnectionStatus = PANM_DEVICE_CONNECTION_STATUS_FAILURE_UNKNOWN;
                  break;
            }

            /* Finally, dispatch the Message.                           */
            MSG_SendMessage((BTPM_Message_t *)&ConnectionStatusMessage);
         }

         if(!(OpenConfirmationData->OpenStatus) && (DispatchOpen))
         {
            /* Connection was successful, so we need to notify all      */
            /* registered clients, go ahead and format the Event.       */
            PANMEventData.EventType                                        = petPANMConnected;
            PANMEventData.EventLength                                      = PANM_CONNECTED_EVENT_DATA_SIZE;

            PANMEventData.EventData.ConnectedEventData.RemoteDeviceAddress = ConnectionEntryPtr->BD_ADDR;
            PANMEventData.EventData.ConnectedEventData.ServiceType         = ConnectionEntryPtr->RemoteServiceType;

            /* Next, format the Message to dispatch.                    */
            BTPS_MemInitialize(&ConnectedMessage, 0, sizeof(ConnectedMessage));

            ConnectedMessage.MessageHeader.AddressID       = 0;
            ConnectedMessage.MessageHeader.MessageID       = MSG_GetNextMessageID();
            ConnectedMessage.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PAN_MANAGER;
            ConnectedMessage.MessageHeader.MessageFunction = PANM_MESSAGE_FUNCTION_DEVICE_CONNECTED;
            ConnectedMessage.MessageHeader.MessageLength   = (PANM_DEVICE_CONNECTED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

            ConnectedMessage.RemoteDeviceAddress           = ConnectionEntryPtr->BD_ADDR;
            ConnectedMessage.ServiceType                   = ConnectionEntryPtr->RemoteServiceType;

            /* Now, note the change in state.                           */
            ConnectionEntryPtr->ConnectionState            = csConnected;

            /* Finally, dispatch the formatted Event and message.       */
            DispatchPANMEvent(&PANMEventData, (BTPM_Message_t *)&ConnectedMessage);
         }
         else
         {
            if(Delete)
            {
               /* Connection failed, so go ahead and delete the entry.  */
               if((ConnectionEntryPtr = DeleteConnectionEntry(&ConnectionEntryList, ConnectionEntryPtr->BD_ADDR)) != NULL)
                  FreeConnectionEntryMemory(ConnectionEntryPtr);
            }
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a PAN Close  */
   /* Indication Event that was been received with the specified        */
   /* information. This function should be called with the Lock         */
   /* protecting the PAN Manager Information held.                      */
static void ProcessCloseIndicationEvent(PAN_Close_Indication_Data_t *CloseIndicationData)
{
   PANM_Event_Data_t                   PANMEventData;
   Connection_Entry_t                 *ConnectionEntry;
   PANM_Device_Disconnected_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(CloseIndicationData)
   {
      /* First, Find the connection we are tracking.                    */
      if((ConnectionEntry = SearchConnectionEntryID(&ConnectionEntryList, CloseIndicationData->PANID)) != NULL)
      {
         /* Next, format up the Event to dispatch.                      */
         PANMEventData.EventType                                           = petPANMDisconnected;
         PANMEventData.EventLength                                         = PANM_DISCONNECTED_EVENT_DATA_SIZE;

         PANMEventData.EventData.DisconnectedEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;
         PANMEventData.EventData.DisconnectedEventData.ServiceType         = ConnectionEntry->RemoteServiceType;

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&Message, 0, sizeof(Message));

         Message.MessageHeader.AddressID       = 0;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PAN_MANAGER;
         Message.MessageHeader.MessageFunction = PANM_MESSAGE_FUNCTION_DEVICE_DISCONNECTED;
         Message.MessageHeader.MessageLength   = (PANM_DEVICE_DISCONNECTED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;
         Message.ServiceType                   = ConnectionEntry->RemoteServiceType;

         /* We are finished with the entry, so release it.              */
         if((ConnectionEntry = DeleteConnectionEntry(&ConnectionEntryList, ConnectionEntry->BD_ADDR)) != NULL)
         {
            if(ConnectionEntry->ConnectionEvent)
               BTPS_CloseEvent(ConnectionEntry->ConnectionEvent);

            FreeConnectionEntryMemory(ConnectionEntry);
         }

         /* Finally, dispatch the formatted Event and Message.          */
         DispatchPANMEvent(&PANMEventData, (BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the function that is responsible for    */
   /* processing PAN Events that have been received. This function      */
   /* should ONLY be called with the Context locked AND ONLY in the     */
   /* context of an arbitrary processing thread.                        */
static void ProcessPANEvent(PANM_PAN_Event_Data_t *PANEventData)
{
   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First make sure that the input parameter appears to be semi-valid.*/
   if(PANEventData)
   {
      /* Process the event based on the event type.                     */
      switch(PANEventData->EventType)
      {
         case etPAN_Open_Request_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("Open Request Indication\n"));

            ProcessOpenRequestIndicationEvent(&(PANEventData->EventData.PAN_Open_Request_Indication_Data));
            break;
         case etPAN_Open_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("Open Indication\n"));

            ProcessOpenIndicationEvent(&(PANEventData->EventData.PAN_Open_Indication_Data));
            break;
         case etPAN_Open_Confirmation:
            DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("Open Confirmation\n"));

            ProcessOpenConfirmationEvent(TRUE, &(PANEventData->EventData.PAN_Open_Confirmation_Data));
            break;
         case etPAN_Close_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("Close Indication\n"));

            ProcessCloseIndicationEvent(&(PANEventData->EventData.PAN_Close_Indication_Data));
            break;
         default:
            /* Invalid/Unknown Event.                                   */
            DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid/Unkown PAN Event Type\n"));
            break;
      }
   }
   else
      DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid PAN Event Data\n"));

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* process Device Manager (DEVM) Status Events (for                  */
   /* incoming/out-going connection management).                        */
static void ProcessDEVMStatusEvent(DEVM_Status_Type_t StatusType, BD_ADDR_t BD_ADDR, int Status)
{
   int                           Result;
   Connection_Entry_t           *ConnectionEntry;
   PAN_Open_Confirmation_Data_t  OpenConfirmationData;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, determine if we are tracking a connection to this device.  */
   if(((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, BD_ADDR)) != NULL) && (ConnectionEntry->ConnectionState != csConnected))
   {
      /* Determine if the connection is out-going.                      */
      if(ConnectionEntry->RemoteConnection)
      {
         /* Initialize common connection event members.                 */
         BTPS_MemInitialize(&OpenConfirmationData, 0, sizeof(PAN_Open_Confirmation_Data_t));

         if(Status)
         {
            /* Error, go ahead and disconnect the device.               */
            DEVM_DisconnectRemoteDevice(BD_ADDR, 0);

            /* Map the status to a known status.                        */
            switch(Status)
            {
               case BTPM_ERROR_CODE_DEVICE_AUTHENTICATION_FAILED:
               case BTPM_ERROR_CODE_DEVICE_ENCRYPTION_FAILED:
                  OpenConfirmationData.OpenStatus = PAN_OPEN_STATUS_CONNECTION_REFUSED;
                  break;
               case BTPM_ERROR_CODE_DEVICE_CONNECTION_FAILED:
               case BTPM_ERROR_CODE_DEVICE_CONNECTION_RETRIES_EXCEEDED:
                  OpenConfirmationData.OpenStatus = PAN_OPEN_STATUS_CONNECTION_TIMEOUT;
                  break;
               default:
                  OpenConfirmationData.OpenStatus = PAN_OPEN_STATUS_UNKNOWN_ERROR;
                  break;
            }

            /* Note the PAN ID of the connection.                       */
            OpenConfirmationData.PANID = ConnectionEntry->PANID;

            /* Go ahead and dispatch the event.                         */
            /* * NOTE * This function will notice the failure and will  */
            /*          remove the Connection Entry from the list.      */
            ProcessOpenConfirmationEvent(TRUE, &OpenConfirmationData);

            /* Flag that the connection had been deleted.               */
            ConnectionEntry = NULL;
         }
         else
         {
            /* Connection succeeded.                                    */

            /* Move to the connecting state.                            */
            ConnectionEntry->ConnectionState = csConnecting;

            if(((Result = _PANM_Open_Remote_Server(BD_ADDR, ConnectionEntry->LocalServiceType, ConnectionEntry->RemoteServiceType)) <= 0) && (Result != BTPAN_ERROR_CONNECTION_ALREADY_EXISTS))
            {
               /* Error opening the device.                             */
               DEVM_DisconnectRemoteDevice(BD_ADDR, 0);

               /* Format up a fake Connection failure event (this will  */
               /* clean up everything).                                 */
               OpenConfirmationData.OpenStatus = PAN_OPEN_STATUS_UNKNOWN_ERROR;
               OpenConfirmationData.PANID      = ConnectionEntry->PANID;

               ProcessOpenConfirmationEvent(TRUE, &OpenConfirmationData);

               /* Flag the connection had been deleted.                 */
               ConnectionEntry = NULL;
            }
            else
            {
               /* If the device is already connected, we will dispach   */
               /* the the Status only (note this case shouldn't really  */
               /* occur, but just to be safe we will clean up our state */
               /* machine).                                             */
               if(Result == BTPAN_ERROR_CONNECTION_ALREADY_EXISTS)
               {
                  ConnectionEntry->ConnectionState = csConnected;

                  OpenConfirmationData.OpenStatus  = 0;
                  OpenConfirmationData.PANID       = ConnectionEntry->PANID;

                  ProcessOpenConfirmationEvent(FALSE, &OpenConfirmationData);
               }
               else
                  ConnectionEntry->PANID = Result;
            }
         }
      }
      else
      {
         /* The connection must be incoming.                            */
         if(!Status)
         {
            /* Success, accept the connection.                          */
            _PANM_Open_Request_Response(ConnectionEntry->PANID, TRUE);
         }
         else
         {
            /* Failure, reject the connection.                          */
            _PANM_Open_Request_Response(ConnectionEntry->PANID, FALSE);

            /* Delete the Connection Entry.                             */
            if((ConnectionEntry = DeleteConnectionEntry(&ConnectionEntryList, BD_ADDR)) != NULL)
               FreeConnectionEntryMemory(ConnectionEntry);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process PAN Manager Asynchronous Events.            */
static void BTPSAPI BTPMDispatchCallback_PANM(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process PAN Manager Notification Events.            */
static void BTPSAPI BTPMDispatchCallback_PAN(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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
            /* Double check that this is an PAN Event Update.           */
            if(((PANM_Update_Data_t *)CallbackParameter)->UpdateType == utPANEvent)
            {
               /* Process the Notification.                             */
               ProcessPANEvent(&(((PANM_Update_Data_t *)CallbackParameter)->UpdateData.PANEventData));
            }

            /* Release the lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
      }

      /* Go ahead and free the memory because we are finished with it.  */
      BTPS_FreeMemory(CallbackParameter);
   }

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process Client Un-Register Asynchronous Events.     */
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function represents the Message Group Handler       */
   /* function that is installed to process all PAN Manager Messages.   */
static void BTPSAPI PANManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the Message specified appears to be semi-valid.   */
   if(Message)
   {
      /* Next, check to see if this message is specified for our        */
      /* specific Message Group.                                        */
      if(Message->MessageHeader.MessageGroup == BTPM_MESSAGE_GROUP_PAN_MANAGER)
      {
         DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("PAN Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

         /* Message is destined for us, make sure we are still          */
         /* processing messages.                                        */
         if(Initialized)
         {
            /* Check to see if this message is a PAN Manager defined    */
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
               /* PAN Manager Thread.                                   */
               if(!BTPM_QueueMailboxCallback(BTPMDispatchCallback_PANM, (void *)Message))
               {
                  DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue PAN Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

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
                        DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue PAN Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));
                     }
                  }
               }
            }
         }
         else
         {
            /* Check to see if this message is an PAN Manager defined   */
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
         DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("Non PAN Manager Message: 0x%08X, 0x%08X\n", Message->MessageHeader.MessageGroup, Message->MessageHeader.MessageFunction));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for initializing/cleaning up*/
   /* the Bluetopia Platform Manager PAN Manager Module.  This function */
   /* should be registered with the Bluetopia Platform Manager Module   */
   /* Handler and will be called when the Platform Manager is           */
   /* initialized (or shut down).                                       */
void BTPSAPI PANM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData)
{
   int Result;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %d\n", Initialize));

   /* First, determine if the request is to initialize or shut down.    */
   if(Initialize)
   {
      DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("Initialization Request\n"));

      if(!Initialized)
      {
         DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing PAN Manager\n"));

         /* Initialize success.                                         */
         Result = 0;

         /* Next, let's attempt to register our Message Group Handler to*/
         /* process PAN Manager messages.                               */
         if(!MSG_RegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_PAN_MANAGER, PANManagerGroupHandler, NULL))
         {
            /* Check to see if any initialization data was specified.   */
            if(InitializationData)
            {
               /* Initialize the actual PAN Manager Implementation      */
               /* Module (this is the module that is actually           */
               /* responsible for actually implementing the PAN Manager */
               /* functionality - this module is just the framework     */
               /* shell).                                               */
               if(!(Result = _PANM_Initialize((PANM_Initialization_Info_t *)InitializationData)))
               {
                  /* Save the current profile configuration.            */
                  ServiceTypeFlags        = (((PANM_Initialization_Info_t *)InitializationData)->ServiceTypeFlags & (PAN_PERSONAL_AREA_NETWORK_USER_SERVICE | PAN_NETWORK_ACCESS_POINT_SERVICE | PAN_GROUP_ADHOC_NETWORK_SERVICE));
                  IncomingConnectionFlags = ((PANM_Initialization_Info_t *)InitializationData)->IncomingConnectionFlags;

                  /* Determine the current Device Power State.          */
                  CurrentPowerState       = (Boolean_t)((DEVM_QueryDevicePowerState() > 0)?TRUE:FALSE);

                  /* Initialize a unique, starting PAN Callback ID.     */
                  NextCallbackID          = 0x000000001;

                  /* Go ahead and flag that this module is initialized. */
                  Initialized             = TRUE;
               }
            }
            else
               Result = BTPM_ERROR_CODE_INVALID_PAN_INITIALIZATION_DATA;
         }
         else
            Result = BTPM_ERROR_CODE_UNABLE_TO_REGISTER_HANDLER;

         /* If an error occurred then we need to free all resources that*/
         /* were allocated.                                             */
         if(Result)
         {
            _PANM_Cleanup();

            MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_PAN_MANAGER);
         }
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("PAN Manager Already initialized\n"));
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("Shutdown Request\n"));

      if(Initialized)
      {
         /* First, let's make sure that we un-register our Message Group*/
         /* Handler (so that we do not process any more incoming        */
         /* messages).                                                  */
         MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_PAN_MANAGER);

         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Make sure we inform the PAN Manager Implementation that  */
            /* we are shutting down.                                    */
            _PANM_Cleanup();

            /* Make sure the incoming Connection List is empty.         */
            FreeConnectionEntryList(&ConnectionEntryList);

            /* Make sure that the PAN Entry Information List is empty.  */
            FreePANEntryInfoList(&PANEntryInfoList);

            CurrentPowerState = FALSE;

            /* Flag that this module is no longer initialized.          */
            Initialized       = FALSE;

            /* Free the Lock we acquired earlier.                       */
            DEVM_ReleaseLock();
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing asynchronous */
   /* Device Manager (DEVM) Events (including Power On/Off events).     */
   /* This function should be registered with the Bluetopia Platform    */
   /* Manager Module Handler and will be called when an asynchronous    */
   /* Device Manager event is dispatched.                               */
void BTPSAPI PANM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData)
{
   int                 Result;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the PAN Manager has been initialized.   */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         switch(EventData->EventType)
         {
            case detDevicePoweredOn:
               DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power On Occurred.\n"));

               /* Power on Event.                                       */

               /* Note the new Power State.                             */
               CurrentPowerState = TRUE;

               /* Go ahead and inform the PAN Manager that it should    */
               /* initialize.                                           */
               if((Result = DEVM_QueryDeviceBluetoothStackID()) > 0)
                  _PANM_SetBluetoothStackID((unsigned int)Result);
               break;
            case detDevicePoweringOff:
            case detDevicePoweredOff:
               DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power Off Occurred.\n"));

               /* Note the new Power State.                             */
               CurrentPowerState = FALSE;

               /* Inform the PAN Manager that the Stack has been closed.*/
               _PANM_SetBluetoothStackID(0);

               /* Loop through all outgoing connections to determine if */
               /* there are any synchronous connections outstanding.    */
               ConnectionEntry = ConnectionEntryList;

               while(ConnectionEntry)
               {
                  /* Check to see if there is a synchronous open        */
                  /* operation.                                         */
                  if(ConnectionEntry->ConnectionEvent)
                  {
                     ConnectionEntry->ConnectionStatus = PANM_DEVICE_CONNECTION_STATUS_FAILURE_DEVICE_POWER_OFF;

                     BTPS_SetEvent(ConnectionEntry->ConnectionEvent);
                  }

                  ConnectionEntry = ConnectionEntry->NextConnectionEntryPtr;
               }

               /* Finally free all Connection Entries (as there cannot  */
               /* be any active connections).                           */
               FreeConnectionEntryList(&ConnectionEntryList);
               break;
            case detRemoteDeviceAuthenticationStatus:
               /* Authentication Status, process the Status Event.      */
               ProcessDEVMStatusEvent(dstAuthentication, EventData->EventData.RemoteDeviceAuthenticationStatusEventData.RemoteDeviceAddress, EventData->EventData.RemoteDeviceAuthenticationStatusEventData.Status);
               break;
            case detRemoteDeviceEncryptionStatus:
               /* Encryption Status, process the Status Event.          */
               ProcessDEVMStatusEvent(dstEncryption, EventData->EventData.RemoteDeviceEncryptionStatusEventData.RemoteDeviceAddress, EventData->EventData.RemoteDeviceEncryptionStatusEventData.Status);
               break;
            case detRemoteDeviceConnectionStatus:
               /* Connection Status, process the Status Event.          */
               ProcessDEVMStatusEvent(dstConnection, EventData->EventData.RemoteDeviceConnectionStatusEventData.RemoteDeviceAddress, EventData->EventData.RemoteDeviceConnectionStatusEventData.Status);
               break;
            default:
               /* Do nothing.                                           */
               break;
         }

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow the caller the ability*/
   /* to notify the PAN Manager of a specific Update Event. The PAN     */
   /* Manager can then take the correct action to process the update.   */
Boolean_t PANM_NotifyUpdate(PANM_Update_Data_t *UpdateData)
{
   Boolean_t ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, make sure that the input parameters appear to be           */
   /* semi-valid.                                                       */
   if(UpdateData)
   {
      switch(UpdateData->UpdateType)
      {
         case utPANEvent:
            DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("Queueing PAN Event: %d\n", UpdateData->UpdateData.PANEventData.EventType));

            /* Simply Queue the Dispatch Data.                          */
            /* * NOTE * To avoid copying again, we will simply submit   */
            /*          the entire Notification Message.                */
            ret_val = BTPM_QueueMailboxCallback(BTPMDispatchCallback_PAN, (void *)UpdateData);
            break;
         default:
            DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled Notification: %d\n", UpdateData->UpdateType));

            ret_val = FALSE;
            break;
      }
   }
   else
      ret_val = FALSE;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* module to respond to a request to connect to a local PAN server.  */
   /* This function returns zero if successful, or a negative return    */
   /* error code if there was an error.                                 */
   /* * NOTE * This function returning success does not necessarily     */
   /*          indicate that the connection has been successfully       */
   /*          opened. A petPANMConnected event will notifiy of this    */
   /*          status.                                                  */
int BTPSAPI PANM_Connection_Request_Response(BD_ADDR_t RemoteDeviceAddress, Boolean_t AcceptConnection)
{
   int                 ret_val;
   Boolean_t           Authenticate;
   Boolean_t           Encrypt;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure that the PAN Module is initialized.                     */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Next, check to see if we are powered up.                    */
         if(CurrentPowerState)
         {
            /* Device is powered on, next, verify that we are already   */
            /* tracking a connection for the specified device.          */
            if(((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress)) != NULL) && (ConnectionEntry->ConnectionState == csAuthorizing))
            {
               DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Respond to Request: %d\n", AcceptConnection));

               /* If the caller has accepted the request then we need to*/
               /* process it differently.                               */
               if(AcceptConnection)
               {
                  /* Determine if Authentication and/or Encryption is   */
                  /* required for this link.                            */
                  if(IncomingConnectionFlags & PANM_INCOMING_CONNECTION_FLAGS_REQUIRE_AUTHENTICATION)
                     Authenticate = TRUE;
                  else
                     Authenticate = FALSE;

                  if(IncomingConnectionFlags & PANM_INCOMING_CONNECTION_FLAGS_REQUIRE_ENCRYPTION)
                     Encrypt = TRUE;
                  else
                     Encrypt = FALSE;

                  if((Authenticate) || (Encrypt))
                  {
                     if(Encrypt)
                        ret_val = DEVM_EncryptRemoteDevice(ConnectionEntry->BD_ADDR, 0);
                     else
                        ret_val = DEVM_AuthenticateRemoteDevice(ConnectionEntry->BD_ADDR, 0);
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_DEVICE_ALREADY_AUTHENTICATED;

                  if((ret_val == BTPM_ERROR_CODE_DEVICE_ALREADY_AUTHENTICATED) || (ret_val == BTPM_ERROR_CODE_DEVICE_ALREADY_ENCRYPTED))
                  {
                     /* Authorization not required, and we are already  */
                     /* in the correct state.                           */
                     ret_val = _PANM_Open_Request_Response(ConnectionEntry->PANID, TRUE);

                     if(ret_val)
                     {
                        _PANM_Open_Request_Response(ConnectionEntry->PANID, FALSE);

                        /* Go ahead and delete the entry because we are */
                        /* finished with tracking it.                   */
                        if((ConnectionEntry = DeleteConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress)) != NULL)
                           FreeConnectionEntryMemory(ConnectionEntry);
                     }
                     else
                     {
                        /* Update the current connection state.         */
                        ConnectionEntry->ConnectionState = csConnecting;
                     }
                  }
                  else
                  {
                     /* If we were successfully able to Authenticate    */
                     /* and/or Encrypt, then we need to set the correct */
                     /* state.                                          */
                     if(!ret_val)
                     {
                        if(Encrypt)
                           ConnectionEntry->ConnectionState = csEncrypting;
                        else
                           ConnectionEntry->ConnectionState = csAuthenticating;

                        /* Flag success.                                */
                        ret_val = 0;
                     }
                     else
                     {
                        /* Error, reject the request.                   */
                        _PANM_Open_Request_Response(ConnectionEntry->PANID, FALSE);

                        /* Go ahead and delete the entry because we are */
                        /* finished with tracking it.                   */
                        if((ConnectionEntry = DeleteConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress)) != NULL)
                           FreeConnectionEntryMemory(ConnectionEntry);
                     }
                  }
               }
               else
               {
                  /* Rejection - Simply respond to the request.         */
                  ret_val = _PANM_Open_Request_Response(ConnectionEntry->PANID, FALSE);

                  /* Go ahead and delete the entry because we are       */
                  /* finished with tracking it.                         */
                  if((ConnectionEntry = DeleteConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress)) != NULL)
                     FreeConnectionEntryMemory(ConnectionEntry);
               }
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CONNECTION_STATE;
         }
         else
            ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      ret_val = BTPM_ERROR_CODE_PAN_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* module to request to open a remote PANM server connection.  This  */
   /* function returns zero if successful and a negative value if there */
   /* was an error.                                                     */
   /* * NOTE * The final parameter to this function, if specified,      */
   /*          instructs this function to block until the connection    */
   /*          status is received (i.e. the connection is completed).   */
   /*          If this parameter is not specified (i.e. NULL) then the  */
   /*          connection status will be returned asynchronously in the */
   /*          PANM Connection Status Event (if specified).             */
   /* * NOTE * The two callback parameters are optional, and if         */
   /*          specified, will be the Callback that the                 */
   /*          petPANMConnectionStatus event will be dispatched to      */
   /*          denote the status of the connection.  This is the ONLY   */
   /*          way to receive this event, as an event callack           */
   /*          registered with the PANM_Register_Event_Callback() will  */
   /*          NOT receive connection status events.                    */
int BTPSAPI PANM_Connect_Remote_Device(BD_ADDR_t RemoteDeviceAddress, PAN_Service_Type_t LocalServiceType, PAN_Service_Type_t RemoteServiceType, unsigned long ConnectionFlags, PANM_Event_Callback_t EventCallback, void *CallbackParameter, unsigned int *ConnectionStatus)
{
   int                 ret_val;
   Event_t             ConnectionEvent;
   Connection_Entry_t  ConnectionEntry;
   Connection_Entry_t *ConnectionEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure that the PAN Module is initialized.                     */
   if(Initialized)
   {
      /* Next, verify that the input parameters appear to be semi-valid.*/
      if(!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* Device is powered up, make sure we are not already    */
               /* connected to this device.                             */
               if((ConnectionEntryPtr = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress)) == NULL)
               {
                  /* No connection found, go ahead and format the entry */
                  /* to add.                                            */
                  BTPS_MemInitialize(&ConnectionEntry, 0, sizeof(Connection_Entry_t));

                  ConnectionEntry.BD_ADDR                        = RemoteDeviceAddress;
                  ConnectionEntry.PANID                          = GetNextCallbackID() | 0x80000000;
                  ConnectionEntry.ConnectionState                = csIdle;
                  ConnectionEntry.RemoteConnection               = TRUE;
                  ConnectionEntry.LocalServiceType               = LocalServiceType;
                  ConnectionEntry.RemoteServiceType              = RemoteServiceType;
                  ConnectionEntry.CallbackInfo.ClientID          = MSG_GetServerAddressID();
                  ConnectionEntry.CallbackInfo.EventCallback     = EventCallback;
                  ConnectionEntry.CallbackInfo.CallbackParameter = CallbackParameter;

                  if(ConnectionStatus)
                     ConnectionEntry.ConnectionEvent = BTPS_CreateEvent(FALSE);

                  if((!ConnectionStatus) || ((ConnectionStatus) && (ConnectionEntry.ConnectionEvent)))
                  {
                     if((ConnectionEntryPtr = AddConnectionEntry(&ConnectionEntryList, &ConnectionEntry)) != NULL)
                     {
                        DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to open remote device\n"));

                        /* Next, attempt to open the remote device.     */
                        if(ConnectionFlags & PANM_CONNECT_REMOTE_DEVICE_FLAGS_REQUIRE_ENCRYPTION)
                           ConnectionEntryPtr->ConnectionState = csEncrypting;
                        else
                        {
                           if(ConnectionFlags & PANM_CONNECT_REMOTE_DEVICE_FLAGS_REQUIRE_AUTHENTICATION)
                              ConnectionEntryPtr->ConnectionState = csAuthenticating;
                           else
                              ConnectionEntryPtr->ConnectionState = csIdle;
                        }

                        ret_val = DEVM_ConnectWithRemoteDevice(RemoteDeviceAddress, (ConnectionEntryPtr->ConnectionState == csIdle)?0:((ConnectionEntryPtr->ConnectionState == csEncrypting)?(DEVM_CONNECT_WITH_REMOTE_DEVICE_FLAGS_AUTHENTICATE | DEVM_CONNECT_WITH_REMOTE_DEVICE_FLAGS_ENCRYPT):DEVM_CONNECT_WITH_REMOTE_DEVICE_FLAGS_AUTHENTICATE));

                        if((ret_val == BTPM_ERROR_CODE_DEVICE_IS_CURRENTLY_CONNECTED) || (ret_val == BTPM_ERROR_CODE_DEVICE_ALREADY_AUTHENTICATED) || (ret_val == BTPM_ERROR_CODE_DEVICE_ALREADY_ENCRYPTED))
                        {
                           /* We are already connected, so attempt to   */
                           /* open a PAN Connection.                    */
                           if((ret_val = _PANM_Open_Remote_Server(RemoteDeviceAddress, LocalServiceType, RemoteServiceType)) > 0)
                           {
                              /* Open attempt successful, so note the   */
                              /* new state.                             */
                              ConnectionEntryPtr->PANID           = ret_val;
                              ConnectionEntryPtr->ConnectionState = csConnecting;

                              /* Flag success to the caller.            */
                              ret_val                             = 0;
                           }
                        }

                        /* If an error occurred, go ahead and delete the*/
                        /* Connection Information that was added.       */
                        if(ret_val)
                        {
                           if((ConnectionEntryPtr = DeleteConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress)) != NULL)
                              FreeConnectionEntryMemory(ConnectionEntryPtr);

                           if(ret_val == BTPAN_ERROR_CONNECTION_ALREADY_EXISTS)
                              ret_val = BTPM_ERROR_CODE_PAN_DEVICE_ALREADY_CONNECTED;
                           else
                              ret_val = BTPM_ERROR_CODE_PAN_UNABLE_TO_CONNECT_TO_DEVICE;
                        }

                        /* Next, determine if the caller has requested a*/
                        /* blocking up.                                 */
                        if((!ret_val) && (ConnectionStatus))
                        {
                           /* Blocking open, go ahead and wait for the  */
                           /* event.                                    */

                           /* Note the Open Event.                      */
                           ConnectionEvent = ConnectionEntryPtr->ConnectionEvent;

                           /* Release the lock because we are finished  */
                           /* with it.                                  */
                           DEVM_ReleaseLock();

                           BTPS_WaitEvent(ConnectionEvent, BTPS_INFINITE_WAIT);

                           /* Re-acquire the Lock.                      */
                           if(DEVM_AcquireLock())
                           {
                              if((ConnectionEntryPtr = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress)) != NULL)
                              {
                                 /* Note the connection status.         */
                                 *ConnectionStatus = ConnectionEntryPtr->ConnectionStatus;

                                 BTPS_CloseEvent(ConnectionEntryPtr->ConnectionEvent);

                                 if(*ConnectionStatus)
                                 {
                                    if((ConnectionEntryPtr = DeleteConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress)) != NULL)
                                       FreeConnectionEntryMemory(ConnectionEntryPtr);
                                 }
                              }
                              else
                                 ret_val = BTPM_ERROR_CODE_PAN_UNABLE_TO_CONNECT_TO_DEVICE;
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
               {
                  if(ConnectionEntryPtr->ConnectionState == csConnected)
                     ret_val = BTPM_ERROR_CODE_PAN_DEVICE_ALREADY_CONNECTED;
                  else
                     ret_val = BTPM_ERROR_CODE_PAN_CONNECTION_IN_PROGRESS;
               }
            }
            else
               ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

            DEVM_ReleaseLock();
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_PAN_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* module to close a previously opened connection.  This function    */
   /* returns zero if successful, or a negative return error code if    */
   /* there was an error.                                               */
   /* * NOTE * This function does not unregister a PAN server.  It only */
   /*          disconnects any currently active connection.             */
int BTPSAPI PANM_Close_Connection(BD_ADDR_t RemoteDeviceAddress)
{
   int                          ret_val;
   Boolean_t                    PerformDisconnect;
   Connection_Entry_t          *ConnectionEntry;
   PAN_Close_Indication_Data_t  CloseData;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure that the PAN Module is initialized.                     */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Next, check to see if we are powered up.                    */
         if(CurrentPowerState)
         {
            /* Make sure we have are tracking a connection.             */
            if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress)) != NULL)
            {
               switch(ConnectionEntry->ConnectionState)
               {
                  case csAuthorizing:
                  case csAuthenticating:
                  case csEncrypting:
                     /* Should not occur.                               */
                     PerformDisconnect = FALSE;
                     break;
                  default:
                  case csConnecting:
                  case csConnected:
                     PerformDisconnect = TRUE;
                     break;
               }

               if(PerformDisconnect)
               {
                  /* Go ahead and attempt to disconnect.                */
                  ret_val = _PANM_Close_Connection(ConnectionEntry->PANID);
               }
               else
                  ret_val = 0;

               CloseData.PANID = ConnectionEntry->PANID;
            }
            else
              ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         }
         else
            ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

         if(!ret_val)
         {
            /* Fake a Close Indication to notify clients of the         */
            /* disconnection.                                           */
            ProcessCloseIndicationEvent(&CloseData);
         }

         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      ret_val = BTPM_ERROR_CODE_PAN_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to determine if there are currently any connected Personal*/
   /* Area Networking devices.  This function accepts the buffer        */
   /* information to receive any currently connected devices.  The first*/
   /* parameter specifies the maximum number of BD_ADDR entries that the*/
   /* buffer will support (i.e. can be copied into the buffer).  The    */
   /* next parameter is optional and, if specified, will be populated   */
   /* with the total number of connected devices if the function is     */
   /* successful.  The final parameter can be used to retrieve the total*/
   /* number of connected devices (regardless of the size of the list   */
   /* specified by the first two parameters).  This function returns a  */
   /* non-negative value if successful which represents the number of   */
   /* connected devices that were copied into the specified input       */
   /* buffer.  This function returns a negative return error code if    */
   /* there was an error.                                               */
   /* * NOTE * It is possible to specify zero for the Maximum Number of */
   /*          Remote Device Entries, in which case the final parameter */
   /*          *MUST* be specified.                                     */
int BTPSAPI PANM_Query_Connected_Devices(unsigned int MaximumRemoteDeviceListEntries, BD_ADDR_t *RemoteDeviceAddressList, unsigned int *TotalNumberConnectedDevices)
{
   int                 ret_val;
   unsigned int        NumberConnected;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure that the PAN Module is initialized.                     */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(((MaximumRemoteDeviceListEntries) && (RemoteDeviceAddressList)) || ((!MaximumRemoteDeviceListEntries) && (TotalNumberConnectedDevices)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* Device is powered up, so now let's determine how many */
               /* devices are connected.                                */
               NumberConnected = 0;
               ConnectionEntry = ConnectionEntryList;

               while(ConnectionEntry)
               {
                  /* Note that we are only counting devices that are    */
                  /* either in the connected state or the connecting    */
                  /* state (i.e. have been authorized OR passed         */
                  /* authentication).                                   */
                  if((ConnectionEntry->ConnectionState == csConnected) || (ConnectionEntry->ConnectionState == csConnecting))
                     NumberConnected++;

                  ConnectionEntry = ConnectionEntry->NextConnectionEntryPtr;
               }

               /* We now have the total number of devices that will     */
               /* satisy the query.                                     */
               if(TotalNumberConnectedDevices)
                  *TotalNumberConnectedDevices = NumberConnected;

               /* Now that have the total, we need to build the         */
               /* Connected Device List.                                */

               /* See if the caller would like to copy some (or all) of */
               /* the list.                                             */
               if(MaximumRemoteDeviceListEntries)
               {
                  /* If there are more entries in the returned list than*/
                  /* the buffer specified, we need to truncate it.      */
                  if(MaximumRemoteDeviceListEntries >= NumberConnected)
                     MaximumRemoteDeviceListEntries = NumberConnected;

                  NumberConnected = 0;

                  ConnectionEntry = ConnectionEntryList;

                  while((ConnectionEntry) && (NumberConnected < MaximumRemoteDeviceListEntries))
                  {
                     /* Note that we are only counting devices that are */
                     /* counting devices that either in the connected   */
                     /* state or the connecting state (i.e. have been   */
                     /* authorized OR passed authentication).           */
                     if((ConnectionEntry->ConnectionState == csConnected) || (ConnectionEntry->ConnectionState == csConnecting))
                        RemoteDeviceAddressList[NumberConnected++] = ConnectionEntry->BD_ADDR;

                     ConnectionEntry = ConnectionEntry->NextConnectionEntryPtr;
                  }

                  /* Note the total number of devices that were copied  */
                  /* into the array.                                    */
                  ret_val = (int)NumberConnected;
               }
               else
                  ret_val = 0;
            }
            else
               ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

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
      ret_val = BTPM_ERROR_CODE_PAN_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to query the current configuration for the Personal Area  */
   /* Networking (PAN) Manager.  This function returns zero if          */
   /* successful, or a negative return error code if there was an error.*/
int BTPSAPI PANM_Query_Current_Configuration(PANM_Current_Configuration_t *CurrentConfiguration)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure that the PAN Module is initialized.                     */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(CurrentConfiguration)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            CurrentConfiguration->ServiceTypeFlags        = ServiceTypeFlags;
            CurrentConfiguration->IncomingConnectionFlags = IncomingConnectionFlags;

            ret_val                                       = 0;

            DEVM_ReleaseLock();
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_PAN_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* module to change the Incoming Connection Flags. This function     */
   /* returns zero if successful, or a negative return error code if    */
   /* there was an error.                                               */
int BTPSAPI PANM_Change_Incoming_Connection_Flags(unsigned long ConnectionFlags)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure that the PAN Module is initialized.                     */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Simply change the flags.                                    */
         IncomingConnectionFlags = ConnectionFlags;

         ret_val                 = 0;

         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      ret_val = BTPM_ERROR_CODE_PAN_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the Personal Area    */
   /* Network (PAN) Manager Service. This Callback will be dispatched   */
   /* by the PAN Manager when various PAN Manager Events occur. This    */
   /* function accepts the Callback Function and Callback Parameter     */
   /* (respectively) to call when a PAN Manager Event needs to be       */
   /* dispatched. This function returns a positive (non-zero) value if  */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * If this function returns success (greater than zero)     */
   /*          then this value can be passed to the                     */
   /*          PANM_Un_Register_Event_Callback() function to un-register*/
   /*          the callback from this module.                           */
int BTPSAPI PANM_Register_Event_Callback(PANM_Event_Callback_t CallbackFunction, void *CallbackParameter)
{
   int              ret_val;
   PAN_Entry_Info_t PANEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the PAN Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(CallbackFunction)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Attempt to add an entry into the PAN Entry list.         */
            BTPS_MemInitialize(&PANEntryInfo, 0, sizeof(PAN_Entry_Info_t));

            PANEntryInfo.CallbackID         = GetNextCallbackID();
            PANEntryInfo.ClientID           = MSG_GetServerAddressID();
            PANEntryInfo.EventCallback      = CallbackFunction;
            PANEntryInfo.CallbackParameter  = CallbackParameter;

            if(AddPANEntryInfoEntry(&PANEntryInfoList, &PANEntryInfo))
               ret_val = PANEntryInfo.CallbackID;
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
      ret_val = BTPM_ERROR_CODE_PAN_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered PAN Manager Event Callback.   */
   /* This function accepts as input the PAN Manager Event Callback ID  */
   /* (return value from PANM_Register_Event_Callback() function).      */
void BTPSAPI PANM_Un_Register_Event_Callback(unsigned int PANManagerCallbackID)
{
   PAN_Entry_Info_t *PANEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the PAN Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(PANManagerCallbackID)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            if((PANEntryInfo = DeletePANEntryInfoEntry(&PANEntryInfoList, PANManagerCallbackID)) != NULL)
            {
               /* Free the memory because we are finished with it.      */
               FreePANEntryInfoEntryMemory(PANEntryInfo);
            }

            /* Release the Lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_PAN | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

