/*****< btpmhidm.c >***********************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMHOGM - HOG Manager for Stonestreet One Bluetooth Protocol Stack       */
/*             Platform Manager.                                              */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   01/16/13  T. Cook        Initial creation.                               */
/******************************************************************************/
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMHOGM.h"            /* BTPM HID Manager Prototypes/Constants.    */
#include "HOGMAPI.h"             /* HID Manager Prototypes/Constants.         */
#include "HOGMMSG.h"             /* BTPM HID Manager Message Formats.         */
#include "HOGMGR.h"              /* HID Manager Impl. Prototypes/Constants.   */

#include "SS1BTPM.h"             /* BTPM Main Prototypes and Constants.       */
#include "BTPMERR.h"             /* BTPM Error Prototypes/Constants.          */
#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

   /* Structure which is used to track Callback Information related to  */
   /* this module.                                                      */
typedef struct _tagHOG_Entry_Info_t
{
   unsigned int                 CallbackID;
   unsigned int                 EventHandlerID;
   unsigned long                Flags;
   HOGM_Event_Callback_t        EventCallback;
   void                        *CallbackParameter;
   struct _tagHOG_Entry_Info_t *NextHOGEntryInfoPtr;
} HOG_Entry_Info_t;

   /* The following functions are used with the Flags member of the     */
   /* HID_Entry_Info_t structure to denote various state information.   */
#define HID_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY           0x40000000

   /* Container structure used when dispatching registered callbacks.   */
typedef struct _tagCallbackInfo_t
{
   HOGM_Event_Callback_t  EventCallback;
   void                  *CallbackParameter;
} CallbackInfo_t;

   /* The following structure is a container structure that contains all*/
   /* of the information on an outstanding transaction.                 */
typedef struct _tagTransaction_Info_t
{
   unsigned int                   TransactionID;
   unsigned int                   HOGMTransactionID;
   unsigned int                   HOGManagerDataCallbackID;
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
static Mutex_t HOGManagerMutex;

   /* Variable which is used to hold the next (unique) Callback ID.     */
static unsigned int NextCallbackID;

   /* Variable which is used to hold the next (unique) Transaction ID.  */
static unsigned int NextTransactionID;

   /* Variable which holds the current Power State of the Device.       */
static Boolean_t CurrentPowerState;

   /* Variable which holds the current HID Events Callback ID           */
   /* (registered with the Server to receive events).                   */
static unsigned int HOGEventsCallbackID;

   /* Variable which holds a pointer to the first element in the HOG    */
   /* Entry Information List (which holds all Callbacks tracked by this */
   /* module).                                                          */
static HOG_Entry_Info_t *HOGEntryInfoList;

   /* Variable which holds a pointer to the first element of the HOG    */
   /* Entry Information List for Data Event Callbacks (which holds all  */
   /* Data Event Callbacks tracked by this module).                     */
static HOG_Entry_Info_t *HOGEntryInfoDataList;

   /* Variable which holds a pointer to the first element in the        */
   /* Transaction Information List.                                     */
static Transaction_Info_t *TransactionInfoList;

   /* Internal Function Prototypes.                                     */
static unsigned int GetNextCallbackID(void);

static HOG_Entry_Info_t *AddHOGEntryInfoEntry(HOG_Entry_Info_t **ListHead, HOG_Entry_Info_t *EntryToAdd);
static HOG_Entry_Info_t *SearchHOGEntryInfoEntry(HOG_Entry_Info_t **ListHead, unsigned int CallbackID);
static HOG_Entry_Info_t *DeleteHOGEntryInfoEntry(HOG_Entry_Info_t **ListHead, unsigned int CallbackID);
static void FreeHOGEntryInfoEntryMemory(HOG_Entry_Info_t *EntryToFree);
static void FreeHOGEntryInfoList(HOG_Entry_Info_t **ListHead);

static Transaction_Info_t *AddTransactionInfo(Transaction_Info_t **ListHead, Transaction_Info_t *EntryToAdd);
static Transaction_Info_t *DeleteTransactionInfo(Transaction_Info_t **ListHead, unsigned int TransactionID);
static Transaction_Info_t *DeleteTransactionInfoByHOGMTransactionID(Transaction_Info_t **ListHead, unsigned int HOGMTransactionID);
static void FreeTransactionInfoMemory(Transaction_Info_t *EntryToFree);
static void FreeTransactionInfoList(Transaction_Info_t **ListHead);

static void DispatchHIDEvent(HOGM_Event_Data_t *HOGMEventData, Boolean_t EventCallbacks, Boolean_t DataCallbacks);

static void ProcessHIDDeviceConnectedEvent(HOGM_HID_Device_Connected_Message_t *Message);
static void ProcessHIDDeviceDisconnectedEvent(HOGM_HID_Device_Disconnected_Message_t *Message);
static void ProcessGetReportResponseEvent(HOGM_HID_Get_Report_Response_Message_t *Message);
static void ProcessSetReportResponseEvent(HOGM_HID_Set_Report_Response_Message_t *Message);
static void ProcessDataIndicationEvent(HOGM_HID_Data_Indication_Message_t *Message);

static void ProcessReceivedMessage(BTPM_Message_t *Message);

static void BTPSAPI BTPMDispatchCallback_HOGM(void *CallbackParameter);
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter);

static void BTPSAPI HOGManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter);

   /* The following function is a utility function that exists to       */
   /* retrieve a unique Callback ID that can be used to add an entry    */
   /* into the HID Entry Information List.                              */
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
static HOG_Entry_Info_t *AddHOGEntryInfoEntry(HOG_Entry_Info_t **ListHead, HOG_Entry_Info_t *EntryToAdd)
{
   HOG_Entry_Info_t *AddedEntry = NULL;
   HOG_Entry_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First let's verify that values passed in are semi-valid.          */
   if((ListHead) && (EntryToAdd))
   {
      /* Make sure that the element that we are adding seems semi-valid.*/
      if(EntryToAdd->CallbackID)
      {
         /* OK, data seems semi-valid, let's allocate a new data        */
         /* structure to add to the list.                               */
         AddedEntry = (HOG_Entry_Info_t *)BTPS_AllocateMemory(sizeof(HOG_Entry_Info_t));

         if(AddedEntry)
         {
            /* Copy All Data over.                                      */
            *AddedEntry                     = *EntryToAdd;

            /* Now Add it to the end of the list.                       */
            AddedEntry->NextHOGEntryInfoPtr = NULL;

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
                     FreeHOGEntryInfoEntryMemory(AddedEntry);
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
                     if(tmpEntry->NextHOGEntryInfoPtr)
                        tmpEntry = tmpEntry->NextHOGEntryInfoPtr;
                     else
                        break;
                  }
               }

               if(AddedEntry)
               {
                  /* Last element found, simply Add the entry.          */
                  tmpEntry->NextHOGEntryInfoPtr = AddedEntry;
               }
            }
            else
               *ListHead = AddedEntry;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", AddedEntry));

   return(AddedEntry);
}

   /* The following function searches the specified List for the        */
   /* specified Callback ID.  This function returns NULL if either the  */
   /* List Head is invalid, the Callback ID is invalid, or the specified*/
   /* Callback ID was NOT found.                                        */
