/*****< msgstore.h >***********************************************************/
/*      Copyright 2009 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  MSGSTORE - Stonestreet One Bluetooth Stack Message Access Profile (MAP)   */
/*             Message Store Implementation for Stonestreet One               */
/*             Bluetooth Protocol Stack sample application.                   */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   07/03/09  D. Lange       Initial creation.                               */
/******************************************************************************/
#ifndef __MSGSTOREH__
#define __MSGSTOREH__

#include "SS1BTMAP.h"      /* Includes/Constants for the MAP.                 */

   /* This module is a sample implementation of the Message Access      */
   /* Profile (MAP) message store.  Specifically, this module provides  */
   /* enough messages to satisfy the following structure:               */
   /*                                                                   */
   /*    - telecom -                                                    */
   /*              - msg -                                              */
   /*                    - inbox -                                      */
   /*                    -       - 2 Messages                           */
   /*                    -                                              */
   /*                    - outbox -                                     */
   /*                    -        - 1 Message                           */
   /*                    -                                              */
   /*                    - sent -                                       */
   /*                    -      - 1 Message                             */
   /*                    -                                              */
   /*                    - deleted -                                    */
   /*                              - No Messages                        */
   /*                                                                   */
   /*                                                                   */
   /* * NOTE * Folder/Message listings need to be built dynamically     */
   /*          utilizing the data contained in this module.             */

   /* The following constants represent the prefix/suffix constants used*/
   /* to construct a folder listing.  A folder listing will             */
   /* have the following format:                                        */
   /*                                                                   */
   /*    TELECOM_FOLDER_LISTING_PREFIX                                  */
   /*       ...                                                         */
   /*    TELECOM_FOLDER_LISTING_SUFFIX                                  */
   /*                                                                   */
   /* where "..." represents zero or more of the following:             */
   /*                                                                   */
   /*    TELECOM_FOLDER_LISTING_ENTRY_PREFIX                            */
   /*       < UTF-8 encoded Folder Name >                               */
   /*    TELECOM_FOLDER_LISTING_ENTRY_MIDDLE                            */
   /*       < OBEX Time/Date Stamp >                                    */
   /*    TELECOM_FOLDER_LISTING_ENTRY_SUFFIX                            */
   /*                                                                   */
   /* where the Folder Name is the name of the folder itself (e.g.      */
   /* "inbox") and the OBEX Time/Date Stamp is the actual time the      */
   /* folder was created.                                               */
#define TELECOM_FOLDER_LISTING_PREFIX                                          \
   "<?xml version=\x22""1.0\x22?>\x0D\x0A"                                     \
   "<!DOCTYPE folder-listing SYSTEM \x22obex-folder-listing.dtd\x22>\x0D\x0A"  \
   "<folder-listing version=\x22""1.0\x22>\x0D\x0A"                            \

#define TELECOM_FOLDER_LISTING_SUFFIX                                          \
   "</folder-listing>\x0D\x0A"

   /* The following constants represent an individual Folder Listing    */
   /* entry prefix, middle, and suffix strings.  These strings are used */
   /* to build an indvidual Folder listing entry.  See the description  */
   /* of the Folder Listing prefix/suffixes above for more information. */
#define TELECOM_FOLDER_LISTING_ENTRY_PREFIX                                   \
   "   <folder name=\x22"                                                     \

#define TELECOM_FOLDER_LISTING_ENTRY_MIDDLE                                   \
   "\x22 created=\x22"

