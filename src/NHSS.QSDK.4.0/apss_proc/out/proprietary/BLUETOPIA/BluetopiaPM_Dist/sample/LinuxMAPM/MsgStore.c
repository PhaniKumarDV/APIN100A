/*****< msgstore.c >***********************************************************/
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
#include "MsgStore.h"      /* Msg Store Implementation Protoypes/Constants.   */

#include "BTPSKRNL.h"      /* Bluetooth O/S Abstration Prototypes/Constants.  */

   /* The following constant represents the 'common' portion of a       */
   /* Message Handle that is supported by this module.  This is the     */
   /* beginning of every Message Handle that is supported by this       */
   /* module.                                                           */
   /* * NOTE * A Handle consists of the Prefix string followed by a     */
   /*          8 digit Hexadecimal ASCII Number.  This means that a     */
   /*          the ASCII Handle is 8 ASCII digits following this prefix.*/
#define MESSAGE_HANDLE_PREFIX_STRING               "200"

   /* /root/telecom/msg/inbox - Message 0 Data                          */
   /* * NOTE * This is simply preformatted BMSG Data, this              */
   /*          implementation does not build the BMSG programmatically. */
#define TELECOM_MSG_INBOX_CONTENTS_MESSAGE_0_DATA_BMESSAGE_DATA                   \
   "BEGIN:BMSG\x0D\x0A"                                                           \
   "VERSION:1.0\x0D\x0A"                                                          \
   "STATUS:READ  \x0D\x0A"                                                        \
   "TYPE:SMS_GSM\x0D\x0A"                                                         \
   "FOLDER:TELECOM/MSG/INBOX\x0D\x0A"                                             \
   "BEGIN:VCARD\x0D\x0A"                                                          \
   "VERSION:2.1\x0D\x0A"                                                          \
   "N:Jamie\x0D\x0A"                                                              \
   "TEL;CELL;VOICE:+1-987-654-3210\x0D\x0A"                                       \
   "END:VCARD\x0D\x0A"                                                            \
   "BEGIN:BENV\x0D\x0A"                                                           \
   "BEGIN:VCARD\x0D\x0A"                                                          \
   "VERSION:2.1\x0D\x0A"                                                          \
   "N:\x0D\x0A"                                                                   \
   "TEL;WORK;VOICE:22+1-0123-456789\x0D\x0A"                                      \
   "END:VCARD\x0D\x0A"                                                            \
   "BEGIN:BBODY\x0D\x0A"                                                          \
   "ENCODING:8BIT\x0D\x0A"                                                        \
   "CHARSET:UTF-8\x0D\x0A"                                                        \
   "LENGTH:71\x0D\x0A"                                                            \
   "BEGIN:MSG\x0D\x0A"                                                            \
   "Date: 13 May 09\x0D\x0A"                                                      \
   "Subject: Hello\x0D\x0A"                                                       \
   "\x0D\x0A"                                                                     \
   "Nothing Really\x0D\x0A"                                                       \
   "END:MSG\x0D\x0A"                                                              \
   "END:BBODY\x0D\x0A"                                                            \
   "END:BENV\x0D\x0A"                                                             \
   "END:BMSG\x0D\x0A"

   /* Create a variable to hold the Message Data.  This will prevent    */
   /* this from being placed in Read Only space if a String Literal is  */
   /* used as with the other fields.                                    */
static char DynamicTelecomMsgInboxContentsMessage0DataBMessageData[] = { TELECOM_MSG_INBOX_CONTENTS_MESSAGE_0_DATA_BMESSAGE_DATA } ;

#define TELECOM_MSG_INBOX_CONTENTS_MESSAGE_0_DATA                                 \
{                                                                                 \
   "20000100001",                                                                 \
   "Hello",                                                                       \
   "20100513T130510Z",                                                            \
   NULL,                /* Sender isn't required                        */        \
   "Jamie",                                                                       \
   "+1-987-6543210",                                                              \
   NULL,                /* Reply-To Addressing isn't required           */        \
   "22+1-0123-456789",                                                            \
   NULL,                /* Recipient Name isn't required                */        \
   "SMS_GSM",                                                                     \
   sizeof(TELECOM_MSG_INBOX_CONTENTS_MESSAGE_0_DATA_BMESSAGE_DATA),               \
   NULL,                /* Text isn't required                          */        \
   "complete",                                                                    \
   0,                                                                             \
   svNo,                                                                          \
   svYes,                                                                         \
   svNo,                                                                          \
   svNo,                                                                          \
   DynamicTelecomMsgInboxContentsMessage0DataBMessageData                         \
}

   /* /root/telecom/msg/inbox - Message 1 Data                          */
   /* * NOTE * This is simply preformatted BMSG Data, this              */
   /*          implementation does not build the BMSG programmatically. */
