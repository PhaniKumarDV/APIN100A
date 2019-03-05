/*****< linuxpba.c >***********************************************************/
/*      Copyright 2009 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  LINUXPBA - Simple Linux application using Phonebook Access Profile (PBAP).*/
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   08/22/09  D. Lange       Initial creation.                               */
/******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>

#include "LinuxPBA.h"      /* Main Application Prototypes and Constants.      */

#include "Phonebook.h"     /* Sample Phonebook Store Prototypes/Constants.    */

#include "SS1BTPS.h"       /* Includes for the SS1 Bluetooth Protocol Stack.  */
#include "SS1BTPBA.h"      /* Includes for the SS1 Phonebook Access Profile.  */

#include "SS1BTDBG.h"      /* Includes/Constants for Bluetooth Debugging.     */

#define LOCAL_NAME_ROOT                      "LinuxPBA"  /* Root of the local */
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

#define DEFAULT_NUMBER_MISSED_CALLS                 (1)  /* Denotes the Missed*/
                                                         /* Call Number that  */
                                                         /* is returned if a  */
                                                         /* Missed Call       */
                                                         /* Query is          */
                                                         /* received.         */

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
                                                         /* create a PBAP     */
                                                         /* Server.           */

#define INDENT_LENGTH                               (3)  /* Denotes the number*/
                                                         /* of character      */
                                                         /* spaces to be used */
                                                         /* for indenting when*/
                                                         /* displaying SDP    */
                                                         /* Data Elements.    */

   /* The following constants represent the default log file names that */
   /* are used if no Log file name is specified when enabling debug.    */
#define DEFAULT_DEBUG_LOG_FILE_NAME  "LinuxPBA_ASC.log"
#define DEFAULT_DEBUG_FTS_FILE_NAME  "LinuxPBA_FTS.log"

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
   coPullPhonebook,
   coSetPhonebook,
   coPullvCardListing,
   coPullvCardEntry,
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

static unsigned int        PBAPID;                  /* Variable used to hold the       */
                                                    /* PBAP Client/Server ID of the    */
                                                    /* currently active PBAP Profile   */
                                                    /* Role.                           */

static DWord_t             ServerSDPRecordHandle;   /* Variable used to hold the       */
                                                    /* PBAP Server SDP Service Record  */
                                                    /* Handle of the PBAP Server SDP   */
                                                    /* Service Record.                 */

static Boolean_t           Connected;               /* Variable which reflects whether */
                                                    /* or not there is an active       */
                                                    /* (on-going) connection present.  */

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

static int OpenRemoteServer(ParameterList_t *TempParam);
static int CloseConnection(ParameterList_t *TempParam);

static int OBEXDisconnect(ParameterList_t *TempParam);
static int Abort(ParameterList_t *TempParam);
static int PullPhonebook(ParameterList_t *TempParam);
static int SetPhonebook(ParameterList_t *TempParam);
static int PullvCardList(ParameterList_t *TempParam);
static int PullvCardEntry(ParameterList_t *TempParam);

static void ProcessPullPhonebook(PBAP_Pull_Phonebook_Indication_Data_t *PBAP_Pull_Phonebook_Indication_Data);
static void ProcessSetPhonebook(PBAP_Set_Phonebook_Indication_Data_t *PBAP_Set_Phonebook_Indication_Data);
static void ProcessPullvCardListing(PBAP_Pull_vCard_Listing_Indication_Data_t *PBAP_Pull_vCard_Listing_Indication_Data);
static void ProcessPullvCardEntry(PBAP_Pull_vCard_Entry_Indication_Data_t *PBAP_Pull_vCard_Entry_Indication_Data);

static void DisplaySDPAttributeResponse(SDP_Service_Attribute_Response_Data_t *SDPServiceAttributeResponse, unsigned int InitLevel);
static void DisplaySDPSearchAttributeResponse(SDP_Service_Search_Attribute_Response_Data_t *SDPServiceSearchAttributeResponse);
static void DisplayDataElement(SDP_Data_Element_t *SDPDataElement, unsigned int Level);

   /* BTPS Callback function prototypes.                                */
