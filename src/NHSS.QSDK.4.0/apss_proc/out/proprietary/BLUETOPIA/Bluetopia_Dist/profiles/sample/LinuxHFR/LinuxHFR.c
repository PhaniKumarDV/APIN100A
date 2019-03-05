/*****< linuxhfr.c >***********************************************************/
/*      Copyright 2002 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  LINUXHFR - Simple Linux application using Hands-Free Profile.             */
/*                                                                            */
/*  Author:  Rory Sledge                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   08/14/02  R. Sledge       Initial creation.                              */
/******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "LinuxHFR.h"      /* Main Application Prototypes and Constants.      */

#include "SS1BTPS.h"       /* Includes for the SS1 Bluetooth Protocol Stack.  */
#include "SS1BTHFR.h"      /* Includes for the SS1 Hands-Free Profile.        */
#include "SS1BTDBG.h"      /* Includes/Constants for Bluetooth Debugging.     */

#define UNUSED(_x)                                  ((void)(_x))

#define LOCAL_NAME_ROOT                      "LinuxHFR"  /* Root of the local */
                                                         /* name              */

#define NUM_EXPECTED_PARAMETERS_USB                 (2)  /* Denotes the number*/
                                                         /* of command line   */
                                                         /* parameters        */
                                                         /* accepted at Run   */
                                                         /* Time when running */
                                                         /* in USB Mode.      */

#define NUM_EXPECTED_PARAMETERS_UART                (4)  /* Denotes the       */
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

#define MAX_SUPPORTED_COMMANDS                     (64)  /* Denotes the       */
                                                         /* maximum number of */
                                                         /* User Commands that*/
                                                         /* are supported by  */
                                                         /* this application. */

#define MAX_NUM_OF_PARAMETERS                       (8)  /* Denotes the max   */
                                                         /* number of         */
                                                         /* parameters a      */
                                                         /* command can have. */

#define MAX_COMMAND_LENGTH                         (64)  /* Denotes the max   */
                                                         /* buffer size used  */
                                                         /* for user commands */
                                                         /* inputted via the  */
                                                         /* UserInterface.    */

#define MAX_INQUIRY_RESULTS                         (32) /* Denotes the max   */
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

#define LOCAL_SERVER_CHANNEL_ID                     (1)  /* Denotes the Server*/
                                                         /* Port ID to use    */
                                                         /* when opening      */
                                                         /* Handsfree/Audio   */
                                                         /* Gateway Server    */
                                                         /* Ports.            */

#define DEFAULT_AG_SUPPORTED_FEATURES           (0x1FF)  /* Denotes the AG    */
                                                         /* default Supported */
                                                         /* Features value.   */

#define DEFAULT_HF_SUPPORTED_FEATURES            (0x7F)  /* Denotes the HF    */
                                                         /* default Supported */
                                                         /* Features value.   */

#define DEFAULT_CALL_HOLDING_SUPPORT             (0x7F)  /* Denotes the AG/HF */
                                                         /* default Call Hold */
                                                         /* and Mutliparty    */
                                                         /* Support value.    */

#define NO_COMMAND_ERROR                           (-1)  /* Denotes that no   */
                                                         /* command was spec. */
                                                         /* to the parser     */

#define INVALID_COMMAND_ERROR                      (-2)  /* Denotes that the  */
                                                         /* Command does not  */
                                                         /* exist for process.*/

#define EXIT_CODE                                  (-3)  /* Denotes that the  */
                                                         /* command is to     */
                                                         /* exit.             */

#define FUNCTION_ERROR                             (-4)  /* Denotes that an   */
                                                         /* error occurred in */
                                                         /* execution of the  */
                                                         /* function.         */

#define TO_MANY_PARAMS                             (-5)  /* Denotes that there*/
                                                         /* are more          */
                                                         /* parameters then   */
                                                         /* will fit in the   */
                                                         /* UserCommand.      */

#define INVALID_PARAMETERS_ERROR                   (-6)  /* Denotes that an   */
                                                         /* error occurred due*/
                                                         /* to the fact that  */
                                                         /* one or more of the*/
                                                         /* required params.  */
                                                         /* were invalid.     */

#define UNABLE_TO_INITIALIZE_STACK                 (-7)  /* Denotes that an   */
                                                         /* error occurred    */
                                                         /* while initializing*/
                                                         /* the stack.        */

#define INVALID_STACK_ID_ERROR                     (-8)  /* Denotes that an   */
                                                         /* error occurred due*/
                                                         /* to attempted      */
                                                         /* execution of a    */
                                                         /* command without a */
                                                         /* valid Bluetooth   */
                                                         /* Stack ID.         */

#define NUMBER_DEFAULT_HFR_INDICATORS              (8)   /* Denotes the       */
                                                         /* number of Hands   */
                                                         /* Free Indicators   */
                                                         /* that will be      */
                                                         /* supported by      */
                                                         /* default.          */

#define INDENT_LENGTH                               (3)  /* Denotes the number*/
                                                         /* of character      */
                                                         /* spaces to be used */
                                                         /* for indenting when*/
                                                         /* displaying SDP    */
                                                         /* Data Elements.    */

#define HFRE_SERVICE_STRING              "SERVICE"    /* Constant Text String */
                                                /* that represents the service*/
                                                /* indicator string.          */

#define HFRE_CALL_STRING                    "CALL"    /* Constant Text String */
                                                /* that represents the call   */
                                                /* indicator string.          */

#define HFRE_CALL_SETUP_STRING        "CALL_SETUP"   /* Constant Text String  */
                                                /* that represents the call   */
                                                /* setup indicator string as  */
                                                /* defined in the HFP Voting  */
                                                /* Draft Version 1.0.         */

#define HFRE_CALLSETUP_STRING          "CALLSETUP"  /* Constant Text String   */
                                                /* that represents the call   */
                                                /* setup indicator string as  */
                                                /* defined in the HFP Adopted */
                                                /* Version 1.0.               */

#define HFRE_CALLHELD_STRING            "CALLHELD"  /* Constant Text String   */
                                                /* that represents the call   */
                                                /* held indicator string as   */
                                                /* defined in the Enhanced    */
                                                /* Call Control Prototype     */
                                                /* Specification.             */

#define HFRE_SIGNAL_STRING                "SIGNAL"  /* Constant Text String   */
                                                /* that represents the signal */
                                                /* strength indicator string  */
                                                /* as defined in the Phone    */
                                                /* Status Indicator Prototype */
                                                /* Specification.             */

#define HFRE_ROAM_STRING                    "ROAM"  /* Constant Text String   */
                                                /* that represents the roam   */
                                                /* indicator string as        */
                                                /* defined in the Phone Status*/
                                                /* Indicator Prototype        */
                                                /* Specification.             */

#define HFRE_BATTCHG_STRING              "BATTCHG"  /* Constant Text String   */
                                                /* that represents the battery*/
                                                /* charge indicator string as */
                                                /* defined in the Phone Status*/
                                                /* Indicator Prototype        */
                                                /* Specification.             */

   /* The following constants are used with the SCOTestMode variable to */
   /* denote the current SCO Audio Data Test Mode.                      */
#define AUDIO_DATA_TEST_NONE         0x00000000
#define AUDIO_DATA_TEST_1KHZ_TONE    0x00000001
#define AUDIO_DATA_TEST_LOOPBACK     0x00000002

   /* The following constants represent the default log file names that */
   /* are used if no Log file name is specified when enabling debug.    */
#define DEFAULT_DEBUG_LOG_FILE_NAME  "LinuxHFR_ASC.log"
#define DEFAULT_DEBUG_FTS_FILE_NAME  "LinuxHFR_FTS.log"

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

   /* The following type definition represents the structure which holds*/
   /* all information about the parameter, in particular the parameter  */
   /* as a string and the parameter as an unsigned int.                 */
typedef struct _tagParameter_t
{
   char *strParam;
   int   intParam;
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

typedef struct _tagNotification_t
{
   struct _tagNotification_t *NextNotification;
   Boolean_t                  Enabled;
   char                      *Indicator;
} Notification_t;

#define NOTIFICATION_DATA_SIZE(_x)           (sizeof(Notification_t) + (_x))

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the         */
   /* compiler as part of standard C/C++).                              */
static unsigned int            BluetoothStackID;    /* Variable which holds the Handle */
                                                    /* of the opened Bluetooth Protocol*/
                                                    /* Stack.                          */

static unsigned int            DebugID;             /* Variable which holds the ID of  */
                                                    /* the currently enabled debugging */
                                                    /* session.                        */

static int                     IsHandsFree;         /* Variable used to indicate if the*/
                                                    /* program is to be ran in Audio   */
                                                    /* Gateway mode, or Hands-Free     */
                                                    /* mode.                           */

static Boolean_t               AGHFDetected;        /* Variable which flags whether or */
                                                    /* not we have determined if we    */
                                                    /* are acting as a Audio Gateway   */
                                                    /* or a Hands-Free unit.           */

static Boolean_t               ServerConnected;     /* Variable which contains the     */
                                                    /* current connection state of the */
                                                    /* HFRE Server.                    */

static DWord_t                 HFREServerSDPHandle; /* Variable which contains the     */
                                                    /* handle to the HFRE SDP Record.  */

static int                     CurrentServerPortID; /* Variable which contains the     */
                                                    /* handle of the most recent       */
                                                    /* HFRE Server port opened.        */

static int                     CurrentClientPortID; /* Variable which contains the     */
                                                    /* handle of the most recent       */
                                                    /* HFRE Client port opened.        */

static BD_ADDR_t               InquiryResultList[MAX_INQUIRY_RESULTS];/* Variable which*/
                                                    /* contains the inquiry result     */
                                                    /* received from the most recently */
                                                    /* preformed inquiry.              */

static unsigned long           RemoteSupportedFeatures;  /* Variable to hold the value */
                                                    /* of the Supported Feature of the */
                                                    /* currently connected device.     */

static unsigned int            NumberofValidResponses; /* Variable which holds the     */
                                                    /* number of valid inquiry results */
                                                    /* within the inquiry results      */
                                                    /* array.                          */

static LinkKeyInfo_t           LinkKeyInfo[16];     /* Variable which holds the list of*/
                                                    /* BD_ADDR <-> Link Keys for       */
                                                    /* pairing.                        */

static BD_ADDR_t               CurrentRemoteBD_ADDR;/* Variable which holds the        */
                                                    /* current BD_ADDR of the device   */
                                                    /* which is currently pairing or   */
                                                    /* authenticating.                 */

static GAP_IO_Capability_t     IOCapability;        /* Variable which holds the        */
                                                    /* current I/O Capabilities that   */
                                                    /* are to be used for Secure Simple*/
                                                    /* Pairing.                        */

static Boolean_t               OOBSupport;          /* Variable which flags whether    */
                                                    /* or not Out of Band Secure Simple*/
                                                    /* Pairing exchange is supported.  */

static Boolean_t               MITMProtection;      /* Variable which flags whether or */
                                                    /* not Man in the Middle (MITM)    */
                                                    /* protection is to be requested   */
                                                    /* during a Secure Simple Pairing  */
                                                    /* procedure.                      */

static unsigned int            NumberCommands;      /* Variable which is used to hold  */
                                                    /* the number of Commands that are */
                                                    /* supported by this application.  */
                                                    /* Commands are added individually.*/

static CommandTable_t          CommandTable[MAX_SUPPORTED_COMMANDS]; /* Variable which */
                                                    /* is used to hold the actual      */
                                                    /* Commands that are supported by  */
                                                    /* this application.               */

static HFRE_Control_Indicator_Entry_t DefaultIndicators[NUMBER_DEFAULT_HFR_INDICATORS];
                                                    /* Variable which holds the        */
                                                    /* Hands Free Default Indicators.  */

static Notification_t         *NotificationList;
                                                    /* Variable which holds the        */
                                                    /* Hands Free Notifications.       */

static unsigned int            EntryIndex;          /* Variable used in SendCallEntry  */
                                                    /* command processing.             */

static unsigned int            AudioDataCount;      /* Variable which is used to filter*/
                                                    /* displaying all of the Audio Data*/
                                                    /* Indications.  Using this        */
                                                    /* variable as a counter, the      */
                                                    /* application will only display   */
                                                    /* Audio Data Indications every    */
                                                    /* second instead of every time an */
                                                    /* Audio Data Packet is received   */
                                                    /* (which overwhelms the           */
                                                    /* application and display).       */

static SCO_Physical_Transport_t AudioTransport;     /* Variable to hold the current    */
                                                    /* SCO Transport.                  */

static Boolean_t               AudioData8BitFormat; /* Variable which flags whether or */
                                                    /* not the SCO Data Format is 8 bit*/
                                                    /* PCM (TRUE) or 16 bit PCM        */
                                                    /* (FALSE).                        */

static unsigned int            AudioTestMode;       /* Variable which holds the        */
                                                    /* current SCO Audio Data Test     */
                                                    /* Mode.                           */

static unsigned int            AudioDataToneIndex;  /* Variable which holds the        */
                                                    /* current memory pointer index    */
                                                    /* into the SCO Tone data array    */
                                                    /* (either 8 or 16 bit) of the     */
                                                    /* current SCO data (used with     */
                                                    /* SCO Test Mode 1KHz Sine Wave).  */

static Boolean_t               AudioConnected;      /* Variable which holds the        */
                                                    /* current state of the Audio      */
                                                    /* Connection (Connected equals    */
                                                    /* TRUE).                          */

static unsigned int           SelectedCodec;
static unsigned int           RequestedCodec;

static Boolean_t              WBS_Supported;

   /* The following data represents an 8 bit, two's complement, Linear  */
   /* PCM encoding of a 1KHz Sine Wave.  This is used for the 1KHz Sine */
   /* Wave Test Mode when operating in 8 bit mode.                      */
static Byte_t AudioDataTone_1KHz_8Bit[] =
{
   0x80,
   0xC3,
   0xDF,
   0xC3,
   0x80,
   0x3C,
   0x20,
   0x3C
} ;

   /* The following data represents a 16 bit (little endian), two's     */
   /* complement, Linear PCM, encoding of a 1KHz Sine Wave.  This is    */
   /* used for the 1KHz Sine Wave Test Mode when operating in 16 bit    */
   /* mode.                                                             */
static Byte_t AudioDataTone_1KHz_16Bit_8KHz[] =
{
   0x00, 0x00,
   0xE1, 0x43,
   0xFF, 0x5F,
   0xE1, 0x43,
   0x00, 0x00,
   0x1F, 0xBC,
   0x01, 0xA0,
   0x1F, 0xBC
} ;

   /* The following data represents a 16 bit (little endian), two's     */
   /* complement, Linear PCM, encoding of a 1KHz Sine Wave.  This is    */
   /* used for the 1KHz Sine Wave Test Mode when operating in 16 bit    */
   /* mode.                                                             */
static Byte_t AudioDataTone_1KHz_16Bit_16KHz[] =
{
   0x00, 0x00,
   0xBC, 0x24,
   0xE1, 0x43,
   0xB1, 0x58,
   0xFF, 0x5F,
   0xB1, 0x58,
   0xE1, 0x43,
   0xBC, 0x24,
   0x00, 0x00,
   0x44, 0xDB,
   0x1F, 0xBC,
   0x4F, 0xA7,
   0x01, 0xA0,
   0x4F, 0xA7,
   0x1F, 0xBC,
   0x44, 0xDB
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

   /* Internal function prototypes.                                     */
static void DisplayAudioGatewayMenu(void);
static void DisplayHandsFreeMenu(void);
static void PopulateAudioGatewayCommandTable(void);
static void PopulateHandsFreeCommandTable(void);

static void UserInterface_AudioGatewayHandsFree(void);
static void UserInterface_Main(void);

static unsigned int StringToUnsignedInteger(char *StringInteger);
static char *StringParser(char *String);
static int CommandParser(UserCommand_t *TempCommand, char *UserInput);
static int CommandInterpreter(UserCommand_t *TempCommand);
static int AddCommand(char *CommandName, CommandFunction_t CommandFunction);
static CommandFunction_t FindCommand(char *Command);
static void ClearCommands(void);

static void BD_ADDRToStr(BD_ADDR_t Board_Address, char *BoardStr);
static void WriteEIRInformation(char *LocalName);

static int DisplayHelp(ParameterList_t *TempParam);
static int EnableDebug(ParameterList_t *TempParam);
static int AddDeviceToList(BD_ADDR_t BD_ADDR);

static int InitializeAudioGateway(ParameterList_t *TempParam);
static int InitializeHandsFree(ParameterList_t *TempParam);

static int OpenStack(HCI_DriverInformation_t *HCI_DriverInformation);
static int CloseStack(void);

static int SetDisc(void);
static int SetConnect(void);
static int SetPairable(void);
static int DeleteLinkKey(BD_ADDR_t BD_ADDR);
static int AddNotificationIndicator(char *Indicator);
static int GetNotificationListCount(void);

static int Inquiry(ParameterList_t *TempParam);
static int DisplayInquiryList(ParameterList_t *TempParam);
static int SetDiscoverabilityMode(ParameterList_t *TempParam);
static int SetConnectabilityMode(ParameterList_t *TempParam);
static int SetPairabilityMode(ParameterList_t *TempParam);
static int ChangeSimplePairingParameters(ParameterList_t *TempParam);
static int ClearSavedLinkKey(ParameterList_t *TempParam);
static int UnPair(ParameterList_t *TempParam);
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

static int SetAuthenticationMode(ParameterList_t *TempParam);

static int SetSCOTransport(ParameterList_t *TempParam);
static int SetSCODataFormat(ParameterList_t *TempParam);
static int SetSCOTestMode(ParameterList_t *TempParam);

static int HFREOpenAudioGatewayServer(void);
static int HFREOpenHandsFreeServer(void);
static int HFREOpenRemoteAudioGatewayPort(ParameterList_t *TempParam);
static int HFREOpenRemoteHandsFreePort(ParameterList_t *TempParam);

static void InitializeDefaultIndicators(void);
static int SetDefaultClientCI(ParameterList_t *TempParam);
static int HFREUpdateCurrentControlIndicatorStatus(ParameterList_t *TempParam);
static int HFREEnableRemoteIndicatorEventNotification(ParameterList_t *TempParam);
static int HFREEnableRemoteCallWaitingNotification(ParameterList_t *TempParam);
static int HFREEnableRemoteCallLineIdentificationNotification(ParameterList_t *TempParam);
static int HFREDisableRemoteEchoCancelationNoiseReduction(ParameterList_t *TempParam);

static int HFREQueryRemoteIndicatorsStatus(ParameterList_t *TempParam);
static int HFREQueryRemoteCallHoldingMultipartyServiceSupport(ParameterList_t *TempParam);
static int HFRESendCallHoldingMultipartySelection(ParameterList_t *TempParam);
static int HFRESendCallWaitingNotification(ParameterList_t *TempParam);
static int HFRESendCallLineIdentificationNotification(ParameterList_t *TempParam);
static int HFREDialPhoneNumber(ParameterList_t *TempParam);
static int HFREDialPhoneNumberFromMemory(ParameterList_t *TempParam);
static int HFRERedialLastPhoneNumber(ParameterList_t *TempParam);
static int HFRESetRingIndication(ParameterList_t *TempParam);
static int HFRERingIndication(ParameterList_t *TempParam);
static int HFREAnswerIncomingCall(ParameterList_t *TempParam);
static int HFRETransmitDTMFCode(ParameterList_t *TempParam);
static int HFRESetRemoteVoiceRecognitionActivation(ParameterList_t *TempParam);
static int HFRESetRemoteSpeakerGain(ParameterList_t *TempParam);
static int HFRESetRemoteMicrophoneGain(ParameterList_t *TempParam);
static int HFREVoiceTagRequest(ParameterList_t *TempParam);
static int HFREVoiceTagResponse(ParameterList_t *TempParam);
static int HFREHangUpCall(ParameterList_t *TempParam);

static int HFREQueryOperator(ParameterList_t *TempParam);
static int HFRESetIndicatorUpdateState(ParameterList_t *TempParam);
static int HFRESetOperatorFormat(ParameterList_t *TempParam);
static int HFREEnableExtendedErrorReporting(ParameterList_t *TempParam);
static int HFREQuerySubscriberNumber(ParameterList_t *TempParam);
static int HFRESendResponseAndHold(ParameterList_t *TempParam);
static int HFREQueryResponseAndHold(ParameterList_t *TempParam);
static int HFREQueryRemoteCallList(ParameterList_t *TempParam);
static int SendCallEntryCommand(ParameterList_t *TempParam);
static int HFRESendExtendedError(ParameterList_t *TempParam);
static int HFRESendOperatorSelection(ParameterList_t *TempParam);
static int HFRESendSubscriberNumber(ParameterList_t *TempParam);

static int ManageAudioConnection(ParameterList_t *TempParam);
static int SelectCodec(ParameterList_t *TempParam);
static int HFRESetupAudioConnection(void);
static int HFREReleaseAudioConnection(void);

static int HFRECloseClient(ParameterList_t *TempParam);

static void DisplaySDPAttributeResponse(SDP_Service_Attribute_Response_Data_t *SDPServiceAttributeResponse, unsigned int InitLevel);
static void DisplaySDPSearchAttributeResponse(SDP_Service_Search_Attribute_Response_Data_t *SDPServiceSearchAttributeResponse);
static void DisplayDataElement(SDP_Data_Element_t *SDPDataElement, unsigned int Level);

   /* Callback Function Prototypes.                                     */
static void BTPSAPI GAP_Event_Callback(unsigned int BluetoothStackID, GAP_Event_Data_t *GAPEventData, unsigned long CallbackParameter);
static void BTPSAPI SDP_Event_Callback(unsigned int BluetoothStackID, unsigned int SDPRequestID, SDP_Response_Data_t *SDP_Response_Data, unsigned long CallbackParameter);
static void BTPSAPI HFRE_Event_Callback(unsigned int BluetoothStackID, HFRE_Event_Data_t *HFRE_Event_Data, unsigned long CallbackParameter);

   /* The following function displays the Audio Gateway Command Menu.   */
static void DisplayAudioGatewayMenu(void)
{
   printf("*********************************************************************\n");
   printf("*  Inquiry                                                          *\n");
   printf("*  DisplayInquiryList                                               *\n");
   printf("*  Pair [Inquiry Index] [Bonding Type]                              *\n");
   printf("*  UnPair [Inquiry Index]                                           *\n");
   printf("*  EndPairing [Inquiry Index]                                       *\n");
   printf("*  PINCodeResponse [PIN Code]                                       *\n");
   printf("*  PassKeyResponse [Numeric Passkey]                                *\n");
   printf("*  UserConfirmationResponse [Confirmation Flag]                     *\n");
   printf("*  SetDiscoverabilityMode [Discoverability Mode]                    *\n");
   printf("*  SetConnectabilityMode [Connectability Mode]                      *\n");
   printf("*  SetPairabilityMode [Pairability Mode]                            *\n");
   printf("*  ChangeSimplePairingParameters [I/O Capabilities] [MITM Flag]     *\n");
   printf("*  GetLocalAddress                                                  *\n");
   printf("*  GetLocalName                                                     *\n");
   printf("*  SetLocalName [Local Device Name (no spaces allowed)]             *\n");
   printf("*  GetClassOfDevice                                                 *\n");
   printf("*  SetClassOfDevice [Class of Device]                               *\n");
   printf("*  GetRemoteName [Inquiry Index]                                    *\n");
   printf("*  ServiceDiscovery [Inquiry Index] [Profile Index] [UUID (Manual)] *\n");
   printf("*  ClearSavedLinkKey                                                *\n");
   printf("*  OpenAudioGatewayClient [Inquiry Index] [Port Number]             *\n");
   printf("*  CloseAudioGatewayClient                                          *\n");
   printf("*  SetAuthenticationMode [Disable = 0, Enable = 1]                  *\n");
   printf("*  UpdateControlIndicators [Index] [Value]                          *\n");
   printf("*  SetDefaultClientCI [Index] [Value]                               *\n");
   printf("*  SendCallWaitingNotification [Phone Number]                       *\n");
   printf("*  SendCallerIDNotification [Phone Number]                          *\n");
   printf("*  SetRingIndication [Disable = 0, Enable = 1]                      *\n");
   printf("*  RingIndication                                                   *\n");
   printf("*  SetVoiceRecognitionActivation [Disable = 0, Enable = 1]          *\n");
   printf("*  SetSpeakerGain [0 =< SpeakerGain =< 15]                          *\n");
   printf("*  SetMicrophoneGain [0 =< MicrophoneGain =< 15]                    *\n");
   printf("*  SendVoiceTagResponse [Phone Number]                              *\n");
   printf("*  DisableRemoteSoundEnhancement                                    *\n");
   printf("*  Audio [Release = 0, Setup = 1]                                   *\n");
   if(WBS_Supported)
      printf("*  SelectCodec [CSVD = 1, mSBC = 2]                                 *\n");
   printf("*  SendRespHold [CallState]                                         *\n");
   printf("*  SendExtendedError [Result Code]                                  *\n");
   printf("*  SendOperatorInfo [OperatorName]                                  *\n");
   printf("*  SendSubNumber [Number]                                           *\n");
   printf("*  SendCallList Final Dir Status Mode Multiparty Number NumFormat   *\n");
   printf("*  SetSCOTransport [Codec = 0, HCI = 1]                             *\n");
   printf("*  SetSCODataFormat [8 bit PCM = 0, 16 bit PCM = 1]                 *\n");
   printf("*  SetSCOTestMode [None = 0, 1KHz Tone = 1, Loopback = 2]           *\n");
   printf("*  EnableDebug [Enable/Disable] [Log Type] [Log File Name]          *\n");
   printf("*  Help                                                             *\n");
   printf("*********************************************************************\n");
}

   /* The following function displays the Hands-Free Command Menu.      */
static void DisplayHandsFreeMenu(void)
{
   /* First display the upper command bar.                              */
   printf("*********************************************************************\n");
   printf("*  Inquiry                                                          *\n");
   printf("*  DisplayInquiryList                                               *\n");
   printf("*  Pair [Inquiry Index] [Bonding Type]                              *\n");
   printf("*  UnPair [Inquiry Index]                                           *\n");
   printf("*  EndPairing [Inquiry Index]                                       *\n");
   printf("*  PINCodeResponse [PIN Code]                                       *\n");
   printf("*  PassKeyResponse [Numeric Passkey]                                *\n");
   printf("*  UserConfirmationResponse [Confirmation Flag]                     *\n");
   printf("*  SetDiscoverabilityMode [Discoverability Mode]                    *\n");
   printf("*  SetConnectabilityMode [Connectability Mode]                      *\n");
   printf("*  SetPairabilityMode [Pairability Mode]                            *\n");
   printf("*  ChangeSimplePairingParameters [I/O Capabilities] [MITM Flag]     *\n");
   printf("*  GetLocalAddress                                                  *\n");
   printf("*  GetLocalName                                                     *\n");
   printf("*  SetLocalName [Local Device Name (no spaces allowed)]             *\n");
   printf("*  GetClassOfDevice                                                 *\n");
   printf("*  SetClassOfDevice [Class of Device]                               *\n");
   printf("*  GetRemoteName [Inquiry Index]                                    *\n");
   printf("*  ServiceDiscovery [Inquiry Index] [Profile Index] [UUID (Manual)] *\n");
   printf("*  ClearSavedLinkKey                                                *\n");
   printf("*  OpenHandsFreeClient [Inquiry Index] [Port Number]                *\n");
   printf("*  CloseHandsFreeClient                                             *\n");
   printf("*  SetAuthenticationMode [Disable = 0, Enable = 1]                  *\n");
   printf("*  EnableRemoteIndicatorNotification [Disable = 0, Enable = 1]      *\n");
   printf("*  EnableRemoteCallWaitingNotification [Disable = 0, Enable = 1]    *\n");
   printf("*  EnableRemoteCallerIDNotification [Disable = 0, Enable = 1]       *\n");
   printf("*  QueryRemoteIndicatorStatus                                       *\n");
   printf("*  SetIndicatorUpdateState [Index] [Disable = 0 Enable = 1]         *\n");
   printf("*  QueryRemoteCallHoldSupport                                       *\n");
   printf("*  SendCallHoldSelection [0 =< Selection =< 4] [Index]              *\n");
   printf("*  DialNumber [Phone Number]                                        *\n");
   printf("*  MemoryDial [MemoryLocation]                                      *\n");
   printf("*  RedialLastNumber                                                 *\n");
   printf("*  AnswerCall                                                       *\n");
   printf("*  HangUpCall                                                       *\n");
   printf("*  SendDTMF [DTMF Code]                                             *\n");
   printf("*  SetVoiceRecognitionActivation [Disable = 0, Enable = 1]          *\n");
   printf("*  SetSpeakerGain [0 =< SpeakerGain =< 15]                          *\n");
   printf("*  SetMicrophoneGain [0 =< MicrophoneGain =< 15]                    *\n");
   printf("*  SendVoiceTagRequest                                              *\n");
   printf("*  DisableRemoteSoundEnhancement                                    *\n");
   printf("*  Audio [Release = 0, Setup = 1]                                   *\n");
   printf("*  SetOperatorFormat                                                *\n");
   printf("*  EnableErrorReports {FALSE = 0 | TRUE = 1]                        *\n");
   printf("*  QueryOperator                                                    *\n");
   printf("*  QueryPhoneNumber                                                 *\n");
   printf("*  QueryCallList                                                    *\n");
   printf("*  QueryRespHold                                                    *\n");
   printf("*  SendRespHold [CallState]                                         *\n");
   printf("*  SetSCOTransport [Codec = 0, HCI = 1]                             *\n");
   printf("*  SetSCODataFormat [8 bit PCM = 0, 16 bit PCM = 1]                 *\n");
   printf("*  SetSCOTestMode [None = 0, 1KHz Tone = 1, Loopback = 2]           *\n");
   printf("*  EnableDebug [Enable/Disable] [Log Type] [Log File Name]          *\n");
   printf("*  Help                                                             *\n");
   printf("*********************************************************************\n");
}

   /* The following function Clears all commands currently in the       */
   /* command table and populates it with the Commands that are         */
   /* available for the Audio Gateway.                                  */
static void PopulateAudioGatewayCommandTable(void)
{
   /* First Clear all of the commands in the Command Table.             */
   ClearCommands();

   /* Now add all of the commands that are associated with the Audio    */
   /* Gateway to the Command Table.                                     */
   AddCommand("OPENAUDIOGATEWAYCLIENT", HFREOpenRemoteHandsFreePort);
   AddCommand("OPENAGCLIENT", HFREOpenRemoteHandsFreePort);
   AddCommand("CLOSEAUDIOGATEWAYCLIENT", HFRECloseClient);
   AddCommand("CLOSEAGCLIENT", HFRECloseClient);
   AddCommand("INQUIRY", Inquiry);
   AddCommand("DISPLAYINQUIRYLIST", DisplayInquiryList);
   AddCommand("PAIR", Pair);
   AddCommand("UNPAIR", UnPair);
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
   AddCommand("CLEARSAVEDLINKKEY", ClearSavedLinkKey);
   AddCommand("SETAUTHENTICATIONMODE", SetAuthenticationMode);
   AddCommand("UPDATECONTROLINDICATORS", HFREUpdateCurrentControlIndicatorStatus);
   AddCommand("SETDEFAULTCLIENTCI", SetDefaultClientCI);
   AddCommand("SENDCALLWAITINGNOTIFICATION", HFRESendCallWaitingNotification);
   AddCommand("CALLWAITING", HFRESendCallWaitingNotification);
   AddCommand("SENDCALLERIDNOTIFICATION", HFRESendCallLineIdentificationNotification);
   AddCommand("CALLERID", HFRESendCallLineIdentificationNotification);
   AddCommand("SETRINGINDICATION", HFRESetRingIndication);
   AddCommand("RINGINDICATION", HFRERingIndication);
   AddCommand("SETVOICERECOGNITIONACTIVATION", HFRESetRemoteVoiceRecognitionActivation);
   AddCommand("SETSPEAKERGAIN", HFRESetRemoteSpeakerGain);
   AddCommand("SETMICROPHONEGAIN", HFRESetRemoteMicrophoneGain);
   AddCommand("SENDVOICETAGRESPONSE", HFREVoiceTagResponse);
   AddCommand("DISABLEREMOTESOUNDENHANCEMENT", HFREDisableRemoteEchoCancelationNoiseReduction);
   AddCommand("AUDIO", ManageAudioConnection);
   if(WBS_Supported)
      AddCommand("SELECTCODEC", SelectCodec);
   AddCommand("SENDRESPHOLD", HFRESendResponseAndHold);
   AddCommand("SENDCALLLIST", SendCallEntryCommand);
   AddCommand("SENDEXTENDEDERROR", HFRESendExtendedError);
   AddCommand("SENDOPERATORINFO", HFRESendOperatorSelection);
   AddCommand("SENDSUBNUMBER", HFRESendSubscriberNumber);
   AddCommand("SETSCOTRANSPORT", SetSCOTransport);
   AddCommand("SETSCODATAFORMAT", SetSCODataFormat);
   AddCommand("SETSCOTESTMODE", SetSCOTestMode);
   AddCommand("ENABLEDEBUG", EnableDebug);
   AddCommand("HELP", DisplayHelp);
}

   /* The following function Clears all commands currently in the       */
   /* command table and populates it with the Commands that are         */
   /* available for the Hands-Free unit.                                */
static void PopulateHandsFreeCommandTable(void)
{
   /* First Clear all of the commands in the Command Table.             */
   ClearCommands();

   /* Now add all of the commands that are associated with the HandsFree*/
   /* unit to the Command Table.                                        */
   AddCommand("OPENHANDSFREECLIENT", HFREOpenRemoteAudioGatewayPort);
   AddCommand("CLOSEHANDSFREECLIENT", HFRECloseClient);
   AddCommand("INQUIRY", Inquiry);
   AddCommand("DISPLAYINQUIRYLIST", DisplayInquiryList);
   AddCommand("PAIR", Pair);
   AddCommand("UNPAIR", UnPair);
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
   AddCommand("CLEARSAVEDLINKKEY", ClearSavedLinkKey);
   AddCommand("SETAUTHENTICATIONMODE", SetAuthenticationMode);
   AddCommand("ENABLEREMOTEINDICATORNOTIFICATION", HFREEnableRemoteIndicatorEventNotification);
   AddCommand("ENABLEREMOTECALLWAITINGNOTIFICATION", HFREEnableRemoteCallWaitingNotification);
   AddCommand("ENABLEREMOTECALLERIDNOTIFICATION", HFREEnableRemoteCallLineIdentificationNotification);
   AddCommand("QUERYREMOTEINDICATORSTATUS", HFREQueryRemoteIndicatorsStatus);
   AddCommand("SETINDICATORUPDATESTATE", HFRESetIndicatorUpdateState);
   AddCommand("QUERYREMOTECALLHOLDSUPPORT", HFREQueryRemoteCallHoldingMultipartyServiceSupport);
   AddCommand("SENDCALLHOLDSELECTION", HFRESendCallHoldingMultipartySelection);
   AddCommand("DIALNUMBER", HFREDialPhoneNumber);
   AddCommand("MEMORYDIAL", HFREDialPhoneNumberFromMemory);
   AddCommand("REDIALLASTNUMBER", HFRERedialLastPhoneNumber);
   AddCommand("ANSWERCALL", HFREAnswerIncomingCall);
   AddCommand("HANGUPCALL", HFREHangUpCall);
   AddCommand("SENDDTMF", HFRETransmitDTMFCode);
   AddCommand("SETVOICERECOGNITIONACTIVATION", HFRESetRemoteVoiceRecognitionActivation);
   AddCommand("SETSPEAKERGAIN", HFRESetRemoteSpeakerGain);
   AddCommand("SETMICROPHONEGAIN", HFRESetRemoteMicrophoneGain);
   AddCommand("SENDVOICETAGREQUEST", HFREVoiceTagRequest);
   AddCommand("DISABLEREMOTESOUNDENHANCEMENT", HFREDisableRemoteEchoCancelationNoiseReduction);
   AddCommand("AUDIO", ManageAudioConnection);
   AddCommand("QUERYOPERATOR", HFREQueryOperator);
   AddCommand("SETOPERATORFORMAT", HFRESetOperatorFormat);
   AddCommand("ENABLEERRORREPORTS", HFREEnableExtendedErrorReporting);
   AddCommand("QUERYPHONENUMBER", HFREQuerySubscriberNumber);
   AddCommand("QUERYRESPHOLD", HFREQueryResponseAndHold);
   AddCommand("SENDRESPHOLD", HFRESendResponseAndHold);
   AddCommand("QUERYCALLLIST", HFREQueryRemoteCallList);
   AddCommand("SETSCOTRANSPORT", SetSCOTransport);
   AddCommand("SETSCODATAFORMAT", SetSCODataFormat);
   AddCommand("SETSCOTESTMODE", SetSCOTestMode);
   AddCommand("ENABLEDEBUG", EnableDebug);
   AddCommand("HELP", DisplayHelp);
}

   /* This function is responsible for taking the users input and do the*/
   /* appropriate thing with it.  First, this function get a string of  */
   /* user input, parse the user input in to command and parameters, and*/
   /* finally executing the command or display an error message if the  */
   /* input is corrupt.                                                 */
static void UserInterface_AudioGatewayHandsFree(void)
{
   UserCommand_t TempCommand;
   int           Result = !EXIT_CODE;
   char          UserInput[MAX_COMMAND_LENGTH];

   /* First add the specified commands to the command list.             */
   ClearCommands();

   AddCommand("AUDIOGATEWAY", InitializeAudioGateway);
   AddCommand("HANDSFREE",    InitializeHandsFree);

   /* Now let's display the available commands.                         */
   printf("*****************************************************************\n");
   printf("* Command Options: AudioGateway, HandsFree, Quit.               *\n");
   printf("*****************************************************************\n");

   /* This is the main loop of the program.  It gets user input from the*/
   /* command window, make a call to the command parser, and command    */
   /* interpreter.  After the function has been ran it then check the   */
   /* return value and displays an error message when appropriate.  If  */
   /* the result returned is ever the EXIT_CODE the loop will exit      */
   /* leading the exit of the program.                                  */
   while(Result != EXIT_CODE)
   {
      /* Initialize the value of the variable used to store the users   */
      /* input and output "Input: " to the command window to inform the */
      /* user that another command may be entered.                      */
      UserInput[0] = '\0';

      /* Output an Input Shell-type prompt.                             */
      printf("Audio Gateway/Hands Free>");

      fflush(stdout);

      /* Retrieve the command entered by the user and store it in the   */
      /* User Input Buffer.  Note that this command will fail if the    */
      /* application receives a signal which cause the standard file    */
      /* streams to be closed.  If this happens the loop will be broken */
      /* out of so the application can exit.                            */
      if(fgets(UserInput, sizeof(UserInput), stdin) != NULL)
      {
         /* Start a newline for the results.                            */
         printf("\n");

         /* Next, check to see if a command was input by the user.      */
         if(strlen(UserInput))
         {
            /* The string input by the user contains a value, now run   */
            /* the string through the Command Parser.                   */
            if(CommandParser(&TempCommand, UserInput) >= 0)
            {
               /* The Command was successfully parsed, run the Command. */
               if((Result = CommandInterpreter(&TempCommand)) != 0)
               {
                  switch(Result)
                  {
                     case INVALID_COMMAND_ERROR:
                        printf("Invalid Command.\n");
                        break;
                     case FUNCTION_ERROR:
                        printf("Function Error.\n");
                        break;
                  }
               }
               else
                  Result = EXIT_CODE;
            }
            else
               printf("Invalid Input.\n");
         }
      }
      else
         Result = EXIT_CODE;
   }
}

   /* This function is responsible for taking the users input and do the*/
   /* appropriate thing with it.  First, this function get a string of  */
   /* user input, parse the user input in to command and parameters, and*/
   /* finally executing the command or display an error message if the  */
   /* input is corrupt.                                                 */
static void UserInterface_Main(void)
{
   UserCommand_t TempCommand;
   int           Result = !EXIT_CODE;
   char          UserInput[MAX_COMMAND_LENGTH];

   /* Determine if we are currently running in Hands-Free or Audio      */
   /* Gateway mode.                                                     */
   if(IsHandsFree)
   {
      /* We are currently running in Hands-Free mode, add the           */
      /* appropriate commands to the command table and display the menu */
      /* options.                                                       */
      PopulateHandsFreeCommandTable();

      DisplayHandsFreeMenu();
   }
   else
   {
      /* We are not currently running in Hands-Free mode therefore we   */
      /* must be running in Audio Gateway mode.  Add the appropriate    */
      /* commands to the command table and display the menu options.    */
      PopulateAudioGatewayCommandTable();

      DisplayAudioGatewayMenu();
   }

   /* This is the main loop of the program.  It gets user input from the*/
   /* command window, make a call to the command parser, and command    */
   /* interpreter.  After the function has been ran it then check the   */
   /* return value and displays an error message when appropriate.  If  */
   /* the result returned is ever the EXIT_CODE the loop will exit      */
   /* leading the exit of the program.                                  */
   while(Result != EXIT_CODE)
   {
      /* Initialize the value of the variable used to store the users   */
      /* input and output "Input: " to the command window to inform the */
      /* user that another command may be entered.                      */
      UserInput[0] = '\0';

      /* Output an Input Shell-type prompt.                             */
      if(IsHandsFree)
         printf("Hands Free>");
      else
         printf("Audio Gateway>");

      fflush(stdout);

      /* Retrieve the command entered by the user and store it in the   */
      /* User Input Buffer.  Note that this command will fail if the    */
      /* application receives a signal which cause the standard file    */
      /* streams to be closed.  If this happens the loop will be broken */
      /* out of so the application can exit.                            */
      if(fgets(UserInput, sizeof(UserInput), stdin) != NULL)
      {
         /* Start a newline for the results.                            */
         printf("\n");

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
                     printf("Invalid Command.\n");
                     break;
                  case FUNCTION_ERROR:
                     printf("Function Error.\n");
                     break;
                  case EXIT_CODE:
                     /* If the user has request to exit we might as well*/
                     /* go ahead an close any Remote Connection to a    */
                     /* Server that we have open.                       */
                     if(CurrentClientPortID > 0)
                        HFRECloseClient(NULL);
                     break;
               }
            }
            else
               printf("Invalid Input.\n");
         }
      }
      else
      {
         /* Close any Remote Connection to a Server that we have open.  */
         if(CurrentClientPortID > 0)
            HFRECloseClient(NULL);

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
      for(Index=0, ret_val=String;Index < (int)strlen(String);Index++)
      {
         /* Is this the space character.                                */
         if((String[Index] == ' ') || (String[Index] == '\r') || (String[Index] == '\n'))
         {
            /* This is the space character, replace it with a NULL      */
            /* terminating character and set the return value to the    */
            /* beginning character of the string.                       */
            String[Index] = '\0';
            break;
         }
      }
   }

   return(ret_val);
}

   /* This function is responsible for taking command strings and       */
   /* parsing them into a command, param1, and param2.  After parsing   */
   /* this string the data is stored into a UserCommand_t structure to  */
   /* be used by the interpreter.  The first parameter of this function */
   /* is the structure used to pass the parsed command string out of the*/
   /* function.  The second parameter of this function is the string    */
   /* that is parsed into the UserCommand structure.  Successful        */
   /* execution of this function is denoted by a return value of zero.  */
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
      /* Retrieve the first token in the string, this should be the     */
      /* command.                                                       */
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
   /* structure containing information about the command to be issued.  */
   /* This information includes the command name and multiple parameters*/
   /* which maybe be passed to the function to be executed.  Successful */
   /* execution of this function is denoted by a return value of zero.  */
   /* A negative return value implies that the command was not found and*/
   /* is invalid.                                                       */
