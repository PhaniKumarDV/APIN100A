/*****< objstore.c >***********************************************************/
/*      Copyright 2006 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  OBJSTORE - Stonestreet One Bluetooth Stack OBEX SYNC Profile Object Store */
/*             Implementation for Stonestreet One Bluetooth Protocol Stack    */
/*             sample application.                                            */
/*                                                                            */
/*  Author:  Josh Toole                                                       */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/01/06  J. Toole       Initial creation.                               */
/******************************************************************************/
#include "ObjStore.h"      /* Object Store Implementation Protoypes/Constants.*/

#include "BTAPITyp.h"      /* Bluetooth API Type Constants.                   */
#include "BTPSKRNL.h"      /* Bluetooth O/S Abstration Prototypes/Constants.  */

   /* This define determines the size of the static buffer used in      */
   /* constructing a single line of output for the change logs, info    */
   /* logs, and device info.  The chosen value is based on the longest  */
   /* and most dynamic line possible, a change log entry that includes  */
   /* an LUID.                                                          */
#define OBJSTORE_LINE_BUFFER_LENGTH                                     (SYNC_MAX_LUID_CHARACTER_SIZE * 2)

   /* This define determines the number of change log entries stored in */
   /* the change log for each store.  The limit should be set in        */
   /* consideration of memory constraints.  Each store will have a      */
   /* change log in ram that can consume up to this value entries.      */
#define OBJSTORE_MAX_CHANGELOG_ENTRIES                                  (128)

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the         */
   /* compiler as part of standard C/C++).                              */

   /* This define determines the initial size buffer allocated when     */
   /* creating a stream object.  Because a stream object contains the   */
   /* contents of an entire object store its size is dynamic.  The      */
   /* buffer is automatically resized as required to accomodate the     */
   /* entire object.                                                    */
#define STREAM_OBJECT_INITIAL_BUFFER_SIZE    (512)

   /* Standard prefix to use for the database ID.  Each database ID will*/
   /* consist of this prefix followed by a randomly generated 32bit     */
   /* numerical value (presented as ASCII).                             */
static char DatabaseID_Prefix[] = "SS1D";

   /* This table contains the starting delimiters for objects of each   */
   /* fundamental object store type supported by this implementation.   */
   /* These delimiters are used in parsing incoming stream (level 2)    */
   /* operations.  The index into this table is the enumerated value of */
   /* each basic store type.                                            */
static char *ObjectStreamDelimiterStart[] =
{
   "BEGIN:VCARD",
   "BEGIN:VEVENT",
   "BEGIN:VMSG",
   "BEGIN:VMSG",
   "BEGIN:VMSG",
   "BEGIN:VNOTE"
};

   /* This table contains the ending delimiters for objects of each     */
   /* fundamental object store type supported by this implementation.   */
   /* These delimiters are used in parsing incoming stream (level 2)    */
   /* operations.  The index into this table is the enumerated value of */
   /* each basic store type.                                            */
static char *ObjectStreamDelimiterEnd[] =
{
   "END:VCARD",
   "END:VEVENT",
   "END:VMSG",
   "END:VMSG",
   "END:VMSG",
   "END:VNOTE"
} ;

   /* These strings represent the alternate Stream delimiters that can  */
   /* be present in a Calendar vCal format in addition to those in the  */
   /* table above.                                                      */
static char ObjectStreamCalStart[] = "BEGIN:VTODO";
static char ObjectStreamCalEnd[]   = "END:VTODO";

   /* Internal Function Prototypes.                                     */
static unsigned int GetIndex(IrMC_Object_Store_t *Store, Boolean_t Reset);
static unsigned int GetNextIndex(IrMC_Object_Store_t *Store);
static DWord_t GetChangeCounter(IrMC_Object_Store_t *Store, Boolean_t Reset);
static IrMC_Object_Entry_t *LookupObjectByIndex(IrMC_Object_Store_t *Store, DWord_t Index);
static IrMC_Object_Entry_t *LookupObjectByLUID(IrMC_Object_Store_t *Store, char *LUID);
static DWord_t GenerateDatabaseID(IrMC_Object_Store_t *Store);
static void InitChangeEntry(IrMC_Change_Log_Entry_t *ChangeEntry);
static void InitObjectEntry(IrMC_Object_Entry_t *ObjectEntry);
static void SetItemHashValues(IrMC_Object_Store_t *Store);
static void SetItemHashValueByIndex(IrMC_Object_Store_t *Store, unsigned int Index);
static DWord_t GetItemHashValueByIndex(IrMC_Object_Store_t *Store, unsigned int Index, int *Result);
static DWord_t BufferHash(char *Buffer, unsigned int BufferLength);
static int AddChangeEntry(IrMC_Change_Log_t *ChangeLog, IrMC_Change_Log_Entry_t *ChangeEntry, DWord_t *ChangeCounter);

static unsigned int GetIndex(IrMC_Object_Store_t *Store, Boolean_t Reset)
{
   unsigned int ret_val;

   /* Verify init and attempt to acquire mutex for this object store    */
   if((Store) && (Store->Mutex) && (BTPS_WaitMutex(Store->Mutex, BTPS_INFINITE_WAIT)))
   {
      /* Allow for reset of ID during store reinit.                     */
      if(Reset)
      {
         Store->Index = 0;
         Store->InfoLog.LastUsedIndex = 0;
      }

      /* Return the Entry ID Number to the caller.                      */
      ret_val = Store->Index;

      BTPS_ReleaseMutex(Store->Mutex);
   }
   else
      ret_val = 0;

   return(ret_val);
}

static unsigned int GetNextIndex(IrMC_Object_Store_t *Store)
{
   unsigned int ret_val;

   /* Verify init and attempt to acquire mutex for this object store    */
   if((Store) && (Store->Mutex) && (BTPS_WaitMutex(Store->Mutex, BTPS_INFINITE_WAIT)))
   {
      /* Return the Entry ID Number to the caller.                      */
      ret_val                      = Store->Index;

      /* Store last used index for information log output.              */
      Store->InfoLog.LastUsedIndex = ret_val;

      /* Increment the Counter to the next number.  We will allow the   */
      /* unsigned value to rollover freely, although store is limited to*/
      /* 65535 max objects.                                             */
      Store->Index++;

      /* Release acquired mutex.                                        */
      BTPS_ReleaseMutex(Store->Mutex);
   }
   else
      ret_val = 0;

   return(ret_val);
}

static DWord_t GetChangeCounter(IrMC_Object_Store_t *Store, Boolean_t Reset)
{
   DWord_t ret_val;

   /* Verify init and attempt to acquire mutex for this object store    */
   if((Store) && (Store->Mutex) && (BTPS_WaitMutex(Store->Mutex, BTPS_INFINITE_WAIT)))
   {
      /* Is this a reset operation?                                     */
      if(Reset)
      {
         /* If so, reset counter and return zero as return value.       */
         Store->ChangeCounter      = 0;

         /* Generate new database ID for this store (for CC rollover).  */
         Store->InfoLog.DatabaseID = GenerateDatabaseID(Store);
      }

      /* Return value                                                   */
      ret_val = Store->ChangeCounter;

      /* Release acquired mutex.                                        */
      BTPS_ReleaseMutex(Store->Mutex);
   }
   else
      ret_val = 0;

   /* Return the number to the caller.                                  */
   return(ret_val);
}

static IrMC_Object_Entry_t *LookupObjectByIndex(IrMC_Object_Store_t *Store, DWord_t Index)
{
   IrMC_Object_Entry_t *FoundEntry ;

   /* Attempt to lock this object store.                                */
   if((Store) &&& (Store->Mutex) && (BTPS_WaitMutex(Store->Mutex, BTPS_INFINITE_WAIT)))
   {
      if(Store->ObjectEntryList)
      {
         /* Now, let's search the list until we find the correct entry. */
         FoundEntry = (Store->ObjectEntryList);

         while((FoundEntry) && (FoundEntry->Index != Index))
            FoundEntry = FoundEntry->NextObjectEntryPtr;
      }
      else
         FoundEntry = NULL;

      /* Release acquired mutex                                         */
      BTPS_ReleaseMutex(Store->Mutex);
   }
   else
      FoundEntry = NULL;

   return(FoundEntry);
}

static IrMC_Object_Entry_t *LookupObjectByLUID(IrMC_Object_Store_t *Store, char *LUID)
{
   IrMC_Object_Entry_t *FoundEntry;

   /* Verify init and attempt to acquire mutex for this object store    */
   if((Store) && (Store->Mutex) && (BTPS_WaitMutex(Store->Mutex, BTPS_INFINITE_WAIT)))
   {
      /* Check parameter(s) for validity.                               */
      if((Store->ObjectEntryList) && (LUID))
      {
         /* Now, let's search the list until we find the correct entry. */
         FoundEntry = (Store->ObjectEntryList);

         while((FoundEntry) && ((BTPS_StringLength(LUID) != BTPS_StringLength(FoundEntry->LUID)) || (BTPS_MemCompare(LUID, FoundEntry->LUID, BTPS_StringLength(LUID)) != 0)))
            FoundEntry = FoundEntry->NextObjectEntryPtr;
      }
      else
         FoundEntry = NULL;

      BTPS_ReleaseMutex(Store->Mutex);
   }
   else
      FoundEntry = NULL;

   return(FoundEntry);
}

static DWord_t GenerateDatabaseID(IrMC_Object_Store_t *Store)
{
   static DWord_t DatabaseID = 0;

   return(DatabaseID++);
}

static void InitChangeEntry(IrMC_Change_Log_Entry_t *ChangeEntry)
{
   /* Zero entire entry object.                                         */
   BTPS_MemInitialize(ChangeEntry, 0, sizeof(IrMC_Change_Log_Entry_t));
}

static void InitObjectEntry(IrMC_Object_Entry_t *ObjectEntry)
{
   /* Zero entire entry object.                                         */
   BTPS_MemInitialize(ObjectEntry, 0, sizeof(IrMC_Object_Entry_t));
}

static DWord_t GetItemHashValueByIndex(IrMC_Object_Store_t *Store, unsigned int Index, int *Result)
{
   int                  Ret;
   DWord_t              ret_val = 0;
   IrMC_Object_Entry_t *Entry = NULL;

   /* This function handles getting the U32 hash value for requested    */
   /* index.                                                            */
   Ret = OBJSTORE_GetObjectByIndex(Store, Index, &Entry);

   if(Result != NULL)
   {
      *Result = Ret;
   }

   /* Did we successfully locate this object by index?                  */
   if((Ret == SYNC_OBEX_RESPONSE_OK) && (Entry != NULL))
   {
      /* Hash function will check parameters for us.  It returns zero on*/
      /* a bad input parameter, just like we would.                     */
      ret_val = BufferHash((char *)(Entry->Data), Entry->DataLength);

      /* Get Object returns COPY of the entry.  We must release this.   */
      OBJSTORE_ReleaseEntry(Entry);
   }
   else
   {
      /* Failed to locate entry.  We will return a ZERO to indicate     */
      /* failure.                                                       */
      if(Entry != NULL)
      {
         /* Handler should have released, but we will cleanup.          */
         OBJSTORE_ReleaseEntry(Entry);
      }
   }

   return(ret_val);
}

static DWord_t BufferHash(char *Buffer, unsigned int BufferLength)
{
   int     c;
   DWord_t hash = 5381;

   /* Check input paramters.                                            */
   if((Buffer != NULL) && (BufferLength > 0))
   {
      while(((c = *Buffer++) != 0) && (BufferLength--))
      {
         hash = ((hash << 5) + hash) + c; /* hash * 33 + c              */
      }
   }
   else
   {
      /* Return zero if initial parameters are invalid.                 */
      hash = 0;
   }

   return hash;
}

   /* This function adds a Change Log entry to the specified change log.*/
   /* The first parameter points to the Change Log to be updated.  The  */
   /* second parameter is a pointer to the Change Entry structure that  */
   /* contains the data to record in this Object Store Change Log.  The */
   /* third parameter is a pointer to a change counter.  NOTE: This     */
   /* function implements a 'reduced change log' as described in the    */
   /* IrMC Specification.  This means that duplicate change entries with*/
   /* the same LUID value are removed and updated to the most recent    */
   /* change.  The values included in the ChangeEntry structure will be */
   /* copied to a newly allocated structure, so this entry can be       */
   /* released after this call completes.                               */
static int AddChangeEntry(IrMC_Change_Log_t *ChangeLog, IrMC_Change_Log_Entry_t *ChangeEntry, DWord_t *ChangeCounter)
{
   int                      ret_val = OBJSTORE_ERROR_INVALID_PARAMETER;
   IrMC_Change_Log_Entry_t *AddedEntry;
   IrMC_Change_Log_Entry_t *tmpEntry;
   IrMC_Change_Log_Entry_t *prevEntry;

   /* First verify that the parameters are valid                        */
   if(ChangeEntry != NULL)
   {
      /* First, Check for duplicate LUID in this list so we can update  */
      /* entry instead of ADD.                                          */
      tmpEntry = ChangeLog->ChangeLogEntriesList;
      prevEntry = NULL;

      /* Head Pointer was not NULL, so we will traverse the list until  */
      /* we reach the last element.                                     */
      while(tmpEntry != NULL)
      {
         if((BTPS_StringLength(tmpEntry->LUID) == BTPS_StringLength(ChangeEntry->LUID)) && ((BTPS_MemCompare(tmpEntry->LUID, ChangeEntry->LUID, BTPS_StringLength(tmpEntry->LUID))) == 0))
         {
            /* This item is a duplicate.  We need to remove the existing*/
            /* entry so we can promote the new entry to the front of the*/
            /* list                                                     */
            if(prevEntry == NULL)
            {
               /* This was the first entry on the list.  Remove this    */
               /* entry and proceed w/ the add.                         */
               ChangeLog->ChangeLogEntriesList = (IrMC_Change_Log_Entry_t *)tmpEntry->NextChangeLogEntryPtr;
            }
            else
            {
               /* This item is in the middle (or end) of the list.      */
               /* Remove and proceed.                                   */
               prevEntry->NextChangeLogEntryPtr = tmpEntry->NextChangeLogEntryPtr;
            }

            /* Release memory of entry to remove.                       */
            BTPS_FreeMemory(tmpEntry);

            /* Decrement Entry Count                                    */
            ChangeLog->NumberChangeLogEntries--;

            /* Move to next item in list                                */
            tmpEntry = ((prevEntry != NULL)?((IrMC_Change_Log_Entry_t *)prevEntry->NextChangeLogEntryPtr):(ChangeLog->ChangeLogEntriesList));
         }
         else
         {
            /* Move to next item in list                                */
            prevEntry = tmpEntry;
            tmpEntry = (IrMC_Change_Log_Entry_t *)tmpEntry->NextChangeLogEntryPtr;
         }
      }

      /* Allocate an entry to add to the list                           */
      AddedEntry = (IrMC_Change_Log_Entry_t *)BTPS_AllocateMemory(sizeof(IrMC_Change_Log_Entry_t));
      if(AddedEntry != NULL)
      {
         /* Copy All Data over.                                         */
         (*AddedEntry) = (*ChangeEntry);

         /* Assign a new entry ID to this entry to be added.            */
         (*ChangeCounter)++;
         AddedEntry->ChangeCounter = *ChangeCounter;

         /* Determine if we need to remove the oldest change entry from */
         /* the list                                                    */
         if(ChangeLog->NumberChangeLogEntries >= OBJSTORE_MAX_CHANGELOG_ENTRIES)
         {
            /* We must remove the oldest entry (end of list)            */
            tmpEntry = ChangeLog->ChangeLogEntriesList;
            prevEntry = NULL;

            /* Move tmpEntry to point to the last item in the list.     */
            while((tmpEntry != NULL) && (tmpEntry->NextChangeLogEntryPtr != NULL))
            {
               prevEntry = tmpEntry;
               tmpEntry = (IrMC_Change_Log_Entry_t *)tmpEntry->NextChangeLogEntryPtr;
            }

            if(tmpEntry != NULL)
            {
               /* tmpEntry should point to the last entry in the list.  */
               /* prevEntry is the item that references this item. if   */
               /* prevEntry is NULL, then the listhead references this  */
               /* item.                                                 */
               if(prevEntry != NULL)
               {
                  /* Remove temp entry.  Next should be NULL.           */
                  prevEntry->NextChangeLogEntryPtr = tmpEntry->NextChangeLogEntryPtr;

                  /* Because we are removing the oldest entry we need to*/
                  /* update our tracking of the oldest CC in this log to*/
                  /* the new last entry remaining.                      */
                  ChangeLog->OldestChangeCounter = prevEntry->ChangeCounter;
               }
               else
               {
                  /* No previous entry means that the list head         */
                  /* references this entry.                             */
                  ChangeLog->ChangeLogEntriesList = (IrMC_Change_Log_Entry_t *)tmpEntry->NextChangeLogEntryPtr;

                  /* We are removing the first entry on the list.       */
                  /* Because tmpEntry->next should always be null this  */
                  /* will cause the list to be empty.  As a result we   */
                  /* will update the oldest CC value in the add code    */
                  /* below.                                             */
               }

               /* Release memory for removed list item                  */
               BTPS_FreeMemory(tmpEntry);

               /* Decrement count of current change log entries         */
               ChangeLog->NumberChangeLogEntries--;
            }
         }

         /* Is this the first entry to be added (even as result of      */
         /* delete, if so we need to track the Oldest CC value          */
         if(ChangeLog->ChangeLogEntriesList == NULL)
         {
            /* Store value as oldest CC.  This will be true until we    */
            /* remove an entry from the end of the list, at which time  */
            /* this value will be updated.                              */
            ChangeLog->OldestChangeCounter = AddedEntry->ChangeCounter;
         }

         /* Set new entries next pointer to head of list (can be NULL)  */
         AddedEntry->NextChangeLogEntryPtr = (struct _tagIrMC_Change_Log_Entry_t *)ChangeLog->ChangeLogEntriesList;

         /* And add entry to the beginning of the list.                 */
         ChangeLog->ChangeLogEntriesList = AddedEntry;

         /* Update change log metric values.                            */
         ChangeLog->NumberChangeLogEntries++;

         /* Return Zero to indicate success.                            */
         ret_val = OBJSTORE_SUCCESS_RESULT;
      }
      else
      {
         /* Error allocating memory for entry                           */
         ret_val = OBJSTORE_ERROR_INSUFFICIENT_RESOURCES;
      }
   }

   return(ret_val);
}

