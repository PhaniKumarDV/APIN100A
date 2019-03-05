/*****< phonebook.c >**********************************************************/
/*      Copyright 2009 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  PHONEBOOK - Stonestreet One Bluetooth Stack Phonebook Access Profile      */
/*              (PBAP) Phonebook Implementation for Stonestreet One           */
/*              Bluetooth Protocol Stack sample application.                  */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   07/03/09  D. Lange       Initial creation.                               */
/******************************************************************************/
#include "Phonebook.h"     /* Phonebook Implementation Protoypes/Constants.   */

#include "BTPSKRNL.h"      /* Bluetooth O/S Abstration Prototypes/Constants.  */

   /* The following constants represent the hard-coded Phonebook path   */
   /* vCards.                                                           */
#define TELECOM_PHONEBOOK_VCARD_CONTENTS_INDEX_0_DATA                         \
   "BEGIN:VCARD\x0D\x0A"                                                      \
   "VERSION:2.1\x0D\x0A"                                                      \
   "FN:Andy Miyajima\x0D\x0A"                                                 \
   "N:Miyajima;AndyQ;;\x0D\x0A"                                               \
   "EMAIL;INTERNET:amiyajima@somewebsite.com\x0D\x0A"                         \
   "TEL;PREF;WORK:555-555-5555\x0D\x0A"                                       \
   "ADR;POSTAL;WORK:123 Some Street;Some City and State;;;;99999;USA\x0D\x0A" \
   "END:VCARD\x0D\x0A"

#define TELECOM_PHONEBOOK_VCARD_CONTENTS_INDEX_1_DATA                         \
   "BEGIN:VCARD\x0D\x0A"                                                      \
   "VERSION:2.1\x0D\x0A"                                                      \
   "FN:Guillaume Paujade\x0D\x0A"                                             \
   "N:Poujade;Guillaume;;\x0D\x0A"                                            \
   "EMAIL;INTERNET:gpoujade@somewebsite.com\x0D\x0A"                          \
   "TEL;PREF;WORK:555-555-5554\x0D\x0A"                                       \
   "ADR;POSTAL;WORK:456 Some Street;Some City and State;;;;99999;USA\x0D\x0A" \
   "END:VCARD\x0D\x0A"

#define TELECOM_PHONEBOOK_VCARD_CONTENTS_INDEX_2_DATA                         \
   "BEGIN:VCARD\x0D\x0A"                                                      \
   "VERSION:2.1\x0D\x0A"                                                      \
   "FN:Scott Hung\x0D\x0A"                                                    \
   "N:Hung;Scott;;\x0D\x0A"                                                   \
   "EMAIL;INTERNET:shung@someotherwebsite.com\x0D\x0A"                        \
   "TEL;PREF;WORK:555-555-5553\x0D\x0A"                                       \
   "ADR;POSTAL;WORK:789 Some Street;Some City and State;;;;99999;USA\x0D\x0A" \
   "END:VCARD\x0D\x0A"

#define TELECOM_PHONEBOOK_VCARD_CONTENTS_INDEX_4_DATA                         \
   "BEGIN:VCARD\x0D\x0A"                                                      \
   "VERSION:2.1\x0D\x0A"                                                      \
   "FN:Arthur Afonso\x0D\x0A"                                                 \
   "N:Afonso;Arthur;;\x0D\x0A"                                                \
   "EMAIL;INTERNET:aafonso@yetanotherwebsite.com\x0D\x0A"                     \
   "TEL;PREF;WORK:555-555-5552\x0D\x0A"                                       \
   "ADR;POSTAL;WORK:234 Some Street;Some City and State;;;;99999;USA\x0D\x0A" \
   "END:VCARD\x0D\x0A"

   /* The following constants represent the hard-coded Incoming Call    */
   /* History path vCards.                                              */
#define TELECOM_INCOMING_CALL_HISTORY_VCARD_CONTENTS_INDEX_1_DATA             \
   "BEGIN:VCARD\x0D\x0A"                                                      \
   "VERSION:2.1\x0D\x0A"                                                      \
   "N:Afonso;Arthur;;\x0D\x0A"                                                \
   "TEL;PREF;WORK:555-555-5552\x0D\x0A"                                       \
   "X-IrMC-INCOMING-CALL-DATETIME:20090101T011002\0x0D\0x0A"                  \
   "END:VCARD\x0D\x0A"

