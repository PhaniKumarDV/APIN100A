/*****< phonebook.h >**********************************************************/
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
#ifndef __PHONEBOOKH__
#define __PHONEBOOKH__

#include "SS1BTPBA.h"      /* Includes/Constants for the PBAP.                */

   /* This module is a sample implementation of the Phonebook Access    */
   /* Profile (PBAP) object store.  Specifically, this module provides  */
   /* enough vCards to satisfy the following structure:                 */
   /*                                                                   */
   /*    - telecom -                                                    */
   /*              - pb -                                               */
   /*              -    - 0.vcf                                         */
   /*              -    - 1.vcf                                         */
   /*              -    - 2.vcf                                         */
   /*              -    - 4.vcf                                         */
   /*              -                                                    */
   /*              - ich -                                              */
   /*              -     - 1.vcf                                        */
   /*              -     - 2.vcf                                        */
   /*              -                                                    */
   /*              - och -                                              */
   /*              -     - 1.vcf                                        */
   /*              -                                                    */
   /*              - mch -                                              */
   /*                    - 1.vcf                                        */
   /*                                                                   */
   /* * NOTE * Folder listings need to be built dynamically utilizing   */
   /*          the data contained in this module.  Also, the entire     */
   /*          phonebooks (e.g. "pb.vcf" which are the contents of the  */
   /*          "pb" path) need to be dynamically constructed as well.   */

   /* The following constants represent the prefix/suffix constants used*/
   /* to construct a vCard folder listing.  A vCard folder listing will */
   /* have the following format:                                        */
   /*                                                                   */
   /*    TELECOM_PHONEBOOK_LISTING_PREFIX                               */
   /*       ...                                                         */
   /*    TELECOM_PHONEBOOK_LISTING_SUFFIX                               */
   /*                                                                   */
   /* where "..." represents zero or more of the following:             */
   /*                                                                   */
   /*    TELECOM_PHONEBOOK_LISTING_ENTRY_PREFIX                         */
   /*       < UTF-8 encoded Handle Name >                               */
   /*    TELECOM_PHONEBOOK_LISTING_ENTRY_MIDDLE                         */
   /*       < UTF-8 encoded vCard Name >                                */
   /*    TELECOM_PHONEBOOK_LISTING_ENTRY_SUFFIX                         */
   /*                                                                   */
   /* where the Handle Name is the name of the vCard itself (e.g.       */
   /* "0.vcf") and the vCard Name is the value taken from the vCard     */
   /* itself.                                                           */
#define TELECOM_PHONEBOOK_LISTING_PREFIX                                \
   "<?xml version=\x22""1.0\x22?>\x0D\x0A"                              \
   "<!DOCTYPE vcard-listing SYSTEM \x22vcard-listing.dtd\x22>\x0D\x0A"  \
   "<vCard-listing version=\x22""1.0\x22>\x0D\x0A"

#define TELECOM_PHONEBOOK_LISTING_SUFFIX                                \
   "</vCard-listing>\x0D\x0A"

   /* The following constants represent an individual Phonebook Listing */
   /* entry prefix, middle, and suffix strings.  These strings are used */
   /* to build an indvidual Phonebook listing entry.  See the           */
   /* description of the Phonebook Listing prefix/suffixes above for    */
   /* more information.                                                 */
#define TELECOM_PHONEBOOK_LISTING_ENTRY_PREFIX                          \
   "   <card handle = \x22"

#define TELECOM_PHONEBOOK_LISTING_ENTRY_MIDDLE                          \
   "\x22 name = \x22"

#define TELECOM_PHONEBOOK_LISTING_ENTRY_SUFFIX                          \
   "\x22/>\x0D\x0A"

   /* The following enumerated type represents the supported paths by   */
   /* this module.                                                      */
typedef enum
{
   cdInvalid,
   cdRoot,
   cdTelecom,
   cdPhonebook,
   cdIncomingCallHistory,
   cdOutgoingCallHistory,
   cdMissedCallHistory,
   cdCombinedCallHistory
} CurrentDirectory_t;

   /* Starts tracking the new connection in the phonebook.              */
Boolean_t AddConnection(unsigned int ConnectionID);

   /* Ends tracking a connection in the phonebook.                      */
void RemoveConnection(unsigned int ConnectionID);

   /* Cleanup all tracked connections.                                  */
void CleanupConnections(void);

   /* The following function is a utility function that is provided to  */
   /* allow a mechanism to query the current directory.                 */
CurrentDirectory_t GetCurrentPhonebookDirectory(unsigned int ConnectionID);

   /* The following function is a utility function that is provided to  */
   /* allow a mechanism to query the current directory (in string form).*/
char *GetCurrentPhonebookDirectoryString(unsigned int ConnectionID);

   /* The following function is a utility function that is provided to  */
   /* map a given Phonebook Name to a specified Phonebook directory.    */
   /* * NOTE * The phonebook specified is required to be in ABSOLUTE    */
   /*          Path Name form (i.e. "telecom/pb.vcf").                  */
CurrentDirectory_t DeterminePhonebookDirectory(char *PhonebookName);

   /* The following function is a utility function that is provided to  */
   /* allow a mechanism to change directories (and thus update the      */
   /* Current Directory).  This function accepts as it's parameters, the*/
   /* path option (up, root, or down), followed by the subdirectory name*/
   /* (only applicable when descending down the directory (i.e.  not    */
   /* applicable when root or up is specified).  This function returns  */
   /* zero if successful, or a negative return value if there was an    */
   /* error.                                                            */
int ChangePhonebookDirectory(unsigned int ConnectionID, PBAP_Set_Path_Option_t Option, char *Name);

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
char **QueryvCardList(CurrentDirectory_t Directory, unsigned int *NumberListEntries);

   /* The following function is a utility function that is provided to  */
   /* allow a mechanism to extract the name (in the format of "Last     */
   /* Name;First Name") from the specified vCARD.  This function accepts*/
   /* as input, the vCard Data, followed by a buffer that is to receive */
   /* the name string.  This function returns a BOOLEAN TRUE if         */
   /* successful, or FALSE if there was an error.                       */
Boolean_t ExtractvCardName(char *vCard, unsigned int NameBufferSize, char *NameBuffer);

#endif