static void BTPSAPI GAP_Event_Callback(unsigned int BluetoothStackID, GAP_Event_Data_t *GAP_Event_Data, unsigned long CallbackParameter);
static void BTPSAPI SDP_Event_Callback(unsigned int BluetoothStackID, unsigned int SDPRequestID, SDP_Response_Data_t *SDP_Response_Data, unsigned long CallbackParameter);
static void BTPSAPI PBAP_Event_Callback_Server(unsigned int BluetoothStackID, PBAP_Event_Data_t *PBAPEventData, unsigned long CallbackParameter);
static void BTPSAPI PBAP_Event_Callback_Client(unsigned int BluetoothStackID, PBAP_Event_Data_t *PBAPEventData, unsigned long CallbackParameter);

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
   AddCommand("OPEN", OpenRemoteServer);
   AddCommand("CLOSE", CloseConnection);
   AddCommand("OBEXDISCONNECT", OBEXDisconnect);
   AddCommand("PULLPHONEBOOK", PullPhonebook);
   AddCommand("SETPHONEBOOK", SetPhonebook);
   AddCommand("PULLVCARDLIST", PullvCardList);
   AddCommand("PULLVCARDENTRY", PullvCardEntry);
   AddCommand("ABORT", Abort);
   AddCommand("ENABLEDEBUG", EnableDebug);
   AddCommand("HELP", DisplayHelp);

   /* Go ahead and initialize the required state variables.             */
   PBAPID           = 0;
   Connected        = FALSE;
   CurrentBuffer    = NULL;
   CurrentOperation = coNone;

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
                     if(PBAPID)
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
         if(PBAPID)
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

   /* Go ahead and initialize the required state variables.             */
   PBAPID           = 0;
   Connected        = FALSE;
   CurrentBuffer    = NULL;
   CurrentOperation = coNone;

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
                     if(PBAPID)
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
         if(PBAPID)
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
      if(PBAPID)
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
   /* Command Options for either PBAP Client or Server.  The input      */
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
      printf("*                  SetPairabilityMode,                           *\r\n");
      printf("*                  ChangeSimplePairingParameters,                *\r\n");
      printf("*                  GetLocalAddress, GetLocalName, SetLocalName,  *\r\n");
      printf("*                  GetClassOfDevice, SetClassOfDevice,           *\r\n");
      printf("*                  GetRemoteName, ServiceDiscovery, Open, Close, *\r\n");
      printf("*                  OBEXDisconnect, PullPhonebook,                *\r\n");
      printf("*                  SetPhonebook, PullvCardList, PullvCARDEntry,  *\r\n");
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
      printf("*                  GetRemoteName, ServiceDiscovery,              *\r\n");
      printf("*                  Open, Close, Disconnect,                      *\r\n");
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

   /* The following function is responsible for creating a local PBAP   */
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
      if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].intParam))
      {
         /* Now check to make sure that a Server doesn't already exist. */
         if(!PBAPID)
         {
            /* The above parameters are valid, attempt to open a Local  */
            /* PBAP Port.                                               */
            Result = PBAP_Open_Server_Port(BluetoothStackID, TempParam->Params[0].intParam, (PBAP_SUPPORTED_REPOSITORIES_LOCAL_PHONEBOOK), PBAP_Event_Callback_Server, 0);

            /* Now check to see if the function call was successfully   */
            /* made.                                                    */
            if(Result > 0)
            {
               /* Now that a Service Name has been created try and      */
               /* Register the SDP Record.                              */
               if(!PBAP_Register_Server_SDP_Record(BluetoothStackID, Result, "PBAP Server", &ServerSDPRecordHandle))
               {
                  /* The Server was opened successfully and the SDP     */
                  /* Record was successfully added.  The return value of*/
                  /* the call is the PBAP ID and is required for future */
                  /* PBAP Profile calls.                                */
                  PBAPID  = Result;

                  ret_val = 0;

                  printf("PBAP_Open_Server_Port() Successful (ID = %04X).\r\n", PBAPID);
               }
               else
               {
                  /* Error registering SDP Record, go ahead and close   */
                  /* down the server.                                   */
                  PBAP_Close_Server_Port(BluetoothStackID, Result);

                  printf("PBAP_Register_Server_SDP_Record() Failure.\r\n");

                  ret_val = FUNCTION_ERROR;
               }
            }
            else
            {
               /* There was an error opening the Server.                */
               printf("PBAP_Open_Server_Port() Failure: %d.\r\n", Result);

               ret_val = Result;
            }
         }
         else
         {
            /* A Connection is already open, this program only supports */
            /* one Server or Client at a time.                          */
            printf("PBAP Server already open.\r\n");

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         printf("Usage: Open [Port Number].\r\n");

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

   /* The following function is responsible for deleting a local PBAP   */
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
      if(PBAPID)
      {
         /* Free any allocated data buffers for ongoing transactions.   */
         if((CurrentOperation != coNone) && (CurrentBuffer))
         {
            free(CurrentBuffer);

            CurrentBuffer = NULL;
         }

         /* PBAP Server opened, close any open SDP Record.              */
         if(ServerSDPRecordHandle)
         {
            PBAP_Un_Register_SDP_Record(BluetoothStackID, PBAPID, ServerSDPRecordHandle);

            ServerSDPRecordHandle = 0;
         }

         /* Next attempt to close this Server.                          */
         Result = PBAP_Close_Server_Port(BluetoothStackID, PBAPID);

         if(!Result)
         {
            /* Display a message indicating that the server was         */
            /* successfully closed.                                     */
            printf("PBAP_Close_Server_Port() Success.\r\n");

            /* Flag the port has been closed.                           */
            PBAPID           = 0;

            ret_val          = 0;

            CurrentOperation = coNone;
         }
         else
         {
            /* An error occurred while attempting to close the server.  */
            printf("PBAP_Close_Server_Port() Failure: %d.\r\n", Result);

            ret_val = Result;
         }
      }
      else
      {
         /* No valid PBAP Server exists.                                */
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
   /* with a Remote PBAP Server.  This function returns zero if         */
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
      if(!PBAPID)
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
               /* the program is about to open a Remote PBAP Port.      */
               printf("Opening Remote PBAP Server (BD_ADDR = %s, Port = %04X)\r\n", BoardStr, TempParam->Params[1].intParam);

               /* Attempt to open a connection to the selected remote   */
               /* Phonebook Access Profile Server.                      */
               Result = PBAP_Open_Remote_Server_Port(BluetoothStackID, InquiryResultList[(TempParam->Params[0].intParam-1)], TempParam->Params[1].intParam, PBAP_Event_Callback_Client, 0);

               /* Now check to see if the function call was successfully*/
               /* made.                                                 */
               if(Result > 0)
               {
                  /* The Client was opened successfully.  The return    */
                  /* value of the call is the PBAP ID and is required   */
                  /* for future PBAP Profile calls.                     */
                  PBAPID  = Result;

                  ret_val = 0;

                  printf("PBAP_Open_Remote_Server_Port: Function Successful (ID = %04X).\r\n", PBAPID);
               }
               else
               {
                  /* There was an error opening the Client.             */
                  printf("PBAP_Open_Remote_Server_Port() Failure: %d.\r\n", Result);

                  ret_val = Result;
               }
            }
            else
            {
               printf("Usage: Open [Inquiry Index] [Server Port].\r\n");

               ret_val = INVALID_PARAMETERS_ERROR;
            }
         }
         else
         {
            printf("Usage: Open [Inquiry Index] [Server Port].\r\n");

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
   /* with a remote PBAP Client or Server.  This function returns zero  */
   /* if successful and a negative value if an error occurred.          */
static int CloseConnection(ParameterList_t *TempParam)
{
   int ret_val;
   int Result;

   /* First check to see if a valid Bluetooth Stack exists.             */
   if(BluetoothStackID)
   {
      /* Check to see if the PBAPID appears to be semi-valid.  This     */
      /* parameter will only be valid if a Connection is open or a      */
      /* Server is Open.                                                */
      if(PBAPID)
      {
         /* Free any allocated data buffers for ongoing transactions.   */
         if((CurrentOperation != coNone) && (CurrentBuffer))
         {
            free(CurrentBuffer);

            CurrentBuffer = NULL;
         }

         /* The Current PBAPID appears to be semi-valid.  Now try to    */
         /* close the connection.                                       */
         Result = PBAP_Close_Connection(BluetoothStackID, PBAPID);

         if(!Result)
         {
            /* The function was called successfully.  Display a message */
            /* indicating that the Connection Close was successfully    */
            /* submitted.                                               */
            printf("PBAP_Close_Connection: Function Successful.\r\n");

            Connected        = FALSE;
            CurrentOperation = coNone;

            ret_val          = 0;

            if(IsClient)
               PBAPID = 0;
         }
         else
         {
            /* An error occurred while attempting to close the          */
            /* Connection.                                              */
            printf("PBAP_Close_Connection() Failure: %d.\r\n", Result);

            ret_val = Result;
         }
      }
      else
      {
         /* The Current PBAPID is invalid so no Connection or Server is */
         /* open.                                                       */
         printf("Invalid PBAP ID: PBAP Close Connection.\r\n");

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

   /* The following function is responsible for terminating a connection*/
   /* with a remote PBAP Server by issuing an OBEX Disconnect.  This    */
   /* function returns zero if successful and a negative value if an    */
   /* error occurred.                                                   */
static int OBEXDisconnect(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the PBAP ID appears to be          */
      /* semi-valid.                                                    */
      if(PBAPID)
      {
         /* The PBAP ID appears to be is a semi-valid value.  Now submit*/
         /* the command to begin this operation.                        */
         Result = PBAP_Disconnect_Request(BluetoothStackID, PBAPID);

         if(!Result)
         {
            /* The function was submitted successfully.                 */

            /* Update the Current Operation.                            */
            CurrentOperation = coDisconnect;

            ret_val          = 0;

            printf("PBAP_Disconnect_Request: Function Successful.\r\n");
         }
         else
         {
            /* There was an error submitting the function.              */
            printf("PBAP_Disconnect_Request() Failure: %d.\r\n", Result);

            ret_val = Result;
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("PBAP OBEX Disconnect Request: Invalid PBAPID.\r\n");

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
   /* remote PBAP Server by issuing an OBEX Abort Command.  This        */
   /* function returns zero if successful and a negative value if an    */
   /* error occurred.                                                   */
static int Abort(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the PBAP ID appears to be          */
      /* semi-valid.                                                    */
      if(PBAPID)
      {
         /* The PBAP ID appears to be is a semi-valid value.  Now submit*/
         /* the command to begin this operation.                        */
         Result = PBAP_Abort_Request(BluetoothStackID, PBAPID);

         if(!Result)
         {
            /* The function was submitted successfully.                 */

            /* Update the Current Operation.                            */
            CurrentOperation = coAbort;

            ret_val          = 0;

            printf("PBAP_Abort_Request: Function Successful.\r\n");
         }
         else
         {
            /* There was an error submitting the function.              */
            printf("PBAP_Abort_Request() Failure: %d.\r\n", Result);

            ret_val = Result;
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("PBAP Abort Request: Invalid PBAPID.\r\n");

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

   /* The following function is responsible for issuing a PBAP Pull     */
   /* Phonebook Request with a remote PBAP Server.  This function       */
   /* returns zero if successful and a negative value if an error       */
   /* occurred.                                                         */
static int PullPhonebook(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the PBAP ID appears to be          */
      /* semi-valid.                                                    */
      if(PBAPID)
      {
         /* The PBAP ID appears to be is a semi-valid value.  Next,     */
         /* check to see if required parameters were specified.         */
         if((TempParam) && (TempParam->NumberofParameters >= 3) && (TempParam->Params[2].strParam) && (strlen(TempParam->Params[2].strParam)))
         {
            /* Check to see if a filename was specified to save the     */
            /* result to.                                               */
            if((TempParam->NumberofParameters >= 4) && (TempParam->Params[3].strParam) && (strlen(TempParam->Params[3].strParam)))
               strcpy(CurrentFileName, TempParam->Params[3].strParam);
            else
               CurrentFileName[0] = '\0';

            Result = PBAP_Pull_Phonebook_Request(BluetoothStackID, PBAPID, TempParam->Params[2].strParam, 0, 0, pfvCard21, (Word_t)TempParam->Params[0].intParam, (Word_t)TempParam->Params[1].intParam);

            if(!Result)
            {
               /* The function was submitted successfully.              */
               printf("PBAP_Pull_Phonebook_Request() Successful.\r\n");

               if((TempParam->Params[0].intParam) && (CurrentFileName[0]))
                  printf("Filename used to store results: %s\r\n", CurrentFileName);

               /* Update the Current Operation.                         */
               CurrentOperation = coPullPhonebook;

               ret_val          = 0;
            }
            else
            {
               /* There was an error submitting the function.           */
               printf("PBAP_Pull_Phonebook_Request() Failure: %d.\r\n", Result);

               ret_val = Result;
            }
         }
         else
         {
            printf("Usage: PullPhonebook [Max List Count] [List Start Offset] [Phonebook Path] [Save Filename (optional)].\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("Pull Phonebook Request: Invalid PBAPID.\r\n");

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

   /* The following function is responsible for issuing a PBAP Set      */
   /* Phonebook Request with a remote PBAP Server.  This function       */
   /* returns zero if successful and a negative value if an error       */
   /* occurred.                                                         */
static int SetPhonebook(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the PBAP ID appears to be          */
      /* semi-valid.                                                    */
      if(PBAPID)
      {
         /* The PBAP ID appears to be is a semi-valid value.  Next,     */
         /* check to see if required parameters were specified.         */
         if((TempParam) && (TempParam->NumberofParameters >= 1) && (TempParam->Params[0].intParam >= (int)spRoot) && (TempParam->Params[0].intParam <= (int)spUp))
         {
            /* Check to see if a Path was specified if the down option  */
            /* was specified.                                           */
            if((TempParam->Params[0].intParam != (int)spDown) || (((TempParam->Params[0].intParam == (int)spDown) && (TempParam->NumberofParameters >= 2) && (TempParam->Params[1].strParam) && (strlen(TempParam->Params[1].strParam)))))
            {
               Result = PBAP_Set_Phonebook_Request(BluetoothStackID, PBAPID, (PBAP_Set_Path_Option_t)TempParam->Params[0].intParam, TempParam->Params[1].strParam);

               if(!Result)
               {
                  /* The function was submitted successfully.                 */
                  printf("PBAP_Set_Phonebook_Request() Successful.\r\n");

                  /* Update the Current Operation.                            */
                  CurrentOperation = coSetPhonebook;

                  ret_val          = 0;
               }
               else
               {
                  /* There was an error submitting the function.              */
                  printf("PBAP_Set_Phonebook_Request() Failure: %d.\r\n", Result);

                  ret_val = Result;
               }
            }
            else
            {
               printf("Usage: SetPhonebook [Path Option (0 = Root, 1 = Down, 2 = Up)] [Path (only required for Down option)].\r\n");

               ret_val = INVALID_PARAMETERS_ERROR;
            }
         }
         else
         {
            printf("Usage: SetPhonebook [Path Option (0 = Root, 1 = Down, 2 = Up)] [Path (only required for Down option)].\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("Set Phonebook Request: Invalid PBAPID.\r\n");

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

   /* The following function is responsible for issuing a PBAP Pull     */
   /* vCARD Listing Request with a remote PBAP Server.  This function   */
   /* returns zero if successful and a negative value if an error       */
   /* occurred.                                                         */
static int PullvCardList(ParameterList_t *TempParam)
{
   int   Result;
   int   ret_val;
   char *Phonebook;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the PBAP ID appears to be          */
      /* semi-valid.                                                    */
      if(PBAPID)
      {
         /* The PBAP ID appears to be is a semi-valid value.  Next,     */
         /* check to see if required parameters were specified.         */
         if((TempParam) && (TempParam->NumberofParameters >= 2))
         {
            /* Determine the correct Phonebook (if specified).          */
            if((TempParam->NumberofParameters >= 3) && (TempParam->Params[2].strParam))
            {
               /* Phonebook specified, check to see if it was only      */
               /* specified so that the save file could be specified.   */
               if((strlen(TempParam->Params[2].strParam) == 1) && (TempParam->Params[2].strParam[0] == '.'))
                  Phonebook = NULL;
               else
                  Phonebook = TempParam->Params[2].strParam;
            }
            else
               Phonebook = NULL;

            /* Check to see if a filename was specified to save the     */
            /* result to.                                               */
            if((TempParam->NumberofParameters >= 4) && (TempParam->Params[3].strParam) && (strlen(TempParam->Params[3].strParam)))
               strcpy(CurrentFileName, TempParam->Params[3].strParam);
            else
               CurrentFileName[0] = '\0';

            Result = PBAP_Pull_vCard_Listing_Request(BluetoothStackID, PBAPID, Phonebook, loDefault, saDefault, NULL, (Word_t)TempParam->Params[0].intParam, (Word_t)TempParam->Params[1].intParam);

            if(!Result)
            {
               /* The function was submitted successfully.              */
               printf("PBAP_Pull_vCard_Listing_Request() Successful.\r\n");

               if((TempParam->Params[0].intParam) && (CurrentFileName[0]))
                  printf("Filename used to store results: %s\r\n", CurrentFileName);

               /* Update the Current Operation.                         */
               CurrentOperation = coPullvCardListing;

               ret_val          = 0;
            }
            else
            {
               /* There was an error submitting the function.           */
               printf("PBAP_Pull_vCard_Listing_Request() Failure: %d.\r\n", Result);

               ret_val = Result;
            }
         }
         else
         {
            printf("Usage: PullvCardList [Max List Count] [List Start Offset] [Phonebook Path (optional - specify '.' to specify Save File Name)] [Save Filename (optional)].\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("Pull vCard Listing Request: Invalid PBAPID.\r\n");

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

   /* The following function is responsible for issuing a PBAP Pull     */
   /* vCARD Entry Request with a remote PBAP Server.  This function     */
   /* returns zero if successful and a negative value if an error       */
   /* occurred.                                                         */
static int PullvCardEntry(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the PBAP ID appears to be          */
      /* semi-valid.                                                    */
      if(PBAPID)
      {
         /* The PBAP ID appears to be is a semi-valid value.  Next,     */
         /* check to see if required parameters were specified.         */
         if((TempParam) && (TempParam->NumberofParameters >= 1) && (TempParam->Params[0].strParam) && (strlen(TempParam->Params[0].strParam)))
         {
            /* Check to see if a filename was specified to save the     */
            /* result to.                                               */
            if((TempParam->NumberofParameters >= 2) && (TempParam->Params[1].strParam) && (strlen(TempParam->Params[1].strParam)))
               strcpy(CurrentFileName, TempParam->Params[1].strParam);
            else
               CurrentFileName[0] = '\0';

            Result = PBAP_Pull_vCard_Entry_Request(BluetoothStackID, PBAPID, TempParam->Params[0].strParam, 0, 0, pfvCard21);

            if(!Result)
            {
               /* The function was submitted successfully.              */
               printf("PBAP_Pull_vCard_Entry_Request() Successful.\r\n");

               if(CurrentFileName[0])
                  printf("Filename used to store results: %s\r\n", CurrentFileName);

               /* Update the Current Operation.                         */
               CurrentOperation = coPullvCardEntry;

               ret_val          = 0;
            }
            else
            {
               /* There was an error submitting the function.           */
               printf("PBAP_Pull_vCard_Entry_Request() Failure: %d.\r\n", Result);

               ret_val = Result;
            }
         }
         else
         {
            printf("Usage: PullvCardEntry [vCard Name] [Save Filename (optional)].\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("Pull vCard Entry Request: Invalid PBAPID.\r\n");

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

   /* The following function is used to Process the PBAP Pull Phonebook */
   /* Indication Event.                                                 */
static void ProcessPullPhonebook(PBAP_Pull_Phonebook_Indication_Data_t *PBAP_Pull_Phonebook_Indication_Data)
{
   int                  Result;
   char               **vCardList;
   Word_t               NumberEntries;
   Byte_t               NumberMissedCalls;
   unsigned int         Index;
   unsigned int         Temp;
   unsigned int         NumbervCards;
   unsigned int         TotalNumberCards;
   CurrentDirectory_t   Directory;

   if((BluetoothStackID) && (PBAPID) && (Connected) && (PBAP_Pull_Phonebook_Indication_Data))
   {
      /* Determine if this is the start of new operation or a           */
      /* continuation.                                                  */
      if(CurrentOperation == coNone)
      {
         /* Start of a new operation.                                   */
         if((PBAP_Pull_Phonebook_Indication_Data->ObjectName) && (BTPS_StringLength(PBAP_Pull_Phonebook_Indication_Data->ObjectName)))
            printf(PBAP_Pull_Phonebook_Indication_Data->ObjectName);
         else
            printf("Invalid Object Name specified.");

         printf("\r\n");

         /* Map the fully qualified path phonebook name to the phonebook*/
         /* directory that we support.                                  */
         Directory = DeterminePhonebookDirectory(PBAP_Pull_Phonebook_Indication_Data->ObjectName);

         if(Directory != cdInvalid)
         {
            /* Phonebook is supported.  Go ahead and retrieve all       */
            /* phonebook entries and build them into the buffer.        */
            if((vCardList = QueryvCardList(Directory, &NumbervCards)) == NULL)
               NumbervCards = 0;

            /* Determine the total size of all the vCards.              */
            /* * NOTE * Here is where we would apply filtering (to      */
            /*          affect the length).                             */
            /* * NOTE * We will honor the List Start Offset and the Max */
            /*          List Count Parameters here.                     */
            for(Index=0,CurrentBufferSize=0,TotalNumberCards=0;Index<NumbervCards;Index++)
            {
               if(vCardList[Index])
               {
                  if((Index >= (unsigned int)PBAP_Pull_Phonebook_Indication_Data->ListStartOffset) && (TotalNumberCards < (unsigned int)PBAP_Pull_Phonebook_Indication_Data->MaxListCount))
                  {
                     CurrentBufferSize += strlen(vCardList[Index]);

                     TotalNumberCards++;
                  }
               }
            }

            /* Now that we know the entire length, go ahead and allocate*/
            /* space to hold the vCards.                                */

            /* Check to see if this is a request to determine the       */
            /* maximum number of entries.                               */
            if(!PBAP_Pull_Phonebook_Indication_Data->MaxListCount)
            {
               printf("Request for Number of Entries: %d\r\n", NumbervCards);

               NumberEntries = (Word_t)NumbervCards;

               Result = PBAP_Pull_Phonebook_Response(BluetoothStackID, PBAPID, PBAP_OBEX_RESPONSE_OK, &NumberEntries, NULL, 0, NULL, NULL);
            }
            else
            {
               /* Go ahead and note the number of missed calls (we might*/
               /* use it later if this is a request to pull the Missed  */
               /* Call History Phonebook).                              */
               NumberMissedCalls = (Byte_t)DEFAULT_NUMBER_MISSED_CALLS;

               /* * NOTE * Here is where we would support all of the    */
               /*          filtering options as well as supplying the   */
               /*          correct vCard format (either 2.1 or 3.0).    */
               /*          For this sample demonstration, however, we   */
               /*          will not actually support filtering and      */
               /*          changing the format to match the requested   */
               /*          format.                                      */

               /* Allocate a buffer to hold all the requested vCards    */
               /* into.                                                 */
               /* * NOTE * We will allocate an extra byte to take care  */
               /*          of the NULL terminator.                      */
               if((CurrentBuffer = malloc(CurrentBufferSize + 1)) != NULL)
               {
                  /* Buffer allocated, go ahead and build the vCard     */
                  /* buffer.                                            */
                  for(Index=0,CurrentBuffer[0]=0,TotalNumberCards=0;Index<NumbervCards;Index++)
                  {
                     if(vCardList[Index])
                     {
                        if((Index >= (unsigned int)PBAP_Pull_Phonebook_Indication_Data->ListStartOffset) && (TotalNumberCards < (unsigned int)PBAP_Pull_Phonebook_Indication_Data->MaxListCount))
                        {
                           BTPS_StringCopy(&(CurrentBuffer[BTPS_StringLength(CurrentBuffer)]), vCardList[Index]);

                           TotalNumberCards++;
                        }
                     }
                  }

                  /* All finished, go ahead and send the phonebook data.*/
                  Result = PBAP_Pull_Phonebook_Response(BluetoothStackID, PBAPID, PBAP_OBEX_RESPONSE_OK, NULL, ((Directory == cdMissedCallHistory)?(&NumberMissedCalls):NULL), CurrentBufferSize, (Byte_t *)CurrentBuffer, &CurrentBufferSent);

                  /* Flag whether or not the operation completed        */
                  /* successfully.                                      */
                  if((Result >= 0) && (CurrentBufferSent != CurrentBufferSize))
                     CurrentOperation = coPullPhonebook;
                  else
                  {
                     free(CurrentBuffer);

                     CurrentBuffer = NULL;
                  }
               }
               else
               {
                  printf("Unable to allocate memory.\r\n");

                  Result = PBAP_Pull_Phonebook_Response(BluetoothStackID, PBAPID, PBAP_OBEX_RESPONSE_SERVICE_UNAVAILABLE, NULL, NULL, 0, NULL, NULL);
               }
            }
         }
         else
         {
            printf("Invalid/Unsupported Phonebook specified.\r\n");

            Result = PBAP_Pull_Phonebook_Response(BluetoothStackID, PBAPID, PBAP_OBEX_RESPONSE_BAD_REQUEST, NULL, NULL, 0, NULL, NULL);
         }
      }
      else
      {
         printf("Continuation for PBAP Pull Phonebook\r\n");

         Result = PBAP_Pull_Phonebook_Response(BluetoothStackID, PBAPID, PBAP_OBEX_RESPONSE_OK, NULL, NULL, (CurrentBufferSize - CurrentBufferSent), (Byte_t *)&(CurrentBuffer[CurrentBufferSent]), &Temp);

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
         printf("PBAP_Pull_Phonebook_Response() Successful.\r\n");
      else
         printf("PBAP_Pull_Phonebook_Response() Failure %d.\r\n", Result);
   }
   else
      printf("Invalid parameter or No longer connected.\r\n");
}

   /* The following function is used to Process the PBAP Set Phonebook  */
   /* Indication Event.                                                 */
static void ProcessSetPhonebook(PBAP_Set_Phonebook_Indication_Data_t *PBAP_Set_Phonebook_Indication_Data)
{
   int     Result;
   char   *NameString;
   Byte_t  ResponseCode;

   if((BluetoothStackID) && (PBAPID) && (Connected) && (PBAP_Set_Phonebook_Indication_Data))
   {
      /* All that is left to do is simply attempt to change the         */
      /* phonebook path.                                                */
      if(!ChangePhonebookDirectory(PBAP_Set_Phonebook_Indication_Data->PathOption, PBAP_Set_Phonebook_Indication_Data->ObjectName))
      {
         printf("Phonebook successfully set to: %s\r\n", ((NameString = GetCurrentPhonebookDirectoryString()) != NULL)?NameString:"");

         ResponseCode = PBAP_OBEX_RESPONSE_OK;
      }
      else
      {
         printf("Unable to set Phonebook: %d, \"%s\"\r\n", PBAP_Set_Phonebook_Indication_Data->PathOption, PBAP_Set_Phonebook_Indication_Data->ObjectName?PBAP_Set_Phonebook_Indication_Data->ObjectName:"");

         ResponseCode = PBAP_OBEX_RESPONSE_NOT_FOUND;
      }

      Result = PBAP_Set_Phonebook_Response(BluetoothStackID, PBAPID, ResponseCode);

      if(Result >= 0)
         printf("PBAP_Set_Phonebook_Response() Successful.\r\n");
      else
         printf("PBAP_Set_Phonebook_Response() Failure %d.\r\n", Result);
   }
   else
      printf("Invalid parameter or No longer connected.\r\n");
}

   /* The following function is used to Process the PBAP Pull vCard     */
   /* Listing Indication Event.                                         */
static void ProcessPullvCardListing(PBAP_Pull_vCard_Listing_Indication_Data_t *PBAP_Pull_vCard_Listing_Indication_Data)
{
   int            Result;
   char           temp[132];
   char         **vCardList;
   Word_t         NumberEntries;
   Byte_t         NumberMissedCalls;
   Boolean_t      ChangeDirectory;
   unsigned int   Index;
   unsigned int   Temp;
   unsigned int   NumbervCards;
   unsigned int   TotalNumberCards;

   if((BluetoothStackID) && (PBAPID) && (Connected) && (PBAP_Pull_vCard_Listing_Indication_Data))
   {
      /* Determine if this is the start of new operation or a           */
      /* continuation.                                                  */
      if(CurrentOperation == coNone)
      {
         /* Start of a new operation.                                   */

         /* Initialize that no error has occurred.                      */
         Temp   = 0;
         Result = 0;

         /* Check to see if the caller is specifying a subdirectory or  */
         /* the current directory.                                      */
         if((PBAP_Pull_vCard_Listing_Indication_Data->ObjectName) && (BTPS_StringLength(PBAP_Pull_vCard_Listing_Indication_Data->ObjectName)))
         {
            printf(PBAP_Pull_vCard_Listing_Indication_Data->ObjectName);
            printf("\r\n");

            /* Subdirectory, so navigate into it.                       */
            if(!ChangePhonebookDirectory(spDown, PBAP_Pull_vCard_Listing_Indication_Data->ObjectName))
               ChangeDirectory = TRUE;
            else
            {
               /* Invalid directory specified, inform remote side of an */
               /* error.                                                */
               printf("Unable to pull vCard Listing: Invalid directory specified.\r\n");

               Result = PBAP_Pull_vCard_Listing_Response(BluetoothStackID, PBAPID, PBAP_OBEX_RESPONSE_NOT_FOUND, NULL, NULL, 0, NULL, NULL);

               /* Flag an error.                                        */
               ChangeDirectory = FALSE;

               Temp            = 1;
            }
         }
         else
            ChangeDirectory = FALSE;

         /* Only continue to process the request if there was not an    */
         /* error.                                                      */
         if(!Temp)
         {
            /* First attempt to see if there are any vCards to be found */
            /* in the current directory.                                */
            if((vCardList = QueryvCardList(GetCurrentPhonebookDirectory(), &NumbervCards)) == NULL)
               NumbervCards = 0;

            if(vCardList)
            {
               /* Determine the total size of all the vCards.           */
               /* * NOTE * Here is where we would apply filtering (to   */
               /*          affect the length).                          */
               /* * NOTE * We will honor the List Start Offset and the  */
               /*          Max List Count Parameters here.              */
               for(Index=0,CurrentBufferSize=(BTPS_StringLength(TELECOM_PHONEBOOK_LISTING_PREFIX) + BTPS_StringLength(TELECOM_PHONEBOOK_LISTING_SUFFIX)),TotalNumberCards=0;Index<NumbervCards;Index++)
               {
                  if(vCardList[Index])
                  {
                     if((Index >= (unsigned int)PBAP_Pull_vCard_Listing_Indication_Data->ListStartOffset) && (TotalNumberCards < (unsigned int)PBAP_Pull_vCard_Listing_Indication_Data->MaxListCount))
                     {
                        /* Add the size of all required headers.        */
                        CurrentBufferSize += (BTPS_StringLength(TELECOM_PHONEBOOK_LISTING_ENTRY_PREFIX) + BTPS_StringLength(TELECOM_PHONEBOOK_LISTING_ENTRY_MIDDLE) + BTPS_StringLength(TELECOM_PHONEBOOK_LISTING_ENTRY_SUFFIX));

                        /* Format Handle and determine it's length.     */
                        sprintf(temp, "%u.vcf", Index);
                        CurrentBufferSize += BTPS_StringLength(temp);

                        /* Build the Name from the vCard and determine  */
                        /* it's length.                                 */
                        ExtractvCardName(vCardList[Index], sizeof(temp), temp);

                        CurrentBufferSize += BTPS_StringLength(temp);

                        TotalNumberCards++;
                     }
                  }
               }

               /* Now that we know the entire length, go ahead and      */
               /* allocate space to hold the vCard Listing.             */

               /* Check to see if this is a request to determine the    */
               /* maximum number of entries.                            */
               if(!PBAP_Pull_vCard_Listing_Indication_Data->MaxListCount)
               {
                  printf(temp, "Request for Number of Entries: %d\r\n", NumbervCards);

                  NumberEntries = (Word_t)NumbervCards;

                  Result = PBAP_Pull_vCard_Listing_Response(BluetoothStackID, PBAPID, PBAP_OBEX_RESPONSE_OK, &NumberEntries, NULL, 0, NULL, NULL);
               }
               else
               {
                  /* Go ahead and note the number of missed calls (we   */
                  /* might use it later if this is a request to pull the*/
                  /* Missed Call History Phonebook).                    */
                  NumberMissedCalls = (Byte_t)DEFAULT_NUMBER_MISSED_CALLS;

                  /* * NOTE * Here is where we would support all of the */
                  /*          sorting and searching options.  For this  */
                  /*          sample demonstration, however, we will not*/
                  /*          actually support sorting and searching to */
                  /*          match the request.                        */

                  /* Allocate a buffer to hold the requested vCard      */
                  /* Listing into.                                      */
                  /* * NOTE * We will allocate an extra byte to take    */
                  /*          care of the NULL terminator.              */
                  if((CurrentBuffer = malloc(CurrentBufferSize + 1)) != NULL)
                  {
                     /* Buffer allocated, go ahead and build the vCard  */
                     /* Listing buffer.                                 */

                     /* Place the Listing Header on the data (required).*/
                     sprintf((char *)CurrentBuffer, TELECOM_PHONEBOOK_LISTING_PREFIX);

                     for(Index=0,TotalNumberCards=0;Index<NumbervCards;Index++)
                     {
                        if(vCardList[Index])
                        {
                           if((Index >= (unsigned int)PBAP_Pull_vCard_Listing_Indication_Data->ListStartOffset) && (TotalNumberCards < (unsigned int)PBAP_Pull_vCard_Listing_Indication_Data->MaxListCount))
                           {
                              BTPS_StringCopy(&(CurrentBuffer[BTPS_StringLength(CurrentBuffer)]), TELECOM_PHONEBOOK_LISTING_ENTRY_PREFIX);

                              /* Format Handle and place into the       */
                              /* listing.                               */
                              sprintf(temp, "%u.vcf", Index);
                              BTPS_StringCopy(&(CurrentBuffer[BTPS_StringLength(CurrentBuffer)]), temp);

                              BTPS_StringCopy(&(CurrentBuffer[BTPS_StringLength(CurrentBuffer)]), TELECOM_PHONEBOOK_LISTING_ENTRY_MIDDLE);

                              /* Build the Name from the vCard and      */
                              /* determine it's length.                 */
                              ExtractvCardName(vCardList[Index], sizeof(temp), temp);
                              BTPS_StringCopy(&(CurrentBuffer[BTPS_StringLength(CurrentBuffer)]), temp);

                              BTPS_StringCopy(&(CurrentBuffer[BTPS_StringLength(CurrentBuffer)]), TELECOM_PHONEBOOK_LISTING_ENTRY_SUFFIX);

                              TotalNumberCards++;
                           }
                        }
                     }

                     /* Place the Listing Footer on the data (required).*/
                     BTPS_StringCopy(&(CurrentBuffer[BTPS_StringLength(CurrentBuffer)]), TELECOM_PHONEBOOK_LISTING_SUFFIX);

                     /* All finished, go ahead and send the vCard       */
                     /* Listing.                                        */
                     Result = PBAP_Pull_vCard_Listing_Response(BluetoothStackID, PBAPID, PBAP_OBEX_RESPONSE_OK, NULL, ((GetCurrentPhonebookDirectory() == cdMissedCallHistory)?(&NumberMissedCalls):NULL), CurrentBufferSize, (Byte_t *)CurrentBuffer, &CurrentBufferSent);

                     /* Flag whether or not the operation completed     */
                     /* successfully.                                   */
                     if((Result >= 0) && (CurrentBufferSent != CurrentBufferSize))
                        CurrentOperation = coPullvCardListing;
                     else
                     {
                        free(CurrentBuffer);

                        CurrentBuffer = NULL;
                     }
                  }
                  else
                  {
                     printf("Unable to allocate memory.\r\n");

                     Result = PBAP_Pull_vCard_Listing_Response(BluetoothStackID, PBAPID, PBAP_OBEX_RESPONSE_SERVICE_UNAVAILABLE, NULL, NULL, 0, NULL, NULL);
                  }
               }
            }
            else
            {
               printf("Unable to pull vCard Listing: Invalid current directory.\r\n");

               Result = PBAP_Pull_vCard_Listing_Response(BluetoothStackID, PBAPID, PBAP_OBEX_RESPONSE_NOT_FOUND, NULL, NULL, 0, NULL, NULL);
            }

            /* If we changed directories, we need to go back to the     */
            /* parent.                                                  */
            if(ChangeDirectory)
               ChangePhonebookDirectory(spUp, NULL);
         }
      }
      else
      {
         printf("Continuation for PBAP Pull vCard Listing\r\n");

         Result = PBAP_Pull_vCard_Listing_Response(BluetoothStackID, PBAPID, PBAP_OBEX_RESPONSE_OK, NULL, NULL, (CurrentBufferSize - CurrentBufferSent), (Byte_t *)&(CurrentBuffer[CurrentBufferSent]), &Temp);

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
         printf("PBAP_Pull_vCard_Listing_Response() Successful.\r\n");
      else
         printf("PBAP_Pull_vCard_Listing_Response() Failure %d.\r\n", Result);
   }
   else
      printf("Invalid parameter or No longer connected.\r\n");
}

   /* The following function is used to Process the PBAP Pull vCard     */
   /* Entry Indication Event.                                           */
static void ProcessPullvCardEntry(PBAP_Pull_vCard_Entry_Indication_Data_t *PBAP_Pull_vCard_Entry_Indication_Data)
{
   int            Result;
   char         **vCardList;
   unsigned int   Index;
   unsigned int   Temp;
   unsigned int   NumbervCards;

   if((BluetoothStackID) && (PBAPID) && (Connected) && (PBAP_Pull_vCard_Entry_Indication_Data))
   {
      /* Determine if this is the start of new operation or a           */
      /* continuation.                                                  */
      if(CurrentOperation == coNone)
      {
         /* Start of a new operation.                                   */

         if((PBAP_Pull_vCard_Entry_Indication_Data->ObjectName) && (BTPS_StringLength(PBAP_Pull_vCard_Entry_Indication_Data->ObjectName)))
            printf(PBAP_Pull_vCard_Entry_Indication_Data->ObjectName, TRUE);
         else
            printf("Invalid Object Name specified.");

         printf("\r\n");

         /* First attempt to see if there are any vCards to be found in */
         /* the current directory.                                      */
         if((vCardList = QueryvCardList(GetCurrentPhonebookDirectory(), &NumbervCards)) == NULL)
            NumbervCards = 0;

         if(vCardList)
         {
            if(PBAP_Pull_vCard_Entry_Indication_Data->ObjectName)
               sscanf(PBAP_Pull_vCard_Entry_Indication_Data->ObjectName, "%u.vcf", &Index);
            else
               Index = 0xFFFFFFFF;

            /* Check to see if the requested vCard is present in the    */
            /* vCard List.                                              */
            if(Index < NumbervCards)
            {
               if(vCardList[Index])
                  CurrentBufferSize = BTPS_StringLength(vCardList[Index]);
               else
                  CurrentBufferSize = 0;

               /* * NOTE * Here is where we would support all of the    */
               /*          filtering and formatting options.  For this  */
               /*          sample demonstration, however, we will not   */
               /*          actually support filtering and formatting to */
               /*          match the request.                           */

               /* Allocate a buffer to hold the requested vCard Entry   */
               /* into.                                                 */
               /* * NOTE * We will allocate an extra byte to take care  */
               /*          of the NULL terminator.                      */
               if((CurrentBufferSize) && ((CurrentBuffer = malloc(CurrentBufferSize + 1)) != NULL))
               {
                  /* Buffer allocated, go ahead and build the vCard     */
                  /* Entry buffer.                                      */
                  if(vCardList[Index])
                     BTPS_StringCopy((char *)CurrentBuffer, vCardList[Index]);

                  /* All finished, go ahead and send the vCard Entry.   */
                  Result = PBAP_Pull_vCard_Entry_Response(BluetoothStackID, PBAPID, PBAP_OBEX_RESPONSE_OK, CurrentBufferSize, (Byte_t *)CurrentBuffer, &CurrentBufferSent);

                  /* Flag whether or not the operation completed        */
                  /* successfully.                                      */
                  if((Result >= 0) && (CurrentBufferSent != CurrentBufferSize))
                     CurrentOperation = coPullvCardEntry;
                  else
                  {
                     free(CurrentBuffer);

                     CurrentBuffer = NULL;
                  }
               }
               else
               {
                  if(CurrentBufferSize)
                     printf("Unable to allocate memory.\r\n");
                  else
                     printf("Invalid vCard Entry specified.\r\n");

                  Result = PBAP_Pull_vCard_Entry_Response(BluetoothStackID, PBAPID, (Byte_t)(CurrentBufferSize?PBAP_OBEX_RESPONSE_NOT_FOUND:PBAP_OBEX_RESPONSE_SERVICE_UNAVAILABLE), 0, NULL, NULL);
               }
            }
            else
            {
               printf("Invalid vCard Entry specified.\r\n");

               Result = PBAP_Pull_vCard_Entry_Response(BluetoothStackID, PBAPID, PBAP_OBEX_RESPONSE_NOT_FOUND, 0, NULL, NULL);
            }
         }
         else
         {
            printf("Unable to pull vCard Entry: Invalid current directory.\r\n");

            Result = PBAP_Pull_vCard_Entry_Response(BluetoothStackID, PBAPID, PBAP_OBEX_RESPONSE_NOT_FOUND, 0, NULL, NULL);
         }
      }
      else
      {
         printf("Continuation for PBAP Pull vCard Entry\r\n");

         Result = PBAP_Pull_vCard_Entry_Response(BluetoothStackID, PBAPID, PBAP_OBEX_RESPONSE_OK, (CurrentBufferSize - CurrentBufferSent), (Byte_t *)&(CurrentBuffer[CurrentBufferSent]), &Temp);

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
         printf("PBAP_Pull_vCard_Entry_Response() Successful.\r\n");
      else
         printf("PBAP_Pull_vCard_Entry_Response() Failure %d.\r\n", Result);
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

   /* The following function represents the PBAP Profile Server Event   */
   /* Callback.  This function will be called whenever a PBAP Server    */
   /* Event occurs that is associated with the specified Bluetooth Stack*/
   /* ID.  This function takes as its parameters the Bluetooth Stack ID,*/
   /* the PBAP Event Data that occurred and the PBAP Profile Event      */
   /* Callback Parameter that was specified when this Callback was      */
   /* installed.  The caller is free to use the contents of the PBAP    */
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
   /* (this argument holds anyway because another PBAP Profile Event    */
   /* will not be processed while this function call is outstanding).   */
   /* * NOTE * This function MUST NOT Block and wait for events that can*/
   /*          only be satisfied by Receiving other PBAP Events.  A     */
   /*          Deadlock WILL occur because NO PBAP Event Callbacks will */
   /*          be issued while this function is currently outstanding.  */
static void BTPSAPI PBAP_Event_Callback_Server(unsigned int BluetoothStackID, PBAP_Event_Data_t *PBAPEventData, unsigned long CallbackParameter)
{
   char BoardStr[13];

   /* Make sure that parameters passed in appear to be at least         */
   /* semi-valid.                                                       */
   if((BluetoothStackID) && (PBAPEventData))
   {
      printf("\r\n");

      switch(PBAPEventData->Event_Data_Type)
      {
         case etPBAP_Open_Port_Indication:
            printf("Open Port Indication\r\n");
            BD_ADDRToStr(PBAPEventData->Event_Data.PBAP_Open_Port_Indication_Data->BD_ADDR, BoardStr);
            printf("BD_ADDR: %s\r\n", BoardStr);

            /* Clear any outstanding operation information.             */
            CurrentOperation = coNone;

            /* Flag that we are connected.                              */
            Connected        = TRUE;

            /* Make sure we inform the Phonebook module that we are     */
            /* currently operating in the Root directory.               */
            ChangePhonebookDirectory(spRoot, NULL);
            break;
         case etPBAP_Close_Port_Indication:
            printf("Close Port Indication\r\n");

            /* Free any allocated data buffers for ongoing transactions.*/
            if((CurrentOperation != coNone) && (CurrentBuffer))
            {
               free(CurrentBuffer);

               CurrentBuffer = NULL;
            }

            /* Clear any outstanding operation information.             */
            CurrentOperation = coNone;

            /* Flag that we are no longer connected.                    */
            Connected        = FALSE;
            break;
         case etPBAP_Abort_Indication:
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
         case etPBAP_Pull_Phonebook_Indication:
            printf("Pull Phonebook Indication\r\n");

            ProcessPullPhonebook(PBAPEventData->Event_Data.PBAP_Pull_Phonebook_Indication_Data);
            break;
         case etPBAP_Set_Phonebook_Indication:
            printf("Set Phonebook Indication\r\n");

            ProcessSetPhonebook(PBAPEventData->Event_Data.PBAP_Set_Phonebook_Indication_Data);
            break;
         case etPBAP_Pull_vCard_Listing_Indication:
            printf("Pull vCard Listing Indication\r\n");

            ProcessPullvCardListing(PBAPEventData->Event_Data.PBAP_Pull_vCard_Listing_Indication_Data);
            break;
         case etPBAP_Pull_vCard_Entry_Indication:
            printf("Pull vCard Entry Indication\r\n");

            ProcessPullvCardEntry(PBAPEventData->Event_Data.PBAP_Pull_vCard_Entry_Indication_Data);
            break;
         default:
            /* Unhandled Event.                                         */
            printf("Unexpected (Unhandled) Event %d.\r\n", PBAPEventData->Event_Data_Type);
            break;
      }

      printf("Server>");

      /* Make sure the output is displayed to the user.                 */
      fflush(stdout);
   }
}

   /* The following function represents the PBAP Profile Client Event   */
   /* Callback.  This function will be called whenever a PBAP Client    */
   /* Event occurs that is associated with the specified Bluetooth Stack*/
   /* ID.  This function takes as its parameters the Bluetooth Stack ID,*/
   /* the PBAP Event Data that occurred and the PBAP Profile Event      */
   /* Callback Parameter that was specified when this Callback was      */
   /* installed.  The caller is free to use the contents of the PBAP    */
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
   /* (this argument holds anyway because another PBAP Profile Event    */
   /* will not be processed while this function call is outstanding).   */
   /* * NOTE * This function MUST NOT Block and wait for events that can*/
   /*          only be satisfied by Receiving other PBAP Events.  A     */
   /*          Deadlock WILL occur because NO PBAP Event Callbacks will */
   /*          be issued while this function is currently outstanding.  */
static void BTPSAPI PBAP_Event_Callback_Client(unsigned int BluetoothStackID, PBAP_Event_Data_t *PBAPEventData, unsigned long CallbackParameter)
{
   int          Result;
   char         Buffer[128];
   unsigned int Index;

   /* Make sure that parameters passed in appear to be at least         */
   /* semi-valid.                                                       */
   if((BluetoothStackID) && (PBAPEventData))
   {
      printf("\r\n");

      switch(PBAPEventData->Event_Data_Type)
      {
         case etPBAP_Open_Port_Confirmation:
            printf("Open Port Confirmation\r\n");

            printf("Response Code: 0x%02X\r\n", PBAPEventData->Event_Data.PBAP_Open_Port_Confirmation_Data->PBAPConnectStatus);

            /* If this confirmation indicates failure we need to clear  */
            /* PBAPID.                                                  */
            if(PBAPEventData->Event_Data.PBAP_Open_Port_Confirmation_Data->PBAPConnectStatus)
            {
               PBAPID    = 0;
               Connected = FALSE;
            }

            /* Reset the current operation value.                       */
            CurrentOperation = coNone;
            break;
         case etPBAP_Close_Port_Indication:
            printf("Close Port Indication\r\n");

            /* Free any allocated data buffers for ongoing transactions.*/
            if((CurrentOperation != coNone) && (CurrentBuffer))
            {
               free(CurrentBuffer);

               CurrentBuffer = NULL;
            }

            /* Reset appropriate internal state info.                   */
            PBAPID           = 0;
            Connected        = FALSE;
            CurrentOperation = coNone;
            break;
         case etPBAP_Abort_Confirmation:
            printf("Abort Confirmation\r\n");

            /* Reset the current operation value.                       */
            CurrentOperation = coNone;
            break;
         case etPBAP_Pull_Phonebook_Confirmation:
            printf("Pull Phonebook Confirmation\r\n");

            printf("Response Code: 0x%02X\r\n", PBAPEventData->Event_Data.PBAP_Pull_Phonebook_Confirmation_Data->ResponseCode);
            printf("Phonebook Size: %d\r\n", PBAPEventData->Event_Data.PBAP_Pull_Phonebook_Confirmation_Data->PhonebookSize);
            printf("NewMissedCalls: %d\r\n", PBAPEventData->Event_Data.PBAP_Pull_Phonebook_Confirmation_Data->NewMissedCalls);
            printf("Final: %d\r\n", PBAPEventData->Event_Data.PBAP_Pull_Phonebook_Confirmation_Data->Final);
            printf("Data Length: %u\r\n", PBAPEventData->Event_Data.PBAP_Pull_Phonebook_Confirmation_Data->BufferSize);

            if(PBAPEventData->Event_Data.PBAP_Pull_Phonebook_Confirmation_Data->BufferSize)
            {
               printf("Data: \r\n");

               for(Index=0,Buffer[0]='\0';(Index<PBAPEventData->Event_Data.PBAP_Pull_Phonebook_Confirmation_Data->BufferSize) && (Index < (sizeof(Buffer) - 1));Index++)
                  sprintf(&(Buffer[strlen(Buffer)]), "%c", PBAPEventData->Event_Data.PBAP_Pull_Phonebook_Confirmation_Data->Buffer[Index]);

               printf(Buffer);
               printf("\r\n");
            }

            /* If this is a successful GET then pass to routine to      */
            /* handle file I/O and further requests (if required).      */
            if(((PBAPEventData->Event_Data.PBAP_Pull_Phonebook_Confirmation_Data->ResponseCode == PBAP_OBEX_RESPONSE_CONTINUE) || (PBAPEventData->Event_Data.PBAP_Pull_Phonebook_Confirmation_Data->ResponseCode == PBAP_OBEX_RESPONSE_OK)) && (PBAPEventData->Event_Data.PBAP_Pull_Phonebook_Confirmation_Data->BufferSize))
               WriteReceivedData(PBAPEventData->Event_Data.PBAP_Pull_Phonebook_Confirmation_Data->BufferSize, PBAPEventData->Event_Data.PBAP_Pull_Phonebook_Confirmation_Data->Buffer);

            /* Determine if we need to issue another request to continue*/
            /* the operation.                                           */
            if(!PBAPEventData->Event_Data.PBAP_Pull_Phonebook_Confirmation_Data->Final)
            {
               /* printf Results.                                       */
               printf("Pull Phonebook not complete, Requesting more data.\r\n");

               /* Continue the Pull Phonebook request.  Note that most  */
               /* parameters passed are ignored on a continuation call. */
               Result = PBAP_Pull_Phonebook_Request(BluetoothStackID, PBAPID, NULL, 0, 0, pfDefault, PBAP_MAX_LIST_COUNT_NOT_RESTRICTED, 0);

               if(Result >= 0)
               {
                  printf("PBAP_Pull_Phonebook_Request() Successful.\r\n");

                  CurrentOperation = coPullPhonebook;
               }
               else
               {
                  printf("PBAP_Pull_Phonebook_Request() Failure %d.\r\n", Result);

                  CurrentOperation = coNone;
               }
            }
            else
            {
               /* Set current operation.                                */
               CurrentOperation = coNone;

               printf("Pull Phonebook Completed.\r\n");
            }
            break;
         case etPBAP_Set_Phonebook_Confirmation:
            printf("Set Phonebook Confirmation\r\n");
            printf("Response Code: 0x%02X\r\n", PBAPEventData->Event_Data.PBAP_Set_Phonebook_Confirmation_Data->ResponseCode);

            CurrentOperation = coNone;
            break;
         case etPBAP_Pull_vCard_Listing_Confirmation:
            printf("Pull vCard Listing Confirmation\r\n");

            printf("Response Code: 0x%02X\r\n", PBAPEventData->Event_Data.PBAP_Pull_vCard_Listing_Confirmation_Data->ResponseCode);
            printf("Phonebook Size: %d\r\n", PBAPEventData->Event_Data.PBAP_Pull_vCard_Listing_Confirmation_Data->PhonebookSize);
            printf("NewMissedCalls: %d\r\n", PBAPEventData->Event_Data.PBAP_Pull_vCard_Listing_Confirmation_Data->NewMissedCalls);
            printf("Final: %d\r\n", PBAPEventData->Event_Data.PBAP_Pull_vCard_Listing_Confirmation_Data->Final);
            printf("Data Length: %u\r\n", PBAPEventData->Event_Data.PBAP_Pull_vCard_Listing_Confirmation_Data->BufferSize);

            if(PBAPEventData->Event_Data.PBAP_Pull_vCard_Listing_Confirmation_Data->BufferSize)
            {
               printf("Data:\r\n");

               for(Index=0,Buffer[0]='\0';(Index<PBAPEventData->Event_Data.PBAP_Pull_vCard_Listing_Confirmation_Data->BufferSize) && (Index < (sizeof(Buffer) - 1));Index++)
                  sprintf(&(Buffer[strlen(Buffer)]), "%c", PBAPEventData->Event_Data.PBAP_Pull_vCard_Listing_Confirmation_Data->Buffer[Index]);

               printf(Buffer);
               printf("\r\n");
            }

            /* If this is a successful GET then pass to routine to      */
            /* handle file I/O and further requests (if required).      */
            if(((PBAPEventData->Event_Data.PBAP_Pull_vCard_Listing_Confirmation_Data->ResponseCode == PBAP_OBEX_RESPONSE_CONTINUE) || (PBAPEventData->Event_Data.PBAP_Pull_vCard_Listing_Confirmation_Data->ResponseCode == PBAP_OBEX_RESPONSE_OK)) && (PBAPEventData->Event_Data.PBAP_Pull_vCard_Listing_Confirmation_Data->BufferSize))
               WriteReceivedData(PBAPEventData->Event_Data.PBAP_Pull_vCard_Listing_Confirmation_Data->BufferSize, PBAPEventData->Event_Data.PBAP_Pull_vCard_Listing_Confirmation_Data->Buffer);

            /* Determine if we need to issue another request to continue*/
            /* the operation.                                           */
            if(!PBAPEventData->Event_Data.PBAP_Pull_vCard_Listing_Confirmation_Data->Final)
            {
               /* printf Results.                                       */
               printf("Pull vCard Listing not complete, Requesting more data.\r\n");

               /* Continue the Pull vCard Listing request.  Note that   */
               /* most parameters passed are ignored on a continuation  */
               /* call.                                                 */
               Result = PBAP_Pull_vCard_Listing_Request(BluetoothStackID, PBAPID, NULL, loDefault, saDefault, NULL, PBAP_MAX_LIST_COUNT_NOT_RESTRICTED, 0);

               if(Result >= 0)
               {
                  printf("PBAP_Pull_vCard_Listing_Request() Successful.\r\n");

                  CurrentOperation = coPullvCardListing;
               }
               else
               {
                  printf("PBAP_Pull_vCard_Listing_Request() Failure %d.\r\n", Result);

                  CurrentOperation = coNone;
               }
            }
            else
            {
               /* Set current operation.                                */
               CurrentOperation = coNone;

               printf("Pull vCard Listing Completed.\r\n");
            }
            break;
         case etPBAP_Pull_vCard_Entry_Confirmation:
            printf("Pull vCard Entry Confirmation\r\n");

            printf("Response Code: 0x%02X\r\n", PBAPEventData->Event_Data.PBAP_Pull_vCard_Entry_Confirmation_Data->ResponseCode);
            printf("Final: %d\r\n", PBAPEventData->Event_Data.PBAP_Pull_vCard_Entry_Confirmation_Data->Final);
            printf("Data Length: %u\r\n", PBAPEventData->Event_Data.PBAP_Pull_vCard_Entry_Confirmation_Data->BufferSize);

            if(PBAPEventData->Event_Data.PBAP_Pull_vCard_Entry_Confirmation_Data->BufferSize)
            {
               printf("Data:\r\n");

               for(Index=0,Buffer[0]='\0';(Index<PBAPEventData->Event_Data.PBAP_Pull_vCard_Entry_Confirmation_Data->BufferSize) && (Index < (sizeof(Buffer) - 1));Index++)
                  sprintf(&(Buffer[strlen(Buffer)]), "%c", PBAPEventData->Event_Data.PBAP_Pull_vCard_Entry_Confirmation_Data->Buffer[Index]);

               printf(Buffer);
               printf("\r\n");
            }

            /* If this is a successful GET then pass to routine to      */
            /* handle file I/O and further requests (if required).      */
            if((PBAPEventData->Event_Data.PBAP_Pull_vCard_Entry_Confirmation_Data->ResponseCode == PBAP_OBEX_RESPONSE_CONTINUE) || (PBAPEventData->Event_Data.PBAP_Pull_vCard_Entry_Confirmation_Data->ResponseCode == PBAP_OBEX_RESPONSE_OK))
               WriteReceivedData(PBAPEventData->Event_Data.PBAP_Pull_vCard_Entry_Confirmation_Data->BufferSize, PBAPEventData->Event_Data.PBAP_Pull_vCard_Entry_Confirmation_Data->Buffer);

            /* Determine if we need to issue another request to continue*/
            /* the operation.                                           */
            if(!PBAPEventData->Event_Data.PBAP_Pull_vCard_Entry_Confirmation_Data->Final)
            {
               /* printf Results.                                       */
               printf("Pull vCard Entry not complete, Requesting more data.\r\n");

               /* Continue the Pull Phonebook request.  Note that most  */
               /* parameters passed are ignored on a continuation call. */
               Result = PBAP_Pull_vCard_Entry_Request(BluetoothStackID, PBAPID, NULL, 0, 0, pfDefault);

               if(Result >= 0)
               {
                  printf("PBAP_Pull_vCard_Entry_Request() Successful.\r\n");

                  CurrentOperation = coPullvCardEntry;
               }
               else
               {
                  printf("PBAP_Pull_vCard_Entry_Request() Failure %d.\r\n", Result);

                  CurrentOperation = coNone;
               }
            }
            else
            {
               /* Set current operation.                                */
               CurrentOperation = coNone;

               printf("Pull vCard Entry Completed.\r\n");
            }
            break;
         default:
            printf("Unexpected (Unhandled) Event %d.\r\n", PBAPEventData->Event_Data_Type);

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

