/*****< objstore.h >***********************************************************/
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
#ifndef __OBJSTOREH__
#define __OBJSTOREH__

#include "SS1BTSYN.h"      /* Includes/Constants for the OBEX SYNC Profile.   */

   /* The following defines are the valid return codes for API function */
   /* in this module.  Not all functions can return all codes.  Refer to*/
   /* the specific function header for information on supported return  */
   /* codes. Most HANDLER related functions return OBEX response codes  */
   /* that can be passed on to clients of the Sync Server service.      */
#define OBJSTORE_SUCCESS_RESULT                                         (0)
#define OBJSTORE_ERROR_INVALID_PARAMETER                                (-1000)
#define OBJSTORE_ERROR_NOT_INITIALIZED                                  (-1001)
#define OBJSTORE_ERROR_INSUFFICIENT_RESOURCES                           (-1002)

   /* The following BIT definitions are used to denote the individual   */
   /* Object Store Permissions that can be applied.                     */
   /* This store imposes the Read/Write/Create/Delete restrictions, but */
   /* the client determines Pb/Cal/Nt/Msg permissions by the stores     */
   /* created by calling RegisterStore().                               */
#define OBJSTORE_PERMISSION_MASK_READ                              0x00000001
#define OBJSTORE_PERMISSION_MASK_WRITE                             0x00000002
#define OBJSTORE_PERMISSION_MASK_CREATE                            0x00000004
#define OBJSTORE_PERMISSION_MASK_DELETE                            0x00000008
#define OBJSTORE_PERMISSION_MASK_PHONEBOOK                         0x00000010
#define OBJSTORE_PERMISSION_MASK_CALENDAR                          0x00000020
#define OBJSTORE_PERMISSION_MASK_NOTES                             0x00000040
#define OBJSTORE_PERMISSION_MASK_MESSAGES                          0x00000080

#define OBJSTORE_PERMISSION_MASK_ALL      (OBJSTORE_PERMISSION_MASK_READ | OBJSTORE_PERMISSION_MASK_WRITE | OBJSTORE_PERMISSION_MASK_CREATE | OBJSTORE_PERMISSION_MASK_DELETE | OBJSTORE_PERMISSION_MASK_PHONEBOOK | OBJSTORE_PERMISSION_MASK_CALENDAR | OBJSTORE_PERMISSION_MASK_NOTES | OBJSTORE_PERMISSION_MASK_MESSAGES)

   /* This is the number of bytes allocated per object store to hold    */
   /* implementation specific information.  The contents placed in this */
   /* buffer are passed in during store creation.  A pointer to this    */
   /* buffer is provided during each IO handler callback.  It can be    */
   /* used to store any implementation specific info, such as the path  */
   /* to the Database containing a given store or the file path in file */
   /* system based implementations.                                     */
#define OBJSTORE_STORE_DATA_SIZE                                        (275)

   /* This structure is used to construct basic name value pairs that   */
   /* make up the extensions portion of the device info log.  This      */
   /* allows any pair to be added and output by this module even if     */
   /* defined at a later date.                                          */
typedef struct _tagIrMC_Name_Value_Pair_t
{
   char *Name;
   char *Value;
} IrMC_Name_Value_Pair_t;

#define IRMC_NAME_VALUE_PAIR_SIZE                                       (sizeof(IrMC_Name_Value_Pair_t))

   /* The IrMC_Device_Information_Log_t structure is used to hold the   */
   /* values require to generate an IrMC Device Info containing data    */
   /* specific to this device.  This structure can also be used to store*/
   /* a parsed version of a device information log retreived from a     */
   /* remote device.  The underlying information is generally static and*/
   /* does not change for the life of a particular device.  Refer to the*/
   /* accompanying samples for examples of initializing this structure  */
   /* with static information.                                          */
typedef struct _tagIrMC_Device_Information_Log_t
{
   char                   *Manufacturer;
   char                   *Model;
   char                   *OEM;
   char                   *FirmwareVersion;
   char                   *FirmwareDate;
   char                   *SoftwareVersion;
   char                   *SoftwareDate;
   char                   *IrMCVersion;
   char                   *HardwareVersion;
   char                   *HardwareDate;
   char                   *SerialNumber;
   char                   *PhonebookType;
   char                   *CalendarType;
   char                   *MessageType;
   char                   *NoteType;
   char                   *InboxCapability;
   char                   *SentboxCapability;
   unsigned int            NumberExtensions;
   IrMC_Name_Value_Pair_t *ExtensionsArray;
} IrMC_Device_Information_Log_t;