#define TELECOM_INCOMING_CALL_HISTORY_VCARD_CONTENTS_INDEX_2_DATA             \
   "BEGIN:VCARD\x0D\x0A"                                                      \
   "VERSION:2.1\x0D\x0A"                                                      \
   "N:;;\x0D\x0A"                                                             \
   "TEL;PREF;WORK:555-555-5555\x0D\x0A"                                       \
   "X-IrMC-INCOMING-CALL-DATETIME:20090501T123000\0x0D\0x0A"                  \
   "END:VCARD\x0D\x0A"

   /* The following constants represent the hard-coded Outgoing Call    */
   /* History path vCards.                                              */
#define TELECOM_OUTGOING_CALL_HISTORY_VCARD_CONTENTS_INDEX_1_DATA             \
   "BEGIN:VCARD\x0D\x0A"                                                      \
   "VERSION:2.1\x0D\x0A"                                                      \
   "N:Hung;Scott;;\x0D\x0A"                                                   \
   "TEL;PREF;WORK:555-555-5553\x0D\x0A"                                       \
   "X-IrMC-OUTGOING-CALL-DATETIME:20090502T171500\0x0D\0x0A"                  \
   "END:VCARD\x0D\x0A"

   /* The following constants represent the hard-coded Missed Call      */
   /* History path vCards.                                              */
#define TELECOM_MISSED_CALL_HISTORY_VCARD_CONTENTS_INDEX_1_DATA               \
   "BEGIN:VCARD\x0D\x0A"                                                      \
   "VERSION:2.1\x0D\x0A"                                                      \
   "N:;;\x0D\x0A"                                                             \
   "TEL;PREF;WORK:555-555-5551\x0D\x0A"                                       \
   "X-IrMC-MISSED-CALL-DATETIME:20090502T153000\0x0D\0x0A"                    \
   "END:VCARD\x0D\x0A"

   /* The following constant holds all (hard-coded) vCards supported in */
   /* the Phonebook (PBAP_PHONEBOOK_PATH_NAME).                         */
static char *PhonebookvCards[] =
{
   TELECOM_PHONEBOOK_VCARD_CONTENTS_INDEX_0_DATA,
   TELECOM_PHONEBOOK_VCARD_CONTENTS_INDEX_1_DATA,
   TELECOM_PHONEBOOK_VCARD_CONTENTS_INDEX_2_DATA,
   NULL,
   TELECOM_PHONEBOOK_VCARD_CONTENTS_INDEX_4_DATA
} ;

   /* The following constant holds all (hard-coded) vCards supported in */
   /* the Incoming Call History (PBAP_INCOMING_CALL_HISTORY_PATH_NAME). */
static char *IncomingCallHistoryvCards[] =
{
   NULL,
   TELECOM_INCOMING_CALL_HISTORY_VCARD_CONTENTS_INDEX_1_DATA,
   TELECOM_INCOMING_CALL_HISTORY_VCARD_CONTENTS_INDEX_2_DATA
} ;

   /* The following constant holds all (hard-coded) vCards supported in */
   /* the Outgoing Call History (PBAP_OUTGOING_CALL_HISTORY_PATH_NAME). */
static char *OutgoingCallHistoryvCards[] =
{
   NULL,
   TELECOM_OUTGOING_CALL_HISTORY_VCARD_CONTENTS_INDEX_1_DATA
} ;

   /* The following constant holds all (hard-coded) vCards supported in */
   /* the Missed Call History (PBAP_MISSED_CALL_HISTORY_PATH_NAME).     */
static char *MissedCallHistoryvCards[] =
{
   NULL,
   TELECOM_MISSED_CALL_HISTORY_VCARD_CONTENTS_INDEX_1_DATA
} ;

   /* The following structure is provided to allow a mechanism to map   */
   /* between a directory and the vCards that are contained in that     */
   /* directory.                                                        */
