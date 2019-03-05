/*****< linuxsyn.c >***********************************************************/
/*      Copyright 2009 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  LINUXSYN - Simple Linux application using OBEX SYNC Profile (SYNC).       */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   09/11/09  D. Lange        Initial creation.                              */
/******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>

#include "LinuxSYN.h"      /* Main Application Prototypes and Constants.      */

#include "ObjStore.h"      /* Sample Object Store Prototypes and Constants.   */

#include "SS1BTPS.h"       /* Includes for the SS1 Bluetooth Protocol Stack.  */
#include "SS1BTSYN.h"      /* Includes for the SS1 OBEX SYNC Profile.         */
#include "SS1BTDBG.h"      /* Includes/Constants for Bluetooth Debugging.     */

#define LOCAL_NAME_ROOT                      "LinuxSYN"  /* Root of the local */
                                                         /* name              */

#define NUM_EXPECTED_PARAMETERS_USB                 (3)  /* Denotes the number*/
                                                         /* of command line   */
                                                         /* parameters        */
                                                         /* accepted at Run   */
                                                         /* Time when running */
                                                         /* in USB Mode.      */

#define NUM_EXPECTED_PARAMETERS_UART                (5)  /* Denotes the       */
                                                         /* number of command */
                                                         /* line parameters   */
                                                         /* accepted at Run   */
                                                         /* Time when running */
                                                         /* in UART Mode.     */

#define USB_PARAMETER_VALUE                         (0)  /* Denotes the value */
                                                         /* passed in on the  */
                                                         /* command line for  */
                                                         /* running with the  */
                                                         /* transport set to  */
                                                         /* USB.              */

#define UART_PARAMETER_VALUE                        (1)  /* Denotes the value */
                                                         /* passed in on the  */
                                                         /* command line for  */
                                                         /* running with the  */
                                                         /* transport set to  */
                                                         /* UART.             */

#define BCSP_PARAMETER_VALUE                        (2)  /* Denotes the value */
                                                         /* passed in on the  */
                                                         /* command line for  */
                                                         /* running with the  */
                                                         /* transport set to  */
                                                         /* BCSP.             */

#define MAX_SUPPORTED_COMMANDS                     (32)  /* Denotes the       */
                                                         /* maximum number of */
                                                         /* User Commands that*/
                                                         /* are supported by  */
                                                         /* this application. */

#define MAX_COMMAND_LENGTH                         (64)  /* Denotes the max   */
                                                         /* buffer size used  */
                                                         /* for user commands */
                                                         /* input via the     */
                                                         /* User Interface.   */

#define MAX_NUM_OF_PARAMETERS                       (5)  /* Denotes the max   */
                                                         /* number of         */
                                                         /* parameters a      */
                                                         /* command can have. */

#define MAX_INQUIRY_RESULTS                        (32)  /* Denotes the max   */
                                                         /* number of inquiry */
                                                         /* results.          */

#define DEFAULT_IO_CAPABILITY          (icDisplayYesNo)  /* Denotes the       */
                                                         /* default I/O       */
                                                         /* Capability that is*/
                                                         /* used with Secure  */
                                                         /* Simple Pairing.   */

#define DEFAULT_MITM_PROTECTION                  (TRUE)  /* Denotes the       */
                                                         /* default value used*/
                                                         /* for Man in the    */
                                                         /* Middle (MITM)     */
                                                         /* protection used   */
                                                         /* with Secure Simple*/
                                                         /* Pairing.          */

#define NO_COMMAND_ERROR                           (-1)  /* Denotes that no   */
                                                         /* command was       */
                                                         /* specified to the  */
                                                         /* parser.           */

#define INVALID_COMMAND_ERROR                      (-2)  /* Denotes that the  */
                                                         /* Command does not  */
                                                         /* exist for         */
                                                         /* processing.       */

#define EXIT_CODE                                  (-3)  /* Denotes that the  */
                                                         /* Command specified */
                                                         /* was the Exit      */
                                                         /* Command.          */

#define FUNCTION_ERROR                             (-4)  /* Denotes that an   */
                                                         /* error occurred in */
                                                         /* execution of the  */
                                                         /* Command Function. */

#define TO_MANY_PARAMS                             (-5)  /* Denotes that there*/
                                                         /* are more          */
                                                         /* parameters then   */
                                                         /* will fit in the   */
                                                         /* UserCommand.      */

#define INVALID_PARAMETERS_ERROR                   (-6)  /* Denotes that an   */
                                                         /* error occurred due*/
                                                         /* to the fact that  */
                                                         /* one or more of the*/
                                                         /* required          */
                                                         /* parameters were   */
                                                         /* invalid.          */

#define UNABLE_TO_INITIALIZE_STACK                 (-7)  /* Denotes that an   */
                                                         /* error occurred    */
                                                         /* while Initializing*/
                                                         /* the Bluetooth     */
                                                         /* Protocol Stack.   */

#define INVALID_STACK_ID_ERROR                     (-8)  /* Denotes that an   */
                                                         /* error occurred due*/
                                                         /* to attempted      */
                                                         /* execution of a    */
                                                         /* Command when a    */
                                                         /* Bluetooth Protocol*/
                                                         /* Stack has not been*/
                                                         /* opened.           */

#define UNABLE_TO_REGISTER_SERVER                  (-9)  /* Denotes that an   */
                                                         /* error occurred    */
                                                         /* when trying to    */
                                                         /* create a SYNC     */
                                                         /* Server.           */

#define INDENT_LENGTH                               (3)  /* Denotes the number*/
                                                         /* of character      */
                                                         /* spaces to be used */
                                                         /* for indenting when*/
                                                         /* displaying SDP    */
                                                         /* Data Elements.    */

   /* The following constants represent the default log file names that */
   /* are used if no Log file name is specified when enabling debug.    */
#define DEFAULT_DEBUG_LOG_FILE_NAME       "LinuxSYN_ASC.log"
#define DEFAULT_DEBUG_FTS_FILE_NAME       "LinuxSYN_FTS.log"

   /* The following type definition represents the container type which */
   /* holds the mapping between Bluetooth devices (based on the BD_ADDR)*/
   /* and the Link Key (BD_ADDR <-> Link Key Mapping).                  */
typedef struct _tagLinkKeyInfo_t
{
   BD_ADDR_t  BD_ADDR;
   Link_Key_t LinkKey;
} LinkKeyInfo_t;

   /* The following type definition represents the container type which */
   /* holds the mapping between Profile UUIDs and Profile Names (UUID   */
   /* <-> Name).                                                        */
typedef struct _tagUUIDInfo_t
{
   char       *Name;
   UUID_128_t  UUID;
} UUIDInfo_t;

#define NUMBER_SUPPORTED_OBJECT_STORES    (osUnknown)

   /* The following enumerated type represents all of the operations    */
   /* that can be on-going at any given instant (either client or       */
   /* server).                                                          */
typedef enum
{
   coNone,
   coObjectGet,
   coObjectPut,
   coSpecialObjectGet,
   coRTCPut,
   coObjectDelete,
   coAbort,
   coDisconnect
} CurrentOperation_t;

   /* The following structure contains all the state information tracked*/
   /* by a running instance of an IrMC Sync Server.  This implementation*/
   /* only supports a single server instance, but these variables are   */
   /* stored in a structure so that alternate implementations might     */
   /* support multiple concurrent server instances.                     */
typedef struct _tagSYNCServer_t
{
   DWord_t              ServiceRecordHandle;
   unsigned int         SYNCID;
   Boolean_t            Connected;
   CurrentOperation_t   CurrentOperation;
   IrMC_Operation_t     Operation;
   IrMC_Object_Entry_t *CurrentObject;
   unsigned int         CurrentObjectDataIndex;
   Byte_t              *Buffer;
   DWord_t              BufferSize;
   DWord_t              DataSize;
   Mutex_t              Mutex;
} SYNCServer_t;

   /* The following type definition represents the structure which holds*/
   /* all information about the parameter, in particular the parameter  */
   /* as a string and the parameter as an unsigned int.                 */
typedef struct _tagParameter_t
{
   char         *strParam;
   unsigned int  intParam;
} Parameter_t;

   /* The following type definition represents the structure which holds*/
   /* a list of parameters that are to be associated with a command The */
   /* NumberofParameters variable holds the value of the number of      */
   /* parameters in the list.                                           */
typedef struct _tagParameterList_t
{
   int         NumberofParameters;
   Parameter_t Params[MAX_NUM_OF_PARAMETERS];
} ParameterList_t;

   /* The following type definition represents the structure which holds*/
   /* the command and parameters to be executed.                        */
typedef struct _tagUserCommand_t
{
   char            *Command;
   ParameterList_t  Parameters;
} UserCommand_t;

   /* The following type definition represents the generic function     */
   /* pointer to be used by all commands that can be executed by the    */
   /* test program.                                                     */
typedef int (*CommandFunction_t)(ParameterList_t *TempParam);

   /* The following type definition represents the structure which holds*/
   /* information used in the interpretation and execution of Commands. */
typedef struct _tagCommandTable_t
{
   char              *CommandName;
   CommandFunction_t  CommandFunction;
} CommandTable_t;

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the         */
   /* compiler as part of standard C/C++).                              */
static int                 IsClient;                /* Variable used to indicate if the*/
                                                    /* program is to be run in Client  */
                                                    /* Mode or Server Mode.            */

static unsigned int        SYNCID;                  /* Variable used to hold the       */
                                                    /* SYNC Client or Server ID of the */
                                                    /* currently active SYNC Profile   */
                                                    /* (Client or Server Role).        */

static unsigned int        CommandServerID;         /* Variable used to hold the       */
                                                    /* SYNC ID of the SYNC Command     */
                                                    /* Server.                         */

static DWord_t             ServerSDPRecordHandle;   /* Variable used to hold the       */
                                                    /* SYNC Server SDP Service Record  */
                                                    /* Handle of the SYNC Server SDP   */
                                                    /* Service Record.                 */

static DWord_t             CommandServerSDPRecordHandle; /* Variable used to hold the  */
                                                    /* SYNC Command Server SDP Service */
                                                    /* Record Handle of the SYNC       */
                                                    /* Command Server SDP Service      */
                                                    /* Record.                         */

static Boolean_t           Connected;               /* Variable which reflects whether */
                                                    /* or not there is an active       */
                                                    /* (on-going) connection present.  */

static char                ObjectStorePath[512];    /* Variable which holds the current*/
                                                    /* path where received objects     */
                                                    /* (server side) are stored.       */

static CurrentOperation_t  CurrentOperation;        /* Variable which holds the current*/
                                                    /* (on-going operation).           */

static char                CurrentFileName[512];    /* Variables are used when sending */
static unsigned int        CurrentBufferSize;       /* and receiving data.  These      */
static unsigned int        CurrentBufferSent;       /* variables are used by both the  */
static unsigned char      *CurrentBuffer;           /* client and server when an       */
                                                    /* operation requires data to be   */
                                                    /* transferred to the remote       */
                                                    /* device.                         */

static SYNCServer_t        SYNCServer;              /* Variable which contains all     */
                                                    /* SYNC Server State Information.  */

static unsigned int        BluetoothStackID;        /* Variable which holds the Handle */
                                                    /* of the opened Bluetooth Protocol*/
                                                    /* Stack.                          */

static unsigned int        DebugID;                 /* Variable which holds the ID of  */
                                                    /* the currently enabled debugging */
                                                    /* session.                        */

static BD_ADDR_t           InquiryResultList[MAX_INQUIRY_RESULTS];  /*Variable which   */
                                                    /* contains the inquiry result     */
                                                    /* received from the most recently */
                                                    /* preformed inquiry.              */

static unsigned int        NumberofValidResponses;  /* Variable which holds the number */
                                                    /* of valid inquiry results within */
                                                    /* the inquiry results array.      */

static LinkKeyInfo_t       LinkKeyInfo[16];         /* Variable which holds the list of*/
                                                    /* BD_ADDR <-> Link Keys for       */
                                                    /* pairing.                        */

static BD_ADDR_t           CurrentRemoteBD_ADDR;    /* Variable which holds the        */
                                                    /* current BD_ADDR of the device   */
                                                    /* which is currently pairing or   */
                                                    /* authenticating.                 */

static GAP_IO_Capability_t IOCapability;            /* Variable which holds the        */
                                                    /* current I/O Capabilities that   */
                                                    /* are to be used for Secure Simple*/
                                                    /* Pairing.                        */

static Boolean_t           OOBSupport;              /* Variable which flags whether    */
                                                    /* or not Out of Band Secure Simple*/
                                                    /* Pairing exchange is supported.  */

static Boolean_t           MITMProtection;          /* Variable which flags whether or */
                                                    /* not Man in the Middle (MITM)    */
                                                    /* protection is to be requested   */
                                                    /* during a Secure Simple Pairing  */
                                                    /* procedure.                      */

static unsigned int        NumberCommands;          /* Variable which is used to hold  */
                                                    /* the number of Commands that are */
                                                    /* supported by this application.  */
                                                    /* Commands are added individually.*/

static CommandTable_t      CommandTable[MAX_SUPPORTED_COMMANDS]; /* Variable which is  */
                                                    /* used to hold the actual Commands*/
                                                    /* that are supported by this      */
                                                    /* application.                    */

   /* The following string table is used to generate LUID strings.      */
static char *ObjectStreamExtensions[] =
{
   ".vcf",
   ".vcs",
   ".vmg",
   ".vmg",
   ".vmg",
   ".vnt",
   ".vbk",
   ".vvv",
   ".vvv"
} ;

   /* The following string table is used to map HCI Version information */
   /* to an easily displayable version string.                          */
static char *HCIVersionStrings[] =
{
   "1.0b",
   "1.1",
   "1.2",
   "2.0",
   "2.1",
   "3.0",
   "4.0",
   "4.1",
   "4.2",
   "5.0",
   "Unknown (greater 5.0)"
} ;

#define NUM_SUPPORTED_HCI_VERSIONS              (sizeof(HCIVersionStrings)/sizeof(char *) - 1)

   /* The following string table is used to map the API I/O Capabilities*/
   /* values to an easily displayable string.                           */
static char *IOCapabilitiesStrings[] =
{
   "Display Only",
   "Display Yes/No",
   "Keyboard Only",
   "No Input/Output"
} ;

static UUIDInfo_t UUIDTable[] =
{
   { "L2CAP",                 { 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB } },
   { "Advanced Audio",        { 0x00, 0x00, 0x11, 0x0D, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB } },
   { "A/V Remote Control",    { 0x00, 0x00, 0x11, 0x0E, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB } },
   { "Basic Imaging",         { 0x00, 0x00, 0x11, 0x1A, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB } },
   { "Basic Printing",        { 0x00, 0x00, 0x11, 0x22, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB } },
   { "Dial-up Networking",    { 0x00, 0x00, 0x11, 0x03, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB } },
   { "FAX",                   { 0x00, 0x00, 0x11, 0x11, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB } },
   { "File Transfer",         { 0x00, 0x00, 0x11, 0x06, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB } },
   { "Hard Copy Cable Repl.", { 0x00, 0x00, 0x11, 0x25, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB } },
   { "Health Device",         { 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB } },
   { "Headset",               { 0x00, 0x00, 0x11, 0x08, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB } },
   { "Handsfree",             { 0x00, 0x00, 0x11, 0x1E, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB } },
   { "HID",                   { 0x00, 0x00, 0x11, 0x24, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB } },
   { "LAN Access",            { 0x00, 0x00, 0x11, 0x02, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB } },
   { "Message Access",        { 0x00, 0x00, 0x11, 0x34, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB } },
   { "Object Push",           { 0x00, 0x00, 0x11, 0x05, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB } },
   { "Personal Area Network", { 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB } },
   { "Phonebook Access",      { 0x00, 0x00, 0x11, 0x30, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB } },
   { "SIM Access",            { 0x00, 0x00, 0x11, 0x2D, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB } },
   { "Serial Port",           { 0x00, 0x00, 0x11, 0x01, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB } },
   { "IrSYNC",                { 0x00, 0x00, 0x11, 0x04, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB } }
} ;

#define NUM_UUIDS                               (sizeof(UUIDTable)/sizeof(UUIDInfo_t))

   /* This table contains text names for the various object store types */
   /* supported by this implementation.  Note that all these types are  */
   /* mandated in the Sync Profile.  The table is organized/indexed by  */
   /* the enumeration value for each type defined in IrMCTypes.h.       */
static char *ObjectStoreTypeStrings[] =
{
   "Phonebook",
   "Calendar",
   "Msg In",
   "Msg Out",
   "Msg Sent",
   "Notes",
   "Bookmarks",
   "Inbox",
   "Unknown"
} ;

   /* This table contains text names for the various object types       */
   /* supported by this implementation.  The table is organized/indexed */
   /* by the enumeration value for each type.                           */
static char *ObjectTypeStrings[] =
{
   "Change Counter",
   "Change Log",
   "Info Log",
   "Device Info",
   "RTC",
   "Sync Command",
   "Unknown"
};

static const IrMC_Name_Value_Pair_t Extensions[] =
{
   { "X-SS1-EXT1", "ONE" },
   { "X-SS1-EXT2", "TWO" }
} ;

   /* The following table defines the information that will be sent in  */
   /* the device's information log.  Because the device information log */
   /* contains unchanging information about a particular device version,*/
   /* this information is stored as constant data and is fixed at       */
   /* runtime.                                                          */
static const IrMC_Device_Information_Log_t DefaultDeviceInfo =
{
   "SS1",               /* Manufacturer                                 */
   "01",                /* Model                                        */
   NULL,                /* OEM (optional)                               */
   NULL,                /* FirmwareVersion (optional)                   */
   NULL,                /* FirmwareDate (optional)                      */
   NULL,                /* SoftwareVersion (optional)                   */
   NULL,                /* SoftwareDate (optional)                      */
   "1.1",               /* IrMCVersion (Must be "1.1")                  */
   NULL,                /* HardwareVersion (optional)                   */
   NULL,                /* HardwareDate (optional)                      */
   "SS100000001",       /* SerialNumber                                 */
   "VCARD2.1",          /* PhonebookType                                */
   "VCAL1.0",           /* CalendarType                                 */
   "VMSG1.1",           /* MessageType                                  */
   "VNOTE1.1",          /* NoteType                                     */
   "SINGLE",            /* InboxCapability                              */
   "NO",                /* SentboxCapability                            */
   (sizeof(Extensions)/sizeof(IrMC_Name_Value_Pair_t)),
   (IrMC_Name_Value_Pair_t *)Extensions
} ;

   /* Dynamic fields defined for the Phonebook information log.         */
static char *PhonebookFields[] =
{
   "N:=20",
   "UID:=4",
   "ADR[1=20;2;6;7]:",
   "TEL;TYPE=HOME;WORK:"
} ;

   /* Phonebook Information Log.  The provided are values are the       */
   /* default values for each info log.  These values will change during*/
   /* the existence of the sync sever and between multiple sessions.    */
static IrMC_Information_Log_t PhonebookInfoLog =
{
   osPhonebook,
   0,                                     /* Total Records              */
   0,                                     /* Last Used Index            */
   FALSE,                                 /* Use Last Used Index        */
   0,                                     /* Maximum Records            */
   IRMC_INFORMATION_EXCHANGE_LEVEL_124,   /* Information Exchange Level */
   TRUE,                                  /* Use Level 4 Format         */
   FALSE,                                 /* Hard Delete Flag           */
   IRMC_SYNC_ANCHOR_TYPE_BIT_CC,          /* Sync Anchor Type           */
   TRUE,                                  /* Sync Anchor Increment      */
   TRUE,                                  /* Sync Anchor Unique         */
   0,                                     /* Database ID                */
   FALSE,                                 /* Incoming Call Log          */
   FALSE,                                 /* Outgoing Call Log          */
   FALSE,                                 /* Missed Call Log            */
   FALSE,                                 /* Missed Message History     */
   4,                                     /* Number of Dynamic Fields   */
   PhonebookFields                        /* Dynamic Fields Array       */
} ;

   /* Calendar Information Log.  The provided are values are the default*/
   /* values for each info log.  These values will change during the    */
   /* existence of the sync sever and between multiple sessions.        */
static IrMC_Information_Log_t CalendarInfoLog =
{
   osCalendar,
   0,                                     /* Total Records              */
   0,                                     /* Last Used Index            */
   FALSE,                                 /* Use Last Used Index        */
   0,                                     /* Maximum Records            */
   IRMC_INFORMATION_EXCHANGE_LEVEL_124,   /* Information Exchange Level */
   TRUE,                                  /* Use Level 4 Format         */
   FALSE,                                 /* Hard Delete Flag           */
   IRMC_SYNC_ANCHOR_TYPE_BIT_CC,          /* Sync Anchor Type           */
   TRUE,                                  /* Sync Anchor Increment      */
   TRUE,                                  /* Sync Anchor Unique         */
   0,                                     /* Database ID                */
   FALSE,                                 /* Incoming Call Log          */
   FALSE,                                 /* Outgoing Call Log          */
   FALSE,                                 /* Missed Call Log            */
   FALSE,                                 /* Missed Message History     */
   0,                                     /* Number of Dynamic Fields   */
   0                                      /* Dynamic Fields Array       */
};

   /* Notes Information Log.  The provided are values are the default   */
   /* values for each info log.  These values will change during the    */
   /* existence of the sync sever and between multiple sessions.        */
static IrMC_Information_Log_t NotesInfoLog =
{
   osNotes,
   0,                                     /* Total Records              */
   0,                                     /* Last Used Index            */
   FALSE,                                 /* Use Last Used Index        */
   0,                                     /* Maximum Records            */
   IRMC_INFORMATION_EXCHANGE_LEVEL_124,   /* Information Exchange Level */
   TRUE,                                  /* Use Level 4 Format         */
   FALSE,                                 /* Hard Delete Flag           */
   IRMC_SYNC_ANCHOR_TYPE_BIT_CC,          /* Sync Anchor Type           */
   TRUE,                                  /* Sync Anchor Increment      */
   TRUE,                                  /* Sync Anchor Unique         */
   0,                                     /* Database ID                */
   FALSE,                                 /* Incoming Call Log          */
   FALSE,                                 /* Outgoing Call Log          */
   FALSE,                                 /* Missed Call Log            */
   FALSE,                                 /* Missed Message History     */
   0,                                     /* Number of Dynamic Fields   */
   0                                      /* Dynamic Fields Array       */
};

   /* Messages Information Log.  The provided are values are the default*/
   /* values for each info log.  These values will change during the    */
   /* existence of the sync sever and between multiple sessions.        */
static IrMC_Information_Log_t MessagesInfoLog =
{
   osMsgIn,
   0,                                     /* Total Records              */
   0,                                     /* Last Used Index            */
   FALSE,                                 /* Use Last Used Index        */
   0,                                     /* Maximum Records            */
   IRMC_INFORMATION_EXCHANGE_LEVEL_124,   /* Information Exchange Level */
   TRUE,                                  /* Use Level 4 Format         */
   FALSE,                                 /* Hard Delete Flag           */
   IRMC_SYNC_ANCHOR_TYPE_BIT_CC,          /* Sync Anchor Type           */
   TRUE,                                  /* Sync Anchor Increment      */
   TRUE,                                  /* Sync Anchor Unique         */
   0,                                     /* Database ID                */
   FALSE,                                 /* Incoming Call Log          */
   FALSE,                                 /* Outgoing Call Log          */
   FALSE,                                 /* Missed Call Log            */
   FALSE,                                 /* Missed Message History     */
   0,                                     /* Number of Dynamic Fields   */
   0                                      /* Dynamic Fields Array       */
} ;

   /* Object Store Storage.  This implementation uses a static number of*/
   /* possible stores equal to the number of different store types      */
   /* supported by the Sync Profile.  This means that only one store of */
   /* each type is supported.  Alternate implementations could provide  */
   /* other options.  The index into the array for each store is equal  */
   /* to that store's numerical value in the Store Types enumeration.   */
static IrMC_Object_Store_t ObjectStore[NUMBER_SUPPORTED_OBJECT_STORES];

   /* Internal function prototypes.                                     */
static void UserInterface_Client(void);
static void UserInterface_Server(void);
static unsigned int StringToUnsignedInteger(char *StringInteger);
static char *StringParser(char *String);
static int CommandParser(UserCommand_t *TempCommand, char *UserInput);
static int CommandInterpreter(UserCommand_t *TempCommand);
static int AddCommand(char *CommandName, CommandFunction_t CommandFunction);
static CommandFunction_t FindCommand(char *Command);
static void ClearCommands(void);

static int ReadFileIntoCurrentBuffer(char *FileName);
static void WriteReceivedData(unsigned int BufferLength, unsigned char *Buffer);
static int TimestampToString(char *Buffer, IrMC_TimeDate_t *TimeDate);
static int TimestampToTimeDate(char *Buffer, IrMC_TimeDate_t *TimeDate);
static void PopulateCurrentTimeDate(IrMC_TimeDate_t *TimeDate);
static int ExtractInt(char *Ptr, int NumberOfDigits);
static Boolean_t VerifyObjectStorePath(char *StorePath);
static void BD_ADDRToStr(BD_ADDR_t Board_Address, char *BoardStr);
static void WriteEIRInformation(char *LocalName);

static int ObjectStoreIORead(IrMC_Object_Store_Type_t Type, IrMC_Object_Entry_t *Entry, char *StoreData);
static int ObjectStoreIOWrite(IrMC_Object_Store_Type_t Type, IrMC_Object_Entry_t *Entry, DWord_t BufferSize, const Byte_t *Buffer, char *StoreData);
static int ObjectStoreIOCreate(IrMC_Object_Store_Type_t Type, IrMC_Object_Entry_t *Entry, DWord_t BufferSize, const Byte_t *Buffer, char *StoreData);
static int ObjectStoreIODelete(IrMC_Object_Store_Type_t Type, IrMC_Object_Entry_t *Entry, Boolean_t HardDelete, char *StoreData);
static int ObjectStoreIOPopulate(IrMC_Object_Store_t *Store, char *StoreData);

static int RegisterObjectStore(IrMC_Object_Store_Type_t ObjectStoreType, IrMC_Information_Log_t *infolog, char *SerialNumber, char *StoreData, IrMC_Object_Store_IO_Interface_t *ObjIOInterface);

int RegisterObjectStore(IrMC_Object_Store_Type_t ObjectStoreType, IrMC_Information_Log_t *infolog, char *SerialNumber, char *StoreData, IrMC_Object_Store_IO_Interface_t *ObjIOInterface);

static int OpenStack(HCI_DriverInformation_t *HCI_DriverInformation);
static int CloseStack(void);

static int SetDisc(void);
static int SetConnect(void);
static int SetPairable(void);
static int DeleteLinkKey(BD_ADDR_t BD_ADDR);

static int DisplayHelp(ParameterList_t *TempParam);
static int EnableDebug(ParameterList_t *TempParam);
static int Inquiry(ParameterList_t *TempParam);
static int DisplayInquiryList(ParameterList_t *TempParam);
static int SetDiscoverabilityMode(ParameterList_t *TempParam);
static int SetConnectabilityMode(ParameterList_t *TempParam);
static int SetPairabilityMode(ParameterList_t *TempParam);
static int ChangeSimplePairingParameters(ParameterList_t *TempParam);
static int Pair(ParameterList_t *TempParam);
static int EndPairing(ParameterList_t *TempParam);
static int PINCodeResponse(ParameterList_t *TempParam);
static int PassKeyResponse(ParameterList_t *TempParam);
static int UserConfirmationResponse(ParameterList_t *TempParam);
static int GetLocalAddress(ParameterList_t *TempParam);
static int SetLocalName(ParameterList_t *TempParam);
static int GetLocalName(ParameterList_t *TempParam);
static int SetClassOfDevice(ParameterList_t *TempParam);
static int GetClassOfDevice(ParameterList_t *TempParam);
static int GetRemoteName(ParameterList_t *TempParam);
static int ServiceDiscovery(ParameterList_t *TempParam);

static int OpenServer(ParameterList_t *TempParam);
static int CloseServer(ParameterList_t *TempParam);

static int OpenRemoteServer(ParameterList_t *TempParam);
static int CloseConnection(ParameterList_t *TempParam);

static int OpenCommandServer(ParameterList_t *TempParam);
static int CloseCommandServer(ParameterList_t *TempParam);

static int Abort(ParameterList_t *TempParam);

static Boolean_t GenerateLUIDString(IrMC_Object_Store_Type_t StoreType, char *LUID, DWord_t LUIDSize);

static int IrMCObjectGet(ParameterList_t *TempParam);
static int IrMCObjectPut(ParameterList_t *TempParam);
static int IrMCSpecialGet(ParameterList_t *TempParam);
static int IrMCRTCPut(ParameterList_t *TempParam);
static int IrMCObjectDelete(ParameterList_t *TempParam);

static void ClearSYNCServerOperation(void);

static void ProcessIrMCObjectGetIndication(SYNC_IrMC_Object_Get_Indication_Data_t *SYNC_IrMC_Object_Get_Indication_Data);
static void ProcessIrMCObjectPutIndication(SYNC_IrMC_Object_Put_Indication_Data_t *SYNC_IrMC_Object_Put_Indication_Data);
static void ProcessIrMCSpecialObjectGetIndication(SYNC_IrMC_Special_Object_Get_Indication_Data_t *SYNC_IrMC_Special_Object_Get_Indication_Data);
static void ProcessIrMCObjectDeleteIndication(SYNC_IrMC_Object_Delete_Indication_Data_t *SYNC_IrMC_Object_Delete_Indication_Data);

static void DisplaySDPAttributeResponse(SDP_Service_Attribute_Response_Data_t *SDPServiceAttributeResponse, unsigned int InitLevel);
static void DisplaySDPSearchAttributeResponse(SDP_Service_Search_Attribute_Response_Data_t *SDPServiceSearchAttributeResponse);
static void DisplayDataElement(SDP_Data_Element_t *SDPDataElement, unsigned int Level);

   /* BTPS Callback function prototypes.                                */
static void BTPSAPI GAP_Event_Callback(unsigned int BluetoothStackID, GAP_Event_Data_t *GAP_Event_Data, unsigned long CallbackParameter);
static void BTPSAPI SDP_Event_Callback(unsigned int BluetoothStackID, unsigned int SDPRequestID, SDP_Response_Data_t *SDP_Response_Data, unsigned long CallbackParameter);
static void BTPSAPI SYNC_Event_Callback_Server(unsigned int BluetoothStackID, SYNC_Event_Data_t *SYNCEventData, unsigned long CallbackParameter);
static void BTPSAPI SYNC_Event_Callback_Client(unsigned int BluetoothStackID, SYNC_Event_Data_t *SYNCEventData, unsigned long CallbackParameter);
static void BTPSAPI SYNC_Event_Callback_SyncCommand(unsigned int BluetoothStackID, SYNC_Event_Data_t *SYNCEventData, unsigned long CallbackParameter);

   /* This function is responsible for taking the input from the user   */
   /* and dispatching the appropriate Command Function.  First, this    */
   /* function retrieves a String of user input, parses the user input  */
   /* into Command and Parameters, and finally executes the Command or  */
   /* Displays an Error Message if the input is not a valid Command.    */
static void UserInterface_Client(void)
{
   UserCommand_t TempCommand;
   int           Result = !EXIT_CODE;
   char          UserInput[MAX_COMMAND_LENGTH];

   /* First let's make sure that we start on new line.                  */
   printf("\r\n");

   /* Next display the available commands.                              */
   DisplayHelp(NULL);

   ClearCommands();

   AddCommand("INQUIRY", Inquiry);
   AddCommand("DISPLAYINQUIRYLIST", DisplayInquiryList);
   AddCommand("PAIR", Pair);
   AddCommand("ENDPAIRING", EndPairing);
   AddCommand("PINCODERESPONSE", PINCodeResponse);
   AddCommand("PASSKEYRESPONSE", PassKeyResponse);
   AddCommand("USERCONFIRMATIONRESPONSE", UserConfirmationResponse);
   AddCommand("SETDISCOVERABILITYMODE", SetDiscoverabilityMode);
   AddCommand("SETCONNECTABILITYMODE", SetConnectabilityMode);
   AddCommand("SETPAIRABILITYMODE", SetPairabilityMode);
   AddCommand("CHANGESIMPLEPAIRINGPARAMETERS", ChangeSimplePairingParameters);
   AddCommand("GETLOCALADDRESS", GetLocalAddress);
   AddCommand("SETLOCALNAME", SetLocalName);
   AddCommand("GETLOCALNAME", GetLocalName);
   AddCommand("SETCLASSOFDEVICE", SetClassOfDevice);
   AddCommand("GETCLASSOFDEVICE", GetClassOfDevice);
   AddCommand("GETREMOTENAME", GetRemoteName);
   AddCommand("SERVICEDISCOVERY", ServiceDiscovery);
   AddCommand("CONNECT", OpenRemoteServer);
   AddCommand("CLOSECONNECTION", CloseConnection);
   AddCommand("IRMCOBJECTGET", IrMCObjectGet);
   AddCommand("IRMCOBJECTPUT", IrMCObjectPut);
   AddCommand("IRMCSPECIALGET", IrMCSpecialGet);
   AddCommand("IRMCRTCPUT", IrMCRTCPut);
   AddCommand("IRMCOBJECTDELETE", IrMCObjectDelete);
   AddCommand("ABORT", Abort);
   AddCommand("OPENCOMMANDSERVER", OpenCommandServer);
   AddCommand("CLOSECOMMANDSERVER", CloseCommandServer);
   AddCommand("ENABLEDEBUG", EnableDebug);
   AddCommand("HELP", DisplayHelp);

   /* This is the main loop of the program.  It gets user input from the*/
   /* command window, make a call to the command parser, and command    */
   /* interpreter.  After the function has been ran it then check the   */
   /* return value and displays an error message when appropriate. If   */
   /* the result returned is ever the EXIT_CODE the loop will exit      */
   /* leading the the exit of the program.                              */
   while(Result != EXIT_CODE)
   {
      /* Initialize the value of the variable used to store the users   */
      /* input and output "Input: " to the command window to inform the */
      /* user that another command may be entered.                      */
      UserInput[0] = '\0';

      /* Output an Input Shell-type prompt.                             */
      printf("Client>");

      /* Retrieve the command entered by the user and store it in the   */
      /* User Input Buffer.  Note that this command will fail if the    */
      /* application receives a signal which cause the standard file    */
      /* streams to be closed.  If this happens the loop will be broken */
      /* out of so the application can exit.                            */
      if(fgets(UserInput, sizeof(UserInput), stdin) != NULL)
      {
         /* Start a newline for the results.                            */
         printf("\r\n");

         /* Next, check to see if a command was input by the user.      */
         if(strlen(UserInput))
         {
            /* The string input by the user contains a value, now run   */
            /* the string through the Command Parser.                   */
            if(CommandParser(&TempCommand, UserInput) >= 0)
            {
               /* The Command was successfully parsed, run the Command. */
               Result = CommandInterpreter(&TempCommand);

               switch(Result)
               {
                  case INVALID_COMMAND_ERROR:
                     printf("Invalid Command.\r\n");
                     break;
                  case FUNCTION_ERROR:
                     printf("Function Error.\r\n");
                     break;
                  case EXIT_CODE:
                     /* If the user has request to exit we might as well*/
                     /* go ahead an close any Remote Server that we have*/
                     /* open.                                           */
                     if(SYNCID)
                        CloseConnection(NULL);
                     break;
               }
            }
            else
               printf("Invalid Input.\r\n");
         }
      }
      else
      {
         /* Close any Remote Server that we have open.                  */
         if(SYNCID)
            CloseConnection(NULL);

         Result = EXIT_CODE;
      }
   }
}

   /* This function is responsible for taking the input from the user   */
   /* and dispatching the appropriate Command Function.  First, this    */
   /* function retrieves a String of user input, parses the user input  */
   /* into Command and Parameters, and finally executes the Command or  */
   /* Displays an Error Message if the input is not a valid Command.    */
