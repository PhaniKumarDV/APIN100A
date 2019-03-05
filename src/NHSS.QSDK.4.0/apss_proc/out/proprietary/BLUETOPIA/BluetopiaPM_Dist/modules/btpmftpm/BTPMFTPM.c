/*****< btpmftpm.c >***********************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMFTPM - Bluetooth Stack OBEX File Transfer Profile Implementation for  */
/*             Stonestreet One Bluetooth Protocol Stack Platform Manager.     */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   04/03/13  T. Cook        Initial creation.                               */
/******************************************************************************/
#include "SS1BTFTPM.h"           /* Bluetooth FTP API Prototypes/Constants.   */
#include "FTPM.h"                /* Bluetooth FTPM Prototypes/Constants.      */

#include "SS1BTPS.h"             /* Bluetooth Stack API Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */
#include "BTPSFILE.h"            /* Bluetooth File I/O Abstraction Layer      */

#include "SS1BTPM.h"             /* BTPM Main Prototypes and Constants.       */
#include "BTPMERR.h"             /* BTPM Error Prototypes/Constants.          */
#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */
#include "BTPMMODC.h"            /* BTPM MODC Prototypes/Constants.           */

#define OTP_DEFAULT_PACKET_LENGTH   (BTPS_CONFIGURATION_FTP_MAXIMUM_OBEX_PACKET_LENGTH)
                                                /* Default Maximum Packet     */
                                                /* Length to use when required*/
                                                /* to specify for the FTPM Mgr*/
                                                /* Transport Layer.           */

   /* SDP Profile UUID's for the OBEX File Transfer Profile.            */

   /* The following MACRO is a utility MACRO that assigns the File      */
   /* Transfer Profile Bluetooth Universally Unique Identifier          */
   /* (FILE_TRANSFER_PROFILE_UUID_16) to the specified UUID_16_t        */
   /* variable.  This MACRO accepts one parameter which is the UUID_16_t*/
   /* variable that is to receive the FILE_TRANSFER_PROFILE_UUID_16     */
   /* Constant value.                                                   */
#define SDP_ASSIGN_FILE_TRANSFER_PROFILE_UUID_16(_x)            ASSIGN_SDP_UUID_16((_x), 0x11, 0x06)

   /* The following MACRO is a utility MACRO that assigns the File      */
   /* Transfer Profile Bluetooth Universally Unique Identifier          */
   /* (FILE_TRANSFER_PROFILE_UUID_32) to the specified UUID_32_t        */
   /* variable.  This MACRO accepts one parameter which is the UUID_32_t*/
   /* variable that is to receive the FILE_TRANSFER_PROFILE_UUID_32     */
   /* Constant value.                                                   */
#define SDP_ASSIGN_FILE_TRANSFER_PROFILE_UUID_32(_x)            ASSIGN_SDP_UUID_32((_x), 0x00, 0x00, 0x11, 0x06)

   /* The following MACRO is a utility MACRO that assigns the File      */
   /* Transfer Profile Bluetooth Universally Unique Identifier          */
   /* (FILE_TRANSFER_PROFILE_UUID_128) to the specified UUID_128_t      */
   /* variable.  This MACRO accepts one parameter which is the          */
   /* UUID_128_t variable that is to receive the                        */
   /* FILE_TRANSFER_PROFILE_UUID_128 Constant value.                    */
#define SDP_ASSIGN_FILE_TRANSFER_PROFILE_UUID_128(_x)           ASSIGN_SDP_UUID_128((_x), 0x00, 0x00, 0x11, 0x06, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB)

   /* Defines the Profile Version Number used within the SDP Record for */
   /* File Transfer Profile Servers.                                    */
#define FTPM_PROFILE_VERSION                                    (0x0100)

   /* The following are the return values from the SetCurrentPath()     */
   /* function.                                                         */
#define SET_PATH_ERROR                          (-1)
#define SET_PATH_ERROR_INVALID_PATH             (-2)
#define SET_PATH_ROOT                            (0)
#define SET_PATH_SUBDIRECTORY                    (1)

   /* The following are the return values from the                      */
   /* DeleteDirectoryEntry() function.                                  */
#define DELETE_ERROR                            (-1)
#define DELETE_ERROR_SUBDIRECTORY_NOT_EMPTY     (-2)
#define DELETE_FILE                              (0)
#define DELETE_SUBDIRECTORY                      (1)

   /* The following are the return values from the GetFileData() and    */
   /* PutFileData() functions.                                          */
#define FILE_DATA_FILE_IO_ERROR                 (-1)
#define FILE_DATA_FILE_IO_SUCCESS                (0)
#define FILE_DATA_MORE_TO_COME                   (0)
#define FILE_DATA_END_OF_FILE                    (1)

   /* The following enumerated type represents the supported Connection */
   /* State types for a FTP Profile Connection.                         */
typedef enum
{
   csNotConnected,
   csConnectionPending,
   csConnected
} ConnectionState_t;

   /* The following enumerated type represents the current state of a   */
   /* multiple part FTPM Mgr Operation that the FTP Server is operating */
   /* in.                                                               */
typedef enum
{
   coNone,
   coDirectoryRequest,
   coChangeDirectory,
   coCreateDirectory,
   coDeleteDirectory,
   coCreateFile,
   coDeleteFile,
   coFileGet,
   coFilePut
} CurrentOperation_t;

   /* File Information Context.  This structure contains the current    */
   /* transfer status information of a File Transfer (either Put or a   */
   /* Get operation).                                                   */
typedef struct _tagFileInfo_t
{
   Boolean_t              FirstPhase;
   unsigned int           FileSize;
   unsigned int           FileIndex;
   BTPS_File_Descriptor_t FileDescriptor;
} FileInfo_t;

   /* File Transfer Profile Context Information Block.  This structure  */
   /* contains All information associated with a specific FTP ID (member*/
   /* is present in this structure).                                    */
typedef struct _tagFTPInfo_t
{
   unsigned int          FTPConnectionID;
   unsigned int          OTPID;
   BD_ADDR_t             BD_ADDR;
   Boolean_t             DeleteEntry;
   Boolean_t             FTPServer;
   ConnectionState_t     ConnectionState;
   unsigned long         ServerPermissions;
   unsigned int          DirectoryDepth;
   Boolean_t             RootIsDrive;
   char                  RootDirectory[BTPS_MAXIMUM_FILE_NAME_LENGTH+1];
   char                  CurrentDirectory[BTPS_MAXIMUM_FILE_NAME_LENGTH+1];
   char                  CurrentObject[BTPS_MAXIMUM_FILE_NAME_LENGTH+1];
   Byte_t               *PutFileBuffer;
   CurrentOperation_t    CurrentOperation;
   Boolean_t             AbortActive;
   Boolean_t             AbortQueued;
   FileInfo_t            FileInfo;
   Boolean_t             CallbackActive;
   void                 *CallbackFunction;
   void                 *CallbackParameter;
   struct _tagFTPInfo_t *NextFTPInfoPtr;
} FTPInfo_t;

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the         */
   /* compiler as part of standard C/C++).                              */

   /* Variable which holds the First Entry (Head of List) of All        */
   /* currently opened FTP Profile instances.                           */
static FTPInfo_t *FTPInfoList;

   /* Variable which serves as a global flag about whether this module  */
   /* is initialized or not.                                            */
static Boolean_t Initialized;

   /* The following holds the next (unique) FTP ID.                     */
static unsigned int NextFTPConnectionID;

   /* Variable which holds the current Power State of the Device.       */
static Boolean_t CurrentPowerState;

   /* Internal Function Prototypes.                                     */
static unsigned int GetNextFTPConnectionID(void);

static FTPInfo_t *AddFTPEntry(FTPInfo_t **ListHead, FTPInfo_t *EntryToAdd);
static FTPInfo_t *SearchFTPEntry(FTPInfo_t **ListHead, unsigned int FTPConnectionID);
static FTPInfo_t *DeleteFTPEntry(FTPInfo_t **ListHead, unsigned int FTPConnectionID);
static void FreeFTPEntryMemory(FTPInfo_t *EntryToFree);
static void FreeFTPList(FTPInfo_t **ListHead);

static int SetCurrentPath(FTPInfo_t *FTPInfo, Boolean_t Backup, Boolean_t Create, char *DirectoryName);
static int DeleteDirectoryEntry(FTPInfo_t *FTPInfo, char *FileFolderName);
static int GetCurrentDirectoryInformation(FTPInfo_t *FTPInfo, OTP_DirectoryInfo_t *DirInfoPtr, char *FolderName);
static int GetFileData(FTPInfo_t *FTPInfo, DWord_t *BufferSize, unsigned char *BufferPtr, char *FileName);
static int PutFileData(FTPInfo_t *FTPInfo, Boolean_t CloseFile, DWord_t BufferSize, unsigned char *BufferPtr, char *FileName);

static char *ExtractNameFromObjectInfo(OTP_ObjectInfo_t *ObjectInfoPtr);
static unsigned int ExtractNameLengthFromObjectInfo(OTP_ObjectInfo_t *ObjectInfoPtr);

static void BTPSAPI OTP_Event_Callback(OTP_Event_Data_t *OTP_Event_Data, unsigned long CallbackParameter);

   /* The following function is responsible for returning the Next      */
   /* Available FTP Profile ID.  This function will only return FTP     */
   /* Profile ID's that will be interpreted as Positive Integers (i.e.  */
   /* the Most Significant Bit will NEVER be set).                      */
static unsigned int GetNextFTPConnectionID(void)
{
   /* Increment the Counter to the next number.  Check the new number to*/
   /* see if it has gone negative (when ID is viewed as a signed        */
   /* integer).  If so, return to the first valid Number (one).         */
   NextFTPConnectionID++;

   if(((int)NextFTPConnectionID) < 0)
      NextFTPConnectionID = 1;

   /* Simply return the FTP ID Number to the caller.                    */
   return(NextFTPConnectionID);
}

   /* The following function adds the specified Entry to the specified  */
   /* List.  This function adds the specified entry to the list (the    */
   /* actual entry itself).  This function will return NULL if NO Entry */
   /* was added.  This can occur if the element passed in was deemed    */
   /* invalid or the actual List Head was invalid.                      */
   /* ** NOTE ** This function does not insert duplicate entries into   */
   /*            the list.  An element is considered a duplicate if the */
   /*            FTP Profile ID field is the same as an entry already   */
   /*            the list.  When this occurs, this function returns in  */
   /*            FALSE.                                                 */
static FTPInfo_t *AddFTPEntry(FTPInfo_t **ListHead, FTPInfo_t *EntryToAdd)
{
   FTPInfo_t *AddedEntry = NULL;
   FTPInfo_t *tmpEntry;

   /* First let's verify that values passed in are semi-valid.          */
   if((ListHead) && (EntryToAdd))
   {
      /* Make sure that the element that we are adding seems semi-valid.*/
      if(EntryToAdd->FTPConnectionID)
      {
         /* OK, data seems semi-valid, let's allocate a new data        */
         /* structure to add to the list.                               */
         AddedEntry = (FTPInfo_t *)BTPS_AllocateMemory(sizeof(FTPInfo_t));

         if(AddedEntry)
         {
            /* Copy All Data over.                                      */
            *AddedEntry                  = *EntryToAdd;

            /* Now Add it to the end of the list.                       */
            AddedEntry->NextFTPInfoPtr = NULL;

            /* First, let's check to see if there are any elements      */
            /* already present in the List that was passed in.          */
            if((tmpEntry = *ListHead) != NULL)
            {
               /* Head Pointer was not NULL, so we will traverse the    */
               /* list until we reach the last element.                 */
               while(tmpEntry)
               {
                  if(tmpEntry->FTPConnectionID == AddedEntry->FTPConnectionID)
                  {
                     /* Entry was already added, so free the memory and */
                     /* flag an error to the caller.                    */
                     FreeFTPEntryMemory(AddedEntry);
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
                     if(tmpEntry->NextFTPInfoPtr)
                        tmpEntry = tmpEntry->NextFTPInfoPtr;
                     else
                        break;
                  }
               }

               if(AddedEntry)
               {
                  /* Last element found, simply Add the entry.          */
                  tmpEntry->NextFTPInfoPtr = AddedEntry;
               }
            }
            else
               *ListHead = AddedEntry;
         }
      }
   }

   return(AddedEntry);
}

   /* The following function searches the specified List for the        */
   /* specified FTP Profile ID.  This function returns NULL if either   */
   /* the List Head is invalid, the FTP Profile ID is invalid, or the   */
   /* specified FTP Profile ID was NOT found.                           */
static FTPInfo_t *SearchFTPEntry(FTPInfo_t **ListHead, unsigned int FTPConnectionID)
{
   FTPInfo_t *FoundEntry = NULL;

   /* Let's make sure the list and FTP Profile ID to search for appear  */
   /* to be valid.                                                      */
   if((ListHead) && (FTPConnectionID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->FTPConnectionID != FTPConnectionID))
         FoundEntry = FoundEntry->NextFTPInfoPtr;
   }

   return(FoundEntry);
}

   /* The following function searches the specified File Transfer       */
   /* Profile List for the specified FTP Profile ID and removes it from */
   /* the List.  This function returns NULL if either the FTP List Head */
   /* is invalid, the FTP Profile ID is invalid, or the specified FTP   */
   /* Entry was NOT present in the list.  The entry returned will have  */
   /* the Next Entry field set to NULL, and the caller is responsible   */
   /* for deleting the memory associated with this entry by calling the */
   /* FreeFTPEntryMemory() function.                                    */
static FTPInfo_t *DeleteFTPEntry(FTPInfo_t **ListHead, unsigned int FTPConnectionID)
{
   FTPInfo_t *FoundEntry = NULL;
   FTPInfo_t *LastEntry  = NULL;

   /* Let's make sure the List and File Transfer Profile ID to search   */
   /* for appear to be semi-valid.                                      */
   if((ListHead) && (FTPConnectionID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->FTPConnectionID != FTPConnectionID))
      {
         LastEntry  = FoundEntry;
         FoundEntry = FoundEntry->NextFTPInfoPtr;
      }

      /* Check to see if we found the specified entry.                  */
      if(FoundEntry)
      {
         /* OK, now let's remove the entry from the list.  We have to   */
         /* check to see if the entry was the first entry in the list.  */
         if(LastEntry)
         {
            /* Entry was NOT the first entry in the list.               */
            LastEntry->NextFTPInfoPtr = FoundEntry->NextFTPInfoPtr;
         }
         else
            *ListHead = FoundEntry->NextFTPInfoPtr;

         FoundEntry->NextFTPInfoPtr = NULL;
      }
   }

   return(FoundEntry);
}

   /* This function frees the specified FTP Information member.  No     */
   /* check is done on this entry other than making sure it is NOT NULL.*/