typedef struct _tagDirectoryvCardsMapping_t
{
   CurrentDirectory_t   Directory;
   char               **vCardList;
   unsigned int         NumberCardEntries;
} DirectoryvCardsMapping_t;

   /* The following structure is provided to allow a mechanism to       */
   /* determine the parent directory of a specific directory (and thus  */
   /* define a directory structure).                                    */
typedef struct _tagDirectoryStructure_t
{
   CurrentDirectory_t ParentDirectory;
   CurrentDirectory_t CurrentDirectory;
} DirectoryStructure_t;

typedef struct _tagDirectoryNameMapping_t
{
   CurrentDirectory_t  Directory;
   char               *DirectoryName;
} DirectoryNameMapping_t;

typedef struct _tagConnectionEntry_t
{
   unsigned int                  ConnectionID;
   CurrentDirectory_t            CurrentDirectory;
   struct _tagConnectionEntry_t *NextConnectionEntryPtr;
} ConnectionEntry_t;

   /* The following table defines the actual vCard data that maps to a  */
   /* particular Directory.                                             */
static BTPSCONST DirectoryvCardsMapping_t DirectoryvCardsMapping[] =
{
   { cdRoot,                NULL,                      0                                                  },
   { cdTelecom,             NULL,                      0                                                  },
   { cdPhonebook,           PhonebookvCards,           (sizeof(PhonebookvCards)/sizeof(char *))           },
   { cdIncomingCallHistory, IncomingCallHistoryvCards, (sizeof(IncomingCallHistoryvCards)/sizeof(char *)) },
   { cdOutgoingCallHistory, OutgoingCallHistoryvCards, (sizeof(OutgoingCallHistoryvCards)/sizeof(char *)) },
   { cdMissedCallHistory,   MissedCallHistoryvCards,   (sizeof(MissedCallHistoryvCards)/sizeof(char *))   },
   { cdCombinedCallHistory, NULL,                      0                                                  }
} ;

   /* The following table defines the actual directory structure        */
   /* supported by this module (lists the parent directories of each    */
   /* supported directory).                                             */
static BTPSCONST DirectoryStructure_t DirectoryStructure[] =
{
   { cdRoot,    cdTelecom             },
   { cdTelecom, cdPhonebook           },
   { cdTelecom, cdIncomingCallHistory },
   { cdTelecom, cdOutgoingCallHistory },
   { cdTelecom, cdMissedCallHistory   },
   { cdTelecom, cdCombinedCallHistory }
} ;

   /* The following table defines the mapping from actual PBAP directory*/
   /* object names to the numberic values used by this module.          */
static BTPSCONST DirectoryNameMapping_t DirectoryMapping[] =
{
   { cdRoot,                "root"                               },
   { cdTelecom,             PBAP_TELECOM_PATH_NAME               },
   { cdPhonebook,           PBAP_PHONEBOOK_PATH_NAME             },
   { cdIncomingCallHistory, PBAP_INCOMING_CALL_HISTORY_PATH_NAME },
   { cdOutgoingCallHistory, PBAP_OUTGOING_CALL_HISTORY_PATH_NAME },
   { cdMissedCallHistory,   PBAP_MISSED_CALL_HISTORY_PATH_NAME   },
   { cdCombinedCallHistory, PBAP_COMBINED_CALL_HISTORY_PATH_NAME }
} ;

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the compiler*/
   /* as part of standard C/C++).                                       */

   /* Pointer to the first entry in the list of connected devices for   */
   /* which we are tracking the phonebook.                              */
static ConnectionEntry_t *ConnectionListHead;

   /* Internal Function Prototypes.                                     */
static Boolean_t VerifyPhonebookDirectoryPath(char *PhonebookDirectoryPath, unsigned int PhonebookDirectoryPathLength);

static ConnectionEntry_t *FindConnection(unsigned int ConnectionID);
   
   /* The following function is a utility function that exists to       */
   /* determine if the specified Phonebook Directory Path Name is valid.*/
   /* The first parameter specifies the Phonebook Path, and the second  */
   /* parameter specifies the length (in characters) of the specified   */
   /* Phonebook path.  Note that the Phonebook Path does not have to be */
   /* NULL terminated.  This function returns a BOOLEAN TRUE if the path*/
   /* is valid, or FALSE if the path is invalid.                        */
