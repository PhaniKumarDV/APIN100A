/*****< linuxbip.c >***********************************************************/
/*      Copyright 2005 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  LINUXBIP - Basic Imaging Profile Stack Application for Linux Main         */
/*             Application File.                                              */
/*                                                                            */
/*  Author:  Rory Sledge                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/10/05  R. Sledge       Initial creation.                              */
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
#include <errno.h>

#include "LinuxBIP.h"      /* Main Application Prototypes and Constants.      */

#include "SS1BTPS.h"       /* Includes for the SS1 Bluetooth Protocol Stack.  */
#include "SS1BTBIP.h"      /* Includes/Constants for the BIP profile.         */
#include "SS1BTDBG.h"      /* Includes/Constants for Bluetooth Debugging.     */

#define LOCAL_NAME_ROOT                      "LinuxBIP"  /* Root of the local */
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

#define MAX_NUM_OF_PARAMETERS                       (3)  /* Denotes the max   */
                                                         /* number of         */
                                                         /* parameters a      */
                                                         /* command can have. */

#define MAX_COMMAND_LENGTH                        (128)  /* Denotes the max   */
                                                         /* buffer size used  */
                                                         /* for user commands */
                                                         /* inputed via the   */
                                                         /* UserInterface.    */

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
                                                         /* error occured due */
                                                         /* to attemped       */
                                                         /* execution of a    */
                                                         /* command without a */
                                                         /* valid Bluetooth   */
                                                         /* Stack ID.         */

#define APPLICATION_ERROR                          (-1)  /* Denotes the       */
                                                         /* Application       */
                                                         /* Function Error    */
                                                         /* Value.            */

#define INDENT_LENGTH                               (3)  /* Denotes the number*/
                                                         /* of character      */
                                                         /* spaces to be used */
                                                         /* for indenting when*/
                                                         /* displaying SDP    */
                                                         /* Data Elements.    */

   /* The following constants represent the default log file names that */
   /* are used if no Log file name is specified when enabling debug.    */
#define DEFAULT_DEBUG_LOG_FILE_NAME  "LinuxBIP_ASC.log"
#define DEFAULT_DEBUG_FTS_FILE_NAME  "LinuxBIP_FTS.log"

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

   /* Denotes some common characters used in processing and building XML*/
   /* objects.                                                          */
#define _NULL_                                          ((char)0x00)
#define _TAB_                                           ((char)0x09)
#define _LESS_THAN_                                              '<'
#define _GREATER_THAN_                                           '>'
#define _SPACE_                                                  ' '
#define _QUOTE_                                                  '"'
#define _EQUAL_                                                  '='
#define _FORWARD_SLASH_                                          '/'
#define _SPACE_                                                  ' '
#define _CARRIAGE_RETURN_                                       '\r'
#define _LINE_FEED_                                             '\n'
#define _WHITE_SPACE_CHARACTER_                         ((char)0xFE)
#define _NON_SPACE_                                     ((char)0xFF)

   /* The following MACRO is a utility MACRO that exists to aid in the  */
   /* testing of a character to see if it is a White Space Character.   */
   /* White Space characters are considered to be the same as the       */
   /* White Space characters that the C Run Time Library isspace()      */
   /* MACRO/function returns.  This MACRO returns a BOOLEAN TRUE if     */
   /* the input argument is either (or FALSE otherwise):                */
   /*   - TAB                   0x09                                    */
   /*   - New Line              0x0A                                    */
   /*   - Vertical TAB          0x0B                                    */
   /*   - Form Feed             0x0C                                    */
   /*   - Carriage Return       0x0D                                    */
   /*   - Space                 0x20                                    */
#define IS_WHITE_SPACE_CHARACTER(_x)             (((_x) == _SPACE_) || (((_x) >= _TAB_) && ((_x) <= _CARRIAGE_RETURN_)))

   /* The following denote the maximum and minimum file/directory       */
   /* numbers that may be used to create Image Handles.                 */
   /* ** NOTE ** These numbers are defined by the JEDIA Standard Design */
   /*            Rule for Camera File Systems.                          */
#define DIRECTORY_NUMBER_MINIMUM                               (1)
#define DIRECTORY_NUMBER_MAXIMUM                               (999)
#define FILE_NUMBER_MINIMUM                                    (1)
#define FILE_NUMBER_MAXIMUM                                    (9999)

   /* The following denote the absolute width and height for a thumbnail*/
   /* image.                                                            */
   /* ** NOTE ** These values are defined by the BIP 1.0 specification  */
   /*            for thumbnail image requirements.                      */
#define THUMBNAIL_IMAGE_WIDTH                                  (160)
#define THUMBNAIL_IMAGE_HEIGHT                                 (120)

   /* The following are the return values from the GetFileData() and    */
   /* PutFileData() functions.                                          */
#define FILE_DATA_FILE_IO_ERROR                                (-1)
#define FILE_DATA_FILE_IO_SUCCESS                               (0)
#define FILE_DATA_END_OF_FILE                                   (1)

   /* The following buffer contains the capabilities object for the     */
   /* server.                                                           */
static const char ImagingCapabilitiesObject[] = "<imaging-capabilities version=\"1.0\">"
                                                "<image-formats encoding=\"JPEG\" pixel=\"0*0-65535*65535\"/>"
                                                "</imaging-capabilities>";

static const char ImageHandlesDescriptorObject[] = "<image-handles-descriptor version=\"1.0\">"
                                                   "<filtering-parameters encoding=\"JPEG\"/>"
                                                   "</image-handles-descriptor>";

static const char EmptyImageDescriptorObject[] = "<image-descriptor version=\"1.0\">"
                                                 "</image-descriptor>";

static const char ImageDescriptorObjectHeader[] = "<image-descriptor version=\"1.0\">";
static const char ImageDescriptorObjectFooter[] = "</image-descriptor>";
static const char ImageDescriptorObjectImageElement[] = "image";
static const char ImageDescriptorObjectEncodingAttribute[] = "encoding";
static const char ImageDescriptorObjectPixelAttribute[] = "pixel";
static const char ImageDescriptorObjectSizeAttribute[] = "size";

static const char ImagesListingObjectHeader[] = "<images-listing version=\"1.0\">";
static const char ImagesListingObjectFooter[] = "</images-listing>";
static const char ImagesListingObjectImageElement[] = "image";
static const char ImagesListingObjectHandleAttribute[] = "handle";
static const char ImagesListingObjectCreatedAttribute[] = "created";
static const char ImagesListingObjectModifiedAttribute[] = "modified";

static const char ImagesListingObjectImagesListingElement[] = "images-listing";
static const char ImagesListingObjectVersionAttribute[] = "version";

static const char ImagePropertiesObjectImagePropertiesElement[] = "image-properties";
static const char ImagePropertiesObjectVersionAttribute[] = "version";
static const char ImagePropertiesObjectHandleAttribute[] = "handle";
static const char ImagePropertiesObjectNativeElement[] = "native";
static const char ImagePropertiesObjectEncodingAttribute[] = "encoding";
static const char ImagePropertiesObjectPixelAttribute[] = "pixel";
static const char ImagePropertiesObjectSizeAttribute[] = "size";
static const char ImagePropertiesObjectFooter[] = "</image-properties>";

   /* The following enumerated types represent the various states that  */
   /* the application processes.                                        */
typedef enum
{
   coNone,
   coGetCapabilities,
   coPutImage,
   coPutLinkedThumbnail,
   coPutLinkedAttachment,
   coGetImagesListFirstPhase,
   coGetImagesList,
   coGetImageProperties,
   coGetImageFirstPhase,
   coGetImage,
   coGetLinkedThumbnail,
   coGetLinkedAttachment,
   coDeleteImage,
   coGetPartialImage,
   coStartPrint,
   coGetStatus,
   coStartArchive,
   coGetMonitoringImage,
   coRemoteDisplay,
   coAbort
} CurrentOperation_t;

typedef enum
{
   pilLookingHeader,
   pilLookingBody,
   pilLookingFooter
} ProcessImagesListingState_t;

   /* The following structure represents the Time and Date Structure    */
   /* used to represent Time/Date information in this program.          */
typedef struct _tagTimeDate_t
{
   Word_t    Year;
   Word_t    Month;
   Word_t    Day;
   Word_t    Hour;
   Word_t    Minute;
   Word_t    Second;
   Boolean_t UTC_Time;
} TimeDate_t;

   /* The following structure represents the structure that is used to  */
   /* hold all information pertaining to an individual Image.           */
typedef struct _tagImageListEntry_t
{
   char       ImageHandle[BIP_IMAGE_HANDLE_LENGTH+1];
   TimeDate_t CreatedTime;
   TimeDate_t ModifiedTime;
} ImageListEntry_t;

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the         */
   /* compiler as part of standard C/C++).                              */
static unsigned int        BluetoothStackID;        /* Variable which holds the Handle */
                                                    /* of the opened Bluetooth Protocol*/
                                                    /* Stack.                          */

static unsigned int        DebugID;                 /* Variable which holds the ID of  */
                                                    /* the currently enabled debugging */
                                                    /* session.                        */

static int                 IsClient;                /* Variable used to indicate if the*/
                                                    /* program is to be run in Client  */
                                                    /* Mode or Server Mode.            */

static unsigned int        BIPID;                   /* Variable used to hold the BIP   */
                                                    /* Client or Server Profile ID.    */

static DWord_t             ServiceRecordHandle;     /* Variable used to hold the SDP   */
                                                    /* Record Handle of a BPP Server.  */

static char                RootDirectory[1024];     /* Current Root directory used     */
                                                    /* to hold all Images.             */

static CurrentOperation_t  CurrentOperation;        /* Current BIP Operation that is   */
                                                    /* in progress.                    */

static BIP_Imaging_Service_Server_Port_Target_t ServiceType; /* Variable used to hold  */
                                                    /* the current Client Service Type */
                                                    /* for the current Client          */
                                                    /* Connection.                     */

static unsigned int        CurrentObjectIndex;      /* Current BIP Object State        */
static unsigned int        CurrentObjectLength;     /* Information used by BIP Clients */
static unsigned char      *CurrentObjectBuffer;     /* and Servers.                    */
static Boolean_t           CurrentObjectBufferAllocated;

static char                CurrentFile[1024+1];     /* Current BIP File State          */
static DWord_t             CurrentFileIndex;        /* Information used by BIP Clients */
static DWord_t             CurrentFileSize;         /* and Servers.                    */
static Boolean_t           FirstPhase;
static DWord_t             CurrentFileBufferIndex;
static Byte_t              CurrentFileBuffer[64*1024];
static char                CurrentImageHandle[BIP_IMAGE_HANDLE_LENGTH+1];

static Boolean_t           Connected;               /* State variable which flags      */
                                                    /* whether or not we are currently */
                                                    /* connected.                      */

static BD_ADDR_t           InquiryResultList[MAX_INQUIRY_RESULTS];  /* Variable which  */
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

static ImageListEntry_t   *ImageList;               /* Variable which holds the        */
                                                    /* currently fetched BIP Image List*/
                                                    /* (when operating in Client Mode).*/

static unsigned int        NumberImageListEntries;  /* Variable which holds the        */
                                                    /* number of Items that are        */
                                                    /* currently present in the BIP    */
                                                    /* Image List (when operating in   */
                                                    /* BIP Client Mode).               */

static int                 ImageDeleteIndex;        /* Variable which holds the        */
                                                    /* current Index of the Image Item */
                                                    /* that is to be delted from the   */
                                                    /* Image List (when operating in   */
                                                    /* BIP Client Mode).               */

static unsigned int        NumberCommands;          /* Variable which is used to hold  */
                                                    /* the number of Commands that are */
                                                    /* supported by this application.  */
                                                    /* Commands are added individually.*/

static CommandTable_t      CommandTable[MAX_SUPPORTED_COMMANDS]; /* Variable which is  */
                                                    /* used to hold the actual Commands*/
                                                    /* that are supported by this      */
                                                    /* application.                    */

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
static void DisplayMenu(void);

static void UserInterface_Client(void);
static void UserInterface_Server(void);

static unsigned int StringToUnsignedInteger(char *StringInteger);
static char *StringParser(char *String);
static int CommandParser(UserCommand_t *TempCommand, char *UserInput);
static int CommandInterpreter(UserCommand_t *TempCommand);
static int AddCommand(char *CommandName, CommandFunction_t CommandFunction);
static CommandFunction_t FindCommand(char *Command);
static void ClearCommands(void);

static void BD_ADDRToStr(BD_ADDR_t Board_Address, char *BoardStr);
static void WriteEIRInformation(char *LocalName);
static void DisplayObject(unsigned int ObjectLength, unsigned char *ObjectData);

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
static int OpenRemoteServer(ParameterList_t *TempParam);
static int CloseServer(ParameterList_t *TempParam);
static int CloseConnection(ParameterList_t *TempParam);

static int DisplayImageList(ParameterList_t *TempParam);
static int Abort(ParameterList_t *TempParam);
static int GetCapabilities(ParameterList_t *TempParam);
static int PutImage(ParameterList_t *TempParam);
static int PutLinkedThumbnail(ParameterList_t *TempParam);
static int GetImagesList(ParameterList_t *TempParam);
static int GetImageProperties(ParameterList_t *TempParam);
static int GetImage(ParameterList_t *TempParam);
static int GetLinkedThumbnail(ParameterList_t *TempParam);
static int DeleteImage(ParameterList_t *TempParam);

static void ParseFileName(char *PathFileName, char *FileName);
static int ExtractInt(char *CharPtr, int NumberOfDigits);
static int ExtractTimeDate(char *TDStringPtr, TimeDate_t *TimeDatePtr);
static int FindToken(char Token, unsigned int Offset, unsigned int DataLength, unsigned char *DataBuffer);

static int GetFileImageProperties(char *FileName, Word_t *ImageWidth, Word_t *ImageHeight, DWord_t *ImageFileSize);
static int GetNextImageHandle(char *ImageHandle);
static int GetFileData(void);
static int PutFileData(unsigned int DataLength, unsigned char *DataBuffer);
static Boolean_t TestImageHandle(char *ImageHandle);

static int InitializeImageList(void);
static void CleanupImageList(void);

static int BuildImageDescriptorObject(char *FileName, unsigned int *DataLength, unsigned char *DataBuffer);
static int BuildImagesListingObject(unsigned int ImageListLength, ImageListEntry_t *ImageListPtr, DWord_t Offset, DWord_t NumberOfHandles, unsigned int *DataLength, unsigned char *DataBuffer);
static int BuildImagePropertiesObject(char *FileName, char *ImageHandle, unsigned int *DataLength, unsigned char *DataBuffer);

static int ProcessImagesListingObject(unsigned int DataLength, unsigned char *DataBuffer, unsigned int *ImageListLength, ImageListEntry_t *ImageListPtr);

static void BIPOpenServer(unsigned int ServerPort, unsigned long ServicesMask);
static void BIPOpenRemoteServer(BD_ADDR_t BD_ADDR, unsigned int ServerPort, BIP_Imaging_Service_Server_Port_Target_t ServiceTarget);
static void BIPCloseServer(void);
static void BIPCloseConnection(void);

static void BIPAbortRequest(void);
static void BIPGetCapabilitiesRequest(void);
static void BIPPutImageRequest(char *Path, char *ImageName);
static void BIPPutLinkedThumbnailRequest(char *Path, char *ImageHandle);
static void BIPGetImagesListRequest(void);
static void BIPGetImagePropertiesRequest(char *ImageHandle);
static void BIPGetImageRequest(char *Path, char *ImageHandle);
static void BIPGetLinkedThumbnailRequest(char *Path, char *ImageHandle);
static void BIPDeleteImageRequest(char *ImageHandle);

static void DisplaySDPAttributeResponse(SDP_Service_Attribute_Response_Data_t *SDPServiceAttributeResponse, unsigned int InitLevel);
static void DisplaySDPSearchAttributeResponse(SDP_Service_Search_Attribute_Response_Data_t *SDPServiceSearchAttributeResponse);
static void DisplayDataElement(SDP_Data_Element_t *SDPDataElement, unsigned int Level);

   /* Callback Function Prototypes.                                     */
static void BTPSAPI GAP_Event_Callback(unsigned int BluetoothStackID, GAP_Event_Data_t *GAPEventData, unsigned long CallbackParameter);
static void BTPSAPI SDP_Event_Callback(unsigned int BluetoothStackID, unsigned int SDPRequestID, SDP_Response_Data_t *SDP_Response_Data, unsigned long CallbackParameter);
static void BTPSAPI BIP_Event_Callback(unsigned int BluetoothStackID, BIP_Event_Data_t *BIPEventData, unsigned long CallbackParameter);

   /* The following function displays the Command Menu.                 */
static void DisplayMenu(void)
{
   if(IsClient)
   {
      /* First display the upper command bar.                           */
      printf("************************** Command Options **************************\r\n");

      /* Next, display all of the commands.                             */
      printf("*  Inquiry                                                          *\r\n");
      printf("*  DisplayInquiryList                                               *\r\n");
      printf("*  Pair [Inquiry Index] [Bonding Type]                              *\r\n");
      printf("*  EndPairing [Inquiry Index]                                       *\r\n");
      printf("*  PINCodeResponse [PIN Code]                                       *\r\n");
      printf("*  PassKeyResponse [Numeric Passkey]                                *\r\n");
      printf("*  UserConfirmationResponse [Confirmation Flag]                     *\r\n");
      printf("*  SetDiscoverabilityMode [Discoverability Mode]                    *\r\n");
      printf("*  SetConnectabilityMode [Connectability Mode]                      *\r\n");
      printf("*  SetPairabilityMode [Pairability Mode]                            *\r\n");
      printf("*  ChangeSimplePairingParameters [I/O Capabilities] [MITM Flag]     *\r\n");
      printf("*  GetLocalAddress                                                  *\r\n");
      printf("*  GetLocalName                                                     *\r\n");
      printf("*  SetLocalName [Local Device Name (no spaces allowed)]             *\r\n");
      printf("*  GetClassOfDevice                                                 *\r\n");
      printf("*  SetClassOfDevice [Class of Device]                               *\r\n");
      printf("*  GetRemoteName [Inquiry Index]                                    *\r\n");
      printf("*  ServiceDiscovery [Inquiry Index] [Profile Index] [UUID (Manual)] *\r\n");
      printf("*  Open [Inquiry Index] [Server Port] [Service Type]                *\r\n");
      printf("*  Close                                                            *\r\n");
      printf("*  Abort                                                            *\r\n");
      printf("*  GetImageList                                                     *\r\n");
      printf("*  DisplayImageList                                                 *\r\n");
      printf("*  GetCapabilities                                                  *\r\n");
      printf("*  PutImage [Local Image File Name]                                 *\r\n");
      printf("*  PutThumb [Image Handle] [Image Name]                             *\r\n");
      printf("*  GetProperties [Image Handle]                                     *\r\n");
      printf("*  GetImage [Image Handle] [Local Image File Name]                  *\r\n");
      printf("*  GetThumb [Image Handle] [Local Image File Name]                  *\r\n");
      printf("*  DeleteImage [Image Handle]                                       *\r\n");
      printf("*  EnableDebug [Enable/Disable] [Log Type] [Log File Name]          *\r\n");
      printf("*  Help                                                             *\r\n");
      printf("*  Quit                                                             *\r\n");

      /* Finally display the lower command bar footer.                  */
      printf("*********************************************************************\r\n");
   }
   else
   {
      /* First display the upper command bar.                           */
      printf("************************** Command Options **************************\r\n");

      /* Next, display all of the commands.                             */
      printf("*  Inquiry                                                          *\r\n");
      printf("*  DisplayInquiryList                                               *\r\n");
      printf("*  Pair [Inquiry Index] [Bonding Type]                              *\r\n");
      printf("*  EndPairing [Inquiry Index]                                       *\r\n");
      printf("*  PINCodeResponse [PIN Code]                                       *\r\n");
      printf("*  PassKeyResponse [Numeric Passkey]                                *\r\n");
      printf("*  UserConfirmationResponse [Confirmation Flag]                     *\r\n");
      printf("*  SetDiscoverabilityMode [Discoverability Mode]                    *\r\n");
      printf("*  SetConnectabilityMode [Connectability Mode]                      *\r\n");
      printf("*  SetPairabilityMode [Pairability Mode]                            *\r\n");
      printf("*  ChangeSimplePairingParameters [I/O Capabilities] [MITM Flag]     *\r\n");
      printf("*  GetLocalAddress                                                  *\r\n");
      printf("*  GetLocalName                                                     *\r\n");
      printf("*  SetLocalName [Local Device Name (no spaces allowed)]             *\r\n");
      printf("*  GetClassOfDevice                                                 *\r\n");
      printf("*  SetClassOfDevice [Class of Device]                               *\r\n");
      printf("*  GetRemoteName [Inquiry Index]                                    *\r\n");
      printf("*  ServiceDiscovery [Inquiry Index] [Profile Index] [UUID (Manual)] *\r\n");
      printf("*  Open [Server Port] [Service Type Bit Mask]                       *\r\n");
      printf("*  Close                                                            *\r\n");
      printf("*  EnableDebug [Enable/Disable] [Log Type] [Log File Name]          *\r\n");
      printf("*  Help                                                             *\r\n");
      printf("*  Quit                                                             *\r\n");

      /* Finally display the lower command bar footer.                  */
      printf("*********************************************************************\r\n");
   }
}

   /* This function is responsible for taking the users input and do the*/
   /* appropriate thing with it.  First, this function get a string of  */
   /* user input, parse the user input in to command and parameters, and*/
   /* finally executing the command or display an error message if the  */
   /* input is corrupt.                                                 */
static void UserInterface_Client(void)
{
   UserCommand_t TempCommand;
   int  Result = !EXIT_CODE;
   char UserInput[MAX_COMMAND_LENGTH];

   /* First let's make sure that we start on new line.                  */
   printf("\r\n");

   /* Next display the available commands.                              */
   DisplayHelp(NULL);

   ClearCommands();

   AddCommand("OPEN", OpenRemoteServer);
   AddCommand("CLOSE", CloseConnection);
   AddCommand("GETIMAGELIST", GetImagesList);
   AddCommand("DISPLAYIMAGELIST", DisplayImageList);
   AddCommand("ABORT", Abort);
   AddCommand("GETCAPABILITIES", GetCapabilities);
   AddCommand("PUTIMAGE", PutImage);
   AddCommand("PUTTHUMB", PutLinkedThumbnail);
   AddCommand("GETPROPERTIES", GetImageProperties);
   AddCommand("GETIMAGE", GetImage);
   AddCommand("GETTHUMB", GetLinkedThumbnail);
   AddCommand("DELETEIMAGE", DeleteImage);
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

      /* Retrieve the command entered by the user and store it in       */
      /* UserInput                                                      */

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
                     /* go ahead an close any Remote BIP Server that we */
                     /* have open.                                      */
                     if(BIPID)
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
         /* Close any Remote BIP Server that we have open.              */
         if(BIPID)
            CloseConnection(NULL);

         Result = EXIT_CODE;
      }
   }
}

   /* This function is responsible for taking the users input and do the*/
   /* appropriate thing with it.  First, this function get a string of  */
   /* user input, parse the user input in to command and parameters, and*/
   /* finally executing the command or display an error message if the  */
   /* input is corrupt.                                                 */