#define TELECOM_MSG_INBOX_CONTENTS_MESSAGE_1_DATA_BMESSAGE_DATA                   \
   "BEGIN:BMSG\x0D\x0A"                                                           \
   "VERSION:1.0\x0D\x0A"                                                          \
   "STATUS:READ  \x0D\x0A"                                                        \
   "TYPE:EMAIL\x0D\x0A"                                                           \
   "FOLDER:TELECOM/MSG/INBOX\x0D\x0A"                                             \
   "BEGIN:VCARD\x0D\x0A"                                                          \
   "VERSION:2.1\x0D\x0A"                                                          \
   "N:\x0D\x0A"                                                                   \
   "TEL;CELL;VOICE::876-543-2109\x0D\x0A"                                         \
   "EMAIL:Demitry@home.com"                                                       \
   "END:VCARD\x0D\x0A"                                                            \
   "BEGIN:BENV\x0D\x0A"                                                           \
   "BEGIN:VCARD\x0D\x0A"                                                          \
   "VERSION:2.1\x0D\x0A"                                                          \
   "N:\x0D\x0A"                                                                   \
   "TEL;WORK;VOICE::22+49-9012-34567\x0D\x0A"                                     \
   "EMAIL:Mary@home.com"                                                          \
   "END:VCARD\x0D\x0A"                                                            \
   "BEGIN:BBODY\x0D\x0A"                                                          \
   "ENCODING:8BIT\x0D\x0A"                                                        \
   "CHARSET:UTF-8\x0D\x0A"                                                        \
   "LENGTH:78\x0D\x0A"                                                            \
   "BEGIN:MSG\x0D\x0A"                                                            \
   "Date: 15 May 09\x0D\x0A"                                                      \
   "Subject: Guten Tag\x0D\x0A"                                                   \
   "\x0D\x0A"                                                                     \
   "Missed your call!\x0D\x0A"                                                    \
   "END:MSG\x0D\x0A"                                                              \
   "END:BBODY\x0D\x0A"                                                            \
   "END:BENV\x0D\x0A"                                                             \
   "END:BMSG\x0D\x0A"

   /* Create a variable to hold the Message Data.  This will prevent    */
   /* this from being placed in Read Only space if a String Literal is  */
   /* used as with the other fields.                                    */
static char DynamicTelecomMsgInboxContentsMessage1DataBMessageData[] = { TELECOM_MSG_INBOX_CONTENTS_MESSAGE_1_DATA_BMESSAGE_DATA } ;

#define TELECOM_MSG_INBOX_CONTENTS_MESSAGE_1_DATA                                 \
{                                                                                 \
   "20000100002",                                                                 \
   "Guten Tag",                                                                   \
   "20100512T092200Z",                                                            \
   NULL,                /* Sender isn't required                        */        \
   "Dmitri",                                                                      \
   "dmitry@home.com",                                                             \
   NULL,                /* Reply-To Addressing isn't required           */        \
   "22+49-9012-34567",                                                            \
   NULL,                /* Recipient Name isn't required                */        \
   "EMAIL",                                                                       \
   sizeof(TELECOM_MSG_INBOX_CONTENTS_MESSAGE_1_DATA_BMESSAGE_DATA),               \
   NULL,                /* Text isn't required                          */        \
   "complete",                                                                    \
   3000,                                                                          \
   svNo,                                                                          \
   svNo,                                                                          \
   svNo,                                                                          \
   svNo,                                                                          \
   DynamicTelecomMsgInboxContentsMessage1DataBMessageData                         \
}

   /* /root/telecom/msg/inbox - Message 2 Data                          */
   /* * NOTE * This is simply preformatted BMSG Data, this              */
   /*          implementation does not build the BMSG programmatically. */
#define TELECOM_MSG_INBOX_CONTENTS_MESSAGE_2_DATA_BMESSAGE_DATA                   \
   "BEGIN:BMSG\x0D\x0A"                                                           \
   "VERSION:1.0\x0D\x0A"                                                          \
   "STATUS:UNREAD\x0D\x0A"                                                        \
   "TYPE:MMS\x0D\x0A"                                                             \
   "FOLDER:TELECOM/MSG/INBOX\x0D\x0A"                                             \
   "BEGIN:VCARD\x0D\x0A"                                                          \
   "VERSION:2.1\x0D\x0A"                                                          \
   "N:\x0D\x0A"                                                                   \
   "TEL;CELL;VOICE::205-876-9032\x0D\x0A"                                         \
   "END:VCARD\x0D\x0A"                                                            \
   "BEGIN:BENV\x0D\x0A"                                                           \
   "BEGIN:VCARD\x0D\x0A"                                                          \
   "VERSION:2.1\x0D\x0A"                                                          \
   "N:\x0D\x0A"                                                                   \
   "TEL;WORK;VOICE::555-555-5555\x0D\x0A"                                         \
   "END:VCARD\x0D\x0A"                                                            \
   "BEGIN:BBODY\x0D\x0A"                                                          \
   "ENCODING:8BIT\x0D\x0A"                                                        \
   "CHARSET:UTF-8\x0D\x0A"                                                        \
   "LENGTH:64\x0D\x0A"                                                            \
   "BEGIN:MSG\x0D\x0A"                                                            \
   "Date: 15 May 10\x0D\x0A"                                                      \
   "Subject: Help\x0D\x0A"                                                        \
   "\x0D\x0A"                                                                     \
   "Call Me!\x0D\x0A"                                                             \
   "END:MSG\x0D\x0A"                                                              \
   "END:BBODY\x0D\x0A"                                                            \
   "END:BENV\x0D\x0A"                                                             \
   "END:BMSG\x0D\x0A"

   /* Create a variable to hold the Message Data.  This will prevent    */
   /* this from being placed in Read Only space if a String Literal is  */
   /* used as with the other fields.                                    */
static char DynamicTelecomMsgInboxContentsMessage2DataBMessageData[] = { TELECOM_MSG_INBOX_CONTENTS_MESSAGE_2_DATA_BMESSAGE_DATA } ;