static Boolean_t VerifyPhonebookDirectoryPath(char *PhonebookDirectoryPath, unsigned int PhonebookDirectoryPathLength)
{
   char               *Ptr;
   Boolean_t           ret_val;
   Boolean_t           Done;
   unsigned int        Index;
   unsigned int        Index1;
   CurrentDirectory_t  CurDir;

   if((PhonebookDirectoryPath) && (PhonebookDirectoryPathLength))
   {
      Ptr     = PhonebookDirectoryPath;
      Done    = FALSE;
      CurDir  = cdRoot;
      ret_val = FALSE;

      while(!Done)
      {
         /* If we are currently at the delimeter character then we need */
         /* to skip over it.                                            */
         if(*Ptr == PBAP_PATH_DELIMETER_CHARACTER)
         {
            Ptr++;

            PhonebookDirectoryPathLength--;
         }

         for(Index=0;(Index<sizeof(DirectoryMapping)/sizeof(DirectoryNameMapping_t)) && (!Done);Index++)
         {
            if(PhonebookDirectoryPathLength)
            {
               if((PhonebookDirectoryPathLength == BTPS_StringLength(DirectoryMapping[Index].DirectoryName)) && (!BTPS_MemCompareI(Ptr, DirectoryMapping[Index].DirectoryName, BTPS_StringLength(DirectoryMapping[Index].DirectoryName))))
               {
                  /* OK, we found a match.  Now check to see if this is */
                  /* a valid directory structure.                       */
                  for(Index1=0;Index1<sizeof(DirectoryStructure)/sizeof(DirectoryStructure_t);Index1++)
                  {
                     if((CurDir == DirectoryStructure[Index1].ParentDirectory) && (DirectoryStructure[Index1].CurrentDirectory == DirectoryMapping[Index].Directory))
                     {
                        Ptr                          += BTPS_StringLength(DirectoryMapping[Index].DirectoryName);

                        PhonebookDirectoryPathLength -= BTPS_StringLength(DirectoryMapping[Index].DirectoryName);

                        if(!PhonebookDirectoryPathLength)
                        {
                           Done    = TRUE;

                           ret_val = TRUE;
                        }

                        break;
                     }
                  }

                  /* Check to see if an invalid directory was specified.*/
                  if(Index1 == (sizeof(DirectoryStructure)/sizeof(DirectoryStructure_t)))
                     Done = TRUE;
               }
            }
            else
               Done = TRUE;
         }
      }
   }
   else
      ret_val = FALSE;

   return(ret_val);
}

   /* Find a connection that we are tracking.                           */
static ConnectionEntry_t *FindConnection(unsigned int ConnectionID)
{
   ConnectionEntry_t *ConnectionEntry = ConnectionListHead;

   while((ConnectionEntry) && (ConnectionEntry->ConnectionID != ConnectionID))
      ConnectionEntry = ConnectionEntry->NextConnectionEntryPtr;

   return(ConnectionEntry);
}

   /* Starts tracking the new connection in the phonebook.              */
Boolean_t AddConnection(unsigned int ConnectionID)
{
   Boolean_t          ret_val         = TRUE;
   ConnectionEntry_t *ConnectionEntry;
   ConnectionEntry_t *tmpEntry;

   if((ConnectionEntry = BTPS_AllocateMemory(sizeof(ConnectionEntry_t))) != NULL)
   {
      ConnectionEntry->ConnectionID           = ConnectionID;
      ConnectionEntry->CurrentDirectory       = cdRoot;
      ConnectionEntry->NextConnectionEntryPtr = NULL;
      
      if(ConnectionListHead)
      {
         tmpEntry = ConnectionListHead;

         while(tmpEntry)
         {
            if(tmpEntry->ConnectionID == ConnectionID)
            {
               ret_val = FALSE;
               break;
            }

            if(tmpEntry->NextConnectionEntryPtr)
               tmpEntry = tmpEntry->NextConnectionEntryPtr;
            else
               break;
         }

         if(ret_val)
         {
            tmpEntry->NextConnectionEntryPtr = ConnectionEntry;
         }
         else
            BTPS_FreeMemory(ConnectionEntry);
      }
      else
         ConnectionListHead = ConnectionEntry;
   }
   else
      ret_val = FALSE;

   return(ret_val);
}

   /* Ends tracking a connection in the phonebook.                      */