static void SetItemHashValues(IrMC_Object_Store_t *Store)
{
   unsigned int i;

   /* Verify init and attempt to acquire mutex for this object store    */
   if((Store) && (Store->Mutex) && (BTPS_WaitMutex(Store->Mutex, BTPS_INFINITE_WAIT)))
   {
      /* This function will interate through all objects in the store   */
      /* and set hash values for each object based on the contents.     */
      for(i=0;(i < (Store->Index));i++)
      {
         SetItemHashValueByIndex(Store, i);
      }

      /* Release acquired mutex                                         */
      BTPS_ReleaseMutex(Store->Mutex);
   }
}

static void SetItemHashValueByIndex(IrMC_Object_Store_t *Store, unsigned int Index)
{
   IrMC_Object_Entry_t *Entry;

   /* Verify init and attempt to acquire mutex for this object store    */
   if((Store) && (Store->Mutex) && (BTPS_WaitMutex(Store->Mutex, BTPS_INFINITE_WAIT)))
   {
      /* First try to get a (real) pointer to the object in the list.   */
      Entry = LookupObjectByIndex(Store, Index);

      /* Did we locate this index? If not go to next index. Gaps in     */
      /* the list by index are possible and expected.                   */
      if(Entry != NULL)
      {
         /* Attempt to get actual item data contents and write the      */
         /* hash value into the signature field for this object.        */
         /* If the lookup fails or IO read fails this will set sig to   */
         /* zero (this is ok).                                          */
         Entry->Signature = GetItemHashValueByIndex(Store, Index, NULL);
      }

      /* Release acquired mutex                                         */
      BTPS_ReleaseMutex(Store->Mutex);
   }
}

   /* This function creates a new Object Store.  The first parameter is */
   /* a pointer to the Object Store structure that will be used to keep */
   /* store information.  This structure is allocated by the caller, but*/
   /* is maintained and accessed through these API functions.  The next */
   /* parameter is a pointer to the Information Log struct to associate */
   /* with this store.  The information contained in this Info Log will */
   /* be copied into the Store storage space and used as defaults.  Many*/
   /* of these values will also change during Store operation.  The next*/
   /* parameter is a pointer to an ASCII, Null-terminated string        */
   /* containing the Serial Number of this device.  The Serial Number is*/
   /* a unique string defined in the Device Info log, but also must be  */
   /* tracked for each Object Store for inclusion in the Change logs.   */
   /* The next parameter is a const pointer to an IO Interface structure*/
   /* that contains pointers to handler functions that will implement   */
   /* the IO operations for this object store.  See ObjectStore.h for   */
   /* more information on IO Interfaces.  The last parameter is an      */
   /* ASCII, Null-termianted String that contains any optional,         */
   /* implementation dependent data the caller wishes to associate with */
   /* this store.  This data is passed to each IO interface function    */
   /* when called.                                                      */
int OBJSTORE_CreateStore(IrMC_Object_Store_t *Store, IrMC_Object_Store_Type_t Type, IrMC_Information_Log_t *InfoLog, DWord_t PermissionMask, char *SerialNumber, IrMC_Object_Store_IO_Interface_t *IOInterface, char *StoreData)
{
   int ret_val = OBJSTORE_SUCCESS_RESULT;
   int StoreDataSize;
   int Result;

   /* Check that the module has been initialized with the device        */
   /* information.                                                      */
   if(Store)
   {
      /* Check all the function handlers for validity.                  */
      if((IOInterface) && (IOInterface->Read) && (IOInterface->Write) && (IOInterface->Create) && (IOInterface->Delete) && (IOInterface->Populate))
      {
         /* Create MUTEX to protect access to this store.               */
         if(!Store->Mutex)
         {
            /* Attempt to create the mutex.                             */
            if((Store->Mutex = BTPS_CreateMutex(FALSE)) != NULL)
            {
               /* Attempt to acquire the newly created mutex.           */
               if(BTPS_WaitMutex(Store->Mutex, BTPS_INFINITE_WAIT))
               {
                  /* Begin initialization of the object store.          */
                  Store->ChangeCounter   = 0;
                  Store->Index           = 0;
                  Store->Type            = Type;
                  Store->ObjectEntryList = NULL;
                  Store->PermissionMask = PermissionMask;

                  /* Clear store data to all zeros.                     */
                  BTPS_MemInitialize(Store->StoreData, 0, OBJSTORE_STORE_DATA_SIZE);

                  /* If we were passed store data, copy it to the       */
                  /* buffer.                                            */
                  if((StoreData) && ((StoreDataSize = BTPS_StringLength(StoreData)) < OBJSTORE_STORE_DATA_SIZE))
                  {
                     /* Copy data to the internal store buffer.         */
                     BTPS_MemCopy(Store->StoreData, StoreData, StoreDataSize);
                  }

                  if(InfoLog)
                  {
                     /* Copy info log to the object store.              */
                     Store->InfoLog               = *InfoLog;

                     /* Zero counter values in info log.  These are     */
                     /* automatically updated as items are added to the */
                     /* object store.                                   */
                     Store->InfoLog.TotalRecords  = 0;
                     Store->InfoLog.LastUsedIndex = 0;
                  }
                  else
                  {
                     /* If we did not receive an Info Log for this store*/
                     /* just set everything to zero.  This will not     */
                     /* function properly for a store that requires a   */
                     /* true Info Log, but can be used for pseudo-stores*/
                     /* like the Inbox store that doesn't actually      */
                     /* require an Info Log.  The zero values will allow*/
                     /* all counters to continue to function as         */
                     /* expected.                                       */
                     BTPS_MemInitialize(&Store->InfoLog, 0, sizeof(IrMC_Information_Log_t));
                  }

                  /* Populate initial change log fields.                */
                  Store->ChangeLog.SerialNumber           = SerialNumber;
                  Store->ChangeLog.NumberChangeLogEntries = 0;
                  Store->ChangeLog.ChangeLogEntriesList   = NULL;

                  /* Store pointer to function handlers.                */
                  Store->Functions                        = *IOInterface;

                  /* Set Store Active Flag.                             */
                  Store->Active                           = TRUE;

                  /* Create database ID for this store (new on each     */
                  /* reset).                                            */
                  Store->InfoLog.DatabaseID               = GenerateDatabaseID(Store);

                  /* Finally we will call the populate hander for this  */
                  /* store.                                             */
                  __BTPSTRY
                  {
                     Result = (*Store->Functions.Populate)(Store, Store->StoreData);
                  }
                  __BTPSEXCEPT(1)
                  {
                     /* Do Nothing.                                     */
                  }

                  /* Did we have an error populating this store?        */
                  if(Result != OBJSTORE_SUCCESS_RESULT)
                  {
                     ret_val = OBJSTORE_ERROR_NOT_INITIALIZED;
                  }
                  else
                  {
                     /* The store was populate successfully. Now we     */
                     /* attempt to setup the hash values for this store.*/
                     SetItemHashValues(Store);
                  }

                  /* Release the previously acquired mutex.             */
                  BTPS_ReleaseMutex(Store->Mutex);
               }
               else
               {
                  /* Error Retreiving Mutex.                            */
                  ret_val = OBJSTORE_ERROR_NOT_INITIALIZED;
               }
            }
            else
            {
               /* Error Retreiving Mutex.                               */
               ret_val = OBJSTORE_ERROR_NOT_INITIALIZED;
            }
         }
         else
         {
            /* Error initializing mutex.  Return error code.            */
            ret_val = OBJSTORE_ERROR_NOT_INITIALIZED;
         }
      }
      else
      {
         /* IO Interface Pointer(s) Invalid.                            */
         ret_val = OBJSTORE_ERROR_INVALID_PARAMETER;
      }
   }
   else
   {
      /* Store Pointer is NULL                                          */
      ret_val = OBJSTORE_ERROR_INVALID_PARAMETER;
   }

   /* Return Error Code                                                 */
   return(ret_val);
}

   /* This function is used to shutdown and cleanup a previously opened */
   /* Object Store created using the OBJSTORE_CreateStore() function.   */
   /* The only parameter to this function is the pointer to the Object  */
   /* Store structure that contains information for this store.  This   */
   /* function will clear and release all storage used for the Change   */
   /* Log and Object List.  It will also release any related buffers.   */
int OBJSTORE_ReleaseStore(IrMC_Object_Store_t *Store)
{
   int                      ret_val;
   void                    *DeletedEntry;
   IrMC_Object_Entry_t     *ObjectEntry;
   IrMC_Change_Log_Entry_t *ChangeLogEntry;

   /* NOTE: It is assumed that storage used in the Info log and Device  */
   /* Info blocks are all statically allocated and will not need to be  */
   /* released.  If a dynamically allocated string or table is used in  */
   /* one of these structures it is up to the caller to release the     */
   /* relevant memory after calling this shutdown function.             */

   /* Get Mutex while we release everything in this store.              */
   if((Store) && (Store->Mutex) && (BTPS_WaitMutex(Store->Mutex, BTPS_INFINITE_WAIT)))
   {
      /* Free change log entries from the list                          */
      ChangeLogEntry = Store->ChangeLog.ChangeLogEntriesList;
      while(ChangeLogEntry)
      {
         DeletedEntry   = (void *)ChangeLogEntry;
         ChangeLogEntry = (IrMC_Change_Log_Entry_t *)ChangeLogEntry->NextChangeLogEntryPtr;

         BTPS_FreeMemory(DeletedEntry);
      }

      /* Mark list as NULL.                                             */
      Store->ChangeLog.ChangeLogEntriesList = NULL;

      /* Free object records from the Object Entry List                 */
      ObjectEntry = Store->ObjectEntryList;
      while(ObjectEntry)
      {
         /* Determine if we need to free the data portion.              */
         if(ObjectEntry->Data)
            BTPS_FreeMemory(ObjectEntry->Data);

         /* Delete main entry                                           */
         DeletedEntry = (void *)ObjectEntry;
         ObjectEntry  = (IrMC_Object_Entry_t *)ObjectEntry->NextObjectEntryPtr;

         BTPS_FreeMemory(DeletedEntry);
      }

      /* Mark list as NULL.                                             */
      Store->ObjectEntryList = NULL;

      /* Set Active to false so noone tries to use this store after     */
      /* mutex is destroyed.                                            */
      Store->Active = FALSE;

      /* Release Acquired Mutex.                                        */
      BTPS_ReleaseMutex(Store->Mutex);

      /* Close Mutex associated with this object store.                 */
      BTPS_CloseMutex(Store->Mutex);

      Store->Mutex = NULL;

      /* Release complete.  Return Success                              */
      ret_val = OBJSTORE_SUCCESS_RESULT;
   }
   else
   {
      /* Unable to acquire mutex for this store.                        */
      ret_val = OBJSTORE_ERROR_NOT_INITIALIZED;
   }

   return(ret_val);
}

   /* ***************************************************************** */
   /* OBJSTORE_AddObjectEntry(...)                                      */
   /* ***************************************************************** */
   /* Add an Object to a Store w/o Generating Change Entry              */
   /* ***************************************************************** */
   /* This function is generally used in populating an Object Store.  It*/
   /* adds an Object Entry to the specified object store, but it does   */
   /* not generate a change log entry for the Add (aka Put).  The first */
   /* parameter is a pointer to the Object Store structure to which the */
   /* user wishes to add an object entry.  The second parameter is a    */
   /* pointer to the object entry to add.  The data contained within    */
   /* this Entry is copied into a newly allocated structure, so this    */
   /* entry copy can be released after this call returns.               */
   /* ***************************************************************** */
int OBJSTORE_AddObjectEntry(IrMC_Object_Store_t *Store, IrMC_Object_Entry_t *Entry)
{
   int                  ret_val = OBJSTORE_ERROR_INVALID_PARAMETER;
   IrMC_Object_Entry_t *AddedEntry;
   IrMC_Object_Entry_t *tmpEntry;

   /* Check input parameters for validity                               */
   if(Entry != NULL)
   {
      /* Verify init and attempt to acquire mutex for this object store */
      if((Store) && (Store->Mutex) && (BTPS_WaitMutex(Store->Mutex, BTPS_INFINITE_WAIT)))
      {
         /* First we will check the list for duplicate LUID entries.    */
         tmpEntry = Store->ObjectEntryList;
         while(tmpEntry != NULL)
         {
            /* Check to see if this is a duplicate LUID of the object we*/
            /* are adding.  If this is a duplicate we will update some  */
            /* info on the existing object.                             */

            /* Is the entry we are adding a duplicate of this entry?    */
            if((BTPS_StringLength(tmpEntry->LUID) == BTPS_StringLength(Entry->LUID)) && ((BTPS_MemCompare(tmpEntry->LUID, Entry->LUID, BTPS_StringLength(tmpEntry->LUID))) == 0))
            {
               /* This item is a duplicate.  Because we do not need     */
               /* to add this item we will just update the IOhandler    */
               /* related data for this item.  Everything else should   */
               /* stay the same.                                        */
               tmpEntry->Handle = Entry->Handle;
               BTPS_MemCopy(&tmpEntry->HandlerData, &Entry->HandlerData, sizeof(Entry->HandlerData));

               /* We intentionally leave the signature intact. As a     */
               /* result, a change log entry will only be created if the*/
               /* actual object contents change.                        */

               /* The index is also kept intact to preserve this items  */
               /* logical location by index.                            */

               /* Stop scanning list.                                   */
               break;
            }
            else
            {
               /* Move to next item in list                             */
               tmpEntry = tmpEntry->NextObjectEntryPtr;
            }
         }

         /* If tmpEntry is NULL then we did not find this LUID on       */
         /* the existing list or the list is empty.  We must            */
         /* allocate an object and it to the list.                      */
         if(tmpEntry == NULL)
         {
            /* Allocate an entry to add to the list                     */
            AddedEntry = (IrMC_Object_Entry_t *)BTPS_AllocateMemory(sizeof(IrMC_Object_Entry_t));
            if(AddedEntry != NULL)
            {
               /* Initialize Entry                                      */
               InitObjectEntry(AddedEntry);

               /* Copy All Data over.                                   */
               (*AddedEntry) = (*Entry);

               /* Assign a new entry ID to this entry to be added.      */
               AddedEntry->Index = GetNextIndex(Store);

               /* Now Add it to the head of the list.                   */
               AddedEntry->NextObjectEntryPtr = Store->ObjectEntryList;
               Store->ObjectEntryList = AddedEntry;

               /* Entry has now been added.  Update object store metric */
               /* values.                                               */
               Store->InfoLog.TotalRecords++;

               /* Return Index to indicate success.                     */
               ret_val = AddedEntry->Index;
            }
            else
            {
               /* Buffer allocation failure.  Return an error.          */
               ret_val = OBJSTORE_ERROR_INSUFFICIENT_RESOURCES;
            }
         }
         else
         {
            /* We updated the entry at tmpEntry.  Lets return the index */
            /* of this entry to indicate success.                       */
            ret_val = tmpEntry->Index;
         }

         BTPS_ReleaseMutex(Store->Mutex);
      }
      else
      {
         /* Mutex not initialized or error acquiring mutex.  Return     */
         /* error.                                                      */
         ret_val = OBJSTORE_ERROR_NOT_INITIALIZED;
      }
   }

   return(ret_val);
}
   /* This function adds a Change Log entry to the specified Object     */
   /* Store's change log.  The first parameter is a pointer to the      */
   /* Object Store structure to which this change should be recorded.   */
   /* The second parameter is a pointer to the Change Entry structure   */
   /* that contains the data to record in this Object Store Change Log. */
   /* NOTE: This function implements a 'reduced change log' as described*/
   /* in the IrMC Specification.  This means that duplicate change      */
   /* entries with the same LUID value are removed and updated to the   */
   /* most recent change.  The values included in the ChangeEntry       */
   /* structure will be copied to a newly allocated structure, so this  */
   /* entry can be released after this call completes.                  */
int OBJSTORE_AddChangeEntry(IrMC_Object_Store_t *Store, IrMC_Change_Log_Entry_t *ChangeEntry)
{
   int     ret_val = OBJSTORE_ERROR_INVALID_PARAMETER;
   int     sts;
   DWord_t tmpChangeCounter;

   /* First verify that the parameters are valid                        */
   if(ChangeEntry != NULL)
   {
      /* Verify init and attempt to acquire mutex for this object store */
      if((Store) && (Store->Mutex) && (BTPS_WaitMutex(Store->Mutex, BTPS_INFINITE_WAIT)))
      {
         tmpChangeCounter = Store->ChangeCounter;

         sts = AddChangeEntry(&(Store->ChangeLog), ChangeEntry, &tmpChangeCounter);

         if(!sts)
         {
            if(tmpChangeCounter < Store->ChangeCounter)
            {
              /* Generate new database ID for this store (for CC        */
              /* rollover).                                             */
              Store->InfoLog.DatabaseID = GenerateDatabaseID(Store);
            }

            Store->ChangeCounter = tmpChangeCounter;

            /* Return Change Counter to indicate success.               */
            ret_val = tmpChangeCounter;
         }

         /* Release Acquired Mutex.                                     */
         BTPS_ReleaseMutex(Store->Mutex);
      }
      else
      {
         /* Mutex not initialized or error acquiring mutex.  Return     */
         /* error.                                                      */
         ret_val = OBJSTORE_ERROR_NOT_INITIALIZED;
      }
   }

   return(ret_val);
}


   /* This function clears the change log for the specified Object      */
   /* Store, deleting all log entries and reseting relevant counters.   */
   /* The only parameter to this function is a pointer to the Object    */
   /* Store structure that should have its Change Log cleared.  This    */
   /* function releases any previously allocated memory and resets the  */
   /* Change Counter.  The change log is ready for use again immediately*/
   /* after this call returns.                                          */
