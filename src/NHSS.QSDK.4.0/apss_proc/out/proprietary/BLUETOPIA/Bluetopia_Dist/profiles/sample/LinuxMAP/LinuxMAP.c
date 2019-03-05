/*****< linuxmap.c >***********************************************************/
/*      Copyright 2009 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  LINUXMAP - Simple Linux application using Message Access Profile (MAP).   */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   08/21/09  D. Lange       Initial creation.                               */
/******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>

#include "LinuxMAP.h"      /* Main Application Prototypes and Constants.      */

#include "MsgStore.h"      /* Sample Data Message Store Prototypes/Constants. */

#include "SS1BTPS.h"       /* Includes for the SS1 Bluetooth Protocol Stack.  */
#include "SS1BTMAP.h"      /* Includes for the SS1 Message Access Profile.    */
#include "SS1BTDBG.h"      /* Includes/Constants for Bluetooth Debugging.     */

#define LOCAL_NAME_ROOT                      "LinuxMAP"  /* Root of the local */
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

#define MAX_SUPPORTED_COMMANDS                     (48)  /* Denotes the       */
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
                                                         /* create a MAP      */
                                                         /* Server.           */

#define READ_STATUS_STRING_OFFSET                    32  /* Offset of the Read*/
                                                         /* Status in a       */
                                                         /* Message string.   */

#define INDENT_LENGTH                               (3)  /* Denotes the number*/
                                                         /* of character      */
                                                         /* spaces to be used */
                                                         /* for indenting when*/
                                                         /* displaying SDP    */
                                                         /* Data Elements.    */

   /* The following constants represent the default log file names that */
   /* are used if no Log file name is specified when enabling debug.    */
#define DEFAULT_DEBUG_LOG_FILE_NAME  "LinuxMAP_ASC.log"
#define DEFAULT_DEBUG_FTS_FILE_NAME  "LinuxMAP_FTS.log"

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

   /* The following enumerated type represents all of the operations    */
   /* that can be on-going at any given instant (either client or       */
   /* server).                                                          */
typedef enum
{
   coNone,
   coAbort,
   coSetFolder,
   coGetFolderListing,
   coGetMessageListing,
   coGetMessage,
   coSetMessageStatus,
   coPushMessage,
   coUpdateInbox,
   coDisconnect
} CurrentOperation_t;

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

static unsigned int        MAPID;                   /* Variable used to hold the       */
                                                    /* MAP Client/Server ID of the     */
                                                    /* currently active MAP Profile    */
                                                    /* Role.                           */

static DWord_t             ServerSDPRecordHandle;   /* Variable used to hold the       */
                                                    /* MAP Server SDP Service Record   */
                                                    /* Handle of the MAP Server SDP    */
                                                    /* Service Record.                 */

static Boolean_t           Connected;               /* Variable which reflects whether */
                                                    /* or not there is an active       */
                                                    /* (on-going) connection present.  */

static DWord_t             NotificationSDPRecordHandle; /* Variable used to hold the   */
                                                    /* MAP Notification Server SDP     */
                                                    /* Service Record Handle of the    */
                                                    /* MAP Notification Server SDP     */
                                                    /* Service Record.                 */

static unsigned int        MAPNID;                  /* Variable used to hold the       */
                                                    /* MAP Notification Client/Server  */
                                                    /* ID of the currently active      */
                                                    /* MAP Notification Profile Role.  */

static Boolean_t           NotificationConnected;   /* Variable which reflects whether */
                                                    /* or not there is an active       */
                                                    /* (on-going) Notification         */
                                                    /* connection present.             */

static char                ReceiveMessagePath[512]; /* Variable which holds the current*/
                                                    /* path where received messages    */
                                                    /* (server side) are stored.       */

static CurrentOperation_t  CurrentOperation;        /* Variable which holds the current*/
                                                    /* (on-going operation).           */

static char                CurrentFileName[512];    /* Variables are used when sending */
static unsigned int        CurrentBufferSize;       /* and receiving data.  These      */
static unsigned int        CurrentBufferSent;       /* variables are used by both the  */
static char               *CurrentBuffer;           /* client and server when an       */
                                                    /* operation requires data to be   */
                                                    /* transferred to the remote       */
                                                    /* device.                         */

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
static void BD_ADDRToStr(BD_ADDR_t Board_Address, char *BoardStr);
static void WriteEIRInformation(char *LocalName);

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

static int OpenNotificationServer(ParameterList_t *TempParam);
static int CloseNotificationServer(ParameterList_t *TempParam);

static int OpenRemoteServer(ParameterList_t *TempParam);
static int CloseConnection(ParameterList_t *TempParam);

static int OpenRemoteNotificationServer(ParameterList_t *TempParam);
static int CloseNotificationConnection(ParameterList_t *TempParam);

static int Abort(ParameterList_t *TempParam);
static int SetFolder(ParameterList_t *TempParam);
static int GetFolderListing(ParameterList_t *TempParam);
static int GetMessageListing(ParameterList_t *TempParam);
static int GetMessage(ParameterList_t *TempParam);
static int SetMessageStatus(ParameterList_t *TempParam);
static int PushMessage(ParameterList_t *TempParam);
static int UpdateInbox(ParameterList_t *TempParam);
static int SendEventReport(ParameterList_t *TempParam);
static int RequestNotification(ParameterList_t *TempParam);

static void SendEventRequest(EventType_t EventType, MessageType_t MessageType, char *FolderName, char *Handle);

static void ProcessSetFolder(MAP_Set_Folder_Indication_Data_t *MAP_Set_Folder_Indication_Data);
static void ProcessGetFolderListing(MAP_Get_Folder_Listing_Indication_Data_t *MAP_Get_Folder_Listing_Indication_Data);
static void ProcessGetMessageListing(MAP_Get_Message_Listing_Indication_Data_t *MAP_Get_Message_Listing_Indication_Data);
static void ProcessGetMessage(MAP_Get_Message_Indication_Data_t *MAP_Get_Message_Indication_Data);
static void ProcessPushMessage(MAP_Push_Message_Indication_Data_t *MAP_Push_Message_Indication_Data);
static void ProcessUpdateInbox(MAP_Update_Inbox_Indication_Data_t *MAP_Update_Inbox_Indication_Data);
static void ProcessNotificationRegistration(MAP_Notification_Registration_Indication_Data_t *MAP_Notification_Registration_Indication_Data);

static void DisplaySDPAttributeResponse(SDP_Service_Attribute_Response_Data_t *SDPServiceAttributeResponse, unsigned int InitLevel);
static void DisplaySDPSearchAttributeResponse(SDP_Service_Search_Attribute_Response_Data_t *SDPServiceSearchAttributeResponse);
static void DisplayDataElement(SDP_Data_Element_t *SDPDataElement, unsigned int Level);

   /* BTPS Callback function prototypes.                                */
static void BTPSAPI GAP_Event_Callback(unsigned int BluetoothStackID, GAP_Event_Data_t *GAP_Event_Data, unsigned long CallbackParameter);
static void BTPSAPI SDP_Event_Callback(unsigned int BluetoothStackID, unsigned int SDPRequestID, SDP_Response_Data_t *SDP_Response_Data, unsigned long CallbackParameter);
static void BTPSAPI MAP_Event_Callback_Server(unsigned int BluetoothStackID, MAP_Event_Data_t *MAPEventData, unsigned long CallbackParameter);
static void BTPSAPI MAP_Event_Callback_Client(unsigned int BluetoothStackID, MAP_Event_Data_t *MAPEventData, unsigned long CallbackParameter);

   /* This function is responsible for taking the input from the user   */
   /* and dispatching the appropriate Command Function.  First, this    */
   /* function retrieves a String of user input, parses the user input  */
   /* into Command and Parameters, and finally executes the Command or  */
   /* Displays an Error Message if the input is not a valid Command.    */
static void UserInterface_Client(void)
{
   int           Result = !EXIT_CODE;
   char          UserInput[MAX_COMMAND_LENGTH];
   UserCommand_t TempCommand;

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
   AddCommand("OPENNOTIFICATION", OpenNotificationServer);
   AddCommand("CLOSENOTIFICATION", CloseNotificationServer);
   AddCommand("REQUESTNOTIFICATION", RequestNotification);
   AddCommand("SETFOLDER", SetFolder);
   AddCommand("GETFOLDERLISTING", GetFolderListing);
   AddCommand("GETMESSAGELISTING", GetMessageListing);
   AddCommand("GETMESSAGE", GetMessage);
   AddCommand("SETMESSAGESTATUS", SetMessageStatus);
   AddCommand("PUSHMESSAGE", PushMessage);
   AddCommand("UPDATEINBOX", UpdateInbox);
   AddCommand("ABORT", Abort);
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
                     if(MAPID)
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
         if(MAPID)
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
   int           Result = !EXIT_CODE;
   char          UserInput[MAX_COMMAND_LENGTH];
   UserCommand_t TempCommand;

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
   AddCommand("OPENSERVER", OpenServer);
   AddCommand("CLOSESERVER", CloseServer);
   AddCommand("OPENNOTIFICATION", OpenRemoteNotificationServer);
   AddCommand("CLOSENOTIFICATION", CloseNotificationConnection);
   AddCommand("SENDEVENTREPORT", SendEventReport);
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
                     if(MAPID)
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
         if(MAPID)
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

   printf("%lu bytes written to file :%s\r\n", BytesWritten, CurrentFileName);
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

            BluetoothStackID   = Result;

            DebugID            = 0;

            /* Initialize the default Secure Simple Pairing parameters. */
            IOCapability       = DEFAULT_IO_CAPABILITY;
            OOBSupport         = FALSE;
            MITMProtection     = DEFAULT_MITM_PROTECTION;

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
      if(MAPID)
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
         printf("Set Pairability Mode Command Error : %d.\r\n", Result);

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
   /* Command Options for either MAP Client or Server.  The input       */
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
      printf("*                  OpenNotification, CloseNotification,          *\r\n");
      printf("*                  RequestNotification, SetFolder,               *\r\n");
      printf("*                  GetFolderListing, GetMessage,                 *\r\n");
      printf("*                  GetMessageListing, SetMessageStatus,          *\r\n");
      printf("*                  PushMessage, UpdateInbox, Abort,              *\r\n");
      printf("*                  EnableDebug, Help, Quit.                      *\r\n");
      printf("******************************************************************\r\n");
   }
   else
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
      printf("*                  GetRemoteName, OpenServer, CloseServer,       *\r\n");
      printf("*                  OpenNotification, CloseNotification,          *\r\n");
      printf("*                  SendEventReport, EnableDebug, Help, Quit.     *\r\n");
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

   /* The following function is responsible for creating a local MAP    */
   /* Server.  This function returns zero if successful and a negative  */
   /* value if an error occurred.                                       */