#define IRMC_DEVICE_INFORMATION_LOG_SIZE                                (sizeof(IrMC_Device_Information_Log_t))

   /* The following constants are used with the InformationExchangeLevel*/
   /* member of the IrMC_Information_Log_t structure.                   */
#define IRMC_INFORMATION_EXCHANGE_LEVEL_1                               (0x01)
#define IRMC_INFORMATION_EXCHANGE_LEVEL_12                              (0x02)
#define IRMC_INFORMATION_EXCHANGE_LEVEL_123                             (0x04)
#define IRMC_INFORMATION_EXCHANGE_LEVEL_124                             (0x08)
#define IRMC_INFORMATION_EXCHANGE_LEVEL_1234                            (0x10)

   /* The following constants are used with the SyncAnchorType member of*/
   /* the IrMC_Information_Log_t structure.                             */
#define IRMC_SYNC_ANCHOR_TYPE_BIT_CC                                    (0x01)
#define IRMC_SYNC_ANCHOR_TYPE_BIT_TS                                    (0x02)

   /* The IrMC_Information_Log_t structure is used to hold the values   */
   /* required to generate an IrMC Information Log containing data      */
   /* specific to an individual object store.  These values can change  */
   /* during a given IrMC session.  Members are as follows:             */
   /*                                                                   */
   /* StoreType -                Standard enumerated value indicating   */
   /*                            the type of store this information log */
   /*                            represents.  This is used in outputting*/
   /*                            some fields which are only present in  */
   /*                            the Phonebook log type.                */
   /* TotalRecords -             Total number of records currently      */
   /*                            within the object store.  This value is*/
   /*                            initially set to zero and is managed   */
   /*                            internally by this module.             */
   /* LastUsedIndex -            Last Used Index for IrMC Level 3 access*/
   /*                            (e.g. PBAP).  This parameter is not    */
   /*                            used for IrMC Level 4 access (e.g.     */
   /*                            Sync).  This field is enabled or       */
   /*                            disabled with the UseLastUsedIndex     */
   /*                            Boolean.  This value is initially set  */
   /*                            to zero and is managed internally by   */
   /*                            this module.                           */
   /* UseLastUsedIndex -         Enables or disables the inclusion of   */
   /*                            the Last-Used-Index field in the log.  */
   /*                            This field is only required for IrMC   */
   /*                            Level 3 access devices (e.g. PBAP),    */
   /*                            not Level 4 (e.g. Sync).  Either       */
   /*                            option is supported by this            */
   /*                            implementation.                        */
   /* MaximumRecords -           Maximum number of records that can be  */
   /*                            stored in this IrMC object store.  If  */
   /*                            this value is set to zero (0) then the */
   /*                            record used will indicate that there is*/
   /*                            no maximum limit and the '*' character */
   /*                            will be placed in the information log. */
   /*                            This implemenation does not impose a   */
   /*                            limit but will report whatever limit is*/
   /*                            listed in this value.  This module does*/
   /*                            not modify this value.                 */
   /* InformationExchangeLevel - The Information Exchange Level         */
   /*                            identifies what IrMC levels are        */
   /*                            supported by this object store.  This  */
   /*                            field is based on a lookup table with  */
   /*                            values for each particular grouping of */
   /*                            supported IrMC levels.  A value of ZERO*/
   /*                            (0) will exclude this field from the   */
   /*                            information log.  This implementation  */
   /*                            supports requests for all IEL levels   */
   /*                            and doesn't currently restrict API     */
   /*                            access based on this field.  The       */
   /*                            InfoLog will output any value supported*/
   /*                            below.                                 */
   /* UseLevel4Format -          This boolean flag determines if this   */
   /*                            IrMC information log is for an IrMC    */
   /*                            Level 4 Object Store.  This flag       */
   /*                            determines the inclusion of the        */
   /*                            portions of the information log that   */
   /*                            are mandatory for Level 4 acess only.  */
   /*                            These fields include HardDelete,       */
   /*                            SyncAnchorType, SyncAnchorIncrement,   */
   /*                            SyncAnchorUnique, and DatabaseID.  This*/
   /*                            fields will be included if this flag is*/
   /*                            TRUE, otherwise they will be excluded  */
   /*                            if this flag is FALSE.  This           */
   /*                            implementation supports Level 4        */
   /*                            formats.  Setting this value to FALSE  */
   /*                            will not automatically restrict Level 4*/
   /*                            API calls.                             */
   /* HardDelete -               LEVEL 4 ONLY - Indicates if this device*/
   /*                            supports a distinction between hard and*/
   /*                            soft deletes.  If this flag is set to  */
   /*                            TRUE then the device is expected to    */
   /*                            distinguish between hard and soft      */
   /*                            delete operations.  This module passes */
   /*                            hard delete information to the IO      */
   /*                            handlers if available but does not     */
   /*                            require the distinction.               */
   /* SyncAnchorType -           LEVEL 4 ONLY - This value determines   */
   /*                            the type of sync anchors supported by  */
   /*                            this information store.  The values for*/
   /*                            this field are organized as a bit mask */
   /*                            with a single bit for each possible    */
   /*                            anchor type (TS or CC).  At least one  */
   /*                            bit must be set for this field to be   */
   /*                            valid.  The information log can output */
   /*                            any of the valid values below.  This   */
   /*                            object store implementation only fully */
   /*                            supports the Change Counter (CC) type, */
   /*                            as recommended by IrMC specifications. */
   /* SyncAnchorIncrement -      LEVEL 4 ONLY - This boolean flag       */
   /*                            indicates is the timestamp values are  */
   /*                            guaranteed to increment, i.e.  the     */
   /*                            reference clock can never be set to an */
   /*                            earlier time.  This field is only      */
   /*                            included if SyncAnchorType is set to   */
   /*                            Timestamp or Both (IRMC_SAT_BIT_TS).   */
   /*                            The information log can output either  */
   /*                            state but this implementation doesn't  */
   /*                            support timestamps internally.         */
   /* SyncAnchorUnique -         LEVEL 4 ONLY - This boolean flag       */
   /*                            indicates if the timestamp values are  */
   /*                            guaranteed to be unique.  This field is*/
   /*                            only included if SyncAnchorType is set */
   /*                            to Timestamp or Both (IRMC_SAT_BIT_TS).*/
   /*                            The information log can output either  */
   /*                            state but this implementation doesn't  */
   /*                            support timestamps internally.         */
   /* DatabaseID -               LEVEL 4 ONLY - This field identifies a */
   /*                            32-bit value that contains this object */
   /*                            store's Database ID.  This value is    */
   /*                            generated randomly on reset and must be*/
   /*                            regenerated in certain cases per the   */
   /*                            IrMC specification.  This value is     */
   /*                            written when the object store is       */
   /*                            created or at certain pre-determined   */
   /*                            times by specification (rollover of CC,*/
   /*                            etc).  The module internally manages   */
   /*                            this value.                            */
   /* IncomingCallLog -          This boolean flag indicates if this    */
   /*                            device supports the incoming call log  */
   /*                            for the Phone Book Object Store.  It is*/
   /*                            only present in the phone book         */
   /*                            information log.  This module does not */
   /*                            include any specific implementation of */
   /*                            these logs, but does support an        */
   /*                            extended store type intended to track  */
   /*                            these as standard object entries.      */
   /* OutgoingCallLog -          This boolean flag indicates if this    */
   /*                            device supports the outgoing call log  */
   /*                            for the Phone Book Object Store.  It is*/
   /*                            only present in the phone book         */
   /*                            information log.  This module does not */
   /*                            include any specific implementation of */
   /*                            these logs, but does support an        */
   /*                            extended store type intended to track  */
   /*                            these as standard object entries.      */
   /* MissedCallLog -            This boolean flag indicates if this    */
   /*                            device supports the missed call log for*/
   /*                            the Phone Book Object Store.  It is    */
   /*                            only present in the phone book         */
   /*                            information log.  This module does not */
   /*                            include any specific implementation of */
   /*                            these logs, but does support an        */
   /*                            extended store type intended to track  */
   /*                            these as standard object entries.      */
   /* MissedMessageHistoryLog -  This boolean flag indicates if this    */
   /*                            device supports the missed message log */
   /*                            for the Message Object Store.  It is   */
   /*                            only present in the Message information*/
   /*                            log.  This module does not include any */
   /*                            specific implementation of these logs, */
   /*                            but does support an extended store type*/
   /*                            intended to track these as standard    */
   /*                            object entries.                        */
   /* NumberFields -             This element contains the number of    */
   /*                            elements present in the FieldsArray    */
   /*                            element.  This is generally initialized*/
   /*                            based on this size of the array        */
   /*                            assigned to the FieldsArray pointer.   */
   /*                            Setting this value to zero will exclude*/
   /*                            the X-IRMC-FIELDS section from being   */
   /*                            added to the log.                      */
   /* FieldsArray -              This pointer should be set to the first*/
   /*                            element of an array of ASCII Null      */
   /*                            Terminated string for inclusion in the */
   /*                            X-IRMC-FIELDS sections.  Each element  */
   /*                            will represent a dynamic field mapping */
   /*                            entry in the information log.  This    */
   /*                            information is included or excluded    */
   /*                            based on the state of the              */
   /*                            UseDynamicFieldMappings element.       */