int OBJSTORE_ClearChangeLog(IrMC_Object_Store_t *Store)
{
   int                      ret_val;
   IrMC_Change_Log_Entry_t *ChangeLogEntry;
   IrMC_Change_Log_Entry_t *DeletedEntry;

   /* Get Mutex while we release everything in this store.              */
   if((Store) && (Store->Mutex) && (BTPS_WaitMutex(Store->Mutex, BTPS_INFINITE_WAIT)))
   {
      /* Free change log entries from the list                          */
      ChangeLogEntry = Store->ChangeLog.ChangeLogEntriesList;
      while(ChangeLogEntry)
      {
         DeletedEntry   = ChangeLogEntry;
         ChangeLogEntry = (IrMC_Change_Log_Entry_t *)ChangeLogEntry->NextChangeLogEntryPtr;

         BTPS_FreeMemory(DeletedEntry);
      }

      /* Mark list as NULL and reset state information.                 */
      Store->ChangeLog.ChangeLogEntriesList   = NULL;
      Store->ChangeLog.OldestChangeCounter    = 0;
      Store->ChangeLog.NumberChangeLogEntries = 0;

      /* Reset Change Counter to ZERO.                                  */
      GetChangeCounter(Store, TRUE);

      /* Release Acquired Mutex.                                        */
      BTPS_ReleaseMutex(Store->Mutex);

      /* Delete complete, return Success                                */
      ret_val = OBJSTORE_SUCCESS_RESULT;
   }
   else
   {
      /* Unable to acquire mutex for this store.                        */
      ret_val = OBJSTORE_ERROR_NOT_INITIALIZED;
   }

   return(ret_val);
}

   /* This function is used to properly cleanup and release memory for  */
   /* an Object Entry allocated by this module.  When receiving an      */
   /* Object Entry as the return value for a function call (e.g.        */
   /* OBJSTORE_ObjectGet()) the Object Entry is a newly allocated copy  */
   /* that is owned exclusively by the caller.  Because the memory used */
   /* for this copy was allocated by this module its necessary to use   */
   /* this function to release the copy when use is complete.  The only */
   /* parameter to this function is a pointer to the Object Entry to be */
   /* released.                                                         */
void OBJSTORE_ReleaseEntry(IrMC_Object_Entry_t *Entry)
{
   /* Check input pointer for validity.                                 */
   if(Entry)
   {
      /* First we must release the data portion if it exists.           */
      if(Entry->Data)
         BTPS_FreeMemory(Entry->Data);

      /* Next release the primary entry.                                */
      BTPS_FreeMemory(Entry);
   }
}

   /* This function is used to delete all objects that have been added  */
   /* to the specified Object Store.  This is generally used during the */
   /* process of a stream operation that modifies the entire object     */
   /* store.  This function will release any previously allocated memory*/
   /* and reset all relevant counters, returning the total objects count*/
   /* to zero.  The only parameter for this function is a pointer to the*/
   /* Object Store to which the delete should occur.                    */
int OBJSTORE_DeleteAllObjects(IrMC_Object_Store_t *Store)
{
   int                  Result;
   int                  ret_val = OBJSTORE_SUCCESS_RESULT;
   unsigned int         Index;
   IrMC_Object_Entry_t *ObjectEntry;

   /* Get Mutex while we release everything in this store.              */
   if((Store) && (Store->Mutex) && (BTPS_WaitMutex(Store->Mutex, BTPS_INFINITE_WAIT)))
   {
      /* Iterate through all possible index values.                     */
      for(Index=0;(Index<GetIndex(Store, FALSE));Index++)
      {
         ObjectEntry = LookupObjectByIndex(Store, Index);

         if(ObjectEntry)
         {
            Result = OBJSTORE_DeleteObjectByIndex(Store, Index, TRUE);

            if(Result != OBJSTORE_SUCCESS_RESULT)
               ret_val = Result;
         }
      }

      /* Release Acquired Mutex.                                        */
      BTPS_ReleaseMutex(Store->Mutex);
   }
   else
   {
      /* Unable to acquire mutex for this store.                        */
      ret_val = OBJSTORE_ERROR_NOT_INITIALIZED;
   }

   return(ret_val);
}

   /* This function is used to perform an IrMC Object Get operation on  */
   /* the specified Object Store.  The details of the operation to      */
   /* perform are included in the Op paramter.  The first parameter is a*/
   /* pointer to the Object Store on which to perform this GET          */
   /* operation.  The second parameter is a pointer to the Operation    */
   /* structure that contains the details for this operation.  This     */
   /* function implements different actions and responses for each of   */
   /* the defined IrMC Info Exchange Levels (1-4).  This should be the  */
   /* primary interface for performing GET operations on an Object      */
   /* Store.                                                            */
int OBJSTORE_GetObject(IrMC_Object_Store_t *Store, IrMC_Operation_t *Op, IrMC_Object_Entry_t **EntryPtr)
{
   int                  ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
   IrMC_Object_Entry_t *ret_ptr = NULL;

   /* Make sure operation pointer is valid.                             */
   if((Store) && (Op))
   {
      /* Examine operation to determine how we should lookup the        */
      /* requested object.                                              */
      switch(Op->Level)
      {
         case olLevel1:
            /* This implementation doesn't support inbox functionality. */
            ret_val = SYNC_OBEX_RESPONSE_UNAUTHORIZED;
            break;
         case olLevel2:
            /* To prevent sending back an empty set when read permission*/
            /* has been denied, we will check here instead of for each  */
            /* individial object get.                                   */
            if((Store != NULL) && (Store->PermissionMask & OBJSTORE_PERMISSION_MASK_READ))
            {
               /* This implementation only supports the base level 2    */
               /* access for each store (pb.vcf, cal.vcs, etc).  It does*/
               /* not support optional history stream objects such as   */
               /* 'ich.vcf' or 'mmh.vmg'.                               */
               ret_val = OBJSTORE_GetObjectStream(Store, Op->Name, &ret_ptr);
            }
            else
            {
               ret_val = SYNC_OBEX_RESPONSE_UNAUTHORIZED;
            }
            break;
         case olLevel3:
            /* Level 3 - Index Level                                    */

            /* Lookup object by the provided index value                */
            ret_val = OBJSTORE_GetObjectByIndex(Store, Op->Index, &ret_ptr);
            break;
         case olLevel4:
            /* Level 4 - Sync Level                                     */

            /* Lookup object by the provided LUID string.               */
            ret_val = OBJSTORE_GetObjectByLUID(Store, Op->Name, &ret_ptr);
            break;
         default:
            /* Unknown access level.  This should never happen.         */
            ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
            break;
      }
   }

   /* Setup object return pointer (or NULL if failed).                  */
   if(EntryPtr)
      *EntryPtr = ret_ptr;


   return(ret_val);
}

   /* This function is used to perform an IrMC Object Put operation on  */
   /* the specified Object Store.  The details of the operation to      */
   /* perform are included in the Op paramter.  The first parameter is a*/
   /* pointer to the Object Store on which to perform this PUT          */
   /* operation.  The second parameter is a pointer to the Operation    */
   /* structure that contains the details for this operation.  The      */
   /* Buffer parameter is a pointer a buffer containing the data to be  */
   /* written in this PUT operation.  The BufferSize parameter is the   */
   /* size of the data at the Buffer pointer.  The SyncAnchor parameter */
   /* is a pointer to a structure that will contain any Sync Anchor     */
   /* relevant return information from this call, typically the new CC  */
   /* value to include in the response packet.  This function implements*/
   /* different actions and responses for each of the defined IrMC Info */
   /* Exchange Levels (1-4).  This should be the primary interface for  */
   /* performing PUT operations on an Object Store.                     */
int OBJSTORE_PutObject(IrMC_Object_Store_t *Store, IrMC_Operation_t *Op, DWord_t BufferSize, Byte_t *Buffer, IrMC_Anchor_t *SyncAnchor)
{
   int ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;

   /* Make sure operation pointer is valid.                             */
   if((Store) && (Op))
   {
      /* Examine operation to determine how we should lookup the        */
      /* requested object.                                              */
      switch(Op->Level)
      {
         case olLevel1:
            /* Level 1 - Inbox Level                                    */

            /* Sync Anchor is not used for this level.  Ensure we do not*/
            /* return a sync acnhor value.                              */
            SyncAnchor->Anchor_Type = atNone;

            /* First we must assure that this operation was sent with   */
            /* the correct store type.                                  */
            if(Store->Type == osInbox)
            {
               /* Inbox access includes writing the object with the name*/
               /* as-is into the inbox object store.  We will use the   */
               /* LUID write function as it handles this properly.  This*/
               /* will also mean that a Change Log for the Inbox is     */
               /* created, although it will likely be ignored by any    */
               /* clients.                                              */
               ret_val = OBJSTORE_PutObjectByLUID(Store, Op->Name, BufferSize, Buffer, NULL, NULL);
            }
            else
            {
               /* If this is against a store other than Inbox, reject   */
               /* this request.                                         */
               ret_val = SYNC_OBEX_RESPONSE_UNAUTHORIZED;
            }
            break;
         case olLevel2:
            /* Level 2 - Access Level                                   */

            /* Sync Anchor is not used for this level.  Ensure we do not*/
            /* return a sync anchor value.                              */
            SyncAnchor->Anchor_Type = atNone;
            /* The defined behavior for a level 2 put operation is to   */
            /* delete all objects in the store and replace them with the*/
            /* new stream.  We will allow this only if the store        */
            /* provides delete permission.  Otherwise the objects in the*/
            /* stream will be added to the existing store.  This is     */
            /* consistent with other implmentations that don't allow    */
            /* deletion in general.                                     */
            if(Store->PermissionMask & OBJSTORE_PERMISSION_MASK_DELETE)
            {
               /* Delete all objects that are currently in the Object   */
               /* Store.  This will also send delete notifications to   */
               /* the IO handler for each object in the store.          */
               OBJSTORE_DeleteAllObjects(Store);

               /* Clear the change log and reset change counter to zero.*/
               OBJSTORE_ClearChangeLog(Store);

               /* Assign a new database ID to the object store.         */
               Store->InfoLog.DatabaseID = GenerateDatabaseID(Store);

               /* Reset the indexes to ensure correct numbering.        */
               GetIndex(Store, TRUE);
            }

            /* Add each entry from the stream to the store.  Assigning  */
            /* each object a new LUID.                                  */
            ret_val = OBJSTORE_PutObjectStream(Store, BufferSize, Buffer);
            break;
         case olLevel3:
            /* Level 3 - Index Level                                    */

            /* Sync Anchor is not used for this level.  Ensure we do not*/
            /* return a sync anchor value.                              */
            SyncAnchor->Anchor_Type = atNone;

            /* Put object by the provided index value                   */
            ret_val = OBJSTORE_PutObjectByIndex(Store, Op->Index, BufferSize, Buffer);
            break;
         case olLevel4:
            /* Level 4 - Sync Level                                     */

            /* Lookup object by the provided LUID string.  This function*/
            /* will set the appropriate fields in the provided          */
            /* SyncAnchor structure if successful.                      */
            ret_val = OBJSTORE_PutObjectByLUID(Store, Op->Name, BufferSize, Buffer, SyncAnchor, ((Op->UseMaxChangeCounter)?(&Op->MaxChangeCounter):(NULL)));
            break;
         default:
            /* Unknown access level.  This should never happen.         */
            ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
            break;
      }
   }

   return(ret_val);
}

   /* This function is used to perform an IrMC Object Delete operation  */
   /* on the specified Object Store.  The details of the operation to   */
   /* perform are included in the Op paramter.  The first parameter is a*/
   /* pointer to the Object Store on which to perform this Delete       */
   /* operation.  The second parameter is a pointer to the Operation    */
   /* structure that contains the details for this operation.  The      */
   /* SyncAnchor parameter is a pointer to a structure that will contain*/
   /* any Sync Anchor relevant return information from this call,       */
   /* typically the new CC value to include in the response packet.     */
   /* This function implements different actions and responses for each */
   /* of the defined IrMC Info Exchange Levels (1-4).  This should be   */
   /* the primary interface for performing Delete operations on an      */
   /* Object Store.                                                     */
int OBJSTORE_DeleteObject(IrMC_Object_Store_t *Store, IrMC_Operation_t *Op, IrMC_Anchor_t *SyncAnchor)
{
   int ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;

   /* This function assumes delete permission from the caller.  The     */
   /* IOhandler functions may also deny permission.                     */

   if(Store->PermissionMask & OBJSTORE_PERMISSION_MASK_DELETE)
   {
      /* Make sure operation pointer is valid.                          */
      if((Store) && (Op))
      {
         /* Examine operation to determine how we should lookup the     */
         /* requested object.                                           */
         switch(Op->Level)
         {
            case olLevel1:
               /* Level 1 - Inbox Level                                 */

               /* This implementation doesn't support inbox             */
               /* functionality.                                        */
               ret_val = SYNC_OBEX_RESPONSE_UNAUTHORIZED;
               break;
            case olLevel2:
               /* Level 2 - Access Level                                */

               /* Sync Anchor is not used for this level.  Ensure we do */
               /* not return a sync anchor value.                       */
               SyncAnchor->Anchor_Type = atNone;

               /* Delete all objects that are currently in the Object   */
               /* Store.  This will also send delete notifications to   */
               /* the IO handler.                                       */
               ret_val = OBJSTORE_DeleteAllObjects(Store);

               /* If any/all objects were deleted we need to clear the  */
               /* changelog and regenerate our databaseID per spec.     */
               if(ret_val == SYNC_OBEX_RESPONSE_OK)
               {
                  /* Clear the change log and reset change counter to   */
                  /* zero.                                              */
                  OBJSTORE_ClearChangeLog(Store);

                  /* Assign a new database ID to the object store.      */
                  Store->InfoLog.DatabaseID = GenerateDatabaseID(Store);
               }
               break;
            case olLevel3:
               /* Level 3 - Index Level                                 */

               /* Sync Anchor is not used for this level.  Ensure we do */
               /* not return a sync acnhor value.                       */
               SyncAnchor->Anchor_Type = atNone;

               /* Delete object by the provided index value Because we  */
               /* do not support the Hard/Soft distinction we will      */
               /* interpret all deletes as Hard deletes.                */
               ret_val = OBJSTORE_DeleteObjectByIndex(Store, Op->Index, TRUE);
               break;
            case olLevel4:
               /* Lookup object by the provided LUID string.  Because we*/
               /* do not support the Hard/Soft distinction we will      */
               /* interpret all deletes as Hard deletes.                */
               ret_val = OBJSTORE_DeleteObjectByLUID(Store, Op->Name, SyncAnchor, TRUE, ((Op->UseMaxChangeCounter)?(&Op->MaxChangeCounter):(NULL)));
               break;
            default:
               break;
         }
      }
   }
   else
   {
      /* Delete permission not granted.  Return unauthorized result.    */
      ret_val = SYNC_OBEX_RESPONSE_UNAUTHORIZED;
   }
   return(ret_val);
}

   /* This function is used to perform an IrMC Level 2 Get Operation.   */
   /* This is generally invoked by OBJSTORE_GetObject based on the IEL, */
   /* but can also be called directly.  This function generates a       */
   /* response Object Entry that contains a buffer with the contents of */
   /* all objects in this object.  The stream object's name is          */
   /* determined by the Name passed to this call.  The first parameter  */
   /* is a pointer to the Object Store structure on which to perform    */
   /* this operation.  The second parameter is an ASCII, Null-terminated*/
   /* string containing the Name of the stream object being requested.  */
   /* NOTE: Currently this function only supports stream objects that   */
   /* contain all objects in the store.  This doesn't currently support */
   /* optional history objects that are also part of a single store.    */
