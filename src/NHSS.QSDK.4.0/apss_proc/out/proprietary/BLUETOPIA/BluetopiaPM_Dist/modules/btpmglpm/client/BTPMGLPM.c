/*****< btpmglpm.c >***********************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMGLPM - Glucose Manager for Stonestreet One Bluetooth Protocol Stack   */
/*             Platform Manager.                                              */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   03/15/13  T. Cook        Initial creation.                               */
/******************************************************************************/
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMGLPM.h"            /* BTPM GLP Manager Prototypes/Constants.    */
#include "GLPMAPI.h"             /* GLP Manager Prototypes/Constants.         */
#include "GLPMMSG.h"             /* BTPM GLP Manager Message Formats.         */
#include "GLPMGR.h"              /* GLP Manager Impl. Prototypes/Constants.   */

#include "SS1BTPM.h"             /* BTPM Main Prototypes and Constants.       */
#include "BTPMERR.h"             /* BTPM Error Prototypes/Constants.          */
#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

   /* Structure which is used to track Callback Information related to  */
   /* this module.                                                      */
typedef struct _tagGLP_Entry_Info_t
{
   unsigned int                 CallbackID;
   unsigned int                 EventHandlerID;
   unsigned long                Flags;
   GLPM_Event_Callback_t        EventCallback;
   void                        *CallbackParameter;
   struct _tagGLP_Entry_Info_t *NextGLPEntryInfoPtr;
} GLP_Entry_Info_t;

   /* The following functions are used with the Flags member of the     */
   /* GLP_Entry_Info_t structure to denote various state information.   */
#define GLP_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY           0x40000000

   /* Container structure used when dispatching registered callbacks.   */
typedef struct _tagCallbackInfo_t
{
   GLPM_Event_Callback_t  EventCallback;
   void                  *CallbackParameter;
} CallbackInfo_t;

   /* The following structure is a container structure that contains all*/
   /* of the information on an outstanding transaction.                 */
typedef struct _tagTransaction_Info_t
{
   unsigned int                   ProcedureID;
   unsigned int                   GLPMProcedureID;
   unsigned int                   GLPMCollectorCallbackID;
   struct _tagTransaction_Info_t *NextTransactionInfoPtr;
} Transaction_Info_t;

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the compiler*/
   /* as part of standard C/C++).                                       */

   /* Variable which serves as a global flag about whether this module  */
   /* is initialized or not.                                            */
static Boolean_t Initialized;

   /* Variable which is used to protect access to the state information */
   /* in this module.                                                   */
static Mutex_t GLPManagerMutex;

   /* Variable which is used to hold the next (unique) Callback ID.     */
static unsigned int NextCallbackID;

   /* Variable which is used to hold the next (unique) Transaction ID.  */
static unsigned int NextProcedureID;

   /* Variable which holds the current Power State of the Device.       */
static Boolean_t CurrentPowerState;

   /* Variable which holds a pointer to the first element in the GLP    */
   /* Entry Information List (which holds all Callbacks tracked by this */
   /* module).                                                          */
static GLP_Entry_Info_t *GLPEntryInfoList;

   /* Variable which holds a pointer to the first element in the        */
   /* Transaction Information List.                                     */
static Transaction_Info_t *TransactionInfoList;

   /* Internal Function Prototypes.                                     */
static unsigned int GetNextCallbackID(void);

static GLP_Entry_Info_t *AddGLPEntryInfoEntry(GLP_Entry_Info_t **ListHead, GLP_Entry_Info_t *EntryToAdd);
static GLP_Entry_Info_t *SearchGLPEntryInfoEntry(GLP_Entry_Info_t **ListHead, unsigned int CallbackID);
static GLP_Entry_Info_t *DeleteGLPEntryInfoEntry(GLP_Entry_Info_t **ListHead, unsigned int CallbackID);
static void FreeGLPEntryInfoEntryMemory(GLP_Entry_Info_t *EntryToFree);
static void FreeGLPEntryInfoList(GLP_Entry_Info_t **ListHead);

static Transaction_Info_t *AddTransactionInfo(Transaction_Info_t **ListHead, Transaction_Info_t *EntryToAdd);
static Transaction_Info_t *SearchTransactionInfo(Transaction_Info_t **ListHead, unsigned int ProcedureID);
static Transaction_Info_t *SearchTransactionInfoByGLPMProcedureID(Transaction_Info_t **ListHead, unsigned int GLPMProcedureID);
static Transaction_Info_t *DeleteTransactionInfo(Transaction_Info_t **ListHead, unsigned int ProcedureID);
static void FreeTransactionInfoMemory(Transaction_Info_t *EntryToFree);
static void FreeTransactionInfoList(Transaction_Info_t **ListHead);

static void DispatchGLPEvent(GLPM_Event_Data_t *GLPMEventData);

static void ProcessGLPConnectedEvent(GLPM_Connected_Message_t *Message);
static void ProcessGLPDisconnectedEvent(GLPM_Disconnected_Message_t *Message);
static void ProcessProcedureStartedEvent(GLPM_Procedure_Started_Message_t *Message);
static void ProcessProcedureStoppedEvent(GLPM_Procedure_Stopped_Message_t *Message);
static void ProcessGlucoseMeasurementEvent(GLPM_Glucose_Measurement_Message_t *Message);

static void ProcessReceivedMessage(BTPM_Message_t *Message);

static void BTPSAPI BTPMDispatchCallback_GLPM(void *CallbackParameter);
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter);

static void BTPSAPI GLPManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter);

   /* The following function is a utility function that exists to       */
   /* retrieve a unique Callback ID that can be used to add an entry    */
   /* into the GLP Entry Information List.                              */
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
   /* into the Transaction Information List.                            */