#define TELECOM_MSG_INBOX_CONTENTS_MESSAGE_2_DATA                                 \
{                                                                                 \
   "20000100003",                                                                 \
   "Help",                                                                        \
   "20100511T032200Z",                                                            \
   NULL,                /* Sender isn't required                        */        \
   "Catherine",                                                                   \
   "2058769032",                                                                  \
   NULL,                /* Reply-To Addressing isn't required           */        \
   "5555555555",                                                                  \
   NULL,                /* Recipient Name isn't required                */        \
   "MMS",                                                                         \
   sizeof(TELECOM_MSG_INBOX_CONTENTS_MESSAGE_2_DATA_BMESSAGE_DATA),               \
   NULL,                /* Text isn't required                          */        \
   "complete",                                                                    \
   3000,                                                                          \
   svNo,                                                                          \
   svYes,                                                                         \
   svNo,                                                                          \
   svNo,                                                                          \
   DynamicTelecomMsgInboxContentsMessage2DataBMessageData                         \
}

   /* /root/telecom/msg/outbox - Message 0 Data                         */
   /* * NOTE * This is simply preformatted BMSG Data, this              */
   /*          implementation does not build the BMSG programmatically. */
#define TELECOM_MSG_OUTBOX_CONTENTS_MESSAGE_0_DATA_BMESSAGE_DATA                  \
   "BEGIN:BMSG\x0D\x0A"                                                           \
   "VERSION:1.0\x0D\x0A"                                                          \
   "STATUS:READ  \x0D\x0A"                                                        \
   "TYPE:SMS_CDMA\x0D\x0A"                                                        \
   "FOLDER:OUTBOX\x0D\x0A"                                                        \
   "BEGIN:VCARD\x0D\x0A"                                                          \
   "VERSION:2.1\x0D\x0A"                                                          \
   "N:Andy\x0D\x0A"                                                               \
   "TEL;CELL;VOICE::+49-7654-321098\x0D\x0A"                                      \
   "END:VCARD\x0D\x0A"                                                            \
   "BEGIN:BENV\x0D\x0A"                                                           \
   "BEGIN:VCARD\x0D\x0A"                                                          \
   "VERSION:2.1\x0D\x0A"                                                          \
   "N:Barney\x0D\x0A"                                                             \
   "TELETEL;WORK;VOICE::+49-89-01234567\x0D\x0A"                                  \
   "END:VCARD\x0D\x0A"                                                            \
   "BEGIN:BBODY\x0D\x0A"                                                          \
   "ENCODING:8BIT\x0D\x0A"                                                        \
   "CHARSET:UTF-8\x0D\x0A"                                                        \
   "LENGTH:85\x0D\x0A"                                                            \
   "BEGIN:MSG\x0D\x0A"                                                            \
   "Date: 15 May 09\x0D\x0A"                                                      \
   "Subject: Nogo on the Fish\x0D\x0A"                                            \
   "\x0D\x0A"                                                                     \
   "Not gonna happen!\x0D\x0A"                                                    \
   "END:MSG\x0D\x0A"                                                              \
   "END:BBODY\x0D\x0A"                                                            \
   "END:BENV\x0D\x0A"                                                             \
   "END:BMSG\x0D\x0A"

   /* Create a variable to hold the Message Data.  This will prevent    */
   /* this from being placed in Read Only space if a String Literal is  */
   /* used as with the other fields.                                    */
static char DynamicTelecomMsgOutboxContentsMessage0DataBMessageData[] = { TELECOM_MSG_OUTBOX_CONTENTS_MESSAGE_0_DATA_BMESSAGE_DATA } ;

#define TELECOM_MSG_OUTBOX_CONTENTS_MESSAGE_0_DATA                                \
{                                                                                 \
   "20000100004",                                                                 \
   "Ohayougozaimasu",                                                             \
   "20100510T134326Z",                                                            \
   NULL,                /* Sender isn't required                        */        \
   "Andy",                                                                        \
   "+49-7654-321098",                                                             \
   NULL,                /* Reply-To Addressing isn't required           */        \
   "+49-89-01234567",                                                             \
   NULL,                /* Recipient Name isn't required                */        \
   "SMS_CDMA",                                                                    \
   sizeof(TELECOM_MSG_OUTBOX_CONTENTS_MESSAGE_0_DATA_BMESSAGE_DATA),              \
   NULL,                /* Text isn't required                          */        \
   "complete",                                                                    \
   0,                                                                             \
   svNo,                                                                          \
   svYes,                                                                         \
   svYes,                                                                         \
   svNo,                                                                          \
   DynamicTelecomMsgOutboxContentsMessage0DataBMessageData                        \
}

   /* /root/telecom/msg/sent - Message 0 Data                           */
   /* * NOTE * This is simply preformatted BMSG Data, this              */
   /*          implementation does not build the BMSG programmatically. */