int OBJSTORE_GetObjectStream(IrMC_Object_Store_t *Store, char *Name, IrMC_Object_Entry_t **EntryPtr)
{
   int                  ret_val;
   void                *tmpPtr;
   DWord_t              Index;
   Boolean_t            OpFailed;
   IrMC_Object_Entry_t *retEntry = NULL;
   IrMC_Object_Entry_t *FoundEntry;

   /* Before we output the object stream we will scan for changes in    */
   /* the object store.                                                 */
   OBJSTORE_RegisterChangesByHash(Store);

   /* Verify init and attempt to acquire mutex for this object store    */
   if((Store) && (Store->Mutex) && (BTPS_WaitMutex(Store->Mutex, BTPS_INFINITE_WAIT)))
   {
      /* First we need to allocate a stream object to contain the       */
      /* results.                                                       */
      retEntry = (IrMC_Object_Entry_t *)BTPS_AllocateMemory(sizeof(IrMC_Object_Entry_t));
      if(retEntry)
      {
         InitObjectEntry(retEntry);

         /* Copy LUID into this object.  This should contain the name of*/
         /* the stream object (e.g.  pb.vcf).                           */
         BTPS_StringCopy(retEntry->LUID, Name);

         /* Next we need to setup a buffer for this stream object to    */
         /* contain the stream data.                                    */
         retEntry->Data = BTPS_AllocateMemory(STREAM_OBJECT_INITIAL_BUFFER_SIZE);
         if(retEntry->Data)
         {
            retEntry->DataLength   = 0;
            retEntry->BufferLength = STREAM_OBJECT_INITIAL_BUFFER_SIZE;

            /* Iterate through objects in the store, retreiving data for*/
            /* each object.                                             */
            for(Index=0,OpFailed=FALSE;((Index < Store->InfoLog.LastUsedIndex + 1) && (!OpFailed));Index++)
            {
               /* Attempt to lookup object for this index value.        */
               FoundEntry = NULL;

               OBJSTORE_GetObjectByIndex(Store, Index, &FoundEntry);

               /* Did we find object for this index?                    */
               if(FoundEntry)
               {
                  if(FoundEntry->Data)
                  {
                     /* Can we fit this data into the current buffer?   */
                     if((retEntry->BufferLength - retEntry->DataLength) > (FoundEntry->DataLength))
                     {
                        /* Data will fit, copy data to buffer.          */
                        BTPS_MemCopy((&retEntry->Data[retEntry->DataLength]), FoundEntry->Data, FoundEntry->DataLength);

                        /* Add new data to the length value.            */
                        retEntry->DataLength += FoundEntry->DataLength;
                     }
                     else
                     {
                        /* Will not fit, we must resize the buffer.     */
                        tmpPtr = BTPS_AllocateMemory(retEntry->BufferLength + FoundEntry->DataLength);
                        if(tmpPtr)
                        {
                           if(retEntry->Data)
                           {
                              BTPS_MemCopy(tmpPtr, retEntry->Data, retEntry->DataLength);

                              BTPS_FreeMemory(retEntry->Data);
                           }

                           /* Successfully resized memory, overwrite old*/
                           /* buffer pointer.                           */
                           retEntry->Data = tmpPtr;

                           /* Get new size.                             */
                           retEntry->BufferLength = (retEntry->BufferLength + FoundEntry->DataLength);

                           /* See if we can now fit data into the       */
                           /* buffer.                                   */
                           if((retEntry->BufferLength - retEntry->DataLength) > (FoundEntry->DataLength))
                           {
                              /* Now data will fit, copy data to buffer.*/
                              BTPS_MemCopy((&retEntry->Data[retEntry->DataLength]), FoundEntry->Data, FoundEntry->DataLength);

                              /* Add new data to the length value.      */
                              retEntry->DataLength += FoundEntry->DataLength;
                           }
                           else
                           {
                              /* Data would not fit after ReAlloc.  This*/
                              /* is a fatal error.                      */
                              OpFailed = TRUE;
                           }
                        }
                        else
                        {
                           /* Memory ReAlloc failed.                    */
                           OpFailed = TRUE;
                        }
                     }
                  }
                  else
                  {
                     /* No data portion to copy.  Skip this entry       */
                  }

                  /* Release entry copy before moving to the next       */
                  /* object.                                            */
                  OBJSTORE_ReleaseEntry(FoundEntry);
               }
               else
               {
                  /* Object not found with the specified index or data  */
                  /* buffer not allocated.  Skip this item.             */
               }
            }

            /* Unless an operation failed, We should now have a Stream  */
            /* Object with the data from all objects in this store.     */
         }
         else
         {
            /* Error allocating Stream Data Buffer.                     */
            OpFailed = TRUE;
         }

         /* Did we fail any part of this operation?                     */
         if(OpFailed)
         {
            /* If so, release return entry and its associated buffers.  */
            OBJSTORE_ReleaseEntry(retEntry);

            /* Return a NULL value to the caller and an error code.     */
            retEntry = NULL;

            ret_val  = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
         }
         else
         {
            /* Return successful result since we are passing an object  */
            /* even if it could be empty.  This is the correct behavior.*/
            ret_val = SYNC_OBEX_RESPONSE_OK;
         }
      }
      else
      {
         /* Error allocating Stream Object.  At this point nothing has  */
         /* been allocated so we can just return a NULL pointer from    */
         /* here.                                                       */
         ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
      }

      /* Release Acquired Mutex.                                        */
      BTPS_ReleaseMutex(Store->Mutex);
   }
   else
   {
      /* Error acquiring mutex or mutex not initialized.  At this point */
      /* nothing has been allocated so we can just return a NULL pointer*/
      /* from here and return error.                                    */
      ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
   }

   /* Setup object return pointer (or NULL if failed).                  */
   if(EntryPtr)
      *EntryPtr = retEntry;

   return(ret_val);
}

   /* This function is used to perform an IrMC Level 3 Get Operation.   */
   /* This is generally invoked by OBJSTORE_GetObject based on the IEL, */
   /* but can also be called directly.  This function generates a       */
   /* response Object Entry that contains a buffer with the contents of */
   /* the object at the specified index (if present).  The first        */
   /* parameter is a pointer to the Object Store structure on which to  */
   /* perform this operation.  The second parameter is the index within */
   /* this store of the object being requested.                         */
int OBJSTORE_GetObjectByIndex(IrMC_Object_Store_t *Store, DWord_t Index, IrMC_Object_Entry_t **EntryPtr)
{
   int                  ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
   IrMC_Object_Entry_t *FoundEntry;
   IrMC_Object_Entry_t *retEntry = NULL;

   /* Verify init and attempt to acquire mutex for this object store    */
   if((Store) && (Store->Mutex) && (BTPS_WaitMutex(Store->Mutex, BTPS_INFINITE_WAIT)))
   {
      /* Check store permissions.                                       */
      if(Store->PermissionMask & OBJSTORE_PERMISSION_MASK_READ)
      {
         /* Now, let's search the list until we find the correct entry. */
         FoundEntry = LookupObjectByIndex(Store, Index);

         /* Did we successfully locate a matching entry by Index?       */
         if(FoundEntry)
         {
            /* Next allocate a private copy of the entry.               */
            retEntry = (IrMC_Object_Entry_t *)BTPS_AllocateMemory(sizeof(IrMC_Object_Entry_t));
            if(retEntry)
            {
               /* Initialize Entry                                      */
               InitObjectEntry(retEntry);

               /* Allocation successful.  Copy entry to local copy.     */
               *retEntry                    = *FoundEntry;

               /* Clear old next pointer                                */
               retEntry->NextObjectEntryPtr = NULL;

               /* We got an entry - now try to populate the data portion*/
               /* via callback function.                                */
               __BTPSTRY
               {
                  ret_val = (*Store->Functions.Read)(Store->Type, retEntry, Store->StoreData);
               }
               __BTPSEXCEPT(1)
               {
                  /* Do Nothing.                                        */
               }

               /* Did we have an error reading this object?             */
               if(ret_val != SYNC_OBEX_RESPONSE_OK)
               {
                  /* An error occured calling the IO handler.  Because  */
                  /* we are only returning a pointer and not a response */
                  /* code we will just release the allocated memory and */
                  /* return a NULL pointer.                             */
                  OBJSTORE_ReleaseEntry(retEntry);
                  retEntry = NULL;
               }
            }
            else
            {
               /* Error allocating buffer for entry copy.               */
               ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
            }
         }
         else
         {
            /* Item not found at requested index.                       */
            ret_val = SYNC_OBEX_RESPONSE_NOT_FOUND;
         }
      }
      else
      {
         /* Read permission is not set for this object store.           */
         ret_val = SYNC_OBEX_RESPONSE_UNAUTHORIZED;
      }

      /* Release Acquired Mutex.                                        */
      BTPS_ReleaseMutex(Store->Mutex);
   }
   else
   {
      /* Error acquiring mutex or mutex not initialized.  Return NULL.  */
      ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
   }

   /* Setup return pointer.                                             */
   if(EntryPtr)
      *EntryPtr = retEntry;

   return(ret_val);
}

   /* This function is used to perform an IrMC Level 4 Get Operation.   */
   /* This is generally invoked by OBJSTORE_GetObject based on the IEL, */
   /* but can also be called directly.  This function generates a       */
   /* response Object Entry that contains a buffer with the contents of */
   /* the object that has the specified LUID (if present).  The first   */
   /* parameter is a pointer to the Object Store structure on which to  */
   /* perform this operation.  The second parameter is an ASCII,        */
   /* Null-terminated string containing the LUID of the Object being    */
   /* requested.                                                        */
int OBJSTORE_GetObjectByLUID(IrMC_Object_Store_t *Store, char *LUID, IrMC_Object_Entry_t **EntryPtr)
{
   int                  ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
   IrMC_Object_Entry_t *FoundEntry;
   IrMC_Object_Entry_t *retEntry = NULL;

   /* Verify init and attempt to acquire mutex for this object store    */
   if((Store) && (Store->Mutex) && (BTPS_WaitMutex(Store->Mutex, BTPS_INFINITE_WAIT)))
   {
      /* Check store permissions.                                       */
      if(Store->PermissionMask & OBJSTORE_PERMISSION_MASK_READ)
      {
         /* Now, let's search the list until we find the correct entry. */
         FoundEntry = LookupObjectByLUID(Store, LUID);

         /* Did we successfully locate a matching entry by Index ?      */
         if(FoundEntry)
         {
            /* Next allocate a private copy of the entry.               */
            retEntry = (IrMC_Object_Entry_t *)BTPS_AllocateMemory(sizeof(IrMC_Object_Entry_t));
            if(retEntry)
            {
               /* Initialize Entry                                      */
               InitObjectEntry(retEntry);

               /* Allocation successful.  Copy entry to local copy.     */
               *retEntry                    = *FoundEntry;

               /* Clear old next pointer                                */
               retEntry->NextObjectEntryPtr = NULL;

               /* We got an entry - now try to populate the data portion*/
               /* via callback function.                                */
               __BTPSTRY
               {
                  ret_val = (*Store->Functions.Read)(Store->Type, retEntry, Store->StoreData);
               }
               __BTPSEXCEPT(1)
               {
                  /* Do Nothing.                                        */
               }

               /* Did we have an error reading this object?             */
               if(ret_val != SYNC_OBEX_RESPONSE_OK)
               {
                  /* An error occured calling the IO handler.  Because  */
                  /* we are only returning a pointer and not a response */
                  /* code we will just release the allocated memory and */
                  /* return a NULL pointer.                             */
                  OBJSTORE_ReleaseEntry(retEntry);
                  retEntry = NULL;
               }
            }
            else
            {
               /* Error allocating buffer for entry copy.               */
               ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
            }
         }
         else
         {
            /* Item not found at requested index.                       */
            ret_val = SYNC_OBEX_RESPONSE_NOT_FOUND;
         }
      }
      else
      {
         /* Read permission is not set for this object store.           */
         ret_val = SYNC_OBEX_RESPONSE_UNAUTHORIZED;
      }

      /* Release Acquired Mutex.                                        */
      BTPS_ReleaseMutex(Store->Mutex);
   }
   else
   {
      /* Error acquiring mutex or mutex not initialized.  Return NULL.  */
      ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
   }

   /* Setup return pointer.                                             */
   if(EntryPtr)
      *EntryPtr = retEntry;

   return(ret_val);
}

   /* This function is used to perform an IrMC Level 2 Put Operation.   */
   /* This is generally invoked by OBJSTORE_PutObject based on the IEL, */
   /* but can also be called directly.  This function takes an incoming */
   /* buffer containing the data from a stream object and parses this   */
   /* data into multiple independent objects in the specified store.    */
   /* The first parameter is a pointer to the Object Store structure on */
   /* which to perform this operation.  The final two parameters are a  */
   /* pointer to the Buffer containing the Stream data, and a BufferSize*/
   /* that specifies the length of this data.                           */
int OBJSTORE_PutObjectStream(IrMC_Object_Store_t *Store, DWord_t BufferSize, Byte_t *Buffer)
{
   int        ret_val = SYNC_OBEX_RESPONSE_OK;
   int        BufferLength;
   int        TempBufferLength;
   int        DelimiterSize;
   Byte_t    *ReadPtr;
   Byte_t    *EntryStart;
   Byte_t    *EntryEnd;
   Byte_t    *EntrySearch;
   DWord_t    EntrySize;
   Boolean_t  FoundStart;
   Boolean_t  FoundEnd;
   Boolean_t  PartialPut = FALSE;

   /* Currently this implementation will allow duplicate entries to be  */
   /* created in the store.  There is no specified way to determine     */
   /* duplicates by vCard data, but some implementations do provide this*/
   /* functionality.                                                    */

   /* Verify that the buffer passed in is valid.                        */
   if((Store) && (Buffer))
   {
      /* Verify init and attempt to acquire mutex for this object store */
      if((Store->Mutex) && (Store->Mutex) && (BTPS_WaitMutex(Store->Mutex, BTPS_INFINITE_WAIT)))
      {
         /* Make sure this is one of the fundamental types supported for*/
         /* stream parsing.                                             */
         if(Store->Type <= osMsgSent)
         {
            /* We must parse the incoming buffer into individual object */
            /* entries.  This routine must be able to parse object      */
            /* formats for all the possible store types.                */

            /* Setup initial read pointer position to the start of the  */
            /* buffer.                                                  */
            ReadPtr      = Buffer;
            BufferLength = BufferSize;

            /* We will process the buffer until we have extracted all   */
            /* the object entries.                                      */
            while(BufferLength > 0)
            {
               /* First we will scan for the next beginning delimiter   */
               /* for this store.  Because the Calendar store actually  */
               /* uses two different object formats that can be embedded*/
               /* in a stream we must also check for two different      */
               /* delimiter strings for the Calendar store.             */
               FoundStart = FALSE;

               EntryStart = ReadPtr;

               while((*EntryStart != 'B') && ((EntryStart - ReadPtr) < BufferLength))
                  EntryStart++;

               /* Find the first instance of the 'B' character for      */
               /* "BEGIN" in the buffer.                                */
               if(*EntryStart == 'B')
               {
                  /* Attempt to compare this potential delimiter to the */
                  /* expected delimiter for this store type.            */
                  if(!BTPS_MemCompare(EntryStart, ObjectStreamDelimiterStart[Store->Type], sizeof(ObjectStreamDelimiterStart[Store->Type])))
                     FoundStart = TRUE;
                  else
                  {
                     /* Because vCalendar objects can have two different*/
                     /* forms of delimiter we must also do a second     */
                     /* comparison for that type.                       */
                     if(Store->Type == osCalendar)
                     {
                        if(!BTPS_MemCompare(EntryStart, ObjectStreamCalStart, sizeof(ObjectStreamCalStart)))
                           FoundStart = TRUE;
                     }
                  }

                  if(FoundStart)
                  {
                     /* We have successfully found a starting delimiter */
                     /* for this object type.  We will remove any bytes */
                     /* which occured before this start position and    */
                     /* proceed to locating the end delimiter.          */
                     EntrySearch = EntryStart;
                     FoundEnd    = FALSE;

                     /* Setup temporary length value that equals        */
                     /* remaining buffer past current EntryStart        */
                     /* location.  This will be dynamically decremented */
                     /* to ensure we don't overrun buffer while         */
                     /* searching for ending delimiter.                 */
                     TempBufferLength = BufferLength - (EntryStart - ReadPtr);

                     while((TempBufferLength > 0) && (FoundStart))
                     {
                        EntryEnd = EntrySearch;

                        while((*EntryEnd != 'E') && ((EntryEnd - EntrySearch) < TempBufferLength))
                           EntryEnd++;

                        if(*EntryEnd == 'E')
                        {
                           /* Attempt to compare this potential         */
                           /* delimiter to the expected delimiter for   */
                           /* this store type.                          */
                           if(!BTPS_MemCompare(EntryEnd, ObjectStreamDelimiterEnd[Store->Type], (DelimiterSize = BTPS_StringLength(ObjectStreamDelimiterEnd[Store->Type]))))
                              FoundEnd = TRUE;
                           else
                           {
                              /* Because vCalendar objects can have two */
                              /* different forms of delimiter we must   */
                              /* also do a second comparison for that   */
                              /* type.                                  */
                              if(Store->Type == osCalendar)
                              {
                                 if(!BTPS_MemCompare(EntryEnd, ObjectStreamCalEnd, (DelimiterSize = BTPS_StringLength(ObjectStreamCalEnd))))
                                    FoundEnd = TRUE;
                              }
                           }

                           if(FoundEnd)
                           {
                              /* We've successfully located an ending   */
                              /* delimiter.  Now move past the end      */
                              /* delimiter, Create a new object entry   */
                              /* with the appropriate portion of the    */
                              /* buffer, remove this portion, and       */
                              /* continue processing.                   */

                              TempBufferLength -= (EntryEnd - EntrySearch);

                              /* Make sure additional data remains in   */
                              /* the buffer, including an expected CRLF */
                              /* following the end delimiter.           */
                              if(TempBufferLength >= (DelimiterSize + 2))
                              {
                                 /* Move the End pointer to include the */
                                 /* ending delimiter and CRLF           */
                                 /* characters.                         */
                                 EntryEnd    += DelimiterSize;

                                 /* Force CRLF.                         */
                                 *EntryEnd++  = '\r';
                                 *EntryEnd++  = '\n';

                                 /* At this point EntryStart should     */
                                 /* point to the start of the buffer for*/
                                 /* this object.  EntryEnd should point */
                                 /* to the first character after the end*/
                                 /* of this buffer.                     */

                                 /* Use Entry pointers to calculate     */
                                 /* total buffer size for this object.  */
                                 EntrySize = (EntryEnd - EntryStart);

                                 /* Now create this object in the object*/
                                 /* store.                              */
                                 ret_val = OBJSTORE_PutObjectByLUID(Store, NULL, EntrySize, EntryStart, NULL, NULL);

                                 if(ret_val == SYNC_OBEX_RESPONSE_OK)
                                 {
                                    /* If any object is added return OK.*/
                                    PartialPut = TRUE;
                                 }

                                 /* We've completed this object, adjust */
                                 /* the overall buffer pointer and      */
                                 /* length, and exit the search for the */
                                 /* end delimiter.  This will restart   */
                                 /* the EntryStart search at the next   */
                                 /* free byte in the buffer             */
                                 BufferLength -= (EntryEnd - ReadPtr);
                                 ReadPtr       = EntryEnd;
                                 FoundStart    = FALSE;
                              }
                           }
                           else
                           {
                              /* We did not find the ending delimiter at*/
                              /* this location.  This can occur because */
                              /* an 'E' character can easily occur      */
                              /* within an object prior to the ending   */
                              /* delimiter.  Just remove this portion of*/
                              /* the search and reprocess.              */
                              TempBufferLength -= (EntryEnd - EntrySearch);

                              /* Make sure additional data remains in   */
                              /* the buffer.                            */
                              if(TempBufferLength > 0)
                              {
                                 /* Move the EntrySearch to one past the*/
                                 /* last 'E' location.                  */
                                 EntrySearch = EntryEnd + 1;

                                 /* Subtract one additional byte from   */
                                 /* the buffer to account for this      */
                                 /* character skip.                     */
                                 TempBufferLength--;
                              }
                           }
                        }
                        else
                        {
                           /* No end delimiter found in the remaining   */
                           /* buffer.  This means we will not be able to*/
                           /* process any remaining objects.  Force     */
                           /* termination of parsing.                   */
                           TempBufferLength = 0;
                           BufferLength     = 0;
                        }
                     }
                  }
                  else
                  {
                     /* No delimiter was located on this pass.  This can*/
                     /* occur if an extra 'B' character occurs prior to */
                     /* the next delimiter.  For this case, we will     */
                     /* discard all bytes to this point and reprocess   */
                     /* buffer.                                         */

                     /* Calculate how many bytes occured between the    */
                     /* last read position and our current 'B' location.*/
                     BufferLength -= (EntryStart - ReadPtr);

                     /* Make sure there is additional data past this    */
                     /* location.                                       */
                     if(BufferLength > 0)
                     {
                        /* Move the readPtr to one past the last 'B'    */
                        /* location.                                    */
                        ReadPtr = EntryStart + 1;

                        /* Subtract one additional byte from the buffer */
                        /* to account for this character skip.          */
                        BufferLength--;
                     }
                  }
               }
               else
               {
                  /* The remaining buffer doesn't contain any 'B'       */
                  /* characters.  This means no additional objects can  */
                  /* exist.  Force termination of parsing.              */
                  BufferLength = 0;
               }
            }

            /* If at least one object was put successfully we will      */
            /* return a success result, otherwise we will return the    */
            /* last error code.                                         */
            if(PartialPut)
            {

               ret_val = SYNC_OBEX_RESPONSE_OK;
            }
         }
         else
         {
            /* This store type is not supported for parsing stream      */
            /* objects.  Return an error indicating Action Not Allowed. */
            ret_val = SYNC_OBEX_RESPONSE_OBJECT_TYPE_NOT_SUPPORTED;
         }

         /* Release Acquired Mutex.                                     */
         BTPS_ReleaseMutex(Store->Mutex);
      }
      else
      {
         /* Error acquiring mutex or mutex not initialized.             */
         ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
      }
   }
   else
   {
      /* Invalid values for incoming buffer.                            */
      ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
   }

   return(ret_val);
}

   /* This function is used to perform an IrMC Level 3 Put Operation.   */
   /* This is generally invoked by OBJSTORE_PutObject based on the IEL, */
   /* but can also be called directly.  This function attempts to write */
   /* the contents of the incoming buffer to the specified object.  The */
   /* first parameter is a pointer to the Object Store structure on     */
   /* which to perform this operation.  The second parameter is the     */
   /* index within this store of the object being written.  The final   */
   /* two parameters are a pointer to the Buffer containing the data to */
   /* be written, and a BufferSize that specifies the length of this    */
   /* data.                                                             */
