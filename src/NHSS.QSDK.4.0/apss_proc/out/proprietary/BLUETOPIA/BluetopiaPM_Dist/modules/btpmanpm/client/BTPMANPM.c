/*****< btpmanpm.c >***********************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMANPM - ANP Manager for Stonestreet One Bluetooth Protocol Stack       */
/*             Platform Manager.                                              */
/*                                                                            */
/*  Author:  Ryan Byrne                                                       */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/06/12  R. Byrne       Initial creation.                               */
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

   /* Structure which is used to track Callback Information related to  */
   /* this module.                                                      */
typedef struct _tagANP_Entry_Info_t
{
   unsigned int                 CallbackID;
   unsigned long                Flags;
   BD_ADDR_t                    BD_ADDR;
   ANPM_Event_Callback_t        EventCallback;
   void                        *CallbackParameter;
   struct _tagANP_Entry_Info_t *NextANPEntryInfoPtr;
} ANP_Entry_Info_t;

   /* The following functions are used with the Flags member of the     */
   /* ANP_Entry_Info_t structure to denote various state information.   */
#define ANP_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY           0x40000000

   /* Structure which is used to track outstanding transactions and     */
   /* their respective callback.                                        */
typedef struct _tagTransaction_Entry_t
{
   unsigned int                    TransactionID;
   unsigned int                    CallbackID;
   unsigned int                    ServerTransactionID;
   struct _tagTransaction_Entry_t *NextTransactionEntryPtr;
} Transaction_Entry_t;

   /* Container structure used when dispatching registered callbacks.   */
typedef struct _tagCallbackInfo_t
{
   ANPM_Event_Callback_t  EventCallback;
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
static Mutex_t ANPManagerMutex;

   /* Variable which is used to hold the next (unique) Callback ID.     */
static unsigned int NextCallbackID;

   /* Variable which is used to hold the next (unique) Transaction ID.  */
static unsigned int NextTransactionID;

   /* Variable which holds the current Power State of the Device.       */
static Boolean_t CurrentPowerState;

   /* Variable which holds the a boolean if ANP events are registered.  */
static Boolean_t ANPEventsRegistered;

   /* Variable which holds the CallbackID returned from the PM server   */
   /* when registering for events.                                      */
static unsigned int ServerCallbackID;

   /* Variable which holds a pointer to the first element in the ANP    */
   /* Entry Information List (which holds all Callbacks tracked by this */
   /* module).                                                          */
static ANP_Entry_Info_t *ANPEntryInfoList;
static ANP_Entry_Info_t *ANPClientEntryInfoList;

   /* Vaiable which holds a pointer to the first element in the         */
   /* transaction list.                                                 */
static Transaction_Entry_t *TransactionEntryList;

   /* Internal Function Prototypes.                                     */
static unsigned int GetNextCallbackID(void);
static unsigned int GetNextTransactionID(void);

static ANP_Entry_Info_t *AddANPEntryInfoEntry(ANP_Entry_Info_t **ListHead, ANP_Entry_Info_t *EntryToAdd);
static ANP_Entry_Info_t *SearchANPEntryInfoEntry(ANP_Entry_Info_t **ListHead, unsigned int CallbackID);
static ANP_Entry_Info_t *DeleteANPEntryInfoEntry(ANP_Entry_Info_t **ListHead, unsigned int CallbackID);
static void FreeANPEntryInfoEntryMemory(ANP_Entry_Info_t *EntryToFree);
static void FreeANPEntryInfoList(ANP_Entry_Info_t **ListHead);

static Transaction_Entry_t *AddTransactionEntry(Transaction_Entry_t **ListHead, Transaction_Entry_t *EntryToAdd);
static Transaction_Entry_t *DeleteTransactionEntry(Transaction_Entry_t **ListHead, unsigned int TransactionID);
static Transaction_Entry_t *DeleteTransactionEntryByServerID(Transaction_Entry_t **ListHead, unsigned int ServerTransactionID);
static void FreeTransactionEntryMemory(Transaction_Entry_t *EntryToFree);
static void FreeTransactionEntryList(Transaction_Entry_t **ListHead);

static void DispatchANPEvent(ANPM_Event_Data_t *ANPMEventData, Boolean_t Server);
static void DispatchANPEventByServerTransactionID(ANPM_Event_Data_t *ANPMEventData, unsigned int ServerTransactionID, unsigned int *TransactionIDField);

static void ProcessANPConnected(ANPM_Connected_Message_t *Message);
static void ProcessANPDisconnected(ANPM_Disconnected_Message_t *Message);
static void ProcessANPNewAlertCategoryEnabledEvent(ANPM_New_Alert_Category_Enabled_Message_t *Message);
static void ProcessANPNewAlertCategoryDisabledEvent(ANPM_New_Alert_Category_Disabled_Message_t *Message);
static void ProcessANPUnReadAlertCategoryEnabledEvent(ANPM_Un_Read_Alert_Category_Enabled_Message_t *Message);
static void ProcessANPUnReadAlertCategoryDisabledEvent(ANPM_Un_Read_Alert_Category_Disabled_Message_t *Message);

static void ProcessSupportedNewAlertCategoriesResultEvent(ANPM_Supported_New_Alert_Categories_Result_Message_t *Message);
static void ProcessSupportedUnreadCategoriesResultEvent(ANPM_Supported_Unread_Categories_Result_Message_t *Message);
static void ProcessNewAlertNotificationEvent(ANPM_New_Alert_Notification_Message_t *Message);
static void ProcessUnreadStatusNotificationEvent(ANPM_Unread_Status_Notification_Message_t *Message);
static void ProcessCommandResultEvent(ANPM_Command_Result_Message_t *Message);

static void ProcessReceivedMessage(BTPM_Message_t *Message);

static void BTPSAPI BTPMDispatchCallback_ANPM(void *CallbackParameter);
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter);

static void BTPSAPI ANPManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter);

   /* The following function is a utility function that exists to       */
   /* retrieve a unique Callback ID that can be used to add an entry    */
   /* into the ANP Entry Information List.                              */
static unsigned int GetNextCallbackID(void)
{
   unsigned int ret_val;

   ret_val = NextCallbackID++;

   if((!NextCallbackID) || (NextCallbackID & 0x80000000))
      NextCallbackID = 0x00000001;

   return(ret_val);
}

   /* The following function is a utility function that exists to       */
   /* retrieve a unique Transaction ID that can be used to add an entry */
   /* into the Transaction List.                                        */