#define TELECOM_MSG_SENT_CONTENTS_MESSAGE_0_DATA_BMESSAGE_DATA                    \
   "BEGIN:BMSG\x0D\x0A"                                                           \
   "VERSION:1.0\x0D\x0A"                                                          \
   "STATUS:UNREAD\x0D\x0A"                                                        \
   "TYPE:EMAIL\x0D\x0A"                                                           \
   "FOLDER:SENT\x0D\x0A"                                                          \
   "BEGIN:VCARD\x0D\x0A"                                                          \
   "VERSION:2.1\x0D\x0A"                                                          \
   "N:Marc\x0D\x0A"                                                               \
   "EMAIL:marc@carworkinggroup.bluetooth.org\x0D\x0A"                             \
   "END:VCARD\x0D\x0A"                                                            \
   "BEGIN:BENV\x0D\x0A"                                                           \
   "BEGIN:VCARD\x0D\x0A"                                                          \
   "VERSION:2.1\x0D\x0A"                                                          \
   "N:Some person named burch\x0D\x0A"                                            \
   "EMAIL:burch@carworkinggroup.bluetooth.org\x0D\x0A"                            \
   "END:VCARD\x0D\x0A"                                                            \
   "BEGIN:BBODY\x0D\x0A"                                                          \
   "ENCODING:8BIT\x0D\x0A"                                                        \
   "CHARSET:UTF-8\x0D\x0A"                                                        \
   "LENGTH:83\x0D\x0A"                                                            \
   "BEGIN:MSG\x0D\x0A"                                                            \
   "Date: 15 May 09\x0D\x0A"                                                      \
   "Subject: Fish\x0D\x0A"                                                        \
   "\x0D\x0A"                                                                     \
   "Let\x27s go fishing!\x0D\x0A"                                                 \
   "BR, Marc\x0D\x0A"                                                             \
   "END:MSG\x0D\x0A"                                                              \
   "END:BBODY\x0D\x0A"                                                            \
   "END:BENV\x0D\x0A"                                                             \
   "END:BMSG\x0D\x0A"

   /* Create a variable to hold the Message Data.  This will prevent    */
   /* this from being placed in Read Only space if a String Literal is  */
   /* used as with the other fields.                                    */
static char DynamicTelecomMsgSentContentsMessage0DataBMessageData[] = { TELECOM_MSG_SENT_CONTENTS_MESSAGE_0_DATA_BMESSAGE_DATA } ;

#define TELECOM_MSG_SENT_CONTENTS_MESSAGE_0_DATA                                  \
{                                                                                 \
   "20000100005",                                                                 \
   "Fish",                                                                        \
   "20100509T171204Z",                                                            \
   NULL,                /* Sender isn't required                        */        \
   "Marc",                                                                        \
   "marc@carworkinggroup.bluetooth.org",                                          \
   NULL,                /* Reply-To Addressing isn't required           */        \
   "Some person named burch",                                                     \
   "burch@carworkinggroup.bluetooth.org",                                         \
   "EMAIL",                                                                       \
   sizeof(TELECOM_MSG_SENT_CONTENTS_MESSAGE_0_DATA_BMESSAGE_DATA),                \
   NULL,                /* Text isn't required                          */        \
   "complete",                                                                    \
   0,                                                                             \
   svNo,                                                                          \
   svYes,                                                                         \
   svNo,                                                                          \
   svNo,                                                                          \
   DynamicTelecomMsgSentContentsMessage0DataBMessageData                          \
}

   /* PTS Message Data                                                  */
   /* * NOTE * This is simply preformatted BMSG Data, this              */
   /*          implementation does not build the BMSG programmatically. */
#define PTS_TELECOM_MSG_CONTENTS_MESSAGE_DATA_BMESSAGE_DATA                       \
   "BEGIN:BMSG\x0D\x0A"                                                           \
   "VERSION:1.0\x0D\x0A"                                                          \
   "STATUS:UNREAD\x0D\x0A"                                                        \
   "TYPE:MMS\x0D\x0A"                                                             \
   "FOLDER:TELECOM/MSG/OUTBOX\x0D\x0A"                                            \
   "BEGIN:VCARD\x0D\x0A"                                                          \
   "VERSION:2.1\x0D\x0A"                                                          \
   "N:PTSPM\x0D\x0A"                                                              \
   "EMAIL:ptspm@bluetooth.com\x0D\x0A"                                            \
   "END:VCARD\x0D\x0A"                                                            \
   "BEGIN:BENV\x0D\x0A"                                                           \
   "BEGIN:VCARD\x0D\x0A"                                                          \
   "VERSION:2.1\x0D\x0A"                                                          \
   "N:IUT\x0D\x0A"                                                                \
   "EMAIL:iut@bluetooth.com\x0D\x0A"                                              \
   "END:VCARD\x0D\x0A"                                                            \
   "BEGIN:BBODY\x0D\x0A"                                                          \
   "ENCODING:8BIT\x0D\x0A"                                                        \
   "LENGTH:135\x0D\x0A"                                                           \
   "BEGIN:MSG\x0D\x0A"                                                            \
   "Date: June 12, 2009\x0D\x0A"                                                  \
   "Subject: Test\x0D\x0A"                                                        \
   "From: ptspm@bluetooth.com\x0D\x0A"                                            \
   "To: iut@bluetooth.com\x0D\x0A"                                                \
   "\x0D\x0A"                                                                     \
   "This text is for testing.\x0D\x0A"                                            \
   "END:MSG\x0D\x0A"                                                              \
   "END:BBODY\x0D\x0A"                                                            \
   "END:BENV\x0D\x0A"                                                             \
   "END:BMSG\x0D\x0A"

   /* Create a variable to hold the Message Data.  This will prevent    */
   /* this from being placed in Read Only space if a String Literal is  */
   /* used as with the other fields.                                    */
static char DynamicPTSTelecomMsgContentsMessageDataBMessageData[] = { PTS_TELECOM_MSG_CONTENTS_MESSAGE_DATA_BMESSAGE_DATA } ;

   /* Create a variable to hold the Message Handle.  This will prevent  */
   /* this from being placed in Read Only space if a String Literal is  */
   /* used as with the other fields.                                    */