int OBJSTORE_PutObjectByIndex(IrMC_Object_Store_t *Store, DWord_t Index, DWord_t BufferSize, Byte_t *Buffer)
{
   int                      ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
   int                      newIdx;
   IrMC_Object_Entry_t     *FoundEntry;
   IrMC_Object_Entry_t     *AddedEntry;
   IrMC_Change_Log_Entry_t  changeEntry;

   /* Verify init and attempt to acquire mutex for this object store    */
   if((Store) && (Store->Mutex) && (BTPS_WaitMutex(Store->Mutex, BTPS_INFINITE_WAIT)))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = LookupObjectByIndex(Store, Index);

      /* Did we successfully locate a matching entry by Index?          */
      if(FoundEntry)
      {
         /* Check store permissions.                                    */
         if(Store->PermissionMask & OBJSTORE_PERMISSION_MASK_WRITE)
         {
            /* We got an entry - now try to write the data portion via  */
            /* callback function.                                       */
            __BTPSTRY
            {
               ret_val = (*Store->Functions.Write)(Store->Type, FoundEntry, BufferSize, Buffer, Store->StoreData);
            }
            __BTPSEXCEPT(1)
            {
               /* Do Nothing.                                           */
            }

            /* Was the PUT operation successful?                        */
            if(ret_val == SYNC_OBEX_RESPONSE_OK)
            {
               /* We must regenerate the hash for this item now.  We    */
               /* will re-read item because IO handler may change object*/
               /* contents.                                             */
               SetItemHashValueByIndex(Store, FoundEntry->Index);

               /* Even though this is a level 3 operation we will track */
               /* the change in the change log without returning sync   */
               /* anchor info.                                          */

               /* Initialize Change Entry Object                        */
               InitChangeEntry(&changeEntry);

               /* Actual CC/TS will be allocated by function.           */
               changeEntry.EntryType = cleModify;
               BTPS_StringCopy(changeEntry.LUID, FoundEntry->LUID);

               /* Now add a change log entry for this operation.        */
               OBJSTORE_AddChangeEntry(Store, &changeEntry);
            }
         }
         else
         {
            /* Write Permission Flag Not Set.                           */
            ret_val = SYNC_OBEX_RESPONSE_UNAUTHORIZED;
         }
      }
      else
      {
         /* Check store permissions.                                    */
         if(Store->PermissionMask & OBJSTORE_PERMISSION_MASK_CREATE)
         {
            /* Entry not found, attempt to create entry by Index        */
            /* Allocate memory for new object.                          */
            AddedEntry = (IrMC_Object_Entry_t *)BTPS_AllocateMemory(sizeof(IrMC_Object_Entry_t));
            if(AddedEntry)
            {
               /* Initialize Entry                                      */
               InitObjectEntry(AddedEntry);

               /* Setup index for this object.                          */
               AddedEntry->Index = Index;

               /* Call create handler function                          */
               __BTPSTRY
               {
                  ret_val = (*Store->Functions.Create)(Store->Type, AddedEntry, BufferSize, Buffer, Store->StoreData);
               }
               __BTPSEXCEPT(1)
               {
                  /* Return an error.                                   */
                  ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
               }

               /* Was the PUT operation successful?                     */
               if(ret_val == SYNC_OBEX_RESPONSE_OK)
               {
                  /* The Create handler should have set the LUID and    */
                  /* the handle/handler data.                           */
                  /* Add entry to object store.                         */
                  if((newIdx = OBJSTORE_AddObjectEntry(Store, AddedEntry)) >= OBJSTORE_SUCCESS_RESULT)
                  {
                     /* We must generate the hash for this item now.    */
                     /* We will re-read item because IO handler may     */
                     /* change object contents.                         */
                     SetItemHashValueByIndex(Store, newIdx);

                     /* Even though this is a level 3 operation we will */
                     /* track the change in the change log without      */
                     /* returning sync anchor info.                     */

                     /* Initialize Change Entry Object                  */
                     InitChangeEntry(&changeEntry);

                     /* Actual CC/TS will be allocated by function.     */
                     changeEntry.EntryType = cleModify;
                     BTPS_StringCopy(changeEntry.LUID, AddedEntry->LUID);

                     /* Now add a change log entry for this operation.  */
                     OBJSTORE_AddChangeEntry(Store, &changeEntry);
                  }
                  else
                  {
                     /* Failed to add this new entry to the Object      */
                     /* Store.  We will now release the object and      */
                     /* return an error.                                */
                     OBJSTORE_ReleaseEntry(AddedEntry);
                     ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
                  }
               }

               /* We can now release our local copy of this obj         */
               OBJSTORE_ReleaseEntry(AddedEntry);
            }
            else
            {
               /* Buffer allocation failed for new object entry.        */
               ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
            }
         }
         else
         {
            /* Create Permission Flag Not Set.                          */
            ret_val = SYNC_OBEX_RESPONSE_UNAUTHORIZED;
         }
      }

      /* Release Acquired Mutex.                                        */
      BTPS_ReleaseMutex(Store->Mutex);
   }
   else
   {
      /* Error acquiring mutex or mutex not initialized.                */
      ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
   }

   return(ret_val);
}

   /* This function is used to perform an IrMC Level 4 Put Operation.   */
   /* This is generally invoked by OBJSTORE_PutObject based on the IEL, */
   /* but can also be called directly.  This function attempts to write */
   /* the contents of the incoming buffer to the specified object.  The */
   /* first parameter is a pointer to the Object Store structure on     */
   /* which to perform this operation.  The second parameter is an      */
   /* ASCII, Null-terminated string containing the LUID of the Object   */
   /* being requested.  The next two parameters are a pointer to the    */
   /* Buffer containing the data to be written, and a BufferSize that   */
   /* specifies the length of this data.  The SnycAnchor parameter is a */
   /* pointer to a structure that is used to return the new Change      */
   /* Counter value after this operation.  The final parameter is the   */
   /* MaxChangeCounter value for this operation.  If this value is to be*/
   /* used, this pointer is valid and points to a variable containing   */
   /* the MaxChangeCounter.  If MaxChangeCounter should be ignored this */
   /* parameter should be set to NULL.                                  */
int OBJSTORE_PutObjectByLUID(IrMC_Object_Store_t *Store, char *LUID, DWord_t BufferSize, Byte_t *Buffer, IrMC_Anchor_t *SyncAnchor, DWord_t *MaxChangeCounter)
{
   int                      ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
   int                      newIdx;
   DWord_t                  ChangeCounter;
   IrMC_Object_Entry_t     *FoundEntry;
   IrMC_Object_Entry_t     *AddedEntry;
   IrMC_Change_Log_Entry_t  ChangeLogEntry;

   /* Verify init and attempt to acquire mutex for this object store    */
   if((Store) && (Store->Mutex) && (BTPS_WaitMutex(Store->Mutex, BTPS_INFINITE_WAIT)))
   {
      /* First we must determine if this operation is allowed based on  */
      /* the MaxChangeCounter value.                                    */
      if((!MaxChangeCounter) || ((MaxChangeCounter) && (*MaxChangeCounter >= GetChangeCounter(Store, FALSE))))
      {
         /* Now, let's search the list until we find the correct entry. */
         FoundEntry = LookupObjectByLUID(Store, LUID);

         /* Did we successfully locate a matching entry?                */
         if(FoundEntry)
         {
            /* Check store permissions.                                 */
            if(Store->PermissionMask & OBJSTORE_PERMISSION_MASK_WRITE)
            {
               /* We got an entry - now try to write the data portion   */
               /* via callback function.                                */
               __BTPSTRY
               {
                  ret_val = (*Store->Functions.Write)(Store->Type, FoundEntry, BufferSize, Buffer, Store->StoreData);
               }
               __BTPSEXCEPT(1)
               {
                  /* Do Nothing.                                        */
               }

               /* Was the PUT operation successful?                     */
               if(ret_val == SYNC_OBEX_RESPONSE_OK)
               {
                  /* We must generate the hash for this item now.  We   */
                  /* will re-read item because IO handler may change    */
                  /* object contents.                                   */
                  SetItemHashValueByIndex(Store, FoundEntry->Index);

                  /* Initialize Change Entry Object                     */
                  InitChangeEntry(&ChangeLogEntry);

                  /* Actual CC/TS will be allocated by function.        */
                  ChangeLogEntry.EntryType = cleModify;
                  BTPS_StringCopy(ChangeLogEntry.LUID, LUID);

                  /* Now add a change log entry for this operation.     */
                  OBJSTORE_AddChangeEntry(Store, &ChangeLogEntry);

                  ChangeCounter = ChangeLogEntry.ChangeCounter;

                  /* If the SyncAnchor pointer is present populate it   */
                  /* with the new CC value.                             */
                  if(SyncAnchor)
                  {
                     SyncAnchor->Anchor_Type               = atChangeCounter;
                     SyncAnchor->Anchor_Data.ChangeCounter = ChangeCounter;
                  }
               }
            }
            else
            {
               /* Write permission flag not set.                        */
               ret_val = SYNC_OBEX_RESPONSE_UNAUTHORIZED;
            }
         }
         else
         {
            /* Check store permissions.                                 */
            if(Store->PermissionMask & OBJSTORE_PERMISSION_MASK_CREATE)
            {
               /* Entry not found, attempt to create entry by LUID      */
               /* Allocate memory for new object.                       */
               AddedEntry = (IrMC_Object_Entry_t *)BTPS_AllocateMemory(sizeof(IrMC_Object_Entry_t));
               if(AddedEntry)
               {
                  /* Initialize Entry                                   */
                  InitObjectEntry(AddedEntry);

                  /* Call create handler function                       */
                  __BTPSTRY
                  {
                     ret_val = (*Store->Functions.Create)(Store->Type, AddedEntry, BufferSize, Buffer, Store->StoreData);
                  }
                  __BTPSEXCEPT(1)
                  {
                     /* Do Nothing.                                     */
                  }

                  /* Was the PUT operation successful?                  */
                  if(ret_val == SYNC_OBEX_RESPONSE_OK)
                  {
                     /* The Create handler should have set the LUID and */
                     /* the handle/handler data.  Add entry to object   */
                     /* store                                           */
                     if((newIdx = OBJSTORE_AddObjectEntry(Store, AddedEntry)) >= OBJSTORE_SUCCESS_RESULT)
                     {
                        /* We must generate the hash for this item now. */
                        /* We will re-read item because IO handler may  */
                        /* change object contents.                      */
                        SetItemHashValueByIndex(Store, newIdx);

                        /* Initialize Change Entry Object               */
                        InitChangeEntry(&ChangeLogEntry);

                        /* Actual CC/TS will be allocated by function.  */
                        ChangeLogEntry.EntryType = cleModify;
                        BTPS_StringCopy(ChangeLogEntry.LUID, AddedEntry->LUID);
                        /* If LUID pointer is valid, set name in Op     */
                        /* structure so that we will return an LUID     */
                        /* in the response.                             */
                        if(LUID != NULL)
                        {
                           BTPS_StringCopy(LUID, AddedEntry->LUID);
                        }

                        /* Now add a change log entry for this          */
                        /* operation.                                   */
                        OBJSTORE_AddChangeEntry(Store, &ChangeLogEntry);

                        ChangeCounter = ChangeLogEntry.ChangeCounter;

                        /* If the SyncAnchor pointer is present populate*/
                        /* it with the new CC value.                    */
                        if(SyncAnchor)
                        {
                           SyncAnchor->Anchor_Type               = atChangeCounter;
                           SyncAnchor->Anchor_Data.ChangeCounter = ChangeCounter;
                        }
                     }
                     else
                     {
                        /* Failed to add this new entry to the object   */
                        /* store.  We will now release the memory       */
                        /* allocated for this object.                   */
                        OBJSTORE_ReleaseEntry(AddedEntry);
                        ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
                     }
                  }

                  /* We can now release our local copy of this obj      */
                  OBJSTORE_ReleaseEntry(AddedEntry);
               }
               else
               {
                  /* Failed to allocate buffer.                         */
                  ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
               }
            }
            else
            {
               /* Create permission flag not set.                       */
               ret_val = SYNC_OBEX_RESPONSE_UNAUTHORIZED;
            }
         }
      }
      else
      {
         /* MaxChangeCounter is less than current change counter for    */
         /* this object store.  We will reject this operation as a      */
         /* conflict.                                                   */
         ret_val = SYNC_OBEX_RESPONSE_CONFLICT;
      }

      /* Release Acquired Mutex.                                        */
      BTPS_ReleaseMutex(Store->Mutex);
   }
   else
   {
      /* Error acquiring mutex or mutex not initialized.                */
      ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
   }

   return(ret_val);
}

   /* This function is used to perform an IrMC Level 3 Delete Operation.*/
   /* This is generally invoked by OBJSTORE_DeleteObject based on the   */
   /* IEL, but can also be called directly.  This function attempts to  */
   /* delete the specified object from the specified object store.  The */
   /* first parameter is a pointer to the Object Store structure on     */
   /* which to perform this operation.  The second parameter is the     */
   /* index within this store of the object being deleted.  The final   */
   /* parameter is a Boolean_t flag this indicates if this is a 'Hard   */
   /* Delete' operation.                                                */
int OBJSTORE_DeleteObjectByIndex(IrMC_Object_Store_t *Store, DWord_t Index, Boolean_t HardDelete)
{
   int                      ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
   IrMC_Object_Entry_t     *FoundEntry;
   IrMC_Object_Entry_t     *prevEntry;
   IrMC_Change_Log_Entry_t  changeEntry;

   /* Verify init and attempt to acquire mutex for this object store    */
   if((Store) && (Store->Mutex) && (BTPS_WaitMutex(Store->Mutex, BTPS_INFINITE_WAIT)))
   {
      /* Check store permissions.                                       */
      if(Store->PermissionMask & OBJSTORE_PERMISSION_MASK_DELETE)
      {
         /* Check parameter(s) for validity.                            */
         if(Store->ObjectEntryList)
         {
            /* Now, let's search the list until we find the correct     */
            /* entry.                                                   */
            FoundEntry = Store->ObjectEntryList;
            prevEntry  = NULL;

            while((FoundEntry) && (FoundEntry->Index != Index))
            {
               prevEntry  = FoundEntry;
               FoundEntry = FoundEntry->NextObjectEntryPtr;
            }

            /* Did we successfully locate a matching entry by Index?    */
            if(FoundEntry)
            {
               /* First send the Delete notification                    */
               __BTPSTRY
               {
                  ret_val = (*Store->Functions.Delete)(Store->Type, FoundEntry, HardDelete, Store->StoreData);
               }
               __BTPSEXCEPT(1)
               {
                  /* Do Nothing.                                        */
               }

               /* Did we have an error deleting this object?  Could have*/
               /* been rejected/unauthorized by IO handler.  We will    */
               /* also accept NOT FOUND as a reason to remove from our  */
               /* object store.                                         */
               if((ret_val == SYNC_OBEX_RESPONSE_OK) || (ret_val == SYNC_OBEX_RESPONSE_NOT_FOUND))
               {
                  /* Now proceed to remove this item from the list      */

                  /* If prevEntry is NULL this item was the first on the*/
                  /* list.                                              */
                  if(!prevEntry)
                  {
                     /* Place next entry onto the list head.            */
                     Store->ObjectEntryList = FoundEntry->NextObjectEntryPtr;
                  }
                  else
                  {
                     /* remove entry and adjust list pointers.          */
                     prevEntry->NextObjectEntryPtr = FoundEntry->NextObjectEntryPtr;
                  }

                  /* Entry has now been Removed.  Update object store   */
                  /* metric values.                                     */
                  Store->InfoLog.TotalRecords--;

                  /* Initialize Change Entry Object                     */
                  InitChangeEntry(&changeEntry);

                  /* Update Change Log for this operation.  Actual CC/TS*/
                  /* will be allocated by function.                     */
                  changeEntry.EntryType = ((HardDelete)?(cleHardDelete):(cleDelete));
                  BTPS_StringCopy(changeEntry.LUID, FoundEntry->LUID);

                  /* Now add a change log entry for this operation.     */
                  OBJSTORE_AddChangeEntry(Store, &changeEntry);

                  /* Release this object and associated data buffers.   */
                  OBJSTORE_ReleaseEntry(FoundEntry);
               }
            }
            else
            {
               /* No object at that index to delete.                    */
               ret_val = SYNC_OBEX_RESPONSE_NOT_FOUND;
            }
         }
         else
         {
            /* Invalid object entry list.                               */
            ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
         }
      }
      else
      {
         /* Delete permission flag not set.                             */
         ret_val = SYNC_OBEX_RESPONSE_UNAUTHORIZED;
      }
      /* Release Acquired Mutex.                                        */
      BTPS_ReleaseMutex(Store->Mutex);
   }
   else
   {
      /* Error acquiring mutex or mutex not initialized.                */
      ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
   }

   return(ret_val);
}

   /* This function is used to perform an IrMC Level 4 Delete Operation.*/
   /* This is generally invoked by OBJSTORE_DeleteObject based on the   */
   /* IEL, but can also be called directly.  This function attempts to  */
   /* delete the specified object from the specified object store.  The */
   /* first parameter is a pointer to the Object Store structure on     */
   /* which to perform this operation.  The second parameter is an      */
   /* ASCII, Null-terminated string containing the LUID of the Object   */
   /* being deleted.  The SnycAnchor parameter is a pointer to a        */
   /* structure that is used to return the new Change Counter value     */
   /* after this operation.  The final parameter is the MaxChangeCounter*/
   /* value for this operation.  If this value is to be used, this      */
   /* pointer is valid and points to a variable containing the          */
   /* MaxChangeCounter.  If MaxChangeCounter should be ignored this     */
   /* parameter should be set to NULL.                                  */
