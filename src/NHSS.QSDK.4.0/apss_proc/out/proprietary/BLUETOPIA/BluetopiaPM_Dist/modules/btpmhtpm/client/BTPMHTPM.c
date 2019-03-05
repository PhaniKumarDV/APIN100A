/*****< btpmhidm.c >***********************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMHTPM - HTP Manager for Stonestreet One Bluetooth Protocol Stack       */
/*             Platform Manager.                                              */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   03/12/13  T. Cook        Initial creation.                               */
/******************************************************************************/
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMHTPM.h"            /* BTPM HTP Manager Prototypes/Constants.    */
#include "HTPMAPI.h"             /* HTP Manager Prototypes/Constants.         */
#include "HTPMMSG.h"             /* BTPM HTP Manager Message Formats.         */
#include "HTPMGR.h"              /* HTP Manager Impl. Prototypes/Constants.   */

#include "SS1BTPM.h"             /* BTPM Main Prototypes and Constants.       */
#include "BTPMERR.h"             /* BTPM Error Prototypes/Constants.          */
#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

   /* Structure which is used to track Callback Information related to  */
   /* this module.                                                      */
typedef struct _tagHTP_Entry_Info_t
{
   unsigned int                 CallbackID;
   unsigned int                 EventHandlerID;
   unsigned long                Flags;
   HTPM_Event_Callback_t        EventCallback;
   void                        *CallbackParameter;
   struct _tagHTP_Entry_Info_t *NextHTPEntryInfoPtr;
} HTP_Entry_Info_t;

   /* The following functions are used with the Flags member of the     */
   /* HTP_Entry_Info_t structure to denote various state information.   */
#define HTP_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY           0x40000000

   /* Container structure used when dispatching registered callbacks.   */
typedef struct _tagCallbackInfo_t
{
   HTPM_Event_Callback_t  EventCallback;
   void                  *CallbackParameter;
} CallbackInfo_t;

   /* The following structure is a container structure that contains all*/
   /* of the information on an outstanding transaction.                 */
typedef struct _tagTransaction_Info_t
{
   unsigned int                   TransactionID;
   unsigned int                   HTPMTransactionID;
   unsigned int                   HTPManagerDataCallbackID;
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
static Mutex_t HTPManagerMutex;

   /* Variable which is used to hold the next (unique) Callback ID.     */
static unsigned int NextCallbackID;

   /* Variable which is used to hold the next (unique) Transaction ID.  */
static unsigned int NextTransactionID;

   /* Variable which holds the current Power State of the Device.       */
static Boolean_t CurrentPowerState;

   /* Variable which holds a pointer to the first element in the HTP    */
   /* Entry Information List (which holds all Callbacks tracked by this */
   /* module).                                                          */
static HTP_Entry_Info_t *HTPEntryInfoList;

   /* Variable which holds a pointer to the first element in the        */
   /* Transaction Information List.                                     */
static Transaction_Info_t *TransactionInfoList;

   /* Internal Function Prototypes.                                     */
static unsigned int GetNextCallbackID(void);

static HTP_Entry_Info_t *AddHTPEntryInfoEntry(HTP_Entry_Info_t **ListHead, HTP_Entry_Info_t *EntryToAdd);
static HTP_Entry_Info_t *SearchHTPEntryInfoEntry(HTP_Entry_Info_t **ListHead, unsigned int CallbackID);
static HTP_Entry_Info_t *DeleteHTPEntryInfoEntry(HTP_Entry_Info_t **ListHead, unsigned int CallbackID);
static void FreeHTPEntryInfoEntryMemory(HTP_Entry_Info_t *EntryToFree);
static void FreeHTPEntryInfoList(HTP_Entry_Info_t **ListHead);

static Transaction_Info_t *AddTransactionInfo(Transaction_Info_t **ListHead, Transaction_Info_t *EntryToAdd);
static Transaction_Info_t *DeleteTransactionInfo(Transaction_Info_t **ListHead, unsigned int TransactionID);
static Transaction_Info_t *DeleteTransactionInfoByHTPMTransactionID(Transaction_Info_t **ListHead, unsigned int HTPMTransactionID);
static void FreeTransactionInfoMemory(Transaction_Info_t *EntryToFree);
static void FreeTransactionInfoList(Transaction_Info_t **ListHead);

static void DispatchHTPEvent(HTPM_Event_Data_t *HTPMEventData, Boolean_t EventCallbacks);

static void ProcessHTPConnectedEvent(HTPM_Connected_Message_t *Message);
static void ProcessHTPDisconnectedEvent(HTPM_Disconnected_Message_t *Message);
static void ProcessGetTemperatureTypeResponseEvent(HTPM_Get_Temperature_Type_Response_Message_t *Message);
static void ProcessGetMeasurementIntervalResponseEvent(HTPM_Get_Measurement_Interval_Response_Message_t *Message);
static void ProcessSetMeasurementIntervalResponseEvent(HTPM_Set_Measurement_Interval_Response_Message_t *Message);
static void ProcessGetMeasurementIntervalValidRangeResponseEvent(HTPM_Get_Measurement_Interval_Valid_Range_Response_Message_t *Message);
static void ProcessTemperatureMeasurementEvent(HTPM_Temperature_Measurement_Message_t *Message);

static void ProcessReceivedMessage(BTPM_Message_t *Message);

static void BTPSAPI BTPMDispatchCallback_HTPM(void *CallbackParameter);
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter);

static void BTPSAPI HTPManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter);

   /* The following function is a utility function that exists to       */
   /* retrieve a unique Callback ID that can be used to add an entry    */
   /* into the HTP Entry Information List.                              */
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
static HTP_Entry_Info_t *AddHTPEntryInfoEntry(HTP_Entry_Info_t **ListHead, HTP_Entry_Info_t *EntryToAdd)
{
   HTP_Entry_Info_t *AddedEntry = NULL;
   HTP_Entry_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First let's verify that values passed in are semi-valid.          */
   if((ListHead) && (EntryToAdd))
   {
      /* Make sure that the element that we are adding seems semi-valid.*/
      if(EntryToAdd->CallbackID)
      {
         /* OK, data seems semi-valid, let's allocate a new data        */
         /* structure to add to the list.                               */
         AddedEntry = (HTP_Entry_Info_t *)BTPS_AllocateMemory(sizeof(HTP_Entry_Info_t));

         if(AddedEntry)
         {
            /* Copy All Data over.                                      */
            *AddedEntry                     = *EntryToAdd;

            /* Now Add it to the end of the list.                       */
            AddedEntry->NextHTPEntryInfoPtr = NULL;

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
                     FreeHTPEntryInfoEntryMemory(AddedEntry);
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
                     if(tmpEntry->NextHTPEntryInfoPtr)
                        tmpEntry = tmpEntry->NextHTPEntryInfoPtr;
                     else
                        break;
                  }
               }

               if(AddedEntry)
               {
                  /* Last element found, simply Add the entry.          */
                  tmpEntry->NextHTPEntryInfoPtr = AddedEntry;
               }
            }
            else
               *ListHead = AddedEntry;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", AddedEntry));

   return(AddedEntry);
}

   /* The following function searches the specified List for the        */
   /* specified Callback ID.  This function returns NULL if either the  */
   /* List Head is invalid, the Callback ID is invalid, or the specified*/
   /* Callback ID was NOT found.                                        */