static char DynamicMessageHandle[MAP_MESSAGE_HANDLE_LENGTH+1] = {"20000100008"};

#define PTS_TELECOM_MSG_CONTENTS_MESSAGE_DATA                                     \
{                                                                                 \
   DynamicMessageHandle,                                                          \
   "Test",                                                                        \
   "20100508T171204Z",                                                            \
   NULL,                /* Sender isn't required                        */        \
   "PTSPM",                                                                       \
   "ptspm@bluetooth.com",                                                         \
   NULL,                /* Reply-To Addressing isn't required           */        \
   "ITU",                                                                         \
   "iut@bluetooth.com",                                                           \
   "MMS",                                                                         \
   sizeof(PTS_TELECOM_MSG_CONTENTS_MESSAGE_DATA_BMESSAGE_DATA),                   \
   NULL,                /* Text isn't required                          */        \
   "complete",                                                                    \
   0,                                                                             \
   svNo,                                                                          \
   svYes,                                                                         \
   svNo,                                                                          \
   svNo,                                                                          \
   DynamicPTSTelecomMsgContentsMessageDataBMessageData                            \
}

   /* The following structure holds all of the Inbox Messages.          */
static MessageEntry_t InboxFolderMessageMapping[] =
{
   TELECOM_MSG_INBOX_CONTENTS_MESSAGE_0_DATA,
   TELECOM_MSG_INBOX_CONTENTS_MESSAGE_1_DATA,
   TELECOM_MSG_INBOX_CONTENTS_MESSAGE_2_DATA,
} ;

   /* The following structure holds all of the Outbox Messages.         */
static MessageEntry_t OutboxFolderMessageMapping[] =
{
   TELECOM_MSG_OUTBOX_CONTENTS_MESSAGE_0_DATA,
} ;

   /* The following structure holds all of the Sent Messages.           */
static MessageEntry_t SentFolderMessageMapping[] =
{
   TELECOM_MSG_SENT_CONTENTS_MESSAGE_0_DATA
} ;

   /* The following structure holds all of the Sent Messages.           */
static MessageEntry_t PTSFolderMessageMapping[] =
{
   PTS_TELECOM_MSG_CONTENTS_MESSAGE_DATA,
   PTS_TELECOM_MSG_CONTENTS_MESSAGE_DATA
} ;

   /* The following structure is provided to allow a mechanism to       */
   /* determine the parent folder of a specific folder (and thus define */
   /* a folder structure).                                              */
typedef struct _tagFolderStructure_t
{
   CurrentFolder_t ParentFolder;
   CurrentFolder_t CurrentFolder;
} FolderStructure_t;

typedef struct _tagFolderNameMapping_t
{
   CurrentFolder_t  Folder;
   char            *FolderName;
   char            *FolderCreationDate;
} FolderNameMapping_t;

   /* The following structure is provided to allow a mechanism to map   */
   /* between a folder and messages that are contained in that folder.  */
typedef struct _tagFolderMessageMapping_t
{
   CurrentFolder_t  Folder;
   MessageEntry_t  *Message;
} FolderMessageMapping_t;

   /* The following table defines the actual Folder Messages that map to*/
   /* a particular Folder.                                              */
static FolderMessageMapping_t FolderMessageMapping[] =
{
   { cfInbox,   &InboxFolderMessageMapping[0]  },
   { cfInbox,   &InboxFolderMessageMapping[1]  },
   { cfInbox,   &InboxFolderMessageMapping[2]  },
   { cfOutbox,  &OutboxFolderMessageMapping[0] },
   { cfSent,    &SentFolderMessageMapping[0]   },
   { cfMessage, NULL                           },
} ;

   /* The following table defines the actual directory structure        */
   /* supported by this module (lists the parent directories of each    */
   /* supported directory).                                             */
static BTPSCONST FolderStructure_t FolderStructure[] =
{
   { cfRoot,    cfTelecom },
   { cfTelecom, cfMessage },
   { cfMessage, cfInbox   },
   { cfMessage, cfOutbox  },
   { cfMessage, cfSent    },
   { cfMessage, cfDeleted }
} ;

   /* The following table defines the mapping from actual PBAP directory*/
   /* object names to the numeric values used by this module.           */
static BTPSCONST FolderNameMapping_t FolderNameMapping[] =
{
   { cfRoot,    "root",    NULL               },
   { cfTelecom, "telecom", "20090526T141500Z" },
   { cfMessage, "msg",     "20090526T141500Z" },
   { cfInbox,   "inbox",   "20090526T141500Z" },
   { cfOutbox,  "outbox",  "20090526T141500Z" },
   { cfSent,    "sent",    "20090526T141500Z" },
   { cfDeleted, "deleted", "20090526T141500Z" }
} ;

   /* The following table defines the mapping from EventType_t to the   */
   /* character string that represents the Event Type.                  */
static BTPSCONST char *EventTypeStr[] =
{
  "NewMessage",
  "DeliverySuccess",
  "SendingSuccess",
  "DeliveryFailure",
  "SendingFailure",
  "MemoryFull",
  "MemoryAvailable",
  "MessageDeleted",
};

   /* The following table defines the mapping from MessageType_t to the */
   /* character string that represents the Message Type.                */
static BTPSCONST char *MessageTypeStr[] =
{
   "EMAIL",
   "SMS_GSM",
   "SMS_CDMA",
   "MMS"
};

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the compiler*/
   /* as part of standard C/C++).                                       */
static CurrentFolder_t CurrentFolder;              /* Holds the current       */
                                                   /* folder that is          */
                                                   /* currently being browsed.*/