typedef struct _tagIrMC_Information_Log_t
{
   IrMC_Object_Store_Type_t   StoreType;
   DWord_t                    TotalRecords;
   DWord_t                    LastUsedIndex;
   Boolean_t                  UseLastUsedIndex;
   DWord_t                    MaximumRecords;
   Byte_t                     InformationExchangeLevel;
   Boolean_t                  UseLevel4Format;
   Boolean_t                  HardDelete;
   Byte_t                     SyncAnchorType;
   Boolean_t                  SyncAnchorIncrement;
   Boolean_t                  SyncAnchorUnique;
   DWord_t                    DatabaseID;
   Boolean_t                  IncomingCallLog;
   Boolean_t                  OutgoingCallLog;
   Boolean_t                  MissedCallLog;
   Boolean_t                  MissedMessageHistoryLog;
   unsigned int               NumberFields;
   char                     **FieldsArray;
} IrMC_Information_Log_t;

#define IRMC_INFORMATION_LOG_SIZE                                       (sizeof(IrMC_Information_Log_t))

   /* This structure represents a single Object Entry.  This structure  */
   /* is passed to IO handler functions below to implement IO           */
   /* operations.  It is also used internally to create lists of objects*/
   /* in memory.  API functions which return an Object Entry pointer    */
   /* always return a recently allocated copy of the entry.  This gives */
   /* the caller exclusive ownership of the entry data.  It is up to the*/
   /* caller of the API function to dispose of this entry by calling the*/
   /* OBJSTORE_ReleaseEntry() function as passing a pointer to this     */
   /* object.                                                           */