#define TELECOM_FOLDER_LISTING_ENTRY_SUFFIX                                   \
   "\x22/>\x0D\x0A"

   /* The following constants represent the prefix/suffix constants used*/
   /* to construct a message listing.  A message listing will           */
   /* have the following format:                                        */
   /*                                                                   */
   /*    TELECOM_MESSAGE_LISTING_PREFIX                                 */
   /*       ...                                                         */
   /*    TELECOM_MESSAGE_LISTING_SUFFIX                                 */
   /*                                                                   */
   /* where "..." represents zero or more of the following:             */
   /*                                                                   */
   /*    TELECOM_MESSAGE_LISTING_ENTRY_PREFIX                           */
   /*       < MAP Message Handle >                                      */
   /*    TELECOM_MESSAGE_LISTING_ENTRY_MIDDLE_1                         */
   /*       < UTF-8 Encoded Message Subject >                           */
   /*    TELECOM_MESSAGE_LISTING_ENTRY_MIDDLE_2                         */
   /*       < Message Time/Date (OBEX Time/Date Stamp) >                */
   /*    TELECOM_MESSAGE_LISTING_ENTRY_MIDDLE_3                         */
   /*       < UTF-8 Encoded Sender >                                    */
   /*    TELECOM_MESSAGE_LISTING_ENTRY_MIDDLE_4                         */
   /*       < UTF-8 Encoded Sender Name >                               */
   /*    TELECOM_MESSAGE_LISTING_ENTRY_MIDDLE_5                         */
   /*       < UTF-8 Encoded Sender Addressing >                         */
   /*    TELECOM_MESSAGE_LISTING_ENTRY_MIDDLE_6                         */
   /*       < UTF-8 Encoded Reply To Addressing >                       */
   /*    TELECOM_MESSAGE_LISTING_ENTRY_MIDDLE_7                         */
   /*       < UTF-8 Encoded Recipient Name >                            */
   /*    TELECOM_MESSAGE_LISTING_ENTRY_MIDDLE_8                         */
   /*       < UTF-8 Encoded Recipient Addressing >                      */
   /*    TELECOM_MESSAGE_LISTING_ENTRY_MIDDLE_9                         */
   /*       < Message Type ("EMAIL"/"SMS_GSM"/"SMS_CDMA"/"MMS") >       */
   /*    TELECOM_MESSAGE_LISTING_ENTRY_MIDDLE_10                        */
   /*       < Message Size >                                            */
   /*    TELECOM_MESSAGE_LISTING_ENTRY_MIDDLE_11                        */
   /*       < Text ("yes"/"no") >                                       */
   /*    TELECOM_MESSAGE_LISTING_ENTRY_MIDDLE_12                        */
   /*       < Reception Status ("complete"/"fractioned"/"notification")>*/
   /*    TELECOM_MESSAGE_LISTING_ENTRY_MIDDLE_13                        */
   /*       < Attachment Size >                                         */
   /*    TELECOM_MESSAGE_LISTING_ENTRY_MIDDLE_14                        */
   /*       < Priority ("yes"/"no") >                                   */
   /*    TELECOM_MESSAGE_LISTING_ENTRY_MIDDLE_15                        */
   /*       < Read ("yes"/"no") >                                       */
   /*    TELECOM_MESSAGE_LISTING_ENTRY_MIDDLE_16                        */
   /*       < Sent ("yes"/"no") >                                       */
   /*    TELECOM_MESSAGE_LISTING_ENTRY_MIDDLE_17                        */
   /*       < Protected ("yes"/"no") >                                  */
   /*    TELECOM_MESSAGE_LISTING_ENTRY_SUFFIX                           */
   /*                                                                   */
#define TELECOM_MESSAGE_LISTING_PREFIX                                          \
   "<MAP-msg-listing version=\x22""1.0\x22>\x0D\x0A"                            \

#define TELECOM_MESSAGE_LISTING_SUFFIX                                          \
   "</MAP-msg-listing>"

   /* The following constants represent an individual Message Listing   */
   /* entry prefix, middle, and suffix strings.  These strings are used */
   /* to build an indvidual Message listing entry.  See the description */
   /* of the Message Listing prefix/suffixes above for more information.*/
#define TELECOM_MESSAGE_LISTING_ENTRY_PREFIX                                    \
   "   <msg handle=\x22"

#define TELECOM_MESSAGE_LISTING_ENTRY_MIDDLE_1                                  \
   "\x22 subject=\x22"

#define TELECOM_MESSAGE_LISTING_ENTRY_MIDDLE_2                                  \
   "\x22 datetime=\x22"

#define TELECOM_MESSAGE_LISTING_ENTRY_MIDDLE_3                                  \
   "\x22 sender=\x22"

#define TELECOM_MESSAGE_LISTING_ENTRY_MIDDLE_4                                  \
   "\x22 sender_name=\x22"

#define TELECOM_MESSAGE_LISTING_ENTRY_MIDDLE_5                                  \
   "\x22 sender_addressing=\x22"

#define TELECOM_MESSAGE_LISTING_ENTRY_MIDDLE_6                                  \
   "\x22 replyto_addressing=\x22"