void RemoveConnection(unsigned int ConnectionID)
{
   ConnectionEntry_t *ConnectionEntry = ConnectionListHead;
   ConnectionEntry_t *PreviousEntry   = NULL;

   while((ConnectionEntry) && (ConnectionEntry->ConnectionID != ConnectionID)) 
   {
      PreviousEntry   = ConnectionEntry;
      ConnectionEntry = ConnectionEntry->NextConnectionEntryPtr;
   }

   if(ConnectionEntry)
   {
      if(PreviousEntry)
         PreviousEntry->NextConnectionEntryPtr = ConnectionEntry->NextConnectionEntryPtr;
      else
         ConnectionListHead = ConnectionEntry->NextConnectionEntryPtr;

      BTPS_FreeMemory(ConnectionEntry);
   }
}

   /* Cleanup all tracked connections.                                  */
void CleanupConnections(void)
{
   ConnectionEntry_t *ConnectionEntry = ConnectionListHead;
   ConnectionEntry_t *tmpConnectionEntry;

   while(ConnectionEntry)
   {
      tmpConnectionEntry = ConnectionEntry->NextConnectionEntryPtr;

      BTPS_FreeMemory(ConnectionEntry);

      ConnectionEntry = tmpConnectionEntry;
   }

   ConnectionListHead = NULL;
}

   /* The following function is a utility function that is provided to  */
   /* allow a mechanism to query the current directory.                 */
CurrentDirectory_t GetCurrentPhonebookDirectory(unsigned int ConnectionID)
{
   ConnectionEntry_t  *ConnectionEntry;
   CurrentDirectory_t  ret_val;

   if((ConnectionEntry = FindConnection(ConnectionID)) != NULL)
      ret_val = ConnectionEntry->CurrentDirectory;
   else
      ret_val = cdInvalid;

   return(ret_val);
}

   /* The following function is a utility function that is provided to  */
   /* map a given Phonebook Name to a specified Phonebook directory.    */
   /* * NOTE * The phonebook specified is required to be in ABSOLUTE    */
   /*          Path Name form (i.e. "telecom/pb.vcf").                  */
CurrentDirectory_t DeterminePhonebookDirectory(char *PhonebookName)
{
   char               *Ptr;
   char               *Ptr1;
   unsigned int        Index;
   CurrentDirectory_t  ret_val;

   if((PhonebookName) && (BTPS_StringLength(PhonebookName)))
   {
      Ptr  = &(PhonebookName[BTPS_StringLength(PhonebookName) - 1]);
      Ptr1 = NULL;

      while((Ptr != PhonebookName) && (*Ptr != PBAP_PATH_DELIMETER_CHARACTER))
      {
         if((!Ptr1) && (*Ptr == '.'))
            Ptr1 = Ptr;

         Ptr--;
      }

      /* Check to see if we found the delimeter character.              */
      if(*Ptr == PBAP_PATH_DELIMETER_CHARACTER)
      {
         /* *Ptr should point to a delimeter, *Ptr1 should point to the */
         /* period in the phonebook name.                               */
         for(Index=0,ret_val=cdInvalid;Index<sizeof(DirectoryMapping)/sizeof(DirectoryNameMapping_t);Index++)
         {
            if(((unsigned int)(Ptr1 - 1 - Ptr) == BTPS_StringLength(DirectoryMapping[Index].DirectoryName)) && (!BTPS_MemCompareI(Ptr+1, DirectoryMapping[Index].DirectoryName, Ptr1 - 1 - Ptr)))
            {
               /* Match found, go ahead and check to see if a valid     */
               /* phonebook path was specified.                         */
               if((BTPS_StringLength(Ptr1) == BTPS_StringLength(".vcf")) && (!BTPS_MemCompareI(Ptr1, ".vcf", BTPS_StringLength(Ptr1))))
               {
                  /* OK, finally make sure the path is valid.           */
                  if(VerifyPhonebookDirectoryPath(PhonebookName, (Ptr - PhonebookName)))
                     ret_val = DirectoryMapping[Index].Directory;
               }

               break;
            }
         }
      }
      else
         ret_val = cdInvalid;
   }
   else
      ret_val = cdInvalid;

   return(ret_val);
}

   /* The following function is a utility function that is provided to  */
   /* allow a mechanism to query the current directory (in string form).*/