static DWord_t NextMessageHandle = 0x000000010;    /* Holds the value that    */
                                                   /* will be used for the    */
                                                   /* Next Calculated Message */

   /* The following function is a utility function that is provided to  */
   /* allow a mechanism to query the current Folder.                    */
CurrentFolder_t GetCurrentMessageFolder(void)
{
   return(CurrentFolder);
}

   /* The following function is a utility function that is provided to  */
   /* allow a mechanism to query the current folder (in string form).   */
char *GetCurrentMessageFolderString(void)
{
   char         *ret_val;
   unsigned int  Index;

   for(Index=0,ret_val=NULL;Index<sizeof(FolderNameMapping)/sizeof(FolderNameMapping_t);Index++)
   {
      if(FolderNameMapping[Index].Folder == CurrentFolder)
      {
         ret_val = FolderNameMapping[Index].FolderName;

         break;
      }
   }

   return(ret_val);
}

   /* The following function is a utility function that is provided to  */
   /* allow a mechansim to change folders (and thus update the Current  */
   /* Folder).  This function accepts as it's parameters, the path      */
   /* option (up, root, or down), followed by the sub-folder name (only */
   /* applicable when descending down the folder (i.e. not applicable   */
   /* when root or up is specified).  This function returns zero if     */
   /* successful, or a negative return value if there was an error.     */