#define TELECOM_MESSAGE_LISTING_ENTRY_MIDDLE_7                                  \
   "\x22 recipient_name=\x22"

#define TELECOM_MESSAGE_LISTING_ENTRY_MIDDLE_8                                  \
   "\x22 recipient_addressing=\x22"

#define TELECOM_MESSAGE_LISTING_ENTRY_MIDDLE_9                                  \
   "\x22 type=\x22"

#define TELECOM_MESSAGE_LISTING_ENTRY_MIDDLE_10                                 \
   "\x22 size=\x22"

#define TELECOM_MESSAGE_LISTING_ENTRY_MIDDLE_11                                 \
   "\x22 text=\x22"

#define TELECOM_MESSAGE_LISTING_ENTRY_MIDDLE_12                                 \
   "\x22 reception_status=\x22"

#define TELECOM_MESSAGE_LISTING_ENTRY_MIDDLE_13                                 \
   "\x22 attachment_size=\x22"

#define TELECOM_MESSAGE_LISTING_ENTRY_MIDDLE_14                                 \
   "\x22 priority=\x22"

#define TELECOM_MESSAGE_LISTING_ENTRY_MIDDLE_15                                 \
   "\x22 read=\x22"

#define TELECOM_MESSAGE_LISTING_ENTRY_MIDDLE_16                                 \
   "\x22 sent=\x22"

#define TELECOM_MESSAGE_LISTING_ENTRY_MIDDLE_17                                 \
   "\x22 protected=\x22"

#define TELECOM_MESSAGE_LISTING_ENTRY_SUFFIX                                    \
   "\x22/>\x0D\x0A"

   /* The following constants represent the prefix/suffix constants used*/
   /* to construct an event report.  An event report have the following */
   /* format:                                                           */
   /*                                                                   */
   /*    TELECOM_MESSAGE_EVENT_REPORT_PREFIX                            */
   /*       ...                                                         */
   /*    TELECOM_MESSAGE_EVENT_REPORT_SUFFIX                            */
   /*                                                                   */
   /* where "..." represents one or more of the following:              */
   /*                                                                   */
   /*    TELECOM_MESSAGE_EVENT_REPORT_ENTRY_PREFIX                      */
   /*   < Event Type ("NewMessage"/"DeliverySuccess"/"SendingSuccess"/  */
   /*                 "DeliveryFailure"/"SendingFailure"/"MemoryFull"/  */
   /*                 "MemoryAvailable"/"MessageDeleted"/"MessageShift")*/
   /*    TELECOM_MESSAGE_EVENT_REPORT_ENTRY_MIDDLE_1                    */
   /*       < MAP Message Handle >                                      */
   /*    TELECOM_MESSAGE_EVENT_REPORT_ENTRY_MIDDLE_2                    */
   /*       < UTF-8 encoded Folder Name >                               */
   /*    TELECOM_MESSAGE_EVENT_REPORT_ENTRY_MIDDLE_3                    */
   /*       < UTF-8 encoded Old Folder Name>                            */
   /*    TELECOM_MESSAGE_EVENT_REPORT_ENTRY_MIDDLE_4                    */
   /*       < Message Type ("EMAIL"/"SMS_GSM"/"SMS_CDMA"/"MMS") >       */
   /*                                                                   */
#define TELECOM_MESSAGE_EVENT_REPORT_PREFIX                                     \
   "<MAP-event-report version=\x22""1.0\x22>\x0D\x0A"                           \

#define TELECOM_MESSAGE_EVENT_REPORT_SUFFIX                                     \
   "</MAP-event-report>"

#define TELECOM_MESSAGE_EVENT_REPORT_ENTRY_PREFIX                               \
   "   <event type=\x22"

#define TELECOM_MESSAGE_EVENT_REPORT_ENTRY_MIDDLE_1                             \
   "\x22 handle=\x22"

#define TELECOM_MESSAGE_EVENT_REPORT_ENTRY_MIDDLE_2                             \
   "\x22 folder=\x22"

#define TELECOM_MESSAGE_EVENT_REPORT_ENTRY_MIDDLE_3                             \
   "\x22 msg_type=\x22"

#define TELECOM_MESSAGE_EVENT_REPORT_ENTRY_SUFFIX                               \
   "\x22/>\x0D\x0A"

   /* The following enumerated type represents the supported Event Types*/
   /* used by this module.                                              */