char *GetCurrentPhonebookDirectoryString(unsigned int ConnectionID)
{
   char              *ret_val;
   unsigned int       Index;
   ConnectionEntry_t *ConnectionEntry;

   if((ConnectionEntry = FindConnection(ConnectionID)) != NULL)
   {
      for(Index=0,ret_val=NULL;Index<sizeof(DirectoryMapping)/sizeof(DirectoryNameMapping_t);Index++)
      {
         if(DirectoryMapping[Index].Directory == ConnectionEntry->CurrentDirectory)
         {
            ret_val = DirectoryMapping[Index].DirectoryName;

            break;
         }
      }
   }
   else
      ret_val = NULL;

   return(ret_val);
}

   /* The following function is a utility function that is provided to  */
   /* allow a mechansim to change directories (and thus update the      */
   /* Current Directory).  This function accepts as it's parameters, the*/
   /* path option (up, root, or down), followed by the subdirectory name*/
   /* (only applicable when descending down the directory (i.e.  not    */
   /* applicable when root or up is specified).  This function returns  */
   /* zero if successful, or a negative return value if there was an    */
   /* error.                                                            */
int ChangePhonebookDirectory(unsigned int ConnectionID, PBAP_Set_Path_Option_t Option, char *Name)
{
   int                ret_val;
   unsigned int       Index;
   unsigned int       Index1;
   ConnectionEntry_t *ConnectionEntry;

   if((ConnectionEntry = FindConnection(ConnectionID)) != NULL)
   {
      if(Option == spRoot)
      {
         ConnectionEntry->CurrentDirectory = cdRoot;

         ret_val          = 0;
      }
      else
      {
         if(Option == spUp)
         {
            /* We need to find the parent directory of the current      */
            /* directory.                                               */
            if(ConnectionEntry->CurrentDirectory == cdRoot)
            {
               /* Already at Root, flag success.                        */
               ret_val = 0;
            }
            else
            {
               /* Loop through the directory structure until we find the*/
               /* current directory we are currently at (to find the    */
               /* parent directory).                                    */
               for(Index=0,ret_val=-1;Index<sizeof(DirectoryStructure)/sizeof(DirectoryStructure_t);Index++)
               {
                  if(DirectoryStructure[Index].CurrentDirectory == ConnectionEntry->CurrentDirectory)
                  {
                     /* Current directory found, note the parent        */
                     /* directory, flag success, and exit.              */
                     ConnectionEntry->CurrentDirectory = DirectoryStructure[Index].ParentDirectory;

                     ret_val          = 0;

                     break;
                  }
               }
            }
         }
         else
         {
            /* Caller is requesting a sub-directory, make sure it's     */
            /* valid.                                                   */
            if((Name) && (BTPS_StringLength(Name)))
            {
               /* Try to see if the directory specified matches a       */
               /* directory name we have in our directory structure.    */
               for(Index=0,ret_val=-1;Index<sizeof(DirectoryMapping)/sizeof(DirectoryNameMapping_t);Index++)
               {
                  if((BTPS_StringLength(Name) == BTPS_StringLength(DirectoryMapping[Index].DirectoryName)) && (!BTPS_MemCompareI(Name, DirectoryMapping[Index].DirectoryName, BTPS_StringLength(Name))))
                  {
                     /* OK, we have found a match, now see if this      */
                     /* directory exists at the level we are currently  */
                     /* at.                                             */
                     for(Index1=0;Index1<sizeof(DirectoryStructure)/sizeof(DirectoryStructure_t);Index1++)
                     {
                        if(DirectoryStructure[Index1].CurrentDirectory == DirectoryMapping[Index].Directory)
                        {
                           if(DirectoryStructure[Index1].ParentDirectory == ConnectionEntry->CurrentDirectory)
                           {
                              /* Match found, set the current directory,*/
                              /* flag succes, and exit.                 */
                              ConnectionEntry->CurrentDirectory = DirectoryMapping[Index].Directory;

                              ret_val          = 0;

                              break;
                           }
                        }
                     }
                     break;
                  }
               }
            }
            else
               ret_val = -1;
         }
      }
   }
   else
      ret_val = -1;

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to query  */
   /* the current vCard Information List (based on the directory        */
   /* specified).  The returned value will be a pointer to an array of  */
   /* character pointers, each pointing to a full vCard (NULL           */
   /* terminated).  The number of entries pointed to by the return value*/
   /* will be placed in the NumberListEntries parameter (final          */
   /* parameter).  This function returns a Non-NULL value if successful */
   /* (and fills in the NumberListEntries with the number of items      */
   /* pointed to by the return value), or NULL if there was an error (or*/
   /* no vCard information was found).                                  */
char **QueryvCardList(CurrentDirectory_t Directory, unsigned int *NumberListEntries)
{
   char         **ret_val;
   unsigned int   Index;

   if(NumberListEntries)
   {
      /* Loop through the Directory/vCard Mapping until we find the     */
      /* specified directory.                                           */
      for(Index=0,ret_val=NULL;Index<(sizeof(DirectoryvCardsMapping)/sizeof(DirectoryvCardsMapping_t));Index++)
      {
         if(Directory == DirectoryvCardsMapping[Index].Directory)
         {
            /* Directory found, note the information, and break out of  */
            /* the loop.                                                */
            *NumberListEntries = DirectoryvCardsMapping[Index].NumberCardEntries;
            ret_val            = DirectoryvCardsMapping[Index].vCardList;

            break;
         }
      }
   }
   else
      ret_val = NULL;

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is a utility function that is provided to  */
   /* allow a mechanism to extract the name (in the format of "Last     */
   /* Name;First Name") from the specified vCARD.  This function accepts*/
   /* as input, the vCard Data, followed by a buffer that is to receive */
   /* the name string.  This function returns a BOOLEAN TRUE if         */
   /* successful, or FALSE if there was an error.                       */
Boolean_t ExtractvCardName(char *vCard, unsigned int NameBufferSize, char *NameBuffer)
{
   char         *LinePtr;
   char         *EndNamePtr;
   Boolean_t     Done;
   Boolean_t     ret_val;
   unsigned int  Length;
   unsigned int  SemicolonCount;

   /* First, make sure the input parameters are semi-valid.             */
   if((vCard) && (NameBufferSize) && (NameBuffer))
   {
      Done          = FALSE;
      LinePtr       = vCard;
      NameBuffer[0] = '\0';

      /* Start scanning the vCard data, a line at a time.               */
      while((!Done) && (*LinePtr))
      {
         if(!BTPS_MemCompareI(LinePtr, "N:", BTPS_StringLength("N:")))
         {
            /* Name found, go ahead and determine the first and last    */
            /* names.                                                   */
            LinePtr        += BTPS_StringLength("N:");
            EndNamePtr      = LinePtr;
            SemicolonCount  = 0;

            while((*EndNamePtr) && (*EndNamePtr != '\x0D') && (SemicolonCount != 2))
            {
               if(*EndNamePtr == ';')
                  SemicolonCount++;

               EndNamePtr++;
            }

            Length = EndNamePtr - LinePtr;

            if(Length >= NameBufferSize)
               Length = NameBufferSize - 1;

            if(Length)
            {
               Length--;

               BTPS_MemCopy(NameBuffer, LinePtr, Length);

               NameBuffer[Length] = '\0';

               Done = TRUE;
            }
         }
         else
         {
            /* This line does not specify the name, go ahead and skip to*/
            /* the next line.                                           */
            while((*LinePtr != '\x0A') && (*LinePtr))
               LinePtr++;

            if(*LinePtr)
               LinePtr++;
         }
      }

      /* Flag success to the caller.                                    */
      ret_val       = TRUE;
   }
   else
      ret_val = FALSE;

   return(ret_val);
}