typedef struct _tagIrMC_Object_Entry_t
{
   DWord_t                         Index;
   char                            LUID[SYNC_MAX_LUID_CHARACTER_SIZE + 1];
   DWord_t                         Handle;
   char                            HandlerData[SYNC_MAX_LUID_CHARACTER_SIZE + 1];
   DWord_t                         Signature;
   IrMC_TimeDate_t                 Timestamp;
   Byte_t                         *Data;
   unsigned int                    DataLength;
   unsigned int                    BufferLength;
   struct _tagIrMC_Object_Entry_t *NextObjectEntryPtr;
} IrMC_Object_Entry_t;

#define IRMC_OBJECT_ENTRY_SIZE                                          (sizeof(IrMC_Object_Entry_t))

   /* The following type definitions is used internally by the Object   */
   /* Store module and should not be used directly by a client          */
   /* application.  It is included here because it is required in       */
   /* declaring storage for an object store.                            */
typedef enum
{
   cleModify,
   cleDelete,
   cleHardDelete
} IrMC_Change_Log_Entry_Type_t;

   /* The following type definitions is used internally by the Object   */
   /* Store module and should not be used directly by a client          */
   /* application.  It is included here because it is required in       */
   /* declaring storage for an object store.                            */
typedef struct _tagIrMc_Change_Log_Entry_t
{
   IrMC_Change_Log_Entry_Type_t        EntryType;
   DWord_t                             ChangeCounter;
   IrMC_TimeDate_t                     Timestamp;
   char                                LUID[SYNC_MAX_LUID_CHARACTER_SIZE + 1];
   struct _tagIrMC_Change_Log_Entry_t *NextChangeLogEntryPtr;
} IrMC_Change_Log_Entry_t;

#define IRMC_CHANGE_LOG_ENTRY_SIZE                                      (sizeof(IrMC_Change_Log_Entry_t))

   /* The following type definitions is used internally by the Object   */
   /* Store module and should not be used directly by a client          */
   /* application.  It is included here because it is required in       */
   /* declaring storage for an object store.                            */