static void FreeFTPEntryMemory(FTPInfo_t *EntryToFree)
{
   if(EntryToFree)
      BTPS_FreeMemory(EntryToFree);
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified FTP Information List.  Upon return of    */
   /* this function, the Head Pointer is set to NULL.                   */
static void FreeFTPList(FTPInfo_t **ListHead)
{
   FTPInfo_t *EntryToFree;
   FTPInfo_t *tmpEntry;

   if(ListHead)
   {
      /* Simply traverse the list and free every element present.       */
      EntryToFree = *ListHead;

      while(EntryToFree)
      {
         tmpEntry    = EntryToFree;
         EntryToFree = EntryToFree->NextFTPInfoPtr;

         /* Close any active connections.                               */
         if(tmpEntry->OTPID)
            _FTPM_Close_Port(tmpEntry->OTPID);

         /* Free any memory that was allocated.                         */
         if(tmpEntry->PutFileBuffer)
            BTPS_FreeMemory(tmpEntry->PutFileBuffer);

         /* Close any open File.                                        */
         if(tmpEntry->FileInfo.FileDescriptor)
            BTPS_Close_File(tmpEntry->FileInfo.FileDescriptor);

         FreeFTPEntryMemory(tmpEntry);
      }

      /* Make sure the List appears to be empty.                        */
      *ListHead = NULL;
   }
}

   /* The following function is responsible for building the current    */
   /* Directory Path for the specified FTP Information Server (first    */
   /* parameter).  The second parameter has precedence over ALL other   */
   /* parameters, and when specified, attempts to go 'up' a directory   */
   /* level (from the current directory to the current directories      */
   /* Parent Directory).  The Last two parameters, specify whether or   */
   /* not the caller is requesting a Directory Request Change (or to    */
   /* Create a new directory).  When the Create Flag is TRUE, the final */
   /* parameter specifies the Directory Name to create.  When the Create*/
   /* flag is FALSE, the final parameter specifies the Directory Name of*/
   /* the directory to change into.  If the final parameter is NULL,    */
   /* then the Root Directory is initialized as the current directory.  */
   /* This function returns a negative value if there was an error, or  */
   /* a positive value if successful.  The returned constants are:      */
   /*    - SET_PATH_ERROR         - Error.                              */
   /*    - SET_PATH_INVALID_PATH  - Error, Invalid Path.                */
   /*    - SET_PATH_ROOT          - Success, Root Directory.            */
   /*    - SET_PATH_SUBDIRECTORY  - Success, Sub Directory (Not Root).  */
   /* * NOTE * This function manipulates the following members of the   */
   /*          FTPInfo parameter:                                       */
   /*             - RootDirectory                                       */
   /*             - CurrentDirectory                                    */
   /*             - DirectoryDepth                                      */
static int SetCurrentPath(FTPInfo_t *FTPInfo, Boolean_t Backup, Boolean_t Create, char *DirectoryName)
{
   int                         ret_val;
   unsigned int                CurrentLength;
   BTPS_Directory_Descriptor_t DirectoryDescriptor;

   /* Before going any further, let's make sure that the information    */
   /* passed to us sems semi-valid.                                     */
   if(FTPInfo)
   {
      /* If the caller has specified that we need to back up a directory*/
      /* Level, then we need to process the Backup Request.             */
      if(Backup)
      {
         /* Before proceding any further, let's make sure that we are   */
         /* not at the root. When we are at the Root Level the Directory*/
         /* Depth is zero.                                              */
         if(FTPInfo->DirectoryDepth)
         {
            /* We were not at the Root Directory, so flag that we are   */
            /* backing up one level, then we will keep removing         */
            /* characters (from the Current Directory) until we reach a */
            /* directory delimiter character.                           */
            FTPInfo->DirectoryDepth--;

            /* Because we always leave the Directory Path with a        */
            /* trailing delimiter we need to remove the trailing        */
            /* delimiter so that we can search for the next one.        */
            FTPInfo->CurrentDirectory[BTPS_StringLength(FTPInfo->CurrentDirectory) - 1] = '\0';

            /* Keep shortening the Current Directory String until we    */
            /* reach a trailing delimiter character.                    */
            while(FTPInfo->CurrentDirectory[BTPS_StringLength(FTPInfo->CurrentDirectory) - 1] != BTPS_DIRECTORY_DELIMITER_CHARACTER)
               FTPInfo->CurrentDirectory[BTPS_StringLength(FTPInfo->CurrentDirectory) - 1] = '\0';

            /* Return success to the caller.                            */
            if(FTPInfo->DirectoryDepth)
               ret_val = SET_PATH_SUBDIRECTORY;
            else
               ret_val = SET_PATH_ROOT;
         }
         else
            ret_val = SET_PATH_ERROR;
      }
      else
      {
         /* Backup Flag was not specified, so let's check to see if a   */
         /* change directory request was requested (Directory Name will */
         /* be valid).                                                  */
         if((DirectoryName) && (DirectoryName[0]))
         {
            /* Before proceding any further let's make sure that there  */
            /* is no security breach (possible) by the user specifying  */
            /* any directory delimiter characters.                      */
            CurrentLength = 0;
            while(DirectoryName[CurrentLength])
            {
               if(DirectoryName[CurrentLength] == BTPS_DIRECTORY_DELIMITER_CHARACTER)
                  break;
               else
                  CurrentLength++;
            }

            /* If we went through every character in the DirectoryName  */
            /* parameter, then we did not find any invalid characters,  */
            /* so we will assume there is no security problem.          */
            if(CurrentLength == BTPS_StringLength(DirectoryName))
            {
               /* Next, let's note the current length of the Current    */
               /* Directory string.                                     */
               CurrentLength = BTPS_StringLength(FTPInfo->CurrentDirectory);

               /* OK, let's add the specified Directory to the current  */
               /* path.                                                 */
               if((CurrentLength + BTPS_StringLength(DirectoryName)) < sizeof(FTPInfo->CurrentDirectory))
               {
                  /* Build the new current directory.                   */
                  BTPS_SprintF(&(FTPInfo->CurrentDirectory[BTPS_StringLength(FTPInfo->CurrentDirectory)]), "%s", DirectoryName);

                  /* If the Create Flag was specified, simply try to    */
                  /* Create the Directory.                              */
                  if(Create)
                  {
                     ret_val = BTPS_Make_Directory(FTPInfo->CurrentDirectory);

                     /* If the Creation of the Directory failed, we need*/
                     /* to check if it failed because of the Directory  */
                     /* already existing.  If the Directory did already */
                     /* exist, then we need flag the Creation as        */
                     /* successful.                                     */
                     /* * NOTE * We are doing this because OBEX claims  */
                     /*          that most implementations behave in    */
                     /*          this manner.                           */
                     if((ret_val == BTPS_FILE_MAKE_DIRECTORY_SUCCESS) || (ret_val == BTPS_FILE_MAKE_DIRECTORY_ERROR_ALREADY_EXISTS))
                     {
                        ret_val = SET_PATH_SUBDIRECTORY;
                     }
                     else
                     {
                        ret_val = SET_PATH_ERROR;
                     }
                  }
                  else
                  {
                     /* Check to see if the directory specified actually*/
                     /* exists.                                         */
                     if((DirectoryDescriptor = BTPS_Open_Directory(FTPInfo->CurrentDirectory)) != NULL)
                     {
                        /* Directory Exists.  We are completely finished*/
                        /* with the Directory so let's simply close it  */
                        /* and be done with it.                         */
                        BTPS_Close_Directory(DirectoryDescriptor);

                        /* Flag that we have successfully changed the   */
                        /* current directory.                           */
                        ret_val = SET_PATH_SUBDIRECTORY;
                     }
                     else
                        ret_val = SET_PATH_ERROR_INVALID_PATH;
                  }

                  /* If the Current Directory has been updated (either a*/
                  /* Directory was created OR we have went into a       */
                  /* Sub-Directory) then we need to update the Directory*/
                  /* Level (increment it) and make sure that the new    */
                  /* path has a trailing directory delimieter character */
                  /* on it.                                             */
                  if((ret_val != SET_PATH_ERROR) && (ret_val != SET_PATH_ERROR_INVALID_PATH))
                  {
                     FTPInfo->DirectoryDepth++;

                     /* Make sure the Current Directory has a trailing  */
                     /* delimiter.                                      */
                     if(FTPInfo->CurrentDirectory[BTPS_StringLength(FTPInfo->CurrentDirectory) - 1] != BTPS_DIRECTORY_DELIMITER_CHARACTER)
                     {
                        FTPInfo->CurrentDirectory[BTPS_StringLength(FTPInfo->CurrentDirectory) + 1] = '\0';
                        FTPInfo->CurrentDirectory[BTPS_StringLength(FTPInfo->CurrentDirectory)]     = BTPS_DIRECTORY_DELIMITER_CHARACTER;
                     }
                  }
                  else
                  {
                     /* Error, so let's return the Current Directory    */
                     /* buffer back to what it was.                     */
                     FTPInfo->CurrentDirectory[CurrentLength] = '\0';
                  }
               }
               else
                  ret_val = SET_PATH_ERROR;
            }
            else
               ret_val = SET_PATH_ERROR;
         }
         else
         {
            /* Request to set the Root Directory.                       */
            FTPInfo->DirectoryDepth = 0;

            BTPS_StringCopy(FTPInfo->CurrentDirectory, FTPInfo->RootDirectory);

            ret_val = SET_PATH_ROOT;
         }
      }
   }
   else
      ret_val = SET_PATH_ERROR;

   return(ret_val);
}

   /* The following function is responsible for deleting the specified  */
   /* File (or Directory) Name from the File Server.  This function     */
   /* accepts as it's input parameters, the FTP Information of the FTP  */
   /* Server and the File (or Directory) Name to delete.  This function */
   /* first tries to delete a File of the specified name from the       */
   /* Current Directory.  If this fails, then this function attempts to */
   /* delete a Directory of the specified name.  This function returns  */
   /* a negative return error code if unsuccessful, or a positive return*/
   /* code if the File (or Directory) specified was deleted.  The return*/
   /* constants are as follows:                                         */
   /*    - DELETE_ERROR                        - Error.  Unknown.       */
   /*    - DELETE_ERROR_SUBDIRECTORY_NOT_EMPTY - Error.  Subdirectory   */
   /*                                            not empty.             */
   /*    - DELETE_FILE                         - Success.  File         */
   /*                                            Deleted.               */
   /*    - DELETE_SUBDIRECTORY                 - Success.  Directory    */
   /*                                            Deleted.               */
static int DeleteDirectoryEntry(FTPInfo_t *FTPInfo, char *FileFolderName)
{
   int                         ret_val;
   char                        NextName[BTPS_MAXIMUM_FILE_NAME_LENGTH];
   unsigned int                NumEntries;
   unsigned int                CurrentLength;
   BTPS_Directory_Descriptor_t DirectoryDescriptor;

   /* Before proceding any further, make sure that we have a semi-valid */
   /* parameters passed to us.                                          */
   if((FTPInfo) && (FileFolderName))
   {
      /* Before proceding any further let's make sure that there is no  */
      /* security breach (possible) by the user specifying any directory*/
      /* delimiter characters.                                          */
      CurrentLength = 0;
      while(FileFolderName[CurrentLength])
      {
         if(FileFolderName[CurrentLength] == BTPS_DIRECTORY_DELIMITER_CHARACTER)
            break;
         else
            CurrentLength++;
      }

      /* If we went through every character in the FileFolderName       */
      /* parameter, then we did not find any invalid characters, so we  */
      /* will assume there is no security problem.                      */
      if(CurrentLength == BTPS_StringLength(FileFolderName))
      {
         /* Next, let's note the current length of the Current Directory*/
         /* string.                                                     */
         CurrentLength = BTPS_StringLength(FTPInfo->CurrentDirectory);

         /* Now check to see if we have enough room in our current      */
         /* directory buffer to build the entire name.                  */
         if((CurrentLength + BTPS_StringLength(FileFolderName)) < sizeof(FTPInfo->CurrentDirectory))
         {
            /* Now build a fully qualified path name.                   */
            BTPS_SprintF(&(FTPInfo->CurrentDirectory[BTPS_StringLength(FTPInfo->CurrentDirectory)]), "%s", FileFolderName);

            if(BTPS_Delete_File(FTPInfo->CurrentDirectory))
               ret_val = DELETE_FILE;
            else
            {
               if(BTPS_Delete_Directory(FTPInfo->CurrentDirectory))
                  ret_val = DELETE_SUBDIRECTORY;
               else
               {
                  /* OK, we were not able to delete the Directory, we   */
                  /* need to check to see if we were unable to delete   */
                  /* the directory due to the directory not being empty.*/
                  if(BTPS_StringLength(FTPInfo->CurrentDirectory) < sizeof(FTPInfo->CurrentDirectory))
                  {
                     if((DirectoryDescriptor = BTPS_Open_Directory(FTPInfo->CurrentDirectory)) != NULL)
                     {
                        NumEntries  = 0;
                        NextName[0] = '\0';

                        /* At least the Path is valid, so let's count   */
                        /* all entries.                                 */
                        while(BTPS_Get_Next_Directory_Entry(DirectoryDescriptor, NextName))
                        {
                           if((!((BTPS_StringLength(NextName) == BTPS_StringLength(".")) && (!BTPS_MemCompare(NextName, ".", BTPS_StringLength("."))))) && (!((BTPS_StringLength(NextName) == BTPS_StringLength("..")) && (BTPS_MemCompare(NextName, "..", BTPS_StringLength(".."))))))
                              NumEntries++;
                        }

                        if(NumEntries)
                           ret_val = DELETE_ERROR_SUBDIRECTORY_NOT_EMPTY;
                        else
                           ret_val = DELETE_ERROR;

                        /* All finished, so close the Directory         */
                        /* Descriptor.                                  */
                        BTPS_Close_Directory(DirectoryDescriptor);
                     }
                     else
                        ret_val = DELETE_ERROR;
                  }
                  else
                     ret_val = DELETE_ERROR;
               }
            }

            /* All finished, so let's return the Current Directory      */
            /* buffer back to what it was.                              */
            FTPInfo->CurrentDirectory[CurrentLength] = '\0';
         }
         else
            ret_val = DELETE_ERROR;
      }
      else
         ret_val = DELETE_ERROR;
   }
   else
      ret_val = DELETE_ERROR;

   return(ret_val);
}

   /* The following function is responsible for populating the          */
   /* OTP_DirectoryInfo_t structure with the current contents to the    */
   /* current directory.  This function takes as it's parameters, the   */
   /* FTP Information Context of the specified FTP Server, the FTPM Mgr */
   /* Directory Information structure to populate, and a pointer to a   */
   /* buffer that (when specified) gets the directory contents of that  */
   /* Folder (specified by FolderName parameter).  The FolderName       */
   /* parameter is optional, and when NULL, is ignored.  This function  */
   /* returns a non-negative return value if the directory information  */
   /* was able to be successfully parsed, or a negative value if there  */
   /* was an error.                                                     */
   /* * NOTE * Because this function populates the OTP_DirectoryInfo_t  */
   /*          structure, it needs to allocate memory.  If this function*/
   /*          returns a non-negative return value then the ObjectInfo  */
   /*          member of this structure will point to a memory block    */
   /*          that was allocated (if non-NULL).  When the caller is    */
   /*          finished with the structure, the caller should call      */
   /*          BTPS_FreeMemory() and pass this value to free the Memory.*/
static int GetCurrentDirectoryInformation(FTPInfo_t *FTPInfo, OTP_DirectoryInfo_t *DirInfoPtr, char *FolderName)
{
   int                          NumEntries;
   char                         NextName[BTPS_MAXIMUM_FILE_NAME_LENGTH];
   char                        *TempCharPtr;
   unsigned int                 CurrentLength;
   unsigned int                 CurrentWorkingLength;
   OTP_ObjectInfo_t            *FolderInfoPtr;
   BTPS_File_Information_t      FileInformation;
   BTPS_Directory_Descriptor_t  DirectoryDescriptor;

   /* Before proceding any further, make sure that we have a semi-valid */
   /* parameters passed to us.                                          */
   if((FTPInfo) && (DirInfoPtr))
   {
      /* Initialize that we have found no entries.                      */
      NumEntries = 0;

      /* If a Folder Name was specified then the client has requested   */
      /* to browse into a Folder instead of the Current Folder.         */
      if(FolderName)
      {
         /* Before proceding any further let's make sure that there is  */
         /* no security breach (possible) by the user specifying any    */
         /* directory delimiter characters.                             */
         CurrentLength = 0;
         while(FolderName[CurrentLength])
         {
            if(FolderName[CurrentLength] == BTPS_DIRECTORY_DELIMITER_CHARACTER)
               break;
            else
               CurrentLength++;
         }

         /* If we went through every character in the FolderName        */
         /* parameter, then we did not find any invalid characters, so  */
         /* we will assume there is no security problem.                */
         if(CurrentLength == BTPS_StringLength(FolderName))
         {
            /* Next, let's note the current length of the Current       */
            /* Directory string.                                        */
            CurrentLength = BTPS_StringLength(FTPInfo->CurrentDirectory);

            /* Make sure we have enough Buffer Space to hold the fully  */
            /* Qualified Path of what we are to search for.             */
            if((CurrentLength + BTPS_StringLength(FolderName)) < sizeof(FTPInfo->CurrentDirectory))
            {
               /* Save the length of the current working directory path.*/
               CurrentWorkingLength = CurrentLength;

               /* Enough Buffer Space was available, so simply build    */
               /* the Search String into the Current Directory Buffer.  */
               BTPS_SprintF(&(FTPInfo->CurrentDirectory[BTPS_StringLength(FTPInfo->CurrentDirectory)]), "%s", FolderName);
               FTPInfo->CurrentDirectory[BTPS_StringLength(FTPInfo->CurrentDirectory) + 1] = '\0';
               FTPInfo->CurrentDirectory[BTPS_StringLength(FTPInfo->CurrentDirectory) + 0] = BTPS_DIRECTORY_DELIMITER_CHARACTER;

               CurrentLength = BTPS_StringLength(FTPInfo->CurrentDirectory);
            }
            else
               CurrentLength = 0;
         }
         else
            CurrentLength = 0;
      }
      else
      {
         /* Next, let's note the current length of the Current Directory*/
         /* string.                                                     */
         CurrentLength = BTPS_StringLength(FTPInfo->CurrentDirectory);

         /* Make sure we have enough Buffer Space to hold the fully     */
         /* Qualified Path of what we are to search for.                */
         if(CurrentLength < sizeof(FTPInfo->CurrentDirectory))
         {
            /* Save the length of the current working directory path.   */
            CurrentWorkingLength = CurrentLength;
         }
         else
            CurrentLength = 0;
      }

      /* Only continue if we were successfully able to build the Search */
      /* String.                                                        */
      if(CurrentLength)
      {
         /* Let's start searching the Search Path and determine how many*/
         /* Directory Entries exist.                                    */
         if((DirectoryDescriptor = BTPS_Open_Directory(FTPInfo->CurrentDirectory)) != NULL)
         {
            NextName[0] = '\0';

            /* At least the Path is valid, so let's count all entries.  */
            while(BTPS_Get_Next_Directory_Entry(DirectoryDescriptor, NextName))
            {
               if((!((BTPS_StringLength(NextName) == BTPS_StringLength(".")) && (!BTPS_MemCompare(NextName, ".", BTPS_StringLength("."))))) && (!((BTPS_StringLength(NextName) == BTPS_StringLength("..")) && (!BTPS_MemCompare(NextName, "..", BTPS_StringLength(".."))))))
                  NumEntries++;
            }

            /* All finished, so close the Directory Descriptor.         */
            BTPS_Close_Directory(DirectoryDescriptor);
         }
         else
            NumEntries = -1;

         /* Only continue if there was no processing error.             */
         if(NumEntries >= 0)
         {
            DirInfoPtr->NumberEntries   = NumEntries;
            DirInfoPtr->ParentDirectory = (Boolean_t)FTPInfo->DirectoryDepth;

            if(NumEntries == 0)
               DirInfoPtr->ObjectInfo = NULL;
            else
            {
               if((DirInfoPtr->ObjectInfo = (OTP_ObjectInfo_t *)BTPS_AllocateMemory((sizeof(OTP_ObjectInfo_t) * NumEntries))) != NULL)
               {
                  FolderInfoPtr = DirInfoPtr->ObjectInfo;
                  if((DirectoryDescriptor = BTPS_Open_Directory(FTPInfo->CurrentDirectory)) != NULL)
                  {
                     NextName[0] = '\0';

                     while(BTPS_Get_Next_Directory_Entry(DirectoryDescriptor, NextName))
                     {
                        /* Simply Build each entry into the newly       */
                        /* allocated array.                             */
                        if((!((BTPS_StringLength(NextName) == BTPS_StringLength(".")) && (!BTPS_MemCompare(NextName, ".", BTPS_StringLength("."))))) && (!((BTPS_StringLength(NextName) == BTPS_StringLength("..")) && (!BTPS_MemCompare(NextName, "..", BTPS_StringLength(".."))))))
                        {
                           /* OK, we need to build the complete file    */
                           /* name including Path Information of the    */
                           /* directory entry.                          */
                           if((BTPS_StringLength(NextName) + BTPS_StringLength(FTPInfo->CurrentDirectory)) < sizeof(FTPInfo->CurrentDirectory))
                           {
                              BTPS_SprintF(&(FTPInfo->CurrentDirectory[BTPS_StringLength(FTPInfo->CurrentDirectory)]), "%s", NextName);

                              if(BTPS_Query_File_Information(FTPInfo->CurrentDirectory, &FileInformation))
                              {
                                 FolderInfoPtr->FieldMask         = (OTP_OBJECT_INFO_MASK_NAME | OTP_OBJECT_INFO_MASK_SIZE | OTP_OBJECT_INFO_MASK_MODIFIED);
                                 FolderInfoPtr->ObjectType        = (FileInformation.FileType == ftFile)?otFile:otFolder;
                                 FolderInfoPtr->Size              = FileInformation.FileSize;

                                 /* Build the Name, truncating if       */
                                 /* necessary.                          */
                                 FolderInfoPtr->NameLength        = (BTPS_StringLength(NextName) <= sizeof(FolderInfoPtr->Name))?BTPS_StringLength(NextName):sizeof(FolderInfoPtr->Name);
                                 BTPS_MemCopy(FolderInfoPtr->Name, NextName, FolderInfoPtr->NameLength);

                                 /* Check to see if the Name had to be  */
                                 /* truncated in order to get it to fit */
                                 /* in the buffer.                      */
                                 /* ** NOTE ** This is a work around for*/
                                 /*            the fact that the Name   */
                                 /*            Buffer in the Object Info*/
                                 /*            Structure is set to '64' */
                                 /*            bytes may result in the  */
                                 /*            truncation of file names.*/
                                 if(BTPS_StringLength(NextName) > sizeof(FolderInfoPtr->Name))
                                 {
                                    /* Now attempt to allocate a buffer */
                                    /* to hold the entire name.         */
                                    if((TempCharPtr = (char *)BTPS_AllocateMemory(BTPS_StringLength(NextName)+sizeof(char))) != NULL)
                                    {
                                       /* Now set the FieldMask to      */
                                       /* indicate that this is an      */
                                       /* extended Name.                */
                                       FolderInfoPtr->FieldMask |= OTP_OBJECT_INFO_MASK_EXTENDED_NAME;

                                       /* Copy the Name into the        */
                                       /* allocated buffer.             */
                                       BTPS_MemCopy(TempCharPtr, NextName, BTPS_StringLength(NextName)+sizeof(char));

                                       /* Next store the pointer to the */
                                       /* end of the Object Info Name   */
                                       /* buffer.                       */
                                       ASSIGN_HOST_DWORD_TO_LITTLE_ENDIAN_UNALIGNED_DWORD(&(FolderInfoPtr->Name[sizeof(FolderInfoPtr->Name) - sizeof(NonAlignedDWord_t)]), TempCharPtr);

                                       /* Change the name length of the */
                                       /* buffer.  Before this addition */
                                       /* the buffer was not '\0'       */
                                       /* terminated, so there no need  */
                                       /* to '\0' terminate the buffer  */
                                       /* now.                          */
                                       FolderInfoPtr->NameLength = (sizeof(FolderInfoPtr->Name) - sizeof(NonAlignedDWord_t));
                                    }
                                 }

                                 /* Store the File Time                 */
                                 FolderInfoPtr->Modified.Year     = FileInformation.FileTime.Year;
                                 FolderInfoPtr->Modified.Month    = FileInformation.FileTime.Month;
                                 FolderInfoPtr->Modified.Day      = FileInformation.FileTime.DayOfMonth;
                                 FolderInfoPtr->Modified.Hour     = FileInformation.FileTime.Hour;
                                 FolderInfoPtr->Modified.Minute   = FileInformation.FileTime.Minute;
                                 FolderInfoPtr->Modified.Second   = FileInformation.FileTime.Seconds;
                                 FolderInfoPtr->Modified.UTC_Time = FileInformation.FileTime.UTC_Time;

                                 FolderInfoPtr++;
                              }
                              else
                                 DirInfoPtr->NumberEntries--;

                              /* Restore the Current Directory back to  */
                              /* what it was before we added the file   */
                              /* name to it.                            */
                              FTPInfo->CurrentDirectory[CurrentLength] = '\0';
                           }
                           else
                              DirInfoPtr->NumberEntries--;
                        }
                     }

                     /* All finished, so close the Directory Descriptor.*/
                     BTPS_Close_Directory(DirectoryDescriptor);
                  }
                  else
                     NumEntries = -1;

                  /* If there was an error retrieving the information   */
                  /* about one or more of the files and it resulted in  */
                  /* No Files being returned, then we need to Free the  */
                  /* memory that we allocated and return that there are */
                  /* no Entries present.                                */
                  if(!DirInfoPtr->NumberEntries)
                  {
                     /* Free the Memory.                                */
                     BTPS_FreeMemory(FolderInfoPtr);

                     /* Flag that there is NO File Information          */
                     /* available.                                      */
                     DirInfoPtr->ObjectInfo = NULL;
                  }
               }
               else
               {
                  /* Error Allocating Memory.                           */
                  NumEntries = -1;
               }
            }
         }

         /* All finished, so let's return the Current Directory buffer  */
         /* back to what it was.                                        */
         FTPInfo->CurrentDirectory[CurrentWorkingLength] = '\0';
      }
      else
         NumEntries = -1;
   }
   else
      NumEntries = -1;

   return(NumEntries);
}

   /* The following function is responsible for reading Data from the   */
   /* specified File Name into the specified Buffer.  This function     */
   /* accepts as it's input parameters, the FTP Information Context,    */
   /* a pointer to a value that represents the size of the Buffer to    */
   /* read the Data into, a pointer to a Buffer, and the name of the    */
   /* File to Read.  This function returns one of the following         */
   /* results to flag the state of the File Data Read:                  */
   /*    - FILE_DATA_FILE_IO_ERROR                                      */
   /*    - FILE_DATA_FILE_IO_SUCCESS                                    */
   /*    - FILE_DATA_MORE_TO_COME                                       */
   /*    - FILE_DATA_END_OF_FILE                                        */
   /* * NOTE * This function determines the start of a Transfer when    */
   /*          the FirstPhase member of the FileInfo member of the      */
   /*          FTPInfo structure is set to TRUE.  This function modifies*/
   /*          the members of this structure as well.                   */
static int GetFileData(FTPInfo_t *FTPInfo, DWord_t *BufferSize, unsigned char *BufferPtr, char *FileName)
{
   int                     ret_val;
   DWord_t                 BytesRead;
   unsigned int            CurrentLength;
   BTPS_File_Information_t FileInformation;

   /* Before proceding any further, make sure that we have a semi-valid */
   /* parameters passed to us.                                          */
   if((FTPInfo) && (BufferSize) && (*BufferSize) && (BufferPtr) && (FileName) && (BTPS_StringLength(FileName)))
   {
      /* Before proceding any further let's make sure that there is no  */
      /* security breach (possible) by the user specifying any directory*/
      /* delimiter characters.                                          */
      CurrentLength = 0;
      while(FileName[CurrentLength])
      {
         if(FileName[CurrentLength] == BTPS_DIRECTORY_DELIMITER_CHARACTER)
            break;
         else
            CurrentLength++;
      }

      /* If we scanned the entire File Name then the user must not have */
      /* specified any potential security breachable characters.        */
      if(CurrentLength == BTPS_StringLength(FileName))
      {
         /* Next, let's note the current length of the Current Directory*/
         /* string.                                                     */
         CurrentLength = BTPS_StringLength(FTPInfo->CurrentDirectory);

         /* Make sure that we have enough Buffer to build the fully     */
         /* qualified Path File Name into.                              */
         if((CurrentLength + BTPS_StringLength(FileName)) < sizeof(FTPInfo->CurrentDirectory))
         {
            if((FTPInfo->FileInfo.FirstPhase) || ((!FTPInfo->FileInfo.FirstPhase) && (FTPInfo->FileInfo.FileDescriptor)))
            {
               /* Determine if this is the first request or not.        */
               if(FTPInfo->FileInfo.FirstPhase)
               {
                  /* Build the fully qualified Path File Name into the  */
                  /* Buffer.                                            */
                  BTPS_SprintF(&(FTPInfo->CurrentDirectory[BTPS_StringLength(FTPInfo->CurrentDirectory)]), "%s", FileName);

                  if((FTPInfo->FileInfo.FileDescriptor = BTPS_Open_File(FTPInfo->CurrentDirectory, omReadOnly)) != NULL)
                  {
                     FTPInfo->FileInfo.FirstPhase = FALSE;
                     FTPInfo->FileInfo.FileIndex  = 0;

                     if(BTPS_Query_File_Information(FTPInfo->CurrentDirectory, &FileInformation))
                        FTPInfo->FileInfo.FileSize = FileInformation.FileSize;
                     else
                        FTPInfo->FileInfo.FileSize = 0;

                     /* Just to be sure, we will seek to the beginning  */
                     /* of the file to make sure we are the beginning of*/
                     /* the file.                                       */
                     if(FTPInfo->FileInfo.FileDescriptor)
                        BTPS_Seek_File(FTPInfo->FileInfo.FileDescriptor, smBeginning, (SDWord_t)0);
                  }
               }

               /* Let's read the next BufferSize Bytes into our Buffer. */
               if((FTPInfo->FileInfo.FileDescriptor) && (BTPS_Read_File(FTPInfo->FileInfo.FileDescriptor, *BufferSize, BufferPtr, &BytesRead)))
               {
                  /* Read successful, now let's update the Amount of    */
                  /* data that we have already read.                    */
                  FTPInfo->FileInfo.FileIndex += BytesRead;

                  *BufferSize                  = BytesRead;

                  ret_val                      = (FTPInfo->FileInfo.FileIndex < FTPInfo->FileInfo.FileSize)?FILE_DATA_MORE_TO_COME:FILE_DATA_END_OF_FILE;
               }
               else
                  ret_val = FILE_DATA_FILE_IO_ERROR;

               /* Unless there is more data to come, we need to make    */
               /* sure that we close the File we have open.             */
               if(ret_val != FILE_DATA_MORE_TO_COME)
               {
                  BTPS_Close_File(FTPInfo->FileInfo.FileDescriptor);

                  FTPInfo->FileInfo.FileDescriptor = NULL;
               }
            }
            else
               ret_val = FILE_DATA_FILE_IO_ERROR;

            /* All finished, so let's return the Current Directory      */
            /* buffer back to what it was.                              */
            FTPInfo->CurrentDirectory[CurrentLength] = '\0';
         }
         else
            ret_val = FILE_DATA_FILE_IO_ERROR;
      }
      else
         ret_val = FILE_DATA_FILE_IO_ERROR;
   }
   else
      ret_val = FILE_DATA_FILE_IO_ERROR;

   return(ret_val);
}

   /* The following function is responsible for writing the specified   */
   /* data to the specified File.  This function accepts as it's input  */
   /* parameters the FTP Server Information Context, whether or not to  */
   /* close the file after the write, the length of the data contained  */
   /* in the Data Buffer, a pointer to the data itself, and the name of */
   /* the File to Write the data to.  This function returns one of the  */
   /* following results to flag the state of the file data write:       */
   /*    - FILE_DATA_FILE_IO_ERROR                                      */
   /*    - FILE_DATA_FILE_IO_SUCCESS                                    */
   /*    - FILE_DATA_MORE_TO_COME                                       */
   /*    - FILE_DATA_END_OF_FILE                                        */
   /* * NOTE * This function determines the start of a Transfer when    */
   /*          the FirstPhase member of the FileInfo member of the      */
   /*          FTPInfo structure is set to TRUE.  This function modifies*/
   /*          the members of this structure as well.                   */
static int PutFileData(FTPInfo_t *FTPInfo, Boolean_t CloseFile, DWord_t BufferSize, unsigned char *BufferPtr, char *FileName)
{
   int          ret_val;
   DWord_t      BytesWritten;
   unsigned int CurrentLength;

   /* Before proceding any further, make sure that we have a semi-valid */
   /* parameters passed to us.                                          */
   if((FTPInfo) && ((!BufferSize) || ((BufferSize) && (BufferPtr))) && (FileName) && (BTPS_StringLength(FileName)))
   {
      /* Before proceding any further let's make sure that there is no  */
      /* security breach (possible) by the user specifying any directory*/
      /* delimiter characters.                                          */
      CurrentLength = 0;
      while(FileName[CurrentLength])
      {
         if(FileName[CurrentLength] == BTPS_DIRECTORY_DELIMITER_CHARACTER)
            break;
         else
            CurrentLength++;
      }

      /* If we scanned the entire File Name then the user must not have */
      /* specified any potential security breachable characters.        */
      if(CurrentLength == BTPS_StringLength(FileName))
      {
         /* Next, let's note the current length of the Current Directory*/
         /* string.                                                     */
         CurrentLength = BTPS_StringLength(FTPInfo->CurrentDirectory);

         /* Make sure that we have enough Buffer to build the fully     */
         /* qualified Path File Name into.                              */
         if((CurrentLength + BTPS_StringLength(FileName)) < sizeof(FTPInfo->CurrentDirectory))
         {
            if((FTPInfo->FileInfo.FirstPhase) || ((!FTPInfo->FileInfo.FirstPhase) && (FTPInfo->FileInfo.FileDescriptor)))
            {
               if(FTPInfo->FileInfo.FirstPhase)
               {
                  /* Build the fully qualified Path File Name into the  */
                  /* Buffer.                                            */
                  BTPS_SprintF(&(FTPInfo->CurrentDirectory[BTPS_StringLength(FTPInfo->CurrentDirectory)]), "%s", FileName);

                  FTPInfo->FileInfo.FileDescriptor = BTPS_Open_File(FTPInfo->CurrentDirectory, omCreate);
               }

               /* Since we have made a pass through this function at    */
               /* least once, we need to flag that we are no longer in  */
               /* the first phase.                                      */
               FTPInfo->FileInfo.FirstPhase = FALSE;

               /* Make sure that we have a valid File Descriptor (open  */
               /* file).                                                */
               if(FTPInfo->FileInfo.FileDescriptor)
               {
                  if(BufferSize)
                  {
                     if(BTPS_Write_File(FTPInfo->FileInfo.FileDescriptor, BufferSize, BufferPtr, &BytesWritten))
                     {
                        FTPInfo->FileInfo.FileIndex += BytesWritten;

                        ret_val                      = FILE_DATA_FILE_IO_SUCCESS;
                     }
                     else
                        ret_val = FILE_DATA_FILE_IO_ERROR;
                  }
                  else
                     ret_val = FILE_DATA_FILE_IO_SUCCESS;

                  /* If there was an error (or we have been asked to    */
                  /* close the file) we need to make sure we close the  */
                  /* open file.                                         */
                  if((ret_val == FILE_DATA_FILE_IO_ERROR) || (CloseFile))
                  {
                     BTPS_Close_File(FTPInfo->FileInfo.FileDescriptor);

                     FTPInfo->FileInfo.FileDescriptor = NULL;
                  }
               }
               else
                  ret_val = FILE_DATA_FILE_IO_ERROR;
            }
            else
               ret_val = FILE_DATA_FILE_IO_ERROR;

            /* All finished, so let's return the Current Directory      */
            /* buffer back to what it was.                              */
            FTPInfo->CurrentDirectory[CurrentLength] = '\0';
         }
         else
            ret_val = FILE_DATA_FILE_IO_ERROR;
      }
      else
         ret_val = FILE_DATA_FILE_IO_ERROR;
   }
   else
      ret_val = FILE_DATA_FILE_IO_ERROR;

   return(ret_val);
}

   /* The following function is responsible for extracting the Name from*/
   /* the Object Information.  The only parameter to this function is   */
   /* the Object Information structure in which the Name member exists  */
   /* to be extracted.  This function returns a pointer to the Name     */
   /* member upon successful execution or NULL upon all errors.         */
   /* * NOTE * This function exists to work around a limitation with the*/
   /*          size of the Name member in the Object Information        */
   /*          structure.                                               */
static char *ExtractNameFromObjectInfo(OTP_ObjectInfo_t *ObjectInfoPtr)
{
   char *ret_val;

   /* First check to make sure that the parameter passed in appears to  */
   /* be at least semi-valid.                                           */
   if(ObjectInfoPtr != NULL)
   {
      /* The parameter passed in appears to be at least semi-valid.     */
      /* Next check to see if the Extended Name Bit is set in the Field */
      /* Mask.                                                          */
      if(ObjectInfoPtr->FieldMask & OTP_OBJECT_INFO_MASK_EXTENDED_NAME)
      {
         /* The Extended Name Bit is set, extract the pointer to the    */
         /* Name from the end of the Name member in the Object          */
         /* Information Structure.                                      */
         ret_val = (char *)READ_OBJECT_INFO_EXTENDED_NAME(ObjectInfoPtr->Name);
      }
      else
      {
         /* The Extended Name Bit is not set, simply set the return     */
         /* value to the start of the Name member in the Object         */
         /* Information Structure.                                      */
         ret_val = ObjectInfoPtr->Name;
      }
   }
   else
      ret_val = NULL;

   return(ret_val);
}

   /* The following function is responsible for extracting the Name     */
   /* Length from the Object Information.  The only parameter to this   */
   /* function is the Object Information structure in which the Name    */
   /* Length member exists to be extracted.  This function returns a the*/
   /* Name Length value upon successful execution or zero upon all      */
   /* errors.                                                           */
   /* * NOTE * This function exists to work around a limitation with the*/
   /*          size of the Name member in the Object Information        */
   /*          structure.                                               */
static unsigned int ExtractNameLengthFromObjectInfo(OTP_ObjectInfo_t *ObjectInfoPtr)
{
   char         *TempCharPtr;
   unsigned int  ret_val;

   /* First check to make sure that the parameter passed in appears to  */
   /* be at least semi-valid.                                           */
   if(ObjectInfoPtr != NULL)
   {
      /* The parameter passed in appears to be at least semi-valid.     */
      /* Next check to see if the Extended Name Bit is set in the Field */
      /* Mask.                                                          */
      if(ObjectInfoPtr->FieldMask & OTP_OBJECT_INFO_MASK_EXTENDED_NAME)
      {
         /* The Extended Name Bit is set, extract the pointer to the    */
         /* Name from the end of the Name member in the Object          */
         /* Information Structure.                                      */
         TempCharPtr = (char *)READ_OBJECT_INFO_EXTENDED_NAME(ObjectInfoPtr->Name);

         /* Set the return value to the length of the name including the*/
         /* '\0' terminating character.                                 */
         ret_val = BTPS_StringLength(TempCharPtr) + sizeof(char);
      }
      else
      {
         /* The Extended Name Bit is not set, simply set the return     */
         /* value to the NameLength member in the Object Information    */
         /* Structure.                                                  */
         ret_val = ObjectInfoPtr->NameLength;
      }
   }
   else
      ret_val = 0;

   return(ret_val);
}

   /* The following function is responsible for processing All incoming */
   /* FTPM Mgr Port Events.  This function is installed for every OBEX  */
   /* FTP Profile.  This function is responsible for processing incoming*/
   /* FTPM Mgr Port Events and processing the Event.                    */
static void BTPSAPI OTP_Event_Callback(OTP_Event_Data_t *OTP_Event_Data, unsigned long CallbackParameter)
{
   int                  Status;
   Byte_t               Response;
   DWord_t              BytesTransferred;
   DWord_t              CurrentLength;
   Boolean_t            DeleteEntry;
   Boolean_t            FTPServer;
   FTPInfo_t           *FTPInfo;
   unsigned int         FTPConnectionID;
   unsigned int         Index;
   FTPM_Event_Data_t    FTP_Event_Data;
   OTP_DirectoryInfo_t  DirInfo;

   DebugPrint((BTPM_DEBUG_ZONE_FTP | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* Make sure that the input parameters that were passed to us are    */
   /* semi-valid.                                                       */
   if((Initialized) && (OTP_Event_Data) && (CallbackParameter))
   {
      /* Search the FTP Profile Information List for the specified FTP  */
      /* Profile Entry.                                                 */
      if((FTPInfo = SearchFTPEntry(&FTPInfoList, CallbackParameter)) != NULL)
      {
         /* Flag that we have not been instructed to delete the FTP     */
         /* Entry from the File Transfer Profile List.                  */
         DeleteEntry     = FALSE;

         /* Note some Context variables into some local variables so    */
         /* that we can use them later.                                 */
         FTPConnectionID = FTPInfo->FTPConnectionID;
         FTPServer       = FTPInfo->FTPServer;

         /* Process the request differently, depending upon if this     */
         /* entry is a FTP Client or FTP Server.                        */
         if(FTPInfo->FTPServer)
         {
            switch(OTP_Event_Data->Event_Data_Type)
            {
               case etOTP_Disconnect_Request:
               case etOTP_Port_Close_Indication:
                  /* It is possible that we have received a Close Port  */
                  /* Indication before the Disconnect Request.  We need */
                  /* to guard against this case so that we do not       */
                  /* dispatch two Disconnect Indication Events (not that*/
                  /* this would really harm anything).                  */
                  if((FTPInfo->ConnectionState == csConnected) || (FTPInfo->ConnectionState == csConnectionPending))
                  {
                     /* Flag that we are no longer in the connected     */
                     /* state.                                          */
                     FTPInfo->ConnectionState = csNotConnected;

                     /* Client was disconnected, go ahead and close any */
                     /* open files.                                     */
                     if(FTPInfo->FileInfo.FileDescriptor)
                     {
                        BTPS_Close_File(FTPInfo->FileInfo.FileDescriptor);

                        FTPInfo->FileInfo.FileDescriptor = NULL;
                     }

                     /* Format a FTP Disconnect Event and inform the the*/
                     /* Event Callback that a Disconnect has occurred.  */
                     FTP_Event_Data.EventType                                               = etFTPMDisconnectIndication;
                     FTP_Event_Data.EventLength                                             = FTPM_DISCONNECT_INDICATION_DATA_SIZE;

                     FTP_Event_Data.EventData.DisconnectIndicationEventData.FTPConnectionID = FTPInfo->FTPConnectionID;

                     /* Finally call the Callback.                      */
                     __BTPSTRY
                     {
                        if(FTPInfo->CallbackFunction)
                        {
                           FTPInfo->CallbackActive = TRUE;

                           (*((FTPM_Event_Callback_t)(FTPInfo->CallbackFunction)))(&FTP_Event_Data, FTPInfo->CallbackParameter);

                           FTPInfo->CallbackActive = FALSE;
                        }
                     }
                     __BTPSEXCEPT(1)
                     {
                        /* Do Nothing.                                  */
                     }
                  }
                  break;
               case etOTP_Port_Open_Request_Indication:
                  /* Simply note the BD_ADDR of the Remote Bluetooth    */
                  /* Device that is requesting to connect to us.        */
                  FTPInfo->BD_ADDR = OTP_Event_Data->Event_Data.OTP_Port_Open_Request_Indication_Data->BD_ADDR;

                  /* Set the Connection State to indicate that there is */
                  /* currently a pending connection request.            */
                  FTPInfo->ConnectionState = csConnectionPending;

                  /* Format a FTP Connect Request Event and inform the  */
                  /* Event Callback that a Connect Request has occurred.*/
                  FTP_Event_Data.EventType                                                              = etFTPMIncomingConnectionRequest;
                  FTP_Event_Data.EventLength                                                            = FTPM_SERVER_INCOMING_CONNECTION_REQUEST_DATA_SIZE;

                  FTP_Event_Data.EventData.ServerIncomingConnectionRequestEventData.FTPConnectionID     = FTPInfo->FTPConnectionID;
                  FTP_Event_Data.EventData.ServerIncomingConnectionRequestEventData.RemoteDeviceAddress = FTPInfo->BD_ADDR;

                  /* Finally call the Callback.                         */
                  __BTPSTRY
                  {
                     if(FTPInfo->CallbackFunction)
                     {
                        FTPInfo->CallbackActive = TRUE;

                        (*((FTPM_Event_Callback_t)(FTPInfo->CallbackFunction)))(&FTP_Event_Data, FTPInfo->CallbackParameter);

                        FTPInfo->CallbackActive = FALSE;
                     }
                  }
                  __BTPSEXCEPT(1)
                  {
                     /* Do Nothing.                                     */
                  }
                  break;
               case etOTP_Port_Open_Indication:
                  /* Simply note the BD_ADDR of the Remote Bluetooth    */
                  /* Device that has connected to us.                   */
                  FTPInfo->BD_ADDR   = OTP_Event_Data->Event_Data.OTP_Port_Open_Indication_Data->BD_ADDR;

                  /* Flag that we are in the connected state.           */
                  FTPInfo->ConnectionState = csConnected;

                  /* Format a FTP Connect Event and inform the the Event*/
                  /* Callback that a Connect has occurred.              */
                  FTP_Event_Data.EventType                                                      = etFTPMConnectIndication;
                  FTP_Event_Data.EventLength                                                    = FTPM_SERVER_CONNECT_INDICATION_DATA_SIZE;

                  FTP_Event_Data.EventData.ServerConnectIndicationEventData.FTPConnectionID     = FTPInfo->FTPConnectionID;
                  FTP_Event_Data.EventData.ServerConnectIndicationEventData.RemoteDeviceAddress = FTPInfo->BD_ADDR;

                  /* Finally call the Callback.                         */
                  __BTPSTRY
                  {
                     if(FTPInfo->CallbackFunction)
                     {
                        FTPInfo->CallbackActive = TRUE;

                        (*((FTPM_Event_Callback_t)(FTPInfo->CallbackFunction)))(&FTP_Event_Data, FTPInfo->CallbackParameter);

                        FTPInfo->CallbackActive = FALSE;
                     }
                  }
                  __BTPSEXCEPT(1)
                  {
                     /* Do Nothing.                                     */
                  }
                  break;
               case etOTP_Connect_Request:
                  /* Set the Path to the Root.                          */
                  if(OTP_Event_Data->Event_Data.OTP_Connect_Request_Data->Target == tFileBrowser)
                  {
                     Status = SetCurrentPath(FTPInfo, FALSE, FALSE, NULL);

                     if((Status != SET_PATH_ERROR) && (Status != SET_PATH_ERROR_INVALID_PATH))
                     {
                        FTPInfo->CurrentOperation = coNone;

                        /* Simply accept the Request.                   */
                        _FTPM_Connect_Response(FTPInfo->OTPID, TRUE, NULL, NULL);
                     }
                     else
                        _FTPM_Connect_Response(FTPInfo->OTPID, FALSE, NULL, NULL);
                  }
                  else
                     _FTPM_Connect_Response(FTPInfo->OTPID, FALSE, NULL, NULL);
                  break;
               case etOTP_Get_Directory_Request:
                  /* Simply populate a Directory Information Structure  */
                  /* and pass the response back to the FTPM Mgr Server  */
                  /* (provided we have Read Access to the File Server). */
                  if(FTPInfo->ServerPermissions & FTPM_SERVER_PERMISSION_MASK_READ)
                  {
                     Status = GetCurrentDirectoryInformation(FTPInfo, &DirInfo, (OTP_Event_Data->Event_Data.OTP_Get_Directory_Request_Data->NameLength)?OTP_Event_Data->Event_Data.OTP_Get_Directory_Request_Data->Name:NULL);
                     if(Status >= 0)
                     {
                        _FTPM_Get_Directory_Request_Response(FTPInfo->OTPID, &DirInfo, OTP_OK_RESPONSE);

                        FTPInfo->CurrentOperation = coDirectoryRequest;

                        /* Format a FTP Get Directory Request Event and */
                        /* inform the Event Callback that the action has*/
                        /* occurred.                                    */
                        FTP_Event_Data.EventType                                                           = etFTPMDirectoryRequestIndication;
                        FTP_Event_Data.EventLength                                                         = FTPM_SERVER_DIRECTORY_REQUEST_INDICATION_DATA_SIZE;

                        FTP_Event_Data.EventData.ServerDirectoryRequestIndicationEventData.FTPConnectionID = FTPInfo->FTPConnectionID;

                        /* Format up the Directory Name that was        */
                        /* Requested.                                   */
                        /* * NOTE * We do not have to worry about       */
                        /*          checking our Buffer Sizes because we*/
                        /*          know everything fits because the    */
                        /*          Directory Request was successful.   */
                        BTPS_StringCopy(FTPInfo->CurrentObject, FTPInfo->CurrentDirectory);

                        /* Let's check to make sure a Sub Directory     */
                        /* Browse wasn't specified.                     */
                        if(OTP_Event_Data->Event_Data.OTP_Get_Directory_Request_Data->NameLength)
                        {
                           BTPS_SprintF(&(FTPInfo->CurrentObject[BTPS_StringLength(FTPInfo->CurrentObject)]), "%s", OTP_Event_Data->Event_Data.OTP_Get_Directory_Request_Data->Name);
                        }
                        else
                        {
                           /* The Current Directory was requested.      */
                           /* Let's check to see if we need to remove   */
                           /* the trailing directory delimiter          */
                           /* characters.                               */
                           if((BTPS_MemCompare(FTPInfo->CurrentObject, FTPInfo->RootDirectory, BTPS_StringLength(FTPInfo->CurrentObject))) || ((!BTPS_MemCompare(FTPInfo->CurrentObject, FTPInfo->RootDirectory, BTPS_StringLength(FTPInfo->CurrentObject))) && (!FTPInfo->RootIsDrive)))
                              FTPInfo->CurrentObject[BTPS_StringLength(FTPInfo->CurrentObject) - 1] = '\0';
                        }

                        FTP_Event_Data.EventData.ServerDirectoryRequestIndicationEventData.DirectoryName = FTPInfo->CurrentObject;

                        /* Finally call the Callback.                   */
                        __BTPSTRY
                        {
                           if(FTPInfo->CallbackFunction)
                           {
                              FTPInfo->CallbackActive = TRUE;

                              (*((FTPM_Event_Callback_t)(FTPInfo->CallbackFunction)))(&FTP_Event_Data, FTPInfo->CallbackParameter);

                              FTPInfo->CallbackActive = FALSE;
                           }
                        }
                        __BTPSEXCEPT(1)
                        {
                           /* Do Nothing.                               */
                        }
                     }
                     else
                        _FTPM_Get_Directory_Request_Response(FTPInfo->OTPID, NULL, OTP_INTERNAL_SERVER_ERROR_RESPONSE);
                  }
                  else
                     _FTPM_Get_Directory_Request_Response(FTPInfo->OTPID, NULL, OTP_FORBIDDEN_RESPONSE);
                  break;
               case etOTP_Delete_Object_Request:
                  /* Before continuing any further, let's make sure that*/
                  /* the Client has been granted Delete Access.         */
                  if(FTPInfo->ServerPermissions & FTPM_SERVER_PERMISSION_MASK_DELETE)
                  {
                     Status = DeleteDirectoryEntry(FTPInfo, ExtractNameFromObjectInfo(&(OTP_Event_Data->Event_Data.OTP_Delete_Object_Request_Data->ObjectInfo)));

                     /* Respond to the Delete Object Response based upon*/
                     /* how we were able to process the request.        */
                     if(Status == DELETE_ERROR)
                        Response = OTP_FORBIDDEN_RESPONSE;
                     else
                     {
                        if(Status == DELETE_ERROR_SUBDIRECTORY_NOT_EMPTY)
                           Response = OTP_PRECONDITION_FAILED_RESPONSE;
                        else
                           Response = OTP_OK_RESPONSE;
                     }

                     _FTPM_Delete_Object_Response(FTPInfo->OTPID, (Byte_t)Response);

                     /* If a File or Directory was Deleted then we need */
                     /* to notify the FTP Server Callback of the Event. */
                     if((Status == DELETE_FILE) || (Status == DELETE_SUBDIRECTORY))
                     {
                        if(Status == DELETE_FILE)
                        {
                           FTP_Event_Data.EventType                                                     = etFTPMFileDeleteIndication;
                           FTP_Event_Data.EventLength                                                   = FTPM_SERVER_FILE_DELETE_INDICATION_DATA_SIZE;

                           FTP_Event_Data.EventData.ServerFileDeleteIndicationEventData.FTPConnectionID = FTPInfo->FTPConnectionID;
                           FTP_Event_Data.EventData.ServerFileDeleteIndicationEventData.DeletedFileName = ExtractNameFromObjectInfo(&(OTP_Event_Data->Event_Data.OTP_Delete_Object_Request_Data->ObjectInfo));

                           BTPS_StringCopy(FTPInfo->CurrentObject, FTPInfo->CurrentDirectory);

                           /* Let's check to see if we need to remove   */
                           /* the trailing directory delimiter          */
                           /* characters.                               */
                           if((BTPS_MemCompare(FTPInfo->CurrentObject, FTPInfo->RootDirectory, BTPS_StringLength(FTPInfo->CurrentObject))) || ((!BTPS_MemCompare(FTPInfo->CurrentObject, FTPInfo->RootDirectory, BTPS_StringLength(FTPInfo->CurrentObject))) && (!FTPInfo->RootIsDrive)))
                              FTPInfo->CurrentObject[BTPS_StringLength(FTPInfo->CurrentObject) - 1] = '\0';

                           FTP_Event_Data.EventData.ServerFileDeleteIndicationEventData.DirectoryName = FTPInfo->CurrentObject;
                        }
                        else
                        {
                           FTP_Event_Data.EventType                                                               = etFTPMDirectoryDeleteIndication;
                           FTP_Event_Data.EventLength                                                             = FTPM_SERVER_DIRECTORY_DELETE_INDICATION_DATA_SIZE;

                           FTP_Event_Data.EventData.ServerDirectoryDeleteIndicationEventData.FTPConnectionID      = FTPInfo->FTPConnectionID;
                           FTP_Event_Data.EventData.ServerDirectoryDeleteIndicationEventData.DeletedDirectoryName = ExtractNameFromObjectInfo(&(OTP_Event_Data->Event_Data.OTP_Delete_Object_Request_Data->ObjectInfo));

                           BTPS_StringCopy(FTPInfo->CurrentObject, FTPInfo->CurrentDirectory);

                           /* Let's check to see if we need to remove   */
                           /* the trailing directory delimiter          */
                           /* characters.                               */
                           if((BTPS_MemCompare(FTPInfo->CurrentObject, FTPInfo->RootDirectory, BTPS_StringLength(FTPInfo->CurrentObject))) || ((!BTPS_MemCompare(FTPInfo->CurrentObject, FTPInfo->RootDirectory, BTPS_StringLength(FTPInfo->CurrentObject))) && (!FTPInfo->RootIsDrive)))
                              FTPInfo->CurrentObject[BTPS_StringLength(FTPInfo->CurrentObject) - 1] = '\0';

                           FTP_Event_Data.EventData.ServerDirectoryDeleteIndicationEventData.DirectoryName = FTPInfo->CurrentObject;
                        }

                        /* Finally call the Callback.                   */
                        __BTPSTRY
                        {
                           if(FTPInfo->CallbackFunction)
                           {
                              FTPInfo->CallbackActive = TRUE;

                              (*((FTPM_Event_Callback_t)(FTPInfo->CallbackFunction)))(&FTP_Event_Data, FTPInfo->CallbackParameter);

                              FTPInfo->CallbackActive = FALSE;
                           }
                        }
                        __BTPSEXCEPT(1)
                        {
                           /* Do Nothing.                               */
                        }
                     }
                  }
                  else
                     _FTPM_Delete_Object_Response(FTPInfo->OTPID, (Byte_t)OTP_FORBIDDEN_RESPONSE);
                  break;
               case etOTP_Get_Object_Request:
                  /* Before continuing any further, let's make sure that*/
                  /* the Client has been granted Read Access.           */
                  if(FTPInfo->ServerPermissions & FTPM_SERVER_PERMISSION_MASK_READ)
                  {
                     /* If this is the beginning of a Transfer then we  */
                     /* need to initialize the File Info Information to */
                     /* Nothing Transfered.                             */
                     if(OTP_Event_Data->Event_Data.OTP_Get_Object_Request_Data->Phase & OTP_OBJECT_PHASE_FIRST)
                     {
                        FTPInfo->FileInfo.FirstPhase     = TRUE;
                        FTPInfo->FileInfo.FileIndex      = 0;
                        FTPInfo->FileInfo.FileSize       = 0;
                        FTPInfo->FileInfo.FileDescriptor = NULL;
                     }

                     /* Let the GetFileData() function take care of the */
                     /* work for us.                                    */
                     CurrentLength                                                      = OTP_Event_Data->Event_Data.OTP_Get_Object_Request_Data->BufferSize;

                     OTP_Event_Data->Event_Data.OTP_Get_Object_Request_Data->BufferSize = 0;

                     Status = GetFileData(FTPInfo, &CurrentLength, OTP_Event_Data->Event_Data.OTP_Get_Object_Request_Data->BufferPtr, ExtractNameFromObjectInfo(&(OTP_Event_Data->Event_Data.OTP_Get_Object_Request_Data->ObjectInfo)));
                     if(Status == FILE_DATA_FILE_IO_ERROR)
                     {
                        Response = OBEX_INTERNAL_SERVER_ERROR_RESPONSE;

                        if((!FTPInfo->FileInfo.FirstPhase) && (FTPInfo->CurrentOperation == coFileGet))
                        {
                           FTP_Event_Data.EventType                                                    = etFTPMFileGetIndication;
                           FTP_Event_Data.EventLength                                                  = FTPM_SERVER_FILE_GET_INDICATION_DATA_SIZE;

                           FTP_Event_Data.EventData.ServerFileGetIndicationEventData.FTPConnectionID   = FTPInfo->FTPConnectionID;
                           FTP_Event_Data.EventData.ServerFileGetIndicationEventData.FileName          = ExtractNameFromObjectInfo(&(OTP_Event_Data->Event_Data.OTP_Get_Object_Request_Data->ObjectInfo));
                           FTP_Event_Data.EventData.ServerFileGetIndicationEventData.TransferComplete  = TRUE;
                           FTP_Event_Data.EventData.ServerFileGetIndicationEventData.TransferredLength = FTPInfo->FileInfo.FileIndex;
                           FTP_Event_Data.EventData.ServerFileGetIndicationEventData.TotalLength       = FTPInfo->FileInfo.FileSize;

                           BTPS_StringCopy(FTPInfo->CurrentObject, FTPInfo->CurrentDirectory);

                           /* Let's check to see if we need to remove   */
                           /* the trailing directory delimiter          */
                           /* characters.                               */
                           if((BTPS_MemCompare(FTPInfo->CurrentObject, FTPInfo->RootDirectory, BTPS_StringLength(FTPInfo->CurrentObject))) || ((!BTPS_MemCompare(FTPInfo->CurrentObject, FTPInfo->RootDirectory, BTPS_StringLength(FTPInfo->CurrentObject))) && (!FTPInfo->RootIsDrive)))
                              FTPInfo->CurrentObject[BTPS_StringLength(FTPInfo->CurrentObject) - 1] = '\0';

                           FTP_Event_Data.EventData.ServerFileGetIndicationEventData.DirectoryName = FTPInfo->CurrentObject;

                           /* Finally call the Callback.                */
                           __BTPSTRY
                           {
                              if(FTPInfo->CallbackFunction)
                              {
                                 FTPInfo->CallbackActive = TRUE;

                                 (*((FTPM_Event_Callback_t)(FTPInfo->CallbackFunction)))(&FTP_Event_Data, FTPInfo->CallbackParameter);

                                 FTPInfo->CallbackActive = FALSE;
                              }
                           }
                           __BTPSEXCEPT(1)
                           {
                              /* Do Nothing.                            */
                           }

                           /* Note the Current Object File name in case */
                           /* an Abort Request comes in.                */
                           BTPS_StringCopy(FTPInfo->CurrentObject, ExtractNameFromObjectInfo(&(OTP_Event_Data->Event_Data.OTP_Get_Object_Request_Data->ObjectInfo)));
                        }
                     }
                     else
                     {
                        OTP_Event_Data->Event_Data.OTP_Get_Object_Request_Data->BufferSize = (unsigned int)CurrentLength;

                        if(Status == FILE_DATA_END_OF_FILE)
                           Response = OBEX_OK_RESPONSE;
                        else
                           Response = OBEX_CONTINUE_RESPONSE;

                        FTP_Event_Data.EventType                                                    = etFTPMFileGetIndication;
                        FTP_Event_Data.EventLength                                                  = FTPM_SERVER_FILE_GET_INDICATION_DATA_SIZE;

                        FTP_Event_Data.EventData.ServerFileGetIndicationEventData.FTPConnectionID   = FTPInfo->FTPConnectionID;
                        FTP_Event_Data.EventData.ServerFileGetIndicationEventData.FileName          = ExtractNameFromObjectInfo(&(OTP_Event_Data->Event_Data.OTP_Get_Object_Request_Data->ObjectInfo));
                        FTP_Event_Data.EventData.ServerFileGetIndicationEventData.TransferComplete  = (Boolean_t)(Response != OBEX_CONTINUE_RESPONSE);
                        FTP_Event_Data.EventData.ServerFileGetIndicationEventData.TransferredLength = FTPInfo->FileInfo.FileIndex;
                        FTP_Event_Data.EventData.ServerFileGetIndicationEventData.TotalLength       = FTPInfo->FileInfo.FileSize;

                        BTPS_StringCopy(FTPInfo->CurrentObject, FTPInfo->CurrentDirectory);

                        /* Let's check to see if we need to remove the  */
                        /* trailing directory delimiter characters.     */
                        if((BTPS_MemCompare(FTPInfo->CurrentObject, FTPInfo->RootDirectory, BTPS_StringLength(FTPInfo->CurrentObject))) || ((!BTPS_MemCompare(FTPInfo->CurrentObject, FTPInfo->RootDirectory, BTPS_StringLength(FTPInfo->CurrentObject))) && (!FTPInfo->RootIsDrive)))
                           FTPInfo->CurrentObject[BTPS_StringLength(FTPInfo->CurrentObject) - 1] = '\0';

                        FTP_Event_Data.EventData.ServerFileGetIndicationEventData.DirectoryName = FTPInfo->CurrentObject;

                        /* Finally call the Callback.                   */
                        __BTPSTRY
                        {
                           if(FTPInfo->CallbackFunction)
                           {
                              FTPInfo->CallbackActive = TRUE;

                              (*((FTPM_Event_Callback_t)(FTPInfo->CallbackFunction)))(&FTP_Event_Data, FTPInfo->CallbackParameter);

                              FTPInfo->CallbackActive = FALSE;
                           }
                        }
                        __BTPSEXCEPT(1)
                        {
                           /* Do Nothing.                               */
                        }

                        /* Note the Current Object File name in case an */
                        /* Abort Request comes in.                      */
                        BTPS_StringCopy(FTPInfo->CurrentObject, ExtractNameFromObjectInfo(&(OTP_Event_Data->Event_Data.OTP_Get_Object_Request_Data->ObjectInfo)));
                     }

                     /* Before we exit, let's make sure that we note the*/
                     /* correct Current Operation State.                */
                     if(Response != OBEX_CONTINUE_RESPONSE)
                        FTPInfo->CurrentOperation = coNone;
                     else
                        FTPInfo->CurrentOperation = coFileGet;
                  }
                  else
                     Response = OBEX_FORBIDDEN_RESPONSE;

                  /* If we have gotten this far and we have changed the */
                  /* current operation to None AND we have a File       */
                  /* Descriptor open, we better make sure that we close */
                  /* the file.                                          */
                  if((FTPInfo->CurrentOperation == coNone) && (FTPInfo->FileInfo.FileDescriptor))
                  {
                     BTPS_Close_File(FTPInfo->FileInfo.FileDescriptor);

                     FTPInfo->FileInfo.FileDescriptor = NULL;
                  }

                  /* Respond to the Object Request with the correct     */
                  /* calculated response code.                          */
                  _FTPM_Get_Object_Response(FTPInfo->OTPID, OTP_Event_Data->Event_Data.OTP_Get_Object_Request_Data->BufferSize, Response, (unsigned long) NULL);
                  break;
               case etOTP_Put_Object_Request:
                  /* Before continuing any further, let's make sure that*/
                  /* the Client has been granted Write Access.          */
                  if(FTPInfo->ServerPermissions & FTPM_SERVER_PERMISSION_MASK_WRITE)
                  {
                     if(OTP_Event_Data->Event_Data.OTP_Put_Object_Request_Data->Phase & OTP_OBJECT_PHASE_FIRST)
                     {
                        FTPInfo->FileInfo.FirstPhase     = TRUE;
                        FTPInfo->FileInfo.FileIndex      = 0;
                        FTPInfo->FileInfo.FileSize       = OTP_Event_Data->Event_Data.OTP_Put_Object_Request_Data->ObjectInfo.Size;
                        FTPInfo->FileInfo.FileDescriptor = NULL;
                     }

                     /* Let the PutFileData() function take care of the */
                     /* work for us.                                    */
                     Status = PutFileData(FTPInfo, (Boolean_t)((OTP_Event_Data->Event_Data.OTP_Put_Object_Request_Data->Phase & OTP_OBJECT_PHASE_LAST)?TRUE:FALSE), OTP_Event_Data->Event_Data.OTP_Put_Object_Request_Data->DataLength, OTP_Event_Data->Event_Data.OTP_Put_Object_Request_Data->DataPtr, ExtractNameFromObjectInfo(&(OTP_Event_Data->Event_Data.OTP_Put_Object_Request_Data->ObjectInfo)));

                     if(Status == FILE_DATA_FILE_IO_SUCCESS)
                     {
                        Response = (Byte_t)((OTP_Event_Data->Event_Data.OTP_Put_Object_Request_Data->Phase & OTP_OBJECT_PHASE_LAST)?(OBEX_OK_RESPONSE | OBEX_FINAL_BIT):OBEX_CONTINUE_RESPONSE);

                        if((Response != OBEX_CONTINUE_RESPONSE) && (!FTPInfo->FileInfo.FileIndex))
                        {
                           FTP_Event_Data.EventType                                                     = etFTPMFileCreateIndication;
                           FTP_Event_Data.EventLength                                                   = FTPM_SERVER_FILE_CREATE_INDICATION_DATA_SIZE;

                           FTP_Event_Data.EventData.ServerFileCreateIndicationEventData.FTPConnectionID = FTPInfo->FTPConnectionID;
                           FTP_Event_Data.EventData.ServerFileCreateIndicationEventData.CreatedFileName = ExtractNameFromObjectInfo(&(OTP_Event_Data->Event_Data.OTP_Put_Object_Request_Data->ObjectInfo));

                           BTPS_StringCopy(FTPInfo->CurrentObject, FTPInfo->CurrentDirectory);

                           /* Let's check to see if we need to remove   */
                           /* the trailing directory delimiter          */
                           /* characters.                               */
                           if((BTPS_MemCompare(FTPInfo->CurrentObject, FTPInfo->RootDirectory, BTPS_StringLength(FTPInfo->CurrentObject))) || ((!BTPS_MemCompare(FTPInfo->CurrentObject, FTPInfo->RootDirectory, BTPS_StringLength(FTPInfo->CurrentObject))) && (!FTPInfo->RootIsDrive)))
                              FTPInfo->CurrentObject[BTPS_StringLength(FTPInfo->CurrentObject) - 1] = '\0';

                           FTP_Event_Data.EventData.ServerFileCreateIndicationEventData.DirectoryName = FTPInfo->CurrentObject;
                        }
                        else
                        {
                           FTP_Event_Data.EventType                                                    = etFTPMFilePutIndication;
                           FTP_Event_Data.EventLength                                                  = FTPM_SERVER_FILE_PUT_INDICATION_DATA_SIZE;

                           FTP_Event_Data.EventData.ServerFilePutIndicationEventData.FTPConnectionID   = FTPInfo->FTPConnectionID;
                           FTP_Event_Data.EventData.ServerFilePutIndicationEventData.FileName          = ExtractNameFromObjectInfo(&(OTP_Event_Data->Event_Data.OTP_Put_Object_Request_Data->ObjectInfo));
                           FTP_Event_Data.EventData.ServerFilePutIndicationEventData.TransferComplete  = (Boolean_t)(Response != OBEX_CONTINUE_RESPONSE);
                           FTP_Event_Data.EventData.ServerFilePutIndicationEventData.TransferredLength = FTPInfo->FileInfo.FileIndex;
                           FTP_Event_Data.EventData.ServerFilePutIndicationEventData.TotalLength       = FTPInfo->FileInfo.FileSize;

                           BTPS_StringCopy(FTPInfo->CurrentObject, FTPInfo->CurrentDirectory);

                           /* Let's check to see if we need to remove   */
                           /* the trailing directory delimiter          */
                           /* characters.                               */
                           if((BTPS_MemCompare(FTPInfo->CurrentObject, FTPInfo->RootDirectory, BTPS_StringLength(FTPInfo->CurrentObject))) || ((!BTPS_MemCompare(FTPInfo->CurrentObject, FTPInfo->RootDirectory, BTPS_StringLength(FTPInfo->CurrentObject))) && (!FTPInfo->RootIsDrive)))
                              FTPInfo->CurrentObject[BTPS_StringLength(FTPInfo->CurrentObject) - 1] = '\0';

                           FTP_Event_Data.EventData.ServerFilePutIndicationEventData.DirectoryName = FTPInfo->CurrentObject;
                        }

                        /* Finally call the Callback.                   */
                        __BTPSTRY
                        {
                           if(FTPInfo->CallbackFunction)
                           {
                              FTPInfo->CallbackActive = TRUE;

                              (*((FTPM_Event_Callback_t)(FTPInfo->CallbackFunction)))(&FTP_Event_Data, FTPInfo->CallbackParameter);

                              FTPInfo->CallbackActive = FALSE;
                           }
                        }
                        __BTPSEXCEPT(1)
                        {
                           /* Do Nothing.                               */
                        }

                        /* Note the Current Object File name in case an */
                        /* Abort Request comes in.                      */
                        BTPS_StringCopy(FTPInfo->CurrentObject, ExtractNameFromObjectInfo(&(OTP_Event_Data->Event_Data.OTP_Put_Object_Request_Data->ObjectInfo)));
                     }
                     else
                     {
                        Response = (Byte_t)(OBEX_INTERNAL_SERVER_ERROR_RESPONSE | OBEX_FINAL_BIT);

                        /* If the File is still open, let's make sure   */
                        /* that we force it closed (as there was an     */
                        /* error).                                      */
                        if(FTPInfo->FileInfo.FileDescriptor)
                        {
                           BTPS_Close_File(FTPInfo->FileInfo.FileDescriptor);

                           FTPInfo->FileInfo.FileDescriptor = NULL;
                        }

                        if(!FTPInfo->FileInfo.FirstPhase)
                        {
                           FTP_Event_Data.EventType                                                    = etFTPMFilePutIndication;
                           FTP_Event_Data.EventLength                                                  = FTPM_SERVER_FILE_PUT_INDICATION_DATA_SIZE;

                           FTP_Event_Data.EventData.ServerFilePutIndicationEventData.FTPConnectionID   = FTPInfo->FTPConnectionID;
                           FTP_Event_Data.EventData.ServerFilePutIndicationEventData.FileName          = ExtractNameFromObjectInfo(&(OTP_Event_Data->Event_Data.OTP_Put_Object_Request_Data->ObjectInfo));
                           FTP_Event_Data.EventData.ServerFilePutIndicationEventData.TransferComplete  = TRUE;
                           FTP_Event_Data.EventData.ServerFilePutIndicationEventData.TransferredLength = FTPInfo->FileInfo.FileIndex;
                           FTP_Event_Data.EventData.ServerFilePutIndicationEventData.TotalLength       = FTPInfo->FileInfo.FileSize;

                           BTPS_StringCopy(FTPInfo->CurrentObject, FTPInfo->CurrentDirectory);

                           /* Let's check to see if we need to remove   */
                           /* the trailing directory delimiter          */
                           /* characters.                               */
                           if((BTPS_MemCompare(FTPInfo->CurrentObject, FTPInfo->RootDirectory, BTPS_StringLength(FTPInfo->CurrentObject))) || ((!BTPS_MemCompare(FTPInfo->CurrentObject, FTPInfo->RootDirectory, BTPS_StringLength(FTPInfo->CurrentObject))) && (!FTPInfo->RootIsDrive)))
                              FTPInfo->CurrentObject[BTPS_StringLength(FTPInfo->CurrentObject) - 1] = '\0';

                           FTP_Event_Data.EventData.ServerFilePutIndicationEventData.DirectoryName = FTPInfo->CurrentObject;

                           /* Finally call the Callback.                */
                           __BTPSTRY
                           {
                              if(FTPInfo->CallbackFunction)
                              {
                                 FTPInfo->CallbackActive = TRUE;

                                 (*((FTPM_Event_Callback_t)(FTPInfo->CallbackFunction)))(&FTP_Event_Data, FTPInfo->CallbackParameter);

                                 FTPInfo->CallbackActive = FALSE;
                              }
                           }
                           __BTPSEXCEPT(1)
                           {
                              /* Do Nothing.                            */
                           }

                           /* Note the Current Object File name in case */
                           /* an Abort Request comes in.                */
                           BTPS_StringCopy(FTPInfo->CurrentObject, ExtractNameFromObjectInfo(&(OTP_Event_Data->Event_Data.OTP_Put_Object_Request_Data->ObjectInfo)));
                        }
                     }

                     if(Response & OBEX_FINAL_BIT)
                        FTPInfo->CurrentOperation = coNone;
                     else
                        FTPInfo->CurrentOperation = coFilePut;

                  }
                  else
                     Response = OBEX_FORBIDDEN_RESPONSE;

                  /* If we have gotten this far and we have changed the */
                  /* current operation to None AND we have a File       */
                  /* Descriptor open, we better make sure that we close */
                  /* the file.                                          */
                  if((FTPInfo->CurrentOperation == coNone) && (FTPInfo->FileInfo.FileDescriptor))
                  {
                     BTPS_Close_File(FTPInfo->FileInfo.FileDescriptor);

                     FTPInfo->FileInfo.FileDescriptor = NULL;
                  }

                  /* Respond to the Object Request with the correct     */
                  /* calculated response code.                          */
                  _FTPM_Put_Object_Response(FTPInfo->OTPID, Response);
                  break;
               case etOTP_Free_Directory_Information:
                  /* Loop through each of the entries.                  */
                  for(Index=0;Index<OTP_Event_Data->Event_Data.OTP_Free_Directory_Information_Data->DirectoryInfo.NumberEntries;Index++)
                  {
                     /* Check to see if this entry contained an extended*/
                     /* Name member.  If so free the memory that was    */
                     /* allocated for this response.                    */
                     if(OTP_Event_Data->Event_Data.OTP_Free_Directory_Information_Data->DirectoryInfo.ObjectInfo[Index].FieldMask & OTP_OBJECT_INFO_MASK_EXTENDED_NAME)
                        BTPS_FreeMemory((void *)READ_OBJECT_INFO_EXTENDED_NAME(OTP_Event_Data->Event_Data.OTP_Free_Directory_Information_Data->DirectoryInfo.ObjectInfo[Index].Name));
                  }

                  /* Simply go ahead and free the memory we allocated   */
                  /* for our directory information structure.           */
                  if(OTP_Event_Data->Event_Data.OTP_Free_Directory_Information_Data->DirectoryInfo.ObjectInfo)
                     BTPS_FreeMemory(OTP_Event_Data->Event_Data.OTP_Free_Directory_Information_Data->DirectoryInfo.ObjectInfo);

                  FTPInfo->CurrentOperation = coNone;
                  break;
               case etOTP_Set_Path_Request:
                  /* Before we change the current directory, let's make */
                  /* a backup of the directory we are currently in.     */
                  BTPS_StringCopy(FTPInfo->CurrentObject, FTPInfo->CurrentDirectory);

                  /* If the Client is trying to create the specified    */
                  /* directory, then we need to make sure that the      */
                  /* Client has Write Access.                           */
                  if((!OTP_Event_Data->Event_Data.OTP_Set_Path_Request_Data->Create) || ((OTP_Event_Data->Event_Data.OTP_Set_Path_Request_Data->Create) && (FTPInfo->ServerPermissions & FTPM_SERVER_PERMISSION_MASK_WRITE)))
                  {
                     /* Attempt to set the path to the specified path.  */
                     Status = SetCurrentPath(FTPInfo, OTP_Event_Data->Event_Data.OTP_Set_Path_Request_Data->Backup, OTP_Event_Data->Event_Data.OTP_Set_Path_Request_Data->Create, OTP_Event_Data->Event_Data.OTP_Set_Path_Request_Data->Folder);

                     /* Respond to the Set Path Request based upon      */
                     /* whether or not we could actually set the path.  */
                     if(Status == SET_PATH_ERROR)
                        Response = OBEX_FORBIDDEN_RESPONSE;
                     else
                     {
                        if(Status == SET_PATH_ERROR_INVALID_PATH)
                           Response = OBEX_NOT_FOUND_RESPONSE;
                        else
                           Response = OBEX_OK_RESPONSE;
                     }

                     _FTPM_Set_Path_Response(FTPInfo->OTPID, (Byte_t)Response);

                     /* If the Set Path Request was successful, notify  */
                     /* the FTP Server Callback of the Event.           */
                     if((Status != SET_PATH_ERROR) && (Status != SET_PATH_ERROR_INVALID_PATH))
                     {
                        /* If a Directory was created then we need to   */
                        /* inform the FTP Server Event Callback of it.  */
                        if((!OTP_Event_Data->Event_Data.OTP_Set_Path_Request_Data->Backup) && (OTP_Event_Data->Event_Data.OTP_Set_Path_Request_Data->Create))
                        {
                           FTP_Event_Data.EventType                                                               = etFTPMDirectoryCreateIndication;
                           FTP_Event_Data.EventLength                                                             = FTPM_SERVER_DIRECTORY_CREATE_INDICATION_DATA_SIZE;

                           FTP_Event_Data.EventData.ServerDirectoryCreateIndicationEventData.FTPConnectionID      = FTPInfo->FTPConnectionID;
                           FTP_Event_Data.EventData.ServerDirectoryCreateIndicationEventData.CreatedDirectoryName = OTP_Event_Data->Event_Data.OTP_Set_Path_Request_Data->Folder;

                           /* Let's check to see if we need to remove   */
                           /* the trailing directory delimiter          */
                           /* characters.                               */
                           /* * NOTE * We made a backup of this         */
                           /*          directory earlier.               */
                           if((BTPS_MemCompare(FTPInfo->CurrentObject, FTPInfo->RootDirectory, BTPS_StringLength(FTPInfo->CurrentObject))) || ((!BTPS_MemCompare(FTPInfo->CurrentObject, FTPInfo->RootDirectory, BTPS_StringLength(FTPInfo->CurrentObject))) && (!FTPInfo->RootIsDrive)))
                              FTPInfo->CurrentObject[BTPS_StringLength(FTPInfo->CurrentObject) - 1] = '\0';

                           FTP_Event_Data.EventData.ServerDirectoryCreateIndicationEventData.DirectoryName = FTPInfo->CurrentObject;

                           __BTPSTRY
                           {
                              if(FTPInfo->CallbackFunction)
                              {
                                 FTPInfo->CallbackActive = TRUE;

                                 (*((FTPM_Event_Callback_t)(FTPInfo->CallbackFunction)))(&FTP_Event_Data, FTPInfo->CallbackParameter);

                                 FTPInfo->CallbackActive = FALSE;
                              }
                           }
                           __BTPSEXCEPT(1)
                           {
                              /* Do Nothing.                            */
                           }
                        }

                        /* Regardless of whether or not a Directory was */
                        /* created, we need to dispatch a Change        */
                        /* Directory Request because we are now in a new*/
                        /* directory.                                   */
                        FTP_Event_Data.EventType                                                          = etFTPMChangeDirectoryIndication;
                        FTP_Event_Data.EventLength                                                        = FTPM_SERVER_CHANGE_DIRECTORY_INDICATION_DATA_SIZE;

                        FTP_Event_Data.EventData.ServerChangeDirectoryIndicationEventData.FTPConnectionID = FTPInfo->FTPConnectionID;

                        BTPS_StringCopy(FTPInfo->CurrentObject, FTPInfo->CurrentDirectory);

                        /* Let's check to see if we need to remove the  */
                        /* trailing directory delimiter character.      */
                        if((BTPS_MemCompare(FTPInfo->CurrentObject, FTPInfo->RootDirectory, BTPS_StringLength(FTPInfo->CurrentObject))) || ((!BTPS_MemCompare(FTPInfo->CurrentObject, FTPInfo->RootDirectory, BTPS_StringLength(FTPInfo->CurrentObject))) && (!FTPInfo->RootIsDrive)))
                           FTPInfo->CurrentObject[BTPS_StringLength(FTPInfo->CurrentObject) - 1] = '\0';

                        FTP_Event_Data.EventData.ServerChangeDirectoryIndicationEventData.DirectoryName = FTPInfo->CurrentObject;

                        /* Finally call the Callback.                   */
                        __BTPSTRY
                        {
                           if(FTPInfo->CallbackFunction)
                           {
                              FTPInfo->CallbackActive = TRUE;

                              (*((FTPM_Event_Callback_t)(FTPInfo->CallbackFunction)))(&FTP_Event_Data, FTPInfo->CallbackParameter);

                              FTPInfo->CallbackActive = FALSE;
                           }
                        }
                        __BTPSEXCEPT(1)
                        {
                           /* Do Nothing.                               */
                        }
                     }
                  }
                  else
                     _FTPM_Set_Path_Response(FTPInfo->OTPID, (Byte_t)OBEX_FORBIDDEN_RESPONSE);
                  break;
               case etOTP_Abort_Request:
                  /* Go ahead and send an OBEX Abort Response.          */
                  _FTPM_Abort_Response(FTPInfo->OTPID);

                  /* Current operation was aborted, go ahead and close  */
                  /* any open files.                                    */
                  if(FTPInfo->FileInfo.FileDescriptor)
                  {
                     BTPS_Close_File(FTPInfo->FileInfo.FileDescriptor);

                     FTPInfo->FileInfo.FileDescriptor = NULL;
                  }

                  /* If we were in the middle of a Put or Get Operation */
                  /* then we need to inform the FTP Server Event        */
                  /* Callback that the transfer is now complete.        */
                  if(FTPInfo->CurrentOperation == coFileGet)
                  {
                     FTP_Event_Data.EventType                                                    = etFTPMFileGetIndication;
                     FTP_Event_Data.EventLength                                                  = FTPM_SERVER_FILE_GET_INDICATION_DATA_SIZE;

                     FTP_Event_Data.EventData.ServerFileGetIndicationEventData.FTPConnectionID   = FTPInfo->FTPConnectionID;
                     FTP_Event_Data.EventData.ServerFileGetIndicationEventData.FileName          = FTPInfo->CurrentObject;
                     FTP_Event_Data.EventData.ServerFileGetIndicationEventData.TransferComplete  = TRUE;
                     FTP_Event_Data.EventData.ServerFileGetIndicationEventData.TransferredLength = FTPInfo->FileInfo.FileIndex;
                     FTP_Event_Data.EventData.ServerFileGetIndicationEventData.TotalLength       = FTPInfo->FileInfo.FileSize;

                     /* Note that we do not have to format up the       */
                     /* Filename because it will still be in the        */
                     /* CurrentObject Buffer.                           */

                     /* We will have to use the Current Directory Buffer*/
                     /* for the Current Directory Buffer.  This means we*/
                     /* might have to remove the Trailing directory     */
                     /* delimiter character (and put it back when we are*/
                     /* finished).                                      */
                     FTP_Event_Data.EventData.ServerFileGetIndicationEventData.DirectoryName     = FTPInfo->CurrentDirectory;

                     if((BTPS_MemCompare(FTPInfo->CurrentDirectory, FTPInfo->RootDirectory, BTPS_StringLength(FTPInfo->CurrentDirectory))) || ((!BTPS_MemCompare(FTPInfo->CurrentDirectory, FTPInfo->RootDirectory, BTPS_StringLength(FTPInfo->CurrentDirectory))) && (!FTPInfo->RootIsDrive)))
                     {
                        CurrentLength = BTPS_StringLength(FTPInfo->CurrentDirectory);

                        FTPInfo->CurrentDirectory[BTPS_StringLength(FTPInfo->CurrentDirectory) - 1] = '\0';
                     }
                     else
                        CurrentLength = 0;

                     /* Finally call the Callback.                      */
                     __BTPSTRY
                     {
                        if(FTPInfo->CallbackFunction)
                        {
                           FTPInfo->CallbackActive = TRUE;

                           (*((FTPM_Event_Callback_t)(FTPInfo->CallbackFunction)))(&FTP_Event_Data, FTPInfo->CallbackParameter);

                           FTPInfo->CallbackActive = FALSE;
                        }
                     }
                     __BTPSEXCEPT(1)
                     {
                        /* Do Nothing.                                  */
                     }

                     /* Restore the Current Directory.                  */
                     if(CurrentLength)
                        FTPInfo->CurrentDirectory[CurrentLength - 1] = BTPS_DIRECTORY_DELIMITER_CHARACTER;
                  }
                  else
                  {
                     if(FTPInfo->CurrentOperation == coFilePut)
                     {
                        FTP_Event_Data.EventType                                                    = etFTPMFilePutIndication;
                        FTP_Event_Data.EventLength                                                  = FTPM_SERVER_FILE_PUT_INDICATION_DATA_SIZE;

                        FTP_Event_Data.EventData.ServerFilePutIndicationEventData.FTPConnectionID   = FTPInfo->FTPConnectionID;
                        FTP_Event_Data.EventData.ServerFilePutIndicationEventData.FileName          = FTPInfo->CurrentObject;
                        FTP_Event_Data.EventData.ServerFilePutIndicationEventData.TransferComplete  = TRUE;
                        FTP_Event_Data.EventData.ServerFilePutIndicationEventData.TransferredLength = FTPInfo->FileInfo.FileIndex;
                        FTP_Event_Data.EventData.ServerFilePutIndicationEventData.TotalLength       = FTPInfo->FileInfo.FileSize;

                        /* Note that we do not have to format up the    */
                        /* Filename because it will still be in the     */
                        /* CurrentObject Buffer.                        */

                        /* We will have to use the Current Directory    */
                        /* Buffer for the Current Directory Buffer.     */
                        /* This means we might have to remove the       */
                        /* Trailing directory delimiter character (and  */
                        /* put it back when we are finished).           */
                        FTP_Event_Data.EventData.ServerFilePutIndicationEventData.DirectoryName     = FTPInfo->CurrentDirectory;

                        if((BTPS_MemCompare(FTPInfo->CurrentDirectory, FTPInfo->RootDirectory, BTPS_StringLength(FTPInfo->CurrentDirectory))) || ((!BTPS_MemCompare(FTPInfo->CurrentDirectory, FTPInfo->RootDirectory, BTPS_StringLength(FTPInfo->CurrentDirectory))) && (!FTPInfo->RootIsDrive)))
                        {
                           CurrentLength = BTPS_StringLength(FTPInfo->CurrentDirectory);

                           FTPInfo->CurrentDirectory[BTPS_StringLength(FTPInfo->CurrentDirectory) - 1] = '\0';
                        }
                        else
                           CurrentLength = 0;

                        /* Finally call the Callback.                   */
                        __BTPSTRY
                        {
                           if(FTPInfo->CallbackFunction)
                           {
                              FTPInfo->CallbackActive = TRUE;

                              (*((FTPM_Event_Callback_t)(FTPInfo->CallbackFunction)))(&FTP_Event_Data, FTPInfo->CallbackParameter);

                              FTPInfo->CallbackActive = FALSE;
                           }
                        }
                        __BTPSEXCEPT(1)
                        {
                           /* Do Nothing.                               */
                        }

                        /* Restore the Current Directory.               */
                        if(CurrentLength)
                           FTPInfo->CurrentDirectory[CurrentLength - 1] = BTPS_DIRECTORY_DELIMITER_CHARACTER;
                     }
                  }

                  /* Flag the current Operation State as none           */
                  /* outstanding.                                       */
                  FTPInfo->CurrentOperation = coNone;
                  break;
               default:
                  /* We are not interested in any other FTPM Mgr Events.*/
                  break;
            }
         }
         else
         {
            switch(OTP_Event_Data->Event_Data_Type)
            {
               case etOTP_Port_Open_Confirmation:
                  if(OTP_Event_Data->Event_Data.OTP_Port_Open_Confirmation_Data->PortOpenStatus == SPPM_OPEN_REMOTE_PORT_STATUS_SUCCESS)
                  {
                     if(_FTPM_Client_Connect(FTPInfo->OTPID, tFileBrowser, NULL, NULL))
                     {
                        FTP_Event_Data.EventType                                                        = etFTPMConnectConfirmation;
                        FTP_Event_Data.EventLength                                                      = FTPM_CLIENT_CONNECT_CONFIRMATION_DATA_SIZE;

                        FTP_Event_Data.EventData.ClientConnectConfirmationEventData.FTPConnectionID     = FTPInfo->FTPConnectionID;
                        FTP_Event_Data.EventData.ClientConnectConfirmationEventData.RemoteDeviceAddress = FTPInfo->BD_ADDR;
                        FTP_Event_Data.EventData.ClientConnectConfirmationEventData.FTPConnectStatus    = SPPM_OPEN_REMOTE_PORT_STATUS_FAILURE_UNKNOWN;

                        /* Now inform the registered caller that the    */
                        /* Connection was not established.              */
                        __BTPSTRY
                        {
                           if(FTPInfo->CallbackFunction)
                           {
                              FTPInfo->CallbackActive = TRUE;

                              (*((FTPM_Event_Callback_t)(FTPInfo->CallbackFunction)))(&FTP_Event_Data, FTPInfo->CallbackParameter);

                              FTPInfo->CallbackActive = FALSE;
                           }
                        }
                        __BTPSEXCEPT(1)
                        {
                           /* Do Nothing.                               */
                        }

                        /* Since there was a connection error we need to*/
                        /* flag the entry for deletion.                 */
                        DeleteEntry = TRUE;
                     }
                  }
                  else
                  {
                     FTP_Event_Data.EventType                                                        = etFTPMConnectConfirmation;
                     FTP_Event_Data.EventLength                                                      = FTPM_CLIENT_CONNECT_CONFIRMATION_DATA_SIZE;

                     FTP_Event_Data.EventData.ClientConnectConfirmationEventData.FTPConnectionID     = FTPInfo->FTPConnectionID;
                     FTP_Event_Data.EventData.ClientConnectConfirmationEventData.RemoteDeviceAddress = FTPInfo->BD_ADDR;
                     FTP_Event_Data.EventData.ClientConnectConfirmationEventData.FTPConnectStatus    = OTP_Event_Data->Event_Data.OTP_Port_Open_Confirmation_Data->PortOpenStatus;

                     /* Now inform the registered caller that the       */
                     /* Connection was not established.                 */
                     __BTPSTRY
                     {
                        if(FTPInfo->CallbackFunction)
                        {
                           FTPInfo->CallbackActive = TRUE;

                           (*((FTPM_Event_Callback_t)(FTPInfo->CallbackFunction)))(&FTP_Event_Data, FTPInfo->CallbackParameter);

                           FTPInfo->CallbackActive = FALSE;
                        }
                     }
                     __BTPSEXCEPT(1)
                     {
                        /* Do Nothing.                                  */
                     }

                     /* Since there was a connection error we need to   */
                     /* flag the entry for deletion.                    */
                     DeleteEntry = TRUE;
                  }
                  break;
               case etOTP_Connect_Response:
                  Response = (Byte_t)(OTP_Event_Data->Event_Data.OTP_Connect_Response_Data->ResponseCode & ~OBEX_FINAL_BIT);
                  if((Response == OBEX_OK_RESPONSE) && (OTP_Event_Data->Event_Data.OTP_Connect_Response_Data->Target == tFileBrowser))
                  {
                     /* Flag the we have a Remote Connection to the     */
                     /* Remote Server.                                  */
                     FTPInfo->ConnectionState = csConnected;

                     /* Flag that there are NO outstanding FTP          */
                     /* Operations.                                     */
                     FTPInfo->CurrentOperation = coNone;

                     /* Need to inform the Client that we have a        */
                     /* successful connection.                          */
                     FTP_Event_Data.EventType                                                        = etFTPMConnectConfirmation;
                     FTP_Event_Data.EventLength                                                      = FTPM_CLIENT_CONNECT_CONFIRMATION_DATA_SIZE;

                     FTP_Event_Data.EventData.ClientConnectConfirmationEventData.FTPConnectionID     = FTPInfo->FTPConnectionID;
                     FTP_Event_Data.EventData.ClientConnectConfirmationEventData.RemoteDeviceAddress = FTPInfo->BD_ADDR;
                     FTP_Event_Data.EventData.ClientConnectConfirmationEventData.FTPConnectStatus    = SPPM_OPEN_REMOTE_PORT_STATUS_SUCCESS;

                     /* Now inform the registered caller that the       */
                     /* Connection was established.                     */
                     __BTPSTRY
                     {
                        if(FTPInfo->CallbackFunction)
                        {
                           FTPInfo->CallbackActive = TRUE;

                           (*((FTPM_Event_Callback_t)(FTPInfo->CallbackFunction)))(&FTP_Event_Data, FTPInfo->CallbackParameter);

                           FTPInfo->CallbackActive = FALSE;
                        }
                     }
                     __BTPSEXCEPT(1)
                     {
                        /* Do Nothing.                                  */
                     }
                  }
                  else
                  {
                     /* Need to inform the Client that we have not made */
                     /* a successful connection.                        */
                     FTP_Event_Data.EventType                                                        = etFTPMConnectConfirmation;
                     FTP_Event_Data.EventLength                                                      = FTPM_CLIENT_CONNECT_CONFIRMATION_DATA_SIZE;

                     FTP_Event_Data.EventData.ClientConnectConfirmationEventData.FTPConnectionID     = FTPInfo->FTPConnectionID;
                     FTP_Event_Data.EventData.ClientConnectConfirmationEventData.RemoteDeviceAddress = FTPInfo->BD_ADDR;
                     FTP_Event_Data.EventData.ClientConnectConfirmationEventData.FTPConnectStatus    = SPPM_OPEN_REMOTE_PORT_STATUS_FAILURE_UNKNOWN;

                     /* Now inform the registered caller that the       */
                     /* Connection was not established.                 */
                     __BTPSTRY
                     {
                        if(FTPInfo->CallbackFunction)
                        {
                           FTPInfo->CallbackActive = TRUE;

                           (*((FTPM_Event_Callback_t)(FTPInfo->CallbackFunction)))(&FTP_Event_Data, FTPInfo->CallbackParameter);

                           FTPInfo->CallbackActive = FALSE;
                        }
                     }
                     __BTPSEXCEPT(1)
                     {
                        /* Do Nothing.                                  */
                     }

                     /* Since there was a connection error we need to   */
                     /* flag the entry for deletion.                    */
                     DeleteEntry = TRUE;
                  }
                  break;
               case etOTP_Disconnect_Request:
                  /* Remote side wants to disconnect for some reason, so*/
                  /* let's inform the user and delete the entry.        */

                  /* Note the fall-through to the Disconnect response.  */
               case etOTP_Disconnect_Response:

                  /* Note the fall-through to the Port Close            */
                  /* Disconnection.                                     */
               case etOTP_Port_Close_Indication:
                  /* Disconnected, go ahead and close any open files.   */
                  if(FTPInfo->FileInfo.FileDescriptor)
                  {
                     BTPS_Close_File(FTPInfo->FileInfo.FileDescriptor);

                     FTPInfo->FileInfo.FileDescriptor = NULL;
                  }

                  FTP_Event_Data.EventType                                               = etFTPMDisconnectIndication;
                  FTP_Event_Data.EventLength                                             = FTPM_DISCONNECT_INDICATION_DATA_SIZE;

                  FTP_Event_Data.EventData.DisconnectIndicationEventData.FTPConnectionID = FTPInfo->FTPConnectionID;

                  /* Now inform the registered caller that the          */
                  /* Connection is no longer established.               */
                  __BTPSTRY
                  {
                     if(FTPInfo->CallbackFunction)
                     {
                        FTPInfo->CallbackActive = TRUE;

                        (*((FTPM_Event_Callback_t)(FTPInfo->CallbackFunction)))(&FTP_Event_Data, FTPInfo->CallbackParameter);

                        FTPInfo->CallbackActive = FALSE;
                     }
                  }
                  __BTPSEXCEPT(1)
                  {
                     /* Do Nothing.                                     */
                  }

                  /* Flag Entry for Deletion.                           */
                  DeleteEntry = TRUE;
                  break;
               case etOTP_Get_Directory_Response:
                  /* Flag that there is currently no outstanding        */
                  /* operation.                                         */
                  FTPInfo->CurrentOperation = coNone;

                  /* Next we need to map the FTPM Mgr Directory Response*/
                  /* Information to FTP Directory Response Information. */
                  FTP_Event_Data.EventType                                                                    = etFTPMDirectoryRequestConfirmation;
                  FTP_Event_Data.EventLength                                                                  = FTPM_CLIENT_DIRECTORY_REQUEST_CONFIRMATION_DATA_SIZE;

                  FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.FTPConnectionID        = FTPInfo->FTPConnectionID;
                  FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.ParentDirectory        = OTP_Event_Data->Event_Data.OTP_Get_Directory_Response_Data->DirInfo.ParentDirectory;
                  FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.DirectoryName          = BTPS_StringLength(FTPInfo->CurrentObject)?FTPInfo->CurrentObject:NULL;
                  FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.NumberDirectoryEntries = 0;
                  FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.FTPDirectoryEntries    = NULL;

                  /* Next, let's map the Directory Information Entries  */
                  /* from the FTPM Mgr Layer to the FTP Layer.          */
                  /* * NOTE * If we can't allocate the required memory  */
                  /*          we will just call the client back with no */
                  /*          entries.                                  */
                  if(OTP_Event_Data->Event_Data.OTP_Get_Directory_Response_Data->DirInfo.NumberEntries)
                  {
                     if((FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.FTPDirectoryEntries = (FTPM_Directory_Entry_t *)BTPS_AllocateMemory(OTP_Event_Data->Event_Data.OTP_Get_Directory_Response_Data->DirInfo.NumberEntries*FTPM_DIRECTORY_ENTRY_SIZE)) != NULL)
                     {
                        FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.NumberDirectoryEntries = OTP_Event_Data->Event_Data.OTP_Get_Directory_Response_Data->DirInfo.NumberEntries;
                        for(Index=0;Index<OTP_Event_Data->Event_Data.OTP_Get_Directory_Response_Data->DirInfo.NumberEntries;Index++)
                        {
                           /* Flag whether or not this is a Directory   */
                           /* Entry or not.                             */
                           if(OTP_Event_Data->Event_Data.OTP_Get_Directory_Response_Data->DirInfo.ObjectInfo[Index].ObjectType == otFolder)
                              FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.FTPDirectoryEntries[Index].DirectoryEntry = TRUE;
                           else
                              FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.FTPDirectoryEntries[Index].DirectoryEntry = FALSE;

                           FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.FTPDirectoryEntries[Index].FieldMask = FTPM_OBJECT_INFO_MASK_CLEAR;

                           if(OTP_Event_Data->Event_Data.OTP_Get_Directory_Response_Data->DirInfo.ObjectInfo[Index].FieldMask & FTPM_OBJECT_INFO_MASK_NAME)
                           {
                              FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.FTPDirectoryEntries[Index].FieldMask  |= FTPM_OBJECT_INFO_MASK_NAME;
                              FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.FTPDirectoryEntries[Index].NameLength  = ExtractNameLengthFromObjectInfo(&(OTP_Event_Data->Event_Data.OTP_Get_Directory_Response_Data->DirInfo.ObjectInfo[Index]));

                              if(FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.FTPDirectoryEntries[Index].NameLength)
                                 FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.FTPDirectoryEntries[Index].Name = ExtractNameFromObjectInfo(&(OTP_Event_Data->Event_Data.OTP_Get_Directory_Response_Data->DirInfo.ObjectInfo[Index]));
                              else
                                 FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.FTPDirectoryEntries[Index].Name = NULL;
                           }

                           if(OTP_Event_Data->Event_Data.OTP_Get_Directory_Response_Data->DirInfo.ObjectInfo[Index].FieldMask & FTPM_OBJECT_INFO_MASK_SIZE)
                           {
                              FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.FTPDirectoryEntries[Index].FieldMask |= FTPM_OBJECT_INFO_MASK_SIZE;
                              FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.FTPDirectoryEntries[Index].Size       = OTP_Event_Data->Event_Data.OTP_Get_Directory_Response_Data->DirInfo.ObjectInfo[Index].Size;
                           }

                           if(OTP_Event_Data->Event_Data.OTP_Get_Directory_Response_Data->DirInfo.ObjectInfo[Index].FieldMask & FTPM_OBJECT_INFO_MASK_TYPE)
                           {
                              FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.FTPDirectoryEntries[Index].FieldMask  |= FTPM_OBJECT_INFO_MASK_TYPE;
                              FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.FTPDirectoryEntries[Index].TypeLength  = OTP_Event_Data->Event_Data.OTP_Get_Directory_Response_Data->DirInfo.ObjectInfo[Index].TypeLength;

                              if(FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.FTPDirectoryEntries[Index].TypeLength)
                                 FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.FTPDirectoryEntries[Index].Type = OTP_Event_Data->Event_Data.OTP_Get_Directory_Response_Data->DirInfo.ObjectInfo[Index].Type;
                              else
                                 FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.FTPDirectoryEntries[Index].Type = NULL;
                           }

                           if(OTP_Event_Data->Event_Data.OTP_Get_Directory_Response_Data->DirInfo.ObjectInfo[Index].FieldMask & FTPM_OBJECT_INFO_MASK_MODIFIED)
                           {
                              FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.FTPDirectoryEntries[Index].FieldMask        |= FTPM_OBJECT_INFO_MASK_MODIFIED;
                              FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.FTPDirectoryEntries[Index].Modified.Year     = OTP_Event_Data->Event_Data.OTP_Get_Directory_Response_Data->DirInfo.ObjectInfo[Index].Modified.Year;
                              FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.FTPDirectoryEntries[Index].Modified.Month    = OTP_Event_Data->Event_Data.OTP_Get_Directory_Response_Data->DirInfo.ObjectInfo[Index].Modified.Month;
                              FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.FTPDirectoryEntries[Index].Modified.Day      = OTP_Event_Data->Event_Data.OTP_Get_Directory_Response_Data->DirInfo.ObjectInfo[Index].Modified.Day;
                              FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.FTPDirectoryEntries[Index].Modified.Hour     = OTP_Event_Data->Event_Data.OTP_Get_Directory_Response_Data->DirInfo.ObjectInfo[Index].Modified.Hour;
                              FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.FTPDirectoryEntries[Index].Modified.Minute   = OTP_Event_Data->Event_Data.OTP_Get_Directory_Response_Data->DirInfo.ObjectInfo[Index].Modified.Minute;
                              FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.FTPDirectoryEntries[Index].Modified.Second   = OTP_Event_Data->Event_Data.OTP_Get_Directory_Response_Data->DirInfo.ObjectInfo[Index].Modified.Second;
                              FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.FTPDirectoryEntries[Index].Modified.UTC_Time = OTP_Event_Data->Event_Data.OTP_Get_Directory_Response_Data->DirInfo.ObjectInfo[Index].Modified.UTC_Time;
                           }

                           if(OTP_Event_Data->Event_Data.OTP_Get_Directory_Response_Data->DirInfo.ObjectInfo[Index].FieldMask & FTPM_OBJECT_INFO_MASK_CREATED)
                           {
                              FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.FTPDirectoryEntries[Index].FieldMask        |= FTPM_OBJECT_INFO_MASK_CREATED;
                              FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.FTPDirectoryEntries[Index].Created.Year     = OTP_Event_Data->Event_Data.OTP_Get_Directory_Response_Data->DirInfo.ObjectInfo[Index].Created.Year;
                              FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.FTPDirectoryEntries[Index].Created.Month    = OTP_Event_Data->Event_Data.OTP_Get_Directory_Response_Data->DirInfo.ObjectInfo[Index].Created.Month;
                              FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.FTPDirectoryEntries[Index].Created.Day      = OTP_Event_Data->Event_Data.OTP_Get_Directory_Response_Data->DirInfo.ObjectInfo[Index].Created.Day;
                              FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.FTPDirectoryEntries[Index].Created.Hour     = OTP_Event_Data->Event_Data.OTP_Get_Directory_Response_Data->DirInfo.ObjectInfo[Index].Created.Hour;
                              FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.FTPDirectoryEntries[Index].Created.Minute   = OTP_Event_Data->Event_Data.OTP_Get_Directory_Response_Data->DirInfo.ObjectInfo[Index].Created.Minute;
                              FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.FTPDirectoryEntries[Index].Created.Second   = OTP_Event_Data->Event_Data.OTP_Get_Directory_Response_Data->DirInfo.ObjectInfo[Index].Created.Second;
                              FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.FTPDirectoryEntries[Index].Created.UTC_Time = OTP_Event_Data->Event_Data.OTP_Get_Directory_Response_Data->DirInfo.ObjectInfo[Index].Created.UTC_Time;
                           }

                           if(OTP_Event_Data->Event_Data.OTP_Get_Directory_Response_Data->DirInfo.ObjectInfo[Index].FieldMask & FTPM_OBJECT_INFO_MASK_ACCESSED)
                           {
                              FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.FTPDirectoryEntries[Index].FieldMask        |= FTPM_OBJECT_INFO_MASK_ACCESSED;
                              FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.FTPDirectoryEntries[Index].Accessed.Year     = OTP_Event_Data->Event_Data.OTP_Get_Directory_Response_Data->DirInfo.ObjectInfo[Index].Accessed.Year;
                              FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.FTPDirectoryEntries[Index].Accessed.Month    = OTP_Event_Data->Event_Data.OTP_Get_Directory_Response_Data->DirInfo.ObjectInfo[Index].Accessed.Month;
                              FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.FTPDirectoryEntries[Index].Accessed.Day      = OTP_Event_Data->Event_Data.OTP_Get_Directory_Response_Data->DirInfo.ObjectInfo[Index].Accessed.Day;
                              FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.FTPDirectoryEntries[Index].Accessed.Hour     = OTP_Event_Data->Event_Data.OTP_Get_Directory_Response_Data->DirInfo.ObjectInfo[Index].Accessed.Hour;
                              FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.FTPDirectoryEntries[Index].Accessed.Minute   = OTP_Event_Data->Event_Data.OTP_Get_Directory_Response_Data->DirInfo.ObjectInfo[Index].Accessed.Minute;
                              FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.FTPDirectoryEntries[Index].Accessed.Second   = OTP_Event_Data->Event_Data.OTP_Get_Directory_Response_Data->DirInfo.ObjectInfo[Index].Accessed.Second;
                              FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.FTPDirectoryEntries[Index].Accessed.UTC_Time = OTP_Event_Data->Event_Data.OTP_Get_Directory_Response_Data->DirInfo.ObjectInfo[Index].Accessed.UTC_Time;
                           }

                           /* Initialize that there are no Permission   */
                           /* Flags.                                    */
                           FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.FTPDirectoryEntries[Index].PermissionMask   = 0;

                           if(OTP_Event_Data->Event_Data.OTP_Get_Directory_Response_Data->DirInfo.ObjectInfo[Index].FieldMask & FTPM_OBJECT_INFO_MASK_USER_PERMISSION)
                           {
                              FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.FTPDirectoryEntries[Index].FieldMask |= FTPM_OBJECT_INFO_MASK_USER_PERMISSION;

                              if(OTP_Event_Data->Event_Data.OTP_Get_Directory_Response_Data->DirInfo.ObjectInfo[Index].Permission & OTP_USER_PERMISSION_READ)
                                 FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.FTPDirectoryEntries[Index].PermissionMask |= (unsigned long)FTPM_USER_PERMISSION_READ;

                              if(OTP_Event_Data->Event_Data.OTP_Get_Directory_Response_Data->DirInfo.ObjectInfo[Index].Permission & OTP_USER_PERMISSION_WRITE)
                                 FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.FTPDirectoryEntries[Index].PermissionMask |= (unsigned long)FTPM_USER_PERMISSION_WRITE;

                              if(OTP_Event_Data->Event_Data.OTP_Get_Directory_Response_Data->DirInfo.ObjectInfo[Index].Permission & OTP_USER_PERMISSION_DELETE)
                                 FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.FTPDirectoryEntries[Index].PermissionMask |= (unsigned long)FTPM_USER_PERMISSION_DELETE;
                           }

                           if(OTP_Event_Data->Event_Data.OTP_Get_Directory_Response_Data->DirInfo.ObjectInfo[Index].FieldMask & FTPM_OBJECT_INFO_MASK_GROUP_PERMISSION)
                           {
                              FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.FTPDirectoryEntries[Index].FieldMask |= FTPM_OBJECT_INFO_MASK_GROUP_PERMISSION;

                              if(OTP_Event_Data->Event_Data.OTP_Get_Directory_Response_Data->DirInfo.ObjectInfo[Index].Permission & OTP_GROUP_PERMISSION_READ)
                                 FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.FTPDirectoryEntries[Index].PermissionMask |= (unsigned long)FTPM_GROUP_PERMISSION_READ;

                              if(OTP_Event_Data->Event_Data.OTP_Get_Directory_Response_Data->DirInfo.ObjectInfo[Index].Permission & OTP_GROUP_PERMISSION_WRITE)
                                 FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.FTPDirectoryEntries[Index].PermissionMask |= (unsigned long)FTPM_GROUP_PERMISSION_WRITE;

                              if(OTP_Event_Data->Event_Data.OTP_Get_Directory_Response_Data->DirInfo.ObjectInfo[Index].Permission & OTP_GROUP_PERMISSION_DELETE)
                                 FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.FTPDirectoryEntries[Index].PermissionMask |= (unsigned long)FTPM_GROUP_PERMISSION_DELETE;
                           }

                           if(OTP_Event_Data->Event_Data.OTP_Get_Directory_Response_Data->DirInfo.ObjectInfo[Index].FieldMask & FTPM_OBJECT_INFO_MASK_OTHER_PERMISSION)
                           {
                              FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.FTPDirectoryEntries[Index].FieldMask |= FTPM_OBJECT_INFO_MASK_OTHER_PERMISSION;

                              if(OTP_Event_Data->Event_Data.OTP_Get_Directory_Response_Data->DirInfo.ObjectInfo[Index].Permission & OTP_OTHER_PERMISSION_READ)
                                 FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.FTPDirectoryEntries[Index].PermissionMask |= (unsigned long)FTPM_OTHER_PERMISSION_READ;

                              if(OTP_Event_Data->Event_Data.OTP_Get_Directory_Response_Data->DirInfo.ObjectInfo[Index].Permission & OTP_OTHER_PERMISSION_WRITE)
                                 FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.FTPDirectoryEntries[Index].PermissionMask |= (unsigned long)FTPM_OTHER_PERMISSION_WRITE;

                              if(OTP_Event_Data->Event_Data.OTP_Get_Directory_Response_Data->DirInfo.ObjectInfo[Index].Permission & OTP_OTHER_PERMISSION_DELETE)
                                 FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.FTPDirectoryEntries[Index].PermissionMask |= (unsigned long)FTPM_OTHER_PERMISSION_DELETE;
                           }

                           if(OTP_Event_Data->Event_Data.OTP_Get_Directory_Response_Data->DirInfo.ObjectInfo[Index].FieldMask & FTPM_OBJECT_INFO_MASK_OWNER)
                           {
                              FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.FTPDirectoryEntries[Index].FieldMask   |= FTPM_OBJECT_INFO_MASK_OWNER;
                              FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.FTPDirectoryEntries[Index].OwnerLength  = OTP_Event_Data->Event_Data.OTP_Get_Directory_Response_Data->DirInfo.ObjectInfo[Index].OwnerLength;

                              if(FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.FTPDirectoryEntries[Index].OwnerLength)
                                 FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.FTPDirectoryEntries[Index].Owner = OTP_Event_Data->Event_Data.OTP_Get_Directory_Response_Data->DirInfo.ObjectInfo[Index].Owner;
                              else
                                 FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.FTPDirectoryEntries[Index].Owner = NULL;
                           }

                           if(OTP_Event_Data->Event_Data.OTP_Get_Directory_Response_Data->DirInfo.ObjectInfo[Index].FieldMask & FTPM_OBJECT_INFO_MASK_GROUP)
                           {
                              FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.FTPDirectoryEntries[Index].FieldMask    |= FTPM_OBJECT_INFO_MASK_GROUP;
                              FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.FTPDirectoryEntries[Index].GroupLength  = OTP_Event_Data->Event_Data.OTP_Get_Directory_Response_Data->DirInfo.ObjectInfo[Index].GroupLength;

                              if(FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.FTPDirectoryEntries[Index].GroupLength)
                                 FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.FTPDirectoryEntries[Index].Group = OTP_Event_Data->Event_Data.OTP_Get_Directory_Response_Data->DirInfo.ObjectInfo[Index].Group;
                              else
                                 FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.FTPDirectoryEntries[Index].Group = NULL;
                           }
                        }
                     }
                  }

                  /* Let's flag the Request as completed if there is no */
                  /* more data to follow.                               */
                  Response = OTP_Event_Data->Event_Data.OTP_Get_Directory_Response_Data->ResponseCode;
                  if((OTP_Event_Data->Event_Data.OTP_Get_Directory_Response_Data->Phase & OTP_OBJECT_PHASE_LAST) || (FTPInfo->AbortActive) || (FTPInfo->AbortQueued))
                     FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.RequestCompleted = TRUE;
                  else
                     FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.RequestCompleted = FALSE;

                  /* If the Request was not completed, then we need to  */
                  /* request the next portion of the Directory Request. */
                  if(FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.RequestCompleted)
                     FTPInfo->CurrentOperation = coNone;

                  /* Now inform the registered caller of the Event that */
                  /* just transpired.                                   */
                  __BTPSTRY
                  {
                     if(FTPInfo->CallbackFunction)
                     {
                        FTPInfo->CallbackActive = TRUE;

                        (*((FTPM_Event_Callback_t)(FTPInfo->CallbackFunction)))(&FTP_Event_Data, FTPInfo->CallbackParameter);

                        FTPInfo->CallbackActive = FALSE;
                     }
                  }
                  __BTPSEXCEPT(1)
                  {
                     /* Do Nothing.                                     */
                  }

                  /* Free any memory that we may have allocated.        */
                  if((FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.NumberDirectoryEntries) && (FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.FTPDirectoryEntries))
                     BTPS_FreeMemory(FTP_Event_Data.EventData.ClientDirectoryRequestConfirmationEventData.FTPDirectoryEntries);
                  break;
               case etOTP_Set_Path_Response:
                  /* First we need to check to see what the Original FTP*/
                  /* Request was.                                       */
                  if(FTPInfo->CurrentOperation == coCreateDirectory)
                  {
                     /* Flag that there is currently no outstanding     */
                     /* operation.                                      */
                     FTPInfo->CurrentOperation = coNone;

                     FTP_Event_Data.EventType                                                                 = etFTPMDirectoryCreateConfirmation;
                     FTP_Event_Data.EventLength                                                               = FTPM_CLIENT_DIRECTORY_CREATE_CONFIRMATION_DATA_SIZE;

                     FTP_Event_Data.EventData.ClientDirectoryCreateConfirmationEventData.FTPConnectionID      = FTPInfo->FTPConnectionID;
                     FTP_Event_Data.EventData.ClientDirectoryCreateConfirmationEventData.CreatedDirectoryName = FTPInfo->CurrentObject;

                     /* Map the response code to an FTP Status Response.*/
                     Response = (Byte_t)(OTP_Event_Data->Event_Data.OTP_Set_Path_Response_Data->ResponseCode & ~OBEX_FINAL_BIT);
                     if((Response == OBEX_OK_RESPONSE) || (!Response))
                        FTP_Event_Data.EventData.ClientDirectoryCreateConfirmationEventData.Success           = TRUE;
                     else
                        FTP_Event_Data.EventData.ClientDirectoryCreateConfirmationEventData.Success           = FALSE;
                  }
                  else
                  {
                     /* Flag that there is currently no outstanding     */
                     /* operation.                                      */
                     FTPInfo->CurrentOperation = coNone;

                     FTP_Event_Data.EventType                                                            = etFTPMChangeDirectoryConfirmation;
                     FTP_Event_Data.EventLength                                                          = FTPM_CLIENT_CHANGE_DIRECTORY_CONFIRMATION_DATA_SIZE;

                     FTP_Event_Data.EventData.ClientChangeDirectoryConfirmationEventData.FTPConnectionID = FTPInfo->FTPConnectionID;

                     if(BTPS_StringLength(FTPInfo->CurrentObject))
                        FTP_Event_Data.EventData.ClientChangeDirectoryConfirmationEventData.ChangedDirectoryName = FTPInfo->CurrentObject;
                     else
                        FTP_Event_Data.EventData.ClientChangeDirectoryConfirmationEventData.ChangedDirectoryName = NULL;

                     /* Map the response code to an FTP Status Response.*/
                     Response = (Byte_t)(OTP_Event_Data->Event_Data.OTP_Set_Path_Response_Data->ResponseCode & ~OBEX_FINAL_BIT);
                     if((Response == OBEX_OK_RESPONSE) || (!Response))
                        FTP_Event_Data.EventData.ClientChangeDirectoryConfirmationEventData.Success              = TRUE;
                     else
                        FTP_Event_Data.EventData.ClientChangeDirectoryConfirmationEventData.Success              = FALSE;
                  }

                  /* Now inform the registered caller of the Event that */
                  /* just transpired.                                   */
                  __BTPSTRY
                  {
                     if(FTPInfo->CallbackFunction)
                     {
                        FTPInfo->CallbackActive = TRUE;

                        (*((FTPM_Event_Callback_t)(FTPInfo->CallbackFunction)))(&FTP_Event_Data, FTPInfo->CallbackParameter);

                        FTPInfo->CallbackActive = FALSE;
                     }
                  }
                  __BTPSEXCEPT(1)
                  {
                     /* Do Nothing.                                     */
                  }
                  break;
               case etOTP_Delete_Object_Response:
                  /* First we need to check to see what the Original FTP*/
                  /* Request was.                                       */
                  if(FTPInfo->CurrentOperation == coDeleteDirectory)
                  {
                     /* Flag that there is currently no outstanding     */
                     /* operation.                                      */
                     FTPInfo->CurrentOperation = coNone;

                     FTP_Event_Data.EventType                                                                 = etFTPMDirectoryDeleteConfirmation;
                     FTP_Event_Data.EventLength                                                               = FTPM_CLIENT_DIRECTORY_DELETE_CONFIRMATION_DATA_SIZE;

                     FTP_Event_Data.EventData.ClientDirectoryDeleteConfirmationEventData.FTPConnectionID      = FTPInfo->FTPConnectionID;
                     FTP_Event_Data.EventData.ClientDirectoryDeleteConfirmationEventData.DeletedDirectoryName = FTPInfo->CurrentObject;

                     /* Map the response code to an FTP Status Response.*/
                     Response = (Byte_t)(OTP_Event_Data->Event_Data.OTP_Delete_Object_Response_Data->ResponseCode & ~OBEX_FINAL_BIT);
                     if((Response == OBEX_OK_RESPONSE) || (!Response))
                     {
                        FTP_Event_Data.EventData.ClientDirectoryDeleteConfirmationEventData.Success              = TRUE;
                        FTP_Event_Data.EventData.ClientDirectoryDeleteConfirmationEventData.DirectoryNotEmpty    = FALSE;
                     }
                     else
                     {
                        FTP_Event_Data.EventData.ClientDirectoryDeleteConfirmationEventData.Success              = FALSE;

                        if(Response == OTP_PRECONDITION_FAILED_RESPONSE)
                           FTP_Event_Data.EventData.ClientDirectoryDeleteConfirmationEventData.DirectoryNotEmpty = TRUE;
                        else
                           FTP_Event_Data.EventData.ClientDirectoryDeleteConfirmationEventData.DirectoryNotEmpty = FALSE;
                     }
                  }
                  else
                  {
                     /* Must be a File Delete.                          */

                     /* Flag that there is currently no outstanding     */
                     /* operation.                                      */
                     FTPInfo->CurrentOperation = coNone;

                     FTP_Event_Data.EventType                                                       = etFTPMFileDeleteConfirmation;
                     FTP_Event_Data.EventLength                                                     = FTPM_CLIENT_FILE_DELETE_CONFIRMATION_DATA_SIZE;

                     FTP_Event_Data.EventData.ClientFileDeleteConfirmationEventData.FTPConnectionID = FTPInfo->FTPConnectionID;
                     FTP_Event_Data.EventData.ClientFileDeleteConfirmationEventData.DeletedFileName = FTPInfo->CurrentObject;

                     /* Map the response code to an FTP Status Response.*/
                     Response = (Byte_t)(OTP_Event_Data->Event_Data.OTP_Delete_Object_Response_Data->ResponseCode & ~OBEX_FINAL_BIT);
                     if((Response == OBEX_OK_RESPONSE) || (!Response))
                        FTP_Event_Data.EventData.ClientFileDeleteConfirmationEventData.Success      = TRUE;
                     else
                        FTP_Event_Data.EventData.ClientFileDeleteConfirmationEventData.Success      = FALSE;
                  }

                  /* Now inform the registered caller of the Event that */
                  /* just transpired.                                   */
                  __BTPSTRY
                  {
                     if(FTPInfo->CallbackFunction)
                     {
                        FTPInfo->CallbackActive = TRUE;

                        (*((FTPM_Event_Callback_t)(FTPInfo->CallbackFunction)))(&FTP_Event_Data, FTPInfo->CallbackParameter);

                        FTPInfo->CallbackActive = FALSE;
                     }
                  }
                  __BTPSEXCEPT(1)
                  {
                     /* Do Nothing.                                     */
                  }
                  break;
               case etOTP_Get_Object_Response:
                  /* Build the Get File Confirmation Event.             */
                  FTP_Event_Data.EventType                                                    = etFTPMFileGetConfirmation;
                  FTP_Event_Data.EventLength                                                  = FTPM_CLIENT_FILE_GET_CONFIRMATION_DATA_SIZE;

                  FTP_Event_Data.EventData.ClientFileGetConfirmationEventData.FTPConnectionID = FTPInfo->FTPConnectionID;
                  FTP_Event_Data.EventData.ClientFileGetConfirmationEventData.FileName        = FTPInfo->CurrentObject;
                  FTP_Event_Data.EventData.ClientFileGetConfirmationEventData.Success         = TRUE;

                  /* Let's flag the Request as completed if there is no */
                  /* more data to follow.                               */
                  if((OTP_Event_Data->Event_Data.OTP_Get_Object_Response_Data->Phase == OTP_OBJECT_PHASE_LAST) || (FTPInfo->AbortActive) || (FTPInfo->AbortQueued))
                     FTP_Event_Data.EventData.ClientFileGetConfirmationEventData.TransferComplete = TRUE;
                  else
                     FTP_Event_Data.EventData.ClientFileGetConfirmationEventData.TransferComplete = FALSE;

                  Response = (Byte_t)(OTP_Event_Data->Event_Data.OTP_Get_Object_Response_Data->ResponseCode & ~OBEX_FINAL_BIT);
                  if((Response == OBEX_OK_RESPONSE) || (!Response))
                  {
                     /* If this is the first phase, we need to go ahead */
                     /* and create the file.                            */
                     if(FTPInfo->FileInfo.FirstPhase)
                        FTPInfo->FileInfo.FileDescriptor = BTPS_Open_File(FTPInfo->CurrentDirectory, omCreate);

                     /* Next, let's write the newly received data to the*/
                     /* file we are saving (if the file was successfully*/
                     /* opened).                                        */
                     if(FTPInfo->FileInfo.FileDescriptor)
                     {
                        /* Flag that we are no longer in the first      */
                        /* phase.                                       */
                        FTPInfo->FileInfo.FirstPhase = FALSE;

                        /* Now, let's write the data to the file.       */
                        if(BTPS_Write_File(FTPInfo->FileInfo.FileDescriptor, OTP_Event_Data->Event_Data.OTP_Get_Object_Response_Data->BufferSize, OTP_Event_Data->Event_Data.OTP_Get_Object_Response_Data->BufferPtr, &BytesTransferred))
                           FTPInfo->FileInfo.FileIndex += BytesTransferred;
                        else
                           FTP_Event_Data.EventData.ClientFileGetConfirmationEventData.Success = FALSE;
                     }
                     else
                        FTP_Event_Data.EventData.ClientFileGetConfirmationEventData.Success = FALSE;
                  }
                  else
                  {
                     FTP_Event_Data.EventData.ClientFileGetConfirmationEventData.TransferComplete = TRUE;
                     FTP_Event_Data.EventData.ClientFileGetConfirmationEventData.Success          = FALSE;
                  }

                  /* Let's update the file statistic information.       */
                  FTP_Event_Data.EventData.ClientFileGetConfirmationEventData.TransferredLength = FTPInfo->FileInfo.FileIndex;
                  FTP_Event_Data.EventData.ClientFileGetConfirmationEventData.TotalLength       = FTPInfo->FileInfo.FileSize;

                  /* If the Request was completed, then we need to      */
                  /* change the current operation state.                */
                  if(FTP_Event_Data.EventData.ClientFileGetConfirmationEventData.TransferComplete)
                     FTPInfo->CurrentOperation = coNone;

                  /* If the Transaction is complete or there was an     */
                  /* error, go ahead and close the file (if opened).    */
                  if(((FTPInfo->CurrentOperation == coNone) || (!FTP_Event_Data.EventData.ClientFileGetConfirmationEventData.Success)) && (FTPInfo->FileInfo.FileDescriptor))
                  {
                     BTPS_Close_File(FTPInfo->FileInfo.FileDescriptor);

                     FTPInfo->FileInfo.FileDescriptor = NULL;
                  }

                  /* Now inform the registered caller of the Event that */
                  /* just transpired.                                   */
                  __BTPSTRY
                  {
                     if(FTPInfo->CallbackFunction)
                     {
                        FTPInfo->CallbackActive = TRUE;

                        (*((FTPM_Event_Callback_t)(FTPInfo->CallbackFunction)))(&FTP_Event_Data, FTPInfo->CallbackParameter);

                        FTPInfo->CallbackActive = FALSE;
                     }
                  }
                  __BTPSEXCEPT(1)
                  {
                     /* Do Nothing.                                     */
                  }
                  break;
               case etOTP_Put_Object_Response:
                  /* First we need to check to see what the Original FTP*/
                  /* Request was.                                       */
                  if(FTPInfo->CurrentOperation == coCreateFile)
                  {
                     /* Flag that there is currently no outstanding     */
                     /* operation.                                      */
                     FTPInfo->CurrentOperation = coNone;

                     FTP_Event_Data.EventType                                                       = etFTPMFileCreateConfirmation;
                     FTP_Event_Data.EventLength                                                     = FTPM_CLIENT_FILE_CREATE_CONFIRMATION_DATA_SIZE;

                     FTP_Event_Data.EventData.ClientFileCreateConfirmationEventData.FTPConnectionID = FTPInfo->FTPConnectionID;
                     FTP_Event_Data.EventData.ClientFileCreateConfirmationEventData.CreatedFileName = FTPInfo->CurrentObject;

                     /* Map the response code to an FTP Status Response.*/
                     Response = (Byte_t)(OTP_Event_Data->Event_Data.OTP_Put_Object_Response_Data->ResponseCode & ~OBEX_FINAL_BIT);
                     if((Response == OBEX_OK_RESPONSE) || (!Response))
                        FTP_Event_Data.EventData.ClientFileCreateConfirmationEventData.Success      = TRUE;
                     else
                        FTP_Event_Data.EventData.ClientFileCreateConfirmationEventData.Success      = FALSE;
                  }
                  else
                  {
                     /* Build the Put File Confirmation Event.          */
                     FTP_Event_Data.EventType                                                    = etFTPMFilePutConfirmation;
                     FTP_Event_Data.EventLength                                                  = FTPM_CLIENT_FILE_PUT_CONFIRMATION_DATA_SIZE;

                     FTP_Event_Data.EventData.ClientFilePutConfirmationEventData.FTPConnectionID = FTPInfo->FTPConnectionID;
                     FTP_Event_Data.EventData.ClientFilePutConfirmationEventData.FileName        = FTPInfo->CurrentObject;
                     FTP_Event_Data.EventData.ClientFilePutConfirmationEventData.Success         = TRUE;

                     /* Let's flag the Request as completed if there is */
                     /* no more data to follow.                         */
                     Response                                                               = (Byte_t)(OTP_Event_Data->Event_Data.OTP_Put_Object_Response_Data->ResponseCode & ~OBEX_FINAL_BIT);
                     BytesTransferred                                                       = 0;

                     if(((Response) && (Response != OBEX_OK_RESPONSE) && (Response != OBEX_CONTINUE_RESPONSE)) || (FTPInfo->AbortActive) || (FTPInfo->AbortQueued))
                     {
                        if((Response != OBEX_OK_RESPONSE) && (Response != OBEX_CONTINUE_RESPONSE))
                           FTP_Event_Data.EventData.ClientFilePutConfirmationEventData.Success = FALSE;

                        FTP_Event_Data.EventData.ClientFilePutConfirmationEventData.TransferComplete = TRUE;
                     }
                     else
                        FTP_Event_Data.EventData.ClientFilePutConfirmationEventData.TransferComplete = FALSE;

                     /* Let's update the file statistic information.    */
                     FTP_Event_Data.EventData.ClientFilePutConfirmationEventData.TransferredLength = FTPInfo->FileInfo.FileIndex;
                     FTP_Event_Data.EventData.ClientFilePutConfirmationEventData.TotalLength       = FTPInfo->FileInfo.FileSize;

                     if(FTPInfo->FileInfo.FileIndex < FTPInfo->FileInfo.FileSize)
                     {
                        if(FTPInfo->FileInfo.FirstPhase)
                        {
                           FTPInfo->FileInfo.FileDescriptor = BTPS_Open_File(FTPInfo->CurrentDirectory, omReadOnly);

                           /* Just to be sure, we will seek to the      */
                           /* beginning of the file to make sure we are */
                           /* the beginning of the file.                */
                           if(FTPInfo->FileInfo.FileDescriptor)
                              BTPS_Seek_File(FTPInfo->FileInfo.FileDescriptor, smBeginning, (SDWord_t)0);
                        }

                        /* Next, let's send the next chunk of data of   */
                        /* the file we are sending.                     */
                        if(FTPInfo->FileInfo.FileDescriptor)
                        {
                           /* Now, let's read the next portion of data  */
                           /* to send to the remote FTP Server.         */
                           if(FTPInfo->FileInfo.FileIndex < FTPInfo->FileInfo.FileSize)
                           {
                              if(OTP_Event_Data->Event_Data.OTP_Put_Object_Response_Data->BufferSize > OTP_DEFAULT_PACKET_LENGTH)
                                 CurrentLength = OTP_DEFAULT_PACKET_LENGTH;
                              else
                                 CurrentLength = OTP_Event_Data->Event_Data.OTP_Put_Object_Response_Data->BufferSize;

                              if(BTPS_Read_File(FTPInfo->FileInfo.FileDescriptor, CurrentLength, FTPInfo->PutFileBuffer, &BytesTransferred))
                              {
                                 FTPInfo->FileInfo.FileIndex += BytesTransferred;

                                 if(!BytesTransferred)
                                    FTP_Event_Data.EventData.ClientFilePutConfirmationEventData.TransferComplete = TRUE;
                              }
                              else
                                 FTP_Event_Data.EventData.ClientFilePutConfirmationEventData.Success = FALSE;
                           }
                           else
                              FTP_Event_Data.EventData.ClientFilePutConfirmationEventData.TransferComplete = TRUE;
                        }
                        else
                        {
                           FTP_Event_Data.EventData.ClientFilePutConfirmationEventData.TransferComplete = TRUE;
                           FTP_Event_Data.EventData.ClientFilePutConfirmationEventData.Success          = FALSE;
                        }
                     }
                     else
                        FTP_Event_Data.EventData.ClientFilePutConfirmationEventData.TransferComplete = TRUE;

                     /* Flag that we are no longer in the first phase of*/
                     /* transmitting the file.                          */
                     FTPInfo->FileInfo.FirstPhase = FALSE;

                     /* If the Request was not completed, then we need  */
                     /* to request the next portion of the File Put     */
                     /* Operation.                                      */
                     if(FTP_Event_Data.EventData.ClientFilePutConfirmationEventData.TransferComplete)
                     {
                        FTPInfo->CurrentOperation = coNone;

                        /* Free the Put File Buffer that we allocated.  */
                        if(FTPInfo->PutFileBuffer)
                        {
                           BTPS_FreeMemory(FTPInfo->PutFileBuffer);

                           FTPInfo->PutFileBuffer = NULL;
                        }
                     }
                     else
                     {
                        /* Since there is more data to send, we need to */
                        /* submit the next Put Object Request.  If there*/
                        /* is an error submitting the request then we   */
                        /* need to flag that there are no operations in */
                        /* progress.                                    */
                        if(_FTPM_Client_Put_Object(FTPInfo->OTPID, (unsigned int)BytesTransferred, (unsigned char *)FTPInfo->PutFileBuffer, (Boolean_t)((FTPInfo->FileInfo.FileIndex >= FTPInfo->FileInfo.FileSize)?TRUE:FALSE), 0) < 0)
                        {
                           FTP_Event_Data.EventData.ClientFilePutConfirmationEventData.TransferComplete = TRUE;
                           FTP_Event_Data.EventData.ClientFilePutConfirmationEventData.Success          = FALSE;

                           FTPInfo->CurrentOperation                                                = coNone;
                        }
                     }
                  }

                  /* If the Transaction is complete or there was an     */
                  /* error, go ahead and close the file (if opened).    */
                  if(((FTPInfo->CurrentOperation == coNone) || (!FTP_Event_Data.EventData.ClientFilePutConfirmationEventData.Success)) && (FTPInfo->FileInfo.FileDescriptor))
                  {
                     BTPS_Close_File(FTPInfo->FileInfo.FileDescriptor);

                     FTPInfo->FileInfo.FileDescriptor = NULL;
                  }

                  /* Now inform the registered caller of the Event that */
                  /* just transpired.                                   */
                  __BTPSTRY
                  {
                     if(FTPInfo->CallbackFunction)
                     {
                        FTPInfo->CallbackActive = TRUE;

                        (*((FTPM_Event_Callback_t)(FTPInfo->CallbackFunction)))(&FTP_Event_Data, FTPInfo->CallbackParameter);

                        FTPInfo->CallbackActive = FALSE;
                     }
                  }
                  __BTPSEXCEPT(1)
                  {
                     /* Do Nothing.                                     */
                  }
                  break;
               case etOTP_Abort_Response:
                  /* First we need to clear out the current FTP         */
                  /* Operation.                                         */
                  FTPInfo->CurrentOperation = coNone;

                  /* Next we need to flag that there is no longer an    */
                  /* Abort Request outstanding.                         */
                  FTPInfo->AbortActive      = FALSE;
                  FTPInfo->AbortQueued      = FALSE;

                  /* If this was a Put File operation then we might have*/
                  /* allocated a Buffer, so we need to free this memory.*/
                  if(FTPInfo->PutFileBuffer)
                  {
                     BTPS_FreeMemory(FTPInfo->PutFileBuffer);

                     FTPInfo->PutFileBuffer = NULL;
                  }

                  /* If there as an open File Descriptor, then we need  */
                  /* to make sure that we close it.                     */
                  if(FTPInfo->FileInfo.FileDescriptor)
                  {
                     BTPS_Close_File(FTPInfo->FileInfo.FileDescriptor);

                     FTPInfo->FileInfo.FileDescriptor = NULL;
                  }

                  /* Format up a FTP Abort Confirmation Event and inform*/
                  /* the callback that the event has occurred.          */
                  FTP_Event_Data.EventType                                                  = etFTPMAbortConfirmation;
                  FTP_Event_Data.EventLength                                                = FTPM_CLIENT_ABORT_CONFIRMATION_DATA_SIZE;

                  FTP_Event_Data.EventData.ClientAbortConfirmationEventData.FTPConnectionID = FTPInfo->FTPConnectionID;

                  /* Now inform the registered caller that the Abort    */
                  /* Confirmation has been received.                    */
                  __BTPSTRY
                  {
                     if(FTPInfo->CallbackFunction)
                     {
                        FTPInfo->CallbackActive = TRUE;

                        (*((FTPM_Event_Callback_t)(FTPInfo->CallbackFunction)))(&FTP_Event_Data, FTPInfo->CallbackParameter);

                        FTPInfo->CallbackActive = FALSE;
                     }
                  }
                  __BTPSEXCEPT(1)
                  {
                     /* Do Nothing.                                     */
                  }
                  break;
               default:
                  /* We are not interested in any other FTPM Mgr Events.*/
                  break;
            }
         }

         /* If there is an Abort Queued to go out then we need to send  */
         /* the Abort out.                                              */
         if((FTPInfo->AbortQueued) && (!FTPInfo->DeleteEntry))
         {
            if(_FTPM_Client_Abort_Request(FTPInfo->OTPID) >= 0)
               FTPInfo->AbortActive = TRUE;

            /* Flag that there is no longer an Abort Queued.            */
            FTPInfo->AbortQueued = FALSE;

            /* If there was an ongoing transfer, we need to make sure we*/
            /* close any open file.                                     */
            if(FTPInfo->FileInfo.FileDescriptor)
            {
               BTPS_Close_File(FTPInfo->FileInfo.FileDescriptor);

               FTPInfo->FileInfo.FileDescriptor = NULL;
            }
         }

         /* Delete the Entry if required.                               */
         if(FTPInfo->DeleteEntry)
            DeleteEntry = TRUE;

         /* Now we need to physically delete the File Transfer Profile  */
         /* Entry from the File Transfer Profile List.  Note at this    */
         /* time we own NO locks.                                       */
         if(DeleteEntry)
         {
            if(FTPServer)
               FTPM_Close_Server(FTPConnectionID);
            else
               FTPM_Close_Client(FTPConnectionID);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_FTP | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit:\n"));
}

   /* File Transfer Profile Module (FTPM) Installation/Support          */
   /* Functions.                                                        */

   /* The following function is responsible for initializing/cleaning up*/
   /* the Bluetopia Platform Manager FTP Manager Module.  This function */
   /* should be registered with the Bluetopia Platform Manager Module   */
   /* Handler and will be called when the Platform Manager is           */
   /* initialized (or shut down).                                       */
void BTPSAPI FTPM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData)
{
   DebugPrint((BTPM_DEBUG_ZONE_FTP | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %d\n", Initialize));

   /* First, determine if the request is to initialize or shut down.    */
   if(Initialize)
   {
      DebugPrint((BTPM_DEBUG_ZONE_FTP | BTPM_DEBUG_LEVEL_VERBOSE), ("Initialization Request\n"));

      if(!Initialized)
      {
         DebugPrint((BTPM_DEBUG_ZONE_FTP | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing FTP Manager\n"));

         /* Determine the current Device Power State.                   */
         CurrentPowerState       = (Boolean_t)((DEVM_QueryDevicePowerState() > 0)?TRUE:FALSE);

         /* Initialize a unique, starting FTP ID.                       */
         NextFTPConnectionID     = 0;

         /* Go ahead and flag that this module is initialized.          */
         Initialized             = TRUE;

         /* If the device is currently powered up go ahead and          */
         /* initialize the FTPM Mgr layer.                              */
         if(CurrentPowerState)
         {
            _FTPM_Initialize();
         }
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_FTP | BTPM_DEBUG_LEVEL_VERBOSE), ("FTP Manager Already initialized\n"));
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_FTP | BTPM_DEBUG_LEVEL_VERBOSE), ("Shutdown Request\n"));

      if(Initialized)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Free the FTP Info list.                                  */
            FreeFTPList(&FTPInfoList);

            /* Flag that the device is not powered on.                  */
            CurrentPowerState       = FALSE;

            /* Flag that this module is no longer initialized.          */
            Initialized             = FALSE;

            /* Free the Lock we acquired earlier.                       */
            DEVM_ReleaseLock();
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_FTP | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing asynchronous */
   /* Device Manager (DEVM) Events (including Power On/Off events).     */
   /* This function should be registered with the Bluetopia Platform    */
   /* Manager Module Handler and will be called when an asynchronous    */
   /* Device Manager event is dispatched.                               */
void BTPSAPI FTPM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData)
{
   DebugPrint((BTPM_DEBUG_ZONE_FTP | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the ANP Manager has been initialized.   */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         switch(EventData->EventType)
         {
            case detDevicePoweredOn:
               /* Power on Event.                                       */

               /* Note the new Power State.                             */
               CurrentPowerState = TRUE;

               /* Device is powered on so go ahead and initialize the   */
               /* FTPM Mgr layer.                                       */
               _FTPM_Initialize();
               break;
            case detDevicePoweringOff:
            case detDevicePoweredOff:
               /* Note the new Power State.                             */
               CurrentPowerState = FALSE;

               /* Free the FTP Info list.                               */
               FreeFTPList(&FTPInfoList);

               /* Device is powered off so go ahead and clean up the    */
               /* FTPM Mgr layer.                                       */
               _FTPM_Cleanup();
               break;
               break;
            default:
               /* Do nothing.                                           */
               break;
         }

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_FTP | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* File Transfer Profile Manager (FTPM) Server Functions.            */

   /* The following function is responsible for Opening a FTP File      */
   /* Server on the specified Bluetooth SPP Serial Port.  This function */
   /* accepts as input the Local Serial Port Server Number to use,      */
   /* followed by the the Port Flags, a pointer to a NULL terminated    */
   /* ASCII string which specifies the Local Directory Path of the root */
   /* directory of the File Server, the File Server Permissions to use  */
   /* when the responding to client requests, and the FTP Server Event  */
   /* Callback function (and parameter) to associate with the specified */
   /* File Server.  This function returns a positive, non zero, value if*/
   /* successful or a negative return error code if an error occurs.  A */
   /* successful return code will be a FTP ID that can be used to       */
   /* reference the Opened FTP Server in ALL other FTP Server functions */
   /* in this module.  Once a FTP Server is opened, it can only be      */
   /* Un-Registered via a call to the FTPM_Close_Server() function      */
   /* (passing the return value from this function).                    */
   /* * NOTE * The RootDirectory parameter should be formatted as a NULL*/
   /*          terminated ASCII string with UTF-8 encoding.             */
int BTPSAPI FTPM_Open_Server(unsigned int ServerPort, unsigned long PortFlags, char *RootDirectory, unsigned long PermissionMask, FTPM_Event_Callback_t CallbackFunction, void *CallbackParameter)
{
   int                             ret_val;
   FTPInfo_t                       FTPInfo;
   BTPS_Directory_Descriptor_t     DirectoryDescriptor;
   DEVM_Local_Device_Properties_t  LocalDeviceProperties;

   DebugPrint((BTPM_DEBUG_ZONE_FTP | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* Make sure that the passed in parameter seems semi-valid.          */
   if((Initialized) && (SPP_VALID_PORT_NUMBER(ServerPort)) && (RootDirectory) && (BTPS_StringLength(RootDirectory)) && (PermissionMask) && (CallbackFunction))
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Make sure that the device is currently powered up.          */
         if(CurrentPowerState)
         {
            /* Initialize the Common Elements among the FTP Server      */
            /* Profile Entries.                                         */
            BTPS_MemInitialize(&FTPInfo, 0, sizeof(FTPInfo_t));

            FTPInfo.FTPConnectionID   = GetNextFTPConnectionID();
            FTPInfo.FTPServer         = TRUE;
            FTPInfo.ConnectionState   = csNotConnected;
            FTPInfo.ServerPermissions = PermissionMask;
            FTPInfo.CurrentOperation  = coNone;
            FTPInfo.CallbackFunction  = (void *)CallbackFunction;
            FTPInfo.CallbackParameter = CallbackParameter;

            /* Let's verify that the Root Directory specified is valid. */
            if(BTPS_StringLength(RootDirectory) <= (sizeof(FTPInfo.RootDirectory) - 2))
            {
               /* Copy the specified Root Directory into the FTP Context*/
               /* structure.                                            */
               BTPS_StringCopy(FTPInfo.RootDirectory, RootDirectory);

               /* Next, let's make sure that the directory exists.      */

               /* First we will strip off any trailing '/' the caller   */
               /* may have specified (unless it is the root of a drive).*/
               if(FTPInfo.RootDirectory[BTPS_StringLength(RootDirectory) - 1] == BTPS_DIRECTORY_DELIMITER_CHARACTER)
               {
                  /* Check to see if the name specifies the root.       */
                  if(BTPS_Query_Directory_Is_Root_Of_Drive(FTPInfo.RootDirectory))
                     FTPInfo.RootIsDrive = TRUE;
                  else
                     FTPInfo.RootDirectory[BTPS_StringLength(RootDirectory) - 1] = '\0';
               }

               /* Next we will actually check to see if the specified   */
               /* path exists.                                          */
               /* * NOTE * We have to determine if the path exists or   */
               /*          not using different methods if the Path      */
               /*          specified is the Root of a Drive or Device.  */
               if(!FTPInfo.RootIsDrive)
               {
                  if((DirectoryDescriptor = BTPS_Open_Directory(FTPInfo.RootDirectory)) != NULL)
                  {
                     /* File Exists, so let's make sure that the Root   */
                     /* Directory has a directory delimiter character at*/
                     /* the end of it.                                  */
                     FTPInfo.RootDirectory[BTPS_StringLength(FTPInfo.RootDirectory) + 1] = '\0';
                     FTPInfo.RootDirectory[BTPS_StringLength(FTPInfo.RootDirectory) + 0] = BTPS_DIRECTORY_DELIMITER_CHARACTER;

                     /* We are completely finished with the Directory so*/
                     /* let's simply close it and be done with it.      */
                     BTPS_Close_Directory(DirectoryDescriptor);
                  }
                  else
                     FTPInfo.RootDirectory[0] = '\0';
               }
               else
               {
                  /* Simply check to make sure that we can open the Root*/
                  /* Directory to see if it exists - it better.         */
                  if((DirectoryDescriptor = BTPS_Open_Directory(FTPInfo.RootDirectory)) != NULL)
                     BTPS_Close_Directory(DirectoryDescriptor);
                  else
                     FTPInfo.RootDirectory[0] = '\0';
               }

               /* Check to make sure the Root Directory existed.        */
               if(BTPS_StringLength(FTPInfo.RootDirectory))
               {
                  /* Now let's open the Bluetooth FTPM Mgr Server Port  */
                  /* that was specified (or attempt to anyway).         */
                  if((int)(FTPInfo.OTPID = _FTPM_Open_Server_Port(ServerPort, PortFlags, tFileBrowser, (Word_t)OTP_DEFAULT_PACKET_LENGTH, OTP_Event_Callback, FTPInfo.FTPConnectionID)) > 0)
                  {
                     /* Now let's add the FTP Server Information to the */
                     /* FTP Information List before we try to register  */
                     /* with any other Libraries.                       */
                     if(AddFTPEntry(&FTPInfoList, &FTPInfo))
                     {
                        /* Finally let's make sure the Class of Device  */
                        /* is set correctly.                            */
                        if(DEVM_QueryLocalDeviceProperties(&LocalDeviceProperties) >= 0)
                        {
                           SET_MAJOR_SERVICE_CLASS(LocalDeviceProperties.ClassOfDevice, GET_MAJOR_SERVICE_CLASS(LocalDeviceProperties.ClassOfDevice) | HCI_LMP_CLASS_OF_DEVICE_SERVICE_CLASS_OBJECT_TRANSFER_BIT);

                           DEVM_UpdateLocalDeviceProperties(DEVM_UPDATE_LOCAL_DEVICE_PROPERTIES_CLASS_OF_DEVICE, &LocalDeviceProperties);
                        }

                        /* Everything initialized successfully so flag  */
                        /* success to the caller.                       */
                        ret_val = FTPInfo.FTPConnectionID;
                     }
                     else
                        ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
                  }
                  else
                  {
                     /* Error Opening FTPM Mgr Port so make sure that we*/
                     /* note the error that occurred and flag that the  */
                     /* FTPM Mgr Port was NOT opened successfully.      */
                     ret_val = (int)FTPInfo.OTPID;
                  }
              }
              else
                 ret_val = BTPM_ERROR_CODE_FTP_INVALID_ROOT_DIRECTORY;
            }
            else
               ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
         }
         else
            ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
   {
      if(Initialized)
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
      else
         ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
   }

   DebugPrint((BTPM_DEBUG_ZONE_FTP | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is responsible for Un-Registering a FTP    */
   /* Server (which was Registered by a successful call to the          */
   /* FTPM_Open_Server() function).  This function accepts as input the */
   /* FTP ID of the server that is to be closed.  This function returns */
   /* zero if successful, or a negative return error code if an error   */
   /* occurred.                                                         */
int BTPSAPI FTPM_Close_Server(unsigned int FTPConnectionID)
{
   int        ret_val;
   FTPInfo_t *FTPInfo;

   DebugPrint((BTPM_DEBUG_ZONE_FTP | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* Make sure that the passed in parameter seems semi-valid.          */
   if((Initialized) && (FTPConnectionID))
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Make sure that the device is currently powered up.          */
         if(CurrentPowerState)
         {
            /* Search the FTP Profile Information List for the specified*/
            /* FTP Profile Entry.                                       */
            if((FTPInfo = SearchFTPEntry(&FTPInfoList, FTPConnectionID)) != NULL)
            {
               /* This function is only applicable to FTP Servers, so   */
               /* check to see if the current FTP Server Entry is a a   */
               /* File Server.                                          */
               if(FTPInfo->FTPServer)
               {
                  /* We need to determine if we have been called in the */
                  /* context of a Callback Routine.  If we have, then we*/
                  /* simply need to flag this FTP Profile for deletion  */
                  /* and return.                                        */
                  if(FTPInfo->CallbackActive)
                  {
                     FTPInfo->DeleteEntry = TRUE;

                     /* Flag success to the caller.                     */
                     ret_val = 0;
                  }
                  else
                  {
                     /* Delete the specified FTP Profile Entry.         */
                     if((FTPInfo = DeleteFTPEntry(&FTPInfoList, FTPConnectionID)) != NULL)
                     {
                        /* Close the FTPM Mgr Server (if present).      */
                        if(FTPInfo->OTPID)
                           _FTPM_Close_Server_Port(FTPInfo->OTPID);

                        /* If there was an ongoing transfer, we need to */
                        /* make sure we close any open file.            */
                        if(FTPInfo->FileInfo.FileDescriptor)
                        {
                           BTPS_Close_File(FTPInfo->FileInfo.FileDescriptor);

                           FTPInfo->FileInfo.FileDescriptor = NULL;
                        }

                        /* Finally Free the Memory that was associated  */
                        /* with this FTP Profile Information List Entry.*/
                        FreeFTPEntryMemory(FTPInfo);

                        /* Flag success to the caller.                  */
                        ret_val = 0;
                     }
                     else
                        ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
                  }
               }
               else
                  ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
            }
            else
               ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
         }
         else
            ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
   {
      if(Initialized)
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
      else
         ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
   }

   DebugPrint((BTPM_DEBUG_ZONE_FTP | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is used to terminate a possible connection */
   /* to the local server.  This function can only be called by the FTP */
   /* Server.  A successfully call to this function will terminate the  */
   /* remote FTP Client connection to the local FTP server.  This       */
   /* function accepts as input the FTP ID that was returned from the   */
   /* FTPM_Open_Server() function.  This function returns zero if       */
   /* successful, or a negative return value if there was an error.     */
   /* This function does NOT Un-Register a FTP Server from the system,  */
   /* it ONLY disconnects any connection that is currently active.  The */
   /* FTPM_Close_Server() function can be used to Un-Register the FTP   */
   /* Server.                                                           */
int BTPSAPI FTPM_Close_Server_Connection(unsigned int FTPConnectionID)
{
   int        ret_val;
   FTPInfo_t *FTPInfo;

   DebugPrint((BTPM_DEBUG_ZONE_FTP | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* Make sure that the passed in parameter seems semi-valid.          */
   if((Initialized) && (FTPConnectionID))
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Make sure that the device is currently powered up.          */
         if(CurrentPowerState)
         {
            /* Search the FTP Profile Information List for the specified*/
            /* FTP Profile Entry.                                       */
            if((FTPInfo = SearchFTPEntry(&FTPInfoList, FTPConnectionID)) != NULL)
            {
               /* This function is only applicable to FTP Servers, so   */
               /* check to see if the current FTP Entry is a FTP Server.*/
               if(FTPInfo->FTPServer)
               {
                  /* Set the Connected flag to indicate that the Server */
                  /* is no longer connected.                            */
                  FTPInfo->ConnectionState = csNotConnected;

                  /* Close the FTPM Mgr Client Port (if present).       */
                  _FTPM_Close_Port(FTPInfo->OTPID);

                  /* If there was an ongoing transfer, we need to make  */
                  /* sure we close any open file.                       */
                  if(FTPInfo->FileInfo.FileDescriptor)
                  {
                     BTPS_Close_File(FTPInfo->FileInfo.FileDescriptor);

                     FTPInfo->FileInfo.FileDescriptor = NULL;
                  }

                  /* Flag success to the caller.                        */
                  ret_val = 0;
               }
               else
                  ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
            }
            else
               ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
         }
         else
            ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
   {
      if(Initialized)
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
      else
         ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
   }

   DebugPrint((BTPM_DEBUG_ZONE_FTP | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is provided to allow a means to respond to */
   /* a request to connect to a FTP Server.  This function accepts as   */
   /* input the FTP ID that was returned from the FTPM_Open_Server()    */
   /* function.  The final parameter to this function is whether to     */
   /* accept the pending connection.  This function returns zero if     */
   /* successful, or a negative return value if there was an error.     */
int BTPSAPI FTPM_Server_Connect_Request_Response(unsigned int FTPConnectionID, Boolean_t AcceptConnection)
{
   int        ret_val;
   FTPInfo_t *FTPInfo;

   DebugPrint((BTPM_DEBUG_ZONE_FTP | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* Make sure that the passed in parameter seems semi-valid.          */
   if((Initialized) && (FTPConnectionID))
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Make sure that the device is currently powered up.          */
         if(CurrentPowerState)
         {
            /* Search the FTP Profile Information List for the specified*/
            /* FTP Profile Entry.                                       */
            if((FTPInfo = SearchFTPEntry(&FTPInfoList, FTPConnectionID)) != NULL)
            {
               /* This function is only applicable to FTP Servers, so   */
               /* check to see if the current FTP Entry is a File Server*/
               /* and that there is currently a connection pending.     */
               if((FTPInfo->FTPServer) && (FTPInfo->ConnectionState == csConnectionPending))
               {
                  /* The FTP Entry is a FTP Server Entry and there is   */
                  /* currently a connection pending.  Next simply submit*/
                  /* the Open Port Request Response to the lower layer. */
                  if((ret_val = _FTPM_Open_Port_Request_Response(FTPInfo->OTPID, AcceptConnection)) == 0)
                  {
                     /* The Open Port Request Response was successfully */
                     /* submitted to the lower layer.  Next check to see*/
                     /* if this was a call to reject the connection.    */
                     if(AcceptConnection == FALSE)
                     {
                        /* The connection was reject, flag that we are  */
                        /* no longer connected.                         */
                        FTPInfo->ConnectionState = csNotConnected;
                     }
                  }
               }
               else
                  ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         }
         else
            ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
   {
      if(Initialized)
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
      else
         ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
   }

   DebugPrint((BTPM_DEBUG_ZONE_FTP | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is provided to allow a means to add a      */
   /* Generic File Server Service Record to the SDP Database.  This     */
   /* function takes as input the FTP Server ID (which *MUST* have been */
   /* obtained by calling the FTPM_Open_Server() function.  The second  */
   /* parameter specifies the Service Name to associate with the SDP    */
   /* Record.  The final parameter is a pointer to a DWord_t which      */
   /* receives the SDP Service Record Handle if this function           */
   /* successfully creates an SDP Service Record.  If this function     */
   /* returns zero, then the SDPServiceRecordHandle entry will contain  */
   /* the Service Record Handle of the added SDP Service Record.  If    */
   /* this function fails, a negative return error code will be returned*/
   /* and the SDPServiceRecordHandle value will be undefined.           */
   /* * NOTE * This function should only be called with the FTP ID that */
   /*          was returned from the FTPM_Open_Server() function.  This */
   /*          function should NEVER be used with FTP ID returned from  */
   /*          the FTPM_Open_Remote_Server() function.                  */
   /* * NOTE * The Service Record Handle that is returned from this     */
   /*          function will remain in the SDP Record Database until it */
   /*          is deleted by calling the FTPM_Close_Server() function.  */
   /* * NOTE * The Service Name is always added at Attribute ID 0x0100. */
   /*          A Language Base Attribute ID List is created that        */
   /*          specifies that 0x0100 is UTF-8 Encoded, English Language.*/
int BTPSAPI FTPM_Register_Server_SDP_Record(unsigned int FTPConnectionID, char *ServiceName, DWord_t *SDPServiceRecordHandle)
{
   int                       ret_val;
   Byte_t                    EIRData[2 + UUID_16_SIZE];
   FTPInfo_t                *FTPInfo;
   unsigned int              EIRDataLength;
   SDP_UUID_Entry_t          SDP_UUID_Entry;
   SDP_Data_Element_t        SDP_Data_Element_Main;
   SDP_Data_Element_t        SDP_Data_Element_List[3];
   OTP_SDP_Service_Record_t  OTP_SDP_Service_Record;

   DebugPrint((BTPM_DEBUG_ZONE_FTP | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* Make sure that the input parameters that were passed to us are    */
   /* semi-valid.                                                       */
   if((FTPConnectionID) && (ServiceName) && (BTPS_StringLength(ServiceName)) && (SDPServiceRecordHandle))
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Make sure that the device is currently powered up.          */
         if(CurrentPowerState)
         {
            /* Search the FTP Profile Information List for the specified*/
            /* FTP Profile Entry.                                       */
            if((FTPInfo = SearchFTPEntry(&FTPInfoList, FTPConnectionID)) != NULL)
            {
               /* Now let's Add a generic OBEX File Server SDP Record to*/
               /* the SDP Database (we will let FTPM Mgr handle all of  */
               /* the grunt work for us).                               */
               OTP_SDP_Service_Record.NumberServiceClassUUID                    = 1;
               OTP_SDP_Service_Record.SDPUUIDEntries                            = &SDP_UUID_Entry;

               SDP_UUID_Entry.SDP_Data_Element_Type                             = deUUID_16;
               SDP_ASSIGN_FILE_TRANSFER_PROFILE_UUID_16(SDP_UUID_Entry.UUID_Value.UUID_16);

               OTP_SDP_Service_Record.ProtocolList                              = NULL;

               /* OK, FTPM Mgr SDP Service Record Built, now have FTPM  */
               /* Mgr Register it for us.                               */
               if((ret_val = _FTPM_Register_SDP_Record(FTPInfo->OTPID, &OTP_SDP_Service_Record, ServiceName, SDPServiceRecordHandle)) >= 0)
               {
                  /* Add the Profile Descriptor List.                   */
                  SDP_Data_Element_Main.SDP_Data_Element_Type                           = deSequence;
                  SDP_Data_Element_Main.SDP_Data_Element_Length                         = 1;
                  SDP_Data_Element_Main.SDP_Data_Element.SDP_Data_Element_Sequence      = SDP_Data_Element_List;

                  SDP_Data_Element_List[0].SDP_Data_Element_Type                        = deSequence;
                  SDP_Data_Element_List[0].SDP_Data_Element_Length                      = 2;
                  SDP_Data_Element_List[0].SDP_Data_Element.SDP_Data_Element_Sequence   = &(SDP_Data_Element_List[1]);

                  SDP_Data_Element_List[1].SDP_Data_Element_Type                        = deUUID_16;
                  SDP_Data_Element_List[1].SDP_Data_Element_Length                      = UUID_16_SIZE;
                  SDP_ASSIGN_FILE_TRANSFER_PROFILE_UUID_16(SDP_Data_Element_List[1].SDP_Data_Element.UUID_16);

                  SDP_Data_Element_List[2].SDP_Data_Element_Type                        = deUnsignedInteger2Bytes;
                  SDP_Data_Element_List[2].SDP_Data_Element_Length                      = WORD_SIZE;
                  SDP_Data_Element_List[2].SDP_Data_Element.UnsignedInteger2Bytes       = FTPM_PROFILE_VERSION;

                  /* Add the Bluetooth Profile Descriptor List.         */
                  if((ret_val = DEVM_AddServiceRecordAttribute(*SDPServiceRecordHandle, SDP_ATTRIBUTE_ID_BLUETOOTH_PROFILE_DESCRIPTOR_LIST, &SDP_Data_Element_Main)) < 0)
                     DEVM_DeleteServiceRecord(*SDPServiceRecordHandle);
                  else
                  {
                     /* Configure the EIR Data.                         */
                     EIRDataLength = ((2 * NON_ALIGNED_BYTE_SIZE) + UUID_16_SIZE);

                     ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&EIRData[0], (NON_ALIGNED_BYTE_SIZE+UUID_16_SIZE));
                     ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&EIRData[1], HCI_EXTENDED_INQUIRY_RESPONSE_DATA_TYPE_16_BIT_SERVICE_CLASS_UUID_PARTIAL);

                     /* Assign the Obex FTP UUID.                       */
                     SDP_ASSIGN_FILE_TRANSFER_PROFILE_UUID_16(SDP_UUID_Entry.UUID_Value.UUID_16);

                     /* Convert the UUID to little endian as required by*/
                     /* EIR data.                                       */
                     CONVERT_SDP_UUID_16_TO_BLUETOOTH_UUID_16(*((UUID_16_t *)&(EIRData[2])), SDP_UUID_Entry.UUID_Value.UUID_16);

                     /* Configure the EIR data.                         */
                     MOD_AddEIRData(EIRDataLength, EIRData);
                  }
               }
            }
            else
               ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
         }
         else
            ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_FTP | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* File Transfer Profile Manager (FTPM) Client Functions.            */

   /* The following function is responsible for opening a Remote FTP    */
   /* File Server.  This function accepts as input the the Remote       */
   /* Bluetooth Device Address of the Bluetooth FTP Server to connect   */
   /* with.  The second parameter specifies the Remote Server Port to   */
   /* connect with.  The third parameter specifies the optional flags to*/
   /* use when opening the port.  The final two parameters specify the  */
   /* FTP Client Event Callback Function and the Callback Parameter to  */
   /* associate with this FTP Client.  This function returns a positive,*/
   /* non zero, value if successful or a negative return error code if  */
   /* an error occurs.  A successful return code will be a FTP ID that  */
   /* can be used to reference the Opened FTP Server in ALL other FTP   */
   /* Client functions in this module.  Once a remote server is opened, */
   /* it can only be closed via a call to the FTPM_Close_Client()       */
   /* function (passing the return value from this function).           */
   /* * NOTE * The final parameter to this function, if specified,      */
   /*          instructs this function to block until the connection    */
   /*          status is received (i.e.  the connection is completed).  */
   /*          If this parameter is not specified (i.e.  NULL) then the */
   /*          connection status will be returned asynchronously in the */
   /*          Open Remote Port Result event.                           */
int BTPSAPI FTPM_Open_Remote_File_Server(BD_ADDR_t RemoteDeviceAddress, unsigned int RemoteServerPort, unsigned long OpenFlags, FTPM_Event_Callback_t CallbackFunction, void *CallbackParameter, unsigned int *ConnectionStatus)
{
   int         ret_val;
   FTPInfo_t   FTPInfo;
   FTPInfo_t  *FTPInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_FTP | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* Make sure that the passed in parameter seems semi-valid.          */
   if((Initialized) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (SPP_VALID_PORT_NUMBER(RemoteServerPort)) &&  (CallbackFunction))
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Make sure that the device is currently powered up.          */
         if(CurrentPowerState)
         {
            /* Initialize the Common Elements among the FTP Server      */
            /* Profile Entries.                                         */
            BTPS_MemInitialize(&FTPInfo, 0, sizeof(FTPInfo_t));

            FTPInfo.FTPConnectionID   = GetNextFTPConnectionID();
            FTPInfo.ConnectionState   = csNotConnected;
            FTPInfo.CurrentOperation  = coNone;
            FTPInfo.BD_ADDR           = RemoteDeviceAddress;
            FTPInfo.CallbackFunction  = (void *)CallbackFunction;
            FTPInfo.CallbackParameter = CallbackParameter;

            /* Now let's add the FTP Client Information to the FTP      */
            /* Information List before we try to register with any other*/
            /* Libraries.                                               */
            if((FTPInfoPtr = AddFTPEntry(&FTPInfoList, &FTPInfo)) != NULL)
            {
               /* Now let's open the Bluetooth FTPM Mgr Server Port that*/
               /* was specified (or attempt to anyway).                 */
               if((int)(FTPInfoPtr->OTPID = _FTPM_Open_Remote_Port(RemoteDeviceAddress, RemoteServerPort, OpenFlags, (Word_t)OTP_DEFAULT_PACKET_LENGTH, OTP_Event_Callback, FTPInfo.FTPConnectionID, ConnectionStatus)) > 0)
               {
                  /* Everything initialized successfully so flag success*/
                  /* to the caller.                                     */
                  ret_val = FTPInfoPtr->FTPConnectionID;
               }
               else
               {
                  /* Error Opening FTPM Mgr Port so make sure that we   */
                  /* note the error that occurred and flag that the FTPM*/
                  /* Mgr Port was NOT opened successfully.              */
                  ret_val = (int)FTPInfoPtr->OTPID;

                  /* Now let's delete the FTP Client Entry from the FTP */
                  /* Entry List.                                        */
                  if((FTPInfoPtr = DeleteFTPEntry(&FTPInfoList, FTPInfoPtr->FTPConnectionID)) != NULL)
                     FreeFTPEntryMemory(FTPInfoPtr);
               }
            }
            else
               ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
         }
         else
            ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
   {
      if(Initialized)
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
      else
         ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
   }

   DebugPrint((BTPM_DEBUG_ZONE_FTP | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is used to terminate a possible connection */
   /* to a remote server.  This function can only be called by the FTP  */
   /* Client.  A successful call to this function will terminate the    */
   /* remote FTP Connection.  This function accepts as input the FTP ID */
   /* that was returned from the the FTPM_Open_Remote_File_Server()     */
   /* function.  This function returns zero if successful, or a negative*/
   /* return value if there was an error.                               */
int BTPSAPI FTPM_Close_Client(unsigned int FTPConnectionID)
{
   int        ret_val;
   FTPInfo_t *FTPInfo;

   DebugPrint((BTPM_DEBUG_ZONE_FTP | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* Make sure that the passed in parameter seems semi-valid.          */
   if((Initialized) && (FTPConnectionID))
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Make sure that the device is currently powered up.          */
         if(CurrentPowerState)
         {
            /* Search the FTP Profile Information List for the specified*/
            /* FTP Profile Entry.                                       */
            if((FTPInfo = SearchFTPEntry(&FTPInfoList, FTPConnectionID)) != NULL)
            {
               /* This function is only applicable to FTP Clients, so   */
               /* check to see if the current FTP Client Entry is a File*/
               /* Client.                                               */
               if(!FTPInfo->FTPServer)
               {
                  /* We need to determine if we have been called in the */
                  /* context of a Callback Routine.  If we have, then we*/
                  /* simply need to flag this FTP Profile for deletion  */
                  /* and return.                                        */
                  if(FTPInfo->CallbackActive)
                  {
                     FTPInfo->DeleteEntry = TRUE;

                     /* Flag success to the caller.                     */
                     ret_val = 0;
                  }
                  else
                  {
                     /* Delete the specified FTP Profile Entry.         */
                     if((FTPInfo = DeleteFTPEntry(&FTPInfoList, FTPConnectionID)) != NULL)
                     {
                        /* Free any memory that was allocated.          */
                        if(FTPInfo->PutFileBuffer)
                          BTPS_FreeMemory(FTPInfo->PutFileBuffer);

                        /* If there was an ongoing transfer, we need to */
                        /* make sure we close any open file.            */
                        if(FTPInfo->FileInfo.FileDescriptor)
                        {
                           BTPS_Close_File(FTPInfo->FileInfo.FileDescriptor);

                           FTPInfo->FileInfo.FileDescriptor = NULL;
                        }

                        /* Close the FTPM Mgr Client Port (if present). */
                        if(FTPInfo->OTPID)
                        {
                           /* Issue the OBEX Disconnect before we shut  */
                           /* down the port.                            */
                           /* * NOTE * We do not have to wait for a     */
                           /*          response because the Disconnect  */
                           /*          cannot be failed, and we are     */
                           /*          guaranteed that it will go out   */
                           /*          before the Close Port because of */
                           /*          the serial nature of the lower   */
                           /*          layers.                          */
                           _FTPM_Client_Disconnect(FTPInfo->OTPID);

                           _FTPM_Close_Port(FTPInfo->OTPID);
                        }

                        /* Finally Free the Memory that was associated  */
                        /* with this FTP Profile Information List Entry.*/
                        FreeFTPEntryMemory(FTPInfo);

                        /* Flag success to the caller.                  */
                        ret_val = 0;
                     }
                     else
                        ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
                  }
               }
               else
                  ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
            }
            else
               ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
         }
         else
            ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
   {
      if(Initialized)
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
      else
         ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
   }

   DebugPrint((BTPM_DEBUG_ZONE_FTP | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is provided to allow a FTP Client to       */
   /* request either the current directory (or a sub-directory off of   */
   /* the current directory) listing.  This function accepts as its     */
   /* first parameter the FTP Client ID (returned from a successful call*/
   /* to the FTPM_Open_Remote_File_Server() function).  The second      */
   /* parameter specifies whether or not the current directory listing  */
   /* is to retrieved (NULL) or a sub-directory listing is to be        */
   /* retrieved (NON-NULL).  If a sub-directory is to be retrieved then */
   /* the final parameter points to a NULL terminated ASCII string that */
   /* represents the Remote sub-directory name to browse.  This function*/
   /* returns zero if successful, or a negative return error code if    */
   /* there was an error.                                               */
   /* * NOTE * The RemoteDirectoryName parameter should be formatted as */
   /*          a NULL terminated ASCII string with UTF-8 encoding.      */
   /* * NOTE * Issuing this command successfully does not mean that the */
   /*          Remote FTP Server successfully issued the command.  The  */
   /*          caller needs to check the confirmation result to         */
   /*          determine if the Remote FTP Server successfully executed */
   /*          the Request.                                             */
   /* * NOTE * Due to an OBEX FTP limitation, there can only be one     */
   /*          outstanding FTP Client Request active at any one time.   */
   /*          Because of this, another FTP Client Request cannot be    */
   /*          issued until either the current request is Aborted (by   */
   /*          calling the FTPM_Abort() function) or the current Request*/
   /*          is complete (this is signified by receiving a            */
   /*          Confirmation Event in the FTP Client Event Callback that */
   /*          was registered when the FTP Client was opened).          */
int BTPSAPI FTPM_Get_Directory(unsigned int FTPConnectionID, char *RemoteDirectoryName)
{
   int        ret_val;
   FTPInfo_t *FTPInfo;

   DebugPrint((BTPM_DEBUG_ZONE_FTP | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* Make sure that the passed in parameter seems semi-valid.          */
   if((Initialized) && (FTPConnectionID) && ((!RemoteDirectoryName) || ((RemoteDirectoryName) && (BTPS_StringLength(RemoteDirectoryName)))))
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Make sure that the device is currently powered up.          */
         if(CurrentPowerState)
         {
            /* Search the FTP Profile Information List for the specified*/
            /* FTP Profile Entry.                                       */
            if((FTPInfo = SearchFTPEntry(&FTPInfoList, FTPConnectionID)) != NULL)
            {
               /* This function is only applicable to FTP Clients, so   */
               /* check to see if the current FTP Client Entry is a File*/
               /* Client.                                               */
               if((!FTPInfo->FTPServer) && (FTPInfo->ConnectionState == csConnected))
               {
                  /* Let's make sure that we do not have any currently  */
                  /* outstanding Operations.                            */
                  if((FTPInfo->CurrentOperation == coNone) && (!FTPInfo->AbortActive))
                  {
                     /* Let's make sure that we have a buffer large     */
                     /* enough to store the directory that we are       */
                     /* requesting (if specified).                      */
                     if((!RemoteDirectoryName) || ((RemoteDirectoryName) && (BTPS_StringLength(RemoteDirectoryName) < sizeof(FTPInfo->CurrentObject))))
                     {
                        /* Store the Current Remote Directory that we   */
                        /* are requesting so that we can return the name*/
                        /* in the Confirmation Event (if specified).    */
                        if(RemoteDirectoryName)
                           BTPS_StringCopy(FTPInfo->CurrentObject, RemoteDirectoryName);
                        else
                            FTPInfo->CurrentObject[0] = '\0';

                        /* If we are not currently doing anything as the*/
                        /* current operation then we need to inform the */
                        /* Remote FTP Server to request the directory   */
                        /* for us.                                      */
                        if((ret_val = _FTPM_Client_Get_Directory(FTPInfo->OTPID, RemoteDirectoryName)) >= 0)
                           FTPInfo->CurrentOperation = coDirectoryRequest;
                     }
                     else
                        ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_FTP_REQUEST_OUTSTANDING;
               }
               else
               {
                  /* Map the correct error code.                        */
                  if(!FTPInfo->FTPServer)
                     ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
                  else
                     ret_val = BTPM_ERROR_CODE_FTP_ACTION_NOT_ALLOWED;
               }
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         }
         else
            ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
   {
      if(Initialized)
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
      else
         ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
   }

   DebugPrint((BTPM_DEBUG_ZONE_FTP | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is provided to allow a FTP Client to change*/
   /* the current working directory on the remote FTP Server.  This     */
   /* function accepts as its first parameter the FTP Client ID         */
   /* (returned from a successful call to the                           */
   /* FTPM_Open_Remote_File_Server() function).  The second parameter   */
   /* specifies the sub-directory to change directories into (NON-NULL),*/
   /* or, instead, whether to back up the parent directory (NULL).  If  */
   /* the RemoteDirectoryName parameter is NULL, then the FTP Client    */
   /* Requests that the FTP Server change working directories to the    */
   /* parent directory of the current directory.  If this parameter is  */
   /* NON-NULL, then it needs to point to a NULL terminated ASCII string*/
   /* that represents the remote sub-directory name to change to.  The  */
   /* RemoteDirectoryName parameter cannot specify path information,    */
   /* only the name of a Remote Directory that exists in the current    */
   /* working directory (if NON-NULL).  This function returns zero if   */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * The RemoteDirectoryName parameter should be formatted as */
   /*          a NULL terminated ASCII string with UTF-8 encoding.      */
   /* * NOTE * Issuing this command successfully does not mean that the */
   /*          Remote FTP Server successfully issued the command.  The  */
   /*          caller needs to check the confirmation result to         */
   /*          determine if the Remote FTP Server successfully executed */
   /*          the Request.                                             */
   /* * NOTE * Due to an OBEX FTP limitation, there can only be one     */
   /*          outstanding FTP Client Request active at any one time.   */
   /*          Because of this, another FTP Client Request cannot be    */
   /*          issued until either the current request is Aborted (by   */
   /*          calling the FTPM_Abort() function) or the current Request*/
   /*          is complete (this is signified by receiving a            */
   /*          Confirmation Event in the FTP Client Event Callback that */
   /*          was registered when the FTP Client was opened).          */
int BTPSAPI FTPM_Set_Directory(unsigned int FTPConnectionID, char *RemoteDirectoryName)
{
   int        ret_val;
   FTPInfo_t *FTPInfo;

   DebugPrint((BTPM_DEBUG_ZONE_FTP | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* Make sure that the passed in parameter seems semi-valid.          */
   if((Initialized) && (FTPConnectionID) && ((!RemoteDirectoryName) || ((RemoteDirectoryName) && (BTPS_StringLength(RemoteDirectoryName)))))
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Make sure that the device is currently powered up.          */
         if(CurrentPowerState)
         {
            /* Search the FTP Profile Information List for the specified*/
            /* FTP Profile Entry.                                       */
            if((FTPInfo = SearchFTPEntry(&FTPInfoList, FTPConnectionID)) != NULL)
            {
               /* This function is only applicable to FTP Clients, so   */
               /* check to see if the current FTP Client Entry is a File*/
               /* Client.                                               */
               if((!FTPInfo->FTPServer) && (FTPInfo->ConnectionState == csConnected))
               {
                  /* Let's make sure that we do not have any currently  */
                  /* outstanding Operations.                            */
                  if((FTPInfo->CurrentOperation == coNone) && (!FTPInfo->AbortActive))
                  {
                     /* Let's make sure that we have a buffer large     */
                     /* enough to store the directory that we are       */
                     /* changing to (if specified).                     */
                     if((!RemoteDirectoryName) || ((RemoteDirectoryName) && (BTPS_StringLength(RemoteDirectoryName) < sizeof(FTPInfo->CurrentObject))))
                     {
                        /* Store the Current Remote Directory that we   */
                        /* are changing to so that we can return the    */
                        /* name in the Confirmation Event (if           */
                        /* specified).                                  */
                        if(RemoteDirectoryName)
                           BTPS_StringCopy(FTPInfo->CurrentObject, RemoteDirectoryName);
                        else
                            FTPInfo->CurrentObject[0] = '\0';

                        /* If we are not currently doing anything as the*/
                        /* current operation then we need to inform the */
                        /* Remote FTP Server to change the directory for*/
                        /* us.                                          */
                        if((ret_val = _FTPM_Client_Set_Path(FTPInfo->OTPID, RemoteDirectoryName, (Boolean_t)(RemoteDirectoryName?FALSE:TRUE), (Boolean_t)FALSE)) >= 0)
                           FTPInfo->CurrentOperation = coChangeDirectory;
                     }
                     else
                        ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_FTP_REQUEST_OUTSTANDING;
               }
               else
               {
                  /* Map the correct error code.                        */
                  if(!FTPInfo->FTPServer)
                     ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
                  else
                     ret_val = BTPM_ERROR_CODE_FTP_ACTION_NOT_ALLOWED;
               }
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         }
         else
            ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
   {
      if(Initialized)
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
      else
         ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
   }

   DebugPrint((BTPM_DEBUG_ZONE_FTP | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is provided to allow a FTP Client to change*/
   /* the current working directory on the remote FTP Server to the Root*/
   /* Directory.  This function accepts as its first parameter the FTP  */
   /* Client ID (returned from a successful call to the                 */
   /* FTPM_Open_Remote_File_Server() function).  This function returns  */
   /* zero successful, or a negative return error code if there was an  */
   /* error.                                                            */
   /* * NOTE * Issuing this command successfully does not mean that the */
   /*          Remote FTP Server successfully issued the command.  The  */
   /*          caller needs to check the confirmation result to         */
   /*          determine if the Remote FTP Server successfully executed */
   /*          the Request.                                             */
   /* * NOTE * Due to an OBEX FTP limitation, there can only be one     */
   /*          outstanding FTP Client Request active at any one time.   */
   /*          Because of this, another FTP Client Request cannot be    */
   /*          issued until either the current request is Aborted (by   */
   /*          calling the FTPM_Abort() function) or the current Request*/
   /*          is complete (this is signified by receiving a            */
   /*          Confirmation Event in the FTP Client Event Callback that */
   /*          was registered when the FTP Client was opened).          */
int BTPSAPI FTPM_Set_Root_Directory(unsigned int FTPConnectionID)
{
   int        ret_val;
   FTPInfo_t *FTPInfo;

   DebugPrint((BTPM_DEBUG_ZONE_FTP | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* Make sure that the passed in parameter seems semi-valid.          */
   if((Initialized) && (FTPConnectionID))
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Make sure that the device is currently powered up.          */
         if(CurrentPowerState)
         {
            /* Search the FTP Profile Information List for the specified*/
            /* FTP Profile Entry.                                       */
            if((FTPInfo = SearchFTPEntry(&FTPInfoList, FTPConnectionID)) != NULL)
            {
               /* This function is only applicable to FTP Clients, so   */
               /* check to see if the current FTP Client Entry is a File*/
               /* Client.                                               */
               if((!FTPInfo->FTPServer) && (FTPInfo->ConnectionState == csConnected))
               {
                  /* Let's make sure that we do not have any currently  */
                  /* outstanding Operations.                            */
                  if((FTPInfo->CurrentOperation == coNone) && (!FTPInfo->AbortActive))
                  {
                     /* Store the Current Remote Directory that we are  */
                     /* creating so that we can return the name in the  */
                     /* Confirmation Event.                             */
                     FTPInfo->CurrentObject[0] = BTPS_DIRECTORY_DELIMITER_CHARACTER;
                     FTPInfo->CurrentObject[1] = '\0';

                     /* If we are not currently doing anything as the   */
                     /* current operation then we need to inform the    */
                     /* Remote FTP Server to change the directory for   */
                     /* us.                                             */
                     if((ret_val = _FTPM_Client_Set_Path(FTPInfo->OTPID, NULL, (Boolean_t)FALSE, (Boolean_t)FALSE)) >= 0)
                        FTPInfo->CurrentOperation = coChangeDirectory;
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_FTP_REQUEST_OUTSTANDING;
               }
               else
               {
                  /* Map the correct error code.                        */
                  if(!FTPInfo->FTPServer)
                     ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
                  else
                     ret_val = BTPM_ERROR_CODE_FTP_ACTION_NOT_ALLOWED;
               }
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         }
         else
            ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
   {
      if(Initialized)
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
      else
         ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
   }

   DebugPrint((BTPM_DEBUG_ZONE_FTP | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is provided to allow a means for the FTP   */
   /* Client to Create a Directory on the Remote FTP Server.  This      */
   /* function accepts as its first parameter the FTP Client ID         */
   /* (returned from a successful call to the                           */
   /* FTPM_Open_Remote_File_Server() function).  The second parameter is*/
   /* a pointer to a NULL terminated ASCII string that specifies the    */
   /* Remote Directory Name of the Remote Directory to Create.  This    */
   /* directory name must be specified and cannot contain any path      */
   /* information.  This directory is created in the current working    */
   /* directory on the remote file server.  This function returns zero  */
   /* if successful or a negative return error code if there was an     */
   /* error.                                                            */
   /* * NOTE * The RemoteDirectoryName parameter should be formatted as */
   /*          a NULL terminated ASCII string with UTF-8 encoding.      */
   /* * NOTE * The Bluetooth File Transfer Profile (using OBEX)         */
   /*          specifies that when a Remote Directory is created (and   */
   /*          created successfully) that the new working directory is  */
   /*          this newly created directory.                            */
   /* * NOTE * Issuing this command successfully does not mean that the */
   /*          Remote FTP Server successfully issued the command.  The  */
   /*          caller needs to check the confirmation result to         */
   /*          determine if the Remote FTP Server successfully executed */
   /*          the Request.                                             */
   /* * NOTE * Due to an OBEX FTP limitation, there can only be one     */
   /*          outstanding FTP Client Request active at any one time.   */
   /*          Because of this, another FTP Client Request cannot be    */
   /*          issued until either the current request is Aborted (by   */
   /*          calling the FTPM_Abort() function) or the current Request*/
   /*          is complete (this is signified by receiving a            */
   /*          Confirmation Event in the FTP Client Event Callback that */
   /*          was registered when the FTP Client was opened).          */
int BTPSAPI FTPM_Create_Directory(unsigned int FTPConnectionID, char *RemoteDirectoryName)
{
   int        ret_val;
   FTPInfo_t *FTPInfo;

   DebugPrint((BTPM_DEBUG_ZONE_FTP | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* Make sure that the passed in parameter seems semi-valid.          */
   if((Initialized) && (FTPConnectionID) && (RemoteDirectoryName) && (BTPS_StringLength(RemoteDirectoryName)))
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Make sure that the device is currently powered up.          */
         if(CurrentPowerState)
         {
            /* Search the FTP Profile Information List for the specified*/
            /* FTP Profile Entry.                                       */
            if((FTPInfo = SearchFTPEntry(&FTPInfoList, FTPConnectionID)) != NULL)
            {
               /* This function is only applicable to FTP Clients, so   */
               /* check to see if the current FTP Client Entry is a File*/
               /* Client.                                               */
               if((!FTPInfo->FTPServer) && (FTPInfo->ConnectionState == csConnected))
               {
                  /* Let's make sure that we do not have any currently  */
                  /* outstanding Operations.                            */
                  if((FTPInfo->CurrentOperation == coNone) && (!FTPInfo->AbortActive))
                  {
                     /* Let's make sure that we have a buffer large     */
                     /* enough to store the directory that we are       */
                     /* creating.                                       */
                     if(BTPS_StringLength(RemoteDirectoryName) < sizeof(FTPInfo->CurrentObject))
                     {
                        /* Store the Current Remote Directory that we   */
                        /* are creating so that we can return the name  */
                        /* in the Confirmation Event.                   */
                        BTPS_StringCopy(FTPInfo->CurrentObject, RemoteDirectoryName);

                        /* If we are not currently doing anything as the*/
                        /* current operation then we need to inform the */
                        /* Remote FTP Server to create a directory for  */
                        /* us.                                          */
                        if((ret_val = _FTPM_Client_Set_Path(FTPInfo->OTPID, RemoteDirectoryName, FALSE, TRUE)) >= 0)
                           FTPInfo->CurrentOperation = coCreateDirectory;
                     }
                     else
                        ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_FTP_REQUEST_OUTSTANDING;
               }
               else
               {
                  /* Map the correct error code.                        */
                  if(!FTPInfo->FTPServer)
                     ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
                  else
                     ret_val = BTPM_ERROR_CODE_FTP_ACTION_NOT_ALLOWED;
               }
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         }
         else
            ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
   {
      if(Initialized)
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
      else
         ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
   }

   DebugPrint((BTPM_DEBUG_ZONE_FTP | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is provided to allow a means for the FTP   */
   /* Client to Delete a Directory on the Remote FTP Server.  This      */
   /* function accepts as its first parameter the second parameter      */
   /* specifies the FTP Client ID (returned from a successful call to   */
   /* the FTPM_Open_Remote_File_Server() function).  The second         */
   /* parameter is a pointer to a NULL terminated ASCII string that     */
   /* specifies the Remote Directory Name of the Remote Directory to    */
   /* Delete.  This directory name must be specified and cannot contain */
   /* any path information.  This directory is deleted in the current   */
   /* working directory on the remote file server.  This function       */
   /* returns zero if successful or a negative return error code if     */
   /* there was an error.                                               */
   /* * NOTE * The RemoteDirectoryName parameter should be formatted as */
   /*          a NULL terminated ASCII string with UTF-8 encoding.      */
   /* * NOTE * Due to an OBEX FTP limitation, when a delete operation is*/
   /*          specified, the client cannot be guaranteed whether a file*/
   /*          or directory of that name will be deleted.  In other     */
   /*          words, if the caller specifies this function and a remote*/
   /*          file exists (in the current remote working directory),   */
   /*          the file will be deleted (even thougth it is not a       */
   /*          directory).  This is due to OBEX FTP which does not allow*/
   /*          the specification of what type of Object to delete (only */
   /*          the name of the Object, which could be a file or         */
   /*          directory in this case).                                 */
   /* * NOTE * Issuing this command successfully does not mean that the */
   /*          Remote FTP Server successfully issued the command.  The  */
   /*          caller needs to check the confirmation result to         */
   /*          determine if the Remote FTP Server successfully executed */
   /*          the Request.                                             */
   /* * NOTE * Due to an OBEX FTP limitation, there can only be one     */
   /*          outstanding FTP Client Request active at any one time.   */
   /*          Because of this, another FTP Client Request cannot be    */
   /*          issued until either the current request is Aborted (by   */
   /*          calling the FTPM_Abort() function) or the current Request*/
   /*          is complete (this is signified by receiving a            */
   /*          Confirmation Event in the FTP Client Event Callback that */
   /*          was registered when the FTP Client was opened).          */
int BTPSAPI FTPM_Delete_Directory(unsigned int FTPConnectionID, char *RemoteDirectoryName)
{
   int        ret_val;
   FTPInfo_t *FTPInfo;

   DebugPrint((BTPM_DEBUG_ZONE_FTP | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* Make sure that the passed in parameter seems semi-valid.          */
   if((Initialized) && (FTPConnectionID) && (RemoteDirectoryName) && (BTPS_StringLength(RemoteDirectoryName)))
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Make sure that the device is currently powered up.          */
         if(CurrentPowerState)
         {
            /* Search the FTP Profile Information List for the specified*/
            /* FTP Profile Entry.                                       */
            if((FTPInfo = SearchFTPEntry(&FTPInfoList, FTPConnectionID)) != NULL)
            {
               /* This function is only applicable to FTP Clients, so   */
               /* check to see if the current FTP Client Entry is a File*/
               /* Client.                                               */
               if((!FTPInfo->FTPServer) && (FTPInfo->ConnectionState == csConnected))
               {
                  /* Let's make sure that we do not have any currently  */
                  /* outstanding Operations.                            */
                  if((FTPInfo->CurrentOperation == coNone) && (!FTPInfo->AbortActive))
                  {
                     /* Let's make sure that we have a buffer large     */
                     /* enough to store the directory that we are       */
                     /* deleting.                                       */
                     if(BTPS_StringLength(RemoteDirectoryName) < sizeof(FTPInfo->CurrentObject))
                     {
                        /* Store the Current Remote Directory that we   */
                        /* are deleting so that we can return the name  */
                        /* in the Confirmation Event.                   */
                        BTPS_StringCopy(FTPInfo->CurrentObject, RemoteDirectoryName);

                        /* If we are not currently doing anything as the*/
                        /* current operation then we need to inform the */
                        /* Remote FTP Server to delete a directory for  */
                        /* us.                                          */
                        if((ret_val = _FTPM_Client_Delete_Object_Request(FTPInfo->OTPID, RemoteDirectoryName)) >= 0)
                           FTPInfo->CurrentOperation = coDeleteDirectory;
                     }
                     else
                        ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_FTP_REQUEST_OUTSTANDING;
               }
               else
               {
                  /* Map the correct error code.                        */
                  if(!FTPInfo->FTPServer)
                     ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
                  else
                     ret_val = BTPM_ERROR_CODE_FTP_ACTION_NOT_ALLOWED;
               }
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         }
         else
            ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
   {
      if(Initialized)
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
      else
         ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
   }

   DebugPrint((BTPM_DEBUG_ZONE_FTP | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is provided to allow a means for the FTP   */
   /* Client to Create a File on the Remote FTP Server.  This function  */
   /* accepts as its first parameter the FTP Client ID (returned from a */
   /* successful call to the FTPM_Open_Remote_File_Server() function).  */
   /* The second parameter is a pointer to a NULL terminated ASCII      */
   /* string that specifies the Remote File Name of the Remote File to  */
   /* Create.  This File Name must be specified and cannot contain any  */
   /* path information.  This file is created in the current working    */
   /* directory on the remote file server.  This function returns zero  */
   /* if successful or a negative return error code if there was an     */
   /* error.                                                            */
   /* * NOTE * The RemoteFileName parameter should be formatted as a    */
   /*          NULL terminated ASCII string with UTF-8 encoding.        */
   /* * NOTE * Issuing this command successfully does not mean that the */
   /*          Remote FTP Server successfully issued the command.  The  */
   /*          caller needs to check the confirmation result to         */
   /*          determine if the Remote FTP Server successfully executed */
   /*          the Request.                                             */
   /* * NOTE * Due to an OBEX FTP limitation, there can only be one     */
   /*          outstanding FTP Client Request active at any one time.   */
   /*          Because of this, another FTP Client Request cannot be    */
   /*          issued until either the current request is Aborted (by   */
   /*          calling the FTPM_Abort() function) or the current Request*/
   /*          is complete (this is signified by receiving a            */
   /*          Confirmation Event in the FTP Client Event Callback that */
   /*          was registered when the FTP Client was opened).          */
int BTPSAPI FTPM_Create_File(unsigned int FTPConnectionID, char *RemoteFileName)
{
   int        ret_val;
   FTPInfo_t *FTPInfo;

   DebugPrint((BTPM_DEBUG_ZONE_FTP | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* Make sure that the passed in parameter seems semi-valid.          */
   if((Initialized) && (FTPConnectionID) && (RemoteFileName) && (BTPS_StringLength(RemoteFileName)))
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Make sure that the device is currently powered up.          */
         if(CurrentPowerState)
         {
            /* Search the FTP Profile Information List for the specified*/
            /* FTP Profile Entry.                                       */
            if((FTPInfo = SearchFTPEntry(&FTPInfoList, FTPConnectionID)) != NULL)
            {
               /* This function is only applicable to FTP Clients, so   */
               /* check to see if the current FTP Client Entry is a File*/
               /* Client.                                               */
               if((!FTPInfo->FTPServer) && (FTPInfo->ConnectionState == csConnected))
               {
                  /* Let's make sure that we do not have any currently  */
                  /* outstanding Operations.                            */
                  if((FTPInfo->CurrentOperation == coNone) && (!FTPInfo->AbortActive))
                  {
                     /* Let's make sure that we have a buffer large     */
                     /* enough to store the file that we are creating.. */
                     if(BTPS_StringLength(RemoteFileName) < sizeof(FTPInfo->CurrentObject))
                     {
                        /* Store the Current Remote File that we are    */
                        /* creating so that we can return the name in   */
                        /* the Confirmation Event.                      */
                        BTPS_StringCopy(FTPInfo->CurrentObject, RemoteFileName);

                        /* If we are not currently doing anything as the*/
                        /* current operation then we need to inform the */
                        /* Remote FTP Server to create the file for us. */
                        if((ret_val = _FTPM_Client_Put_Object_Request(FTPInfo->OTPID, TRUE, 0, NULL, RemoteFileName, 0)) >= 0)
                           FTPInfo->CurrentOperation = coCreateFile;
                     }
                     else
                        ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_FTP_REQUEST_OUTSTANDING;
               }
               else
               {
                  /* Map the correct error code.                        */
                  if(!FTPInfo->FTPServer)
                     ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
                  else
                     ret_val = BTPM_ERROR_CODE_FTP_ACTION_NOT_ALLOWED;
               }
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         }
         else
            ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
   {
      if(Initialized)
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
      else
         ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
   }

   DebugPrint((BTPM_DEBUG_ZONE_FTP | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is provided to allow a means for the FTP   */
   /* Client to Delete a File on the Remote FTP Server.  This function  */
   /* accepts as its first parameter the FTP Client ID (returned from a */
   /* successful call to the FTPM_Open_Remote_File_Server() function).  */
   /* The second parameter is a pointer to a NULL terminated ASCII      */
   /* string that specifies the Remote File Name of the Remote File to  */
   /* Delete.  This File Name must be specified and cannot contain any  */
   /* path information.  This file is deleted in the current working    */
   /* directory on the remote file server.  This function returns zero  */
   /* if successful or a negative return error code if there was an     */
   /* error.                                                            */
   /* * NOTE * The RemoteFileName parameter should be formatted as a    */
   /*          NULL terminated ASCII string with UTF-8 encoding.        */
   /* * NOTE * Due to an OBEX FTP limitation, when a delete operation is*/
   /*          specified, the client cannot be guaranteed whether a file*/
   /*          or directory of that name will be deleted.  In other     */
   /*          words, if the caller specifies this function and a remote*/
   /*          directory exists (in the current remote working          */
   /*          directory), the directory will be deleted (even thougth  */
   /*          it is not a file).  This is due to OBEX FTP which does   */
   /*          not allow the specification of what type of Object to    */
   /*          delete (only the name of the Object, which could be a    */
   /*          file or directory in this case).                         */
   /* * NOTE * Issuing this command successfully does not mean that the */
   /*          Remote FTP Server successfully issued the command.  The  */
   /*          caller needs to check the confirmation result to         */
   /*          determine if the Remote FTP Server successfully executed */
   /*          the Request.                                             */
   /* * NOTE * Due to an OBEX FTP limitation, there can only be one     */
   /*          outstanding FTP Client Request active at any one time.   */
   /*          Because of this, another FTP Client Request cannot be    */
   /*          issued until either the current request is Aborted (by   */
   /*          calling the FTPM_Abort() function) or the current Request*/
   /*          is complete (this is signified by receiving a            */
   /*          Confirmation Event in the FTP Client Event Callback that */
   /*          was registered when the FTP Client was opened).          */
int BTPSAPI FTPM_Delete_File(unsigned int FTPConnectionID, char *RemoteFileName)
{
   int        ret_val;
   FTPInfo_t *FTPInfo;

   DebugPrint((BTPM_DEBUG_ZONE_FTP | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* Make sure that the passed in parameter seems semi-valid.          */
   if((Initialized) && (FTPConnectionID) && (RemoteFileName) && (BTPS_StringLength(RemoteFileName)))
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Make sure that the device is currently powered up.          */
         if(CurrentPowerState)
         {
            /* Search the FTP Profile Information List for the specified*/
            /* FTP Profile Entry.                                       */
            if((FTPInfo = SearchFTPEntry(&FTPInfoList, FTPConnectionID)) != NULL)
            {
               /* This function is only applicable to FTP Clients, so   */
               /* check to see if the current FTP Client Entry is a File*/
               /* Client.                                               */
               if((!FTPInfo->FTPServer) && (FTPInfo->ConnectionState == csConnected))
               {
                  /* Let's make sure that we do not have any currently  */
                  /* outstanding Operations.                            */
                  if((FTPInfo->CurrentOperation == coNone) && (!FTPInfo->AbortActive))
                  {
                     /* Let's make sure that we have a buffer large     */
                     /* enough to store the file that we are deleting.  */
                     if(BTPS_StringLength(RemoteFileName) < sizeof(FTPInfo->CurrentObject))
                     {
                        /* Store the Current Remote File that we are    */
                        /* deleting so that we can return the name in   */
                        /* the Confirmation Event.                      */
                        BTPS_StringCopy(FTPInfo->CurrentObject, RemoteFileName);

                        /* If we are not currently doing anything as the*/
                        /* current operation then we need to inform the */
                        /* Remote FTP Server to delete the file for us. */
                        if((ret_val = _FTPM_Client_Delete_Object_Request(FTPInfo->OTPID, RemoteFileName)) >= 0)
                           FTPInfo->CurrentOperation = coDeleteFile;
                     }
                     else
                        ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_FTP_REQUEST_OUTSTANDING;
               }
               else
               {
                  /* Map the correct error code.                        */
                  if(!FTPInfo->FTPServer)
                     ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
                  else
                     ret_val = BTPM_ERROR_CODE_FTP_ACTION_NOT_ALLOWED;
               }
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         }
         else
            ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
   {
      if(Initialized)
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
      else
         ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
   }

   DebugPrint((BTPM_DEBUG_ZONE_FTP | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is responsible for retrieving the specified*/
   /* Remote File from the specified Remote FTP Server.  This function  */
   /* accepts as its input parameters the FTP Client ID (returned from a*/
   /* successful call to the FTPM_Open_Remote_File_Server() function).  */
   /* The third parameter specifies the Remote File Name of the Remote  */
   /* File to retrieve.  This File Name is a pointer to a NULL          */
   /* terminated ASCII string and cannot contain any path information   */
   /* (or be empty or NULL).  The next parameter, RemoteFileLength, is a*/
   /* parameter that specifies the size (in Bytes) of the file that is  */
   /* being retrieved.  The last two parameters specify the local path  */
   /* (and filename) to store the retrieved file.  The LocalPath member */
   /* (if specified) contains the Local Path to store the file name, and*/
   /* the final parameter specifies the Filename to store the file into.*/
   /* The last parameter is not optional and *MUST* be specified.  If   */
   /* the LocalPath parameter is NULL, then the file is written to the  */
   /* current directory (on the local machine).  This function returns  */
   /* zero if successful or a negative return error code if there was an*/
   /* error.                                                            */
   /* * NOTE * The RemoteFileName, LocalPath, and LocalFileName         */
   /*          parameters should be formatted as NULL terminated ASCII  */
   /*          strings with UTF-8 encoding.                             */
   /* * NOTE * The Remote File Length parameter exists simply to allow a*/
   /*          Total Length value to be passed to the caller on File Get*/
   /*          Confirmations.  This value can be any value the caller   */
   /*          would like it to be because the FTP Profile ignores this */
   /*          value and simply passes this value back to the caller in */
   /*          the File Get Confirmation Event.                         */
   /* * NOTE * Issuing this command successfully does not mean that the */
   /*          Remote FTP Server successfully issued the command.  The  */
   /*          caller needs to check the confirmation result to         */
   /*          determine if the Remote FTP Server successfully executed */
   /*          the Request.                                             */
   /* * NOTE * Due to an OBEX FTP limitation, there can only be one     */
   /*          outstanding FTP Client Request active at any one time.   */
   /*          Because of this, another FTP Client Request cannot be    */
   /*          issued until either the current request is Aborted (by   */
   /*          calling the FTPM_Abort() function) or the current Request*/
   /*          is complete (this is signified by receiving a            */
   /*          Confirmation Event in the FTP Client Event Callback that */
   /*          was registered when the FTP Client was opened).          */
int BTPSAPI FTPM_Get_File(unsigned int FTPConnectionID, char *RemoteFileName, unsigned int RemoteFileLength, char *LocalPath, char *LocalFileName)
{
   int                     ret_val;
   FTPInfo_t              *FTPInfo;
   BTPS_File_Descriptor_t  FileDescriptor;

   DebugPrint((BTPM_DEBUG_ZONE_FTP | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* Make sure that the passed in parameter seems semi-valid.          */
   if((Initialized) && (FTPConnectionID) && (RemoteFileName) && (BTPS_StringLength(RemoteFileName)) && ((!LocalPath) || ((LocalPath) && (BTPS_StringLength(LocalPath)))) && (LocalFileName) && (BTPS_StringLength(LocalFileName)))
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Make sure that the device is currently powered up.          */
         if(CurrentPowerState)
         {
            /* Search the FTP Profile Information List for the specified*/
            /* FTP Profile Entry.                                       */
            if((FTPInfo = SearchFTPEntry(&FTPInfoList, FTPConnectionID)) != NULL)
            {
               /* This function is only applicable to FTP Clients, so   */
               /* check to see if the current FTP Client Entry is a File*/
               /* Client.                                               */
               if((!FTPInfo->FTPServer) && (FTPInfo->ConnectionState == csConnected))
               {
                  /* Let's make sure that we do not have any currently  */
                  /* outstanding Operations.                            */
                  if((FTPInfo->CurrentOperation == coNone) && (!FTPInfo->AbortActive))
                  {
                     /* Let's make sure that we have a buffer large     */
                     /* enough to store the file that we are retrieving.*/
                     if(BTPS_StringLength(RemoteFileName) < sizeof(FTPInfo->CurrentObject))
                     {
                        /* Store the Current Remote File that we are    */
                        /* transferring from the remote file server so  */
                        /* that we can return the name in the           */
                        /* Confirmation Event.                          */
                        BTPS_StringCopy(FTPInfo->CurrentObject, RemoteFileName);

                        /* Initialize that there have been no parsing   */
                        /* errors.                                      */
                        ret_val = 0;

                        /* OK, now let's build the Local Filename with  */
                        /* any specified path information into the      */
                        /* Current Directory member of the FTP Context  */
                        /* Information structure.                       */
                        if(LocalPath)
                        {
                           if((BTPS_StringLength(LocalPath) + 2) < sizeof(FTPInfo->CurrentDirectory))
                           {
                              BTPS_StringCopy(FTPInfo->CurrentDirectory, LocalPath);

                              /* If there is no trailing directory      */
                              /* delimiter character then we need to    */
                              /* make sure that the path has the        */
                              /* directory delimiter character at the   */
                              /* end of it.                             */
                              if(FTPInfo->CurrentDirectory[BTPS_StringLength(FTPInfo->CurrentDirectory)-1] != BTPS_DIRECTORY_DELIMITER_CHARACTER)
                              {
                                 FTPInfo->CurrentDirectory[BTPS_StringLength(FTPInfo->CurrentDirectory) + 1] = '\0';
                                 FTPInfo->CurrentDirectory[BTPS_StringLength(FTPInfo->CurrentDirectory)]     = BTPS_DIRECTORY_DELIMITER_CHARACTER;
                              }
                           }
                           else
                              ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
                        }
                        else
                           FTPInfo->CurrentDirectory[0] = '\0';

                        /* Only add the File Name to the Current Path if*/
                        /* there have been no parsing errors.           */
                        if(!ret_val)
                        {
                           /* Next, let's concatenate the Local File    */
                           /* Name to the Local Path Information to     */
                           /* build a fully qualified path name for the */
                           /* Local File.                               */
                           if((BTPS_StringLength(LocalFileName) + BTPS_StringLength(FTPInfo->CurrentDirectory) + 1) < sizeof(FTPInfo->CurrentDirectory))
                           {
                              BTPS_SprintF(&(FTPInfo->CurrentDirectory[BTPS_StringLength(FTPInfo->CurrentDirectory)]), "%s", LocalFileName);

                              /* Now that we have built the fully       */
                              /* qualified path for the Local File Name */
                              /* we need to make sure that we can       */
                              /* actually create (and write to) the     */
                              /* file.                                  */
                              if((FileDescriptor = BTPS_Open_File(FTPInfo->CurrentDirectory, omCreate)) != NULL)
                              {
                                 /* All Finished with the File, so let's*/
                                 /* close the file that we opened.      */
                                 BTPS_Close_File(FileDescriptor);

                                 /* Let's go ahead an delete the file,  */
                                 /* in case there is an error (so we    */
                                 /* don't leave an empty file hanging   */
                                 /* around.                             */
                                 BTPS_Delete_File(FTPInfo->CurrentDirectory);

                                 /* Note the existing File statistics.  */
                                 FTPInfo->FileInfo.FileSize       = RemoteFileLength;
                                 FTPInfo->FileInfo.FileIndex      = 0;
                                 FTPInfo->FileInfo.FirstPhase     = TRUE;
                                 FTPInfo->FileInfo.FileDescriptor = NULL;

                                 /* Finally, let's submit the FTP Get   */
                                 /* File Request.                       */
                                 if((ret_val = _FTPM_Client_Get_Object(FTPInfo->OTPID, NULL, FTPInfo->CurrentObject, 0)) >= 0)
                                    FTPInfo->CurrentOperation = coFileGet;
                              }
                              else
                                 ret_val = BTPM_ERROR_CODE_FTP_UNABLE_TO_CREATE_LOCAL_FILE;
                           }
                           else
                              ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
                        }
                     }
                     else
                        ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_FTP_REQUEST_OUTSTANDING;
               }
               else
               {
                  /* Map the correct error code.                        */
                  if(!FTPInfo->FTPServer)
                     ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
                  else
                     ret_val = BTPM_ERROR_CODE_FTP_ACTION_NOT_ALLOWED;
               }
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         }
         else
            ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
   {
      if(Initialized)
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
      else
         ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
   }

   DebugPrint((BTPM_DEBUG_ZONE_FTP | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is responsible for sending the specified   */
   /* Local File to the specified Remote FTP Server.  This function     */
   /* accepts as its input parameters the FTP Client ID (returned from a*/
   /* successful call to the FTPM_Open_Remote_File_Server() function).  */
   /* The second and third parameters specify the Local Path and Local  */
   /* Filename of the source file.  These two parameters are pointers to*/
   /* NULL terminated ASCII strings which specify the Path and File Name*/
   /* (respectively) of the Local File.  The Local Path parameter is    */
   /* optional, and if NON-NULL specifies the Local Path of the Local   */
   /* File.  The Local File Name parameter is NOT optional and specifies*/
   /* the File Name of the Local File.  The RemoteFileName parameter    */
   /* specifies the name of the file that is to be stored on the Remote */
   /* FTP Server.  This file name cannot specify any path information.  */
   /* The file that is sent to the Remote FTP Server is stored in the   */
   /* current working directory on the Remote FTP Server.  This function*/
   /* returns zero if successful or a negative return error code if     */
   /* there was an error.                                               */
   /* * NOTE * The LocalPath, LocalFileName, and RemoteFileName         */
   /*          parameters should be formatted as NULL terminated ASCII  */
   /*          strings with UTF-8 encoding.                             */
   /* * NOTE * Issuing this command successfully does not mean that the */
   /*          Remote FTP Server successfully issued the command.  The  */
   /*          caller needs to check the confirmation result to         */
   /*          determine if the Remote FTP Server successfully executed */
   /*          the Request.                                             */
   /* * NOTE * Due to an OBEX FTP limitation, there can only be one     */
   /*          outstanding FTP Client Request active at any one time.   */
   /*          Because of this, another FTP Client Request cannot be    */
   /*          issued until either the current request is Aborted (by   */
   /*          calling the FTPM_Abort() function) or the current Request*/
   /*          is complete (this is signified by receiving a            */
   /*          Confirmation Event in the FTP Client Event Callback that */
   /*          was registered when the FTP Client was opened).          */
int BTPSAPI FTPM_Put_File(unsigned int FTPConnectionID, char *LocalPath, char *LocalFileName, char *RemoteFileName)
{
   int                      ret_val;
   FTPInfo_t               *FTPInfo;
   unsigned int             Length;
   BTPS_File_Information_t  FileInformation;

   DebugPrint((BTPM_DEBUG_ZONE_FTP | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* Make sure that the passed in parameter seems semi-valid.          */
   if((Initialized) && (FTPConnectionID) && (RemoteFileName) && (BTPS_StringLength(RemoteFileName)) && ((!LocalPath) || ((LocalPath) && (BTPS_StringLength(LocalPath)))) && (LocalFileName) && (BTPS_StringLength(LocalFileName)))
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Make sure that the device is currently powered up.          */
         if(CurrentPowerState)
         {
            /* Search the FTP Profile Information List for the specified*/
            /* FTP Profile Entry.                                       */
            if((FTPInfo = SearchFTPEntry(&FTPInfoList, FTPConnectionID)) != NULL)
            {
               /* This function is only applicable to FTP Clients, so   */
               /* check to see if the current FTP Client Entry is a File*/
               /* Client.                                               */
               if((!FTPInfo->FTPServer) && (FTPInfo->ConnectionState == csConnected))
               {
                  /* Let's make sure that we do not have any currently  */
                  /* outstanding Operations.                            */
                  if((FTPInfo->CurrentOperation == coNone) && (!FTPInfo->AbortActive))
                  {
                     /* Let's make sure that we have a buffer large     */
                     /* enough to store the file that we are sending.   */
                     if(BTPS_StringLength(RemoteFileName) < sizeof(FTPInfo->CurrentObject))
                     {
                        /* Store the Current Local File that we are     */
                        /* transferring from the local machine to the   */
                        /* remote file server that we can return the    */
                        /* name in the Confirmation Event.              */
                        BTPS_StringCopy(FTPInfo->CurrentObject, RemoteFileName);

                        /* Initialize that there have been no parsing   */
                        /* errors.                                      */
                        ret_val = 0;

                        /* OK, now let's build the Local Filename with  */
                        /* any specified path information into the      */
                        /* Current Directory member of the FTP Context  */
                        /* Information structure.                       */
                        if(LocalPath)
                        {
                           if((BTPS_StringLength(LocalPath) + 2) < sizeof(FTPInfo->CurrentDirectory))
                           {
                              BTPS_StringCopy(FTPInfo->CurrentDirectory, LocalPath);

                              /* If there is no trailing directory      */
                              /* delimiter character then we need to    */
                              /* make sure that the path has the        */
                              /* directory delimiter character at the   */
                              /* end of it.                             */
                              if(FTPInfo->CurrentDirectory[BTPS_StringLength(FTPInfo->CurrentDirectory)-1] != BTPS_DIRECTORY_DELIMITER_CHARACTER)
                              {
                                 FTPInfo->CurrentDirectory[BTPS_StringLength(FTPInfo->CurrentDirectory) + 1] = '\0';
                                 FTPInfo->CurrentDirectory[BTPS_StringLength(FTPInfo->CurrentDirectory)]     = BTPS_DIRECTORY_DELIMITER_CHARACTER;
                              }
                           }
                           else
                              ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
                        }
                        else
                           FTPInfo->CurrentDirectory[0] = '\0';

                        /* Only add the File Name to the Current Path if*/
                        /* there have been no parsing errors.           */
                        if(!ret_val)
                        {
                           /* Next, let's concatenate the Local File    */
                           /* Name to the Local Path Information to     */
                           /* build a fully qualified path name for the */
                           /* Local File.                               */
                           if((BTPS_StringLength(LocalFileName) + BTPS_StringLength(FTPInfo->CurrentDirectory) + 1) < sizeof(FTPInfo->CurrentDirectory))
                           {
                              BTPS_SprintF(&(FTPInfo->CurrentDirectory[BTPS_StringLength(FTPInfo->CurrentDirectory)]), "%s", LocalFileName);

                              /* Next, let's try to allocate an input   */
                              /* Buffer so that we can send the actual  */
                              /* File Data to the remote server.        */
                              if((FTPInfo->PutFileBuffer = (Byte_t *)BTPS_AllocateMemory(OTP_DEFAULT_PACKET_LENGTH)) != NULL)
                              {
                                 /* Now that we have built the fully    */
                                 /* qualified path for the Local File   */
                                 /* Name we need to make sure that we   */
                                 /* can actually find (and read from)   */
                                 /* the file.                           */
                                 if(BTPS_Query_File_Information(FTPInfo->CurrentDirectory, &FileInformation))
                                 {
                                    /* Note the Length of the file so we*/
                                    /* can inform remote server how     */
                                    /* large the file is.               */
                                    Length = FileInformation.FileSize;

                                    /* Note the existing File           */
                                    /* statistics.                      */
                                    FTPInfo->FileInfo.FileSize       = (unsigned int)Length;
                                    FTPInfo->FileInfo.FileIndex      = 0;
                                    FTPInfo->FileInfo.FirstPhase     = TRUE;
                                    FTPInfo->FileInfo.FileDescriptor = NULL;

                                    /* Finally, let's submit the FTP Put*/
                                    /* File Request.                    */
                                    if((ret_val = _FTPM_Client_Put_Object_Request(FTPInfo->OTPID, (Boolean_t)(Length?FALSE:TRUE), (unsigned int)Length, NULL, FTPInfo->CurrentObject, 0)) >= 0)
                                       FTPInfo->CurrentOperation = coFilePut;
                                    else
                                    {
                                       /* Free the memory that we       */
                                       /* allocated.                    */
                                       BTPS_FreeMemory(FTPInfo->PutFileBuffer);

                                       FTPInfo->PutFileBuffer = NULL;
                                    }
                                 }
                                 else
                                 {
                                    /* Free the memory that we          */
                                    /* allocated.                       */
                                    BTPS_FreeMemory(FTPInfo->PutFileBuffer);

                                    FTPInfo->PutFileBuffer = NULL;

                                    ret_val                = BTPM_ERROR_CODE_FTP_UNABLE_TO_CREATE_LOCAL_FILE;
                                 }
                              }
                              else
                                 ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
                           }
                           else
                              ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
                        }
                     }
                     else
                        ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_FTP_REQUEST_OUTSTANDING;
               }
               else
               {
                  /* Map the correct error code.                        */
                  if(!FTPInfo->FTPServer)
                     ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
                  else
                     ret_val = BTPM_ERROR_CODE_FTP_ACTION_NOT_ALLOWED;
               }
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         }
         else
            ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
   {
      if(Initialized)
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
      else
         ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
   }

   DebugPrint((BTPM_DEBUG_ZONE_FTP | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is responsible for Aborting ANY currently  */
   /* outstanding FTP Client Request.  This function accepts as its     */
   /* input parameters the FTP Client ID (returned from a successful    */
   /* call to the FTPM_Open_Remote_File_Server() function).  This       */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error.                                       */
   /* * NOTE * This FTP Client Request is no different than the rest of */
   /*          the FTP Client Request functions in that a response to   */
   /*          this Request must be received before any other FTP Client*/
   /*          Request can be issued.                                   */
   /* * NOTE * Because of transmission latencies, it may be possible    */
   /*          that a FTP Client Request that is to be aborted may have */
   /*          completed before the server was able to Abort the        */
   /*          request.  In either case, the caller will be notified via*/
   /*          FTP Client Callback of the status of the previous        */
   /*          Request.                                                 */
   /* * NOTE * Due to the nature of only one outstanding OBEX Command   */
   /*          when an Abort is issued, it may be queued (for           */
   /*          transmission when the response to the currently          */
   /*          outstanding OBEX Command is received.  A problem can     */
   /*          occur if the Remote OBEX Server does not respond to the  */
   /*          original request because the queued Abort Packet will    */
   /*          never be sent.  This is a problem because no new OBEX    */
   /*          commands can be issued because the OBEX layer on the     */
   /*          local machine thinks a Request is outstanding and will   */
   /*          not issue another request.  To aid in error recovery,    */
   /*          this function forces an Abort Request out (and the       */
   /*          clearing of the current OBEX Command Request) if this    */
   /*          function is called twice.  An application can call this  */
   /*          function a second time to force a local cleanup if a     */
   /*          response on the first Abort Packet is never received (via*/
   /*          the FTP Client Callback).  It should be noted that under */
   /*          normal circumstances (i.e.  the Remote Server is         */
   /*          functioning properly) this function will NEVER have to be*/
   /*          called twice.                                            */
int BTPSAPI FTPM_Abort(unsigned int FTPConnectionID)
{
   int        ret_val;
   FTPInfo_t *FTPInfo;

   DebugPrint((BTPM_DEBUG_ZONE_FTP | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* Make sure that the passed in parameter seems semi-valid.          */
   if((Initialized) && (FTPConnectionID))
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Make sure that the device is currently powered up.          */
         if(CurrentPowerState)
         {
            /* Search the FTP Profile Information List for the specified*/
            /* FTP Profile Entry.                                       */
            if((FTPInfo = SearchFTPEntry(&FTPInfoList, FTPConnectionID)) != NULL)
            {
               /* This function is only applicable to FTP Clients, so   */
               /* check to see if the current FTP Client Entry is a File*/
               /* Client.                                               */
               if((!FTPInfo->FTPServer) && (FTPInfo->ConnectionState == csConnected))
               {
                  /* If we are not in the process or Aborting the       */
                  /* current operation, simply issue an FTPM Mgr Abort  */
                  /* Request.                                           */
                  if(!FTPInfo->AbortActive)
                  {
                     /* If an Abort action is already queued then we    */
                     /* will go ahead and send an Abort Command         */
                     /* Immediately.  If an Abort is NOT active and we  */
                     /* do not have an Abort Queued then we will simply */
                     /* Queue the Abort Command.                        */
                     /* * NOTE * If there is no command outstanding then*/
                     /*          we can simply issue the Abort          */
                     /*          Immediately.                           */
                     if((FTPInfo->AbortQueued) || (FTPInfo->CurrentOperation == coNone))
                     {
                        /* Flag that there is no Abort Queued.          */
                        FTPInfo->AbortQueued = FALSE;

                        if((ret_val = _FTPM_Client_Abort_Request(FTPInfo->OTPID)) >= 0)
                           FTPInfo->AbortActive = TRUE;

                        /* If there was an ongoing transfer, we need to */
                        /* make sure we close any open file.            */
                        if(FTPInfo->FileInfo.FileDescriptor)
                        {
                           BTPS_Close_File(FTPInfo->FileInfo.FileDescriptor);

                           FTPInfo->FileInfo.FileDescriptor = NULL;
                        }
                     }
                     else
                     {
                        FTPInfo->AbortQueued = TRUE;

                        ret_val              = 0;
                     }
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_FTP_REQUEST_OUTSTANDING;
               }
               else
               {
                  /* Map the correct error code.                        */
                  if(!FTPInfo->FTPServer)
                     ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
                  else
                     ret_val = BTPM_ERROR_CODE_FTP_ACTION_NOT_ALLOWED;
               }
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         }
         else
            ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
   {
      if(Initialized)
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
      else
         ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
   }

   DebugPrint((BTPM_DEBUG_ZONE_FTP | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Return the result to the caller.                                  */
   return(ret_val);
}