static void UserInterface_Server(void)
{
   UserCommand_t TempCommand;
   int           Result = !EXIT_CODE;
   char          UserInput[MAX_COMMAND_LENGTH];

   /* First let's make sure that we start on new line.                  */
   printf("\r\n");

   /* Next display the available commands.                              */
   DisplayHelp(NULL);

   ClearCommands();

   AddCommand("INQUIRY", Inquiry);
   AddCommand("DISPLAYINQUIRYLIST", DisplayInquiryList);
   AddCommand("PAIR", Pair);
   AddCommand("ENDPAIRING", EndPairing);
   AddCommand("PINCODERESPONSE", PINCodeResponse);
   AddCommand("PASSKEYRESPONSE", PassKeyResponse);
   AddCommand("USERCONFIRMATIONRESPONSE", UserConfirmationResponse);
   AddCommand("SETDISCOVERABILITYMODE", SetDiscoverabilityMode);
   AddCommand("SETCONNECTABILITYMODE", SetConnectabilityMode);
   AddCommand("SETPAIRABILITYMODE", SetPairabilityMode);
   AddCommand("CHANGESIMPLEPAIRINGPARAMETERS", ChangeSimplePairingParameters);
   AddCommand("GETLOCALADDRESS", GetLocalAddress);
   AddCommand("SETLOCALNAME", SetLocalName);
   AddCommand("GETLOCALNAME", GetLocalName);
   AddCommand("SETCLASSOFDEVICE", SetClassOfDevice);
   AddCommand("GETCLASSOFDEVICE", GetClassOfDevice);
   AddCommand("GETREMOTENAME", GetRemoteName);
   AddCommand("SERVICEDISCOVERY", ServiceDiscovery);
   AddCommand("OPEN", OpenServer);
   AddCommand("CLOSE", CloseServer);
   AddCommand("DISCONNECT", CloseConnection);
   AddCommand("ENABLEDEBUG", EnableDebug);
   AddCommand("HELP", DisplayHelp);

   /* This is the main loop of the program.  It gets user input from the*/
   /* command window, make a call to the command parser, and command    */
   /* interpreter.  After the function has been ran it then check the   */
   /* return value and displays an error message when appropriate. If   */
   /* the result returned is ever the EXIT_CODE the loop will exit      */
   /* leading the the exit of the program.                              */
   while(Result != EXIT_CODE)
   {
      /* Initialize the value of the variable used to store the users   */
      /* input and output "Input: " to the command window to inform the */
      /* user that another command may be entered.                      */
      UserInput[0] = '\0';

      /* Output an Input Shell-type prompt.                             */
      printf("Server>");

      /* Retrieve the command entered by the user and store it in the   */
      /* User Input Buffer.  Note that this command will fail if the    */
      /* application receives a signal which cause the standard file    */
      /* streams to be closed.  If this happens the loop will be broken */
      /* out of so the application can exit.                            */
      if(fgets(UserInput, sizeof(UserInput), stdin) != NULL)
      {
         /* Start a newline for the results.                            */
         printf("\r\n");

         /* Next, check to see if a command was input by the user.      */
         if(strlen(UserInput))
         {
            /* The string input by the user contains a value, now run   */
            /* the string through the Command Parser.                   */
            if(CommandParser(&TempCommand, UserInput) >= 0)
            {
               /* The Command was successfully parsed, run the Command. */
               Result = CommandInterpreter(&TempCommand);

               switch(Result)
               {
                  case INVALID_COMMAND_ERROR:
                     printf("Invalid Command.\r\n");
                     break;
                  case FUNCTION_ERROR:
                     printf("Function Error.\r\n");
                     break;
                  case EXIT_CODE:
                     /* If the user has request to exit we might as well*/
                     /* go ahead an close any Server that we have open. */
                     if(SYNCID)
                        CloseServer(NULL);
                     break;
               }
            }
            else
               printf("Invalid Input.\r\n");
         }
      }
      else
      {
         /* Close any Server that we have open.                         */
         if(SYNCID)
            CloseServer(NULL);

         Result = EXIT_CODE;
      }
   }
}

   /* The following function is responsible for converting number       */
   /* strings to there unsigned integer equivalent.  This function can  */
   /* handle leading and tailing white space, however it does not handle*/
   /* signed or comma delimited values.  This function takes as its     */
   /* input the string which is to be converted.  The function returns  */
   /* zero if an error occurs otherwise it returns the value parsed from*/
   /* the string passed as the input parameter.                         */
static unsigned int StringToUnsignedInteger(char *StringInteger)
{
   int          IsHex;
   unsigned int Index;
   unsigned int ret_val = 0;

   /* Before proceeding make sure that the parameter that was passed as */
   /* an input appears to be at least semi-valid.                       */
   if((StringInteger) && (strlen(StringInteger)))
   {
      /* Initialize the variable.                                       */
      Index = 0;

      /* Next check to see if this is a hexadecimal number.             */
      if(strlen(StringInteger) > 2)
      {
         if((StringInteger[0] == '0') && ((StringInteger[1] == 'x') || (StringInteger[1] == 'X')))
         {
            IsHex = 1;

            /* Increment the String passed the Hexadecimal prefix.      */
            StringInteger += 2;
         }
         else
            IsHex = 0;
      }
      else
         IsHex = 0;

      /* Process the value differently depending on whether or not a    */
      /* Hexadecimal Number has been specified.                         */
      if(!IsHex)
      {
         /* Decimal Number has been specified.                          */
         while(1)
         {
            /* First check to make sure that this is a valid decimal    */
            /* digit.                                                   */
            if((StringInteger[Index] >= '0') && (StringInteger[Index] <= '9'))
            {
               /* This is a valid digit, add it to the value being      */
               /* built.                                                */
               ret_val += (StringInteger[Index] & 0xF);

               /* Determine if the next digit is valid.                 */
               if(((Index + 1) < strlen(StringInteger)) && (StringInteger[Index+1] >= '0') && (StringInteger[Index+1] <= '9'))
               {
                  /* The next digit is valid so multiply the current    */
                  /* return value by 10.                                */
                  ret_val *= 10;
               }
               else
               {
                  /* The next value is invalid so break out of the loop.*/
                  break;
               }
            }

            Index++;
         }
      }
      else
      {
         /* Hexadecimal Number has been specified.                      */
         while(1)
         {
            /* First check to make sure that this is a valid Hexadecimal*/
            /* digit.                                                   */
            if(((StringInteger[Index] >= '0') && (StringInteger[Index] <= '9')) || ((StringInteger[Index] >= 'a') && (StringInteger[Index] <= 'f')) || ((StringInteger[Index] >= 'A') && (StringInteger[Index] <= 'F')))
            {
               /* This is a valid digit, add it to the value being      */
               /* built.                                                */
               if((StringInteger[Index] >= '0') && (StringInteger[Index] <= '9'))
                  ret_val += (StringInteger[Index] & 0xF);
               else
               {
                  if((StringInteger[Index] >= 'a') && (StringInteger[Index] <= 'f'))
                     ret_val += (StringInteger[Index] - 'a' + 10);
                  else
                     ret_val += (StringInteger[Index] - 'A' + 10);
               }

               /* Determine if the next digit is valid.                 */
               if(((Index + 1) < strlen(StringInteger)) && (((StringInteger[Index+1] >= '0') && (StringInteger[Index+1] <= '9')) || ((StringInteger[Index+1] >= 'a') && (StringInteger[Index+1] <= 'f')) || ((StringInteger[Index+1] >= 'A') && (StringInteger[Index+1] <= 'F'))))
               {
                  /* The next digit is valid so multiply the current    */
                  /* return value by 16.                                */
                  ret_val *= 16;
               }
               else
               {
                  /* The next value is invalid so break out of the loop.*/
                  break;
               }
            }

            Index++;
         }
      }
   }

   return(ret_val);
}

   /* The following function is responsible for parsing strings into    */
   /* components.  The first parameter of this function is a pointer to */
   /* the String to be parsed.  This function will return the start of  */
   /* the string upon success and a NULL pointer on all errors.         */
static char *StringParser(char *String)
{
   int   Index;
   char *ret_val = NULL;

   /* Before proceeding make sure that the string passed in appears to  */
   /* be at least semi-valid.                                           */
   if((String) && (strlen(String)))
   {
      /* The string appears to be at least semi-valid.  Search for the  */
      /* first space character and replace it with a NULL terminating   */
      /* character.                                                     */
      for(Index=0, ret_val=String;Index < strlen(String);Index++)
      {
         /* Is this the space character.                                */
         if((String[Index] == ' ') || (String[Index] == '\r') || (String[Index] == '\n'))
         {
            /* This is the space character, replace it with a NULL      */
            /* terminating character and set the return value to the    */
            /* begining character of the string.                        */
            String[Index] = '\0';
            break;
         }
      }
   }

   return(ret_val);
}

   /* This function is responsable for taking command strings and       */
   /* parsing them into a command, param1, and param2.  After parsing   */
   /* this string the data is stored into a UserCommand_t structure to  */
   /* be used by the interpreter.  The first parameter of this function */
   /* is the structure used to pass the parsed command string out of the*/
   /* function.  The second parameter of this function is the string    */
   /* that is parsed into the UserCommand structure.  Successful        */
   /* execution of this function is denoted by a retrun value of zero.  */
   /* Negative return values denote an error in the parsing of the      */
   /* string parameter.                                                 */
static int CommandParser(UserCommand_t *TempCommand, char *UserInput)
{
   int            ret_val;
   int            StringLength;
   char          *LastParameter;
   unsigned int   Count         = 0;

   /* Before proceeding make sure that the passed parameters appear to  */
   /* be at least semi-valid.                                           */
   if((TempCommand) && (UserInput) && (strlen(UserInput)))
   {
      /* First get the initial string length.                           */
      StringLength = strlen(UserInput);

      /* Retrieve the first token in the string, this should be the     */
      /* commmand.                                                      */
      TempCommand->Command = StringParser(UserInput);

      /* Flag that there are NO Parameters for this Command Parse.      */
      TempCommand->Parameters.NumberofParameters = 0;

       /* Check to see if there is a Command                            */
      if(TempCommand->Command)
      {
         /* Initialize the return value to zero to indicate success on  */
         /* commands with no parameters.                                */
         ret_val    = 0;

         /* Adjust the UserInput pointer and StringLength to remove the */
         /* Command from the data passed in before parsing the          */
         /* parameters.                                                 */
         UserInput    += strlen(TempCommand->Command)+1;
         StringLength  = strlen(UserInput);

         /* There was an available command, now parse out the parameters*/
         while((StringLength > 0) && ((LastParameter = StringParser(UserInput)) != NULL))
         {
            /* There is an available parameter, now check to see if     */
            /* there is room in the UserCommand to store the parameter  */
            if(Count < (sizeof(TempCommand->Parameters.Params)/sizeof(Parameter_t)))
            {
               /* Save the parameter as a string.                       */
               TempCommand->Parameters.Params[Count].strParam = LastParameter;

               /* Save the parameter as an unsigned int intParam will   */
               /* have a value of zero if an error has occurred.        */
               TempCommand->Parameters.Params[Count].intParam = StringToUnsignedInteger(LastParameter);

               Count++;
               UserInput    += strlen(LastParameter)+1;
               StringLength -= strlen(LastParameter)+1;

               ret_val = 0;
            }
            else
            {
               /* Be sure we exit out of the Loop.                      */
               StringLength = 0;

               ret_val      = TO_MANY_PARAMS;
            }
         }

         /* Set the number of parameters in the User Command to the     */
         /* number of found parameters                                  */
         TempCommand->Parameters.NumberofParameters = Count;
      }
      else
      {
         /* No command was specified                                    */
         ret_val = NO_COMMAND_ERROR;
      }
   }
   else
   {
      /* One or more of the passed parameters appear to be invalid.     */
      ret_val = INVALID_PARAMETERS_ERROR;
   }

   return(ret_val);
}

   /* This function is responsible for determining the command in which */
   /* the user entered and running the appropriate function associated  */
   /* with that command.  The first parameter of this function is a     */
   /* structure containing information about the commmand to be issued. */
   /* This information includes the command name and multiple parameters*/
   /* which maybe be passed to the function to be executed.  Successful */
   /* execution of this function is denoted by a return value of zero.  */
   /* A negative return value implies that that command was not found   */
   /* and is invalid.                                                   */