typedef struct _tagIrMC_Change_Log_t
{
   char                    *SerialNumber;
   DWord_t                  NumberChangeLogEntries;
   IrMC_Change_Log_Entry_t *ChangeLogEntriesList;
   DWord_t                  OldestChangeCounter;
} IrMC_Change_Log_t;

#define IRMC_CHANGE_LOG_SIZE                                            (sizeof(IrMC_Change_Log_t))

   /* Forward declaration so that it can be used below (before it is    */
   /* declared).                                                        */
typedef struct _tagIrMC_Object_Store_t IrMC_Object_Store_t;

   /* The following function pointer definitions define the IO handler  */
   /* functions that should be implemented and registered with the      */
   /* object store at creation.  Each handler represents a specific     */
   /* action against a single underlying object tracked by the object   */
   /* store.                                                            */

   /* The following return values are allowed for each handler.         */
   /*    OBJSTORE_HANDLER_SUCCESS_RESULT - The operation was successful.*/
   /*    OBJSTORE_HANDLER_ERROR_UNAUTHORIZED - Operation not allowed.   */
   /*    OBJSTORE_HANDLER_ERROR_IOERROR - Operation failed internally.  */
   /*    OBJSTORE_HANDLER_ERROR_INSUFFICIENT_RESOURCES - Buffer         */
   /*      allocation failed.                                           */

   /* The Read handler is used when the object store needs to read the  */
   /* underlying contents of an object tracked by a given object store. */
   /* The call passes all the information required to access this       */
   /* object.  The Type parameter specifies the object store type being */
   /* read.  The Entry pointer references an Object Entry.  This entry  */
   /* contains the LUID/name of the object being read.  The             */
   /* implementation of this handler is expected to attempt to allocate */
   /* and populate a buffer referenced by Entry->Data and update the    */
   /* Entry->DataLength and Entry->BufferLength structure members if the*/
   /* read operation was successful.                                    */
typedef int (*OBJSTORE_Read_Handler_t)(IrMC_Object_Store_Type_t Type, IrMC_Object_Entry_t *Entry, char *StoreData);

   /* The Write handler is used when the object store needs to write to */
   /* the underlying contents of an object tracked by a given object    */
   /* store.  The call passes all information required to complete this */
   /* write.  The Type parameter specifies the object store type being  */
   /* written.  The Entry pointer references an Object Entry structure. */
   /* This entry contains the LUID/name of the object being written.    */
   /* The Buffer parameter is a pointer to a buffer that contains the   */
   /* data to be written.  The BufferSize paramter is the length of the */
   /* data referenced by the Buffer parameter.                          */
typedef int (*OBJSTORE_Write_Handler_t)(IrMC_Object_Store_Type_t Type, IrMC_Object_Entry_t *Entry, DWord_t BufferSize, const Byte_t *Buffer, char *StoreData);

   /* The Create handler is used when the object store needs to Create a*/
   /* new object being tracked by the specified store.  The Type        */
   /* parameter specifies the object store type of the object being     */
   /* created.  The Entry pointer references an Object Entry structure. */
   /* This entry contains the LUID/name of the object being created.    */
   /* The Buffer parameter is a pointer to a buffer that contains the   */
   /* data to be written in the new object.  The BufferSize paramter is */
   /* the length of the data referenced by the Buffer parameter.        */
typedef int (*OBJSTORE_Create_Handler_t)(IrMC_Object_Store_Type_t Type, IrMC_Object_Entry_t *Entry, DWord_t BufferSize, const Byte_t *Buffer, char *StoreData);

   /* The Delete handler is used when the object store needs to Delete  */
   /* an object being tracked by the specified store.  The Type         */
   /* parameter specifies the object store type of the object being     */
   /* deleted.  The Entry pointer references an Object Entry structure. */
   /* This entry contains the LUID/name of the object being deleted.    */
   /* The HardDelete flag determines if the requested operation is a    */
   /* Hard Delete as defined by the IrMC specification.                 */