static void UserInterface_Server(void)
{
   UserCommand_t TempCommand;
   int  Result = !EXIT_CODE;
   char UserInput[MAX_COMMAND_LENGTH];

   /* First let's make sure that we start on new line.                  */
   printf("\r\n");

   /* Next display the available commands.                              */
   DisplayHelp(NULL);

   ClearCommands();

   AddCommand("OPEN", OpenServer);
   AddCommand("CLOSE", CloseServer);
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

      /* Retrieve the command entered by the user and store it in       */
      /* UserInput                                                      */

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
                     /* go ahead an close any BIP Server that we have   */
                     /* open.                                           */
                     if(BIPID)
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
         /* Close any BIP Server that we have open.                     */
         if(BIPID)
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

   /* The following function is responsible for outputting the specified*/
   /* Object String Data to the display.                                */
static void DisplayObject(unsigned int ObjectLength, unsigned char *ObjectData)
{
   char *BufPtr;

   if((ObjectLength > 0) && (ObjectData != NULL))
   {
      if((BufPtr = (char *)malloc(ObjectLength+3)) != NULL)
      {
         memcpy(BufPtr, ObjectData, ObjectLength);
         BufPtr[ObjectLength + 0] = '\r';
         BufPtr[ObjectLength + 1] = '\n';
         BufPtr[ObjectLength + 2] = '\0';

         printf(BufPtr);

         free(BufPtr);
      }
   }
}

   /* The following function is responsible for redisplaying the Menu   */
   /* options to the user.  This function returns zero on successful    */
   /* execution or a negative value on all errors.                      */
static int DisplayHelp(ParameterList_t *TempParam)
{
   /* Simply Display the Help Menu.                                     */
   DisplayMenu();

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
   Class_of_Device_t          ClassOfDevice;
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

            /* Finally let's set the Class of Device to Desktop         */
            /* Workstation.                                             */
            ASSIGN_CLASS_OF_DEVICE(ClassOfDevice, 0x00, 0x00, 0x00);

            SET_MAJOR_DEVICE_CLASS(ClassOfDevice, HCI_LMP_CLASS_OF_DEVICE_MAJOR_DEVICE_CLASS_COMPUTER);
            SET_MINOR_DEVICE_CLASS(ClassOfDevice, HCI_LMP_CLASS_OF_DEVICE_MINOR_DEVICE_CLASS_COMPUTER_DESKTOP);

            /* Write out the Class of Device.                           */
            GAP_Set_Class_Of_Device(BluetoothStackID, ClassOfDevice);

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
      /* If debugging is enabled, go ahead and clean it up.             */
      if(DebugID)
         BTPS_Debug_Cleanup(BluetoothStackID, DebugID);

      /* Simply close the Stack                                         */
      BSC_Shutdown(BluetoothStackID);

      printf("Stack Shutdown Successfully.\r\n");

      /* Flag that the Stack is no longer initialized.                  */
      BluetoothStackID = 0;

      DebugID          = 0;

      /* Flag success to the caller.                                    */
      ret_val          = 0;
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
         printf("Set Connectability Mode Command Error : %d.\r\n", ret_val);
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
         printf("GAP_Set_Pairability_Mode(pmPairableMode).\r\n");

         /* The device has been set to pairable mode, now register an   */
         /* Authentication Callback to handle the Authentication events */
         /* if required.                                                */
         ret_val = GAP_Register_Remote_Authentication(BluetoothStackID, GAP_Event_Callback, (unsigned long)0);

         /* Next, check the return value of the GAP Register Remote     */
         /* Authentication command for successful execution.            */
         if(!ret_val)
         {
            /* The command appears to have been successful.             */
            printf("GAP_Register_Remote_Authentication() Success.\r\n");
         }
         else
         {
            /* An error occurred while trying to execute this function. */
            printf("GAP_Register_Remote_Authentication() Failure: %d\r\n", ret_val);
         }
      }
      else
      {
         /* An error occurred while trying to make the device pairable. */
         printf("Set Pairability Mode Command Error : %d.\r\n", ret_val);
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
         /* There are currently no active connections, make sure that   */
         /* all of the parameters required for this function appear to  */
         /* be at least semi-valid.                                     */
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

   /* The following function is responsible for opening a BIP Serrver on*/
   /* the Local Device.  This function opens the BIP Server in the      */
   /* Directory specified in the parameters and also opens the Server   */
   /* Port on the RFCOMM Channel specified in the parameters.  This     */
   /* function returns zero if successful, or a negative return value if*/
   /* an error occurred.                                                */
static int OpenServer(ParameterList_t *TempParam)
{
   int  ret_val;
   DIR *DirectoryHandle;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Next, check that we are NOT in client mode.                    */
      if(!IsClient)
      {
         /* Next, let's make sure that there is not a BIP Server already*/
         /* open.                                                       */
         if(!BIPID)
         {
            /* Let's check to see if a Remote RFCOMM Port has been      */
            /* specified.                                               */
            if((TempParam->NumberofParameters > 0) && (TempParam->Params[0].intParam) && ((TempParam->Params[0].intParam >= SPP_PORT_NUMBER_MINIMUM) && (TempParam->Params[0].intParam <= SPP_PORT_NUMBER_MAXIMUM)))
            {
               /* RFCOMM Port appears to be valid, so now let's check to*/
               /* see if the Service Bit Mask has been specified.       */
               if((TempParam->NumberofParameters > 1) && (TempParam->Params[1].intParam))
               {
                  /* We need to initialize the Root Directory that will */
                  /* be used for the application.                       */

                  /* First, let's figure out the current working        */
                  /* directory of the program so we can use that for our*/
                  /* Root Directory.                                    */
                  if(!getcwd(RootDirectory, sizeof(RootDirectory)))
                  {
                     /* There was an error fetching the Directory, so   */
                     /* let's just default it to the Root Directory.    */
                     RootDirectory[0] = '\0';
                  }

                  /* Let's verify that the Root Directory specified is  */
                  /* valid.                                             */
                  /* ** NOTE ** It is required that a DCIM directory    */
                  /*            exists under the ROOT directory         */
                  /*            specified.  See Annex B: Implementation */
                  /*            Guidelines for the DCF Devices in the   */
                  /*            BIP 1.0 Specification.                  */

                  /* Next we will strip off any trailing '\' the caller */
                  /* may have specified (unless it is the root of a     */
                  /* drive).                                            */
                  if((strlen(RootDirectory)) && (RootDirectory[strlen(RootDirectory) - 1] == '\\'))
                     RootDirectory[strlen(RootDirectory) - 1] = '\0';

                  /* Now check to make sure there is enough room to add */
                  /* the DCIM directory to the specified PATH.          */
                  if((strlen(RootDirectory) + strlen("/DCIM/")) < sizeof(RootDirectory))
                  {
                     /* There is enough room to add the DCIM directory  */
                     /* to the path specified.                          */
                     strcat(RootDirectory, "/DCIM");

                     /* Check to see if the Folder associated with the  */
                     /* Root Directory exists.                          */
                     if((DirectoryHandle = opendir(RootDirectory)) != NULL)
                     {
                        /* A Folder for the Root Directory exists, so   */
                        /* Free the previously allocated Directory      */
                        /* Handle.                                      */
                        closedir(DirectoryHandle);

                        /* Directory Exists, so let's make sure that the*/
                        /* Root Directory has a '/' character at the end*/
                        /* of it.                                       */
                        RootDirectory[strlen(RootDirectory) + 1] = '\0';
                        RootDirectory[strlen(RootDirectory) + 0] = '/';

                        /* Service Bit Mask appears to be specified, so */
                        /* simply attempt to Create the Server.         */
                        BIPOpenServer(TempParam->Params[0].intParam, TempParam->Params[1].intParam);

                        /* Flag success to the caller.                  */
                        ret_val = 0;
                     }
                     else
                     {
                        printf("Invalid Root Directory, unable to open server.\r\n\r\n NOTE: DCIM subdirectory must exist in current directory.\r\n");

                        ret_val = FUNCTION_ERROR;
                     }
                  }
                  else
                  {
                     printf("Invalid Root Directory, unable to open server.\r\n");

                     ret_val = FUNCTION_ERROR;
                  }
               }
               else
               {
                  printf("Usage: Open [RFCOMM Server Port] [Service Bit Mask].\r\n\r\n where Service Bit Mask:\r\n   1 - Image Push\r\n   2 - Image Pull\r\n");

                  /* One or more of the necessary parameters is/are     */
                  /* invalid.                                           */
                  ret_val = INVALID_PARAMETERS_ERROR;
               }
            }
            else
            {
               printf("Usage: Open [RFCOMM Server Port] [Service Bit Mask].\r\n\r\n where Service Bit Mask:\r\n   1 - Image Push\r\n   2 - Image Pull\r\n");

               /* One or more of the necessary parameters is/are        */
               /* invalid.                                              */
               ret_val = INVALID_PARAMETERS_ERROR;
            }
         }
         else
         {
            printf("Unable to open BIP Server.\r\nBIP Server is already open.\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
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

   /* The following function is responsible for initiating a connection */
   /* with a Remote BIP Server.  This function returns zero if          */
   /* successful and a negative value if an error occurred.             */
static int OpenRemoteServer(ParameterList_t *TempParam)
{
   int       ret_val;
   BD_ADDR_t NullADDR;

   ASSIGN_BD_ADDR(NullADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Next, check that we are in client mode.                        */
      if(IsClient)
      {
         /* Next, let's make sure that there is not a BIP Client already*/
         /* open.                                                       */
         if(!BIPID)
         {
            /* Next, let's determine if the user has specified a valid  */
            /* Bluetooth Device Address to connect with.                */
            if((TempParam) && (TempParam->NumberofParameters > 1) && (TempParam->Params[0].intParam) && (NumberofValidResponses) && (TempParam->Params[0].intParam <= NumberofValidResponses) && (!COMPARE_BD_ADDR(InquiryResultList[(TempParam->Params[0].intParam - 1)], NullADDR)) && (TempParam->Params[1].intParam >= SPP_PORT_NUMBER_MINIMUM) && (TempParam->Params[1].intParam <= SPP_PORT_NUMBER_MAXIMUM))
            {
               /* Inquiry Index and RFCOMM Port appears to be valid, so */
               /* now let's check to see if the Service Type has been   */
               /* specified.                                            */
               if((TempParam->NumberofParameters > 2) && (TempParam->Params[2].intParam >= 0) && (TempParam->Params[2].intParam <= 1))
               {
                  /* All parameters have been specified, so let's       */
                  /* attempt to connect to the remote server.           */

                  ServiceType = (BIP_Imaging_Service_Server_Port_Target_t)(TempParam->Params[2].intParam);

                  BIPOpenRemoteServer(InquiryResultList[(TempParam->Params[0].intParam - 1)], TempParam->Params[1].intParam, ServiceType);

                  /* Flag that there is no Image flagged for Deletion.  */
                  ImageDeleteIndex       = -1;

                  /* Flag that there are no Image List Entries present  */
                  /* in the Image List.                                 */
                  NumberImageListEntries = 0;
                  ImageList              = NULL;

                  /* Flag success to the caller.                        */
                  ret_val = 0;
               }
               else
               {
                  printf("Usage: Open [Inquiry Index] [RFCOMM Server Port] [Service Type].\r\n\r\n where Service Type:\r\n   0 - Image Push\r\n   1 - Image Pull\r\n");

                  /* One or more of the necessary parameters is/are     */
                  /* invalid.                                           */
                  ret_val = INVALID_PARAMETERS_ERROR;
               }
            }
            else
            {
               printf("Usage: Open [Inquiry Index] [RFCOMM Server Port] [Service Type].\r\n\r\n where Service Type:\r\n   0 - Image Push\r\n   1 - Image Pull\r\n");

               /* One or more of the necessary parameters is/are        */
               /* invalid.                                              */
               ret_val = INVALID_PARAMETERS_ERROR;
            }
         }
         else
         {
            printf("Unable to open BIP Client.\r\nBIP Client is already open.\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
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

   /* The following function is responsible for closing a BIP Server    */
   /* that was previously opened via a successful call to the           */
   /* OpenServer() function.  This function returns zero if successful  */
   /* or a negative return error code if there was an error.            */
static int CloseServer(ParameterList_t *TempParam)
{
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      if(!IsClient)
      {
         /* If a BIP Server is already opened, then simply close it.    */
         if(BIPID)
         {
            /* Simply Close the BIP Server.                             */
            BIPCloseServer();

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            printf("There is NO BIP Server currently open.\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
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

   /* The following function is responsible for terminating a connection*/
   /* with a Remote BIP Server.  This function returns zero if          */
   /* successful and a negative value if an error occurred.             */
static int CloseConnection(ParameterList_t *TempParam)
{
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      if(IsClient)
      {
         /* If a BIP Client is opened, then simply close it.            */
         if(BIPID)
         {
            /* Simply Close the BIP Client.                             */
            BIPCloseConnection();

            if((NumberImageListEntries) && (ImageList))
               free(ImageList);

            /* Flag that there are no Image List Entries present in the */
            /* Image List.                                              */
            NumberImageListEntries = 0;
            ImageList              = NULL;

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            printf("There is NO BIP Client Connection currently open.\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
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

   /* The following function is responsible for displaying the cached   */
   /* Image List of a Remote BIP Server.  This function returns zero if */
   /* successful and a negative value if an error occurred.             */
static int DisplayImageList(ParameterList_t *TempParam)
{
   int          ret_val;
   char         ModifiedBuffer[32];
   char         CreatedBuffer[32];
   Boolean_t    AM_Flag;
   unsigned int Index;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      if(IsClient)
      {
         /* If a BIP Client is opened, then simply display the cached   */
         /* image list.                                                 */
         if(BIPID)
         {
            if((NumberImageListEntries) && (ImageList))
            {
               for(Index=0;Index<NumberImageListEntries;Index++)
               {
                  if(ImageList[Index].ImageHandle[0])
                  {
                     if(ImageList[Index].CreatedTime.Year > 0)
                     {
                        /* Now retrieve the created date and time       */
                        /* information.                                 */
                        AM_Flag = (Boolean_t)(ImageList[Index].CreatedTime.Hour < 12);
                        if(AM_Flag == FALSE)
                        {
                           ImageList[Index].CreatedTime.Hour -= ((Byte_t)12);
                        }

                        sprintf(CreatedBuffer, "%d/%02d/%04d %d:%02d %s %s", ImageList[Index].CreatedTime.Month,
                                                                             ImageList[Index].CreatedTime.Day,
                                                                             ImageList[Index].CreatedTime.Year,
                                                                             ImageList[Index].CreatedTime.Hour,
                                                                             ImageList[Index].CreatedTime.Minute,
                                                                             (AM_Flag)?"AM":"PM",
                                                                             (ImageList[Index].CreatedTime.UTC_Time)?"UTC":"");
                     }

                     if(ImageList[Index].ModifiedTime.Year > 0)
                     {
                        /* Now retrieve the modified date and time      */
                        /* information.                                 */
                        AM_Flag = (Boolean_t)(ImageList[Index].ModifiedTime.Hour < 12);
                        if(AM_Flag == FALSE)
                        {
                           ImageList[Index].ModifiedTime.Hour -= ((Byte_t)12);
                        }

                        sprintf(ModifiedBuffer, "%d/%02d/%04d %d:%02d %s %s", ImageList[Index].ModifiedTime.Month,
                                                                              ImageList[Index].ModifiedTime.Day,
                                                                              ImageList[Index].ModifiedTime.Year,
                                                                              ImageList[Index].ModifiedTime.Hour,
                                                                              ImageList[Index].ModifiedTime.Minute,
                                                                              (AM_Flag)?"AM":"PM",
                                                                              (ImageList[Index].ModifiedTime.UTC_Time)?"UTC":"");
                     }

                     printf("Image Handle: %s Created Time: %s Modified Time: %s.\r\n", ImageList[Index].ImageHandle, CreatedBuffer, ModifiedBuffer);
                  }
               }
            }
            else
               printf("Image List Entry List is empty.\r\n");

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            printf("There is NO BIP Client Connection currently open.\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
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

   /* The following function is responsible for aborting any outstanding*/
   /* BIP Action on a Remote BIP Server.  This function returns zero if */
   /* successful and a negative value if an error occurred.             */
static int Abort(ParameterList_t *TempParam)
{
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      if(IsClient)
      {
         /* If a BIP Client is opened, then simply issue the Abort      */
         /* Request.                                                    */
         if(BIPID)
         {
            /* Simply Issue the Abort Request.                          */
            BIPAbortRequest();

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            printf("There is NO BIP Client Connection currently open.\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
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

   /* The following function is responsible for issuing a Get           */
   /* Capabilities Request on a Remote BIP Server.  This function       */
   /* returns zero if successful and a negative value if an error       */
   /* occurred.                                                         */
static int GetCapabilities(ParameterList_t *TempParam)
{
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      if(IsClient)
      {
         /* If a BIP Client is opened, then simply issue the Get        */
         /* Capabilities Request.                                       */
         if(BIPID)
         {
            /* Simply Issue the Get Capabilities Request.               */
            BIPGetCapabilitiesRequest();

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            printf("There is NO BIP Client Connection currently open.\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
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

   /* The following function is responsible for issuing a Put Image     */
   /* Request on a Remote BIP Server.  This function returns zero if    */
   /* successful and a negative value if an error occurred.             */
static int PutImage(ParameterList_t *TempParam)
{
   int  ret_val;
   char Name[256+1];

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      if(IsClient)
      {
         /* If a BIP Client is opened, then we need to issue the Put    */
         /* Image Request.                                              */
         if(BIPID)
         {
            /* Next, let's make sure that an Image File Name was        */
            /* specified as a parameter to this command.                */
            if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].strParam) && (strlen(TempParam->Params[0].strParam)))
            {
               /* Issue the BIP Put Image Request.                      */

               /* We need to parse out the Path and File Name           */
               /* information from the string that was specified.       */
               ParseFileName(TempParam->Params[0].strParam, Name);

               BIPPutImageRequest(TempParam->Params[0].strParam, Name);

               /* Flag success to the caller.                           */
               ret_val = 0;
            }
            else
            {
               printf("Usage: PutImage [Local Image File Name].\r\n");

               /* One or more of the necessary parameters is/are        */
               /* invalid.                                              */
               ret_val = INVALID_PARAMETERS_ERROR;
            }
         }
         else
         {
            printf("There is NO BIP Client Connection currently open.\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
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

   /* The following function is responsible for issuing a Put Linked    */
   /* Thumbnail Request on a Remote BIP Server.  This function returns  */
   /* zero if successful and a negative value if an error occurred.     */
static int PutLinkedThumbnail(ParameterList_t *TempParam)
{
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      if(IsClient)
      {
         /* If a BIP Client is opened, then we need to issue the Put    */
         /* Linked Thumbnail Request.                                   */
         if(BIPID)
         {
            /* Next, let's make sure that an Image Handle was specified */
            /* as a parameter to this command.                          */
            if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].strParam) && (strlen(TempParam->Params[0].strParam)))
            {
               /* Next, let's make sure that an Image File Name was     */
               /* specified as a parameter to this command.             */
               if((TempParam) && (TempParam->NumberofParameters > 1) && (TempParam->Params[1].strParam) && (strlen(TempParam->Params[1].strParam)))
               {
                  /* Issue the BIP Put Linked Thumbnail Request.        */
                  BIPPutLinkedThumbnailRequest(TempParam->Params[1].strParam, TempParam->Params[0].strParam);

                  /* Flag success to the caller.                        */
                  ret_val = 0;
               }
               else
               {
                  printf("Usage: PutThumb [Image Handle] [Local Image File Name].\r\n\r\n where Image Handle:\r\n   7 ASCII characters ('0' - '9' only)\r\n");

                  /* One or more of the necessary parameters is/are     */
                  /* invalid.                                           */
                  ret_val = INVALID_PARAMETERS_ERROR;
               }
            }
            else
            {
               printf("Usage: PutThumb [Image Handle] [Local Image File Name].\r\n\r\n where Image Handle:\r\n   7 ASCII characters ('0' - '9' only)\r\n");

               /* One or more of the necessary parameters is/are        */
               /* invalid.                                              */
               ret_val = INVALID_PARAMETERS_ERROR;
            }
         }
         else
         {
            printf("There is NO BIP Client Connection currently open.\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
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

   /* The following function is responsible for issuing a Get Images    */
   /* List Request on a Remote BIP Server.  This function returns zero  */
   /* if successful and a negative value if an error occurred.          */
static int GetImagesList(ParameterList_t *TempParam)
{
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      if(IsClient)
      {
         /* If a BIP Client is opened, then simply issue the Get Images */
         /* List Request.                                               */
         if(BIPID)
         {
            /* Simply Issue the Get Images List Request.                */
            BIPGetImagesListRequest();

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            printf("There is NO BIP Client Connection currently open.\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
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

   /* The following function is responsible for issuing a Get Image     */
   /* Properties Request on a Remote BIP Server.  This function returns */
   /* zero if successful and a negative value if an error occurred.     */
static int GetImageProperties(ParameterList_t *TempParam)
{
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      if(IsClient)
      {
         /* If a BIP Client is opened, then simply issue the Get Image  */
         /* Properties Request.                                         */
         if(BIPID)
         {
            /* Next, let's make sure that an Image Handle was specified */
            /* as a parameter to this command.                          */
            if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].strParam) && (strlen(TempParam->Params[0].strParam)))
            {
               /* Issue the BIP Get Image Properties Request.           */
               BIPGetImagePropertiesRequest(TempParam->Params[0].strParam);

               /* Flag success to the caller.                           */
               ret_val = 0;
            }
            else
            {
               printf("Usage: GetProperties [Image Handle].\r\n\r\n where Image Handle:\r\n   7 ASCII characters ('0' - '9' only)\r\n");

               /* One or more of the necessary parameters is/are        */
               /* invalid.                                              */
               ret_val = INVALID_PARAMETERS_ERROR;
            }
         }
         else
         {
            printf("There is NO BIP Client Connection currently open.\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
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

   /* The following function is responsible for issuing a Get Image     */
   /* Request on a Remote BIP Server.  This function returns zero if    */
   /* successful and a negative value if an error occurred.             */
static int GetImage(ParameterList_t *TempParam)
{
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      if(IsClient)
      {
         /* If a BIP Client is opened, then we need to issue the Get    */
         /* Image Request.                                              */
         if(BIPID)
         {
            /* Next, let's make sure that an Image Handle was specified */
            /* as a parameter to this command.                          */
            if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].strParam) && (strlen(TempParam->Params[0].strParam)))
            {
               /* Next, let's make sure that an Image File Name was     */
               /* specified as a parameter to this command.             */
               if((TempParam) && (TempParam->NumberofParameters > 1) && (TempParam->Params[1].strParam) && (strlen(TempParam->Params[1].strParam)))
               {
                  /* Issue the BIP Get Image Request.                   */
                  BIPGetImageRequest(TempParam->Params[1].strParam, TempParam->Params[0].strParam);

                  /* Flag success to the caller.                        */
                  ret_val = 0;
               }
               else
               {
                  printf("Usage: GetImage [Image Handle] [Local Image File Name].\r\n\r\n where Image Handle:\r\n   7 ASCII characters ('0' - '9' only)\r\n");

                  /* One or more of the necessary parameters is/are     */
                  /* invalid.                                           */
                  ret_val = INVALID_PARAMETERS_ERROR;
               }
            }
            else
            {
               printf("Usage: GetImage [Image Handle] [Local Image File Name].\r\n\r\n where Image Handle:\r\n   7 ASCII characters ('0' - '9' only)\r\n");

               /* One or more of the necessary parameters is/are        */
               /* invalid.                                              */
               ret_val = INVALID_PARAMETERS_ERROR;
            }
         }
         else
         {
            printf("There is NO BIP Client Connection currently open.\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
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

   /* The following function is responsible for issuing a Get Linked    */
   /* Thumbnail on a Remote BIP Server.  This function returns zero if  */
   /* successful and a negative value if an error occurred.             */
static int GetLinkedThumbnail(ParameterList_t *TempParam)
{
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      if(IsClient)
      {
         /* If a BIP Client is opened, then we need to issue the Get    */
         /* Linked Thumbnail Image Request.                             */
         if(BIPID)
         {
            /* Next, let's make sure that an Image Handle was specified */
            /* as a parameter to this command.                          */
            if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].strParam) && (strlen(TempParam->Params[0].strParam)))
            {
               /* Next, let's make sure that an Image File Name was     */
               /* specified as a parameter to this command.             */
               if((TempParam) && (TempParam->NumberofParameters > 1) && (TempParam->Params[1].strParam) && (strlen(TempParam->Params[1].strParam)))
               {
                  /* Issue the BIP Get Linked Thumbnail Image Request.  */
                  BIPGetLinkedThumbnailRequest(TempParam->Params[1].strParam, TempParam->Params[0].strParam);

                  /* Flag success to the caller.                        */
                  ret_val = 0;
               }
               else
               {
                  printf("Usage: GetThumb [Image Handle] [Local Image File Name].\r\n\r\n where Image Handle:\r\n   7 ASCII characters ('0' - '9' only)\r\n");

                  /* One or more of the necessary parameters is/are     */
                  /* invalid.                                           */
                  ret_val = INVALID_PARAMETERS_ERROR;
               }
            }
            else
            {
               printf("Usage: GetThumb [Image Handle] [Local Image File Name].\r\n\r\n where Image Handle:\r\n   7 ASCII characters ('0' - '9' only)\r\n");

               /* One or more of the necessary parameters is/are        */
               /* invalid.                                              */
               ret_val = INVALID_PARAMETERS_ERROR;
            }
         }
         else
         {
            printf("There is NO BIP Client Connection currently open.\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
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

   /* The following function is responsible for issuing a Delete Image  */
   /* on a Remote BIP Server.  This function returns zero if successful */
   /* and a negative value if an error occurred.                        */
static int DeleteImage(ParameterList_t *TempParam)
{
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      if(IsClient)
      {
         /* If a BIP Client is opened, then simply issue the Delete     */
         /* Image Request.                                              */
         if(BIPID)
         {
            /* Next, let's make sure that an Image Handle was specified */
            /* as a parameter to this command.                          */
            if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].strParam) && (strlen(TempParam->Params[0].strParam)))
            {
               /* Issue the BIP Delete Image Request.                   */
               BIPDeleteImageRequest(TempParam->Params[0].strParam);

               /* Flag success to the caller.                           */
               ret_val = 0;
            }
            else
            {
               printf("Usage: DeleteImage [Image Handle].\r\n\r\n where Image Handle:\r\n   7 ASCII characters ('0' - '9' only)\r\n");

               /* One or more of the necessary parameters is/are        */
               /* invalid.                                              */
               ret_val = INVALID_PARAMETERS_ERROR;
            }
         }
         else
         {
            printf("There is NO BIP Client Connection currently open.\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
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

   /* The following function is a utility function that exists to parse */
   /* a fully Path qualified File Name (first parameter) into it's      */
   /* component File Name.                                              */
static void ParseFileName(char *PathFileName, char *FileName)
{
   int Index;

   if((PathFileName) && (strlen(PathFileName)) && (FileName))
   {
      Index       = strlen(PathFileName) - 1;

      FileName[0] = '\0';

      /* If the very last character is a path delimeter then we will    */
      /* skip it.                                                       */
      if(PathFileName[Index] == '/')
         PathFileName[Index--] = '\0';

      /* Now move backward in the string until we either run out of     */
      /* string or we find a path delimeter.                            */
      while((Index >= 0) && (PathFileName[Index] != '/'))
         Index--;

      /* Check to see if just a filename was given (no path             */
      /* information).                                                  */
      if((Index < 0) && (strlen(PathFileName)))
         strcpy(FileName, PathFileName);
      else
      {
         /* Check to see if Path delimeter was found.                   */
         if((Index >= 0) && (Index != (int)(strlen(PathFileName) - 1)) && (PathFileName[Index] == '/'))
         {
            strcpy(FileName, &(PathFileName[Index+1]));
         }
      }
   }
   else
   {
      if(FileName)
         FileName[0] = '\0';
   }
}

   /* The following function is responsible for parsing an integer from */
   /* the specified Data Stream.                                        */
static int ExtractInt(char *CharPtr, int NumberOfDigits)
{
   int value = 0;

   /* Stop if we reach a NULL or the Number of Digits have been         */
   /* extracted.                                                        */
   while((*CharPtr) && (NumberOfDigits))
   {
      /* Check to make sure that the current character is a valid digit */
      /* of 0 through 9.                                                */
      if((*CharPtr >= '0') && (*CharPtr <= '9'))
      {
         value = (value * 10) + (*CharPtr & 0x0F);
      }
      else
      {
         /* Exit on Error.                                              */
         break;
      }

      /* Advance to the next character in the string.                   */
      CharPtr++;
      NumberOfDigits--;
   }

   /* Check to see if an error occurred while parsing the string.  If   */
   /* the Number of Digits is greater then Zero, then we stopped early  */
   /* due to some error condition.                                      */
   if(NumberOfDigits)
   {
      value = APPLICATION_ERROR;
   }

   return(value);
}

   /* The following function is responsible for parsing the date and    */
   /* time information from the specified Data Stream.                  */
static int ExtractTimeDate(char *TDStringPtr, TimeDate_t *TimeDatePtr)
{
   int ret_val = APPLICATION_ERROR;
   int value;

   /* Ensure the pointer are not NULL.                                  */
   if((TDStringPtr) && (TimeDatePtr))
   {
      /* Since we will allow the use to provide a pointer to may not be */
      /* positioned at the beginning of the string, we will start by    */
      /* scanning past all characters that are not a valid ASCII digit. */
      while((*TDStringPtr) && ((*TDStringPtr < '0') || (*TDStringPtr > '9')))
      {
         TDStringPtr++;
      }

      /* The first item to be extracted is the Year.  This is a 4 digit */
      /* value.  If an error occurs trying to extract the value, exit   */
      /* with an error.                                                 */
      if((value = ExtractInt(TDStringPtr, 4)) >= 0)
      {
         /* Adjust the pointer past the 4 characters we have just       */
         /* extracted and save the Year value in the Structure.         */
         TDStringPtr       += 4;
         TimeDatePtr->Year  = (Word_t)value;

         /* Extract the 2 digit Month.  If an error occurs, exit with   */
         /* and error code.                                             */
         if((value = ExtractInt(TDStringPtr, 2)) >= 0)
         {
            /* Adjust the pointer past the 2 characters that we have    */
            /* just extracted and save the value in the structure.      */
            TDStringPtr        += 2;
            TimeDatePtr->Month  = (Word_t)value;

            /* Extract the 2 digit Day.  If an error occurs, exit with  */
            /* and error code.                                          */
            if((value = ExtractInt(TDStringPtr, 2)) >= 0)
            {
               /* Adjust the pointer past the 2 characters that we have */
               /* just extracted and save the value in the structure.   */
               TDStringPtr      += 2;
               TimeDatePtr->Day  = (Word_t)value;

               /* For a consistency check, make sure that the next      */
               /* character is a T.  This is a separator between the    */
               /* Date and Time.                                        */
               if(*TDStringPtr == 'T')
               {
                  /* Adjust the pointer past the separator.             */
                  TDStringPtr += 1;

                  /* Extract the 2 digit Hour.  If an error occurs, exit*/
                  /* with and error code.                               */
                  if((value = ExtractInt(TDStringPtr, 2)) >= 0)
                  {
                     /* Adjust the pointer past the 2 characters that we*/
                     /* have just extracted and save the value in the   */
                     /* structure.                                      */
                     TDStringPtr       += 2;
                     TimeDatePtr->Hour  = (Word_t)value;

                     /* Extract the 2 digit Minute.  If an error occurs,*/
                     /* exit with and error code.                       */
                     if((value = ExtractInt(TDStringPtr, 2)) >= 0)
                     {
                        /* Adjust the pointer past the 2 characters that*/
                        /* we have just extracted and save the value in */
                        /* the structure.                               */
                        TDStringPtr         += 2;
                        TimeDatePtr->Minute  = (Word_t)value;

                        /* Extract the 2 digit Minute.  If an error     */
                        /* occurs, exit with and error code.            */
                        if((value = ExtractInt(TDStringPtr, 2)) >= 0)
                        {
                           /* Adjust the pointer past the 2 characters  */
                           /* that we have just extracted and save the  */
                           /* value in the structure.                   */
                           TDStringPtr         += 2;
                           TimeDatePtr->Second  = (Word_t)value;

                           /* The last character may contain a Z to     */
                           /* denote UTC formatted time.                */
                           TimeDatePtr->UTC_Time = (Boolean_t)((*TDStringPtr == 'Z')?TRUE:FALSE);

                           /* No errors were detected, return zero to   */
                           /* denote success.                           */
                           ret_val = 0;
                        }
                     }
                  }
               }
            }
         }
      }
   }

   return(ret_val);
}

   /* The following function is responsible for finding the specified   */
   /* Token in the specified Data Stream.                               */
static int FindToken(char Token, unsigned int Offset, unsigned int DataLength, unsigned char *DataBuffer)
{
   int ret_val;

   /* Check the input parameter to make sure that appear to be at least */
   /* semi-valid.                                                       */
   if((Offset < DataLength) && (DataLength > 0) && (DataBuffer != NULL))
   {
      /* Loop through until the token is located.                       */
      for(ret_val=(int)Offset;ret_val<(int)DataLength;ret_val++)
      {
         /* Check to see if the current character is the character we   */
         /* are looking for.                                            */
         if(DataBuffer[ret_val] == Token)
            break;

         /* If we are searching for a <SPACE> characters, then we need  */
         /* to also treat all whitespace characters as a <SPACE>.       */
         if((Token == _WHITE_SPACE_CHARACTER_) && (IS_WHITE_SPACE_CHARACTER(DataBuffer[ret_val])))
            break;

         /* If we are searching for any character but a <SPACE>         */
         /* character, then we also must include all white space        */
         /* characters.                                                 */
         if((Token == _NON_SPACE_) && (!IS_WHITE_SPACE_CHARACTER(DataBuffer[ret_val])))
            break;
      }

      /* Check to see if the token was located.                         */
      if(ret_val == (int)DataLength)
      {
         /* The token was not located, set the return value to indicate */
         /* this.                                                       */
         ret_val = APPLICATION_ERROR;
      }
   }
   else
      ret_val = APPLICATION_ERROR;

   return(ret_val);
}

   /* The following function is responsible for retrieving the Image    */
   /* Properties of the specified Image.                                */
static int GetFileImageProperties(char *FileName, Word_t *ImageWidth, Word_t *ImageHeight, DWord_t *ImageFileSize)
{
   int         ret_val;
   Byte_t      Buffer[4];
   int         AmountRead;
   int         FileDescriptor;
   Word_t      Marker;
   Word_t      Length;
   Boolean_t   Done;
   struct stat FileStat;

   /* Check the input parameter to make sure that appear to be at least */
   /* semi-valid.                                                       */
   if((FileName != NULL) && (strlen(FileName) > 0) && (ImageWidth != NULL) && (ImageHeight != NULL) && (ImageFileSize != NULL))
   {
      /* Attempt to open the specified file.                            */
      if((FileDescriptor = open(FileName, (O_NONBLOCK | O_RDONLY), 0)) >= 0)
      {
         /* Get the total file size.                                    */
         if((!fstat(FileDescriptor, &FileStat)) && (S_ISREG(FileStat.st_mode) || S_ISLNK(FileStat.st_mode)))
            (*ImageFileSize) = FileStat.st_size;
         else
            (*ImageFileSize) = 0;

         /* Initialize the variables to a known state.                  */
         Done           = FALSE;
         ret_val        = 0;
         (*ImageHeight) = 0;
         (*ImageWidth)  = 0;

         /* Search for the start of frame header.                       */
         while((!Done) && (ret_val == 0))
         {
            if(((AmountRead = read(FileDescriptor, Buffer, sizeof(Buffer))) != (unsigned int)(-1)) && (AmountRead == sizeof(Buffer)))
            {
               /* Retrieve the Marker and the Length from the data read.*/
               Marker = READ_UNALIGNED_WORD_BIG_ENDIAN(Buffer);
               Length = READ_UNALIGNED_WORD_BIG_ENDIAN(&(Buffer[sizeof(Word_t)]));

               /* Check to see if this appears to tbe a valid start of  */
               /* frame header.                                         */
               if((Marker == 0xFFC0) && (Length == 17))
               {
                  /* This appears to be a valid start of frame header.  */
                  /* Next adjust the file pointer to the "Vertical      */
                  /* Lines" location in this header.                    */
                  lseek(FileDescriptor, sizeof(Byte_t), SEEK_CUR);

                  /* Next attempt to read out the Width and Height (in  */
                  /* pixels) of this image.                             */
                  if(((AmountRead = read(FileDescriptor, Buffer, sizeof(Buffer))) != (unsigned int)(-1)) && (AmountRead == sizeof(Buffer)))
                  {
                     /* Next get the Height and Width of the image.     */
                     (*ImageHeight) = (Word_t)READ_UNALIGNED_WORD_BIG_ENDIAN(Buffer);
                     (*ImageWidth)  = (Word_t)READ_UNALIGNED_WORD_BIG_ENDIAN(&(Buffer[sizeof(Word_t)]));

                     Done = TRUE;
                  }
                  else
                     ret_val = APPLICATION_ERROR;
               }
               else
               {
                  /* This was not a start of frame.  Check to see it was*/
                  /* the start of image marker.                         */
                  if(Marker == 0xFFD8)
                  {
                     /* This was the start of image marker.  In this    */
                     /* case we want to set the File Pointer to read the*/
                     /* first header after the start of image in the    */
                     /* next read.  Set the File Pointer back one word. */
                     lseek(FileDescriptor, -((int)(sizeof(Word_t))), SEEK_CUR);
                  }
                  else
                  {
                     /* This is not the start of image marker.  Move the*/
                     /* next header location.                           */
                     lseek(FileDescriptor, Length-sizeof(Word_t), SEEK_CUR);
                  }
               }
            }
            else
               ret_val = APPLICATION_ERROR;
         }

         /* Close the previously opened file descriptor.                */
         close(FileDescriptor);
      }
      else
         ret_val = APPLICATION_ERROR;
   }
   else
      ret_val = APPLICATION_ERROR;

   return(ret_val);
}

   /* The following function is responsible for retrieving the next     */
   /* available Image Handle.                                           */
static int GetNextImageHandle(char *ImageHandle)
{
   int           ret_val;
   char          CurrentPath[512+1];
   char          Name[32];
   Boolean_t     Done;
   Boolean_t     Done2;
   DIR          *DirectoryHandle;
   int           FileDescriptor;
   unsigned int  CurrentPathEndIndex;
   unsigned int  CurrentPathWithDirectoryEndIndex;
   unsigned int  DirectoryNumber;
   unsigned int  FileNumber;

   /* Before proceeding check to see if the parameters passed in appear */
   /* to be at least semi-valid.                                        */
   if(ImageHandle != NULL)
   {
      /* The parameters passed in appear to be at least semi-valid.     */
      /* Check to make sure that the root directory path string will fit*/
      /* in the current path buffer with the folder name and file name  */
      /* appended to it.                                                */
      if(sizeof(CurrentPath) >= (strlen(RootDirectory) + (22*sizeof(char))))
      {
         /* The Root Directory will fit in the current path buffer.     */
         /* ** NOTE ** The root directory already has the '/' character */
         /*            built on it.                                     */
         strcpy(CurrentPath, RootDirectory);

         /* Initialized the variables required to find the next         */
         /* available image handle.                                     */
         CurrentPathEndIndex = strlen(CurrentPath);
         DirectoryNumber     = DIRECTORY_NUMBER_MINIMUM;
         FileNumber          = FILE_NUMBER_MINIMUM;
         ret_val             = 0;
         Done                = FALSE;

         /* Loop through the directories and files until an image handle*/
         /* is found.                                                   */
         while((!Done) && (ret_val == 0))
         {
            /* Valid Directory Numbers are between                      */
            /* DIRECTORY_NUMBER_MINIMUM and DIRECTORY_NUMBER_MAXIMUM.   */
            /* Check to see if the maximum directory number has been    */
            /* reached.                                                 */
            if(DirectoryNumber <= DIRECTORY_NUMBER_MAXIMUM)
            {
               /* Build the Folder Name and Path to the Folder.         */
               sprintf(Name, "%03uFOLDR", DirectoryNumber);
               sprintf(&CurrentPath[strlen(CurrentPath)], "%s", Name);

               /* Check to see if the Folder associated with the current*/
               /* directory number exists.                              */
               if((DirectoryHandle = opendir(CurrentPath)) != NULL)
               {
                  /* A Folder with the current directory number exists. */
                  /* Free the previously allocated Directory Handle.    */
                  closedir(DirectoryHandle);

                  /* Initialize the variables required to find the next */
                  /* next available file number in the current          */
                  /* directory.                                         */
                  CurrentPathWithDirectoryEndIndex = strlen(CurrentPath);
                  FileNumber                       = FILE_NUMBER_MINIMUM;
                  Done2                            = FALSE;

                  /* Loop through the files until the next valid image  */
                  /* handle is found.                                   */
                  while((!Done2) && (ret_val == 0))
                  {
                     /* Valid File Numbers are between                  */
                     /* FILE_NUMBER_MINIMUM and FILE_NUMBER_MAXIMUM.    */
                     /* check to see if the maximum file number has been*/
                     /* reached.                                        */
                     if(FileNumber <= FILE_NUMBER_MAXIMUM)
                     {
                        /* Build the File Name for this file number.    */
                        sprintf(Name, "IMGE%04u.JPG", FileNumber);
                        sprintf(&CurrentPath[strlen(CurrentPath)], "/%s", Name);

                        /* Check to see if a File with this name already*/
                        /* exists.                                      */
                        if((FileDescriptor = open(CurrentPath, (O_NONBLOCK | O_RDONLY), 0)) >= 0)
                        {
                           /* A File with the file number already       */
                           /* exists.  Free the opened File Descriptor. */
                           close(FileDescriptor);

                           /* Increment the File Number.                */
                           FileNumber++;
                        }
                        else
                        {
                           /* A file with this File Number does not     */
                           /* exists.  The next image handle has been   */
                           /* found.  Set the variables to force the    */
                           /* search to end.                            */
                           Done2 = TRUE;
                           Done  = TRUE;
                        }

                        /* Remove the Current File Name from the        */
                        /* currently build path.                        */
                        CurrentPath[CurrentPathWithDirectoryEndIndex] = '\0';
                     }
                     else
                     {
                        /* Invalid File Number increment the directory  */
                        /* number and exit the File Number search loop. */
                        DirectoryNumber++;
                        Done2 = TRUE;
                     }
                  }
               }
               else
               {
                  /* A directory with this folder number does not exist.*/
                  /* The next image handle has been found.  Set the     */
                  /* variables to force the search to end.              */
                  Done = TRUE;

                  /* Since this directory doesn't exist, go ahead an    */
                  /* create it now.                                     */
                  mkdir(CurrentPath, (S_IRWXG | S_IRWXU | S_IRWXO));
               }

               /* Remove the Current Folder Name from the currently     */
               /* build path.                                           */
               CurrentPath[CurrentPathEndIndex] = '\0';
            }
            else
               ret_val = APPLICATION_ERROR;
         }

         /* Check to see the next available image handle was            */
         /* successfully located.                                       */
         if(ret_val == 0)
         {
            /* The next available image handle was located.  Build the  */
            /* image handle to return to the caller.                    */
            sprintf(ImageHandle, "%03u%04u", DirectoryNumber, FileNumber);
         }
      }
      else
         ret_val = APPLICATION_ERROR;
   }
   else
      ret_val = APPLICATION_ERROR;

   return(ret_val);
}

   /* The following function is responsible for retrieving Image File   */
   /* Data from an Image File.                                          */
static int GetFileData(void)
{
   int         ret_val;
   DWord_t     AmountToRead;
   int         AmountRead;
   int         FileDescriptor;
   struct stat FileStat;

   /* Check to see if the entire file has been read.                    */
   if((CurrentFileIndex < CurrentFileSize) || (FirstPhase))
   {
      /* The entire file has not been read, next check to make sure that*/
      /* is room in the buffer to read more of the file into.           */
      if(CurrentFileBufferIndex < sizeof(CurrentFileBuffer))
      {
         /* Next attempt to open the current file being read from.      */
         if((FileDescriptor = open(CurrentFile, (O_NONBLOCK | O_RDONLY), 0)) >= 0)
         {
            /* Check to see if this is the first phase.                 */
            if(FirstPhase)
            {
               /* This is the first phase of this operation, get the    */
               /* file size of this image.                              */
               if((!fstat(FileDescriptor, &FileStat)) && (S_ISREG(FileStat.st_mode) || S_ISLNK(FileStat.st_mode)))
                  CurrentFileSize = FileStat.st_size;
               else
                  CurrentFileSize = 0;

               FirstPhase = FALSE;
            }

            /* Seek to the current file location.                       */
            lseek(FileDescriptor, CurrentFileIndex, SEEK_SET);

            /* Calculate the amount to read based on the current buffer */
            /* space.                                                   */
            AmountToRead = sizeof(CurrentFileBuffer) - CurrentFileBufferIndex;

            /* Attempt to read some data from the file.                 */
            if((AmountRead = read(FileDescriptor, &CurrentFileBuffer[CurrentFileBufferIndex], AmountToRead)) != (unsigned int)(-1))
            {
               /* Some data was successfully read, adjust the current   */
               /* file buffer index by the amount read.                 */
               CurrentFileBufferIndex += AmountRead;
               CurrentFileIndex       += AmountRead;

               /* Set the return value to indicate success.             */
               ret_val = FILE_DATA_FILE_IO_SUCCESS;
            }
            else
               ret_val = FILE_DATA_FILE_IO_ERROR;

            /* Close the previously opened file descriptor.             */
            close(FileDescriptor);
         }
         else
            ret_val = FILE_DATA_FILE_IO_ERROR;
      }
      else
         ret_val = FILE_DATA_FILE_IO_SUCCESS;
   }
   else
      ret_val = FILE_DATA_END_OF_FILE;

   return(ret_val);
}

   /* The following function is responsible for writing the specified   */
   /* data to an Image File.                                            */
static int PutFileData(unsigned int DataLength, unsigned char *DataBuffer)
{
   int ret_val;
   int BytesWritten;
   int FileDescriptor;

   /* Check to make sure that the parameters passed in appear to be at  */
   /* least semi-valid.                                                 */
   if((DataLength > 0) && (DataBuffer != NULL))
   {
      /* The parameters passed in appear to be at least semi-valid.     */
      /* Attempt to create/open the current file.                       */
      if((FileDescriptor = open(CurrentFile, ((FirstPhase?(O_TRUNC | O_CREAT):0) | O_NONBLOCK | O_WRONLY), (S_IRWXG | S_IRWXU | S_IRWXO))) >= 0)
      {
         /* Check to see if this is the first phase.                    */
         if(!FirstPhase)
         {
            /* Seek to the end of the file.                             */
            lseek(FileDescriptor, 0, SEEK_END);
         }

         /* Indicate that this is not longer the first phase.           */
         FirstPhase = FALSE;

         /* Attempt to write the data to the file.                      */
         if((BytesWritten = write(FileDescriptor, DataBuffer, DataLength)) != (unsigned int)(-1))
         {
            /* Adjust the amount written.                               */
            CurrentFileIndex += BytesWritten;

            /* Set the return value to indicate success.                */
            ret_val = FILE_DATA_FILE_IO_SUCCESS;
         }
         else
            ret_val = FILE_DATA_FILE_IO_ERROR;

         /* Close the descriptor of the previously open file.           */
         close(FileDescriptor);
      }
      else
         ret_val = FILE_DATA_FILE_IO_ERROR;
   }
   else
      ret_val = FILE_DATA_FILE_IO_ERROR;

   return(ret_val);
}

   /* The following function is responsible for verifying that an       */
   /* associated Image exists for the specified Image Handle.           */
static Boolean_t TestImageHandle(char *ImageHandle)
{
   char      TestPath[512+1];
   int       FileDescriptor;
   Boolean_t ret_val = FALSE;

   /* Check to make sure that the Image Handle passed in appears to be  */
   /* at least semi-valid.                                              */
   if((ImageHandle != NULL) && (strlen(ImageHandle) == BIP_IMAGE_HANDLE_LENGTH))
   {
      /* The Image Handle appears to be at least semi-valid.  Next build*/
      /* the Test Path.                                                 */
      strcpy(TestPath, RootDirectory);
      sprintf(&TestPath[strlen(TestPath)], "%c%c%cFOLDR", ImageHandle[0], ImageHandle[1], ImageHandle[2]);
      sprintf(&TestPath[strlen(TestPath)], "/IMGE%c%c%c%c.JPG", ImageHandle[3], ImageHandle[4], ImageHandle[5], ImageHandle[6]);

      /* Check to see if the file associated with this image handle     */
      /* exists..                                                       */
      if((FileDescriptor = open(TestPath, (O_NONBLOCK | O_RDONLY), 0)) >= 0)
      {
         /* There is a file associated with this image handle.  Close   */
         /* the open File Descriptor.                                   */
         close(FileDescriptor);

         /* Set the return value to indicate success.                   */
         ret_val = TRUE;
      }
   }

   return(ret_val);
}

   /* The following function is responsible for initializing the Image  */
   /* List.                                                             */
static int InitializeImageList(void)
{
   int           ret_val;
   char          CurrentPath[512+1];
   char          Name[32];
   Boolean_t     Done;
   Boolean_t     Done2;
   DIR          *DirectoryHandle;
   int           FileDescriptor;
   struct stat   FileStat;
   struct tm    *Time;
   unsigned int  Count;
   unsigned int  FileNumber;
   unsigned int  DirectoryNumber;
   unsigned int  CurrentPathEndIndex;
   unsigned int  CurrentPathWithDirectoryEndIndex;

   /* Check to see if the image list appears to be already initialized. */
   if((NumberImageListEntries == 0) && (ImageList == NULL))
   {
      /* There is not currently an image list already initialized.      */
      /* Check to make sure that the root directory path string will fit*/
      /* in the current path buffer with the folder name and file name  */
      /* appended to it.                                                */
      if(sizeof(CurrentPath) >= (strlen(RootDirectory) + (22*sizeof(char))))
      {
         /* The Root Directory will fit in the current path buffer.     */
         /* ** NOTE ** The root directory already has the '/' character */
         /*            built on it.                                     */
         strcpy(CurrentPath, RootDirectory);

         /* Initialized the variables required to count the number of   */
         /* images in the file system.                                  */
         CurrentPathEndIndex = strlen(CurrentPath);
         DirectoryNumber     = DIRECTORY_NUMBER_MINIMUM;
         Count               = 0;
         ret_val             = 0;
         Done                = FALSE;

         /* Loop through the directories and files until all valid      */
         /* images are found.                                           */
         while((!Done) && (ret_val == 0))
         {
            /* Valid Directory Numbers are between                      */
            /* DIRECTORY_NUMBER_MINIMUM and DIRECTORY_NUMBER_MAXIMUM.   */
            /* Check to see if the maximum directory number has been    */
            /* reached.                                                 */
            if(DirectoryNumber <= DIRECTORY_NUMBER_MAXIMUM)
            {
               /* Build the Folder Name and Path to the Folder.         */
               sprintf(Name, "%03uFOLDR", DirectoryNumber);
               sprintf(&CurrentPath[strlen(CurrentPath)], "%s", Name);

               /* Check to see if the Folder associated with the current*/
               /* directory number exists.                              */
               if((DirectoryHandle = opendir(CurrentPath)) != NULL)
               {
                  /* A Folder with the current directory number exists. */
                  /* Free the previously allocated Directory Handle.    */
                  closedir(DirectoryHandle);

                  /* Initialize the variables required to find the next */
                  /* next available file number in the current          */
                  /* directory.                                         */
                  CurrentPathWithDirectoryEndIndex = strlen(CurrentPath);
                  FileNumber                       = FILE_NUMBER_MINIMUM;
                  Done2                            = FALSE;

                  /* Loop through the files until the next valid image  */
                  /* handle is found.                                   */
                  while((!Done2) && (ret_val == 0))
                  {
                     /* Valid File Numbers are between                  */
                     /* FILE_NUMBER_MINIMUM and FILE_NUMBER_MAXIMUM.    */
                     /* check to see if the maximum file number has been*/
                     /* reached.                                        */
                     if(FileNumber <= FILE_NUMBER_MAXIMUM)
                     {
                        /* Build the File Name for this file number.    */
                        sprintf(Name, "IMGE%04u.JPG", FileNumber);
                        sprintf(&CurrentPath[strlen(CurrentPath)], "/%s", Name);

                        /* Check to see if a File with this name already*/
                        /* exists.                                      */
                        if((FileDescriptor = open(CurrentPath, (O_NONBLOCK | O_RDONLY), 0)) >= 0)
                        {
                           /* A File with the file number already       */
                           /* exists.  Free the opened File Descriptor. */
                           close(FileDescriptor);

                           /* Increment the Count of the number images  */
                           /* that exist.                               */
                           Count++;
                        }

                        /* Increment the File Number.                   */
                        FileNumber++;

                        /* Remove the Current File Name from the        */
                        /* currently build path.                        */
                        CurrentPath[CurrentPathWithDirectoryEndIndex] = '\0';
                     }
                     else
                     {
                        /* Invalid File Number increment the directory  */
                        /* number and exit the File Number search loop. */
                        DirectoryNumber++;
                        Done2 = TRUE;
                     }
                  }
               }
               else
               {
                  /* A directory with this folder number does not exist.*/
                  /* Increment to the next folder number.               */
                  DirectoryNumber++;
               }

               /* Remove the Current Folder Name from the currently     */
               /* build path.                                           */
               CurrentPath[CurrentPathEndIndex] = '\0';
            }
            else
            {
               /* All directories have been searched.  Set Done to have */
               /* the loop exit.                                        */
               Done = TRUE;
            }
         }

         /* Check to see if any valid image file were located.          */
         if(Count > 0)
         {
            /* Next attempt to allocate the memory required hold the    */
            /* entire image list.                                       */
            if((ImageList = (ImageListEntry_t *)malloc(sizeof(ImageListEntry_t)*Count)) != NULL)
            {
               /* Initialize the image list.                            */
               memset(ImageList, 0, sizeof(sizeof(ImageListEntry_t)*Count));

               /* Initialized the variables required to build the image */
               /* list.                                                 */
               DirectoryNumber        = DIRECTORY_NUMBER_MINIMUM;
               NumberImageListEntries = 0;
               ret_val                = 0;
               Done                   = FALSE;

               /* Loop through the directories and files until an image */
               /* handle is found.                                      */
               while((NumberImageListEntries < Count) && (!Done) && (ret_val == 0))
               {
                  /* Valid Directory Numbers are between                */
                  /* DIRECTORY_NUMBER_MINIMUM and                       */
                  /* DIRECTORY_NUMBER_MAXIMUM.  Check to see if the     */
                  /* maximum directory number has been reached.         */
                  if(DirectoryNumber <= DIRECTORY_NUMBER_MAXIMUM)
                  {
                     /* Build the Folder Name and Path to the Folder.   */
                     sprintf(Name, "%03uFOLDR", DirectoryNumber);
                     sprintf(&CurrentPath[strlen(CurrentPath)], "%s", Name);

                     /* Check to see if the Folder associated with the  */
                     /* current directory number exists.                */
                     if((DirectoryHandle = opendir(CurrentPath)) != NULL)
                     {
                        /* A Folder with the current directory number   */
                        /* exists.  Free the previously allocated       */
                        /* Directory Handle.                            */
                        closedir(DirectoryHandle);

                        /* Initialize the variables required to find the*/
                        /* next next available file number in the       */
                        /* current directory.                           */
                        CurrentPathWithDirectoryEndIndex = strlen(CurrentPath);
                        FileNumber                       = FILE_NUMBER_MINIMUM;
                        Done2                            = FALSE;

                        /* Loop through the files until the next valid  */
                        /* image handle is found.                       */
                        while((!Done2) && (ret_val == 0))
                        {
                           /* Valid File Numbers are between            */
                           /* FILE_NUMBER_MINIMUM and                   */
                           /* FILE_NUMBER_MAXIMUM.  check to see if the */
                           /* maximum file number has been reached.     */
                           if(FileNumber <= FILE_NUMBER_MAXIMUM)
                           {
                              /* Build the File Name for this file      */
                              /* number.                                */
                              sprintf(Name, "IMGE%04u.JPG", FileNumber);
                              sprintf(&CurrentPath[strlen(CurrentPath)], "/%s", Name);

                              if((!lstat(CurrentPath, &FileStat)) && (S_ISREG(FileStat.st_mode) || S_ISLNK(FileStat.st_mode)))
                              {
                                 /* Initialize the Image List Entry with*/
                                 /* this file data.                     */
                                 sprintf(ImageList[NumberImageListEntries].ImageHandle, "%03u%04u", DirectoryNumber, FileNumber);

                                 /* Convert the File Time.              */
                                 if((Time = localtime((time_t *)&(FileStat.st_ctime))) != NULL)
                                 {
                                    ImageList[NumberImageListEntries].CreatedTime.Year     = Time->tm_year + 1900;
                                    ImageList[NumberImageListEntries].CreatedTime.Month    = Time->tm_mon;
                                    ImageList[NumberImageListEntries].CreatedTime.Day      = Time->tm_mday;
                                    ImageList[NumberImageListEntries].CreatedTime.Hour     = Time->tm_hour;
                                    ImageList[NumberImageListEntries].CreatedTime.Minute   = Time->tm_min;
                                    ImageList[NumberImageListEntries].CreatedTime.Second   = Time->tm_sec;
                                    ImageList[NumberImageListEntries].CreatedTime.UTC_Time = FALSE;
                                 }

                                 /* Convert the File Time.              */
                                 if((Time = localtime((time_t *)&(FileStat.st_mtime))) != NULL)
                                 {
                                    ImageList[NumberImageListEntries].ModifiedTime.Year     = Time->tm_year + 1900;
                                    ImageList[NumberImageListEntries].ModifiedTime.Month    = Time->tm_mon;
                                    ImageList[NumberImageListEntries].ModifiedTime.Day      = Time->tm_mday;
                                    ImageList[NumberImageListEntries].ModifiedTime.Hour     = Time->tm_hour;
                                    ImageList[NumberImageListEntries].ModifiedTime.Minute   = Time->tm_min;
                                    ImageList[NumberImageListEntries].ModifiedTime.Second   = Time->tm_sec;
                                    ImageList[NumberImageListEntries].ModifiedTime.UTC_Time = FALSE;
                                 }

                                 /* Increment the Number of Image List  */
                                 /* Entries.                            */
                                 NumberImageListEntries++;
                              }

                              /* Increment the File Number.             */
                              FileNumber++;

                              /* Remove the Current File Name from the  */
                              /* currently build path.                  */
                              CurrentPath[CurrentPathWithDirectoryEndIndex] = '\0';
                           }
                           else
                           {
                              /* Invalid File Number increment the      */
                              /* directory number and exit the File     */
                              /* Number search loop.                    */
                              DirectoryNumber++;
                              Done2 = TRUE;
                           }
                        }
                     }
                     else
                     {
                        /* A directory with this folder number does not */
                        /* exist.  Increment to the next folder number. */
                        DirectoryNumber++;
                     }

                     /* Remove the Current Folder Name from the         */
                     /* currently build path.                           */
                     CurrentPath[CurrentPathEndIndex] = '\0';
                  }
                  else
                  {
                     /* All directories have been searched.  Set Done to*/
                     /* have the loop exit.                             */
                     Done = TRUE;
                  }
               }
            }
            else
               ret_val = APPLICATION_ERROR;
         }
         else
         {
            /* No entries found, set the return value to indicate       */
            /* success.                                                 */
            ret_val = 0;
         }
      }
      else
         ret_val = APPLICATION_ERROR;
   }
   else
      ret_val = APPLICATION_ERROR;

   return(ret_val);
}

   /* The following function is responsible for cleaning up (freeing all*/
   /* resources) of a previously initialized image list.                */
static void CleanupImageList(void)
{
   /* Check to see if the image list appear to be currently valid.      */
   if((NumberImageListEntries > 0) && (ImageList != NULL))
   {
      /* The image list appears to be currently valid.  Free the list   */
      /* entries and reset its state.                                   */
      free(ImageList);

      ImageList              = NULL;

      NumberImageListEntries = 0;
   }
}

   /* The following function is responsible for building an Image       */
   /* Descriptor Object.                                                */
static int BuildImageDescriptorObject(char *FileName, unsigned int *DataLength, unsigned char *DataBuffer)
{
   int     ret_val;
   char    PixelValueBuffer[32];
   char    SizeValueBuffer[32];
   Word_t  ImageHeight;
   Word_t  ImageWidth;
   DWord_t ImageFileSize;
   DWord_t ImageDescriptorLength;

   /* Check the required input parameters.                              */
   if((FileName != NULL) && (strlen(FileName) > 0) && (DataLength != NULL) && ((*DataLength == 0) || ((*DataLength > 0) && (DataBuffer != NULL))))
   {
      /* Attempt to get the image properties.                           */
      if((ret_val = GetFileImageProperties(FileName, &ImageWidth, &ImageHeight, &ImageFileSize)) == 0)
      {
         /* The information need to build an image descriptor object has*/
         /* been retrieved now determine the length required to build an*/
         /* image descriptor object.                                    */
         sprintf(PixelValueBuffer, "%u*%u", ImageWidth, ImageHeight);
         sprintf(SizeValueBuffer, "%u", (unsigned int)ImageFileSize);

         ImageDescriptorLength  = 0;
         ImageDescriptorLength += strlen(ImageDescriptorObjectHeader);
         ImageDescriptorLength += sizeof(_LESS_THAN_) + strlen(ImageDescriptorObjectImageElement) + sizeof(_SPACE_);
         ImageDescriptorLength += strlen(ImageDescriptorObjectEncodingAttribute) + sizeof(_EQUAL_) + sizeof(_QUOTE_) + strlen("JPEG") + sizeof(_QUOTE_) + sizeof(_SPACE_);
         ImageDescriptorLength += strlen(ImageDescriptorObjectPixelAttribute) + sizeof(_EQUAL_) + sizeof(_QUOTE_) + strlen(PixelValueBuffer) + sizeof(_QUOTE_) + sizeof(_SPACE_);
         ImageDescriptorLength += strlen(ImageDescriptorObjectSizeAttribute) + sizeof(_EQUAL_) + sizeof(_QUOTE_) + strlen(SizeValueBuffer) + sizeof(_QUOTE_);
         ImageDescriptorLength += sizeof(_FORWARD_SLASH_) + sizeof(_GREATER_THAN_);
         ImageDescriptorLength += strlen(ImageDescriptorObjectFooter);
         ImageDescriptorLength += sizeof(char);

         /* Check to see if a buffer large enough to build the object   */
         /* was specified.                                              */
         if(((*DataLength) >= ImageDescriptorLength) && (DataBuffer != NULL))
         {
            /* A buffer large enough to build the object was specified. */
            /* Attempt to build the Image Descriptor Object.            */
            strcpy((char *)DataBuffer, ImageDescriptorObjectHeader);
            sprintf((char *)&DataBuffer[strlen((char *)DataBuffer)], "%c%s%c", _LESS_THAN_, ImageDescriptorObjectImageElement, _SPACE_);
            sprintf((char *)&DataBuffer[strlen((char *)DataBuffer)], "%s%c%c%s%c%c", ImageDescriptorObjectEncodingAttribute, _EQUAL_, _QUOTE_, "JPEG", _QUOTE_, _SPACE_);
            sprintf((char *)&DataBuffer[strlen((char *)DataBuffer)], "%s%c%c%s%c%c", ImageDescriptorObjectPixelAttribute, _EQUAL_, _QUOTE_, PixelValueBuffer, _QUOTE_, _SPACE_);
            sprintf((char *)&DataBuffer[strlen((char *)DataBuffer)], "%s%c%c%s%c", ImageDescriptorObjectSizeAttribute, _EQUAL_, _QUOTE_, SizeValueBuffer, _QUOTE_);
            sprintf((char *)&DataBuffer[strlen((char *)DataBuffer)], "%c%c", _FORWARD_SLASH_, _GREATER_THAN_);
            sprintf((char *)&DataBuffer[strlen((char *)DataBuffer)], "%s", ImageDescriptorObjectFooter);

            /* Set the Data Length to the length of the buffer required */
            /* to build the object.                                     */
            (*DataLength) = ImageDescriptorLength;
         }
         else
         {
            /* Check to see if a buffer was specified to build into.    */
            if(DataBuffer == NULL)
            {
               /* A data buffer was not specified, so this must have    */
               /* been a call to determine the length required to build */
               /* the object.  Set the Data Length to the length of the */
               /* buffer required to build the object.                  */
               (*DataLength) = ImageDescriptorLength;
            }
            else
               ret_val = APPLICATION_ERROR;
         }
      }
   }
   else
      ret_val = APPLICATION_ERROR;

   return(ret_val);
}

   /* The following function is responsible for building an Image       */
   /* Listing Object.                                                   */
static int BuildImagesListingObject(unsigned int ImageListLength, ImageListEntry_t *ImageListPtr, DWord_t Offset, DWord_t NumberOfHandles, unsigned int *DataLength, unsigned char *DataBuffer)
{
   int          ret_val;
   char         UTCTimeBuffer[20];
   DWord_t      ImagesListingLength;
   unsigned int Index;

   /* Check the required input parameters.                              */
   if(((ImageListLength == 0) || ((ImageListLength > 0) && (ImageListPtr != NULL))) && (NumberOfHandles != 0) && (ImageListLength <= (Offset+NumberOfHandles)) && (DataLength != NULL) && ((*DataLength == 0) || ((*DataLength > 0) && (DataBuffer != NULL))))
   {
      ret_val              = 0;
      ImagesListingLength  = 0;
      ImagesListingLength += strlen(ImagesListingObjectHeader);

      for(Index=Offset;((Index<ImageListLength)&&(Index<(Offset+NumberOfHandles)));Index++)
      {
         ImagesListingLength += sizeof(_LESS_THAN_) + strlen(ImagesListingObjectImageElement) + sizeof(_SPACE_);
         ImagesListingLength += strlen(ImagesListingObjectHandleAttribute) + sizeof(_EQUAL_) + sizeof(_QUOTE_) + strlen(ImageListPtr[Index].ImageHandle) + sizeof(_QUOTE_) + sizeof(_SPACE_);

         sprintf(UTCTimeBuffer, "%04u%02u%02uT%02u%02u%02u%s", ImageListPtr[Index].CreatedTime.Year, ImageListPtr[Index].CreatedTime.Month, ImageListPtr[Index].CreatedTime.Day, ImageListPtr[Index].CreatedTime.Hour, ImageListPtr[Index].CreatedTime.Minute, ImageListPtr[Index].CreatedTime.Second, ((ImageListPtr[Index].CreatedTime.UTC_Time)?("Z"):("")));
         ImagesListingLength += strlen(ImagesListingObjectCreatedAttribute) + sizeof(_EQUAL_) + sizeof(_QUOTE_) + strlen(UTCTimeBuffer) + sizeof(_QUOTE_) + sizeof(_SPACE_);

         sprintf(UTCTimeBuffer, "%04u%02u%02uT%02u%02u%02u%s", ImageListPtr[Index].ModifiedTime.Year, ImageListPtr[Index].ModifiedTime.Month, ImageListPtr[Index].ModifiedTime.Day, ImageListPtr[Index].ModifiedTime.Hour, ImageListPtr[Index].ModifiedTime.Minute, ImageListPtr[Index].ModifiedTime.Second, ((ImageListPtr[Index].ModifiedTime.UTC_Time)?("Z"):("")));
         ImagesListingLength += strlen(ImagesListingObjectModifiedAttribute) + sizeof(_EQUAL_) + sizeof(_QUOTE_) + strlen(UTCTimeBuffer) + sizeof(_QUOTE_) + sizeof(_SPACE_);

         ImagesListingLength += sizeof(_FORWARD_SLASH_) + sizeof(_GREATER_THAN_);
      }

      ImagesListingLength += strlen(ImagesListingObjectFooter);
      ImagesListingLength += sizeof(char);

      /* Check to see if a buffer large enough to build the object was  */
      /* specified.                                                     */
      if(((*DataLength) >= ImagesListingLength) && (DataBuffer != NULL))
      {
         /* A buffer large enough to build the object was specified.    */
         /* Attempt to build the Images Listing Object.                 */
         strcpy((char *)DataBuffer, ImagesListingObjectHeader);

         for(Index=Offset;((Index<ImageListLength)&&(Index<(Offset+NumberOfHandles)));Index++)
         {
            sprintf((char *)&DataBuffer[strlen((char *)DataBuffer)], "%c%s%c", _LESS_THAN_, ImagesListingObjectImageElement, _SPACE_);
            sprintf((char *)&DataBuffer[strlen((char *)DataBuffer)], "%s%c%c%s%c%c", ImagesListingObjectHandleAttribute, _EQUAL_, _QUOTE_, ImageListPtr[Index].ImageHandle, _QUOTE_, _SPACE_);

            sprintf(UTCTimeBuffer, "%04u%02u%02uT%02u%02u%02u%s", ImageListPtr[Index].CreatedTime.Year, ImageListPtr[Index].CreatedTime.Month, ImageListPtr[Index].CreatedTime.Day, ImageListPtr[Index].CreatedTime.Hour, ImageListPtr[Index].CreatedTime.Minute, ImageListPtr[Index].CreatedTime.Second, ((ImageListPtr[Index].CreatedTime.UTC_Time)?("Z"):("")));
            sprintf((char *)&DataBuffer[strlen((char *)DataBuffer)], "%s%c%c%s%c%c", ImagesListingObjectCreatedAttribute, _EQUAL_, _QUOTE_, UTCTimeBuffer, _QUOTE_, _SPACE_);

            sprintf(UTCTimeBuffer, "%04u%02u%02uT%02u%02u%02u%s", ImageListPtr[Index].ModifiedTime.Year, ImageListPtr[Index].ModifiedTime.Month, ImageListPtr[Index].ModifiedTime.Day, ImageListPtr[Index].ModifiedTime.Hour, ImageListPtr[Index].ModifiedTime.Minute, ImageListPtr[Index].ModifiedTime.Second, ((ImageListPtr[Index].ModifiedTime.UTC_Time)?("Z"):("")));
            sprintf((char *)&DataBuffer[strlen((char *)DataBuffer)], "%s%c%c%s%c%c", ImagesListingObjectModifiedAttribute, _EQUAL_, _QUOTE_, UTCTimeBuffer, _QUOTE_, _SPACE_);

            sprintf((char *)&DataBuffer[strlen((char *)DataBuffer)], "%c%c", _FORWARD_SLASH_, _GREATER_THAN_);
         }

         sprintf((char *)&DataBuffer[strlen((char *)DataBuffer)], "%s", ImagesListingObjectFooter);

         /* Set the Data Length to the length of the buffer required to */
         /* build the object.                                           */
         (*DataLength) = ImagesListingLength;
      }
      else
      {
         /* Check to see if a buffer was specified to build into.       */
         if(DataBuffer == NULL)
         {
            /* A data buffer was not specified, so this must have been a*/
            /* call to determine the length required to build the       */
            /* object.  Set the Data Length to the length of the buffer */
            /* required to build the object.                            */
            (*DataLength) = ImagesListingLength;
         }
         else
            ret_val = APPLICATION_ERROR;
      }
   }
   else
      ret_val = APPLICATION_ERROR;

   return(ret_val);
}

   /* The following function is responsible for building and Image      */
   /* Properties Object.                                                */
static int BuildImagePropertiesObject(char *FileName, char *ImageHandle, unsigned int *DataLength, unsigned char *DataBuffer)
{
   int     ret_val;
   char    PixelValueBuffer[32];
   char    SizeValueBuffer[32];
   Word_t  ImageHeight;
   Word_t  ImageWidth;
   DWord_t ImageFileSize;
   DWord_t ImagePropertiesLength;

   /* Check the required input parameters.                              */
   if((FileName != NULL) && (strlen(FileName) > 0) && (ImageHandle != NULL) && (strlen(ImageHandle) == BIP_IMAGE_HANDLE_LENGTH) && (DataLength != NULL) && ((*DataLength == 0) || ((*DataLength > 0) && (DataBuffer != NULL))))
   {
      /* Attempt to get the image properties.                           */
      if((ret_val = GetFileImageProperties(FileName, &ImageWidth, &ImageHeight, &ImageFileSize)) == 0)
      {
         /* The information need to build an image properties object has*/
         /* been retrieved now determine the length required to build an*/
         /* image properties object.                                    */
         sprintf(PixelValueBuffer, "%u*%u", ImageWidth, ImageHeight);
         sprintf(SizeValueBuffer, "%u", (unsigned int)ImageFileSize);

         ImagePropertiesLength  = 0;
         ImagePropertiesLength += sizeof(_LESS_THAN_) + strlen(ImagePropertiesObjectImagePropertiesElement) + sizeof(_SPACE_);
         ImagePropertiesLength += strlen(ImagePropertiesObjectVersionAttribute) + sizeof(_EQUAL_) + sizeof(_QUOTE_) + strlen("1.0") + sizeof(_QUOTE_) + sizeof(_SPACE_);
         ImagePropertiesLength += strlen(ImagePropertiesObjectHandleAttribute) + sizeof(_EQUAL_) + sizeof(_QUOTE_) + strlen(ImageHandle) + sizeof(_QUOTE_) + sizeof(_GREATER_THAN_);
         ImagePropertiesLength += sizeof(_LESS_THAN_) + strlen(ImagePropertiesObjectNativeElement) + sizeof(_SPACE_);
         ImagePropertiesLength += strlen(ImagePropertiesObjectEncodingAttribute) + sizeof(_EQUAL_) + sizeof(_QUOTE_) + strlen("JPEG") + sizeof(_QUOTE_) + sizeof(_SPACE_);
         ImagePropertiesLength += strlen(ImagePropertiesObjectPixelAttribute) + sizeof(_EQUAL_) + sizeof(_QUOTE_) + strlen(PixelValueBuffer) + sizeof(_QUOTE_) + sizeof(_SPACE_);
         ImagePropertiesLength += strlen(ImagePropertiesObjectSizeAttribute) + sizeof(_EQUAL_) + sizeof(_QUOTE_) + strlen(SizeValueBuffer) + sizeof(_QUOTE_);
         ImagePropertiesLength += sizeof(_FORWARD_SLASH_) + sizeof(_GREATER_THAN_);
         ImagePropertiesLength += strlen(ImagePropertiesObjectFooter);
         ImagePropertiesLength += sizeof(char);

         /* Check to see if a buffer large enough to build the object   */
         /* was specified.                                              */
         if(((*DataLength) >= ImagePropertiesLength) && (DataBuffer != NULL))
         {
            /* A buffer large enough to build the object was specified. */
            /* Attempt to build the Image Properties Object.            */
            DataBuffer[0] = '\0';
            sprintf((char *)&DataBuffer[strlen((char *)DataBuffer)], "%c%s%c", _LESS_THAN_, ImagePropertiesObjectImagePropertiesElement, _SPACE_);
            sprintf((char *)&DataBuffer[strlen((char *)DataBuffer)], "%s%c%c%s%c%c", ImagePropertiesObjectVersionAttribute, _EQUAL_, _QUOTE_, "1.0", _QUOTE_, _SPACE_);
            sprintf((char *)&DataBuffer[strlen((char *)DataBuffer)], "%s%c%c%s%c%c", ImagePropertiesObjectHandleAttribute, _EQUAL_, _QUOTE_, ImageHandle, _QUOTE_, _GREATER_THAN_);
            sprintf((char *)&DataBuffer[strlen((char *)DataBuffer)], "%c%s%c", _LESS_THAN_, ImagePropertiesObjectNativeElement, _SPACE_);
            sprintf((char *)&DataBuffer[strlen((char *)DataBuffer)], "%s%c%c%s%c%c", ImagePropertiesObjectEncodingAttribute, _EQUAL_, _QUOTE_, "JPEG", _QUOTE_, _SPACE_);
            sprintf((char *)&DataBuffer[strlen((char *)DataBuffer)], "%s%c%c%s%c%c", ImagePropertiesObjectPixelAttribute, _EQUAL_, _QUOTE_, PixelValueBuffer, _QUOTE_, _SPACE_);
            sprintf((char *)&DataBuffer[strlen((char *)DataBuffer)], "%s%c%c%s%c", ImagePropertiesObjectSizeAttribute, _EQUAL_, _QUOTE_, SizeValueBuffer, _QUOTE_);
            sprintf((char *)&DataBuffer[strlen((char *)DataBuffer)], "%c%c", _FORWARD_SLASH_, _GREATER_THAN_);
            sprintf((char *)&DataBuffer[strlen((char *)DataBuffer)], "%s", ImagePropertiesObjectFooter);

            /* Set the Data Length to the length of the buffer required */
            /* to build the object.                                     */
            (*DataLength) = ImagePropertiesLength;
         }
         else
         {
            /* Check to see if a buffer was specified to build into.    */
            if(DataBuffer == NULL)
            {
               /* A data buffer was not specified, so this must have    */
               /* been a call to determine the length required to build */
               /* the object.  Set the Data Length to the length of the */
               /* buffer required to build the object.                  */
               (*DataLength) = ImagePropertiesLength;
            }
            else
               ret_val = APPLICATION_ERROR;
         }
      }
   }
   else
      ret_val = APPLICATION_ERROR;

   return(ret_val);
}

   /* The following function is responsible for building and Image      */
   /* Listing Object.                                                   */
static int ProcessImagesListingObject(unsigned int DataLength, unsigned char *DataBuffer, unsigned int *ImageListLength, ImageListEntry_t *ImageListPtr)
{
   int                         ret_val;
   int                         Index;
   int                         StartHandleIndex;
   int                         StartIndex;
   int                         EndIndex;
   Boolean_t                   Done;
   unsigned int                Count;
   TimeDate_t                  TimeDate;
   ProcessImagesListingState_t ProcessState;

   /* Check the required input parameters.                              */
   if((DataLength > 0) && (DataBuffer != NULL) && (ImageListLength != NULL) && (((*ImageListLength) == 0) || (((*ImageListLength) > 0) && (ImageListPtr != NULL))))
   {
      /* Initialize the required state variables.                       */
      ret_val      = 0;
      EndIndex     = 0;
      Count        = 0;
      Done         = FALSE;
      ProcessState = pilLookingHeader;

      /* Loop through the data buffer and process the images listing    */
      /* object.                                                        */
      while((((*ImageListLength) == 0) || (((*ImageListLength) > 0) && (Count < (*ImageListLength)))) && (!Done) && (ret_val == 0))
      {
         StartIndex = FindToken(_LESS_THAN_, EndIndex, DataLength, DataBuffer);
         EndIndex   = FindToken(_GREATER_THAN_, StartIndex, DataLength, DataBuffer);

         /* Check to see if a valid segment was located.                */
         if((StartIndex != APPLICATION_ERROR) && (EndIndex != APPLICATION_ERROR))
         {
            /* Determine the current processing state.                  */
            switch(ProcessState)
            {
               case pilLookingHeader:
                  /* Check to see if the segment is at least long enough*/
                  /* to be an Images Listing Header Segment.            */
                  if(((EndIndex+sizeof(char)) - StartIndex) >= strlen(ImagesListingObjectHeader))
                  {
                     /* The segment is at least long enough to be an    */
                     /* Images Listing Header Segment.  Look for the    */
                     /* first character of the images listing element in*/
                     /* the segment.                                    */
                     if((Index = FindToken(ImagesListingObjectImagesListingElement[0], StartIndex, EndIndex, DataBuffer)) != APPLICATION_ERROR)
                     {
                        /* The first character of the images listing    */
                        /* element was found.  Next check to see if this*/
                        /* is actually the images listing element       */
                        /* string.                                      */
                        if(((EndIndex - Index) >= (int)strlen(ImagesListingObjectImagesListingElement)) && (!memcmp(&DataBuffer[Index], ImagesListingObjectImagesListingElement, strlen(ImagesListingObjectImagesListingElement))))
                        {
                           /* A segment that looks like the header was  */
                           /* located, change the state to start looking*/
                           /* for body segments.                        */
                           ProcessState = pilLookingBody;
                        }
                     }
                  }
                  break;
               case pilLookingBody:
                  /* Check to see if the segment appears to be a body   */
                  /* segment that contains the image element.           */
                  if((Index = FindToken(ImagesListingObjectImageElement[0], StartIndex, EndIndex, DataBuffer)) != APPLICATION_ERROR)
                  {
                     if(((EndIndex - Index) >= (int)strlen(ImagesListingObjectImageElement)) && (!memcmp(&DataBuffer[Index], ImagesListingObjectImageElement, strlen(ImagesListingObjectImageElement))))
                     {
                        /* This appears to be a body segment that       */
                        /* contains a image element.  Check to see if   */
                        /* this image element contains a handle         */
                        /* attribute (it is required to).               */
                        if((Index = FindToken(ImagesListingObjectHandleAttribute[0], Index, EndIndex, DataBuffer)) != APPLICATION_ERROR)
                        {
                           if(((EndIndex - Index) >= (int)strlen(ImagesListingObjectHandleAttribute)) && (!memcmp(&DataBuffer[Index], ImagesListingObjectHandleAttribute, strlen(ImagesListingObjectHandleAttribute))))
                           {
                              /* This image element contains a handle   */
                              /* attribute, next make sure that it      */
                              /* appears to be valid.                   */
                              if((Index = FindToken(_EQUAL_, Index, EndIndex, DataBuffer)) != APPLICATION_ERROR)
                              {
                                 if((Index = FindToken(_QUOTE_, Index, EndIndex, DataBuffer)) != APPLICATION_ERROR)
                                 {
                                    if((StartHandleIndex = FindToken(_NON_SPACE_, Index+sizeof(char), EndIndex, DataBuffer)) != APPLICATION_ERROR)
                                    {
                                       if((Index = FindToken(_QUOTE_, StartHandleIndex, EndIndex, DataBuffer)) != APPLICATION_ERROR)
                                       {
                                          if((Index - StartHandleIndex) == BIP_IMAGE_HANDLE_LENGTH)
                                          {
                                             /* A valid handle attribute*/
                                             /* was located, check to   */
                                             /* see this is a call to   */
                                             /* calculate the number of */
                                             /* image list entries      */
                                             /* required or to actually */
                                             /* populate an image list. */
                                             if(ImageListPtr != NULL)
                                             {
                                                /* This is a call to    */
                                                /* actually populate the*/
                                                /* image list.  Populate*/
                                                /* the image list       */
                                                /* entries image handle.*/
                                                memcpy(ImageListPtr[Count].ImageHandle, &DataBuffer[StartHandleIndex], (sizeof(ImageListPtr[Count].ImageHandle) - sizeof(char)));
                                                ImageListPtr[Count].ImageHandle[sizeof(ImageListPtr[Count].ImageHandle) - sizeof(char)] = '\0';

                                                /* Check to see if this */
                                                /* image element        */
                                                /* contains a create    */
                                                /* attribute.           */
                                                if((Index = FindToken(ImagesListingObjectCreatedAttribute[0], Index, EndIndex, DataBuffer)) != APPLICATION_ERROR)
                                                {
                                                   if(((EndIndex - Index) >= (int)strlen(ImagesListingObjectCreatedAttribute)) && (!memcmp(&DataBuffer[Index], ImagesListingObjectCreatedAttribute, strlen(ImagesListingObjectCreatedAttribute))))
                                                   {
                                                      /* Attempt to     */
                                                      /* extract the    */
                                                      /* created        */
                                                      /* attribute.     */
                                                      Index += strlen(ImagesListingObjectCreatedAttribute);
                                                      if(ExtractTimeDate((char *)&DataBuffer[Index], &TimeDate) == 0)
                                                      {
                                                         /* Add the     */
                                                         /* created     */
                                                         /* information */
                                                         /* to the image*/
                                                         /* list.       */
                                                         ImageListPtr[Count].CreatedTime = TimeDate;
                                                      }
                                                   }
                                                }

                                                /* Check to see if this */
                                                /* image element        */
                                                /* contains a modified  */
                                                /* attribute.           */
                                                if((Index = FindToken(ImagesListingObjectModifiedAttribute[0], Index, EndIndex, DataBuffer)) != APPLICATION_ERROR)
                                                {
                                                   if(((EndIndex - Index) >= (int)strlen(ImagesListingObjectModifiedAttribute)) && (!memcmp(&DataBuffer[Index], ImagesListingObjectModifiedAttribute, strlen(ImagesListingObjectModifiedAttribute))))
                                                   {
                                                      /* Attempt to     */
                                                      /* extract the    */
                                                      /* modified       */
                                                      /* attribute.     */
                                                      Index += strlen(ImagesListingObjectModifiedAttribute);
                                                      if(ExtractTimeDate((char *)&DataBuffer[Index], &TimeDate) == 0)
                                                      {
                                                         /* Add the     */
                                                         /* modified    */
                                                         /* information */
                                                         /* to the image*/
                                                         /* list.       */
                                                         ImageListPtr[Count].ModifiedTime = TimeDate;
                                                      }
                                                   }
                                                }
                                             }

                                             /* Since at least a valid  */
                                             /* image handle was        */
                                             /* located, increment the  */
                                             /* count to the next entry */
                                             /* to be populated in the  */
                                             /* image list.             */
                                             Count++;
                                          }
                                       }
                                    }
                                 }
                              }
                           }
                        }
                     }
                  }

                  /* ** NOTE ** The break was left out here             */
                  /*            intentionally.  If this was not a       */
                  /*            recognized body segment it needs to be  */
                  /*            check to see if it is a footer segment. */
               case pilLookingFooter:
                  /* Check to see if the segment is at least long enough*/
                  /* to be an Images Listing Footer Segment.            */
                  if(((EndIndex+sizeof(char)) - StartIndex) >= strlen(ImagesListingObjectFooter))
                  {
                     /* The segment is at least long enough to be an    */
                     /* Images Listing Footer Segment.  Look for the    */
                     /* forward slash.                                  */
                     if((Index = FindToken(_FORWARD_SLASH_, StartIndex, EndIndex, DataBuffer)) != APPLICATION_ERROR)
                     {
                        /* The forward slash was found, now look for the*/
                        /* first character of the Images Listing Element*/
                        /* String.                                      */
                        if((Index = FindToken(ImagesListingObjectImagesListingElement[0], Index, EndIndex, DataBuffer)) != APPLICATION_ERROR)
                        {
                           /* The first character of the images listing */
                           /* element was found.  Next check to see if  */
                           /* this is actually the images listing       */
                           /* element string.                           */
                           if(((EndIndex - Index) >= (int)strlen(ImagesListingObjectImagesListingElement)) && (!memcmp(&DataBuffer[Index], ImagesListingObjectImagesListingElement, strlen(ImagesListingObjectImagesListingElement))))
                           {
                              /* The images listing footer was located, */
                              /* set done to TRUE to exit the processing*/
                              /* loop.                                  */
                              Done = TRUE;
                           }
                        }
                     }
                  }
                  break;
               default:
                  /* Unknown State.                                     */
                  ret_val = APPLICATION_ERROR;
                  break;
            }
         }
         else
            ret_val = APPLICATION_ERROR;
      }

      /* Check to see if an error occurred up until this point.         */
      if(ret_val == 0)
      {
         /* No error occurred up until this point.  Set the Image List  */
         /* Length to the number of entries that were existed in the    */
         /* Images Listing Object.                                      */
         (*ImageListLength) = Count;
      }
   }
   else
      ret_val = APPLICATION_ERROR;

   return(ret_val);
}

   /* The following function is responsible for actually opening a BIP  */
   /* Server with the specified parameters.                             */
static void BIPOpenServer(unsigned int ServerPort, unsigned long ServicesMask)
{
   int     Result;
   char    ServiceName[132];
   Word_t  SupportedFeaturesMask;
   DWord_t SupportedFunctionsMask;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the Server Port ID passed to this  */
      /* function is valid.                                             */
      if(ServerPort)
      {
         /* Now check to make sure that a Server doesn't already exist. */
         if(!BIPID)
         {
            /* Now try and open a BIP Server.                           */
            Result = BIP_Open_Imaging_Responder_Server(BluetoothStackID, ServerPort, ServicesMask, BIP_Event_Callback, 0);

            /* Check to see if the call was executed successfully.      */
            if(Result > 0)
            {
               /* The Server was successfully opened.  Save the returned*/
               /* result as the BIP ID because it will be used by later */
               /* function calls.                                       */
               BIPID            = Result;

               Connected        = FALSE;
               CurrentOperation = coNone;

               printf("BIP_Open_Imaging_Responder_Server: Function Successful.\r\n");

               /* The Server was opened successfully, now register a SDP*/
               /* Record indicating that a Basic Imaging Server exists. */
               /* Do this by first creating a Service Name.             */
               sprintf(ServiceName, "Imaging Port %u", ServerPort);

               /* Initialize the Supported Features and Supported       */
               /* Functions Mask.                                       */
               SupportedFeaturesMask  = 0;
               SupportedFunctionsMask = 0;

               /* Check to see if the Service Mask indicates the Push   */
               /* Service is supported.                                 */
               if(ServicesMask & BIP_IMAGING_SERVICE_SERVER_PORT_TARGET_IMAGE_PUSH_SERVICE_BIT)
               {
                  SupportedFeaturesMask |= BIP_SUPPORTED_FEATURE_IMAGE_PUSH;

                  SupportedFunctionsMask |= BIP_SUPPORTED_FUNCTION_GET_CAPABILITIES;
                  SupportedFunctionsMask |= BIP_SUPPORTED_FUNCTION_PUT_IMAGE;
                  SupportedFunctionsMask |= BIP_SUPPORTED_FUNCTION_PUT_LINKED_THUMBNAIL;
               }

               /* Check to see if the Service Mask indicates that Pull  */
               /* Service is supported.                                 */
               if(ServicesMask & BIP_IMAGING_SERVICE_SERVER_PORT_TARGET_IMAGE_PULL_SERVICE_BIT)
               {
                  SupportedFeaturesMask |= BIP_SUPPORTED_FEATURE_IMAGE_PULL;

                  SupportedFunctionsMask |= BIP_SUPPORTED_FUNCTION_GET_IMAGES_LIST;
                  SupportedFunctionsMask |= BIP_SUPPORTED_FUNCTION_GET_IMAGE_PROPERTIES;
                  SupportedFunctionsMask |= BIP_SUPPORTED_FUNCTION_GET_IMAGE;
                  SupportedFunctionsMask |= BIP_SUPPORTED_FUNCTION_GET_LINKED_THUMBNAIL;
                  SupportedFunctionsMask |= BIP_SUPPORTED_FUNCTION_DELETE_IMAGE;
               }

               /* Now that a Service Name has been created try and      */
               /* Register the SDP Record.                              */
               Result = BIP_Register_Imaging_Responder_Service_SDP_Record(BluetoothStackID, BIPID, ServiceName, BIP_SUPPORTED_CAPABILITY_GENERIC_IMAGING, SupportedFeaturesMask, SupportedFunctionsMask, 0, 0xFFFFFFFF, &ServiceRecordHandle);

               /* Check the result of the above function call for       */
               /* success.                                              */
               if(!Result)
               {
                  /* Display a message indicating that the SDP record   */
                  /* for the Basic Imaging Server was registered        */
                  /* successfully.                                      */
                  printf("BIP_Register_Imaging_Responder_Service_SDP_Record: Function Successful.\r\n");
               }
               else
               {
                  /* Display an Error Message and make sure the Current */
                  /* Server SDP Handle is invalid, and close the BIP    */
                  /* Server Port we just opened because we weren't able */
                  /* to register a SDP Record.                          */
                  printf("BIP_Register_Imaging_Responder_Service_SDP_Record: Function Failure.\r\n");
                  ServiceRecordHandle = 0;

                  /* Now try and close the opened Server.               */
                  Result = BIP_Close_Server(BluetoothStackID, BIPID);
                  BIPID  = 0;

                  /* Next check the return value of the issued command  */
                  /* see if it was successful.                          */
                  if(!Result)
                  {
                     /* Display a message indicating that the Server was*/
                     /* successfully closed.                            */
                     printf("BIP_Close_Server: Function Successful.\r\n");
                  }
                  else
                  {
                     /* An error occurred while attempting to close the */
                     /* Server.                                         */
                     printf("BIP_Close_Server() Failure: %d.\r\n", Result);
                  }
               }
            }
            else
            {
               /* There was an error while trying to open the BIP       */
               /* Server.                                               */
               printf("BIP_Open_Imaging_Responder_Server() Failure: %d.\r\n", Result);
            }
         }
         else
         {
            /* A Server is already open, this program only supports one */
            /* Server or Client at a time.                              */
            printf("Port already open.\r\n");
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("Open Basic Imaging Profile Server: Invalid Parameter.\r\n");
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      printf("Stack ID Invalid.\r\n");
   }
}

   /* The following function is responsible for opening a Basic Imaging */
   /* Profile Client connection to the specified Remote Server .        */
static void BIPOpenRemoteServer(BD_ADDR_t BD_ADDR, unsigned int ServerPort, BIP_Imaging_Service_Server_Port_Target_t ServiceTarget)
{
   int        Result;
   BD_ADDR_t  NullADDR;
   char       BoardStr[32];

   ASSIGN_BD_ADDR(NullADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the Server Port ID passed to this  */
      /* function is valid.                                             */
      if(ServerPort)
      {
         /* Now check to make sure that a Client doesn't already exist. */
         if(!BIPID)
         {
            if((!COMPARE_BD_ADDR(BD_ADDR, NullADDR)))
            {
               BD_ADDRToStr(BD_ADDR, (char *)BoardStr);

               /* The above parameters are valid, inform the user that  */
               /* the program is about to open a connection to the      */
               /* Remote Server.                                        */
               printf("Open Remote Basic Imaging Profile Server(BD_ADDR = %s, Port = %04X)\r\n", BoardStr, ServerPort);

               /* Attempt to open a connection to the selected remote   */
               /* Basic Imaging Profile Server.                         */
               Result = BIP_Open_Remote_Imaging_Responder_Server(BluetoothStackID, BD_ADDR, ServerPort, ServiceTarget, BIP_Event_Callback, 0);

               /* Now check to see if the function call was successfully*/
               /* made.                                                 */
               if(Result > 0)
               {
                  /* The Client was opened successfully.  The return    */
                  /* value of the call is the BIP ID and is required for*/
                  /* future Basic Imaging Profile calls.                */
                  BIPID            = Result;

                  CurrentOperation = coNone;

                  printf("BIP_Open_Remote_Imaging_Responder_Server: Function Successful (ID = %04X).\r\n", BIPID);
               }
               else
               {
                  /* There was an error opening the Client.             */
                  printf("BIP_Open_Remote_Imaging_Responder_Server() Failure: %d.\r\n", Result);
               }
            }
            else
            {
               /* The BD_ADDR retrieved from the list is invalid.       */
               printf("Open Remote Server: Invalid BD_ADDR.\r\n");
            }
         }
         else
         {
            /* A Connection is already open, this program only supports */
            /* one Server or Client at a time.                          */
            printf("Port already open.\r\n");
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("Open Remote Server: Invalid Parameter.\r\n");
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      printf("Stack ID Invalid.\r\n");
   }
}

   /* The following function is responsible for closing any existing    */
   /* Basic Imaging Profile Server on the Local Device.                 */
static void BIPCloseServer(void)
{
   int Result;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Check to see if the Server SDP Record Handle appears to be     */
      /* valid.                                                         */
      if(ServiceRecordHandle)
      {
         /* The SDP Record Handle is valid so remove the Record from the*/
         /* SDP Database.                                               */
         Result = BIP_Un_Register_SDP_Record(BluetoothStackID, BIPID, ServiceRecordHandle);

         /* Check the result of the above function call for success.    */
         if(!Result)
         {
            /* The function was called successfully.  Display a message */
            /* indicating that the SDP Record was successfully removed. */
            printf("BIP_Un_Register_SDP_Record: Function Successful.\r\n");

            ServiceRecordHandle = 0;
         }
         else
         {
            /* An error occurred while attempting to Un-Register the SDP*/
            /* Record.                                                  */
            printf("BIP_Un_Register_SDP_Record() Failure: %d.\r\n", Result);
         }
      }
      else
      {
         /* The Current Server SDP Handle is invalid so the Record must */
         /* already have been removed or never installed.               */
         printf("Invalid SDP Record Handle: BIP Close Server.\r\n");
      }

      /* Now check to see if the BIPID appears to be semi-valid.  This  */
      /* will indicate if a Server is open.                             */
      if(BIPID)
      {
         /* The Current BIPID appears to be valid so go ahead and try   */
         /* and close the Server.                                       */
         Result = BIP_Close_Server(BluetoothStackID, BIPID);

         /* Check the result of the above function call for success.    */
         if(!Result)
         {
            /* The function was called successfully.  Display a message */
            /* indicating that the Server was successfully closed.      */
            printf("BIP_Close_Server: Function Successful.\r\n");

            BIPID            = 0;
            Connected        = FALSE;
            CurrentOperation = coNone;

            /* Cleanup the image list if it is currently initialized.   */
            CleanupImageList();

            /* Reset all other state variables.                         */
            CurrentImageHandle[0]  = '\0';
            FirstPhase             = TRUE;
            CurrentFile[0]         = '\0';
            CurrentFileIndex       = 0;
            CurrentFileSize        = 0;

            CurrentFileBufferIndex = 0;

            CurrentObjectIndex     = 0;
            CurrentObjectLength    = 0;

            if(CurrentObjectBuffer != NULL)
            {
               if(CurrentObjectBufferAllocated)
                  free(CurrentObjectBuffer);

               CurrentObjectBuffer          = NULL;
               CurrentObjectBufferAllocated = FALSE;
            }
         }
         else
         {
            /* An error occurred while attempting to close the Server.  */
            printf("BIP_Close_Server() Failure: %d.\r\n", Result);
         }
      }
      else
      {
         /* The Current BIPID is invalid so no Server must be open.     */
         printf("Invalid BIPID: BIP Close Server.\r\n");
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      printf("Stack ID Invalid.\r\n");
   }
}

   /* The following function is responsible for closing a connection to */
   /* an Open Basic Imaging Profile Server.                             */
static void BIPCloseConnection(void)
{
   int Result;

   /* First check to see if a valid Bluetooth Stack exists.             */
   if(BluetoothStackID)
   {
      /* Check to see if the BIPID appears to be semi-valid.  This      */
      /* parameter will only be valid if a Connection is open or a      */
      /* Server is Open.                                                */
      if(BIPID)
      {
         /* The Current BIPID appears to be semi-valid.  Now try to     */
         /* close the connection.                                       */
         Result = BIP_Close_Connection(BluetoothStackID, BIPID);

         if(!Result)
         {
            /* The function was called successfully.  Display a message */
            /* indicating that the Connection Close was successfully    */
            /* submitted.                                               */
            printf("BIP_Close_Connection: Function Successful.\r\n");

            /* If this is a Client Connection invalidate the BIP ID.    */
            if(IsClient)
               BIPID = 0;

            Connected        = FALSE;
            CurrentOperation = coNone;

            /* Cleanup the image list if it is currently initialized.   */
            CleanupImageList();

            /* Reset all other state variables.                         */
            CurrentImageHandle[0]  = '\0';
            FirstPhase             = TRUE;
            CurrentFile[0]         = '\0';
            CurrentFileIndex       = 0;
            CurrentFileSize        = 0;

            CurrentFileBufferIndex = 0;

            CurrentObjectIndex     = 0;
            CurrentObjectLength    = 0;

            if(CurrentObjectBuffer != NULL)
            {
               if(CurrentObjectBufferAllocated)
                  free(CurrentObjectBuffer);

               CurrentObjectBuffer          = NULL;
               CurrentObjectBufferAllocated = FALSE;
            }
         }
         else
         {
            /* An error occurred while attempting to close the          */
            /* Connection.                                              */
            printf("BIP_Close_Connection() Failure: %d.\r\n", Result);
         }
      }
      else
      {
         /* The Current BIPID is invalid so no Connection or Server is  */
         /* open.                                                       */
         printf("Invalid BIP ID: BIP Close Connection.\r\n");
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      printf("Stack ID Invalid.\r\n");
   }
}

   /* The following function is responsible for submitting an Abort     */
   /* Request to the currently connected Remote Basic Imaging Server.   */
static void BIPAbortRequest(void)
{
   int Result;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the BIP ID appears to be           */
      /* semi-valid.                                                    */
      if(BIPID)
      {
         /* The BIP ID appears to be is a semi-valid value.  Now submit */
         /* the command to begin this operation.                        */
         Result = BIP_Abort_Request(BluetoothStackID, BIPID);

         if(!Result)
         {
            /* The function was submitted successfully.                 */

            /* Update the Current Operation.                            */
            CurrentOperation = coAbort;

            printf("BIP_Abort_Request: Function Successful.\r\n");
         }
         else
         {
            /* There was an error submitting the function.              */
            printf("BIP_Abort_Request() Failure: %d.\r\n", Result);
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("BIP Abort Request: Invalid BIPID.\r\n");
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      printf("Stack ID Invalid.\r\n");
   }
}

   /* The following function is responsible for submitting a Get        */
   /* Capabilities Object Request to the currently connected Remote     */
   /* Basic Imaging Server.                                             */
static void BIPGetCapabilitiesRequest(void)
{
   int Result;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the BIP ID appears to be           */
      /* semi-valid.                                                    */
      if(BIPID)
      {
         /* Check to make sure there isn't already a current operation. */
         if(CurrentOperation == coNone)
         {
            /* Now submit the command to begin this operation.          */
            Result = BIP_Get_Capabilities_Request(BluetoothStackID, BIPID);

            if(!Result)
            {
               /* The function was submitted successfully.              */

               /* Update the Current Operation.                         */
               CurrentOperation = coGetCapabilities;

               /* Reset the state variables.                            */
               CurrentObjectIndex           = 0;
               CurrentObjectLength          = 0;
               CurrentObjectBuffer          = NULL;
               CurrentObjectBufferAllocated = FALSE;

               printf("BIP_Get_Capabilities_Request: Function Successful.\r\n");
            }
            else
            {
               /* There was an error submitting the function.           */
               printf("BIP_Get_Capabilities_Request() Failure: %d.\r\n", Result);
            }
         }
         else
         {
            /* One or more of the parameters are invalid.               */
            printf("BIP Get Capabilities Request: Currently Outstanding Operation.\r\n");
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("BIP Get Capabilities Request: Invalid BIPID.\r\n");
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      printf("Stack ID Invalid.\r\n");
   }
}

   /* The following function is responsible for submitting a Put Image  */
   /* Request to the currently connected Remote Basic Imaging Server.   */
static void BIPPutImageRequest(char *Path, char *ImageName)
{
   int          Result;
   unsigned int AmountWritten;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the BIP ID appears to be           */
      /* semi-valid.                                                    */
      if(BIPID)
      {
         /* Check to make sure that the parameters passed in appear to  */
         /* be valid.                                                   */
         if((Path != NULL) && (strlen(Path) > 0) && (strlen(Path) < sizeof(CurrentFile)) && (ImageName != NULL) && (strlen(ImageName) > 0))
         {
            /* Check to make sure there isn't already a current         */
            /* operation.                                               */
            if(CurrentOperation == coNone)
            {
               /* There is currently not an on going operation.  Get the*/
               /* length required to build the Image Descriptor for the */
               /* selected Image.                                       */
               CurrentObjectLength = 0;
               if(BuildImageDescriptorObject(Path, &CurrentObjectLength, NULL) == 0)
               {
                  /* The Length was successfully determined, next       */
                  /* attempt to allocate a buffer to build the object   */
                  /* into.                                              */
                  if((CurrentObjectBuffer = (unsigned char *)malloc(CurrentObjectLength)) != NULL)
                  {
                     /* Now attempt to build the Image Descriptor       */
                     /* Object.                                         */
                     if(BuildImageDescriptorObject(Path, &CurrentObjectLength, CurrentObjectBuffer) == 0)
                     {
                        /* The Image Descriptor Object has been         */
                        /* successfully build.  Now initialize the      */
                        /* current file variables.                      */
                        FirstPhase             = TRUE;
                        CurrentFileIndex       = 0;
                        CurrentFileSize        = 0;
                        CurrentFileBufferIndex = 0;

                        strcpy(CurrentFile, Path);

                        /* Get some file data to send.                  */
                        if(GetFileData() == FILE_DATA_FILE_IO_SUCCESS)
                        {
                           Result = BIP_Put_Image_Request(BluetoothStackID, BIPID, ImageName, CurrentObjectLength, CurrentObjectBuffer, CurrentFileBufferIndex, CurrentFileBuffer, &AmountWritten, FALSE);

                           if(!Result)
                           {
                              /* The function was submitted             */
                              /* successfully.                          */

                              /* Update the Current Operation.          */
                              CurrentOperation = coPutImage;

                              /* Adjust the current file buffer.        */
                              if(AmountWritten)
                              {
                                 memmove(CurrentFileBuffer, &CurrentFileBuffer[AmountWritten], (CurrentFileBufferIndex - AmountWritten));
                                 CurrentFileBufferIndex -= AmountWritten;
                              }

                              printf("BIP_Put_Image_Request: Function Successful.\r\n");
                           }
                           else
                           {
                              /* There was an error submitting the      */
                              /* function.                              */
                              printf("BIP_Put_Image_Request() Failure: %d.\r\n", Result);
                           }
                        }
                        else
                        {
                           /* Unable to get file data.                  */
                           printf("BIP Put Image Request: Unable to Get File Data.\r\n");
                        }
                     }

                     /* Free the built Image Descriptor Object.  It is  */
                     /* sent as a header in the first transaction of    */
                     /* this operation and it not need after that.      */
                     free(CurrentObjectBuffer);

                     CurrentObjectBuffer          = NULL;
                     CurrentObjectLength          = 0;
                     CurrentObjectBufferAllocated = FALSE;
                  }
                  else
                  {
                     /* Unable to allocated the Image Descriptor Object */
                     /* Buffer, display an error message.               */
                     printf("BIP Put Image Request: Unable to allocated Image Descriptor Buffer.\r\n");
                  }
               }
               else
               {
                  /* Unable to build Image Descriptor Object, display an*/
                  /* error message.                                     */
                  printf("BIP Put Image Request: Unable to Build Image Descriptor Object.\r\n");
               }
            }
            else
            {
               /* One or more of the parameters are invalid.            */
               printf("BIP Put Image Request: Currently Outstanding Operation.\r\n");
            }
         }
         else
         {
            /* One or more of the parameters passed in are invalid.     */
            printf("BIP Put Image Request: Invalid parameter.\r\n");
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("BIP Put Image Request: Invalid BIPID.\r\n");
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      printf("Stack ID Invalid.\r\n");
   }
}

   /* The following function is responsible for submitting a Put Linked */
   /* Thumbnail Image Request to the currently connected Remote Basic   */
   /* Imaging Server.                                                   */
static void BIPPutLinkedThumbnailRequest(char *Path, char *ImageHandle)
{
   int          Result;
   unsigned int AmountWritten;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the BIP ID appears to be           */
      /* semi-valid.                                                    */
      if(BIPID)
      {
         /* Check to make sure that the parameters passed in appear to  */
         /* be valid.                                                   */
         if((Path != NULL) && (strlen(Path) > 0) && (strlen(Path) < sizeof(CurrentFile)) && (ImageHandle != NULL) && (strlen(ImageHandle) == BIP_IMAGE_HANDLE_LENGTH))
         {
            /* Check to make sure there isn't already a current         */
            /* operation.                                               */
            if(CurrentOperation == coNone)
            {
               /* Now initialize the current file variables.            */
               FirstPhase             = TRUE;
               CurrentFileIndex       = 0;
               CurrentFileSize        = 0;
               CurrentFileBufferIndex = 0;

               strcpy(CurrentFile, Path);

               /* Get some file data to send.                           */
               if(GetFileData() == FILE_DATA_FILE_IO_SUCCESS)
               {
                  Result = BIP_Put_Linked_Thumbnail_Request(BluetoothStackID, BIPID, ImageHandle, CurrentFileBufferIndex, CurrentFileBuffer, &AmountWritten, FALSE);

                  if(!Result)
                  {
                     /* The function was submitted successfully.        */

                     /* Update the Current Operation.                   */
                     CurrentOperation = coPutLinkedThumbnail;

                     /* Adjust the current file buffer.                 */
                     if(AmountWritten)
                     {
                        memmove(CurrentFileBuffer, &CurrentFileBuffer[AmountWritten], (CurrentFileBufferIndex - AmountWritten));
                        CurrentFileBufferIndex -= AmountWritten;
                     }

                     printf("BIP_Put_Linked_Thumbnail_Request: Function Successful.\r\n");
                  }
                  else
                  {
                     /* There was an error submitting the function.     */
                     printf("BIP_Put_Linked_Thumbnail_Request() Failure: %d.\r\n", Result);
                  }
               }
               else
               {
                  /* Unable to get file data.                           */
                  printf("BIP Put Linked Thumbnail Request: Unable to Get File Data.\r\n");
               }
            }
            else
            {
               /* One or more of the parameters are invalid.            */
               printf("BIP Put Linked Thumbnail Request: Currently Outstanding Operation.\r\n");
            }
         }
         else
         {
            /* One or more of the parameters passed in are invalid.     */
            printf("BIP Put Linked Thumbnail Request: Invalid parameter.\r\n");
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("BIP Put Linked Thumbnail Request: Invalid BIPID.\r\n");
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      printf("Stack ID Invalid.\r\n");
   }
}

   /* The following function is responsible for submitting a Get Image  */
   /* List Request to the currently connected Remote Basic Imaging      */
   /* Server.                                                           */
static void BIPGetImagesListRequest(void)
{
   int Result;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the BIP ID appears to be           */
      /* semi-valid.                                                    */
      if(BIPID)
      {
         /* Check to make sure there isn't already a current operation. */
         if(CurrentOperation == coNone)
         {
            /* Now submit the command to begin this operation.          */
            Result = BIP_Get_Images_List_Request(BluetoothStackID, BIPID, BIP_NUMBER_OF_RETURNED_HANDLES_MAXIMUM_VALUE, BIP_LIST_START_OFFSET_MINIMUM_VALUE, BIP_LATEST_CAPTURED_IMAGES_FALSE_VALUE, (sizeof(ImageHandlesDescriptorObject)/sizeof(char)), (unsigned char *)ImageHandlesDescriptorObject);

            if(!Result)
            {
               /* The function was submitted successfully.              */

               /* Update the Current Operation.                         */
               CurrentOperation = coGetImagesList;

               /* Reset the state variables.                            */
               CurrentObjectIndex           = 0;
               CurrentObjectLength          = 0;
               CurrentObjectBuffer          = NULL;
               CurrentObjectBufferAllocated = FALSE;

               /* If a prior Image List existed, we will go ahead and   */
               /* free the resources that were allocated.               */
               if((NumberImageListEntries) && (ImageList))
               {
                  free(ImageList);

                  NumberImageListEntries = 0;
                  ImageList              = NULL;
               }

               printf("BIP_Get_Images_List_Request: Function Successful.\r\n");
            }
            else
            {
               /* There was an error submitting the function.           */
               printf("BIP_Get_Images_List_Request() Failure: %d.\r\n", Result);
            }
         }
         else
         {
            /* One or more of the parameters are invalid.               */
            printf("BIP Get Image List Request: Currently Outstanding Operation.\r\n");
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("BIP Get Image List Request: Invalid BIPID.\r\n");
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      printf("Stack ID Invalid.\r\n");
   }
}

   /* The following function is responsible for submitting a Get Image  */
   /* Properties Request to the currently connected Remote Basic Imaging*/
   /* Server.                                                           */
static void BIPGetImagePropertiesRequest(char *ImageHandle)
{
   int Result;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the BIP ID appears to be           */
      /* semi-valid.                                                    */
      if(BIPID)
      {
         /* Check to make sure that the parameters passed in appear to  */
         /* be valid.                                                   */
         if((ImageHandle != NULL) && (strlen(ImageHandle) == BIP_IMAGE_HANDLE_LENGTH))
         {
            /* Check to make sure there isn't already a current         */
            /* operation.                                               */
            if(CurrentOperation == coNone)
            {
               /* The BIP ID appears to be is a semi-valid value.  Now  */
               /* submit the command to begin this operation.           */
               Result = BIP_Get_Image_Properties_Request(BluetoothStackID, BIPID, ImageHandle);

               if(!Result)
               {
                  /* The function was submitted successfully.           */

                  /* Update the Current Operation.                      */
                  CurrentOperation = coGetImageProperties;

                  /* Reset the state variables.                         */
                  CurrentObjectIndex           = 0;
                  CurrentObjectLength          = 0;
                  CurrentObjectBuffer          = NULL;
                  CurrentObjectBufferAllocated = FALSE;

                  printf("BIP_Get_Image_Properties_Request: Function Successful.\r\n");
               }
               else
               {
                  /* There was an error submitting the function.        */
                  printf("BIP_Get_Image_Properties_Request() Failure: %d.\r\n", Result);
               }
            }
            else
            {
               /* One or more of the parameters are invalid.            */
               printf("BIP Get Image Properties Request: Currently Outstanding Operation.\r\n");
            }
         }
         else
         {
            /* One or more of the parameters passed in are invalid.     */
            printf("BIP Get Image Properties Request: Invalid parameter.\r\n");
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("BIP Get Image Properties Request: Invalid BIPID.\r\n");
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      printf("Stack ID Invalid.\r\n");
   }
}

   /* The following function is responsible for submitting a Get Image  */
   /* Request to the currently connected Remote Basic Imaging Server.   */
static void BIPGetImageRequest(char *Path, char *ImageHandle)
{
   int Result;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the BIP ID appears to be           */
      /* semi-valid.                                                    */
      if(BIPID)
      {
         /* Check to make sure that the parameters passed in appear to  */
         /* be valid.                                                   */
         if((Path != NULL) && (strlen(Path) > 0) && (strlen(Path) < sizeof(CurrentFile)) && (ImageHandle != NULL) && (strlen(ImageHandle) == BIP_IMAGE_HANDLE_LENGTH))
         {
            /* Check to make sure there isn't already a current         */
            /* operation.                                               */
            if(CurrentOperation == coNone)
            {
               /* Reset the current file index and set that this is the */
               /* first phase.                                          */
               CurrentFileIndex = 0;
               FirstPhase       = TRUE;

               /* Set the Current File to the save as path specifed.    */
               strcpy(CurrentFile, Path);

               /* Attempt to submit the request.                        */
               Result = BIP_Get_Image_Request(BluetoothStackID, BIPID, ImageHandle, sizeof(EmptyImageDescriptorObject)/sizeof(char), (unsigned char *)EmptyImageDescriptorObject);

               if(!Result)
               {
                  /* The function was submitted successfully.           */

                  /* Update the Current Operation.                      */
                  CurrentOperation = coGetImage;

                  printf("BIP_Get_Image_Request: Function Successful.\r\n");
               }
               else
               {
                  /* There was an error submitting the function.        */
                  printf("BIP_Get_Image_Request() Failure: %d.\r\n", Result);
               }
            }
            else
            {
               /* One or more of the parameters are invalid.            */
               printf("BIP Get Image Request: Currently Outstanding Operation.\r\n");
            }
         }
         else
         {
            /* One or more of the parameters passed in are invalid.     */
            printf("BIP Get Image Request: Invalid parameter.\r\n");
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("BIP Get Image Request: Invalid BIPID.\r\n");
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      printf("Stack ID Invalid.\r\n");
   }
}

   /* The following function is responsible for submitting a Get Linked */
   /* Thumbnail Image Request to the currently connected Remote Basic   */
   /* Imaging Server.                                                   */
static void BIPGetLinkedThumbnailRequest(char *Path, char *ImageHandle)
{
   int Result;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the BIP ID appears to be           */
      /* semi-valid.                                                    */
      if(BIPID)
      {
         /* Check to make sure that the parameters passed in appear to  */
         /* be valid.                                                   */
         if((Path != NULL) && (strlen(Path) > 0) && (strlen(Path) < sizeof(CurrentFile)) && (ImageHandle != NULL) && (strlen(ImageHandle) == BIP_IMAGE_HANDLE_LENGTH))
         {
            /* Check to make sure there isn't already a current         */
            /* operation.                                               */
            if(CurrentOperation == coNone)
            {
               /* Reset the current file index and set that this is the */
               /* first phase.                                          */
               CurrentFileIndex = 0;
               FirstPhase       = TRUE;

               /* Set the Current File to the save as path specifed.    */
               strcpy(CurrentFile, Path);

               /* Attempt to submit the request.                        */
               Result = BIP_Get_Linked_Thumbnail_Request(BluetoothStackID, BIPID, ImageHandle);

               if(!Result)
               {
                  /* The function was submitted successfully.           */

                  /* Update the Current Operation.                      */
                  CurrentOperation = coGetLinkedThumbnail;

                  printf("BIP_Get_Linked_Thumbnail_Request: Function Successful.\r\n");
               }
               else
               {
                  /* There was an error submitting the function.        */
                  printf("BIP_Get_Linked_Thumbnail_Request() Failure: %d.\r\n", Result);
               }
            }
            else
            {
               /* One or more of the parameters are invalid.            */
               printf("BIP Get Linked Thumbnail Request: Currently Outstanding Operation.\r\n");
            }
         }
         else
         {
            /* One or more of the parameters passed in are invalid.     */
            printf("BIP Get Linked Thumbnail Request: Invalid parameter.\r\n");
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("BIP Get Linked Thumbnail Request: Invalid BIPID.\r\n");
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      printf("Stack ID Invalid.\r\n");
   }
}

   /* The following function is responsible for submitting a Delete     */
   /* Image Request to the currently connected Remote Basic Imaging     */
   /* Server.                                                           */
static void BIPDeleteImageRequest(char *ImageHandle)
{
   int Result;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the BIP ID appears to be           */
      /* semi-valid.                                                    */
      if(BIPID)
      {
         /* Check to make sure that the parameters passed in appear to  */
         /* be valid.                                                   */
         if((ImageHandle != NULL) && (strlen(ImageHandle) == BIP_IMAGE_HANDLE_LENGTH))
         {
            /* Check to make sure there isn't already a current         */
            /* operation.                                               */
            if(CurrentOperation == coNone)
            {
               /* Attempt to submit the request.                        */
               Result = BIP_Delete_Image_Request(BluetoothStackID, BIPID, ImageHandle);

               if(!Result)
               {
                  /* The function was submitted successfully.           */

                  /* Update the Current Operation.                      */
                  CurrentOperation = coDeleteImage;

                  printf("BIP_Delete_Image_Request: Function Successful.\r\n");
               }
               else
               {
                  /* There was an error submitting the function.        */
                  printf("BIP_Delete_Image_Request() Failure: %d.\r\n", Result);
               }
            }
            else
            {
               /* One or more of the parameters are invalid.            */
               printf("BIP Delete Image Request: Currently Outstanding Operation.\r\n");
            }
         }
         else
         {
            /* One or more of the parameters passed in are invalid.     */
            printf("BIP Delete Image Request: Invalid parameter.\r\n");
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("BIP Delete Image Request: Invalid BIPID.\r\n");
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      printf("Stack ID Invalid.\r\n");
   }
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

   /* The following function is for an BIP Event Callback.  This        */
   /* function will be called whenever a BIP Client or Server Event     */
   /* occurs that is associated with the Bluetooth Stack.  This function*/
   /* passes to the caller the BIP Event Data that occurred and the BIP */
   /* Event Callback Parameter that was specified when this Callback was*/
   /* installed.  The caller is free to use the contents of the BIP     */
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
   /* (this argument holds anyway because another BIP Event will not be */
   /* processed while this function call is outstanding).               */
   /* * NOTE * This function MUST NOT Block and wait for Events that can*/
   /*          only be satisfied by Receiving BIP Event Data.  A        */
   /*          Deadlock WILL occur because NO BIP Event Callbacks will  */
   /*          be issued while this function is currently outstanding.  */
static void BTPSAPI BIP_Event_Callback(unsigned int BluetoothStackID, BIP_Event_Data_t *BIPEventData, unsigned long CallbackParameter)
{
   int               Result = 0;
   char              BoardStr[32];
   Byte_t            ResponseCode;
   Word_t            NumberOfReturnedHandles;
   unsigned int      AmountWritten;
   unsigned int      TempLength;
   unsigned char    *TempBufferPtr;
   ImageListEntry_t *ImageListPtr;

   /* Make sure that parameters passed in appear to be at least         */
   /* semi-valid.                                                       */
   if((BluetoothStackID) && (BIPEventData))
   {
      printf("\r\n");

      /* The parameters passed in appear to be at least semi-valid.     */
      /* Next determine the event that has occurred.                    */
      switch(BIPEventData->Event_Data_Type)
      {
         case etBIP_Open_Imaging_Service_Port_Indication:
            /* A Client has connected to the server, display the BD_ADDR*/
            /* of the Connecting Device.                                */
            BD_ADDRToStr(BIPEventData->Event_Data.BIP_Open_Imaging_Service_Port_Indication_Data->BD_ADDR, (char *)BoardStr);
            printf("BIP Open Imaging Service Port Indication, ID: %u, Board: %s, ServiceType %s.\r\n", BIPEventData->Event_Data.BIP_Open_Imaging_Service_Port_Indication_Data->BIPID, BoardStr, (BIPEventData->Event_Data.BIP_Open_Imaging_Service_Port_Indication_Data->ImagingServiceServerPortTarget == istImagePushService)?("Image Push Service"):("Image Pull Service"));

            /* Set the Connection information.                          */
            Connected   = TRUE;
            ServiceType = BIPEventData->Event_Data.BIP_Open_Imaging_Service_Port_Indication_Data->ImagingServiceServerPortTarget;

            /* Cleanup the current image list.                          */
            CleanupImageList();

            /* Check to see if the current service type is the image    */
            /* pull service.                                            */
            if(ServiceType == istImagePullService)
            {
               /* The Current Service is the image pull service.        */
               /* Attempt to initialize the image list.                 */
               if(InitializeImageList() == 0)
                  printf("InitializeImageList: Function Successful.\r\n");
               else
                  printf("InitializeImageList() Failure.\r\n");
            }
            break;
         case etBIP_Open_Imaging_Service_Port_Confirmation:
            /* The Client has received a Connect Confirmation, display  */
            /* relevant information.                                    */
            printf("BIP Open Imaging Service Port Confirmation, ID: %u, Status 0x%08X.\r\n", BIPEventData->Event_Data.BIP_Open_Imaging_Service_Port_Confirmation_Data->BIPID,
                                                                                             (unsigned int)BIPEventData->Event_Data.BIP_Open_Imaging_Service_Port_Confirmation_Data->OpenStatus);

            /* Check to see if the Open Status indicates success.       */
            if(BIPEventData->Event_Data.BIP_Open_Imaging_Service_Port_Confirmation_Data->OpenStatus != BIP_OPEN_STATUS_SUCCESS)
            {
               /* The Open Status indicates that the connection was not */
               /* successfully created.  Reset the connection state     */
               /* information.                                          */
               BIPID     = 0;
               Connected = FALSE;
            }
            else
            {
               /* The Open Status indicates that the connection was     */
               /* successfully created, update the connection state     */
               /* information.                                          */
               Connected = TRUE;

               /* Check to see if the service type that was connected   */
               /* to.                                                   */
               if(ServiceType == istImagePullService)
               {
                  /* The image pull service was connected to.  Make the */
                  /* request to get the current image list.             */
                  BIPGetImagesListRequest();
               }
            }
            break;
         case etBIP_Close_Imaging_Service_Port_Indication:
            /* A Close Port Indication was received, this means that the*/
            /* Client has Disconnected from the Local Device.  Display  */
            /* the appropriate message.                                 */
            printf("BIP Close Imaging Service Port Indication, ID: %u.\r\n", BIPEventData->Event_Data.BIP_Close_Imaging_Service_Port_Indication_Data->BIPID);

            /* Check to see if the local entity is a client.            */
            if(IsClient)
            {
               /* The local device is not a server.  Reset the BIPID.   */
               BIPID = 0;
            }

            /* Change the connection state to indicate that we are no   */
            /* longer connected.                                        */
            Connected        = FALSE;
            CurrentOperation = coNone;

            /* Cleanup the image list if it is currently initialized.   */
            CleanupImageList();

            /* Reset all other state variables.                         */
            CurrentImageHandle[0]  = '\0';
            FirstPhase             = TRUE;
            CurrentFile[0]         = '\0';
            CurrentFileIndex       = 0;
            CurrentFileSize        = 0;

            CurrentFileBufferIndex = 0;

            CurrentObjectIndex     = 0;
            CurrentObjectLength    = 0;

            if(CurrentObjectBuffer != NULL)
            {
               if(CurrentObjectBufferAllocated)
                  free(CurrentObjectBuffer);

               CurrentObjectBuffer          = NULL;
               CurrentObjectBufferAllocated = FALSE;
            }
            break;
         case etBIP_Abort_Indication:
            /* An Abort Indication has been received, display all       */
            /* relevant information.                                    */
            printf("BIP Abort Indication, ID: %u.\r\n", BIPEventData->Event_Data.BIP_Abort_Indication_Data->BIPID);

            /* Reset the Current Operation State.                       */
            CurrentOperation = coNone;

            /* Reset all other state variables.                         */
            CurrentImageHandle[0]  = '\0';
            FirstPhase             = TRUE;
            CurrentFile[0]         = '\0';
            CurrentFileIndex       = 0;
            CurrentFileSize        = 0;

            CurrentFileBufferIndex = 0;

            CurrentObjectIndex     = 0;
            CurrentObjectLength    = 0;

            if(CurrentObjectBuffer != NULL)
            {
               if(CurrentObjectBufferAllocated)
                  free(CurrentObjectBuffer);

               CurrentObjectBuffer          = NULL;
               CurrentObjectBufferAllocated = FALSE;
            }
            break;
         case etBIP_Abort_Confirmation:
            /* An Abort Confirmation has been received, display all     */
            /* relevant information.                                    */
            printf("BIP Abort Confirmation, ID: %u.\r\n", BIPEventData->Event_Data.BIP_Abort_Confirmation_Data->BIPID);

            /* Reset the Current Operation State.                       */
            CurrentOperation = coNone;

            /* Reset all other state variables.                         */
            CurrentImageHandle[0]  = '\0';
            FirstPhase             = TRUE;
            CurrentFile[0]         = '\0';
            CurrentFileIndex       = 0;
            CurrentFileSize        = 0;

            CurrentFileBufferIndex = 0;

            CurrentObjectIndex     = 0;
            CurrentObjectLength    = 0;

            if(CurrentObjectBuffer != NULL)
            {
               if(CurrentObjectBufferAllocated)
                  free(CurrentObjectBuffer);

               CurrentObjectBuffer          = NULL;
               CurrentObjectBufferAllocated = FALSE;
            }
            break;
         case etBIP_Get_Capabilities_Indication:
            /* A Get Capabilities Indication was received, display all  */
            /* relevant information.                                    */
            printf("BIP Get Capabilities Indication, ID: %u.\r\n", BIPEventData->Event_Data.BIP_Get_Capabilities_Indication_Data->BIPID);

            /* Initialize the Response Code to an Error Response.       */
            ResponseCode = BIP_OBEX_RESPONSE_BAD_REQUEST;

            /* Check to see if this is the first phase of the operation.*/
            if(CurrentOperation == coNone)
            {
               /* This is the first phase of the operation.  Set up the */
               /* state variables.                                      */
               CurrentOperation             = coGetCapabilities;
               CurrentObjectIndex           = 0;
               CurrentObjectLength          = sizeof(ImagingCapabilitiesObject);
               CurrentObjectBuffer          = (unsigned char *)ImagingCapabilitiesObject;
               CurrentObjectBufferAllocated = FALSE;
               ResponseCode                 = BIP_OBEX_RESPONSE_CONTINUE;
            }
            else
            {
               /* This is not the first phase of the operation, check to*/
               /* see if this is a continuation of a currently on going */
               /* operation.                                            */
               if(CurrentOperation == coGetCapabilities)
               {
                  /* Check to see if the entire object has been sent.   */
                  if(CurrentObjectIndex >= CurrentObjectLength)
                  {
                     /* The entire object has been sent, set the        */
                     /* response code to OK to complete this operation. */
                     ResponseCode = BIP_OBEX_RESPONSE_OK;
                  }
                  else
                  {
                     /* The entire object has not been sent, set the    */
                     /* response code to CONTINUE.                      */
                     ResponseCode = BIP_OBEX_RESPONSE_CONTINUE;
                  }
               }
            }

            /* Attempt to submit the response.                          */
            if((Result = BIP_Get_Capabilities_Response(BluetoothStackID, BIPID, ResponseCode, (CurrentObjectLength-CurrentObjectIndex), &(CurrentObjectBuffer[CurrentObjectIndex]), &AmountWritten)) == 0)
            {
               /* Adjust the index into the current object by the amount*/
               /* written.                                              */
               CurrentObjectIndex += AmountWritten;

               printf("BIP_Get_Capabilities_Response() Success, CurrentObjectIndex: %u, CurrentObjectLength: %u.\r\n", CurrentObjectIndex, CurrentObjectLength);
            }
            else
            {
               /* An error occurred while attempting to submit the Get  */
               /* Capabilites Response.                                 */
               printf("BIP_Get_Capabilities_Response() Failure: %d.\r\n", Result);
            }

            /* If an error occurred, or the operation was completed     */
            /* update the current operation.                            */
            if((Result != 0) || (ResponseCode != BIP_OBEX_RESPONSE_CONTINUE))
            {
               /* Reset the Current Operation.                          */
               CurrentOperation = coNone;
            }
            break;
         case etBIP_Get_Capabilities_Confirmation:
            /* A Get Capabilities Confirmation was received, display all*/
            /* relevant information.                                    */
            printf("BIP Get Capabilities Confirmation, ID: %u, ResponseCode: 0x%02X, DataLength: %u.\r\n", BIPEventData->Event_Data.BIP_Get_Capabilities_Confirmation_Data->BIPID, BIPEventData->Event_Data.BIP_Get_Capabilities_Confirmation_Data->ResponseCode, BIPEventData->Event_Data.BIP_Get_Capabilities_Confirmation_Data->DataLength);

            /* Check for non-error response code.                       */
            if((BIPEventData->Event_Data.BIP_Get_Capabilities_Confirmation_Data->ResponseCode == BIP_OBEX_RESPONSE_CONTINUE) || (BIPEventData->Event_Data.BIP_Get_Capabilities_Confirmation_Data->ResponseCode == BIP_OBEX_RESPONSE_OK))
            {
               /* Next check to make sure that their is some data       */
               /* associated with this phase of the operation.          */
               if((BIPEventData->Event_Data.BIP_Get_Capabilities_Confirmation_Data->DataLength > 0) && (BIPEventData->Event_Data.BIP_Get_Capabilities_Confirmation_Data->DataBuffer != NULL))
               {
                  /* There is some data associated with this phase of   */
                  /* the operation.                                     */
                  TempLength = CurrentObjectLength + BIPEventData->Event_Data.BIP_Get_Capabilities_Confirmation_Data->DataLength;

                  /* Next attempt to allocated a buffer large enough to */
                  /* hold the exist object data including the newly     */
                  /* received portion.                                  */
                  if((TempBufferPtr = (unsigned char *)malloc(TempLength)) != NULL)
                  {
                     memcpy(TempBufferPtr, CurrentObjectBuffer, CurrentObjectLength);
                     memcpy(&TempBufferPtr[CurrentObjectLength], BIPEventData->Event_Data.BIP_Get_Capabilities_Confirmation_Data->DataBuffer, BIPEventData->Event_Data.BIP_Get_Capabilities_Confirmation_Data->DataLength);

                     CurrentObjectLength += BIPEventData->Event_Data.BIP_Get_Capabilities_Confirmation_Data->DataLength;

                     if((CurrentObjectBuffer != NULL) && (CurrentObjectBufferAllocated))
                        free(CurrentObjectBuffer);

                     CurrentObjectBuffer          = TempBufferPtr;
                     CurrentObjectBufferAllocated = TRUE;
                  }
               }
            }

            /* Check to see if this was the last phase of this          */
            /* operation.                                               */
            if(BIPEventData->Event_Data.BIP_Get_Capabilities_Confirmation_Data->ResponseCode == BIP_OBEX_RESPONSE_CONTINUE)
            {
               /* The response code indicates there is more to the      */
               /* object being received.  Request the next piece of the */
               /* object.                                               */
               if((Result = BIP_Get_Capabilities_Request(BluetoothStackID, BIPID)) == 0)
               {
                  /* The function was submitted successfully.           */
                  printf("BIP_Get_Capabilities_Request: Function Successful.\r\n");
               }
               else
               {
                  /* There was an error submitting the function.        */
                  printf("BIP_Get_Capabilities_Request() Failure: %d.\r\n", Result);
               }
            }
            else
            {
               if(BIPEventData->Event_Data.BIP_Get_Capabilities_Confirmation_Data->ResponseCode == BIP_OBEX_RESPONSE_OK)
               {
                  /* This was the final phase of this operation.        */
                  /* Display the object that has been received.         */
                  DisplayObject(CurrentObjectLength, CurrentObjectBuffer);
               }
            }

            /* If an error occurred, or the operation was completed     */
            /* update the current operation.                            */
            if((Result != 0) || (BIPEventData->Event_Data.BIP_Get_Capabilities_Confirmation_Data->ResponseCode != BIP_OBEX_RESPONSE_CONTINUE))
            {
               /* Reset the Current Operation.                          */
               CurrentOperation = coNone;

               /* Reset the state variables and free any allocated      */
               /* resources.                                            */
               CurrentObjectIndex  = 0;
               CurrentObjectLength = 0;

               if((CurrentObjectBuffer != NULL) && (CurrentObjectBufferAllocated))
                  free(CurrentObjectBuffer);

               CurrentObjectBuffer          = NULL;
               CurrentObjectBufferAllocated = FALSE;
            }
            break;
         case etBIP_Put_Image_Indication:
            /* A Put Image Indication was received, display all relevant*/
            /* information.                                             */
            printf("BIP Put Image Indication, ID: %u.\r\n", BIPEventData->Event_Data.BIP_Put_Image_Indication_Data->BIPID);

            /* Initialize the Response Code to an Error Response.       */
            ResponseCode = BIP_OBEX_RESPONSE_BAD_REQUEST;

            /* Check to see if this is the first phase of the operation.*/
            if(CurrentOperation == coNone)
            {
               /* This is the first phase of the operation.  Set up the */
               /* state variables.                                      */
               if(GetNextImageHandle(CurrentImageHandle) == 0)
               {
                  /* Check to see if the Image Name appears to be valid.*/
                  if(BIPEventData->Event_Data.BIP_Put_Image_Indication_Data->ImageName != NULL)
                  {
                     /* The Image Name is valid.  Display the Image     */
                     /* Name.                                           */
                     printf("    ImageName: %s.\r\n", BIPEventData->Event_Data.BIP_Put_Image_Indication_Data->ImageName);
                  }

                  /* Check to see if the Image Descriptor appears to be */
                  /* valid.                                             */
                  if((BIPEventData->Event_Data.BIP_Put_Image_Indication_Data->ImageDescriptorLength > 0) && (BIPEventData->Event_Data.BIP_Put_Image_Indication_Data->ImageDescriptor != NULL))
                  {
                     /* Display the Image Descriptor Object Received.   */
                     DisplayObject(BIPEventData->Event_Data.BIP_Put_Image_Indication_Data->ImageDescriptorLength, BIPEventData->Event_Data.BIP_Put_Image_Indication_Data->ImageDescriptor);
                  }

                  /* Check to make sure that the path to the new        */
                  /* filename can be successfully built.                */
                  if(sizeof(CurrentFile) >= (strlen(RootDirectory) + (22*sizeof(char))))
                  {
                     /* Reset the current file index and set that this  */
                     /* is the first phase.                             */
                     CurrentFileIndex = 0;
                     FirstPhase       = TRUE;

                     /* Build the full path to the storage location for */
                     /* the file being received.                        */
                     strcpy(CurrentFile, RootDirectory);
                     sprintf(&CurrentFile[strlen(CurrentFile)], "%c%c%cFOLDR", CurrentImageHandle[0], CurrentImageHandle[1], CurrentImageHandle[2]);
                     sprintf(&CurrentFile[strlen(CurrentFile)], "/IMGE%c%c%c%c.JPG", CurrentImageHandle[3], CurrentImageHandle[4], CurrentImageHandle[5], CurrentImageHandle[6]);

                     /* Check to see if there is image data associated  */
                     /* with this transaction.                          */
                     if((BIPEventData->Event_Data.BIP_Put_Image_Indication_Data->DataLength > 0) && (BIPEventData->Event_Data.BIP_Put_Image_Indication_Data->DataBuffer != NULL))
                     {
                        /* There is image data associated with this     */
                        /* transaction.  Put the data to a file.        */
                        if(PutFileData(BIPEventData->Event_Data.BIP_Put_Image_Indication_Data->DataLength, BIPEventData->Event_Data.BIP_Put_Image_Indication_Data->DataBuffer) == FILE_DATA_FILE_IO_SUCCESS)
                        {
                           /* Data was successfully written to the file,*/
                           /* set the current operation and response    */
                           /* code.                                     */
                           CurrentOperation = coPutImage;
                           ResponseCode     = BIP_OBEX_RESPONSE_CONTINUE;
                        }
                     }
                     else
                     {
                        /* No image data to write.  Set the current     */
                        /* operation and response code.                 */
                        CurrentOperation = coPutImage;
                        ResponseCode     = BIP_OBEX_RESPONSE_CONTINUE;
                     }
                  }
               }
               else
               {
                  /* Unable to allocated an image handle.  Display an   */
                  /* appropriate error message.                         */
                  printf("BIP Put Image Indication: Unable to allocated image handle.\r\n");
               }
            }
            else
            {
               /* This is not the first phase of the operation, check to*/
               /* see if this is a continuation of a currently on going */
               /* operation.                                            */
               if(CurrentOperation == coPutImage)
               {
                  /* This is a continuation of the currently ongoing    */
                  /* operation.  Check to see if the Image Name appears */
                  /* to be valid.                                       */
                  if(BIPEventData->Event_Data.BIP_Put_Image_Indication_Data->ImageName != NULL)
                  {
                     /* The Image Name is valid.  Display the Image     */
                     /* Name.                                           */
                     printf("    ImageName: %s.\r\n", BIPEventData->Event_Data.BIP_Put_Image_Indication_Data->ImageName);
                  }

                  /* Check to see if the Image Descriptor appears to be */
                  /* valid.                                             */
                  if((BIPEventData->Event_Data.BIP_Put_Image_Indication_Data->ImageDescriptorLength > 0) && (BIPEventData->Event_Data.BIP_Put_Image_Indication_Data->ImageDescriptor != NULL))
                  {
                     /* Display the Image Descriptor Object Received.   */
                     DisplayObject(BIPEventData->Event_Data.BIP_Put_Image_Indication_Data->ImageDescriptorLength, BIPEventData->Event_Data.BIP_Put_Image_Indication_Data->ImageDescriptor);
                  }

                  /* Check to see if there is image data associated with*/
                  /* this transaction.                                  */
                  if((BIPEventData->Event_Data.BIP_Put_Image_Indication_Data->DataLength > 0) && (BIPEventData->Event_Data.BIP_Put_Image_Indication_Data->DataBuffer != NULL))
                  {
                     /* There is image data associated with this        */
                     /* transaction.  Put the data to a file.           */
                     if(PutFileData(BIPEventData->Event_Data.BIP_Put_Image_Indication_Data->DataLength, BIPEventData->Event_Data.BIP_Put_Image_Indication_Data->DataBuffer) == FILE_DATA_FILE_IO_SUCCESS)
                     {
                        /* Set the response code to indicate success.   */
                        ResponseCode = BIP_OBEX_RESPONSE_CONTINUE;
                     }
                  }

                  /* Display a message indicating the total length      */
                  /* received up until this point.                      */
                  printf("    Length Received: %u.\r\n", (unsigned int)CurrentFileIndex);
               }
            }

            /* Check to see if this was the final piece of this         */
            /* tranaction.                                              */
            if(BIPEventData->Event_Data.BIP_Put_Image_Indication_Data->Final)
            {
               /* This is the final piece of this transaction.  Reset   */
               /* the Current Operation and set the Response Code to    */
               /* indicate success.                                     */
               CurrentOperation = coNone;
               ResponseCode     = BIP_OBEX_RESPONSE_OK;
            }

            /* Attempt to submit the response.                          */
            if((Result = BIP_Put_Image_Response(BluetoothStackID, BIPID, ResponseCode, CurrentImageHandle)) == 0)
            {
               printf("BIP_Put_Image_Response() Success, ImageHandle: %s.\r\n", CurrentImageHandle);
            }
            else
            {
               /* An error occurred while attempting to submit the Put  */
               /* Image Response.                                       */
               printf("BIP_Put_Image_Response() Failure: %d.\r\n", Result);
            }

            /* If an error occurred, or the operation was completed     */
            /* update the current operation.                            */
            if((Result != 0) || (ResponseCode != BIP_OBEX_RESPONSE_CONTINUE))
            {
               /* Reset the Current Operation.                          */
               CurrentOperation = coNone;
            }
            break;
         case etBIP_Put_Image_Confirmation:
            /* A Put Image Confirmation was received, display all       */
            /* relevant information.                                    */
            printf("BIP Put Image Confirmation, ID: %u, ResponseCode: 0x%02X.\r\n", BIPEventData->Event_Data.BIP_Put_Image_Confirmation_Data->BIPID, BIPEventData->Event_Data.BIP_Put_Image_Confirmation_Data->ResponseCode);

            /* Check to see if this event data contains the Image       */
            /* Handle.                                                  */
            if(BIPEventData->Event_Data.BIP_Put_Image_Confirmation_Data->ImageHandle)
            {
               /* Display the Image Handle information.                 */
               printf("    ImageHandle: %s.\r\n", BIPEventData->Event_Data.BIP_Put_Image_Confirmation_Data->ImageHandle);
            }

            /* Check to see if this was the last phase of this          */
            /* operation.                                               */
            if(BIPEventData->Event_Data.BIP_Put_Image_Confirmation_Data->ResponseCode == BIP_OBEX_RESPONSE_CONTINUE)
            {
               /* Get some more file data.                              */
               GetFileData();

               Result = BIP_Put_Image_Request(BluetoothStackID, BIPID, NULL, 0, NULL, CurrentFileBufferIndex, CurrentFileBuffer, &AmountWritten, (Boolean_t)((CurrentFileBufferIndex == 0)?(TRUE):(FALSE)));

               if(!Result)
               {
                  /* The function was submitted successfully.           */
                  printf("BIP_Put_Image_Request: Function Successful.\r\n");

                  /* Adjust the current file buffer.                    */
                  if(AmountWritten)
                  {
                     memmove(CurrentFileBuffer, &CurrentFileBuffer[AmountWritten], (CurrentFileBufferIndex - AmountWritten));
                     CurrentFileBufferIndex -= AmountWritten;
                  }
               }
               else
               {
                  /* There was an error submitting the function.        */
                  printf("BIP_Put_Image_Request() Failure: %d.\r\n", Result);
               }
            }

            /* If an error occurred, or the operation was completed     */
            /* update the current operation.                            */
            if((Result != 0) || (BIPEventData->Event_Data.BIP_Put_Image_Confirmation_Data->ResponseCode != BIP_OBEX_RESPONSE_CONTINUE))
            {
               /* Reset the Current Operation.                          */
               CurrentOperation = coNone;
            }
            break;
         case etBIP_Put_Linked_Thumbnail_Indication:
            /* A Put Linked Thumbnail Indication was received, display  */
            /* all relevant information.                                */
            printf("BIP Put Linked Thumbnail Indication, ID: %u.\r\n", BIPEventData->Event_Data.BIP_Put_Linked_Thumbnail_Indication_Data->BIPID);

            /* Initialize the Response Code to an Error Response.       */
            ResponseCode = BIP_OBEX_RESPONSE_BAD_REQUEST;

            /* Check to see if this is the first phase of the operation.*/
            if(CurrentOperation == coNone)
            {
               /* Initialize the current image handle to have no handle.*/
               CurrentImageHandle[0] = '\0';

               /* Check to see if the Image Handle appears to be at     */
               /* least semi-valid.                                     */
               if(BIPEventData->Event_Data.BIP_Put_Linked_Thumbnail_Indication_Data->ImageHandle)
               {
                  /* Display the Image Handle information.              */
                  printf("    ImageHandle: %s.\r\n", BIPEventData->Event_Data.BIP_Put_Linked_Thumbnail_Indication_Data->ImageHandle);
               }

               /* Only accept thumbnails for Image Handles that are     */
               /* valid.                                                */
               if(TestImageHandle(BIPEventData->Event_Data.BIP_Put_Linked_Thumbnail_Indication_Data->ImageHandle))
               {
                  /* An image with this handle exists.  We can accept   */
                  /* this thumbnail.  Check to make sure that the path  */
                  /* to the new filename can be successfully built.     */
                  if(sizeof(CurrentFile) >= (strlen(RootDirectory) + (22*sizeof(char))))
                  {
                     /* Reset the current file index and set that this  */
                     /* is the first phase.                             */
                     CurrentFileIndex   = 0;
                     FirstPhase         = TRUE;

                     strcpy(CurrentImageHandle, BIPEventData->Event_Data.BIP_Put_Linked_Thumbnail_Indication_Data->ImageHandle);

                     /* Build the full path to the storage location for */
                     /* the file being received.                        */
                     strcpy(CurrentFile, RootDirectory);
                     sprintf(&CurrentFile[strlen(CurrentFile)], "%c%c%cFOLDR", CurrentImageHandle[0], CurrentImageHandle[1], CurrentImageHandle[2]);
                     sprintf(&CurrentFile[strlen(CurrentFile)], "/THMB%c%c%c%c.JPG", CurrentImageHandle[3], CurrentImageHandle[4], CurrentImageHandle[5], CurrentImageHandle[6]);

                     /* Check to see if there is image data associated  */
                     /* with this transaction.                          */
                     if((BIPEventData->Event_Data.BIP_Put_Linked_Thumbnail_Indication_Data->DataLength > 0) && (BIPEventData->Event_Data.BIP_Put_Linked_Thumbnail_Indication_Data->DataBuffer != NULL))
                     {
                        /* There is image data associated with this     */
                        /* transaction.  Put the data to a file.        */
                        if(PutFileData(BIPEventData->Event_Data.BIP_Put_Linked_Thumbnail_Indication_Data->DataLength, BIPEventData->Event_Data.BIP_Put_Linked_Thumbnail_Indication_Data->DataBuffer) == FILE_DATA_FILE_IO_SUCCESS)
                        {
                           /* Data was successfully written to the file,*/
                           /* set the current operation and response    */
                           /* code.                                     */
                           CurrentOperation = coPutLinkedThumbnail;
                           ResponseCode     = BIP_OBEX_RESPONSE_CONTINUE;
                        }
                     }
                     else
                     {
                        /* No image data to write.  Set the current     */
                        /* operation and response code.                 */
                        CurrentOperation = coPutLinkedThumbnail;
                        ResponseCode     = BIP_OBEX_RESPONSE_CONTINUE;
                     }
                  }
               }
            }
            else
            {
               /* This is not the first phase of the operation, check to*/
               /* see if this is a continuation of a currently on going */
               /* operation.                                            */
               if(CurrentOperation == coPutLinkedThumbnail)
               {
                  /* This is a continuation of the currently ongoing    */
                  /* operation.  Check to see if there is image data    */
                  /* associated with this transaction.                  */
                  if((BIPEventData->Event_Data.BIP_Put_Linked_Thumbnail_Indication_Data->DataLength > 0) && (BIPEventData->Event_Data.BIP_Put_Linked_Thumbnail_Indication_Data->DataBuffer != NULL))
                  {
                     /* There is image data associated with this        */
                     /* transaction.  Put the data to a file.           */
                     if(PutFileData(BIPEventData->Event_Data.BIP_Put_Linked_Thumbnail_Indication_Data->DataLength, BIPEventData->Event_Data.BIP_Put_Linked_Thumbnail_Indication_Data->DataBuffer) == FILE_DATA_FILE_IO_SUCCESS)
                     {
                        /* Set the response code to indicate success.   */
                        ResponseCode = BIP_OBEX_RESPONSE_CONTINUE;
                     }
                  }

                  /* Display a message indicating the total length      */
                  /* received up until this point.                      */
                  printf("    Length Received: %u.\r\n", (unsigned int)CurrentFileIndex);
               }
            }

            /* Check to see if this was the final piece of this         */
            /* tranaction.                                              */
            if(BIPEventData->Event_Data.BIP_Put_Linked_Thumbnail_Indication_Data->Final)
            {
               /* This is the final piece of this transaction.  Reset   */
               /* the Current Operation and set the Response Code to    */
               /* indicate success.                                     */
               CurrentOperation = coNone;
               ResponseCode     = BIP_OBEX_RESPONSE_OK;
            }

            /* Attempt to submit the response.                          */
            if((Result = BIP_Put_Linked_Thumbnail_Response(BluetoothStackID, BIPID, ResponseCode)) == 0)
            {
               printf("BIP_Put_Linked_Thumbnail_Response() Success, ImageHandle: %s.\r\n", CurrentImageHandle);
            }
            else
            {
               /* An error occurred while attempting to submit the Put  */
               /* Linked Thumbnail Response.                            */
               printf("BIP_Put_Linked_Thumbnail_Response() Failure: %d.\r\n", Result);
            }

            /* If an error occurred, or the operation was completed     */
            /* update the current operation.                            */
            if((Result != 0) || (ResponseCode != BIP_OBEX_RESPONSE_CONTINUE))
            {
               /* Reset the Current Operation.                          */
               CurrentOperation = coNone;

               /* Reset the current image handle to have no handle.     */
               CurrentImageHandle[0] = '\0';
            }
            break;
         case etBIP_Put_Linked_Thumbnail_Confirmation:
            /* A Put Linked Thumbnail Confirmation was received, display*/
            /* all relevant information.                                */
            printf("BIP Put Linked Thumbnail Confirmation, ID: %u, ResponseCode: 0x%02X.\r\n", BIPEventData->Event_Data.BIP_Put_Linked_Thumbnail_Confirmation_Data->BIPID, BIPEventData->Event_Data.BIP_Put_Linked_Thumbnail_Confirmation_Data->ResponseCode);

            /* Check to see if this was the last phase of this          */
            /* operation.                                               */
            if(BIPEventData->Event_Data.BIP_Put_Linked_Thumbnail_Confirmation_Data->ResponseCode == BIP_OBEX_RESPONSE_CONTINUE)
            {
               /* Get some more file data.                              */
               GetFileData();

               Result = BIP_Put_Linked_Thumbnail_Request(BluetoothStackID, BIPID, NULL, CurrentFileBufferIndex, CurrentFileBuffer, &AmountWritten, (Boolean_t)((CurrentFileBufferIndex == 0)?(TRUE):(FALSE)));

               if(!Result)
               {
                  /* The function was submitted successfully.           */
                  printf("BIP_Put_Linked_Thumbnail_Request: Function Successful.\r\n");

                  /* Adjust the current file buffer.                    */
                  if(AmountWritten)
                  {
                     memmove(CurrentFileBuffer, &CurrentFileBuffer[AmountWritten], (CurrentFileBufferIndex - AmountWritten));
                     CurrentFileBufferIndex -= AmountWritten;
                  }
               }
               else
               {
                  /* There was an error submitting the function.        */
                  printf("BIP_Put_Linked_Thumbnail_Request() Failure: %d.\r\n", Result);
               }
            }

            /* If an error occurred, or the operation was completed     */
            /* update the current operation.                            */
            if((Result != 0) || (BIPEventData->Event_Data.BIP_Put_Linked_Thumbnail_Confirmation_Data->ResponseCode != BIP_OBEX_RESPONSE_CONTINUE))
            {
               /* Reset the Current Operation.                          */
               CurrentOperation = coNone;
            }
            break;
         case etBIP_Get_Images_List_Indication:
            /* A Get Image List Indication was received, display all    */
            /* relevant information.                                    */
            printf("BIP Get Images List Indication, ID: %u, NumberOfReturnedHandles: %d, ListStartOffset: %d, LatestCapturedImages: 0x%02X.\r\n", BIPEventData->Event_Data.BIP_Get_Images_List_Indication_Data->BIPID, (int)BIPEventData->Event_Data.BIP_Get_Images_List_Indication_Data->NumberOfReturnedHandles, (int)BIPEventData->Event_Data.BIP_Get_Images_List_Indication_Data->ListStartOffset, BIPEventData->Event_Data.BIP_Get_Images_List_Indication_Data->LatestCapturedImages);

            /* With this application implementation filter using the    */
            /* Image Handles Descriptor is not support.  If a valid     */
            /* Image Handles Descriptor is present, simply display it.  */
            if((BIPEventData->Event_Data.BIP_Get_Images_List_Indication_Data->ImageHandlesDescriptorLength > 0) && (BIPEventData->Event_Data.BIP_Get_Images_List_Indication_Data->ImageHandlesDescriptor != NULL))
            {
               printf("    ImageHandlesDescriptor:\r\n");
               DisplayObject(BIPEventData->Event_Data.BIP_Get_Images_List_Indication_Data->ImageHandlesDescriptorLength, BIPEventData->Event_Data.BIP_Get_Images_List_Indication_Data->ImageHandlesDescriptor);
            }

            /* Initialize the Response Code to an Error Response.       */
            ResponseCode = BIP_OBEX_RESPONSE_BAD_REQUEST;

            /* Initialize the Number Of Returned Handles to Zero.       */
            NumberOfReturnedHandles = 0;

            /* Check to see if this is the first phase of the operation.*/
            if(CurrentOperation == coNone)
            {
               /* This is the first phase of the operation.  Initialize */
               /* the state variables to a known state.                 */
               CurrentObjectIndex           = 0;
               CurrentObjectLength          = 0;
               CurrentObjectBuffer          = NULL;
               CurrentObjectBufferAllocated = FALSE;

               /* Next check to make sure that the members that should  */
               /* be valid in this phase of the operation appear to be  */
               /* valid.                                                */
               if((BIPEventData->Event_Data.BIP_Get_Images_List_Indication_Data->NumberOfReturnedHandles != BIP_NUMBER_OF_RETURNED_HANDLES_INVALID_VALUE) && (BIPEventData->Event_Data.BIP_Get_Images_List_Indication_Data->ListStartOffset != BIP_LIST_START_OFFSET_INVALID_VALUE) && (BIPEventData->Event_Data.BIP_Get_Images_List_Indication_Data->LatestCapturedImages != BIP_LATEST_CAPTURED_IMAGES_INVALID_VALUE))
               {
                  /* Next check to see if this is a request to retrieve */
                  /* the number of handles currently in the image list. */
                  if(BIPEventData->Event_Data.BIP_Get_Images_List_Indication_Data->NumberOfReturnedHandles == BIP_NUMBER_OF_RETURNED_HANDLES_MINIMUM_VALUE)
                  {
                     /* This is a request to retrieve the current number*/
                     /* of handles in the image list.  This should      */
                     /* complete this operation so set the Response Code*/
                     /* to OK.                                          */
                     NumberOfReturnedHandles = (Word_t)NumberImageListEntries;
                     ResponseCode            = BIP_OBEX_RESPONSE_OK;
                  }
                  else
                  {
                     /* This is not a request to retrieve the current   */
                     /* number of handles in the image list.  It is a   */
                     /* request to get at least a portion of the current*/
                     /* image list.  Determine the length of the buffer */
                     /* required to build the images listing object.    */
                     if(BuildImagesListingObject(NumberImageListEntries, ImageList, BIPEventData->Event_Data.BIP_Get_Images_List_Indication_Data->ListStartOffset, BIPEventData->Event_Data.BIP_Get_Images_List_Indication_Data->NumberOfReturnedHandles, &CurrentObjectLength, NULL) == 0)
                     {
                        /* The Length was successfully determined, next */
                        /* attempt to allocate a buffer to build the    */
                        /* object into.                                 */
                        if((CurrentObjectBuffer = (unsigned char *)malloc(CurrentObjectLength)) != NULL)
                        {
                           /* Now attempt to build the Images Listing   */
                           /* Object.                                   */
                           if(BuildImagesListingObject(NumberImageListEntries, ImageList, BIPEventData->Event_Data.BIP_Get_Images_List_Indication_Data->ListStartOffset, BIPEventData->Event_Data.BIP_Get_Images_List_Indication_Data->NumberOfReturnedHandles, &CurrentObjectLength, CurrentObjectBuffer) == 0)
                           {
                              /* The Images Listing Object was          */
                              /* successfully built.  Initialize the    */
                              /* required state variables.              */
                              CurrentOperation             = coGetImagesList;
                              ResponseCode                 = BIP_OBEX_RESPONSE_CONTINUE;
                              CurrentObjectBufferAllocated = TRUE;

                              /* Set the Number Of Returned Handles to  */
                              /* the Number Of Images Currently in the  */
                              /* list being sent back.                  */
                              NumberOfReturnedHandles = (Word_t)NumberImageListEntries;
                           }
                           else
                           {

                              /* An error occurred while attempting to  */
                              /* build the Images Listing Object.  Free */
                              /* the Images Listing Object Memory.      */
                              free(CurrentObjectBuffer);
                              CurrentObjectBuffer          = NULL;
                              CurrentObjectLength          = 0;
                              CurrentObjectIndex           = 0;
                              CurrentObjectBufferAllocated = FALSE;
                           }
                        }
                     }

                     /* ** NOTE ** Currently the Latest Captured Images */
                     /*            functionality is not supported by    */
                     /*            this application.                    */
                  }
               }
            }
            else
            {
               /* This is not the first phase of the operation, check to*/
               /* see if this is a continuation of a currently on going */
               /* operation.                                            */
               if(CurrentOperation == coGetImagesList)
               {
                  /* Check to see if the entire object has been sent.   */
                  if(CurrentObjectIndex >= CurrentObjectLength)
                  {
                     /* The entire object has been sent, set the        */
                     /* response code to OK to complete this operation. */
                     ResponseCode = BIP_OBEX_RESPONSE_OK;
                  }
                  else
                  {
                     /* The entire object has not been sent, set the    */
                     /* response code to CONTINUE.                      */
                     ResponseCode = BIP_OBEX_RESPONSE_CONTINUE;
                  }
               }
            }

            /* Attempt to submit the response.                          */
            /* ** NOTE ** Filtering is not currently supported by this  */
            /*            applications implementation.  The list will   */
            /*            always include all JPEG images.  Send an Image*/
            /*            Handles Descriptor indicating this.           */
            if((Result = BIP_Get_Images_List_Response(BluetoothStackID, BIPID, ResponseCode, NumberOfReturnedHandles, sizeof(ImageHandlesDescriptorObject), (unsigned char *)ImageHandlesDescriptorObject, (CurrentObjectLength-CurrentObjectIndex), &(CurrentObjectBuffer[CurrentObjectIndex]), &AmountWritten)) == 0)
            {
               /* Adjust the index into the current object by the amount*/
               /* written.                                              */
               CurrentObjectIndex += AmountWritten;

               printf("BIP_Get_Images_List_Response() Success, ResponseCode: 0x%02X, CurrentObjectIndex: %u, CurrentObjectLength: %u.\r\n", ResponseCode, CurrentObjectIndex, CurrentObjectLength);
            }
            else
            {
               /* An error occurred while attempting to submit the Get  */
               /* Get Image List Response.                              */
               printf("BIP_Get_Images_List_Response() Failure: %d.\r\n", Result);
            }

            /* If an error occurred, or the operation was completed     */
            /* update the current operation.                            */
            if((Result != 0) || (ResponseCode != BIP_OBEX_RESPONSE_CONTINUE))
            {
               /* Reset the Current Operation.                          */
               CurrentOperation = coNone;

               /* Free the Current Object Buffer and reset the state    */
               /* variables.                                            */
               if((CurrentObjectBuffer != NULL) && (CurrentObjectBufferAllocated))
                  free(CurrentObjectBuffer);

               CurrentObjectBuffer          = NULL;
               CurrentObjectLength          = 0;
               CurrentObjectIndex           = 0;
               CurrentObjectBufferAllocated = FALSE;
            }
            break;
         case etBIP_Get_Images_List_Confirmation:
            /* A Get Image List Confirmation was received, display all  */
            /* relevant information.                                    */
            printf("BIP Get Images List Confirmation, ID: %u, ResponseCode: 0x%02X, NumberOfReturnedHandles: %d.\r\n", BIPEventData->Event_Data.BIP_Get_Images_List_Confirmation_Data->BIPID, BIPEventData->Event_Data.BIP_Get_Images_List_Confirmation_Data->ResponseCode, (int)BIPEventData->Event_Data.BIP_Get_Images_List_Confirmation_Data->NumberOfReturnedHandles);

            /* With this application implementation filter using the    */
            /* Image Handles Descriptor is not support.  If a valid     */
            /* Image Handles Descriptor is present, simply display it.  */
            if((BIPEventData->Event_Data.BIP_Get_Images_List_Confirmation_Data->ImageHandlesDescriptorLength > 0) && (BIPEventData->Event_Data.BIP_Get_Images_List_Confirmation_Data->ImageHandlesDescriptor != NULL))
            {
               printf("    ImageHandlesDescriptor:\r\n");
               DisplayObject(BIPEventData->Event_Data.BIP_Get_Images_List_Confirmation_Data->ImageHandlesDescriptorLength, BIPEventData->Event_Data.BIP_Get_Images_List_Confirmation_Data->ImageHandlesDescriptor);
            }

            /* Check for non-error response code.                       */
            if((BIPEventData->Event_Data.BIP_Get_Images_List_Confirmation_Data->ResponseCode == BIP_OBEX_RESPONSE_CONTINUE) || (BIPEventData->Event_Data.BIP_Get_Images_List_Confirmation_Data->ResponseCode == BIP_OBEX_RESPONSE_OK))
            {
               /* Next check to make sure that their is some data       */
               /* associated with this phase of the operation.          */
               if((BIPEventData->Event_Data.BIP_Get_Images_List_Confirmation_Data->DataLength > 0) && (BIPEventData->Event_Data.BIP_Get_Images_List_Confirmation_Data->DataBuffer != NULL))
               {
                  /* There is some data associated with this phase of   */
                  /* the operation.                                     */
                  TempLength = CurrentObjectLength + BIPEventData->Event_Data.BIP_Get_Images_List_Confirmation_Data->DataLength;

                  /* Next attempt to allocated a buffer large enough to */
                  /* hold the existing object data including the newly  */
                  /* received portion.                                  */
                  if((TempBufferPtr = (unsigned char *)malloc(TempLength)) != NULL)
                  {
                     memcpy(TempBufferPtr, CurrentObjectBuffer, CurrentObjectLength);
                     memcpy(&TempBufferPtr[CurrentObjectLength], BIPEventData->Event_Data.BIP_Get_Images_List_Confirmation_Data->DataBuffer, BIPEventData->Event_Data.BIP_Get_Images_List_Confirmation_Data->DataLength);

                     CurrentObjectLength += BIPEventData->Event_Data.BIP_Get_Images_List_Confirmation_Data->DataLength;

                     if((CurrentObjectBuffer != NULL) && (CurrentObjectBufferAllocated))
                        free(CurrentObjectBuffer);

                     CurrentObjectBuffer          = TempBufferPtr;
                     CurrentObjectBufferAllocated = TRUE;
                  }
               }
            }

            /* Check to see if this was the last phase of this          */
            /* operation.                                               */
            if(BIPEventData->Event_Data.BIP_Get_Images_List_Confirmation_Data->ResponseCode == BIP_OBEX_RESPONSE_CONTINUE)
            {
               /* The response code indicates there is more to the      */
               /* object being received.  Request the next piece of the */
               /* object.                                               */
               if((Result = BIP_Get_Images_List_Request(BluetoothStackID, BIPID, BIP_NUMBER_OF_RETURNED_HANDLES_MAXIMUM_VALUE, BIP_LIST_START_OFFSET_MINIMUM_VALUE, BIP_LATEST_CAPTURED_IMAGES_FALSE_VALUE, sizeof(ImageHandlesDescriptorObject), (unsigned char *)ImageHandlesDescriptorObject)) == 0)
               {
                  /* The function was submitted successfully.           */
                  printf("BIP_Get_Images_List_Request: Function Successful.\r\n");
               }
               else
               {
                  /* There was an error submitting the function.        */
                  printf("BIP_Get_Images_List_Request() Failure: %d.\r\n", Result);
               }
            }
            else
            {
               if(BIPEventData->Event_Data.BIP_Get_Images_List_Confirmation_Data->ResponseCode == BIP_OBEX_RESPONSE_OK)
               {
                  /* This was the final phase of this operation.        */
                  /* Display the object that has been received.         */
                  DisplayObject(CurrentObjectLength, CurrentObjectBuffer);

                  /* Determine how many image list entries are required */
                  /* to build the image list.                           */
                  TempLength = 0;
                  if(ProcessImagesListingObject(CurrentObjectLength, CurrentObjectBuffer, &TempLength, NULL) == 0)
                  {
                     /* Next attempt to allocate the memory required to */
                     /* build the image list.                           */
                     if((ImageListPtr = malloc(sizeof(ImageListEntry_t)*TempLength)) != NULL)
                     {
                        /* Initialize the list entries to a known state.*/
                        memset(ImageListPtr, 0, sizeof(ImageListEntry_t)*TempLength);

                        /* Now process the images listing object and    */
                        /* actually build the image list.               */
                        if(ProcessImagesListingObject(CurrentObjectLength, CurrentObjectBuffer, &TempLength, ImageListPtr) == 0)
                        {
                           if(ImageList)
                              free(ImageList);

                           NumberImageListEntries = TempLength;

                           ImageList              = ImageListPtr;
                        }
                        else
                        {
                           /* An error occurred free the previously     */
                           /* allocated resources.                      */
                           free(ImageListPtr);
                        }
                     }
                  }
               }
            }

            /* If an error occurred, or the operation was completed     */
            /* update the current operation.                            */
            if((Result != 0) || (BIPEventData->Event_Data.BIP_Get_Images_List_Confirmation_Data->ResponseCode != BIP_OBEX_RESPONSE_CONTINUE))
            {
               /* Reset the Current Operation.                          */
               CurrentOperation = coNone;

               /* Reset the state variables and free any allocated      */
               /* resources.                                            */
               CurrentObjectIndex  = 0;
               CurrentObjectLength = 0;

               if((CurrentObjectBuffer != NULL) && (CurrentObjectBufferAllocated))
                  free(CurrentObjectBuffer);

               CurrentObjectBuffer          = NULL;
               CurrentObjectBufferAllocated = FALSE;
            }
            break;
         case etBIP_Get_Image_Properties_Indication:
            /* A Get Image Properties Indication was received, display  */
            /* all relevant information.                                */
            printf("BIP Get Image Properties Indication, ID: %u.\r\n", BIPEventData->Event_Data.BIP_Get_Image_Properties_Indication_Data->BIPID);

            /* Initialize the Response Code to an Error Response.       */
            ResponseCode = BIP_OBEX_RESPONSE_BAD_REQUEST;

            /* Check to see if this is the first phase of the operation.*/
            if(CurrentOperation == coNone)
            {
               /* Make sure that the required parameters appear to be at*/
               /* least semi-valid.                                     */
               if(BIPEventData->Event_Data.BIP_Get_Image_Properties_Indication_Data->ImageHandle != NULL)
               {
                  /* Display the Image Handle of the image to get the   */
                  /* properties for.                                    */
                  printf("    ImageHandle: %s\r\n", BIPEventData->Event_Data.BIP_Get_Image_Properties_Indication_Data->ImageHandle);

                  /* Check to make sure that the path to the file to    */
                  /* retrieve the properties for can be successfully    */
                  /* built.                                             */
                  if(sizeof(CurrentFile) >= (strlen(RootDirectory) + (22*sizeof(char))))
                  {
                     /* Build the full path to the storage location for */
                     /* the file whose properties are being retrieved.  */
                     strcpy(CurrentFile, RootDirectory);
                     sprintf(&CurrentFile[strlen(CurrentFile)], "%c%c%cFOLDR", BIPEventData->Event_Data.BIP_Get_Image_Properties_Indication_Data->ImageHandle[0], BIPEventData->Event_Data.BIP_Get_Image_Properties_Indication_Data->ImageHandle[1], BIPEventData->Event_Data.BIP_Get_Image_Properties_Indication_Data->ImageHandle[2]);
                     sprintf(&CurrentFile[strlen(CurrentFile)], "/IMGE%c%c%c%c.JPG", BIPEventData->Event_Data.BIP_Get_Image_Properties_Indication_Data->ImageHandle[3], BIPEventData->Event_Data.BIP_Get_Image_Properties_Indication_Data->ImageHandle[4], BIPEventData->Event_Data.BIP_Get_Image_Properties_Indication_Data->ImageHandle[5], BIPEventData->Event_Data.BIP_Get_Image_Properties_Indication_Data->ImageHandle[6]);

                     /* Initialize the state variables to a known state.*/
                     CurrentObjectIndex           = 0;
                     CurrentObjectLength          = 0;
                     CurrentObjectBuffer          = NULL;
                     CurrentObjectBufferAllocated = FALSE;

                     /* Get the length required to build the Image      */
                     /* Properties for the selected Image.              */
                     if(BuildImagePropertiesObject(CurrentFile, BIPEventData->Event_Data.BIP_Get_Image_Properties_Indication_Data->ImageHandle, &CurrentObjectLength, NULL) == 0)
                     {
                        /* The Length was successfully determined, next */
                        /* attempt to allocate a buffer to build the    */
                        /* object into.                                 */
                        if((CurrentObjectBuffer = (unsigned char *)malloc(CurrentObjectLength)) != NULL)
                        {
                           /* Now attempt to build the Image Properties */
                           /* Object.                                   */
                           if(BuildImagePropertiesObject(CurrentFile, BIPEventData->Event_Data.BIP_Get_Image_Properties_Indication_Data->ImageHandle, &CurrentObjectLength, CurrentObjectBuffer) == 0)
                           {
                              /* This is the first phase of the         */
                              /* operation.  Set up the state variables.*/
                              CurrentOperation             = coGetImageProperties;
                              ResponseCode                 = BIP_OBEX_RESPONSE_CONTINUE;
                              CurrentObjectBufferAllocated = TRUE;
                           }
                           else
                           {
                              /* An error occurred while attempting to  */
                              /* build the object, free the previously  */
                              /* allocated memory.                      */
                              free(CurrentObjectBuffer);
                              CurrentObjectBuffer          = NULL;
                              CurrentObjectIndex           = 0;
                              CurrentObjectLength          = 0;
                              CurrentObjectBufferAllocated = FALSE;
                           }
                        }
                     }
                  }
               }
            }
            else
            {
               /* This is not the first phase of the operation, check to*/
               /* see if this is a continuation of a currently on going */
               /* operation.                                            */
               if(CurrentOperation == coGetImageProperties)
               {
                  /* Check to see if the entire object has been sent.   */
                  if(CurrentObjectIndex >= CurrentObjectLength)
                  {
                     /* The entire object has been sent, set the        */
                     /* response code to OK to complete this operation. */
                     ResponseCode = BIP_OBEX_RESPONSE_OK;
                  }
                  else
                  {
                     /* The entire object has not been sent, set the    */
                     /* response code to CONTINUE.                      */
                     ResponseCode = BIP_OBEX_RESPONSE_CONTINUE;
                  }
               }
            }

            /* Attempt to submit the response.                          */
            if((Result = BIP_Get_Image_Properties_Response(BluetoothStackID, BIPID, ResponseCode, (CurrentObjectLength-CurrentObjectIndex), &(CurrentObjectBuffer[CurrentObjectIndex]), &AmountWritten)) == 0)
            {
               /* Adjust the index into the current object by the amount*/
               /* written.                                              */
               CurrentObjectIndex += AmountWritten;

               printf("BIP_Get_Image_Properties_Response() Success, CurrentObjectIndex: %u, CurrentObjectLength: %u.\r\n", CurrentObjectIndex, CurrentObjectLength);
            }
            else
            {
               /* An error occurred while attempting to submit the Get  */
               /* Capabilites Response.                                 */
               printf("BIP_Get_Image_Properties_Response() Failure: %d.\r\n", Result);
            }

            /* If an error occurred, or the operation was completed     */
            /* update the current operation.                            */
            if((Result != 0) || (ResponseCode != BIP_OBEX_RESPONSE_CONTINUE))
            {
               /* Reset the Current Operation.                          */
               CurrentOperation = coNone;

               /* Free the Current Object Buffer and reset the state    */
               /* variables.                                            */
               if((CurrentObjectBuffer != NULL) && (CurrentObjectBufferAllocated))
                  free(CurrentObjectBuffer);

               CurrentObjectBuffer          = NULL;
               CurrentObjectLength          = 0;
               CurrentObjectIndex           = 0;
               CurrentObjectBufferAllocated = FALSE;

               CurrentFile[0] = '\0';
            }
            break;
         case etBIP_Get_Image_Properties_Confirmation:
            /* A Get Image Properties Confirmation was received, display*/
            /* all relevant information.                                */
            printf("BIP Get Image Properties Confirmation, ID: %u, ResponseCode: 0x%02X, DataLength: %u.\r\n", BIPEventData->Event_Data.BIP_Get_Image_Properties_Confirmation_Data->BIPID, BIPEventData->Event_Data.BIP_Get_Image_Properties_Confirmation_Data->ResponseCode, BIPEventData->Event_Data.BIP_Get_Image_Properties_Confirmation_Data->DataLength);

            /* Check for non-error response code.                       */
            if((BIPEventData->Event_Data.BIP_Get_Image_Properties_Confirmation_Data->ResponseCode == BIP_OBEX_RESPONSE_CONTINUE) || (BIPEventData->Event_Data.BIP_Get_Image_Properties_Confirmation_Data->ResponseCode == BIP_OBEX_RESPONSE_OK))
            {
               /* Next check to make sure that their is some data       */
               /* associated with this phase of the operation.          */
               if((BIPEventData->Event_Data.BIP_Get_Image_Properties_Confirmation_Data->DataLength > 0) && (BIPEventData->Event_Data.BIP_Get_Image_Properties_Confirmation_Data->DataBuffer != NULL))
               {
                  /* There is some data associated with this phase of   */
                  /* the operation.                                     */
                  TempLength = CurrentObjectLength + BIPEventData->Event_Data.BIP_Get_Image_Properties_Confirmation_Data->DataLength;

                  /* Next attempt to allocated a buffer large enough to */
                  /* hold the exist object data including the newly     */
                  /* received portion.                                  */
                  if((TempBufferPtr = (unsigned char *)malloc(TempLength)) != NULL)
                  {
                     memcpy(TempBufferPtr, CurrentObjectBuffer, CurrentObjectLength);
                     memcpy(&TempBufferPtr[CurrentObjectLength], BIPEventData->Event_Data.BIP_Get_Image_Properties_Confirmation_Data->DataBuffer, BIPEventData->Event_Data.BIP_Get_Image_Properties_Confirmation_Data->DataLength);

                     CurrentObjectLength += BIPEventData->Event_Data.BIP_Get_Image_Properties_Confirmation_Data->DataLength;

                     if((CurrentObjectBuffer != NULL) && (CurrentObjectBufferAllocated))
                        free(CurrentObjectBuffer);

                     CurrentObjectBuffer          = TempBufferPtr;
                     CurrentObjectBufferAllocated = TRUE;
                  }
               }
            }

            /* Check to see if this was the last phase of this          */
            /* operation.                                               */
            if(BIPEventData->Event_Data.BIP_Get_Image_Properties_Confirmation_Data->ResponseCode == BIP_OBEX_RESPONSE_CONTINUE)
            {
               /* The response code indicates there is more to the      */
               /* object being received.  Request the next piece of the */
               /* object.                                               */
               if((Result = BIP_Get_Image_Properties_Request(BluetoothStackID, BIPID, NULL)) == 0)
               {
                  /* The function was submitted successfully.           */
                  printf("BIP_Get_Image_Properties_Request: Function Successful.\r\n");
               }
               else
               {
                  /* There was an error submitting the function.        */
                  printf("BIP_Get_Image_Properties_Request() Failure: %d.\r\n", Result);
               }
            }
            else
            {
               if(BIPEventData->Event_Data.BIP_Get_Image_Properties_Confirmation_Data->ResponseCode == BIP_OBEX_RESPONSE_OK)
               {
                  /* This was the final phase of this operation.        */
                  /* Display the object that has been received.         */
                  DisplayObject(CurrentObjectLength, CurrentObjectBuffer);
               }
            }

            /* If an error occurred, or the operation was completed     */
            /* update the current operation.                            */
            if((Result != 0) || (BIPEventData->Event_Data.BIP_Get_Image_Properties_Confirmation_Data->ResponseCode != BIP_OBEX_RESPONSE_CONTINUE))
            {
               /* Reset the Current Operation.                          */
               CurrentOperation = coNone;

               /* Reset the state variables and free any allocated      */
               /* resources.                                            */
               CurrentObjectIndex  = 0;
               CurrentObjectLength = 0;

               if((CurrentObjectBuffer != NULL) && (CurrentObjectBufferAllocated))
                  free(CurrentObjectBuffer);

               CurrentObjectBuffer          = NULL;
               CurrentObjectBufferAllocated = FALSE;
            }
            break;
         case etBIP_Get_Image_Indication:
            /* A Get Image Indication was received, display all relevant*/
            /* information.                                             */
            printf("BIP Get Image Indication, ID: %u.\r\n", BIPEventData->Event_Data.BIP_Get_Image_Indication_Data->BIPID);

            /* Initialize the Response Code to an Error Response.       */
            ResponseCode = BIP_OBEX_RESPONSE_BAD_REQUEST;

            /* Check to see if this is the first phase of the operation.*/
            if(CurrentOperation == coNone)
            {
               /* Check to make sure that an image with the request     */
               /* handle actually exists.                               */
               if((BIPEventData->Event_Data.BIP_Get_Image_Indication_Data->ImageHandle != NULL) && (TestImageHandle(BIPEventData->Event_Data.BIP_Get_Image_Indication_Data->ImageHandle)))
               {
                  /* An image with the specified handle exists.  Display*/
                  /* the Image Handle of the image to get.              */
                  printf("    ImageHandle: %s\r\n", BIPEventData->Event_Data.BIP_Get_Image_Indication_Data->ImageHandle);

                  /* Check to see if the Image Descriptor appears to be */
                  /* valid.                                             */
                  /* ** NOTE ** This application only handle sending the*/
                  /*            image in its native format.             */
                  if((BIPEventData->Event_Data.BIP_Get_Image_Indication_Data->ImageDescriptorLength > 0) && (BIPEventData->Event_Data.BIP_Get_Image_Indication_Data->ImageDescriptor != NULL))
                  {
                     /* Display the Image Descriptor Object Received.   */
                     DisplayObject(BIPEventData->Event_Data.BIP_Get_Image_Indication_Data->ImageDescriptorLength, BIPEventData->Event_Data.BIP_Get_Image_Indication_Data->ImageDescriptor);
                  }

                  /* Check to make sure that the path to the new        */
                  /* filename can be successfully built.                */
                  if(sizeof(CurrentFile) >= (strlen(RootDirectory) + (22*sizeof(char))))
                  {
                     /* Initialize the current file information.        */
                     FirstPhase             = TRUE;
                     CurrentFileIndex       = 0;
                     CurrentFileSize        = 0;
                     CurrentFileBufferIndex = 0;

                     /* Build the full path to the storage location for */
                     /* the file being retrieved.                       */
                     strcpy(CurrentFile, RootDirectory);
                     sprintf(&CurrentFile[strlen(CurrentFile)], "%c%c%cFOLDR", BIPEventData->Event_Data.BIP_Get_Image_Indication_Data->ImageHandle[0], BIPEventData->Event_Data.BIP_Get_Image_Indication_Data->ImageHandle[1], BIPEventData->Event_Data.BIP_Get_Image_Indication_Data->ImageHandle[2]);
                     sprintf(&CurrentFile[strlen(CurrentFile)], "/IMGE%c%c%c%c.JPG", BIPEventData->Event_Data.BIP_Get_Image_Indication_Data->ImageHandle[3], BIPEventData->Event_Data.BIP_Get_Image_Indication_Data->ImageHandle[4], BIPEventData->Event_Data.BIP_Get_Image_Indication_Data->ImageHandle[5], BIPEventData->Event_Data.BIP_Get_Image_Indication_Data->ImageHandle[6]);

                     /* Attempt to get some file data to send.          */
                     if(GetFileData() == FILE_DATA_FILE_IO_SUCCESS)
                     {
                        /* Some data was successfully read from the     */
                        /* file, set the current operation and response */
                        /* code.                                        */
                        CurrentOperation = coGetImage;
                        ResponseCode     = BIP_OBEX_RESPONSE_CONTINUE;
                     }
                  }
               }
            }
            else
            {
               /* This is not the first phase of the operation, check to*/
               /* see if this is a continuation of a currently on going */
               /* operation.                                            */
               if(CurrentOperation == coGetImage)
               {
                  /* Get some more file data.                           */
                  GetFileData();

                  /* Check to see if there is more file data to be sent.*/
                  if(CurrentFileBufferIndex > 0)
                  {
                     /* There is more file data to be sent, in this case*/
                     /* the Response Code needs to be set to continue.  */
                     ResponseCode = BIP_OBEX_RESPONSE_CONTINUE;
                  }
                  else
                  {
                     /* There is no more file data to be sent, in this  */
                     /* case the Response Code needs to be set to ok.   */
                     ResponseCode = BIP_OBEX_RESPONSE_OK;
                  }
               }
            }

            /* Attempt to submit the response.                          */
            if((Result = BIP_Get_Image_Response(BluetoothStackID, BIPID, ResponseCode, CurrentFileSize, CurrentFileBufferIndex, CurrentFileBuffer, &AmountWritten)) == 0)
            {
               /* The function was submitted successfully.              */
               printf("BIP_Get_Image_Response() Success, CurrentFileIndex: %u, CurrentFileSize: %u.\r\n", (unsigned int)CurrentFileIndex, (unsigned int)CurrentFileSize);

               /* Adjust the current file buffer.                       */
               if(AmountWritten)
               {
                  memmove(CurrentFileBuffer, &CurrentFileBuffer[AmountWritten], (CurrentFileBufferIndex - AmountWritten));
                  CurrentFileBufferIndex -= AmountWritten;
               }
            }
            else
            {
               /* There was an error submitting the function.           */
               printf("BIP_Get_Image_Response() Failure: %d.\r\n", Result);
            }

            /* If an error occurred, or the operation was completed     */
            /* update the current operation.                            */
            if((Result != 0) || (ResponseCode != BIP_OBEX_RESPONSE_CONTINUE))
            {
               /* Reset the Current Operation.                          */
               CurrentOperation       = coNone;

               /* Reset the state information.                          */
               FirstPhase             = TRUE;
               CurrentFileIndex       = 0;
               CurrentFileSize        = 0;
               CurrentFileBufferIndex = 0;
            }
            break;
         case etBIP_Get_Image_Confirmation:
            /* A Get Image Confirmation was received, display all       */
            /* relevant information.                                    */
            printf("BIP Get Image Confirmation, ID: %u, ResponseCode: 0x%02X, ImageLength: %u.\r\n", BIPEventData->Event_Data.BIP_Get_Image_Confirmation_Data->BIPID, BIPEventData->Event_Data.BIP_Get_Image_Confirmation_Data->ResponseCode, BIPEventData->Event_Data.BIP_Get_Image_Confirmation_Data->ImageLength);

            /* Check for non-error response code.                       */
            if((BIPEventData->Event_Data.BIP_Get_Image_Confirmation_Data->ResponseCode == BIP_OBEX_RESPONSE_CONTINUE) || (BIPEventData->Event_Data.BIP_Get_Image_Confirmation_Data->ResponseCode == BIP_OBEX_RESPONSE_OK))
            {
               /* Check to see if this segment of the operation contains*/
               /* any retrieved data.                                   */
               if((BIPEventData->Event_Data.BIP_Get_Image_Confirmation_Data->DataLength > 0) && (BIPEventData->Event_Data.BIP_Get_Image_Confirmation_Data->DataBuffer != NULL))
               {
                  /* Put the new received file data to the file.        */
                  PutFileData(BIPEventData->Event_Data.BIP_Get_Image_Confirmation_Data->DataLength, BIPEventData->Event_Data.BIP_Get_Image_Confirmation_Data->DataBuffer);

                  /* Display the current file information.              */
                  printf("    DataLength: %u, CurrentFileIndex: %u.\r\n", (unsigned int)BIPEventData->Event_Data.BIP_Get_Image_Confirmation_Data->DataLength, (unsigned int)CurrentFileIndex);
               }

               /* Check to see if this was the last phase of this       */
               /* operation.                                            */
               if(BIPEventData->Event_Data.BIP_Get_Image_Confirmation_Data->ResponseCode == BIP_OBEX_RESPONSE_CONTINUE)
               {
                  /* The response code indicates there is more to the   */
                  /* object being received.  Request the next piece of  */
                  /* the object.                                        */
                  if((Result = BIP_Get_Image_Request(BluetoothStackID, BIPID, 0, 0, NULL)) == 0)
                  {
                     /* The function was submitted successfully.        */
                     printf("BIP_Get_Image_Request: Function Successful.\r\n");
                  }
                  else
                  {
                     /* There was an error submitting the function.     */
                     printf("BIP_Get_Image_Request() Failure: %d.\r\n", Result);
                  }
               }
            }

            /* If an error occurred, or the operation was completed     */
            /* update the current operation.                            */
            if((Result != 0) || (BIPEventData->Event_Data.BIP_Get_Image_Confirmation_Data->ResponseCode != BIP_OBEX_RESPONSE_CONTINUE))
            {
               /* Reset the Current Operation.                          */
               CurrentOperation = coNone;

               /* Reset the state variables.                            */
               CurrentFile[0]   = '\0';
               CurrentFileIndex = 0;
            }
            break;
         case etBIP_Get_Linked_Thumbnail_Indication:
            /* A Get Linked Thumbnail Indication was received, display  */
            /* all relevant information.                                */
            printf("BIP Get Linked Thumbnail Indication, ID: %u.\r\n", BIPEventData->Event_Data.BIP_Get_Linked_Thumbnail_Indication_Data->BIPID);

            /* Initialize the Response Code to an Error Response.       */
            ResponseCode = BIP_OBEX_RESPONSE_BAD_REQUEST;

            /* Check to see if this is the first phase of the operation.*/
            if(CurrentOperation == coNone)
            {
               /* Check to make sure that the specified image handle    */
               /* appears to be at least semi-valid.                    */
               if(BIPEventData->Event_Data.BIP_Get_Linked_Thumbnail_Indication_Data->ImageHandle != NULL)
               {
                  /* The Image Handle appears to be at least semi-valid.*/
                  /* Display the Image Handle of the image to get.      */
                  printf("    ImageHandle: %s\r\n", BIPEventData->Event_Data.BIP_Get_Linked_Thumbnail_Indication_Data->ImageHandle);

                  /* Check to make sure that the path to the new        */
                  /* filename can be successfully built.                */
                  if(sizeof(CurrentFile) >= (strlen(RootDirectory) + (22*sizeof(char))))
                  {
                     /* Initialize the current file information.        */
                     FirstPhase             = TRUE;
                     CurrentFileIndex       = 0;
                     CurrentFileSize        = 0;
                     CurrentFileBufferIndex = 0;

                     /* Build the full path to the storage location for */
                     /* the thumbnail file being retrieved.             */
                     strcpy(CurrentFile, RootDirectory);
                     sprintf(&CurrentFile[strlen(CurrentFile)], "%c%c%cFOLDR", BIPEventData->Event_Data.BIP_Get_Linked_Thumbnail_Indication_Data->ImageHandle[0], BIPEventData->Event_Data.BIP_Get_Linked_Thumbnail_Indication_Data->ImageHandle[1], BIPEventData->Event_Data.BIP_Get_Linked_Thumbnail_Indication_Data->ImageHandle[2]);
                     sprintf(&CurrentFile[strlen(CurrentFile)], "/THMB%c%c%c%c.JPG", BIPEventData->Event_Data.BIP_Get_Linked_Thumbnail_Indication_Data->ImageHandle[3], BIPEventData->Event_Data.BIP_Get_Linked_Thumbnail_Indication_Data->ImageHandle[4], BIPEventData->Event_Data.BIP_Get_Linked_Thumbnail_Indication_Data->ImageHandle[5], BIPEventData->Event_Data.BIP_Get_Linked_Thumbnail_Indication_Data->ImageHandle[6]);

                     /* Attempt to get some file data to send.          */
                     if(GetFileData() == FILE_DATA_FILE_IO_SUCCESS)
                     {
                        /* Some data was successfully read from the     */
                        /* file, set the current operation and response */
                        /* code.                                        */
                        CurrentOperation = coGetLinkedThumbnail;
                        ResponseCode     = BIP_OBEX_RESPONSE_CONTINUE;
                     }
                  }
               }
            }
            else
            {
               /* This is not the first phase of the operation, check to*/
               /* see if this is a continuation of a currently on going */
               /* operation.                                            */
               if(CurrentOperation == coGetLinkedThumbnail)
               {
                  /* Get some more file data.                           */
                  GetFileData();

                  /* Check to see if there is more file data to be sent.*/
                  if(CurrentFileBufferIndex > 0)
                  {
                     /* There is more file data to be sent, in this case*/
                     /* the Response Code needs to be set to continue.  */
                     ResponseCode = BIP_OBEX_RESPONSE_CONTINUE;
                  }
                  else
                  {
                     /* There is no more file data to be sent, in this  */
                     /* case the Response Code needs to be set to ok.   */
                     ResponseCode = BIP_OBEX_RESPONSE_OK;
                  }
               }
            }

            /* Attempt to submit the response.                          */
            if((Result = BIP_Get_Linked_Thumbnail_Response(BluetoothStackID, BIPID, ResponseCode, CurrentFileBufferIndex, CurrentFileBuffer, &AmountWritten)) == 0)
            {
               /* The function was submitted successfully.              */
               printf("BIP_Get_Linked_Thumbnail_Response() Success, CurrentFileIndex: %u, CurrentFileSize: %u.\r\n", (unsigned int)CurrentFileIndex, (unsigned int)CurrentFileSize);

               /* Adjust the current file buffer.                       */
               if(AmountWritten)
               {
                  memmove(CurrentFileBuffer, &CurrentFileBuffer[AmountWritten], (CurrentFileBufferIndex - AmountWritten));
                  CurrentFileBufferIndex -= AmountWritten;
               }
            }
            else
            {
               /* There was an error submitting the function.           */
               printf("BIP_Get_Linked_Thumbnail_Response() Failure: %d.\r\n", Result);
            }

            /* If an error occurred, or the operation was completed     */
            /* update the current operation.                            */
            if((Result != 0) || (ResponseCode != BIP_OBEX_RESPONSE_CONTINUE))
            {
               /* Reset the Current Operation.                          */
               CurrentOperation       = coNone;

               /* Reset the state information.                          */
               FirstPhase             = TRUE;
               CurrentFileIndex       = 0;
               CurrentFileSize        = 0;
               CurrentFileBufferIndex = 0;
            }
            break;
         case etBIP_Get_Linked_Thumbnail_Confirmation:
            /* A Get Linked Thumbnail Confirmation was received, display*/
            /* all relevant information.                                */
            printf("BIP Get Linked Thumbnail Confirmation, ID: %u, ResponseCode: 0x%02X.\r\n", BIPEventData->Event_Data.BIP_Get_Linked_Thumbnail_Confirmation_Data->BIPID, BIPEventData->Event_Data.BIP_Get_Linked_Thumbnail_Confirmation_Data->ResponseCode);

            /* Check for non-error response code.                       */
            if((BIPEventData->Event_Data.BIP_Get_Linked_Thumbnail_Confirmation_Data->ResponseCode == BIP_OBEX_RESPONSE_CONTINUE) || (BIPEventData->Event_Data.BIP_Get_Linked_Thumbnail_Confirmation_Data->ResponseCode == BIP_OBEX_RESPONSE_OK))
            {
               /* Check to see if this segment of the operation contains*/
               /* any retrieved data.                                   */
               if((BIPEventData->Event_Data.BIP_Get_Linked_Thumbnail_Confirmation_Data->DataLength > 0) && (BIPEventData->Event_Data.BIP_Get_Linked_Thumbnail_Confirmation_Data->DataBuffer != NULL))
               {
                  /* Put the new received file data to the file.        */
                  PutFileData(BIPEventData->Event_Data.BIP_Get_Linked_Thumbnail_Confirmation_Data->DataLength, BIPEventData->Event_Data.BIP_Get_Linked_Thumbnail_Confirmation_Data->DataBuffer);

                  /* Display the current file information.              */
                  printf("    DataLength: %u, CurrentFileIndex: %u.\r\n", (unsigned int)BIPEventData->Event_Data.BIP_Get_Linked_Thumbnail_Confirmation_Data->DataLength, (unsigned int)CurrentFileIndex);
               }

               /* Check to see if this was the last phase of this       */
               /* operation.                                            */
               if(BIPEventData->Event_Data.BIP_Get_Linked_Thumbnail_Confirmation_Data->ResponseCode == BIP_OBEX_RESPONSE_CONTINUE)
               {
                  /* The response code indicates there is more to the   */
                  /* object being received.  Request the next piece of  */
                  /* the object.                                        */
                  if((Result = BIP_Get_Linked_Thumbnail_Request(BluetoothStackID, BIPID, NULL)) == 0)
                  {
                     /* The function was submitted successfully.        */
                     printf("BIP_Get_Linked_Thumbnail_Request: Function Successful.\r\n");
                  }
                  else
                  {
                     /* There was an error submitting the function.     */
                     printf("BIP_Get_Linked_Thumbnail_Request() Failure: %d.\r\n", Result);
                  }
               }
            }

            /* If an error occurred, or the operation was completed     */
            /* update the current operation.                            */
            if((Result != 0) || (BIPEventData->Event_Data.BIP_Get_Linked_Thumbnail_Confirmation_Data->ResponseCode != BIP_OBEX_RESPONSE_CONTINUE))
            {
               /* Reset the Current Operation.                          */
               CurrentOperation = coNone;

               /* Reset the state variables.                            */
               CurrentFile[0]   = '\0';
               CurrentFileIndex = 0;
            }
            break;
         case etBIP_Delete_Image_Indication:
            /* A Delete Image Indication was received, display all      */
            /* relevant information.                                    */
            printf("BIP Delete Image Indication, ID: %u.\r\n", BIPEventData->Event_Data.BIP_Delete_Image_Indication_Data->BIPID);

            /* Initialize the Response Code to an Error Response.       */
            ResponseCode = BIP_OBEX_RESPONSE_BAD_REQUEST;

            /* Check to make sure that is not currently an on going     */
            /* operation.                                               */
            if(CurrentOperation == coNone)
            {
               /* There is not currently an on going operation, check to*/
               /* make sure that the image handle that was specified    */
               /* appears to be valid.                                  */
               if(BIPEventData->Event_Data.BIP_Delete_Image_Indication_Data->ImageHandle != NULL)
               {
                  /* The Image Handle appears to be at least semi-valid.*/
                  /* Display the Image Handle of the image to get.      */
                  printf("    ImageHandle: %s\r\n", BIPEventData->Event_Data.BIP_Delete_Image_Indication_Data->ImageHandle);

                  /* Build the full path to the storage location for the*/
                  /* image file being deleted.                          */
                  strcpy(CurrentFile, RootDirectory);
                  sprintf(&CurrentFile[strlen(CurrentFile)], "%c%c%cFOLDR", BIPEventData->Event_Data.BIP_Delete_Image_Indication_Data->ImageHandle[0], BIPEventData->Event_Data.BIP_Delete_Image_Indication_Data->ImageHandle[1], BIPEventData->Event_Data.BIP_Delete_Image_Indication_Data->ImageHandle[2]);
                  sprintf(&CurrentFile[strlen(CurrentFile)], "/IMGE%c%c%c%c.JPG", BIPEventData->Event_Data.BIP_Delete_Image_Indication_Data->ImageHandle[3], BIPEventData->Event_Data.BIP_Delete_Image_Indication_Data->ImageHandle[4], BIPEventData->Event_Data.BIP_Delete_Image_Indication_Data->ImageHandle[5], BIPEventData->Event_Data.BIP_Delete_Image_Indication_Data->ImageHandle[6]);

                  /* Display which image is being deleted.              */
                  printf("Deleting Image: %s\r\n", CurrentFile);

                  /* Attempt to Delete the image file.                  */
                  unlink(CurrentFile);

                  /* Build the full path to the storage location for the*/
                  /* thumbnail associated with the image handle file    */
                  /* being deleted.                                     */
                  strcpy(CurrentFile, RootDirectory);
                  sprintf(&CurrentFile[strlen(CurrentFile)], "%c%c%cFOLDR", BIPEventData->Event_Data.BIP_Delete_Image_Indication_Data->ImageHandle[0], BIPEventData->Event_Data.BIP_Delete_Image_Indication_Data->ImageHandle[1], BIPEventData->Event_Data.BIP_Delete_Image_Indication_Data->ImageHandle[2]);
                  sprintf(&CurrentFile[strlen(CurrentFile)], "/THMB%c%c%c%c.JPG", BIPEventData->Event_Data.BIP_Delete_Image_Indication_Data->ImageHandle[3], BIPEventData->Event_Data.BIP_Delete_Image_Indication_Data->ImageHandle[4], BIPEventData->Event_Data.BIP_Delete_Image_Indication_Data->ImageHandle[5], BIPEventData->Event_Data.BIP_Delete_Image_Indication_Data->ImageHandle[6]);

                  /* Display which thumbnail is being deleted.          */
                  printf("Deleting Thumbnail: %s\r\n", CurrentFile);

                  /* Attempt to Delete the Thumbnail file.              */
                  unlink(CurrentFile);

                  /* Set the Response Code to indicate success.         */
                  ResponseCode = BIP_OBEX_RESPONSE_OK;
               }
            }

            /* Attempt to submit the response.                          */
            if((Result = BIP_Delete_Image_Response(BluetoothStackID, BIPID, ResponseCode)) == 0)
            {
               /* The function was submitted successfully.              */
               printf("BIP_Delete_Image_Response() Success, ResponseCode: 0x%02X\r\n", ResponseCode);
            }
            else
            {
               /* There was an error submitting the function.           */
               printf("BIP_Delete_Image_Response() Failure: %d.\r\n", Result);
            }
            break;
         case etBIP_Delete_Image_Confirmation:
            /* A Delete Image Confirmation was received, display all    */
            /* relevant information.                                    */
            printf("BIP Delete Image Confirmation, ID: %u, ResponseCode: 0x%02X.\r\n", BIPEventData->Event_Data.BIP_Delete_Image_Confirmation_Data->BIPID, BIPEventData->Event_Data.BIP_Delete_Image_Confirmation_Data->ResponseCode);

            if((ImageDeleteIndex != (-1)) && (BIPEventData->Event_Data.BIP_Delete_Image_Confirmation_Data->ResponseCode == BIP_OBEX_RESPONSE_OK))
            {
               /* Flag the element specified at the current delete index*/
               /* as deleted.                                           */
               if((ImageDeleteIndex < NumberImageListEntries) && (ImageList))
                  ImageList[ImageDeleteIndex].ImageHandle[0] = '\0';
            }

            ImageDeleteIndex = -1;

            /* Reset the Current Operation.                             */
            CurrentOperation = coNone;
            break;
         default:
            /* Unknown BIP Event Received.                              */
            printf("Unknown BIP Event Received.\r\n");
            break;
      }

      if(IsClient)
         printf("\r\nClient>");
      else
         printf("\r\nServer>");

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
            /* The stack was opened successfully.  Now set some         */
            /* defaults.                                                */

            /* First, attempt to set the Device to be Connectable.      */
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
                     /* Next, check to see if the program is running in */
                     /* Client or Server Mode.                          */
                     if(IsClient)
                     {
                        /* The program is currently running in Client   */
                        /* Mode.  Start the User Interface.             */
                        UserInterface_Client();
                     }
                     else
                     {
                        /* The Server has been set up successfully,     */
                        /* start the User Interface.                    */
                        UserInterface_Server();
                     }
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