static int CommandInterpreter(UserCommand_t *TempCommand)
{
   int               i;
   int               ret_val;
   CommandFunction_t CommandFunction;

   /* If the command is not found in the table return with an invalid   */
   /* command error                                                     */
   ret_val = INVALID_COMMAND_ERROR;

   /* Let's make sure that the data passed to us appears semi-valid.    */
   if((TempCommand) && (TempCommand->Command))
   {
      /* Now, let's make the Command string all upper case so that we   */
      /* compare against it.                                            */
      for(i=0;i<(int)strlen(TempCommand->Command);i++)
      {
         if((TempCommand->Command[i] >= 'a') && (TempCommand->Command[i] <= 'z'))
            TempCommand->Command[i] -= (char)('a' - 'A');
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
   /* programmaticly add Commands the Global (to this module) Command   */
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

         EIRData.Extended_Inquiry_Response_Data[Index++] = (Byte_t)Length;
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

   /* The following function is responsible for redisplaying the Menu   */
   /* options to the user.  This function returns zero on successful    */
   /* execution or a negative value on all errors.                      */
static int DisplayHelp(ParameterList_t *TempParam)
{
   /* First check to if Currently in Hands-Free or Audio Gateway Mode.  */
   if(IsHandsFree)
   {
      /* Currently in Hands-Free Mode, display the Hands-Free Menu.     */
      DisplayHandsFreeMenu();
   }
   else
   {
      /* Currently in Audio Gateway Mode, display the Audio Gateway     */
      /* Menu.                                                          */
      DisplayAudioGatewayMenu();
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

               printf("Debugging is now disabled.\n");

               /* Flag that debugging is no longer enabled.             */
               DebugID = 0;

               /* Flag success to the caller.                           */
               ret_val = 0;
            }
            else
            {
               printf("Debugging is not currently enabled.\n");

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

                     printf("BTPS_Debug_Initialize() Success: %d.\n", Result);

                     if(TempParam->Params[1].intParam != dtDebugTerminal)
                        printf("   Log File Name: %s\n", LogFileName);

                     /* Flag success to the caller.                     */
                     ret_val = 0;
                  }
                  else
                  {
                     printf("BTPS_Debug_Initialize() Failure: %d.\n", Result);

                     /* Flag that an error occurred while submitting the*/
                     /* command.                                        */
                     ret_val = FUNCTION_ERROR;
                  }
               }
               else
               {
                  /* Invalid parameters specified so flag an error to   */
                  /* the user.                                          */
                  printf("Usage: EnableDebug [Enable/Disable (Enable = 1, Disable = 0)] [DebugType (ASCII File = 0, Debug Console = 1, FTS File = 2)] [[Log File Name] (optional)].\n");

                  /* Flag that an error occurred while submitting the   */
                  /* command.                                           */
                  ret_val = INVALID_PARAMETERS_ERROR;
               }
            }
            else
            {
               printf("Debugging is already enabled.\n");

               /* Flag success to the caller.                           */
               ret_val = 0;
            }
         }
      }
      else
      {
         /* Invalid parameters specified so flag an error to the user.  */
         printf("Usage: EnableDebug [Enable/Disable (Enable = 1, Disable = 0)] [DebugType (ASCII File = 0, Debug Console = 1, FTS File = 2)] [[Log File Name] (optional)].\n");

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

   /* Add the specified Device to the Inquiry List.  Return the Index of*/
   /* the Entry Added or a negative number on Failure.                  */
static int AddDeviceToList(BD_ADDR_t BD_ADDR)
{
   int ndx;

   /* Scan the list to find a match.                                    */
   ndx = 0;
   while(ndx < (int)NumberofValidResponses)
   {
      /* Check to see if the device is already in the list.             */
      if(COMPARE_BD_ADDR(BD_ADDR, InquiryResultList[ndx]))
         break;

      ndx++;
   }
   if((ndx == (int)NumberofValidResponses) && (NumberofValidResponses < MAX_INQUIRY_RESULTS))
   {
      /* Add the device to the list.                                    */
      InquiryResultList[ndx] = BD_ADDR;
      NumberofValidResponses++;
      printf("Added Device to Index %d\n", (ndx+1));
   }
   else
      ndx = -1;

   return(ndx);
}

   /* The following function is responsible for setting the initial     */
   /* state of the Main Application to be an Audio Gateway.  This       */
   /* function returns zero on successful execution and a negative value*/
   /* on all errors.                                                    */
static int InitializeAudioGateway(ParameterList_t *TempParam)
{
   int ret_val;

   /* The program is currently running in Audio Gateway mode.  Ready the*/
   /* device to be a Audio Gateway Server.  First, attempt to set the   */
   /* device to be connectable.                                         */
   if(!SetConnect())
   {
      /* Now that the device is connectable attempt to make it          */
      /* discoverable.                                                  */
      if(!SetDisc())
      {
         /* Now that the device is discoverable attempt to make it      */
         /* pairable.                                                   */
         if(!SetPairable())
         {
            /* The device is now connectable and discoverable so open up*/
            /* a Audio Gateway Server.                                  */
            if(!HFREOpenAudioGatewayServer())
            {
               IsHandsFree      = FALSE;
               AGHFDetected     = TRUE;
               ServerConnected  = FALSE;

               // Initialize some defaults.
               InitializeDefaultIndicators();

               ret_val              = 0;
            }
            else
               ret_val = FUNCTION_ERROR;
         }
         else
            ret_val = FUNCTION_ERROR;
      }
      else
         ret_val = FUNCTION_ERROR;
   }
   else
      ret_val = FUNCTION_ERROR;

   return(ret_val);
}

   /* The following function is responsible for setting the initial     */
   /* state of the Main Application to be an Audio Gateway.  This       */
   /* function returns zero on successful execution and a negative value*/
   /* on all errors.                                                    */
static int InitializeHandsFree(ParameterList_t *TempParam)
{
   int ret_val;

   /* The program is currently running in Hands-Free mode.  Ready the   */
   /* device to be a Hands-Free Server.  First, attempt to set the      */
   /* device to be connectable.                                         */
   if(!SetConnect())
   {
      /* Now that the device is connectable attempt to make it          */
      /* discoverable.                                                  */
      if(!SetDisc())
      {
         /* Now that the device is discoverable attempt to make it      */
         /* pairable.                                                   */
         if(!SetPairable())
         {
            /* The device is now connectable and discoverable so open up*/
            /* a Hands-Free Server.                                     */
            if(!HFREOpenHandsFreeServer())
            {
               IsHandsFree     = TRUE;
               AGHFDetected    = TRUE;
               ServerConnected = FALSE;
               SelectedCodec   = 0;
               RequestedCodec  = 0;

               ret_val         = 0;
            }
            else
               ret_val = FUNCTION_ERROR;
         }
         else
            ret_val = FUNCTION_ERROR;
      }
      else
         ret_val = FUNCTION_ERROR;
   }
   else
      ret_val = FUNCTION_ERROR;

   return(ret_val);
}

   /* The following function is responsible for cleaning up the HFRE    */
   /* profile in preparation for shutting down.  This includes          */
   /* determining whether AG or HFRE device, unregistered the SDP       */
   /* records and shutting down the servers.  All remote connections    */
   /* should already be closed.                                         */
static void CloseHFRE(void)
{
   int   Result;

   /* Note that in either the HFR Device or Audio Gateway instance,     */
   /* the CurrentServerPortID contains the HFR or AG server port ID.    */
   /* This makes closing things down the same in either case.           */

   /* First remove SDP record.                                          */
   Result = HFRE_Un_Register_SDP_Record(BluetoothStackID, CurrentServerPortID, HFREServerSDPHandle);

   printf("HFRE_Un_Register_SDP_Record: Function Returns %d.\n", Result);

   /* Now close the server.                                             */
   Result = HFRE_Close_Server_Port(BluetoothStackID, CurrentServerPortID);

   printf("HFRE_Close_Server_Port: Function Returns %d.\n", Result);

   /* Flag that an Audio Connection is not present.                     */
   AudioConnected = FALSE;
}

   /* The following function is responsible for opening the SS1         */
   /* Bluetooth Protocol Stack.  This function requires as input        */
   /* parameters a valid Comm Port Number, a valid Baud Rate, and a     */
   /* flag that specifies whether or not BCSP is to be used as the      */
   /* Protocol (BCSP is used when this flag is non-zero).  The function */
   /* returns zero on successful execution and a negative value on all  */
   /* errors.                                                           */
static int OpenStack(HCI_DriverInformation_t *HCI_DriverInformation)
{
   int                              Result;
   int                              ret_val;
   char                             BluetoothAddress[13];
   char                            *LocalName;
   Byte_t                           Status;
   Byte_t                           PacketLength;
   Word_t                           tmpWord;
   BD_ADDR_t                        BD_ADDR;
   HCI_Version_t                    HCIVersion;
   SCO_Data_Format_t                SCO_Data_Format;
   L2CA_Link_Connect_Params_t       L2CA_Link_Connect_Params;
   HCI_Local_Supported_Codec_Info_t Codecs;
   SCO_Packet_Information_t         SCO_Packet_Information;

   /* First check to see if the Stack has already been opened.          */
   if(!BluetoothStackID)
   {
      /* Next, makes sure that the Driver Information passed appears to */
      /* be semi-valid.                                                 */
      if(HCI_DriverInformation)
      {
         /* Initialize the Stack.                                       */
         Result = BSC_Initialize(HCI_DriverInformation, 0);

         /* Next, check the return value of the initialization to see if*/
         /* it was successful.                                          */
         if(Result > 0)
         {
            /* The Stack was initialized successfully, inform the user  */
            /* and set the return value of the initialization function  */
            /* to the Bluetooth Stack ID.                               */
            if(HCI_DriverInformation->DriverType == hdtUSB)
               printf("Stack Initialization on USB Successful.\n");
            else
               printf("Stack Initialization on Port %d %ld (%s) Successful.\n", HCI_DriverInformation->DriverInformation.COMMDriverInformation.COMPortNumber, HCI_DriverInformation->DriverInformation.COMMDriverInformation.BaudRate, ((HCI_DriverInformation->DriverInformation.COMMDriverInformation.Protocol == cpBCSP)?"BCSP":"UART"));

            BluetoothStackID = Result;

            DebugID          = 0;

            ret_val          = 0;

            /* Initialize the default Secure Simple Pairing parameters. */
            IOCapability     = DEFAULT_IO_CAPABILITY;
            OOBSupport       = FALSE;
            MITMProtection   = DEFAULT_MITM_PROTECTION;

            if(!HCI_Version_Supported(BluetoothStackID, &HCIVersion))
               printf("Device Chipset Version: %s\n", ((int)HCIVersion <= NUM_SUPPORTED_HCI_VERSIONS)?HCIVersionStrings[HCIVersion]:HCIVersionStrings[NUM_SUPPORTED_HCI_VERSIONS]);

            /* Let's output the Bluetooth Device Address so that the    */
            /* user knows what the Device Address is.                   */
            if(!GAP_Query_Local_BD_ADDR(BluetoothStackID, &BD_ADDR))
            {
               BD_ADDRToStr(BD_ADDR, BluetoothAddress);

               printf("Bluetooth Device Address: %s\n", BluetoothAddress);

               /* Set the local name and the EIR data.                  */
               if((LocalName = (char *)BTPS_AllocateMemory((sizeof(LOCAL_NAME_ROOT) / sizeof(char)) + 5)) != NULL)
               {
                  snprintf(LocalName, ((sizeof(LOCAL_NAME_ROOT) / sizeof(char)) + 5),"%s_%02X%02X", LOCAL_NAME_ROOT, BD_ADDR.BD_ADDR1, BD_ADDR.BD_ADDR0);

                  if(!GAP_Set_Local_Device_Name(BluetoothStackID, LocalName))
                  {
                     /* Add the EIR data.                               */
                     WriteEIRInformation(LocalName);
                  }
                  BTPS_FreeMemory(LocalName);
               }
            }

            /* Default the transport to Codec.                          */
            Result = SCO_Set_Physical_Transport(BluetoothStackID, sptCodec);
            if(!Result)
               AudioTransport = sptCodec;
            else
               AudioTransport = sptHCI;

            /* Inform the user of the current SCO Data Format.          */
            Result = SCO_Query_Data_Format(BluetoothStackID, &SCO_Data_Format);
            if(Result >= 0)
               AudioData8BitFormat = (Boolean_t)((SCO_Data_Format.SCO_PCM_Data_Sample_Size == ds8Bit)?TRUE:FALSE);
            else
               AudioData8BitFormat = FALSE;

            printf("Current SCO Parameters: %s, %u bit, %s Test Mode active.\n", (char *)((AudioTransport == sptCodec)?"CODEC":"HCI"), AudioData8BitFormat?8:16, (AudioTestMode == AUDIO_DATA_TEST_NONE)?"No":((AudioTestMode == AUDIO_DATA_TEST_1KHZ_TONE)?"1KHz Tone":"Loopback"));

            /* If the driver type is USB, then make sure that the Flow  */
            /* Control is disabled.                                     */
            if(HCI_DriverInformation->DriverType == hdtUSB)
            {
               /* Read the size of the SCO packet buffer.               */
               if(!HCI_Read_Buffer_Size(BluetoothStackID, &Status, &tmpWord, &PacketLength, &tmpWord, &tmpWord))
               {
                  /* Set the number of Outstanding packkets to Zero to  */
                  /* disable Flow Control.                              */
                  SCO_Packet_Information.MaximumOutstandingSCOPackets = 0;
                  SCO_Packet_Information.MaximumSCOPacketSize         = PacketLength;
                  SCO_Change_Packet_Information(BluetoothStackID, &SCO_Packet_Information);
               }
            }

            /* Flag that an Audio Connection is not present.            */
            AudioConnected = FALSE;
            SelectedCodec  = 1;
            RequestedCodec = 0;

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

            NotificationList = NULL;

            Result = HCI_Read_Local_Supported_Codecs(BluetoothStackID, &Status, &Codecs);
            if((!Result) && (!Status))
            {
               WBS_Supported = TRUE;
               printf("WBS Supported in Controller\n");
               HCI_Free_Local_Supported_Codec_Info(&Codecs);
            }
            else
            {
               WBS_Supported = FALSE;
               printf("No WBS Support in Controller\n");
            }
         }
         else
         {
            /* The Stack was NOT initialized successfully, inform the   */
            /* user and set the return value of the initialization      */
            /* function to an error.                                    */
            if(HCI_DriverInformation->DriverType == hdtUSB)
               printf("Stack Initialization on USB Failed: %d.\n", Result);
            else
            {
               if(HCI_DriverInformation->DriverInformation.COMMDriverInformation.COMPortNumber == -1)
               {
                  printf("Stack Initialization using device file %s %ld (%s) Failed: %d.\n", HCI_DriverInformation->DriverInformation.COMMDriverInformation.COMDeviceName, HCI_DriverInformation->DriverInformation.COMMDriverInformation.BaudRate, ((HCI_DriverInformation->DriverInformation.COMMDriverInformation.Protocol == cpBCSP)?"BCSP":"UART"), Result);
               }
               else
               {
                  printf("Stack Initialization on Port %d %ld (%s) Failed: %d.\n", HCI_DriverInformation->DriverInformation.COMMDriverInformation.COMPortNumber, HCI_DriverInformation->DriverInformation.COMMDriverInformation.BaudRate, ((HCI_DriverInformation->DriverInformation.COMMDriverInformation.Protocol == cpBCSP)?"BCSP":"UART"), Result);
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
      printf("Stack Already Initialized.\n");

      ret_val = 0;
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
   int             ret_val;
   Notification_t *NotificationEntry;

   /* First check to see if the Stack has been opened.                  */
   if(BluetoothStackID)
   {
      /* Cleanup the notification list.                                 */
      while(NotificationList)
      {
         NotificationEntry = NotificationList;
         NotificationList  = NotificationList->NextNotification;
         BTPS_FreeMemory(NotificationEntry);
      }

      /* If debugging is enabled, go ahead and clean it up.             */
      if(DebugID)
         BTPS_Debug_Cleanup(BluetoothStackID, DebugID);

      /* Simply close the Stack                                         */
      BSC_Shutdown(BluetoothStackID);

      printf("Stack Shutdown Successfully.\n");

      /* Flag that the Stack is no longer initialized.                  */
      BluetoothStackID = 0;

      DebugID          = 0;

      /* Flag success to the caller.                                    */
      ret_val          = 0;
   }
   else
   {
      /* A valid Stack ID does not exist, inform to user.               */
      printf("Stack not Initialized.\n");

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
         printf("GAP_Set_Discoverability_Mode(dmGeneralDiscoverable, 0).\n");
      }
      else
      {
         /* An error occurred while trying to set the Discoverability   */
         /* Mode of the Device.                                         */
         printf("Set Discoverable Mode Command Error : %d.\n", ret_val);
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
         printf("GAP_Set_Connectability_Mode(cmConnectable).\n");
      }
      else
      {
         /* An error occurred while trying to make the Device           */
         /* Connectable.                                                */
         printf("Set Connectability Mode Command Error : %d.\n", ret_val);
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
   int ret_val;

   /* First, check that a valid Bluetooth Stack ID exists.              */
   if(BluetoothStackID)
   {
      /* Attempt to set the attached device to be pairable.             */
      ret_val = GAP_Set_Pairability_Mode(BluetoothStackID, pmPairableMode);

      /* Next, check the return value of the GAP Set Pairability mode   */
      /* command for successful execution.                              */
      if(!ret_val)
      {
         /* The command appears to have been successful.  The attached  */
         /* device is now in pairable mode.                             */
         printf("GAP_Set_Pairability_Mode(pmPairableMode).\n");

         /* The device has been set to pairable mode, now register an   */
         /* Authentication Callback to handle the Authentication events */
         /* if required.                                                */
         ret_val = GAP_Register_Remote_Authentication(BluetoothStackID, GAP_Event_Callback, (unsigned long)0);

         /* Next, check the return value of the GAP Register Remote     */
         /* Authentication command for successful execution.            */
         if(!ret_val)
         {
            /* The command appears to have been successful.             */
            printf("GAP_Register_Remote_Authentication() Success.\n");
         }
         else
         {
            /* An error occurred while trying to execute this function. */
            printf("GAP_Register_Remote_Authentication() Failure: %d\n", ret_val);
         }
      }
      else
      {
         /* An error occurred while trying to make the device pairable. */
         printf("Set Pairability Mode Command Error : %d.\n", ret_val);
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
      printf("Deleting Stored Link Key(s) FAILED!\n");

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

   /* Add the Indicator name to the Notification list if it not already */
   /* in the list.                                                      */
static int AddNotificationIndicator(char *Indicator)
{
   int             ret_val = 0;
   int             Length;
   Notification_t *NotificationEntry;

   /* Verify the parameter appears valid.                               */
   if(Indicator)
   {
      /* Get the length of the indicator.                               */
      Length = BTPS_StringLength(Indicator);

      /* Check against all registered notifications.                    */
      NotificationEntry = NotificationList;
      while(NotificationEntry)
      {
         /* Check for a match.                                          */
         if(!BTPS_MemCompareI(NotificationEntry->Indicator, Indicator, Length))
            break;
         /* Get the next indicator in the list.                         */
         NotificationEntry = NotificationEntry->NextNotification;
      }

      /* If a match was not located, add it to the end of the list.     */
      if(!NotificationEntry)
      {
         /* Allocate memory for the next entry.                         */
         NotificationEntry = (Notification_t *)BTPS_AllocateMemory(NOTIFICATION_DATA_SIZE(Length+1));
         if(NotificationEntry)
         {
            BTPS_MemInitialize(NotificationEntry, 0, NOTIFICATION_DATA_SIZE(Length+1));
            NotificationEntry->Indicator = (char *)(&NotificationEntry[1]);
            BTPS_StringCopy(NotificationEntry->Indicator, Indicator);
            NotificationEntry->Enabled = TRUE;
            BSC_AddGenericListEntry_Actual(ekNone, 0, 0, (void **)&NotificationList, (void *)NotificationEntry);
            ret_val = 1;
         }
      }
   }

   return(ret_val);
}

   /* Total the number of entries in the Notification List.             */
static int GetNotificationListCount(void)
{
   int             ret_val = 0;
   Notification_t *NotificationEntry;

   /* Start at the head of the list.                                    */
   NotificationEntry = NotificationList;
   while(NotificationEntry)
   {
      ret_val++;
      NotificationEntry = NotificationEntry->NextNotification;
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
   int ret_val;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Use the GAP_Perform_Inquiry() function to perform an Inquiry.  */
      /* The Inquiry will last the specified amount of time or until the*/
      /* specified number of Bluetooth Device are found.  When the      */
      /* Inquiry Results become available the GAP_Event_Callback is     */
      /* called.                                                        */
      ret_val = GAP_Perform_Inquiry(BluetoothStackID, itGeneralInquiry, 0, 0, 10, MAX_INQUIRY_RESULTS, GAP_Event_Callback, 0);

      /* Next, check to see if the GAP_Perform_Inquiry() function was   */
      /* successful.                                                    */
      if(!ret_val)
      {
         /* The Inquiry appears to have been sent successfully.         */
         /* Processing of the results returned from this command occurs */
         /* within the GAP_Event_Callback() function.                   */
         printf("Return Value is %d GAP_Perform_Inquiry() SUCCESS.\n", ret_val);

         NumberofValidResponses = 0;
      }
      else
      {
         /* A error occurred while performing the Inquiry.              */
         printf("Return Value is %d GAP_Perform_Inquiry() FAILURE.\n", ret_val);
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
   /* Bluetooth Device was located in.  This function returns zero on   */
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
      printf("Inquiry List: %d Devices%s\n\n", NumberofValidResponses, NumberofValidResponses?":":".");

      for(Index=0;Index<NumberofValidResponses;Index++)
      {
         BD_ADDRToStr(InquiryResultList[Index], BoardStr);

         printf(" Inquiry Result: %d, %s.\n", (Index+1), BoardStr);
      }

      if(NumberofValidResponses)
         printf("\n");

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
            printf("Discoverability Mode successfully set to: %s Discoverable.\n", (DiscoverabilityMode == dmNonDiscoverableMode)?"Non":((DiscoverabilityMode == dmGeneralDiscoverableMode)?"General":"Limited"));

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            /* There was an error setting the Mode.                     */
            printf("GAP_Set_Discoverability_Mode() Failure: %d.\n", Result);

            /* Flag that an error occurred while submitting the command.*/
            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: SetDiscoverabilityMode [Mode (0 = Non Discoverable, 1 = Limited Discoverable, 2 = General Discoverable)].\n");

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
            printf("Connectability Mode successfully set to: %s.\n", (ConnectableMode == cmNonConnectableMode)?"Non Connectable":"Connectable");

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            /* There was an error setting the Mode.                     */
            printf("GAP_Set_Connectability_Mode() Failure: %d.\n", Result);

            /* Flag that an error occurred while submitting the command.*/
            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: SetConnectabilityMode [Mode (0 = Non Conectable, 1 = Connectable)].\n");

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
            printf("Pairability Mode successfully set to: %s.\n", (PairabilityMode == pmNonPairableMode)?"Non Pairable":((PairabilityMode == pmPairableMode)?"Pairable":"Pairable (Secure Simple Pairing)"));

            /* If Secure Simple Pairing has been enabled, inform the    */
            /* user of the current Secure Simple Pairing parameters.    */
            if(PairabilityMode == pmPairableMode_EnableSecureSimplePairing)
               printf("Current I/O Capabilities: %s, MITM Protection: %s.\n", IOCapabilitiesStrings[(unsigned int)IOCapability], MITMProtection?"TRUE":"FALSE");

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            /* There was an error setting the Mode.                     */
            printf("GAP_Set_Pairability_Mode() Failure: %d.\n", Result);

            /* Flag that an error occurred while submitting the command.*/
            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: SetPairabilityMode [Mode (0 = Non Pairable, 1 = Pairable, 2 = Pairable (Secure Simple Pairing)].\n");

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

         /* Finally map the Man in the Middle (MITM) Protection value.  */
         MITMProtection = (Boolean_t)(TempParam->Params[1].intParam?TRUE:FALSE);

         /* Inform the user of the New I/O Capabilities.                */
         printf("Current I/O Capabilities: %s, MITM Protection: %s.\n", IOCapabilitiesStrings[(unsigned int)IOCapability], MITMProtection?"TRUE":"FALSE");

         /* Flag success to the caller.                                 */
         ret_val = 0;
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: ChangeSimplePairingParameters [I/O Capability (0 = Display Only, 1 = Display Yes/No, 2 = Keyboard Only, 3 = No Input/Output)] [MITM Requirement (0 = No, 1 = Yes)].\n");

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

   /* The following function is responsible for clearing the link key   */
   /* saved during bonding.  This special purpose functionality was     */
   /* added to support PTS testing, and may not be desired for general. */
   /* demonstration purposes.                                           */
static int ClearSavedLinkKey(ParameterList_t *TempParam)
{
   int       ret_val;
   BD_ADDR_t NULL_BD_ADDR;

   ASSIGN_BD_ADDR(NULL_BD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   ret_val = DeleteLinkKey(NULL_BD_ADDR);

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
      if((!ServerConnected) && (!CurrentClientPortID))
      {
         /* There is currently no active connection, make sure that all */
         /* of the parameters required for this function appear to be at*/
         /* least semi-valid.                                           */
         if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].intParam) && ((int)NumberofValidResponses) && (TempParam->Params[0].intParam <= (int)NumberofValidResponses) && (!COMPARE_BD_ADDR(InquiryResultList[(TempParam->Params[0].intParam - 1)], NullADDR)))
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
               /* Display a message indicating that Bonding was         */
               /* initiated successfully.                               */
               printf("GAP_Initiate_Bonding (%s): Function Successful.\n", (BondingType == btDedicated)?"Dedicated":"General");

               /* Flag success to the caller.                           */
               ret_val = 0;
            }
            else
            {
               /* Display a message indicating that an error occurred   */
               /* while initiating bonding.                             */
               printf("GAP_Initiate_Bonding() Failure: %d.\n", Result);

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* One or more of the necessary parameters is/are invalid.  */
            printf("Usage: Pair [Inquiry Index] [Bonding Type (0 = Dedicated, 1 = General) (optional).\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* Display an error to the user describing that Pairing can    */
         /* only occur when we are not connected.                       */
         printf("The Pair command can only be issued when not already connected.\n");

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

   /* The following function is responsible for deleting an existing    */
   /* Link Key from the Link Key List.  This function returns zero on   */
   /* successful execution and a negative value on all errors.          */
static int UnPair(ParameterList_t *TempParam)
{
   int       ret_val;
   BD_ADDR_t NullADDR;

   ASSIGN_BD_ADDR(NullADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].intParam) && (NumberofValidResponses) && (TempParam->Params[0].intParam <= (int)NumberofValidResponses) && (!COMPARE_BD_ADDR(InquiryResultList[(TempParam->Params[0].intParam - 1)], NullADDR)))
      {
         /* We need to clear out any Link Key we have stored for the    */
         /* specified device.                                           */
         DeleteLinkKey(InquiryResultList[(TempParam->Params[0].intParam - 1)]);

         printf("Device has been UnPaired.\n");
         ret_val = 0;
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: UnPair [Inquiry Index].\n");

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
      if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].intParam) && (NumberofValidResponses) && (TempParam->Params[0].intParam <= (int)NumberofValidResponses) && (!COMPARE_BD_ADDR(InquiryResultList[(TempParam->Params[0].intParam - 1)], NullADDR)))
      {
         /* Attempt to submit the command.                              */
         Result = GAP_End_Bonding(BluetoothStackID, InquiryResultList[(TempParam->Params[0].intParam - 1)]);

         /* Check the return value of the submitted command for success.*/
         if(!Result)
         {
            /* Display a message indicating that the End bonding was    */
            /* successfully submitted.                                  */
            printf("GAP_End_Bonding: Function Successful.\n");

            /* Flag success to the caller.                              */
            ret_val = 0;

            /* Flag that there is no longer a current Authentication    */
            /* procedure in progress.                                   */
            ASSIGN_BD_ADDR(CurrentRemoteBD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
         }
         else
         {
            /* Display a message indicating that an error occurred while*/
            /* ending bonding.                                          */
            printf("GAP_End_Bonding() Failure: %d.\n", Result);

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: EndPairing [Inquiry Index].\n");

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
               printf("GAP_Authentication_Response(), Pin Code Response Success.\n");

               /* Flag success to the caller.                           */
               ret_val = 0;
            }
            else
            {
               /* Inform the user that the Authentication Response was  */
               /* not successful.                                       */
               printf("GAP_Authentication_Response() Failure: %d.\n", Result);

               ret_val = FUNCTION_ERROR;
            }

            /* Flag that there is no longer a current Authentication    */
            /* procedure in progress.                                   */
            ASSIGN_BD_ADDR(CurrentRemoteBD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
         }
         else
         {
            /* One or more of the necessary parameters is/are invalid.  */
            printf("Usage: PINCodeResponse [PIN Code].\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* There is not currently an on-going authentication operation,*/
         /* inform the user of this error condition.                    */
         printf("Unable to issue PIN Code Authentication Response: Authentication is not currently in progress.\n");

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
               printf("GAP_Authentication_Response(), Passkey Response Success.\n");

               /* Flag success to the caller.                           */
               ret_val = 0;
            }
            else
            {
               /* Inform the user that the Authentication Response was  */
               /* not successful.                                       */
               printf("GAP_Authentication_Response() Failure: %d.\n", Result);

               ret_val = FUNCTION_ERROR;
            }

            /* Flag that there is no longer a current Authentication    */
            /* procedure in progress.                                   */
            ASSIGN_BD_ADDR(CurrentRemoteBD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
         }
         else
         {
            /* One or more of the necessary parameters is/are invalid.  */
            printf("Usage: PassKeyResponse [Numeric Passkey (0 - 999999)].\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* There is not currently an on-going authentication operation,*/
         /* inform the user of this error condition.                    */
         printf("Unable to issue Pass Key Authentication Response: Authentication is not currently in progress.\n");

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
               printf("GAP_Authentication_Response(), User Confirmation Response Success.\n");

               /* Flag success to the caller.                           */
               ret_val = 0;
            }
            else
            {
               /* Inform the user that the Authentication Response was  */
               /* not successful.                                       */
               printf("GAP_Authentication_Response() Failure: %d.\n", Result);

               ret_val = FUNCTION_ERROR;
            }

            /* Flag that there is no longer a current Authentication    */
            /* procedure in progress.                                   */
            ASSIGN_BD_ADDR(CurrentRemoteBD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
         }
         else
         {
            /* One or more of the necessary parameters is/are invalid.  */
            printf("Usage: UserConfirmationResponse [Confirmation (0 = No, 1 = Yes)].\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* There is not currently an on-going authentication operation,*/
         /* inform the user of this error condition.                    */
         printf("Unable to issue User Confirmation Authentication Response: Authentication is not currently in progress.\n");

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

         printf("BD_ADDR of Local Device is: %s.\n", BoardStr);

         /* Flag success to the caller.                                 */
         ret_val = 0;
      }
      else
      {
         /* Display a message indicating that an error occurred while   */
         /* attempting to query the Local Device Address.               */
         printf("GAP_Query_Local_BD_ADDR() Failure: %d.\n", Result);

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
            /* Display a message indicating that the Device Name was    */
            /* successfully submitted.                                  */
            printf("Local Device Name set to: %s.\n", TempParam->Params[0].strParam);

            /* Rewrite extended inquiry information.                    */
            WriteEIRInformation(TempParam->Params[0].strParam);

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            /* Display a message indicating that an error occurred while*/
            /* attempting to set the local Device Name.                 */
            printf("GAP_Set_Local_Device_Name() Failure: %d.\n", Result);

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: SetLocalName [Local Name (no spaces allowed)].\n");

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
         printf("Name of Local Device is: %s.\n", LocalName);

         /* Flag success to the caller.                                 */
         ret_val = 0;
      }
      else
      {
         /* Display a message indicating that an error occurred while   */
         /* attempting to query the Local Device Name.                  */
         printf("GAP_Query_Local_Device_Name() Failure: %d.\n", Result);

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
            /* Display a message indicating that the Class of Device was*/
            /* successfully submitted.                                  */
            printf("Set Class of Device to 0x%02X%02X%02X.\n", Class_of_Device.Class_of_Device0, Class_of_Device.Class_of_Device1, Class_of_Device.Class_of_Device2);

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            /* Display a message indicating that an error occurred while*/
            /* attempting to set the local Class of Device.             */
            printf("GAP_Set_Class_Of_Device() Failure: %d.\n", Result);

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: SetClassOfDevice [Class of Device].\n");

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
         printf("Local Class of Device is: 0x%02X%02X%02X.\n", Class_of_Device.Class_of_Device0, Class_of_Device.Class_of_Device1, Class_of_Device.Class_of_Device2);

         /* Flag success to the caller.                                 */
         ret_val = 0;
      }
      else
      {
         /* Display a message indicating that an error occurred while   */
         /* attempting to query the Local Class of Device.              */
         printf("GAP_Query_Class_Of_Device() Failure: %d.\n", Result);

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
      if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].intParam) && (NumberofValidResponses) && (TempParam->Params[0].intParam <= (int)NumberofValidResponses) && (!COMPARE_BD_ADDR(InquiryResultList[(TempParam->Params[0].intParam - 1)], NullADDR)))
      {
         /* Attempt to submit the command.                              */
         Result = GAP_Query_Remote_Device_Name(BluetoothStackID, InquiryResultList[(TempParam->Params[0].intParam - 1)], GAP_Event_Callback, (unsigned long)0);

         /* Check the return value of the submitted command for success.*/
         if(!Result)
         {
            /* Display a message indicating that Remote Name request was*/
            /* initiated successfully.                                  */
            printf("GAP_Query_Remote_Device_Name: Function Successful.\n");

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            /* Display a message indicating that an error occurred while*/
            /* initiating the Remote Name request.                      */
            printf("GAP_Query_Remote_Device_Name() Failure: %d.\n", Result);

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: GetRemoteName [Inquiry Index].\n");

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
      if((TempParam) && (TempParam->NumberofParameters > 1) && (((TempParam->Params[1].intParam) && (TempParam->Params[1].intParam <= NUM_UUIDS)) || ((!TempParam->Params[1].intParam) && (TempParam->NumberofParameters > 2))) && (TempParam->Params[0].intParam) && (NumberofValidResponses) && (TempParam->Params[0].intParam <= (int)NumberofValidResponses) && (!COMPARE_BD_ADDR(InquiryResultList[(TempParam->Params[0].intParam - 1)], NullADDR)))
      {
         /* OK, parameters appear to be semi-valid, now let's attempt to*/
         /* issue the SDP Service Attribute Request.                    */
         if(!TempParam->Params[1].intParam)
         {
            /* First let's build the UUID 32 value(s).                  */
            SDPUUIDEntry.SDP_Data_Element_Type = deUUID_32;

            ASSIGN_SDP_UUID_32(SDPUUIDEntry.UUID_Value.UUID_32, (Byte_t)((TempParam->Params[2].intParam & 0xFF000000) >> 24), (Byte_t)((TempParam->Params[2].intParam & 0x00FF0000) >> 16), (Byte_t)((TempParam->Params[2].intParam & 0x0000FF00) >> 8), (Byte_t)(TempParam->Params[2].intParam & 0x000000FF));
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
            printf("SDP_Service_Search_Attribute_Request(%s) Success.\n", TempParam->Params[1].intParam?UUIDTable[TempParam->Params[1].intParam - 1].Name:"Manual");

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            /* There was an error submitting the SDP Request.           */
            printf("SDP_Service_Search_Attribute_Request(%s) Failure: %d.\n", TempParam->Params[1].intParam?UUIDTable[TempParam->Params[1].intParam - 1].Name:"Manual", Result);

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: ServiceDiscovery [Inquiry Index] [Profile Index] [16/32 bit UUID (Manual only)].\n");
         printf("\n   Profile Index:\n");
         printf("       0) Manual (MUST specify 16/32 bit UUID)\n");
         for(Index=0;Index<NUM_UUIDS;Index++)
            printf("      %2d) %s\n", Index + 1, UUIDTable[Index].Name);
         printf("\n");

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

   /* The following function is responsible for changing the operating  */
   /* mode to require, or not require, authentication on a connection.  */
static int SetAuthenticationMode(ParameterList_t *TempParam)
{
   int ret_val;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Next, make sure that we are not already connected.             */
      if((!ServerConnected) && (!CurrentClientPortID))
      {
         /* The program is running in client mode, make sure that all of*/
         /* the parameters required for this function appear to be at   */
         /* least semi-valid.                                           */
         if((TempParam) && (TempParam->NumberofParameters > 0))
         {
            /* Attempt to submit the command.                           */
            ret_val = GAP_Set_Authentication_Mode(BluetoothStackID, (GAP_Authentication_Mode_t) TempParam->Params[0].intParam);

            /* Check the return value of the submitted command for      */
            /* success.                                                 */
            if(!ret_val)
            {
               /* Display a message indicating that Bonding was         */
               /* initiated successfully.                               */
               printf("GAP_Set_Authentication_Mode: Function Successful.\n");
            }
            else
            {
               /* Display a message indicating that an error occurred   */
               /* while initiating bonding.                             */
               printf("GAP_Set_Authentication_Mode() Failure: %d.\n", ret_val);
            }
         }
         else
         {
            /* One or more of the necessary parameters is/are invalid.  */
            printf("Usage: SetAuthenticationMode [Disable = 0, Enable = 1].\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* Display an error to the user describing that Pairing can    */
         /* only occur when we are not connected.                       */
         printf("The SetAuthenticationMode command can only be issued when not already connected.\n");

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

   /* The following function is responsible for changing the SCO        */
   /* Physical Transport.  The input parameter to this function         */
   /* specifies the SCO Transport parameter.  This function returns zero*/
   /* if successful and a negative value if an error occurred.          */
static int SetSCOTransport(ParameterList_t *TempParam)
{
   int                      ret_val;
   int                      Result;
   SCO_Physical_Transport_t Transport;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      if(!AudioConnected)
      {
         /* Next, determine if the specified Transport is a valid       */
         /* transport.                                                  */
         if((TempParam) && (TempParam->NumberofParameters > 0))
         {
            /* Initialize success.                                      */
            Result = 0;

            switch(TempParam->Params[0].intParam)
            {
               case 0:
                  Transport = sptCodec;
                  break;
               case 1:
                  Transport = sptHCI;
                  break;
               default:
                  Result    = INVALID_PARAMETERS_ERROR;
                  break;
            }

            if(!Result)
            {
               Result = SCO_Set_Physical_Transport(BluetoothStackID, Transport);

               if(!Result)
               {
                  /* SCO_Set_Physical_Transport() command was issued    */
                  /* successfully.                                      */
                  printf("SCO_Set_Physical_Transport(%s) Success.\n", (Transport == sptCodec)?"CODEC":"HCI");

                  /* Flag success to the caller.                        */
                  AudioTransport = Transport;
                  ret_val        = 0;
               }
               else
               {
                  /* Error submitting the SCO_Set_Physical_Transport()  */
                  /* command so flag the error result to the user.      */
                  printf("SCO_Set_Physical_Transport() Failure: %d.\n", Result);

                  /* Flag that an error occurred while submitting the   */
                  /* command.                                           */
                  ret_val = Result;
               }
            }
            else
            {
               /* Invalid parameters specified so flag an error to the  */
               /* user.                                                 */
               printf("Usage: SetSCOTransport [Transport (Codec = 0, HCI = 1)].\n");

               /* Flag that an error occurred while submitting the      */
               /* command.                                              */
               ret_val = INVALID_PARAMETERS_ERROR;
            }
         }
         else
         {
            /* Invalid parameters specified so flag an error to the     */
            /* user.                                                    */
            printf("Usage: SetSCOTransport [Transport (Codec = 0, HCI = 1)].\n");

            /* Flag that invalid parameters were specified.             */
            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* Inform the user that the Physical Transport cannot be       */
         /* changed while a SCO connection is present.                  */
         printf("Cannot change Physical Transport while SCO Connection is active.\n");

         /* Flag that an error occurred.                                */
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

   /* The following function is responsible for changing the SCO Data   */
   /* Format (8 bit PCM data or 16 bit PCM data).  The input parameter  */
   /* to this function specifies the SCO Data Format.  This function    */
   /* returns zero if successful and a negative value if an error       */
   /* occurred.                                                         */
static int SetSCODataFormat(ParameterList_t *TempParam)
{
   int               Result;
   int               ret_val;
   SCO_Data_Format_t SCO_Data_Format;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      if(!AudioConnected)
      {
         /* Next, check to make sure that a valid Data format was       */
         /* specified.                                                  */
         if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].intParam >= 0) && (TempParam->Params[0].intParam <= 1))
         {
            /* Parameters appear to be valid.  First query the current  */
            /* SCO Data Format (so we can only change the parameters we */
            /* are interested in).                                      */
            Result = SCO_Query_Data_Format(BluetoothStackID, &SCO_Data_Format);

            if(Result >= 0)
            {
               /* Format queried, successfully, note the new Sample     */
               /* Size.                                                 */
               SCO_Data_Format.SCO_PCM_Data_Sample_Size = (SCO_PCM_Data_Sample_Size_t)(TempParam->Params[0].intParam?ds16Bit:ds8Bit);

               /* Write out the SCO Data Format settings.               */
               Result = SCO_Change_Data_Format(BluetoothStackID, &SCO_Data_Format);

               if(Result >= 0)
               {
                  /* Note the new data format.                          */
                  if(!TempParam->Params[0].intParam)
                     AudioData8BitFormat = TRUE;
                  else
                     AudioData8BitFormat = FALSE;

                  printf("SCO Sample Size successfully changed to %u bit PCM.\n", (!TempParam->Params[0].intParam)?8:16);

                  /* Flag success to the caller.                        */
                  ret_val = 0;
               }
               else
               {
                  printf("SCO_Change_Data_Format(): Function Error %d.\n", Result);

                  /* Flag that an error occurred.                       */
                  ret_val = FUNCTION_ERROR;
               }
            }
            else
            {
               printf("SCO_Query_Data_Format(): Function Error %d.\n", Result);

               /* Flag that an error occurred.                          */
               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* Invalid parameters specified so flag an error to the     */
            /* user.                                                    */
            printf("Usage: SetSCODataFormat [Sample Size (8 bit = 0, 16 bit = 1)].\n");

            /* Flag that invalid parameters were specified.             */
            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* Inform the user that it is not allowable to change the SCO  */
         /* Data Format while there is an ongoing SCO Connection.       */
         printf("Cannot change Data Format while SCO Connection is active.\n");

         /* Flag that an error occurred.                                */
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

   /* The following function is responsible for changing the current SCO*/
   /* Data Test Mode (None/1KHz Sine Wave/Loopback).  The input         */
   /* parameter to this function specifies the new SCO Test Mode.  This */
   /* function returns zero if successful and a negative value if an    */
   /* error occurred.                                                   */
static int SetSCOTestMode(ParameterList_t *TempParam)
{
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Next, check to make sure that a valid Data format was          */
      /* specified.                                                     */
      if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].intParam >= 0) && (TempParam->Params[0].intParam <= 2))
      {
         /* Verify that the transport is set to HCI.                    */
         if(AudioTransport == sptHCI)
         {
            /* Parameters appear to be valid.  Go ahead and set the new */
            /* test mode.                                               */
            if(TempParam->Params[0].intParam == 2)
               AudioTestMode = AUDIO_DATA_TEST_LOOPBACK;
            else
            {
               if(TempParam->Params[0].intParam == 1)
               {
                  AudioTestMode      = AUDIO_DATA_TEST_1KHZ_TONE;

                  /* Reset the SCO Tone Index.                          */
                  AudioDataToneIndex = 0;
               }
               else
                  AudioTestMode = AUDIO_DATA_TEST_NONE;
            }

            printf("SCO Test Mode Set to: %s.\n", (AudioTestMode == AUDIO_DATA_TEST_NONE)?"None":((AudioTestMode == AUDIO_DATA_TEST_1KHZ_TONE)?"1 KHz Tone":"Loopback"));

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            printf("This feature can only be used with HCI Transport.\n");
            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* Invalid parameters specified so flag an error to the user.  */
         printf("Usage: SetSCOTestMode [Mode (None = 0, 1KHz Tone = 1, Loopback = 2)].\n");

         /* Flag that invalid parameters were specified.                */
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

   /* The following function is responsible for opening an Audio Gateway*/
   /* Server.  This function returns zero on successful execution and a */
   /* negative value on all errors.                                     */
static int HFREOpenAudioGatewayServer(void)
{
   int           Result;
   int           ret_val;
   char          ServiceName[132];
   unsigned long SupportedFeatures;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Now check to make sure that a Server doesn't already exist.    */
      if(!CurrentServerPortID)
      {
         /* Add support for Codec Negotiation.                          */
         SupportedFeatures = DEFAULT_AG_SUPPORTED_FEATURES;
         if(WBS_Supported)
            SupportedFeatures |= HFRE_AG_CODEC_NEGOTIATION_SUPPORTED_BIT;

         /* Now try and open an Audio Gateway Server Port.              */
         Result = HFRE_Open_Audio_Gateway_Server_Port(BluetoothStackID, LOCAL_SERVER_CHANNEL_ID, SupportedFeatures, DEFAULT_CALL_HOLDING_SUPPORT, 0, NULL, HFRE_Event_Callback, (unsigned long)0);

         /* Check to see if the call was executed successfully.         */
         if(Result > 0)
         {
            /* The Server was successfully opened.  Save the returned   */
            /* result as the Current Server Port ID because it will be  */
            /* used by later function calls.                            */
            CurrentServerPortID = Result;

            printf("HFRE_Open_Audio_Gateway_Server_Port: Function Successful.\n");

            /* The Server was opened successfully, now register a SDP   */
            /* Record indicating that an Audio Gateway Server exists.   */
            /* Do this by first creating a Service Name.                */
            sprintf(ServiceName, "Audio Gateway Port %u", LOCAL_SERVER_CHANNEL_ID);

            /* Now that a Service Name has been created try and Register*/
            /* the SDP Record.                                          */
            Result = HFRE_Register_Audio_Gateway_SDP_Record(BluetoothStackID, CurrentServerPortID, HFRE_NETWORK_TYPE_ABILITY_TO_REJECT_CALLS, ServiceName, &HFREServerSDPHandle);

            /* Check the result of the above function call for success. */
            if(!Result)
            {
               /* Display a message indicating that the SDP Record for  */
               /* the Audio Gateway Server was registered successfully. */
               printf("HFRE_Register_Audio_Gateway_SDP_Record: Function Successful.\n");

               ret_val = Result;
            }
            else
            {
               /* Display an Error Message and make sure the Audio      */
               /* Gateway Server SDP Handle is invalid, and close the   */
               /* Audio Gateway Server Port we just opened because we   */
               /* weren't able to register a SDP Record.                */
               printf("HFRE_Register_Audio_Gateway_SDP_Record: Function Failure.\n");

               HFREServerSDPHandle = 0;

               ret_val             = Result;

               /* Now try and close the opened Port.                    */
               Result = HFRE_Close_Server_Port(BluetoothStackID, CurrentServerPortID);

               CurrentServerPortID = 0;

               /* Next check the return value of the issued command see */
               /* if it was successful.                                 */
               if(!Result)
               {
                  /* Display a message indicating that the Port was     */
                  /* successfully closed.                               */
                  printf("HFRE_Close_Server_Port: Function Successful.\n");

                  /* Flag that an Audio Connection is not present.      */
                  AudioConnected = FALSE;
               }
               else
               {
                  /* An error occurred while attempting to close the    */
                  /* Port.                                              */
                  printf("HFRE_Close_Server_Port() Failure: %d.\n", Result);
               }
            }
         }
         else
         {
            /* There was an error while trying to open the Audio Gateway*/
            /* Server.                                                  */
            printf("HFRE_Open_Audio_Gateway_Server_Port() Failure: %d.\n", Result);

            ret_val = Result;
         }
      }
      else
      {
         /* A Port is already open, this program only supports one      */
         /* connection at a time.                                       */
         printf("Audio Gateway Server Port already open.\n");

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

   /* The following function is responsible for opening a Hands-Free    */
   /* Server.  This function returns zero on successful execution and a */
   /* negative value on all errors.                                     */
static int HFREOpenHandsFreeServer(void)
{
   int               Result;
   int               ret_val;
   char              ServiceName[132];
   Class_of_Device_t ClassOfDevice;
   unsigned long     SupportedFeatures;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Now check to make sure that a Server doesn't already exist.    */
      if(!CurrentServerPortID)
      {
         /* Add support for Codec Negotiation.                          */
         SupportedFeatures = DEFAULT_HF_SUPPORTED_FEATURES;
         if(WBS_Supported)
            SupportedFeatures |= HFRE_HF_CODEC_NEGOTIATION_SUPPORTED_BIT;

         /* Now try and open a HandsFree Server Port.                   */
         Result = HFRE_Open_HandsFree_Server_Port(BluetoothStackID, LOCAL_SERVER_CHANNEL_ID, SupportedFeatures, 0, NULL, HFRE_Event_Callback, (unsigned long)0);

         /* Check to see if the call was executed successfully.         */
         if(Result > 0)
         {
            /* The Server was successfully opened.  Save the returned   */
            /* result as the Current Server Port ID because it will be  */
            /* used by later function calls.                            */
            CurrentServerPortID = Result;

            printf("HFRE_Open_HandsFree_Server_Port: Function Successful.\n");

            /* Finally let's make sure the Class of Device is set       */
            /* correctly.                                               */
            if(!GAP_Query_Class_Of_Device(BluetoothStackID, &ClassOfDevice))
            {
               SET_MAJOR_DEVICE_CLASS(ClassOfDevice, HCI_LMP_CLASS_OF_DEVICE_MAJOR_DEVICE_CLASS_AUDIO_VIDEO);
               SET_MINOR_DEVICE_CLASS(ClassOfDevice, HCI_LMP_CLASS_OF_DEVICE_MINOR_DEVICE_CLASS_AUDIO_VIDEO_HANDS_FREE);

               /* Write out the Class of Device.                        */
               GAP_Set_Class_Of_Device(BluetoothStackID, ClassOfDevice);
            }

            /* The Server was opened successfully, now register a SDP   */
            /* Record indicating that a HandsFree Server exists.  Do    */
            /* this by first creating a Service Name.                   */
            sprintf(ServiceName, "HandsFree Port %u", LOCAL_SERVER_CHANNEL_ID);

            /* Now that a Service Name has been created try and Register*/
            /* the SDP Record.                                          */
            Result = HFRE_Register_HandsFree_SDP_Record(BluetoothStackID, CurrentServerPortID, ServiceName, &HFREServerSDPHandle);

            /* Check the result of the above function call for success. */
            if(!Result)
            {
               /* Display a message indicating that the SDP record for  */
               /* the HandsFree Server was registered successfully.     */
               printf("HFRE_Register_HandsFree_SDP_Record: Function Successful.\n");

               /* Initialize Indicator Notifications                    */
               NotificationList = NULL;

               ret_val = Result;
            }
            else
            {
               /* Display an Error Message and make sure the Current    */
               /* Server SDP Handle is invalid, and close the HandsFree */
               /* Server Port we just opened because we weren't able to */
               /* register a SDP Record.                                */
               printf("HFRE_Register_HandsFree_SDP_Record: Function Failure.\n");

               HFREServerSDPHandle = 0;

               ret_val             = Result;

               /* Now try and close the opened Port.                    */
               Result = HFRE_Close_Server_Port(BluetoothStackID, CurrentServerPortID);

               CurrentServerPortID = 0;

               /* Next check the return value of the issued command see */
               /* if it was successful.                                 */
               if(!Result)
               {
                  /* Display a message indicating that the Port was     */
                  /* successfully closed.                               */
                  printf("HFRE_Close_Server_Port: Function Successful.\n");

                  /* Flag that an Audio Connection is not present.      */
                  AudioConnected = FALSE;
               }
               else
               {
                  /* An error occurred while attempting to close the    */
                  /* Port.                                              */
                  printf("HFRE_Close_Server_Port() Failure: %d.\n", Result);
               }
            }
         }
         else
         {
            /* There was an error while trying to open the HandsFree    */
            /* Server.                                                  */
            printf("HFRE_Open_HandsFree_Server_Port() Failure: %d.\n", Result);

            ret_val = Result;
         }
      }
      else
      {
         /* A Port is already open, this program only supports one      */
         /* connection at a time.                                       */
         printf("HandsFree Server Port already open.\n");

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

   /* The following function is responsible for opening a Hands-Free    */
   /* client port.  This function returns zero on successful execution  */
   /* and a negative value on all errors.                               */
static int HFREOpenRemoteAudioGatewayPort(ParameterList_t *TempParam)
{
   int               Result;
   int               ret_val;
   char              BoardStr[13];
   BD_ADDR_t         NullADDR;
   Class_of_Device_t ClassOfDevice;
   unsigned long     SupportedFeatures;

   ASSIGN_BD_ADDR(NullADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Now check to make sure that a Client doesn't already exist and */
      /* that the Server is not currently connected.                    */
      if((!CurrentClientPortID) && (!ServerConnected))
      {
         /* A Client port is not already open, now check to make sure   */
         /* that the parameters required for this function appear to be */
         /* valid.                                                      */
         if((TempParam) && (TempParam->NumberofParameters > 1) && (TempParam->Params->intParam) && (NumberofValidResponses) && (TempParam->Params->intParam<=(int)NumberofValidResponses) && (!COMPARE_BD_ADDR(InquiryResultList[(TempParam->Params[0].intParam-1)], NullADDR)) && (TempParam->Params[1].intParam))
         {
            /* The above parameters are valid, inform the that the      */
            /* program is about to open a remote port.                  */
            BD_ADDRToStr(InquiryResultList[(TempParam->Params[0].intParam-1)], BoardStr);
            printf("Open Remote Audio Gateway Port(BD_ADDR = %s, Port = %04X)\n", BoardStr, TempParam->Params[1].intParam);

            /* Add support for Codec Negotiation.                       */
            SupportedFeatures = DEFAULT_HF_SUPPORTED_FEATURES;
            if(WBS_Supported)
               SupportedFeatures |= HFRE_HF_CODEC_NEGOTIATION_SUPPORTED_BIT;

            /* Attempt to open an Audio Gateway Client to the selected  */
            /* Device.                                                  */
            Result = HFRE_Open_Remote_Audio_Gateway_Port(BluetoothStackID, InquiryResultList[(TempParam->Params[0].intParam-1)], TempParam->Params[1].intParam, SupportedFeatures, 0, NULL, HFRE_Event_Callback, (unsigned long)0);

            /* Now check to see if the function call was successfully   */
            /* made.                                                    */
            if(Result > 0)
            {
               /* The Client was opened successfully.  The return value */
               /* of the call is the Current Client Port ID and is      */
               /* required for future Audio Gateway calls.              */
               CurrentClientPortID   = Result;

               printf("HFRE_Open_Remote_Audio_Gateway_Port: Function Successful (ID = %04X).\n", CurrentClientPortID);

               /* Finally let's make sure the Class of Device is set    */
               /* correctly.                                            */
               if(!GAP_Query_Class_Of_Device(BluetoothStackID, &ClassOfDevice))
               {
                  SET_MAJOR_DEVICE_CLASS(ClassOfDevice, HCI_LMP_CLASS_OF_DEVICE_MAJOR_DEVICE_CLASS_AUDIO_VIDEO);
                  SET_MINOR_DEVICE_CLASS(ClassOfDevice, HCI_LMP_CLASS_OF_DEVICE_MINOR_DEVICE_CLASS_AUDIO_VIDEO_HANDS_FREE);

                  /* Write out the Class of Device.                     */
                  GAP_Set_Class_Of_Device(BluetoothStackID, ClassOfDevice);
               }

               ret_val = 0;
            }
            else
            {
               /* There was an error opening the Client Port.           */
               printf("HFRE_Open_Remote_Audio_Gateway_Port() Failure: %d.\n", Result);

               ret_val = Result;
            }
         }
         else
         {
            /* One or more of the necessary parameters is/are invalid.  */
            printf("Usage: OpenHandsFreeClient [Inquiry Index] [Port Number].\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* A Port is already open, this program only supports one      */
         /* connection at a time.                                       */
         printf("Hands-Free Client Port already open.\n");

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

   /* The following function is responsible for opening an Audio Gateway*/
   /* client port.  This function returns zero on successful execution  */
   /* and a negative value on all errors.                               */
static int HFREOpenRemoteHandsFreePort(ParameterList_t *TempParam)
{
   int                     Result;
   int                     ret_val;
   char                    BoardStr[13];
   BD_ADDR_t               NullADDR;
   unsigned long           SupportedFeatures;
   HFRE_Indicator_Update_t Indicator[NUMBER_DEFAULT_HFR_INDICATORS];

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      ASSIGN_BD_ADDR(NullADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

      /* Now check to make sure that a Client doesn't already exist and */
      /* that the Server is not currently connected.                    */
      if((!CurrentClientPortID) && (!ServerConnected))
      {
         /* A Client port is not already open, now check to make sure   */
         /* that the parameters required for this function appear to be */
         /* valid.                                                      */
         if((TempParam) && (TempParam->NumberofParameters > 1) && (TempParam->Params->intParam) && (NumberofValidResponses) && (TempParam->Params->intParam<=(int)NumberofValidResponses) && (!COMPARE_BD_ADDR(InquiryResultList[(TempParam->Params[0].intParam-1)], NullADDR)) && (TempParam->Params[1].intParam))
         {
            /* The above parameters are valid, inform the that the      */
            /* program is about to open a remote port.                  */
            BD_ADDRToStr(InquiryResultList[(TempParam->Params->intParam-1)], BoardStr);
            printf("Open Remote HandsFree Port(BD_ADDR = %s, Port = %04X)\n", BoardStr, TempParam->Params[1].intParam);

            /* Add support for Codec Negotiation.                       */
            SupportedFeatures = DEFAULT_AG_SUPPORTED_FEATURES;
            if(WBS_Supported)
               SupportedFeatures |= HFRE_AG_CODEC_NEGOTIATION_SUPPORTED_BIT;

            /* Attempt to open a HandsFree Client to the selected       */
            /* Device.                                                  */
            Result = HFRE_Open_Remote_HandsFree_Port(BluetoothStackID, InquiryResultList[(TempParam->Params[0].intParam-1)], TempParam->Params[1].intParam, SupportedFeatures, DEFAULT_CALL_HOLDING_SUPPORT, 0, NULL, HFRE_Event_Callback, (unsigned long)0);

            /* Now check to see if the function call was successfully   */
            /* made.                                                    */
            if(Result > 0)
            {
               /* The Client was opened successfully.  The return value */
               /* of the call is the Current Client Port ID and is      */
               /* required for future HandsFree calls.                  */
               CurrentClientPortID = Result;

               printf("HFRE_Open_Remote_HandsFree_Port: Function Successful (ID = %04X).\n", CurrentClientPortID);

               ret_val = 0;

               /* Note, if you want specific control indicators to be   */
               /* sent to the handsfree device, you MUST do so here.    */
               /* The connection just made is NOT using the created     */
               /* audio gateway server, rather it is using a new server.*/
               Result = 0;
               while(Result < NUMBER_DEFAULT_HFR_INDICATORS)
               {
                  Indicator[Result].IndicatorDescription = DefaultIndicators[Result].IndicatorDescription;
                  if(DefaultIndicators[Result].ControlIndicatorType == ciBoolean)
                     Indicator[Result].Indicator_Update_Data.CurrentBooleanValue.CurrentIndicatorValue = DefaultIndicators[Result].Control_Indicator_Data.ControlIndicatorBooleanType.CurrentIndicatorValue;
                  else
                     Indicator[Result].Indicator_Update_Data.CurrentRangeValue.CurrentIndicatorValue = DefaultIndicators[Result].Control_Indicator_Data.ControlIndicatorRangeType.CurrentIndicatorValue;
                  Result++;
               }

               /* Now submit the command.                               */
               Result = HFRE_Update_Current_Control_Indicator_Status(BluetoothStackID, CurrentClientPortID, NUMBER_DEFAULT_HFR_INDICATORS, Indicator);

               printf("HFREOpenRemoteAudioGatewayPort: HFRE_Update_Current_Control_Indicator_Status Function Status %d.\n", Result);

               /* Any other specific indicator settings could be made   */
               /* as shown in the block, just add the indicators        */
            }
            else
            {
               /* There was an error opening the Client Port.           */
               printf("HFRE_Open_Remote_HandsFree_Port() Failure: %d.\n", Result);

               ret_val = Result;
            }
         }
         else
         {
            /* One or more of the necessary parameters is/are invalid.  */
            printf("Usage: OpenAudioGatewayClient [Inquiry Index] [Port Number].\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* A Port is already open, this program only supports one      */
         /* connection at a time.                                       */
         printf("Audio Gateway Client Port already open.\n");

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

   /* The following function is responsible for updating the current    */
   /* state of the Control Indicators on the Remote Hands-Free unit.    */
   /* This function returns zero on successful execution and a negative */
   /* value on all errors.                                              */
static int HFREUpdateCurrentControlIndicatorStatus(ParameterList_t *TempParam)
{
   int                     ret_val;
   int                     Result;
   int                     Index;
   int                     PortToUse;
   unsigned int            RangeLow;
   unsigned int            RangeHigh;
   HFRE_Indicator_Update_t HFREIndicatorUpdate;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      PortToUse = ((ServerConnected) || ((CurrentClientPortID == 0) && (CurrentServerPortID != 0))) ? CurrentServerPortID : CurrentClientPortID;

      /* Now check to make sure that the Port ID appears to be          */
      /* semi-valid.                                                    */
      if((PortToUse) && (TempParam))
      {
         /* The port ID appears to be at least semi-valid, now check to */
         /* make sure the passed in parameters appears to be semi-valid.*/
         if((TempParam->NumberofParameters >= 2) && ((TempParam->Params[0].intParam > 0) && (TempParam->Params[0].intParam <= NUMBER_DEFAULT_HFR_INDICATORS)))
         {
            /* Get the selected Index and make it Zero Relative.        */
            Index = TempParam->Params[0].intParam-1;

            /* Save the pointer to the Indicator Description.           */
            HFREIndicatorUpdate.IndicatorDescription = DefaultIndicators[Index].IndicatorDescription;

            if(DefaultIndicators[Index].ControlIndicatorType == ciRange)
            {
               /* Get the valid Range.                                  */
               RangeLow  = DefaultIndicators[Index].Control_Indicator_Data.ControlIndicatorRangeType.RangeStart;
               RangeHigh = DefaultIndicators[Index].Control_Indicator_Data.ControlIndicatorRangeType.RangeEnd;
            }
            else
            {
               RangeLow  = 0;
               RangeHigh = 1;
            }

            /* Check to see if the value is in the valid range.         */
            if((TempParam->Params[1].intParam >= (int)RangeLow) && (TempParam->Params[1].intParam <= (int)RangeHigh))
               ret_val = 0;
            else
               ret_val = INVALID_PARAMETERS_ERROR;

            if(!ret_val)
            {
               if(DefaultIndicators[Index].ControlIndicatorType == ciRange)
                  HFREIndicatorUpdate.Indicator_Update_Data.CurrentRangeValue.CurrentIndicatorValue = TempParam->Params[1].intParam;
               else
                  HFREIndicatorUpdate.Indicator_Update_Data.CurrentBooleanValue.CurrentIndicatorValue = (Boolean_t)TempParam->Params[1].intParam;

               printf("ControlIndicator: Control %s set to %d\n", DefaultIndicators[Index].IndicatorDescription, TempParam->Params[1].intParam);

               /* The parameters appear to be is a semi-valid value.    */
               /* Now submit the command.                               */
               Result = HFRE_Update_Current_Control_Indicator_Status(BluetoothStackID, PortToUse, 1, &HFREIndicatorUpdate);

               /* Set the return value of this function equal to the    */
               /* Result of the function call.                          */
               ret_val = Result;

               if(!Result)
               {
                  /* The function was submitted successfully.           */
                  printf("HFRE_Update_Current_Control_Indicator_Status: Function Successful Port %d .\n", PortToUse);
               }
               else
               {
                  /* There was an error submitting the function.        */
                  printf("HFRE_Update_Current_Control_Indicator_Status() Port %d Failure: %d.\n", PortToUse, Result);
               }
            }
            else
               printf("UpdateControlIndicator: Invalid Parameter %d\n", TempParam->Params[1].intParam);

         }
         else
            ret_val = INVALID_PARAMETERS_ERROR;

         /* Check for an invalid parameter.                             */
         if(ret_val)
         {
            printf("\n");
            Index = 0;
            while(Index < NUMBER_DEFAULT_HFR_INDICATORS)
            {
               if(DefaultIndicators[Index].ControlIndicatorType == ciRange)
               {
                  RangeLow  = DefaultIndicators[Index].Control_Indicator_Data.ControlIndicatorRangeType.RangeStart;
                  RangeHigh = DefaultIndicators[Index].Control_Indicator_Data.ControlIndicatorRangeType.RangeEnd;
                  printf("%d. Range %d-%d %s\n", (Index+1), RangeLow, RangeHigh, DefaultIndicators[Index].IndicatorDescription);
               }
               else
                  printf("%d. Boolean   %s\n", (Index+1), DefaultIndicators[Index].IndicatorDescription);

               Index++;
            }
            printf("\n");

            /* One or more of the necessary parameters is/are invalid.  */
            printf("Usage: UpdateControlIndicators [Index] [Value].\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("Update Current Control Indicator Status: Invalid Port ID.\n");

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

   /* The following function is responsible for initializing a set of   */
   /* control indicators.  These structures are used in the instance an */
   /* audio gateway server is executing, but a connection to a hands    */
   /* free server is initiated.  These indicators are NOT the same as as*/
   /* the indicators for the executing gateway server, since opening the*/
   /* remote hands free port is essentially opening a second AG.  Note  */
   /* that these indicators go nowhere, unless the command to open a    */
   /* handsfree server is executed.                                     */
static void InitializeDefaultIndicators(void)
{
   int Index = 0;

   DefaultIndicators[Index].IndicatorDescription = HFRE_SERVICE_STRING;
   DefaultIndicators[Index].ControlIndicatorType = ciBoolean;
   DefaultIndicators[Index++].Control_Indicator_Data.ControlIndicatorBooleanType.CurrentIndicatorValue = FALSE;

   DefaultIndicators[Index].IndicatorDescription = HFRE_CALL_STRING;
   DefaultIndicators[Index].ControlIndicatorType = ciBoolean;
   DefaultIndicators[Index++].Control_Indicator_Data.ControlIndicatorBooleanType.CurrentIndicatorValue = FALSE;

   DefaultIndicators[Index].IndicatorDescription = HFRE_CALL_SETUP_STRING;
   DefaultIndicators[Index].ControlIndicatorType = ciRange;
   DefaultIndicators[Index].Control_Indicator_Data.ControlIndicatorRangeType.RangeStart              = 0;
   DefaultIndicators[Index].Control_Indicator_Data.ControlIndicatorRangeType.RangeEnd                = 3;
   DefaultIndicators[Index++].Control_Indicator_Data.ControlIndicatorRangeType.CurrentIndicatorValue = 0;

   DefaultIndicators[Index].IndicatorDescription = HFRE_CALLSETUP_STRING;
   DefaultIndicators[Index].ControlIndicatorType = ciRange;
   DefaultIndicators[Index].Control_Indicator_Data.ControlIndicatorRangeType.RangeStart              = 0;
   DefaultIndicators[Index].Control_Indicator_Data.ControlIndicatorRangeType.RangeEnd                = 3;
   DefaultIndicators[Index++].Control_Indicator_Data.ControlIndicatorRangeType.CurrentIndicatorValue = 0;

   DefaultIndicators[Index].IndicatorDescription = HFRE_CALLHELD_STRING;
   DefaultIndicators[Index].ControlIndicatorType = ciRange;
   DefaultIndicators[Index].Control_Indicator_Data.ControlIndicatorRangeType.RangeStart              = 0;
   DefaultIndicators[Index].Control_Indicator_Data.ControlIndicatorRangeType.RangeEnd                = 2;
   DefaultIndicators[Index++].Control_Indicator_Data.ControlIndicatorRangeType.CurrentIndicatorValue = 0;

   DefaultIndicators[Index].IndicatorDescription = HFRE_SIGNAL_STRING;
   DefaultIndicators[Index].ControlIndicatorType = ciRange;
   DefaultIndicators[Index].Control_Indicator_Data.ControlIndicatorRangeType.RangeStart              = 0;
   DefaultIndicators[Index].Control_Indicator_Data.ControlIndicatorRangeType.RangeEnd                = 5;
   DefaultIndicators[Index++].Control_Indicator_Data.ControlIndicatorRangeType.CurrentIndicatorValue = 0;

   DefaultIndicators[Index].IndicatorDescription = HFRE_ROAM_STRING;
   DefaultIndicators[Index].ControlIndicatorType = ciBoolean;
   DefaultIndicators[Index++].Control_Indicator_Data.ControlIndicatorBooleanType.CurrentIndicatorValue = FALSE;

   DefaultIndicators[Index].IndicatorDescription = HFRE_BATTCHG_STRING;
   DefaultIndicators[Index].ControlIndicatorType = ciRange;
   DefaultIndicators[Index].Control_Indicator_Data.ControlIndicatorRangeType.RangeStart              = 0;
   DefaultIndicators[Index].Control_Indicator_Data.ControlIndicatorRangeType.RangeEnd                = 5;
   DefaultIndicators[Index].Control_Indicator_Data.ControlIndicatorRangeType.CurrentIndicatorValue   = 0;
}

   /* The following function is responsible for updating a stored set of*/
   /* control indicators.  These indicators are used in the instance an */
   /* audio gateway server is executing, but a connection to a hands    */
   /* free server is initiated.  These indicators are NOT the same as as*/
   /* the indicators for the executing gateway server, since opening the*/
   /* remote hands free port is essentially opening a second AG.  Note  */
   /* that these indicators go nowhere, unless the command to open a    */
   /* handsfree server is executed.                                     */
static int SetDefaultClientCI(ParameterList_t *TempParam)
{
   int          ret_val;
   int          Index;
   unsigned int RangeLow;
   unsigned int RangeHigh;

   /* Make sure the passed in parameters appears to be semi-valid.      */
   if(TempParam)
   {
      if((TempParam->NumberofParameters >= 2) && ((TempParam->Params[0].intParam > 0) && (TempParam->Params[0].intParam <= NUMBER_DEFAULT_HFR_INDICATORS)))
      {
         /* Get the selected Index and make it Zero Relative.           */
         Index = TempParam->Params[0].intParam-1;

         if(DefaultIndicators[Index].ControlIndicatorType == ciRange)
         {
            /* Get the valid Range.                                     */
            RangeLow  = DefaultIndicators[Index].Control_Indicator_Data.ControlIndicatorRangeType.RangeStart;
            RangeHigh = DefaultIndicators[Index].Control_Indicator_Data.ControlIndicatorRangeType.RangeEnd;
         }
         else
         {
            RangeLow  = 0;
            RangeHigh = 1;
         }

         /* Check to see if the value is in the valid range.            */
         if((TempParam->Params[1].intParam >= (int)RangeLow) && (TempParam->Params[1].intParam <= (int)RangeHigh))
            ret_val = 0;
         else
            ret_val = INVALID_PARAMETERS_ERROR;

         if(!ret_val)
         {
            if(DefaultIndicators[Index].ControlIndicatorType == ciRange)
               DefaultIndicators[Index].Control_Indicator_Data.ControlIndicatorRangeType.CurrentIndicatorValue = TempParam->Params[1].intParam;
            else
               DefaultIndicators[Index].Control_Indicator_Data.ControlIndicatorBooleanType.CurrentIndicatorValue = (Boolean_t)(TempParam->Params[1].intParam != 0);

            printf("SetClientControlIndicator: Control %s set to %d\n", DefaultIndicators[Index].IndicatorDescription, TempParam->Params[1].intParam);
         }
         else
            printf("SetDefaultClientCI: Invalid Parameter %d.\n", TempParam->Params[1].intParam);
      }
      else
      {
         /* The user appeared to give an invalid indicator name         */
         printf("SetDefaultClientCI: Invalid Parameter %d.\n", TempParam->Params[0].intParam);

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
      ret_val = INVALID_PARAMETERS_ERROR;

   if(ret_val)
   {
      printf("\n");

      /* Cycle through the list and print the known indicators.         */
      Index = 0;
      while(Index < NUMBER_DEFAULT_HFR_INDICATORS)
      {
         if(DefaultIndicators[Index].ControlIndicatorType == ciRange)
         {
            RangeLow  = DefaultIndicators[Index].Control_Indicator_Data.ControlIndicatorRangeType.RangeStart;
            RangeHigh = DefaultIndicators[Index].Control_Indicator_Data.ControlIndicatorRangeType.RangeEnd;
            printf("%d. Range %d-%d %s\n", (Index+1), RangeLow, RangeHigh, DefaultIndicators[Index].IndicatorDescription);
         }
         else
            printf("%d. Boolean   %s\n", (Index+1), DefaultIndicators[Index].IndicatorDescription);

         Index++;
      }
      printf("\n");

      /* One or more of the necessary parameters is/are invalid.        */
      printf("Usage: SetDefaultClientCI  [Name] [Value].\n");

      ret_val = INVALID_PARAMETERS_ERROR;
   }
   return(ret_val);
}

   /* The following function is responsible for enabling or disabling   */
   /* remote indicator event notification on the remote audio gateway.  */
   /* This function returns zero on successful execution and a negative */
   /* value on all errors.                                              */
static int HFREEnableRemoteIndicatorEventNotification(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the Port ID appears to be          */
      /* semi-valid.                                                    */
      if(((ServerConnected)?CurrentServerPortID:CurrentClientPortID))
      {
         /* The port ID appears to be at least semi-valid, now check to */
         /* make sure the passed in parameters appears to be semi-valid.*/
         if((TempParam) && (TempParam->NumberofParameters > 0))
         {
            /* The Port ID appears to be is a semi-valid value.  Now    */
            /* submit the command.                                      */
            Result  = HFRE_Enable_Remote_Indicator_Event_Notification(BluetoothStackID, ((ServerConnected)?CurrentServerPortID:CurrentClientPortID), (Boolean_t)((TempParam->Params->intParam)?TRUE:FALSE));

            /* Set the return value of this function equal to the Result*/
            /* of the function call.                                    */
            ret_val = Result;

            if(!Result)
            {
               /* The function was submitted successfully.              */
               printf("HFRE_Set_Remote_Indicator_Event_Notification: Function Successful.\n");
            }
            else
            {
               /* There was an error submitting the function.           */
               printf("HFRE_Set_Remote_Indicator_Event_Notification() Failure: %d.\n", Result);
            }
         }
         else
         {
            /* One or more of the necessary parameters is/are invalid.  */
            printf("Usage: EnableRemoteIndicatorNotification [Disable = 0, Enable = 1].\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("Set Remote Indicator Event Notification: Invalid Port ID.\n");

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

   /* The following function is responsible for enabling or disabling   */
   /* Call Waiting Notification on the remote audio gateway.  This      */
   /* function returns zero on successful execution and a negative value*/
   /* on all errors.                                                    */
static int HFREEnableRemoteCallWaitingNotification(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the Port ID appears to be          */
      /* semi-valid.                                                    */
      if(((ServerConnected)?CurrentServerPortID:CurrentClientPortID))
      {
         /* The port ID appears to be at least semi-valid, now check to */
         /* make sure the passed in parameters appears to be semi-valid.*/
         if((TempParam) && (TempParam->NumberofParameters > 0))
         {
            /* The Port ID appears to be is a semi-valid value.  Now    */
            /* submit the command.                                      */
            Result  = HFRE_Enable_Remote_Call_Waiting_Notification(BluetoothStackID, ((ServerConnected)?CurrentServerPortID:CurrentClientPortID), (Boolean_t)((TempParam->Params->intParam)?TRUE:FALSE));

            /* Set the return value of this function equal to the Result*/
            /* of the function call.                                    */
            ret_val = Result;

            if(!Result)
            {
               /* The function was submitted successfully.              */
               printf("HFRE_Set_Remote_Call_Waiting_Notification: Function Successful.\n");
            }
            else
            {
               /* There was an error submitting the function.           */
               printf("HFRE_Set_Remote_Call_Waiting_Notification() Failure: %d.\n", Result);
            }
         }
         else
         {
            /* One or more of the necessary parameters is/are invalid.  */
            printf("Usage: EnableRemoteCallWaitingNotifcation [Disable = 0, Enable = 1].\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("Set Remote Call Waiting Notification: Invalid Port ID.\n");

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

   /* The following function is responsible for enabling or disabling   */
   /* Remote Call Line Identification Notification on the Remote Audio  */
   /* Gateway.  This function returns zero on successful execution and a*/
   /* negative value on all errors.                                     */
static int HFREEnableRemoteCallLineIdentificationNotification(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the Port ID appears to be          */
      /* semi-valid.                                                    */
      if(((ServerConnected)?CurrentServerPortID:CurrentClientPortID))
      {
         /* The port ID appears to be at least semi-valid, now check to */
         /* make sure the passed in parameters appears to be semi-valid.*/
         if((TempParam) && (TempParam->NumberofParameters > 0))
         {
            /* The Port ID appears to be is a semi-valid value.  Now    */
            /* submit the command.                                      */
            Result  = HFRE_Enable_Remote_Call_Line_Identification_Notification(BluetoothStackID, ((ServerConnected)?CurrentServerPortID:CurrentClientPortID), (Boolean_t)((TempParam->Params->intParam)?TRUE:FALSE));

            /* Set the return value of this function equal to the Result*/
            /* of the function call.                                    */
            ret_val = Result;

            if(!Result)
            {
               /* The function was submitted successfully.              */
               printf("HFRE_Set_Remote_Call_Line_Identification_Notification: Function Successful.\n");
            }
            else
            {
               /* There was an error submitting the function.           */
               printf("HFRE_Set_Remote_Call_Line_Identification_Notification() Failure: %d.\n", Result);
            }
         }
         else
         {
            /* One or more of the necessary parameters is/are invalid.  */
            printf("Usage: EnableRemoteCallerIDNotification [Disable = 0, Enable = 1].\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("Set Remote Call Line Identification Notification: Invalid Port ID.\n");

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

   /* The following function is responsible for disabling Sound         */
   /* Enhancement on the Remote Device.  This function returns zero on  */
   /* successful execution and a negative value on all errors.          */
static int HFREDisableRemoteEchoCancelationNoiseReduction(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the Port ID appears to be          */
      /* semi-valid.                                                    */
      if(((ServerConnected)?CurrentServerPortID:CurrentClientPortID))
      {
         /* The Port ID appears to be is a semi-valid value.  Now submit*/
         /* the command.                                                */
         Result  = HFRE_Disable_Remote_Echo_Cancelation_Noise_Reduction(BluetoothStackID, ((ServerConnected)?CurrentServerPortID:CurrentClientPortID));

         /* Set the return value of this function equal to the Result of*/
         /* the function call.                                          */
         ret_val = Result;

         if(!Result)
         {
            /* The function was submitted successfully.                 */
            printf("HFRE_Disable_Remote_Echo_Cancelation_Noise_Reduction: Function Successful.\n");
         }
         else
         {
            /* There was an error submitting the function.              */
            printf("HFRE_Disable_Remote_Echo_Cancelation_Noise_Reduction() Failure: %d.\n", Result);
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("Disable Remote Echo Cancelation Noise Reduction: Invalid Port ID.\n");

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

   /* The following function is responsible for Querying the Current    */
   /* Indicator Status of the Remote Audio Gateway.  This function      */
   /* returns zero on successful execution and a negative value on all  */
   /* errors.                                                           */
static int HFREQueryRemoteIndicatorsStatus(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the Port ID appears to be          */
      /* semi-valid.                                                    */
      if(((ServerConnected)?CurrentServerPortID:CurrentClientPortID))
      {
         /* The Port ID appears to be a semi-valid value.  Now submit   */
         /* the command.                                                */
         Result  = HFRE_Query_Remote_Control_Indicator_Status(BluetoothStackID, ((ServerConnected)?CurrentServerPortID:CurrentClientPortID));

         /* Set the return value of this function equal to the Result of*/
         /* the function call.                                          */
         ret_val = Result;

         /* Now check to see if the command was submitted successfully. */
         if(!Result)
         {
            /* The function was submitted successfully.                 */
            printf("HFRE_Query_Remote_Control_Indicator_Status: Function Successful.\n");
         }
         else
         {
            /* There was an error submitting the function.              */
            printf("HFRE_Query_Remote_Control_Indicator_Status() Failure: %d.\n", Result);
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("Query Remote Indicators Status: Invalid Port ID.\n");
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

   /* The following function is responsible for Querying the Call       */
   /* Holding and Mutliparty Services Supported by the Remote Audio     */
   /* Gateway.  This function returns zero on successful execution and a*/
   /* negative value on all errors.                                     */
static int HFREQueryRemoteCallHoldingMultipartyServiceSupport(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the Port ID appears to be          */
      /* semi-valid.                                                    */
      if(((ServerConnected)?CurrentServerPortID:CurrentClientPortID))
      {
         /* The Port ID appears to be a semi-valid value.  Now submit   */
         /* the command.                                                */
         Result  = HFRE_Query_Remote_Call_Holding_Multiparty_Service_Support(BluetoothStackID, ((ServerConnected)?CurrentServerPortID:CurrentClientPortID));

         /* Set the return value of this function equal to the Result of*/
         /* the function call.                                          */
         ret_val = Result;

         /* Now check to see if the command was submitted successfully. */
         if(!Result)
         {
            /* The function was submitted successfully.                 */
            printf("HFRE_Query_Remote_Call_Holding_Multiparty_Service_Support: Function Successful.\n");
         }
         else
         {
            /* There was an error submitting the function.              */
            printf("HFRE_Query_Remote_Call_Holding_Multiparty_Service_Support() Failure: %d.\n", Result);
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("Query Remote Call Holding Multiparty Service Support: Invalid Port ID.\n");

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

   /* The following function is responsible for sending a Call Holding  */
   /* and Mutliparty Service Selection to the Remote Audio Gateway.     */
   /* This function returns zero on successful execution and a negative */
   /* value on all errors.                                              */
static int HFRESendCallHoldingMultipartySelection(ParameterList_t *TempParam)
{
   int                                       Result;
   int                                       ret_val;
   int                                       Index;
   HFRE_Call_Hold_Multiparty_Handling_Type_t Selection;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the Port ID appears to be          */
      /* semi-valid.                                                    */
      if(((ServerConnected)?CurrentServerPortID:CurrentClientPortID))
      {
         /* The port ID appears to be at least semi-valid, now check to */
         /* make sure the passed in parameters appears to be semi-valid.*/
         if((TempParam) && (TempParam->NumberofParameters > 0))
         {
            /* If no index parameter was on the command line, make sure */
            /* we have a zero value for Index, and use the first        */
            /* parameter as HFRE_Call_Hold_Multiparty_Handling_Type_t   */
            /* selection value.                                         */
            if(TempParam->NumberofParameters < 2)
            {
               Index     = 0;

               Selection = TempParam->Params[0].intParam;
            }
            else
            {
               /* If we have an index parameter, we have to advise the  */
               /* profile that this is either a release specified call  */
               /* or a private consultation with the specified call     */
               /* Our enum values allow us to use the first parameter   */
               /* as a selection with an offset value.                  */
               Index     = TempParam->Params[1].intParam;

               Selection = TempParam->Params[0].intParam + chConnectTwoCallsAndDisconnect;
            }

            /* Now submit the command.                                  */
            Result  = HFRE_Send_Call_Holding_Multiparty_Selection(BluetoothStackID, ((ServerConnected)?CurrentServerPortID:CurrentClientPortID), Selection, Index);

            /* Set the return value of this function equal to the Result*/
            /* of the function call.                                    */
            ret_val = Result;

            if(!Result)
            {
               /* The function was submitted successfully.              */
               printf("HFRE_Send_Call_Holding_Multiparty_Selection: Function Successful.\n");
            }
            else
            {
               /* There was an error submitting the function.           */
               printf("HFRE_Send_Call_Holding_Multiparty_Selection() Failure: %d.\n", Result);
            }
         }
         else
         {
            /* One or more of the necessary parameters is/are invalid.  */
            printf("Usage: SendCallHoldSelection [Call Hold Selection] [Optional Index].\n");
            printf("       Call Hold Selection 0=Release All Held Calls.\n");
            printf("       Call Hold Selection 1=Release All Active Calls, Accept Waiting Call.\n");
            printf("       Call Hold Selection 2=Place All Calls on Hold, Accept Waiting Call.\n");
            printf("       Call Hold Selection 3=Add Held Call to Conversation.\n");
            printf("       Call Hold Selection 4=Connect Two Calls and Disconnect.\n");
            printf("       Call Hold Selection 5=Release Specified Call, Accept Waiting Call.\n");
            printf("       Call Hold Selection 6=Private Consultation with Specified Call.\n");
            printf("       Optional Index = Specified call for CHLD Selection values 5 or 6\r          (defaults to zero).\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("Send Call Holding Multiparty Selection: Invalid Port ID.\n");

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

   /* The following function is responsible for sending a Call Waiting  */
   /* Notification to the Remote Hands-Free unit.  This function returns*/
   /* zero on successful execution and a negative value on all errors.  */
static int HFRESendCallWaitingNotification(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the Port ID appears to be          */
      /* semi-valid.                                                    */
      if(((ServerConnected)?CurrentServerPortID:CurrentClientPortID))
      {
         /* The Port ID appears to be at least semi-valid, now check the*/
         /* required parameters for this command.                       */
         if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params->strParam) && (strlen(TempParam->Params->strParam)))
         {
            /* The parameters appear to be is a semi-valid value.  Now  */
            /* submit the command.                                      */
            Result  = HFRE_Send_Call_Waiting_Notification(BluetoothStackID, ((ServerConnected)?CurrentServerPortID:CurrentClientPortID), TempParam->Params->strParam);

            /* Set the return value of this function equal to the Result*/
            /* of the function call.                                    */
            ret_val = Result;

            if(!Result)
            {
               /* The function was submitted successfully.              */
               printf("HFRE_Send_Call_Waiting_Notification: Function Successful.\n");
            }
            else
            {
               /* There was an error submitting the function.           */
               printf("HFRE_Send_Call_Waiting_Notification() Failure: %d.\n", Result);
            }
         }
         else
         {
            /* The Phone Number parameter is invalid.                   */
            printf("Usage: SendCallWaitingNotification [Phone Number].\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("Send Call Waiting Notification: Invalid Port ID.\n");

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

   /* The following function is responsible for sending a Call Line     */
   /* Identification Notification to the Remote Hands-Free unit.  This  */
   /* function returns zero on successful execution and a negative value*/
   /* on all errors.                                                    */
static int HFRESendCallLineIdentificationNotification(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the Port ID appears to be          */
      /* semi-valid.                                                    */
      if(((ServerConnected)?CurrentServerPortID:CurrentClientPortID))
      {
         /* The Port ID appears to be at least semi-valid, now check the*/
         /* required parameters for this command.                       */
         if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params->strParam) && (strlen(TempParam->Params->strParam)))
         {
            /* The parameters appear to be is a semi-valid value.  Now  */
            /* submit the command.                                      */
            Result  = HFRE_Send_Call_Line_Identification_Notification(BluetoothStackID, ((ServerConnected)?CurrentServerPortID:CurrentClientPortID), TempParam->Params->strParam);

            /* Set the return value of this function equal to the Result*/
            /* of the function call.                                    */
            ret_val = Result;

            if(!Result)
            {
               /* The function was submitted successfully.              */
               printf("HFRE_Send_Call_Line_Identification_Notification: Function Successful.\n");
            }
            else
            {
               /* There was an error submitting the function.           */
               printf("HFRE_Send_Call_Line_Identification_Notification() Failure: %d.\n", Result);
            }
         }
         else
         {
            /* The Phone Number parameter is invalid.                   */
            printf("Usage: SendCallerIDNotification [Phone Number].\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("Send Call Line Identification Notification: Invalid Port ID.\n");

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

   /* The following function is responsible for sending the command to  */
   /* dial the specified phone number on the Remote Audio Gateway.  This*/
   /* function returns zero on successful execution and a negative value*/
   /* on all errors.                                                    */
static int HFREDialPhoneNumber(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the Port ID appears to be          */
      /* semi-valid.                                                    */
      if(((ServerConnected)?CurrentServerPortID:CurrentClientPortID))
      {
         /* The Port ID appears to be at least semi-valid, now check the*/
         /* required parameters for this command.                       */
         if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params->strParam) && (strlen(TempParam->Params->strParam)))
         {
            /* The parameters appear to be is a semi-valid value.  Now  */
            /* submit the command.                                      */
            Result  = HFRE_Dial_Phone_Number(BluetoothStackID, ((ServerConnected)?CurrentServerPortID:CurrentClientPortID), TempParam->Params->strParam);

            /* Set the return value of this function equal to the Result*/
            /* of the function call.                                    */
            ret_val = Result;

            if(!Result)
            {
               /* The function was submitted successfully.              */
               printf("HFRE_Dial_Phone_Number: Function Successful.\n");
            }
            else
            {
               /* There was an error submitting the function.           */
               printf("HFRE_Dial_Phone_Number() Failure: %d.\n", Result);
            }
         }
         else
         {
            /* The Phone Number parameter is invalid.                   */
            printf("Usage: DialNumber [Phone Number].\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("Dial Phone Number: Invalid Port ID.\n");

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

   /* The following function is responsible for sending the command to  */
   /* dial a phone number from memory on the Remote Audio Gateway.  This*/
   /* function returns zero on successful execution and a negative value*/
   /* on all errors.                                                    */
static int HFREDialPhoneNumberFromMemory(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the Port ID appears to be          */
      /* semi-valid.                                                    */
      if(((ServerConnected)?CurrentServerPortID:CurrentClientPortID))
      {
         /* The Port ID appears to be at least semi-valid, now check the*/
         /* required parameters for this command.                       */
         if((TempParam) && (TempParam->NumberofParameters > 0))
         {
            /* The parameters appear to be is a semi-valid value.  Now  */
            /* submit the command.                                      */
            Result  = HFRE_Dial_Phone_Number_From_Memory(BluetoothStackID, ((ServerConnected)?CurrentServerPortID:CurrentClientPortID), TempParam->Params->intParam);

            /* Set the return value of this function equal to the Result*/
            /* of the function call.                                    */
            ret_val = Result;

            if(!Result)
            {
               /* The function was submitted successfully.              */
               printf("HFRE_Dial_Phone_Number_From_Memory: Function Successful.\n");
            }
            else
            {
               /* There was an error submitting the function.           */
               printf("HFRE_Dial_Phone_Number_From_Memory() Failure: %d.\n", Result);
            }
         }
         else
         {
            /* The Memory Location parameter is invalid.                */
            printf("Usage: MemoryDial [MemoryLocation].\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("Dial Phone Number From Memory: Invalid Port ID.\n");

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

   /* The following function is responsible for sending the command to  */
   /* dial the last number dialed on the Remote Audio Gateway.  This    */
   /* function returns zero on successful execution and a negative value*/
   /* on all errors.                                                    */
static int HFRERedialLastPhoneNumber(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the Port ID appears to be          */
      /* semi-valid.                                                    */
      if(((ServerConnected)?CurrentServerPortID:CurrentClientPortID))
      {
         /* The Port ID appears to be a semi-valid value.  Now submit   */
         /* the command.                                                */
         Result  = HFRE_Redial_Last_Phone_Number(BluetoothStackID, ((ServerConnected)?CurrentServerPortID:CurrentClientPortID));

         /* Set the return value of this function equal to the Result of*/
         /* the function call.                                          */
         ret_val = Result;

         /* Now check to see if the command was submitted successfully. */
         if(!Result)
         {
            /* The function was submitted successfully.                 */
            printf("HFRE_Redial_Last_Phone_Number: Function Successful.\n");
         }
         else
         {
            /* There was an error submitting the function.              */
            printf("HFRE_Redial_Last_Phone_Number() Failure: %d.\n", Result);
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("Redial Last Phone Number: Invalid Port ID.\n");

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

   /* The following function is responsible for sending an enable or    */
   /* disable In-Band Ring Indication to the Remote Hands-Free unit.    */
   /* This function returns zero on successful execution and a negative */
   /* value on all errors.                                              */
static int HFRESetRingIndication(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the Port ID appears to be          */
      /* semi-valid.                                                    */
      if(((ServerConnected)?CurrentServerPortID:CurrentClientPortID))
      {
         /* The Port ID appears to be a semi-valid value.  Now submit   */
         /* the command.                                                */

         /* if no params are submitted, default the parameter to true   */
         if(!TempParam->NumberofParameters)
            TempParam->Params[0].intParam = 1;

         Result  = HFRE_Enable_Remote_InBand_Ring_Tone_Setting(BluetoothStackID, ((ServerConnected)?CurrentServerPortID:CurrentClientPortID), (Boolean_t)(TempParam->Params[0].intParam?TRUE:FALSE));

         /* Set the return value of this function equal to the Result of*/
         /* the function call.                                          */
         ret_val = Result;

         /* Now check to see if the command was submitted successfully. */
         if(!Result)
         {
            /* The function was submitted successfully.                 */
            printf("HFRE_Ring_Indication: Function Successful.\n");
         }
         else
         {
            /* There was an error submitting the function.              */
            printf("HFRE_Ring_Indication() Failure: %d.\n", Result);
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("Ring Indication: Invalid Port ID.\n");

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

   /* The following function is responsible for sending a Ring          */
   /* Indication to the Remote Hands-Free unit.  This function returns  */
   /* zero on successful execution and a negative value on all errors.  */
static int HFRERingIndication(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the Port ID appears to be          */
      /* semi-valid.                                                    */
      if(((ServerConnected)?CurrentServerPortID:CurrentClientPortID))
      {
         /* The Port ID appears to be a semi-valid value.  Now submit   */
         /* the command.                                                */
         Result  = HFRE_Ring_Indication(BluetoothStackID, ((ServerConnected)?CurrentServerPortID:CurrentClientPortID));

         /* Set the return value of this function equal to the Result of*/
         /* the function call.                                          */
         ret_val = Result;

         /* Now check to see if the command was submitted successfully. */
         if(!Result)
         {
            /* The function was submitted successfully.                 */
            printf("HFRE_Ring_Indication: Function Successful.\n");
         }
         else
         {
            /* There was an error submitting the function.              */
            printf("HFRE_Ring_Indication() Failure: %d.\n", Result);
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("Ring Indication: Invalid Port ID.\n");

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

   /* The following function is responsible for sending the command to  */
   /* Answer an Incoming Call on the Remote Audio Gateway.  This        */
   /* function returns zero on successful execution and a negative value*/
   /* on all errors.                                                    */
static int HFREAnswerIncomingCall(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the Port ID appears to be          */
      /* semi-valid.                                                    */
      if(((ServerConnected)?CurrentServerPortID:CurrentClientPortID))
      {
         /* The Port ID appears to be a semi-valid value.  Now submit   */
         /* the command.                                                */
         Result  = HFRE_Answer_Incoming_Call(BluetoothStackID, ((ServerConnected)?CurrentServerPortID:CurrentClientPortID));

         /* Set the return value of this function equal to the Result of*/
         /* the function call.                                          */
         ret_val = Result;

         /* Now check to see if the command was submitted successfully. */
         if(!Result)
         {
            /* The function was submitted successfully.                 */
            printf("HFRE_Answer_Incoming_Call: Function Successful.\n");
         }
         else
         {
            /* There was an error submitting the function.              */
            printf("HFRE_Answer_Incoming_Call() Failure: %d.\n", Result);
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("Answer Incoming Call: Invalid Port ID.\n");

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

   /* The following function is responsible for sending a DTMF Code to  */
   /* the Remote Audio Gateway.  This function returns zero on          */
   /* successful execution and a negative value on all errors.          */
static int HFRETransmitDTMFCode(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the Port ID appears to be          */
      /* semi-valid.                                                    */
      if(((ServerConnected)?CurrentServerPortID:CurrentClientPortID))
      {
         /* The Port ID appears to be semi-valid, now check the DTMF    */
         /* Code.                                                       */
         if((TempParam) && (TempParam->NumberofParameters>0) && (TempParam->Params->strParam) && (strlen(TempParam->Params->strParam)))
         {
            /* The DTMF Code is a semi-valid value.  Now submit the     */
            /* command.                                                 */
            Result  = HFRE_Transmit_DTMF_Code(BluetoothStackID, ((ServerConnected)?CurrentServerPortID:CurrentClientPortID), TempParam->Params->strParam[0]);

            /* Set the return value of this function equal to the Result*/
            /* of the function call.                                    */
            ret_val = Result;

            if(!Result)
            {
               /* The function was submitted successfully.              */
               printf("HFRE_Transmit_DTMF_Code(): Function Successful.\n");
            }
            else
            {
               /* There was an error submitting the function.           */
               printf("HFRE_Transmit_DTMF_Code() Failure: %d.\n", Result);
            }
         }
         else
         {
            /* The DTMF Code parameter is invalid.                      */
            printf("Usage: SendDTMF [DTMF Code].\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* The Port ID parameter is invalid.                           */
         printf("Transmit DTMF Code: Invalid Port ID.\n");

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

   /* The following function is responsible for deactivating Voice      */
   /* Recognition Activation on the Audio Gateway and for change the    */
   /* Voice Recognition Activation state on the Hands Free Unit.  This  */
   /* function returns zero on successful execution and a negative value*/
   /* on all errors.                                                    */
static int HFRESetRemoteVoiceRecognitionActivation(ParameterList_t *TempParam)
{
   int ret_val;
   int Result;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the Port ID appears to be          */
      /* semi-valid.                                                    */
      if(((ServerConnected)?CurrentServerPortID:CurrentClientPortID))
      {
         /* The Port ID appears to be at least semi-valid, now check the*/
         /* required parameters for this command.                       */
         if((TempParam) && (TempParam->NumberofParameters > 0))
         {
            /* The Port ID appears to be is a semi-valid value.  Now    */
            /* submit the command.                                      */
            Result  = HFRE_Set_Remote_Voice_Recognition_Activation(BluetoothStackID, ((ServerConnected)?CurrentServerPortID:CurrentClientPortID), (Boolean_t)TempParam->Params->intParam);

            /* Set the return value of this function equal to the Result*/
            /* of the function call.                                    */
            ret_val = Result;

            if(!Result)
            {
               /* The function was submitted successfully.              */
               printf("HFRE_Set_Remote_Voice_Recognition_Activation: Function Successful.\n");
            }
            else
            {
               /* There was an error submitting the function.           */
               printf("HFRE_Set_Remote_Voice_Recognition_Activation() Failure: %d.\n", Result);
            }
         }
         else
         {
            /* The required parameter is invalid.                       */
            printf("Usage: SetVoiceRecognitionActivation [Disable = 0, Enable = 1].\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("Set Remote Voice Recognition Activation: Invalid Port ID.\n");

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

   /* The following function is responsible for setting the Speaker Gain*/
   /* on Remote Device.  This function returns zero on successful       */
   /* execution and a negative value on all errors.                     */
static int HFRESetRemoteSpeakerGain(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the Port ID appears to be          */
      /* semi-valid.                                                    */
      if(((ServerConnected)?CurrentServerPortID:CurrentClientPortID))
      {
         /* The Port ID appears to be semi-valid, now check the Speaker */
         /* Gain.                                                       */
         if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params->intParam <= HFRE_SPEAKER_GAIN_MAXIMUM))
         {
            /* The Speaker Gain is a valid value.  Now submit the       */
            /* command.                                                 */
            Result  = HFRE_Set_Remote_Speaker_Gain(BluetoothStackID, ((ServerConnected)?CurrentServerPortID:CurrentClientPortID), TempParam->Params->intParam);

            /* Set the return value of this function equal to the Result*/
            /* of the function call.                                    */
            ret_val = Result;

            if(!Result)
            {
               /* The function was submitted successfully.              */
               printf("HFRE_Set_Remote_Speaker_Gain: Function Successful.\n");
            }
            else
            {
               /* There was an error submitting the function.           */
               printf("HFRE_Set_Remote_Speaker_Gain() Failure: %d.\n", Result);
            }
         }
         else
         {
            /* The Speaker Gain parameter is invalid.                   */
            printf("Usage: SetSpeakerGain [0 <= SpeakerGain <= 15].\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* The Port ID parameter is invalid.                           */
         printf("Set Remote Speaker Gain: Invalid Port ID.\n");

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

   /* The following function is responsible for setting the Microphone  */
   /* Gain on Remote Device.  This function returns zero on successful  */
   /* execution and a negative value on all errors.                     */
static int HFRESetRemoteMicrophoneGain(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the Port ID appears to be          */
      /* semi-valid.                                                    */
      if(((ServerConnected)?CurrentServerPortID:CurrentClientPortID))
      {
         /* The Port ID appears to be semi-valid, now check the         */
         /* Microphone Gain.                                            */
         if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params->intParam <= HFRE_MICROPHONE_GAIN_MAXIMUM))
         {
            /* The Microphone Gain is a valid value.  Now submit the    */
            /* command.                                                 */
            Result  = HFRE_Set_Remote_Microphone_Gain(BluetoothStackID, ((ServerConnected)?CurrentServerPortID:CurrentClientPortID), TempParam->Params->intParam);

            /* Set the return value of this function equal to the Result*/
            /* of the function call.                                    */
            ret_val = Result;

            if(!Result)
            {
               /* The function was submitted successfully.              */
               printf("HFRE_Set_Remote_Microphone_Gain(): Function Successful.\n");
            }
            else
            {
               /* There was an error submitting the function.           */
               printf("HFRE_Set_Remote_Microphone_Gain() Failure: %d.\n", Result);
            }
         }
         else
         {
            /* The Microphone Gain parameter is invalid.                */
            printf("Usage: SetMicrophoneGain [0 <= MicrophoneGain <= 15].\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* The Port ID parameter is invalid.                           */
         printf("Set Remote Microphone Gain: Invalid Port ID.\n");

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

   /* The following function is responsible for sending a Voice Tag     */
   /* Request to the Remote Audio Gateway.  This function returns zero  */
   /* on successful execution and a negative value on all errors.       */
static int HFREVoiceTagRequest(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the Port ID appears to be          */
      /* semi-valid.                                                    */
      if(((ServerConnected)?CurrentServerPortID:CurrentClientPortID))
      {
         /* The Port ID appears to be a semi-valid value.  Now submit   */
         /* the command.                                                */
         Result  = HFRE_Voice_Tag_Request(BluetoothStackID, ((ServerConnected)?CurrentServerPortID:CurrentClientPortID));

         /* Set the return value of this function equal to the Result of*/
         /* the function call.                                          */
         ret_val = Result;

         /* Now check to see if the command was submitted successfully. */
         if(!Result)
         {
            /* The function was submitted successfully.                 */
            printf("HFRE_Voice_Tag_Request: Function Successful.\n");
         }
         else
         {
            /* There was an error submitting the function.              */
            printf("HFRE_Voice_Tag_Request() Failure: %d.\n", Result);
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("Voice Tag Request: Invalid Port ID.\n");

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

   /* The following function is responsible for sending a Voice Tag     */
   /* Response to the Remote Hands-Free unit.  This function returns    */
   /* zero on successful execution and a negative value on all errors.  */
static int HFREVoiceTagResponse(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the Port ID appears to be          */
      /* semi-valid.                                                    */
      if(((ServerConnected)?CurrentServerPortID:CurrentClientPortID))
      {
         /* The Port ID appears to be at least semi-valid, now check the*/
         /* required parameters for this command.                       */
         if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params->strParam) && (strlen(TempParam->Params->strParam)))
         {
            /* The parameters appear to be is a semi-valid value.  Now  */
            /* submit the command.                                      */
            Result  = HFRE_Voice_Tag_Response(BluetoothStackID, ((ServerConnected)?CurrentServerPortID:CurrentClientPortID), TempParam->Params->strParam);

            /* Set the return value of this function equal to the Result*/
            /* of the function call.                                    */
            ret_val = Result;

            if(!Result)
            {
               /* The function was submitted successfully.              */
               printf("HFRE_Voice_Tag_Response: Function Successful.\n");
            }
            else
            {
               /* There was an error submitting the function.           */
               printf("HFRE_Voice_Tag_Response() Failure: %d.\n", Result);
            }
         }
         else
         {
            /* The Phone Number parameter is invalid.                   */
            printf("Usage: SendVoiceTagResponse [Phone Number].\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("Voice Tag Response: Invalid Port ID.\n");

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

   /* The following function is responsible for sending the command to  */
   /* HangUp ongoing calls or reject incoming calls on the Remote Audio */
   /* Gateway.  This function returns zero on successful execution and a*/
   /* negative value on all errors.                                     */
static int HFREHangUpCall(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the Port ID appears to be          */
      /* semi-valid.                                                    */
      if(((ServerConnected)?CurrentServerPortID:CurrentClientPortID))
      {
         /* The Port ID appears to be a semi-valid value.  Now submit   */
         /* the command.                                                */
         Result  = HFRE_Hang_Up_Call(BluetoothStackID, ((ServerConnected)?CurrentServerPortID:CurrentClientPortID));

         /* Set the return value of this function equal to the Result of*/
         /* the function call.                                          */
         ret_val = Result;

         /* Now check to see if the command was submitted successfully. */
         if(!Result)
         {
            /* The function was submitted successfully.                 */
            printf("HFRE_Hang_Up_Call: Function Successful.\n");
         }
         else
         {
            /* There was an error submitting the function.              */
            printf("HFRE_Hang_Up_Call() Failure: %d.\n", Result);
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("Hang Up Call: Invalid Port ID.\n");

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

   /* The following function is responsible for sending the command to  */
   /* Query the current Operator selection on the Remote Audio Gateway  */
   /* Device.  This function returns zero on successful execution and a */
   /* negative value on all errors.                                     */
static int HFREQueryOperator(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the Port ID appears to be          */
      /* semi-valid.                                                    */
      if((ServerConnected)?CurrentServerPortID:CurrentClientPortID)
      {
         /* The Port ID appears to be is a semi-valid value.  Now submit*/
         /* the command.                                                */
         Result = HFRE_Query_Remote_Network_Operator_Selection(BluetoothStackID, ((ServerConnected)?CurrentServerPortID:CurrentClientPortID));

         printf("HFRE_Query_Remote_Network_Operator_Selection: Function %s, Returned Result %d\n", Result?"Failed ":"Succeeded", Result);

         /* Set the return value of this function equal to the Result of*/
         /* the function call.                                          */
         ret_val = Result;
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("HFRE_Query_Remote_Network_Operator_Selection: Invalid Port ID.\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      printf("Stack ID Invalid.\n");

      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for sending the command to  */
   /* Set the Operator Format on the Remote Audio Gateway Device.  This */
   /* function returns zero on successful execution and a negative value*/
   /* on all errors.                                                    */
static int HFRESetOperatorFormat(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the Port ID appears to be          */
      /* semi-valid.                                                    */
      if((ServerConnected)?CurrentServerPortID:CurrentClientPortID)
      {
         /* The Port ID appears to be is a semi-valid value.  Now submit*/
         /* the command.                                                */
         Result = HFRE_Set_Network_Operator_Selection_Format(BluetoothStackID, ((ServerConnected)?CurrentServerPortID:CurrentClientPortID));

         printf("HFRE_Set_Network_Operator_Selection_Format: Function %s, Returned Result %d\n", Result?"Failed ":"Succeeded", Result);

         /* Set the return value of this function equal to the Result of*/
         /* the function call.                                          */
         ret_val = Result;
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("HFRE_Set_Network_Operator_Selection_Format: Invalid Port ID.\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      printf("Stack ID Invalid.\n");

      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for sending the command to  */
   /* Enable/Disable a specific indicator (+BIA) on the Remote Audio    */
   /* Gateway Device.  This function returns zero on successful         */
   /* execution and a negative value on all errors.                     */
static int HFRESetIndicatorUpdateState(ParameterList_t *TempParam)
{
   int                         ret_val;
   unsigned int                Count;
   Notification_t             *NotificationEntry;
   HFRE_Notification_Update_t  UpdateItem;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the Port ID appears to be          */
      /* semi-valid.                                                    */
      if((ServerConnected)?CurrentServerPortID:CurrentClientPortID)
      {
         /* Get the count of the notifications recorded.                */
         Count = (unsigned int)GetNotificationListCount();

         /* The port ID appears to be at least semi-valid, now check to */
         /* make sure the passed in parameters appears to be semi-valid.*/
         if((TempParam) && (TempParam->NumberofParameters >= 2 ) && ((TempParam->Params[0].intParam > 0) && (TempParam->Params[0].intParam <= (int)Count)) && ((TempParam->Params[1].intParam == 0) || (TempParam->Params[1].intParam == 1)))
         {
            /* Locate the indicator specified.                          */
            Count             = TempParam->Params[0].intParam-1;
            NotificationEntry = NotificationList;
            while(Count)
            {
               NotificationEntry = NotificationEntry->NextNotification;
               Count--;
            }

            if(NotificationEntry)
            {
               /* Set the State of the Notification.                    */
               NotificationEntry->Enabled      = (Boolean_t)(TempParam->Params[1].intParam == 1);
               UpdateItem.IndicatorDescription = NotificationEntry->Indicator;
               UpdateItem.NotificationEnabled  = NotificationEntry->Enabled;

               /* Looks like we have a valid enable/disable flag also.  */
               /* Now submit the command.                               */
               ret_val = HFRE_Update_Remote_Indicator_Notification_State(BluetoothStackID, ((ServerConnected)?CurrentServerPortID:CurrentClientPortID), 1, &UpdateItem);
            }
            else
               ret_val = -1;

            printf("HFRESetIndicatorUpdateState: Function %s, Returned Result %d\n", ret_val?"Failed ":"Succeeded", ret_val);
         }
         else
         {
            /* Verify that we have entries in the list.                 */
            if(NotificationList)
            {
               /* Cycle through the list and print the known indicators.*/
               ret_val = 0;
               NotificationEntry = NotificationList;
               while(NotificationEntry)
               {
                  printf("%d. %s %s\n", (ret_val+1), (char *)((NotificationEntry->Enabled)?"Enabled ":"Disabled"), NotificationEntry->Indicator);
                  NotificationEntry = NotificationEntry->NextNotification;
                  ret_val++;
               }
               printf("\n");
            }
            else
               printf("No know notification recorded\n\n");

            printf("Usage: HFRESetIndicatorUpdateState [Index] [0(Disable), 1(Enable)]\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         printf("HFRE_Enable_Remote_Extended_Error_Result: Invalid Port ID.\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      printf("Stack ID Invalid.\n");

      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for sending the command to  */
   /* Enable extended Error Reporting (+CMEE) on the Remote Audio       */
   /* Gateway Device.  This function returns zero on successful         */
   /* execution and a negative value on all errors.                     */
static int HFREEnableExtendedErrorReporting(ParameterList_t *TempParam)
{
   int       ret_val;
   Boolean_t EnableExtendedErrorResults;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the Port ID appears to be          */
      /* semi-valid.                                                    */
      if((ServerConnected)?CurrentServerPortID:CurrentClientPortID)
      {
         /* The port ID appears to be at least semi-valid, now check to */
         /* make sure the passed in parameters appears to be semi-valid.*/
         if((TempParam) && (TempParam->NumberofParameters > 0))
         {
            /* Determine the selection type.                            */
            switch(TempParam->Params[0].intParam)
            {
               case 0:
                  EnableExtendedErrorResults = FALSE;
                  break;
               default:
                  EnableExtendedErrorResults = TRUE;
                  break;
            }

            /* Looks like we have a valid enable/disable flag also.     */
            /* Now submit the command.                                  */
            ret_val = HFRE_Enable_Remote_Extended_Error_Result(BluetoothStackID, ((ServerConnected)?CurrentServerPortID:CurrentClientPortID), EnableExtendedErrorResults);

            printf("HFRE_Enable_Remote_Extended_Error_Result: Function %s, Returned Result %d\n", ret_val?"Failed ":"Succeeded", ret_val);
         }
         else
         {
            printf("Usage: EnableErrorReports [FALSE = 0 | TRUE = 1]\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         printf("HFRE_Enable_Remote_Extended_Error_Result: Invalid Port ID.\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      printf("Stack ID Invalid.\n");

      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for sending the command to  */
   /* Request the Subscriber Number(s) from the Remote Audio Gateway    */
   /* Device.  This function returns zero on successful execution and a */
   /* negative value on all errors.                                     */
static int HFREQuerySubscriberNumber(ParameterList_t *TempParam)
{
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the Port ID appears to be          */
      /* semi-valid.                                                    */
      if(((ServerConnected)?CurrentServerPortID:CurrentClientPortID))
      {
         /* The Port ID appears to be is a semi-valid value.  Now submit*/
         /* the command.                                                */
         ret_val = HFRE_Query_Subscriber_Number_Information(BluetoothStackID, ((ServerConnected)?CurrentServerPortID:CurrentClientPortID));

         printf("HFRE_Query_Subscriber_Number_Information: Function %s, Returned Result %d\n", ret_val?"Failed ":"Succeeded", ret_val);
      }
      else
      {
         printf("HFRE_Query_Subscriber_Number_Information: Invalid Port ID.\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      printf("Stack ID Invalid.\n");

      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for sending the command to  */
   /* query the Response and Hold Status from the Remote Audio Gateway  */
   /* Device.  This function returns zero on successful execution and a */
   /* negative value on all errors.                                     */
static int HFREQueryResponseAndHold(ParameterList_t *TempParam)
{
   int Result;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the Port ID appears to be          */
      /* semi-valid.                                                    */
      if(((ServerConnected)?CurrentServerPortID:CurrentClientPortID))
      {
         /* The Port ID appears to be is a semi-valid value.  Now submit*/
         /* the command.                                                */
         Result = HFRE_Query_Response_Hold_Status(BluetoothStackID, ((ServerConnected)?CurrentServerPortID:CurrentClientPortID));

         printf("HFRE_Query_Subscriber_Number_Information: Function %s, Returned Result %d\n", Result?"Failed ":"Succeeded", Result);
      }
      else
      {
         printf("HFRE_Query_Response_Hold_Status: Invalid Port ID.\n");

         Result = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      printf("Stack ID Invalid.\n");

      Result = INVALID_STACK_ID_ERROR;
   }

   return(Result);
}

   /* The following function is responsible for sending the Response and*/
   /* Hold Command (+BTRH) to the remote device.  This function returns */
   /* zero on successful execution and a negative value on all errors.  */
static int HFRESendResponseAndHold(ParameterList_t *TempParam)
{
   int  Result;
   int  CallState;
   char temp[32];

   strcpy(temp, "Not Called");

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* make sure the passed in parameters appears to be semi-valid.   */
      if((TempParam) && (TempParam->NumberofParameters > 0))
      {
         /* If we have a parameter, use it.                             */
         CallState = TempParam->Params[0].intParam;

         /* Check if in HF or AG role                                   */
         if(IsHandsFree)
         {
            /* Now check to make sure that the Port ID appears to be    */
            /* semi-valid.                                              */
            if((ServerConnected)?CurrentServerPortID:CurrentClientPortID)
            {
               /* HF role needs to send the AT command version.         */
               Result = HFRE_Set_Incoming_Call_State(BluetoothStackID, ((ServerConnected)?CurrentServerPortID:CurrentClientPortID), (HFRE_Call_State_t) CallState);

               strcpy(temp, "HFRE_Set_Incoming_Call_State");
            }
            else
            {
               printf("HFRESendResponseAndHold: Invalid Client Port ID.\n");

               Result = INVALID_PARAMETERS_ERROR;
            }
         }
         else
         {
            if((ServerConnected)?CurrentServerPortID:CurrentClientPortID)
            {
               /* AG uses the response format.                          */
               Result = HFRE_Send_Incoming_Call_State(BluetoothStackID, ((ServerConnected)?CurrentServerPortID:CurrentClientPortID), (HFRE_Call_State_t) CallState);

               strcpy(temp, "HFRE_Set_Incoming_Call_State");
            }
            else
            {
               printf("HFRESendResponseAndHold: Invalid Client Server Port ID.\n");

               Result = INVALID_PARAMETERS_ERROR;
            }
         }

         printf("HFRESendResponseAndHold function %s: Function %s, Returned Result %d\n", temp, Result?"Failed ":"Succeeded", Result);
      }
      else
      {
         printf("Usage: SendRespHold [CallState].\n");

         Result = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      printf("Stack ID Invalid.\n");

      Result = INVALID_STACK_ID_ERROR;
   }

   return(Result);
}

   /* The following function is responsible for sending the command to  */
   /* request the current calls list (+CLCC) from the Remote Audio      */
   /* Gateway Device.  This function returns zero on successful         */
   /* execution and a negative value on all errors.                     */
static int HFREQueryRemoteCallList(ParameterList_t *TempParam)
{
   int Result;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the Port ID appears to be          */
      /* semi-valid.                                                    */
      if((ServerConnected)?CurrentServerPortID:CurrentClientPortID)
      {
         /* The Port ID appears to be is a semi-valid value.  Now submit*/
         /* the command.                                                */
         Result = HFRE_Query_Remote_Current_Calls_List(BluetoothStackID, ((ServerConnected)?CurrentServerPortID:CurrentClientPortID));

         printf("HFRE_Query_Remote_Current_Calls_List function: %s, Returned Result %d\n", Result?"Failed ":"Succeeded", Result);
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("HFRE_Query_Remote_Current_Calls_List: Invalid Client Port ID.\n");

         Result = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      printf("Stack ID Invalid.\n");

      Result = INVALID_STACK_ID_ERROR;
   }

   return(Result);
}

   /* The following function is responsible for sending the command(s)  */
   /* to send the Call Entry to a remote Hands Free Device.  This       */
   /* function returns zero on successful execution and a negative value*/
   /* on all errors.                                                    */
static int SendCallEntryCommand(ParameterList_t *TempParam)
{
   int                             Result;
   int                             Loop;
   char                            Buffer[132];
   char                            SampleCommand[]="1 1 1 0 0 5551212 129";
   char                           *Array[8 + 1];
   Boolean_t                       Final;
   HFRE_Current_Call_List_Entry_t  CurrentCallListEntry;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the Port ID appears to be          */
      /* semi-valid.                                                    */
      if((ServerConnected)?CurrentServerPortID:CurrentClientPortID)
      {
         /* Check to see if no parameters were entered.  If so, then    */
         /* give usage message.                                         */
         if(TempParam->NumberofParameters)
         {
            /* Check to see if an empty list is being specified.        */
            if((TempParam->NumberofParameters == 1) && (TempParam->Params[0].intParam == 1))
            {
               /* Just send a terminating response.                     */
               Result = HFRE_Send_Current_Calls_List(BluetoothStackID, ((ServerConnected)?CurrentServerPortID:CurrentClientPortID), NULL, TRUE);

               printf("HFRE_Send_Current_Calls_List function: %s, Returned Result %d\n", Result?"Failed ":"Succeeded", Result);

               EntryIndex = 0;
            }
            else
            {
               /* The Port ID appears to be a semi-valid value.  If we  */
               /* don't have enough parameters for a command, we default*/
               /* to an interactive mode.                               */
               if(TempParam->NumberofParameters < ((sizeof(Array)/sizeof(char *)) - 2))
               {
                  /* Now lead the user through the use of this command. */
                  printf("Enter information about a call as follows:\n");
                  printf("\n");
                  printf("   FinalEntry Dir Status Mode Multiparty Number NumFormat\n");
                  printf("\n");
                  printf("where:\n");
                  printf("\n");
                  printf("   - FinalEntry is an integer 0 (not the last entry) or 1 (is the last entry)\n");
                  printf("   - Dir is an integer 0 (Outbound) or 1 (Inbound)\n");
                  printf("   - Status is an integer between 0-5, and represents (in order of 0-5)\n");
                  printf("        (Active, csHeld, csDialing, csAlerting, csIncoming, or csWaiting)\n");
                  printf("   - Mode is an integer between 0-2, and represents (in order of 0-2\n");
                  printf("        (Voice(0), Data(1) or Fax(2) call)\n");
                  printf("   - Multiparty is an integer 0 (Not multiparty) or 1 (multiparty)\n");
                  printf("   - Number is a string indicating the phone number\n");
                  printf("   - NumFormat is a string indicating the phone number format\n");
                  printf("\n");
                  printf("For example the line:\n%s\n", SampleCommand);
                  printf("has the final entry flag set.  It indicates an inbound, held, voice call to the\nsingle party number 5551212 using the HFRE Default number format.\n");
                  printf("If nothing is entered, the sample line above is used for this test.\n\n");

                  Final  = FALSE;
                  Result = 0;
                  while((!Final) && (!Result))
                  {
                     printf("Audio Gateway Enter Call>");

                     Buffer[0] = '\0';
                     if(fgets(Buffer, sizeof(Buffer), stdin) != NULL)
                     {
                        /* Determine the number of arguments.           */
                        Loop = 0;
                        while(Loop <((sizeof(Array)/sizeof(char *))- 1))
                        {
                           Array[Loop] = strtok((Loop == 0)?Buffer:NULL, " ");
                           if(!Array[Loop])
                              break;

                           Loop++;
                        }

                        /* verify that we have the proper number of     */
                        /* arguments.                                   */
                        if(Loop < 5)
                        {
                            printf("no command entered using default parameters.\n");

                            /* copy default command for parsing for     */
                            /* here.                                    */
                            strcpy(Buffer, SampleCommand);

                            /* determine the number of arguments.       */
                            Loop = 0;
                            while(Loop <((sizeof(Array)/sizeof(char *))- 1))
                            {
                               Array[Loop] = strtok((Loop == 0)?Buffer:NULL, " ");
                               if(!Array[Loop])
                                  break;

                               Loop++;
                            }
                        }

                        /* make sure we have a full command line.       */
                        if(Loop >= 5)
                        {
                           /* Note we don't really pay attention to bad */
                           /* data entry.                               */
                           Final = (Boolean_t)atoi(Array[0]);

                           /* Store index and increment.                */
                           CurrentCallListEntry.Index = EntryIndex++;

                           /* Get call direction.                       */
                           CurrentCallListEntry.CallDirection = (HFRE_Call_Direction_t)atoi(Array[1]);

                           /* Get call status.                          */
                           CurrentCallListEntry.CallStatus = (HFRE_Call_Status_t)atoi(Array[2]);

                           /* Get call Mode.                            */
                           CurrentCallListEntry.CallMode = (HFRE_Call_Mode_t)atoi(Array[3]);

                           /* Get call Mode.                            */
                           CurrentCallListEntry.Multiparty = (Boolean_t)atoi(Array[4]);

                           /* Note that the phone number is optional.   */
                           /* If it has not been specified then there   */
                           /* really isn't any need to process any more.*/
                           if((Array[5]) && (Array[6]))
                           {
                              /* Get Phone Number.                      */
                              CurrentCallListEntry.PhoneNumber  = Array[5];

                              /* Finally, number format.                */
                              CurrentCallListEntry.NumberFormat = atoi(Array[6]);
                           }
                           else
                           {
                              /* Number not specified.                  */
                              CurrentCallListEntry.PhoneNumber  = NULL;

                              CurrentCallListEntry.NumberFormat = HFRE_DEFAULT_NUMBER_FORMAT;
                           }

                           /* Dispatch the call entry to the stack.     */
                           Result = HFRE_Send_Current_Calls_List(BluetoothStackID, ((ServerConnected)?CurrentServerPortID:CurrentClientPortID), &CurrentCallListEntry, Final);

                           printf("HFRE_Send_Current_Calls_List function: %s, Returned Result %d\n", Result?"Failed ":"Succeeded", Result);

                           /* Exit the loop if necessary.               */
                           if(Result)
                           {
                              Final      = TRUE;

                              EntryIndex = 0;
                           }
                        }
                        else
                        {
                           printf("Improper command entered using default parameters.\n");

                           break;
                        }
                     }
                     else
                     {
                        printf("Input Read Error.\n");
                        break;
                     }
                  }
               }
               else
               {
                  /* We have enough parameters to try and interpret a   */
                  /* command.                                           */

                  /* Note we don't really pay attention to bad data     */
                  /* entry.                                             */
                  Final = (Boolean_t)(TempParam->Params[0].intParam);

                  /* Store index and increment.                         */
                  CurrentCallListEntry.Index = EntryIndex++;

                  /* Get call direction.                                */
                  CurrentCallListEntry.CallDirection = (HFRE_Call_Direction_t)TempParam->Params[1].intParam;

                  /* Get call status.                                   */
                  CurrentCallListEntry.CallStatus = (HFRE_Call_Status_t)TempParam->Params[2].intParam;

                  /* Get call Mode.                                     */
                  CurrentCallListEntry.CallMode = (HFRE_Call_Mode_t)TempParam->Params[3].intParam;

                  /* Get call Mode.                                     */
                  CurrentCallListEntry.Multiparty = (Boolean_t)TempParam->Params[4].intParam;

                  /* Get Phone Number.                                  */
                  CurrentCallListEntry.PhoneNumber = TempParam->Params[5].strParam;

                  /* Finally, number format.                            */
                  CurrentCallListEntry.NumberFormat = TempParam->Params[6].intParam;

                  /* Dispatch the call entry to the stack.              */
                  Result = HFRE_Send_Current_Calls_List(BluetoothStackID, ((ServerConnected)?CurrentServerPortID:CurrentClientPortID), &CurrentCallListEntry, Final);

                  printf("HFRE_Send_Current_Calls_List function: %s, Returned Result %d\n", Result?"Failed ":"Succeeded", Result);

                  if(Final)
                     EntryIndex = 0;
               }
            }
         }
         else
         {
            /* We need a result code parameter.                         */
            printf("Usage: SendCallList [Mode | Call Entry ].\n");
            printf("   Where    Mode: 0 - Manual Entry Mode.\n");
            printf("            Mode: 1 - Auto 'No Calls' Response.\n");
            printf("       CallEntry: [Final] [Dir] [Status] [Multi] [Number] [Format].\n");

            Result = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("HFRE_Send_Current_Calls_List: Invalid Port ID.\n");

         Result = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      printf("Stack ID Invalid.\n");

      Result = INVALID_STACK_ID_ERROR;
   }

   return(Result);
}

   /* The following function is responsible for sending the command to  */
   /* send an extended error code to the Remote Hands Free Device.  This*/
   /* function returns zero on successful execution and a negative value*/
   /* on all errors.                                                    */
static int HFRESendExtendedError(ParameterList_t *TempParam)
{
   int          Result;
   unsigned int ResultCode;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the Port ID appears to be          */
      /* semi-valid.                                                    */
      if((ServerConnected)?CurrentServerPortID:CurrentClientPortID)
      {
         /* The Port ID appears to be is a semi-valid value.            */

         /* Make sure the passed in parameters appears to be semi-valid.*/
         if((TempParam) && (TempParam->NumberofParameters > 0))
         {
            /* If we have a parameter, use it.                          */
            ResultCode = TempParam->Params[0].intParam;

            Result = HFRE_Send_Extended_Error_Result(BluetoothStackID, ((ServerConnected)?CurrentServerPortID:CurrentClientPortID), ResultCode);

            printf("HFRE_Send_Extended_Error_Result function: %s, Returned Result %d\n", Result?"Failed ":"Succeeded", Result);
         }
         else
         {
            /* We need a result code parameter.                         */
            printf("Usage: SendExtendedError [Result Code].\n");

            Result = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("HFRE_Send_Extended_Error_Result: Invalid Port ID.\n");

         Result = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      printf("Stack ID Invalid.\n");

      Result = INVALID_STACK_ID_ERROR;
   }

   return(Result);
}

   /* The following function is responsible for sending the command to  */
   /* send a subscriber number to the Remote Hands Free Device.  This   */
   /* function returns zero on successful execution and a negative value*/
   /* on all errors.                                                    */
static int HFRESendSubscriberNumber(ParameterList_t *TempParam)
{
   int Result;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the Port ID appears to be          */
      /* semi-valid.                                                    */
      if((ServerConnected)?CurrentServerPortID:CurrentClientPortID)
      {
         /* The Port ID appears to be a semi-valid value.               */

         /* Make sure the passed in parameters appears to be semi-valid.*/
         if((TempParam) && (TempParam->NumberofParameters > 0))
         {
            /* If we have a parameter, use it.                          */
            Result = HFRE_Send_Subscriber_Number_Information(BluetoothStackID, ((ServerConnected)?CurrentServerPortID:CurrentClientPortID), TempParam->Params[0].strParam, 4, HFRE_DEFAULT_NUMBER_FORMAT, TRUE);

            printf("HFRE_Send_Subscriber_Number_Information function: %s, Returned Result %d\n", Result?"Failed ":"Succeeded", Result);
         }
         else
         {
            /* We need a subscriber number parameter.                   */
            printf("Usage: SendSubNumber [Number].\n");

            Result = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("HFRE_Send_Subscriber_Number_Information: Invalid Port ID.\n");

         Result = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      printf("Stack ID Invalid.\n");

      Result = INVALID_STACK_ID_ERROR;
   }

   return(Result);
}

   /* The following function is responsible for sending the command to  */
   /* send an Operator Selection Response to the Remote Hands Free      */
   /* Device.  This function returns zero on successful execution and a */
   /* negative value on all errors.                                     */
static int HFRESendOperatorSelection(ParameterList_t *TempParam)
{
   int Result;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the Port ID appears to be          */
      /* semi-valid.                                                    */
      if((ServerConnected)?CurrentServerPortID:CurrentClientPortID)
      {
         /* The Port ID appears to be a semi-valid value.               */

         /* Make sure the passed in parameters appears to be semi-valid.*/
         if((TempParam) && (TempParam->NumberofParameters > 0))
         {
            /* If we have a parameter, use it.                          */
            Result = HFRE_Send_Network_Operator_Selection(BluetoothStackID, ((ServerConnected)?CurrentServerPortID:CurrentClientPortID), 1, TempParam->Params[0].strParam);

            printf("HFRE_Send_Network_Operator_Selection function: %s, Returned Result %d\n", Result?"Failed ":"Succeeded", Result);
         }
         else
         {
            /* We need an operator name parameter.                      */
            printf("Usage: SendOperatorInfo [OperatorName].\n");

            Result = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("HFRE_Send_Network_Operator_Selection: Invalid Port ID.\n");

         Result = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      printf("Stack ID Invalid.\n");

      Result = INVALID_STACK_ID_ERROR;
   }

   return(Result);
}

   /* The following function is responsible for setting up or releasing */
   /* an audio connection.  This function returns zero on successful    */
   /* execution and a negative value on all errors.                     */
static int ManageAudioConnection(ParameterList_t *TempParam)
{
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* The Port ID appears to be at least semi-valid, now check the   */
      /* required parameters for this command.                          */
      if((TempParam) && (TempParam->NumberofParameters > 0))
      {
         /* Check to see if this is a request to setup an audio         */
         /* connection or disconnect an audio connection.               */
         if(TempParam->Params->intParam)
         {
            /* This is a request to setup an audio connection, call the */
            /* Setup Audio Connection function.                         */
            ret_val = HFRESetupAudioConnection();
         }
         else
         {
            /* This is a request to disconnect an audio connection, call*/
            /* the Release Audio Connection function.                   */
            ret_val = HFREReleaseAudioConnection();
         }
      }
      else
      {
         /* The required parameter is invalid.                          */
         printf("Usage: Audio [Release = 0, Setup = 1].\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      printf("Stack ID Invalid.\n");

      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for setting up or releasing */
   /* an audio connection.  This function returns zero on successful    */
   /* execution and a negative value on all errors.                     */
static int SelectCodec(ParameterList_t *TempParam)
{
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* The Port ID appears to be at least semi-valid, now check the   */
      /* required parameters for this command.                          */
      if((TempParam) && (TempParam->NumberofParameters > 0) && ((TempParam->Params->intParam == 1) || (TempParam->Params->intParam == 2)))
      {
         /* Verify that we have a connection and the remote device      */
         /* supports this function.  Send the selected Codec.           */
         if(CurrentClientPortID)
         {
            if(RemoteSupportedFeatures & HFRE_HF_CODEC_NEGOTIATION_SUPPORTED_BIT)
            {
               RequestedCodec = TempParam->Params->intParam;
               ret_val = HFRE_Send_Select_Codec(BluetoothStackID, CurrentClientPortID, (unsigned char)RequestedCodec);
            }
            else
            {
               printf("Select Codec not supported by remote\n");
               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            printf("No Device Connected\n");
            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* The required parameter is invalid.                          */
         printf("Usage: SelectCodec [CSVD = 1, mSBD = 2].\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      printf("Stack ID Invalid.\n");

      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}


   /* The following function is responsible for Setting up an Audio     */
   /* Connection.  This function returns zero on successful execution   */
   /* and a negative value on all errors.                               */
static int HFRESetupAudioConnection(void)
{
   int Result;
   int ret_val;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the Port ID appears to be          */
      /* semi-valid.                                                    */
      if(((ServerConnected)?CurrentServerPortID:CurrentClientPortID))
      {
         /* The Port ID appears to be a semi-valid value.  Now submit   */
         /* the command.                                                */
         Result  = HFRE_Setup_Audio_Connection(BluetoothStackID, ((ServerConnected)?CurrentServerPortID:CurrentClientPortID));

         /* Set the return value of this function equal to the Result of*/
         /* the function call.                                          */
         ret_val = Result;

         /* Now check to see if the command was submitted successfully. */
         if(!Result)
         {
            /* The function was submitted successfully.                 */
            printf("HFRE_Setup_Audio_Connection: Function Successful.\n");
         }
         else
         {
            /* There was an error submitting the function.              */
            printf("HFRE_Setup_Audio_Connection() Failure: %d.\n", Result);
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("Setup Audio Connection: Invalid Port ID.\n");

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      printf("Stack ID Invalid.\n");

      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for Releasing an existing   */
   /* Audio Connection.  This function returns zero on successful       */
   /* execution and a negative value on all errors.                     */
static int HFREReleaseAudioConnection(void)
{
   int Result;
   int ret_val;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the Port ID appears to be          */
      /* semi-valid.                                                    */
      if(((ServerConnected)?CurrentServerPortID:CurrentClientPortID))
      {
         /* The Port ID appears to be a semi-valid value.  Now submit   */
         /* the command.                                                */
         Result  = HFRE_Release_Audio_Connection(BluetoothStackID, ((ServerConnected)?CurrentServerPortID:CurrentClientPortID));

         /* Set the return value of this function equal to the Result of*/
         /* the function call.                                          */
         ret_val = Result;

         /* Now check to see if the command was submitted successfully. */
         if(!Result)
         {
            /* The function was submitted successfully.                 */
            printf("HFRE_Release_Audio_Connection: Function Successful.\n");
         }
         else
         {
            /* There was an error submitting the function.              */
            printf("HFRE_Release_Audio_Connection() Failure: %d.\n", Result);
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("Release Audio Connection: Invalid Port ID.\n");

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      printf("Stack ID Invalid.\n");

      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for closing any open client */
   /* port.  This function returns zero on successful execution and a   */
   /* negative value on all errors.                                     */
static int HFRECloseClient(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Check to see if the Current Port ID appears to be semi-valid.  */
      /* This parameter will only be valid if a Client Port is open.    */
      if(((ServerConnected)?CurrentServerPortID:CurrentClientPortID))
      {
         /* The Port ID appears to be semi-valid.  Now try to close the */
         /* Port.                                                       */
         Result  = HFRE_Close_Port(BluetoothStackID, ((ServerConnected)?CurrentServerPortID:CurrentClientPortID));

         /* Set the return value of this function equal to the Result of*/
         /* the function call.                                          */
         ret_val = Result;

         /* Now check to see if the command was submitted successfully. */
         if(!Result)
         {
            /* The function was called successfully.  Display a message */
            /* indicating that the Client was successfully closed.      */
            printf("HFRE_Close_Port: Function Successful.\n");

            CurrentClientPortID = 0;
            ServerConnected     = FALSE;

            /* Flag that an Audio Connection is not present.            */
            AudioConnected      = FALSE;
         }
         else
         {
            /* An error occurred while attempting to close the Client.  */
            printf("HFRE_Close_Port() Failure: %d.\n", Result);
         }
      }
      else
      {
         /* The Current Port ID is invalid so no Client Port is open.   */
         printf("Invalid Client ID: HFREClosetClient.\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      printf("Stack ID Invalid.\n");

      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
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
         printf("%*s Attribute ID 0x%04X\n", (InitLevel*INDENT_LENGTH), "", SDPServiceAttributeResponse->SDP_Service_Attribute_Value_Data[Index].Attribute_ID);

         /* Now Print out all of the SDP Data Elements that were        */
         /* returned that are associated with the SDP Attribute.        */
         DisplayDataElement(SDPServiceAttributeResponse->SDP_Service_Attribute_Value_Data[Index].SDP_Data_Element, (InitLevel + 1));
      }
   }
   else
      printf("No SDP Attributes Found.\n");
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
         printf("Service Record: %u:\n", (Index + 1));

         /* Call Display SDPAttributeResponse for all SDP Service       */
         /* Records received.                                           */
         DisplaySDPAttributeResponse(&(SDPServiceSearchAttributeResponse->SDP_Service_Attribute_Response_Data[Index]), 1);
      }
   }
   else
      printf("No SDP Service Records Found.\n");
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
         printf("%*s Type: NIL\n", (Level*INDENT_LENGTH), "");
         break;
      case deNULL:
         /* Display the NULL Type.                                      */
         printf("%*s Type: NULL\n", (Level*INDENT_LENGTH), "");
         break;
      case deUnsignedInteger1Byte:
         /* Display the Unsigned Integer (1 Byte) Type.                 */
         printf("%*s Type: Unsigned Int = 0x%02X\n", (Level*INDENT_LENGTH), "", SDPDataElement->SDP_Data_Element.UnsignedInteger1Byte);
         break;
      case deUnsignedInteger2Bytes:
         /* Display the Unsigned Integer (2 Bytes) Type.                */
         printf("%*s Type: Unsigned Int = 0x%04X\n", (Level*INDENT_LENGTH), "", SDPDataElement->SDP_Data_Element.UnsignedInteger2Bytes);
         break;
      case deUnsignedInteger4Bytes:
         /* Display the Unsigned Integer (4 Bytes) Type.                */
         printf("%*s Type: Unsigned Int = 0x%08X\n", (Level*INDENT_LENGTH), "", (unsigned int)SDPDataElement->SDP_Data_Element.UnsignedInteger4Bytes);
         break;
      case deUnsignedInteger8Bytes:
         /* Display the Unsigned Integer (8 Bytes) Type.                */
         printf("%*s Type: Unsigned Int = 0x%02X%02X%02X%02X%02X%02X%02X%02X\n", (Level*INDENT_LENGTH), "", SDPDataElement->SDP_Data_Element.UnsignedInteger8Bytes[7],
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
         printf("%*s Type: Unsigned Int = 0x%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X\n", (Level*INDENT_LENGTH), "", SDPDataElement->SDP_Data_Element.UnsignedInteger16Bytes[15],
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
         printf("%*s Type: Signed Int = 0x%02X\n", (Level*INDENT_LENGTH), "", SDPDataElement->SDP_Data_Element.SignedInteger1Byte);
         break;
      case deSignedInteger2Bytes:
         /* Display the Signed Integer (2 Bytes) Type.                  */
         printf("%*s Type: Signed Int = 0x%04X\n", (Level*INDENT_LENGTH), "", SDPDataElement->SDP_Data_Element.SignedInteger2Bytes);
         break;
      case deSignedInteger4Bytes:
         /* Display the Signed Integer (4 Bytes) Type.                  */
         printf("%*s Type: Signed Int = 0x%08X\n", (Level*INDENT_LENGTH), "", (unsigned int)SDPDataElement->SDP_Data_Element.SignedInteger4Bytes);
         break;
      case deSignedInteger8Bytes:
         /* Display the Signed Integer (8 Bytes) Type.                  */
         printf("%*s Type: Signed Int = 0x%02X%02X%02X%02X%02X%02X%02X%02X\n", (Level*INDENT_LENGTH), "", SDPDataElement->SDP_Data_Element.SignedInteger8Bytes[7],
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
         printf("%*s Type: Signed Int = 0x%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X\n", (Level*INDENT_LENGTH), "", SDPDataElement->SDP_Data_Element.SignedInteger16Bytes[15],
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

         printf("%*s Type: Text String = %s\n", (Level*INDENT_LENGTH), "", Buffer);
         break;
      case deBoolean:
         printf("%*s Type: Boolean = %s\n", (Level*INDENT_LENGTH), "", (SDPDataElement->SDP_Data_Element.Boolean)?"TRUE":"FALSE");
         break;
      case deURL:
         /* First retrieve the Length of the URL String so that we can  */
         /* copy the Data into our Buffer.                              */
         Index = (SDPDataElement->SDP_Data_Element_Length < sizeof(Buffer))?SDPDataElement->SDP_Data_Element_Length:(sizeof(Buffer)-1);

         /* Copy the URL String into the Buffer and then NULL terminate */
         /* it.                                                         */
         memcpy(Buffer, SDPDataElement->SDP_Data_Element.URL, Index);
         Buffer[Index] = '\0';

         printf("%*s Type: URL = %s\n", (Level*INDENT_LENGTH), "", Buffer);
         break;
      case deUUID_16:
         printf("%*s Type: UUID_16 = 0x%02X%02X\n", (Level*INDENT_LENGTH), "", SDPDataElement->SDP_Data_Element.UUID_16.UUID_Byte0,
                                                                                 SDPDataElement->SDP_Data_Element.UUID_16.UUID_Byte1);
         break;
      case deUUID_32:
         printf("%*s Type: UUID_32 = 0x%02X%02X%02X%02X\n", (Level*INDENT_LENGTH), "", SDPDataElement->SDP_Data_Element.UUID_32.UUID_Byte0,
                                                                                         SDPDataElement->SDP_Data_Element.UUID_32.UUID_Byte1,
                                                                                         SDPDataElement->SDP_Data_Element.UUID_32.UUID_Byte2,
                                                                                         SDPDataElement->SDP_Data_Element.UUID_32.UUID_Byte3);
         break;
      case deUUID_128:
         printf("%*s Type: UUID_128 = 0x%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X\n", (Level*INDENT_LENGTH), "", SDPDataElement->SDP_Data_Element.UUID_128.UUID_Byte0,
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
         printf("%*s Type: Data Element Sequence\n", (Level*INDENT_LENGTH), "");

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
         printf("%*s Type: Data Element Alternative\n", (Level*INDENT_LENGTH), "");

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
         printf("%*s Unknown SDP Data Element Type\n", (Level*INDENT_LENGTH), "");
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
   /* installed callback (i.e.  this function DOES NOT have be          */
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
   char                              NameBuffer[256];
   char                             *Device_Name;
   unsigned long                     Data_Type;
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
      printf("\n");

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
               printf("GAP_Inquiry_Result: %d Found.\n", GAP_Inquiry_Event_Data->Number_Devices);

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

                     printf("GAP Inquiry Result: %d, %s.\n", (Index+1), BoardStr);
                  }

                  NumberofValidResponses = GAP_Inquiry_Event_Data->Number_Devices;
               }
            }
            break;
         case etInquiry_Entry_Result:
            /* Next convert the BD_ADDR to a string.                    */
            BD_ADDRToStr(GAP_Event_Data->Event_Data.GAP_Inquiry_Entry_Event_Data->BD_ADDR, BoardStr);

            /* Display this GAP Inquiry Entry Result.                   */
            printf("GAP Inquiry Entry Result - %s\n", BoardStr);
            break;
         case etInquiry_With_RSSI_Entry_Result:
            /* Next convert the BD_ADDR to a string.                    */
            BD_ADDRToStr(GAP_Event_Data->Event_Data.GAP_Inquiry_Entry_Event_Data->BD_ADDR, BoardStr);

            /* Display this GAP Inquiry Entry Result.                   */
            printf("GAP Inquiry Entry Result - %s RSSI: %4d\n", BoardStr, GAP_Event_Data->Event_Data.GAP_Inquiry_With_RSSI_Entry_Event_Data->RSSI);
            break;
         case etExtended_Inquiry_Entry_Result:
            /* Next convert the BD_ADDR to a string.                    */
            BD_ADDRToStr(GAP_Event_Data->Event_Data.GAP_Inquiry_Entry_Event_Data->BD_ADDR, BoardStr);

            /* Search for the local name element in the EIR data.       */
            Device_Name = NULL;
            for(Index=0;(Index<(int)GAP_Event_Data->Event_Data.GAP_Extended_Inquiry_Entry_Event_Data->Extended_Inquiry_Response_Data.Number_Data_Entries);Index++)
            {
               Data_Type = (unsigned long)GAP_Event_Data->Event_Data.GAP_Extended_Inquiry_Entry_Event_Data->Extended_Inquiry_Response_Data.Data_Entries[Index].Data_Type;

               if((Data_Type == HCI_EXTENDED_INQUIRY_RESPONSE_DATA_TYPE_LOCAL_NAME_SHORTENED) || Data_Type == HCI_EXTENDED_INQUIRY_RESPONSE_DATA_TYPE_LOCAL_NAME_COMPLETE)
               {
                  BTPS_MemCopy(NameBuffer, GAP_Event_Data->Event_Data.GAP_Extended_Inquiry_Entry_Event_Data->Extended_Inquiry_Response_Data.Data_Entries[Index].Data_Buffer, GAP_Event_Data->Event_Data.GAP_Extended_Inquiry_Entry_Event_Data->Extended_Inquiry_Response_Data.Data_Entries[Index].Data_Length);
                  NameBuffer[GAP_Event_Data->Event_Data.GAP_Extended_Inquiry_Entry_Event_Data->Extended_Inquiry_Response_Data.Data_Entries[Index].Data_Length] = 0;
                  Device_Name = NameBuffer;
                  break;
               }
            }

            /* Display this GAP Inquiry Entry Result.                   */
            if(Device_Name)
               printf("GAP Inquiry Entry Result - %s RSSI: %4d Device Name: %s\n", BoardStr, GAP_Event_Data->Event_Data.GAP_Inquiry_With_RSSI_Entry_Event_Data->RSSI, Device_Name);
            else
               printf("GAP Inquiry Entry Result - %s RSSI: %4d\n", BoardStr, GAP_Event_Data->Event_Data.GAP_Inquiry_With_RSSI_Entry_Event_Data->RSSI);

            break;
         case etAuthentication:
            /* An authentication event occurred, determine which type of*/
            /* authentication event occurred.                           */
            switch(GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->GAP_Authentication_Event_Type)
            {
               case atLinkKeyRequest:
                  BD_ADDRToStr(GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device, BoardStr);
                  printf("atLinkKeyRequest: %s\n", BoardStr);

                  /* If not already known, add device to inquiry list.  */
                  AddDeviceToList(GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device);

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
                     printf("GAP_Authentication_Response() Success.\n");
                  else
                     printf("GAP_Authentication_Response() Failure: %d.\n", Result);
                  break;
               case atPINCodeRequest:
                  /* A pin code request event occurred, first display   */
                  /* the BD_ADD of the remote device requesting the pin.*/
                  BD_ADDRToStr(GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device, BoardStr);
                  printf("atPINCodeRequest: %s\n", BoardStr);

                  /* Note the current Remote BD_ADDR that is requesting */
                  /* the PIN Code.                                      */
                  CurrentRemoteBD_ADDR = GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device;

                  /* Inform the user that they will need to respond with*/
                  /* a PIN Code Response.                               */
                  printf("\nRespond with the command: PINCodeResponse\n");
                  break;
               case atAuthenticationStatus:
                  /* An authentication status event occurred, display   */
                  /* all relevant information.                          */
                  BD_ADDRToStr(GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device, BoardStr);
                  printf("atAuthenticationStatus: %d Board: %s\n", GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Authentication_Event_Data.Authentication_Status, BoardStr);

                  /* Flag that there is no longer a current             */
                  /* Authentication procedure in progress.              */
                  ASSIGN_BD_ADDR(CurrentRemoteBD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
                  break;
               case atLinkKeyCreation:
                  /* A link key creation event occurred, first display  */
                  /* the remote device that caused this event.          */
                  BD_ADDRToStr(GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device, BoardStr);
                  printf("atLinkKeyCreation: %s\n", BoardStr);

                  /* The BD_ADDR of the remote device has been displayed*/
                  /* now display the link key being created.            */
                  printf("Link Key: 0x");

                  for(Index = 0;Index<sizeof(Link_Key_t);Index++)
                     printf("%02X", ((Byte_t *)(&(GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Authentication_Event_Data.Link_Key_Info.Link_Key)))[(sizeof(Link_Key_t)-1)-Index]);

                  printf("\n");

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

                     printf("Link Key Stored locally.\n");
                  }
                  else
                     printf("Link Key NOT Stored locally: Link Key array is full.\n");
                  break;
               case atIOCapabilityRequest:
                  BD_ADDRToStr(GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device, BoardStr);
                  printf("atIOCapabilityRequest: %s\n", BoardStr);

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
                     printf("GAP_Authentication_Response() Success.\n");
                  else
                     printf("GAP_Authentication_Response() Failure: %d.\n", Result);
                  break;
               case atIOCapabilityResponse:
                  BD_ADDRToStr(GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device, BoardStr);
                  printf("atIOCapabilityResponse: %s\n", BoardStr);

                  RemoteIOCapability = (int)GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Authentication_Event_Data.IO_Capabilities.IO_Capability;
                  MITM               = (Boolean_t)GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Authentication_Event_Data.IO_Capabilities.MITM_Protection_Required;
                  OOB_Data           = (Boolean_t)GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Authentication_Event_Data.IO_Capabilities.OOB_Data_Present;

                  printf("Remote Capabilities: %s%s%s\n", IOCapabilitiesStrings[RemoteIOCapability], ((MITM)?", MITM":""), ((OOB_Data)?", OOB Data":""));
                  break;
               case atUserConfirmationRequest:
                  BD_ADDRToStr(GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device, BoardStr);
                  printf("atUserConfirmationRequest: %s\n", BoardStr);

                  CurrentRemoteBD_ADDR = GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device;

                  if(IOCapability != icDisplayYesNo)
                  {
                     /* Invoke JUST Works Process...                    */
                     GAP_Authentication_Information.GAP_Authentication_Type          = atUserConfirmation;
                     GAP_Authentication_Information.Authentication_Data_Length       = (Byte_t)sizeof(Byte_t);
                     GAP_Authentication_Information.Authentication_Data.Confirmation = TRUE;

                     /* Submit the Authentication Response.             */
                     printf("\nAuto Accepting: %u\n", GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Authentication_Event_Data.Numeric_Value);

                     Result = GAP_Authentication_Response(BluetoothStackID, GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device, &GAP_Authentication_Information);

                     if(!Result)
                        printf("GAP_Authentication_Response() Success.\n");
                     else
                        printf("GAP_Authentication_Response() Failure: %d.\n", Result);

                     /* Flag that there is no longer a current          */
                     /* Authentication procedure in progress.           */
                     ASSIGN_BD_ADDR(CurrentRemoteBD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
                  }
                  else
                  {
                     printf("User Confirmation: %u\n", GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Authentication_Event_Data.Numeric_Value);

                     /* Inform the user that they will need to respond  */
                     /* with a PIN Code Response.                       */
                     printf("\nRespond with the command: UserConfirmationResponse\n");
                  }
                  break;
               case atPasskeyRequest:
                  BD_ADDRToStr(GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device, BoardStr);
                  printf("atPasskeyRequest: %s\n", BoardStr);

                  /* Note the current Remote BD_ADDR that is requesting */
                  /* the Passkey.                                       */
                  CurrentRemoteBD_ADDR = GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device;

                  /* Inform the user that they will need to respond with*/
                  /* a Passkey Response.                                */
                  printf("\nRespond with the command: PassKeyResponse\n");
                  break;
               case atRemoteOutOfBandDataRequest:
                  BD_ADDRToStr(GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device, BoardStr);
                  printf("atRemoteOutOfBandDataRequest: %s\n", BoardStr);

                  /* This application does not support OOB data so      */
                  /* respond with a data length of Zero to force a      */
                  /* negative reply.                                    */
                  GAP_Authentication_Information.GAP_Authentication_Type    = atOutOfBandData;
                  GAP_Authentication_Information.Authentication_Data_Length = 0;

                  Result = GAP_Authentication_Response(BluetoothStackID, GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device, &GAP_Authentication_Information);

                  if(!Result)
                     printf("GAP_Authentication_Response() Success.\n");
                  else
                     printf("GAP_Authentication_Response() Failure: %d.\n", Result);
                  break;
               case atPasskeyNotification:
                  BD_ADDRToStr(GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device, BoardStr);
                  printf("atPasskeyNotification: %s\n", BoardStr);

                  printf("Passkey Value: %u\n", GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Authentication_Event_Data.Numeric_Value);
                  break;
               case atKeypressNotification:
                  BD_ADDRToStr(GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device, BoardStr);
                  printf("atKeypressNotification: %s\n", BoardStr);

                  printf("Keypress: %d\n", (int)GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Authentication_Event_Data.Keypress_Type);
                  break;
               default:
                  printf("Un-handled GAP Authentication Event.\n");
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

               printf("GAP Remote Name Result: BD_ADDR: %s.\n", BoardStr);

               if(GAP_Remote_Name_Event_Data->Remote_Name)
                  printf("GAP Remote Name Result: %s.\n", GAP_Remote_Name_Event_Data->Remote_Name);
               else
                  printf("GAP Remote Name Result: NULL.\n");
            }
            break;
         default:
            /* An unknown/unexpected GAP event was received.            */
            printf("Unknown/Unhandled GAP Event: %d.\n", GAP_Event_Data->Event_Data_Type);
            break;
      }

      if(IsHandsFree)
         printf("Hands Free>");
      else
         printf("Audio Gateway>");
   }
   else
   {
      /* There was an error with one or more of the input parameters.   */
      printf("\n");

      printf("GAP Callback Data: Event_Data = NULL.\n");

      if(IsHandsFree)
         printf("Hands Free>");
      else
         printf("Audio Gateway>");
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
            printf("\n");
            printf("SDP Timeout Received (Size = 0x%04X).\n", sizeof(SDP_Response_Data_t));
            break;
         case rdConnectionError:
            /* A SDP Connection Error was received, display a message   */
            /* indicating this.                                         */
            printf("\n");
            printf("SDP Connection Error Received (Size = 0x%04X).\n", sizeof(SDP_Response_Data_t));
            break;
         case rdErrorResponse:
            /* A SDP error response was received, display all relevant  */
            /* information regarding this event.                        */
            printf("\n");
            printf("SDP Error Response Received (Size = 0x%04X) - Error Code: %d.\n", sizeof(SDP_Response_Data_t), SDP_Response_Data->SDP_Response_Data.SDP_Error_Response_Data.Error_Code);
            break;
         case rdServiceSearchResponse:
            /* A SDP Service Search Response was received, display all  */
            /* relevant information regarding this event                */
            printf("\n");
            printf("SDP Service Search Response Received (Size = 0x%04X) - Record Count: %d\n", sizeof(SDP_Response_Data_t), SDP_Response_Data->SDP_Response_Data.SDP_Service_Search_Response_Data.Total_Service_Record_Count);

            /* First, check to see if any SDP Service Records were      */
            /* found.                                                   */
            if(SDP_Response_Data->SDP_Response_Data.SDP_Service_Search_Response_Data.Total_Service_Record_Count)
            {
               printf("Record Handles:\n");

               for(Index = 0; (Word_t)Index < SDP_Response_Data->SDP_Response_Data.SDP_Service_Search_Response_Data.Total_Service_Record_Count; Index++)
               {
                  printf("Record %u: 0x%08X\n", (Index + 1), (unsigned int)SDP_Response_Data->SDP_Response_Data.SDP_Service_Search_Response_Data.Service_Record_List[Index]);
               }
            }
            else
               printf("No SDP Service Records Found.\n");
            break;
         case rdServiceAttributeResponse:
            /* A SDP Service Attribute Response was received, display   */
            /* all relevant information regarding this event            */
            printf("\n");
            printf("SDP Service Attribute Response Received (Size = 0x%04X)\n", sizeof(SDP_Response_Data_t));

            DisplaySDPAttributeResponse(&SDP_Response_Data->SDP_Response_Data.SDP_Service_Attribute_Response_Data, 0);
            break;
         case rdServiceSearchAttributeResponse:
            /* A SDP Service Search Attribute Response was received,    */
            /* display all relevant information regarding this event    */
            printf("\n");
            printf("SDP Service Search Attribute Response Received (Size = 0x%04X)\n", sizeof(SDP_Response_Data_t));

            DisplaySDPSearchAttributeResponse(&SDP_Response_Data->SDP_Response_Data.SDP_Service_Search_Attribute_Response_Data);
            break;
         default:
            /* An unknown/unexpected SDP event was received.            */
            printf("\n");
            printf("Unknown SDP Event.\n");
            break;
      }
   }
   else
   {
      /* There was an error with one or more of the input parameters.   */
      printf("\n");
      printf("SDP callback data: Response_Data = NULL.\n");
   }

   if(IsHandsFree)
      printf("Hands Free>");
   else
      printf("Audio Gateway>");

   /* Make sure the output is displayed to the user.                    */
   fflush(stdout);
}

   /* The following function is for an HFRE Event Callback.  This       */
   /* function will be called whenever a HFRE Event occurs that is      */
   /* associated with the Bluetooth Stack.  This function passes to the */
   /* caller the HFRE Event Data that occurred and the HFRE Event       */
   /* Callback Parameter that was specified when this Callback was      */
   /* installed.  The caller is free to use the contents of the HFRE    */
   /* Event Data ONLY in the context of this callback.  If the caller   */
   /* requires the Data for a longer period of time, then the callback  */
   /* function MUST copy the data into another Data Buffer.  This       */
   /* function is guaranteed NOT to be invoked more than once           */
   /* simultaneously for the specified installed callback (i.e.  this   */
   /* function DOES NOT have be reentrant).  It Needs to be noted       */
   /* however, that if the same Callback is installed more than once,   */
   /* then the callbacks will be called serially.  Because of this, the */
   /* processing in this function should be as efficient as possible.   */
   /* It should also be noted that this function is called in the Thread*/
   /* Context of a Thread that the User does NOT own.  Therefore,       */
   /* processing in this function should be as efficient as possible    */
   /* (this argument holds anyway because another HFRE Event will not be*/
   /* processed while this function call is outstanding).               */
   /* * NOTE * This function MUST NOT Block and wait for events that    */
   /*          can only be satisfied by Receiving SPP Event Packets.  A */
   /*          Deadlock WILL occur because NO SPP Event Callbacks will  */
   /*          be issued while this function is currently outstanding.  */
static void BTPSAPI HFRE_Event_Callback(unsigned int BluetoothStackID, HFRE_Event_Data_t *HFRE_Event_Data, unsigned long CallbackParameter)
{
   int          ret_val;
   char         BoardStr[13];
   Byte_t       AudioData[256];
   unsigned int Index;

   /* First, check to see if the required parameters appear to be       */
   /* semi-valid.                                                       */
   if(HFRE_Event_Data != NULL)
   {
      if(HFRE_Event_Data->Event_Data_Type != etHFRE_Audio_Data_Indication)
         printf("\n");

      /* The parameters appear to be semi-valid, now check to see what  */
      /* type the incoming event is.                                    */
      switch(HFRE_Event_Data->Event_Data_Type)
      {
         case etHFRE_Open_Port_Indication:
            /* A Client has connected to the Server, display the BD_ADDR*/
            /* of the connecting device.                                */
            SelectedCodec       = 1;
            RequestedCodec      = 0;
            CurrentClientPortID = HFRE_Event_Data->Event_Data.HFRE_Open_Port_Indication_Data->HFREPortID;
            BD_ADDRToStr(HFRE_Event_Data->Event_Data.HFRE_Open_Port_Indication_Data->BD_ADDR, BoardStr);
            printf("HFRE Open Port Indication, ID: 0x%04X, Board: %s.\n", HFRE_Event_Data->Event_Data.HFRE_Open_Port_Indication_Data->HFREPortID, BoardStr);

            ServerConnected = TRUE;
            break;
         case etHFRE_Open_Port_Confirmation:
            /* The Client received a Connect Confirmation, display      */
            /* relevant information.                                    */
            SelectedCodec  = 1;
            RequestedCodec = 0;
            printf("HFRE Open Port Confirmation, ID: 0x%04X, Status: 0x%04X.\n", HFRE_Event_Data->Event_Data.HFRE_Open_Port_Confirmation_Data->HFREPortID,
                                                                                   HFRE_Event_Data->Event_Data.HFRE_Open_Port_Confirmation_Data->PortOpenStatus);

            /* Check to see if the Client Port was opened successfully. */
            if(HFRE_Event_Data->Event_Data.HFRE_Open_Port_Confirmation_Data->PortOpenStatus)
            {
               /* There was an error while trying to open a Client Port.*/
               /* Invalidate the current Port ID.                       */
               CurrentClientPortID = 0;
            }
            break;
         case etHFRE_Open_Service_Level_Connection_Indication:
            /* A Open Service Level Indication was received, display    */
            /* relevant information.                                    */
            printf("HFRE Open Service Level Connection Indication, ID: 0x%04X, RemoteSupportedFeaturesValid: %s, RemoteSupportedFeatures: 0x%08lX, RemoteCallHoldMultipartySupport: 0x%08lX\n", HFRE_Event_Data->Event_Data.HFRE_Open_Service_Level_Connection_Indication_Data->HFREPortID, (HFRE_Event_Data->Event_Data.HFRE_Open_Service_Level_Connection_Indication_Data->RemoteSupportedFeaturesValid)?"TRUE":"FALSE", HFRE_Event_Data->Event_Data.HFRE_Open_Service_Level_Connection_Indication_Data->RemoteSupportedFeatures, HFRE_Event_Data->Event_Data.HFRE_Open_Service_Level_Connection_Indication_Data->RemoteCallHoldMultipartySupport);
            RemoteSupportedFeatures = HFRE_Event_Data->Event_Data.HFRE_Open_Service_Level_Connection_Indication_Data->RemoteSupportedFeatures;

            /* Initialize the Service Indicators.                       */
            InitializeDefaultIndicators();
            break;
         case etHFRE_Control_Indicator_Status_Indication:
            /* A Control Indicator Status Indication was received,      */
            /* display all relevant information.                        */
            switch(HFRE_Event_Data->Event_Data.HFRE_Control_Indicator_Status_Indication_Data->HFREControlIndicatorEntry.ControlIndicatorType)
            {
               case ciBoolean:
                  printf("HFRE Control Indicator Status Indication, ID: 0x%04X, Description: %s, Value: %s.\n", HFRE_Event_Data->Event_Data.HFRE_Control_Indicator_Status_Indication_Data->HFREPortID,
                                                                                                                  HFRE_Event_Data->Event_Data.HFRE_Control_Indicator_Status_Indication_Data->HFREControlIndicatorEntry.IndicatorDescription,
                                                                                                                  (HFRE_Event_Data->Event_Data.HFRE_Control_Indicator_Status_Indication_Data->HFREControlIndicatorEntry.Control_Indicator_Data.ControlIndicatorBooleanType.CurrentIndicatorValue)?"TRUE":"FALSE");
                  break;
               case ciRange:
                  printf("HFRE Control Indicator Status Indication, ID: 0x%04X, Description: %s, Value: %u.\n", HFRE_Event_Data->Event_Data.HFRE_Control_Indicator_Status_Indication_Data->HFREPortID,
                                                                                                                  HFRE_Event_Data->Event_Data.HFRE_Control_Indicator_Status_Indication_Data->HFREControlIndicatorEntry.IndicatorDescription,
                                                                                                                  HFRE_Event_Data->Event_Data.HFRE_Control_Indicator_Status_Indication_Data->HFREControlIndicatorEntry.Control_Indicator_Data.ControlIndicatorRangeType.CurrentIndicatorValue);
                  break;
            }
            break;
         case etHFRE_Control_Indicator_Status_Confirmation:
            /* A Control Indicator Status Confirmation was received,    */
            /* display all relevant information.                        */
            switch(HFRE_Event_Data->Event_Data.HFRE_Control_Indicator_Status_Confirmation_Data->HFREControlIndicatorEntry.ControlIndicatorType)
            {
               case ciBoolean:
                  printf("HFRE Control Indicator Status Confirmation, ID: 0x%04X, Description: %s, Value: %s.\n", HFRE_Event_Data->Event_Data.HFRE_Control_Indicator_Status_Confirmation_Data->HFREPortID,
                                                                                                                    HFRE_Event_Data->Event_Data.HFRE_Control_Indicator_Status_Confirmation_Data->HFREControlIndicatorEntry.IndicatorDescription,
                                                                                                                    (HFRE_Event_Data->Event_Data.HFRE_Control_Indicator_Status_Confirmation_Data->HFREControlIndicatorEntry.Control_Indicator_Data.ControlIndicatorBooleanType.CurrentIndicatorValue)?"TRUE":"FALSE");
                  break;
               case ciRange:
                  printf("HFRE Control Indicator Status Confirmation, ID: 0x%04X, Description: %s, Value: %u.\n", HFRE_Event_Data->Event_Data.HFRE_Control_Indicator_Status_Confirmation_Data->HFREPortID,
                                                                                                                    HFRE_Event_Data->Event_Data.HFRE_Control_Indicator_Status_Confirmation_Data->HFREControlIndicatorEntry.IndicatorDescription,
                                                                                                                    HFRE_Event_Data->Event_Data.HFRE_Control_Indicator_Status_Confirmation_Data->HFREControlIndicatorEntry.Control_Indicator_Data.ControlIndicatorRangeType.CurrentIndicatorValue);
                  break;
            }
            /* Add the Indicator to the Notification List if not already*/
            /* present.                                                 */
            AddNotificationIndicator(HFRE_Event_Data->Event_Data.HFRE_Control_Indicator_Status_Indication_Data->HFREControlIndicatorEntry.IndicatorDescription);
            break;
         case etHFRE_Call_Hold_Multiparty_Support_Confirmation:
            /* A Call Hold and Multiparty Support Confirmation was      */
            /* received, display all relevant information.              */
            printf("HFRE Call Hold Multiparty Support Confirmation, ID: 0x%04X, Support Mask: 0x%08lX.\n", HFRE_Event_Data->Event_Data.HFRE_Call_Hold_Multiparty_Support_Confirmation_Data->HFREPortID,
                                                                                                             HFRE_Event_Data->Event_Data.HFRE_Call_Hold_Multiparty_Support_Confirmation_Data->CallHoldSupportMask);
            break;
         case etHFRE_Call_Hold_Multiparty_Selection_Indication:
            /* A Call Hold and Multiparty Selection was received,       */
            /* display all relevant information.                        */
            printf("HFRE Call Hold Multiparty Selection Indication, ID: 0x%04X, Selection: 0x%04X.\n", HFRE_Event_Data->Event_Data.HFRE_Call_Hold_Multiparty_Selection_Indication_Data->HFREPortID,
                                                                                                         HFRE_Event_Data->Event_Data.HFRE_Call_Hold_Multiparty_Selection_Indication_Data->CallHoldMultipartyHandling);

            /* Send an OK terminating response code.                    */
            HFRE_Send_Terminating_Response(BluetoothStackID, HFRE_Event_Data->Event_Data.HFRE_Call_Hold_Multiparty_Selection_Indication_Data->HFREPortID, erOK, 0);
            break;
         case etHFRE_Call_Waiting_Notification_Activation_Indication:
            /* A Call Waiting Notification Activation was received,     */
            /* display all relevant information.                        */
            printf("HFRE Call Waiting Notification Activation Indication, ID: 0x%04X, Enabled: %s.\n", HFRE_Event_Data->Event_Data.HFRE_Call_Waiting_Notification_Activation_Indication_Data->HFREPortID,
                                                                                                         (HFRE_Event_Data->Event_Data.HFRE_Call_Waiting_Notification_Activation_Indication_Data->Enabled)?"TRUE":"FALSE");

            /* Send an OK terminating response code.                    */
            HFRE_Send_Terminating_Response(BluetoothStackID, HFRE_Event_Data->Event_Data.HFRE_Call_Waiting_Notification_Activation_Indication_Data->HFREPortID, erOK, 0);
            break;
         case etHFRE_Call_Waiting_Notification_Indication:
            /* A Call Waiting Notification Indication was received,     */
            /* display all relevant information.                        */
            printf("HFRE Call Waiting Notification Indication, ID: 0x%04X, Phone Number %s.\n", HFRE_Event_Data->Event_Data.HFRE_Call_Waiting_Notification_Indication_Data->HFREPortID,
                                                                                                  (HFRE_Event_Data->Event_Data.HFRE_Call_Waiting_Notification_Indication_Data->PhoneNumber)?HFRE_Event_Data->Event_Data.HFRE_Call_Waiting_Notification_Indication_Data->PhoneNumber:"<None>");
            break;
         case etHFRE_Call_Line_Identification_Notification_Activation_Indication:
            /* A Call Line Identification Notification Activation was   */
            /* received, display all relevant information.              */
            printf("HFRE Call Line Identification Notification Activation Indication, ID: 0x%04X, Enabled: %s.\n", HFRE_Event_Data->Event_Data.HFRE_Call_Line_Identification_Notification_Activation_Indication_Data->HFREPortID,
                                                                                                                     (HFRE_Event_Data->Event_Data.HFRE_Call_Line_Identification_Notification_Activation_Indication_Data->Enabled)?"TRUE":"FALSE");

            /* Send an OK terminating response code.                    */
            HFRE_Send_Terminating_Response(BluetoothStackID, HFRE_Event_Data->Event_Data.HFRE_Call_Line_Identification_Notification_Activation_Indication_Data->HFREPortID, erOK, 0);
            break;
         case etHFRE_Call_Line_Identification_Notification_Indication:
            /* A Call Line Identification Notification Indication was   */
            /* received, display all relevant information.              */
            printf("HFRE Call Line Identification Notification Indication, ID: 0x%04X, Phone Number %s.\n", HFRE_Event_Data->Event_Data.HFRE_Call_Line_Identification_Notification_Indication_Data->HFREPortID,
                                                                                                              HFRE_Event_Data->Event_Data.HFRE_Call_Line_Identification_Notification_Indication_Data->PhoneNumber);
            break;
         case etHFRE_Disable_Sound_Enhancement_Indication:
            /* A Disable Sound Enhancement Indication was received,     */
            /* display all relevant information.                        */
            printf("HFRE Disable Sound Enhancement Indication, ID: 0x%04X.\n", HFRE_Event_Data->Event_Data.HFRE_Disable_Sound_Enhancement_Indication_Data->HFREPortID);

            /* Send an OK terminating response code.                    */
            HFRE_Send_Terminating_Response(BluetoothStackID, HFRE_Event_Data->Event_Data.HFRE_Disable_Sound_Enhancement_Indication_Data->HFREPortID, erOK, 0);
            break;
         case etHFRE_Dial_Phone_Number_Indication:
            /* A Dial Phone Number Indication was received, display all */
            /* relevant information.                                    */
            printf("HFRE Dial Phone Number Indication, ID: 0x%04X, Phone Number %s.\n", HFRE_Event_Data->Event_Data.HFRE_Dial_Phone_Number_Indication_Data->HFREPortID,
                                                                                          HFRE_Event_Data->Event_Data.HFRE_Dial_Phone_Number_Indication_Data->PhoneNumber);

            /* Send an OK terminating response code.                    */
            HFRE_Send_Terminating_Response(BluetoothStackID, HFRE_Event_Data->Event_Data.HFRE_Dial_Phone_Number_Indication_Data->HFREPortID, erOK, 0);
            break;
         case etHFRE_Dial_Phone_Number_From_Memory_Indication:
            /* A Dial Phone Number from Memory Indication was received, */
            /* display all relevant information.                        */
            printf("HFRE Dial Phone Number From Memory Indication, ID: 0x%04X, Memory Location 0x%04X.\n", HFRE_Event_Data->Event_Data.HFRE_Dial_Phone_Number_From_Memory_Indication_Data->HFREPortID,
                                                                                                             HFRE_Event_Data->Event_Data.HFRE_Dial_Phone_Number_From_Memory_Indication_Data->MemoryLocation);

            /* Send an OK terminating response code.                    */
            HFRE_Send_Terminating_Response(BluetoothStackID, HFRE_Event_Data->Event_Data.HFRE_Dial_Phone_Number_From_Memory_Indication_Data->HFREPortID, erOK, 0);
            break;
         case etHFRE_ReDial_Last_Phone_Number_Indication:
            /* A ReDial Last Phone Number Indication was received,      */
            /* display all relevant information.                        */
            printf("HFRE ReDial Last Phone Number Indication, ID: 0x%04X.\n", HFRE_Event_Data->Event_Data.HFRE_ReDial_Last_Phone_Number_Indication_Data->HFREPortID);

            /* Send an OK terminating response code.                    */
            HFRE_Send_Terminating_Response(BluetoothStackID, HFRE_Event_Data->Event_Data.HFRE_ReDial_Last_Phone_Number_Indication_Data->HFREPortID, erOK, 0);
            break;
         case etHFRE_Ring_Indication:
            /* A Ring Indication was received, display all relevant     */
            /* information.                                             */
            printf("HFRE Ring Indication, ID: 0x%04X.\n", HFRE_Event_Data->Event_Data.HFRE_Ring_Indication_Data->HFREPortID);
            break;
         case etHFRE_Generate_DTMF_Tone_Indication:
            /* A Generate DTMF Tone Indication was received, display all*/
            /* relevant information.                                    */
            printf("HFRE Generate DTMF Tone Indication, ID: 0x%04X, DTMF Code %c.\n", HFRE_Event_Data->Event_Data.HFRE_Generate_DTMF_Tone_Indication_Data->HFREPortID,
                                                                                        HFRE_Event_Data->Event_Data.HFRE_Generate_DTMF_Tone_Indication_Data->DTMFCode);

            /* Send an OK terminating response code.                    */
            HFRE_Send_Terminating_Response(BluetoothStackID, HFRE_Event_Data->Event_Data.HFRE_Generate_DTMF_Tone_Indication_Data->HFREPortID, erOK, 0);
            break;
         case etHFRE_Answer_Call_Indication:
            /* A Answer Call Indication was received, display all       */
            /* relevant information.                                    */
            printf("HFRE Answer Call Indication, ID: 0x%04X.\n", HFRE_Event_Data->Event_Data.HFRE_Answer_Call_Indication_Data->HFREPortID);

            /* Send an OK terminating response code.                    */
            HFRE_Send_Terminating_Response(BluetoothStackID, HFRE_Event_Data->Event_Data.HFRE_Answer_Call_Indication_Data->HFREPortID, erOK, 0);
            break;
         case etHFRE_InBand_Ring_Tone_Setting_Indication:
            /* An InBand Ring Tone Setting Indication was received,     */
            /* display all relevant information.                        */
            printf("HFRE InBand Ring Tone Setting Indication, ID: 0x%04X, Enabled: %s.\n", HFRE_Event_Data->Event_Data.HFRE_InBand_Ring_Tone_Setting_Indication_Data->HFREPortID,
                                                                                             (HFRE_Event_Data->Event_Data.HFRE_InBand_Ring_Tone_Setting_Indication_Data->Enabled)?"TRUE":"FALSE");
            break;
         case etHFRE_Voice_Recognition_Notification_Indication:
            /* A Voice Recognition Notification Indication was received,*/
            /* display all relevant information.                        */
            printf("HFRE Voice Recognition Notification Indication, ID: 0x%04X, Active: %s.\n", HFRE_Event_Data->Event_Data.HFRE_Voice_Recognition_Notification_Indication_Data->HFREPortID,
                                                                                                  (HFRE_Event_Data->Event_Data.HFRE_Voice_Recognition_Notification_Indication_Data->VoiceRecognitionActive)?"TRUE":"FALSE");

            /* Send an OK terminating response code.                    */
            HFRE_Send_Terminating_Response(BluetoothStackID, HFRE_Event_Data->Event_Data.HFRE_Voice_Recognition_Notification_Indication_Data->HFREPortID, erOK, 0);
            break;
         case etHFRE_Speaker_Gain_Indication:
            /* A Speaker Gain Indication was received, display all      */
            /* relevant information.                                    */
            printf("HFRE Speaker Gain Indication, ID: 0x%04X, Speaker Gain 0x%04X.\n", HFRE_Event_Data->Event_Data.HFRE_Speaker_Gain_Indication_Data->HFREPortID,
                                                                                         HFRE_Event_Data->Event_Data.HFRE_Speaker_Gain_Indication_Data->SpeakerGain);
            /* Send an OK terminating response code.                    */
            HFRE_Send_Terminating_Response(BluetoothStackID, HFRE_Event_Data->Event_Data.HFRE_Speaker_Gain_Indication_Data->HFREPortID, erOK, 0);
            break;
         case etHFRE_Microphone_Gain_Indication:
            /* A Microphone Gain Indication was received, display all   */
            /* relevant information.                                    */
            printf("HFRE Microphone Gain Indication, ID: 0x%04X, Microphone Gain 0x%04X.\n", HFRE_Event_Data->Event_Data.HFRE_Microphone_Gain_Indication_Data->HFREPortID,
                                                                                               HFRE_Event_Data->Event_Data.HFRE_Microphone_Gain_Indication_Data->MicrophoneGain);

            /* Send an OK terminating response code.                    */
            HFRE_Send_Terminating_Response(BluetoothStackID, HFRE_Event_Data->Event_Data.HFRE_Microphone_Gain_Indication_Data->HFREPortID, erOK, 0);
            break;
         case etHFRE_Voice_Tag_Request_Indication:
            /* A Voice Tag Request Indication was received, display all */
            /* relevant information.                                    */
            printf("HFRE Voice Tag Request Indication, ID: 0x%04X.\n", HFRE_Event_Data->Event_Data.HFRE_Voice_Tag_Request_Indication_Data->HFREPortID);
            break;
         case etHFRE_Voice_Tag_Request_Confirmation:
            /* A Voice Tag Request Confirmation was received, display   */
            /* all relevant information.                                */
            if(HFRE_Event_Data->Event_Data.HFRE_Voice_Tag_Request_Confirmation_Data->PhoneNumber)
            {
               printf("HFRE Voice Tag Request Confirmation, ID: 0x%04X, Phone Number %s.\n", HFRE_Event_Data->Event_Data.HFRE_Voice_Tag_Request_Confirmation_Data->HFREPortID,
                                                                                            HFRE_Event_Data->Event_Data.HFRE_Voice_Tag_Request_Confirmation_Data->PhoneNumber);
            }
            else
            {
               printf("HFRE Voice Tag Request Confirmation, ID: 0x%04X, Request Rejected.\n", HFRE_Event_Data->Event_Data.HFRE_Voice_Tag_Request_Confirmation_Data->HFREPortID);
            }
            break;
         case etHFRE_Hang_Up_Indication:
            /* A Hang-Up Indication was received, display all relevant  */
            /* information.                                             */
            printf("HFRE Hang Up Indication, ID: 0x%04X.\n", HFRE_Event_Data->Event_Data.HFRE_Hang_Up_Indication_Data->HFREPortID);

            /* Send an OK terminating response code.                    */
            HFRE_Send_Terminating_Response(BluetoothStackID, HFRE_Event_Data->Event_Data.HFRE_Hang_Up_Indication_Data->HFREPortID, erOK, 0);
            break;
         case etHFRE_Close_Port_Indication:
            /* A Close Port Indication was received, display all        */
            /* relevant information.                                    */
            printf("HFRE Close Port Indication, ID: 0x%04X, Status: 0x%04X.\n", HFRE_Event_Data->Event_Data.HFRE_Close_Port_Indication_Data->HFREPortID,
                                                                                  HFRE_Event_Data->Event_Data.HFRE_Close_Port_Indication_Data->PortCloseStatus);

            /* Check to see if the current Port is a Client.            */
            if(!ServerConnected)
            {
               /* The Client Port was closed so invalidate the ID.      */
               CurrentClientPortID = 0;
            }
            else
               ServerConnected = FALSE;

            /* Flag that an Audio Connection is no longer present.      */
            AudioConnected = FALSE;
            break;
         case etHFRE_Audio_Connection_Indication:
            /* An Audio Connection Indication was received, display all */
            /* relevant information.                                    */
            printf("HFRE Audio Connection Indication, ID: 0x%04X, Status: 0x%04X.\n", HFRE_Event_Data->Event_Data.HFRE_Audio_Connection_Indication_Data->HFREPortID, HFRE_Event_Data->Event_Data.HFRE_Audio_Connection_Indication_Data->AudioConnectionOpenStatus);

            /* Flag that we would like the Audio Data Indication message*/
            /* to be displayed on the first reception of Audio Data.    */
            AudioDataCount     = 1;
            AudioDataToneIndex = 0;

            /* Flag that an Audio Connection is present.                */
            AudioConnected = TRUE;
            break;
         case etHFRE_Audio_Disconnection_Indication:
            /* An Audio Disconnection Indication was received, display  */
            /* all relevant information.                                */
            printf("HFRE Audio Disconnection Indication, ID: 0x%04X.\n", HFRE_Event_Data->Event_Data.HFRE_Audio_Disconnection_Indication_Data->HFREPortID);

            /* Flag that an Audio Connection is no longer present.      */
            AudioConnected = FALSE;
            break;
         case etHFRE_Audio_Data_Indication:
            /* An Audio Data Indication was received, display the       */
            /* relevant information.                                    */

            /* To avoid flooding the user with Audio Data Messages, only*/
            /* print out a message every second.  The number below is   */
            /* loosely based on the fact that the Bluetooth             */
            /* Specification mentions 3ms per SCO Packet, which is what */
            /* most chips appear to use.  So we arrive at the 'magical' */
            /* number below.                                            */
            AudioDataCount--;
            if(!AudioDataCount)
            {
               printf("\n");
               printf("HFRE Audio Data Indication, ID: 0x%04X, Length 0x%04X: %02X%02X%02X%02X.\n", HFRE_Event_Data->Event_Data.HFRE_Audio_Data_Indication_Data->HFREPortID, HFRE_Event_Data->Event_Data.HFRE_Audio_Data_Indication_Data->AudioDataLength, HFRE_Event_Data->Event_Data.HFRE_Audio_Data_Indication_Data->AudioData[0], HFRE_Event_Data->Event_Data.HFRE_Audio_Data_Indication_Data->AudioData[1], HFRE_Event_Data->Event_Data.HFRE_Audio_Data_Indication_Data->AudioData[2], HFRE_Event_Data->Event_Data.HFRE_Audio_Data_Indication_Data->AudioData[3]);

               if(IsHandsFree)
                  printf("Hands Free>");
               else
                  printf("Audio Gateway>");

               fflush(stdout);

               /* Reset the Audio Data Count so we only print out       */
               /* received data a second later.                         */
               AudioDataCount = 333;
            }

            /* The following code demonstrates two methods of testing   */
            /* SCO Audio Data.  The two methods are a simple Loopback   */
            /* and injecting a 1KHz tone into the audio data.           */
            /* * NOTE * For non-loop back audio data, simply send 3ms   */
            /*          worth of data (or the same amount as was        */
            /*          received) from the real data to send.           */
            /* * NOTE * The size of the data packets will always be     */
            /*          constant, i.e.  it will not change.             */
            if(AudioTestMode == AUDIO_DATA_TEST_LOOPBACK)
            {
               /* Simply loop the data back.                            */
               HFRE_Send_Audio_Data(BluetoothStackID, HFRE_Event_Data->Event_Data.HFRE_Audio_Data_Indication_Data->HFREPortID, HFRE_Event_Data->Event_Data.HFRE_Audio_Data_Indication_Data->AudioDataLength, HFRE_Event_Data->Event_Data.HFRE_Audio_Data_Indication_Data->AudioData);
            }
            else
            {
               if(AudioTestMode == AUDIO_DATA_TEST_1KHZ_TONE)
               {
                  /* Build the Audio Data Test 1KHz tone into our local */
                  /* Audio Buffer.                                      */
                  for(Index=0;Index<(unsigned int)HFRE_Event_Data->Event_Data.HFRE_Audio_Data_Indication_Data->AudioDataLength;Index++)
                  {
                     if(AudioData8BitFormat)
                     {
                        AudioData[Index]    = AudioDataTone_1KHz_8Bit[AudioDataToneIndex++];

                        AudioDataToneIndex %= sizeof(AudioDataTone_1KHz_8Bit);
                     }
                     else
                     {
                        if(SelectedCodec == 1)
                        {
                           AudioData[Index]    = AudioDataTone_1KHz_16Bit_8KHz[AudioDataToneIndex++];

                           AudioDataToneIndex %= sizeof(AudioDataTone_1KHz_16Bit_8KHz);
                        }
                        else
                        {
                           AudioData[Index]    = AudioDataTone_1KHz_16Bit_16KHz[AudioDataToneIndex++];

                           AudioDataToneIndex %= sizeof(AudioDataTone_1KHz_16Bit_16KHz);
                        }
                     }
                  }

                  /* Data has been formatted, go ahead and send the     */
                  /* data.                                              */
                  ret_val = HFRE_Send_Audio_Data(BluetoothStackID, HFRE_Event_Data->Event_Data.HFRE_Audio_Data_Indication_Data->HFREPortID, HFRE_Event_Data->Event_Data.HFRE_Audio_Data_Indication_Data->AudioDataLength, AudioData);
                  if(ret_val)
                     printf("HFRE_Send_Audio_Data failed with %d\n", ret_val);
               }
            }
            break;
         case etHFRE_Command_Result:
            /* An Command Confirmation was received, display the        */
            /* relevant information.                                    */
            printf("HFRE Command Result, ID: 0x%04X, Type %d Code %d.\n", HFRE_Event_Data->Event_Data.HFRE_Command_Result_Data->HFREPortID, HFRE_Event_Data->Event_Data.HFRE_Command_Result_Data->ResultType, HFRE_Event_Data->Event_Data.HFRE_Command_Result_Data->ResultValue);
            break;
         case etHFRE_Current_Calls_List_Indication:
            printf("etHFRE Current Calls List Indication, ID: 0x%04X.\n", HFRE_Event_Data->Event_Data.HFRE_Current_Calls_List_Indication_Data->HFREPortID);
            break;
         case etHFRE_Current_Calls_List_Confirmation:
            /* Verify that all expected fields are in the CLCC response */
            /* entry.                                                   */
            if(HFRE_Event_Data->Event_Data_Size >= HFRE_CURRENT_CALLS_LIST_CONFIRMATION_DATA_SIZE)
            {
               printf("HFRE Current_Calls_List_Confirmation, ID: 0x%04X.\n", HFRE_Event_Data->Event_Data.HFRE_Current_Calls_List_Confirmation_Data->HFREPortID);

               printf("+CLCC: [%d] Dir: %d Status: %d Mode: %d Multi: %c Format: %d Num: %s Name: %s\n", HFRE_Event_Data->Event_Data.HFRE_Current_Calls_List_Confirmation_Data->HFRECurrentCallListEntry.Index,
                                                                                                           HFRE_Event_Data->Event_Data.HFRE_Current_Calls_List_Confirmation_Data->HFRECurrentCallListEntry.CallDirection,
                                                                                                           HFRE_Event_Data->Event_Data.HFRE_Current_Calls_List_Confirmation_Data->HFRECurrentCallListEntry.CallStatus,
                                                                                                           HFRE_Event_Data->Event_Data.HFRE_Current_Calls_List_Confirmation_Data->HFRECurrentCallListEntry.CallMode,
                                                                                                           ((HFRE_Event_Data->Event_Data.HFRE_Current_Calls_List_Confirmation_Data->HFRECurrentCallListEntry.Multiparty)?('Y'):('N')),
                                                                                                           HFRE_Event_Data->Event_Data.HFRE_Current_Calls_List_Confirmation_Data->HFRECurrentCallListEntry.NumberFormat,
                                                                                                           HFRE_Event_Data->Event_Data.HFRE_Current_Calls_List_Confirmation_Data->HFRECurrentCallListEntry.PhoneNumber?HFRE_Event_Data->Event_Data.HFRE_Current_Calls_List_Confirmation_Data->HFRECurrentCallListEntry.PhoneNumber:"",
                                                                                                           HFRE_Event_Data->Event_Data.HFRE_Current_Calls_List_Confirmation_Data->HFRECurrentCallListEntry.PhonebookName?HFRE_Event_Data->Event_Data.HFRE_Current_Calls_List_Confirmation_Data->HFRECurrentCallListEntry.PhonebookName:"");
            }
            else
            {
               /* The Phonebook Name entry is not available in older    */
               /* Hands Free implementations, so check the length       */
               /* excluding the field for backwards compatibility       */
               if(HFRE_Event_Data->Event_Data_Size >= BTPS_STRUCTURE_OFFSET(HFRE_Current_Calls_List_Confirmation_Data_t, HFRECurrentCallListEntry.PhonebookName))
               {
                  printf("HFRE Current_Calls_List_Confirmation, ID: 0x%04X.\n", HFRE_Event_Data->Event_Data.HFRE_Current_Calls_List_Confirmation_Data->HFREPortID);

                  printf("+CLCC: [%d] Dir: %d Status: %d Mode: %d Multi: %c Format: %d Num: %s\n", HFRE_Event_Data->Event_Data.HFRE_Current_Calls_List_Confirmation_Data->HFRECurrentCallListEntry.Index,
                                                                                                     HFRE_Event_Data->Event_Data.HFRE_Current_Calls_List_Confirmation_Data->HFRECurrentCallListEntry.CallDirection,
                                                                                                     HFRE_Event_Data->Event_Data.HFRE_Current_Calls_List_Confirmation_Data->HFRECurrentCallListEntry.CallStatus,
                                                                                                     HFRE_Event_Data->Event_Data.HFRE_Current_Calls_List_Confirmation_Data->HFRECurrentCallListEntry.CallMode,
                                                                                                     ((HFRE_Event_Data->Event_Data.HFRE_Current_Calls_List_Confirmation_Data->HFRECurrentCallListEntry.Multiparty)?('Y'):('N')),
                                                                                                     HFRE_Event_Data->Event_Data.HFRE_Current_Calls_List_Confirmation_Data->HFRECurrentCallListEntry.NumberFormat,
                                                                                                     HFRE_Event_Data->Event_Data.HFRE_Current_Calls_List_Confirmation_Data->HFRECurrentCallListEntry.PhoneNumber?HFRE_Event_Data->Event_Data.HFRE_Current_Calls_List_Confirmation_Data->HFRECurrentCallListEntry.PhoneNumber:"");
               }
            }
            break;
         case etHFRE_Network_Operator_Selection_Format_Indication:
            printf("HFRE Network Operator Selection Format Indication, ID: 0x%04X Format: %d.\n", HFRE_Event_Data->Event_Data.HFRE_Network_Operator_Selection_Format_Indication_Data->HFREPortID, HFRE_Event_Data->Event_Data.HFRE_Network_Operator_Selection_Format_Indication_Data->Format);

            /* Send an OK terminating response code.                    */
            HFRE_Send_Terminating_Response(BluetoothStackID, HFRE_Event_Data->Event_Data.HFRE_Network_Operator_Selection_Format_Indication_Data->HFREPortID, erOK, 0);
            break;
         case etHFRE_Network_Operator_Selection_Indication:
            printf("HFRE Network Operator Selection Indication, ID: 0x%04X.\n", HFRE_Event_Data->Event_Data.HFRE_Network_Operator_Selection_Indication_Data->HFREPortID);
            break;
         case etHFRE_Network_Operator_Selection_Confirmation:
            printf("HFRE Network Operator Selection Confirmation, ID: 0x%04X.\n", HFRE_Event_Data->Event_Data.HFRE_Network_Operator_Selection_Confirmation_Data->HFREPortID);

            printf("+COPS: Mode: %d Network: %s\n", HFRE_Event_Data->Event_Data.HFRE_Network_Operator_Selection_Confirmation_Data->NetworkMode,
                                                      (HFRE_Event_Data->Event_Data.HFRE_Network_Operator_Selection_Confirmation_Data->NetworkOperator)?HFRE_Event_Data->Event_Data.HFRE_Network_Operator_Selection_Confirmation_Data->NetworkOperator:"<None>");
            break;
         case etHFRE_Extended_Error_Result_Activation_Indication:
            printf("HFRE Extended Error Result Activation Indication, ID: 0x%04X. Enabled: %c\n", HFRE_Event_Data->Event_Data.HFRE_Extended_Error_Result_Activation_Indication_Data->HFREPortID, ((HFRE_Event_Data->Event_Data.HFRE_Extended_Error_Result_Activation_Indication_Data->Enabled==TRUE)?('Y'):('N')));

            /* Send an OK terminating response code.                    */
            HFRE_Send_Terminating_Response(BluetoothStackID, HFRE_Event_Data->Event_Data.HFRE_Extended_Error_Result_Activation_Indication_Data->HFREPortID, erOK, 0);
            break;
         case etHFRE_Subscriber_Number_Information_Indication:
            printf("HFRE Subscriber Number Information Indication, ID: 0x%04X.\n", HFRE_Event_Data->Event_Data.HFRE_Subscriber_Number_Information_Indication_Data->HFREPortID);
            break;
         case etHFRE_Subscriber_Number_Information_Confirmation:
            printf("HFRE Subscriber Number Information Confirmation, ID: 0x%04X.\n", HFRE_Event_Data->Event_Data.HFRE_Subscriber_Number_Information_Confirmation_Data->HFREPortID);

            printf("+CNUM: SvcType: %d Format: %d Num: %s\n", HFRE_Event_Data->Event_Data.HFRE_Subscriber_Number_Information_Confirmation_Data->ServiceType,
                                                                HFRE_Event_Data->Event_Data.HFRE_Subscriber_Number_Information_Confirmation_Data->NumberFormat,
                                                                HFRE_Event_Data->Event_Data.HFRE_Subscriber_Number_Information_Confirmation_Data->PhoneNumber);
            break;
         case etHFRE_Response_Hold_Status_Indication:
            printf("HFRE Response Hold Status Indication, ID: 0x%04X.\n", HFRE_Event_Data->Event_Data.HFRE_Response_Hold_Status_Indication_Data->HFREPortID);

            /* Send an OK terminating response code.                    */
            HFRE_Send_Terminating_Response(BluetoothStackID, HFRE_Event_Data->Event_Data.HFRE_Response_Hold_Status_Indication_Data->HFREPortID, erOK, 0);
            break;
         case etHFRE_Response_Hold_Status_Confirmation:
            printf("HFRE Response Hold Status Confirmation, ID: 0x%04X CallState: %d.\n", HFRE_Event_Data->Event_Data.HFRE_Response_Hold_Status_Confirmation_Data->HFREPortID, HFRE_Event_Data->Event_Data.HFRE_Response_Hold_Status_Confirmation_Data->CallState);
            break;
         case etHFRE_Incoming_Call_State_Indication:
            printf("HFRE Incoming Call State Indication, ID: 0x%04X CallState: %d.\n", HFRE_Event_Data->Event_Data.HFRE_Incoming_Call_State_Indication_Data->HFREPortID, HFRE_Event_Data->Event_Data.HFRE_Incoming_Call_State_Indication_Data->CallState);
            break;
         case etHFRE_Incoming_Call_State_Confirmation:
            printf("HFRE Incoming Call State Confirmation, ID: 0x%04X CallState: %d.\n", HFRE_Event_Data->Event_Data.HFRE_Incoming_Call_State_Confirmation_Data->HFREPortID, HFRE_Event_Data->Event_Data.HFRE_Incoming_Call_State_Confirmation_Data->CallState);
            break;
         case etHFRE_Open_Port_Request_Indication:
            printf("etHFRE_Open_Port_Request_Indication... Accepting\n");
            HFRE_Open_Port_Request_Response(BluetoothStackID, HFRE_Event_Data->Event_Data.HFRE_Open_Port_Request_Indication_Data->HFREPortID, TRUE);
            break;
         case etHFRE_Control_Indicator_Request_Indication:
            printf("etHFRE_Control_Indicator_Request_Indication\n");
            break;
         case etHFRE_Available_Codec_List_Indication:
            printf("etHFRE_Available_Codec_List_Indication\n");
            printf("  %d Codec(s) supported\n", HFRE_Event_Data->Event_Data.HFRE_Available_Codec_List_Indication_Data->NumSupportedCodecs);
            Index = 0;
            while(Index < HFRE_Event_Data->Event_Data.HFRE_Available_Codec_List_Indication_Data->NumSupportedCodecs)
            {
               printf("   %d. Codec ID: %d\n", Index+1, (HFRE_Event_Data->Event_Data.HFRE_Available_Codec_List_Indication_Data->AvailableCodecList)[Index]);
               Index++;
            }
            break;
         case etHFRE_Codec_Select_Request_Indication:
            printf("etHFRE_Codec_Select_Request_Indication\n");
            printf("   Accepting Codec ID: %d\n", HFRE_Event_Data->Event_Data.HFRE_Codec_Select_Indication_Data->CodecID);
            SelectedCodec = HFRE_Event_Data->Event_Data.HFRE_Codec_Select_Indication_Data->CodecID;
            HFRE_Send_Select_Codec(BluetoothStackID, HFRE_Event_Data->Event_Data.HFRE_Codec_Select_Indication_Data->HFREPortID, (char)SelectedCodec);
            break;
         case etHFRE_Codec_Select_Confirmation:
            printf("etHFRE_Codec_Select_Confirmation\n");
            printf("   Selected Codec ID: %d\n", HFRE_Event_Data->Event_Data.HFRE_Codec_Select_Confirmation_Data->AcceptedCodec);
            if(HFRE_Event_Data->Event_Data.HFRE_Codec_Select_Confirmation_Data->AcceptedCodec == RequestedCodec)
            {
               SelectedCodec = RequestedCodec;
               HFRE_Send_Terminating_Response(BluetoothStackID, HFRE_Event_Data->Event_Data.HFRE_Codec_Select_Confirmation_Data->HFREPortID, erOK, 0);
            }
            else
               HFRE_Send_Terminating_Response(BluetoothStackID, HFRE_Event_Data->Event_Data.HFRE_Codec_Select_Confirmation_Data->HFREPortID, erError, 0);

            RequestedCodec = 0;
            break;
         case etHFRE_Codec_Connection_Setup_Indication:
            printf("etHFRE_Codec_Connection_Setup_Indication\n");

            /* Send an OK terminating response code.                    */
            HFRE_Send_Terminating_Response(BluetoothStackID, HFRE_Event_Data->Event_Data.HFRE_Available_Codec_List_Indication_Data->HFREPortID, erOK, 0);
            break;
         default:
            /* An unknown/unexpected HFRE event was received.           */
            printf("\nUnknown HFRE Event Received: %d.\n", HFRE_Event_Data->Event_Data_Type);
            break;
      }
   }
   else
   {
      /* There was an error with one or more of the input parameters.   */
      printf("\nHFRE callback data: Event_Data = NULL.\n");
   }

   /* Output an Input Shell-type prompt.                                */
   if((HFRE_Event_Data == NULL) || (HFRE_Event_Data->Event_Data_Type != etHFRE_Audio_Data_Indication))
   {
      if(IsHandsFree)
         printf("Hands Free>");
      else
         printf("Audio Gateway>");

      /* Make sure the output is displayed to the user.                 */
      fflush(stdout);
   }
}

   /* Main Program Entry Point.                                         */
int main(int argc, char* argv[])
{
   int                      CommPortNumber;
   int                      BaudRate;
   char                    *endptr = NULL;
   HCI_COMM_Protocol_t      Protocol = cpUART_RTS_CTS;
   HCI_DriverInformation_t  HCI_DriverInformation;
   HCI_DriverInformation_t *HCI_DriverInformationPtr;

   /* First, check to see if the minimum number of parameters were      */
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
            /* Information Structure to use USB has the HCI Transport.  */
            HCI_DRIVER_SET_USB_INFORMATION(&HCI_DriverInformation);

            HCI_DriverInformationPtr = &HCI_DriverInformation;
            break;
         case BCSP_PARAMETER_VALUE:
            Protocol = cpBCSP;
            /* Let case fall through.                                   */
         case UART_PARAMETER_VALUE:
            /* The Transport selected was UART, check to see if the     */
            /* number of parameters entered on the command line is      */
            /* correct.                                                 */
            if(argc >= NUM_EXPECTED_PARAMETERS_UART)
            {
               /* The correct number of parameters were entered, convert*/
               /* the command line parameters to the proper format.     */
               BaudRate       = strtol(argv[3], &endptr, 10);

               /* Either a port number or a device file can be used.    */
               if((argv[2][0] >= '0') && (argv[2][0] <= '9'))
               {
                  CommPortNumber = strtol(argv[2], &endptr, 10);
                  HCI_DRIVER_SET_COMM_INFORMATION(&HCI_DriverInformation, CommPortNumber, BaudRate, Protocol);
               }
               else
               {
                  HCI_DRIVER_SET_EXTENDED_COMM_INFORMATION_DEVICE_NAME(&HCI_DriverInformation, -1, BaudRate, Protocol, 0, argv[2]);
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

      /* Check to see if the HCI_Driver Information Structure was       */
      /* successfully setup.                                            */
      if(HCI_DriverInformationPtr)
      {
         /* Try to Open the stack and check if it was successful.       */
         if(!OpenStack(HCI_DriverInformationPtr))
         {
            /* Before we continue, we need to wait to see if we are     */
            /* going to function as a Audio Gateway or a Hands-Free     */
            /* Unit.                                                    */
            UserInterface_AudioGatewayHandsFree();

            if(AGHFDetected)
            {
               /* Now that we have determined if we are an Audio Gateway*/
               /* or a Hands-Free Unit we can now display the menu      */
               /* choices to the user and wait for the commands.        */
               UserInterface_Main();

               CloseHFRE();
            }

            /* Close the Bluetooth Stack.                               */
            CloseStack();
         }
         else
         {
            /* There was an error while attempting to open the Stack.   */
            printf("Unable to open the stack.\n");
         }
      }
      else
      {
         /* One or more of the Command Line parameters appear to be     */
         /* invalid.                                                    */
         printf("Parameter Error (Expecting = [USB = 0, UART = 1, BCSP = 2 Flag] [IF !USB [Comm Port or Device File] [Baud Rate]])\n");
      }
   }
   else
   {
      /* An invalid number of parameters was entered on the Command     */
      /* Line.                                                          */
      printf("Parameter Error (Expecting = [USB = 0, UART = 1, BCSP = 2 Flag] [IF !USB [Comm Port or Device File] [Baud Rate]])\n");
   }

   return 0;
}