static unsigned int GetNextProcedureID(void)
{
   unsigned int ret_val;

   ret_val = NextProcedureID++;

   if((!NextProcedureID) || (NextProcedureID & 0x80000000))
      NextProcedureID = 0x00000001;

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
static GLP_Entry_Info_t *AddGLPEntryInfoEntry(GLP_Entry_Info_t **ListHead, GLP_Entry_Info_t *EntryToAdd)
{
   GLP_Entry_Info_t *AddedEntry = NULL;
   GLP_Entry_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First let's verify that values passed in are semi-valid.          */
   if((ListHead) && (EntryToAdd))
   {
      /* Make sure that the element that we are adding seems semi-valid.*/
      if(EntryToAdd->CallbackID)
      {
         /* OK, data seems semi-valid, let's allocate a new data        */
         /* structure to add to the list.                               */
         AddedEntry = (GLP_Entry_Info_t *)BTPS_AllocateMemory(sizeof(GLP_Entry_Info_t));

         if(AddedEntry)
         {
            /* Copy All Data over.                                      */
            *AddedEntry                     = *EntryToAdd;

            /* Now Add it to the end of the list.                       */
            AddedEntry->NextGLPEntryInfoPtr = NULL;

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
                     FreeGLPEntryInfoEntryMemory(AddedEntry);
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
                     if(tmpEntry->NextGLPEntryInfoPtr)
                        tmpEntry = tmpEntry->NextGLPEntryInfoPtr;
                     else
                        break;
                  }
               }

               if(AddedEntry)
               {
                  /* Last element found, simply Add the entry.          */
                  tmpEntry->NextGLPEntryInfoPtr = AddedEntry;
               }
            }
            else
               *ListHead = AddedEntry;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", AddedEntry));

   return(AddedEntry);
}

   /* The following function searches the specified List for the        */
   /* specified Callback ID.  This function returns NULL if either the  */
   /* List Head is invalid, the Callback ID is invalid, or the specified*/
   /* Callback ID was NOT found.                                        */
static GLP_Entry_Info_t *SearchGLPEntryInfoEntry(GLP_Entry_Info_t **ListHead, unsigned int CallbackID)
{
   GLP_Entry_Info_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", CallbackID));

   /* Let's make sure the list and Callback ID to search for appear to  */
   /* be valid.                                                         */
   if((ListHead) && (CallbackID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->CallbackID != CallbackID))
         FoundEntry = FoundEntry->NextGLPEntryInfoPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* The following function searches the specified GLP Entry           */
   /* Information List for the specified Callback ID and removes it from*/
   /* the List.  This function returns NULL if either the GLP Entry     */
   /* Information List Head is invalid, the Callback ID is invalid, or  */
   /* the specified Callback ID was NOT present in the list.  The entry */
   /* returned will have the Next Entry field set to NULL, and the      */
   /* caller is responsible for deleting the memory associated with this*/
   /* entry by calling FreeGLPEntryInfoEntryMemory().                   */
static GLP_Entry_Info_t *DeleteGLPEntryInfoEntry(GLP_Entry_Info_t **ListHead, unsigned int CallbackID)
{
   GLP_Entry_Info_t *FoundEntry = NULL;
   GLP_Entry_Info_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", CallbackID));

   /* Let's make sure the List and Callback ID to search for appear to  */
   /* be semi-valid.                                                    */
   if((ListHead) && (CallbackID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->CallbackID != CallbackID))
      {
         LastEntry  = FoundEntry;
         FoundEntry = FoundEntry->NextGLPEntryInfoPtr;
      }

      /* Check to see if we found the specified entry.                  */
      if(FoundEntry)
      {
         /* OK, now let's remove the entry from the list.  We have to   */
         /* check to see if the entry was the first entry in the list.  */
         if(LastEntry)
         {
            /* Entry was NOT the first entry in the list.               */
            LastEntry->NextGLPEntryInfoPtr = FoundEntry->NextGLPEntryInfoPtr;
         }
         else
            *ListHead = FoundEntry->NextGLPEntryInfoPtr;

         FoundEntry->NextGLPEntryInfoPtr = NULL;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* This function frees the specified GLP Entry Information member.   */
   /* No check is done on this entry other than making sure it NOT NULL.*/
static void FreeGLPEntryInfoEntryMemory(GLP_Entry_Info_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToFree)
      BTPS_FreeMemory(EntryToFree);

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified GLP Entry Information List.  Upon return */
   /* of this function, the Head Pointer is set to NULL.                */
static void FreeGLPEntryInfoList(GLP_Entry_Info_t **ListHead)
{
   GLP_Entry_Info_t *EntryToFree;
   GLP_Entry_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ListHead)
   {
      /* Simply traverse the list and free every element present.       */
      EntryToFree = *ListHead;

      while(EntryToFree)
      {
         tmpEntry    = EntryToFree;
         EntryToFree = EntryToFree->NextGLPEntryInfoPtr;

         FreeGLPEntryInfoEntryMemory(tmpEntry);
      }

      /* Make sure the List appears to be empty.                        */
      *ListHead = NULL;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function adds the specified Entry to the specified  */
   /* List.  This function allocates and adds an entry to the list that */
   /* has the same attributes as the Entry passed into this function.   */
   /* This function will return NULL if NO Entry was added.  This can   */
   /* occur if the element passed in was deemed invalid or the actual   */
   /* List Head was invalid.                                            */
   /* ** NOTE ** This function does not insert duplicate entries into   */
   /*            the list.  An element is considered a duplicate if the */
   /*            the ProcedureID field is the same as an entry already  */
   /*            in the list.  When this occurs, this function returns  */
   /*            NULL.                                                  */
static Transaction_Info_t *AddTransactionInfo(Transaction_Info_t **ListHead, Transaction_Info_t *EntryToAdd)
{
   Transaction_Info_t *AddedEntry = NULL;
   Transaction_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First let's verify that values passed in are semi-valid.          */
   if((ListHead) && (EntryToAdd))
   {
      /* Make sure that the element that we are adding seems semi-valid.*/
      if(EntryToAdd->ProcedureID)
      {
         /* OK, data seems semi-valid, let's allocate a new data        */
         /* structure to add to the list.                               */
         AddedEntry = (Transaction_Info_t *)BTPS_AllocateMemory(sizeof(Transaction_Info_t));

         if(AddedEntry)
         {
            /* Copy All Data over.                                      */
            *AddedEntry                        = *EntryToAdd;

            /* Now Add it to the end of the list.                       */
            AddedEntry->NextTransactionInfoPtr = NULL;

            /* First, let's check to see if there are any elements      */
            /* already present in the List that was passed in.          */
            if((tmpEntry = *ListHead) != NULL)
            {
               /* Head Pointer was not NULL, so we will traverse the    */
               /* list until we reach the last element.                 */
               while(tmpEntry)
               {
                  if(tmpEntry->ProcedureID == AddedEntry->ProcedureID)
                  {
                     /* Entry was already added, so free the memory and */
                     /* flag an error to the caller.                    */
                     FreeTransactionInfoMemory(AddedEntry);
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
                     if(tmpEntry->NextTransactionInfoPtr)
                        tmpEntry = tmpEntry->NextTransactionInfoPtr;
                     else
                        break;
                  }
               }

               if(AddedEntry)
               {
                  /* Last element found, simply Add the entry.          */
                  tmpEntry->NextTransactionInfoPtr = AddedEntry;
               }
            }
            else
               *ListHead = AddedEntry;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", AddedEntry));

   return(AddedEntry);
}

   /* The following function searches the specified Transaction Info    */
   /* List for the Transaction Info with the specified ProcedureID .    */
   /* This function returns NULL if either the Transaction Info List    */
   /* Head is invalid, the Transaction ID is invalid, or the specified  */
   /* Entry was NOT present in the list.                                */
static Transaction_Info_t *SearchTransactionInfo(Transaction_Info_t **ListHead, unsigned int ProcedureID)
{
   Transaction_Info_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's make sure the list and Procedure ID to search for appear to */
   /* be valid.                                                         */
   if((ListHead) && (ProcedureID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->ProcedureID != ProcedureID))
         FoundEntry = FoundEntry->NextTransactionInfoPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* The following function searches the specified Transaction Info    */
   /* List for the Transaction Info with the specified GLPM Procedure   */
   /* ID.  This function returns NULL if either the Transaction Info    */
   /* List Head is invalid, the GLPM Procedure ID is invalid, or the    */
   /* specified Entry was NOT present in the list.                      */
static Transaction_Info_t *SearchTransactionInfoByGLPMProcedureID(Transaction_Info_t **ListHead, unsigned int GLPMProcedureID)
{
   Transaction_Info_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's make sure the list and GLPM Procedure ID to search for      */
   /* appear to be valid.                                               */
   if((ListHead) && (GLPMProcedureID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->GLPMProcedureID != GLPMProcedureID))
         FoundEntry = FoundEntry->NextTransactionInfoPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* The following function searches the specified Transaction Info    */
   /* List for the Transaction Info with the specified ProcedureID and  */
   /* removes it from the List.  This function returns NULL if either   */
   /* the Transaction Info List Head is invalid, the Transaction ID is  */
   /* invalid, or the specified Entry was NOT present in the list.  The */
   /* entry returned will have the Next Entry field set to NULL, and the*/
   /* caller is responsible for deleting the memory associated with this*/
   /* entry by calling FreeTransactionInfoMemory().                     */
static Transaction_Info_t *DeleteTransactionInfo(Transaction_Info_t **ListHead, unsigned int ProcedureID)
{
   Transaction_Info_t *FoundEntry = NULL;
   Transaction_Info_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's make sure the list and Procedure ID to search for appear to */
   /* be valid.                                                         */
   if((ListHead) && (ProcedureID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->ProcedureID != ProcedureID))
      {
         LastEntry  = FoundEntry;
         FoundEntry = FoundEntry->NextTransactionInfoPtr;
      }

      /* Check to see if we found the specified entry.                  */
      if(FoundEntry)
      {
         /* OK, now let's remove the entry from the list.  We have to   */
         /* check to see if the entry was the first entry in the list.  */
         if(LastEntry)
         {
            /* Entry was NOT the first entry in the list.               */
            LastEntry->NextTransactionInfoPtr = FoundEntry->NextTransactionInfoPtr;
         }
         else
            *ListHead = FoundEntry->NextTransactionInfoPtr;

         FoundEntry->NextTransactionInfoPtr = NULL;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* This function frees the specified Transaction Information member. */
   /* No check is done on this entry other than making sure it NOT NULL.*/
static void FreeTransactionInfoMemory(Transaction_Info_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToFree)
      BTPS_FreeMemory(EntryToFree);

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified Transaction Information List.  Upon      */
   /* return of this function, the Head Pointer is set to NULL.         */
static void FreeTransactionInfoList(Transaction_Info_t **ListHead)
{
   Transaction_Info_t *EntryToFree;
   Transaction_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ListHead)
   {
      /* Simply traverse the list and free every element present.       */
      EntryToFree = *ListHead;

      while(EntryToFree)
      {
         tmpEntry    = EntryToFree;
         EntryToFree = EntryToFree->NextTransactionInfoPtr;

         FreeTransactionInfoMemory(tmpEntry);
      }

      /* Make sure the List appears to be empty.                        */
      *ListHead = NULL;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch the specified GLP event to every registered GLP Event    */
   /* Callback.                                                         */
   /* * NOTE * This function should be called with the GLP Manager Mutex*/
   /*          held.  Upon exit from this function it will free the GLP */
   /*          Manager Mutex.                                           */
static void DispatchGLPEvent(GLPM_Event_Data_t *GLPMEventData)
{
   unsigned int      Index;
   unsigned int      NumberCallbacks;
   CallbackInfo_t    CallbackInfoArray[16];
   CallbackInfo_t   *CallbackInfoArrayPtr;
   GLP_Entry_Info_t *GLPEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure that the input parameters appear to be semi-valid.      */
   if((GLPEntryInfoList) && (GLPMEventData))
   {
      /* Next, let's determine how many callbacks are registered.       */
      NumberCallbacks = 0;

      /* First, add the default event handlers.                         */
      GLPEntryInfo    = GLPEntryInfoList;
      while(GLPEntryInfo)
      {
         if((GLPEntryInfo->EventCallback) && (GLPEntryInfo->Flags & GLP_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY))
            NumberCallbacks++;

         GLPEntryInfo = GLPEntryInfo->NextGLPEntryInfoPtr;
      }

      /* Only continue if we have some callbacks to dispatch to.        */
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
            NumberCallbacks = 0;

            /* First, add the default event handlers.                   */
            GLPEntryInfo    = GLPEntryInfoList;
            while(GLPEntryInfo)
            {
               if((GLPEntryInfo->EventCallback) && (GLPEntryInfo->Flags & GLP_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY))
               {
                  CallbackInfoArrayPtr[NumberCallbacks].EventCallback     = GLPEntryInfo->EventCallback;
                  CallbackInfoArrayPtr[NumberCallbacks].CallbackParameter = GLPEntryInfo->CallbackParameter;

                  NumberCallbacks++;
               }

               GLPEntryInfo = GLPEntryInfo->NextGLPEntryInfoPtr;
            }

            /* Release the Mutex because we have already built the      */
            /* Callback Array.                                          */
            BTPS_ReleaseMutex(GLPManagerMutex);

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
                     (*CallbackInfoArrayPtr[Index].EventCallback)(GLPMEventData, CallbackInfoArrayPtr[Index].CallbackParameter);
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
            BTPS_ReleaseMutex(GLPManagerMutex);
         }
      }
      else
      {
         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(GLPManagerMutex);
      }
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(GLPManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the GLP      */
   /* Connected asynchronous message.                                   */
static void ProcessGLPConnectedEvent(GLPM_Connected_Message_t *Message)
{
   GLPM_Event_Data_t GLPMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if(Message)
   {
      /* Format up the Event.                                           */
      GLPMEventData.EventType                                        = getGLPMConnected;
      GLPMEventData.EventLength                                      = GLPM_CONNECTED_EVENT_DATA_SIZE;

      GLPMEventData.EventData.ConnectedEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;
      GLPMEventData.EventData.ConnectedEventData.ConnectionType      = Message->ConnectionType;
      GLPMEventData.EventData.ConnectedEventData.SupportedFeatures   = Message->SupportedFeatures;

      /* Now that the event is formatted, dispatch it.                  */
      DispatchGLPEvent(&GLPMEventData);
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(GLPManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the GLP      */
   /* Disconnected asynchronous message.                                */
static void ProcessGLPDisconnectedEvent(GLPM_Disconnected_Message_t *Message)
{
   GLPM_Event_Data_t GLPMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if(Message)
   {
      /* Format up the Event.                                           */
      GLPMEventData.EventType                                           = getGLPMDisconnected;
      GLPMEventData.EventLength                                         = GLPM_DISCONNECTED_EVENT_DATA_SIZE;

      GLPMEventData.EventData.DisconnectedEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;
      GLPMEventData.EventData.DisconnectedEventData.ConnectionType      = Message->ConnectionType;

      /* Now that the event is formatted, dispatch it.                  */
      DispatchGLPEvent(&GLPMEventData);
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(GLPManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Procedure*/
   /* Started asynchronous message.                                     */
static void ProcessProcedureStartedEvent(GLPM_Procedure_Started_Message_t *Message)
{
   void                  *CallbackParameter;
   GLP_Entry_Info_t      *GLPEntryInfo;
   GLPM_Event_Data_t      GLPMEventData;
   Transaction_Info_t    *TransactionInfo;
   GLPM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if(Message)
   {
      /* Search for and delete the Transaction Info for the request that*/
      /* generated this response.                                       */
      if((TransactionInfo = SearchTransactionInfoByGLPMProcedureID(&TransactionInfoList, Message->ProcedureID)) != NULL)
      {
         /* Search for the callback that this event should be sent to.  */
         if((GLPEntryInfo = SearchGLPEntryInfoEntry(&GLPEntryInfoList, TransactionInfo->GLPMCollectorCallbackID)) != NULL)
         {
            /* Format up the Event.                                     */
            GLPMEventData.EventType                                                      = getGLPMProcedureStarted;
            GLPMEventData.EventLength                                                    = GLPM_PROCEDURE_STARTED_EVENT_DATA_SIZE;

            GLPMEventData.EventData.ProcedureStartedEventData.RemoteDeviceAddress        = Message->RemoteDeviceAddress;
            GLPMEventData.EventData.ProcedureStartedEventData.ProcedureID                = TransactionInfo->ProcedureID;
            GLPMEventData.EventData.ProcedureStartedEventData.Success                    = Message->Success;
            GLPMEventData.EventData.ProcedureStartedEventData.AttributeProtocolErrorCode = Message->AttributeProtocolErrorCode;

            /* Note the Callback Information.                           */
            EventCallback     = GLPEntryInfo->EventCallback;
            CallbackParameter = GLPEntryInfo->CallbackParameter;

            /* If an error occurred starting the procedure we will      */
            /* delete the transaction info entry.                       */
            if(Message->Success == FALSE)
            {
               if((TransactionInfo = DeleteTransactionInfo(&TransactionInfoList, TransactionInfo->ProcedureID)) != NULL)
                  FreeTransactionInfoMemory(TransactionInfo);
            }

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(GLPManagerMutex);

            __BTPSTRY
            {
               (*EventCallback)(&GLPMEventData, CallbackParameter);
            }
            __BTPSEXCEPT(1)
            {
               /* Do Nothing.                                           */
            }
         }
         else
         {
            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(GLPManagerMutex);
         }
      }
      else
      {
         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(GLPManagerMutex);
      }
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(GLPManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Procedure*/
   /* Stopped asynchronous message.                                     */
static void ProcessProcedureStoppedEvent(GLPM_Procedure_Stopped_Message_t *Message)
{
   void                  *CallbackParameter;
   GLP_Entry_Info_t      *GLPEntryInfo;
   GLPM_Event_Data_t      GLPMEventData;
   Transaction_Info_t    *TransactionInfo;
   GLPM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if(Message)
   {
      /* Search for and delete the Transaction Info for the request that*/
      /* generated this response.                                       */
      if((TransactionInfo = SearchTransactionInfoByGLPMProcedureID(&TransactionInfoList, Message->ProcedureID)) != NULL)
      {
         /* Search for the callback that this event should be sent to.  */
         if((GLPEntryInfo = SearchGLPEntryInfoEntry(&GLPEntryInfoList, TransactionInfo->GLPMCollectorCallbackID)) != NULL)
         {
            /* Format up the Event.                                     */
            GLPMEventData.EventType                                               = getGLPMProcedureStopped;
            GLPMEventData.EventLength                                             = GLPM_PROCEDURE_STOPPED_EVENT_DATA_SIZE;

            GLPMEventData.EventData.ProcedureStoppedEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;
            GLPMEventData.EventData.ProcedureStoppedEventData.ProcedureID         = TransactionInfo->ProcedureID;
            GLPMEventData.EventData.ProcedureStoppedEventData.ProcedureType       = Message->ProcedureType;
            GLPMEventData.EventData.ProcedureStoppedEventData.ResponseCode        = Message->ResponseCode;
            GLPMEventData.EventData.ProcedureStoppedEventData.NumberStoredRecords = Message->NumberStoredRecords;

            /* Note the Callback Information.                           */
            EventCallback     = GLPEntryInfo->EventCallback;
            CallbackParameter = GLPEntryInfo->CallbackParameter;

            /* Unless this is a failed abort procedure that just stopped*/
            /* we should delete the transaction info entry.             */
            if(!((Message->ProcedureType == gptAbortProcedure) && (Message->ResponseCode != grcSuccess)))
            {
               if((TransactionInfo = DeleteTransactionInfo(&TransactionInfoList, TransactionInfo->ProcedureID)) != NULL)
                  FreeTransactionInfoMemory(TransactionInfo);
            }

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(GLPManagerMutex);

            __BTPSTRY
            {
               (*EventCallback)(&GLPMEventData, CallbackParameter);
            }
            __BTPSEXCEPT(1)
            {
               /* Do Nothing.                                           */
            }
         }
         else
         {
            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(GLPManagerMutex);
         }
      }
      else
      {
         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(GLPManagerMutex);
      }
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(GLPManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Glucose  */
   /* Measurement asynchronous message.                                 */
static void ProcessGlucoseMeasurementEvent(GLPM_Glucose_Measurement_Message_t *Message)
{
   void                  *CallbackParameter;
   GLP_Entry_Info_t      *GLPEntryInfo;
   GLPM_Event_Data_t      GLPMEventData;
   Transaction_Info_t    *TransactionInfo;
   GLPM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if(Message)
   {
      /* Search for and delete the Transaction Info for the request that*/
      /* generated this response.                                       */
      if((TransactionInfo = SearchTransactionInfoByGLPMProcedureID(&TransactionInfoList, Message->ProcedureID)) != NULL)
      {
         /* Search for the callback that this event should be sent to.  */
         if((GLPEntryInfo = SearchGLPEntryInfoEntry(&GLPEntryInfoList, TransactionInfo->GLPMCollectorCallbackID)) != NULL)
         {
            /* Format up the Event.                                     */
            GLPMEventData.EventType                                                           = getGLPMGlucoseMeasurement;
            GLPMEventData.EventLength                                                         = GLPM_GLUCOSE_MEASUREMENT_EVENT_DATA_SIZE;

            GLPMEventData.EventData.GlucoseMeasurementEventData.RemoteDeviceAddress           = Message->RemoteDeviceAddress;
            GLPMEventData.EventData.GlucoseMeasurementEventData.ProcedureID                   = TransactionInfo->ProcedureID;
            GLPMEventData.EventData.GlucoseMeasurementEventData.MeasurementFlags              = Message->MeasurementFlags;
            GLPMEventData.EventData.GlucoseMeasurementEventData.MeasurementErrorType          = Message->MeasurementErrorType;
            GLPMEventData.EventData.GlucoseMeasurementEventData.GlucoseMeasurementData        = Message->GlucoseMeasurementData;
            GLPMEventData.EventData.GlucoseMeasurementEventData.GlucoseMeasurementContextData = Message->GlucoseMeasurementContextData;

            /* Note the Callback Information.                           */
            EventCallback     = GLPEntryInfo->EventCallback;
            CallbackParameter = GLPEntryInfo->CallbackParameter;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(GLPManagerMutex);

            __BTPSTRY
            {
               (*EventCallback)(&GLPMEventData, CallbackParameter);
            }
            __BTPSEXCEPT(1)
            {
               /* Do Nothing.                                           */
            }
         }
         else
         {
            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(GLPManagerMutex);
         }
      }
      else
      {
         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(GLPManagerMutex);
      }
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(GLPManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is called from the Main Bluetopia Dispatch */
   /* thread in response to a Process Received Message.  This function  */
   /* SHOULD ONLY be called in this capacity and NOT under any other    */
   /* circumstances.                                                    */
   /* * NOTE * This function *MUST* be called with the GLP Manager Mutex*/
   /*          held.  This function will release the Mutex before it    */
   /*          exits (i.e. the caller SHOULD NOT RELEASE THE MUTEX).    */
static void ProcessReceivedMessage(BTPM_Message_t *Message)
{
   Boolean_t ReleaseMutex = TRUE;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", Message->MessageHeader.MessageFunction));

   /* Check to make sure this isn't some kind of spoofed response (we do*/
   /* not process responses).                                           */
   if((Message) && (!(Message->MessageHeader.MessageID & BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK)))
   {
      /* Request received, go ahead and process it.                     */
      switch(Message->MessageHeader.MessageFunction)
      {
         case GLPM_MESSAGE_FUNCTION_CONNECTED:
            DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_VERBOSE), ("Connected Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= GLPM_CONNECTED_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Connected Event.                                      */
               ProcessGLPConnectedEvent((GLPM_Connected_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case GLPM_MESSAGE_FUNCTION_DISCONNECTED:
            DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_VERBOSE), ("Disconnected Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= GLPM_DISCONNECTED_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Disconnected Event.                                   */
               ProcessGLPDisconnectedEvent((GLPM_Disconnected_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case GLPM_MESSAGE_FUNCTION_PROCEDURE_STARTED_EVENT:
            DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_VERBOSE), ("Procedure Started Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= GLPM_PROCEDURE_STARTED_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Procedure Started Event.                              */
               ProcessProcedureStartedEvent((GLPM_Procedure_Started_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case GLPM_MESSAGE_FUNCTION_PROCEDURE_STOPPED_EVENT:
            DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_VERBOSE), ("Procedure Stopped Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= GLPM_PROCEDURE_STOPPED_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Procedure Stopped Event.                              */
               ProcessProcedureStoppedEvent((GLPM_Procedure_Stopped_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case GLPM_MESSAGE_FUNCTION_GLUCOSE_MEASUREMENT_EVENT:
            DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_VERBOSE), ("Glucose Measurement Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= GLPM_GLUCOSE_MEASUREMENT_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Glucose Measurement Event.                            */
               ProcessGlucoseMeasurementEvent((GLPM_Glucose_Measurement_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         default:
            DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled Message\n"));
            break;
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid or Response Message Received\n"));
   }

   /* If we need to Release the Mutex then we will do it at this time.  */
   if(ReleaseMutex)
      BTPS_ReleaseMutex(GLPManagerMutex);

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process GLP Manager Asynchronous Events.            */
static void BTPSAPI BTPMDispatchCallback_GLPM(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's determine if the specified parameter appears to be          */
   /* semi-valid.                                                       */
   if(CallbackParameter)
   {
      /* Next, let's check to see if we need to process it.             */
      if(Initialized)
      {
         /* Attempt to wait for access to the GLP state information.    */
         if(BTPS_WaitMutex(GLPManagerMutex, BTPS_INFINITE_WAIT))
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

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process Server un-register asynchronous events.     */
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's determine if the specified parameter appears to be          */
   /* semi-valid.                                                       */
   if(CallbackParameter)
   {
      /* Next, let's check to see if we need to process it.             */
      if(Initialized)
      {
         /* Attempt to wait for access to the GLP state information.    */
         if(BTPS_WaitMutex(GLPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Free the GLP Entry Info List.                            */
            FreeGLPEntryInfoList(&GLPEntryInfoList);

            /* Free the Transaction Information List.                   */
            FreeTransactionInfoList(&TransactionInfoList);

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(GLPManagerMutex);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function represents the Message Group Handler       */
   /* function that is installed to process all GLP Manager Messages.   */
static void BTPSAPI GLPManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the Message specified appears to be semi-valid.   */
   if(Message)
   {
      /* Next, check to see if this message is specified for our        */
      /* specific Message Group.                                        */
      if(Message->MessageHeader.MessageGroup == BTPM_MESSAGE_GROUP_GLUCOSE_MANAGER)
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_VERBOSE), ("GLP Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

         /* Message is destined for us, make sure we are still          */
         /* processing messages.                                        */
         if(Initialized)
         {
            /* Check to see if this message is a GLP Manager defined    */
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
               /* GLP Manager Thread.                                   */
               if(!BTPM_QueueMailboxCallback(BTPMDispatchCallback_GLPM, (void *)Message))
               {
                  DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue GLP Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

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
                        DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue GLP Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));
                     }
                  }
               }
            }
         }
         else
         {
            /* Check to see if this message is an GLP Manager defined   */
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
         DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_VERBOSE), ("Non GLP Manager Message: 0x%08X, 0x%08X\n", Message->MessageHeader.MessageGroup, Message->MessageHeader.MessageFunction));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for initializing/cleaning up*/
   /* the Bluetopia Platform Manager GLP Manager Module.  This function */
   /* should be registered with the Bluetopia Platform Manager Module   */
   /* Handler and will be called when the Platform Manager is           */
   /* initialized (or shut down).                                       */
void BTPSAPI GLPM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData)
{
   int               Result;
   GLP_Entry_Info_t *GLPEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %d\n", Initialize));

   /* First, determine if the request is to initialize or shut down.    */
   if(Initialize)
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_VERBOSE), ("Initialization Request\n"));

      if(!Initialized)
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing GLP Manager\n"));

         /* First, let's create a Mutex to guard access to this module. */
         if((GLPManagerMutex = BTPS_CreateMutex(FALSE)) != NULL)
         {
            /* Next, let's attempt to register our Message Group Handler*/
            /* to process GLP Manager messages.                         */
            if(!MSG_RegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_GLUCOSE_MANAGER, GLPManagerGroupHandler, NULL))
            {
               /* Initialize the actual GLP Manager Implementation      */
               /* Module (this is the module that is actually           */
               /* responsible for actually implementing the GLP Manager */
               /* functionality - this module is just the framework     */
               /* shell).                                               */
               if(!(Result = _GLPM_Initialize()))
               {
                  /* Note the current Power State.                      */
                  CurrentPowerState = (Boolean_t)((DEVM_QueryDevicePowerState() > 0)?TRUE:FALSE);

                  /* Initialize a unique, starting GLP Callback ID.     */
                  NextCallbackID      = 0x000000001;

                  /* Initialize a unique, starting Transaction ID.      */
                  NextProcedureID   = 0x000000001;

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
            if(GLPManagerMutex)
               BTPS_CloseMutex(GLPManagerMutex);

            MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_GLUCOSE_MANAGER);

            /* Flag that none of the resources are allocated.           */
            GLPManagerMutex     = NULL;
         }
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_VERBOSE), ("GLP Manager Already initialized\n"));
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_VERBOSE), ("Shutdown Request\n"));

      if(Initialized)
      {
         /* First, let's make sure that we un-register our Message Group*/
         /* Handler (so that we do not process any more incoming        */
         /* messages).                                                  */
         MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_GLUCOSE_MANAGER);

         /* Wait for access to the Mutex that guards access to this     */
         /* module.                                                     */
         if(BTPS_WaitMutex(GLPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Next, Un-Register for any GLP Collector Events.          */
            GLPEntryInfo = GLPEntryInfoList;
            while(GLPEntryInfo)
            {
               _GLPM_Un_Register_Collector_Events(GLPEntryInfo->EventHandlerID);

               GLPEntryInfo = GLPEntryInfo->NextGLPEntryInfoPtr;
            }

            /* Make sure we inform the GLP Manager Implementation that  */
            /* we are shutting down.                                    */
            _GLPM_Cleanup();

            /* Make sure that the GLP Entry Information List is empty.  */
            FreeGLPEntryInfoList(&GLPEntryInfoList);

            /* Free the Transaction Information List.                   */
            FreeTransactionInfoList(&TransactionInfoList);

            /* Close the GLP Manager Mutex.                             */
            BTPS_CloseMutex(GLPManagerMutex);

            /* Flag that the resources are no longer allocated.         */
            GLPManagerMutex     = NULL;
            CurrentPowerState   = FALSE;

            /* Flag that this module is no longer initialized.          */
            Initialized         = FALSE;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing asynchronous */
   /* Device Manager (DEVM) Events (including Power On/Off events).     */
   /* This function should be registered with the Bluetopia Platform    */
   /* Manager Module Handler and will be called when an asynchronous    */
   /* Device Manager event is dispatched.                               */
void BTPSAPI GLPM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the GLP Manager has been initialized.   */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(BTPS_WaitMutex(GLPManagerMutex, BTPS_INFINITE_WAIT))
      {
         switch(EventData->EventType)
         {
            case detDevicePoweredOn:
               DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power On Occurred.\n"));

               /* Note the new Power State.                             */
               CurrentPowerState = TRUE;
               break;
            case detDevicePoweringOff:
            case detDevicePoweredOff:
               DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power Off Occurred.\n"));

               /* Note the new Power State.                             */
               CurrentPowerState   = FALSE;
               break;
            default:
               /* Do nothing.                                           */
               break;
         }

         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(GLPManagerMutex);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the Glucose Profile  */
   /* (GLPM) Manager Service.  This Callback will be dispatched by the  */
   /* GLP Manager when various GLP Manager Events occur.  This function */
   /* accepts the Callback Function and Callback Parameter              */
   /* (respectively) to call when a GLP Manager Event needs to be       */
   /* dispatched.  This function returns a positive (non-zero) value if */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          GLPM_Un_Register_Collector_Event_Callback() function to  */
   /*          un-register the callback from this module.               */
int BTPSAPI GLPM_Register_Collector_Event_Callback(GLPM_Event_Callback_t CallbackFunction, void *CallbackParameter)
{
   int               ret_val;
   GLP_Entry_Info_t  GLPEntryInfo;
   GLP_Entry_Info_t *GLPEntryInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the GLP Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(CallbackFunction)
      {
         /* Attempt to wait for access to the GLP Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(GLPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Attempt to add an entry into the GLP Entry list.         */
            BTPS_MemInitialize(&GLPEntryInfo, 0, sizeof(GLP_Entry_Info_t));

            GLPEntryInfo.CallbackID         = GetNextCallbackID();
            GLPEntryInfo.EventCallback      = CallbackFunction;
            GLPEntryInfo.CallbackParameter  = CallbackParameter;
            GLPEntryInfo.Flags              = GLP_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY;

            if((GLPEntryInfoPtr = AddGLPEntryInfoEntry(&GLPEntryInfoList, &GLPEntryInfo)) != NULL)
            {
               /* Attempt to register an event handler with the server. */
               ret_val = _GLPM_Register_Collector_Events();
               if(ret_val > 0)
               {
                  /* Collector Event Handler registered, go ahead and   */
                  /* flag success to the caller.                        */
                  GLPEntryInfoPtr->EventHandlerID = ret_val;

                  /* Return the local Callback ID to the user.          */
                  ret_val                         = GLPEntryInfoPtr->CallbackID;
               }
               else
               {
                  /* Since an error occurred delete the entry from the  */
                  /* list.                                              */
                  if((GLPEntryInfoPtr = DeleteGLPEntryInfoEntry(&GLPEntryInfoList, GLPEntryInfoPtr->CallbackID)) != NULL)
                     FreeGLPEntryInfoEntryMemory(GLPEntryInfoPtr);
               }
            }
            else
               ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(GLPManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HEALTH_THERMOMETER_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered GLP Manager Event Callback    */
   /* (registered via a successful call to the                          */
   /* GLPM_Register_Collector_Event_Callback() function).  This function*/
   /* accepts as input the GLP Manager Event Callback ID (return value  */
   /* from GLPM_Register_Collector_Event_Callback() function).          */
void BTPSAPI GLPM_Un_Register_Collector_Event_Callback(unsigned int GLPMCollectorCallbackID)
{
   GLP_Entry_Info_t *GLPEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the GLP Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(GLPMCollectorCallbackID)
      {
         /* Attempt to wait for access to the GLP Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(GLPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Delete the entry from the list.                          */
            if((GLPEntryInfo = DeleteGLPEntryInfoEntry(&GLPEntryInfoList, GLPMCollectorCallbackID)) != NULL)
            {
               /* Unregister with the server.                           */
               _GLPM_Un_Register_Collector_Events(GLPEntryInfo->EventHandlerID);

               /* Free the memory because we are finished with it.      */
               FreeGLPEntryInfoEntryMemory(GLPEntryInfo);
            }

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(GLPManagerMutex);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism of        */
   /* starting a Glucose Procedure to a remote Glucose Device.  This    */
   /* function accepts as input the GLP Manager Data Event Callback ID  */
   /* (return value from GLPM_Register_Collector_Event_Callback()       */
   /* function), the BD_ADDR of the remote Glucose Device and a pointer */
   /* to a structure containing the procedure data.  This function      */
   /* returns the positive, non-zero, Procedure ID of the request on    */
   /* success or a negative error code.                                 */
   /* * NOTE * The getGLPMProcedureStarted event will be generated when */
   /*          the remote Glucose Device responds to the Start Procedure*/
   /*          Request.                                                 */
   /* * NOTE * Only 1 Glucose procedure can be outstanding at a time for*/
   /*          each remote Glucose device.  A procedure is completed    */
   /*          when either the getGLPMProcedureStarted event is received*/
   /*          with a error code or if the getGLPMProcedureStopped event*/
   /*          is received for a procedure that started successfully.   */
int BTPSAPI GLPM_Start_Procedure_Request(unsigned int GLPMCollectorCallbackID, BD_ADDR_t RemoteDeviceAddress, GLPM_Procedure_Data_t *ProcedureData)
{
   int                 ret_val;
   GLP_Entry_Info_t   *GLPEntryInfo;
   Transaction_Info_t  TransactionInfo;
   Transaction_Info_t *TransactionInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the GLP Manager has been initialized.   */
   if(Initialized)
   {
      /* GLP Manager has been initialized, let's check the input        */
      /* parameters to see if they are semi-valid.                      */
      if((GLPMCollectorCallbackID) && (ProcedureData))
      {
         /* Attempt to wait for access to the GLP Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(GLPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First search for the specified Data Callback Information.*/
            if((GLPEntryInfo = SearchGLPEntryInfoEntry(&GLPEntryInfoList, GLPMCollectorCallbackID)) != NULL)
            {
               /* Initialize the Transaction Information entry.         */
               BTPS_MemInitialize(&TransactionInfo, 0, sizeof(Transaction_Info_t));

               TransactionInfo.ProcedureID             = GetNextProcedureID();
               TransactionInfo.GLPMCollectorCallbackID = GLPEntryInfo->CallbackID;

               /* Add the Transaction Information to the Transaction    */
               /* Info List.                                            */
               if((TransactionInfoPtr = AddTransactionInfo(&TransactionInfoList, &TransactionInfo)) != NULL)
               {
                  /* Simply call the Impl.  Manager to send the correct */
                  /* message to the server.                             */
                  ret_val = _GLPM_Start_Procedure_Request(GLPEntryInfo->EventHandlerID, RemoteDeviceAddress, ProcedureData);
                  if(ret_val > 0)
                  {
                     /* Save the GLPM Transaction ID.                   */
                     TransactionInfoPtr->GLPMProcedureID = (unsigned int)ret_val;

                     /* Return the Local Transaction ID to the caller.  */
                     ret_val                               = (int)TransactionInfoPtr->ProcedureID;
                  }
                  else
                  {
                     /* An error occurred so delete the transaction     */
                     /* information that was added to the list.         */
                     if((TransactionInfoPtr = DeleteTransactionInfo(&TransactionInfoList, TransactionInfoPtr->ProcedureID)) != NULL)
                        FreeTransactionInfoMemory(TransactionInfoPtr);
                  }
               }
               else
                  ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(GLPManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_GLUCOSE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is provided to allow a mechanism of        */
   /* stopping a previouly started Glucose Procedure to a remote Glucose*/
   /* Device.  This function accepts as input the GLP Manager Data Event*/
   /* Callback ID (return value from                                    */
   /* GLPM_Register_Collector_Event_Callback() function), the BD_ADDR of*/
   /* the remote Glucose Device and the Procedure ID that was returned  */
   /* via a successfull call to GLPM_Start_Procedure_Request().  This   */
   /* function returns zero on success or a negative error code.        */
   /* * NOTE * The getGLPMProcedureStoped event will be generated when  */
   /*          the remote Glucse Device responds to the Stop Procedure  */
   /*          Request.                                                 */
int BTPSAPI GLPM_Stop_Procedure_Request(unsigned int GLPMCollectorCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned int ProcedureID)
{
   int                 ret_val;
   GLP_Entry_Info_t   *GLPEntryInfo;
   Transaction_Info_t *TransactionInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the GLP Manager has been initialized.   */
   if(Initialized)
   {
      /* GLP Manager has been initialized, let's check the input        */
      /* parameters to see if they are semi-valid.                      */
      if((GLPMCollectorCallbackID) && (ProcedureID))
      {
         /* Attempt to wait for access to the GLP Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(GLPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First search for the specified Data Callback Information.*/
            if((GLPEntryInfo = SearchGLPEntryInfoEntry(&GLPEntryInfoList, GLPMCollectorCallbackID)) != NULL)
            {
               /* Verify that we have a transaction info entry for the  */
               /* transaction to cancel.                                */
               if(((TransactionInfoPtr = SearchTransactionInfo(&TransactionInfoList, ProcedureID)) != NULL) && (TransactionInfoPtr->GLPMCollectorCallbackID == GLPEntryInfo->CallbackID))
               {
                  /* Simply call the Impl.  Manager to send the correct */
                  /* message to the server.                             */
                  ret_val = _GLPM_Stop_Procedure_Request(GLPEntryInfo->EventHandlerID, RemoteDeviceAddress, TransactionInfoPtr->GLPMProcedureID);
               }
               else
                  ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(GLPManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_GLUCOSE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