int OBJSTORE_DeleteObjectByLUID(IrMC_Object_Store_t *Store, char *LUID, IrMC_Anchor_t *SyncAnchor, Boolean_t HardDelete, DWord_t *MaxChangeCounter)
{
   int                      ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
   DWord_t                  ChangeCounter;
   IrMC_Object_Entry_t     *FoundEntry;
   IrMC_Object_Entry_t     *prevEntry;
   IrMC_Change_Log_Entry_t  ChangeLogEntry;

   /* Verify init and attempt to acquire mutex for this object store    */
   if((Store) && (Store->Mutex) && (BTPS_WaitMutex(Store->Mutex, BTPS_INFINITE_WAIT)))
   {
      /* Check store permissions.                                       */
      if(Store->PermissionMask & OBJSTORE_PERMISSION_MASK_DELETE)
      {
         /* Check parameter(s) for validity.                            */
         if((Store->ObjectEntryList) && (LUID))
         {
            /* First we must determine if this operation is allowed     */
            /* based on the MaxChangeCounter value.                     */
            if((!MaxChangeCounter) || ((MaxChangeCounter) && (*MaxChangeCounter >= GetChangeCounter(Store, FALSE))))
            {
               /* Now, let's search the list until we find the correct  */
               /* entry.                                                */
               FoundEntry = Store->ObjectEntryList;
               prevEntry  = NULL;

               while((FoundEntry) && (BTPS_StringLength(LUID) == BTPS_StringLength(FoundEntry->LUID)) && (BTPS_MemCompare(LUID, FoundEntry->LUID, BTPS_StringLength(LUID))))
               {
                  prevEntry  = FoundEntry;
                  FoundEntry = FoundEntry->NextObjectEntryPtr;
               }

               /* Did we successfully locate a matching entry by Index? */
               if(FoundEntry)
               {
                  /* First send the Delete notification                 */
                  __BTPSTRY
                  {
                     ret_val = (*Store->Functions.Delete)(Store->Type, FoundEntry, HardDelete, Store->StoreData);
                  }
                  __BTPSEXCEPT(1)
                  {
                     /* Do Nothing.                                     */
                  }

                  /* Did we have an error deleting this object?  Could  */
                  /* have been rejected/unauthorized by IO handler.  We */
                  /* will also accept NOT FOUND as a reason to remove   */
                  /* from our object store.                             */
                  if((ret_val == SYNC_OBEX_RESPONSE_OK) || (ret_val == SYNC_OBEX_RESPONSE_NOT_FOUND))
                  {
                     /* Now proceed to remove this item from the list   */

                     /* If prevEntry is NULL this item was the first on */
                     /* the list.                                       */
                     if(!prevEntry)
                     {
                        /* Place next entry onto the list head.         */
                        Store->ObjectEntryList = FoundEntry->NextObjectEntryPtr;
                     }
                     else
                     {
                        /* remove entry and adjust list pointers.       */
                        prevEntry->NextObjectEntryPtr = FoundEntry->NextObjectEntryPtr;
                     }

                     /* Entry has now been Removed.  Update object store*/
                     /* metric values.                                  */
                     Store->InfoLog.TotalRecords--;

                     /* Initialize Change Entry Object                  */
                     InitChangeEntry(&ChangeLogEntry);

                     /* Update Change Log for this operation.  Actual   */
                     /* CC/TS will be allocated by function.            */
                     ChangeLogEntry.EntryType = HardDelete?cleHardDelete:cleDelete;
                     BTPS_StringCopy(ChangeLogEntry.LUID, LUID);

                     /* Now add a change log entry for this operation.  */
                     OBJSTORE_AddChangeEntry(Store, &ChangeLogEntry);

                     ChangeCounter = ChangeLogEntry.ChangeCounter;

                     /* If the SyncAnchor pointer is present populate it*/
                     /* with the new CC value.                          */
                     if(SyncAnchor)
                     {
                        SyncAnchor->Anchor_Type               = atChangeCounter;
                        SyncAnchor->Anchor_Data.ChangeCounter = ChangeCounter;
                     }

                     /* Release this object entry and its associated    */
                     /* data buffers.                                   */
                     OBJSTORE_ReleaseEntry(FoundEntry);
                  }
               }
               else
               {
                  /* No object at that index to delete.                 */
                  ret_val = SYNC_OBEX_RESPONSE_NOT_FOUND;
               }
            }
            else
            {
               /* MaxChangeCounter is less than current change counter  */
               /* for this object store.  We will reject this operation */
               /* as a conflict.                                        */
               ret_val = SYNC_OBEX_RESPONSE_CONFLICT;
            }
         }
         else
         {
            /* Invalid input parameters.                                */
            ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
         }
      }
      else
      {
         /* Delete permission flag not set.                             */
         ret_val = SYNC_OBEX_RESPONSE_UNAUTHORIZED;
      }

      /* Release Acquired Mutex.                                        */
      BTPS_ReleaseMutex(Store->Mutex);
   }
   else
   {
      /* Error acquiring mutex or mutex not initialized.                */
      ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
   }

   return(ret_val);
}

int OBJSTORE_WriteChangeCounterToBuffer(IrMC_Object_Store_t *Store, DWord_t BufferSize, Byte_t *Buffer)
{
   int  ret_val = SYNC_OBEX_RESPONSE_OK;
   int  Length;
   char LineBuffer[OBJSTORE_LINE_BUFFER_LENGTH];

   /* Verify that buffer pointer and buffer size are valid for this     */
   /* operation.                                                        */
   if((Buffer) && (BufferSize))
   {
      /* Before we output the change counter we will scan for changes   */
      /* in the object store.                                           */
      OBJSTORE_RegisterChangesByHash(Store);

      if((Store) && (Store->Mutex) && (BTPS_WaitMutex(Store->Mutex, BTPS_INFINITE_WAIT)))
      {
         /* Output string to internal Line Buffer.                      */
         Length = BTPS_SprintF(LineBuffer, "%lu", Store->ChangeCounter);

         /* See if output returned an error (negative value) or the     */
         /* number of bytes exceeds the buffer length.                  */
         if((Length > 0) && ((DWord_t)(Length + 1) <= BufferSize))
         {
            /* Copy line to output buffer                               */
            BTPS_MemCopy(Buffer, LineBuffer, (Length + 1));
         }
         else
         {
            ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
         }
         BTPS_ReleaseMutex(Store->Mutex);
      }
      else
      {
         ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
      }
   }
   else
   {
      ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
   }
   return(ret_val);
}

   /* The following function writes a passed device information log into*/
   /* the buffer pointed to by the Buffer parameter.  The BufferSize    */
   /* parameter indicates the maximum number of bytes that can be safely*/
   /* written to the Buffer provided.  The device information log is    */
   /* output in 7-bit ASCII format, including embedded CRLF characters  */
   /* to represent multiline data.  The buffer will be terminated with a*/
   /* NULL character.  If the entire device information log can't be    */
   /* written within BufferSize characters, the return value will       */
   /* indicate an error.                                                */
int OBJSTORE_WriteDevInfoToBuffer(const IrMC_Device_Information_Log_t *DevInfo, DWord_t BufferSize, Byte_t *Buffer)
{
   int      Index;
   int      Length;
   int      ret_val = SYNC_OBEX_RESPONSE_OK;
   char     LineBuffer[OBJSTORE_LINE_BUFFER_LENGTH];
   Byte_t  *WritePtr;
   DWord_t  BufferLength;

   /* Verify parameters for validity                                    */
   if((DevInfo) && (BufferSize) && (Buffer))
   {
      BufferLength = BufferSize;
      WritePtr     = Buffer;

      /* Write to line buffer so size can be calculated.                */
      Length = BTPS_SprintF(LineBuffer, "MANU:%s\r\n", DevInfo->Manufacturer?DevInfo->Manufacturer:"");

      if((Length > 0) && ((DWord_t)Length <= BufferLength))
      {
         /* Subtract from available length                              */
         BufferLength -= Length;

         /* Copy to output buffer                                       */
         BTPS_MemCopy(WritePtr, LineBuffer, Length);

         WritePtr += Length;
      }
      else
         ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;

      /* Ensure that we did not run out of space on the last pair.      */
      if(ret_val == SYNC_OBEX_RESPONSE_OK)
      {
         /* Write to line buffer so size can be calculated.             */
         Length = BTPS_SprintF(LineBuffer, "MOD:%s\r\n", DevInfo->Model?DevInfo->Model:"");
         if((Length > 0) && ((DWord_t)Length <= BufferLength))
         {
            /* Subtract from available length                           */
            BufferLength -= Length;

            /* Copy to output buffer                                    */
            BTPS_MemCopy(WritePtr, LineBuffer, Length);

            WritePtr += Length;
         }
         else
            ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
      }

      /* Ensure that we did not run out of space on the last pair.      */
      if((!ret_val) && (DevInfo->OEM))
      {
         /* Write to line buffer so size can be calculated.             */
         Length = BTPS_SprintF(LineBuffer, "OEM:%s\r\n", DevInfo->OEM);
         if((Length > 0) && ((DWord_t)Length <= BufferLength))
         {
            /* Subtract from available length                           */
            BufferLength -= Length;

            /* Copy to output buffer                                    */
            BTPS_MemCopy(WritePtr, LineBuffer, Length);

            WritePtr += Length;
         }
         else
         {
            ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
         }
      }

      /* Ensure that we did not run out of space on the last pair.      */
      if((ret_val == SYNC_OBEX_RESPONSE_OK) && (DevInfo->FirmwareVersion != NULL))
      {
         /* Write to line buffer so size can be calculated.             */
         Length = BTPS_SprintF(LineBuffer, "FW-VERSION:%s\r\n", DevInfo->FirmwareVersion);
         if((Length > 0) && ((DWord_t)Length <= BufferLength))
         {
            /* Subtract from available length                           */
            BufferLength -= Length;

            /* Copy to output buffer                                    */
            BTPS_MemCopy(WritePtr, LineBuffer, Length);

            WritePtr += Length;
         }
         else
            ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
      }

      /* Ensure that we did not run out of space on the last pair.      */
      if((ret_val == SYNC_OBEX_RESPONSE_OK) && (DevInfo->FirmwareDate != NULL))
      {
         /* Write to line buffer so size can be calculated.             */
         Length = BTPS_SprintF(LineBuffer, "FW-DATE:%s\r\n", DevInfo->FirmwareDate);
         if((Length > 0) && ((DWord_t)Length <= BufferLength))
         {
            /* Subtract from available length                           */
            BufferLength -= Length;

            /* Copy to output buffer                                    */
            BTPS_MemCopy(WritePtr, LineBuffer, Length);

            WritePtr += Length;
         }
         else
            ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
      }

      /* Ensure that we did not run out of space on the last pair.      */
      if((ret_val == SYNC_OBEX_RESPONSE_OK) && (DevInfo->SoftwareVersion != NULL))
      {
         /* Write to line buffer so size can be calculated.             */
         Length = BTPS_SprintF(LineBuffer, "SW-VERSION:%s\r\n", DevInfo->SoftwareVersion);
         if((Length > 0) && ((DWord_t)Length <= BufferLength))
         {
            /* Subtract from available length                           */
            BufferLength -= Length;

            /* Copy to output buffer                                    */
            BTPS_MemCopy(WritePtr, LineBuffer, Length);

            WritePtr += Length;
         }
         else
            ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
      }

      /* Ensure that we did not run out of space on the last pair.      */
      if((ret_val == SYNC_OBEX_RESPONSE_OK) && (DevInfo->SoftwareDate != NULL))
      {
         /* Write to line buffer so size can be calculated.             */
         Length = BTPS_SprintF(LineBuffer, "SW-DATE:%s\r\n", DevInfo->SoftwareDate);
         if((Length > 0) && ((DWord_t)Length <= BufferLength))
         {
            /* Subtract from available length                           */
            BufferLength -= Length;

            /* Copy to output buffer                                    */
            BTPS_MemCopy(WritePtr, LineBuffer, Length);

            WritePtr += Length;
         }
         else
            ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
      }

      /* Ensure that we did not run out of space on the last pair.      */
      if((ret_val == SYNC_OBEX_RESPONSE_OK) && (DevInfo->IrMCVersion))
      {
         /* Write to line buffer so size can be calculated.             */
         Length = BTPS_SprintF(LineBuffer, "IRMC-VERSION:%s\r\n", DevInfo->IrMCVersion);
         if((Length > 0) && ((DWord_t)Length <= BufferLength))
         {
            /* Subtract from available length                           */
            BufferLength -= Length;

            /* Copy to output buffer                                    */
            BTPS_MemCopy(WritePtr, LineBuffer, Length);

            WritePtr += Length;
         }
         else
            ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
      }

      /* Ensure that we did not run out of space on the last pair.      */
      if((ret_val == SYNC_OBEX_RESPONSE_OK) && (DevInfo->HardwareVersion != NULL))
      {
         /* Write to line buffer so size can be calculated.             */
         Length = BTPS_SprintF(LineBuffer, "HW-VERSION:%s\r\n", DevInfo->HardwareVersion);
         if((Length > 0) && ((DWord_t)Length <= BufferLength))
         {
            /* Subtract from available length                           */
            BufferLength -= Length;

            /* Copy to output buffer                                    */
            BTPS_MemCopy(WritePtr, LineBuffer, Length);

            WritePtr += Length;
         }
         else
            ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
      }

      /* Ensure that we did not run out of space on the last pair.      */
      if((ret_val == SYNC_OBEX_RESPONSE_OK) && (DevInfo->HardwareDate != NULL))
      {
         /* Write to line buffer so size can be calculated.             */
         Length = BTPS_SprintF(LineBuffer, "HW-DATE:%s\r\n", DevInfo->HardwareDate);
         if((Length > 0) && ((DWord_t)Length <= BufferLength))
         {
            /* Subtract from available length                           */
            BufferLength -= Length;

            /* Copy to output buffer                                    */
            BTPS_MemCopy(WritePtr, LineBuffer, Length);

            WritePtr += Length;
         }
         else
            ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
      }

      /* Ensure that we did not run out of space on the last pair.      */
      if(ret_val == SYNC_OBEX_RESPONSE_OK)
      {
         /* Write to line buffer so size can be calculated.             */
         Length = BTPS_SprintF(LineBuffer, "SN:%s\r\n", DevInfo->SerialNumber?DevInfo->SerialNumber:"");
         if((Length > 0) && ((DWord_t)Length <= BufferLength))
         {
            /* Subtract from available length                           */
            BufferLength -= Length;

            /* Copy to output buffer                                    */
            BTPS_MemCopy(WritePtr, LineBuffer, Length);

            WritePtr += Length;
         }
         else
            ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
      }

      /* Ensure that we did not run out of space on the last pair.      */
      if(ret_val == SYNC_OBEX_RESPONSE_OK)
      {
         /* Write to line buffer so size can be calculated.             */
         Length = BTPS_SprintF(LineBuffer, "PB-TYPE-TX:%s\r\n", DevInfo->PhonebookType?DevInfo->PhonebookType:"");
         if((Length > 0) && ((DWord_t)Length <= BufferLength))
         {
            /* Subtract from available length                           */
            BufferLength -= Length;

            /* Copy to output buffer                                    */
            BTPS_MemCopy(WritePtr, LineBuffer, Length);

            WritePtr += Length;
         }
         else
            ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
      }

      /* Ensure that we did not run out of space on the last pair.      */
      if(ret_val == SYNC_OBEX_RESPONSE_OK)
      {
         /* Write to line buffer so size can be calculated.             */
         Length = BTPS_SprintF(LineBuffer, "PB-TYPE-RX:%s\r\n", DevInfo->PhonebookType?DevInfo->PhonebookType:"");
         if((Length > 0) && ((DWord_t)Length <= BufferLength))
         {
            /* Subtract from available length                           */
            BufferLength -= Length;

            /* Copy to output buffer                                    */
            BTPS_MemCopy(WritePtr, LineBuffer, Length);

            WritePtr += Length;
         }
         else
            ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
      }

      /* Ensure that we did not run out of space on the last pair.      */
      if(ret_val == SYNC_OBEX_RESPONSE_OK)
      {
         /* Write to line buffer so size can be calculated.             */
         Length = BTPS_SprintF(LineBuffer, "CAL-TYPE-TX:%s\r\n", DevInfo->CalendarType?DevInfo->CalendarType:"");
         if((Length > 0) && ((DWord_t)Length <= BufferLength))
         {
            /* Subtract from available length                           */
            BufferLength -= Length;

            /* Copy to output buffer                                    */
            BTPS_MemCopy(WritePtr, LineBuffer, Length);

            WritePtr += Length;
         }
         else
            ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
      }

      /* Ensure that we did not run out of space on the last pair.      */
      if(!ret_val)
      {
         /* Write to line buffer so size can be calculated.             */
         Length = BTPS_SprintF(LineBuffer, "CAL-TYPE-RX:%s\r\n", DevInfo->CalendarType?DevInfo->CalendarType:"");
         if((Length > 0) && ((DWord_t)Length <= BufferLength))
         {
            /* Subtract from available length                           */
            BufferLength -= Length;

            /* Copy to output buffer                                    */
            BTPS_MemCopy(WritePtr, LineBuffer, Length);

            WritePtr += Length;
         }
         else
            ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
      }

      /* Ensure that we did not run out of space on the last pair.      */
      if(ret_val == SYNC_OBEX_RESPONSE_OK)
      {
         /* Write to line buffer so size can be calculated.             */
         Length = BTPS_SprintF(LineBuffer, "MSG-TYPE-TX:%s\r\n", DevInfo->MessageType?DevInfo->MessageType:"");
         if((Length > 0) && ((DWord_t)Length <= BufferLength))
         {
            /* Subtract from available length                           */
            BufferLength -= Length;

            /* Copy to output buffer                                    */
            BTPS_MemCopy(WritePtr, LineBuffer, Length);

            WritePtr += Length;
         }
         else
            ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
      }

      /* Ensure that we did not run out of space on the last pair.      */
      if(ret_val == SYNC_OBEX_RESPONSE_OK)
      {
         /* Write to line buffer so size can be calculated.             */
         Length = BTPS_SprintF(LineBuffer, "MSG-TYPE-RX:%s\r\n", DevInfo->MessageType?DevInfo->MessageType:"");
         if((Length > 0) && ((DWord_t)Length <= BufferLength))
         {
            /* Subtract from available length                           */
            BufferLength -= Length;

            /* Copy to output buffer                                    */
            BTPS_MemCopy(WritePtr, LineBuffer, Length);

            WritePtr += Length;
         }
         else
            ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
      }

      /* Ensure that we did not run out of space on the last pair.      */
      if(ret_val == SYNC_OBEX_RESPONSE_OK)
      {
         /* Write to line buffer so size can be calculated.             */
         Length = BTPS_SprintF(LineBuffer, "NOTE-TYPE-TX:%s\r\n", DevInfo->NoteType?DevInfo->NoteType:"");
         if((Length > 0) && ((DWord_t)Length <= BufferLength))
         {
            /* Subtract from available length                           */
            BufferLength -= Length;

            /* Copy to output buffer                                    */
            BTPS_MemCopy(WritePtr, LineBuffer, Length);

            WritePtr += Length;
         }
         else
            ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
      }

      /* Ensure that we did not run out of space on the last pair.      */
      if(ret_val == SYNC_OBEX_RESPONSE_OK)
      {
         /* Write to line buffer so size can be calculated.             */
         Length = BTPS_SprintF(LineBuffer, "NOTE-TYPE-RX:%s\r\n", DevInfo->NoteType?DevInfo->NoteType:"");
         if((Length > 0) && ((DWord_t)Length <= BufferLength))
         {
            /* Subtract from available length                           */
            BufferLength -= Length;

            /* Copy to output buffer                                    */
            BTPS_MemCopy(WritePtr, LineBuffer, Length);

            WritePtr += Length;
         }
         else
            ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
      }

      /* Ensure that we did not run out of space on the last pair.      */
      if(ret_val == SYNC_OBEX_RESPONSE_OK)
      {
         /* Write to line buffer so size can be calculated.             */
         Length = BTPS_SprintF(LineBuffer, "INBOX:%s\r\n", DevInfo->InboxCapability?DevInfo->InboxCapability:"");
         if((Length > 0) && ((DWord_t)Length <= BufferLength))
         {
            /* Subtract from available length                           */
            BufferLength -= Length;

            /* Copy to output buffer                                    */
            BTPS_MemCopy(WritePtr, LineBuffer, Length);

            WritePtr += Length;
         }
         else
            ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
      }

      /* Ensure that we did not run out of space on the last pair.      */
      if(ret_val == SYNC_OBEX_RESPONSE_OK)
      {
         /* Write to line buffer so size can be calculated.             */
         Length = BTPS_SprintF(LineBuffer, "MSG-SENT-BOX:%s\r\n", DevInfo->SentboxCapability?DevInfo->SentboxCapability:"");
         if((Length > 0) && ((DWord_t)Length <= BufferLength))
         {
            /* Subtract from available length                           */
            BufferLength -= Length;

            /* Copy to output buffer                                    */
            BTPS_MemCopy(WritePtr, LineBuffer, Length);

            WritePtr += Length;
         }
         else
            ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
      }

      /* Now finally we must write the extensions if they are present.  */
      if((ret_val == SYNC_OBEX_RESPONSE_OK) && (DevInfo->ExtensionsArray != NULL) && (DevInfo->NumberExtensions > 0))
      {
         for(Index=0;((unsigned int)Index < (DevInfo->NumberExtensions));Index++)
         {
            Length = BTPS_SprintF(LineBuffer, "%s:%s\r\n", DevInfo->ExtensionsArray[Index].Name, DevInfo->ExtensionsArray[Index].Value);
            if((Length > 0) && ((DWord_t)Length <= BufferLength))
            {
               /* Subtract from available length                        */
               BufferLength -= Length;

               /* Copy to output buffer                                 */
               BTPS_MemCopy(WritePtr, LineBuffer, Length);

               WritePtr += Length;
            }
            else
               ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
         }
      }

      /* See if we have any buffer space remaining to append a final    */
      /* NULL terminating character.                                    */
      if((ret_val == SYNC_OBEX_RESPONSE_OK) && (BufferLength > 0))
      {
         /* Finally append terminating NULL to the buffer.              */
         *WritePtr = '\0';
      }
      else
         ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
   }
   else
      ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;

   return(ret_val);
}

   /* The following function writes a passed information log into the   */
   /* buffer pointed to by the Buffer parameter.  The BufferSize        */
   /* parameter indicates the maximum number of bytes that can be safely*/
   /* written to the Buffer provided.  The information log is output in */
   /* 7-bit ASCII format, including embedded CRLF characters to         */
   /* represent multiline data.  The buffer will be terminated with a   */
   /* NULL character.  If the entire information log can't be written   */
   /* within BufferSize characters, the return value will indicate an   */
   /* error.                                                            */