static int OpenServer(ParameterList_t *TempParam)
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
         if(!MAPID)
         {
            /* Check to see if an optional Received Message Store Path  */
            /* was specified.                                           */
            if((TempParam->NumberofParameters >= 2) && (TempParam->Params[1].strParam) && (strlen(TempParam->Params[1].strParam)))
               strcpy(ReceiveMessagePath, TempParam->Params[1].strParam);
            else
            {
               /* Received Message Store Path, not specified, go ahead  */
               /* and use the current directory.                        */
               if(!getcwd(ReceiveMessagePath, sizeof(ReceiveMessagePath)))
               {
                  /* There was an error fetching the Directory, so let's*/
                  /* just default it to the Root Directory.             */
                  ReceiveMessagePath[0] = '/';
               }
            }

            /* Make sure the Received Messages Path ends in '/'         */
            /* character.  (this way we can append the received file    */
            /* names directly to the path).                             */
            if(ReceiveMessagePath[strlen(ReceiveMessagePath) - 1] != '/')
               ReceiveMessagePath[strlen(ReceiveMessagePath)] = '/';

            /* The above parameters are valid, attempt to open a Local  */
            /* MAP Port.                                                */
            Result = MAP_Open_Message_Access_Server(BluetoothStackID, TempParam->Params[0].intParam, MAP_Event_Callback_Server, 0);

            /* Now check to see if the function call was successfully   */
            /* made.                                                    */
            if(Result > 0)
            {
               /* Now that a Service Name has been created try and      */
               /* Register the SDP Record.                              */
               if(!MAP_Register_Message_Access_Server_SDP_Record(BluetoothStackID, Result, "MAP Server", 1, MAP_SERVER_SUPPORTED_MESSAGE_TYPE_EMAIL, &ServerSDPRecordHandle))
               {
                  /* The Server was opened successfully and the SDP     */
                  /* Record was successfully added.  The return value of*/
                  /* the call is the MAP ID and is required for future  */
                  /* MAP Profile calls.                                 */
                  MAPID   = Result;

                  ret_val = 0;

                  printf("MAP_Open_Message_Access_Server() Successful (ID = %04X).\r\n", MAPID);
                  printf("  Received Messages Path: %s\r\n", ReceiveMessagePath);
               }
               else
               {
                  /* Error registering SDP Record, go ahead and close   */
                  /* down the server.                                   */
                  MAP_Close_Server(BluetoothStackID, Result);

                  printf("MAP_Register_Message_Access_Server_SDP_Record() Failure.\r\n");

                  ret_val = FUNCTION_ERROR;
               }
            }
            else
            {
               /* There was an error opening the Server.                */
               printf("MAP_Open_Message_Access_Server() Failure: %d.\r\n", Result);

               ret_val = Result;
            }
         }
         else
         {
            /* A Connection is already open, this program only supports */
            /* one Server or Client at a time.                          */
            printf("MAP Server already open.\r\n");

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         printf("Usage: OpenServer [Port Number] [Received Messages Path (optional)].\r\n");

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

   /* The following function is responsible for deleting a local MAP    */
   /* Server that was created via a successful call to the OpenServer() */
   /* function.  This function returns zero if successful and a negative*/
   /* value if an error occurred.                                       */
static int CloseServer(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      if(MAPID)
      {
         /* Free any allocated data buffers for ongoing transactions.   */
         if((CurrentOperation != coNone) && (CurrentBuffer))
         {
            free(CurrentBuffer);

            CurrentBuffer = NULL;
         }

         /* MAP Server opened, close any open SDP Record.               */
         if(ServerSDPRecordHandle)
         {
            MAP_Un_Register_SDP_Record(BluetoothStackID, MAPID, ServerSDPRecordHandle);

            ServerSDPRecordHandle = 0;
         }

         /* Next attempt to close this Server.                          */
         Result = MAP_Close_Server(BluetoothStackID, MAPID);

         if(!Result)
         {
            /* Display a message indicating that the server was         */
            /* successfully closed.                                     */
            printf("MAP_Close_Server() Success.\r\n");

            /* Flag the port has been closed.                           */
            MAPID            = 0;

            ret_val          = 0;

            CurrentOperation = coNone;

            /* If a Notification Connection is present, go ahead and    */
            /* close it as well.                                        */
            if((NotificationConnected) && (MAPNID))
               CloseNotificationConnection(NULL);
         }
         else
         {
            /* An error occurred while attempting to close the server.  */
            printf("MAP_Close_Server() Failure: %d.\r\n", Result);

            ret_val = Result;
         }
      }
      else
      {
         /* No valid MAP Server exists.                                 */
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

   /* The following function is responsible for creating a local MAP    */
   /* Notification Server.  This function returns zero if successful and*/
   /* a negative value if an error occurred.                            */
static int OpenNotificationServer(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the Server Port ID passed to this  */
      /* function is valid.                                             */
      if((TempParam) && (TempParam->NumberofParameters >= 1) && (TempParam->Params[0].intParam))
      {
         /* Now check to make sure that there is already a connection.  */
         if((Connected) && (MAPID))
         {
            /* Now check to make sure that a Server doesn't already     */
            /* exist.                                                   */
            if(!MAPNID)
            {
               /* The above parameters are valid, attempt to open a     */
               /* Local MAP Port.                                       */
               Result = MAP_Open_Message_Notification_Server(BluetoothStackID, TempParam->Params[0].intParam, MAPID, MAP_Event_Callback_Client, 0);

               /* Now check to see if the function call was successfully*/
               /* made.                                                 */
               if(Result > 0)
               {
                  /* Register the SDP Record.                           */
                  if(!MAP_Register_Message_Notification_Server_SDP_Record(BluetoothStackID, Result, "MAP Notification Server", &NotificationSDPRecordHandle))
                  {
                     /* The Server was opened successfully and the SDP  */
                     /* Record was successfully added.  The return value*/
                     /* of the call is the MAP ID and is required for   */
                     /* future MAP Profile calls.                       */
                     MAPNID  = Result;

                     printf("MAP_Open_Message_Notification_Server() Successful (ID = %04X).\r\n", MAPNID);

                     ret_val = 0;
                  }
                  else
                  {
                     /* Error registering SDP Record, go ahead and close*/
                     /* down the server.                                */
                     MAP_Close_Server(BluetoothStackID, Result);

                     printf("MAP_Register_Message_Notification_Server_SDP_Record() Failure.\r\n");

                     ret_val = FUNCTION_ERROR;
                  }
               }
               else
               {
                  /* There was an error opening the Server.             */
                  printf("MAP_Open_Message_Notification_Server() Failure: %d.\r\n", Result);

                  ret_val = FUNCTION_ERROR;
               }
            }
            else
            {
               /* A Server is already registered.                       */
               printf("Server already registered.\r\n");

               ret_val = INVALID_PARAMETERS_ERROR;
            }
         }
         else
         {
            /* Client is NOT connected.                                 */
            printf("MAP Client is not connected.\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         printf("Usage: OpenNotification [Port Number].\r\n");

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

   /* The following function is responsible for deleting a local MAP    */
   /* Notification Server that was created via a successful call to the */
   /* OpenNotificationServer() function.  This function returns zero if */
   /* successful and a negative value if an error occurred.             */
static int CloseNotificationServer(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      if(MAPNID)
      {
         /* MAP Server opened, close any open SDP Record.               */
         if(NotificationSDPRecordHandle)
         {
            MAP_Un_Register_SDP_Record(BluetoothStackID, MAPNID, NotificationSDPRecordHandle);

            ServerSDPRecordHandle = 0;
         }

         /* Next attempt to close this Server.                          */
         Result = MAP_Close_Server(BluetoothStackID, MAPNID);

         if(!Result)
         {
            /* Display a message indicating that the server was         */
            /* successfully closed.                                     */
            printf("MAP_Close_Server() Success.\r\n");

            /* Flag the port has been closed.                           */
            MAPNID                = 0;
            NotificationConnected = FALSE;

            ret_val               = 0;
         }
         else
         {
            /* An error occurred while attempting to close the server.  */
            printf("MAP_Close_Server() Failure: %d.\r\n", Result);

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* No valid MAP Server exists.                                 */
         printf("No Server is currently open.\r\n");

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

   /* The following function is responsible for initiating a connection */
   /* with a Remote MAP Server.  This function returns zero if          */
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
      if(!MAPID)
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
               /* the program is about to open a Remote MAP Port.       */
               printf("Opening Remote MAP Server (BD_ADDR = %s, Port = %04X)\r\n", BoardStr, TempParam->Params[1].intParam);

               /* Attempt to open a connection to the selected remote   */
               /* MAP Access Profile Server.                            */
               Result = MAP_Open_Remote_Message_Access_Server_Port(BluetoothStackID, InquiryResultList[(TempParam->Params[0].intParam-1)], TempParam->Params[1].intParam, MAP_Event_Callback_Client, 0);

               /* Now check to see if the function call was successfully*/
               /* made.                                                 */
               if(Result > 0)
               {
                  /* The Client was opened successfully.  The return    */
                  /* value of the call is the MAP ID and is required for*/
                  /* future MAP Profile calls.                          */
                  MAPID     = Result;

                  Connected = TRUE;

                  ret_val   = 0;

                  printf("MAP_Open_Remote_Message_Access_Server_Port: Function Successful (ID = %04X).\r\n", MAPID);
               }
               else
               {
                  /* There was an error opening the Client.             */
                  printf("MAP_Open_Remote_Message_Access_Server_Port() Failure: %d.\r\n", Result);

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
   /* with a remote MAP Client or Server.  This function returns zero   */
   /* if successful and a negative value if an error occurred.          */
static int CloseConnection(ParameterList_t *TempParam)
{
   int ret_val;
   int Result;

   /* First check to see if a valid Bluetooth Stack exists.             */
   if(BluetoothStackID)
   {
      /* Check to see if the MAPID appears to be semi-valid.  This      */
      /* parameter will only be valid if a Connection is open or a      */
      /* Server is Open.                                                */
      if(MAPID)
      {
         /* Free any allocated data buffers for ongoing transactions.   */
         if((CurrentOperation != coNone) && (CurrentBuffer))
         {
            free(CurrentBuffer);

            CurrentBuffer = NULL;
         }

         /* The Current MAPID appears to be semi-valid.  Now try to     */
         /* close the connection.                                       */
         Result = MAP_Close_Connection(BluetoothStackID, MAPID);

         if(!Result)
         {
            /* The function was called successfully.  Display a message */
            /* indicating that the Connection Close was successfully    */
            /* submitted.                                               */
            printf("MAP_Close_Connection: Function Successful.\r\n");

            Connected        = FALSE;
            CurrentOperation = coNone;

            ret_val          = 0;

            if(IsClient)
               MAPID = 0;
         }
         else
         {
            /* An error occurred while attempting to close the          */
            /* Connection.                                              */
            printf("MAP_Close_Connection() Failure: %d.\r\n", Result);

            ret_val = Result;
         }
      }
      else
      {
         /* The Current MAPID is invalid so no Connection or Server is  */
         /* open.                                                       */
         printf("Invalid MAP ID: MAP Close Connection.\r\n");

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

   /* The following function is responsible for initiating a connection */
   /* with a Remote MAP Notification Server.  This function returns zero*/
   /* if successful and a negative value if an error occurred.          */
static int OpenRemoteNotificationServer(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* First, make sure that a connection already exists (MAP).       */
      if((Connected) && (MAPID))
      {
         /* Now check to make sure that a Client doesn't already exist  */
         /* (Notification).                                             */
         if(!MAPNID)
         {
            /* A Client port is not already open, now check to make sure*/
            /* that the parameters required for this function appear to */
            /* be valid.                                                */
            if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].intParam))
            {
               /* The above parameters are valid, inform the user that  */
               /* the program is about to open a Remote MAP Notification*/
               /* Port.                                                 */
               printf("Opening Remote MAP Notification Server (Port = %04X)\r\n", TempParam->Params[0].intParam);

               /* Attempt to open a connection to the selected remote   */
               /* MAP Access Profile Server.                            */
               Result = MAP_Open_Remote_Message_Notification_Server_Port(BluetoothStackID, MAPID, TempParam->Params[0].intParam, MAP_Event_Callback_Server, 0);

               /* Now check to see if the function call was successfully*/
               /* made.                                                 */
               if(Result > 0)
               {
                  /* The Client was opened successfully.  The return    */
                  /* value of the call is the MAP Notification ID and is*/
                  /* required for future MAP Profile calls.             */
                  MAPNID   = Result;

                  ret_val  = 0;

                  printf("MAP_Open_Remote_Message_Notification_Server_Port: Function Successful (ID = %04X).\r\n", MAPNID);
               }
               else
               {
                  /* There was an error opening the Client.             */
                  printf("MAP_Open_Remote_Message_Notification_Server_Port() Failure: %d.\r\n", Result);

                  ret_val = Result;
               }
            }
            else
            {
               printf("Usage: OpenNotification [Server Port].\r\n");

               ret_val = INVALID_PARAMETERS_ERROR;
            }
         }
         else
         {
            /* Map Notification Client already open.                    */
            printf("Map Notification Client already open.\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* MAP Server connection was not connected.                    */
         printf("MAP Connection does not exist.\r\n");

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
   /* with a remote MAP Notification Client or Server.  This function   */
   /* returns zero if successful and a negative value if an error       */
   /* occurred.                                                         */
static int CloseNotificationConnection(ParameterList_t *TempParam)
{
   int ret_val;
   int Result;

   /* First check to see if a valid Bluetooth Stack exists.             */
   if(BluetoothStackID)
   {
      /* Check to see if the MAP Notification ID appears to be          */
      /* semi-valid.  This parameter will only be valid if a Connection */
      /* is open.                                                       */
      if(MAPNID)
      {
         /* The Current MAP Notification ID appears to be semi-valid.   */
         /* Now try to close the connection.                            */
         Result = MAP_Close_Connection(BluetoothStackID, MAPNID);

         if(!Result)
         {
            /* The function was called successfully.  Display a message */
            /* indicating that the Connection Close was successfully    */
            /* submitted.                                               */
            printf("MAP_Close_Connection: Function Successful.\r\n");

            NotificationConnected  = FALSE;
            MAPNID                 = 0;

            ret_val                = 0;
         }
         else
         {
            /* An error occurred while attempting to close the          */
            /* Connection.                                              */
            printf("MAP_Close_Connection() Failure: %d.\r\n", Result);

            ret_val = Result;
         }
      }
      else
      {
         /* The Current MAP Notification ID is invalid so no Connection */
         /* open.                                                       */
         printf("Invalid MAP Notification ID: MAP Close Connection.\r\n");

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

   /* The following function is responsible for issuing an Abort with a */
   /* remote MAP Server by issuing an OBEX Abort Command.  This         */
   /* function returns zero if successful and a negative value if an    */
   /* error occurred.                                                   */
static int Abort(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the MAP ID appears to be           */
      /* semi-valid.                                                    */
      if(MAPID)
      {
         /* The MAP ID appears to be is a semi-valid value.  Now submit */
         /* the command to begin this operation.                        */
         Result = MAP_Abort_Request(BluetoothStackID, MAPID);

         if(!Result)
         {
            /* The function was submitted successfully.                 */

            /* Update the Current Operation.                            */
            CurrentOperation = coAbort;

            ret_val          = 0;

            printf("MAP_Abort_Request: Function Successful.\r\n");
         }
         else
         {
            /* There was an error submitting the function.              */
            printf("MAP_Abort_Request() Failure: %d.\r\n", Result);

            ret_val = Result;
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("MAP Abort Request: Invalid MAPID.\r\n");

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

   /* The following function is responsible for issuing a MAP Set Folder*/
   /* Request to a remote MAP Server.  This function returns zero if    */
   /* successful and a negative value if an error occurred.             */
static int SetFolder(ParameterList_t *TempParam)
{
   int    Result;
   int    ret_val;
   int    Index;
   Word_t FolderName[256];

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the MAP ID appears to be           */
      /* semi-valid.                                                    */
      if(MAPID)
      {
         /* The MAP ID appears to be is a semi-valid value.  Next, check*/
         /* to see if required parameters were specified.               */
         if((TempParam) && (TempParam->NumberofParameters >= 1) && (TempParam->Params[0].intParam >= (int)sfRoot) && (TempParam->Params[0].intParam <= (int)sfUp))
         {
            /* Check to see if a Folder was specified if the down option*/
            /* was specified.                                           */
            if((TempParam->Params[0].intParam != (int)sfDown) || (((TempParam->Params[0].intParam == (int)sfDown) && (TempParam->NumberofParameters >= 2) && (TempParam->Params[1].strParam) && (strlen(TempParam->Params[1].strParam)))))
            {
               /* Folder Name needs to be specified in UTF-16.  Go ahead*/
               /* and simply convert the ASCII into UTF-16 Encoded      */
               /* ASCII.                                                */
               if(TempParam->Params[0].intParam == (int)sfDown)
               {
                  /* Convert from start until the end into UTF-16.      */
                  Index = 0;
                  while(TempParam->Params[1].strParam[Index])
                  {
                     FolderName[Index] = TempParam->Params[1].strParam[Index];

                     Index++;
                  }

                  /* NULL terminate the String.                         */
                  FolderName[Index] = 0;
               }
               else
                  FolderName [0] = '\0';

               Result = MAP_Set_Folder_Request(BluetoothStackID, MAPID, (MAP_Set_Folder_Option_t)TempParam->Params[0].intParam, FolderName);

               if(!Result)
               {
                  /* The function was submitted successfully.                 */
                  printf("MAP_Set_Folder_Request() Successful.\r\n");

                  /* Update the Current Operation.                            */
                  CurrentOperation = coSetFolder;

                  ret_val          = 0;
               }
               else
               {
                  /* There was an error submitting the function.              */
                  printf("MAP_Set_Folder_Request() Failure: %d.\r\n", Result);

                  ret_val = Result;
               }
            }
            else
            {
               printf("Usage: SetFolder [Path Option (0 = Root, 1 = Down, 2 = Up)] [Folder (only required for Down option)].\r\n");

               ret_val = INVALID_PARAMETERS_ERROR;
            }
         }
         else
         {
            printf("Usage: SetFolder [Path Option (0 = Root, 1 = Down, 2 = Up)] [Folder (only required for Down option)].\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("Set Folder Request: Invalid MAPID.\r\n");

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

   /* The following function is responsible for issuing a MAP Get Folder*/
   /* Listing Request to a remote MAP Server.  This function returns    */
   /* zero if successful and a negative value if an error occurred.     */
static int GetFolderListing(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the MAP ID appears to be           */
      /* semi-valid.                                                    */
      if(MAPID)
      {
         /* The MAP ID appears to be is a semi-valid value.  Next, check*/
         /* to see if required parameters were specified.               */
         if((TempParam) && (TempParam->NumberofParameters >= 2))
         {
            /* Check to see if a filename was specified to save the     */
            /* result to.                                               */
            if((TempParam->NumberofParameters >= 3) && (TempParam->Params[2].strParam) && (strlen(TempParam->Params[2].strParam)))
               strcpy(CurrentFileName, TempParam->Params[2].strParam);
            else
               CurrentFileName[0] = '\0';

            Result = MAP_Get_Folder_Listing_Request(BluetoothStackID, MAPID, (Word_t)TempParam->Params[0].intParam, (Word_t)TempParam->Params[1].intParam);

            if(!Result)
            {
               /* The function was submitted successfully.              */
               printf("MAP_Get_Folder_Listing_Request() Successful.\r\n");

               if((TempParam->Params[0].intParam) && (CurrentFileName[0]))
                  printf("Filename used to store results: %s\r\n", CurrentFileName);

               /* Update the Current Operation.                         */
               CurrentOperation = coGetFolderListing;

               ret_val          = 0;
            }
            else
            {
               /* There was an error submitting the function.           */
               printf("MAP_Get_Folder_Listing_Request() Failure: %d.\r\n", Result);

               ret_val = Result;
            }
         }
         else
         {
            printf("Usage: GetFolderListing [Max List Count] [List Start Offset] [Save Filename (optional)].\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("Get Folder Listing Request: Invalid MAPID.\r\n");

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

   /* The following function is responsible for issuing a MAP Get       */
   /* Message Listing Request to a remote MAP Server.  This function    */
   /* returns zero if successful and a negative value if an error       */
   /* occurred.                                                         */
static int GetMessageListing(ParameterList_t *TempParam)
{
   int    Result;
   int    ret_val;
   char  *Folder;
   int    Index;
   Word_t FolderName[256];

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the MAP ID appears to be           */
      /* semi-valid.                                                    */
      if(MAPID)
      {
         /* The MAP ID appears to be is a semi-valid value.  Next, check*/
         /* to see if required parameters were specified.               */
         if((TempParam) && (TempParam->NumberofParameters >= 2))
         {
            /* Determine the correct Folder (if specified).             */
            if((TempParam->NumberofParameters >= 3) && (TempParam->Params[2].strParam))
            {
               /* Folder specified, check to see if it was only         */
               /* specified so that the save file could be specified.   */
               if((strlen(TempParam->Params[2].strParam) == 1) && (TempParam->Params[2].strParam[0] == '.'))
                  Folder = NULL;
               else
                  Folder = TempParam->Params[2].strParam;
            }
            else
               Folder = NULL;

            /* Check to see if a filename was specified to save the     */
            /* result to.                                               */
            if((TempParam->NumberofParameters >= 4) && (TempParam->Params[3].strParam) && (strlen(TempParam->Params[3].strParam)))
               strcpy(CurrentFileName, TempParam->Params[3].strParam);
            else
               CurrentFileName[0] = '\0';

            /* Folder Name needs to be specified in UTF-16.  Go ahead   */
            /* and simply convert the ASCII into UTF-16 Encoded ASCII.  */
            if(Folder)
            {
               /* Convert from start until the end into UTF-16.         */
               Index = 0;
               while(Folder[Index])
               {
                  FolderName[Index] = Folder[Index];

                  Index++;
               }

               /* NULL terminate the String.                            */
               FolderName[Index] = 0;
            }

            Result = MAP_Get_Message_Listing_Request(BluetoothStackID, MAPID, Folder?FolderName:NULL, (Word_t)TempParam->Params[0].intParam, (Word_t)TempParam->Params[1].intParam, NULL);

            if(!Result)
            {
               /* The function was submitted successfully.              */
               printf("MAP_Get_Message_Listing_Request() Successful.\r\n");

               if((TempParam->Params[0].intParam) && (CurrentFileName[0]))
                  printf("Filename used to store results: %s\r\n", CurrentFileName);

               /* Update the Current Operation.                         */
               CurrentOperation = coGetMessageListing;

               ret_val          = 0;
            }
            else
            {
               /* There was an error submitting the function.           */
               printf("MAP_Get_Message_Listing_Request() Failure: %d.\r\n", Result);

               ret_val = Result;
            }
         }
         else
         {
            printf("Usage: GetMessageListing [Max List Count] [List Start Offset] [Folder Name (optional - specify '.' to specify Save File Name)] [Save Filename (optional)].\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("Get Message Listing Request: Invalid MAPID.\r\n");

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

   /* The following function is responsible for issuing a MAP Get       */
   /* Message Request to a remote MAP Server.  This function returns    */
   /* zero if successful and a negative value if an error occurred.     */
static int GetMessage(ParameterList_t *TempParam)
{
   int           Result;
   int           ret_val;
   MAP_CharSet_t CharSet;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the MAP ID appears to be           */
      /* semi-valid.                                                    */
      if(MAPID)
      {
         /* The MAP ID appears to be is a semi-valid value.  Next, check*/
         /* to see if required parameters were specified.               */
         if((TempParam) && (TempParam->NumberofParameters >= 3) && (TempParam->Params[0].strParam) && (strlen(TempParam->Params[0].strParam)) && ((TempParam->Params[2].intParam == (int)ftUnfragmented) || (TempParam->Params[2].intParam == (int)ftFirst) || (TempParam->Params[2].intParam == (int)ftLast)))
         {
            /* Note the Character Set.                                  */
            if(TempParam->Params[1].intParam)
               CharSet = csUTF8;
            else
               CharSet = csNative;

            /* Check to see if a filename was specified to save the     */
            /* result to.                                               */
            if((TempParam->NumberofParameters >= 4) && (TempParam->Params[3].strParam) && (strlen(TempParam->Params[3].strParam)))
               strcpy(CurrentFileName, TempParam->Params[3].strParam);
            else
               CurrentFileName[0] = '\0';

            Result = MAP_Get_Message_Request(BluetoothStackID, MAPID, TempParam->Params[0].strParam, FALSE, CharSet, (MAP_Fractional_Type_t)TempParam->Params[1].intParam);

            if(!Result)
            {
               /* The function was submitted successfully.              */
               printf("MAP_Get_Message_Request() Successful.\r\n");

               if(CurrentFileName[0])
                  printf("Filename used to store results: %s\r\n", CurrentFileName);

               /* Update the Current Operation.                         */
               CurrentOperation = coGetMessage;

               ret_val          = 0;
            }
            else
            {
               /* There was an error submitting the function.           */
               printf("MAP_Get_Message_Request() Failure: %d.\r\n", Result);

               ret_val = Result;
            }
         }
         else
         {
            printf("Usage: GetMessage [Message Handle] [Char Set (0 = Native, 1 = UTF8)] [Fractional Type (0 = Unfragmented, 1 = First, 4 = Last)] [Save Filename (optional)].\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("Get Message Request: Invalid MAPID.\r\n");

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

   /* The following function is responsible for issuing a MAP Set       */
   /* Message Status Request to the remote MAP Server.  This function   */
   /* returns zero if successful and a negative value if an error       */
   /* occurred.                                                         */
static int SetMessageStatus(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the MAP ID appears to be           */
      /* semi-valid.                                                    */
      if(MAPID)
      {
         /* The MAP ID appears to be is a semi-valid value.  Next, check*/
         /* to see if required parameters were specified.               */
         if((TempParam) && (TempParam->NumberofParameters >= 3) && (TempParam->Params[0].strParam) && (strlen(TempParam->Params[0].strParam)) && (TempParam->Params[1].intParam >= (int)siReadStatus) && (TempParam->Params[1].intParam <= (int)siDeletedStatus))
         {
            Result = MAP_Set_Message_Status_Request(BluetoothStackID, MAPID, TempParam->Params[0].strParam, (MAP_Status_Indicator_t)TempParam->Params[1].intParam, TempParam->Params[2].intParam);

            if(!Result)
            {
               /* The function was submitted successfully.              */
               printf("MAP_Set_Message_Status_Request() Successful.\r\n");

               /* Update the Current Operation.                         */
               CurrentOperation = coSetMessageStatus;

               ret_val          = 0;
            }
            else
            {
               /* There was an error submitting the function.           */
               printf("MAP_Set_Message_Status_Request() Failure: %d.\r\n", Result);

               ret_val = Result;
            }
         }
         else
         {
            printf("Usage: SetMessageStatus [Message Handle] [Status Indicator (0 = Read Status, 1 = Delete Status)] [Status Value].\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("Set Message Status: Invalid MAPID.\r\n");

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

   /* The following function is responsible for issuing a MAP Push      */
   /* Message Request to a remote MAP Server.  This function returns    */
   /* zero if successful and a negative value if an error occurred.     */
static int PushMessage(ParameterList_t *TempParam)
{
   int           Result;
   int           ret_val;
   char         *Folder;
   int           Index;
   Word_t        FolderName[256];
   unsigned int  ParamIndex;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the MAP ID appears to be           */
      /* semi-valid.                                                    */
      if(MAPID)
      {
         /* The MAP ID appears to be is a semi-valid value.  Next, check*/
         /* to see if required parameters were specified.               */
         if((TempParam) && (TempParam->NumberofParameters >= 3))
         {
            /* Determine the correct Folder (if specified).             */
            if((TempParam->NumberofParameters >= 4) && (TempParam->Params[2].strParam))
            {
               /* Set the parameter index of the File Name to 3 (past   */
               /* the folder name).                                     */
               ParamIndex = 3;

               /* Folder specified, check to see if it was only         */
               /* specified so that the save file could be specified.   */
               if((strlen(TempParam->Params[2].strParam) == 1) && (TempParam->Params[2].strParam[0] == '.'))
                  Folder = NULL;
               else
                  Folder = TempParam->Params[2].strParam;
            }
            else
            {
               ParamIndex = 2;
               Folder     = NULL;
            }

            /* Note the Filename that was specified.                    */
            if((TempParam->NumberofParameters >= 3) && (TempParam->Params[ParamIndex].strParam) && (strlen(TempParam->Params[ParamIndex].strParam)) && (!ReadFileIntoCurrentBuffer(TempParam->Params[ParamIndex].strParam)))
            {
               strcpy(CurrentFileName, TempParam->Params[ParamIndex].strParam);

               /* Folder Name needs to be specified in UTF-16.  Go ahead*/
               /* and simply convert the ASCII into UTF-16 Encoded      */
               /* ASCII.                                                */
               if(Folder)
               {
                  /* Convert from start until the end into UTF-16.      */
                  Index = 0;
                  while(Folder[Index])
                  {
                     FolderName[Index] = Folder[Index];

                     Index++;
                  }

                  /* NULL terminate the String.                         */
                  FolderName[Index] = 0;
               }

               Result = MAP_Push_Message_Request(BluetoothStackID, MAPID, Folder?FolderName:NULL, TempParam->Params[0].intParam, TempParam->Params[1].intParam, csUTF8, CurrentBufferSize, (Byte_t *)CurrentBuffer, &CurrentBufferSent, TRUE);

               if(!Result)
               {
                  /* The function was submitted successfully.           */
                  printf("MAP_Push_Message_Request() Successful.\r\n");

                  printf("Message Filename: %s\r\n", CurrentFileName);

                  /* Update the Current Operation.                      */
                  CurrentOperation = coPushMessage;

                  ret_val          = 0;
               }
               else
               {
                  /* There was an error submitting the function.        */
                  printf("MAP_Push_Message_Request() Failure: %d.\r\n", Result);

                  /* Free any data that was allocated.                  */
                  if((CurrentBuffer) && (CurrentBufferSize))
                  {
                     free(CurrentBuffer);

                     CurrentBuffer     = NULL;
                     CurrentBufferSize = 0;
                  }

                  ret_val = Result;
               }
            }
            else
            {
               printf("Usage: PushMessage [Transparent (0 = False, 1 = True)] [Retry (0 = False, 1 = True)] [Message Folder (optional)] [Message File Name].\r\n");

               ret_val = INVALID_PARAMETERS_ERROR;
            }
         }
         else
         {
            printf("Usage: PushMessage [Transparent (0 = False, 1 = True)] [Retry (0 = False, 1 = True)] [Message Folder (optional)] [Message File Name].\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("Get Message Request: Invalid MAPID.\r\n");

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

   /* The following function is responsible for issuing a MAP           */
   /* UpdateInbox Request to the remote MAP Server.  This function      */
   /* returns zero if successful and a negative value if an error       */
   /* occurred.                                                         */
static int UpdateInbox(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the MAP ID appears to be           */
      /* semi-valid.                                                    */
      if(MAPID)
      {
         /* The MAP ID appears to be is a semi-valid value.  Now submit */
         /* the command to begin this operation.                        */
         Result = MAP_Update_Inbox_Request(BluetoothStackID, MAPID);

         if(!Result)
         {
            /* The function was submitted successfully.                 */

            /* Update the Current Operation.                            */
            CurrentOperation = coUpdateInbox;

            ret_val          = 0;

            printf("MAP_Update_Inbox_Request: Function Successful.\r\n");
         }
         else
         {
            /* There was an error submitting the function.              */
            printf("MAP_Update_Inbox_Request() Failure: %d.\r\n", Result);

            ret_val = Result;
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("MAP Update Inbox Request: Invalid MAPID.\r\n");

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

   /* The following function is responsible for issuing a MAP Send      */
   /* Status Event Request to the remote MAP Server.  This function     */
   /* returns zero if successful and a negative value if an error       */
   /* occurred.                                                         */
static int SendEventReport(ParameterList_t *TempParam)
{
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the MAP ID appears to be           */
      /* semi-valid.                                                    */
      if((MAPID) && (MAPNID))
      {
         /* The MAP ID and MAP Notification ID appears to be is a       */
         /* semi-valid value.  Next, check to see if required parameters*/
         /* were specified.                                             */
         if((TempParam->NumberofParameters >= 3) && (TempParam->Params[0].strParam) && (TempParam->Params[1].strParam))
         {
            /* Check to make sure the Event Type and Message Types are  */
            /* valid.                                                   */
            if((TempParam->Params[2].intParam >= (unsigned int)etNewMessage) && (TempParam->Params[2].intParam <= (unsigned int)etMessageDeleted))
            {
               if((TempParam->Params[3].intParam >= (unsigned int)mtEMAIL) && (TempParam->Params[3].intParam <= (unsigned int)mtMMS))
               {
                  /* Parameters appear to be semi-valid, submit the     */
                  /* request.                                           */
                  SendEventRequest((EventType_t)(TempParam->Params[2].intParam), (MessageType_t)(TempParam->Params[3].intParam), TempParam->Params[1].strParam, TempParam->Params[0].strParam);

                  ret_val = 0;
               }
               else
               {
                  printf("Usage: SendEventReport [Handle] [Folder] [EventType (0 - 7)] [Message Type (0-3)].\r\n");

                  ret_val = INVALID_PARAMETERS_ERROR;
               }
            }
            else
            {
               printf("Usage: SendEventReport [Handle] [Folder] [EventType (0 - 7)] [Message Type (0-3)].\r\n");

               ret_val = INVALID_PARAMETERS_ERROR;
            }
         }
         else
         {
            printf("Usage: SendEventReport [Handle] [Folder] [EventType (0 - 7)] [Message Type (0-3)].\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("Get Message Request: Invalid MAPID.\r\n");

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

   /* The following function is responsible for issuing a MAP Request   */
   /* Notification Request to the remote MAP Server.  This function     */
   /* returns zero if successful and a negative value if an error       */
   /* occurred.                                                         */
static int RequestNotification(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the MAP Notification ID appears to */
      /* be semi-valid.                                                 */
      if(MAPNID)
      {
         /* The MAP ID appears to be is a semi-valid value.  Next,      */
         /* determine if this is a request for a notification or a      */
         /* request to disconnect the notification.                     */
         if((TempParam) && (TempParam->NumberofParameters > 0))
         {
            /* Now submit the command to begin this operation.          */
            Result = MAP_Set_Notification_Registration_Request(BluetoothStackID, MAPID, (Boolean_t)((TempParam->Params[0].intParam)?TRUE:FALSE));

            if(!Result)
            {
               /* The function was submitted successfully.              */
               printf("MAP_Set_Notification_Registration_Request: Function Successful.\r\n");

               ret_val = 0;
            }
            else
            {
               /* There was an error submitting the function.           */
               printf("MAP_Set_Notification_Registration_Request() Failure: %d.\r\n", Result);

               ret_val = Result;
            }
         }
         else
         {
            printf("Usage: RequestNotification [Connection (0 = Disconnect, 1 = Connect)].\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         printf("No Notification Server Open.\r\n");

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
   /* actually send a MAP Send Event Request to the remote MAP          */
   /* Notification Server.                                              */
static void SendEventRequest(EventType_t EventType, MessageType_t MessageType, char *FolderName, char *Handle)
{
   int          Result;
   int          Index;
   char         temp[256];
   unsigned int AmountSent;

   if((BluetoothStackID) && (MAPNID) && (NotificationConnected))
   {
      /* Place the Event Report Header in the fixed buffer.  There are  */
      /* no Event Reports that can not fit in the Minimum OBEX packet.  */
      Index = sprintf(temp, "%s%s%s", TELECOM_MESSAGE_EVENT_REPORT_PREFIX, TELECOM_MESSAGE_EVENT_REPORT_ENTRY_PREFIX, GetEventTypeString(EventType));

      if(EventType == etNewMessage)
      {
         Index += sprintf((char *)&temp[Index], "%s%s", TELECOM_MESSAGE_EVENT_REPORT_ENTRY_MIDDLE_1, Handle);
         Index += sprintf((char *)&temp[Index], "%stelecom/msg/%s", TELECOM_MESSAGE_EVENT_REPORT_ENTRY_MIDDLE_2, FolderName);
         Index += sprintf((char *)&temp[Index], "%s%s", TELECOM_MESSAGE_EVENT_REPORT_ENTRY_MIDDLE_3, GetMessageTypeString(MessageType));
      }
      else
      {
         if((EventType != etMemoryFull) && (EventType != etMemoryAvailable))
         {
            Index += sprintf((char *)&temp[Index], "%s%s", TELECOM_MESSAGE_EVENT_REPORT_ENTRY_MIDDLE_1, Handle);
         }
      }

      Index += sprintf((char *)&temp[Index], "%s%s", TELECOM_MESSAGE_EVENT_REPORT_ENTRY_SUFFIX, TELECOM_MESSAGE_EVENT_REPORT_SUFFIX);

      Result = MAP_Send_Event_Request(BluetoothStackID, MAPNID, Index, (Byte_t *)temp, &AmountSent, TRUE);

      if(Result >= 0)
         printf("MAP_Send_Event_Request() Successful.\r\n");
      else
         printf("MAP_Send_Event_Request() Failure %d.\r\n", Result);
   }
   else
      printf("Invalid parameter or No longer connected.\r\n");
}

   /* The following function is used to Process the MAP Set Folder      */
   /* Indication Event.                                                 */
static void ProcessSetFolder(MAP_Set_Folder_Indication_Data_t *MAP_Set_Folder_Indication_Data)
{
   int           Result;
   char         *NameString;
   char          FolderName[256];
   Byte_t        ResponseCode;
   unsigned int  NameLength;

   if((BluetoothStackID) && (MAPID) && (Connected) && (MAP_Set_Folder_Indication_Data))
   {
      /* All that is left to do is simply attempt to change the Folder  */
      /* path.                                                          */

      /* First we need to map the UTF-16 Folder name to ASCII (this     */
      /* application will only support ASCII folder names).             */
      if(MAP_Set_Folder_Indication_Data->FolderName)
      {
         NameLength = 0;
         while((MAP_Set_Folder_Indication_Data->FolderName[NameLength]) && (NameLength < (sizeof(FolderName) - 1)))
         {
            FolderName[NameLength] = (char)MAP_Set_Folder_Indication_Data->FolderName[NameLength];

            NameLength++;
         }

         /* Make sure the name is NULL terminated.                      */
         FolderName[NameLength] = '\0';
      }

      if(!ChangeMessageFolder(MAP_Set_Folder_Indication_Data->PathOption, FolderName))
      {
         printf("Folder successfully set to: %s\r\n", ((NameString = GetCurrentMessageFolderString()) != NULL)?NameString:"");

         ResponseCode = MAP_OBEX_RESPONSE_OK;
      }
      else
      {
         printf("Unable to set Folder: %d, \"%s\"\r\n", MAP_Set_Folder_Indication_Data->PathOption, MAP_Set_Folder_Indication_Data->FolderName?FolderName:"");

         ResponseCode = MAP_OBEX_RESPONSE_NOT_FOUND;
      }

      Result = MAP_Set_Folder_Response(BluetoothStackID, MAPID, ResponseCode);

      if(Result >= 0)
         printf("MAP_Set_Folder_Response() Successful.\r\n");
      else
         printf("MAP_Set_Folder_Response() Failure %d.\r\n", Result);
   }
   else
      printf("Invalid parameter or No longer connected.\r\n");
}

   /* The following function is used to Process the MAP Get Folder      */
   /* Listing Indication Event.                                         */
static void ProcessGetFolderListing(MAP_Get_Folder_Listing_Indication_Data_t *MAP_Get_Folder_Listing_Indication_Data)
{
   int           Result;
   int           NumberFolders;
   Word_t        NumberEntries;
   unsigned int  Temp;
   unsigned int  Index;
   unsigned int  TotalNumberFolders;
   FolderEntry_t FolderEntry;

   if((BluetoothStackID) && (MAPID) && (Connected) && (MAP_Get_Folder_Listing_Indication_Data))
   {
      /* Determine if this is the start of new operation or a           */
      /* continuation.                                                  */
      if(CurrentOperation == coNone)
      {
         /* Start of a new operation.                                   */

         /* Determine the number of Folders in the current folder.      */
         if((NumberFolders = QueryNumberFolderEntries(GetCurrentMessageFolder())) >= 0)
         {
            /* Determine the total size of the folder listing.          */
            /* * NOTE * We will honor the List Start Offset and the Max */
            /*          List Count Parameters here.                     */
            for(Index=0,CurrentBufferSize=(BTPS_StringLength(TELECOM_FOLDER_LISTING_PREFIX) + BTPS_StringLength(TELECOM_FOLDER_LISTING_SUFFIX)),TotalNumberFolders=0;Index<(unsigned int)NumberFolders;Index++)
            {
               if((Index >= (unsigned int)MAP_Get_Folder_Listing_Indication_Data->ListStartOffset) && (TotalNumberFolders < (unsigned int)MAP_Get_Folder_Listing_Indication_Data->MaxListCount))
               {
                  if(QueryFolderEntry(GetCurrentMessageFolder(), Index, &FolderEntry))
                  {
                     /* Now add up the necessary length for this entry. */
                     if(FolderEntry.FolderName)
                     {
                        CurrentBufferSize += (BTPS_StringLength(TELECOM_FOLDER_LISTING_ENTRY_PREFIX) + BTPS_StringLength(TELECOM_FOLDER_LISTING_ENTRY_MIDDLE) + BTPS_StringLength(TELECOM_FOLDER_LISTING_ENTRY_SUFFIX));

                        CurrentBufferSize += BTPS_StringLength(FolderEntry.FolderName);

                        if(FolderEntry.CreateDateTime)
                           CurrentBufferSize += BTPS_StringLength(FolderEntry.CreateDateTime);

                        TotalNumberFolders++;
                     }
                  }
               }
            }

            /* Now that we know the entire length, go ahead and allocate*/
            /* space to hold the Folder Listing.                        */

            /* Check to see if this is a request to determine the       */
            /* maximum number of entries.                               */
            if(!MAP_Get_Folder_Listing_Indication_Data->MaxListCount)
            {
               printf("Request for Number of Folders: %d\r\n", NumberFolders);

               NumberEntries = (Word_t)NumberFolders;

               Result = MAP_Get_Folder_Listing_Response(BluetoothStackID, MAPID, MAP_OBEX_RESPONSE_OK, &NumberEntries, 0, NULL, NULL);
            }
            else
            {
               /* Allocate a buffer to hold the requested Folder Listing*/
               /* into.                                                 */
               /* * NOTE * We will allocate an extra byte to take care  */
               /*          of the NULL terminator.                      */
               if((CurrentBuffer = malloc(CurrentBufferSize + 1)) != NULL)
               {
                  /* Buffer allocated, go ahead and build the Folder    */
                  /* Listing buffer.  Place the Listing Header on the   */
                  /* data (required).                                   */
                  sprintf((char *)CurrentBuffer, TELECOM_FOLDER_LISTING_PREFIX);

                  for(Index=0,TotalNumberFolders=0;Index<(unsigned int)NumberFolders;Index++)
                  {
                     if((Index >= (unsigned int)MAP_Get_Folder_Listing_Indication_Data->ListStartOffset) && (TotalNumberFolders < (unsigned int)MAP_Get_Folder_Listing_Indication_Data->MaxListCount))
                     {
                        if(QueryFolderEntry(GetCurrentMessageFolder(), Index, &FolderEntry))
                        {
                           if(FolderEntry.FolderName)
                           {
                              BTPS_StringCopy(&(CurrentBuffer[BTPS_StringLength(CurrentBuffer)]), TELECOM_FOLDER_LISTING_ENTRY_PREFIX);

                              BTPS_StringCopy(&(CurrentBuffer[BTPS_StringLength(CurrentBuffer)]), FolderEntry.FolderName);

                              BTPS_StringCopy(&(CurrentBuffer[BTPS_StringLength(CurrentBuffer)]), TELECOM_FOLDER_LISTING_ENTRY_MIDDLE);

                              if(FolderEntry.CreateDateTime)
                                 BTPS_StringCopy(&(CurrentBuffer[BTPS_StringLength(CurrentBuffer)]), FolderEntry.CreateDateTime);

                              BTPS_StringCopy(&(CurrentBuffer[BTPS_StringLength(CurrentBuffer)]), TELECOM_FOLDER_LISTING_ENTRY_SUFFIX);

                              TotalNumberFolders++;
                           }
                        }
                     }
                  }

                  /* Place the Listing Footer on the data (required).   */
                  BTPS_StringCopy(&(CurrentBuffer[BTPS_StringLength(CurrentBuffer)]), TELECOM_FOLDER_LISTING_SUFFIX);

                  CurrentBufferSize = BTPS_StringLength(CurrentBuffer);

                  /* All finished, go ahead and send the Folder Listing.*/
                  Result = MAP_Get_Folder_Listing_Response(BluetoothStackID, MAPID, MAP_OBEX_RESPONSE_OK, NULL, CurrentBufferSize, (Byte_t *)CurrentBuffer, &CurrentBufferSent);

                  /* Flag whether or not the operation completed        */
                  /* successfully.                                      */
                  if((Result >= 0) && (CurrentBufferSent != CurrentBufferSize))
                     CurrentOperation = coGetFolderListing;
                  else
                  {
                     free(CurrentBuffer);

                     CurrentBuffer = NULL;
                  }
               }
               else
               {
                  printf("Unable to allocate memory.\r\n");

                  Result = MAP_Get_Folder_Listing_Response(BluetoothStackID, MAPID, MAP_OBEX_RESPONSE_SERVICE_UNAVAILABLE, NULL, 0, NULL, NULL);
               }
            }
         }
         else
         {
            printf("Unable to Get Folder Listing: Invalid current directory.\r\n");

            Result = MAP_Get_Folder_Listing_Response(BluetoothStackID, MAPID, MAP_OBEX_RESPONSE_NOT_FOUND, NULL, 0, NULL, NULL);
         }
      }
      else
      {
         printf("Continuation for MAP Get Folder Listing\r\n");

         Result = MAP_Get_Folder_Listing_Response(BluetoothStackID, MAPID, MAP_OBEX_RESPONSE_OK, NULL, (CurrentBufferSize - CurrentBufferSent), (Byte_t *)&(CurrentBuffer[CurrentBufferSent]), &Temp);

         /* Flag whether or not the operation completed successfully.   */
         if((Result >= 0) && (Temp != (CurrentBufferSize - CurrentBufferSent)))
            CurrentBufferSent += Temp;
         else
         {
            free(CurrentBuffer);

            CurrentBuffer    = NULL;

            CurrentOperation = coNone;
         }
      }

      if(Result >= 0)
         printf("MAP_Get_Folder_Listing_Response() Successful.\r\n");
      else
         printf("MAP_Get_Folder_Listing_Response() Failure %d.\r\n", Result);
   }
   else
      printf("Invalid parameter or No longer connected.\r\n");
}

   /* The following function is used to Process the MAP Get Message     */
   /* Listing Indication Event.                                         */
static void ProcessGetMessageListing(MAP_Get_Message_Listing_Indication_Data_t *MAP_Get_Message_Listing_Indication_Data)
{
   int              Result;
   int              NumberMessages;
   char             temp[256];
   Word_t           NumberEntries;
   char             FolderName[256];
   Boolean_t        ChangeFolder;
   struct tm       *Time;
   unsigned int     NameLength;
   unsigned int     Temp;
   unsigned int     Index;
   unsigned int     TotalNumberMessages;
   struct timeval   TimeVal;
   MessageEntry_t  *MessageEntry;
   MAP_TimeDate_t   CurrentTime;
   CurrentFolder_t  Folder;

   if((BluetoothStackID) && (MAPID) && (Connected) && (MAP_Get_Message_Listing_Indication_Data))
   {
      /* Determine if this is the start of new operation or a           */
      /* continuation.                                                  */
      if(CurrentOperation == coNone)
      {
         /* Start of a new operation.                                   */

         /* Initialize that no error has occurred.                      */
         Temp   = 0;
         Result = 0;

         /* Check to see if the caller is specifying a sub-folder or the*/
         /* current folder.                                             */
         if((MAP_Get_Message_Listing_Indication_Data->FolderName) && (MAP_Get_Message_Listing_Indication_Data->FolderName[0]))
         {
            /* First we need to map the UTF-16 Folder name to ASCII     */
            /* (this application will only support ASCII folder names). */
            NameLength = 0;
            while((MAP_Get_Message_Listing_Indication_Data->FolderName[NameLength]) && (NameLength < (sizeof(FolderName) - 1)))
            {
               FolderName[NameLength] = (char)MAP_Get_Message_Listing_Indication_Data->FolderName[NameLength];

               NameLength++;
            }

            /* Make sure the name is NULL terminated.                   */
            FolderName[NameLength] = '\0';

            printf("%s\r\n", FolderName);

            /* Sub-folder, so navigate into it.                         */
            if(!ChangeMessageFolder(sfDown, FolderName))
               ChangeFolder = TRUE;
            else
            {
               /* Invalid directory specified, inform remote side of an */
               /* error.                                                */
               printf("Unable to Get Message Listing: Invalid folder specified.\r\n");

               Result = MAP_Get_Message_Listing_Response(BluetoothStackID, MAPID, MAP_OBEX_RESPONSE_NOT_FOUND, NULL, FALSE, NULL, 0, NULL, NULL);

               /* Flag an error.                                        */
               Temp         = 1;
               ChangeFolder = FALSE;
            }
         }
         else
            ChangeFolder = FALSE;

         /* Only continue to process the request if there was not an    */
         /* error.                                                      */
         if(!Temp)
         {
            /* Determine the number of Messages in the current folder.  */
            Folder = GetCurrentMessageFolder();
            if((NumberMessages = QueryNumberMessageEntries(GetCurrentMessageFolder())) >= 0)
            {
               /* Determine the total size of the message listing.      */
               /* * NOTE * We will honor the List Start Offset and the  */
               /*          Max List Count Parameters here.              */
               /* * NOTE * We do NOT SUPPORT any type of filtering in   */
               /*          this implementation !!!!!!!!!!!!!!!!!!!!!!!! */
               for(Index=0,CurrentBufferSize=(BTPS_StringLength(TELECOM_MESSAGE_LISTING_PREFIX) + BTPS_StringLength(TELECOM_MESSAGE_LISTING_SUFFIX)),TotalNumberMessages=0;Index<(unsigned int)NumberMessages;Index++)
               {
                  if((Index >= (unsigned int)MAP_Get_Message_Listing_Indication_Data->ListStartOffset) && (TotalNumberMessages < (unsigned int)MAP_Get_Message_Listing_Indication_Data->MaxListCount))
                  {
                     if(QueryMessageEntryByIndex(Folder, Index, &MessageEntry))
                     {
                        /* Now add up the necessary length for this     */
                        /* entry.                                       */
                        if((MessageEntry) && (MessageEntry->MessageHandle))
                        {
                           CurrentBufferSize += (BTPS_StringLength(TELECOM_MESSAGE_LISTING_ENTRY_PREFIX) + BTPS_StringLength(TELECOM_MESSAGE_LISTING_ENTRY_SUFFIX));

                           CurrentBufferSize += BTPS_StringLength(MessageEntry->MessageHandle);

                           CurrentBufferSize += BTPS_StringLength(TELECOM_MESSAGE_LISTING_ENTRY_MIDDLE_1);

                           if(MessageEntry->Subject)
                              CurrentBufferSize += BTPS_StringLength(MessageEntry->Subject);

                           CurrentBufferSize += BTPS_StringLength(TELECOM_MESSAGE_LISTING_ENTRY_MIDDLE_2);

                           if(MessageEntry->DateTime)
                              CurrentBufferSize += BTPS_StringLength(MessageEntry->DateTime);

                           CurrentBufferSize += BTPS_StringLength(TELECOM_MESSAGE_LISTING_ENTRY_MIDDLE_3);

                           if(MessageEntry->Sender)
                              CurrentBufferSize += BTPS_StringLength(MessageEntry->Sender);

                           CurrentBufferSize += BTPS_StringLength(TELECOM_MESSAGE_LISTING_ENTRY_MIDDLE_4);

                           if(MessageEntry->SenderName)
                              CurrentBufferSize += BTPS_StringLength(MessageEntry->SenderName);

                           CurrentBufferSize += BTPS_StringLength(TELECOM_MESSAGE_LISTING_ENTRY_MIDDLE_5);

                           if(MessageEntry->SenderAddressing)
                              CurrentBufferSize += BTPS_StringLength(MessageEntry->SenderAddressing);

                           CurrentBufferSize += BTPS_StringLength(TELECOM_MESSAGE_LISTING_ENTRY_MIDDLE_6);

                           if(MessageEntry->ReplyToAddressing)
                              CurrentBufferSize += BTPS_StringLength(MessageEntry->ReplyToAddressing);

                           CurrentBufferSize += BTPS_StringLength(TELECOM_MESSAGE_LISTING_ENTRY_MIDDLE_7);

                           if(MessageEntry->RecipientName)
                              CurrentBufferSize += BTPS_StringLength(MessageEntry->RecipientName);

                           CurrentBufferSize += BTPS_StringLength(TELECOM_MESSAGE_LISTING_ENTRY_MIDDLE_8);

                           if(MessageEntry->RecipientAddressing)
                              CurrentBufferSize += BTPS_StringLength(MessageEntry->RecipientAddressing);

                           CurrentBufferSize += BTPS_StringLength(TELECOM_MESSAGE_LISTING_ENTRY_MIDDLE_9);

                           if(MessageEntry->Type)
                              CurrentBufferSize += BTPS_StringLength(MessageEntry->Type);

                           CurrentBufferSize += BTPS_StringLength(TELECOM_MESSAGE_LISTING_ENTRY_MIDDLE_10);

                           sprintf(temp, "%lu", MessageEntry->MessageSize);
                           CurrentBufferSize += BTPS_StringLength(temp);

                           CurrentBufferSize += BTPS_StringLength(TELECOM_MESSAGE_LISTING_ENTRY_MIDDLE_11);

                           if(MessageEntry->Text)
                              CurrentBufferSize += BTPS_StringLength(MessageEntry->Text);

                           CurrentBufferSize += BTPS_StringLength(TELECOM_MESSAGE_LISTING_ENTRY_MIDDLE_12);

                           if(MessageEntry->ReceptionStatus)
                              CurrentBufferSize += BTPS_StringLength(MessageEntry->ReceptionStatus);

                           CurrentBufferSize += BTPS_StringLength(TELECOM_MESSAGE_LISTING_ENTRY_MIDDLE_13);

                           sprintf(temp, "%lu", MessageEntry->AttachmentSize);
                           CurrentBufferSize += BTPS_StringLength(temp);

                           CurrentBufferSize += BTPS_StringLength(TELECOM_MESSAGE_LISTING_ENTRY_MIDDLE_14);

                           if(MessageEntry->Priority < svNotDefined)
                              CurrentBufferSize += BTPS_StringLength("yes");

                           CurrentBufferSize += BTPS_StringLength(TELECOM_MESSAGE_LISTING_ENTRY_MIDDLE_15);

                           if(MessageEntry->Read < svNotDefined)
                              CurrentBufferSize += BTPS_StringLength("yes");

                           CurrentBufferSize += BTPS_StringLength(TELECOM_MESSAGE_LISTING_ENTRY_MIDDLE_16);

                           if(MessageEntry->Sent < svNotDefined)
                              CurrentBufferSize += BTPS_StringLength("yes");

                           CurrentBufferSize += BTPS_StringLength(TELECOM_MESSAGE_LISTING_ENTRY_MIDDLE_17);

                           if(MessageEntry->Protected < svNotDefined)
                              CurrentBufferSize += BTPS_StringLength("yes");

                           TotalNumberMessages++;
                        }
                     }
                  }
               }

               /* Format the Current Local Time in the Time/Date        */
               /* structure.                                            */
               gettimeofday(&TimeVal, NULL);

               if((Time = localtime(&(TimeVal.tv_sec))) != NULL)
               {
                  CurrentTime.Day        = Time->tm_mday;
                  CurrentTime.Month      = Time->tm_mon + 1;
                  CurrentTime.Year       = Time->tm_year + 1900;
                  CurrentTime.Hour       = Time->tm_hour;
                  CurrentTime.Minute     = Time->tm_min;
                  CurrentTime.Second     = Time->tm_sec;
                  CurrentTime.UTC_Time   = FALSE;
                  CurrentTime.UTC_Offset = 0;
               }

               /* Check to see if this is a request to determine the    */
               /* maximum number of entries.                            */
               if(!MAP_Get_Message_Listing_Indication_Data->MaxListCount)
               {
                  printf("Request for Number of Messages: %d\r\n", NumberMessages);

                  NumberEntries = (Word_t)NumberMessages;

                  Result        = MAP_Get_Message_Listing_Response(BluetoothStackID, MAPID, MAP_OBEX_RESPONSE_OK, &NumberEntries, FALSE, &CurrentTime, 0, NULL, NULL);
               }
               else
               {
                  /* Allocate a buffer to hold the requested Message    */
                  /* Listing into.                                      */
                  /* * NOTE * We will allocate an extra byte to take    */
                  /*          care of the NULL terminator.              */
                  if((CurrentBuffer = malloc(CurrentBufferSize + 1)) != NULL)
                  {
                     /* Buffer allocated, go ahead and build the Message*/
                     /* Listing buffer.  Place the Listing Header on the*/
                     /* data (required).                                */
                     sprintf((char *)CurrentBuffer, TELECOM_MESSAGE_LISTING_PREFIX);

                     for(Index=0,TotalNumberMessages=0;Index<(unsigned int)NumberMessages;Index++)
                     {
                        if((Index >= (unsigned int)MAP_Get_Message_Listing_Indication_Data->ListStartOffset) && (TotalNumberMessages < (unsigned int)MAP_Get_Message_Listing_Indication_Data->MaxListCount))
                        {
                           if(QueryMessageEntryByIndex(Folder, Index, &MessageEntry))
                           {
                              if((MessageEntry) && (MessageEntry->MessageHandle))
                              {
                                 BTPS_StringCopy(&(CurrentBuffer[BTPS_StringLength(CurrentBuffer)]), TELECOM_MESSAGE_LISTING_ENTRY_PREFIX);

                                 BTPS_StringCopy(&(CurrentBuffer[BTPS_StringLength(CurrentBuffer)]), MessageEntry->MessageHandle);

                                 BTPS_StringCopy(&(CurrentBuffer[BTPS_StringLength(CurrentBuffer)]), TELECOM_MESSAGE_LISTING_ENTRY_MIDDLE_1);

                                 if(MessageEntry->Subject)
                                    BTPS_StringCopy(&(CurrentBuffer[BTPS_StringLength(CurrentBuffer)]), MessageEntry->Subject);

                                 BTPS_StringCopy(&(CurrentBuffer[BTPS_StringLength(CurrentBuffer)]), TELECOM_MESSAGE_LISTING_ENTRY_MIDDLE_2);

                                 if(MessageEntry->DateTime)
                                    BTPS_StringCopy(&(CurrentBuffer[BTPS_StringLength(CurrentBuffer)]), MessageEntry->DateTime);

                                 BTPS_StringCopy(&(CurrentBuffer[BTPS_StringLength(CurrentBuffer)]), TELECOM_MESSAGE_LISTING_ENTRY_MIDDLE_3);

                                 if(MessageEntry->Sender)
                                    BTPS_StringCopy(&(CurrentBuffer[BTPS_StringLength(CurrentBuffer)]), MessageEntry->Sender);

                                 BTPS_StringCopy(&(CurrentBuffer[BTPS_StringLength(CurrentBuffer)]), TELECOM_MESSAGE_LISTING_ENTRY_MIDDLE_4);

                                 if(MessageEntry->SenderName)
                                    BTPS_StringCopy(&(CurrentBuffer[BTPS_StringLength(CurrentBuffer)]), MessageEntry->SenderName);

                                 BTPS_StringCopy(&(CurrentBuffer[BTPS_StringLength(CurrentBuffer)]), TELECOM_MESSAGE_LISTING_ENTRY_MIDDLE_5);

                                 if(MessageEntry->SenderAddressing)
                                    BTPS_StringCopy(&(CurrentBuffer[BTPS_StringLength(CurrentBuffer)]), MessageEntry->SenderAddressing);

                                 BTPS_StringCopy(&(CurrentBuffer[BTPS_StringLength(CurrentBuffer)]), TELECOM_MESSAGE_LISTING_ENTRY_MIDDLE_6);

                                 if(MessageEntry->ReplyToAddressing)
                                    BTPS_StringCopy(&(CurrentBuffer[BTPS_StringLength(CurrentBuffer)]), MessageEntry->ReplyToAddressing);

                                 BTPS_StringCopy(&(CurrentBuffer[BTPS_StringLength(CurrentBuffer)]), TELECOM_MESSAGE_LISTING_ENTRY_MIDDLE_7);

                                 if(MessageEntry->RecipientName)
                                    BTPS_StringCopy(&(CurrentBuffer[BTPS_StringLength(CurrentBuffer)]), MessageEntry->RecipientName);

                                 BTPS_StringCopy(&(CurrentBuffer[BTPS_StringLength(CurrentBuffer)]), TELECOM_MESSAGE_LISTING_ENTRY_MIDDLE_8);

                                 if(MessageEntry->RecipientAddressing)
                                    BTPS_StringCopy(&(CurrentBuffer[BTPS_StringLength(CurrentBuffer)]), MessageEntry->RecipientAddressing);

                                 BTPS_StringCopy(&(CurrentBuffer[BTPS_StringLength(CurrentBuffer)]), TELECOM_MESSAGE_LISTING_ENTRY_MIDDLE_9);

                                 if(MessageEntry->Type)
                                    BTPS_StringCopy(&(CurrentBuffer[BTPS_StringLength(CurrentBuffer)]), MessageEntry->Type);

                                 BTPS_StringCopy(&(CurrentBuffer[BTPS_StringLength(CurrentBuffer)]), TELECOM_MESSAGE_LISTING_ENTRY_MIDDLE_10);

                                 sprintf(temp, "%lu", MessageEntry->MessageSize);
                                 BTPS_StringCopy(&(CurrentBuffer[BTPS_StringLength(CurrentBuffer)]), temp);

                                 BTPS_StringCopy(&(CurrentBuffer[BTPS_StringLength(CurrentBuffer)]), TELECOM_MESSAGE_LISTING_ENTRY_MIDDLE_11);

                                 if(MessageEntry->Text)
                                    BTPS_StringCopy(&(CurrentBuffer[BTPS_StringLength(CurrentBuffer)]), MessageEntry->Text);

                                 BTPS_StringCopy(&(CurrentBuffer[BTPS_StringLength(CurrentBuffer)]), TELECOM_MESSAGE_LISTING_ENTRY_MIDDLE_12);

                                 if(MessageEntry->ReceptionStatus)
                                    BTPS_StringCopy(&(CurrentBuffer[BTPS_StringLength(CurrentBuffer)]), MessageEntry->ReceptionStatus);

                                 BTPS_StringCopy(&(CurrentBuffer[BTPS_StringLength(CurrentBuffer)]), TELECOM_MESSAGE_LISTING_ENTRY_MIDDLE_13);

                                 sprintf(temp, "%lu", MessageEntry->AttachmentSize);
                                 BTPS_StringCopy(&(CurrentBuffer[BTPS_StringLength(CurrentBuffer)]), temp);

                                 BTPS_StringCopy(&(CurrentBuffer[BTPS_StringLength(CurrentBuffer)]), TELECOM_MESSAGE_LISTING_ENTRY_MIDDLE_14);

                                 if(MessageEntry->Priority < svNotDefined)
                                    BTPS_StringCopy(&(CurrentBuffer[BTPS_StringLength(CurrentBuffer)]), (MessageEntry->Priority == svNo)?"no":"yes");

                                 BTPS_StringCopy(&(CurrentBuffer[BTPS_StringLength(CurrentBuffer)]), TELECOM_MESSAGE_LISTING_ENTRY_MIDDLE_15);

                                 if(MessageEntry->Read < svNotDefined)
                                    BTPS_StringCopy(&(CurrentBuffer[BTPS_StringLength(CurrentBuffer)]), (MessageEntry->Read == svNo)?"no":"yes");

                                 BTPS_StringCopy(&(CurrentBuffer[BTPS_StringLength(CurrentBuffer)]), TELECOM_MESSAGE_LISTING_ENTRY_MIDDLE_16);

                                 if(MessageEntry->Sent < svNotDefined)
                                    BTPS_StringCopy(&(CurrentBuffer[BTPS_StringLength(CurrentBuffer)]), (MessageEntry->Sent == svNo)?"no":"yes");

                                 BTPS_StringCopy(&(CurrentBuffer[BTPS_StringLength(CurrentBuffer)]), TELECOM_MESSAGE_LISTING_ENTRY_MIDDLE_17);

                                 if(MessageEntry->Protected < svNotDefined)
                                    BTPS_StringCopy(&(CurrentBuffer[BTPS_StringLength(CurrentBuffer)]), (MessageEntry->Protected == svNo)?"no":"yes");

                                 BTPS_StringCopy(&(CurrentBuffer[BTPS_StringLength(CurrentBuffer)]), TELECOM_MESSAGE_LISTING_ENTRY_SUFFIX);

                                 TotalNumberMessages++;
                              }
                           }
                        }
                     }

                     /* Place the Listing Footer on the data (required).*/
                     BTPS_StringCopy(&(CurrentBuffer[BTPS_StringLength(CurrentBuffer)]), TELECOM_MESSAGE_LISTING_SUFFIX);

                     /* All finished, go ahead and send the Message     */
                     /* Listing.                                        */
                     CurrentBufferSize = BTPS_StringLength(CurrentBuffer);

                     /* Note the Nunber of Entries to return in the     */
                     /* response.                                       */
                     NumberEntries     = (Word_t)TotalNumberMessages;

                     /* All finished, go ahead and send the Message     */
                     /* Listing.                                        */
                     Result            = MAP_Get_Message_Listing_Response(BluetoothStackID, MAPID, MAP_OBEX_RESPONSE_OK, &NumberEntries, FALSE, &CurrentTime, CurrentBufferSize, (Byte_t *)CurrentBuffer, &CurrentBufferSent);

                     /* Flag whether or not the operation completed     */
                     /* successfully.                                   */
                     if((Result >= 0) && (CurrentBufferSent != CurrentBufferSize))
                        CurrentOperation = coGetMessageListing;
                     else
                     {
                        free(CurrentBuffer);

                        CurrentBuffer = NULL;
                     }
                  }
                  else
                  {
                     printf("Unable to allocate memory.\r\n");

                     Result = MAP_Get_Message_Listing_Response(BluetoothStackID, MAPID, MAP_OBEX_RESPONSE_SERVICE_UNAVAILABLE, NULL, FALSE, NULL, 0, NULL, NULL);
                  }
               }
            }
            else
            {
               printf("Unable to Get Message Listing: Invalid current directory.\r\n");

               Result = MAP_Get_Message_Listing_Response(BluetoothStackID, MAPID, MAP_OBEX_RESPONSE_NOT_FOUND, NULL, FALSE, NULL, 0, NULL, NULL);
            }

            /* If we changed folders, we need to go back to the parent. */
            if(ChangeFolder)
               ChangeMessageFolder(sfUp, NULL);
         }
      }
      else
      {
         printf("Continuation for MAP Get Message Listing\r\n");

         Result = MAP_Get_Message_Listing_Response(BluetoothStackID, MAPID, MAP_OBEX_RESPONSE_OK, NULL, FALSE, NULL, (CurrentBufferSize - CurrentBufferSent), (Byte_t *)&(CurrentBuffer[CurrentBufferSent]), &Temp);

         /* Flag whether or not the operation completed successfully.   */
         if((Result >= 0) && (Temp != (CurrentBufferSize - CurrentBufferSent)))
            CurrentBufferSent += Temp;
         else
         {
            free(CurrentBuffer);

            CurrentBuffer    = NULL;

            CurrentOperation = coNone;
         }
      }

      if(Result >= 0)
         printf("MAP_Get_Message_Listing_Response() Successful.\r\n");
      else
         printf("MAP_Get_Message_Listing_Response() Failure %d.\r\n", Result);
   }
   else
      printf("Invalid parameter or No longer connected.\r\n");
}

   /* The following function is used to Process the MAP Get Message     */
   /* Indication Event.                                                 */
static void ProcessGetMessage(MAP_Get_Message_Indication_Data_t *MAP_Get_Message_Indication_Data)
{
   int             Result;
   unsigned int    Temp;
   MessageEntry_t *MessageEntry;

   if((BluetoothStackID) && (MAPID) && (Connected) && (MAP_Get_Message_Indication_Data))
   {
      /* Determine if this is the start of new operation or a           */
      /* continuation.                                                  */
      if(CurrentOperation == coNone)
      {
         /* Start of a new operation.                                   */

         if((MAP_Get_Message_Indication_Data->MessageHandle) && (BTPS_StringLength(MAP_Get_Message_Indication_Data->MessageHandle)))
         {
            printf("%s\r\n", MAP_Get_Message_Indication_Data->MessageHandle);
         }

         /* Query the specified Message.                                */
         if((QueryMessageEntryByHandle(cfInvalid, MAP_Get_Message_Indication_Data->MessageHandle, &MessageEntry)) && (MessageEntry))
         {
            /* Message found, format up the correct BMSG.               */
            /* * NOTE * This implementation does not format up a BMSG   */
            /*          dynamically.  We will simply return the         */
            /*          retrieved, pre-formatted, BMSG.                 */

            /* Allocate a buffer to hold the requested Message into.    */
            CurrentBufferSize = BTPS_StringLength(MessageEntry->MessageData);

            if((CurrentBuffer = malloc(CurrentBufferSize)) != NULL)
            {
               BTPS_MemCopy(CurrentBuffer, MessageEntry->MessageData, CurrentBufferSize);

               /* Everything is taken care of, send the response.       */
               Result = MAP_Get_Message_Response(BluetoothStackID, MAPID, MAP_OBEX_RESPONSE_OK, ftUnfragmented, CurrentBufferSize, (Byte_t *)CurrentBuffer, &CurrentBufferSent);

               /* Flag whether or not the operation completed           */
               /* successfully.                                         */
               if((Result >= 0) && (CurrentBufferSent != CurrentBufferSize))
                  CurrentOperation = coGetMessage;
               else
               {
                  free(CurrentBuffer);

                  CurrentBuffer = NULL;
               }
            }
            else
            {
               printf("Unable to allocate memory.\r\n");

               Result = MAP_Get_Message_Response(BluetoothStackID, MAPID, MAP_OBEX_RESPONSE_SERVICE_UNAVAILABLE, ftUnfragmented, 0, NULL, NULL);
            }
         }
         else
         {
            printf("Unable to Get Message: Invalid Message Handle.\r\n");

            Result = MAP_Get_Message_Response(BluetoothStackID, MAPID, MAP_OBEX_RESPONSE_NOT_FOUND, ftUnfragmented, 0, NULL, NULL);
         }
      }
      else
      {
         printf("Continuation for MAP Get Message\r\n");

         Result = MAP_Get_Message_Response(BluetoothStackID, MAPID, MAP_OBEX_RESPONSE_OK, ftUnfragmented, (CurrentBufferSize - CurrentBufferSent), (Byte_t *)&(CurrentBuffer[CurrentBufferSent]), &Temp);

         /* Flag whether or not the operation completed successfully.   */
         if((Result >= 0) && (Temp != (CurrentBufferSize - CurrentBufferSent)))
            CurrentBufferSent += Temp;
         else
         {
            free(CurrentBuffer);

            CurrentBuffer    = NULL;

            CurrentOperation = coNone;
         }
      }

      if(Result >= 0)
         printf("MAP_Get_Message_Response() Successful.\r\n");
      else
         printf("MAP_Get_Message_Response() Failure %d.\r\n", Result);
   }
   else
      printf("Invalid parameter or No longer connected.\r\n");
}

   /* The following function is used to Process the MAP Set Message     */
   /* Status Indication Event.                                          */
static void ProcessSetMessageStatus(MAP_Set_Message_Status_Indication_Data_t *MAP_Set_Message_Status_Indication_Data)
{
   int             Result;
   MessageEntry_t *MessageEntry;

   if((BluetoothStackID) && (MAPID) && (Connected) && (MAP_Set_Message_Status_Indication_Data))
   {
      if((MAP_Set_Message_Status_Indication_Data->MessageHandle) && (BTPS_StringLength(MAP_Set_Message_Status_Indication_Data->MessageHandle)))
      {
         printf("%s\r\n", MAP_Set_Message_Status_Indication_Data->MessageHandle);
      }

      /* Query the specified Message.                                   */
      if((QueryMessageEntryByHandle(cfInvalid, MAP_Set_Message_Status_Indication_Data->MessageHandle, &MessageEntry)) && (MessageEntry))
      {
         /* Message found.  Note that in this implementation we will    */
         /* ignore actually updating the actual Message.                */

         printf("Message Handle: %s\r\n", MAP_Set_Message_Status_Indication_Data->MessageHandle);

         printf("Status Indicator: %d\r\n", MAP_Set_Message_Status_Indication_Data->StatusIndicator);

         printf("Status Value: %d\r\n", MAP_Set_Message_Status_Indication_Data->StatusValue);

         printf("Message %s found: no updates applied (not implemented).\r\n", MAP_Set_Message_Status_Indication_Data->MessageHandle);

         /* Handle the Read Status.                                     */
         if(MAP_Set_Message_Status_Indication_Data->StatusIndicator == siReadStatus)
         {
            /* Check to see if we are to mark as Read or UnRead.        */
            if(MAP_Set_Message_Status_Indication_Data->StatusValue)
            {
               MessageEntry->Read = svYes;

               BTPS_MemCopy(&MessageEntry->MessageData[READ_STATUS_STRING_OFFSET], "READ  ", 6);
            }
            else
            {
               MessageEntry->Read = svNo;

               BTPS_MemCopy(&MessageEntry->MessageData[READ_STATUS_STRING_OFFSET], "UNREAD", 6);
            }
         }

         /* Handle the Delete Status.                                   */
         if(MAP_Set_Message_Status_Indication_Data->StatusIndicator == siDeletedStatus)
         {
            /* Check to see if we are to mark as Delete or UnDelete.    */
            if(MAP_Set_Message_Status_Indication_Data->StatusValue)
            {
               if(!MoveMessageEntryToFolderByHandle(cfDeleted, MAP_Set_Message_Status_Indication_Data->MessageHandle))
                  printf("Unable to Set Message Status: Invalid Message Handle.\r\n");
            }
            else
               MoveMessageEntryToFolderByHandle(cfInbox, MAP_Set_Message_Status_Indication_Data->MessageHandle);
         }

         Result = MAP_Set_Message_Status_Response(BluetoothStackID, MAPID, MAP_OBEX_RESPONSE_OK);
      }
      else
      {
         printf("Unable to Set Message Status: Invalid Message Handle.\r\n");

         Result = MAP_Set_Message_Status_Response(BluetoothStackID, MAPID, MAP_OBEX_RESPONSE_NOT_FOUND);
      }

      if(Result >= 0)
         printf("MAP_Set_Message_Status_Response() Successful.\r\n");
      else
         printf("MAP_Set_Message_Status_Response() Failure %d.\r\n", Result);
   }
   else
      printf("Invalid parameter or No longer connected.\r\n");
}

   /* The following function is used to Process the MAP Push Message    */
   /* Indication Event.                                                 */
static void ProcessPushMessage(MAP_Push_Message_Indication_Data_t *MAP_Push_Message_Indication_Data)
{
   int             Result;
   char            temp[512];
   char            FolderName[256];
   Boolean_t       Final;
   static char     MessageHandle[MAP_MESSAGE_HANDLE_LENGTH+1];
   unsigned int    NameLength;
   unsigned int    Temp;
   CurrentFolder_t Folder;

   if((BluetoothStackID) && (MAPID) && (Connected) && (MAP_Push_Message_Indication_Data))
   {
      /* Determine if this is the start of new operation or a           */
      /* continuation.                                                  */
      if(CurrentOperation == coNone)
      {
         /* Start of a new operation.                                   */

         printf("Start of MAP Push Message\r\n");

         /* Initialize that no error has occurred.                      */
         Temp          = 0;
         FolderName[0] = 0;
         Folder        = GetCurrentMessageFolder();

         /* Check to see if the caller is specifying a sub-folder or the*/
         /* current folder.                                             */
         if((MAP_Push_Message_Indication_Data->FolderName) && (MAP_Push_Message_Indication_Data->FolderName[0]))
         {
            /* First we need to map the UTF-16 Folder name to ASCII     */
            /* (this application will only support ASCII folder names). */
            NameLength = 0;
            while((MAP_Push_Message_Indication_Data->FolderName[NameLength]) && (NameLength < (sizeof(FolderName) - 1)))
            {
               FolderName[NameLength] = (char)MAP_Push_Message_Indication_Data->FolderName[NameLength];

               NameLength++;
            }

            /* Make sure the name is NULL terminated.                   */
            FolderName[NameLength] = '\0';

            printf("%s\r\n", FolderName);

            /* Sub-folder, so navigate into it.                         */
            if(!ChangeMessageFolder(sfDown, FolderName))
            {
               /* get the Destination Folder.                           */
               Folder = GetCurrentMessageFolder();

               /* Go ahead and Change the Folder back since we are not  */
               /* actually going to store the Message into the Message  */
               /* Store (we will just save it as a file).               */
               ChangeMessageFolder(sfUp, NULL);
            }
            else
            {
               /* Invalid directory specified, inform remote side of an */
               /* error.                                                */
               printf("Unable to Push Message: Invalid folder specified.\r\n");

               Result = MAP_Push_Message_Response(BluetoothStackID, MAPID, MAP_OBEX_RESPONSE_NOT_FOUND, NULL);

               /* Flag an error.                                        */
               Temp = 1;
            }
         }

         /* Only continue to process the request if there was not an    */
         /* error.                                                      */
         if(!Temp)
         {
            /* Next, attempt to get a Message Handle for the new        */
            /* message.                                                 */
            /* * NOTE * We will use this Message Handle as the actual   */
            /*          File Name to store the File into.               */
            if(GenerateUniqueMessageHandle(sizeof(MessageHandle), MessageHandle))
            {
               printf("Message Handle Generated: %s\r\n", MessageHandle);

               /* Format up the correct File Name (using the Message    */
               /* Handle and the Current Receive Message Directory.     */
               /* (note that we already made sure that we had a         */
               /* delimiter character at the end of the Current Receive */
               /* Message Path, so we just need to concatenate the      */
               /* Message Handle with the Receive Message Path.         */
               strcpy(CurrentFileName, ReceiveMessagePath);
               strcat(CurrentFileName, MessageHandle);

               /* All finished processing everything, simply inform the */
               /* user of all the received information (as well as the  */
               /* Message Handle).                                      */
               printf("Final: %d\r\n", MAP_Push_Message_Indication_Data->Final);

               printf("Folder Name: %s\r\n", FolderName[0]?FolderName:"");

               printf("Transparent Value: %d\r\n", MAP_Push_Message_Indication_Data->Transparent);

               printf("Retry Value: %d\r\n", MAP_Push_Message_Indication_Data->Retry);

               printf("Character Set: %d\r\n", MAP_Push_Message_Indication_Data->CharSet);

               printf("Data Length: %u\r\n", MAP_Push_Message_Indication_Data->DataLength);

               if(MAP_Push_Message_Indication_Data->DataLength)
               {
                  printf("Data: \r\n");

                  for(Temp=0,temp[0]='\0';(Temp<MAP_Push_Message_Indication_Data->DataLength) && (Temp < (sizeof(temp) - 1));Temp++)
                     sprintf(&(temp[strlen(temp)]), "%c", MAP_Push_Message_Indication_Data->DataBuffer[Temp]);

                  printf("%s\r\n", temp);

                  WriteReceivedData(MAP_Push_Message_Indication_Data->DataLength, MAP_Push_Message_Indication_Data->DataBuffer);
               }

               Final = MAP_Push_Message_Indication_Data->Final;

               /* Set the current operation to a Put Message.           */
               CurrentOperation = (Final)?coNone:coPushMessage;

               if(Final)
               {
                  /* If we have received the entire message than process*/
                  /* the message that was received.                     */
                  InsertMessageEntryIntoFolder(Folder, MessageHandle);
               }

               /* All that is left to do is to respond to the request.  */
               Result = MAP_Push_Message_Response(BluetoothStackID, MAPID, (Byte_t)((Final)?MAP_OBEX_RESPONSE_OK:MAP_OBEX_RESPONSE_CONTINUE), ((Final)?MessageHandle:NULL));
            }
            else
            {
               printf("Unable to Push Message: Unable to create Message Handle.\r\n");

               Result = MAP_Push_Message_Response(BluetoothStackID, MAPID, MAP_OBEX_RESPONSE_SERVER_ERROR, NULL);
            }
         }
      }
      else
      {
         printf("Continuation for MAP Push Message\r\n");

         printf("Data Length: %u\r\n", MAP_Push_Message_Indication_Data->DataLength);

         if(MAP_Push_Message_Indication_Data->DataLength)
         {
            printf("Data:\r\n");

            for(Temp=0,temp[0]='\0';(Temp<MAP_Push_Message_Indication_Data->DataLength) && (Temp < (sizeof(temp) - 1));Temp++)
               sprintf(&(temp[strlen(temp)]), "%c", MAP_Push_Message_Indication_Data->DataBuffer[Temp]);

            printf("%s\r\n", temp);

            WriteReceivedData(MAP_Push_Message_Indication_Data->DataLength, MAP_Push_Message_Indication_Data->DataBuffer);
         }

         /* Get the operation state.                                    */
         Final = MAP_Push_Message_Indication_Data->Final;

         /* Set the current operation to a Put Message.                 */
         CurrentOperation = (Final)?coNone:coPushMessage;

         Result = MAP_Push_Message_Response(BluetoothStackID, MAPID, (Byte_t)((Final)?MAP_OBEX_RESPONSE_OK:MAP_OBEX_RESPONSE_CONTINUE), ((Final)?MessageHandle:NULL));
      }

      if(Result >= 0)
         printf("MAP_Push_Message_Response() Successful.\r\n");
      else
         printf("MAP_Push_Message_Response() Failure %d.\r\n", Result);
   }
   else
      printf("Invalid parameter or No longer connected.\r\n");
}

   /* The following function is used to Process the MAP Process Update  */
   /* Inbox Indication Event.                                           */
static void ProcessUpdateInbox(MAP_Update_Inbox_Indication_Data_t *MAP_Update_Inbox_Indication_Data)
{
   int Result;

   if((BluetoothStackID) && (MAPID) && (Connected) && (MAP_Update_Inbox_Indication_Data))
   {
      printf("Inbox Update Received.\r\n");

      /* There really isn't anything in this implementation to do at    */
      /* this time other than to response with success.                 */
      Result = MAP_Update_Inbox_Response(BluetoothStackID, MAPID, MAP_OBEX_RESPONSE_OK);

      if(Result >= 0)
         printf("MAP_Update_Inbox_Response() Successful.\r\n");
      else
         printf("MAP_Update_Inbox_Response() Failure %d.\r\n", Result);
   }
   else
      printf("Invalid parameter or No longer connected.\r\n");
}

   /* The following function is used to Process the MAP Notification    */
   /* Registration Indication Event.                                    */
static void ProcessNotificationRegistration(MAP_Notification_Registration_Indication_Data_t *MAP_Notification_Registration_Indication_Data)
{
   int Result;

   if((BluetoothStackID) && (Connected) && (MAP_Notification_Registration_Indication_Data))
   {
      printf("Notification Status %d.\r\n", MAP_Notification_Registration_Indication_Data->NotificationStatus);

      if(MAP_Notification_Registration_Indication_Data->NotificationStatus)
      {
         if(!MAPNID)
         {
            /* Send a Successful response to the remote device.         */
            Result = MAP_Set_Notification_Registration_Response(BluetoothStackID, MAPID, MAP_OBEX_RESPONSE_OK);
         }
         else
            Result = MAP_Set_Notification_Registration_Response(BluetoothStackID, MAPID, MAP_OBEX_RESPONSE_FORBIDDEN);

         if(Result >= 0)
            printf("MAP_Set_Notification_Registration_Response() Successful.\r\n");
         else
            printf("MAP_Set_Notification_Registration_Response() Failure %d.\r\n", Result);
      }
      else
      {
         MAP_Set_Notification_Registration_Response(BluetoothStackID, MAPID, MAP_OBEX_RESPONSE_OK);
         if(MAPNID)
         {
            Result = MAP_Disconnect_Request(BluetoothStackID, MAPNID);
            MAPNID = 0;

            if(Result >= 0)
               printf("MAP_Disconnect_Request() Successful.\r\n");
            else
               printf("MAP_Disconnect_Request() Failure %d.\r\n", Result);
         }
      }
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

   /* The following function represents the MAP Profile Server Event    */
   /* Callback.  This function will be called whenever a MAP Server     */
   /* Event occurs that is associated with the specified Bluetooth Stack*/
   /* ID.  This function takes as its parameters the Bluetooth Stack ID,*/
   /* the MAP Event Data that occurred and the MAP Profile Event        */
   /* Callback Parameter that was specified when this Callback was      */
   /* installed.  The caller is free to use the contents of the MAP     */
   /* Event Data ONLY in the context of this callback.  If the caller   */
   /* requires the Data for a longer period of time, then the callback  */
   /* function MUST copy the data into another Data Buffer.  This       */
   /* function is guaranteed NOT to be invoked more than once           */
   /* simultaneously for the specified installed callback (i.e. this    */
   /* function DOES NOT have be reentrant).  It Needs to be noted       */
   /* however, that if the same Callback is installed more than once,   */
   /* then the callbacks will be called serially.  Because of this, the */
   /* processing in this function should be as efficient as possible.   */
   /* It should also be noted that this function is called in the Thread*/
   /* Context of a Thread that the User does NOT own.  Therefore,       */
   /* processing in this function should be as efficient as possible    */
   /* (this argument holds anyway because another MAP Profile Event     */
   /* will not be processed while this function call is outstanding).   */
   /* * NOTE * This function MUST NOT Block and wait for events that can*/
   /*          only be satisfied by Receiving other MAP Events.  A      */
   /*          Deadlock WILL occur because NO MAP Event Callbacks will  */
   /*          be issued while this function is currently outstanding.  */
static void BTPSAPI MAP_Event_Callback_Server(unsigned int BluetoothStackID, MAP_Event_Data_t *MAPEventData, unsigned long CallbackParameter)
{
   char BoardStr[13];

   /* Make sure that parameters passed in appear to be at least         */
   /* semi-valid.                                                       */
   if((BluetoothStackID) && (MAPEventData))
   {
      printf("\r\n");

      switch(MAPEventData->Event_Data_Type)
      {
         case etMAP_Open_Port_Indication:
            printf("Open Port Indication\r\n");
            BD_ADDRToStr(MAPEventData->Event_Data.MAP_Open_Port_Indication_Data->BD_ADDR, BoardStr);
            printf("BD_ADDR: %s\r\n", BoardStr);

            /* Clear any outstanding operation information.             */
            CurrentOperation = coNone;

            /* Flag that we are connected.                              */
            Connected        = TRUE;

            /* Make sure we inform the Message Store module that we are */
            /* currently operating in the Root directory.               */
            ChangeMessageFolder(sfRoot, NULL);
            break;
         case etMAP_Open_Port_Confirmation:
            if(MAPEventData->Event_Data.MAP_Open_Port_Confirmation_Data->OpenStatus == MAP_OPEN_STATUS_SUCCESS)
            {
               NotificationConnected = TRUE;
            }
            else
            {
               MAPNID = 0;
            }

            printf("etMAP_Open_Port_Confirmation Result %d\r\n", MAPEventData->Event_Data.MAP_Open_Port_Confirmation_Data->OpenStatus);
            break;
         case etMAP_Close_Port_Indication:
            printf("Close Port Indication\r\n");

            if(MAPEventData->Event_Data.MAP_Close_Port_Indication_Data->MAPID == (unsigned int)MAPID)
            {
               /* Free any allocated data buffers for ongoing           */
               /* transactions.                                         */
               if((CurrentOperation != coNone) && (CurrentBuffer))
               {
                  free(CurrentBuffer);

                  CurrentBuffer = NULL;
               }

               /* Reset appropriate internal state info.                */
               Connected        = FALSE;
               CurrentOperation = coNone;

               if(MAPNID)
               {
                  MAP_Close_Connection(BluetoothStackID, MAPNID);

                  MAPNID                = 0;
                  NotificationConnected = FALSE;
               }
            }
            else
            {
               MAPNID                = 0;
               NotificationConnected = FALSE;
            }
            break;
         case etMAP_Set_Folder_Indication:
            printf("Set Folder Indication\r\n");

            ProcessSetFolder(MAPEventData->Event_Data.MAP_Set_Folder_Indication_Data);
            break;
         case etMAP_Get_Folder_Listing_Indication:
            printf("Get Folder Listing Indication\r\n");

            ProcessGetFolderListing(MAPEventData->Event_Data.MAP_Get_Folder_Listing_Indication_Data);
            break;
         case etMAP_Get_Message_Listing_Indication:
            printf("Get Message Listing Indication\r\n");

            ProcessGetMessageListing(MAPEventData->Event_Data.MAP_Get_Message_Listing_Indication_Data);
            break;
         case etMAP_Get_Message_Indication:
            printf("Get Message Indication\r\n");

            ProcessGetMessage(MAPEventData->Event_Data.MAP_Get_Message_Indication_Data);
            break;
         case etMAP_Set_Message_Status_Indication:
            printf("Set Message Status Indication\r\n");

            ProcessSetMessageStatus(MAPEventData->Event_Data.MAP_Set_Message_Status_Indication_Data);
            break;
         case etMAP_Push_Message_Indication:
            printf("Push Message Indication\r\n");

            ProcessPushMessage(MAPEventData->Event_Data.MAP_Push_Message_Indication_Data);
            break;
         case etMAP_Update_Inbox_Indication:
            printf("Update Inbox Indication\r\n");

            ProcessUpdateInbox(MAPEventData->Event_Data.MAP_Update_Inbox_Indication_Data);
            break;
         case etMAP_Abort_Indication:
            printf("Abort Indication\r\n");

            /* Free any allocated data buffers for ongoing transactions.*/
            if((CurrentOperation != coNone) && (CurrentBuffer))
            {
               free(CurrentBuffer);

               CurrentBuffer = NULL;
            }

            /* Clear any outstanding operation information.             */
            CurrentOperation = coNone;
            break;
         case etMAP_Notification_Registration_Indication:
            printf("Notification Registration Indication\r\n");

            ProcessNotificationRegistration(MAPEventData->Event_Data.MAP_Notification_Registration_Indication_Data);
            break;
         case etMAP_Send_Event_Confirmation:
            printf("Send Event Confirmation Result 0x%02X\r\n", MAPEventData->Event_Data.MAP_Send_Event_Confirmation_Data->ResponseCode);
            break;
         default:
            /* Unhandled Event.                                         */
            printf("Unexpected (Unhandled) Event %d.\r\n", MAPEventData->Event_Data_Type);
            break;
      }

      printf("Server>");

      /* Make sure the output is displayed to the user.                 */
      fflush(stdout);
   }
}

   /* The following function represents the MAP Profile Client Event    */
   /* Callback.  This function will be called whenever a MAP Client     */
   /* Event occurs that is associated with the specified Bluetooth Stack*/
   /* ID.  This function takes as its parameters the Bluetooth Stack ID,*/
   /* the MAP Event Data that occurred and the MAP Profile Event        */
   /* Callback Parameter that was specified when this Callback was      */
   /* installed.  The caller is free to use the contents of the MAP     */
   /* Event Data ONLY in the context of this callback.  If the caller   */
   /* requires the Data for a longer period of time, then the callback  */
   /* function MUST copy the data into another Data Buffer.  This       */
   /* function is guaranteed NOT to be invoked more than once           */
   /* simultaneously for the specified installed callback (i.e. this    */
   /* function DOES NOT have be reentrant).  It Needs to be noted       */
   /* however, that if the same Callback is installed more than once,   */
   /* then the callbacks will be called serially.  Because of this, the */
   /* processing in this function should be as efficient as possible.   */
   /* It should also be noted that this function is called in the Thread*/
   /* Context of a Thread that the User does NOT own.  Therefore,       */
   /* processing in this function should be as efficient as possible    */
   /* (this argument holds anyway because another MAP Profile Event     */
   /* will not be processed while this function call is outstanding).   */
   /* * NOTE * This function MUST NOT Block and wait for events that can*/
   /*          only be satisfied by Receiving other MAP Events.  A      */
   /*          Deadlock WILL occur because NO MAP Event Callbacks will  */
   /*          be issued while this function is currently outstanding.  */
static void BTPSAPI MAP_Event_Callback_Client(unsigned int BluetoothStackID, MAP_Event_Data_t *MAPEventData, unsigned long CallbackParameter)
{
   int          Result;
   char         Buffer[256];
   unsigned int Index;
   unsigned int TempLength;

   /* Make sure that parameters passed in appear to be at least         */
   /* semi-valid.                                                       */
   if((BluetoothStackID) && (MAPEventData))
   {
      printf("\r\n");

      switch(MAPEventData->Event_Data_Type)
      {
         case etMAP_Open_Port_Indication:
            printf("Open Port Indication:  Notification Client Connected\r\n");

            NotificationConnected = TRUE;
            break;
         case etMAP_Open_Port_Confirmation:
            printf("Open Port Confirmation\r\n");

            printf("Response Code: 0x%02X\r\n", MAPEventData->Event_Data.MAP_Open_Port_Confirmation_Data->OpenStatus);

            /* If this confirmation indicates failure we need to clear  */
            /* MAPID.                                                   */
            if(MAPEventData->Event_Data.MAP_Open_Port_Confirmation_Data->OpenStatus)
            {
               MAPID     = 0;
               Connected = FALSE;
            }

            /* Reset the current operation value.                       */
            CurrentOperation = coNone;
            break;
         case etMAP_Close_Port_Indication:
            printf("Close Port Indication\r\n");

            /* Get the ID of the port that was closed.                  */
            if(MAPEventData->Event_Data.MAP_Close_Port_Indication_Data->MAPID == (unsigned int)MAPID)
            {
               /* Free any allocated data buffers for ongoing           */
               /* transactions.                                         */
               if((CurrentOperation != coNone) && (CurrentBuffer))
               {
                  free(CurrentBuffer);

                  CurrentBuffer = NULL;
               }

               /* Reset appropriate internal state info.                */
               MAPID            = 0;
               Connected        = FALSE;
               CurrentOperation = coNone;

               if(MAPNID)
               {
                  MAP_Close_Server(BluetoothStackID, MAPNID);

                  MAP_Un_Register_SDP_Record(BluetoothStackID, MAPNID, NotificationSDPRecordHandle);

                  MAPNID                      = 0;
                  NotificationConnected       = FALSE;
                  NotificationSDPRecordHandle = 0;
               }
            }
            else
               NotificationConnected = FALSE;
            break;
         case etMAP_Set_Folder_Confirmation:
            printf("Set Folder Confirmation\r\n");
            printf("Response Code: 0x%02X\r\n", MAPEventData->Event_Data.MAP_Set_Folder_Confirmation_Data->ResponseCode);

            CurrentOperation = coNone;
            break;
         case etMAP_Get_Folder_Listing_Confirmation:
            printf("Get Folder Listing Confirmation\r\n");
            printf("Response Code: 0x%02X\r\n", MAPEventData->Event_Data.MAP_Get_Folder_Listing_Confirmation_Data->ResponseCode);
            printf("Number Folders: %d\r\n", MAPEventData->Event_Data.MAP_Get_Folder_Listing_Confirmation_Data->NumberOfFolders);
            printf("Data Length: %u\r\n", MAPEventData->Event_Data.MAP_Get_Folder_Listing_Confirmation_Data->DataLength);

            if(MAPEventData->Event_Data.MAP_Get_Folder_Listing_Confirmation_Data->DataLength)
            {
               printf("Data:\r\n");

               for(Index=0,Buffer[0]='\0';(Index<MAPEventData->Event_Data.MAP_Get_Folder_Listing_Confirmation_Data->DataLength) && (Index < (sizeof(Buffer) - 1));Index++)
                  sprintf(&(Buffer[strlen(Buffer)]), "%c", MAPEventData->Event_Data.MAP_Get_Folder_Listing_Confirmation_Data->DataBuffer[Index]);

               printf("%s\r\n", Buffer);
            }

            /* If this is a successful GET then pass to routine to      */
            /* handle file I/O and further requests (if required).      */
            if(((MAPEventData->Event_Data.MAP_Get_Folder_Listing_Confirmation_Data->ResponseCode == MAP_OBEX_RESPONSE_CONTINUE) || (MAPEventData->Event_Data.MAP_Get_Folder_Listing_Confirmation_Data->ResponseCode == MAP_OBEX_RESPONSE_OK)) && (MAPEventData->Event_Data.MAP_Get_Folder_Listing_Confirmation_Data->DataLength))
               WriteReceivedData(MAPEventData->Event_Data.MAP_Get_Folder_Listing_Confirmation_Data->DataLength, MAPEventData->Event_Data.MAP_Get_Folder_Listing_Confirmation_Data->DataBuffer);

            /* Determine if we need to issue another request to continue*/
            /* the operation.                                           */
            if(MAPEventData->Event_Data.MAP_Get_Folder_Listing_Confirmation_Data->ResponseCode == MAP_OBEX_RESPONSE_CONTINUE)
            {
               /* printf Results.                                       */
               printf("Get Folder Listing not complete, Requesting more data.\r\n");

               /* Continue the Get Folder Listing request.  Note that   */
               /* most parameters passed are ignored on a continuation  */
               /* call.                                                 */
               Result = MAP_Get_Folder_Listing_Request(BluetoothStackID, MAPID, MAP_MAX_LIST_COUNT_NOT_RESTRICTED, 0);

               if(Result >= 0)
               {
                  printf("MAP_Get_Folder_Listing_Request() Successful.\r\n");

                  CurrentOperation = coGetFolderListing;
               }
               else
               {
                  printf("MAP_Get_Folder_Listing_Request() Failure %d.\r\n", Result);

                  CurrentOperation = coNone;
               }
            }
            else
            {
               /* Set current operation.                                */
               CurrentOperation = coNone;

               printf("Get Folder Listing Completed.\r\n");
            }
            break;
         case etMAP_Get_Message_Listing_Confirmation:
            printf("Get Message Listing Confirmation\r\n");
            printf("Response Code: 0x%02X\r\n", MAPEventData->Event_Data.MAP_Get_Message_Listing_Confirmation_Data->ResponseCode);
            printf("New Message: %d\r\n", MAPEventData->Event_Data.MAP_Get_Message_Listing_Confirmation_Data->NewMessage);
            printf("MSE Time: %02u/%02u/%04u %02u:%02u:%02u", MAPEventData->Event_Data.MAP_Get_Message_Listing_Confirmation_Data->MSETime.Month, MAPEventData->Event_Data.MAP_Get_Message_Listing_Confirmation_Data->MSETime.Day, MAPEventData->Event_Data.MAP_Get_Message_Listing_Confirmation_Data->MSETime.Year, MAPEventData->Event_Data.MAP_Get_Message_Listing_Confirmation_Data->MSETime.Hour, MAPEventData->Event_Data.MAP_Get_Message_Listing_Confirmation_Data->MSETime.Minute, MAPEventData->Event_Data.MAP_Get_Message_Listing_Confirmation_Data->MSETime.Second);
            if(MAPEventData->Event_Data.MAP_Get_Message_Listing_Confirmation_Data->MSETime.UTC_Time)
               printf(" (%+05d)\r\n", MAPEventData->Event_Data.MAP_Get_Message_Listing_Confirmation_Data->MSETime.UTC_Offset);
            else
               printf(" (UTC Offset Unspecified)\r\n");
            printf("Number Messages: %d\r\n", MAPEventData->Event_Data.MAP_Get_Message_Listing_Confirmation_Data->NumberOfMessages);
            printf("Data Length: %u\r\n", MAPEventData->Event_Data.MAP_Get_Message_Listing_Confirmation_Data->DataLength);

            if(MAPEventData->Event_Data.MAP_Get_Message_Listing_Confirmation_Data->DataLength)
            {
               printf("Data:\r\n");

               for(Index=0,Buffer[0]='\0';(Index<MAPEventData->Event_Data.MAP_Get_Message_Listing_Confirmation_Data->DataLength) && (Index < (sizeof(Buffer) - 1));Index++)
                  sprintf(&(Buffer[strlen(Buffer)]), "%c", MAPEventData->Event_Data.MAP_Get_Message_Listing_Confirmation_Data->DataBuffer[Index]);

               printf("%s\r\n", Buffer);
            }

            /* If this is a successful GET then pass to routine to      */
            /* handle file I/O and further requests (if required).      */
            if(((MAPEventData->Event_Data.MAP_Get_Message_Listing_Confirmation_Data->ResponseCode == MAP_OBEX_RESPONSE_CONTINUE) || (MAPEventData->Event_Data.MAP_Get_Message_Listing_Confirmation_Data->ResponseCode == MAP_OBEX_RESPONSE_OK)) && (MAPEventData->Event_Data.MAP_Get_Message_Listing_Confirmation_Data->DataLength))
               WriteReceivedData(MAPEventData->Event_Data.MAP_Get_Message_Listing_Confirmation_Data->DataLength, MAPEventData->Event_Data.MAP_Get_Message_Listing_Confirmation_Data->DataBuffer);

            /* Determine if we need to issue another request to continue*/
            /* the operation.                                           */
            if(MAPEventData->Event_Data.MAP_Get_Message_Listing_Confirmation_Data->ResponseCode == MAP_OBEX_RESPONSE_CONTINUE)
            {
               /* printf Results.                                       */
               printf("Get Message Listing not complete, Requesting more data.\r\n");

               /* Continue the Get Message Listing request.  Note that  */
               /* most parameters passed are ignored on a continuation  */
               /* call.                                                 */
               Result = MAP_Get_Message_Listing_Request(BluetoothStackID, MAPID, NULL, MAP_MAX_LIST_COUNT_NOT_RESTRICTED, 0, NULL);

               if(Result >= 0)
               {
                  printf("MAP_Get_Message_Listing_Request() Successful.\r\n");

                  CurrentOperation = coGetMessageListing;
               }
               else
               {
                  printf("MAP_Get_Message_Listing_Request() Failure %d.\r\n", Result);

                  CurrentOperation = coNone;
               }
            }
            else
            {
               /* Set current operation.                                */
               CurrentOperation = coNone;

               printf("Get Message Listing Completed.\r\n");
            }
            break;
         case etMAP_Get_Message_Confirmation:
            printf("Get Message Confirmation\r\n");
            printf("Response Code: 0x%02X\r\n", MAPEventData->Event_Data.MAP_Get_Message_Confirmation_Data->ResponseCode);
            printf("Fractional Type: %d\r\n", MAPEventData->Event_Data.MAP_Get_Message_Confirmation_Data->FractionalType);
            printf("Data Length: %u\r\n", MAPEventData->Event_Data.MAP_Get_Message_Confirmation_Data->DataLength);

            if(MAPEventData->Event_Data.MAP_Get_Message_Confirmation_Data->DataLength)
            {
               printf("Data:\r\n");

               for(Index=0,Buffer[0]='\0';(Index<MAPEventData->Event_Data.MAP_Get_Message_Confirmation_Data->DataLength) && (Index < (sizeof(Buffer) - 1));Index++)
                  sprintf(&(Buffer[strlen(Buffer)]), "%c", MAPEventData->Event_Data.MAP_Get_Message_Confirmation_Data->DataBuffer[Index]);

               printf("%s\r\n", Buffer);
            }

            /* If this is a successful GET then pass to routine to      */
            /* handle file I/O and further requests (if required).      */
            if(((MAPEventData->Event_Data.MAP_Get_Message_Listing_Confirmation_Data->ResponseCode == MAP_OBEX_RESPONSE_CONTINUE) || (MAPEventData->Event_Data.MAP_Get_Message_Confirmation_Data->ResponseCode == MAP_OBEX_RESPONSE_OK)) && (MAPEventData->Event_Data.MAP_Get_Message_Confirmation_Data->DataLength))
               WriteReceivedData(MAPEventData->Event_Data.MAP_Get_Message_Confirmation_Data->DataLength, MAPEventData->Event_Data.MAP_Get_Message_Confirmation_Data->DataBuffer);

            /* Determine if we need to issue another request to continue*/
            /* the operation.                                           */
            if(MAPEventData->Event_Data.MAP_Get_Message_Confirmation_Data->ResponseCode == MAP_OBEX_RESPONSE_CONTINUE)
            {
               /* printf Results.                                       */
               printf("Get Message not complete, Requesting more data.\r\n");

               /* Continue the Get Message request.  Note that most     */
               /* parameters passed are ignored on a continuation call. */
               Result = MAP_Get_Message_Request(BluetoothStackID, MAPID, NULL, FALSE, csUTF8, ftUnfragmented);

               if(Result >= 0)
               {
                  printf("MAP_Get_Message_Request() Successful.\r\n");

                  CurrentOperation = coGetMessage;
               }
               else
               {
                  printf("MAP_Get_Message_Request() Failure %d.\r\n", Result);

                  CurrentOperation = coNone;
               }
            }
            else
            {
               /* Set current operation.                                */
               CurrentOperation = coNone;

               printf("Get Message Completed.\r\n");
            }
            break;
         case etMAP_Set_Message_Status_Confirmation:
            printf("Set Message Status Confirmation\r\n");
            printf("Response Code: 0x%02X\r\n", MAPEventData->Event_Data.MAP_Set_Message_Status_Confirmation_Data->ResponseCode);

            CurrentOperation = coNone;
            break;
         case etMAP_Push_Message_Confirmation:
            printf("Push MessageConfirmation\r\n");
            printf("Response Code: 0x%02X\r\n", MAPEventData->Event_Data.MAP_Push_Message_Confirmation_Data->ResponseCode);

            /* Determine if we need to issue another request to continue*/
            /* the operation.                                           */
            if((MAPEventData->Event_Data.MAP_Push_Message_Confirmation_Data->ResponseCode == MAP_OBEX_RESPONSE_CONTINUE) && (CurrentBufferSent != CurrentBufferSize))
            {
               /* printf Results.                                       */
               printf("Message Push not complete, Sending more data.\r\n");

               /* Continue the Push Message request.  Note that most    */
               /* parameters passed are ignored on a continuation call. */
               Result = MAP_Push_Message_Request(BluetoothStackID, MAPID, NULL, FALSE, FALSE, csUTF8, (CurrentBufferSize - CurrentBufferSent), (Byte_t *)&(CurrentBuffer[CurrentBufferSent]), &TempLength, TRUE);

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

               printf("Message Push Completed.\r\n");

               if(CurrentBuffer)
                  free(CurrentBuffer);

               CurrentBuffer = NULL;
            }
            break;
         case etMAP_Update_Inbox_Confirmation:
            printf("Update Inbox Confirmation\r\n");
            printf("Response Code: 0x%02X\r\n", MAPEventData->Event_Data.MAP_Update_Inbox_Confirmation_Data->ResponseCode);

            CurrentOperation = coNone;
            break;
         case etMAP_Abort_Confirmation:
            printf("Abort Confirmation\r\n");

            /* Reset the current operation value.                       */
            CurrentOperation = coNone;
            break;
         case etMAP_Notification_Registration_Confirmation:
            printf("Notification Registration Confirmation Result 0x%02X\r\n", MAPEventData->Event_Data.MAP_Notification_Registration_Confirmation_Data->ResponseCode);
            break;
         case etMAP_Send_Event_Indication:
            if(MAPEventData->Event_Data.MAP_Send_Event_Indication_Data->DataLength)
            {
               printf("Data: ");

               for(Index=0;Index<MAPEventData->Event_Data.MAP_Send_Event_Indication_Data->DataLength;Index++)
                  printf("%c", MAPEventData->Event_Data.MAP_Send_Event_Indication_Data->DataBuffer[Index]);

               printf("\r\n");
            }

            MAP_Send_Event_Response(BluetoothStackID, MAPNID, MAP_OBEX_RESPONSE_OK);
            break;
         default:
            printf("Unexpected (Unhandled) Event %d.\r\n", MAPEventData->Event_Data_Type);
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