typedef int (*OBJSTORE_Delete_Handler_t)(IrMC_Object_Store_Type_t Type, IrMC_Object_Entry_t *Entry, Boolean_t HardDelete, char *StoreData);

   /* The Populate handler is used when the object store is first       */
   /* created and gives the IO subsystem an opportunity to populate the */
   /* newly created object store.  This handler is called immediately   */
   /* after an object store has completed initialization.  The Store    */
   /* parameter is a pointer to the underlying store being populated    */
   /* (required for adding entries).  The StoreData parameter is a      */
   /* pointer to an implementation specific string specified at Store   */
   /* creation time.  Even if the populate handler does not create any  */
   /* entries it must return success for the store initialization to be */
   /* considered successful.  An error code should only be returned for */
   /* a fatal error that will prevent the store from operating.         */
typedef int (*OBJSTORE_Populate_Handler_t)(IrMC_Object_Store_t *Store, char *StoreData);

   /* This structure is used to group together the function pointers    */
   /* that make up a complete Object Store IO Interface.  This structure*/
   /* should be a statically allocated structure referencing each       */
   /* handler function.  It is passed to this module during object store*/
   /* creation.                                                         */
typedef struct _tagIrMC_Object_Store_IO_Interface_t
{
   OBJSTORE_Read_Handler_t     Read;
   OBJSTORE_Write_Handler_t    Write;
   OBJSTORE_Create_Handler_t   Create;
   OBJSTORE_Delete_Handler_t   Delete;
   OBJSTORE_Populate_Handler_t Populate;
} IrMC_Object_Store_IO_Interface_t;

#define IRMC_OBJECT_STORE_IO_INTERFACE_SIZE                             (sizeof(IrMC_Object_Store_IO_Interface_t))

   /* The Object Store structure is used as the basis for tracking the  */
   /* state and contents of an object store.  The storage for this      */
   /* structure is declared by the client application but the internal  */
   /* values are all managed by this module.  Client applications should*/
   /* use API functions to perform actions upon an object store rather  */
   /* than directly manipulating this structure.  The only exception is */
   /* the Active flag which can be safely read to determine if this     */
   /* store structure has been initialized and needs to be released by a*/
   /* call to OBJSTORE_ReleaseStore() prior to shutdown.                */
struct _tagIrMC_Object_Store_t
{
   Mutex_t                           Mutex;
   Boolean_t                         Active;
   char                              StoreData[OBJSTORE_STORE_DATA_SIZE];
   IrMC_Object_Store_Type_t          Type;
   IrMC_Information_Log_t            InfoLog;
   DWord_t                           ChangeCounter;
   DWord_t                           Index;
   IrMC_Change_Log_t                 ChangeLog;
   DWord_t                           PermissionMask;
   IrMC_Object_Entry_t              *ObjectEntryList;
   IrMC_Object_Store_IO_Interface_t  Functions;
   struct _tagIrMC_Object_Store_t   *NextObjectStorePtr;
} ;

#define IRMC_OBJECT_STORE_SIZE                                          (sizeof(IrMC_Object_Store_t))

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
int OBJSTORE_CreateStore(IrMC_Object_Store_t *Store, IrMC_Object_Store_Type_t Type, IrMC_Information_Log_t *InfoLog, DWord_t PermissionMask, char *SerialNumber, IrMC_Object_Store_IO_Interface_t *IOInterface, char *StoreData);

   /* This function is used to shutdown and cleanup a previously opened */
   /* Object Store created using the OBJSTORE_CreateStore() function.   */
   /* The only parameter to this function is the pointer to the Object  */
   /* Store structure that contains information for this store.  This   */
   /* function will clear and release all storage used for the Change   */
   /* Log and Object List.  It will also release any related buffers.   */
int OBJSTORE_ReleaseStore(IrMC_Object_Store_t *Store);

   /* This function is generally used in populating an Object Store.  It*/
   /* adds an Object Entry to the specified object store, but it does   */
   /* not generate a change log entry for the Add (aka Put).  The first */
   /* parameter is a pointer to the Object Store structure to which the */
   /* user wishes to add an object entry.  The second parameter is a    */
   /* pointer to the object entry to add.  The data contained within    */
   /* this Entry is copied into a newly allocated structure, so this    */
   /* entry copy can be released after this call returns.               */
int OBJSTORE_AddObjectEntry(IrMC_Object_Store_t *Store, IrMC_Object_Entry_t *Entry);

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
int OBJSTORE_AddChangeEntry(IrMC_Object_Store_t *Store, IrMC_Change_Log_Entry_t *ChangeEntry);

   /* This function clears the change log for the specified Object      */
   /* Store, deleting all log entries and reseting relevant counters.   */
   /* The only parameter to this function is a pointer to the Object    */
   /* Store structure that should have its Change Log cleared.  This    */
   /* function releases any previously allocated memory and resets the  */
   /* Change Counter.  The change log is ready for use again immediately*/
   /* after this call returns.                                          */