int OBJSTORE_WriteInfoLogToBuffer(IrMC_Object_Store_t *Store, DWord_t BufferSize, Byte_t *Buffer)
{
   int                           Length;
   int                           ret_val = SYNC_OBEX_RESPONSE_OK;
   char                          LineBuffer[OBJSTORE_LINE_BUFFER_LENGTH];
   Byte_t                       *WritePtr;
   DWord_t                       BufferLength;
   unsigned int                  Index;
   const IrMC_Information_Log_t *InfoLog;

   /* Verify parameters for validity.                                   */
   if((BufferSize) && (Buffer))
   {
      /* Before we output the info log we will scan for changes in the  */
      /* object store.                                                  */
      OBJSTORE_RegisterChangesByHash(Store);

      if((Store) && (Store->Mutex) && (BTPS_WaitMutex(Store->Mutex, BTPS_INFINITE_WAIT)))
      {
         /* Setup pointer to InfoLog for this object store.             */
         InfoLog = &(Store->InfoLog);

         BufferLength = BufferSize;
         WritePtr     = Buffer;

         /* Write to line buffer so size can be calculated.             */
         Length = BTPS_SprintF(LineBuffer, "Total-Records:%lu\r\n", InfoLog->TotalRecords);
         if((Length > 0) && ((DWord_t)Length <= BufferLength))
         {
            /* Subtract from available length                           */
            BufferLength -= Length;

            /* Copy to output buffer                                    */
            BTPS_MemCopy(WritePtr, LineBuffer, Length);

            WritePtr += Length;
         }
         else
            ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;

         /* Ensure that we did not run out of space on the last pair.   */
         if((ret_val == SYNC_OBEX_RESPONSE_OK) && (InfoLog->UseLastUsedIndex))
         {
            /* Write to line buffer so size can be calculated.          */
            Length = BTPS_SprintF(LineBuffer, "Last-Used-Index:%lu\r\n", InfoLog->LastUsedIndex);
            if((Length > 0) && ((DWord_t)Length <= BufferLength))
            {
               /* Subtract from available length                        */
               BufferLength -= Length;

               /* Copy to output buffer                                 */
               BTPS_MemCopy(WritePtr, LineBuffer, Length);

               WritePtr += Length;
            }
            else
               ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
         }

         /* Ensure that we did not run out of space on the last pair.   */
         if(ret_val == SYNC_OBEX_RESPONSE_OK)
         {
            /* Zero indicates an unlimited number of maximum records    */
            if(!InfoLog->MaximumRecords)
            {
               Length = BTPS_SprintF(LineBuffer, "Maximum-Records:*\r\n");
            }
            else
            {
               Length = BTPS_SprintF(LineBuffer, "Maximum-Records:%lu\r\n", InfoLog->MaximumRecords);
            }

            if((Length > 0) && ((DWord_t)Length <= BufferLength))
            {
               /* Subtract from available length                        */
               BufferLength -= Length;

               /* Copy to output buffer                                 */
               BTPS_MemCopy(WritePtr, LineBuffer, Length);

               WritePtr += Length;
            }
            else
               ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
         }

         /* Ensure that we did not run out of space on the last pair.   */
         if((ret_val == SYNC_OBEX_RESPONSE_OK) && (InfoLog->InformationExchangeLevel != 0))
         {
            /* NOTE: IrMC Specification has a typo regarding this field.*/
            /* The errata, IrMC_Errata_991014.pdf, clarifies that the   */
            /* values listed in the table (0x01, 0x02, 0x04, 0x08, and  */
            /* 0x10) are the correct values instead of (1,2,3,4,5) as   */
            /* the example shows.  The Errata seems to suggest that the */
            /* actual string "0x01" should be used.  Examining other    */
            /* available implementations shows that others are using    */
            /* (1,2,4,8,10) without the "0x" preceeding.  This          */
            /* implementation will follow this form until further       */
            /* testing and verification against other implementations   */
            /* can be performed.                                        */
            Length = BTPS_SprintF(LineBuffer, "IEL:%u\r\n", (unsigned int)InfoLog->InformationExchangeLevel);
            if((Length > 0) && ((DWord_t)Length <= BufferLength))
            {
               /* Subtract from available length                        */
               BufferLength -= Length;

               /* Copy to output buffer                                 */
               BTPS_MemCopy(WritePtr, LineBuffer, Length);

               WritePtr += Length;
            }
            else
               ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
         }

         /* The following field types are only used in a Level 4 (Sync) */
         /* IrMC Implementation.                                        */
         if(InfoLog->UseLevel4Format)
         {
            /* Ensure that we did not run out of space on the last pair.*/
            if(ret_val == SYNC_OBEX_RESPONSE_OK)
            {
               /* Write to line buffer so size can be calculated.       */
               Length = BTPS_SprintF(LineBuffer, "HD:%s\r\n", ((InfoLog->HardDelete)?("YES"):("NO")));
               if((Length > 0) && ((DWord_t)Length <= BufferLength))
               {
                  /* Subtract from available length                     */
                  BufferLength -= Length;

                  /* Copy to output buffer                              */
                  BTPS_MemCopy(WritePtr, LineBuffer, Length);

                  WritePtr += Length;
               }
               else
                  ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
            }

            /* Ensure that we did not run out of space on the last pair.*/
            if(ret_val == SYNC_OBEX_RESPONSE_OK)
            {
               /* Write to line buffer so size can be calculated.       */
               switch(InfoLog->SyncAnchorType)
               {
                  case IRMC_SYNC_ANCHOR_TYPE_BIT_CC:
                     Length = BTPS_SprintF(LineBuffer, "SAT:CC\r\n");
                     break;
                  case IRMC_SYNC_ANCHOR_TYPE_BIT_TS:
                     Length = BTPS_SprintF(LineBuffer, "SAT:TS\r\n");
                     break;
                  case (IRMC_SYNC_ANCHOR_TYPE_BIT_CC | IRMC_SYNC_ANCHOR_TYPE_BIT_TS):
                     Length = BTPS_SprintF(LineBuffer, "SAT:CT\r\n");
                     break;
               }

               if((Length > 0) && ((DWord_t)Length <= BufferLength))
               {
                  /* Subtract from available length                     */
                  BufferLength -= Length;

                  /* Copy to output buffer                              */
                  BTPS_MemCopy(WritePtr, LineBuffer, Length);

                  WritePtr += Length;
               }
               else
                  ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
            }

            /* Ensure that we did not run out of space on the last pair.*/
            if((ret_val == SYNC_OBEX_RESPONSE_OK) && (InfoLog->SyncAnchorType & IRMC_SYNC_ANCHOR_TYPE_BIT_TS))
            {
               /* Write to line buffer so size can be calculated.       */
               Length = BTPS_SprintF(LineBuffer, "SAI:%s\r\n", ((InfoLog->SyncAnchorIncrement)?("YES"):("NO")));
               if((Length > 0) && ((DWord_t)Length <= BufferLength))
               {
                  /* Subtract from available length                     */
                  BufferLength -= Length;

                  /* Copy to output buffer                              */
                  BTPS_MemCopy(WritePtr, LineBuffer, Length);

                  WritePtr += Length;
               }
               else
                  ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
            }

            /* Ensure that we did not run out of space on the last pair.*/
            if((ret_val == SYNC_OBEX_RESPONSE_OK) && (InfoLog->SyncAnchorType & IRMC_SYNC_ANCHOR_TYPE_BIT_TS))
            {
               /* Write to line buffer so size can be calculated.       */
               Length = BTPS_SprintF(LineBuffer, "SAU:%s\r\n", ((InfoLog->SyncAnchorUnique)?("YES"):("NO")));
               if((Length > 0) && ((DWord_t)Length <= BufferLength))
               {
                  /* Subtract from available length                     */
                  BufferLength -= Length;

                  /* Copy to output buffer                              */
                  BTPS_MemCopy(WritePtr, LineBuffer, Length);

                  WritePtr += Length;
               }
               else
                  ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
            }

            /* Ensure that we did not run out of space on the last pair.*/
            if(ret_val == SYNC_OBEX_RESPONSE_OK)
            {
               /* Write to line buffer so size can be calculated.       */
               Length = BTPS_SprintF(LineBuffer, "DID:%s%lu\r\n", DatabaseID_Prefix, InfoLog->DatabaseID);
               if((Length > 0) && ((DWord_t)Length <= BufferLength))
               {
                  /* Subtract from available length                     */
                  BufferLength -= Length;

                  /* Copy to output buffer                              */
                  BTPS_MemCopy(WritePtr, LineBuffer, Length);

                  WritePtr += Length;
               }
               else
                  ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
            }
         }

         /* Dynamic Field Mappings                                      */
         if((ret_val == SYNC_OBEX_RESPONSE_OK) && (InfoLog->NumberFields > 0) && (InfoLog->FieldsArray != NULL))
         {
            /* Write opening tags of dynamic fields section.            */
            Length = BTPS_SprintF(LineBuffer, "X-IRMC-FIELDS:\r\n<Begin>\r\n");
            if((Length > 0) && ((DWord_t)Length <= BufferLength))
            {
               /* Subtract from available length                        */
               BufferLength -= Length;

               /* Copy to output buffer                                 */
               BTPS_MemCopy(WritePtr, LineBuffer, Length);

               WritePtr += Length;
            }
            else
               ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;

            for(Index=0; ((Index < (InfoLog->NumberFields)) && (ret_val == SYNC_OBEX_RESPONSE_OK)); Index++)
            {
               Length = BTPS_SprintF(LineBuffer, "%s\r\n", InfoLog->FieldsArray[Index]);
               if((Length > 0) && ((DWord_t)Length <= BufferLength))
               {
                  /* Subtract from available length                     */
                  BufferLength -= Length;

                  /* Copy to output buffer                              */
                  BTPS_MemCopy(WritePtr, LineBuffer, Length);

                  WritePtr += Length;
               }
               else
                  ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
            }

            if(ret_val == SYNC_OBEX_RESPONSE_OK)
            {
               /* Write closing tags of dynamic fields section.         */
               Length = BTPS_SprintF(LineBuffer, "<End>\r\n");
               if((Length > 0) && ((DWord_t)Length <= BufferLength))
               {
                  /* Subtract from available length                     */
                  BufferLength -= Length;

                  /* Copy to output buffer                              */
                  BTPS_MemCopy(WritePtr, LineBuffer, Length);

                  WritePtr += Length;
               }
               else
                  ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
            }
         }


         /* The following field types are only used in a Level 4 (Sync) */
         /* IrMC Implementation for the Phonebook store type.           */
         if((InfoLog->UseLevel4Format) && (InfoLog->StoreType == osPhonebook))
         {
            /* Ensure that we did not run out of space on the last pair.*/
            if(ret_val == SYNC_OBEX_RESPONSE_OK)
            {
               /* Write to line buffer so size can be calculated.       */
               Length = BTPS_SprintF(LineBuffer, "ICL:%s\r\n", ((InfoLog->IncomingCallLog)?("YES"):("NO")));
               if((Length > 0) && ((DWord_t)Length <= BufferLength))
               {
                  /* Subtract from available length                     */
                  BufferLength -= Length;

                  /* Copy to output buffer                              */
                  BTPS_MemCopy(WritePtr, LineBuffer, Length);

                  WritePtr += Length;
               }
               else
                  ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
            }

            /* Ensure that we did not run out of space on the last pair.*/
            if(ret_val == SYNC_OBEX_RESPONSE_OK)
            {
               /* Write to line buffer so size can be calculated.       */
               Length = BTPS_SprintF(LineBuffer, "OCL:%s\r\n", ((InfoLog->OutgoingCallLog)?("YES"):("NO")));
               if((Length > 0) && ((DWord_t)Length <= BufferLength))
               {
                  /* Subtract from available length                     */
                  BufferLength -= Length;

                  /* Copy to output buffer                              */
                  BTPS_MemCopy(WritePtr, LineBuffer, Length);

                  WritePtr += Length;
               }
               else
                  ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
            }

            /* Ensure that we did not run out of space on the last pair.*/
            if(ret_val == SYNC_OBEX_RESPONSE_OK)
            {
               /* Write to line buffer so size can be calculated.       */
               Length = BTPS_SprintF(LineBuffer, "MCL:%s\r\n", ((InfoLog->MissedCallLog)?("YES"):("NO")));
               if((Length > 0) && ((DWord_t)Length <= BufferLength))
               {
                  /* Subtract from available length                     */
                  BufferLength -= Length;

                  /* Copy to output buffer                              */
                  BTPS_MemCopy(WritePtr, LineBuffer, Length);

                  WritePtr += Length;
               }
               else
                  ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
            }
         }

         /* The following field types are only used in a Level 4 (Sync) */
         /* IrMC Implementation for the Message store type.             */
         if((InfoLog->UseLevel4Format) && ((InfoLog->StoreType == osMsgIn) || (InfoLog->StoreType == osMsgOut) || (InfoLog->StoreType == osMsgSent)))
         {
            /* Ensure that we did not run out of space on the last pair.*/
            if(ret_val == SYNC_OBEX_RESPONSE_OK)
            {
               /* Write to line buffer so size can be calculated.       */
               Length = BTPS_SprintF(LineBuffer, "MMHL:%s\r\n", ((InfoLog->MissedMessageHistoryLog)?("YES"):("NO")));
               if((Length > 0) && ((DWord_t)Length <= BufferLength))
               {
                  /* Subtract from available length                     */
                  BufferLength -= Length;

                  /* Copy to output buffer                              */
                  BTPS_MemCopy(WritePtr, LineBuffer, Length);

                  WritePtr += Length;
               }
               else
                  ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
            }
         }

         /* See if we have any buffer space remaining to append a final */
         /* NULL terminating character.                                 */
         if((ret_val == SYNC_OBEX_RESPONSE_OK) && (BufferLength > 0))
         {
            /* Finally append terminating NULL to the buffer.           */
            *WritePtr = '\0';
         }
         else
            ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;

         /* Release acquired mutex                                      */
         BTPS_ReleaseMutex(Store->Mutex);
      }
      else
      {
         ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
      }
   }
   else
   {
      ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
   }

   return(ret_val);
}