int ChangeMessageFolder(MAP_Set_Folder_Option_t Option, char *Name)
{
   int          ret_val;
   unsigned int Index;
   unsigned int Index1;

   if(Option == sfRoot)
   {
      CurrentFolder = cfRoot;

      ret_val       = 0;
   }
   else
   {
      if(Option == sfUp)
      {
         /* We need to find the parent folder of the current folder.    */
         if(CurrentFolder == cfRoot)
         {
            /* Already at Root, flag success.                           */
            ret_val = 0;
         }
         else
         {
            /* Loop through the folder structure until we find the      */
            /* current folder we are currently at (to find the parent   */
            /* folder).                                                 */
            for(Index=0,ret_val=-1;Index<sizeof(FolderStructure)/sizeof(FolderStructure_t);Index++)
            {
               if(FolderStructure[Index].CurrentFolder == CurrentFolder)
               {
                  /* Current folder found, note the parent folder, flag */
                  /* success, and exit.                                 */
                  CurrentFolder = FolderStructure[Index].ParentFolder;

                  ret_val          = 0;

                  break;
               }
            }
         }
      }
      else
      {
         /* Caller is requesting a sub-folder, make sure it's valid.    */
         if((Name) && (BTPS_StringLength(Name)))
         {
            /* Try to see if the folder specified matches a folder name */
            /* we have in our folder structure.                         */
            for(Index=0,ret_val=-1;Index<sizeof(FolderNameMapping)/sizeof(FolderNameMapping_t);Index++)
            {
               if((BTPS_StringLength(Name) == BTPS_StringLength(FolderNameMapping[Index].FolderName)) && (!BTPS_MemCompareI(Name, FolderNameMapping[Index].FolderName, BTPS_StringLength(Name))))
               {
                  /* OK, we have found a match, now see if this folder  */
                  /* exists at the level we are currently at.           */
                  for(Index1=0;Index1<sizeof(FolderStructure)/sizeof(FolderStructure_t);Index1++)
                  {
                     if(FolderStructure[Index1].CurrentFolder == FolderNameMapping[Index].Folder)
                     {
                        if(FolderStructure[Index1].ParentFolder == CurrentFolder)
                        {
                           /* Match found, set the current folder, flag */
                           /* succes, and exit.                         */
                           CurrentFolder = FolderNameMapping[Index].Folder;

                           ret_val       = 0;

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

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is a utility function that is provided to  */
   /* allow a mechanism to query the total number of Folder Entries that*/
   /* exist at the specified Folder.  This value can be used as an upper*/
   /* bound to iterate the Folder Information in the specified folder   */
   /* (using the QueryFolderEntry() function).  This function returns   */
   /* zero if there are no folders present in the specified, directory, */
   /* a positive non-zero number that represents the number of actual   */
   /* folders located at the specified folder, or a negative value if   */
   /* there was an error.                                               */
int QueryNumberFolderEntries(CurrentFolder_t Folder)
{
   int          ret_val;
   unsigned int Index;

   /* OK, we need to locate andy directorires that have the specified   */
   /* Current Folder as it's parent.                                    */
   for(Index=0,ret_val=0;(Index<sizeof(FolderStructure)/sizeof(FolderStructure_t));Index++)
   {
      /* Check to see if the specified entry specifies the requested    */
      /* folder as it's parent.  If so, increment the folder count.     */
      if(FolderStructure[Index].ParentFolder == Folder)
         ret_val++;
   }

   /* Return result to the caller.                                      */
   return(ret_val);
}

   /* The following function is a utility function that is provided to  */
   /* allow a mechansim to query information about a specific folder    */
   /* entry in the specified directory.  The specific folder is         */
   /* specified by specifying the index of the folder that the caller   */
   /* is interested in.  Valid index values are zero through one less   */
   /* than the return value returned from QueryNumberFolderEntries().   */
   /* This function returns a BOOLEAN TRUE if the folder information was*/
   /* retrieved, or FALSE if there was an error.                        */
Boolean_t QueryFolderEntry(CurrentFolder_t Folder, unsigned int Index, FolderEntry_t *FolderEntry)
{
   Boolean_t    ret_val;
   unsigned int Index1;
   unsigned int Index2;

   /* OK, we need to loop through the Folder Structure until we find the*/
   /* correct number of matches with the Parent Folder (to match up the */
   /* specified Index).                                                 */
   for(Index1=0,Index2=0,ret_val=FALSE;(Index1<sizeof(FolderStructure)/sizeof(FolderStructure_t));Index1++)
   {
      if(FolderStructure[Index1].ParentFolder == Folder)
      {
         /* Match found (for Parent Folder), see if this is the Index   */
         /* that is being requested (note that indexes are zero based). */
         if(Index == Index2)
         {
            /* Correct Index found, now determine the Folder            */
            /* Information.                                             */
            for(Index=0;Index<(sizeof(FolderNameMapping)/sizeof(FolderNameMapping_t));Index++)
            {
               /* We need to loop through the Folder Structure until we */
               /* find the Current Folder match.  Once we find this, we */
               /* have the information we need to return.               */
               if(FolderNameMapping[Index].Folder == FolderStructure[Index1].CurrentFolder)
               {
                  /* Check to make sure input parameter specified was   */
                  /* valid.                                             */
                  if(FolderEntry)
                  {
                     /* Note the Folder Information.                    */
                     FolderEntry->FolderName     = FolderNameMapping[Index].FolderName;
                     FolderEntry->CreateDateTime = FolderNameMapping[Index].FolderCreationDate;

                     ret_val                     = TRUE;
                  }
                  else
                     ret_val = FALSE;

                  /* Break out of the inner loop as match as been found.*/
                  break;
               }
            }

            /* Break out of outter loop as match has been fouud.        */
            break;
         }

         /* Increment the Index to note that we have found a Folder that*/
         /* is located within the specified current directory.          */
         Index2++;
      }
   }

   /* Return result to the caller.                                      */
   return(ret_val);
}

   /* The following function is a utility function that is provided to  */
   /* allow a mechanism to query the total number of Message Entries    */
   /* that exist at the specified Folder.  This value can be used as an */
   /* upper bound to iterate the Message Information in the specified   */
   /* folder (using the QueryMessageEntry() function).  This function   */
   /* returns zero if there are no messages present in the specified,   */
   /* directory, a positive non-zero number that represents the number  */
   /* of actual messages located at the specified folder, or a negative */
   /* value if there was an error.                                      */
int QueryNumberMessageEntries(CurrentFolder_t Folder)
{
   int          ret_val;
   unsigned int Index;

   /* Simply loop through the Folder Message Mapping array intil we find*/
   /* the specified folder.                                             */
   for(Index=0,ret_val=0;Index<(sizeof(FolderMessageMapping)/sizeof(FolderMessageMapping_t));Index++)
   {
      if((FolderMessageMapping[Index].Folder == Folder) && (FolderMessageMapping[Index].Message))
      {
         /* Folder found, go ahead and note the Number of Messages and  */
         /* break out of loop.                                          */
         ret_val++;
      }
   }

   /* Return result to the caller.                                      */
   return(ret_val);
}

   /* The following function is a utility function that is provided to  */
   /* allow a mechansim to query information about a specific message   */
   /* entry in the specified directory.  The specific message is        */
   /* specified by specifying the index of the message that the caller  */
   /* is interested in.  Valid index values are zero through one less   */
   /* than the return value returned from QueryNumberMessageEntries().  */
   /* This function returns a BOOLEAN TRUE if the message information   */
   /* was retrieved, or FALSE if there was an error.                    */
Boolean_t QueryMessageEntryByIndex(CurrentFolder_t Folder, unsigned int Index, MessageEntry_t **MessageEntry)
{
   Boolean_t    ret_val;
   unsigned int Index1;
   unsigned int Index2;

   /* Simply loop through the Folder Message Mapping array intil we find*/
   /* the specified folder.                                             */
   for(Index1=0,Index2=0,ret_val=FALSE;Index1<(sizeof(FolderMessageMapping)/sizeof(FolderMessageMapping_t));Index1++)
   {
      if(FolderMessageMapping[Index1].Folder == Folder)
      {
         /* If Index specified matches the item index, go ahead and note*/
         /* the Message Information.                                    */
         if(Index == Index2)
         {
            if(MessageEntry)
            {
               *MessageEntry = FolderMessageMapping[Index1].Message;

               ret_val       = TRUE;

               break;
            }
         }
         else
           Index2++;
      }
   }

   /* Return result to the caller.                                      */
   return(ret_val);
}

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
Boolean_t QueryMessageEntryByHandle(CurrentFolder_t Folder, char *MessageHandle, MessageEntry_t **MessageEntry)
{
   Boolean_t    ret_val;
   unsigned int Index;

   /* Simply loop through the Folder Message Mapping array intil we find*/
   /* the specified folder.                                             */
   for(Index=0,ret_val=FALSE;Index<(sizeof(FolderMessageMapping)/sizeof(FolderMessageMapping_t));Index++)
   {
      if((Folder == cfInvalid) || (FolderMessageMapping[Index].Folder == Folder))
      {
         if((MessageHandle) && (FolderMessageMapping[Index].Message) && (BTPS_StringLength((FolderMessageMapping[Index].Message)->MessageHandle) == BTPS_StringLength(MessageHandle)) && (!BTPS_MemCompare((FolderMessageMapping[Index].Message)->MessageHandle, MessageHandle, BTPS_StringLength((FolderMessageMapping[Index].Message)->MessageHandle))))
         {
            /* Match found.                                             */
            if(MessageEntry)
            {
               *MessageEntry = FolderMessageMapping[Index].Message;

               ret_val       = TRUE;
            }
         }

         /* Folder found, go ahead and break out of the loop.           */
         if((ret_val) || (Folder != cfInvalid))
            break;
      }
   }

   /* Return result to the caller.                                      */
   return(ret_val);
}

   /* The following function is a utility function that is provided to  */
   /* allow a mechansim to move a message from one folder to another.   */
   /* The specific message is specified by specifying the actual Message*/
   /* Handle of the message that the caller is interested in (cannot be */
   /* NULL).  If located, the current assigned folder will be assigned  */
   /* the folder value that was passed in as long as the folder         */
   /* specified can contain messages.  This function returns a BOOLEAN  */
   /* TRUE if the message information was retrieved, or FALSE if there  */
   /* was an error.                                                     */
Boolean_t MoveMessageEntryToFolderByHandle(CurrentFolder_t Folder, char *MessageHandle)
{
   Boolean_t    ret_val;
   unsigned int Index;

   /* Simply loop through the Folder Message Mapping array intil we find*/
   /* the specified folder.                                             */
   for(Index=0,ret_val=FALSE;Index<(sizeof(FolderMessageMapping)/sizeof(FolderMessageMapping_t));Index++)
   {
      if((MessageHandle) && (FolderMessageMapping[Index].Message) && (BTPS_StringLength((FolderMessageMapping[Index].Message)->MessageHandle) == BTPS_StringLength(MessageHandle)) && (!BTPS_MemCompare((FolderMessageMapping[Index].Message)->MessageHandle, MessageHandle, BTPS_StringLength((FolderMessageMapping[Index].Message)->MessageHandle))))
      {
         /* Match found.                                                */
         if(Folder > cfMessage)
         {
            FolderMessageMapping[Index].Folder = Folder;

            ret_val       = TRUE;

            break;
         }
      }
   }

   /* Return result to the caller.                                      */
   return(ret_val);
}

   /* The following function is a utility function to insert a message  */
   /* into the message listings array.  The function receives the Folder*/
   /* where the message is to be inserted, the message handle that has  */
   /* been assigned, a pointer to the data and the length of the data.  */
   /* The function will return a non-zero value if the function fails to*/
   /* insert the message into the list.                                 */
void InsertMessageEntryIntoFolder(CurrentFolder_t Folder, char *MessageHandle)
{
   int Index;

   /* Get the index of the last entry in the table.                     */
   Index = (sizeof(FolderMessageMapping)/sizeof(FolderMessageMapping_t))-1;

   /* Assign the folder to the message.                                 */
   if(Folder == cfOutbox)
   {
      FolderMessageMapping[Index].Folder        = cfSent;
      FolderMessageMapping[Index].Message       = &PTSFolderMessageMapping[0];
      FolderMessageMapping[Index].Message->Sent = svYes;
   }
   else
   {
      FolderMessageMapping[Index].Folder  = Folder;
      FolderMessageMapping[Index].Message = &PTSFolderMessageMapping[0];
   }

   /* Replace the Message Handle that was assigned.                     */
   BTPS_StringCopy(FolderMessageMapping[Index].Message->MessageHandle, MessageHandle);
}

   /* The following function is a utility function that is provided to  */
   /* allow a mechanism to retreive the Event Type (in string form).    */
char *GetEventTypeString(EventType_t EventType)
{
   /* Verify that the value passed in is in the valid range.            */
   return((char *)(((EventType >= etNewMessage) && (EventType <= etMessageDeleted))?EventTypeStr[EventType]:NULL));
}

   /* The following function is a utility function that is provided to  */
   /* allow a mechanism to retreive the Message Type (in string form).  */
char *GetMessageTypeString(MessageType_t MessageType)
{
   /* Verify that the value passed in is in the valid range.            */
   return((char *)(((MessageType >= mtEMAIL) && (MessageType <= mtMMS))?MessageTypeStr[MessageType]:NULL));
}

   /* The following function is a utility function that is provided to  */
   /* allow a mechansim to generate a unique Message Handle that can be */
   /* used to uniquely identify a received message.  This function      */
   /* returns a BOOLEAN TRUE if a unique Message Handle was able to be  */
   /* generated and placed in the input buffer, or FALSE if there was an*/
   /* error storing the unique handle in the input buffer (i.e. the     */
   /* buffer passed in was invalid).                                    */
Boolean_t GenerateUniqueMessageHandle(unsigned int BufferLength, char *MessageHandleBuffer)
{
   Boolean_t ret_val;

   /* Check to see if the input parameters appear to be semi-valid.     */
   if((BufferLength >= (BTPS_StringLength(MESSAGE_HANDLE_PREFIX_STRING) + 8)) && (MessageHandleBuffer))
   {
      BTPS_SprintF(MessageHandleBuffer, "%s%08lX", MESSAGE_HANDLE_PREFIX_STRING, (unsigned long)NextMessageHandle);

      ret_val = TRUE;
   }
   else
      ret_val = FALSE;

   /* Return result to the caller.                                      */
   return(ret_val);
}

   /* The following function is a utility function that is used to      */
   /* increment the Unique File Handle that is used for pushed messages.*/
   /* * NOTE * This should only be called after the final message of a  */
   /*          Push Indication.                                         */
void IncrementMessageHandle(void)
{
   NextMessageHandle++;
}