static HTP_Entry_Info_t *SearchHTPEntryInfoEntry(HTP_Entry_Info_t **ListHead, unsigned int CallbackID)
{
   HTP_Entry_Info_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", CallbackID));

   /* Let's make sure the list and Callback ID to search for appear to  */
   /* be valid.                                                         */
   if((ListHead) && (CallbackID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->CallbackID != CallbackID))
         FoundEntry = FoundEntry->NextHTPEntryInfoPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* The following function searches the specified HTP Entry           */
   /* Information List for the specified Callback ID and removes it from*/
   /* the List.  This function returns NULL if either the HTP Entry     */
   /* Information List Head is invalid, the Callback ID is invalid, or  */
   /* the specified Callback ID was NOT present in the list.  The entry */
   /* returned will have the Next Entry field set to NULL, and the      */
   /* caller is responsible for deleting the memory associated with this*/
   /* entry by calling FreeHTPEntryInfoEntryMemory().                   */
static HTP_Entry_Info_t *DeleteHTPEntryInfoEntry(HTP_Entry_Info_t **ListHead, unsigned int CallbackID)
{
   HTP_Entry_Info_t *FoundEntry = NULL;
   HTP_Entry_Info_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", CallbackID));

   /* Let's make sure the List and Callback ID to search for appear to  */
   /* be semi-valid.                                                    */
   if((ListHead) && (CallbackID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->CallbackID != CallbackID))
      {
         LastEntry  = FoundEntry;
         FoundEntry = FoundEntry->NextHTPEntryInfoPtr;
      }

      /* Check to see if we found the specified entry.                  */
      if(FoundEntry)
      {
         /* OK, now let's remove the entry from the list.  We have to   */
         /* check to see if the entry was the first entry in the list.  */
         if(LastEntry)
         {
            /* Entry was NOT the first entry in the list.               */
            LastEntry->NextHTPEntryInfoPtr = FoundEntry->NextHTPEntryInfoPtr;
         }
         else
            *ListHead = FoundEntry->NextHTPEntryInfoPtr;

         FoundEntry->NextHTPEntryInfoPtr = NULL;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* This function frees the specified HTP Entry Information member.   */
   /* No check is done on this entry other than making sure it NOT NULL.*/
static void FreeHTPEntryInfoEntryMemory(HTP_Entry_Info_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToFree)
      BTPS_FreeMemory(EntryToFree);

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified HTP Entry Information List.  Upon return */
   /* of this function, the Head Pointer is set to NULL.                */
static void FreeHTPEntryInfoList(HTP_Entry_Info_t **ListHead)
{
   HTP_Entry_Info_t *EntryToFree;
   HTP_Entry_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ListHead)
   {
      /* Simply traverse the list and free every element present.       */
      EntryToFree = *ListHead;

      while(EntryToFree)
      {
         tmpEntry    = EntryToFree;
         EntryToFree = EntryToFree->NextHTPEntryInfoPtr;

         FreeHTPEntryInfoEntryMemory(tmpEntry);
      }

      /* Make sure the List appears to be empty.                        */
      *ListHead = NULL;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function adds the specified Entry to the specified  */
   /* List.  This function allocates and adds an entry to the list that */
   /* has the same attributes as the Entry passed into this function.   */
   /* This function will return NULL if NO Entry was added.  This can   */
   /* occur if the element passed in was deemed invalid or the actual   */
   /* List Head was invalid.                                            */
   /* ** NOTE ** This function does not insert duplicate entries into   */
   /*            the list.  An element is considered a duplicate if the */
   /*            the TransactionID field is the same as an entry already*/
   /*            in the list.  When this occurs, this function returns  */
   /*            NULL.                                                  */
static Transaction_Info_t *AddTransactionInfo(Transaction_Info_t **ListHead, Transaction_Info_t *EntryToAdd)
{
   Transaction_Info_t *AddedEntry = NULL;
   Transaction_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First let's verify that values passed in are semi-valid.          */
   if((ListHead) && (EntryToAdd))
   {
      /* Make sure that the element that we are adding seems semi-valid.*/
      if(EntryToAdd->TransactionID)
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
                  if(tmpEntry->TransactionID == AddedEntry->TransactionID)
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

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", AddedEntry));

   return(AddedEntry);
}

   /* The following function searches the specified Transaction Info    */
   /* List for the Transaction Info with the specified TransactionID and*/
   /* removes it from the List.  This function returns NULL if either   */
   /* the Transaction Info List Head is invalid, the Transaction ID is  */
   /* invalid, or the specified Entry was NOT present in the list.  The */
   /* entry returned will have the Next Entry field set to NULL, and the*/
   /* caller is responsible for deleting the memory associated with this*/
   /* entry by calling FreeTransactionInfoMemory().                     */
static Transaction_Info_t *DeleteTransactionInfo(Transaction_Info_t **ListHead, unsigned int TransactionID)
{
   Transaction_Info_t *FoundEntry = NULL;
   Transaction_Info_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's make sure the list and Transaction ID to search for appear  */
   /* to be valid.                                                      */
   if((ListHead) && (TransactionID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->TransactionID != TransactionID))
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

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* The following function searches the specified Transaction Info    */
   /* List for the Transaction Info with the specified HTPM             */
   /* TransactionID and removes it from the List.  This function returns*/
   /* NULL if either the Transaction Info List Head is invalid, the HTPM*/
   /* Transaction ID is invalid, or the specified Entry was NOT present */
   /* in the list.  The entry returned will have the Next Entry field   */
   /* set to NULL, and the caller is responsible for deleting the memory*/
   /* associated with this entry by calling FreeTransactionInfoMemory().*/
static Transaction_Info_t *DeleteTransactionInfoByHTPMTransactionID(Transaction_Info_t **ListHead, unsigned int HTPMTransactionID)
{
   Transaction_Info_t *FoundEntry = NULL;
   Transaction_Info_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's make sure the list and GATM Transaction ID to search for    */
   /* appear to be valid.                                               */
   if((ListHead) && (HTPMTransactionID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->HTPMTransactionID != HTPMTransactionID))
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

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* This function frees the specified Transaction Information member. */
   /* No check is done on this entry other than making sure it NOT NULL.*/
static void FreeTransactionInfoMemory(Transaction_Info_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToFree)
      BTPS_FreeMemory(EntryToFree);

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified Transaction Information List.  Upon      */
   /* return of this function, the Head Pointer is set to NULL.         */
static void FreeTransactionInfoList(Transaction_Info_t **ListHead)
{
   Transaction_Info_t *EntryToFree;
   Transaction_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch the specified HTP event to every registered HTP Event    */
   /* Callback.                                                         */
   /* * NOTE * This function should be called with the HTP Manager Mutex*/
   /*          held.  Upon exit from this function it will free the HTP */
   /*          Manager Mutex.                                           */
static void DispatchHTPEvent(HTPM_Event_Data_t *HTPMEventData, Boolean_t EventCallbacks)
{
   unsigned int      Index;
   unsigned int      NumberCallbacks;
   CallbackInfo_t    CallbackInfoArray[16];
   CallbackInfo_t   *CallbackInfoArrayPtr;
   HTP_Entry_Info_t *HTPEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure that the input parameters appear to be semi-valid.      */
   if((HTPEntryInfoList) && (HTPMEventData))
   {
      /* Next, let's determine how many callbacks are registered.       */
      NumberCallbacks = 0;

      /* First, add the default event handlers.                         */
      if(EventCallbacks)
      {
         HTPEntryInfo    = HTPEntryInfoList;
         while(HTPEntryInfo)
         {
            if((HTPEntryInfo->EventCallback) && (HTPEntryInfo->Flags & HTP_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY))
               NumberCallbacks++;

            HTPEntryInfo = HTPEntryInfo->NextHTPEntryInfoPtr;
         }
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
            if(EventCallbacks)
            {
               HTPEntryInfo    = HTPEntryInfoList;
               while(HTPEntryInfo)
               {
                  if((HTPEntryInfo->EventCallback) && (HTPEntryInfo->Flags & HTP_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY))
                  {
                     CallbackInfoArrayPtr[NumberCallbacks].EventCallback     = HTPEntryInfo->EventCallback;
                     CallbackInfoArrayPtr[NumberCallbacks].CallbackParameter = HTPEntryInfo->CallbackParameter;

                     NumberCallbacks++;
                  }

                  HTPEntryInfo = HTPEntryInfo->NextHTPEntryInfoPtr;
               }
            }

            /* Release the Mutex because we have already built the      */
            /* Callback Array.                                          */
            BTPS_ReleaseMutex(HTPManagerMutex);

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
                     (*CallbackInfoArrayPtr[Index].EventCallback)(HTPMEventData, CallbackInfoArrayPtr[Index].CallbackParameter);
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
            BTPS_ReleaseMutex(HTPManagerMutex);
         }
      }
      else
      {
         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(HTPManagerMutex);
      }
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(HTPManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the HTP      */
   /* Connected asynchronous message.                                   */
static void ProcessHTPConnectedEvent(HTPM_Connected_Message_t *Message)
{
   HTPM_Event_Data_t HTPMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if(Message)
   {
      /* Format up the Event.                                           */
      HTPMEventData.EventType                                        = hetHTPConnected;
      HTPMEventData.EventLength                                      = HTPM_CONNECTED_EVENT_DATA_SIZE;

      HTPMEventData.EventData.ConnectedEventData.ConnectionType      = Message->ConnectionType;
      HTPMEventData.EventData.ConnectedEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;
      HTPMEventData.EventData.ConnectedEventData.ConnectedFlags      = Message->ConnectedFlags;

      /* Now that the event is formatted, dispatch it.                  */
      DispatchHTPEvent(&HTPMEventData, TRUE);
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(HTPManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the HTP      */
   /* Disconnected asynchronous message.                                */
static void ProcessHTPDisconnectedEvent(HTPM_Disconnected_Message_t *Message)
{
   HTPM_Event_Data_t HTPMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if(Message)
   {
      /* Format up the Event.                                           */
      HTPMEventData.EventType                                           = hetHTPDisconnected;
      HTPMEventData.EventLength                                         = HTPM_DISCONNECTED_EVENT_DATA_SIZE;

      HTPMEventData.EventData.DisconnectedEventData.ConnectionType      = Message->ConnectionType;
      HTPMEventData.EventData.DisconnectedEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;

      /* Now that the event is formatted, dispatch it.                  */
      DispatchHTPEvent(&HTPMEventData, TRUE);
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(HTPManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Get      */
   /* Temperature Type Response asynchronous message.                   */
static void ProcessGetTemperatureTypeResponseEvent(HTPM_Get_Temperature_Type_Response_Message_t *Message)
{
   void                  *CallbackParameter;
   HTP_Entry_Info_t      *HTPEntryInfo;
   HTPM_Event_Data_t      HTPMEventData;
   Transaction_Info_t    *TransactionInfo;
   HTPM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if(Message)
   {
      /* Search for and delete the Transaction Info for the request that*/
      /* generated this response.                                       */
      if((TransactionInfo = DeleteTransactionInfoByHTPMTransactionID(&TransactionInfoList, Message->TransactionID)) != NULL)
      {
         /* Search for the callback that this event should be sent to.  */
         if((HTPEntryInfo = SearchHTPEntryInfoEntry(&HTPEntryInfoList, TransactionInfo->HTPManagerDataCallbackID)) != NULL)
         {
            /* Format up the Event.                                     */
            HTPMEventData.EventType                                                         = hetHTPGetTemperatureTypeResponse;
            HTPMEventData.EventLength                                                       = HTPM_GET_TEMPERATURE_TYPE_RESPONSE_EVENT_DATA_SIZE;

            HTPMEventData.EventData.GetTemperatureTypeResponseEventData.TransactionID       = TransactionInfo->TransactionID;
            HTPMEventData.EventData.GetTemperatureTypeResponseEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;
            HTPMEventData.EventData.GetTemperatureTypeResponseEventData.Success             = Message->Success;
            HTPMEventData.EventData.GetTemperatureTypeResponseEventData.AttributeErrorCode  = Message->AttributeErrorCode;
            HTPMEventData.EventData.GetTemperatureTypeResponseEventData.TemperatureType     = Message->TemperatureType;

            /* Note the Callback Information.                           */
            EventCallback     = HTPEntryInfo->EventCallback;
            CallbackParameter = HTPEntryInfo->CallbackParameter;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HTPManagerMutex);

            __BTPSTRY
            {
               (*EventCallback)(&HTPMEventData, CallbackParameter);
            }
            __BTPSEXCEPT(1)
            {
               /* Do Nothing.                                           */
            }
         }
         else
         {
            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HTPManagerMutex);
         }

         /* Free the memory that is allocated for this entry.           */
         FreeTransactionInfoMemory(TransactionInfo);
      }
      else
      {
         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(HTPManagerMutex);
      }
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(HTPManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Get      */
   /* Measurement Interval Response asynchronous message.               */
static void ProcessGetMeasurementIntervalResponseEvent(HTPM_Get_Measurement_Interval_Response_Message_t *Message)
{
   void                  *CallbackParameter;
   unsigned int           TransactionID;
   unsigned int           CallbackID;
   HTP_Entry_Info_t      *HTPEntryInfo;
   HTPM_Event_Data_t      HTPMEventData;
   Transaction_Info_t    *TransactionInfo;
   HTPM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if(Message)
   {
      /* Check to see if this event was generated asynchronously.       */
      if(Message->TransactionID)
      {
         /* Search for and delete the Transaction Info for the request  */
         /* that generated this response.                               */
         if((TransactionInfo = DeleteTransactionInfoByHTPMTransactionID(&TransactionInfoList, Message->TransactionID)) != NULL)
         {
            TransactionID = TransactionInfo->TransactionID;
            CallbackID    = TransactionInfo->HTPManagerDataCallbackID;

            FreeTransactionInfoMemory(TransactionInfo);
         }
         else
         {
            TransactionID = 0;
            CallbackID    = 0;
         }
      }
      else
      {
         TransactionID = 0;
         CallbackID    = 0;
      }

      /* Format up the Event.                                           */
      HTPMEventData.EventType                                                             = hetHTPGetMeasurementIntervalResponse;
      HTPMEventData.EventLength                                                           = HTPM_GET_MEASUREMENT_INTERVAL_RESPONSE_EVENT_DATA_SIZE;

      HTPMEventData.EventData.GetMeasurementIntervalResponseEventData.TransactionID       = TransactionID;
      HTPMEventData.EventData.GetMeasurementIntervalResponseEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;
      HTPMEventData.EventData.GetMeasurementIntervalResponseEventData.Success             = Message->Success;
      HTPMEventData.EventData.GetMeasurementIntervalResponseEventData.AttributeErrorCode  = Message->AttributeErrorCode;
      HTPMEventData.EventData.GetMeasurementIntervalResponseEventData.MeasurementInterval = Message->MeasurementInterval;

      /* Check to see if we are dispatching this to a specific callback */
      /* or to every callback.                                          */
      if(TransactionID)
      {
         /* Search for the callback that this event should be sent to.  */
         if((HTPEntryInfo = SearchHTPEntryInfoEntry(&HTPEntryInfoList, CallbackID)) != NULL)
         {
            /* Note the Callback Information.                           */
            EventCallback     = HTPEntryInfo->EventCallback;
            CallbackParameter = HTPEntryInfo->CallbackParameter;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HTPManagerMutex);

            __BTPSTRY
            {
               (*EventCallback)(&HTPMEventData, CallbackParameter);
            }
            __BTPSEXCEPT(1)
            {
               /* Do Nothing.                                           */
            }
         }
         else
         {
            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HTPManagerMutex);
         }
      }
      else
      {
         /* If the event is a global event dispatch to all callbacks.   */
         if(Message->TransactionID == 0)
         {
            /* Now that the event is formatted, dispatch it.            */
            DispatchHTPEvent(&HTPMEventData, TRUE);
         }
         else
         {
            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HTPManagerMutex);
         }
      }
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(HTPManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Set      */
   /* Measurement Interval Response asynchronous message.               */
static void ProcessSetMeasurementIntervalResponseEvent(HTPM_Set_Measurement_Interval_Response_Message_t *Message)
{
   void                  *CallbackParameter;
   HTP_Entry_Info_t      *HTPEntryInfo;
   HTPM_Event_Data_t      HTPMEventData;
   Transaction_Info_t    *TransactionInfo;
   HTPM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if(Message)
   {
      /* Search for and delete the Transaction Info for the request that*/
      /* generated this response.                                       */
      if((TransactionInfo = DeleteTransactionInfoByHTPMTransactionID(&TransactionInfoList, Message->TransactionID)) != NULL)
      {
         /* Search for the callback that this event should be sent to.  */
         if((HTPEntryInfo = SearchHTPEntryInfoEntry(&HTPEntryInfoList, TransactionInfo->HTPManagerDataCallbackID)) != NULL)
         {
            /* Format up the Event.                                     */
            HTPMEventData.EventType                                                             = hetHTPSetMeasurementIntervalResponse;
            HTPMEventData.EventLength                                                           = HTPM_SET_MEASUREMENT_INTERVAL_RESPONSE_EVENT_DATA_SIZE;

            HTPMEventData.EventData.SetMeasurementIntervalResponseEventData.TransactionID       = TransactionInfo->TransactionID;
            HTPMEventData.EventData.SetMeasurementIntervalResponseEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;
            HTPMEventData.EventData.SetMeasurementIntervalResponseEventData.Success             = Message->Success;
            HTPMEventData.EventData.SetMeasurementIntervalResponseEventData.AttributeErrorCode  = Message->AttributeErrorCode;

            /* Note the Callback Information.                           */
            EventCallback     = HTPEntryInfo->EventCallback;
            CallbackParameter = HTPEntryInfo->CallbackParameter;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HTPManagerMutex);

            __BTPSTRY
            {
               (*EventCallback)(&HTPMEventData, CallbackParameter);
            }
            __BTPSEXCEPT(1)
            {
               /* Do Nothing.                                           */
            }
         }
         else
         {
            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HTPManagerMutex);
         }

         /* Free the memory that is allocated for this entry.           */
         FreeTransactionInfoMemory(TransactionInfo);
      }
      else
      {
         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(HTPManagerMutex);
      }
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(HTPManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Get      */
   /* Measurement Interval Valid Range Response asynchronous message.   */
static void ProcessGetMeasurementIntervalValidRangeResponseEvent(HTPM_Get_Measurement_Interval_Valid_Range_Response_Message_t *Message)
{
   void                  *CallbackParameter;
   HTP_Entry_Info_t      *HTPEntryInfo;
   HTPM_Event_Data_t      HTPMEventData;
   Transaction_Info_t    *TransactionInfo;
   HTPM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if(Message)
   {
      /* Search for and delete the Transaction Info for the request that*/
      /* generated this response.                                       */
      if((TransactionInfo = DeleteTransactionInfoByHTPMTransactionID(&TransactionInfoList, Message->TransactionID)) != NULL)
      {
         /* Search for the callback that this event should be sent to.  */
         if((HTPEntryInfo = SearchHTPEntryInfoEntry(&HTPEntryInfoList, TransactionInfo->HTPManagerDataCallbackID)) != NULL)
         {
            /* Format up the Event.                                     */
            HTPMEventData.EventType                                                                       = hetHTPGetMeasurementIntervalValidRangeResponse;
            HTPMEventData.EventLength                                                                     = HTPM_GET_MEASUREMENT_INTERVAL_VALID_RANGE_RESPONSE_EVENT_DATA_SIZE;

            HTPMEventData.EventData.GetMeasurementIntervalValidRangeResponseEventData.TransactionID       = TransactionInfo->TransactionID;
            HTPMEventData.EventData.GetMeasurementIntervalValidRangeResponseEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;
            HTPMEventData.EventData.GetMeasurementIntervalValidRangeResponseEventData.Success             = Message->Success;
            HTPMEventData.EventData.GetMeasurementIntervalValidRangeResponseEventData.AttributeErrorCode  = Message->AttributeErrorCode;
            HTPMEventData.EventData.GetMeasurementIntervalValidRangeResponseEventData.LowerBounds         = Message->LowerBounds;
            HTPMEventData.EventData.GetMeasurementIntervalValidRangeResponseEventData.UpperBounds         = Message->UpperBounds;

            /* Note the Callback Information.                           */
            EventCallback     = HTPEntryInfo->EventCallback;
            CallbackParameter = HTPEntryInfo->CallbackParameter;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HTPManagerMutex);

            __BTPSTRY
            {
               (*EventCallback)(&HTPMEventData, CallbackParameter);
            }
            __BTPSEXCEPT(1)
            {
               /* Do Nothing.                                           */
            }
         }
         else
         {
            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HTPManagerMutex);
         }

         /* Free the memory that is allocated for this entry.           */
         FreeTransactionInfoMemory(TransactionInfo);
      }
      else
      {
         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(HTPManagerMutex);
      }
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(HTPManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the          */
   /* Temperature Measurement asynchronous message.                     */
static void ProcessTemperatureMeasurementEvent(HTPM_Temperature_Measurement_Message_t *Message)
{
   HTPM_Event_Data_t HTPMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if(Message)
   {
      /* Format up the Event.                                           */
      HTPMEventData.EventType                                                     = hetHTPTemperatureMeasurement;
      HTPMEventData.EventLength                                                   = HTPM_TEMPERATURE_MEASUREMENT_EVENT_DATA_SIZE;

      HTPMEventData.EventData.TemperatureMeasurementEventData.MeasurementFlags    = Message->MeasurementFlags;
      HTPMEventData.EventData.TemperatureMeasurementEventData.MeasurementType     = Message->MeasurementType;
      HTPMEventData.EventData.TemperatureMeasurementEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;
      HTPMEventData.EventData.TemperatureMeasurementEventData.TemperatureExponent = Message->TemperatureExponent;
      HTPMEventData.EventData.TemperatureMeasurementEventData.TemperatureMantissa = Message->TemperatureMantissa;
      HTPMEventData.EventData.TemperatureMeasurementEventData.TemperatureType     = Message->TemperatureType;
      HTPMEventData.EventData.TemperatureMeasurementEventData.TimeStamp           = Message->TimeStamp;

      /* Now that the event is formatted, dispatch it.                  */
      DispatchHTPEvent(&HTPMEventData, TRUE);
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(HTPManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is called from the Main Bluetopia Dispatch */
   /* thread in response to a Process Received Message.  This function  */
   /* SHOULD ONLY be called in this capacity and NOT under any other    */
   /* circumstances.                                                    */
   /* * NOTE * This function *MUST* be called with the HTP Manager Mutex*/
   /*          held.  This function will release the Mutex before it    */
   /*          exits (i.e. the caller SHOULD NOT RELEASE THE MUTEX).    */
static void ProcessReceivedMessage(BTPM_Message_t *Message)
{
   Boolean_t ReleaseMutex = TRUE;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", Message->MessageHeader.MessageFunction));

   /* Check to make sure this isn't some kind of spoofed response (we do*/
   /* not process responses).                                           */
   if((Message) && (!(Message->MessageHeader.MessageID & BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK)))
   {
      /* Request received, go ahead and process it.                     */
      switch(Message->MessageHeader.MessageFunction)
      {
         case HTPM_MESSAGE_FUNCTION_CONNECTED:
            DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_VERBOSE), ("Connected Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HTPM_CONNECTED_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Connected Indication Event.                           */
               ProcessHTPConnectedEvent((HTPM_Connected_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HTPM_MESSAGE_FUNCTION_DISCONNECTED:
            DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_VERBOSE), ("Disconnected Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HTPM_DISCONNECTED_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Disconnected Event.                                   */
               ProcessHTPDisconnectedEvent((HTPM_Disconnected_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HTPM_MESSAGE_FUNCTION_GET_TEMPERATURE_TYPE_RESPONSE:
            DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_VERBOSE), ("Get Temperature Type Response Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HTPM_GET_TEMPERATURE_TYPE_RESPONSE_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the Get */
               /* Temperature Type Response Event.                      */
               ProcessGetTemperatureTypeResponseEvent((HTPM_Get_Temperature_Type_Response_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HTPM_MESSAGE_FUNCTION_GET_MEASUREMENT_INTERVAL_RESPONSE:
            DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_VERBOSE), ("Get Measurement Interval Response Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HTPM_GET_MEASUREMENT_INTERVAL_RESPONSE_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the Get */
               /* Measurement Interval Response Event.                  */
               ProcessGetMeasurementIntervalResponseEvent((HTPM_Get_Measurement_Interval_Response_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HTPM_MESSAGE_FUNCTION_SET_MEASUREMENT_INTERVAL_RESPONSE:
            DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_VERBOSE), ("Set Measurement Interval Response Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HTPM_SET_MEASUREMENT_INTERVAL_RESPONSE_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the Set */
               /* Measurement Interval Response Event.                  */
               ProcessSetMeasurementIntervalResponseEvent((HTPM_Set_Measurement_Interval_Response_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HTPM_MESSAGE_FUNCTION_GET_MEASUREMENT_INTERVAL_VALID_RANGE_RESPONSE:
            DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_VERBOSE), ("Get Measurement Interval Valid Range Response Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HTPM_GET_MEASUREMENT_INTERVAL_VALID_RANGE_RESPONSE_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the Get */
               /* Measurement Interval Valid Range Response Event.      */
               ProcessGetMeasurementIntervalValidRangeResponseEvent((HTPM_Get_Measurement_Interval_Valid_Range_Response_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HTPM_MESSAGE_FUNCTION_TEMPERATURE_MEASUREMENT:
            DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_VERBOSE), ("Temperature Measurement Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HTPM_TEMPERATURE_MEASUREMENT_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Temperature Measurement Event.                        */
               ProcessTemperatureMeasurementEvent((HTPM_Temperature_Measurement_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         default:
            DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled Message\n"));
            break;
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid or Response Message Received\n"));
   }

   /* If we need to Release the Mutex then we will do it at this time.  */
   if(ReleaseMutex)
      BTPS_ReleaseMutex(HTPManagerMutex);

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process HTP Manager Asynchronous Events.            */
static void BTPSAPI BTPMDispatchCallback_HTPM(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's determine if the specified parameter appears to be          */
   /* semi-valid.                                                       */
   if(CallbackParameter)
   {
      /* Next, let's check to see if we need to process it.             */
      if(Initialized)
      {
         /* Attempt to wait for access to the HTP state information.    */
         if(BTPS_WaitMutex(HTPManagerMutex, BTPS_INFINITE_WAIT))
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

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process Server un-register asynchronous events.     */
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's determine if the specified parameter appears to be          */
   /* semi-valid.                                                       */
   if(CallbackParameter)
   {
      /* Next, let's check to see if we need to process it.             */
      if(Initialized)
      {
         /* Attempt to wait for access to the HTP state information.    */
         if(BTPS_WaitMutex(HTPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Free the HTP Entry Info List.                            */
            FreeHTPEntryInfoList(&HTPEntryInfoList);

            /* Free the Transaction Information List.                   */
            FreeTransactionInfoList(&TransactionInfoList);

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HTPManagerMutex);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function represents the Message Group Handler       */
   /* function that is installed to process all HTP Manager Messages.   */
static void BTPSAPI HTPManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the Message specified appears to be semi-valid.   */
   if(Message)
   {
      /* Next, check to see if this message is specified for our        */
      /* specific Message Group.                                        */
      if(Message->MessageHeader.MessageGroup == BTPM_MESSAGE_GROUP_HEALTH_THERMOMETER_MANAGER)
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_VERBOSE), ("HTP Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

         /* Message is destined for us, make sure we are still          */
         /* processing messages.                                        */
         if(Initialized)
         {
            /* Check to see if this message is a HTP Manager defined    */
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
               /* HTP Manager Thread.                                   */
               if(!BTPM_QueueMailboxCallback(BTPMDispatchCallback_HTPM, (void *)Message))
               {
                  DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue HTP Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

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
                        DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue HTP Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));
                     }
                  }
               }
            }
         }
         else
         {
            /* Check to see if this message is an HTP Manager defined   */
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
         DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_VERBOSE), ("Non HTP Manager Message: 0x%08X, 0x%08X\n", Message->MessageHeader.MessageGroup, Message->MessageHeader.MessageFunction));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for initializing/cleaning up*/
   /* the Bluetopia Platform Manager HTP Manager Module.  This function */
   /* should be registered with the Bluetopia Platform Manager Module   */
   /* Handler and will be called when the Platform Manager is           */
   /* initialized (or shut down).                                       */
void BTPSAPI HTPM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData)
{
   int               Result;
   HTP_Entry_Info_t *HTPEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %d\n", Initialize));

   /* First, determine if the request is to initialize or shut down.    */
   if(Initialize)
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_VERBOSE), ("Initialization Request\n"));

      if(!Initialized)
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing HTP Manager\n"));

         /* First, let's create a Mutex to guard access to this module. */
         if((HTPManagerMutex = BTPS_CreateMutex(FALSE)) != NULL)
         {
            /* Next, let's attempt to register our Message Group Handler*/
            /* to process HTP Manager messages.                         */
            if(!MSG_RegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_HEALTH_THERMOMETER_MANAGER, HTPManagerGroupHandler, NULL))
            {
               /* Initialize the actual HTP Manager Implementation      */
               /* Module (this is the module that is actually           */
               /* responsible for actually implementing the HTP Manager */
               /* functionality - this module is just the framework     */
               /* shell).                                               */
               if(!(Result = _HTPM_Initialize()))
               {
                  /* Note the current Power State.                      */
                  CurrentPowerState = (Boolean_t)((DEVM_QueryDevicePowerState() > 0)?TRUE:FALSE);

                  /* Initialize a unique, starting HTP Callback ID.     */
                  NextCallbackID    = 0x000000001;

                  /* Initialize a unique, starting Transaction ID.      */
                  NextTransactionID = 0x000000001;

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
            if(HTPManagerMutex)
               BTPS_CloseMutex(HTPManagerMutex);

            MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_HEALTH_THERMOMETER_MANAGER);

            /* Flag that none of the resources are allocated.           */
            HTPManagerMutex     = NULL;
         }
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_VERBOSE), ("HTP Manager Already initialized\n"));
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_VERBOSE), ("Shutdown Request\n"));

      if(Initialized)
      {
         /* First, let's make sure that we un-register our Message Group*/
         /* Handler (so that we do not process any more incoming        */
         /* messages).                                                  */
         MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_HEALTH_THERMOMETER_MANAGER);

         /* Wait for access to the Mutex that guards access to this     */
         /* module.                                                     */
         if(BTPS_WaitMutex(HTPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Next, Un-Register for any HTP Data Events.               */
            HTPEntryInfo = HTPEntryInfoList;
            while(HTPEntryInfo)
            {
               _HTPM_Un_Register_Collector_Events(HTPEntryInfo->EventHandlerID);

               HTPEntryInfo = HTPEntryInfo->NextHTPEntryInfoPtr;
            }

            /* Make sure we inform the HTP Manager Implementation that  */
            /* we are shutting down.                                    */
            _HTPM_Cleanup();

            /* Make sure that the HTP Entry Information List is empty.  */
            FreeHTPEntryInfoList(&HTPEntryInfoList);

            /* Free the Transaction Information List.                   */
            FreeTransactionInfoList(&TransactionInfoList);

            /* Close the HTP Manager Mutex.                             */
            BTPS_CloseMutex(HTPManagerMutex);

            /* Flag that the resources are no longer allocated.         */
            HTPManagerMutex     = NULL;
            CurrentPowerState   = FALSE;

            /* Flag that this module is no longer initialized.          */
            Initialized         = FALSE;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing asynchronous */
   /* Device Manager (DEVM) Events (including Power On/Off events).     */
   /* This function should be registered with the Bluetopia Platform    */
   /* Manager Module Handler and will be called when an asynchronous    */
   /* Device Manager event is dispatched.                               */
void BTPSAPI HTPM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HTP Manager has been initialized.   */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(BTPS_WaitMutex(HTPManagerMutex, BTPS_INFINITE_WAIT))
      {
         switch(EventData->EventType)
         {
            case detDevicePoweredOn:
               DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power On Occurred.\n"));

               /* Note the new Power State.                             */
               CurrentPowerState = TRUE;
               break;
            case detDevicePoweringOff:
            case detDevicePoweredOff:
               DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power Off Occurred.\n"));

               /* Note the new Power State.                             */
               CurrentPowerState   = FALSE;
               break;
            default:
               /* Do nothing.                                           */
               break;
         }

         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(HTPManagerMutex);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the Health           */
   /* Thermometer (HTP) Manager Service.  This Callback will be         */
   /* dispatched by the HTP Manager when various HTP Manager Events     */
   /* occur.  This function accepts the Callback Function and Callback  */
   /* Parameter (respectively) to call when a HTP Manager Event needs to*/
   /* be dispatched.  This function returns a positive (non-zero) value */
   /* if successful, or a negative return error code if there was an    */
   /* error.                                                            */
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          HTPM_Un_Register_Collector_Event_Callback() function to  */
   /*          un-register the callback from this module.               */
int BTPSAPI HTPM_Register_Collector_Event_Callback(HTPM_Event_Callback_t CallbackFunction, void *CallbackParameter)
{
   int               ret_val;
   HTP_Entry_Info_t  HTPEntryInfo;
   HTP_Entry_Info_t *HTPEntryInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HTP Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(CallbackFunction)
      {
         /* Attempt to wait for access to the HTP Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(HTPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Attempt to add an entry into the HTP Entry list.         */
            BTPS_MemInitialize(&HTPEntryInfo, 0, sizeof(HTP_Entry_Info_t));

            HTPEntryInfo.CallbackID        = GetNextCallbackID();
            HTPEntryInfo.EventCallback     = CallbackFunction;
            HTPEntryInfo.CallbackParameter = CallbackParameter;
            HTPEntryInfo.Flags             = HTP_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY;

            if((HTPEntryInfoPtr = AddHTPEntryInfoEntry(&HTPEntryInfoList, &HTPEntryInfo)) != NULL)
            {
               /* Attempt to register an event handler with the server. */
               ret_val = _HTPM_Register_Collector_Events();
               if(ret_val > 0)
               {
                  /* Collector Event Handler registered, go ahead and   */
                  /* flag success to the caller.                        */
                  HTPEntryInfoPtr->EventHandlerID = ret_val;

                  /* Return the local Callback ID to the user.          */
                  ret_val                         = HTPEntryInfoPtr->CallbackID;
               }
               else
               {
                  /* Since an error occurred delete the entry from the  */
                  /* list.                                              */
                  if((HTPEntryInfoPtr = DeleteHTPEntryInfoEntry(&HTPEntryInfoList, HTPEntryInfoPtr->CallbackID)) != NULL)
                     FreeHTPEntryInfoEntryMemory(HTPEntryInfoPtr);
               }
            }
            else
               ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HTPManagerMutex);
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
   /* un-register a previously registered HTP Manager Event Callback    */
   /* (registered via a successful call to the                          */
   /* HTPM_Register_Collector_Event_Callback() function).  This function*/
   /* accepts as input the HTP Manager Event Callback ID (return value  */
   /* from HTPM_Register_Collector_Event_Callback() function).          */
void BTPSAPI HTPM_Un_Register_Collector_Event_Callback(unsigned int HTPMCollectorCallbackID)
{
   HTP_Entry_Info_t *HTPEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HTP Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(HTPMCollectorCallbackID)
      {
         /* Attempt to wait for access to the HTP Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(HTPManagerMutex, BTPS_INFINITE_WAIT))
         {
            if((HTPEntryInfo = DeleteHTPEntryInfoEntry(&HTPEntryInfoList, HTPMCollectorCallbackID)) != NULL)
            {
               /* Un-register with the server.                          */
               _HTPM_Un_Register_Collector_Events(HTPEntryInfo->EventHandlerID);

               /* Free the memory because we are finished with it.      */
               FreeHTPEntryInfoEntryMemory(HTPEntryInfo);
            }

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HTPManagerMutex);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism of        */
   /* performing a HTP Get Temperature Type procedure to a remote HTP   */
   /* Sensor.  This function accepts as input the HTP Collector Callback*/
   /* ID (return value from HTPM_Register_Collector_Event_Callback()    */
   /* function), and the BD_ADDR of the remote HTP Sensor.  This        */
   /* function returns the positive, non-zero, Transaction ID of the    */
   /* request on success or a negative error code.                      */
   /* * NOTE * The hetHTPGetTemperatureTypeResponse event will be       */
   /*          generated when the remote HTP Sensor responds to the Get */
   /*          Temperature Type Request.                                */
int BTPSAPI HTPM_Get_Temperature_Type_Request(unsigned int HTPMCollectorCallbackID, BD_ADDR_t RemoteDeviceAddress)
{
   int                 ret_val;
   HTP_Entry_Info_t   *HTPEntryInfo;
   Transaction_Info_t  TransactionInfo;
   Transaction_Info_t *TransactionInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HTP Manager has been initialized.   */
   if(Initialized)
   {
      /* HTP Manager has been initialized, let's check the input        */
      /* parameters to see if they are semi-valid.                      */
      if(HTPMCollectorCallbackID)
      {
         /* Attempt to wait for access to the HTP Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(HTPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First search for the specified Data Callback Information.*/
            if((HTPEntryInfo = SearchHTPEntryInfoEntry(&HTPEntryInfoList, HTPMCollectorCallbackID)) != NULL)
            {
               /* Initialize the Transaction Information entry.         */
               BTPS_MemInitialize(&TransactionInfo, 0, sizeof(Transaction_Info_t));

               TransactionInfo.TransactionID            = GetNextTransactionID();
               TransactionInfo.HTPManagerDataCallbackID = HTPEntryInfo->CallbackID;

               /* Add the Transaction Information to the Transaction    */
               /* Info List.                                            */
               if((TransactionInfoPtr = AddTransactionInfo(&TransactionInfoList, &TransactionInfo)) != NULL)
               {
                  /* Simply call the Impl.  Manager to send the correct */
                  /* message to the server.                             */
                  ret_val = _HTPM_Get_Temperature_Type_Request(HTPEntryInfo->EventHandlerID, RemoteDeviceAddress);
                  if(ret_val > 0)
                  {
                     /* Save the HTPM Transaction ID.                   */
                     TransactionInfoPtr->HTPMTransactionID = (unsigned int)ret_val;

                     /* Return the Local Transaction ID to the caller.  */
                     ret_val                               = (int)TransactionInfoPtr->TransactionID;
                  }
                  else
                  {
                     /* An error occurred so delete the transaction     */
                     /* information that was added to the list.         */
                     if((TransactionInfoPtr = DeleteTransactionInfo(&TransactionInfoList, TransactionInfoPtr->TransactionID)) != NULL)
                        FreeTransactionInfoMemory(TransactionInfoPtr);
                  }
               }
               else
                  ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HTPManagerMutex);
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

   return(ret_val);
}

   /* The following function is provided to allow a mechanism of        */
   /* performing a HTP Get Measurement Interval procedure to a remote   */
   /* HTP Sensor.  This function accepts as input the HTP Collector     */
   /* Callback ID (return value from                                    */
   /* HTPM_Register_Collector_Event_Callback() function), and the       */
   /* BD_ADDR of the remote HTP Sensor.  This function returns the      */
   /* positive, non-zero, Transaction ID of the request on success or a */
   /* negative error code.                                              */
   /* * NOTE * The hetHTPGetMeasurementIntervalResponse event will be   */
   /*          generated when the remote HTP Sensor responds to the Get */
   /*          Measurement Interval Request.                            */
int BTPSAPI HTPM_Get_Measurement_Interval_Request(unsigned int HTPMCollectorCallbackID, BD_ADDR_t RemoteDeviceAddress)
{
   int                 ret_val;
   HTP_Entry_Info_t   *HTPEntryInfo;
   Transaction_Info_t  TransactionInfo;
   Transaction_Info_t *TransactionInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HTP Manager has been initialized.   */
   if(Initialized)
   {
      /* HTP Manager has been initialized, let's check the input        */
      /* parameters to see if they are semi-valid.                      */
      if(HTPMCollectorCallbackID)
      {
         /* Attempt to wait for access to the HTP Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(HTPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First search for the specified Data Callback Information.*/
            if((HTPEntryInfo = SearchHTPEntryInfoEntry(&HTPEntryInfoList, HTPMCollectorCallbackID)) != NULL)
            {
               /* Initialize the Transaction Information entry.         */
               BTPS_MemInitialize(&TransactionInfo, 0, sizeof(Transaction_Info_t));

               TransactionInfo.TransactionID            = GetNextTransactionID();
               TransactionInfo.HTPManagerDataCallbackID = HTPEntryInfo->CallbackID;

               /* Add the Transaction Information to the Transaction    */
               /* Info List.                                            */
               if((TransactionInfoPtr = AddTransactionInfo(&TransactionInfoList, &TransactionInfo)) != NULL)
               {
                  /* Simply call the Impl.  Manager to send the correct */
                  /* message to the server.                             */
                  ret_val = _HTPM_Get_Measurement_Interval_Request(HTPEntryInfo->EventHandlerID, RemoteDeviceAddress);
                  if(ret_val > 0)
                  {
                     /* Save the HTPM Transaction ID.                   */
                     TransactionInfoPtr->HTPMTransactionID = (unsigned int)ret_val;

                     /* Return the Local Transaction ID to the caller.  */
                     ret_val                               = (int)TransactionInfoPtr->TransactionID;
                  }
                  else
                  {
                     /* An error occurred so delete the transaction     */
                     /* information that was added to the list.         */
                     if((TransactionInfoPtr = DeleteTransactionInfo(&TransactionInfoList, TransactionInfoPtr->TransactionID)) != NULL)
                        FreeTransactionInfoMemory(TransactionInfoPtr);
                  }
               }
               else
                  ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HTPManagerMutex);
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

   return(ret_val);
}

   /* The following function is provided to allow a mechanism of        */
   /* performing a HTP Set Measurement Interval procedure to a remote   */
   /* HTP Sensor.  This function accepts as input the HTP Collector     */
   /* Callback ID (return value from                                    */
   /* HTPM_Register_Collector_Event_Callback() function), the BD_ADDR of*/
   /* the remote HTP Sensor, and the Measurement Interval to attempt to */
   /* set.  This function returns the positive, non-zero, Transaction ID*/
   /* of the request on success or a negative error code.               */
   /* * NOTE * The hetHTPSetMeasurementIntervalResponse event will be   */
   /*          generated when the remote HTP Sensor responds to the Set */
   /*          Measurement Interval Request.                            */
int BTPSAPI HTPM_Set_Measurement_Interval_Request(unsigned int HTPMCollectorCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned int MeasurementInterval)
{
   int                 ret_val;
   HTP_Entry_Info_t   *HTPEntryInfo;
   Transaction_Info_t  TransactionInfo;
   Transaction_Info_t *TransactionInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HTP Manager has been initialized.   */
   if(Initialized)
   {
      /* HTP Manager has been initialized, let's check the input        */
      /* parameters to see if they are semi-valid.                      */
      if(HTPMCollectorCallbackID)
      {
         /* Attempt to wait for access to the HTP Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(HTPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First search for the specified Data Callback Information.*/
            if((HTPEntryInfo = SearchHTPEntryInfoEntry(&HTPEntryInfoList, HTPMCollectorCallbackID)) != NULL)
            {
               /* Initialize the Transaction Information entry.         */
               BTPS_MemInitialize(&TransactionInfo, 0, sizeof(Transaction_Info_t));

               TransactionInfo.TransactionID            = GetNextTransactionID();
               TransactionInfo.HTPManagerDataCallbackID = HTPEntryInfo->CallbackID;

               /* Add the Transaction Information to the Transaction    */
               /* Info List.                                            */
               if((TransactionInfoPtr = AddTransactionInfo(&TransactionInfoList, &TransactionInfo)) != NULL)
               {
                  /* Simply call the Impl.  Manager to send the correct */
                  /* message to the server.                             */
                  ret_val = _HTPM_Set_Measurement_Interval_Request(HTPEntryInfo->EventHandlerID, RemoteDeviceAddress, MeasurementInterval);
                  if(ret_val > 0)
                  {
                     /* Save the HTPM Transaction ID.                   */
                     TransactionInfoPtr->HTPMTransactionID = (unsigned int)ret_val;

                     /* Return the Local Transaction ID to the caller.  */
                     ret_val                               = (int)TransactionInfoPtr->TransactionID;
                  }
                  else
                  {
                     /* An error occurred so delete the transaction     */
                     /* information that was added to the list.         */
                     if((TransactionInfoPtr = DeleteTransactionInfo(&TransactionInfoList, TransactionInfoPtr->TransactionID)) != NULL)
                        FreeTransactionInfoMemory(TransactionInfoPtr);
                  }
               }
               else
                  ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HTPManagerMutex);
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

   return(ret_val);
}

   /* The following function is provided to allow a mechanism of        */
   /* performing a HTP Get Measurement Interval Valid Range procedure to*/
   /* a remote HTP Sensor.  This function accepts as input the HTP      */
   /* Collector Callback ID (return value from                          */
   /* HTPM_Register_Collector_Event_Callback() function), and the       */
   /* BD_ADDR of the remote HTP Sensor.  This function returns the      */
   /* positive, non-zero, Transaction ID of the request on success or a */
   /* negative error code.                                              */
   /* * NOTE * The hetHTPGetMeasurementIntervalValidRangeResponse event */
   /*          will be generated when the remote HTP Sensor responds to */
   /*          the Get Measurement Interval Request.                    */
int BTPSAPI HTPM_Get_Measurement_Interval_Valid_Range_Request(unsigned int HTPMCollectorCallbackID, BD_ADDR_t RemoteDeviceAddress)
{
   int                 ret_val;
   HTP_Entry_Info_t   *HTPEntryInfo;
   Transaction_Info_t  TransactionInfo;
   Transaction_Info_t *TransactionInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HTP Manager has been initialized.   */
   if(Initialized)
   {
      /* HTP Manager has been initialized, let's check the input        */
      /* parameters to see if they are semi-valid.                      */
      if(HTPMCollectorCallbackID)
      {
         /* Attempt to wait for access to the HTP Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(HTPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First search for the specified Data Callback Information.*/
            if((HTPEntryInfo = SearchHTPEntryInfoEntry(&HTPEntryInfoList, HTPMCollectorCallbackID)) != NULL)
            {
               /* Initialize the Transaction Information entry.         */
               BTPS_MemInitialize(&TransactionInfo, 0, sizeof(Transaction_Info_t));

               TransactionInfo.TransactionID            = GetNextTransactionID();
               TransactionInfo.HTPManagerDataCallbackID = HTPEntryInfo->CallbackID;

               /* Add the Transaction Information to the Transaction    */
               /* Info List.                                            */
               if((TransactionInfoPtr = AddTransactionInfo(&TransactionInfoList, &TransactionInfo)) != NULL)
               {
                  /* Simply call the Impl.  Manager to send the correct */
                  /* message to the server.                             */
                  ret_val = _HTPM_Get_Measurement_Interval_Valid_Range_Request(HTPEntryInfo->EventHandlerID, RemoteDeviceAddress);
                  if(ret_val > 0)
                  {
                     /* Save the HTPM Transaction ID.                   */
                     TransactionInfoPtr->HTPMTransactionID = (unsigned int)ret_val;

                     /* Return the Local Transaction ID to the caller.  */
                     ret_val                               = (int)TransactionInfoPtr->TransactionID;
                  }
                  else
                  {
                     /* An error occurred so delete the transaction     */
                     /* information that was added to the list.         */
                     if((TransactionInfoPtr = DeleteTransactionInfo(&TransactionInfoList, TransactionInfoPtr->TransactionID)) != NULL)
                        FreeTransactionInfoMemory(TransactionInfoPtr);
                  }
               }
               else
                  ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HTPManagerMutex);
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

   return(ret_val);
}