static int CommandInterpreter(UserCommand_t *TempCommand)
{
   int               i;
   int               ret_val;
   CommandFunction_t CommandFunction;

   /* If the command is not found in the table return with an invaild   */
   /* command error                                                     */
   ret_val = INVALID_COMMAND_ERROR;

   /* Let's make sure that the data passed to us appears semi-valid.    */
   if((TempCommand) && (TempCommand->Command))
   {
      /* Now, let's make the Command string all upper case so that we   */
      /* compare against it.                                            */
      for(i=0;i<strlen(TempCommand->Command);i++)
      {
         if((TempCommand->Command[i] >= 'a') && (TempCommand->Command[i] <= 'z'))
            TempCommand->Command[i] -= ('a' - 'A');
      }

      /* Check to see if the command which was entered was exit.        */
      if(memcmp(TempCommand->Command, "QUIT", strlen("QUIT")) != 0)
      {
         /* The command entered is not exit so search for command in    */
         /* table.                                                      */
         if((CommandFunction = FindCommand(TempCommand->Command)) != NULL)
         {
            /* The command was found in the table so call the command.  */
            if(!((*CommandFunction)(&TempCommand->Parameters)))
            {
               /* Return success to the caller.                         */
               ret_val = 0;
            }
            else
               ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* The command entered is exit, set return value to EXIT_CODE  */
         /* and return.                                                 */
         ret_val = EXIT_CODE;
      }
   }
   else
      ret_val = INVALID_PARAMETERS_ERROR;

   return(ret_val);
}

   /* The following function is provided to allow a means to            */
   /* programatically add Commands the Global (to this module) Command  */
   /* Table.  The Command Table is simply a mapping of Command Name     */
   /* (NULL terminated ASCII string) to a command function.  This       */
   /* function returns zero if successful, or a non-zero value if the   */
   /* command could not be added to the list.                           */
static int AddCommand(char *CommandName, CommandFunction_t CommandFunction)
{
   int ret_val;

   /* First, make sure that the parameters passed to us appear to be    */
   /* semi-valid.                                                       */
   if((CommandName) && (CommandFunction))
   {
      /* Next, make sure that we still have room in the Command Table   */
      /* to add commands.                                               */
      if(NumberCommands < MAX_SUPPORTED_COMMANDS)
      {
         /* Simply add the command data to the command table and        */
         /* increment the number of supported commands.                 */
         CommandTable[NumberCommands].CommandName       = CommandName;
         CommandTable[NumberCommands++].CommandFunction = CommandFunction;

         /* Return success to the caller.                               */
         ret_val                                        = 0;
      }
      else
         ret_val = 1;
   }
   else
      ret_val = 1;

   return(ret_val);
}

   /* The following function searches the Command Table for the         */
   /* specified Command.  If the Command is found, this function returns*/
   /* a NON-NULL Command Function Pointer.  If the command is not found */
   /* this function returns NULL.                                       */
static CommandFunction_t FindCommand(char *Command)
{
   unsigned int      Index;
   CommandFunction_t ret_val;

   /* First, make sure that the command specified is semi-valid.        */
   if(Command)
   {
      /* Now loop through each element in the table to see if there is  */
      /* a match.                                                       */
      for(Index=0,ret_val=NULL;((Index<NumberCommands) && (!ret_val));Index++)
      {
         if(memcmp(Command, CommandTable[Index].CommandName, strlen(CommandTable[Index].CommandName)) == 0)
            ret_val = CommandTable[Index].CommandFunction;
      }
   }
   else
      ret_val = NULL;

   return(ret_val);
}

   /* The following function is provided to allow a means to clear out  */
   /* all available commands from the command table.                    */
static void ClearCommands(void)
{
   /* Simply flag that there are no commands present in the table.      */
   NumberCommands = 0;
}

   /* The following function is a utility function that exists to read  */
   /* the specified file's data into a buffer that will pointed to by   */
   /* the global variable 'CurrentBuffer'.                              */
static int ReadFileIntoCurrentBuffer(char *FileName)
{
   int         ret_val;
   int         FileDescriptor;
   struct stat FileStat;

   if((FileName) && (strlen(FileName)))
   {
      /* Filename appears to be semi-valid, go ahead and attempt to open*/
      /* it for reading.                                                */
      if((FileDescriptor = open(FileName, (O_NONBLOCK | O_RDONLY), 0)) != (-1))
      {
         /* File opened, go ahead and determine how large it is.        */
         if(!lstat(FileName, &FileStat))
         {
            if(FileStat.st_size)
            {
               if((CurrentBuffer = malloc(FileStat.st_size)) != NULL)
               {
                  CurrentBufferSize = FileStat.st_size;

                  /* Next, attempt to read the data into the file.      */
                  if(read(FileDescriptor, CurrentBuffer, CurrentBufferSize) >= 0)
                     ret_val = 0;
                  else
                  {
                     free(CurrentBuffer);

                     CurrentBuffer     = NULL;
                     CurrentBufferSize = 0;

                     ret_val           = -5;
                  }
               }
               else
               {
                  ret_val           = -4;
                  CurrentBufferSize = 0;
               }
            }
            else
            {
               CurrentBuffer     = 0;
               CurrentBufferSize = 0;

               ret_val           = 0;
            }
         }
         else
            ret_val = -3;

         close(FileDescriptor);
      }
      else
         ret_val = -2;
   }
   else
      ret_val = -1;

   return(ret_val);
}

   /* The following function is a utility function that exists to write */
   /* the specified data to the file specified by the contents of the   */
   /* global variable 'CurrentFileName'.                                */
static void WriteReceivedData(unsigned int BufferLength, unsigned char *Buffer)
{
   int     FileDescriptor;
   DWord_t BytesWritten;

   if((BufferLength) && (Buffer) && (CurrentFileName[0]))
   {
      /* File Name built, attempt to open it.                           */
      FileDescriptor = open(CurrentFileName, (O_NONBLOCK | O_APPEND | O_CREAT | O_WRONLY), (S_IRWXG | S_IRWXU | S_IRWXO));

      if(FileDescriptor)
      {
         /* Seek to the end of the file.                                */
         lseek(FileDescriptor, 0, SEEK_END);

         /* Write the specified data to the file.                       */
         if((BytesWritten = write(FileDescriptor, Buffer, BufferLength)) == (DWord_t)(-1))
            BytesWritten = 0;

         /* Close the file, we are finished with it.                    */
         close(FileDescriptor);
      }
      else
         BytesWritten = 0;
   }
   else
      BytesWritten = 0;

   printf("%lu bytes written to file: %s\r\n", BytesWritten, CurrentFileName);
}

   /* The following function is a utility function that exists to format*/
   /* the specified IrMC Time Stamp (second parameter) into a human     */
   /* readable ASCII text string.  This function returns a positive,    */
   /* non-zero, value if successful (which represents the length of the */
   /* string), or zero if unsuccessful.                                 */
static int TimestampToString(char *Buffer, IrMC_TimeDate_t *TimeDate)
{
   int ret_val;

   if(Buffer)
      ret_val = sprintf(Buffer, "%02d/%02d/%04d %02d:%02d:%02d (%d)", TimeDate->Month, TimeDate->Day, TimeDate->Year, TimeDate->Hour, TimeDate->Minute, TimeDate->Second, TimeDate->UTC_Time);
   else
      ret_val = 0;

   return(ret_val);
}

   /* The following function is a utility function that exists to       */
   /* convert the specified ASCII string Timestamp (first parameter)    */
   /* into an IrMC Time/Date structure.  This function returns zero if  */
   /* successful, or a negative return value if there was an error.     */
static int TimestampToTimeDate(char *Buffer, IrMC_TimeDate_t *TimeDate)
{
   int ret_val;
   int Value;

   if((Buffer) && (TimeDate))
   {
      /* Since we will allow the use to provide a pointer to may not be */
      /* positioned at the beginning of the string, we will start by    */
      /* scanning past all characters that are not a valid ASCII digit. */
      while((*Buffer) && ((*Buffer < '0') || (*Buffer > '9')))
         Buffer++;

      /* The first item to be extracted is the Year.  This is a 4 digit */
      /* value.  If an error occurs trying to extract the value, exit   */
      /* with an error.                                                 */
      if((Value = ExtractInt(Buffer, 4)) >= 0)
      {
         /* Adjust the pointer past the 4 characters we have just       */
         /* extracted and save the Year value in the Structure.         */
         Buffer         += 4;
         TimeDate->Year  = (Word_t)Value;

         /* Extract the 2 digit Month.  If an error occurs, exit with   */
         /* and error code.                                             */
         if((Value = ExtractInt(Buffer, 2)) >= 0)
         {
            /* Adjust the pointer past the 2 characters that we have    */
            /* just extracted and save the value in the structure.      */
            Buffer          += 2;
            TimeDate->Month  = (Word_t)Value;

            /* Extract the 2 digit Day.  If an error occurs, exit with  */
            /* and error code.                                          */
            if((Value = ExtractInt(Buffer, 2)) >= 0)
            {
               /* Adjust the pointer past the 2 characters that we have */
               /* just extracted and save the value in the structure.   */
               Buffer        += 2;
               TimeDate->Day  = (Word_t)Value;

               /* For a consistency check, make sure that the next      */
               /* character is a T.  This is a separator between the    */
               /* Date and Time.                                        */
               if(*Buffer == 'T')
               {
                  /* Adjust the pointer past the separator.             */
                  Buffer += 1;

                  /* Extract the 2 digit Hour.  If an error occurs, exit*/
                  /* with and error code.                               */
                  if((Value = ExtractInt(Buffer, 2)) >= 0)
                  {
                     /* Adjust the pointer past the 2 characters that we*/
                     /* have just extracted and save the value in the   */
                     /* structure.                                      */
                     Buffer         += 2;
                     TimeDate->Hour  = (Word_t)Value;

                     /* Extract the 2 digit Minute.  If an error occurs,*/
                     /* exit with and error code.                       */
                     if((Value = ExtractInt(Buffer, 2)) >= 0)
                     {
                        /* Adjust the pointer past the 2 characters that*/
                        /* we have just extracted and save the value in */
                        /* the structure.                               */
                        Buffer           += 2;
                        TimeDate->Minute  = (Word_t)Value;

                        /* Extract the 2 digit Minute.  If an error     */
                        /* occurs, exit with and error code.            */
                        if((Value = ExtractInt(Buffer, 2)) >= 0)
                        {
                           /* Adjust the pointer past the 2 characters  */
                           /* that we have just extracted and save the  */
                           /* value in the structure.                   */
                           Buffer             += 2;
                           TimeDate->Second    = (Word_t)Value;

                           /* The last character may contain a Z to     */
                           /* denote UTC formatted time.                */
                           TimeDate->UTC_Time  = (Boolean_t)((*Buffer == 'Z')?TRUE:FALSE);

                           /* No errors were detected, return zero to   */
                           /* denote success.                           */
                           ret_val             = 0;
                        }
                        else
                           ret_val = -8;
                     }
                     else
                        ret_val = -7;
                  }
                  else
                     ret_val = -6;
               }
               else
                  ret_val = -5;
            }
            else
               ret_val = -4;
         }
         else
            ret_val = -3;
      }
      else
         ret_val = -2;
   }
   else
      ret_val = -1;

   return(ret_val);
}

   /* The following function is a utility function that exists to       */
   /* convert the current time/date into an IrMC Time/Date formatted    */
   /* time/date.                                                        */
static void PopulateCurrentTimeDate(IrMC_TimeDate_t *TimeDate)
{
   time_t     Time;
   struct tm *GMTime;

   if(TimeDate)
   {
      Time               = time(NULL);
      GMTime             = gmtime(&Time);

      TimeDate->Day      = (Word_t)(GMTime->tm_mday);
      TimeDate->Month    = (Word_t)(GMTime->tm_mon + 1);
      TimeDate->Year     = (Word_t)(GMTime->tm_year + 1900);
      TimeDate->Hour     = (Word_t)(GMTime->tm_hour);
      TimeDate->Minute   = (Word_t)(GMTime->tm_min);
      TimeDate->Second   = (Word_t)(GMTime->tm_sec);
      TimeDate->UTC_Time = TRUE;
   }
}

   /* The following function is a utililty function that exists to      */
   /* convert the specified number of ASCII digits into an integer      */
   /* (based on the specified input string).  This function returns the */
   /* value of the converted string.                                    */
static int ExtractInt(char *Ptr, int NumberOfDigits)
{
   int Value = 0;

   /* Stop if we reach a NULL or the Number of Digits have been         */
   /* extracted.                                                        */
   while((*Ptr) && (NumberOfDigits))
   {
      /* Check to make sure that the current character is a valid digit */
      /* of 0 through 9.                                                */
      if((*Ptr >= '0') && (*Ptr <= '9'))
         Value = (Value * 10) + (*Ptr & 0x0F);
      else
      {
         /* Exit on Error.                                              */
         break;
      }

      /* Advance to the next character in the string.                   */
      Ptr++;

      NumberOfDigits--;
   }

   /* Check to see if an error occurred while parsing the string.  If   */
   /* the Number of Digits is greater then Zero, then we stopped early  */
   /* due to some error condition.                                      */
   if(NumberOfDigits)
      Value = (-1);

   return(Value);
}

   /* The following function is a utility function that exists to       */
   /* determine if the specified string represents a valid Object Store */
   /* Path.  'Valid' in this case means that the directory exists.  This*/
   /* function returns TRUE if the path exists, or FALSE if the path    */
   /* does not.                                                         */
static Boolean_t VerifyObjectStorePath(char *StorePath)
{
   DIR       *DirectoryHandle;
   Boolean_t  ret_val;

   /* First, make sure that the input parameter appears to be           */
   /* semi-valid.                                                       */
   if((StorePath) && (strlen(StorePath)))
   {
      if((DirectoryHandle = opendir(StorePath)) != NULL)
      {
         ret_val = TRUE;

         closedir(DirectoryHandle);
      }
      else
         ret_val = FALSE;
   }
   else
      ret_val = FALSE;

   return(ret_val);
}

   /* The following function is responsible for converting data of type */
   /* BD_ADDR to a string.  The first parameter of this function is the */
   /* BD_ADDR to be converted to a string.  The second parameter of this*/
   /* function is a pointer to the string in which the converted BD_ADDR*/
   /* is to be stored.                                                  */
static void BD_ADDRToStr(BD_ADDR_t Board_Address, char *BoardStr)
{
   sprintf(BoardStr, "%02X%02X%02X%02X%02X%02X", Board_Address.BD_ADDR5,
                                                 Board_Address.BD_ADDR4,
                                                 Board_Address.BD_ADDR3,
                                                 Board_Address.BD_ADDR2,
                                                 Board_Address.BD_ADDR1,
                                                 Board_Address.BD_ADDR0);
}

   /* WriteEIRInformation formats and writes extended inquiry response  */
   /* data.                                                             */
static void WriteEIRInformation(char *LocalName)
{
   int                               Index;
   int                               Length;
   Byte_t                            Status;
   SByte_t                           TxPowerEIR;
   Extended_Inquiry_Response_Data_t  EIRData;

   if((BluetoothStackID) && (HCI_Command_Supported(BluetoothStackID, HCI_SUPPORTED_COMMAND_WRITE_EXTENDED_INQUIRY_RESPONSE_BIT_NUMBER) > 0))
   {
      Index  = 0;

      /* Add the local name.                                            */
      if(LocalName)
      {
         Length = strlen(LocalName) + 1;

         EIRData.Extended_Inquiry_Response_Data[Index++] = Length;
         EIRData.Extended_Inquiry_Response_Data[Index++] = HCI_EXTENDED_INQUIRY_RESPONSE_DATA_TYPE_LOCAL_NAME_COMPLETE;
         BTPS_MemCopy(&(EIRData.Extended_Inquiry_Response_Data[Index]), LocalName, Length);

         Index += Length;
      }

      /* Read the transmit power level.                                 */
      if((HCI_Read_Inquiry_Response_Transmit_Power_Level(BluetoothStackID, &Status, &TxPowerEIR)) && (Status == HCI_ERROR_CODE_NO_ERROR))
      {
         EIRData.Extended_Inquiry_Response_Data[Index++] = 2;
         EIRData.Extended_Inquiry_Response_Data[Index++] = HCI_EXTENDED_INQUIRY_RESPONSE_DATA_TYPE_TX_POWER_LEVEL;
         EIRData.Extended_Inquiry_Response_Data[Index++] = TxPowerEIR;
      }

      /* Write the EIR Data.                                            */
      if(Index)
        GAP_Write_Extended_Inquiry_Information(BluetoothStackID, HCI_EXTENDED_INQUIRY_RESPONSE_FEC_REQUIRED, &EIRData);
   }
}

   /* The following function is the Callback function that is registered*/
   /* with the Object Store to Read data from the specified Object.     */
static int ObjectStoreIORead(IrMC_Object_Store_Type_t Type, IrMC_Object_Entry_t *Entry, char *StoreData)
{
   int         ret_val = SYNC_OBEX_RESPONSE_OK;
   int         FileDescriptor;
   char        FilePath[512 + 1];
   long        AmountRead;
   struct stat FileStat;

   /* Notify the user of the read operation.                            */
   printf("ObjectStoreIORead Type: %d\r\n", Type);

   printf("LUID: %s\r\n", Entry->LUID);

   /* First we will determine if we can successfully open and read this */
   /* file.  Construct the full file path.                              */
   strcpy(FilePath, StoreData);

   /* Selected object stores use alternate paths for the root directory.*/
   switch(Type)
   {
      case osMsgIn:
         strcat(FilePath, "MsgIn/");
         break;
      case osMsgOut:
         strcat(FilePath, "MsgOut/");
         break;
      case osMsgSent:
         strcat(FilePath, "MsgSent/");
         break;
      case osInbox:
         strcat(FilePath, "Inbox/");
         break;
      default:
         break;
   }

   strcat(FilePath, Entry->LUID);

   printf("File Path: %s\r\n", FilePath);

   /* Attempt to open the file for reading.                             */
   if((FileDescriptor = open(FilePath, O_NONBLOCK | O_RDONLY, 0)) >= 0)
   {
      /* File open successful, now determine the data size so we can    */
      /* allocate an appropriate buffer.                                */
      if(!fstat(FileDescriptor, &FileStat))
      {
         Entry->DataLength   = FileStat.st_size;
         Entry->BufferLength = Entry->DataLength;

         /* Allocate a buffer equal to the data length to store the     */
         /* actual data.                                                */
         Entry->Data = BTPS_AllocateMemory(Entry->DataLength);

         /* Were we able to allocate buffer?                            */
         if(Entry->Data)
         {
            /* Buffer allocated successfully, now we will attempt to    */
            /* read the file data into the buffer.                      */
            AmountRead = read(FileDescriptor, Entry->Data, Entry->DataLength);

            if(AmountRead > 0)
            {
               if(AmountRead != Entry->DataLength)
               {
                  /* We did not successfully read the entire file       */
                  /* contents.                                          */
                  printf("Warning: Partial File Read.\r\n");

                  ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
               }
            }
            else
            {
               /* Error reading file data after open.                   */
               printf("Error Reading File Data.\r\n");

               ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
            }
         }
         else
         {
            /* Error allocating resources for data.                     */
            printf("Error Allocating Data Buffer.\r\n");

            ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
         }
      }
      else
      {
         /* Error allocating resources for data.                        */
         printf("Error Retrieving File Information.\r\n");

         ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
      }

      /* Close File Descriptor.                                         */
      close(FileDescriptor);
   }
   else
   {
      /* Error opening File for Read Operation.                         */
      printf("Error Opening File for Read.\r\n");

      ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
   }

   return (ret_val);
}

   /* The following function is the Callback function that is registered*/
   /* with the Object Store to Write data to the specified Object.      */
static int ObjectStoreIOWrite(IrMC_Object_Store_Type_t Type, IrMC_Object_Entry_t *Entry, DWord_t BufferSize, const Byte_t *Buffer, char *StoreData)
{
   int  ret_val = SYNC_OBEX_RESPONSE_OK;
   int  FileDescriptor;
   char FilePath[512 + 1];
   long lenWritten;

   /* Notify the user of the write operation.                           */
   printf("ObjectStoreIOWrite Type: %d\r\n", Type);

   printf("LUID: %s\r\n", Entry->LUID);

   /* First we will determine if we can successfully open and read this */
   /* file.  Construct the full file path.                              */
   strcpy(FilePath, StoreData);

   /* Selected object stores use alternate paths for the root directory.*/
   switch(Type)
   {
      case osMsgIn:
         strcat(FilePath, "MsgIn/");
         break;
      case osMsgOut:
         strcat(FilePath, "MsgOut/");
         break;
      case osMsgSent:
         strcat(FilePath, "MsgSent/");
         break;
      case osInbox:
         strcat(FilePath, "Inbox/");
         break;
      default:
         break;
   }

   strcat(FilePath, Entry->LUID);

   printf("File Path: %s\r\n", FilePath);

   /* Attempt to open the file for writing (overwrite).                 */
   if((FileDescriptor = open(FilePath, (O_NONBLOCK | O_APPEND | O_CREAT | O_WRONLY), (S_IRWXG | S_IRWXU | S_IRWXO))) >= 0)
   {
      if(!((lenWritten = write(FileDescriptor, Buffer, BufferSize)) & 0x80000000))
      {
         if(lenWritten != BufferSize)
         {
            /* We did not successfully write the entire file contents.  */
            printf("Warning: Partial File Write.\r\n");

            ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
         }
         else
            printf("Write Successful.\r\n");
      }
      else
      {
         /* File Write Failed                                           */
         printf("Error Writing File Data.\r\n");

         ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
      }

      /* Close File Descriptor.                                         */
      close(FileDescriptor);
   }
   else
   {
      /* Error opening File for Write Operation.                        */
      printf("Error Opening File for Write.\r\n");

      ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
   }

   return (ret_val);
}

   /* The following function is the Callback function that is registered*/
   /* with the Object Store to Create the specified Object.             */
static int ObjectStoreIOCreate(IrMC_Object_Store_Type_t Type, IrMC_Object_Entry_t *Entry, DWord_t BufferSize, const Byte_t *Buffer, char *StoreData)
{
   int  ret_val = SYNC_OBEX_RESPONSE_OK;
   int  FileDescriptor;
   char FilePath[512 + 1];
   long lenWritten;

   /* Notify the user of the create operation.                          */
   printf("ObjectStoreIOCreate Type: %d\r\n", Type);

   printf("LUID: %s\r\n", Entry->LUID);

   /* First we will determine if we can successfully open and read this */
   /* file.  Construct the full file path.                              */
   strcpy(FilePath, StoreData);

   /* Selected object stores use alternate paths for the root directory.*/
   switch(Type)
   {
      case osMsgIn:
         strcat(FilePath, "MsgIn/");
         break;
      case osMsgOut:
         strcat(FilePath, "MsgOut/");
         break;
      case osMsgSent:
         strcat(FilePath, "MsgSent/");
         break;
      case osInbox:
         strcat(FilePath, "Inbox/");
         break;
      default:
         break;
   }

   if(strlen(Entry->LUID) == 0)
   {
      GenerateLUIDString(Type, Entry->LUID, sizeof(Entry->LUID));
   }

   strcat(FilePath, Entry->LUID);

   printf("File Path: %s\r\n", FilePath);

   if((FileDescriptor = open(FilePath, (O_NONBLOCK | O_TRUNC | O_CREAT | O_WRONLY), (S_IRWXG | S_IRWXU | S_IRWXO))) >= 0)
   {
      if(!((lenWritten = write(FileDescriptor, Buffer, BufferSize)) & 0x80000000))
      {
         if(lenWritten != BufferSize)
         {
            /* We did not successfully write the entire file contents.  */
            printf("Warning: Partial File Write.\r\n");

            ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
         }
         else
            printf("Create Successful.\r\n");
      }
      else
      {
         /* File Write Failed                                           */
         printf("Error Writing File Data.\r\n");

         ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
      }

      /* Close File Descriptor.                                         */
      close(FileDescriptor);
   }
   else
   {
      /* Error opening File for Write Operation.                        */
      printf("Error Opening File for Write.\r\n");

      ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
   }

   return (ret_val);
}

   /* The following function is the Callback function that is registered*/
   /* with the Object Store to Delete the specified Object.             */
static int ObjectStoreIODelete(IrMC_Object_Store_Type_t Type, IrMC_Object_Entry_t *Entry, Boolean_t HardDelete, char *StoreData)
{
   int  ret_val = SYNC_OBEX_RESPONSE_OK;
   char FilePath[512 + 1];

   /* Notify the user of the delete operation.                          */
   printf("ObjectStoreIODelete Type: %d\r\n", Type);

   printf("LUID: %s\r\n", Entry->LUID);

   printf("HD: %c\r\n", ((HardDelete)?('Y'):('N')));

   /* First we will determine if we can successfully open and read this */
   /* file.  Construct the full file path.                              */
   strcpy(FilePath, StoreData);

   /* Selected object stores use alternate paths for the root directory.*/
   switch(Type)
   {
      case osMsgIn:
         strcat(FilePath, "MsgIn/");
         break;
      case osMsgOut:
         strcat(FilePath, "MsgOut/");
         break;
      case osMsgSent:
         strcat(FilePath, "MsgSent/");
         break;
      case osInbox:
         strcat(FilePath, "Inbox/");
         break;
      default:
         break;
   }

   strcat(FilePath, Entry->LUID);

   printf("File Path: %s\r\n", FilePath);

   /* Attempt to delete the file.                                       */
   if(!unlink(FilePath))
      printf("Delete Successful.\r\n");
   else
   {
      /* Error deleting File.                                           */
      printf("Error Deleting File.\r\n");

      ret_val = SYNC_OBEX_RESPONSE_INTERNAL_ERROR;
   }

   return (ret_val);
}

   /* The following function is the Callback function that is registered*/
   /* with the Object Store to Populate the Object Store Database.      */
static int ObjectStoreIOPopulate(IrMC_Object_Store_t *Store, char *StoreData)
{
   int                  ret_val;
   DIR                 *DirectoryHandle;
   char                 FileSearchString[16];
   char                 FileSearchPath[512 + 16];
   struct tm           *Time;
   struct stat          FileStat;
   struct dirent       *DirectoryEntry;
   IrMC_Object_Entry_t  NewEntry;

   /* Notify the GUI client of the read operation.                      */
   printf("ObjectStoreIOPopulate Data: %s\r\n", StoreData);

   /* The StoreData string contains the root path of this object store  */
   /* to populate.                                                      */
   if((Store) && (StoreData) && (strlen(StoreData) < sizeof(FileSearchPath)))
   {
      /* First copy the data into the local buffer.                     */
      strcpy(FileSearchPath, StoreData);

      /* Next, add extension for search string based on the object store*/
      /* type                                                           */
      switch(Store->Type)
      {
         case osPhonebook:
            strcpy(FileSearchString, ".vcf");
            break;
         case osCalendar:
            strcpy(FileSearchString, ".vcs");
            break;
         case osNotes:
            strcpy(FileSearchString, ".vnt");
            break;
         case osMsgIn:
            strcat(FileSearchPath, "MsgIn");
            strcpy(FileSearchString, ".vmg");
            break;
         case osMsgOut:
            strcat(FileSearchPath, "MsgOut");
            strcpy(FileSearchString, ".vmg");
            break;
         case osMsgSent:
            strcat(FileSearchPath, "MsgSent");
            strcpy(FileSearchString, ".vmg");
            break;
         case osBookmark:
            strcpy(FileSearchString, ".vbm");
            break;
         case osInbox:
            strcat(FileSearchPath, "Inbox");
            FileSearchString[0] = '\0';
            break;
         default:
         case osUnknown:
            /* This case should be excluded by our check at function    */
            /* entry.                                                   */
            FileSearchString[0] = '\0';
            break;
      }

      /* Notify GUI client of the final file search path for            */
      /* verification.                                                  */
      printf("Searching For Files: %s\r\n", FileSearchPath);

      if((DirectoryHandle = opendir(FileSearchPath)) != NULL)
      {
         /* Loop through all files located by this search path.         */
         while((DirectoryEntry = readdir(DirectoryHandle)) != NULL)
         {
            /* Add this file to the object store (if applicable).       */
            if((!strlen(FileSearchPath)) || ((strlen(FileSearchPath)) && (strlen(DirectoryEntry->d_name) > strlen(FileSearchString)) && (!strcasecmp(FileSearchString, &(DirectoryEntry->d_name[strlen(DirectoryEntry->d_name) - strlen(FileSearchString)]))) && (strcmp(DirectoryEntry->d_name, ".")) && (strcmp(DirectoryEntry->d_name, ".."))))
            {
               /* Initialize a Object Entry with default values.        */
               memset(&NewEntry, 0, sizeof(IrMC_Object_Entry_t));

               /* Store file name as LUID of this object.               */
               strcpy(NewEntry.LUID, DirectoryEntry->d_name);

               /* Next, attempt to convert the file's last write time to*/
               /* a system time.                                        */

               /* Build the fully qualified Path File Name.             */
               strcpy(FileSearchPath, ObjectStorePath);
               strcat(FileSearchPath, DirectoryEntry->d_name);

               if(!lstat(FileSearchPath, &FileStat))
               {
                  /* Now we will map the system time object to the      */
                  /* IrMC_TimeDate_t structure of the object.           */

                  /* Convert the File Time to the System Time.          */
                  if((Time = gmtime((time_t *)&(FileStat.st_mtime))) != NULL)
                  {
                     NewEntry.Timestamp.Day      = Time->tm_mday;
                     NewEntry.Timestamp.Hour     = Time->tm_hour;
                     NewEntry.Timestamp.Minute   = Time->tm_min;
                     NewEntry.Timestamp.Month    = Time->tm_mon + 1;
                     NewEntry.Timestamp.Second   = Time->tm_sec;
                     NewEntry.Timestamp.Year     = Time->tm_year + 1900;
                     NewEntry.Timestamp.UTC_Time = TRUE;
                  }
               }

               /* Notify GUI Client of Add data.                        */
               printf("Adding File: %s\r\n", NewEntry.LUID);

               /* Now that we've completed creating our Object Entry we */
               /* will attempt to add it to the object store.  We will  */
               /* pass the resulting return value directly out to the   */
               /* caller.                                               */
               OBJSTORE_AddObjectEntry(Store, &NewEntry);
            }
         }

         /* We should have completed creating the store and populating  */
         /* with all matching objects.  We will now exit with a         */
         /* successful result code.                                     */
         ret_val = OBJSTORE_SUCCESS_RESULT;

         closedir(DirectoryHandle);
      }
      else
      {
         /* No files were found using the selected file search path.  In*/
         /* this case we will assume this is an empty object store and  */
         /* consider the operation a success.                           */
         ret_val = OBJSTORE_SUCCESS_RESULT;
      }
   }
   else
   {
      /* Store data is not present.  We cannot populate store.          */
      printf("Error No Path Present.\r\n");

      ret_val = OBJSTORE_ERROR_INVALID_PARAMETER;
   }

   return (ret_val);
}

   /* The following function registers and individual Object Store and  */
   /* Initializes the Store Storage and Context.  This function returns */
   /* zero if successful, or a negative return error code if there was  */
   /* an error.                                                         */
int RegisterObjectStore(IrMC_Object_Store_Type_t ObjectStoreType, IrMC_Information_Log_t *infolog, char *SerialNumber, char *StoreData, IrMC_Object_Store_IO_Interface_t *IOInterface)
{
   int ret_val;
   int Result;

   /* Check to ensure that we've passed in a valid info log pointer and */
   /* file path.                                                        */
   if(ObjectStoreType < NUMBER_SUPPORTED_OBJECT_STORES)
   {
      /* Now attempt to create the object store.  This will perform     */
      /* initial setup so that we can then populate the store with      */
      /* appropriate objects.                                           */
      Result = OBJSTORE_CreateStore(&(ObjectStore[ObjectStoreType]), ObjectStoreType, infolog, OBJSTORE_PERMISSION_MASK_ALL, SerialNumber, IOInterface, StoreData);

      /* Were we successful in creating the Object Store?               */
      if(!Result)
      {
         /* All tasks have been completed successfully.  Return success */
         /* code.                                                       */
         ret_val = OBJSTORE_SUCCESS_RESULT;
      }
      else
      {
         /* Error initializing Object Store.  Return error code.        */
         ret_val = OBJSTORE_ERROR_NOT_INITIALIZED;
      }
   }
   else
      ret_val = OBJSTORE_ERROR_INVALID_PARAMETER;

   /* Return result code.                                               */
   return(ret_val);
}

   /* The following function is responsible for opening the SS1         */
   /* Bluetooth Protocol Stack.  This function accepts a pre-populated  */
   /* HCI Driver Information structure that contains the HCI Driver     */
   /* Transport Information.  This function returns zero on successful  */
   /* execution and a negative value on all errors.                     */
static int OpenStack(HCI_DriverInformation_t *HCI_DriverInformation)
{
   int                        Result;
   int                        ret_val = 0;
   char                       BluetoothAddress[13];
   char                      *LocalName;
   Byte_t                     Status;
   BD_ADDR_t                  BD_ADDR;
   HCI_Version_t              HCIVersion;
   L2CA_Link_Connect_Params_t L2CA_Link_Connect_Params;

   /* First check to see if the Stack has already been opened.          */
   if(!BluetoothStackID)
   {
      /* Next, makes sure that the Driver Information passed appears to */
      /* be semi-valid.                                                 */
      if(HCI_DriverInformation)
      {
         /* Initialize the Stack                                        */
         Result = BSC_Initialize(HCI_DriverInformation, 0);

         /* Next, check the return value of the initialization to see if*/
         /* it was successful.                                          */
         if(Result > 0)
         {
            /* The Stack was initialized successfully, inform the user  */
            /* and set the return value of the initialization function  */
            /* to the Bluetooth Stack ID.                               */
            if(HCI_DriverInformation->DriverType == hdtUSB)
               printf("Stack Initialization on USB Successful.\r\n");
            else
               printf("Stack Initialization on Port %d %ld (%s) Successful.\r\n", HCI_DriverInformation->DriverInformation.COMMDriverInformation.COMPortNumber, HCI_DriverInformation->DriverInformation.COMMDriverInformation.BaudRate, ((HCI_DriverInformation->DriverInformation.COMMDriverInformation.Protocol == cpBCSP)?"BCSP":"UART"));

            BluetoothStackID = Result;

            DebugID          = 0;

            /* Initialize the default Secure Simple Pairing parameters. */
            IOCapability     = DEFAULT_IO_CAPABILITY;
            OOBSupport       = FALSE;
            MITMProtection   = DEFAULT_MITM_PROTECTION;

            if(!HCI_Version_Supported(BluetoothStackID, &HCIVersion))
               printf("Device Chipset Version: %s\r\n", (HCIVersion <= NUM_SUPPORTED_HCI_VERSIONS)?HCIVersionStrings[HCIVersion]:HCIVersionStrings[NUM_SUPPORTED_HCI_VERSIONS]);

            /* Let's output the Bluetooth Device Address so that the    */
            /* user knows what the Device Address is.                   */
            if(!GAP_Query_Local_BD_ADDR(BluetoothStackID, &BD_ADDR))
            {
               BD_ADDRToStr(BD_ADDR, BluetoothAddress);

               printf("Bluetooth Device Address: %s\r\n", BluetoothAddress);

               /* Set the local name and the EIR data.                  */
               if((LocalName = (char *)malloc((sizeof(LOCAL_NAME_ROOT) / sizeof(char)) + 5)) != NULL)
               {
                  snprintf(LocalName, ((sizeof(LOCAL_NAME_ROOT) / sizeof(char)) + 5),"%s_%02X%02X", LOCAL_NAME_ROOT, BD_ADDR.BD_ADDR1, BD_ADDR.BD_ADDR0);

                  if(!GAP_Set_Local_Device_Name(BluetoothStackID, LocalName))
                  {
                     /* Add the EIR data.                               */
                     WriteEIRInformation(LocalName);
                  }

                  free(LocalName);
               }
            }

            /* Go ahead and allow Master/Slave Role Switch.             */
            L2CA_Link_Connect_Params.L2CA_Link_Connect_Request_Config  = cqAllowRoleSwitch;
            L2CA_Link_Connect_Params.L2CA_Link_Connect_Response_Config = csMaintainCurrentRole;

            L2CA_Set_Link_Connection_Configuration(BluetoothStackID, &L2CA_Link_Connect_Params);

            if(HCI_Command_Supported(BluetoothStackID, HCI_SUPPORTED_COMMAND_WRITE_DEFAULT_LINK_POLICY_BIT_NUMBER) > 0)
               HCI_Write_Default_Link_Policy_Settings(BluetoothStackID, HCI_LINK_POLICY_SETTINGS_ENABLE_MASTER_SLAVE_SWITCH, &Status);

            /* Enable extended inquiry.                                 */
            if((Result = HCI_Command_Supported(BluetoothStackID, HCI_SUPPORTED_COMMAND_WRITE_INQUIRY_MODE_BIT_NUMBER)) > 0)
            {
               if((Result = GAP_Set_Inquiry_Mode(BluetoothStackID, imExtended)) < 0)
                  printf("GAP_Set_Inquiry_Mode failed - Result: %d\n", Result);
            }
            else
               printf("write inquiry mode not supported - Result: %d\n", Result);

            /* Delete all Stored Link Keys.                             */
            ASSIGN_BD_ADDR(BD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

            DeleteLinkKey(BD_ADDR);
         }
         else
         {
            /* The Stack was NOT initialized successfully, inform the   */
            /* user and set the return value of the initialization      */
            /* function to an error.                                    */
            if(HCI_DriverInformation->DriverType == hdtUSB)
               printf("Stack Initialization on USB Failed: %d.\r\n", Result);
            else
            {
               if(HCI_DriverInformation->DriverInformation.COMMDriverInformation.COMPortNumber == -1)
               {
                  printf("Stack Initialization using device file %s %ld (%s) Failed: %d.\r\n", HCI_DriverInformation->DriverInformation.COMMDriverInformation.COMDeviceName, HCI_DriverInformation->DriverInformation.COMMDriverInformation.BaudRate, ((HCI_DriverInformation->DriverInformation.COMMDriverInformation.Protocol == cpBCSP)?"BCSP":"UART"), Result);
               }
               else
               {
                  printf("Stack Initialization on Port %d %ld (%s) Failed: %d.\r\n", HCI_DriverInformation->DriverInformation.COMMDriverInformation.COMPortNumber, HCI_DriverInformation->DriverInformation.COMMDriverInformation.BaudRate, ((HCI_DriverInformation->DriverInformation.COMMDriverInformation.Protocol == cpBCSP)?"BCSP":"UART"), Result);
               }
            }

            BluetoothStackID = 0;

            ret_val          = UNABLE_TO_INITIALIZE_STACK;
         }
      }
      else
      {
         /* One or more of the necessary parameters are invalid.        */
         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* A valid Stack ID already exists, inform to user.               */
      printf("Stack Already Initialized.\r\n");
   }

   return(ret_val);
}

   /* The following function is responsible for closing the SS1         */
   /* Bluetooth Protocol Stack.  This function requires that the        */
   /* Bluetooth Protocol stack previously have been initialized via the */
   /* OpenStack() function.  This function returns zero on successful   */
   /* execution and a negative value on all errors.                     */
static int CloseStack(void)
{
   int ret_val;

   /* First check to see if the Stack has been opened.                  */
   if(BluetoothStackID)
   {
      /* Check to see if there is currently a Connection or Server Open.*/
      if(SYNCID)
      {
         /* There is currently a Server or Connection open, check to see*/
         /* if the Current Service Type is Client.                      */
         if(IsClient)
         {
            /* There is a Client Connection open, Close the Connection. */
            CloseConnection(NULL);
         }
         else
         {
            /* There is a Server open, Close the Server.                */
            CloseServer(NULL);
         }
      }

      /* If debugging is enabled, go ahead and clean it up.             */
      if(DebugID)
         BTPS_Debug_Cleanup(BluetoothStackID, DebugID);

      /* Simply close the Stack                                         */
      BSC_Shutdown(BluetoothStackID);

      printf("Stack Shutdown Successfully.\r\n");

      /* Flag that the Stack is no longer initialized.                  */
      BluetoothStackID      = 0;

      SYNCID                = 0;
      ServerSDPRecordHandle = 0;

      DebugID               = 0;

      /* Flag success to the caller.                                    */
      ret_val               = 0;
   }
   else
   {
      /* A valid Stack ID does not exist, inform to user.               */
      printf("Stack not Initialized.\r\n");

      ret_val = UNABLE_TO_INITIALIZE_STACK;
   }

   return(ret_val);
}

   /* The following function is responsible for placing the Local       */
   /* Bluetooth Device into General Discoverablity Mode.  Once in this  */
   /* mode the Device will respond to Inquiry Scans from other Bluetooth*/
   /* Devices.  This function requires that a valid Bluetooth Stack ID  */
   /* exists before running.  This function returns zero on successful  */
   /* execution and a negative value if an error occurred.              */
static int SetDisc(void)
{
   int ret_val;

   /* First, check that a valid Bluetooth Stack ID exists.              */
   if(BluetoothStackID)
   {
      /* A semi-valid Bluetooth Stack ID exists, now attempt to set the */
      /* attached Devices Discoverablity Mode to General.               */
      ret_val = GAP_Set_Discoverability_Mode(BluetoothStackID, dmGeneralDiscoverableMode, 0);

      /* Next, check the return value of the GAP Set Discoverability    */
      /* Mode command for successful execution.                         */
      if(!ret_val)
      {
         /* The command appears to have been successful.  The attached  */
         /* Device is now in General Discoverability Mode.              */
         printf("GAP_Set_Discoverability_Mode(dmGeneralDiscoverable, 0).\r\n");
      }
      else
      {
         /* An error occurred while trying to set the Discoverability   */
         /* Mode of the Device.                                         */
         printf("Set Discoverable Mode Command Error : %d.\r\n", ret_val);
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for placing the Local       */
   /* Bluetooth Device into Connectable Mode.  Once in this mode the    */
   /* Device will respond to Page Scans from other Bluetooth Devices.   */
   /* This function requires that a valid Bluetooth Stack ID exists     */
   /* before running.  This function returns zero on success and a      */
   /* negative value if an error occurred.                              */
static int SetConnect(void)
{
   int ret_val;

   /* First, check that a valid Bluetooth Stack ID exists.              */
   if(BluetoothStackID)
   {
      /* Attempt to set the attached Device to be Connectable.          */
      ret_val = GAP_Set_Connectability_Mode(BluetoothStackID, cmConnectableMode);

      /* Next, check the return value of the                            */
      /* GAP_Set_Connectability_Mode() function for successful          */
      /* execution.                                                     */
      if(!ret_val)
      {
         /* The command appears to have been successful.  The attached  */
         /* Device is now in Connectable Mode.                          */
         printf("GAP_Set_Connectability_Mode(cmConnectable).\r\n");
      }
      else
      {
         /* An error occurred while trying to make the Device           */
         /* Connectable.                                                */
         printf("Set Connectability Mode Command Error: %d.\r\n", ret_val);
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for placing the local       */
   /* Bluetooth device into Pairable mode.  Once in this mode the device*/
   /* will response to pairing requests from other Bluetooth devices.   */
   /* This function returns zero on successful execution and a negative */
   /* value on all errors.                                              */
static int SetPairable(void)
{
   int Result;
   int ret_val;

   /* First, check that a valid Bluetooth Stack ID exists.              */
   if(BluetoothStackID)
   {
      /* Attempt to set the attached device to be pairable.             */
      Result = GAP_Set_Pairability_Mode(BluetoothStackID, pmPairableMode);

      /* Next, check the return value of the GAP Set Pairability mode   */
      /* command for successful execution.                              */
      if(!Result)
      {
         /* The command appears to have been successful.  The attached  */
         /* device is now in pairable mode.                             */
         printf("GAP_Set_Pairability_Mode(pmPairableMode).\r\n");

         /* The device has been set to pairable mode, now register an   */
         /* Authentication Callback to handle the Authentication events */
         /* if required.                                                */
         Result = GAP_Register_Remote_Authentication(BluetoothStackID, GAP_Event_Callback, (unsigned long)0);

         /* Next, check the return value of the GAP Register Remote     */
         /* Authentication command for successful execution.            */
         if(!Result)
         {
            /* The command appears to have been successful.             */
            printf("GAP_Register_Remote_Authentication() Success.\r\n");

            ret_val = 0;
         }
         else
         {
            /* An error occurred while trying to execute this function. */
            printf("GAP_Register_Remote_Authentication() Failure: %d\r\n", Result);

            ret_val = Result;
         }
      }
      else
      {
         /* An error occurred while trying to make the device pairable. */
         printf("Set Pairability Mode Command Error: %d.\r\n", Result);

         ret_val = Result;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is a utility function that exists to delete*/
   /* the specified Link Key from the Local Bluetooth Device.  If a NULL*/
   /* Bluetooth Device Address is specified, then all Link Keys will be */
   /* deleted.                                                          */
static int DeleteLinkKey(BD_ADDR_t BD_ADDR)
{
   int       Result;
   Byte_t    Status_Result;
   Word_t    Num_Keys_Deleted = 0;
   BD_ADDR_t NULL_BD_ADDR;

   Result = HCI_Delete_Stored_Link_Key(BluetoothStackID, BD_ADDR, TRUE, &Status_Result, &Num_Keys_Deleted);
   if(Result)
      printf("Deleting Stored Link Key(s) FAILED!\r\n");

   /* Any stored link keys for the specified address (or all) have been */
   /* deleted from the chip.  Now, let's make sure that our stored Link */
   /* Key Array is in sync with these changes.                          */

   /* First check to see all Link Keys were deleted.                    */
   ASSIGN_BD_ADDR(NULL_BD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   if(COMPARE_BD_ADDR(BD_ADDR, NULL_BD_ADDR))
      memset(LinkKeyInfo, 0, sizeof(LinkKeyInfo));
   else
   {
      /* Individual Link Key.  Go ahead and see if know about the entry */
      /* in the list.                                                   */
      for(Result=0;(Result<sizeof(LinkKeyInfo)/sizeof(LinkKeyInfo_t));Result++)
      {
         if(COMPARE_BD_ADDR(BD_ADDR, LinkKeyInfo[Result].BD_ADDR))
         {
            LinkKeyInfo[Result].BD_ADDR = NULL_BD_ADDR;

            break;
         }
      }
   }

   return(Result);
}

   /* The following function is responsible for displaying the current  */
   /* Command Options for either SYNC Client or Server.  The input      */
   /* parameter to this function is completely ignored, and only needs  */
   /* to be passed in because all Commands that can be entered at the   */
   /* Prompt pass in the parsed information.  This function displays the*/
   /* current Command Options that are available and always returns     */
   /* zero.                                                             */
static int DisplayHelp(ParameterList_t *TempParam)
{
   if(IsClient)
   {
      printf("******************************************************************\r\n");
      printf("* Command Options: Inquiry, DisplayInquiryList, Pair,            *\r\n");
      printf("*                  EndPairing, PINCodeResponse, PassKeyResponse, *\r\n");
      printf("*                  UserConfirmationResponse,                     *\r\n");
      printf("*                  SetDiscoverabilityMode, SetConnectabilityMode,*\r\n");
      printf("*                  SetPairabilityMode, ServiceDiscovery,         *\r\n");
      printf("*                  ChangeSimplePairingParameters,                *\r\n");
      printf("*                  GetLocalAddress, GetLocalName, SetLocalName,  *\r\n");
      printf("*                  GetClassOfDevice, SetClassOfDevice,           *\r\n");
      printf("*                  GetRemoteName, Connect, CloseConnection,      *\r\n");
      printf("*                  OpenCommandServer, CloseCommandServer,        *\r\n");
      printf("*                  IrMCObjectGet, IrMCObjectPut                  *\r\n");
      printf("*                  IrMCSpecialGet, IrMCRTCPut, IrMCObjectDelete  *\r\n");
      printf("*                  Abort, EnableDebug, Help, Quit.               *\r\n");
      printf("******************************************************************\r\n");
   }
   else
   {
      printf("******************************************************************\r\n");
      printf("* Command Options: Inquiry, DisplayInquiryList, Pair,            *\r\n");
      printf("*                  EndPairing, PINCodeResponse, PassKeyResponse, *\r\n");
      printf("*                  UserConfirmationResponse,                     *\r\n");
      printf("*                  SetDiscoverabilityMode, SetConnectabilityMode,*\r\n");
      printf("*                  SetPairabilityMode,                           *\r\n");
      printf("*                  ChangeSimplePairingParameters,                *\r\n");
      printf("*                  GetLocalAddress, GetLocalName, SetLocalName,  *\r\n");
      printf("*                  GetClassOfDevice, SetClassOfDevice,           *\r\n");
      printf("*                  GetRemoteName, ServiceDiscovery, Open, Close, *\r\n");
      printf("*                  EnableDebug, Help, Quit.                      *\r\n");
      printf("******************************************************************\r\n");
   }

   return(0);
}

   /* The following function is responsible for enabling or disabling   */
   /* HCI Debugging.  This function returns zero if successful and a    */
   /* negative value if an error occurred.                              */
static int EnableDebug(ParameterList_t *TempParam)
{
   int                      ret_val;
   int                      Result;
   char                    *LogFileName;
   BTPS_Debug_Parameters_t  DebugParameters;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Bluetooth Stack is initialized, go ahead and check to see if   */
      /* the user is enabling or disabling Debugging.                   */
      if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].intParam >= dtLogFile) && (TempParam->Params[0].intParam <= dtFTS))
      {
         /* Check to see if the user is requesting to enable or disable */
         /* debugging.                                                  */
         if(!TempParam->Params[0].intParam)
         {
            /* Disable, check to see if debugging is enabled.           */
            if(DebugID)
            {
               BTPS_Debug_Cleanup(BluetoothStackID, DebugID);

               printf("Debugging is now disabled.\r\n");

               /* Flag that debugging is no longer enabled.             */
               DebugID = 0;

               /* Flag success to the caller.                           */
               ret_val = 0;
            }
            else
            {
               printf("Debugging is not currently enabled.\r\n");

               /* Flag success to the caller.                           */
               ret_val = 0;
            }
         }
         else
         {
            /* Enable, check to see if debugging is not already enabled.*/
            if(!DebugID)
            {
               /* Parameters appear to be semi-valid, AND the user is   */
               /* requesting to enable debugging.  Go ahead and see if  */
               /* any additional parameters were specified.             */
               switch(TempParam->Params[1].intParam)
               {
                  case dtLogFile:
                     if((TempParam->NumberofParameters > 2) && (TempParam->Params[2].strParam))
                        LogFileName = TempParam->Params[2].strParam;
                     else
                        LogFileName = DEFAULT_DEBUG_LOG_FILE_NAME;
                     break;
                  case dtFTS:
                     if((TempParam->NumberofParameters > 2) && (TempParam->Params[2].strParam))
                        LogFileName = TempParam->Params[2].strParam;
                     else
                        LogFileName = DEFAULT_DEBUG_FTS_FILE_NAME;
                     break;
                  default:
                     LogFileName = NULL;
                     break;
               }

               /* Verify that all specified parameters were correct.    */
               if((TempParam->Params[1].intParam == dtDebugTerminal) || ((TempParam->Params[1].intParam != dtDebugTerminal) && (LogFileName)  && (strlen(LogFileName))))
               {
                  DebugParameters.DebugType       = (BTPS_Debug_Type_t)TempParam->Params[1].intParam;
                  DebugParameters.DebugFlags      = 0;
                  DebugParameters.ParameterString = LogFileName;

                  if((Result = BTPS_Debug_Initialize(BluetoothStackID, &DebugParameters)) > 0)
                  {
                     DebugID = (unsigned int)Result;

                     printf("BTPS_Debug_Initialize() Success: %d.\r\n", Result);

                     if(TempParam->Params[1].intParam != dtDebugTerminal)
                        printf("   Log File Name: %s\r\n", LogFileName);

                     /* Flag success to the caller.                     */
                     ret_val = 0;
                  }
                  else
                  {
                     printf("BTPS_Debug_Initialize() Failure: %d.\r\n", Result);

                     /* Flag that an error occurred while submitting the*/
                     /* command.                                        */
                     ret_val = FUNCTION_ERROR;
                  }
               }
               else
               {
                  /* Invalid parameters specified so flag an error to   */
                  /* the user.                                          */
                  printf("Usage: EnableDebug [Enable/Disable (Enable = 1, Disable = 0)] [DebugType (ASCII File = 0, Debug Console = 1, FTS File = 2)] [[Log File Name] (optional)].\r\n");

                  /* Flag that an error occurred while submitting the   */
                  /* command.                                           */
                  ret_val = INVALID_PARAMETERS_ERROR;
               }
            }
            else
            {
               printf("Debugging is already enabled.\r\n");

               /* Flag success to the caller.                           */
               ret_val = 0;
            }
         }
      }
      else
      {
         /* Invalid parameters specified so flag an error to the user.  */
         printf("Usage: EnableDebug [Enable/Disable (Enable = 1, Disable = 0)] [DebugType (ASCII File = 0, Debug Console = 1, FTS File = 2)] [[Log File Name] (optional)].\r\n");

         /* Flag that an error occurred while submitting the command.   */
         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for performing a General    */
   /* Inquiry for discovering Bluetooth Devices.  This function requires*/
   /* that a valid Bluetooth Stack ID exists before running.  This      */
   /* function returns zero is successful or a negative value if there  */
   /* was an error.                                                     */
static int Inquiry(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Use the GAP_Perform_Inquiry() function to perform an Inquiry.  */
      /* The Inquiry will last 10 seconds or until 1 Bluetooth Device is*/
      /* found.  When the Inquiry Results become available the          */
      /* GAP_Event_Callback is called.                                  */
      Result = GAP_Perform_Inquiry(BluetoothStackID, itGeneralInquiry, 0, 0, 10, MAX_INQUIRY_RESULTS, GAP_Event_Callback, (unsigned long)NULL);

      /* Next, check to see if the GAP_Perform_Inquiry() function was   */
      /* successful.                                                    */
      if(!Result)
      {
         /* The Inquiry appears to have been sent successfully.         */
         /* Processing of the results returned from this command occurs */
         /* within the GAP_Event_Callback() function.                   */
         printf("Return Value is %d GAP_Perform_Inquiry() SUCCESS.\r\n", Result);

         /* Flag that we have found NO Bluetooth Devices.               */
         NumberofValidResponses = 0;

         ret_val                = 0;
      }
      else
      {
         /* A error occurred while performing the Inquiry.              */
         printf("Return Value is %d GAP_Perform_Inquiry() FAILURE.\r\n", Result);

         ret_val = Result;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is a utility function that exists to       */
   /* display the current Inquiry List (with Indexes).  This is useful  */
   /* in case the user has forgotten what Inquiry Index a particular    */
   /* Bluteooth Device was located in.  This function returns zero on   */
   /* successful execution and a negative value on all errors.          */
static int DisplayInquiryList(ParameterList_t *TempParam)
{
   int          ret_val;
   char         BoardStr[13];
   unsigned int Index;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Simply display all of the items in the Inquiry List.           */
      printf("Inquiry List: %d Devices%s\r\n\r\n", NumberofValidResponses, NumberofValidResponses?":":".");

      for(Index=0;Index<NumberofValidResponses;Index++)
      {
         BD_ADDRToStr(InquiryResultList[Index], BoardStr);

         printf(" Inquiry Result: %d, %s.\r\n", (Index+1), BoardStr);
      }

      if(NumberofValidResponses)
         printf("\r\n");

      /* All finished, flag success to the caller.                      */
      ret_val = 0;
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for setting the             */
   /* Discoverability Mode of the local device.  This function returns  */
   /* zero on successful execution and a negative value on all errors.  */
static int SetDiscoverabilityMode(ParameterList_t *TempParam)
{
   int                        Result;
   int                        ret_val;
   GAP_Discoverability_Mode_t DiscoverabilityMode;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].intParam >= 0) && (TempParam->Params[0].intParam <= 2))
      {
         /* Parameters appear to be valid, map the specified parameters */
         /* into the API specific parameters.                           */
         if(TempParam->Params[0].intParam == 1)
            DiscoverabilityMode = dmLimitedDiscoverableMode;
         else
         {
            if(TempParam->Params[0].intParam == 2)
               DiscoverabilityMode = dmGeneralDiscoverableMode;
            else
               DiscoverabilityMode = dmNonDiscoverableMode;
         }

         /* Parameters mapped, now set the Discoverability Mode.        */
         Result = GAP_Set_Discoverability_Mode(BluetoothStackID, DiscoverabilityMode, (DiscoverabilityMode == dmLimitedDiscoverableMode)?60:0);

         /* Next, check the return value to see if the command was      */
         /* issued successfully.                                        */
         if(Result >= 0)
         {
            /* The Mode was changed successfully.                       */
            printf("Discoverability Mode successfully set to: %s Discoverable.\r\n", (DiscoverabilityMode == dmNonDiscoverableMode)?"Non":((DiscoverabilityMode == dmGeneralDiscoverableMode)?"General":"Limited"));

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            /* There was an error setting the Mode.                     */
            printf("GAP_Set_Discoverability_Mode() Failure: %d.\r\n", Result);

            /* Flag that an error occurred while submitting the command.*/
            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: SetDiscoverabilityMode [Mode (0 = Non Discoverable, 1 = Limited Discoverable, 2 = General Discoverable)].\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for setting the             */
   /* Connectability Mode of the local device.  This function returns   */
   /* zero on successful execution and a negative value on all errors.  */
static int SetConnectabilityMode(ParameterList_t *TempParam)
{
   int                       Result;
   int                       ret_val;
   GAP_Connectability_Mode_t ConnectableMode;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].intParam >= 0) && (TempParam->Params[0].intParam <= 1))
      {
         /* Parameters appear to be valid, map the specified parameters */
         /* into the API specific parameters.                           */
         if(TempParam->Params[0].intParam == 0)
            ConnectableMode = cmNonConnectableMode;
         else
            ConnectableMode = cmConnectableMode;

         /* Parameters mapped, now set the Connectabilty Mode.          */
         Result = GAP_Set_Connectability_Mode(BluetoothStackID, ConnectableMode);

         /* Next, check the return value to see if the command was      */
         /* issued successfully.                                        */
         if(Result >= 0)
         {
            /* The Mode was changed successfully.                       */
            printf("Connectability Mode successfully set to: %s.\r\n", (ConnectableMode == cmNonConnectableMode)?"Non Connectable":"Connectable");

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            /* There was an error setting the Mode.                     */
            printf("GAP_Set_Connectability_Mode() Failure: %d.\r\n", Result);

            /* Flag that an error occurred while submitting the command.*/
            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: SetConnectabilityMode [Mode (0 = Non Conectable, 1 = Connectable)].\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for setting the Pairability */
   /* Mode of the local device.  This function returns zero on          */
   /* successful execution and a negative value on all errors.          */
static int SetPairabilityMode(ParameterList_t *TempParam)
{
   int                    Result;
   int                    ret_val;
   GAP_Pairability_Mode_t PairabilityMode;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].intParam >= 0) && (TempParam->Params[0].intParam <= 2))
      {
         /* Parameters appear to be valid, map the specified parameters */
         /* into the API specific parameters.                           */
         if(TempParam->Params[0].intParam == 0)
            PairabilityMode = pmNonPairableMode;
         else
         {
            if(TempParam->Params[0].intParam == 1)
               PairabilityMode = pmPairableMode;
            else
               PairabilityMode = pmPairableMode_EnableSecureSimplePairing;
         }

         /* Parameters mapped, now set the Pairability Mode.            */
         Result = GAP_Set_Pairability_Mode(BluetoothStackID, PairabilityMode);

         /* Next, check the return value to see if the command was      */
         /* issued successfully.                                        */
         if(Result >= 0)
         {
            /* The Mode was changed successfully.                       */
            printf("Pairability Mode successfully set to: %s.\r\n", (PairabilityMode == pmNonPairableMode)?"Non Pairable":((PairabilityMode == pmPairableMode)?"Pairable":"Pairable (Secure Simple Pairing)"));

            /* If Secure Simple Pairing has been enabled, inform the    */
            /* user of the current Secure Simple Pairing parameters.    */
            if(PairabilityMode == pmPairableMode_EnableSecureSimplePairing)
               printf("Current I/O Capabilities: %s, MITM Protection: %s.\r\n", IOCapabilitiesStrings[(unsigned int)IOCapability], MITMProtection?"TRUE":"FALSE");

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            /* There was an error setting the Mode.                     */
            printf("GAP_Set_Pairability_Mode() Failure: %d.\r\n", Result);

            /* Flag that an error occurred while submitting the command.*/
            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: SetPairabilityMode [Mode (0 = Non Pairable, 1 = Pairable, 2 = Pairable (Secure Simple Pairing)].\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for changing the Secure     */
   /* Simple Pairing Parameters that are exchanged during the Pairing   */
   /* procedure when Secure Simple Pairing (Security Level 4) is used.  */
   /* This function returns zero on successful execution and a negative */
   /* value on all errors.                                              */
static int ChangeSimplePairingParameters(ParameterList_t *TempParam)
{
   int ret_val;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 2) && (TempParam->Params[0].intParam >= 0) && (TempParam->Params[0].intParam <= 3))
      {
         /* Parameters appear to be valid, map the specified parameters */
         /* into the API specific parameters.                           */
         if(TempParam->Params[0].intParam == 0)
            IOCapability = icDisplayOnly;
         else
         {
            if(TempParam->Params[0].intParam == 1)
               IOCapability = icDisplayYesNo;
            else
            {
               if(TempParam->Params[0].intParam == 2)
                  IOCapability = icKeyboardOnly;
               else
                  IOCapability = icNoInputNoOutput;
            }
         }

         /* Finally map the Man in the Middle (MITM) Protection valud.  */
         MITMProtection = (Boolean_t)(TempParam->Params[1].intParam?TRUE:FALSE);

         /* Inform the user of the New I/O Capablities.                 */
         printf("Current I/O Capabilities: %s, MITM Protection: %s.\r\n", IOCapabilitiesStrings[(unsigned int)IOCapability], MITMProtection?"TRUE":"FALSE");

         /* Flag success to the caller.                                 */
         ret_val = 0;
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: ChangeSimplePairingParameters [I/O Capability (0 = Display Only, 1 = Display Yes/No, 2 = Keyboard Only, 3 = No Input/Output)] [MITM Requirement (0 = No, 1 = Yes)].\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for initiating bonding with */
   /* a remote device.  This function returns zero on successful        */
   /* execution and a negative value on all errors.                     */
static int Pair(ParameterList_t *TempParam)
{
   int                Result;
   int                ret_val;
   BD_ADDR_t          NullADDR;
   GAP_Bonding_Type_t BondingType;

   ASSIGN_BD_ADDR(NullADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Next, make sure that we are not already connected.             */
      if(!Connected)
      {
         /* There is currently no active connection, make sure that all */
         /* of the parameters required for this function appear to be at*/
         /* least semi-valid.                                           */
         if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].intParam) && (NumberofValidResponses) && (TempParam->Params[0].intParam <= NumberofValidResponses) && (!COMPARE_BD_ADDR(InquiryResultList[(TempParam->Params[0].intParam - 1)], NullADDR)))
         {
            /* Check to see if General Bonding was specified.           */
            if(TempParam->NumberofParameters > 1)
               BondingType = TempParam->Params[1].intParam?btGeneral:btDedicated;
            else
               BondingType = btDedicated;

            /* Before we submit the command to the stack, we need to    */
            /* make sure that we clear out any Link Key we have stored  */
            /* for the specified device.                                */
            DeleteLinkKey(InquiryResultList[(TempParam->Params[0].intParam - 1)]);

            /* Attempt to submit the command.                           */
            Result = GAP_Initiate_Bonding(BluetoothStackID, InquiryResultList[(TempParam->Params[0].intParam - 1)], BondingType, GAP_Event_Callback, (unsigned long)0);

            /* Check the return value of the submitted command for      */
            /* success.                                                 */
            if(!Result)
            {
               /* Display a messsage indicating that Bonding was        */
               /* initiated successfully.                               */
               printf("GAP_Initiate_Bonding (%s): Function Successful.\r\n", (BondingType == btDedicated)?"Dedicated":"General");

               /* Flag success to the caller.                           */
               ret_val = 0;
            }
            else
            {
               /* Display a message indicating that an error occured    */
               /* while initiating bonding.                             */
               printf("GAP_Initiate_Bonding() Failure: %d.\r\n", Result);

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* One or more of the necessary parameters is/are invalid.  */
            printf("Usage: Pair [Inquiry Index] [Bonding Type (0 = Dedicated, 1 = General) (optional).\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* Display an error to the user describing that Pairing can    */
         /* only occur when we are not connected.                       */
         printf("The Pair command can only be issued when not already connected.\r\n");

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for ending a previously     */
   /* initiated bonding session with a remote device.  This function    */
   /* returns zero on successful execution and a negative value on all  */
   /* errors.                                                           */
static int EndPairing(ParameterList_t *TempParam)
{
   int       Result;
   int       ret_val;
   BD_ADDR_t NullADDR;

   ASSIGN_BD_ADDR(NullADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].intParam) && (NumberofValidResponses) && (TempParam->Params[0].intParam <= NumberofValidResponses) && (!COMPARE_BD_ADDR(InquiryResultList[(TempParam->Params[0].intParam - 1)], NullADDR)))
      {
         /* Attempt to submit the command.                              */
         Result = GAP_End_Bonding(BluetoothStackID, InquiryResultList[(TempParam->Params[0].intParam - 1)]);

         /* Check the return value of the submitted command for success.*/
         if(!Result)
         {
            /* Display a messsage indicating that the End bonding was   */
            /* successfully submitted.                                  */
            printf("GAP_End_Bonding: Function Successful.\r\n");

            /* Flag success to the caller.                              */
            ret_val = 0;

            /* Flag that there is no longer a current Authentication    */
            /* procedure in progress.                                   */
            ASSIGN_BD_ADDR(CurrentRemoteBD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
         }
         else
         {
            /* Display a message indicating that an error occured while */
            /* ending bonding.                                          */
            printf("GAP_End_Bonding() Failure: %d.\r\n", Result);

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: EndPairing [Inquiry Index].\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for issuing a GAP           */
   /* Authentication Response with a PIN Code value specified via the   */
   /* input parameter.  This function returns zero on successful        */
   /* execution and a negative value on all errors.                     */
static int PINCodeResponse(ParameterList_t *TempParam)
{
   int                              Result;
   int                              ret_val;
   BD_ADDR_t                        NullADDR;
   PIN_Code_t                       PINCode;
   GAP_Authentication_Information_t GAP_Authentication_Information;

   ASSIGN_BD_ADDR(NullADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* First, check to see if there is an on-going Pairing operation  */
      /* active.                                                        */
      if(!COMPARE_BD_ADDR(CurrentRemoteBD_ADDR, NullADDR))
      {
         /* Make sure that all of the parameters required for this      */
         /* function appear to be at least semi-valid.                  */
         if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].strParam) && (strlen(TempParam->Params[0].strParam) > 0) && (strlen(TempParam->Params[0].strParam) <= sizeof(PIN_Code_t)))
         {
            /* Parameters appear to be valid, go ahead and convert the  */
            /* input parameter into a PIN Code.                         */

            /* Initialize the PIN code.                                 */
            ASSIGN_PIN_CODE(PINCode, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

            memcpy(&PINCode, TempParam->Params[0].strParam, strlen(TempParam->Params[0].strParam));

            /* Populate the response structure.                         */
            GAP_Authentication_Information.GAP_Authentication_Type      = atPINCode;
            GAP_Authentication_Information.Authentication_Data_Length   = (Byte_t)(strlen(TempParam->Params[0].strParam));
            GAP_Authentication_Information.Authentication_Data.PIN_Code = PINCode;

            /* Submit the Authentication Response.                      */
            Result = GAP_Authentication_Response(BluetoothStackID, CurrentRemoteBD_ADDR, &GAP_Authentication_Information);

            /* Check the return value for the submitted command for     */
            /* success.                                                 */
            if(!Result)
            {
               /* Operation was successful, inform the user.            */
               printf("GAP_Authentication_Response(), Pin Code Response Success.\r\n");

               /* Flag success to the caller.                           */
               ret_val = 0;
            }
            else
            {
               /* Inform the user that the Authentication Response was  */
               /* not successful.                                       */
               printf("GAP_Authentication_Response() Failure: %d.\r\n", Result);

               ret_val = FUNCTION_ERROR;
            }

            /* Flag that there is no longer a current Authentication    */
            /* procedure in progress.                                   */
            ASSIGN_BD_ADDR(CurrentRemoteBD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
         }
         else
         {
            /* One or more of the necessary parameters is/are invalid.  */
            printf("Usage: PINCodeResponse [PIN Code].\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* There is not currently an on-going authentication operation,*/
         /* inform the user of this error condition.                    */
         printf("Unable to issue PIN Code Authentication Response: Authentication is not currently in progress.\r\n");

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for issuing a GAP           */
   /* Authentication Response with a Pass Key value specified via the   */
   /* input parameter.  This function returns zero on successful        */
   /* execution and a negative value on all errors.                     */
static int PassKeyResponse(ParameterList_t *TempParam)
{
   int                              Result;
   int                              ret_val;
   BD_ADDR_t                        NullADDR;
   GAP_Authentication_Information_t GAP_Authentication_Information;

   ASSIGN_BD_ADDR(NullADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* First, check to see if there is an on-going Pairing operation  */
      /* active.                                                        */
      if(!COMPARE_BD_ADDR(CurrentRemoteBD_ADDR, NullADDR))
      {
         /* Make sure that all of the parameters required for this      */
         /* function appear to be at least semi-valid.                  */
         if((TempParam) && (TempParam->NumberofParameters > 0) && (strlen(TempParam->Params[0].strParam) <= GAP_PASSKEY_MAXIMUM_NUMBER_OF_DIGITS))
         {
            /* Parameters appear to be valid, go ahead and populate the */
            /* response structure.                                      */
            GAP_Authentication_Information.GAP_Authentication_Type     = atPassKey;
            GAP_Authentication_Information.Authentication_Data_Length  = (Byte_t)(sizeof(DWord_t));
            GAP_Authentication_Information.Authentication_Data.Passkey = (DWord_t)(TempParam->Params[0].intParam);

            /* Submit the Authentication Response.                      */
            Result = GAP_Authentication_Response(BluetoothStackID, CurrentRemoteBD_ADDR, &GAP_Authentication_Information);

            /* Check the return value for the submitted command for     */
            /* success.                                                 */
            if(!Result)
            {
               /* Operation was successful, inform the user.            */
               printf("GAP_Authentication_Response(), Passkey Response Success.\r\n");

               /* Flag success to the caller.                           */
               ret_val = 0;
            }
            else
            {
               /* Inform the user that the Authentication Response was  */
               /* not successful.                                       */
               printf("GAP_Authentication_Response() Failure: %d.\r\n", Result);

               ret_val = FUNCTION_ERROR;
            }

            /* Flag that there is no longer a current Authentication    */
            /* procedure in progress.                                   */
            ASSIGN_BD_ADDR(CurrentRemoteBD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
         }
         else
         {
            /* One or more of the necessary parameters is/are invalid.  */
            printf("Usage: PassKeyResponse [Numeric Passkey (0 - 999999)].\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* There is not currently an on-going authentication operation,*/
         /* inform the user of this error condition.                    */
         printf("Unable to issue Pass Key Authentication Response: Authentication is not currently in progress.\r\n");

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for issuing a GAP           */
   /* Authentication Response with a User Confirmation value specified  */
   /* via the input parameter.  This function returns zero on successful*/
   /* execution and a negative value on all errors.                     */
static int UserConfirmationResponse(ParameterList_t *TempParam)
{
   int                              Result;
   int                              ret_val;
   BD_ADDR_t                        NullADDR;
   GAP_Authentication_Information_t GAP_Authentication_Information;

   ASSIGN_BD_ADDR(NullADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* First, check to see if there is an on-going Pairing operation  */
      /* active.                                                        */
      if(!COMPARE_BD_ADDR(CurrentRemoteBD_ADDR, NullADDR))
      {
         /* Make sure that all of the parameters required for this      */
         /* function appear to be at least semi-valid.                  */
         if((TempParam) && (TempParam->NumberofParameters > 0))
         {
            /* Parameters appear to be valid, go ahead and populate the */
            /* response structure.                                      */
            GAP_Authentication_Information.GAP_Authentication_Type          = atUserConfirmation;
            GAP_Authentication_Information.Authentication_Data_Length       = (Byte_t)(sizeof(Byte_t));
            GAP_Authentication_Information.Authentication_Data.Confirmation = (Boolean_t)(TempParam->Params[0].intParam?TRUE:FALSE);

            /* Submit the Authentication Response.                      */
            Result = GAP_Authentication_Response(BluetoothStackID, CurrentRemoteBD_ADDR, &GAP_Authentication_Information);

            /* Check the return value for the submitted command for     */
            /* success.                                                 */
            if(!Result)
            {
               /* Operation was successful, inform the user.            */
               printf("GAP_Authentication_Response(), User Confirmation Response Success.\r\n");

               /* Flag success to the caller.                           */
               ret_val = 0;
            }
            else
            {
               /* Inform the user that the Authentication Response was  */
               /* not successful.                                       */
               printf("GAP_Authentication_Response() Failure: %d.\r\n", Result);

               ret_val = FUNCTION_ERROR;
            }

            /* Flag that there is no longer a current Authentication    */
            /* procedure in progress.                                   */
            ASSIGN_BD_ADDR(CurrentRemoteBD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
         }
         else
         {
            /* One or more of the necessary parameters is/are invalid.  */
            printf("Usage: UserConfirmationResponse [Confirmation (0 = No, 1 = Yes)].\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* There is not currently an on-going authentication operation,*/
         /* inform the user of this error condition.                    */
         printf("Unable to issue User Confirmation Authentication Response: Authentication is not currently in progress.\r\n");

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for querying the Bluetooth  */
   /* Device Address of the local Bluetooth Device.  This function      */
   /* returns zero on successful execution and a negative value on all  */
   /* errors.                                                           */
static int GetLocalAddress(ParameterList_t *TempParam)
{
   int       Result;
   int       ret_val;
   char      BoardStr[13];
   BD_ADDR_t BD_ADDR;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Attempt to submit the command.                                 */
      Result = GAP_Query_Local_BD_ADDR(BluetoothStackID, &BD_ADDR);

      /* Check the return value of the submitted command for success.   */
      if(!Result)
      {
         BD_ADDRToStr(BD_ADDR, BoardStr);

         printf("BD_ADDR of Local Device is: %s.\r\n", BoardStr);

         /* Flag success to the caller.                                 */
         ret_val = 0;
      }
      else
      {
         /* Display a message indicating that an error occured while    */
         /* attempting to query the Local Device Address.               */
         printf("GAP_Query_Local_BD_ADDR() Failure: %d.\r\n", Result);

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for setting the name of the */
   /* local Bluetooth Device to a specified name.  This function returns*/
   /* zero on successful execution and a negative value on all errors.  */
static int SetLocalName(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].strParam))
      {
         /* Attempt to submit the command.                              */
         Result = GAP_Set_Local_Device_Name(BluetoothStackID, TempParam->Params[0].strParam);

         /* Check the return value of the submitted command for success.*/
         if(!Result)
         {
            /* Display a messsage indicating that the Device Name was   */
            /* successfully submitted.                                  */
            printf("Local Device Name set to: %s.\r\n", TempParam->Params[0].strParam);

            /* Rewrite extended inquiry information.                    */
            WriteEIRInformation(TempParam->Params[0].strParam);

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            /* Display a message indicating that an error occured while */
            /* attempting to set the local Device Name.                 */
            printf("GAP_Set_Local_Device_Name() Failure: %d.\r\n", Result);

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: SetLocalName [Local Name (no spaces allowed)].\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for querying the Bluetooth  */
   /* Device Name of the local Bluetooth Device.  This function returns */
   /* zero on successful execution and a negative value on all errors.  */
static int GetLocalName(ParameterList_t *TempParam)
{
   int  Result;
   int  ret_val;
   char LocalName[257];

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Attempt to submit the command.                                 */
      Result = GAP_Query_Local_Device_Name(BluetoothStackID, sizeof(LocalName), (char *)LocalName);

      /* Check the return value of the submitted command for success.   */
      if(!Result)
      {
         printf("Name of Local Device is: %s.\r\n", LocalName);

         /* Flag success to the caller.                                 */
         ret_val = 0;
      }
      else
      {
         /* Display a message indicating that an error occured while    */
         /* attempting to query the Local Device Name.                  */
         printf("GAP_Query_Local_Device_Name() Failure: %d.\r\n", Result);

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for setting the Class of    */
   /* Device of the local Bluetooth Device to a Class Of Device value.  */
   /* This function returns zero on successful execution and a negative */
   /* value on all errors.                                              */
static int SetClassOfDevice(ParameterList_t *TempParam)
{
   int               Result;
   int               ret_val;
   Class_of_Device_t Class_of_Device;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters > 0))
      {
         /* Attempt to submit the command.                              */
         ASSIGN_CLASS_OF_DEVICE(Class_of_Device, (Byte_t)((TempParam->Params[0].intParam) & 0xFF), (Byte_t)(((TempParam->Params[0].intParam) >> 8) & 0xFF), (Byte_t)(((TempParam->Params[0].intParam) >> 16) & 0xFF));

         Result = GAP_Set_Class_Of_Device(BluetoothStackID, Class_of_Device);

         /* Check the return value of the submitted command for success.*/
         if(!Result)
         {
            /* Display a messsage indicating that the Class of Device   */
            /* was successfully submitted.                              */
            printf("Set Class of Device to 0x%02X%02X%02X.\r\n", Class_of_Device.Class_of_Device0, Class_of_Device.Class_of_Device1, Class_of_Device.Class_of_Device2);

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            /* Display a message indicating that an error occured while */
            /* attempting to set the local Class of Device.             */
            printf("GAP_Set_Class_Of_Device() Failure: %d.\r\n", Result);

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: SetClassOfDevice [Class of Device].\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for querying the Bluetooth  */
   /* Class of Device of the local Bluetooth Device.  This function     */
   /* returns zero on successful execution and a negative value on all  */
   /* errors.                                                           */
static int GetClassOfDevice(ParameterList_t *TempParam)
{
   int               Result;
   int               ret_val;
   Class_of_Device_t Class_of_Device;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Attempt to submit the command.                                 */
      Result = GAP_Query_Class_Of_Device(BluetoothStackID, &Class_of_Device);

      /* Check the return value of the submitted command for success.   */
      if(!Result)
      {
         printf("Local Class of Device is: 0x%02X%02X%02X.\r\n", Class_of_Device.Class_of_Device0, Class_of_Device.Class_of_Device1, Class_of_Device.Class_of_Device2);

         /* Flag success to the caller.                                 */
         ret_val = 0;
      }
      else
      {
         /* Display a message indicating that an error occured while    */
         /* attempting to query the Local Class of Device.              */
         printf("GAP_Query_Class_Of_Device() Failure: %d.\r\n", Result);

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for querying the Bluetooth  */
   /* Device Name of the specified remote Bluetooth Device.  This       */
   /* function returns zero on successful execution and a negative value*/
   /* on all errors.                                                    */
static int GetRemoteName(ParameterList_t *TempParam)
{
   int       Result;
   int       ret_val;
   BD_ADDR_t NullADDR;

   ASSIGN_BD_ADDR(NullADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].intParam) && (NumberofValidResponses) && (TempParam->Params[0].intParam <= NumberofValidResponses) && (!COMPARE_BD_ADDR(InquiryResultList[(TempParam->Params[0].intParam - 1)], NullADDR)))
      {
         /* Attempt to submit the command.                              */
         Result = GAP_Query_Remote_Device_Name(BluetoothStackID, InquiryResultList[(TempParam->Params[0].intParam - 1)], GAP_Event_Callback, (unsigned long)0);

         /* Check the return value of the submitted command for success.*/
         if(!Result)
         {
            /* Display a messsage indicating that Remote Name request   */
            /* was initiated successfully.                              */
            printf("GAP_Query_Remote_Device_Name: Function Successful.\r\n");

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            /* Display a message indicating that an error occured while */
            /* initiating the Remote Name request.                      */
            printf("GAP_Query_Remote_Device_Name() Failure: %d.\r\n", Result);

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: GetRemoteName [Inquiry Index].\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for issuing a Service Search*/
   /* Attribute Request to a Remote SDP Server.  This function returns  */
   /* zero if successful and a negative value if an error occurred.     */
static int ServiceDiscovery(ParameterList_t *TempParam)
{
   int                           Result;
   int                           ret_val;
   int                           Index;
   BD_ADDR_t                     NullADDR;
   SDP_UUID_Entry_t              SDPUUIDEntry;
   SDP_Attribute_ID_List_Entry_t AttributeID;

   ASSIGN_BD_ADDR(NullADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Now let's make sure that all of the parameters required for    */
      /* this function appear to be at least semi-valid.                */
      if((TempParam) && (TempParam->NumberofParameters > 1) && (((TempParam->Params[1].intParam) && (TempParam->Params[1].intParam <= NUM_UUIDS)) || ((!TempParam->Params[1].intParam) && (TempParam->NumberofParameters > 2))) && (TempParam->Params[0].intParam) && (NumberofValidResponses) && (TempParam->Params[0].intParam <= NumberofValidResponses) && (!COMPARE_BD_ADDR(InquiryResultList[(TempParam->Params[0].intParam - 1)], NullADDR)))
      {
         /* OK, parameters appear to be semi-valid, now let's attempt to*/
         /* issue the SDP Service Attribute Request.                    */
         if(!TempParam->Params[1].intParam)
         {
            /* First let's build the UUID 32 value(s).                  */
            SDPUUIDEntry.SDP_Data_Element_Type = deUUID_32;

            ASSIGN_SDP_UUID_32(SDPUUIDEntry.UUID_Value.UUID_32, ((TempParam->Params[2].intParam & 0xFF000000) >> 24), ((TempParam->Params[2].intParam & 0x00FF0000) >> 16), ((TempParam->Params[2].intParam & 0x0000FF00) >> 8), (TempParam->Params[2].intParam & 0x000000FF));
         }
         else
         {
            SDPUUIDEntry.SDP_Data_Element_Type = deUUID_128;

            SDPUUIDEntry.UUID_Value.UUID_128   = UUIDTable[TempParam->Params[1].intParam - 1].UUID;
         }

         AttributeID.Attribute_Range    = (Boolean_t)TRUE;
         AttributeID.Start_Attribute_ID = 0;
         AttributeID.End_Attribute_ID   = 65335;

         /* Finally submit the SDP Request.                             */
         Result = SDP_Service_Search_Attribute_Request(BluetoothStackID, InquiryResultList[(TempParam->Params[0].intParam - 1)], 1, &SDPUUIDEntry, 1, &AttributeID, SDP_Event_Callback, (unsigned long)0);

         if(Result > 0)
         {
            /* The SDP Request was submitted successfully.              */
            printf("SDP_Service_Search_Attribute_Request(%s) Success.\r\n", TempParam->Params[1].intParam?UUIDTable[TempParam->Params[1].intParam - 1].Name:"Manual");

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            /* There was an error submitting the SDP Request.           */
            printf("SDP_Service_Search_Attribute_Request(%s) Failure: %d.\r\n", TempParam->Params[1].intParam?UUIDTable[TempParam->Params[1].intParam - 1].Name:"Manual", Result);

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: ServiceDiscovery [Inquiry Index] [Profile Index] [16/32 bit UUID (Manual only)].\r\n");
         printf("\r\n   Profile Index:\r\n");
         printf("       0) Manual (MUST specify 16/32 bit UUID)\r\n");
         for(Index=0;Index<NUM_UUIDS;Index++)
            printf("      %2d) %s\r\n", Index + 1, UUIDTable[Index].Name);
         printf("\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for creating a local SYNC   */
   /* Server.  This function returns zero if successful and a negative  */
   /* value if an error occurred.                                       */
static int OpenServer(ParameterList_t *TempParam)
{
   int                              ret_val;
   int                              Result;
   IrMC_Object_Store_IO_Interface_t IOInterface;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the Server Port ID passed to this  */
      /* function is valid.                                             */
      if((TempParam) && (TempParam->NumberofParameters >= 1) && (TempParam->Params[0].intParam))
      {
         /* Now check to make sure that a Server doesn't already exist. */
         if(!SYNCID)
         {
            /* Check to see if an optional Received Object Store Path   */
            /* was specified.                                           */
            if((TempParam->NumberofParameters >= 2) && (TempParam->Params[1].strParam) && (strlen(TempParam->Params[1].strParam)))
               strcpy(ObjectStorePath, TempParam->Params[1].strParam);
            else
            {
               /* Received Object Store Path, not specified, go ahead   */
               /* and use the current directory.                        */
               if(!getcwd(ObjectStorePath, sizeof(ObjectStorePath)))
               {
                  /* There was an error fetching the Directory, so let's*/
                  /* just default it to the Root Directory.             */
                  ObjectStorePath[0] = '/';
               }
            }

            /* Make sure the Received Object Store Path ends in '/'     */
            /* character (this way we can append the received file names*/
            /* directly to the path).                                   */
            if(ObjectStorePath[strlen(ObjectStorePath) - 1] != '/')
               ObjectStorePath[strlen(ObjectStorePath)] = '/';

            /* Verify that the specified Path is valid.                 */
            if(VerifyObjectStorePath(ObjectStorePath))
            {
               /* The above parameters are valid, attempt to open a     */
               /* Local SYNC Port.                                      */
               Result = SYNC_Open_Server_Port(BluetoothStackID, TempParam->Params[0].intParam, stIrMC_Server, SYNC_ALL_OBJECT_STORES_SUPPORTED_BIT, SYNC_Event_Callback_Server, 0);

               /* Now check to see if the function call was successfully*/
               /* made.                                                 */
               if(Result > 0)
               {
                  /* Now that a Service has been created try and        */
                  /* Register the SDP Record.                           */
                  if(!SYNC_Register_Server_SDP_Record(BluetoothStackID, Result, "IrMC Synchronization", &ServerSDPRecordHandle))
                  {
                     /* The Server was opened successfully and the SDP  */
                     /* Record was successfully added.  The return value*/
                     /* of the call is the SYNC ID and is required for  */
                     /* future SYNC Profile calls.                      */
                     SYNCID = Result;

                     printf("SYNC_Open_Server_Port() Successful (ID = %04X).\r\n", SYNCID);
                     printf("  Object Store Path: %s\r\n", ObjectStorePath);

                     /* Finally we will attempt to register the Object  */
                     /* Stores used for this application                */
                     IOInterface.Read     = ObjectStoreIORead;
                     IOInterface.Write    = ObjectStoreIOWrite;
                     IOInterface.Create   = ObjectStoreIOCreate;
                     IOInterface.Delete   = ObjectStoreIODelete;
                     IOInterface.Populate = ObjectStoreIOPopulate;

                     /* Initialize Object Store I/O Interface structure.*/
                     Result = RegisterObjectStore(osPhonebook, &PhonebookInfoLog, DefaultDeviceInfo.SerialNumber, ObjectStorePath, &IOInterface);
                     if(Result == OBJSTORE_SUCCESS_RESULT)
                        printf("Object Store Initialization Complete (Phonebook)\r\n");
                     else
                        printf("Error Initializing Object Store (Phonebook).\r\n");

                     Result = RegisterObjectStore(osNotes, &NotesInfoLog, DefaultDeviceInfo.SerialNumber, ObjectStorePath, &IOInterface);
                     if(Result == OBJSTORE_SUCCESS_RESULT)
                        printf("Object Store Initialization Complete (Notes).\r\n");
                     else
                        printf("Error Initializing Object Store (Notes)\r\n");

                     Result = RegisterObjectStore(osCalendar, &CalendarInfoLog, DefaultDeviceInfo.SerialNumber, ObjectStorePath, &IOInterface);
                     if(Result == OBJSTORE_SUCCESS_RESULT)
                        printf("Object Store Initialization Complete (Calendar).\r\n");
                     else
                        printf("Error Initializing Object Store (Calendar).\r\n");

                     Result = RegisterObjectStore(osMsgIn, &MessagesInfoLog, DefaultDeviceInfo.SerialNumber, ObjectStorePath, &IOInterface);
                     if(Result == OBJSTORE_SUCCESS_RESULT)
                        printf("Object Store Initialization Complete (In).\r\n");
                     else
                        printf("Error Initializing Object Store (In).\r\n");

                     Result = RegisterObjectStore(osMsgOut, &MessagesInfoLog, DefaultDeviceInfo.SerialNumber, ObjectStorePath, &IOInterface);
                     if(Result == OBJSTORE_SUCCESS_RESULT)
                        printf("Object Store Initialization Complete (Out).\r\n");
                     else
                        printf("Error Initializing Object Store (Out).\r\n");

                     Result = RegisterObjectStore(osMsgSent, &MessagesInfoLog, DefaultDeviceInfo.SerialNumber, ObjectStorePath, &IOInterface);
                     if(Result == OBJSTORE_SUCCESS_RESULT)
                        printf("Object Store Initialization Complete (Sent).\r\n");
                     else
                        printf("Error Initializing Object Store (Sent).\r\n");

                     Result = RegisterObjectStore(osInbox, NULL, DefaultDeviceInfo.SerialNumber, ObjectStorePath, &IOInterface);
                     if(Result == OBJSTORE_SUCCESS_RESULT)
                        printf("Object Store Initialization Complete (Inbox).\r\n");
                     else
                        printf("Error Initializing Object Store (Inbox).\r\n");

                     /* Initialize the SYNC Server State Information.   */
                     memset(&SYNCServer, 0, sizeof(SYNCServer));

                     SYNCServer.SYNCID              = SYNCID;
                     SYNCServer.Connected           = FALSE;
                     SYNCServer.CurrentOperation    = coNone;
                     SYNCServer.ServiceRecordHandle = ServerSDPRecordHandle;

                     /* Flag success to the caller.                     */
                     ret_val                        = 0;
                  }
                  else
                  {
                     /* Error registering SDP Record, go ahead and close*/
                     /* down the server.                                */
                     SYNC_Close_Server_Port(BluetoothStackID, Result);

                     printf("SYNC_Register_Server_SDP_Record() Failure.\r\n");

                     ret_val = FUNCTION_ERROR;
                  }
               }
               else
               {
                  /* There was an error opening the Server.             */
                  printf("SYNC_Open_Server_Port() Failure: %d.\r\n", Result);

                  ret_val = Result;
               }
            }
            else
            {
               printf("Invalid Object Store Path: %s\r\n", ObjectStorePath);

               ret_val = INVALID_PARAMETERS_ERROR;
            }
         }
         else
         {
            /* A Connection is already open, this program only supports */
            /* one Server or Client at a time.                          */
            printf("SYNC Server already open.\r\n");

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         printf("Usage: Open [Port Number] [Object Store Path (optional)].\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      printf("Stack ID Invalid.\r\n");

      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for deleting a local SYNC   */
   /* Profile Server that was created via a successful call to the      */
   /* OpenServer() function.  This function returns zero if successful  */
   /* and a negative value if an error occurred.                        */
static int CloseServer(ParameterList_t *TempParam)
{
   int          Result;
   int          ret_val;
   unsigned int Index;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      if(SYNCID)
      {
         /* SYNC Server opened, close any open SDP Record.              */
         if(ServerSDPRecordHandle)
         {
            SYNC_Un_Register_SDP_Record(BluetoothStackID, SYNCID, ServerSDPRecordHandle);

            ServerSDPRecordHandle = 0;
         }

         /* Next attempt to close this Server.                          */
         Result = SYNC_Close_Server_Port(BluetoothStackID, SYNCID);

         if(!Result)
         {
            /* Display a message indicating that the server was         */
            /* successfully closed.                                     */
            printf("SYNC_Close_Server_Port() Success.\r\n");

            /* Iterate through allocated store entries and release.     */
            for(Index=0;(Index < NUMBER_SUPPORTED_OBJECT_STORES);Index++)
            {
               if(ObjectStore[Index].Active)
               {
                  Result = OBJSTORE_ReleaseStore(&(ObjectStore[Index]));
                  if(Result == OBJSTORE_SUCCESS_RESULT)
                     printf("Store %d Released Successfully.\r\n", Index);
                  else
                     printf("Store %d Released Failed.\r\n", Index);
               }
            }

            /* Clear any current on-going operation.                    */
            ClearSYNCServerOperation();

            /* Flag the port has been closed.                           */
            SYNCID                      = 0;
            Connected                   = FALSE;

            SYNCServer.SYNCID           = 0;
            SYNCServer.Connected        = FALSE;
            SYNCServer.CurrentOperation = coNone;

            ret_val                     = 0;
         }
         else
         {
            /* An error occurred while attempting to close the server.  */
            printf("SYNC_Close_Server_Port() Failure: %d.\r\n", Result);

            ret_val = Result;
         }
      }
      else
      {
         /* No valid SYNC Server exists.                                */
         printf("No Server is currently open.\r\n");

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      printf("Stack ID Invalid.\r\n");

      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for creating a local SYNC   */
   /* Command Server.  This function returns zero if successful and a   */
   /* negative value if an error occurred.                              */
static int OpenCommandServer(ParameterList_t *TempParam)
{
   int ret_val;
   int Result;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the Server Port ID passed to this  */
      /* function is valid.                                             */
      if((TempParam) && (TempParam->NumberofParameters >= 1) && (TempParam->Params[0].intParam))
      {
         /* Now check to make sure that a Server doesn't already exist. */
         if(!CommandServerID)
         {
            /* The above parameters are valid, attempt to open a Local  */
            /* SYNC Command Server Port.                                */
            Result = SYNC_Open_Server_Port(BluetoothStackID, TempParam->Params[0].intParam, stIrMC_SyncCommand, SYNC_ALL_OBJECT_STORES_SUPPORTED_BIT, SYNC_Event_Callback_SyncCommand, 0);

            /* Now check to see if the function call was successfully   */
            /* made.                                                    */
            if(Result > 0)
            {
               /* Now that a Service has been created try and Register  */
               /* the SDP Record.                                       */
               if(!SYNC_Register_Server_SDP_Record(BluetoothStackID, Result, "Sync Command Service", &CommandServerSDPRecordHandle))
               {
                  /* The Server was opened successfully and the SDP     */
                  /* Record was successfully added.  The return value of*/
                  /* the call is the SYNC ID and is required for future */
                  /* SYNC Profile calls.                                */
                  CommandServerID = Result;

                  printf("SYNC_Open_Server_Port() Successful (ID = %04X).\r\n", CommandServerID);

                  /* Flag success to the caller.                        */
                  ret_val = 0;
               }
               else
               {
                  /* Error registering SDP Record, go ahead and close   */
                  /* down the server.                                   */
                  SYNC_Close_Server_Port(BluetoothStackID, Result);

                  printf("SYNC_Register_Server_SDP_Record() Failure.\r\n");

                  ret_val = FUNCTION_ERROR;
               }
            }
            else
            {
               /* There was an error opening the Server.                */
               printf("SYNC_Open_Server_Port() Failure: %d.\r\n", Result);

               ret_val = Result;
            }
         }
         else
         {
            /* A Server is already open, this program only supports one */
            /* Command Server at a time.                                */
            printf("SYNC Command Server already open.\r\n");

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         printf("Usage: OpenCommandServer [Port Number].\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      printf("Stack ID Invalid.\r\n");

      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for deleting a local SYNC   */
   /* Profile Command Server that was created via a successful call to  */
   /* the OpenCommandServer() function.  This function returns zero if  */
   /* successful and a negative value if an error occurred.             */
static int CloseCommandServer(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      if(CommandServerID)
      {
         /* SYNC Server opened, close any open SDP Record.              */
         if(CommandServerSDPRecordHandle)
         {
            SYNC_Un_Register_SDP_Record(BluetoothStackID, CommandServerID, CommandServerSDPRecordHandle);

            CommandServerSDPRecordHandle = 0;
         }

         /* Next attempt to close this Server.                          */
         Result = SYNC_Close_Server_Port(BluetoothStackID, CommandServerID);

         if(!Result)
         {
            /* Display a message indicating that the server was         */
            /* successfully closed.                                     */
            printf("SYNC_Close_Server_Port() Success.\r\n");

            CommandServerID = 0;

            ret_val         = 0;
         }
         else
         {
            /* An error occurred while attempting to close the server.  */
            printf("SYNC_Close_Server_Port() Failure: %d.\r\n", Result);

            ret_val = Result;
         }
      }
      else
      {
         /* No valid SYNC Command Server exists.                        */
         printf("No Server is currently open.\r\n");

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      printf("Stack ID Invalid.\r\n");

      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for initiating a connection */
   /* with a Remote SYNC Server.  This function returns zero if         */
   /* successful and a negative value if an error occurred.             */
static int OpenRemoteServer(ParameterList_t *TempParam)
{
   int       Result;
   int       ret_val;
   char      BoardStr[13];
   BD_ADDR_t NullADDR;

   ASSIGN_BD_ADDR(NullADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that a Client doesn't already exist.    */
      if(!SYNCID)
      {
         /* A Client port is not already open, now check to make sure   */
         /* that the parameters required for this function appear to be */
         /* valid.                                                      */
         if((TempParam) && (TempParam->NumberofParameters > 1) && (TempParam->Params[0].intParam) && (NumberofValidResponses) && (TempParam->Params[0].intParam<=NumberofValidResponses) && (!COMPARE_BD_ADDR(InquiryResultList[(TempParam->Params[0].intParam-1)], NullADDR)))
         {
            /* Now check to make sure that the Server Port ID passed to */
            /* this function is valid.                                  */
            if(TempParam->Params[1].intParam)
            {
               BD_ADDRToStr(InquiryResultList[(TempParam->Params[0].intParam-1)], BoardStr);

               /* The above parameters are valid, inform the user that  */
               /* the program is about to open a Remote SYNC Port.      */
               printf("Opening Remote SYNC Server (BD_ADDR = %s, Port = %04X)\r\n", BoardStr, TempParam->Params[1].intParam);

               /* Attempt to open a connection to the selected remote   */
               /* SYNC Profile Server.                                  */
               Result = SYNC_Open_Remote_Server_Port(BluetoothStackID, InquiryResultList[(TempParam->Params[0].intParam-1)], TempParam->Params[1].intParam, stIrMC_Server, SYNC_Event_Callback_Client, 0);

               /* Now check to see if the function call was successfully*/
               /* made.                                                 */
               if(Result > 0)
               {
                  /* The Client was opened successfully.  The return    */
                  /* value of the call is the SYNC Client ID and is     */
                  /* required for future SYNC Profile calls.            */
                  SYNCID  = Result;

                  ret_val = 0;

                  printf("SYNC_Open_Remote_Server_Port: Function Successful (ID = %04X).\r\n", SYNCID);
               }
               else
               {
                  /* There was an error opening the Client.             */
                  printf("SYNC_Open_Remote_Server_Port() Failure: %d.\r\n", Result);

                  ret_val = Result;
               }
            }
            else
            {
               printf("Usage: Connect [Inquiry Index] [Server Port].\r\n");

               ret_val = INVALID_PARAMETERS_ERROR;
            }
         }
         else
         {
            printf("Usage: Connect [Inquiry Index] [Server Port].\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* A Connection is already open, this program only supports one*/
         /* Server or Client at a time.                                 */
         printf("Port already open.\r\n");

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      printf("Stack ID Invalid.\r\n");

      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for terminating a connection*/
   /* with a remote SYNC Client or Server.  This function returns zero  */
   /* if successful and a negative value if an error occurred.          */
static int CloseConnection(ParameterList_t *TempParam)
{
   int ret_val;
   int Result;

   /* First check to see if a valid Bluetooth Stack exists.             */
   if(BluetoothStackID)
   {
      /* Check to see if the SYNC ID appears to be semi-valid.  This    */
      /* parameter will only be valid if a Connection is open or a      */
      /* Server is Open.                                                */
      if((SYNCID) && (Connected))
      {
         /* Free any allocated data buffers for ongoing transactions.   */
         if((CurrentOperation != coNone) && (CurrentBuffer))
         {
            free(CurrentBuffer);

            CurrentBuffer = NULL;
         }

         /* The Current SYNCID appears to be semi-valid.  Now try to    */
         /* close the connection.                                       */
         Result = SYNC_Close_Connection(BluetoothStackID, SYNCID);

         if(!Result)
         {
            /* The function was called successfully.  Display a message */
            /* indicating that the Connection Close was successfully    */
            /* submitted.                                               */
            printf("SYNC_Close_Connection: Function Successful.\r\n");

            Connected        = FALSE;
            CurrentOperation = coNone;

            if(IsClient)
               SYNCID = 0;

            ret_val          = 0;
         }
         else
         {
            /* An error occurred while attempting to close the          */
            /* Connection.                                              */
            printf("SYNC_Close_Connection() Failure: %d.\r\n", Result);

            ret_val = Result;
         }
      }
      else
      {
         /* The Current SYNC ID is invalid so no Connection or Server is*/
         /* open.                                                       */
         printf("Invalid SYNC ID: SYNC Close Connection.\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      printf("Stack ID Invalid.\r\n");

      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for issuing an Abort to a   */
   /* remote SYNC Server by issuing an OBEX Abort Command.  This        */
   /* function returns zero if successful and a negative value if an    */
   /* error occurred.                                                   */
static int Abort(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the SYNC ID appears to be          */
      /* semi-valid.                                                    */
      if(SYNCID)
      {
         /* The SYNC ID appears to be is a semi-valid value.  Now submit*/
         /* the command to begin this operation.                        */
         Result = SYNC_Abort_Request(BluetoothStackID, SYNCID);

         if(!Result)
         {
            /* The function was submitted successfully.                 */

            /* Update the Current Operation.                            */
            CurrentOperation = coAbort;

            ret_val          = 0;

            printf("SYNC_Abort_Request: Function Successful.\r\n");
         }
         else
         {
            /* There was an error submitting the function.              */
            printf("SYNC_Abort_Request() Failure: %d.\r\n", Result);

            ret_val = Result;
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("SYNC Abort Request: Invalid SYNC ID.\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      printf("Stack ID Invalid.\r\n");

      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is a utility function that exists to       */
   /* generate a unique LUID string based on the following input        */
   /* criteria.                                                         */
static Boolean_t GenerateLUIDString(IrMC_Object_Store_Type_t StoreType, char *LUID, DWord_t LUIDSize)
{
   Boolean_t     ret_val = FALSE;
   static Word_t PostFix = 0;

   if(LUID)
   {
      if(LUIDSize >= 16)
      {
         BTPS_SprintF(LUID, "%08X%04X%s", rand(), PostFix++, ObjectStreamExtensions[StoreType]);

         ret_val = TRUE;
      }
   }

   return(ret_val);
}

   /* The following function is responsible for issuing the request to  */
   /* Get an Object from a remote SYNC Server by issuing an OBEX GET    */
   /* Object Request.  This function returns zero if successful and a   */
   /* negative value if an error occurred.                              */
static int IrMCObjectGet(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the SYNC ID appears to be          */
      /* semi-valid.                                                    */
      if(SYNCID)
      {
         /* The SYNC ID appears to be is a semi-valid value.  Now check */
         /* to make sure that there is not already an outstanding       */
         /* operation in progress.                                      */
         if(CurrentOperation == coNone)
         {
            /* No current operation in progress, go ahead and see if the*/
            /* Object Name and Filename were specified (and are valid). */
            if((TempParam->NumberofParameters > 1) && (TempParam->Params[0].strParam) && (strlen(TempParam->Params[0].strParam)) && (TempParam->Params[1].strParam) && (strlen(TempParam->Params[1].strParam)))
            {
               /* Parameters appear to be semi-valid, go ahead and fetch*/
               /* the object.                                           */
               strcpy(CurrentFileName, TempParam->Params[1].strParam);

               /* First delete the file we are saving into.             */
               unlink(CurrentFileName);

               /* Issue request with selected options.                  */
               Result = SYNC_IrMC_Object_Get_Request(BluetoothStackID, SYNCID, TempParam->Params[0].strParam);

               if(Result >= 0)
               {
                  printf("SYNC_IrMC_Object_Get_Request() Successful.\r\n");

                  printf("Filename used to store results: %s\r\n", TempParam->Params[1].strParam);

                  CurrentOperation = coObjectGet;

                  ret_val          = 0;
               }
               else
               {
                  printf("SYNC_IrMC_Object_Get_Request() Failure %d.\r\n", Result);

                  ret_val = FUNCTION_ERROR;
               }
            }
            else
            {
               /* Invalid parameters specified.                         */
               printf("Usage: IrMCObjectGet [ObjectName] [Filename of Object to Save].\r\n");

               ret_val = INVALID_PARAMETERS_ERROR;
            }
         }
         else
         {
            /* Unable to submit new operation (one is already in        */
            /* progress).                                               */
            printf("Unable to Issue Get Object: Operation already in progress.\r\n");

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("IrMC Get Object: Invalid SYNC ID.\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      printf("Stack ID Invalid.\r\n");

      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for issuing the request to  */
   /* Put an Object to a remote SYNC Server by issuing an OBEX PUT      */
   /* Object Request.  This function returns zero if successful and a   */
   /* negative value if an error occurred.                              */
static int IrMCObjectPut(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the SYNC ID appears to be          */
      /* semi-valid.                                                    */
      if(SYNCID)
      {
         /* The SYNC ID appears to be is a semi-valid value.  Now check */
         /* to make sure that there is not already an outstanding       */
         /* operation in progress.                                      */
         if(CurrentOperation == coNone)
         {
            /* No current operation in progress, go ahead and see if the*/
            /* Object Name and Filename were specified (and are valid). */
            if((TempParam->NumberofParameters > 1) && (TempParam->Params[0].strParam) && (strlen(TempParam->Params[0].strParam)) && (TempParam->Params[1].strParam) && (strlen(TempParam->Params[1].strParam)))
            {
               /* Everything appears to be semi-valid, go ahead and read*/
               /* all of the file object that was specified into the    */
               /* Current Buffer.                                       */
               if(!ReadFileIntoCurrentBuffer(TempParam->Params[1].strParam))
               {
                  /* Object read successfully, go ahead and Push the    */
                  /* Object.                                            */
                  strcpy(CurrentFileName, TempParam->Params[1].strParam);

                  /* Issue request with selected options.               */
                  Result = SYNC_IrMC_Object_Put_Request(BluetoothStackID, SYNCID, TempParam->Params[0].strParam, CurrentBufferSize, CurrentBufferSize, (Byte_t *)CurrentBuffer, NULL, TRUE, &CurrentBufferSent);

                  if(!Result)
                  {
                     /* The function was submitted successfully.        */
                     printf("SYNC_IrMC_Object_Put_Request() Successful.\r\n");

                     printf("Filename used of object to put: %s\r\n", CurrentFileName);

                     CurrentOperation = coObjectPut;

                     ret_val          = 0;
                  }
                  else
                  {
                     /* There was an error submitting the function.     */
                     printf("SYNC_IrMC_Object_Put_Request() Failure %d.\r\n", Result);

                     free(CurrentBuffer);

                     CurrentBuffer = NULL;

                     ret_val       = FUNCTION_ERROR;
                  }
               }
               else
               {
                  printf("IrMC Object Put: Unable to read specified object file: %s.\r\n", TempParam->Params[1].strParam);

                  ret_val = INVALID_PARAMETERS_ERROR;
               }
            }
            else
            {
               /* Invalid parameters specified.                         */
               printf("Usage: IrMCObjectPut [Object Name] [Filename of Object to Put].\r\n");

               ret_val = INVALID_PARAMETERS_ERROR;
            }
         }
         else
         {
            /* Unable to submit new operation (one is already in        */
            /* progress).                                               */
            printf("Unable to Issue Put Object: Operation already in progress.\r\n");

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("IrMC Object Put: Invalid SYNC ID.\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      printf("Stack ID Invalid.\r\n");

      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for issuing the request to  */
   /* Get a Special Object from a remote SYNC Server by issuing an OBEX */
   /* GET Object Request.  This function returns zero if successful and */
   /* a negative value if an error occurred.                            */
static int IrMCSpecialGet(ParameterList_t *TempParam)
{
   int           Result;
   int           ret_val;
   IrMC_Anchor_t SyncAnchor;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the SYNC ID appears to be          */
      /* semi-valid.                                                    */
      if(SYNCID)
      {
         /* The SYNC ID appears to be is a semi-valid value.  Now check */
         /* to make sure that there is not already an outstanding       */
         /* operation in progress.                                      */
         if(CurrentOperation == coNone)
         {
            /* No current operation in progress, go ahead and see if the*/
            /* correct parameters were specified.                       */
            if((TempParam->NumberofParameters >= 5) && (TempParam->Params[3].strParam) && (strlen(TempParam->Params[3].strParam)) && (TempParam->Params[4].strParam) && (strlen(TempParam->Params[4].strParam)))
            {
               if((TempParam->Params[0].intParam >= 0) && (TempParam->Params[0].intParam <= (int)sotUnknown) && (TempParam->Params[1].intParam >= 0) && (TempParam->Params[1].intParam < osUnknown) && (TempParam->Params[2].intParam >= 0) && (TempParam->Params[2].intParam < (int)atNone))
               {
                  /* Parameters appear to be sem-valid, go ahead and map*/
                  /* the Sync Anchor to the correct format.             */
                  SyncAnchor.Anchor_Type = (IrMC_Anchor_Type_t)TempParam->Params[2].intParam;

                  if(TempParam->Params[2].intParam == (int)atChangeCounter)
                     SyncAnchor.Anchor_Data.ChangeCounter = TempParam->Params[3].intParam;
                  else
                     TimestampToTimeDate(TempParam->Params[3].strParam, &(SyncAnchor.Anchor_Data.Timestamp));

                  /* Parameters appear to be semi-valid, go ahead and   */
                  /* fetch the object.                                  */
                  strcpy(CurrentFileName, TempParam->Params[4].strParam);

                  /* First delete the file we are saving into.          */
                  unlink(CurrentFileName);

                  /* Issue request with selected options.               */
                  Result = SYNC_IrMC_Special_Object_Get_Request(BluetoothStackID, SYNCID, (IrMC_Object_Store_Type_t)TempParam->Params[1].intParam, (SYNC_Special_Object_Type_t)TempParam->Params[0].intParam, &SyncAnchor);

                  if(Result >= 0)
                  {
                     printf("SYNC_IrMC_Special_Object_Get_Request() Successful.\r\n");

                     printf("Filename used to store results: %s\r\n", CurrentFileName);

                     CurrentOperation = coSpecialObjectGet;

                     ret_val          = 0;
                  }
                  else
                  {
                     printf("SYNC_IrMC_Special_Object_Get_Request() Failure %d.\r\n", Result);

                     ret_val = FUNCTION_ERROR;
                  }
               }
               else
               {
                  /* Invalid parameters specified.                      */
                  printf("Usage: IrMCSpecialGet [ObjectType (0 = ChangeCounter, 1 = Change Log, 2 = Info Log, 3 = Device Info, 4 = RTC)] [Object Store Type (0 = Phonebook, 1 = Calendar, 2 = Notes, 3 = Inbox, 4 = Outbox, 5 = Sent box)] [Anchor Type (0 = Synch Anchor, 1 = Time Stamp)] [Synch Anchor] [Filename of Object to Save].\r\n");

                  ret_val = INVALID_PARAMETERS_ERROR;
               }
            }
            else
            {
               /* Invalid parameters specified.                         */
               printf("Usage: IrMCSpecialGet [ObjectType (0 = ChangeCounter, 1 = Change Log, 2 = Info Log, 3 = Device Info, 4 = RTC)] [Object Store Type (0 = Phonebook, 1 = Calendar, 2 = Notes, 3 = Inbox, 4 = Outbox, 5 = Sent box)] [Anchor Type (0 = Synch Anchor, 1 = Time Stamp)] [Synch Anchor] [Filename of Object to Save].\r\n");

               ret_val = INVALID_PARAMETERS_ERROR;
            }
         }
         else
         {
            /* Unable to submit new operation (one is already in        */
            /* progress).                                               */
            printf("Unable to Issue Get Object: Operation already in progress.\r\n");

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("IrMC Get Object: Invalid SYNC ID.\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      printf("Stack ID Invalid.\r\n");

      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for issuing the request to  */
   /* update the Real Time Clock (RTC) on a remote SYNC Server.  This   */
   /* function returns zero if successful and a negative value if an    */
   /* error occurred.                                                   */
static int IrMCRTCPut(ParameterList_t *TempParam)
{
   int             Result;
   int             ret_val;
   IrMC_TimeDate_t TimeDate;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the SYNC ID appears to be          */
      /* semi-valid.                                                    */
      if(SYNCID)
      {
         /* The SYNC ID appears to be is a semi-valid value.  Now check */
         /* to make sure that there is not already an outstanding       */
         /* operation in progress.                                      */
         if(CurrentOperation == coNone)
         {
            /* Determine the current Time and Date so that we can inform*/
            /* the Server of the new Time/Date.                         */
            PopulateCurrentTimeDate(&TimeDate);

            /* Simply submit the RTC Put.                               */
            Result = SYNC_IrMC_RTC_Put_Request(BluetoothStackID, SYNCID, &TimeDate);

            if(Result >= 0)
            {
               printf("SYNC_IrMC_RTC_Put_Request() Successful.\r\n");

               CurrentOperation = coRTCPut;

               ret_val          = 0;
            }
            else
            {
               printf("SYNC_IrMC_RTC_Put_Request() Failure %d.\r\n", Result);

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* Unable to submit new operation (one is already in        */
            /* progress).                                               */
            printf("Unable to Issue RTC Put: Operation already in progress.\r\n");

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("IrMC Object Delete: Invalid SYNC ID.\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      printf("Stack ID Invalid.\r\n");

      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for issuing the request to  */
   /* Delete an Object on a remote SYNC Server.  This function returns  */
   /* zero if successful and a negative value if an error occurred.     */
static int IrMCObjectDelete(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the SYNC ID appears to be          */
      /* semi-valid.                                                    */
      if(SYNCID)
      {
         /* The SYNC ID appears to be is a semi-valid value.  Now check */
         /* to make sure that there is not already an outstanding       */
         /* operation in progress.                                      */
         if(CurrentOperation == coNone)
         {
            /* No current operation in progress, go ahead and see if the*/
            /* Object Name was specified.                               */
            if((TempParam->NumberofParameters > 0) && (TempParam->Params[0].strParam) && (strlen(TempParam->Params[0].strParam)))
            {
               /* Issue request with selected options.                  */
               Result = SYNC_IrMC_Object_Delete_Request(BluetoothStackID, SYNCID, TempParam->Params[0].strParam, NULL, TRUE);

               if(Result >= 0)
               {
                  printf("SYNC_IrMC_Object_Delete_Request() Successful, Object: %s\r\n", TempParam->Params[0].strParam);

                  CurrentOperation = coObjectDelete;

                  ret_val          = 0;
               }
               else
               {
                  printf("SYNC_IrMC_Object_Delete_Request() Failure %d.\r\n", Result);

                  ret_val = FUNCTION_ERROR;
               }
            }
            else
            {
               /* Invalid parameters specified.                         */
               printf("Usage: IrMCObjectDelete [Object Name].\r\n");

               ret_val = INVALID_PARAMETERS_ERROR;
            }
         }
         else
         {
            /* Unable to submit new operation (one is already in        */
            /* progress).                                               */
            printf("Unable to Issue Delete Object: Operation already in progress.\r\n");

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("IrMC Object Delete: Invalid SYNC ID.\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      printf("Stack ID Invalid.\r\n");

      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for Clearing the current    */
   /* operation context for the Sync Server.  This will not close a     */
   /* connection but only clears any pending or partial operations and  */
   /* re-init the operation context.                                    */
static void ClearSYNCServerOperation(void)
{
   /* Initialize all the operation context storage to appropriate       */
   /* initialization values.                                            */
   SYNCServer.CurrentOperation              = coNone;
   SYNCServer.Operation.HardDelete          = FALSE;
   SYNCServer.Operation.Index               = 0;
   SYNCServer.Operation.MaxChangeCounter    = 0;
   SYNCServer.Operation.UseMaxChangeCounter = FALSE;
   SYNCServer.Operation.Name[0]             = '\0';
   SYNCServer.Operation.Level               = 0;
   SYNCServer.Operation.StoreType           = 0;
   SYNCServer.CurrentObjectDataIndex        = 0;

   /* If there is an object copy reference still allocated call the     */
   /* function to release this entry.                                   */
   if(SYNCServer.CurrentObject)
   {
      OBJSTORE_ReleaseEntry(SYNCServer.CurrentObject);

      SYNCServer.CurrentObject = NULL;
   }

   /* If a data buffer has been allocated then release this buffer.     */
   if(SYNCServer.Buffer)
   {
      free(SYNCServer.Buffer);

      SYNCServer.Buffer = NULL;
   }

   /* Reset buffer size and data size variables.                        */
   SYNCServer.BufferSize = 0;
   SYNCServer.DataSize   = 0;
}

   /* The following function is responsible for processing the IrMC     */
   /* Object Get Indication Event.                                      */
static void ProcessIrMCObjectGetIndication(SYNC_IrMC_Object_Get_Indication_Data_t *SYNC_IrMC_Object_Get_Indication_Data)
{
   int                  Ret;
   int                  Result;
   Boolean_t            GetRequired;
   Boolean_t            SendLength;
   unsigned int         AmountWritten;
   IrMC_Object_Entry_t *ObjectEntry;

   /* Attempt to make sure the Server Context is valid, as well as the  */
   /* input parameters.                                                 */
   if((SYNCServer.SYNCID) && (SYNC_IrMC_Object_Get_Indication_Data))
   {
      /* Do we currently have an ongoing OBEX operation?                */
      if(SYNCServer.CurrentOperation == coNone)
      {
         /* Flag that we are not to actually GET any data (haven't      */
         /* parsed the operation yet).                                  */
         GetRequired = FALSE;

         /* Start a new Object GET operation.                           */
         if((SYNC_IrMC_Object_Get_Indication_Data->ObjectName) && (strlen(SYNC_IrMC_Object_Get_Indication_Data->ObjectName)))
         {
            printf("ObjectName: %s\r\n", SYNC_IrMC_Object_Get_Indication_Data->ObjectName);

            /* First clear all state information - ensures that any     */
            /* previously allocated memory is released.                 */
            ClearSYNCServerOperation();

            /* Attempt to parse the object name into Store, Level,      */
            /* Parsed Name, and/or Index Value                          */
            if(SYNC_Parse_Object_Name(SYNC_IrMC_Object_Get_Indication_Data->ObjectName, &SYNCServer.Operation, FALSE))
            {
               printf("Level: %d Type: %d\r\n", (SYNCServer.Operation.Level + 1), SYNCServer.Operation.StoreType);

               printf("Name: %s Index: %lu\r\n", SYNCServer.Operation.Name, SYNCServer.Operation.Index);

               printf("Use MCC: %c MCC: %lu HD: %c\r\n", ((SYNCServer.Operation.UseMaxChangeCounter)?('Y'):('N')), SYNCServer.Operation.MaxChangeCounter, ((SYNCServer.Operation.HardDelete)?('Y'):('N')));

               /* We've successfully parsed the Name header.  Now       */
               /* attempt this requested GET operation on the Object    */
               /* Store.                                                */
               Ret = OBJSTORE_GetObject(&(ObjectStore[SYNCServer.Operation.StoreType]), &SYNCServer.Operation, &ObjectEntry);

               /* Were we able to locate the entry?                     */
               if((Ret == SYNC_OBEX_RESPONSE_OK) && (ObjectEntry))
               {
                  /* We have verified that everything is in place for   */
                  /* this to be a valid Object Get Request.  set the    */
                  /* current operation accordingly.                     */
                  SYNCServer.CurrentOperation = coObjectGet;

                  /* We were successful at locating this entry in the   */
                  /* object store database.  Next we will store our     */
                  /* local copy of this object so that we can use it to */
                  /* service multi-part requests if required.           */
                  SYNCServer.CurrentObject = ObjectEntry;

                  /* Reset the data index since we are starting to read */
                  /* from scratch.                                      */
                  SYNCServer.CurrentObjectDataIndex = 0;

                  /* Flag we need to actually process the GET operation.*/
                  GetRequired = TRUE;

                  /* Flag that we need to send the Length Header in the */
                  /* GET Operation processing.                          */
                  SendLength  = TRUE;
               }
               else
               {
                  printf("Bad Request, Object Not Found.\r\n");

                  /* Failed to locate the entry.  Return an error code. */
                  Result = SYNC_IrMC_Object_Get_Response(BluetoothStackID, SYNCServer.SYNCID, SYNC_OBEX_RESPONSE_NOT_FOUND, NULL, 0, 0, NULL, NULL);

                  ClearSYNCServerOperation();
               }
            }
            else
            {
               printf("Bad Request, Invalid Object Name specified (parse error).\r\n");

               Result = SYNC_IrMC_Object_Get_Response(BluetoothStackID, SYNCServer.SYNCID, SYNC_OBEX_RESPONSE_BAD_REQUEST, NULL, 0, 0, NULL, NULL);
            }
         }
         else
         {
            printf("Bad Request, Invalid Object Name specified.\r\n");

            Result = SYNC_IrMC_Object_Get_Response(BluetoothStackID, SYNCServer.SYNCID, SYNC_OBEX_RESPONSE_BAD_REQUEST, NULL, 0, 0, NULL, NULL);
         }
      }
      else
      {
         /* Another operation is on-going.  Check to see if this is a   */
         /* OBEX GET operation.                                         */
         if(SYNCServer.CurrentOperation == coObjectGet)
         {
            /* Flag we now need to process the PUT operation.           */
            GetRequired = TRUE;
            SendLength  = FALSE;
         }
         else
         {
            printf("Bad Request, Operation already in progress.\r\n");

            /* We have an on-going operation of a different type.  We   */
            /* must deny this incoming request.                         */
            Result = SYNC_IrMC_Object_Get_Response(BluetoothStackID, SYNCServer.SYNCID, SYNC_OBEX_RESPONSE_BAD_REQUEST, NULL, 0, 0, NULL, NULL);

            /* Flag that we do not need to continue to process the GET  */
            /* operation.                                               */
            GetRequired = FALSE;
         }
      }

      /* Check to see if we need to actually process the GET (either a  */
      /* continuation or start of operation).                           */
      if(GetRequired)
      {
         /* We have an object for to process.  Attempt to send the next */
         /* portion of data for this object based on the stored index.  */
         /* We must subtract our current position from the total data   */
         /* length for the BufferSize parameter.                        */
         Result = SYNC_IrMC_Object_Get_Response(BluetoothStackID, SYNCServer.SYNCID, SYNC_OBEX_RESPONSE_OK, NULL, ((SendLength)?(SYNCServer.CurrentObject->DataLength):(0)), ((SYNCServer.CurrentObject->DataLength) - (SYNCServer.CurrentObjectDataIndex)), (Byte_t *)(&SYNCServer.CurrentObject->Data[SYNCServer.CurrentObjectDataIndex]), &AmountWritten);

         /* Were we able to send all of the remaining data?             */
         if((!Result) && (AmountWritten == (SYNCServer.CurrentObject->DataLength - SYNCServer.CurrentObjectDataIndex)))
         {
            /* We were able to send all the remaining data and complete */
            /* this operation.  Release stored object and reset state   */
            /* variables.                                               */
            ClearSYNCServerOperation();
         }
         else
         {
            if(!Result)
            {
               /* Only a portion of the remaining data was sent.  This  */
               /* means a continuation response was substituted for OK. */
               /* We just need to update our index and wait for the next*/
               /* request.                                              */
               SYNCServer.CurrentObjectDataIndex += AmountWritten;
            }
            else
            {
               /* Error sending response.  Update the state.            */
               ClearSYNCServerOperation();
            }
         }
      }

      if(Result >= 0)
         printf("SYNC_IrMC_Object_Get_Response() Successful.\r\n");
      else
         printf("SYNC_IrMC_Object_Get_Response() Failure %d.\r\n", Result);
   }
   else
      printf("Invalid parameter or No longer connected.\r\n");
}

   /* The following function is responsible for processing the IrMC     */
   /* Object Put Indication Event.                                      */
static void ProcessIrMCObjectPutIndication(SYNC_IrMC_Object_Put_Indication_Data_t *SYNC_IrMC_Object_Put_Indication_Data)
{
   int            Result;
   Byte_t        *tmpBuffer;
   Byte_t         ResponseCode;
   Boolean_t      PutRequired;
   Boolean_t      AbortOperation;
   unsigned int   tmpBufferSize;
   IrMC_Anchor_t  SyncAnchor;

   /* Attempt to make sure the Server Context is valid, as well as the  */
   /* input parameters.                                                 */
   if((SYNCServer.SYNCID) && (SYNC_IrMC_Object_Put_Indication_Data))
   {
      /* Do we currently have an ongoing OBEX operation?                */
      if(SYNCServer.CurrentOperation == coNone)
      {
         /* Flag that we are not to actually PUT any data (haven't      */
         /* parsed the operation yet).                                  */
         PutRequired = FALSE;

         /* Start a new Object PUT operation.                           */
         if((SYNC_IrMC_Object_Put_Indication_Data->ObjectName) && (strlen(SYNC_IrMC_Object_Put_Indication_Data->ObjectName)))
         {
            printf("ObjectName: %s\r\n", SYNC_IrMC_Object_Put_Indication_Data->ObjectName);

            /* First clear all state information - ensures that any     */
            /* previously allocated memory is released.                 */
            ClearSYNCServerOperation();

            /* Attempt to parse the object name into Store, Level,      */
            /* Parsed Name, and/or Index Value                          */
            if(SYNC_Parse_Object_Name(SYNC_IrMC_Object_Put_Indication_Data->ObjectName, &SYNCServer.Operation, FALSE))
            {
               /* We have verified that this appears to be a valid      */
               /* Object Put operation.  Set the current operation      */
               /* variable and begin to populate operation structures.  */
               SYNCServer.CurrentOperation     = coObjectPut;

               /* Setup operation dependent parameters for this         */
               /* operation.                                            */
               SYNCServer.Operation.HardDelete = FALSE;

               printf("Level: %d Type: %d\r\n", (SYNCServer.Operation.Level + 1), SYNCServer.Operation.StoreType);

               printf("Name: %s Index: %lu\r\n", SYNCServer.Operation.Name, SYNCServer.Operation.Index);

               printf("Use MCC: %c MCC: %lu HD: %c\r\n", ((SYNCServer.Operation.UseMaxChangeCounter)?('Y'):('N')), SYNCServer.Operation.MaxChangeCounter, ((SYNCServer.Operation.HardDelete)?('Y'):('N')));

               /* Determine if the MaxChangeCounter value was present in*/
               /* this operation.                                       */
               if(SYNC_IrMC_Object_Put_Indication_Data->MaxChangeCounter)
               {
                  SYNCServer.Operation.MaxChangeCounter    = *(SYNC_IrMC_Object_Put_Indication_Data->MaxChangeCounter);
                  SYNCServer.Operation.UseMaxChangeCounter = TRUE;
               }
               else
                  SYNCServer.Operation.UseMaxChangeCounter = FALSE;

               /* Setup the data buffer to receive incoming data.  Do we*/
               /* have an length header to indicate the total object    */
               /* size we will receive in all combined requests?  If so,*/
               /* we will also make sure that ObjectLength is greater   */
               /* than the current BufferSize (if not, this is an       */
               /* invalid ObjectLength value).                          */
               if((SYNC_IrMC_Object_Put_Indication_Data->ObjectLength) && (SYNC_IrMC_Object_Put_Indication_Data->BufferSize <= SYNC_IrMC_Object_Put_Indication_Data->ObjectLength))
               {
                  /* We have a valid object length value, use this to   */
                  /* attempt to allocate a data buffer.  At this stage, */
                  /* if the allocation fails we will just leave the Data*/
                  /* ptr set to NULL.  We might make a second allocation*/
                  /* attempt, but we will eventually fail properly and  */
                  /* return an error response.                          */
                  SYNCServer.Buffer = malloc(SYNC_IrMC_Object_Put_Indication_Data->ObjectLength);

                  /* If we successfully allocated the buffer, set the   */
                  /* BufferSize property to track the allocated size    */
                  /* available, and the DataSize property to show the   */
                  /* amount of this buffer consumed (zero).             */
                  if(SYNCServer.Buffer)
                  {
                     SYNCServer.BufferSize = SYNC_IrMC_Object_Put_Indication_Data->ObjectLength;
                     SYNCServer.DataSize   = 0;
                  }

                  /* Flag we now need to process the PUT operation.     */
                  PutRequired = TRUE;
               }
               else
               {
                  printf("Bad Request, Empty Object.\r\n");

                  /* Failed to locate the entry.  Return an error code. */
                  Result = SYNC_IrMC_Object_Put_Response(BluetoothStackID, SYNCServer.SYNCID, SYNC_OBEX_RESPONSE_NOT_FOUND, NULL, NULL);

                  ClearSYNCServerOperation();
               }
            }
            else
            {
               printf("Bad Request, Invalid Object Name specified (parse error).\r\n");

               Result = SYNC_IrMC_Object_Put_Response(BluetoothStackID, SYNCServer.SYNCID, SYNC_OBEX_RESPONSE_BAD_REQUEST, NULL, NULL);
            }
         }
         else
         {
            printf("Bad Request, Invalid Object Name specified.\r\n");

            Result = SYNC_IrMC_Object_Put_Response(BluetoothStackID, SYNCServer.SYNCID, SYNC_OBEX_RESPONSE_BAD_REQUEST, NULL, NULL);
         }
      }
      else
      {
         /* Another operation is on-going.  Check to see if this is a   */
         /* OBEX PUT operation.                                         */
         if(SYNCServer.CurrentOperation == coObjectPut)
         {
            /* Flag we now need to process the PUT operation.           */
            PutRequired = TRUE;
         }
         else
         {
            printf("Bad Request, Operation already in progress.\r\n");

            /* We have an on-going operation of a different type.  We   */
            /* must deny this incoming request.                         */
            Result = SYNC_IrMC_Object_Put_Response(BluetoothStackID, SYNCServer.SYNCID, SYNC_OBEX_RESPONSE_BAD_REQUEST, NULL, NULL);

            /* Flag that we do not need to continue to process the PUT  */
            /* operation.                                               */
            PutRequired = FALSE;
         }
      }

      /* Check to see if we need to actually process the PUT (either a  */
      /* continuation or start of operation).                           */
      if(PutRequired)
      {
         /* Do we have a valid data segment in this continuation        */
         /* request?  Handle copying data into the current object.      */
         if((SYNC_IrMC_Object_Put_Indication_Data->Buffer) && (SYNC_IrMC_Object_Put_Indication_Data->BufferSize))
         {
            /* Flag that we have not encountered an error.              */
            AbortOperation = FALSE;

            /* Do we have a previously allocated buffer for this entry? */
            if((SYNCServer.Buffer) && (SYNCServer.BufferSize))
            {
               /* We have a previously allocated buffer, next determine */
               /* if this buffer has sufficient space for this data     */
               /* payload.                                              */
               if((SYNCServer.BufferSize - SYNCServer.DataSize) >= SYNC_IrMC_Object_Put_Indication_Data->BufferSize)
               {
                  /* There is space in the existing buffer to add the   */
                  /* remaining data from this packet.  Add and update   */
                  /* size parameters.                                   */
                  memcpy(&(SYNCServer.Buffer[SYNCServer.DataSize]), SYNC_IrMC_Object_Put_Indication_Data->Buffer, SYNC_IrMC_Object_Put_Indication_Data->BufferSize);

                  SYNCServer.DataSize += SYNC_IrMC_Object_Put_Indication_Data->BufferSize;
               }
               else
               {
                  /* Insufficient buffer space available.  Allocate an  */
                  /* alternate buffer that will hold the old and new    */
                  /* data together.                                     */
                  tmpBuffer = malloc(SYNCServer.DataSize + SYNC_IrMC_Object_Put_Indication_Data->BufferSize);
                  if(tmpBuffer)
                  {
                     /* Store new buffer size.                          */
                     tmpBufferSize = (SYNCServer.DataSize + SYNC_IrMC_Object_Put_Indication_Data->BufferSize);

                     /* Copy existing data to the new buffer.           */
                     memcpy(tmpBuffer, SYNCServer.Buffer, SYNCServer.DataSize);

                     /* Add new incoming data to the new buffer.        */
                     memcpy(&(tmpBuffer[SYNCServer.DataSize]), SYNC_IrMC_Object_Put_Indication_Data->Buffer, SYNC_IrMC_Object_Put_Indication_Data->BufferSize);

                     /* Next release the old buffer associated with the */
                     /* stored object.                                  */
                     free(SYNCServer.Buffer);

                     /* Set ptr to newly allocated buffer and setup size*/
                     /* parameters.                                     */
                     SYNCServer.Buffer     = tmpBuffer;
                     SYNCServer.DataSize   = tmpBufferSize;
                     SYNCServer.BufferSize = tmpBufferSize;
                  }
                  else
                  {
                     /* Error allocating new buffer.  We will have to   */
                     /* abort operation, release buffers, return an     */
                     /* error code, and exit.                           */
                     AbortOperation = TRUE;

                     /* Cleanup any state information for this          */
                     /* operation.                                      */
                     ClearSYNCServerOperation();
                  }
               }
            }
            else
            {
               /* No buffer allocated for this entry.  Attempt to       */
               /* allocate a buffer to handle this request only (the    */
               /* best we can do since we don't have an ObjectLength    */
               /* header at this stage).                                */
               SYNCServer.Buffer = malloc(SYNC_IrMC_Object_Put_Indication_Data->BufferSize);

               /* Was the allocation successful?                        */
               if(SYNCServer.Buffer)
               {
                  /* Allocation successful.  Copy incoming data to the  */
                  /* newly allocated buffer.                            */
                  memcpy(SYNCServer.Buffer, SYNC_IrMC_Object_Put_Indication_Data->Buffer, SYNC_IrMC_Object_Put_Indication_Data->BufferSize);

                  /* Update size parameters.                            */
                  SYNCServer.BufferSize = SYNC_IrMC_Object_Put_Indication_Data->BufferSize;
                  SYNCServer.DataSize   = SYNC_IrMC_Object_Put_Indication_Data->BufferSize;
               }
               else
               {
                  /* Error allocating new buffer.  We will have to abort*/
                  /* operation, release buffers, return an error code,  */
                  /* and exit.                                          */
                  AbortOperation = TRUE;

                  /* Cleanup any state information for this operation.  */
                  ClearSYNCServerOperation();
               }
            }

            /* Check to ensure that we did not fail any allocations.    */
            if(!AbortOperation)
            {
               /* Check final flag to determine if we need to send an OK*/
               /* or CONTINUE response.                                 */
               if(SYNC_IrMC_Object_Put_Indication_Data->Final)
               {
                  /* This is the final operation.  No further data will */
                  /* be sent.  Lets write this object entry back to the */
                  /* Object Store now.                                  */
                  ResponseCode = (Byte_t)(OBJSTORE_PutObject(&(ObjectStore[SYNCServer.Operation.StoreType]), &SYNCServer.Operation, SYNCServer.DataSize, SYNCServer.Buffer, &SyncAnchor));

                  if(ResponseCode == SYNC_OBEX_RESPONSE_OK)
                  {
                     /* Return succesful response code indicating object*/
                     /* PUT was successful.                             */
                     Result = SYNC_IrMC_Object_Put_Response(BluetoothStackID, SYNCServer.SYNCID, SYNC_OBEX_RESPONSE_OK, SYNCServer.Operation.Name, &SyncAnchor);
                  }
                  else
                  {
                     Result = SYNC_IrMC_Object_Put_Response(BluetoothStackID, SYNCServer.SYNCID, ResponseCode, NULL, NULL);
                  }

                  /* Clear and Reset all operation state info.          */
                  ClearSYNCServerOperation();
               }
               else
               {
                  /* This operation is still not completed.  Return a   */
                  /* request asking for continuation.                   */
                  Result = SYNC_IrMC_Object_Put_Response(BluetoothStackID, SYNCServer.SYNCID, SYNC_OBEX_RESPONSE_CONTINUE, NULL, NULL);
               }
            }
            else
            {
               printf("ERROR: Buffer Allocation Failed.\r\n");

               Result = SYNC_IrMC_Object_Put_Response(BluetoothStackID, SYNCServer.SYNCID, SYNC_OBEX_RESPONSE_INTERNAL_ERROR, NULL, NULL);

               /* Clear and Reset all operation state info.             */
               ClearSYNCServerOperation();
            }
         }
         else
         {
            /* Buffer pointer was NULL.  Nothing to write.              */
            Result = SYNC_IrMC_Object_Put_Response(BluetoothStackID, SYNCServer.SYNCID, (Byte_t)((SYNC_IrMC_Object_Put_Indication_Data->Final)?SYNC_OBEX_RESPONSE_OK:SYNC_OBEX_RESPONSE_CONTINUE), NULL, NULL);
         }
      }

      if(Result >= 0)
         printf("SYNC_IrMC_Object_Put_Response() Successful.\r\n");
      else
         printf("SYNC_IrMC_Object_Put_Response() Failure %d.\r\n", Result);
   }
   else
      printf("Invalid parameter or No longer connected.\r\n");
}

   /* The following function is responsible for processing the IrMC     */
   /* Special Object Get Indication Event.                              */
static void ProcessIrMCSpecialObjectGetIndication(SYNC_IrMC_Special_Object_Get_Indication_Data_t *SYNC_IrMC_Special_Object_Get_Indication_Data)
{
   int             Result;
   char            temp[1024];
   unsigned int    AmountWritten;
   IrMC_TimeDate_t TimeDate;

   /* Attempt to make sure the Server Context is valid, as well as the  */
   /* input parameters.                                                 */
   if((SYNCServer.SYNCID) && (SYNC_IrMC_Special_Object_Get_Indication_Data))
   {
      /* Do we currently have an ongoing OBEX operation?                */
      if(SYNCServer.CurrentOperation == coNone)
      {
         if((SYNC_IrMC_Special_Object_Get_Indication_Data->ObjectName) && (strlen(SYNC_IrMC_Special_Object_Get_Indication_Data->ObjectName)))
            printf("ObjectName: %s\r\n", SYNC_IrMC_Special_Object_Get_Indication_Data->ObjectName);
         else
            printf("ObjectName: NONE\r\n");

         printf("Store: %s (%d)\r\n", ObjectStoreTypeStrings[SYNC_IrMC_Special_Object_Get_Indication_Data->ObjectStoreType], SYNC_IrMC_Special_Object_Get_Indication_Data->ObjectStoreType);

         printf("Object: %s (%d)\r\n", ObjectTypeStrings[SYNC_IrMC_Special_Object_Get_Indication_Data->ObjectType], SYNC_IrMC_Special_Object_Get_Indication_Data->ObjectType);

         /* Determine the type of object requested and create output    */
         /* buffer containing the response.                             */
         switch(SYNC_IrMC_Special_Object_Get_Indication_Data->ObjectType)
         {
            case sotChangeCounter:
               /* Call Object Store function to write object to the     */
               /* buffer.                                               */
               Result = OBJSTORE_WriteChangeCounterToBuffer(&(ObjectStore[SYNC_IrMC_Special_Object_Get_Indication_Data->ObjectStoreType]), sizeof(temp), (Byte_t *)temp);
               break;
            case sotChangeLog:
               /* Call Object Store function to write object to the buffer.   */
               Result = OBJSTORE_WriteChangeLogToBuffer(&(ObjectStore[SYNC_IrMC_Special_Object_Get_Indication_Data->ObjectStoreType]), SYNC_IrMC_Special_Object_Get_Indication_Data->SyncAnchor, sizeof(temp), (Byte_t *)temp);
               break;
            case sotInfoLog:
               /* Call Object Store function to write object to the buffer.   */
               Result = OBJSTORE_WriteInfoLogToBuffer(&(ObjectStore[SYNC_IrMC_Special_Object_Get_Indication_Data->ObjectStoreType]), sizeof(temp), (Byte_t *)temp);
               break;
            case sotDeviceInfo:
               /* Call Object Store function to write object to the buffer.   */
               Result = OBJSTORE_WriteDevInfoToBuffer(&DefaultDeviceInfo, sizeof(temp), (Byte_t *)temp);
               break;
            case sotRTC:
               /* Store the current Time/Date (UTC) into TimeDate       */
               /* structure.                                            */
               PopulateCurrentTimeDate(&TimeDate);

               /* Convert TimeDate structure into string and store length of  */
               /* data written.                                               */
               TimestampToString(temp, &TimeDate);

               Result = SYNC_OBEX_RESPONSE_OK;
               break;
            case sotSyncCommand:
            case sotUnknown:
            default:
               Result = SYNC_OBEX_RESPONSE_BAD_REQUEST;
               break;
         }

         /* Examine return code and send response.                            */
         switch(SYNC_IrMC_Special_Object_Get_Indication_Data->ObjectType)
         {
            case sotChangeCounter:
            case sotChangeLog:
            case sotInfoLog:
            case sotRTC:
            case sotDeviceInfo:
               /* Check return value to see if buffer is valid.         */
               if(Result == SYNC_OBEX_RESPONSE_OK)
               {
                  printf("Special Get OK\r\n");

                  /* Send the response if call was successful.          */
                  Result = SYNC_IrMC_Special_Object_Get_Response(BluetoothStackID, SYNCServer.SYNCID, SYNC_OBEX_RESPONSE_OK, NULL, strlen(temp), strlen(temp), (Byte_t *)temp, &AmountWritten);

                  /* Check to see if were able to send all of the data. */
                  if(AmountWritten != strlen(temp))
                  {
                     /* All of the data wasn't able to be sent, go ahead*/
                     /* and note the remaining the data to send (and    */
                     /* change the mode).                               */
                     SYNCServer.Buffer = malloc(strlen(temp));

                     /* If we successfully allocated the buffer, set the*/
                     /* BufferSize property to track the allocated size */
                     /* available, and the DataSize property to show the*/
                     /* amount of this buffer consumed (zero).          */
                     if(SYNCServer.Buffer)
                     {
                        SYNCServer.BufferSize       = strlen(temp);
                        SYNCServer.DataSize         = AmountWritten;

                        SYNCServer.CurrentOperation = coSpecialObjectGet;
                     }
                     else
                     {
                        /* Cleanup sync server operation context.       */
                        ClearSYNCServerOperation();
                     }
                  }
               }
               else
               {
                  printf("Special Get Failed.\r\n");

                  /* Send an error response.                            */
                  Result = SYNC_IrMC_Special_Object_Get_Response(BluetoothStackID, SYNCServer.SYNCID, SYNC_OBEX_RESPONSE_INTERNAL_ERROR, NULL, 0, 0, NULL, NULL);

                  /* Cleanup sync server operation context.             */
                  ClearSYNCServerOperation();
               }
               break;
            case sotSyncCommand:
            case sotUnknown:
            default:
               printf("Special Get Unknown/Unsupported.\r\n");

               /* Unknown or Invalid Request Type.  Return Error        */
               /* response.                                             */
               Result = SYNC_IrMC_Special_Object_Get_Response(BluetoothStackID, SYNCServer.SYNCID, SYNC_OBEX_RESPONSE_BAD_REQUEST, NULL, 0, 0, NULL, NULL);

               /* Cleanup sync server operation context.                */
               ClearSYNCServerOperation();
               break;
         }
      }
      else
      {
         /* Another operation is on-going.  Check to see if this is a   */
         /* OBEX PUT operation.                                         */
         if(SYNCServer.CurrentOperation == coObjectDelete)
         {
            /* Send the response if call was successful.                */
            Result = SYNC_IrMC_Special_Object_Get_Response(BluetoothStackID, SYNCServer.SYNCID, SYNC_OBEX_RESPONSE_OK, NULL, SYNCServer.BufferSize, (SYNCServer.BufferSize - SYNCServer.DataSize), (Byte_t *)&(SYNCServer.Buffer[SYNCServer.DataSize]), &AmountWritten);

            /* Check to see if were able to send all of the data.       */
            if((SYNCServer.DataSize + AmountWritten) != SYNCServer.BufferSize)
               SYNCServer.DataSize += AmountWritten;
            else
            {
               /* Cleanup sync server operation context.                */
               ClearSYNCServerOperation();
            }
         }
         else
         {
            printf("Bad Request, Operation already in progress.\r\n");

            /* We have an on-going operation, we must deny this incoming*/
            /* request.                                                 */
            Result = SYNC_IrMC_Special_Object_Get_Response(BluetoothStackID, SYNCServer.SYNCID, SYNC_OBEX_RESPONSE_BAD_REQUEST, NULL, 0, 0, NULL, NULL);
         }
      }

      if(Result >= 0)
         printf("SYNC_IrMC_Special_Object_Get_Response() Successful.\r\n");
      else
         printf("SYNC_IrMC_Special_Object_Get_Response() Failure %d.\r\n", Result);
   }
   else
      printf("Invalid parameter or No longer connected.\r\n");
}

   /* The following function is responsible for processing the IrMC     */
   /* Object Delete Indication Event.                                   */
static void ProcessIrMCObjectDeleteIndication(SYNC_IrMC_Object_Delete_Indication_Data_t *SYNC_IrMC_Object_Delete_Indication_Data)
{
   int           Result;
   Byte_t        ResponseCode;
   IrMC_Anchor_t SyncAnchor;

   /* Attempt to make sure the Server Context is valid, as well as the  */
   /* input parameters.                                                 */
   if((SYNCServer.SYNCID) && (SYNC_IrMC_Object_Delete_Indication_Data))
   {
      /* Do we currently have an ongoing OBEX operation?                */
      if(SYNCServer.CurrentOperation == coNone)
      {
         /* Start a new Object Delete operation.                        */
         if((SYNC_IrMC_Object_Delete_Indication_Data->ObjectName) && (strlen(SYNC_IrMC_Object_Delete_Indication_Data->ObjectName)))
         {
            printf("ObjectName: %s\r\n", SYNC_IrMC_Object_Delete_Indication_Data->ObjectName);

            /* First clear all state information - ensures that any     */
            /* previously allocated memory is released.                 */
            ClearSYNCServerOperation();

            /* Attempt to parse the object name into Store, Level,      */
            /* Parsed Name, and/or Index Value                          */
            if(SYNC_Parse_Object_Name(SYNC_IrMC_Object_Delete_Indication_Data->ObjectName, &SYNCServer.Operation, FALSE))
            {
               /* Fill in remaining operation details.                  */
               SYNCServer.Operation.HardDelete = SYNC_IrMC_Object_Delete_Indication_Data->HardDelete;

               /* Determine if the MaxChangeCounter value was present in*/
               /* this operation.                                       */
               if(SYNC_IrMC_Object_Delete_Indication_Data->MaxChangeCounter)
               {
                  SYNCServer.Operation.MaxChangeCounter   = *(SYNC_IrMC_Object_Delete_Indication_Data->MaxChangeCounter);
                  SYNCServer.Operation.UseMaxChangeCounter = TRUE;
               }
               else
                  SYNCServer.Operation.UseMaxChangeCounter = FALSE;

               printf("Level: %d Type: %d\r\n", (SYNCServer.Operation.Level + 1), SYNCServer.Operation.StoreType);

               printf("Name: %s Index: %lu\r\n", SYNCServer.Operation.Name, SYNCServer.Operation.Index);

               printf("Use MCC: %c MCC: %lu HD: %c\r\n", ((SYNCServer.Operation.UseMaxChangeCounter)?('Y'):('N')), SYNCServer.Operation.MaxChangeCounter, ((SYNCServer.Operation.HardDelete)?('Y'):('N')));

               /* Because delete operations are always single request   */
               /* packets we can immediately attempt this operation     */
               /* against the object store.                             */
               ResponseCode = (Byte_t)(OBJSTORE_DeleteObject(&(ObjectStore[SYNCServer.Operation.StoreType]), &SYNCServer.Operation, &SyncAnchor));

               if(ResponseCode == SYNC_OBEX_RESPONSE_OK)
               {
                  printf("Completed Successfully (OK)\r\n");

                  /* Return succesful response code indicating object   */
                  /* PUT was successful.                                */
                  Result = SYNC_IrMC_Object_Delete_Response(BluetoothStackID, SYNCServer.SYNCID, SYNC_OBEX_RESPONSE_OK, SYNCServer.Operation.Name, &SyncAnchor);
               }
               else
               {
                  /* Finally return the correct Response Code (error).  */
                  Result = SYNC_IrMC_Object_Delete_Response(BluetoothStackID, SYNCServer.SYNCID, ResponseCode, NULL, NULL);
               }
            }
            else
            {
               printf("Bad Request, Invalid Object Name specified (parse error).\r\n");

               Result = SYNC_IrMC_Object_Delete_Response(BluetoothStackID, SYNCServer.SYNCID, SYNC_OBEX_RESPONSE_BAD_REQUEST, NULL, NULL);
            }

            /* Clear and Reset all operation state info.                */
            ClearSYNCServerOperation();
         }
         else
         {
            printf("Bad Request, Invalid Object Name specified.\r\n");

            Result = SYNC_IrMC_Object_Delete_Response(BluetoothStackID, SYNCServer.SYNCID, SYNC_OBEX_RESPONSE_BAD_REQUEST, NULL, NULL);
         }
      }
      else
      {
         printf("Bad Request, Operation already in progress.\r\n");

         /* We have an on-going operation, we must deny this incoming   */
         /* request.                                                    */
         Result = SYNC_IrMC_Object_Delete_Response(BluetoothStackID, SYNCServer.SYNCID, SYNC_OBEX_RESPONSE_BAD_REQUEST, NULL, NULL);
      }

      if(Result >= 0)
         printf("SYNC_IrMC_Object_Delete_Response() Successful.\r\n");
      else
         printf("SYNC_IrMC_Object_Delete_Response() Failure %d.\r\n", Result);
   }
   else
      printf("Invalid parameter or No longer connected.\r\n");
}

   /* The following function is responsible for Displaying the contents */
   /* of an SDP Service Attribute Response to the display.              */
static void DisplaySDPAttributeResponse(SDP_Service_Attribute_Response_Data_t *SDPServiceAttributeResponse, unsigned int InitLevel)
{
   int Index;

   /* First, check to make sure that there were Attributes returned.    */
   if(SDPServiceAttributeResponse->Number_Attribute_Values)
   {
      /* Loop through all returned SDP Attribute Values.                */
      for(Index = 0; Index < SDPServiceAttributeResponse->Number_Attribute_Values; Index++)
      {
         /* First Print the Attribute ID that was returned.             */
         printf("%*s Attribute ID 0x%04X\r\n", (InitLevel*INDENT_LENGTH), "", SDPServiceAttributeResponse->SDP_Service_Attribute_Value_Data[Index].Attribute_ID);

         /* Now Print out all of the SDP Data Elements that were        */
         /* returned that are associated with the SDP Attribute.        */
         DisplayDataElement(SDPServiceAttributeResponse->SDP_Service_Attribute_Value_Data[Index].SDP_Data_Element, (InitLevel + 1));
      }
   }
   else
      printf("No SDP Attributes Found.\r\n");
}

   /* The following function is responsible for displaying the contents */
   /* of an SDP Service Search Attribute Response to the display.       */
static void DisplaySDPSearchAttributeResponse(SDP_Service_Search_Attribute_Response_Data_t *SDPServiceSearchAttributeResponse)
{
   int Index;

   /* First, check to see if Service Records were returned.             */
   if(SDPServiceSearchAttributeResponse->Number_Service_Records)
   {
      /* Loop through all returned SDP Service Records.                 */
      for(Index = 0; Index < SDPServiceSearchAttributeResponse->Number_Service_Records; Index++)
      {
         /* First display the number of SDP Service Records we are      */
         /* currently processing.                                       */
         printf("Service Record: %u:\r\n", (Index + 1));

         /* Call Display SDPAttributeResponse for all SDP Service       */
         /* Records received.                                           */
         DisplaySDPAttributeResponse(&(SDPServiceSearchAttributeResponse->SDP_Service_Attribute_Response_Data[Index]), 1);
      }
   }
   else
      printf("No SDP Service Records Found.\r\n");
}

   /* The following function is responsible for actually displaying an  */
   /* individual SDP Data Element to the Display.  The Level Parameter  */
   /* is used in conjunction with the defined INDENT_LENGTH constant to */
   /* make readability easier when displaying Data Element Sequences    */
   /* and Data Element Alternatives.  This function will recursively    */
   /* call itself to display the contents of Data Element Sequences and */
   /* Data Element Alternatives when it finds these Data Types (and     */
   /* increments the Indent Level accordingly).                         */
static void DisplayDataElement(SDP_Data_Element_t *SDPDataElement, unsigned int Level)
{
   unsigned int Index;
   char         Buffer[256];

   switch(SDPDataElement->SDP_Data_Element_Type)
   {
      case deNIL:
         /* Display the NIL Type.                                       */
         printf("%*s Type: NIL\r\n", (Level*INDENT_LENGTH), "");
         break;
      case deNULL:
         /* Display the NULL Type.                                      */
         printf("%*s Type: NULL\r\n", (Level*INDENT_LENGTH), "");
         break;
      case deUnsignedInteger1Byte:
         /* Display the Unsigned Integer (1 Byte) Type.                 */
         printf("%*s Type: Unsigned Int = 0x%02X\r\n", (Level*INDENT_LENGTH), "", SDPDataElement->SDP_Data_Element.UnsignedInteger1Byte);
         break;
      case deUnsignedInteger2Bytes:
         /* Display the Unsigned Integer (2 Bytes) Type.                */
         printf("%*s Type: Unsigned Int = 0x%04X\r\n", (Level*INDENT_LENGTH), "", SDPDataElement->SDP_Data_Element.UnsignedInteger2Bytes);
         break;
      case deUnsignedInteger4Bytes:
         /* Display the Unsigned Integer (4 Bytes) Type.                */
         printf("%*s Type: Unsigned Int = 0x%08X\r\n", (Level*INDENT_LENGTH), "", (unsigned int)SDPDataElement->SDP_Data_Element.UnsignedInteger4Bytes);
         break;
      case deUnsignedInteger8Bytes:
         /* Display the Unsigned Integer (8 Bytes) Type.                */
         printf("%*s Type: Unsigned Int = 0x%02X%02X%02X%02X%02X%02X%02X%02X\r\n", (Level*INDENT_LENGTH), "", SDPDataElement->SDP_Data_Element.UnsignedInteger8Bytes[7],
                                                                                   SDPDataElement->SDP_Data_Element.UnsignedInteger8Bytes[6],
                                                                                   SDPDataElement->SDP_Data_Element.UnsignedInteger8Bytes[5],
                                                                                   SDPDataElement->SDP_Data_Element.UnsignedInteger8Bytes[4],
                                                                                   SDPDataElement->SDP_Data_Element.UnsignedInteger8Bytes[3],
                                                                                   SDPDataElement->SDP_Data_Element.UnsignedInteger8Bytes[2],
                                                                                   SDPDataElement->SDP_Data_Element.UnsignedInteger8Bytes[1],
                                                                                   SDPDataElement->SDP_Data_Element.UnsignedInteger8Bytes[0]);
         break;
      case deUnsignedInteger16Bytes:
         /* Display the Unsigned Integer (16 Bytes) Type.               */
         printf("%*s Type: Unsigned Int = 0x%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X\r\n", (Level*INDENT_LENGTH), "", SDPDataElement->SDP_Data_Element.UnsignedInteger16Bytes[15],
                                                                                                                   SDPDataElement->SDP_Data_Element.UnsignedInteger16Bytes[14],
                                                                                                                   SDPDataElement->SDP_Data_Element.UnsignedInteger16Bytes[13],
                                                                                                                   SDPDataElement->SDP_Data_Element.UnsignedInteger16Bytes[12],
                                                                                                                   SDPDataElement->SDP_Data_Element.UnsignedInteger16Bytes[11],
                                                                                                                   SDPDataElement->SDP_Data_Element.UnsignedInteger16Bytes[10],
                                                                                                                   SDPDataElement->SDP_Data_Element.UnsignedInteger16Bytes[9],
                                                                                                                   SDPDataElement->SDP_Data_Element.UnsignedInteger16Bytes[8],
                                                                                                                   SDPDataElement->SDP_Data_Element.UnsignedInteger16Bytes[7],
                                                                                                                   SDPDataElement->SDP_Data_Element.UnsignedInteger16Bytes[6],
                                                                                                                   SDPDataElement->SDP_Data_Element.UnsignedInteger16Bytes[5],
                                                                                                                   SDPDataElement->SDP_Data_Element.UnsignedInteger16Bytes[4],
                                                                                                                   SDPDataElement->SDP_Data_Element.UnsignedInteger16Bytes[3],
                                                                                                                   SDPDataElement->SDP_Data_Element.UnsignedInteger16Bytes[2],
                                                                                                                   SDPDataElement->SDP_Data_Element.UnsignedInteger16Bytes[1],
                                                                                                                   SDPDataElement->SDP_Data_Element.UnsignedInteger16Bytes[0]);
         break;
      case deSignedInteger1Byte:
         /* Display the Signed Integer (1 Byte) Type.                   */
         printf("%*s Type: Signed Int = 0x%02X\r\n", (Level*INDENT_LENGTH), "", SDPDataElement->SDP_Data_Element.SignedInteger1Byte);
         break;
      case deSignedInteger2Bytes:
         /* Display the Signed Integer (2 Bytes) Type.                  */
         printf("%*s Type: Signed Int = 0x%04X\r\n", (Level*INDENT_LENGTH), "", SDPDataElement->SDP_Data_Element.SignedInteger2Bytes);
         break;
      case deSignedInteger4Bytes:
         /* Display the Signed Integer (4 Bytes) Type.                  */
         printf("%*s Type: Signed Int = 0x%08X\r\n", (Level*INDENT_LENGTH), "", (unsigned int)SDPDataElement->SDP_Data_Element.SignedInteger4Bytes);
         break;
      case deSignedInteger8Bytes:
         /* Display the Signed Integer (8 Bytes) Type.                  */
         printf("%*s Type: Signed Int = 0x%02X%02X%02X%02X%02X%02X%02X%02X\r\n", (Level*INDENT_LENGTH), "", SDPDataElement->SDP_Data_Element.SignedInteger8Bytes[7],
                                                                                 SDPDataElement->SDP_Data_Element.SignedInteger8Bytes[6],
                                                                                 SDPDataElement->SDP_Data_Element.SignedInteger8Bytes[5],
                                                                                 SDPDataElement->SDP_Data_Element.SignedInteger8Bytes[4],
                                                                                 SDPDataElement->SDP_Data_Element.SignedInteger8Bytes[3],
                                                                                 SDPDataElement->SDP_Data_Element.SignedInteger8Bytes[2],
                                                                                 SDPDataElement->SDP_Data_Element.SignedInteger8Bytes[1],
                                                                                 SDPDataElement->SDP_Data_Element.SignedInteger8Bytes[0]);
         break;
      case deSignedInteger16Bytes:
         /* Display the Signed Integer (16 Bytes) Type.                 */
         printf("%*s Type: Signed Int = 0x%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X\r\n", (Level*INDENT_LENGTH), "", SDPDataElement->SDP_Data_Element.SignedInteger16Bytes[15],
                                                                                                                 SDPDataElement->SDP_Data_Element.SignedInteger16Bytes[14],
                                                                                                                 SDPDataElement->SDP_Data_Element.SignedInteger16Bytes[13],
                                                                                                                 SDPDataElement->SDP_Data_Element.SignedInteger16Bytes[12],
                                                                                                                 SDPDataElement->SDP_Data_Element.SignedInteger16Bytes[11],
                                                                                                                 SDPDataElement->SDP_Data_Element.SignedInteger16Bytes[10],
                                                                                                                 SDPDataElement->SDP_Data_Element.SignedInteger16Bytes[9],
                                                                                                                 SDPDataElement->SDP_Data_Element.SignedInteger16Bytes[8],
                                                                                                                 SDPDataElement->SDP_Data_Element.SignedInteger16Bytes[7],
                                                                                                                 SDPDataElement->SDP_Data_Element.SignedInteger16Bytes[6],
                                                                                                                 SDPDataElement->SDP_Data_Element.SignedInteger16Bytes[5],
                                                                                                                 SDPDataElement->SDP_Data_Element.SignedInteger16Bytes[4],
                                                                                                                 SDPDataElement->SDP_Data_Element.SignedInteger16Bytes[3],
                                                                                                                 SDPDataElement->SDP_Data_Element.SignedInteger16Bytes[2],
                                                                                                                 SDPDataElement->SDP_Data_Element.SignedInteger16Bytes[1],
                                                                                                                 SDPDataElement->SDP_Data_Element.SignedInteger16Bytes[0]);
         break;
      case deTextString:
         /* First retrieve the Length of the Text String so that we can */
         /* copy the Data into our Buffer.                              */
         Index = (SDPDataElement->SDP_Data_Element_Length < sizeof(Buffer))?SDPDataElement->SDP_Data_Element_Length:(sizeof(Buffer)-1);

         /* Copy the Text String into the Buffer and then NULL terminate*/
         /* it.                                                         */
         memcpy(Buffer, SDPDataElement->SDP_Data_Element.TextString, Index);
         Buffer[Index] = '\0';

         printf("%*s Type: Text String = %s\r\n", (Level*INDENT_LENGTH), "", Buffer);
         break;
      case deBoolean:
         printf("%*s Type: Boolean = %s\r\n", (Level*INDENT_LENGTH), "", (SDPDataElement->SDP_Data_Element.Boolean)?"TRUE":"FALSE");
         break;
      case deURL:
         /* First retrieve the Length of the URL String so that we can  */
         /* copy the Data into our Buffer.                              */
         Index = (SDPDataElement->SDP_Data_Element_Length < sizeof(Buffer))?SDPDataElement->SDP_Data_Element_Length:(sizeof(Buffer)-1);

         /* Copy the URL String into the Buffer and then NULL terminate */
         /* it.                                                         */
         memcpy(Buffer, SDPDataElement->SDP_Data_Element.URL, Index);
         Buffer[Index] = '\0';

         printf("%*s Type: URL = %s\r\n", (Level*INDENT_LENGTH), "", Buffer);
         break;
      case deUUID_16:
         printf("%*s Type: UUID_16 = 0x%02X%02X\r\n", (Level*INDENT_LENGTH), "", SDPDataElement->SDP_Data_Element.UUID_16.UUID_Byte0,
                                                                                 SDPDataElement->SDP_Data_Element.UUID_16.UUID_Byte1);
         break;
      case deUUID_32:
         printf("%*s Type: UUID_32 = 0x%02X%02X%02X%02X\r\n", (Level*INDENT_LENGTH), "", SDPDataElement->SDP_Data_Element.UUID_32.UUID_Byte0,
                                                                                         SDPDataElement->SDP_Data_Element.UUID_32.UUID_Byte1,
                                                                                         SDPDataElement->SDP_Data_Element.UUID_32.UUID_Byte2,
                                                                                         SDPDataElement->SDP_Data_Element.UUID_32.UUID_Byte3);
         break;
      case deUUID_128:
         printf("%*s Type: UUID_128 = 0x%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X\r\n", (Level*INDENT_LENGTH), "", SDPDataElement->SDP_Data_Element.UUID_128.UUID_Byte0,
                                                                                                               SDPDataElement->SDP_Data_Element.UUID_128.UUID_Byte1,
                                                                                                               SDPDataElement->SDP_Data_Element.UUID_128.UUID_Byte2,
                                                                                                               SDPDataElement->SDP_Data_Element.UUID_128.UUID_Byte3,
                                                                                                               SDPDataElement->SDP_Data_Element.UUID_128.UUID_Byte4,
                                                                                                               SDPDataElement->SDP_Data_Element.UUID_128.UUID_Byte5,
                                                                                                               SDPDataElement->SDP_Data_Element.UUID_128.UUID_Byte6,
                                                                                                               SDPDataElement->SDP_Data_Element.UUID_128.UUID_Byte7,
                                                                                                               SDPDataElement->SDP_Data_Element.UUID_128.UUID_Byte8,
                                                                                                               SDPDataElement->SDP_Data_Element.UUID_128.UUID_Byte9,
                                                                                                               SDPDataElement->SDP_Data_Element.UUID_128.UUID_Byte10,
                                                                                                               SDPDataElement->SDP_Data_Element.UUID_128.UUID_Byte11,
                                                                                                               SDPDataElement->SDP_Data_Element.UUID_128.UUID_Byte12,
                                                                                                               SDPDataElement->SDP_Data_Element.UUID_128.UUID_Byte13,
                                                                                                               SDPDataElement->SDP_Data_Element.UUID_128.UUID_Byte14,
                                                                                                               SDPDataElement->SDP_Data_Element.UUID_128.UUID_Byte15);
         break;
      case deSequence:
         /* Display that this is a SDP Data Element Sequence.           */
         printf("%*s Type: Data Element Sequence\r\n", (Level*INDENT_LENGTH), "");

         /* Loop through each of the SDP Data Elements in the SDP Data  */
         /* Element Sequence.                                           */
         for(Index = 0; Index < SDPDataElement->SDP_Data_Element_Length; Index++)
         {
            /* Call this function again for each of the SDP Data        */
            /* Elements in this SDP Data Element Sequence.              */
            DisplayDataElement(&(SDPDataElement->SDP_Data_Element.SDP_Data_Element_Sequence[Index]), (Level + 1));
         }
         break;
      case deAlternative:
         /* Display that this is a SDP Data Element Alternative.        */
         printf("%*s Type: Data Element Alternative\r\n", (Level*INDENT_LENGTH), "");

         /* Loop through each of the SDP Data Elements in the SDP Data  */
         /* Element Alternative.                                        */
         for(Index = 0; Index < SDPDataElement->SDP_Data_Element_Length; Index++)
         {
            /* Call this function again for each of the SDP Data        */
            /* Elements in this SDP Data Element Alternative.           */
            DisplayDataElement(&(SDPDataElement->SDP_Data_Element.SDP_Data_Element_Alternative[Index]), (Level + 1));
         }
         break;
      default:
         printf("%*s Unknown SDP Data Element Type\r\n", (Level*INDENT_LENGTH), "");
         break;
   }
}

   /*********************************************************************/
   /*                         Event Callbacks                           */
   /*********************************************************************/

   /* The following function is for the GAP Event Receive Data Callback.*/
   /* This function will be called whenever a Callback has been         */
   /* registered for the specified GAP Action that is associated with   */
   /* the Bluetooth Stack.  This function passes to the caller the GAP  */
   /* Event Data of the specified Event and the GAP Event Callback      */
   /* Parameter that was specified when this Callback was installed.    */
   /* The caller is free to use the contents of the GAP Event Data ONLY */
   /* in the context of this callback.  If the caller requires the Data */
   /* for a longer period of time, then the callback function MUST copy */
   /* the data into another Data Buffer.  This function is guaranteed   */
   /* NOT to be invoked more than once simultaneously for the specified */
   /* installed callback (i.e. this function DOES NOT have be           */
   /* reentrant).  It Needs to be noted however, that if the same       */
   /* Callback is installed more than once, then the callbacks will be  */
   /* called serially.  Because of this, the processing in this function*/
   /* should be as efficient as possible.  It should also be noted that */
   /* this function is called in the Thread Context of a Thread that the*/
   /* User does NOT own.  Therefore, processing in this function should */
   /* be as efficient as possible (this argument holds anyway because   */
   /* other GAP Events will not be processed while this function call is*/
   /* outstanding).                                                     */
   /* * NOTE * This function MUST NOT Block and wait for events that    */
   /*          can only be satisfied by Receiving other GAP Events.  A  */
   /*          Deadlock WILL occur because NO GAP Event Callbacks will  */
   /*          be issued while this function is currently outstanding.  */
static void BTPSAPI GAP_Event_Callback(unsigned int BluetoothStackID, GAP_Event_Data_t *GAP_Event_Data, unsigned long CallbackParameter)
{
   int                               Result;
   int                               Index;
   char                              BoardStr[13];
   unsigned long                     Data_Type;
   unsigned char                    *Device_Name;
   BD_ADDR_t                         NULL_BD_ADDR;
   Boolean_t                         OOB_Data;
   Boolean_t                         MITM;
   GAP_IO_Capability_t               RemoteIOCapability;
   GAP_Inquiry_Event_Data_t         *GAP_Inquiry_Event_Data;
   GAP_Remote_Name_Event_Data_t     *GAP_Remote_Name_Event_Data;
   GAP_Authentication_Information_t  GAP_Authentication_Information;

   /* First, check to see if the required parameters appear to be       */
   /* semi-valid.                                                       */
   if((BluetoothStackID) && (GAP_Event_Data))
   {
      printf("\r\n");

      /* The parameters appear to be semi-valid, now check to see what  */
      /* type the incoming event is.                                    */
      switch(GAP_Event_Data->Event_Data_Type)
      {
         case etInquiry_Result:
            /* The GAP event received was of type Inquiry_Result.       */
            GAP_Inquiry_Event_Data = GAP_Event_Data->Event_Data.GAP_Inquiry_Event_Data;

            /* Next, Check to see if the inquiry event data received    */
            /* appears to be semi-valid.                                */
            if(GAP_Inquiry_Event_Data)
            {
               /* The inquiry event data received appears to be         */
               /* semi-valid.                                           */
               printf("GAP Inquiry Result: %2d Found\r\n", GAP_Inquiry_Event_Data->Number_Devices);

               /* Now, check to see if the gap inquiry event data's     */
               /* inquiry data appears to be semi-valid.                */
               if(GAP_Inquiry_Event_Data->GAP_Inquiry_Data)
               {
                  /* Display a list of all the devices found from       */
                  /* performing the inquiry.                            */
                  for(Index=0;(Index<GAP_Inquiry_Event_Data->Number_Devices) && (Index<MAX_INQUIRY_RESULTS);Index++)
                  {
                     InquiryResultList[Index] = GAP_Inquiry_Event_Data->GAP_Inquiry_Data[Index].BD_ADDR;
                     BD_ADDRToStr(GAP_Inquiry_Event_Data->GAP_Inquiry_Data[Index].BD_ADDR, BoardStr);

                     printf("GAP Inquiry Result: %2d %s\r\n", (Index+1), BoardStr);
                  }

                  NumberofValidResponses = GAP_Inquiry_Event_Data->Number_Devices;
               }
            }
            break;
         case etInquiry_Entry_Result:
            /* Next convert the BD_ADDR to a string.                    */
            BD_ADDRToStr(GAP_Event_Data->Event_Data.GAP_Inquiry_Entry_Event_Data->BD_ADDR, BoardStr);

            /* Display this GAP Inquiry Entry Result.                   */
            printf("GAP Inquiry Entry Result - %s\r\n", BoardStr);
            break;
         case etInquiry_With_RSSI_Entry_Result:
            /* Next convert the BD_ADDR to a string.                    */
            BD_ADDRToStr(GAP_Event_Data->Event_Data.GAP_Inquiry_Entry_Event_Data->BD_ADDR, BoardStr);

            /* Display this GAP Inquiry Entry Result.                   */
            printf("GAP Inquiry Entry Result - %s RSSI: %4d\r\n", BoardStr, GAP_Event_Data->Event_Data.GAP_Inquiry_With_RSSI_Entry_Event_Data->RSSI);
            break;
         case etExtended_Inquiry_Entry_Result:
            /* Next convert the BD_ADDR to a string.                    */
            BD_ADDRToStr(GAP_Event_Data->Event_Data.GAP_Inquiry_Entry_Event_Data->BD_ADDR, BoardStr);

            /* Search for the local name element in the EIR data.       */
            Device_Name = NULL;
            for(Index=0;(Index<GAP_Event_Data->Event_Data.GAP_Extended_Inquiry_Entry_Event_Data->Extended_Inquiry_Response_Data.Number_Data_Entries);Index++)
            {
               Data_Type = (unsigned long)GAP_Event_Data->Event_Data.GAP_Extended_Inquiry_Entry_Event_Data->Extended_Inquiry_Response_Data.Data_Entries[Index].Data_Type;

               if((Data_Type == HCI_EXTENDED_INQUIRY_RESPONSE_DATA_TYPE_LOCAL_NAME_SHORTENED) || Data_Type == HCI_EXTENDED_INQUIRY_RESPONSE_DATA_TYPE_LOCAL_NAME_COMPLETE)
               {
                  Device_Name = GAP_Event_Data->Event_Data.GAP_Extended_Inquiry_Entry_Event_Data->Extended_Inquiry_Response_Data.Data_Entries[Index].Data_Buffer;
                  break;
               }
            }

            /* Display this GAP Inquiry Entry Result.                   */
            if(Device_Name)
               printf("GAP Inquiry Entry Result - %s RSSI: %4d Device Name: %s\r\n", BoardStr, GAP_Event_Data->Event_Data.GAP_Inquiry_With_RSSI_Entry_Event_Data->RSSI, Device_Name);
            else
               printf("GAP Inquiry Entry Result - %s RSSI: %4d\r\n", BoardStr, GAP_Event_Data->Event_Data.GAP_Inquiry_With_RSSI_Entry_Event_Data->RSSI);

            break;
         case etAuthentication:
            /* An authentication event occurred, determine which type of*/
            /* authentication event occurred.                           */
            switch(GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->GAP_Authentication_Event_Type)
            {
               case atLinkKeyRequest:
                  BD_ADDRToStr(GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device, BoardStr);
                  printf("atLinkKeyRequest: %s\r\n", BoardStr);

                  /* Setup the authentication information response      */
                  /* structure.                                         */
                  GAP_Authentication_Information.GAP_Authentication_Type    = atLinkKey;
                  GAP_Authentication_Information.Authentication_Data_Length = 0;

                  /* See if we have stored a Link Key for the specified */
                  /* device.                                            */
                  for(Index=0;Index<(sizeof(LinkKeyInfo)/sizeof(LinkKeyInfo_t));Index++)
                  {
                     if(COMPARE_BD_ADDR(LinkKeyInfo[Index].BD_ADDR, GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device))
                     {
                        /* Link Key information stored, go ahead and    */
                        /* respond with the stored Link Key.            */
                        GAP_Authentication_Information.Authentication_Data_Length   = sizeof(Link_Key_t);
                        GAP_Authentication_Information.Authentication_Data.Link_Key = LinkKeyInfo[Index].LinkKey;

                        break;
                     }
                  }

                  /* Submit the authentication response.                */
                  Result = GAP_Authentication_Response(BluetoothStackID, GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device, &GAP_Authentication_Information);

                  /* Check the result of the submitted command.         */
                  if(!Result)
                     printf("GAP_Authentication_Response() Success.\r\n");
                  else
                     printf("GAP_Authentication_Response() Failure: %d.\r\n", Result);
                  break;
               case atPINCodeRequest:
                  /* A pin code request event occurred, first display   */
                  /* the BD_ADD of the remote device requesting the pin.*/
                  BD_ADDRToStr(GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device, BoardStr);
                  printf("atPINCodeRequest: %s\r\n", BoardStr);

                  /* Note the current Remote BD_ADDR that is requesting */
                  /* the PIN Code.                                      */
                  CurrentRemoteBD_ADDR = GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device;

                  /* Inform the user that they will need to respond with*/
                  /* a PIN Code Response.                               */
                  printf("\r\nRespond with the command: PINCodeResponse\r\n");
                  break;
               case atAuthenticationStatus:
                  /* An authentication status event occurred, display   */
                  /* all relevant information.                          */
                  BD_ADDRToStr(GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device, BoardStr);
                  printf("atAuthenticationStatus: %d Board: %s\r\n", GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Authentication_Event_Data.Authentication_Status, BoardStr);

                  /* Flag that there is no longer a current             */
                  /* Authentication procedure in progress.              */
                  ASSIGN_BD_ADDR(CurrentRemoteBD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
                  break;
               case atLinkKeyCreation:
                  /* A link key creation event occurred, first display  */
                  /* the remote device that caused this event.          */
                  BD_ADDRToStr(GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device, BoardStr);
                  printf("atLinkKeyCreation: %s\r\n", BoardStr);

                  /* The BD_ADDR of the remote device has been displayed*/
                  /* now display the link key being created.            */
                  printf("Link Key: 0x");

                  for(Index = 0;Index<sizeof(Link_Key_t);Index++)
                     printf("%02X", ((Byte_t *)(&(GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Authentication_Event_Data.Link_Key_Info.Link_Key)))[Index]);

                  printf("\r\n");

                  /* Now store the link Key in either a free location OR*/
                  /* over the old key location.                         */
                  ASSIGN_BD_ADDR(NULL_BD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

                  for(Index=0,Result=-1;Index<(sizeof(LinkKeyInfo)/sizeof(LinkKeyInfo_t));Index++)
                  {
                     if(COMPARE_BD_ADDR(LinkKeyInfo[Index].BD_ADDR, GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device))
                        break;
                     else
                     {
                        if((Result == (-1)) && (COMPARE_BD_ADDR(LinkKeyInfo[Index].BD_ADDR, NULL_BD_ADDR)))
                           Result = Index;
                     }
                  }

                  /* If we didn't find a match, see if we found an empty*/
                  /* location.                                          */
                  if(Index == (sizeof(LinkKeyInfo)/sizeof(LinkKeyInfo_t)))
                     Index = Result;

                  /* Check to see if we found a location to store the   */
                  /* Link Key information into.                         */
                  if(Index != (-1))
                  {
                     LinkKeyInfo[Index].BD_ADDR = GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device;
                     LinkKeyInfo[Index].LinkKey = GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Authentication_Event_Data.Link_Key_Info.Link_Key;

                     printf("Link Key Stored locally.\r\n");
                  }
                  else
                     printf("Link Key NOT Stored locally: Link Key array is full.\r\n");
                  break;
               case atIOCapabilityRequest:
                  BD_ADDRToStr(GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device, BoardStr);
                  printf("atIOCapabilityRequest: %s\r\n", BoardStr);

                  /* Setup the Authentication Information Response      */
                  /* structure.                                         */
                  BTPS_MemInitialize(&(GAP_Authentication_Information.Authentication_Data.IO_Capabilities), 0, GAP_IO_CAPABILITIES_SIZE);

                  /* Setup the Authentication Information Response      */
                  /* structure.                                         */
                  GAP_Authentication_Information.GAP_Authentication_Type                                      = atIOCapabilities;
                  GAP_Authentication_Information.Authentication_Data_Length                                   = sizeof(GAP_IO_Capabilities_t);
                  GAP_Authentication_Information.Authentication_Data.IO_Capabilities.IO_Capability            = (GAP_IO_Capability_t)IOCapability;
                  GAP_Authentication_Information.Authentication_Data.IO_Capabilities.MITM_Protection_Required = MITMProtection;
                  GAP_Authentication_Information.Authentication_Data.IO_Capabilities.OOB_Data_Present         = OOBSupport;

                  /* Submit the Authentication Response.                */
                  Result = GAP_Authentication_Response(BluetoothStackID, GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device, &GAP_Authentication_Information);

                  /* Check the result of the submitted command.         */
                  /* Check the result of the submitted command.         */
                  if(!Result)
                     printf("GAP_Authentication_Response() Success.\r\n");
                  else
                     printf("GAP_Authentication_Response() Failure: %d.\r\n", Result);
                  break;
               case atIOCapabilityResponse:
                  BD_ADDRToStr(GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device, BoardStr);
                  printf("atIOCapabilityResponse: %s\r\n", BoardStr);

                  RemoteIOCapability = (int)GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Authentication_Event_Data.IO_Capabilities.IO_Capability;
                  MITM               = (Boolean_t)GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Authentication_Event_Data.IO_Capabilities.MITM_Protection_Required;
                  OOB_Data           = (Boolean_t)GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Authentication_Event_Data.IO_Capabilities.OOB_Data_Present;

                  printf("Remote Capabilities: %s%s%s\r\n", IOCapabilitiesStrings[RemoteIOCapability], ((MITM)?", MITM":""), ((OOB_Data)?", OOB Data":""));
                  break;
               case atUserConfirmationRequest:
                  BD_ADDRToStr(GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device, BoardStr);
                  printf("atUserConfirmationRequest: %s\r\n", BoardStr);

                  CurrentRemoteBD_ADDR = GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device;

                  if(IOCapability != icDisplayYesNo)
                  {
                     /* Invoke JUST Works Process...                    */
                     GAP_Authentication_Information.GAP_Authentication_Type          = atUserConfirmation;
                     GAP_Authentication_Information.Authentication_Data_Length       = (Byte_t)sizeof(Byte_t);
                     GAP_Authentication_Information.Authentication_Data.Confirmation = TRUE;

                     /* Submit the Authentication Response.             */
                     printf("\r\nAuto Accepting: %lu\r\n", GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Authentication_Event_Data.Numeric_Value);

                     Result = GAP_Authentication_Response(BluetoothStackID, GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device, &GAP_Authentication_Information);

                     if(!Result)
                        printf("GAP_Authentication_Response() Success.\r\n");
                     else
                        printf("GAP_Authentication_Response() Failure: %d.\r\n", Result);

                     /* Flag that there is no longer a current          */
                     /* Authentication procedure in progress.           */
                     ASSIGN_BD_ADDR(CurrentRemoteBD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
                  }
                  else
                  {
                     printf("User Confirmation: %lu\r\n", GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Authentication_Event_Data.Numeric_Value);

                     /* Inform the user that they will need to respond  */
                     /* with a PIN Code Response.                       */
                     printf("\r\nRespond with the command: UserConfirmationResponse\r\n");
                  }
                  break;
               case atPasskeyRequest:
                  BD_ADDRToStr(GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device, BoardStr);
                  printf("atPasskeyRequest: %s\r\n", BoardStr);

                  /* Note the current Remote BD_ADDR that is requesting */
                  /* the Passkey.                                       */
                  CurrentRemoteBD_ADDR = GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device;

                  /* Inform the user that they will need to respond with*/
                  /* a Passkey Response.                                */
                  printf("\r\nRespond with the command: PassKeyResponse\r\n");
                  break;
               case atRemoteOutOfBandDataRequest:
                  BD_ADDRToStr(GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device, BoardStr);
                  printf("atRemoteOutOfBandDataRequest: %s\r\n", BoardStr);

                  /* This application does not support OOB data so      */
                  /* respond with a data length of Zero to force a      */
                  /* negative reply.                                    */
                  GAP_Authentication_Information.GAP_Authentication_Type    = atOutOfBandData;
                  GAP_Authentication_Information.Authentication_Data_Length = 0;

                  Result = GAP_Authentication_Response(BluetoothStackID, GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device, &GAP_Authentication_Information);

                  if(!Result)
                     printf("GAP_Authentication_Response() Success.\r\n");
                  else
                     printf("GAP_Authentication_Response() Failure: %d.\r\n", Result);
                  break;
               case atPasskeyNotification:
                  BD_ADDRToStr(GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device, BoardStr);
                  printf("atPasskeyNotification: %s\r\n", BoardStr);

                  printf("Passkey Value: %lu\r\n", GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Authentication_Event_Data.Numeric_Value);
                  break;
               case atKeypressNotification:
                  BD_ADDRToStr(GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device, BoardStr);
                  printf("atKeypressNotification: %s\r\n", BoardStr);

                  printf("Keypress: %d\r\n", (int)GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Authentication_Event_Data.Keypress_Type);
                  break;
               default:
                  printf("Un-handled GAP Authentication Event.\r\n");
                  break;
            }
            break;
         case etRemote_Name_Result:
            /* Bluetooth Stack has responded to a previously issued     */
            /* Remote Name Request that was issued.                     */
            GAP_Remote_Name_Event_Data = GAP_Event_Data->Event_Data.GAP_Remote_Name_Event_Data;
            if(GAP_Remote_Name_Event_Data)
            {
               /* Inform the user of the Result.                        */
               BD_ADDRToStr(GAP_Remote_Name_Event_Data->Remote_Device, BoardStr);

               printf("GAP Remote Name Result: BD_ADDR: %s.\r\n", BoardStr);

               if(GAP_Remote_Name_Event_Data->Remote_Name)
                  printf("GAP Remote Name Result: %s.\r\n", GAP_Remote_Name_Event_Data->Remote_Name);
               else
                  printf("GAP Remote Name Result: NULL.\r\n");
            }
            break;
         default:
            /* An unknown/unexpected GAP event was received.            */
            printf("Unknown/Unhandled GAP Event: %d.\n", GAP_Event_Data->Event_Data_Type);
            break;
      }

      if(IsClient)
         printf("\r\nClient>");
      else
         printf("\r\nServer>");
   }
   else
   {
      /* There was an error with one or more of the input parameters.   */
      printf("\r\n");

      printf("GAP Callback Data: Event_Data = NULL.\r\n");

      if(IsClient)
         printf("\r\nClient>");
      else
         printf("\r\nServer>");
   }

   /* Make sure the output is displayed to the user.                    */
   fflush(stdout);
}

   /* The following function is for the SDP Event Receive Data Callback.*/
   /* This function will be called whenever a Callback has been         */
   /* registered for the specified SDP Action that is associated with   */
   /* the Bluetooth Stack.  This function passes to the caller the SDP  */
   /* Request ID of the SDP Request, the SDP Response Event Data of the */
   /* specified Response Event and the SDP Response Event Callback      */
   /* Parameter that was specified when this Callback was installed.    */
   /* The caller is free to use the contents of the SDP Event Data ONLY */
   /* in the context of this callback.  If the caller requires the Data */
   /* for a longer period of time, then the callback function MUST copy */
   /* the data into another Data Buffer.  This function is guaranteed   */
   /* NOT to be invoked more than once simultaneously for the specified */
   /* installed callback (i.e. this function DOES NOT have be           */
   /* reentrant).  It Needs to be noted however, that if the same       */
   /* Callback is installed more than once, then the callbacks will be  */
   /* called serially.  Because of this, the processing in this function*/
   /* should be as efficient as possible.  It should also be noted that */
   /* this function is called in the Thread Context of a Thread that the*/
   /* User does NOT own.  Therefore, processing in this function should */
   /* be as efficient as possible (this argument holds anyway because   */
   /* other SDP Events will not be processed while this function call is*/
   /* outstanding).                                                     */
   /* * NOTE * This function MUST NOT Block and wait for events that    */
   /*          can only be satisfied by Receiving other SDP Events.  A  */
   /*          Deadlock WILL occur because NO SDP Event Callbacks will  */
   /*          be issued while this function is currently outstanding.  */
static void BTPSAPI SDP_Event_Callback(unsigned int BluetoothStackID, unsigned int SDPRequestID, SDP_Response_Data_t *SDP_Response_Data, unsigned long CallbackParameter)
{
   int Index;

   /* First, check to see if the required parameters appear to be       */
   /* semi-valid.                                                       */
   if((SDP_Response_Data != NULL) && (BluetoothStackID))
   {
      /* The parameters appear to be semi-valid, now check to see what  */
      /* type the incoming Event is.                                    */
      switch(SDP_Response_Data->SDP_Response_Data_Type)
      {
         case rdTimeout:
            /* A SDP Timeout was received, display a message indicating */
            /* this.                                                    */
            printf("\r\n");
            printf("SDP Timeout Received (Size = 0x%04X).\r\n", sizeof(SDP_Response_Data_t));
            break;
         case rdConnectionError:
            /* A SDP Connection Error was received, display a message   */
            /* indicating this.                                         */
            printf("\r\n");
            printf("SDP Connection Error Received (Size = 0x%04X).\r\n", sizeof(SDP_Response_Data_t));
            break;
         case rdErrorResponse:
            /* A SDP error response was received, display all relevant  */
            /* information regarding this event.                        */
            printf("\r\n");
            printf("SDP Error Response Received (Size = 0x%04X) - Error Code: %d.\r\n", sizeof(SDP_Response_Data_t), SDP_Response_Data->SDP_Response_Data.SDP_Error_Response_Data.Error_Code);
            break;
         case rdServiceSearchResponse:
            /* A SDP Service Search Response was received, display all  */
            /* relevant information regarding this event                */
            printf("\r\n");
            printf("SDP Service Search Response Received (Size = 0x%04X) - Record Count: %d\r\n", sizeof(SDP_Response_Data_t), SDP_Response_Data->SDP_Response_Data.SDP_Service_Search_Response_Data.Total_Service_Record_Count);

            /* First, check to see if any SDP Service Records were      */
            /* found.                                                   */
            if(SDP_Response_Data->SDP_Response_Data.SDP_Service_Search_Response_Data.Total_Service_Record_Count)
            {
               printf("Record Handles:\r\n");

               for(Index = 0; (Word_t)Index < SDP_Response_Data->SDP_Response_Data.SDP_Service_Search_Response_Data.Total_Service_Record_Count; Index++)
               {
                  printf("Record %u: 0x%08X\r\n", (Index + 1), (unsigned int)SDP_Response_Data->SDP_Response_Data.SDP_Service_Search_Response_Data.Service_Record_List[Index]);
               }
            }
            else
               printf("No SDP Service Records Found.\r\n");
            break;
         case rdServiceAttributeResponse:
            /* A SDP Service Attribute Response was received, display   */
            /* all relevant information regarding this event            */
            printf("\r\n");
            printf("SDP Service Attribute Response Received (Size = 0x%04X)\r\n", sizeof(SDP_Response_Data_t));

            DisplaySDPAttributeResponse(&SDP_Response_Data->SDP_Response_Data.SDP_Service_Attribute_Response_Data, 0);
            break;
         case rdServiceSearchAttributeResponse:
            /* A SDP Service Search Attribute Response was received,    */
            /* display all relevant information regarding this event    */
            printf("\r\n");
            printf("SDP Service Search Attribute Response Received (Size = 0x%04X)\r\n", sizeof(SDP_Response_Data_t));

            DisplaySDPSearchAttributeResponse(&SDP_Response_Data->SDP_Response_Data.SDP_Service_Search_Attribute_Response_Data);
            break;
         default:
            /* An unknown/unexpected SDP event was received.            */
            printf("\r\n");
            printf("Unknown SDP Event.\r\n");
            break;
      }
   }
   else
   {
      /* There was an error with one or more of the input parameters.   */
      printf("\r\n");
      printf("SDP callback data: Response_Data = NULL.\r\n");
   }

   if(IsClient)
      printf("\r\nClient>");
   else
      printf("\r\nServer>");

   /* Make sure the output is displayed to the user.                    */
   fflush(stdout);
}

   /* The following function represents the OBEX SYNC Profile Server    */
   /* Event Callback.  This function will be called whenever a SYNC     */
   /* Server Event occurs that is associated with the specified         */
   /* Bluetooth Stack ID.  This function takes as its parameters the    */
   /* Bluetooth Stack ID, the SYNC Event Data that occurred and the SYNC*/
   /* Profile Event Callback Parameter that was specified when this     */
   /* Callback was installed.  The caller is free to use the contents of*/
   /* the SYNC Event Data ONLY in the context of this callback.  If the */
   /* caller requires the Data for a longer period of time, then the    */
   /* callback function MUST copy the data into another Data Buffer.    */
   /* This function is guaranteed NOT to be invoked more than once      */
   /* simultaneously for the specified installed callback (i.e. this    */
   /* function DOES NOT have be reentrant).  It Needs to be noted       */
   /* however, that if the same Callback is installed more than once,   */
   /* then the callbacks will be called serially.  Because of this, the */
   /* processing in this function should be as efficient as possible.   */
   /* It should also be noted that this function is called in the Thread*/
   /* Context of a Thread that the User does NOT own.  Therefore,       */
   /* processing in this function should be as efficient as possible    */
   /* (this argument holds anyway because another SYNC Profile Event    */
   /* will not be processed while this function call is outstanding).   */
   /* * NOTE * This function MUST NOT Block and wait for events that can*/
   /*          only be satisfied by Receiving other SYNC Events.  A     */
   /*          Deadlock WILL occur because NO SYNC Event Callbacks will */
   /*          be issued while this function is currently outstanding.  */
static void BTPSAPI SYNC_Event_Callback_Server(unsigned int BluetoothStackID, SYNC_Event_Data_t *SYNCEventData, unsigned long CallbackParameter)
{
   int  Result;
   char temp[128];
   char BoardStr[13];

   /* Make sure that parameters passed in appear to be at least         */
   /* semi-valid.                                                       */
   if((BluetoothStackID) && (SYNCEventData))
   {
      printf("\r\n");

      switch(SYNCEventData->Event_Data_Type)
      {
         case etSYNC_Open_Port_Indication:
            printf("Open Port Indication\r\n");
            BD_ADDRToStr(SYNCEventData->Event_Data.SYNC_Open_Port_Indication_Data->BD_ADDR, BoardStr);
            printf("BD_ADDR: %s\r\n", BoardStr);

            /* Clear any outstanding operation information.             */
            ClearSYNCServerOperation();

            /* Flag that we are connected.                              */
            SYNCServer.Connected = TRUE;
            Connected            = TRUE;
            break;
         case etSYNC_Close_Port_Indication:
            printf("Close Port Indication\r\n");

            /* Clear any outstanding operation information.             */
            ClearSYNCServerOperation();

            /* Flag that we are no longer connected.                    */
            SYNCServer.Connected = FALSE;
            Connected            = FALSE;
            break;
         case etSYNC_IrMC_Object_Get_Indication:
            printf("IrMC Object Get Indication\r\n");

            ProcessIrMCObjectGetIndication(SYNCEventData->Event_Data.SYNC_IrMC_Object_Get_Indication_Data);
            break;
         case etSYNC_IrMC_Object_Put_Indication:
            printf("IrMC Object Put Indication\r\n");

            ProcessIrMCObjectPutIndication(SYNCEventData->Event_Data.SYNC_IrMC_Object_Put_Indication_Data);
            break;
         case etSYNC_IrMC_Special_Object_Get_Indication:
            printf("IrMC Special Object Get Indication\r\n");

            ProcessIrMCSpecialObjectGetIndication(SYNCEventData->Event_Data.SYNC_IrMC_Special_Object_Get_Indication_Data);
            break;
         case etSYNC_IrMC_Object_Delete_Indication:
            printf("IrMC Object Delete Indication\r\n");

            ProcessIrMCObjectDeleteIndication(SYNCEventData->Event_Data.SYNC_IrMC_Object_Delete_Indication_Data);
            break;
         case etSYNC_IrMC_RTC_Put_Indication:
            printf("RTC Put Indication (Not Supported in implementation).\r\n");

            Result = SYNC_IrMC_RTC_Put_Response(BluetoothStackID, SYNCID, SYNC_OBEX_RESPONSE_BAD_REQUEST);

            if(Result >= 0)
               printf("SYNC_IrMC_RTC_Put_Response() Successful.\r\n");
            else
               printf("SYNC_IrMC_RTC_Put_Response() Failure %d.\r\n", Result);
            break;
         case etSYNC_Abort_Indication:
            printf("Abort Indication\r\n");

            /* Clear any outstanding operation information.             */
            ClearSYNCServerOperation();
            break;
         case etSYNC_IrMC_Sync_Command_Indication:
            /* Determine if we need to truncate the string.             */
            if(SYNCEventData->Event_Data.SYNC_IrMC_Sync_Command_Indication_Data->BufferSize >= sizeof(temp))
            {
               SYNCEventData->Event_Data.SYNC_IrMC_Sync_Command_Indication_Data->BufferSize = (sizeof(temp) - 1);
            }

            /* Copy to our temp buffer and display.                     */
            memcpy(temp, SYNCEventData->Event_Data.SYNC_IrMC_Sync_Command_Indication_Data->Buffer, SYNCEventData->Event_Data.SYNC_IrMC_Sync_Command_Indication_Data->BufferSize);
            temp[SYNCEventData->Event_Data.SYNC_IrMC_Sync_Command_Indication_Data->BufferSize] = '\0';

            /* No action is taken automatically.  User will perform     */
            /* Sync.                                                    */
            printf("IrMC Sync Command Indication: %s\r\n", temp);
            break;
         default:
            /* Unhandled Event.                                         */
            printf("Unexpected (Unhandled) Event %d.\r\n", SYNCEventData->Event_Data_Type);
            break;
      }

      printf("Server>");

      /* Make sure the output is displayed to the user.                 */
      fflush(stdout);
   }
}

   /* The following function represents the OBEX SYNC Profile Client    */
   /* Event Callback.  This function will be called whenever a SYNC     */
   /* Client Event occurs that is associated with the specified         */
   /* Bluetooth Stack ID.  This function takes as its parameters the    */
   /* Bluetooth Stack ID, the SYNC Event Data that occurred and the SYNC*/
   /* Profile Event Callback Parameter that was specified when this     */
   /* Callback was installed.  The caller is free to use the contents of*/
   /* the SYNC Event Data ONLY in the context of this callback.  If the */
   /* caller requires the Data for a longer period of time, then the    */
   /* callback function MUST copy the data into another Data Buffer.    */
   /* This function is guaranteed NOT to be invoked more than once      */
   /* simultaneously for the specified installed callback (i.e. this    */
   /* function DOES NOT have be reentrant).  It Needs to be noted       */
   /* however, that if the same Callback is installed more than once,   */
   /* then the callbacks will be called serially.  Because of this, the */
   /* processing in this function should be as efficient as possible.   */
   /* It should also be noted that this function is called in the Thread*/
   /* Context of a Thread that the User does NOT own.  Therefore,       */
   /* processing in this function should be as efficient as possible    */
   /* (this argument holds anyway because another SYNC Profile Event    */
   /* will not be processed while this function call is outstanding).   */
   /* * NOTE * This function MUST NOT Block and wait for events that can*/
   /*          only be satisfied by Receiving other SYNC Events.  A     */
   /*          Deadlock WILL occur because NO SYNC Event Callbacks will */
   /*          be issued while this function is currently outstanding.  */
static void BTPSAPI SYNC_Event_Callback_Client(unsigned int BluetoothStackID, SYNC_Event_Data_t *SYNCEventData, unsigned long CallbackParameter)
{
   int          Result;
   char         Buffer[128];
   unsigned int Index;
   unsigned int TempLength;

   /* Make sure that parameters passed in appear to be at least         */
   /* semi-valid.                                                       */
   if((BluetoothStackID) && (SYNCEventData))
   {
      printf("\r\n");

      switch(SYNCEventData->Event_Data_Type)
      {
         case etSYNC_Open_Port_Confirmation:
            printf("Open Port Confirmation\r\n");

            printf("Response Code: 0x%02X\r\n", SYNCEventData->Event_Data.SYNC_Open_Port_Confirmation_Data->SYNCConnectStatus);

            /* If this confirmation indicates failure we need to clear  */
            /* SYNCID.                                                  */
            if(SYNCEventData->Event_Data.SYNC_Open_Port_Confirmation_Data->SYNCConnectStatus)
            {
               SYNCID    = 0;
               Connected = FALSE;
            }

            /* Reset the current operation value.                       */
            CurrentOperation = coNone;
            break;
         case etSYNC_Close_Port_Indication:
            printf("Close Port Indication\r\n");

            /* Free any allocated data buffers for ongoing transactions.*/
            if((CurrentOperation != coNone) && (CurrentBuffer))
            {
               free(CurrentBuffer);

               CurrentBuffer = NULL;
            }

            /* Reset appropriate internal state info.                   */
            SYNCID           = 0;
            Connected        = FALSE;
            CurrentOperation = coNone;
            break;
         case etSYNC_IrMC_Object_Get_Confirmation:
            printf("IrMC Object Get Confirmation\r\n");

            printf("Response Code: 0x%02X\r\n", SYNCEventData->Event_Data.SYNC_IrMC_Object_Get_Confirmation_Data->ResponseCode);
            printf("Name: %s\r\n", SYNCEventData->Event_Data.SYNC_IrMC_Object_Get_Confirmation_Data->ObjectName?SYNCEventData->Event_Data.SYNC_IrMC_Object_Get_Confirmation_Data->ObjectName:"");
            printf("Length: %lu\r\n", SYNCEventData->Event_Data.SYNC_IrMC_Object_Get_Confirmation_Data->ObjectLength);
            printf("Final: %d\r\n", SYNCEventData->Event_Data.SYNC_IrMC_Object_Get_Confirmation_Data->Final);
            printf("Data: \r\n");

            for(Index=0;(Index<SYNCEventData->Event_Data.SYNC_IrMC_Object_Get_Confirmation_Data->BufferSize) && (Index < (sizeof(Buffer) - 1));Index++)
               printf("%c", SYNCEventData->Event_Data.SYNC_IrMC_Object_Get_Confirmation_Data->Buffer[Index]);

            printf("\r\n");

            /* If this is a successful GET then pass to routine to      */
            /* handle file I/O and further requests (if required).      */
            if((SYNCEventData->Event_Data.SYNC_IrMC_Object_Get_Confirmation_Data->ResponseCode == SYNC_OBEX_RESPONSE_CONTINUE) || (SYNCEventData->Event_Data.SYNC_IrMC_Object_Get_Confirmation_Data->ResponseCode == SYNC_OBEX_RESPONSE_OK))
               WriteReceivedData(SYNCEventData->Event_Data.SYNC_IrMC_Object_Get_Confirmation_Data->BufferSize, SYNCEventData->Event_Data.SYNC_IrMC_Object_Get_Confirmation_Data->Buffer);

            /* Determine if we need to issue another request to continue*/
            /* the operation.                                           */
            if(!SYNCEventData->Event_Data.SYNC_IrMC_Object_Get_Confirmation_Data->Final)
            {
               /* Display Results.                                      */
               printf("IrMC Object Get not complete, Requesting more data.\r\n");

               /* Continue the Object Get request.  Note that most      */
               /* parameters passed are ignored on a continuation call. */
               Result = SYNC_IrMC_Object_Get_Request(BluetoothStackID, SYNCID, NULL);

               if(Result >= 0)
               {
                  printf("SYNC_IrMC_Object_Get_Request() Successful.\r\n");

                  CurrentOperation = coObjectGet;
               }
               else
               {
                  printf("SYNC_IrMC_Object_Get_Request() Failure %d.\r\n", Result);

                  CurrentOperation = coNone;
               }
            }
            else
            {
               /* Set current operation.                                */
               CurrentOperation = coNone;

               printf("IrMC Object Get Completed.\r\n");
            }
            break;
         case etSYNC_IrMC_Special_Object_Get_Confirmation:
            printf("IrMC Special Object Get Confirmation\r\n");

            printf("Response Code: 0x%02X\r\n", SYNCEventData->Event_Data.SYNC_IrMC_Special_Object_Get_Confirmation_Data->ResponseCode);
            printf("Final: %d\r\n", SYNCEventData->Event_Data.SYNC_IrMC_Special_Object_Get_Confirmation_Data->Final);
            printf("Data: \r\n");

            for(Index=0;(Index<SYNCEventData->Event_Data.SYNC_IrMC_Special_Object_Get_Confirmation_Data->BufferSize) && (Index < (sizeof(Buffer) - 1));Index++)
               printf("%c", SYNCEventData->Event_Data.SYNC_IrMC_Special_Object_Get_Confirmation_Data->Buffer[Index]);

            printf("\r\n");

            /* If this is a successful GET then pass to routine to      */
            /* handle file I/O and further requests (if required).      */
            if((SYNCEventData->Event_Data.SYNC_IrMC_Special_Object_Get_Confirmation_Data->ResponseCode == SYNC_OBEX_RESPONSE_CONTINUE) || (SYNCEventData->Event_Data.SYNC_IrMC_Special_Object_Get_Confirmation_Data->ResponseCode == SYNC_OBEX_RESPONSE_OK))
               WriteReceivedData(SYNCEventData->Event_Data.SYNC_IrMC_Special_Object_Get_Confirmation_Data->BufferSize, SYNCEventData->Event_Data.SYNC_IrMC_Special_Object_Get_Confirmation_Data->Buffer);

            /* Determine if we need to issue another request to continue*/
            /* the operation.                                           */
            if(!SYNCEventData->Event_Data.SYNC_IrMC_Special_Object_Get_Confirmation_Data->Final)
            {
               /* Display Results.                                      */
               printf("IrMC Special Object Get not complete, Requesting more data.\r\n");

               /* Continue the Special Object Get request.  Note that   */
               /* most parameters passed are ignored on a continuation  */
               /* call.                                                 */
               Result = SYNC_IrMC_Special_Object_Get_Request(BluetoothStackID, SYNCID, 0, 0, NULL);

               if(Result >= 0)
               {
                  printf("SYNC_IrMC_Special_Object_Get_Request() Successful.\r\n");

                  CurrentOperation = coSpecialObjectGet;
               }
               else
               {
                  printf("SYNC_IrMC_Special_Object_Get_Request() Failure %d.\r\n", Result);

                  CurrentOperation = coNone;
               }
            }
            else
            {
               /* Set current operation.                                */
               CurrentOperation = coNone;

               printf("IrMC Special Object Get Completed.\r\n");
            }
            break;
         case etSYNC_IrMC_Object_Put_Confirmation:
            printf("IrMC Object Put Confirmation\r\n");

            printf("Response Code: 0x%02X\r\n", SYNCEventData->Event_Data.SYNC_IrMC_Object_Put_Confirmation_Data->ResponseCode);
            printf("LUID: %s\r\n", SYNCEventData->Event_Data.SYNC_IrMC_Object_Put_Confirmation_Data->LUID?SYNCEventData->Event_Data.SYNC_IrMC_Object_Put_Confirmation_Data->LUID:"");
            if(SYNCEventData->Event_Data.SYNC_IrMC_Object_Put_Confirmation_Data->SyncAnchor != NULL)
            {
               switch(SYNCEventData->Event_Data.SYNC_IrMC_Object_Put_Confirmation_Data->SyncAnchor->Anchor_Type)
               {
                  case atChangeCounter:
                     printf("Anchor Type: CC\r\n");
                     printf("Change Counter: %lu\r\n", SYNCEventData->Event_Data.SYNC_IrMC_Object_Put_Confirmation_Data->SyncAnchor->Anchor_Data.ChangeCounter);
                     break;
                  case atTimestamp:
                     printf("Anchor Type: TS\r\n");
                     TimestampToString(Buffer, &SYNCEventData->Event_Data.SYNC_IrMC_Object_Put_Confirmation_Data->SyncAnchor->Anchor_Data.Timestamp);
                     printf("Timestamp: %s\r\n", Buffer);
                     break;
                  default:
                     printf("Anchor Type: ??\r\n");
                     break;
               }
            }
            else
               printf("Anchor Type: NULL\r\n");

            /* Determine if we need to issue another request to continue*/
            /* the operation.                                           */
            if((SYNCEventData->Event_Data.SYNC_IrMC_Object_Put_Confirmation_Data->ResponseCode == SYNC_OBEX_RESPONSE_CONTINUE) && (CurrentBufferSent != CurrentBufferSize))
            {
               /* Display Results.                                      */
               printf("Object Put not complete, Sending more data.\r\n");

               Result = SYNC_IrMC_Object_Put_Request(BluetoothStackID, SYNCID, Buffer, CurrentBufferSize, (CurrentBufferSize - CurrentBufferSent), (Byte_t *)&(CurrentBuffer[CurrentBufferSent]), NULL, TRUE, &TempLength);

               /* Flag whether or not the operation completed           */
               /* successfully.                                         */
               if(Result >= 0)
                  CurrentBufferSent += TempLength;
               else
               {
                  free(CurrentBuffer);

                  CurrentBuffer    = NULL;

                  CurrentOperation = coNone;
               }
            }
            else
            {
               /* Set current operation.                                */
               CurrentOperation = coNone;

               printf("Object Put Completed.\r\n");

               if(CurrentBuffer)
                  free(CurrentBuffer);

               CurrentBuffer = NULL;
            }
            break;
         case etSYNC_IrMC_RTC_Put_Confirmation:
            printf("IrMC RTC Put Confirmation\r\n");
            printf("Response Code: 0x%02X\r\n", SYNCEventData->Event_Data.SYNC_IrMC_RTC_Put_Confirmation_Data->ResponseCode);

            /* Set current operation.                                   */
            CurrentOperation = coNone;

            printf("IrMC RTC Put Completed.\r\n");
            break;
         case etSYNC_IrMC_Object_Delete_Confirmation:
            printf("IrMC Object Delete Confirmation\r\n");
            printf("Response Code: 0x%02X\r\n", SYNCEventData->Event_Data.SYNC_IrMC_Object_Delete_Confirmation_Data->ResponseCode);
            printf("LUID: %s\r\n", SYNCEventData->Event_Data.SYNC_IrMC_Object_Delete_Confirmation_Data->LUID?SYNCEventData->Event_Data.SYNC_IrMC_Object_Delete_Confirmation_Data->LUID:"");
            if(SYNCEventData->Event_Data.SYNC_IrMC_Object_Delete_Confirmation_Data->SyncAnchor != NULL)
            {
               switch(SYNCEventData->Event_Data.SYNC_IrMC_Object_Delete_Confirmation_Data->SyncAnchor->Anchor_Type)
               {
                  case atChangeCounter:
                     printf("Anchor Type: CC\r\n");
                     printf("Change Counter: %lu\r\n", SYNCEventData->Event_Data.SYNC_IrMC_Object_Delete_Confirmation_Data->SyncAnchor->Anchor_Data.ChangeCounter);
                     break;
                  case atTimestamp:
                     printf("Anchor Type: TS\r\n");
                     TimestampToString(Buffer, &SYNCEventData->Event_Data.SYNC_IrMC_Object_Delete_Confirmation_Data->SyncAnchor->Anchor_Data.Timestamp);
                     printf("Timestamp: %s\r\n", Buffer);
                     break;
                  default:
                     printf("Anchor Type: ??\r\n");
                     break;
               }
            }
            else
               printf("Anchor Type: NULL\r\n");

            /* Set current operation.                                   */
            CurrentOperation = coNone;

            printf("IrMC Object Delete Completed.\r\n");
            break;
         case etSYNC_Abort_Confirmation:
            printf("Abort Confirmation\r\n");

            /* Free any allocated data buffers for ongoing transactions.*/
            if((CurrentOperation != coNone) && (CurrentBuffer))
            {
               free(CurrentBuffer);

               CurrentBuffer = NULL;
            }

            /* Reset the current operation value.                       */
            CurrentOperation = coNone;
            break;
         default:
            printf("Unexpected (Unhandled) Event %d.\r\n", SYNCEventData->Event_Data_Type);
            break;
      }

      printf("Client>");

      /* Make sure the output is displayed to the user.                 */
      fflush(stdout);
   }
}

   /* The following function represents the OBEX SYNC Profile Command   */
   /* Event Callback.  This function will be called whenever a SYNC     */
   /* Commmand Event occurs that is associated with the specified       */
   /* Bluetooth Stack ID.  This function takes as its parameters the    */
   /* Bluetooth Stack ID, the SYNC Event Data that occurred and the SYNC*/
   /* Profile Event Callback Parameter that was specified when this     */
   /* Callback was installed.  The caller is free to use the contents of*/
   /* the SYNC Event Data ONLY in the context of this callback.  If the */
   /* caller requires the Data for a longer period of time, then the    */
   /* callback function MUST copy the data into another Data Buffer.    */
   /* This function is guaranteed NOT to be invoked more than once      */
   /* simultaneously for the specified installed callback (i.e. this    */
   /* function DOES NOT have be reentrant).  It Needs to be noted       */
   /* however, that if the same Callback is installed more than once,   */
   /* then the callbacks will be called serially.  Because of this, the */
   /* processing in this function should be as efficient as possible.   */
   /* It should also be noted that this function is called in the Thread*/
   /* Context of a Thread that the User does NOT own.  Therefore,       */
   /* processing in this function should be as efficient as possible    */
   /* (this argument holds anyway because another SYNC Profile Event    */
   /* will not be processed while this function call is outstanding).   */
   /* * NOTE * This function MUST NOT Block and wait for events that can*/
   /*          only be satisfied by Receiving other SYNC Events.  A     */
   /*          Deadlock WILL occur because NO SYNC Event Callbacks will */
   /*          be issued while this function is currently outstanding.  */
static void BTPSAPI SYNC_Event_Callback_SyncCommand(unsigned int BluetoothStackID, SYNC_Event_Data_t *SYNCEventData, unsigned long CallbackParameter)
{
   char temp[128];
   char BoardStr[13];

   /* Make sure that parameters passed in appear to be at least         */
   /* semi-valid.                                                       */
   if((BluetoothStackID) && (SYNCEventData))
   {
      printf("\r\n");

      switch(SYNCEventData->Event_Data_Type)
      {
         case etSYNC_Open_Port_Indication:
            printf("[SYNCCMD] Open Port Indication\r\n");

            BD_ADDRToStr(SYNCEventData->Event_Data.SYNC_Open_Port_Indication_Data->BD_ADDR, BoardStr);
            printf("BD_ADDR: %s\r\n", BoardStr);
            break;
         case etSYNC_Close_Port_Indication:
            printf("[SYNCCMD] Close Port Indication\r\n");
            break;
         case etSYNC_IrMC_Object_Get_Indication:
            printf("[SYNCCMD] IrMC Object Get Indication\r\n");
            break;
         case etSYNC_IrMC_Object_Put_Indication:
            printf("[SYNCCMD] IrMC Object Put Indication\r\n");
            break;
         case etSYNC_IrMC_Special_Object_Get_Indication:
            printf("[SYNCCMD] IrMC Special Object Get Indication\r\n");
            break;
         case etSYNC_IrMC_Object_Delete_Indication:
            printf("[SYNCCMD] IrMC Object Delete Indication\r\n");
            break;
         case etSYNC_IrMC_RTC_Put_Indication:
            printf("[SYNCCMD] RTC Put Indication (Not Supported in implementation).\r\n");
            break;
         case etSYNC_Abort_Indication:
            printf("[SYNCCMD] Abort Indication\r\n");
            break;
         case etSYNC_IrMC_Sync_Command_Indication:
            /* Determine if we need to truncate the string.             */
            if(SYNCEventData->Event_Data.SYNC_IrMC_Sync_Command_Indication_Data->BufferSize >= sizeof(temp))
            {
               SYNCEventData->Event_Data.SYNC_IrMC_Sync_Command_Indication_Data->BufferSize = (sizeof(temp) - 1);
            }

            /* Copy to our temp buffer and display.                     */
            memcpy(temp, SYNCEventData->Event_Data.SYNC_IrMC_Sync_Command_Indication_Data->Buffer, SYNCEventData->Event_Data.SYNC_IrMC_Sync_Command_Indication_Data->BufferSize);
            temp[SYNCEventData->Event_Data.SYNC_IrMC_Sync_Command_Indication_Data->BufferSize] = '\0';

            /* No action is taken automatically.  User will perform     */
            /* Sync.                                                    */
            printf("[SYNCCMD] IrMC Sync Command Indication: %s\r\n", temp);
            break;
         default:
            /* Unhandled Event.                                         */
            printf("[SYNCCMD] Unexpected (Unhandled) Event %d.\r\n", SYNCEventData->Event_Data_Type);
            break;
      }

      printf("Client>");

      /* Make sure the output is displayed to the user.                 */
      fflush(stdout);
   }
}

   /* Main Program Entry Point.                                         */
int main(int argc, char* argv[])
{
   char                    *endptr = NULL;
   int                      Result   = 0;
   unsigned int             CommPortNumber;
   unsigned int             BaudRate;
   HCI_DriverInformation_t  HCI_DriverInformation;
   HCI_DriverInformation_t *HCI_DriverInformationPtr;

   /* Seed Random Number Generator.                                     */
   srand((unsigned int)time(NULL));

   /* First, check to see if the right number of parameters where       */
   /* entered at the Command Line.                                      */
   if(argc >= NUM_EXPECTED_PARAMETERS_USB)
   {
      /* The minimum number of parameters were entered at the Command   */
      /* Line.  Check the first parameter to determine if the           */
      /* application is supposed to run in UART or USB mode.            */
      switch(strtol(argv[1], &endptr, 10))
      {
         case USB_PARAMETER_VALUE:
            /* The Transport selected was USB, setup the Driver         */
            /* Information Structure to use USB as the HCI Transport.   */
            HCI_DRIVER_SET_USB_INFORMATION(&HCI_DriverInformation);

            HCI_DriverInformationPtr = &HCI_DriverInformation;
            break;
         case UART_PARAMETER_VALUE:
            /* The Transport selected was UART, check to see if the     */
            /* number of parameters entered on the command line is      */
            /* correct.                                                 */
            if(argc == NUM_EXPECTED_PARAMETERS_UART)
            {
               /* The correct number of parameters were entered, convert*/
               /* the command line parameters to the proper format.     */
               BaudRate       = strtol(argv[4], &endptr, 10);

               /* Either a port number or a device file can be used.    */
               if((argv[3][0] >= '0') && (argv[3][0] <= '9'))
               {
                  CommPortNumber = strtol(argv[3], &endptr, 10);
                  HCI_DRIVER_SET_COMM_INFORMATION(&HCI_DriverInformation, CommPortNumber, BaudRate, cpUART);
               }
               else
               {
                  HCI_DRIVER_SET_EXTENDED_COMM_INFORMATION_DEVICE_NAME(&HCI_DriverInformation, -1, BaudRate, cpUART, 0, argv[3]);
               }

               HCI_DriverInformationPtr = &HCI_DriverInformation;
            }
            else
               HCI_DriverInformationPtr =  NULL;
            break;
         case BCSP_PARAMETER_VALUE:
            /* The Transport selected was BCSP, check to see if the     */
            /* number of parameters entered on the command line is      */
            /* correct.                                                 */
            if(argc == NUM_EXPECTED_PARAMETERS_UART)
            {
               /* The correct number of parameters were entered, convert*/
               /* the command line parameters to the proper format.     */
               BaudRate = strtol(argv[4], &endptr, 10);

               /* Either a port number or a device file can be used.    */
               if((argv[3][0] >= '0') && (argv[3][0] <= '9'))
               {
                  CommPortNumber = strtol(argv[3], &endptr, 10);
                  HCI_DRIVER_SET_COMM_INFORMATION(&HCI_DriverInformation, CommPortNumber, BaudRate, cpBCSP);
               }
               else
               {
                  HCI_DRIVER_SET_EXTENDED_COMM_INFORMATION_DEVICE_NAME(&HCI_DriverInformation, -1, BaudRate, cpBCSP, 0, argv[3]);
               }

               HCI_DriverInformationPtr = &HCI_DriverInformation;
            }
            else
               HCI_DriverInformationPtr =  NULL;
            break;
         default:
            HCI_DriverInformationPtr =  NULL;
            break;
      }

      /* Check to see if the HCI_Driver Information Strucuture was      */
      /* successfully setup.                                            */
      if(HCI_DriverInformationPtr)
      {
         /* Convert the Command Line parameters to the proper format.   */
         IsClient = strtol(argv[2], &endptr, 10);

         /* Try to Open the stack and check if it was successful.       */
         if(!OpenStack(HCI_DriverInformationPtr))
         {
            /* First attempt to set the Device to be Connectable.       */
            Result = SetConnect();

            /* Next, check to see if the Device was successfully made   */
            /* Connectable.                                             */
            if(!Result)
            {
               /* Now that the device is Connectable attempt to make it */
               /* Discoverable.                                         */
               Result = SetDisc();

               /* Next, check to see if the Device was successfully made*/
               /* Discoverable.                                         */
               if(!Result)
               {
                  /* Now that the device is discoverable attempt to make*/
                  /* it pairable.                                       */
                  Result = SetPairable();

                  if(!Result)
                  {
                     /* The Device is now Connectable, Discoverable, and*/
                     /* Pairable, go ahead and start up the main        */
                     /* application loop.                               */
                     if(IsClient)
                        UserInterface_Client();
                     else
                        UserInterface_Server();
                  }
               }
            }

            /* Close the Bluetooth Stack.                               */
            CloseStack();
         }
         else
         {
            /* There was an error while attempting to open the Stack.   */
            printf("Unable to open the stack.\r\n");
         }
      }
      else
      {
         /* One or more of the Command Line parameters appear to be     */
         /* invalid.                                                    */
         printf("Parameter Error (Expecting = [USB = 0, UART = 1, BCSP = 2 Flag] [Server/Client Flag (0 = Server)] [IF !USB [Comm Port or Device File] [Baud Rate]])\r\n");
      }
   }
   else
   {
      /* An invalid number of parameters was entered on the Command     */
      /* Line.                                                          */
      printf("Parameter Error (Expecting = [USB = 0, UART = 1, BCSP = 2 Flag] [Server/Client Flag (0 = Server)] [IF !USB [Comm Port or Device File] [Baud Rate]])\r\n");
   }

   return 0;
}