typedef enum
{
  etNewMessage,
  etDeliverySuccess,
  etSendingSuccess,
  etDeliveryFailure,
  etSendingFailure,
  etMemoryFull,
  etMemoryAvailable,
  etMessageDeleted,
} EventType_t;

   /* The following enumerated type represents the supported Message    */
   /* Types used by this module.                                        */
typedef enum
{
  mtEMAIL,
  mtSMS_GSM,
  mtSMS_CDMA,
  mtMMS
} MessageType_t;

   /* The following enumerated type represents the supported Folders by */
   /* this module.                                                      */
typedef enum
{
   cfInvalid,
   cfRoot,
   cfTelecom,
   cfMessage,
   cfInbox,
   cfOutbox,
   cfSent,
   cfDeleted
} CurrentFolder_t;

  /* The following structure defines the container structure that is    */
  /* used with the QueryFolderEntry() to query information about an     */
  /* individual Folder Entry.                                           */
typedef struct _tagFolderEntry_t
{
   char *FolderName;
   char *CreateDateTime;
} FolderEntry_t;

   /* The following defines the status values that can be assigned      */
   /* fields in the message.                                            */
typedef enum {svNo, svYes, svNotDefined} StatusValue_t;

  /* The following structure defines the container structure that is    */
  /* used with the QueryMessageEntry() to query information about an    */
  /* individual Message Entry.                                          */
typedef struct _tagMessageEntry_t
{
   char          *MessageHandle;
   char          *Subject;
   char          *DateTime;
   char          *Sender;
   char          *SenderName;
   char          *SenderAddressing;
   char          *ReplyToAddressing;
   char          *RecipientName;
   char          *RecipientAddressing;
   char          *Type;
   DWord_t        MessageSize;
   char          *Text;
   char          *ReceptionStatus;
   DWord_t        AttachmentSize;
   StatusValue_t  Priority;
   StatusValue_t  Read;
   StatusValue_t  Sent;
   StatusValue_t  Protected;
   char          *MessageData;
} MessageEntry_t;

typedef struct _tagNewMessage_t
{
   char            MessageHandle[MAP_MESSAGE_HANDLE_LENGTH+1];
   CurrentFolder_t Folder;
   char            InfoBuffer[512];
   unsigned int    DataLength;
   char            DataBuffer[1];
} NewMessage_t;

#define NEW_MESSAGE_DATA_SIZE(_x)         (sizeof(NewMessage_t) - sizeof(Byte_t) + (_x))

   /* The following function is a utility function that is provided to  */
   /* allow a mechanism to query the current Folder.                    */
CurrentFolder_t GetCurrentMessageFolder(void);

   /* The following function is a utility function that is provided to  */
   /* allow a mechanism to query the current folder (in string form).   */
char *GetCurrentMessageFolderString(void);

   /* The following function is a utility function that is provided to  */
   /* allow a mechansim to change folders (and thus update the Current  */
   /* Folder).  This function accepts as it's parameters, the path      */
   /* option (up, root, or down), followed by the sub-folder name (only */
   /* applicable when descending down the folder (i.e. not applicable   */
   /* when root or up is specified).  This function returns zero if     */
   /* successful, or a negative return value if there was an error.     */
int ChangeMessageFolder(MAP_Set_Folder_Option_t Option, char *Name);

   /* The following function is a utility function that is provided to  */
   /* allow a mechanism to query the total number of Folder Entries that*/
   /* exist at the specified Folder.  This value can be used as an upper*/
   /* bound to iterate the Folder Information in the specified folder   */
   /* (using the QueryFolderEntry() function).  This function returns   */
   /* zero if there are no folders present in the specified, directory, */
   /* a positive non-zero number that represents the number of actual   */
   /* folders located at the specified folder, or a negative value if   */
   /* there was an error.                                               */
int QueryNumberFolderEntries(CurrentFolder_t Folder);

   /* The following function is a utility function that is provided to  */
   /* allow a mechansim to query information about a specific folder    */
   /* entry in the specified directory.  The specific folder is         */
   /* specified by specifying the index of the folder that the caller   */
   /* is interested in.  Valid index values are zero through one less   */
   /* than the return value returned from QueryNumberFolderEntries().   */
   /* This function returns a BOOLEAN TRUE if the folder information was*/
   /* retrieved, or FALSE if there was an error.                        */