int OBJSTORE_ClearChangeLog(IrMC_Object_Store_t *Store);

   /* This function is used to properly cleanup and release memory for  */
   /* an Object Entry allocated by this module.  When receiving an      */
   /* Object Entry as the return value for a function call (e.g.        */
   /* OBJSTORE_ObjectGet()) the Object Entry is a newly allocated copy  */
   /* that is owned exclusively by the caller.  Because the memory used */
   /* for this copy was allocated by this module its necessary to use   */
   /* this function to release the copy when use is complete.  The only */
   /* parameter to this function is a pointer to the Object Entry to be */
   /* released.                                                         */
void OBJSTORE_ReleaseEntry(IrMC_Object_Entry_t *Entry);

   /* This function is used to delete all objects that have been added  */
   /* to the specified Object Store.  This is generally used during the */
   /* process of a stream operation that modifies the entire object     */
   /* store.  This function will release any previously allocated memory*/
   /* and reset all relevant counters, returning the total objects count*/
   /* to zero.  The only parameter for this function is a pointer to the*/
   /* Object Store to which the delete should occur.                    */
int OBJSTORE_DeleteAllObjects(IrMC_Object_Store_t *Store);

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
int OBJSTORE_GetObject(IrMC_Object_Store_t *Store, IrMC_Operation_t *Op, IrMC_Object_Entry_t **EntryPtr);

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
int OBJSTORE_PutObject(IrMC_Object_Store_t *Store, IrMC_Operation_t *Op, DWord_t BufferSize, Byte_t *Buffer, IrMC_Anchor_t *SyncAnchor);

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
int OBJSTORE_DeleteObject(IrMC_Object_Store_t *Store, IrMC_Operation_t *Op, IrMC_Anchor_t *SyncAnchor);

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
int OBJSTORE_GetObjectStream(IrMC_Object_Store_t *Store, char *Name, IrMC_Object_Entry_t **EntryPtr);

   /* This function is used to perform an IrMC Level 3 Get Operation.   */
   /* This is generally invoked by OBJSTORE_GetObject based on the IEL, */
   /* but can also be called directly.  This function generates a       */
   /* response Object Entry that contains a buffer with the contents of */
   /* the object at the specified index (if present).  The first        */
   /* parameter is a pointer to the Object Store structure on which to  */
   /* perform this operation.  The second parameter is the index within */
   /* this store of the object being requested.                         */
int OBJSTORE_GetObjectByIndex(IrMC_Object_Store_t *Store, DWord_t Index, IrMC_Object_Entry_t **EntryPtr);

   /* This function is used to perform an IrMC Level 4 Get Operation.   */
   /* This is generally invoked by OBJSTORE_GetObject based on the IEL, */
   /* but can also be called directly.  This function generates a       */
   /* response Object Entry that contains a buffer with the contents of */
   /* the object that has the specified LUID (if present).  The first   */
   /* parameter is a pointer to the Object Store structure on which to  */
   /* perform this operation.  The second parameter is an ASCII,        */
   /* Null-terminated string containing the LUID of the Object being    */
   /* requested.                                                        */
int OBJSTORE_GetObjectByLUID(IrMC_Object_Store_t *Store, char *LUID, IrMC_Object_Entry_t **EntryPtr);

   /* This function is used to perform an IrMC Level 2 Put Operation.   */
   /* This is generally invoked by OBJSTORE_PutObject based on the IEL, */
   /* but can also be called directly.  This function takes an incoming */
   /* buffer containing the data from a stream object and parses this   */
   /* data into multiple independent objects in the specified store.    */
   /* The first parameter is a pointer to the Object Store structure on */
   /* which to perform this operation.  The final two parameters are a  */
   /* pointer to the Buffer containing the Stream data, and a BufferSize*/
   /* that specifies the length of this data.                           */