static unsigned int GetNextTransactionID(void)
{
   unsigned int ret_val;

   ret_val = NextTransactionID++;

   if((!NextTransactionID) || (NextTransactionID & 0x80000000))
      NextTransactionID = 0x00000001;

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
static ANP_Entry_Info_t *AddANPEntryInfoEntry(ANP_Entry_Info_t **ListHead, ANP_Entry_Info_t *EntryToAdd)
{
   ANP_Entry_Info_t *AddedEntry = NULL;
   ANP_Entry_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First let's verify that values passed in are semi-valid.          */
   if((ListHead) && (EntryToAdd))
   {
      /* Make sure that the element that we are adding seems semi-valid.*/
      if(EntryToAdd->CallbackID)
      {
         /* OK, data seems semi-valid, let's allocate a new data        */
         /* structure to add to the list.                               */
         AddedEntry = (ANP_Entry_Info_t *)BTPS_AllocateMemory(sizeof(ANP_Entry_Info_t));

         if(AddedEntry)
         {
            /* Copy All Data over.                                      */
            *AddedEntry                     = *EntryToAdd;

            /* Now Add it to the end of the list.                       */
            AddedEntry->NextANPEntryInfoPtr = NULL;

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
                     FreeANPEntryInfoEntryMemory(AddedEntry);
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
                     if(tmpEntry->NextANPEntryInfoPtr)
                        tmpEntry = tmpEntry->NextANPEntryInfoPtr;
                     else
                        break;
                  }
               }

               if(AddedEntry)
               {
                  /* Last element found, simply Add the entry.          */
                  tmpEntry->NextANPEntryInfoPtr = AddedEntry;
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
   /* specified Callback ID.  This function returns NULL if either the  */
   /* List Head is invalid, the Callback ID is invalid, or the specified*/
   /* Callback ID was NOT found.                                        */
static ANP_Entry_Info_t *SearchANPEntryInfoEntry(ANP_Entry_Info_t **ListHead, unsigned int CallbackID)
{
   ANP_Entry_Info_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", CallbackID));

   /* Let's make sure the list and Callback ID to search for appear to  */
   /* be valid.                                                         */
   if((ListHead) && (CallbackID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->CallbackID != CallbackID))
         FoundEntry = FoundEntry->NextANPEntryInfoPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* The following function searches the specified ANP Entry           */
   /* Information List for the specified Callback ID and removes it from*/
   /* the List.  This function returns NULL if either the ANP Entry     */
   /* Information List Head is invalid, the Callback ID is invalid, or  */
   /* the specified Callback ID was NOT present in the list.  The entry */
   /* returned will have the Next Entry field set to NULL, and the      */
   /* caller is responsible for deleting the memory associated with this*/
   /* entry by calling FreeANPEntryInfoEntryMemory().                   */
static ANP_Entry_Info_t *DeleteANPEntryInfoEntry(ANP_Entry_Info_t **ListHead, unsigned int CallbackID)
{
   ANP_Entry_Info_t *FoundEntry = NULL;
   ANP_Entry_Info_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", CallbackID));

   /* Let's make sure the List and Callback ID to search for appear to  */
   /* be semi-valid.                                                    */
   if((ListHead) && (CallbackID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->CallbackID != CallbackID))
      {
         LastEntry  = FoundEntry;
         FoundEntry = FoundEntry->NextANPEntryInfoPtr;
      }

      /* Check to see if we found the specified entry.                  */
      if(FoundEntry)
      {
         /* OK, now let's remove the entry from the list.  We have to   */
         /* check to see if the entry was the first entry in the list.  */
         if(LastEntry)
         {
            /* Entry was NOT the first entry in the list.               */
            LastEntry->NextANPEntryInfoPtr = FoundEntry->NextANPEntryInfoPtr;
         }
         else
            *ListHead = FoundEntry->NextANPEntryInfoPtr;

         FoundEntry->NextANPEntryInfoPtr = NULL;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* This function frees the specified ANP Entry Information member.   */
   /* No check is done on this entry other than making sure it NOT NULL.*/
static void FreeANPEntryInfoEntryMemory(ANP_Entry_Info_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToFree)
      BTPS_FreeMemory(EntryToFree);

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified ANP Entry Information List.  Upon return */
   /* of this function, the Head Pointer is set to NULL.                */
static void FreeANPEntryInfoList(ANP_Entry_Info_t **ListHead)
{
   ANP_Entry_Info_t *EntryToFree;
   ANP_Entry_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ListHead)
   {
      /* Simply traverse the list and free every element present.       */
      EntryToFree = *ListHead;

      while(EntryToFree)
      {
         tmpEntry    = EntryToFree;
         EntryToFree = EntryToFree->NextANPEntryInfoPtr;

         FreeANPEntryInfoEntryMemory(tmpEntry);
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
   /*            TransactionID field is the same as an entry already in */
   /*            the list.  When this occurs, this function returns     */
   /*            NULL.                                                  */
static Transaction_Entry_t *AddTransactionEntry(Transaction_Entry_t **ListHead, Transaction_Entry_t *EntryToAdd)
{
   Transaction_Entry_t *AddedEntry = NULL;
   Transaction_Entry_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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
            *AddedEntry                     = *EntryToAdd;

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", AddedEntry));

   return(AddedEntry);
}

   /* The following function searches the specified Transaction List    */
   /* for the specified Transaction ID and removes it from the List.    */
   /* This function returns NULL if either the Transaction List Head    */
   /* is invalid, the Transaction ID is invalid, or the specified       */
   /* Transaction ID was NOT present in the list.  The entry returned   */
   /* will have the Next Entry field set to NULL, and the caller is     */
   /* responsible for deleting the memory associated with this entry by */
   /* calling FreeTransactionEntryMemory().                             */
static Transaction_Entry_t *DeleteTransactionEntry(Transaction_Entry_t **ListHead, unsigned int TransactionID)
{
   Transaction_Entry_t *FoundEntry = NULL;
   Transaction_Entry_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", TransactionID));

   /* Let's make sure the List and Transaction ID to search for appear  */
   /* to be semi-valid.                                                 */
   if((ListHead) && (TransactionID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->TransactionID != TransactionID))
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

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* The following function searches the specified Transaction List    */
   /* for the specified Transaction ID and removes it from the List.    */
   /* This function returns NULL if either the Transaction List Head    */
   /* is invalid, the Transaction ID is invalid, or the specified       */
   /* Transaction ID was NOT present in the list.  The entry returned   */
   /* will have the Next Entry field set to NULL, and the caller is     */
   /* responsible for deleting the memory associated with this entry by */
   /* calling FreeTransactionEntryMemory().                             */
static Transaction_Entry_t *DeleteTransactionEntryByServerID(Transaction_Entry_t **ListHead, unsigned int ServerTransactionID)
{
   Transaction_Entry_t *FoundEntry = NULL;
   Transaction_Entry_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", ServerTransactionID));

   /* Let's make sure the List and Transaction ID to search for appear  */
   /* to be semi-valid.                                                 */
   if((ListHead) && (ServerTransactionID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->ServerTransactionID != ServerTransactionID))
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

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* This function frees the specified Transaction member.  No check is*/
   /* done on this entry other than making sure it NOT NULL.            */
static void FreeTransactionEntryMemory(Transaction_Entry_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToFree)
      BTPS_FreeMemory(EntryToFree);

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified Transaction List.  Upon return of this   */
   /* function, the Head Pointer is set to NULL.                        */
static void FreeTransactionEntryList(Transaction_Entry_t **ListHead)
{
   Transaction_Entry_t *EntryToFree;
   Transaction_Entry_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch the specified ANP event to every registered ANP Event    */
   /* Callback.                                                         */
   /* * NOTE * This function should be called with the ANP Manager Mutex*/
   /*          held.  Upon exit from this function it will free the ANP */
   /*          Manager Mutex.                                           */
static void DispatchANPEvent(ANPM_Event_Data_t *ANPMEventData, Boolean_t Server)
{
   unsigned int      Index;
   unsigned int      NumberCallbacks;
   CallbackInfo_t    CallbackInfoArray[16];
   CallbackInfo_t   *CallbackInfoArrayPtr;
   ANP_Entry_Info_t *ANPEntryInfo;
   ANP_Entry_Info_t *ListHead;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   ListHead = Server?ANPEntryInfoList:ANPClientEntryInfoList;

   /* Make sure that the input parameters appear to be semi-valid.      */
   if((ListHead) && (ANPMEventData))
   {
      /* Next, let's determine how many callbacks are registered.       */
      ANPEntryInfo    = ListHead;
      NumberCallbacks = 0;

      /* First, add the default event handlers.                         */
      while(ANPEntryInfo)
      {
         if((ANPEntryInfo->EventCallback) && (ANPEntryInfo->Flags & ANP_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY))
            NumberCallbacks++;

         ANPEntryInfo = ANPEntryInfo->NextANPEntryInfoPtr;
      }

      if(NumberCallbacks)
      {
         if(NumberCallbacks <= (sizeof(CallbackInfoArray)/sizeof(CallbackInfo_t)))
            CallbackInfoArrayPtr = CallbackInfoArray;
         else
            CallbackInfoArrayPtr = BTPS_AllocateMemory((NumberCallbacks*sizeof(CallbackInfo_t)));

         /* Make sure that we have memory to copy the Callback List     */
         /* into.                                                       */
         if(CallbackInfoArrayPtr)
         {
            ANPEntryInfo    = ListHead;
            NumberCallbacks = 0;

            /* First, add the default event handlers.                   */
            while(ANPEntryInfo)
            {
               if((ANPEntryInfo->EventCallback) && (ANPEntryInfo->Flags & ANP_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY))
               {
                  CallbackInfoArrayPtr[NumberCallbacks].EventCallback     = ANPEntryInfo->EventCallback;
                  CallbackInfoArrayPtr[NumberCallbacks].CallbackParameter = ANPEntryInfo->CallbackParameter;

                  NumberCallbacks++;
               }

               ANPEntryInfo = ANPEntryInfo->NextANPEntryInfoPtr;
            }

            /* Release the Mutex because we have already built the      */
            /* Callback Array.                                          */
            BTPS_ReleaseMutex(ANPManagerMutex);

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
                  {
                     (*CallbackInfoArrayPtr[Index].EventCallback)(ANPMEventData, CallbackInfoArrayPtr[Index].CallbackParameter);
                  }
               }
               __BTPSEXCEPT(1)
               {
                  /* Do Nothing.                                        */
               }

               Index++;
            }

            /* Free any memory that was allocated.                      */
            if(CallbackInfoArrayPtr != CallbackInfoArray)
               BTPS_FreeMemory(CallbackInfoArrayPtr);
         }
         else
         {
            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(ANPManagerMutex);
         }
      }
      else
      {
         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(ANPManagerMutex);
      }
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(ANPManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch the specified ANP event to a single registered ANP Event */
   /* Callback mapped by the completion of a transaction.               */
   /* * NOTE * This function should be called with the ANP Manager Mutex*/
   /*          held.  Upon exit from this function it will free the ANP */
   /*          Manager Mutex.                                           */
static void DispatchANPEventByServerTransactionID(ANPM_Event_Data_t *ANPMEventData, unsigned int ServerTransactionID, unsigned int *TransactionIDField)
{
   void                  *CallbackParameter;
   ANP_Entry_Info_t      *ANPEntryInfo;
   Transaction_Entry_t   *TransactionEntry;
   ANPM_Event_Callback_t  CallbackFunction;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the supplied Event Data is valid.                       */
   if((ANPMEventData) && (TransactionIDField))
   {
      /* Attempt to look up the transaction to get the callback ID.     */
      if((TransactionEntry = DeleteTransactionEntryByServerID(&TransactionEntryList, ServerTransactionID)) != NULL)
      {
         /* Attempt to find the callback information from the           */
         /* transaction.                                                */
         if((ANPEntryInfo = SearchANPEntryInfoEntry(&ANPClientEntryInfoList, TransactionEntry->CallbackID)) != NULL)
         {
            /* Note the callback information so we can release the lock.*/
            CallbackFunction  = ANPEntryInfo->EventCallback;
            CallbackParameter = ANPEntryInfo->CallbackParameter;

            /* Update the Transaction ID with the locally generated one.*/
            *TransactionIDField = TransactionEntry->TransactionID;

            /* Now release the lock to make the callback.               */
            BTPS_ReleaseMutex(ANPManagerMutex);

            __BTPSTRY
            {
               if(CallbackFunction)
               {
                  (*CallbackFunction)(ANPMEventData, CallbackParameter);
               }
            }
            __BTPSEXCEPT(1)
            {
               /* Do Nothing.                                        */
            }
         }
         else
            BTPS_ReleaseMutex(ANPManagerMutex);

         FreeTransactionEntryMemory(TransactionEntry);
      }
      else
         BTPS_ReleaseMutex(ANPManagerMutex);
   }
   else
      BTPS_ReleaseMutex(ANPManagerMutex);

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the ANP      */
   /* Client Connected asynchronous message.                            */
static void ProcessANPConnected(ANPM_Connected_Message_t *Message)
{
   ANPM_Event_Data_t ANPMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   ANPMEventData.EventType                                        = aetANPConnected;
   ANPMEventData.EventLength                                      = ANPM_CONNECTED_EVENT_DATA_SIZE;

   ANPMEventData.EventData.ConnectedEventData.ConnectionType      = Message->ConnectionType;
   ANPMEventData.EventData.ConnectedEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;

   /* Now that the event is formatted, dispatch it to both client and   */
   /* server callbacks.                                                 */
   DispatchANPEvent(&ANPMEventData, TRUE);
   DispatchANPEvent(&ANPMEventData, FALSE);

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the ANP      */
   /* Client Disconnected asynchronous message.                         */
static void ProcessANPDisconnected(ANPM_Disconnected_Message_t *Message)
{
   ANPM_Event_Data_t ANPMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   ANPMEventData.EventType                                           = aetANPDisconnected;
   ANPMEventData.EventLength                                         = ANPM_DISCONNECTED_EVENT_DATA_SIZE;

   ANPMEventData.EventData.DisconnectedEventData.ConnectionType      = Message->ConnectionType;
   ANPMEventData.EventData.DisconnectedEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;

   /* Now that the event is formatted, dispatch it to both client and   */
   /* server callbacks.                                                 */
   DispatchANPEvent(&ANPMEventData, TRUE);
   DispatchANPEvent(&ANPMEventData, FALSE);

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the ANP New  */
   /* Alert Category Enabled asynchronous message.                      */
static void ProcessANPNewAlertCategoryEnabledEvent(ANPM_New_Alert_Category_Enabled_Message_t *Message)
{
   ANPM_Event_Data_t ANPMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   ANPMEventData.EventType                                                      = aetANPNewAlertCategoryEnabled;
   ANPMEventData.EventLength                                                    = ANPM_NEW_ALERT_CATEGORY_ENABLED_MESSAGE_SIZE;

   ANPMEventData.EventData.NewAlertCategoryEnabledEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;
   ANPMEventData.EventData.NewAlertCategoryEnabledEventData.CategoryEnabled     = Message->CategoryEnabled;
   ANPMEventData.EventData.NewAlertCategoryEnabledEventData.EnabledCategories   = Message->EnabledCategories;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchANPEvent(&ANPMEventData, TRUE);

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the ANP New  */
   /* Alert Category Disabled asynchronous message.                     */
static void ProcessANPNewAlertCategoryDisabledEvent(ANPM_New_Alert_Category_Disabled_Message_t *Message)
{
   ANPM_Event_Data_t ANPMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   ANPMEventData.EventType                                                       = aetANPNewAlertCategoryDisabled;
   ANPMEventData.EventLength                                                     = ANPM_NEW_ALERT_CATEGORY_DISABLED_MESSAGE_SIZE;

   ANPMEventData.EventData.NewAlertCategoryDisabledEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;
   ANPMEventData.EventData.NewAlertCategoryDisabledEventData.CategoryDisabled    = Message->CategoryDisabled;
   ANPMEventData.EventData.NewAlertCategoryDisabledEventData.EnabledCategories   = Message->EnabledCategories;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchANPEvent(&ANPMEventData, TRUE);

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the ANP Un   */
   /* Read Alert Category Enabled asynchronous message.                 */
static void ProcessANPUnReadAlertCategoryEnabledEvent(ANPM_Un_Read_Alert_Category_Enabled_Message_t *Message)
{
   ANPM_Event_Data_t ANPMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   ANPMEventData.EventType                                                         = aetANPUnReadAlertCategoryEnabled;
   ANPMEventData.EventLength                                                       = ANPM_UN_READ_ALERT_CATEGORY_ENABLED_MESSAGE_SIZE;

   ANPMEventData.EventData.UnReadAlertCategoryEnabledEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;
   ANPMEventData.EventData.UnReadAlertCategoryEnabledEventData.CategoryEnabled     = Message->CategoryEnabled;
   ANPMEventData.EventData.UnReadAlertCategoryEnabledEventData.EnabledCategories   = Message->EnabledCategories;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchANPEvent(&ANPMEventData, TRUE);

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the ANP Un   */
   /* Read Alert Category Disabled asynchronous message.                */
static void ProcessANPUnReadAlertCategoryDisabledEvent(ANPM_Un_Read_Alert_Category_Disabled_Message_t *Message)
{
   ANPM_Event_Data_t ANPMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   ANPMEventData.EventType                                                          = aetANPUnReadAlertCategoryDisabled;
   ANPMEventData.EventLength                                                        = ANPM_UN_READ_ALERT_CATEGORY_DISABLED_MESSAGE_SIZE;

   ANPMEventData.EventData.UnReadAlertCategoryDisabledEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;
   ANPMEventData.EventData.UnReadAlertCategoryDisabledEventData.CategoryDisabled    = Message->CategoryDisabled;
   ANPMEventData.EventData.UnReadAlertCategoryDisabledEventData.EnabledCategories   = Message->EnabledCategories;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchANPEvent(&ANPMEventData, TRUE);

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Supported*/
   /* New Alert Categories Result asynchronous message.                 */
static void ProcessSupportedNewAlertCategoriesResultEvent(ANPM_Supported_New_Alert_Categories_Result_Message_t *Message)
{
   ANPM_Event_Data_t ANPMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   ANPMEventData.EventType                                                           = aetANPSupportedNewAlertCategoriesResult;
   ANPMEventData.EventLength                                                         = ANPM_SUPPORTED_NEW_ALERT_CATEGORIES_RESULT_EVENT_DATA_SIZE;

   ANPMEventData.EventData.SupportedNewAlertCategoriesEventData.RemoteDeviceAddress  = Message->RemoteDeviceAddress;
   ANPMEventData.EventData.SupportedNewAlertCategoriesEventData.Status               = Message->Status;
   ANPMEventData.EventData.SupportedNewAlertCategoriesEventData.AttProtocolErrorCode = Message->AttProtocolErrorCode;
   ANPMEventData.EventData.SupportedNewAlertCategoriesEventData.SupportedCategories  = Message->SupportedCategories;

   /* Dispatch the event to the callback mapped by the transaction ID.  */
   DispatchANPEventByServerTransactionID(&ANPMEventData, Message->TransactionID, &ANPMEventData.EventData.SupportedNewAlertCategoriesEventData.TransactionID);

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Supported*/
   /* Unread Alert Status Categories Result asynchronous message.       */
static void ProcessSupportedUnreadCategoriesResultEvent(ANPM_Supported_Unread_Categories_Result_Message_t *Message)
{
   ANPM_Event_Data_t ANPMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   ANPMEventData.EventType                                                         = aetANPSupportedUnreadCategoriesResult;
   ANPMEventData.EventLength                                                       = ANPM_SUPPORTED_UNREAD_CATEGORIES_RESULT_EVENT_DATA_SIZE;

   ANPMEventData.EventData.SupportedUnreadCategoriesEventData.RemoteDeviceAddress  = Message->RemoteDeviceAddress;
   ANPMEventData.EventData.SupportedUnreadCategoriesEventData.Status               = Message->Status;
   ANPMEventData.EventData.SupportedUnreadCategoriesEventData.AttProtocolErrorCode = Message->AttProtocolErrorCode;
   ANPMEventData.EventData.SupportedUnreadCategoriesEventData.SupportedCategories  = Message->SupportedCategories;

   /* Dispatch the event to the callback mapped by the transaction ID.  */
   DispatchANPEventByServerTransactionID(&ANPMEventData, Message->TransactionID, &ANPMEventData.EventData.SupportedUnreadCategoriesEventData.TransactionID);

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the New Alert*/
   /* Notification asynchronous message.                                */
static void ProcessNewAlertNotificationEvent(ANPM_New_Alert_Notification_Message_t *Message)
{
   ANPM_Event_Data_t ANPMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   ANPMEventData.EventType                                                   = aetANPNewAlertNotification;
   ANPMEventData.EventLength                                                 = ANPM_NEW_ALERT_NOTIFICATION_EVENT_DATA_SIZE;

   ANPMEventData.EventData.NewAlertNotificationEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;
   ANPMEventData.EventData.NewAlertNotificationEventData.CategoryID          = Message->CategoryID;
   ANPMEventData.EventData.NewAlertNotificationEventData.NumberNewAlerts     = Message->NumberNewAlerts;
   ANPMEventData.EventData.NewAlertNotificationEventData.LastAlertText       = (Message->LastAlertTextLength)?Message->LastAlertText:NULL;

   /* Dispatch the event to all client callbacks.                       */
   DispatchANPEvent(&ANPMEventData, FALSE);

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Unread   */
   /* Status Notification asynchronous message.                         */
static void ProcessUnreadStatusNotificationEvent(ANPM_Unread_Status_Notification_Message_t *Message)
{
   ANPM_Event_Data_t ANPMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   ANPMEventData.EventType                                                       = aetANPUnreadStatusNotification;
   ANPMEventData.EventLength                                                     = ANPM_UNREAD_STATUS_NOTIFICATION_EVENT_DATA_SIZE;

   ANPMEventData.EventData.UnreadStatusNotificationEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;
   ANPMEventData.EventData.UnreadStatusNotificationEventData.CategoryID          = Message->CategoryID;
   ANPMEventData.EventData.UnreadStatusNotificationEventData.NumberUnreadAlerts  = Message->NumberUnreadAlerts;

   /* Dispatch the event to all client callbacks.                       */
   DispatchANPEvent(&ANPMEventData, FALSE);

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Command  */
   /* Result asynchronous message.                                      */
static void ProcessCommandResultEvent(ANPM_Command_Result_Message_t *Message)
{
   ANPM_Event_Data_t ANPMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   ANPMEventData.EventType                                             = aetANPCommandResult;
   ANPMEventData.EventLength                                           = ANPM_COMMAND_RESULT_EVENT_DATA_SIZE;

   ANPMEventData.EventData.CommandResultEventData.RemoteDeviceAddress  = Message->RemoteDeviceAddress;
   ANPMEventData.EventData.CommandResultEventData.Status               = Message->Status;
   ANPMEventData.EventData.CommandResultEventData.AttProtocolErrorCode = Message->AttProtocolErrorCode;

   /* Dispatch the event to the callback mapped by the transaction ID.  */
   DispatchANPEventByServerTransactionID(&ANPMEventData, Message->TransactionID, &ANPMEventData.EventData.CommandResultEventData.TransactionID);

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is called from the Main Bluetopia Dispatch */
   /* thread in response to a Process Received Message.  This function  */
   /* SHOULD ONLY be called in this capacity and NOT under any other    */
   /* circumstances.                                                    */
   /* * NOTE * This function *MUST* be called with the ANP Manager Mutex*/
   /*          held.  This function will release the Mutex before it    */
   /*          exits (i.e. the caller SHOULD NOT RELEASE THE MUTEX).    */
static void ProcessReceivedMessage(BTPM_Message_t *Message)
{
   Boolean_t ReleaseMutex = TRUE;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", Message->MessageHeader.MessageFunction));

   /* Check to make sure this isn't some kind of spoofed response (we do*/
   /* not process responses).                                           */
   if((Message) && (!(Message->MessageHeader.MessageID & BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK)))
   {
      /* Request received, go ahead and process it.                     */
      switch(Message->MessageHeader.MessageFunction)
      {
         case ANPM_MESSAGE_FUNCTION_CONNECTED:
            DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("ANP Connection Established\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANPM_CONNECTED_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Connected Event.                                      */
               ProcessANPConnected((ANPM_Connected_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case ANPM_MESSAGE_FUNCTION_DISCONNECTED:
            DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("ANP Disconnect\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANPM_DISCONNECTED_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Disconnected Event.                                   */
               ProcessANPDisconnected((ANPM_Disconnected_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case ANPM_MESSAGE_FUNCTION_NEW_ALERT_CATEGORY_ENABLED:
            DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("New Alert Category Enabled Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANPM_NEW_ALERT_CATEGORY_ENABLED_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Device Connection Status Event.                       */
               ProcessANPNewAlertCategoryEnabledEvent((ANPM_New_Alert_Category_Enabled_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case ANPM_MESSAGE_FUNCTION_NEW_ALERT_CATEGORY_DISABLED:
            DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("New Alert Category Disabled Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANPM_NEW_ALERT_CATEGORY_DISABLED_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Device Disconnected Event.                            */
               ProcessANPNewAlertCategoryDisabledEvent((ANPM_New_Alert_Category_Disabled_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case ANPM_MESSAGE_FUNCTION_UN_READ_ALERT_CATEGORY_ENABLED:
            DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Un Read Alert Category Enabled Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANPM_UN_READ_ALERT_CATEGORY_ENABLED_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Device Connection Status Event.                       */
               ProcessANPUnReadAlertCategoryEnabledEvent((ANPM_Un_Read_Alert_Category_Enabled_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case ANPM_MESSAGE_FUNCTION_UN_READ_ALERT_CATEGORY_DISABLED:
            DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Un Read Alert Category Disabled Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANPM_UN_READ_ALERT_CATEGORY_DISABLED_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Device Disconnected Event.                            */
               ProcessANPUnReadAlertCategoryDisabledEvent((ANPM_Un_Read_Alert_Category_Disabled_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case ANPM_MESSAGE_FUNCTION_SUPPORTED_NEW_ALERT_CATEGORIES_RESULT:
            DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Supported New Alert Categories Result Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANPM_SUPPORTED_NEW_ALERT_CATEGORIES_RESULT_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Event.                                                */
               ProcessSupportedNewAlertCategoriesResultEvent((ANPM_Supported_New_Alert_Categories_Result_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case ANPM_MESSAGE_FUNCTION_SUPPORTED_UNREAD_CATEGORIES_RESULT:
            DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Supported Unread Categories Result Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANPM_SUPPORTED_UNREAD_CATEGORIES_RESULT_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Event.                                                */
               ProcessSupportedUnreadCategoriesResultEvent((ANPM_Supported_Unread_Categories_Result_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case ANPM_MESSAGE_FUNCTION_NEW_ALERT_NOTIFICATION:
            DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("New Alert Notification Message\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANPM_NEW_ALERT_NOTIFICATION_MESSAGE_SIZE(0)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANPM_NEW_ALERT_NOTIFICATION_MESSAGE_SIZE(((ANPM_New_Alert_Notification_Message_t *)Message)->LastAlertTextLength)))
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Event.                                                */
               ProcessNewAlertNotificationEvent((ANPM_New_Alert_Notification_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case ANPM_MESSAGE_FUNCTION_UNREAD_STATUS_NOTIFICATION:
            DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Unread Status Notification Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANPM_UNREAD_STATUS_NOTIFICATION_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Event.                                                */
               ProcessUnreadStatusNotificationEvent((ANPM_Unread_Status_Notification_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case ANPM_MESSAGE_FUNCTION_COMMAND_RESULT:
            DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Command Result Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= ANPM_COMMAND_RESULT_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Event.                                                */
               ProcessCommandResultEvent((ANPM_Command_Result_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
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

   /* If we need to Release the Mutex then we will do it at this time.  */
   if(ReleaseMutex)
      BTPS_ReleaseMutex(ANPManagerMutex);

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
         /* Attempt to wait for access to the ANP state information.    */
         if(BTPS_WaitMutex(ANPManagerMutex, BTPS_INFINITE_WAIT))
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

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process Server un-register asynchronous events.     */
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
         /* Attempt to wait for access to the ANP state information.    */
         if(BTPS_WaitMutex(ANPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Free the Event Callback List.                            */
            FreeANPEntryInfoList(&ANPEntryInfoList);
            FreeANPEntryInfoList(&ANPClientEntryInfoList);

            /* Free the Transaction list.                               */
            FreeTransactionEntryList(&TransactionEntryList);

            /* Flag that we are not registered for events.              */
            ANPEventsRegistered = FALSE;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(ANPManagerMutex);
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
               /* Dispatch to the main handler that the server has      */
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
         DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing ANP Manager\n"));

         /* First, let's create a Mutex to guard access to this module. */
         if((ANPManagerMutex = BTPS_CreateMutex(FALSE)) != NULL)
         {
            /* Next, let's attempt to register our Message Group Handler*/
            /* to process ANP Manager messages.                         */
            if(!MSG_RegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_ALERT_NOTIFICATION_MANAGER, ANPManagerGroupHandler, NULL))
            {
               /* Initialize the actual ANP Manager Implementation      */
               /* Module (this is the module that is actually           */
               /* responsible for actually implementing the ANP Manager */
               /* functionality - this module is just the framework     */
               /* shell).                                               */
               if(!(Result = _ANPM_Initialize()))
               {
                  /* Note the current Power State.                      */
                  CurrentPowerState = (Boolean_t)((DEVM_QueryDevicePowerState() > 0)?TRUE:FALSE);

                  /* Go ahead and register with the ANP Manager Server. */
                  Result = _ANPM_Register_ANP_Events();

                  if(!Result)
                  {
                     /* Flag that ANP Events have been registered.      */
                     ANPEventsRegistered = TRUE;

                     /* Initialize a unique starting Callback ID.       */
                     NextCallbackID    = 0x000000001;
                     NextTransactionID = 0x000000001;

                     /* Go ahead and flag that this module is           */
                     /* initialized.                                    */
                     Initialized         = TRUE;
                  }
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
            if(ANPEventsRegistered)
               _ANPM_Un_Register_ANP_Events();

            if(ANPManagerMutex)
               BTPS_CloseMutex(ANPManagerMutex);

            MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_ALERT_NOTIFICATION_MANAGER);

            /* Flag that none of the resources are allocated.           */
            ANPManagerMutex     = NULL;
            ANPEventsRegistered = FALSE;
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

         /* Wait for access to the Mutex that guards access to this     */
         /* module.                                                     */
         if(BTPS_WaitMutex(ANPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Un-Register for ANP Events.                              */
            if(ANPEventsRegistered)
               _ANPM_Un_Register_ANP_Events();

            if(ServerCallbackID)
               _ANPM_Un_Register_Client_Event_Callback(ServerCallbackID);

            ServerCallbackID = 0;

            /* Make sure we inform the ANP Manager Implementation that  */
            /* we are shutting down.                                    */
            _ANPM_Cleanup();

            BTPS_CloseMutex(ANPManagerMutex);

            /* Make sure that the ANP Entry Information List is empty.  */
            FreeANPEntryInfoList(&ANPEntryInfoList);
            FreeANPEntryInfoList(&ANPClientEntryInfoList);

            /* Free the transaction list.                               */
            FreeTransactionEntryList(&TransactionEntryList);

            /* Flag that the resources are no longer allocated.         */
            ANPManagerMutex     = NULL;
            CurrentPowerState   = FALSE;
            ANPEventsRegistered = 0;

            /* Flag that this module is no longer initialized.          */
            Initialized         = FALSE;
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
   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANP Manager has been initialized.   */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(BTPS_WaitMutex(ANPManagerMutex, BTPS_INFINITE_WAIT))
      {
         switch(EventData->EventType)
         {
            case detDevicePoweredOn:
               DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power On Occurred.\n"));

               /* Note the new Power State.                             */
               CurrentPowerState = TRUE;
               break;
            case detDevicePoweringOff:
            case detDevicePoweredOff:
               DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power Off Occurred.\n"));

               /* Note the new Power State.                             */
               CurrentPowerState   = FALSE;
               break;
            default:
               /* Do nothing.                                           */
               break;
         }

         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(ANPManagerMutex);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
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

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: Category = %d, Count = %u, Text = \"%s\"\n", CategoryID, NewAlertCount, LastAlertText));

   /* First, check to make sure the module has been initialized.        */
   if(Initialized)
   {
      /* Nothing to do here other than to call the actual function to   */
      /* set the alert.                                                 */
      ret_val = _ANPM_Set_New_Alert(CategoryID, NewAlertCount, LastAlertText);
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

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: Category = %d, Count = %u\n", CategoryID, UnReadAlertCount));

   /* First, check to make sure the module has been initialized.        */
   if(Initialized)
   {
      /* Nothing to do here other than to call the actual function to   */
      /* set the alert.                                                 */
      ret_val = _ANPM_Set_Un_Read_Alert(CategoryID, UnReadAlertCount);
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
   int              ret_val;
   ANP_Entry_Info_t ANPEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANP Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(CallbackFunction)
      {
         /* Attempt to wait for access to the ANP Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(ANPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Attempt to add an entry into the ANP Entry list.         */
            BTPS_MemInitialize(&ANPEntryInfo, 0, sizeof(ANP_Entry_Info_t));

            ANPEntryInfo.CallbackID         = GetNextCallbackID();
            ANPEntryInfo.EventCallback      = CallbackFunction;
            ANPEntryInfo.CallbackParameter  = CallbackParameter;
            ANPEntryInfo.Flags              = ANP_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY;

            if(AddANPEntryInfoEntry(&ANPEntryInfoList, &ANPEntryInfo))
               ret_val = ANPEntryInfo.CallbackID;
            else
               ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(ANPManagerMutex);
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
   ANP_Entry_Info_t *ANPEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANP Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(ANPManagerCallbackID)
      {
         /* Attempt to wait for access to the ANP Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(ANPManagerMutex, BTPS_INFINITE_WAIT))
         {
            if((ANPEntryInfo = DeleteANPEntryInfoEntry(&ANPEntryInfoList, ANPManagerCallbackID)) != NULL)
            {
               /* Free the memory because we are finished with it.      */
               FreeANPEntryInfoEntryMemory(ANPEntryInfo);
            }

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(ANPManagerMutex);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* Alert Notification Client API definitions. */

   /* This functions submits a request to a remote ANP Server to get the*/
   /* supported New Alert Categories.  The ClientCallbackID parameter   */
   /* should be an ID return from ANPM_Register_Client_Event_Callback().*/
   /* The RemoteDeviceAddress parameter is the Bluetooth Address of the */
   /* remote server.  This function returns zero if successful or a     */
   /* negative return error code if there was an error.                 */
   /* * NOTE * An aetANPSupportedNewAlertCategoriesResult event will be */
   /*          dispatched when this request completes.                  */
int BTPSAPI ANPM_Get_Supported_New_Alert_Categories(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress)
{
   int                  ret_val;
   ANP_Entry_Info_t    *ANPEntryInfo;
   Transaction_Entry_t  TransactionEntry;
   Transaction_Entry_t *TransactionEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANP Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear semi-valid.   */
      if((ClientCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Attempt to wait for access to the ANP Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(ANPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Check that the supplied callback is registered.          */
            if((ANPEntryInfo = SearchANPEntryInfoEntry(&ANPClientEntryInfoList, ClientCallbackID)) != NULL)
            {
               /* Format the transaction entry to track the callback.   */
               BTPS_MemInitialize(&TransactionEntry, 0, sizeof(TransactionEntry));

               TransactionEntry.TransactionID = GetNextTransactionID();
               TransactionEntry.CallbackID    = ClientCallbackID;

               /* Add the transaction to the list.                      */
               if((TransactionEntryPtr = AddTransactionEntry(&TransactionEntryList, &TransactionEntry)) != NULL)
               {
                  /* Now submit the message to the server.              */
                  if((ret_val = _ANPM_Get_Supported_Categories(ServerCallbackID, RemoteDeviceAddress, ntNewAlert)) > 0)
                  {
                     /* Note the Transaction ID from the server.        */
                     TransactionEntryPtr->ServerTransactionID = (unsigned int)ret_val;
                     ret_val                                  = TransactionEntryPtr->TransactionID;
                  }
                  else
                  {
                     /* No transaction was submitted, so remove the     */
                     /* entry.                                          */
                     if((TransactionEntryPtr = DeleteTransactionEntry(&TransactionEntryList, TransactionEntryPtr->TransactionID)) != NULL)
                        FreeTransactionEntryMemory(TransactionEntryPtr);
                  }
               }
               else
                  ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
            }
            else
               ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_INVALID_CALLBACK_ID;

            BTPS_ReleaseMutex(ANPManagerMutex);
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
   int                  ret_val;
   ANP_Entry_Info_t    *ANPEntryInfo;
   Transaction_Entry_t  TransactionEntry;
   Transaction_Entry_t *TransactionEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANP Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear semi-valid.   */
      if((ClientCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Attempt to wait for access to the ANP Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(ANPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Check that the supplied callback is registered.          */
            if((ANPEntryInfo = SearchANPEntryInfoEntry(&ANPClientEntryInfoList, ClientCallbackID)) != NULL)
            {
               /* Format the transaction entry to track the callback.   */
               BTPS_MemInitialize(&TransactionEntry, 0, sizeof(TransactionEntry));

               TransactionEntry.TransactionID = GetNextTransactionID();
               TransactionEntry.CallbackID    = ClientCallbackID;

               /* Add the transaction to the list.                      */
               if((TransactionEntryPtr = AddTransactionEntry(&TransactionEntryList, &TransactionEntry)) != NULL)
               {
                  /* Now submit the message to the server.              */
                  if((ret_val = _ANPM_Enable_Disable_Notifications(ServerCallbackID, RemoteDeviceAddress, ntNewAlert, TRUE)) > 0)
                  {
                     /* Note the Transaction ID from the server.        */
                     TransactionEntryPtr->ServerTransactionID = (unsigned int)ret_val;
                     ret_val                                  = TransactionEntryPtr->TransactionID;
                  }
                  else
                  {
                     /* No transaction was submitted, so remove the     */
                     /* entry.                                          */
                     if((TransactionEntryPtr = DeleteTransactionEntry(&TransactionEntryList, TransactionEntryPtr->TransactionID)) != NULL)
                        FreeTransactionEntryMemory(TransactionEntryPtr);
                  }
               }
               else
                  ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
            }
            else
               ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_INVALID_CALLBACK_ID;

            BTPS_ReleaseMutex(ANPManagerMutex);
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
   int               ret_val;
   ANP_Entry_Info_t *ANPEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANP Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear semi-valid.   */
      if((ClientCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Attempt to wait for access to the ANP Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(ANPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Check that the supplied callback is registered.          */
            if((ANPEntryInfo = SearchANPEntryInfoEntry(&ANPClientEntryInfoList, ClientCallbackID)) != NULL)
            {
               /* Now submit the message to the server.                 */
               ret_val = _ANPM_Enable_Disable_Notifications(ServerCallbackID, RemoteDeviceAddress, ntNewAlert, FALSE);
            }
            else
               ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_INVALID_CALLBACK_ID;

            BTPS_ReleaseMutex(ANPManagerMutex);
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
   /* function returns zero if successful or a negative return error    */
   /* code if there was an error.                                       */
   /* * NOTE * An aetANPSupportedUnreadCategoriesResult event will be   */
   /*          dispatched when this request completes.                  */
int BTPSAPI ANPM_Get_Supported_Unread_Status_Categories(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress)
{
   int                  ret_val;
   ANP_Entry_Info_t    *ANPEntryInfo;
   Transaction_Entry_t  TransactionEntry;
   Transaction_Entry_t *TransactionEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANP Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear semi-valid.   */
      if((ClientCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Attempt to wait for access to the ANP Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(ANPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Check that the supplied callback is registered.          */
            if((ANPEntryInfo = SearchANPEntryInfoEntry(&ANPClientEntryInfoList, ClientCallbackID)) != NULL)
            {
               /* Format the transaction entry to track the callback.   */
               BTPS_MemInitialize(&TransactionEntry, 0, sizeof(TransactionEntry));

               TransactionEntry.TransactionID = GetNextTransactionID();
               TransactionEntry.CallbackID    = ClientCallbackID;

               /* Add the transaction to the list.                      */
               if((TransactionEntryPtr = AddTransactionEntry(&TransactionEntryList, &TransactionEntry)) != NULL)
               {
                  /* Now submit the message to the server.              */
                  if((ret_val = _ANPM_Get_Supported_Categories(ServerCallbackID, RemoteDeviceAddress, ntUnreadStatus)) > 0)
                  {
                     /* Note the Transaction ID from the server.        */
                     TransactionEntryPtr->ServerTransactionID = (unsigned int)ret_val;
                     ret_val                                  = TransactionEntryPtr->TransactionID;
                  }
                  else
                  {
                     /* No transaction was submitted, so remove the     */
                     /* entry.                                          */
                     if((TransactionEntryPtr = DeleteTransactionEntry(&TransactionEntryList, TransactionEntryPtr->TransactionID)) != NULL)
                        FreeTransactionEntryMemory(TransactionEntryPtr);
                  }
               }
               else
                  ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
            }
            else
               ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_INVALID_CALLBACK_ID;

            BTPS_ReleaseMutex(ANPManagerMutex);
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
   int                  ret_val;
   ANP_Entry_Info_t    *ANPEntryInfo;
   Transaction_Entry_t  TransactionEntry;
   Transaction_Entry_t *TransactionEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANP Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear semi-valid.   */
      if((ClientCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Attempt to wait for access to the ANP Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(ANPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Check that the supplied callback is registered.          */
            if((ANPEntryInfo = SearchANPEntryInfoEntry(&ANPClientEntryInfoList, ClientCallbackID)) != NULL)
            {
               /* Format the transaction entry to track the callback.   */
               BTPS_MemInitialize(&TransactionEntry, 0, sizeof(TransactionEntry));

               TransactionEntry.TransactionID = GetNextTransactionID();
               TransactionEntry.CallbackID    = ClientCallbackID;

               /* Add the transaction to the list.                      */
               if((TransactionEntryPtr = AddTransactionEntry(&TransactionEntryList, &TransactionEntry)) != NULL)
               {
                  /* Now submit the message to the server.              */
                  if((ret_val = _ANPM_Enable_Disable_Notifications(ServerCallbackID, RemoteDeviceAddress, ntUnreadStatus, TRUE)) > 0)
                  {
                     /* Note the Transaction ID from the server.        */
                     TransactionEntryPtr->ServerTransactionID = (unsigned int)ret_val;
                     ret_val                                  = TransactionEntryPtr->TransactionID;
                  }
                  else
                  {
                     /* No transaction was submitted, so remove the     */
                     /* entry.                                          */
                     if((TransactionEntryPtr = DeleteTransactionEntry(&TransactionEntryList, TransactionEntryPtr->TransactionID)) != NULL)
                        FreeTransactionEntryMemory(TransactionEntryPtr);
                  }
               }
               else
                  ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
            }
            else
               ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_INVALID_CALLBACK_ID;

            BTPS_ReleaseMutex(ANPManagerMutex);
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
   int               ret_val;
   ANP_Entry_Info_t *ANPEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANP Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear semi-valid.   */
      if((ClientCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Attempt to wait for access to the ANP Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(ANPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Check that the supplied callback is registered.          */
            if((ANPEntryInfo = SearchANPEntryInfoEntry(&ANPClientEntryInfoList, ClientCallbackID)) != NULL)
            {
               /* Now submit the message to the server.                 */
               ret_val = _ANPM_Enable_Disable_Notifications(ServerCallbackID, RemoteDeviceAddress, ntUnreadStatus, FALSE);
            }
            else
               ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_INVALID_CALLBACK_ID;

            BTPS_ReleaseMutex(ANPManagerMutex);
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
   int                  ret_val;
   ANP_Entry_Info_t    *ANPEntryInfo;
   Transaction_Entry_t  TransactionEntry;
   Transaction_Entry_t *TransactionEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANP Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear semi-valid.   */
      if((ClientCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Attempt to wait for access to the ANP Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(ANPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Check that the supplied callback is registered.          */
            if((ANPEntryInfo = SearchANPEntryInfoEntry(&ANPClientEntryInfoList, ClientCallbackID)) != NULL)
            {
               /* Format the transaction entry to track the callback.   */
               BTPS_MemInitialize(&TransactionEntry, 0, sizeof(TransactionEntry));

               TransactionEntry.TransactionID = GetNextTransactionID();
               TransactionEntry.CallbackID    = ClientCallbackID;

               /* Add the transaction to the list.                      */
               if((TransactionEntryPtr = AddTransactionEntry(&TransactionEntryList, &TransactionEntry)) != NULL)
               {
                  /* Now submit the message to the server.              */
                  if((ret_val = _ANPM_Enable_Disable_Category(ServerCallbackID, RemoteDeviceAddress, CategoryID, ntNewAlert, TRUE)) > 0)
                  {
                     /* Note the Transaction ID from the server.        */
                     TransactionEntryPtr->ServerTransactionID = (unsigned int)ret_val;
                     ret_val                                  = TransactionEntryPtr->TransactionID;
                  }
                  else
                  {
                     /* No transaction was submitted, so remove the     */
                     /* entry.                                          */
                     if((TransactionEntryPtr = DeleteTransactionEntry(&TransactionEntryList, TransactionEntryPtr->TransactionID)) != NULL)
                        FreeTransactionEntryMemory(TransactionEntryPtr);
                  }
               }
               else
                  ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
            }
            else
               ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_INVALID_CALLBACK_ID;

            BTPS_ReleaseMutex(ANPManagerMutex);
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
   int               ret_val;
   ANP_Entry_Info_t *ANPEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANP Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear semi-valid.   */
      if((ClientCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Attempt to wait for access to the ANP Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(ANPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Check that the supplied callback is registered.          */
            if((ANPEntryInfo = SearchANPEntryInfoEntry(&ANPClientEntryInfoList, ClientCallbackID)) != NULL)
            {
               /* Now submit the message to the server.                 */
               ret_val = _ANPM_Enable_Disable_Category(ServerCallbackID, RemoteDeviceAddress, CategoryID, ntNewAlert, FALSE);
            }
            else
               ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_INVALID_CALLBACK_ID;

            BTPS_ReleaseMutex(ANPManagerMutex);
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
   int                  ret_val;
   ANP_Entry_Info_t    *ANPEntryInfo;
   Transaction_Entry_t  TransactionEntry;
   Transaction_Entry_t *TransactionEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANP Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear semi-valid.   */
      if((ClientCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Attempt to wait for access to the ANP Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(ANPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Check that the supplied callback is registered.          */
            if((ANPEntryInfo = SearchANPEntryInfoEntry(&ANPClientEntryInfoList, ClientCallbackID)) != NULL)
            {
               /* Format the transaction entry to track the callback.   */
               BTPS_MemInitialize(&TransactionEntry, 0, sizeof(TransactionEntry));

               TransactionEntry.TransactionID = GetNextTransactionID();
               TransactionEntry.CallbackID    = ClientCallbackID;

               /* Add the transaction to the list.                      */
               if((TransactionEntryPtr = AddTransactionEntry(&TransactionEntryList, &TransactionEntry)) != NULL)
               {
                  /* Now submit the message to the server.              */
                  if((ret_val = _ANPM_Enable_Disable_Category(ServerCallbackID, RemoteDeviceAddress, CategoryID, ntUnreadStatus, TRUE)) > 0)
                  {
                     /* Note the Transaction ID from the server.        */
                     TransactionEntryPtr->ServerTransactionID = (unsigned int)ret_val;
                     ret_val                                  = TransactionEntryPtr->TransactionID;
                  }
                  else
                  {
                     /* No transaction was submitted, so remove the     */
                     /* entry.                                          */
                     if((TransactionEntryPtr = DeleteTransactionEntry(&TransactionEntryList, TransactionEntryPtr->TransactionID)) != NULL)
                        FreeTransactionEntryMemory(TransactionEntryPtr);
                  }
               }
               else
                  ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
            }
            else
               ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_INVALID_CALLBACK_ID;

            BTPS_ReleaseMutex(ANPManagerMutex);
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
   int               ret_val;
   ANP_Entry_Info_t *ANPEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANP Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear semi-valid.   */
      if((ClientCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Attempt to wait for access to the ANP Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(ANPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Check that the supplied callback is registered.          */
            if((ANPEntryInfo = SearchANPEntryInfoEntry(&ANPClientEntryInfoList, ClientCallbackID)) != NULL)
            {
               /* Now submit the message to the server.                 */
               ret_val = _ANPM_Enable_Disable_Category(ServerCallbackID, RemoteDeviceAddress, CategoryID, ntUnreadStatus, FALSE);
            }
            else
               ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_INVALID_CALLBACK_ID;

            BTPS_ReleaseMutex(ANPManagerMutex);
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
   int                  ret_val;
   ANP_Entry_Info_t    *ANPEntryInfo;
   Transaction_Entry_t  TransactionEntry;
   Transaction_Entry_t *TransactionEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANP Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear semi-valid.   */
      if((ClientCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Attempt to wait for access to the ANP Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(ANPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Check that the supplied callback is registered.          */
            if((ANPEntryInfo = SearchANPEntryInfoEntry(&ANPClientEntryInfoList, ClientCallbackID)) != NULL)
            {
               /* Format the transaction entry to track the callback.   */
               BTPS_MemInitialize(&TransactionEntry, 0, sizeof(TransactionEntry));

               TransactionEntry.TransactionID = GetNextTransactionID();
               TransactionEntry.CallbackID    = ClientCallbackID;

               /* Add the transaction to the list.                      */
               if((TransactionEntryPtr = AddTransactionEntry(&TransactionEntryList, &TransactionEntry)) != NULL)
               {
                  /* Now submit the message to the server.              */
                  if((ret_val = _ANPM_Request_Notification(ServerCallbackID, RemoteDeviceAddress, CategoryID, ntNewAlert)) > 0)
                  {
                     /* Note the Transaction ID from the server.        */
                     TransactionEntryPtr->ServerTransactionID = (unsigned int)ret_val;
                     ret_val                                  = TransactionEntryPtr->TransactionID;
                  }
                  else
                  {
                     /* No transaction was submitted, so remove the     */
                     /* entry.                                          */
                     if((TransactionEntryPtr = DeleteTransactionEntry(&TransactionEntryList, TransactionEntryPtr->TransactionID)) != NULL)
                        FreeTransactionEntryMemory(TransactionEntryPtr);
                  }
               }
               else
                  ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
            }
            else
               ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_INVALID_CALLBACK_ID;

            BTPS_ReleaseMutex(ANPManagerMutex);
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
   int                  ret_val;
   ANP_Entry_Info_t    *ANPEntryInfo;
   Transaction_Entry_t  TransactionEntry;
   Transaction_Entry_t *TransactionEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANP Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear semi-valid.   */
      if((ClientCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Attempt to wait for access to the ANP Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(ANPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Check that the supplied callback is registered.          */
            if((ANPEntryInfo = SearchANPEntryInfoEntry(&ANPClientEntryInfoList, ClientCallbackID)) != NULL)
            {
               /* Format the transaction entry to track the callback.   */
               BTPS_MemInitialize(&TransactionEntry, 0, sizeof(TransactionEntry));

               TransactionEntry.TransactionID = GetNextTransactionID();
               TransactionEntry.CallbackID    = ClientCallbackID;

               /* Add the transaction to the list.                      */
               if((TransactionEntryPtr = AddTransactionEntry(&TransactionEntryList, &TransactionEntry)) != NULL)
               {
                  /* Now submit the message to the server.              */
                  if((ret_val = _ANPM_Request_Notification(ServerCallbackID, RemoteDeviceAddress, CategoryID, ntUnreadStatus)) > 0)
                  {
                     /* Note the Transaction ID from the server.        */
                     TransactionEntryPtr->ServerTransactionID = (unsigned int)ret_val;
                     ret_val                                  = TransactionEntryPtr->TransactionID;
                  }
                  else
                  {
                     /* No transaction was submitted, so remove the     */
                     /* entry.                                          */
                     if((TransactionEntryPtr = DeleteTransactionEntry(&TransactionEntryList, TransactionEntryPtr->TransactionID)) != NULL)
                        FreeTransactionEntryMemory(TransactionEntryPtr);
                  }
               }
               else
                  ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
            }
            else
               ret_val = BTPM_ERROR_CODE_ALERT_NOTIFICATION_INVALID_CALLBACK_ID;

            BTPS_ReleaseMutex(ANPManagerMutex);
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
   int               ret_val;
   ANP_Entry_Info_t  ANPEntryInfo;
   ANP_Entry_Info_t *ANPEntryInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANP Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear semi-valid.   */
      if(CallbackFunction)
      {
         /* Attempt to wait for access to the ANP Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(ANPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Format the entry and add it to the list.                 */
            BTPS_MemInitialize(&ANPEntryInfo, 0, sizeof(ANPEntryInfo));

            ANPEntryInfo.CallbackID        = GetNextCallbackID();
            ANPEntryInfo.EventCallback     = CallbackFunction;
            ANPEntryInfo.CallbackParameter = CallbackParameter;
            ANPEntryInfo.Flags             = ANP_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY;

            if((ANPEntryInfoPtr = AddANPEntryInfoEntry(&ANPClientEntryInfoList, &ANPEntryInfo)) != NULL)
            {
               /* Check if we need to register with the PM server.      */
               if(!ServerCallbackID)
               {
                  /* Attempt to register.                               */
                  if((ret_val = _ANPM_Register_Client_Event_Callback()) > 0)
                  {
                     /* Note the server Callback ID and return the      */
                     /* locally generated ID.                           */
                     ServerCallbackID = (unsigned int)ret_val;
                     ret_val          = ANPEntryInfoPtr->CallbackID;
                  }
                  else
                  {
                     /* We failed to register with the PM server, so    */
                     /* delete the entry we added.                      */
                     if((ANPEntryInfoPtr = DeleteANPEntryInfoEntry(&ANPClientEntryInfoList, ANPEntryInfoPtr->CallbackID)) != NULL)
                        FreeANPEntryInfoEntryMemory(ANPEntryInfoPtr);
                  }
               }
               else
               {
                  /* Return the callback ID.                            */
                  ret_val = ANPEntryInfoPtr->CallbackID;
               }
            }
            else
               ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;

            BTPS_ReleaseMutex(ANPManagerMutex);
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
   ANP_Entry_Info_t *ANPEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANP Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear semi-valid.   */
      if(ANPManagerCallbackID)
      {
         /* Attempt to wait for access to the ANP Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(ANPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Delete the entry from the list.                          */
            if((ANPEntryInfo = DeleteANPEntryInfoEntry(&ANPClientEntryInfoList, ANPManagerCallbackID)) != NULL)
            {
               /* If this was the last entry, go ahead and un-register  */
               /* from the PM server.                                   */
               if(!ANPClientEntryInfoList)
               {
                  _ANPM_Un_Register_Client_Event_Callback(ANPManagerCallbackID);

                  ServerCallbackID = 0;
               }

               /* Free the entry.                                       */
               FreeANPEntryInfoEntryMemory(ANPEntryInfo);
            }

            BTPS_ReleaseMutex(ANPManagerMutex);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}