int OBJSTORE_WriteChangeLogToBuffer(IrMC_Object_Store_t *Store, IrMC_Anchor_t *SyncAnchor, DWord_t BufferSize, Byte_t *Buffer)
{
   int                      Length;
   int                      ret_val = SYNC_OBEX_RESPONSE_OK;
   char                     LineBuffer[OBJSTORE_LINE_BUFFER_LENGTH];
   char                     LUIDBuffer[SYNC_MAX_LUID_CHARACTER_SIZE + 1];
   char                    *tmpPtr;
   Byte_t                  *WritePtr;
   DWord_t                  BufferLength;
   Boolean_t                HardDeleteOnly;
   IrMC_Change_Log_Entry_t *ChangeLogEntry;

   /* Verify parameters for validity                                    */
   if((SyncAnchor) && (BufferSize) && (Buffer))
   {
      /* Make sure this is a Change Counter type request.  Timestamp is */
      /* not supported by this implementation.                          */
      if(SyncAnchor->Anchor_Type == atChangeCounter)
      {
         /* Before we output the change log we will scan for changes in */
         /* the object store.                                           */
         OBJSTORE_RegisterChangesByHash(Store);

         if((Store) && (Store->Mutex) && (BTPS_WaitMutex(Store->Mutex, BTPS_INFINITE_WAIT)))
         {
            BufferLength = BufferSize;
            WritePtr     = Buffer;

            /* Write header of the change log format Write to line      */
            /* buffer so size can be calculated.                        */
            Length = BTPS_SprintF(LineBuffer, "SN:%s\r\n", Store->ChangeLog.SerialNumber?Store->ChangeLog.SerialNumber:"");
            if((Length > 0) && ((DWord_t)Length <= BufferLength))
            {
               /* Subtract from available length                        */
               BufferLength -= Length;

               /* Copy to output buffer                                 */
               BTPS_MemCopy(WritePtr, LineBuffer, Length);

               WritePtr += Length;
            }
            else
               ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;

            /* Ensure that we did not run out of space on the last pair.*/
            if(ret_val == SYNC_OBEX_RESPONSE_OK)
            {
               /* Write to line buffer so size can be calculated.       */
               Length = BTPS_SprintF(LineBuffer, "DID:%s%lu\r\n", DatabaseID_Prefix, Store->InfoLog.DatabaseID);
               if((Length > 0) && ((DWord_t)Length <= BufferLength))
               {
                  /* Subtract from available length                     */
                  BufferLength -= Length;

                  /* Copy to output buffer                              */
                  BTPS_MemCopy(WritePtr, LineBuffer, Length);

                  WritePtr += Length;
               }
               else
                  ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
            }

            /* Ensure that we did not run out of space on the last pair.*/
            if(ret_val == SYNC_OBEX_RESPONSE_OK)
            {
               /* Write to line buffer so size can be calculated.       */
               Length = BTPS_SprintF(LineBuffer, "Total-Records:%lu\r\n", Store->InfoLog.TotalRecords);
               if((Length > 0) && ((DWord_t)Length <= BufferLength))
               {
                  /* Subtract from available length                     */
                  BufferLength -= Length;

                  /* Copy to output buffer                              */
                  BTPS_MemCopy(WritePtr, LineBuffer, Length);

                  WritePtr += Length;
               }
               else
                  ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
            }

            /* Ensure that we did not run out of space on the last pair.*/
            if(ret_val == SYNC_OBEX_RESPONSE_OK)
            {
               /* Zero indicates an unlimited number of maximum records */
               if(!Store->InfoLog.MaximumRecords)
               {
                  Length = BTPS_SprintF(LineBuffer, "Maximum-Records:*\r\n");
               }
               else
               {
                  Length = BTPS_SprintF(LineBuffer, "Maximum-Records:%lu\r\n", Store->InfoLog.MaximumRecords);
               }

               if((Length > 0) && ((DWord_t)Length <= BufferLength))
               {
                  /* Subtract from available length                     */
                  BufferLength -= Length;

                  /* Copy to output buffer                              */
                  BTPS_MemCopy(WritePtr, LineBuffer, Length);

                  WritePtr += Length;
               }
               else
                  ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
            }

            /* Check for case where anchor is larger than the newest    */
            /* record.  In this case we do not need to return the       */
            /* entries.                                                 */
            if((SyncAnchor->Anchor_Data.ChangeCounter >= GetChangeCounter(Store, FALSE)))
            {
               /* Anchor is larger than the newest record in our store. */
               /* We will return a '*' character to indicate full sync  */
               /* may be required.                                      */
               if(ret_val == SYNC_OBEX_RESPONSE_OK)
               {
                  Length = BTPS_SprintF(LineBuffer, "*\r\n");

                  /* Ensure we will not overrun the buffer here.        */
                  if((Length > 0) && ((DWord_t)Length <= BufferLength))
                  {
                     /* Subtract from available length                  */
                     BufferLength -= Length;

                     /* Copy to output buffer                           */
                     BTPS_MemCopy(WritePtr, LineBuffer, Length);

                     WritePtr += Length;
                  }
                  else
                     ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
               }
            }
            else
            {
               /* Check for case where anchor is SMALLER than the OLDEST*/
               /* record.  In this case we need to send a '*' and       */
               /* followed by only Hard Delete records.                 */
               if((SyncAnchor->Anchor_Data.ChangeCounter) < (Store->ChangeLog.OldestChangeCounter))
               {
                  /* Proceed w/ iteration through the change entries,   */
                  /* but only output the hard delete entries.  After    */
                  /* entries are output we will add an '*' on the last  */
                  /* line.                                              */
                  HardDeleteOnly = TRUE;
               }
               else
                  HardDeleteOnly = FALSE;

               /* This is not an out of range request.  Setup to iterate*/
               /* through log entries and compare to passed sync anchor.*/
               ChangeLogEntry = Store->ChangeLog.ChangeLogEntriesList;
               while((ChangeLogEntry != NULL) && (ret_val == SYNC_OBEX_RESPONSE_OK))
               {
                  /* Check if this entry is within range.               */
                  if(ChangeLogEntry->ChangeCounter > SyncAnchor->Anchor_Data.ChangeCounter)
                  {
                     /* If we are only sending hard delete entries,     */
                     /* check if this entry is of the appropriate type, */
                     /* otherwise it will be skipped.                   */
                     if((!HardDeleteOnly) || ((HardDeleteOnly) && (ChangeLogEntry->EntryType == cleHardDelete)))
                     {
                        /* This is a change counter entry that is within*/
                        /* range.  Ensure that we did not run out of    */
                        /* space on the last line.                      */
                        if(ret_val == SYNC_OBEX_RESPONSE_OK)
                        {
                           /* Copy LUID into the local LUID buffer.     */
                           /* This will be used to remove any extensions*/
                           /* that may be stored in the LUID field.     */
                           BTPS_StringCopy(LUIDBuffer, ChangeLogEntry->LUID);

                           /* Search for '.' that indicates extension   */
                           tmpPtr = LUIDBuffer;

                           while((*tmpPtr) && (*tmpPtr != '.'))
                              tmpPtr++;

                           /* If we found a '.' then replace w/ NULL to */
                           /* remove extension.                         */
                           if(*tmpPtr == '.')
                              *tmpPtr = '\0';

                           /* Use format that includes the Change       */
                           /* Counter field but excludes the timestamp  */
                           /* field.                                    */
                           Length = BTPS_SprintF(LineBuffer, "%c:%lu::%s\r\n", ((ChangeLogEntry->EntryType == cleModify)?('M'):(((ChangeLogEntry->EntryType == cleHardDelete)?('H'):('D')))), ChangeLogEntry->ChangeCounter, LUIDBuffer);

                           if((Length > 0) && ((DWord_t)Length <= BufferLength))
                           {
                              /* Subtract from available length         */
                              BufferLength -= Length;

                              /* Copy to output buffer                  */
                              BTPS_MemCopy(WritePtr, LineBuffer, Length);

                              WritePtr += Length;
                           }
                           else
                           {
                              /* We are using this value as a special   */
                              /* response code.  This means that we have*/
                              /* run out of buffer space on the change  */
                              /* entries.  We will attempt to return the*/
                              /* entries we were succesful in writing.  */
                              ret_val = SYNC_OBEX_RESPONSE_OBJECT_TOO_LARGE;
                           }
                        }
                     }
                  }

                  /* Next change log entry.                             */
                  ChangeLogEntry = (IrMC_Change_Log_Entry_t *)ChangeLogEntry->NextChangeLogEntryPtr;
               }

               /* If this was a HardDeleteOnly type log, then write the */
               /* '*' as the last entry in this log.                    */
               if(((ret_val == SYNC_OBEX_RESPONSE_OK) || (ret_val == SYNC_OBEX_RESPONSE_OBJECT_TOO_LARGE)) && (HardDeleteOnly))
               {
                  Length = BTPS_SprintF(LineBuffer, "*\r\n");

                  /* Ensure we will not overrun the buffer here.        */
                  if((Length > 0) && ((DWord_t)Length <= BufferLength))
                  {
                     /* Subtract from available length                  */
                     BufferLength -= Length;

                     /* Copy to output buffer                           */
                     BTPS_MemCopy(WritePtr, LineBuffer, Length);

                     WritePtr += Length;
                  }
                  else
                  {
                     /* We are using this value as a special response   */
                     /* code.  This means that we have run out of buffer*/
                     /* space on the change entries.  We will attempt to*/
                     /* return the entries we were succesful in writing.*/
                     ret_val = SYNC_OBEX_RESPONSE_OBJECT_TOO_LARGE;
                  }
               }
            }

            /* See if we have any buffer space remaining to append a    */
            /* final NULL terminating character.                        */
            if(((ret_val == SYNC_OBEX_RESPONSE_OK) || (ret_val == SYNC_OBEX_RESPONSE_OBJECT_TOO_LARGE)) && (BufferLength > 0))
            {
               /* Finally append terminating NULL to the buffer.        */
               *WritePtr = '\0';
               ret_val = SYNC_OBEX_RESPONSE_OK;
            }
            else
            {
               ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
            }

            /* Release Acquired Mutex                                   */
            BTPS_ReleaseMutex(Store->Mutex);
         }
         else
         {
            /* This implementation only supports Change Counter type    */
            /* SyncAnchors.  We will reject this request becuase it uses*/
            /* timestamp anchors.                                       */
            ret_val = SYNC_OBEX_RESPONSE_SERVICE_UNAVAILABLE;
         }
      }
      else
      {
         /* Invalid input parameter.                                    */
         ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
      }
   }

   return(ret_val);
}

   /* The following function iterates through the specied object store  */
   /* using the populate interface and determine if any of the objects  */
   /* have been modified or if any items have been added or deleted.    */
   /* The Store parameter is a pointer to the store to check for        */
   /* changes.                                                          */
void OBJSTORE_RegisterChangesByHash(IrMC_Object_Store_t *Store)
{
   int                      Result;
   unsigned int             i;
   unsigned int             newHash;
   IrMC_Object_Entry_t     *Entry;
   IrMC_Object_Entry_t     *FoundEntry;
   IrMC_Object_Entry_t     *prevEntry;
   IrMC_Change_Log_Entry_t  changeEntry;

   /* This function will iterage through the store and check the hash   */
   /* for each object.  Any objects which have changed will have log    */
   /* entries created in change log.                                    */

   /* Verify init and attempt to acquire mutex for this object store    */
   if((Store) && (Store->Mutex) && (BTPS_WaitMutex(Store->Mutex, BTPS_INFINITE_WAIT)))
   {
      /* First we will call the populate handler to refresh the store   */
      /* and catch any items that need to be added.  The function below */
      /* will see these as hash mismatches and add the change entry for */
      /* each.                                                          */
      __BTPSTRY
      {
         (*Store->Functions.Populate)(Store, Store->StoreData);
      }
      __BTPSEXCEPT(1)
      {
         /* Do Nothing.                                                 */
      }

      /* Iterate through the objects in this store.                     */
      for(i=0;(i < (Store->Index));i++)
      {
         /* First try to get a (real) pointer to the object in the list.*/
         Entry = LookupObjectByIndex(Store, i);

         /* Did we locate this index? If not go to next index. Gaps in  */
         /* the list by index are possible and expected.                */
         if(Entry != NULL)
         {
            /* Attempt to get actual item data contents and create hash.*/
            /* If the lookup fails or IO read fails this will set hash  */
            /* to zero (this is ok).                                    */
            newHash = GetItemHashValueByIndex(Store, i, &Result);

            /* Has this items hash value changed since last pass?       */
            /* We will also mark a delete if item isn't found.          */
            if(((newHash != Entry->Signature) && Result == SYNC_OBEX_RESPONSE_OK) || (Result == SYNC_OBEX_RESPONSE_NOT_FOUND))
            {
               /* Initialize Change Entry Object                        */
               InitChangeEntry(&changeEntry);

               /* Actual CC/TS value will be allocated by function.     */
               if(Result != SYNC_OBEX_RESPONSE_OK)
               {
                  /* We failed to read the object. Mark as deleted.     */
                  changeEntry.EntryType = cleHardDelete;

                  /* We should also remove this object from the store.  */

                  /* First, let's search the list until we find the     */
                  /* correct entry.  We must find the previous entry.   */
                  FoundEntry = (Store->ObjectEntryList);
                  prevEntry = NULL;

                  while((FoundEntry) && ((BTPS_StringLength(Entry->LUID) != BTPS_StringLength(FoundEntry->LUID)) || ((BTPS_MemCompare(Entry->LUID, FoundEntry->LUID, BTPS_StringLength(Entry->LUID))) != 0)))
                  {
                     prevEntry = FoundEntry;
                     FoundEntry = FoundEntry->NextObjectEntryPtr;
                  }

                  /* Did we locate entry?                               */
                  if(FoundEntry != NULL)
                  {
                     /* Now proceed to remove this item from the list   */
                     /* If prevEntry is NULL this item was the first on */
                     /* the list.                                       */
                     if(prevEntry == NULL)
                     {
                        /* Place next entry onto the list head.         */
                        (Store->ObjectEntryList) = FoundEntry->NextObjectEntryPtr;
                     }
                     else
                     {
                        /* remove entry and adjust list pointers.       */
                        prevEntry->NextObjectEntryPtr = FoundEntry->NextObjectEntryPtr;
                     }

                     /* Decrement the number of records in the store.   */
                     Store->InfoLog.TotalRecords--;
                  }
               }
               else
               {
                  /* Read was successful, so this is a modify change.   */
                  changeEntry.EntryType = cleModify;
               }

               /* Set LUID for the change log entry (Required).         */
               BTPS_StringCopy(changeEntry.LUID, Entry->LUID);

               /* Now add a change log entry for this operation.        */
               OBJSTORE_AddChangeEntry(Store, &changeEntry);

               /* Finally write new hash to the object so that we don't */
               /* mark it changed on the next pass.                     */
               Entry->Signature = newHash;
            }

            /* Entry points to actual list item.  Do NOT release here.  */
         }
      }

      /* Release acquired mutex                                         */
      BTPS_ReleaseMutex(Store->Mutex);
   }
}