Boolean_t QueryFolderEntry(CurrentFolder_t Folder, unsigned int Index, FolderEntry_t *FolderEntry);

   /* The following function is a utility function that is provided to  */
   /* allow a mechanism to query the total number of Message Entries    */
   /* that exist at the specified Folder.  This value can be used as an */
   /* upper bound to iterate the Message Information in the specified   */
   /* folder (using the QueryMessageEntry() function).  This function   */
   /* returns zero if there are no messages present in the specified,   */
   /* directory, a positive non-zero number that represents the number  */
   /* of actual messages located at the specified folder, or a negative */
   /* value if there was an error.                                      */
int QueryNumberMessageEntries(CurrentFolder_t Folder);

   /* The following function is a utility function that is provided to  */
   /* allow a mechansim to query information about a specific message   */
   /* entry in the specified directory.  The specific message is        */
   /* specified by specifying the index of the message that the caller  */
   /* is interested in.  Valid index values are zero through one less   */
   /* than the return value returned from QueryNumberMessageEntries().  */
   /* This function returns a BOOLEAN TRUE if the message information   */
   /* was retrieved, or FALSE if there was an error.                    */
Boolean_t QueryMessageEntryByIndex(CurrentFolder_t Folder, unsigned int Index, MessageEntry_t **MessageEntry);

   /* The following function is a utility function that is provided to  */
   /* allow a mechansim to query information about a specific message   */
   /* entry in the specified directory.  The specific message is        */
   /* specified by specifying the actual Message Handle of the message  */
   /* that the caller is interested in (cannot be NULL).  This function */
   /* returns a BOOLEAN TRUE if the message information was retrieved,  */
   /* or FALSE if there was an error.                                   */
   /* * NOTE * If the caller simply wants to pull the Message from      */
   /*          anywhere (all Message Handles are unique in the system)  */
   /*          then cfInvalid can be passed as the Folder parameter.    */
Boolean_t QueryMessageEntryByHandle(CurrentFolder_t Folder, char *MessageHandle, MessageEntry_t **MessageEntry);

   /* The following function is a utility function that is provided to  */
   /* allow a mechansim to query information about a specific message   */
   /* entry in the specified directory.  The specific message is        */
   /* specified by specifying the actual Message Handle of the message  */
   /* that the caller is interested in (cannot be NULL).  This function */
   /* returns a BOOLEAN TRUE if the message information was retrieved,  */
   /* or FALSE if there was an error.                                   */
   /* * NOTE * If the caller simply wants to pull the Message from      */
   /*          anywhere (all Message Handles are unique in the system)  */
   /*          then cfInvalid can be passed as the Folder parameter.    */
Boolean_t MoveMessageEntryToFolderByHandle(CurrentFolder_t Folder, char *MessageHandle);

   /* The following function is a utility function to insert a message  */
   /* into the message listings array.  The function receives the Folder*/
   /* where the message is to be inserted, the message handle that has  */
   /* been assigned, a pointer to the data and the length of the data.  */
   /* The function will return a non-zero value if the function fails to*/
   /* insert the message into the list.                                 */
void InsertMessageEntryIntoFolder(CurrentFolder_t Folder, char *MessageHandle);

   /* The following function is a utility function that is provided to  */
   /* allow a mechanism to retreive the Event Type (in string form).    */
char *GetEventTypeString(EventType_t EventType);

   /* The following function is a utility function that is provided to  */
   /* allow a mechanism to retreive the Message Type (in string form).  */
char *GetMessageTypeString(MessageType_t MessageType);

   /* The following function is a utility function that is provided to  */
   /* allow a mechansim to generate a unique Message Handle that can be */
   /* used to uniquely identify a received message.  This function      */
   /* returns a BOOLEAN TRUE if a unique Message Handle was able to be  */
   /* generated and placed in the input buffer, or FALSE if there was an*/
   /* error storing the unique handle in the input buffer (i.e. the     */
   /* buffer passed in was invalid).                                    */
Boolean_t GenerateUniqueMessageHandle(unsigned int BufferLength, char *MessageHandleBuffer);

   /* The following function is a utility function that is used to      */
   /* increment the Unique File Handle that is used for pushed messages.*/
   /* * NOTE * This should only be called after the final message of a  */
   /*          Push Indication.                                         */
void IncrementMessageHandle(void);

#endif