static HOG_Entry_Info_t *SearchHOGEntryInfoEntry(HOG_Entry_Info_t **ListHead, unsigned int CallbackID)
{
   HOG_Entry_Info_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", CallbackID));

   /* Let's make sure the list and Callback ID to search for appear to  */
   /* be valid.                                                         */
   if((ListHead) && (CallbackID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->CallbackID != CallbackID))
         FoundEntry = FoundEntry->NextHOGEntryInfoPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* The following function searches the specified HID Entry           */
   /* Information List for the specified Callback ID and removes it from*/
   /* the List.  This function returns NULL if either the HID Entry     */
   /* Information List Head is invalid, the Callback ID is invalid, or  */
   /* the specified Callback ID was NOT present in the list.  The entry */
   /* returned will have the Next Entry field set to NULL, and the      */
   /* caller is responsible for deleting the memory associated with this*/
   /* entry by calling FreeHOGEntryInfoEntryMemory().                   */
static HOG_Entry_Info_t *DeleteHOGEntryInfoEntry(HOG_Entry_Info_t **ListHead, unsigned int CallbackID)
{
   HOG_Entry_Info_t *FoundEntry = NULL;
   HOG_Entry_Info_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", CallbackID));

   /* Let's make sure the List and Callback ID to search for appear to  */
   /* be semi-valid.                                                    */
   if((ListHead) && (CallbackID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->CallbackID != CallbackID))
      {
         LastEntry  = FoundEntry;
         FoundEntry = FoundEntry->NextHOGEntryInfoPtr;
      }

      /* Check to see if we found the specified entry.                  */
      if(FoundEntry)
      {
         /* OK, now let's remove the entry from the list.  We have to   */
         /* check to see if the entry was the first entry in the list.  */
         if(LastEntry)
         {
            /* Entry was NOT the first entry in the list.               */
            LastEntry->NextHOGEntryInfoPtr = FoundEntry->NextHOGEntryInfoPtr;
         }
         else
            *ListHead = FoundEntry->NextHOGEntryInfoPtr;

         FoundEntry->NextHOGEntryInfoPtr = NULL;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* This function frees the specified HID Entry Information member.   */
   /* No check is done on this entry other than making sure it NOT NULL.*/
static void FreeHOGEntryInfoEntryMemory(HOG_Entry_Info_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToFree)
      BTPS_FreeMemory(EntryToFree);

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified HID Entry Information List.  Upon return */
   /* of this function, the Head Pointer is set to NULL.                */
static void FreeHOGEntryInfoList(HOG_Entry_Info_t **ListHead)
{
   HOG_Entry_Info_t *EntryToFree;
   HOG_Entry_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ListHead)
   {
      /* Simply traverse the list and free every element present.       */
      EntryToFree = *ListHead;

      while(EntryToFree)
      {
         tmpEntry    = EntryToFree;
         EntryToFree = EntryToFree->NextHOGEntryInfoPtr;

         FreeHOGEntryInfoEntryMemory(tmpEntry);
      }

      /* Make sure the List appears to be empty.                        */
      *ListHead = NULL;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
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

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", AddedEntry));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* The following function searches the specified Transaction Info    */
   /* List for the Transaction Info with the specified HOGM             */
   /* TransactionID and removes it from the List.  This function returns*/
   /* NULL if either the Transaction Info List Head is invalid, the HOGM*/
   /* Transaction ID is invalid, or the specified Entry was NOT present */
   /* in the list.  The entry returned will have the Next Entry field   */
   /* set to NULL, and the caller is responsible for deleting the memory*/
   /* associated with this entry by calling FreeTransactionInfoMemory().*/
static Transaction_Info_t *DeleteTransactionInfoByHOGMTransactionID(Transaction_Info_t **ListHead, unsigned int HOGMTransactionID)
{
   Transaction_Info_t *FoundEntry = NULL;
   Transaction_Info_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's make sure the list and GATM Transaction ID to search for    */
   /* appear to be valid.                                               */
   if((ListHead) && (HOGMTransactionID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->HOGMTransactionID != HOGMTransactionID))
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

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* This function frees the specified Transaction Information member. */
   /* No check is done on this entry other than making sure it NOT NULL.*/
static void FreeTransactionInfoMemory(Transaction_Info_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToFree)
      BTPS_FreeMemory(EntryToFree);

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified Transaction Information List.  Upon      */
   /* return of this function, the Head Pointer is set to NULL.         */
static void FreeTransactionInfoList(Transaction_Info_t **ListHead)
{
   Transaction_Info_t *EntryToFree;
   Transaction_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch the specified HID event to every registered HID Event    */
   /* Callback.                                                         */
   /* * NOTE * This function should be called with the HID Manager Mutex*/
   /*          held.  Upon exit from this function it will free the HID */
   /*          Manager Mutex.                                           */
static void DispatchHIDEvent(HOGM_Event_Data_t *HOGMEventData, Boolean_t EventCallbacks, Boolean_t DataCallbacks)
{
   unsigned int      Index;
   unsigned int      NumberCallbacks;
   CallbackInfo_t    CallbackInfoArray[16];
   CallbackInfo_t   *CallbackInfoArrayPtr;
   HOG_Entry_Info_t *HOGEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure that the input parameters appear to be semi-valid.      */
   if(((HOGEntryInfoList) || (HOGEntryInfoDataList)) && (HOGMEventData))
   {
      /* Next, let's determine how many callbacks are registered.       */
      NumberCallbacks = 0;

      /* First, add the default event handlers.                         */
      if(EventCallbacks)
      {
         HOGEntryInfo    = HOGEntryInfoList;
         while(HOGEntryInfo)
         {
            if((HOGEntryInfo->EventCallback) && (HOGEntryInfo->Flags & HID_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY))
               NumberCallbacks++;

            HOGEntryInfo = HOGEntryInfo->NextHOGEntryInfoPtr;
         }
      }

      /* We need to add the HID Data Entry Information List as well.    */
      if(DataCallbacks)
      {
         HOGEntryInfo  = HOGEntryInfoDataList;
         while(HOGEntryInfo)
         {
            if(HOGEntryInfo->EventCallback)
               NumberCallbacks++;

            HOGEntryInfo = HOGEntryInfo->NextHOGEntryInfoPtr;
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
               HOGEntryInfo    = HOGEntryInfoList;
               while(HOGEntryInfo)
               {
                  if((HOGEntryInfo->EventCallback) && (HOGEntryInfo->Flags & HID_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY))
                  {
                     CallbackInfoArrayPtr[NumberCallbacks].EventCallback     = HOGEntryInfo->EventCallback;
                     CallbackInfoArrayPtr[NumberCallbacks].CallbackParameter = HOGEntryInfo->CallbackParameter;

                     NumberCallbacks++;
                  }

                  HOGEntryInfo = HOGEntryInfo->NextHOGEntryInfoPtr;
               }
            }

            /* We need to add the HID Data Entry Information List as    */
            /* well.                                                    */
            if(DataCallbacks)
            {
               HOGEntryInfo  = HOGEntryInfoDataList;
               while(HOGEntryInfo)
               {
                  if(HOGEntryInfo->EventCallback)
                  {
                     CallbackInfoArrayPtr[NumberCallbacks].EventCallback     = HOGEntryInfo->EventCallback;
                     CallbackInfoArrayPtr[NumberCallbacks].CallbackParameter = HOGEntryInfo->CallbackParameter;

                     NumberCallbacks++;
                  }

                  HOGEntryInfo = HOGEntryInfo->NextHOGEntryInfoPtr;
               }
            }

            /* Release the Mutex because we have already built the      */
            /* Callback Array.                                          */
            BTPS_ReleaseMutex(HOGManagerMutex);

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
                     (*CallbackInfoArrayPtr[Index].EventCallback)(HOGMEventData, CallbackInfoArrayPtr[Index].CallbackParameter);
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
            BTPS_ReleaseMutex(HOGManagerMutex);
         }
      }
      else
      {
         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(HOGManagerMutex);
      }
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(HOGManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the HID      */
   /* Device Connected asynchronous message.                            */
static void ProcessHIDDeviceConnectedEvent(HOGM_HID_Device_Connected_Message_t *Message)
{
   HOGM_Event_Data_t HOGMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if(Message)
   {
      /* Format up the Event.                                           */
      HOGMEventData.EventType                                                 = hetHOGMHIDDeviceConnected;
      HOGMEventData.EventLength                                               = HOGM_HID_DEVICE_CONNECTED_EVENT_DATA_SIZE;

      HOGMEventData.EventData.DeviceConnectedEventData.RemoteDeviceAddress    = Message->RemoteDeviceAddress;
      HOGMEventData.EventData.DeviceConnectedEventData.SupportedFeatures      = Message->SupportedFeatures;
      HOGMEventData.EventData.DeviceConnectedEventData.HIDInformation         = Message->HIDInformation;
      HOGMEventData.EventData.DeviceConnectedEventData.ReportDescriptorLength = Message->ReportDescriptorLength;
      HOGMEventData.EventData.DeviceConnectedEventData.ReportDescriptor       = Message->ReportDescriptor;

      /* Now that the event is formatted, dispatch it.                  */
      DispatchHIDEvent(&HOGMEventData, TRUE, TRUE);
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(HOGManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the HID      */
   /* Device Disconnected asynchronous message.                         */
static void ProcessHIDDeviceDisconnectedEvent(HOGM_HID_Device_Disconnected_Message_t *Message)
{
   HOGM_Event_Data_t HOGMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if(Message)
   {
      /* Format up the Event.                                           */
      HOGMEventData.EventType                                                 = hetHOGMHIDDeviceDisconnected;
      HOGMEventData.EventLength                                               = HOGM_HID_DEVICE_DISCONNECTED_EVENT_DATA_SIZE;

      HOGMEventData.EventData.DeviceDisconnectedEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;

      /* Now that the event is formatted, dispatch it.                  */
      DispatchHIDEvent(&HOGMEventData, TRUE, TRUE);
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(HOGManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Get      */
   /* Report Response asynchronous message.                             */
static void ProcessGetReportResponseEvent(HOGM_HID_Get_Report_Response_Message_t *Message)
{
   void                  *CallbackParameter;
   HOG_Entry_Info_t      *HOGEntryInfo;
   HOGM_Event_Data_t      HOGMEventData;
   Transaction_Info_t    *TransactionInfo;
   HOGM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if(Message)
   {
      /* Search for and delete the Transaction Info for the request that*/
      /* generated this response.                                       */
      if((TransactionInfo = DeleteTransactionInfoByHOGMTransactionID(&TransactionInfoList, Message->TransactionID)) != NULL)
      {
         /* Search for the callback that this event should be sent to.  */
         if((HOGEntryInfo = SearchHOGEntryInfoEntry(&HOGEntryInfoDataList, TransactionInfo->HOGManagerDataCallbackID)) != NULL)
         {
            /* Format up the Event.                                     */
            HOGMEventData.EventType                                                = hetHOGMHIDGetReportResponse;
            HOGMEventData.EventLength                                              = HOGM_HID_GET_REPORT_RESPONSE_EVENT_DATA_SIZE;

            HOGMEventData.EventData.GetReportResponseEventData.TransactionID       = TransactionInfo->TransactionID;
            HOGMEventData.EventData.GetReportResponseEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;
            HOGMEventData.EventData.GetReportResponseEventData.Success             = Message->Success;
            HOGMEventData.EventData.GetReportResponseEventData.AttributeErrorCode  = Message->AttributeErrorCode;
            HOGMEventData.EventData.GetReportResponseEventData.ReportInformation   = Message->ReportInformation;
            HOGMEventData.EventData.GetReportResponseEventData.ReportDataLength    = Message->ReportDataLength;
            HOGMEventData.EventData.GetReportResponseEventData.ReportData          = Message->ReportData;

            /* Note the Callback Information.                           */
            EventCallback     = HOGEntryInfo->EventCallback;
            CallbackParameter = HOGEntryInfo->CallbackParameter;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HOGManagerMutex);

            __BTPSTRY
            {
               (*EventCallback)(&HOGMEventData, CallbackParameter);
            }
            __BTPSEXCEPT(1)
            {
               /* Do Nothing.                                           */
            }
         }
         else
         {
            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HOGManagerMutex);
         }

         /* Free the memory that is allocated for this entry.           */
         FreeTransactionInfoMemory(TransactionInfo);
      }
      else
      {
         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(HOGManagerMutex);
      }
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(HOGManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Set      */
   /* Report Response asynchronous message.                             */
static void ProcessSetReportResponseEvent(HOGM_HID_Set_Report_Response_Message_t *Message)
{
   void                  *CallbackParameter;
   HOG_Entry_Info_t      *HOGEntryInfo;
   HOGM_Event_Data_t      HOGMEventData;
   Transaction_Info_t    *TransactionInfo;
   HOGM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if(Message)
   {
      /* Search for and delete the Transaction Info for the request that*/
      /* generated this response.                                       */
      if((TransactionInfo = DeleteTransactionInfoByHOGMTransactionID(&TransactionInfoList, Message->TransactionID)) != NULL)
      {
         /* Search for the callback that this event should be sent to.  */
         if((HOGEntryInfo = SearchHOGEntryInfoEntry(&HOGEntryInfoDataList, TransactionInfo->HOGManagerDataCallbackID)) != NULL)
         {
            /* Format up the Event.                                     */
            HOGMEventData.EventType                                                = hetHOGMHIDSetReportResponse;
            HOGMEventData.EventLength                                              = HOGM_HID_SET_REPORT_RESPONSE_EVENT_DATA_SIZE;

            HOGMEventData.EventData.SetReportResponseEventData.TransactionID       = TransactionInfo->TransactionID;
            HOGMEventData.EventData.SetReportResponseEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;
            HOGMEventData.EventData.SetReportResponseEventData.Success             = Message->Success;
            HOGMEventData.EventData.SetReportResponseEventData.AttributeErrorCode  = Message->AttributeErrorCode;
            HOGMEventData.EventData.SetReportResponseEventData.ReportInformation   = Message->ReportInformation;

            /* Note the Callback Information.                           */
            EventCallback     = HOGEntryInfo->EventCallback;
            CallbackParameter = HOGEntryInfo->CallbackParameter;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HOGManagerMutex);

            __BTPSTRY
            {
               (*EventCallback)(&HOGMEventData, CallbackParameter);
            }
            __BTPSEXCEPT(1)
            {
               /* Do Nothing.                                           */
            }
         }
         else
         {
            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HOGManagerMutex);
         }

         /* Free the memory that is allocated for this entry.           */
         FreeTransactionInfoMemory(TransactionInfo);
      }
      else
      {
         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(HOGManagerMutex);
      }
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(HOGManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing the Data     */
   /* Indication asynchronous message.                                  */
static void ProcessDataIndicationEvent(HOGM_HID_Data_Indication_Message_t *Message)
{
   HOGM_Event_Data_t HOGMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if(Message)
   {
      /* Format up the Event.                                           */
      HOGMEventData.EventType                                             = hetHOGMHIDDataIndication;
      HOGMEventData.EventLength                                           = HOGM_HID_DATA_INDICATION_EVENT_DATA_SIZE;

      HOGMEventData.EventData.DataIndicationEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;
      HOGMEventData.EventData.DataIndicationEventData.ReportInformation   = Message->ReportInformation;
      HOGMEventData.EventData.DataIndicationEventData.ReportDataLength    = Message->ReportDataLength;
      HOGMEventData.EventData.DataIndicationEventData.ReportData          = Message->ReportData;

      /* Now that the event is formatted, dispatch it.                  */
      DispatchHIDEvent(&HOGMEventData, FALSE, TRUE);
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(HOGManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is called from the Main Bluetopia Dispatch */
   /* thread in response to a Process Received Message.  This function  */
   /* SHOULD ONLY be called in this capacity and NOT under any other    */
   /* circumstances.                                                    */
   /* * NOTE * This function *MUST* be called with the HID Manager Mutex*/
   /*          held.  This function will release the Mutex before it    */
   /*          exits (i.e. the caller SHOULD NOT RELEASE THE MUTEX).    */
static void ProcessReceivedMessage(BTPM_Message_t *Message)
{
   Boolean_t ReleaseMutex = TRUE;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", Message->MessageHeader.MessageFunction));

   /* Check to make sure this isn't some kind of spoofed response (we do*/
   /* not process responses).                                           */
   if((Message) && (!(Message->MessageHeader.MessageID & BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK)))
   {
      /* Request received, go ahead and process it.                     */
      switch(Message->MessageHeader.MessageFunction)
      {
         case HOGM_MESSAGE_FUNCTION_HID_DEVICE_CONNECTED:
            DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Connected Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HOGM_HID_DEVICE_CONNECTED_MESSAGE_SIZE(0))
            {
               if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HOGM_HID_DEVICE_CONNECTED_MESSAGE_SIZE(((HOGM_HID_Device_Connected_Message_t *)Message)->ReportDescriptorLength))
               {
                  /* Size seems to be valid, go ahead and dispatch the  */
                  /* Device Connected Indication Event.                 */
                  ProcessHIDDeviceConnectedEvent((HOGM_HID_Device_Connected_Message_t *)Message);

                  /* Flag that we do not need to release the Mutex (it  */
                  /* has already been released).                        */
                  ReleaseMutex = FALSE;
               }
               else
               {
                  DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
               }
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HOGM_MESSAGE_FUNCTION_HID_DEVICE_DISCONNECTED:
            DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Disconnected Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HOGM_HID_DEVICE_DISCONNECTED_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Device Disconnected Event.                            */
               ProcessHIDDeviceDisconnectedEvent((HOGM_HID_Device_Disconnected_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HOGM_MESSAGE_FUNCTION_GET_REPORT_RESPONSE_EVENT:
            DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_VERBOSE), ("Get Report Response Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HOGM_HID_GET_REPORT_RESPONSE_MESSAGE_SIZE(0))
            {
               if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HOGM_HID_GET_REPORT_RESPONSE_MESSAGE_SIZE(((HOGM_HID_Get_Report_Response_Message_t *)Message)->ReportDataLength))
               {
                  /* Size seems to be valid, go ahead and process the   */
                  /* Get Report Response Event.                         */
                  ProcessGetReportResponseEvent((HOGM_HID_Get_Report_Response_Message_t *)Message);

                  /* Flag that we do not need to release the Mutex (it  */
                  /* has already been released).                        */
                  ReleaseMutex = FALSE;
               }
               else
               {
                  DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
               }
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HOGM_MESSAGE_FUNCTION_SET_REPORT_RESPONSE_EVENT:
            DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_VERBOSE), ("Set Report Response Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HOGM_HID_SET_REPORT_RESPONSE_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and process the Set  */
               /* Report Response Event.                                */
               ProcessSetReportResponseEvent((HOGM_HID_Set_Report_Response_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HOGM_MESSAGE_FUNCTION_DATA_INDICATION_EVENT:
            DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_VERBOSE), ("Data Indication Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HOGM_HID_DATA_INDICATION_MESSAGE_SIZE(0))
            {
               if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HOGM_HID_DATA_INDICATION_MESSAGE_SIZE(((HOGM_HID_Data_Indication_Message_t *)Message)->ReportDataLength))
               {
                  /* Size seems to be valid, go ahead and process the   */
                  /* Data Indication Event.                             */
                  ProcessDataIndicationEvent((HOGM_HID_Data_Indication_Message_t *)Message);

                  /* Flag that we do not need to release the Mutex (it  */
                  /* has already been released).                        */
                  ReleaseMutex = FALSE;
               }
               else
               {
                  DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
               }
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;

         default:
            DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled Message\n"));
            break;
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid or Response Message Received\n"));
   }

   /* If we need to Release the Mutex then we will do it at this time.  */
   if(ReleaseMutex)
      BTPS_ReleaseMutex(HOGManagerMutex);

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process HID Manager Asynchronous Events.            */
static void BTPSAPI BTPMDispatchCallback_HOGM(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's determine if the specified parameter appears to be          */
   /* semi-valid.                                                       */
   if(CallbackParameter)
   {
      /* Next, let's check to see if we need to process it.             */
      if(Initialized)
      {
         /* Attempt to wait for access to the HID state information.    */
         if(BTPS_WaitMutex(HOGManagerMutex, BTPS_INFINITE_WAIT))
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

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process Server un-register asynchronous events.     */
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's determine if the specified parameter appears to be          */
   /* semi-valid.                                                       */
   if(CallbackParameter)
   {
      /* Next, let's check to see if we need to process it.             */
      if(Initialized)
      {
         /* Attempt to wait for access to the HID state information.    */
         if(BTPS_WaitMutex(HOGManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Free the HID Entry Info List.                            */
            FreeHOGEntryInfoList(&HOGEntryInfoList);
            FreeHOGEntryInfoList(&HOGEntryInfoDataList);

            /* Free the Transaction Information List.                   */
            FreeTransactionInfoList(&TransactionInfoList);

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HOGManagerMutex);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function represents the Message Group Handler       */
   /* function that is installed to process all HID Manager Messages.   */
static void BTPSAPI HOGManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the Message specified appears to be semi-valid.   */
   if(Message)
   {
      /* Next, check to see if this message is specified for our        */
      /* specific Message Group.                                        */
      if(Message->MessageHeader.MessageGroup == BTPM_MESSAGE_GROUP_HOGP_MANAGER)
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_VERBOSE), ("HID Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

         /* Message is destined for us, make sure we are still          */
         /* processing messages.                                        */
         if(Initialized)
         {
            /* Check to see if this message is a HID Manager defined    */
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
               /* HID Manager Thread.                                   */
               if(!BTPM_QueueMailboxCallback(BTPMDispatchCallback_HOGM, (void *)Message))
               {
                  DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue HID Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

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
                        DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue HID Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));
                     }
                  }
               }
            }
         }
         else
         {
            /* Check to see if this message is an HID Manager defined   */
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
         DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_VERBOSE), ("Non HID Manager Message: 0x%08X, 0x%08X\n", Message->MessageHeader.MessageGroup, Message->MessageHeader.MessageFunction));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for initializing/cleaning up*/
   /* the Bluetopia Platform Manager HID Manager Module.  This function */
   /* should be registered with the Bluetopia Platform Manager Module   */
   /* Handler and will be called when the Platform Manager is           */
   /* initialized (or shut down).                                       */
void BTPSAPI HOGM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData)
{
   int               Result;
   HOG_Entry_Info_t *HOGEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %d\n", Initialize));

   /* First, determine if the request is to initialize or shut down.    */
   if(Initialize)
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_VERBOSE), ("Initialization Request\n"));

      if(!Initialized)
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing HID Manager\n"));

         /* First, let's create a Mutex to guard access to this module. */
         if((HOGManagerMutex = BTPS_CreateMutex(FALSE)) != NULL)
         {
            /* Next, let's attempt to register our Message Group Handler*/
            /* to process HID Manager messages.                         */
            if(!MSG_RegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_HOGP_MANAGER, HOGManagerGroupHandler, NULL))
            {
               /* Initialize the actual HID Manager Implementation      */
               /* Module (this is the module that is actually           */
               /* responsible for actually implementing the HID Manager */
               /* functionality - this module is just the framework     */
               /* shell).                                               */
               if(!(Result = _HOGM_Initialize()))
               {
                  /* Note the current Power State.                      */
                  CurrentPowerState = (Boolean_t)((DEVM_QueryDevicePowerState() > 0)?TRUE:FALSE);

                  /* Go ahead and register with the HID Manager Server. */
                  Result = _HOGM_Register_HID_Events();

                  if(Result > 0)
                  {
                     HOGEventsCallbackID = (unsigned int)Result;

                     /* Initialize a unique, starting HID Callback ID.  */
                     NextCallbackID      = 0x000000001;

                     /* Initialize a unique, starting Transaction ID.   */
                     NextTransactionID   = 0x000000001;

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
            if(HOGEventsCallbackID)
               _HOGM_Un_Register_HID_Events(HOGEventsCallbackID);

            if(HOGManagerMutex)
               BTPS_CloseMutex(HOGManagerMutex);

            MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_HOGP_MANAGER);

            /* Flag that none of the resources are allocated.           */
            HOGManagerMutex     = NULL;
            HOGEventsCallbackID = 0;
         }
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_VERBOSE), ("HID Manager Already initialized\n"));
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_VERBOSE), ("Shutdown Request\n"));

      if(Initialized)
      {
         /* First, let's make sure that we un-register our Message Group*/
         /* Handler (so that we do not process any more incoming        */
         /* messages).                                                  */
         MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_HOGP_MANAGER);

         /* Wait for access to the Mutex that guards access to this     */
         /* module.                                                     */
         if(BTPS_WaitMutex(HOGManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Un-Register for HID Events.                              */
            if(HOGEventsCallbackID)
               _HOGM_Un_Register_HID_Events(HOGEventsCallbackID);

            /* Next, Un-Register for any HID Data Events.               */
            HOGEntryInfo = HOGEntryInfoDataList;
            while(HOGEntryInfo)
            {
               _HOGM_Un_Register_HID_Data_Events(HOGEntryInfo->EventHandlerID);

               HOGEntryInfo = HOGEntryInfo->NextHOGEntryInfoPtr;
            }

            /* Make sure we inform the HID Manager Implementation that  */
            /* we are shutting down.                                    */
            _HOGM_Cleanup();

            /* Make sure that the HID Entry Information List is empty.  */
            FreeHOGEntryInfoList(&HOGEntryInfoList);

            /* Make sure that the HID Entry Data Information List is    */
            /* empty.                                                   */
            FreeHOGEntryInfoList(&HOGEntryInfoDataList);

            /* Free the Transaction Information List.                   */
            FreeTransactionInfoList(&TransactionInfoList);

            /* Close the HOG Manager Mutex.                             */
            BTPS_CloseMutex(HOGManagerMutex);

            /* Flag that the resources are no longer allocated.         */
            HOGManagerMutex     = NULL;
            CurrentPowerState   = FALSE;
            HOGEventsCallbackID = 0;

            /* Flag that this module is no longer initialized.          */
            Initialized         = FALSE;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing asynchronous */
   /* Device Manager (DEVM) Events (including Power On/Off events).     */
   /* This function should be registered with the Bluetopia Platform    */
   /* Manager Module Handler and will be called when an asynchronous    */
   /* Device Manager event is dispatched.                               */
void BTPSAPI HOGM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HID Manager has been initialized.   */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(BTPS_WaitMutex(HOGManagerMutex, BTPS_INFINITE_WAIT))
      {
         switch(EventData->EventType)
         {
            case detDevicePoweredOn:
               DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power On Occurred.\n"));

               /* Note the new Power State.                             */
               CurrentPowerState = TRUE;
               break;
            case detDevicePoweringOff:
            case detDevicePoweredOff:
               DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power Off Occurred.\n"));

               /* Note the new Power State.                             */
               CurrentPowerState   = FALSE;
               break;
            default:
               /* Do nothing.                                           */
               break;
         }

         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(HOGManagerMutex);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the Human Interface  */
   /* Device (HOG) Manager Service.  This Callback will be dispatched by*/
   /* the HOG Manager when various HOG Manager Events occur.  This      */
   /* function accepts the Callback Function and Callback Parameter     */
   /* (respectively) to call when a HOG Manager Event needs to be       */
   /* dispatched.  This function returns a positive (non-zero) value if */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * If this function returns success (greater than zero)     */
   /*          then this value can be passed to the                     */
   /*          HOGM_UnRegisterEventCallback() function to un-register   */
   /*          the callback from this module.                           */
int BTPSAPI HOGM_Register_Event_Callback(HOGM_Event_Callback_t CallbackFunction, void *CallbackParameter)
{
   int              ret_val;
   HOG_Entry_Info_t HOGEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HOG Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(CallbackFunction)
      {
         /* Attempt to wait for access to the HID Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(HOGManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Attempt to add an entry into the HID Entry list.         */
            BTPS_MemInitialize(&HOGEntryInfo, 0, sizeof(HOG_Entry_Info_t));

            HOGEntryInfo.CallbackID         = GetNextCallbackID();
            HOGEntryInfo.EventCallback      = CallbackFunction;
            HOGEntryInfo.CallbackParameter  = CallbackParameter;
            HOGEntryInfo.Flags              = HID_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY;

            if(AddHOGEntryInfoEntry(&HOGEntryInfoList, &HOGEntryInfo))
               ret_val = HOGEntryInfo.CallbackID;
            else
               ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HOGManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HID_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered HOG Manager Event Callback    */
   /* (registered via a successful call to the                          */
   /* HOGM_RegisterEventCallback() function).  This function accepts as */
   /* input the HOG Manager Event Callback ID (return value from        */
   /* HOGM_RegisterEventCallback() function).                           */
void BTPSAPI HOGM_Un_Register_Event_Callback(unsigned int HOGManagerCallbackID)
{
   HOG_Entry_Info_t *HOGEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HOG Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(HOGManagerCallbackID)
      {
         /* Attempt to wait for access to the HID Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(HOGManagerMutex, BTPS_INFINITE_WAIT))
         {
            if((HOGEntryInfo = DeleteHOGEntryInfoEntry(&HOGEntryInfoList, HOGManagerCallbackID)) != NULL)
            {
               /* Free the memory because we are finished with it.      */
               FreeHOGEntryInfoEntryMemory(HOGEntryInfo);
            }

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HOGManagerMutex);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the Human Interface  */
   /* Device (HOG) Manager Service to explicitly process HOG report     */
   /* data.  This Callback will be dispatched by the HOG Manager when   */
   /* various HOG Manager Events occur.  This function accepts the      */
   /* Callback Function and Callback Parameter (respectively) to call   */
   /* when a HOG Manager Event needs to be dispatched.  This function   */
   /* returns a positive (non-zero) value if successful, or a negative  */
   /* return error code if there was an error.                          */
   /* * NOTE * The return value from this function (if successful) is   */
   /*          the value that is must be passed to the                  */
   /*          HOGM_Send_Report_Data() function to send report data.    */
   /* * NOTE * There can only be a single Report Data event handler     */
   /*          registered.                                              */
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          HOGM_Un_Register_Data_Event_Callback() function to       */
   /*          un-register the callback from this module.               */
int BTPSAPI HOGM_Register_Data_Event_Callback(HOGM_Event_Callback_t CallbackFunction, void *CallbackParameter)
{
   int               ret_val;
   HOG_Entry_Info_t  HOGEntryInfo;
   HOG_Entry_Info_t *HOGEntryInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HOG Manager has been initialized.   */
   if(Initialized)
   {
      /* HOG Manager has been initialized, let's check the input        */
      /* parameters to see if they are semi-valid.                      */
      if(CallbackFunction)
      {
         /* Attempt to wait for access to the HID Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(HOGManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First, Register the handler locally.                     */
            /* * NOTE * We will use the ConnectionStatus member to hold */
            /*          the real Event Callback ID returned from the    */
            /*          server.                                         */
            BTPS_MemInitialize(&HOGEntryInfo, 0, sizeof(HOG_Entry_Info_t));

            HOGEntryInfo.CallbackID         = GetNextCallbackID();
            HOGEntryInfo.EventCallback      = CallbackFunction;
            HOGEntryInfo.CallbackParameter  = CallbackParameter;
            HOGEntryInfo.Flags              = HID_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY;

            if((HOGEntryInfoPtr = AddHOGEntryInfoEntry(&HOGEntryInfoDataList, &HOGEntryInfo)) != NULL)
            {
               /* Attempt to register it with the system.               */
               if((ret_val = _HOGM_Register_HID_Data_Events()) > 0)
               {
                  /* Data Handler registered, go ahead and flag success */
                  /* to the caller.                                     */
                  HOGEntryInfoPtr->EventHandlerID = ret_val;

                  ret_val                         = HOGEntryInfoPtr->CallbackID;
               }
               else
               {
                  /* Error, go ahead and delete the entry we added      */
                  /* locally.                                           */
                  if((HOGEntryInfoPtr = DeleteHOGEntryInfoEntry(&HOGEntryInfoDataList, HOGEntryInfoPtr->CallbackID)) != NULL)
                     FreeHOGEntryInfoEntryMemory(HOGEntryInfoPtr);
               }
            }
            else
               ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HOGManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
   }
   else
      ret_val = BTPM_ERROR_CODE_HID_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered HOG Manager Event Callback    */
   /* (registered via a successful call to the                          */
   /* HOGM_Register_Data_Event_Callback() function).  This function     */
   /* accepts as input the HOG Manager Data Event Callback ID (return   */
   /* value from HOGM_Register_Data_Event_Callback() function).         */
void BTPSAPI HOGM_Un_Register_Data_Event_Callback(unsigned int HOGManagerDataCallbackID)
{
   HOG_Entry_Info_t *HOGEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HOG Manager has been initialized.   */
   if(Initialized)
   {
      /* HID Manager has been initialized, let's check the input        */
      /* parameters to see if they are semi-valid.                      */
      if(HOGManagerDataCallbackID)
      {
         /* Attempt to wait for access to the HID Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(HOGManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First, delete the local handler.                         */
            if((HOGEntryInfo = DeleteHOGEntryInfoEntry(&HOGEntryInfoDataList, HOGManagerDataCallbackID)) != NULL)
            {
               /* Handler found, go ahead and delete it from the server.*/
               _HOGM_Un_Register_HID_Data_Events(HOGEntryInfo->EventHandlerID);

               /* All finished with the entry, delete it.               */
               FreeHOGEntryInfoEntryMemory(HOGEntryInfo);
            }

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HOGManagerMutex);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism of setting*/
   /* the HID Protocol Mode on a remote HID Device.  This function      */
   /* accepts as input the HOG Manager Data Event Callback ID (return   */
   /* value from HOGM_Register_Data_Event_Callback() function), the     */
   /* BD_ADDR of the remote HID Device and the Protocol Mode to set.    */
   /* This function returns zero on success or a negative error code.   */
   /* * NOTE * On each connection to a HID Device the Protocol Mode     */
   /*          defaults to Report Mode.                                 */
int BTPSAPI HOGM_Set_Protocol_Mode(unsigned int HOGManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HOGM_HID_Protocol_Mode_t ProtocolMode)
{
   int               ret_val;
   HOG_Entry_Info_t *HOGEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HOG Manager has been initialized.   */
   if(Initialized)
   {
      /* HOG Manager has been initialized, let's check the input        */
      /* parameters to see if they are semi-valid.                      */
      if(HOGManagerDataCallbackID)
      {
         /* Attempt to wait for access to the HOG Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(HOGManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First search for the specified Data Callback Information.*/
            if((HOGEntryInfo = SearchHOGEntryInfoEntry(&HOGEntryInfoDataList, HOGManagerDataCallbackID)) != NULL)
            {
               /* Simply call the Impl.  Manager to send the correct    */
               /* message to the server.                                */
               ret_val = _HOGM_Set_Protocol_Mode(HOGEntryInfo->EventHandlerID, RemoteDeviceAddress, ProtocolMode);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HOGManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HID_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is provided to allow a mechanism of        */
   /* informing the specified remote HID Device that the local HID Host */
   /* is entering/exiting the Suspend State.  This function accepts as  */
   /* input the HOG Manager Data Event Callback ID (return value from   */
   /* HOGM_Register_Data_Event_Callback() function), the BD_ADDR of the */
   /* remote HID Device and the a Boolean that indicates if the Host is */
   /* entering suspend state (TRUE) or exiting suspend state (FALSE).   */
   /* This function returns zero on success or a negative error code.   */
int BTPSAPI HOGM_Set_Suspend_Mode(unsigned int HOGManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, Boolean_t Suspend)
{
   int               ret_val;
   HOG_Entry_Info_t *HOGEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HOG Manager has been initialized.   */
   if(Initialized)
   {
      /* HOG Manager has been initialized, let's check the input        */
      /* parameters to see if they are semi-valid.                      */
      if(HOGManagerDataCallbackID)
      {
         /* Attempt to wait for access to the HID Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(HOGManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First search for the specified Data Callback Information.*/
            if((HOGEntryInfo = SearchHOGEntryInfoEntry(&HOGEntryInfoDataList, HOGManagerDataCallbackID)) != NULL)
            {
               /* Simply call the Impl.  Manager to send the correct    */
               /* message to the server.                                */
               ret_val = _HOGM_Set_Suspend_Mode(HOGEntryInfo->EventHandlerID, RemoteDeviceAddress, Suspend);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HOGManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HID_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is provided to allow a mechanism of        */
   /* performing a HID Get Report procedure to a remote HID Device.     */
   /* This function accepts as input the HOG Manager Data Event Callback*/
   /* ID (return value from HOGM_Register_Data_Event_Callback()         */
   /* function), the BD_ADDR of the remote HID Device and a pointer to a*/
   /* structure containing information on the Report to set.  This      */
   /* function returns the positive, non-zero, Transaction ID of the    */
   /* request on success or a negative error code.                      */
   /* * NOTE * The hetHOGMHIDGetReportResponse event will be generated  */
   /*          when the remote HID Device responds to the Get Report    */
   /*          Request.                                                 */
int BTPSAPI HOGM_Get_Report_Request(unsigned int HOGManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HOGM_HID_Report_Information_t *ReportInformation)
{
   int                 ret_val;
   HOG_Entry_Info_t   *HOGEntryInfo;
   Transaction_Info_t  TransactionInfo;
   Transaction_Info_t *TransactionInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HOG Manager has been initialized.   */
   if(Initialized)
   {
      /* HOG Manager has been initialized, let's check the input        */
      /* parameters to see if they are semi-valid.                      */
      if(HOGManagerDataCallbackID)
      {
         /* Attempt to wait for access to the HID Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(HOGManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First search for the specified Data Callback Information.*/
            if((HOGEntryInfo = SearchHOGEntryInfoEntry(&HOGEntryInfoDataList, HOGManagerDataCallbackID)) != NULL)
            {
               /* Initialize the Transaction Information entry.         */
               BTPS_MemInitialize(&TransactionInfo, 0, sizeof(Transaction_Info_t));

               TransactionInfo.TransactionID            = GetNextTransactionID();
               TransactionInfo.HOGManagerDataCallbackID = HOGEntryInfo->CallbackID;

               /* Add the Transaction Information to the Transaction    */
               /* Info List.                                            */
               if((TransactionInfoPtr = AddTransactionInfo(&TransactionInfoList, &TransactionInfo)) != NULL)
               {
                  /* Simply call the Impl.  Manager to send the correct */
                  /* message to the server.                             */
                  ret_val = _HOGM_Get_Report_Request(HOGEntryInfo->EventHandlerID, RemoteDeviceAddress, ReportInformation);
                  if(ret_val > 0)
                  {
                     /* Save the HOGM Transaction ID.                   */
                     TransactionInfoPtr->HOGMTransactionID = (unsigned int)ret_val;

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
            BTPS_ReleaseMutex(HOGManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HID_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is provided to allow a mechanism of        */
   /* performing a HID Set Report procedure to a remote HID Device.     */
   /* This function accepts as input the HOG Manager Data Event Callback*/
   /* ID (return value from HOGM_Register_Data_Event_Callback()         */
   /* function), the BD_ADDR of the remote HID Device, a pointer to a   */
   /* structure containing information on the Report to set, a Boolean  */
   /* that indicates if a response is expected, and the Report Data to  */
   /* set.  This function returns the positive, non-zero, Transaction ID*/
   /* of the request (if a Response is expected, ZERO if no response is */
   /* expected) on success or a negative error code.                    */
   /* * NOTE * If a response is expected the hetHOGMHIDSetReportResponse*/
   /*          event will be generated when the remote HID Device       */
   /*          responds to the Set Report Request.                      */
   /* * NOTE * The ResponseExpected parameter can be set to TRUE to     */
   /*          indicate that no response is expected.  This can only be */
   /*          set to TRUE if the Report to set is an Output Report and */
   /*          corresponds to a HID Data Output procedure.              */
   /* * NOTE * The ReportID, if valid for the specified Report, MUST be */
   /*          appended to the data that is passed to this function.    */
int BTPSAPI HOGM_Set_Report_Request(unsigned int HOGManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HOGM_HID_Report_Information_t *ReportInformation, Boolean_t ResponseExpected, unsigned int ReportDataLength, Byte_t *ReportData)
{
   int                 ret_val;
   HOG_Entry_Info_t   *HOGEntryInfo;
   Transaction_Info_t  TransactionInfo;
   Transaction_Info_t *TransactionInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HOG Manager has been initialized.   */
   if(Initialized)
   {
      /* HOG Manager has been initialized, let's check the input        */
      /* parameters to see if they are semi-valid.                      */
      if(HOGManagerDataCallbackID)
      {
         /* Attempt to wait for access to the HID Manager State         */
         /* information.                                                */
         if(BTPS_WaitMutex(HOGManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* First search for the specified Data Callback Information.*/
            if((HOGEntryInfo = SearchHOGEntryInfoEntry(&HOGEntryInfoDataList, HOGManagerDataCallbackID)) != NULL)
            {
               /* Initialize the Transaction Information entry.         */
               BTPS_MemInitialize(&TransactionInfo, 0, sizeof(Transaction_Info_t));

               TransactionInfo.TransactionID            = GetNextTransactionID();
               TransactionInfo.HOGManagerDataCallbackID = HOGEntryInfo->CallbackID;

               /* Add the Transaction Information to the Transaction    */
               /* Info List IFF a response is expected.                 */
               if(ResponseExpected)
                  TransactionInfoPtr = AddTransactionInfo(&TransactionInfoList, &TransactionInfo);
               else
                  TransactionInfoPtr = NULL;

               /* Continue only if the transaction info pointer was     */
               /* added to the list for Set Report Requests that expect */
               /* a response.                                           */
               if((!ResponseExpected) || (TransactionInfoPtr))
               {
                  /* Simply call the Impl.  Manager to send the correct */
                  /* message to the server.                             */
                  ret_val = _HOGM_Set_Report_Request(HOGEntryInfo->EventHandlerID, RemoteDeviceAddress, ReportInformation, ResponseExpected, ReportDataLength, ReportData);
                  if(ret_val >= 0)
                  {
                     /* If a response is expected we need to save and   */
                     /* return the appropriate Transaction IDs.         */
                     if(TransactionInfoPtr)
                     {
                        /* Save the HOGM Transaction ID if a response is*/
                        /* expected.                                    */
                        TransactionInfoPtr->HOGMTransactionID = (unsigned int)ret_val;

                        /* Return the Local Transaction ID to the       */
                        /* caller.                                      */
                        ret_val                               = (int)TransactionInfoPtr->TransactionID;
                     }
                     else
                     {
                        /* Return success to the caller.                */
                        ret_val                               = 0;
                     }
                  }
                  else
                  {
                     /* An error occurred so delete the transaction     */
                     /* information that was added to the list (if one  */
                     /* was added to the list).                         */
                     if(TransactionInfoPtr)
                     {
                        if((TransactionInfoPtr = DeleteTransactionInfo(&TransactionInfoList, TransactionInfoPtr->TransactionID)) != NULL)
                           FreeTransactionInfoMemory(TransactionInfoPtr);
                     }
                  }
               }
               else
                  ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HOGManagerMutex);
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HID_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