int OBJSTORE_PutObjectStream(IrMC_Object_Store_t *Store, DWord_t BufferSize, Byte_t *Buffer);

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
int OBJSTORE_PutObjectByIndex(IrMC_Object_Store_t *Store, DWord_t Index, DWord_t BufferSize, Byte_t *Buffer);

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
int OBJSTORE_PutObjectByLUID(IrMC_Object_Store_t *Store, char *LUID, DWord_t BufferSize, Byte_t *Buffer, IrMC_Anchor_t *SyncAnchor, DWord_t *MaxChangeCounter);

   /* This function is used to perform an IrMC Level 3 Delete Operation.*/
   /* This is generally invoked by OBJSTORE_DeleteObject based on the   */
   /* IEL, but can also be called directly.  This function attempts to  */
   /* delete the specified object from the specified object store.  The */
   /* first parameter is a pointer to the Object Store structure on     */
   /* which to perform this operation.  The second parameter is the     */
   /* index within this store of the object being deleted.  The final   */
   /* parameter is a BOOLEAN flag this indicates if this is a 'Hard     */
   /* Delete' operation.                                                */
int OBJSTORE_DeleteObjectByIndex(IrMC_Object_Store_t *Store, DWord_t Index, Boolean_t HardDelete);

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
int OBJSTORE_DeleteObjectByLUID(IrMC_Object_Store_t *Store, char *LUID, IrMC_Anchor_t *SyncAnchor, Boolean_t HardDelete, DWord_t *MaxChangeCounter);

   /* The following function writes a Change Counter object into the    */
   /* provided buffer pointer to by the Buffer parameter.  The          */
   /* BufferSize parameter indicates the maximum number of bytes that   */
   /* can be safely written to the Buffer provided.  The current Change */
   /* Counter value is taken from the Object Store referenced by the    */
   /* Store parameter.                                                  */
int OBJSTORE_WriteChangeCounterToBuffer(IrMC_Object_Store_t *Store, DWord_t BufferSize, Byte_t *Buffer);

   /* The following function writes a passed device information log into*/
   /* the buffer pointed to by the Buffer parameter.  The BufferSize    */
   /* parameter indicates the maximum number of bytes that can be safely*/
   /* written to the Buffer provided.  The device information log is    */
   /* output in 7-bit ASCII format, including embedded CRLF characters  */
   /* to represent multiline data.  The buffer will be terminated with a*/
   /* NULL character.  If the entire device information log can't be    */
   /* written within BufferSize characters, the return value will       */
   /* indicate an error.                                                */
int OBJSTORE_WriteDevInfoToBuffer(const IrMC_Device_Information_Log_t *DevInfo, DWord_t BufferSize, Byte_t *Buffer);

   /* The following function writes a passed information log into the   */
   /* buffer pointed to by the Buffer parameter.  The BufferSize        */
   /* parameter indicates the maximum number of bytes that can be safely*/
   /* written to the Buffer provided.  The information log is output in */
   /* 7-bit ASCII format, including embedded CRLF characters to         */
   /* represent multiline data.  The buffer will be terminated with a   */
   /* NULL character.  If the entire information log can't be written   */
   /* within BufferSize characters, the return value will indicate an   */
   /* error.                                                            */
int OBJSTORE_WriteInfoLogToBuffer(IrMC_Object_Store_t *Store, DWord_t BufferSize, Byte_t *Buffer);

   /* The following function writes the current change log into the     */
   /* buffer pointed to by the Buffer parameter.  The BufferSize        */
   /* parameter indicates the maximum number of bytes that can be safely*/
   /* written to the Buffer provided.  The change log is taken from the */
   /* Object Store referenced by the Store parameter.  The SyncAnchor   */
   /* parameter is a pointer to a IrMC_Anchor_t structure that contains */
   /* Anchor related data for this request, generally a Change Counter  */
   /* value.  This function does not fully support outputting Timestamp */
   /* format changelogs.The output is in 7-bit ASCII format, including  */
   /* embedded CRLF characters to represent multiline data.  The buffer */
   /* will be terminated with a NULL character.  If the entire change   */
   /* log can't be written within BufferSize characters, the return     */
   /* value will indicate an error.                                     */
int OBJSTORE_WriteChangeLogToBuffer(IrMC_Object_Store_t *Store, IrMC_Anchor_t *SyncAnchor, DWord_t BufferSize, Byte_t *Buffer);

   /* The following function iterates through the specied object store  */
   /* using the populate interface and determine if any of the objects  */
   /* have been modified or if any items have been added or deleted.    */
   /* The Store parameter is a pointer to the store to check for        */
   /* changes.                                                          */
void OBJSTORE_RegisterChangesByHash(IrMC_Object_Store_t *Store);

#endif
