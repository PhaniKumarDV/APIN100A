/*****< linuxhcr.c >***********************************************************/
/*      Copyright 2003 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  LINUXHCR - Simple Linux application using Hardcopy Cable Replacement      */
/*             Profile.                                                       */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   02/27/03  D. Lange        Initial creation.                              */
/******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "LinuxHCR.h"      /* Main Application Prototypes and Constants.      */

#include "SS1BTPS.h"       /* Includes for the SS1 Bluetooth Protocol Stack.  */
#include "SS1BTHCR.h"      /* Includes for the SS1 HCR Profile.               */
#include "SS1BTDBG.h"      /* Includes/Constants for Bluetooth Debugging.     */

#define LOCAL_NAME_ROOT                      "LinuxHCR"  /* Root of the local */
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

#define MAX_NUM_OF_PARAMETERS                       (6)  /* Denotes the max   */
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

#define HCR_SAMPLE_1284_ID_STRING   "Sample HCR Server 1284 ID String."
                                                         /* Sample 1284 ID    */
                                                         /* String to use when*/
                                                         /* Registering the   */
                                                         /* HCR Server SDP    */
                                                         /* Record.           */

#define HCR_SAMPLE_DATA             "Sample HCR Data."   /* Sample HCR Data   */
                                                         /* that is sent as   */
                                                         /* precanned data    */
                                                         /* when a Data Write */
                                                         /* is performed.     */
                                                         /* This is for       */
                                                         /* demonstration     */
                                                         /* purposes only.    */
                                                         /* Normally this     */
                                                         /* data would be     */
                                                         /* Binary Printer    */
                                                         /* Data (and it would*/
                                                         /* vary obviously).  */

#define INDENT_LENGTH                               (3)  /* Denotes the number*/
                                                         /* of character      */
                                                         /* spaces to be used */
                                                         /* for indenting when*/
                                                         /* displaying SDP    */
                                                         /* Data Elements.    */

   /* The following constants represent the default log file names that */
   /* are used if no Log file name is specified when enabling debug.    */
#define DEFAULT_DEBUG_LOG_FILE_NAME  "LinuxHCR_ASC.log"
#define DEFAULT_DEBUG_FTS_FILE_NAME  "LinuxHCR_FTS.log"

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

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the         */
   /* compiler as part of standard C/C++).                              */
static unsigned int        BluetoothStackID;        /* Variable which holds the Handle */
                                                    /* of the opened Bluetooth Protocol*/
                                                    /* Stack.                          */

static unsigned int        DebugID;                 /* Variable which holds the ID of  */
                                                    /* the currently enabled debugging */
                                                    /* session.                        */

static unsigned int        HCRID;                   /* Variable which holds the Handle */
                                                    /* of the active HCR Client or HCR */
                                                    /* Server Connection.              */

static unsigned int        NotificationID;          /* Variable which holds the Handle */
                                                    /* of the active Notification      */
                                                    /* Client or Notification Server   */
                                                    /* Connection.                     */

static DWord_t             HCRServerSDPHandle;      /* Variable which holds the Handle */
                                                    /* of the HCR Server SDP Service   */
                                                    /* Record.                         */

static DWord_t             NotificationServerSDPHandle; /* Variable which holds the    */
                                                    /* Handle of the Notification      */
                                                    /* Server SDP Service Record.      */

static int                 IsClient;                /* Variable used to indicate if the*/
                                                    /* program is to be run in HCR     */
                                                    /* Client Mode (TRUE) or HCR       */
                                                    /* Server Mode (FALSE).            */

static BD_ADDR_t           InquiryResultList[MAX_INQUIRY_RESULTS]; /* Variable which   */
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
static void DisplayHCRClientMenu(void);
static void DisplayHCRServerMenu(void);
static void PopulateHCRClientCommandTable(void);
static void PopulateHCRServerCommandTable(void);

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

static int InitializeHCRClient(void);
static int InitializeHCRServer(void);

static int OpenStack(HCI_DriverInformation_t *HCI_DriverInformation);
static int CloseStack(void);

static int SetDisc(void);
static int SetConnect(void);
static int SetPairable(void);
static int DeleteLinkKey(BD_ADDR_t BD_ADDR);

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

static int RegisterHCRServer(ParameterList_t *TempParam);
static int UnRegisterHCRServer(ParameterList_t *TempParam);
static int RegisterNotificationServer(ParameterList_t *TempParam);
static int UnRegisterNotificationServer(ParameterList_t *TempParam);

static int ConnectRemoteHCRServer(ParameterList_t *TempParam);
static int ConnectRemoteNotificationServer(ParameterList_t *TempParam);
static int CloseHCRConnection(ParameterList_t *TempParam);
static int CloseNotificationConnection(ParameterList_t *TempParam);

static int OpenDataChannel(ParameterList_t *TempParam);
static int PerformDataWrite(ParameterList_t *TempParam);
static int PerformCreditGrantRequest(ParameterList_t *TempParam);
static int PerformCreditGrantReply(ParameterList_t *TempParam);
static int PerformCreditRequestRequest(ParameterList_t *TempParam);
static int PerformCreditRequestReply(ParameterList_t *TempParam);
static int PerformCreditQueryRequest(ParameterList_t *TempParam);
static int PerformCreditQueryReply(ParameterList_t *TempParam);
static int PerformLPTStatusRequest(ParameterList_t *TempParam);
static int PerformLPTStatusReply(ParameterList_t *TempParam);
static int Perform1284IDRequest(ParameterList_t *TempParam);
static int Perform1284IDReply(ParameterList_t *TempParam);
static int PerformSoftResetRequest(ParameterList_t *TempParam);
static int PerformSoftResetReply(ParameterList_t *TempParam);
static int PerformRegisterNotificationRequest(ParameterList_t *TempParam);
static int PerformRegisterNotificationReply(ParameterList_t *TempParam);
static int PerformNotificationRequest(ParameterList_t *TempParam);

static void DisplaySDPAttributeResponse(SDP_Service_Attribute_Response_Data_t *SDPServiceAttributeResponse, unsigned int InitLevel);
static void DisplaySDPSearchAttributeResponse(SDP_Service_Search_Attribute_Response_Data_t *SDPServiceSearchAttributeResponse);
static void DisplayDataElement(SDP_Data_Element_t *SDPDataElement, unsigned int Level);

   /* Callback Function Prototypes.                                     */
static void BTPSAPI GAP_Event_Callback(unsigned int BluetoothStackID, GAP_Event_Data_t *GAPEventData, unsigned long CallbackParameter);
static void BTPSAPI SDP_Event_Callback(unsigned int BluetoothStackID, unsigned int SDPRequestID, SDP_Response_Data_t *SDP_Response_Data, unsigned long CallbackParameter);
static void BTPSAPI HCR_Event_Callback(unsigned int BluetoothStackID, HCR_Event_Data_t *HCREventData, unsigned long CallbackParameter);

   /* The following function displays the HCR Client Command Menu.      */
static void DisplayHCRClientMenu(void)
{
   /* First display the upper command bar.                              */
   printf("************************** Command Options **************************\r\n");

   /* Next, display all of the commands.                                */
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
   printf("*  RegisterNotificationServer [Notification Channel]                *\r\n");
   printf("*  UnRegisterNotificationServer                                     *\r\n");
   printf("*  ConnectRemoteHCRServer [Inquiry Index] [Control Channel]         *\r\n");
   printf("*  CloseHCRClientConnection                                         *\r\n");
   printf("*  CloseNotificationConnection                                      *\r\n");
   printf("*  OpenDataChannel [Data Channel]                                   *\r\n");
   printf("*  CreditGrantRequest [Credit]                                      *\r\n");
   printf("*  CreditRequestRequest                                             *\r\n");
   printf("*  CreditQueryRequest                                               *\r\n");
   printf("*  LPTStatusRequest                                                 *\r\n");
   printf("*  Get1284IDRequest [Starting Index]                                *\r\n");
   printf("*  SoftResetRequest                                                 *\r\n");
   printf("*  RegisterNotificationRequest [Callback ID] [Callback Timeout]     *\r\n");
   printf("*  DataWrite                                                        *\r\n");
   printf("*  EnableDebug [Enable/Disable] [Log Type] [Log File Name]          *\r\n");
   printf("*  Help                                                             *\r\n");
   printf("*  Quit                                                             *\r\n");

   printf("*********************************************************************\r\n");
}

   /* The following function displays the HCR Server Command Menu.      */
static void DisplayHCRServerMenu(void)
{
   /* First display the upper command bar.                              */
   printf("************************** Command Options **************************\r\n");

   /* Next, display all of the commands.                                */
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
   printf("*  RegisterHCRServer [Control Channel] [Data Channel]               *\r\n");
   printf("*  UnRegisterHCRServer                                              *\r\n");
   printf("*  ConnectRemoteNotificationServer [Inquiry Index] [Not. Channel]   *\r\n");
   printf("*  CloseHCRServerConnection                                         *\r\n");
   printf("*  CloseNotificationConnection                                      *\r\n");
   printf("*  CreditGrantReply                                                 *\r\n");
   printf("*  CreditRequestReply [Credit]                                      *\r\n");
   printf("*  CreditQueryReply                                                 *\r\n");
   printf("*  LPTStatusReply [LPT Status Byte]                                 *\r\n");
   printf("*  Get1284IDReply                                                   *\r\n");
   printf("*  SoftResetReply                                                   *\r\n");
   printf("*  RegisterNotificationReply [Not. Timeout] [Callback Timeout]      *\r\n");
   printf("*  SendNotification [Callback ID]                                   *\r\n");
   printf("*  DataWrite                                                        *\r\n");
   printf("*  EnableDebug [Enable/Disable] [Log Type] [Log File Name]          *\r\n");
   printf("*  Help                                                             *\r\n");
   printf("*  Quit                                                             *\r\n");

   printf("*********************************************************************\r\n");
}

   /* The following function Clears all commands currently in the       */
   /* command table and populates it with the Commands that are         */
   /* available for the HCR Client.                                     */
static void PopulateHCRClientCommandTable(void)
{
   /* First Clear all of the commands in the Command Table.             */
   ClearCommands();

   /* Now add all of the commands that are associated with the HCR      */
   /* Client to the Command Table.                                      */
   AddCommand("REGISTERNOTIFICATIONSERVER", RegisterNotificationServer);
   AddCommand("UNREGISTERNOTIFICATIONSERVER", UnRegisterNotificationServer);
   AddCommand("CONNECTREMOTEHCRSERVER", ConnectRemoteHCRServer);
   AddCommand("CLOSEHCRCLIENTCONNECTION", CloseHCRConnection);
   AddCommand("CLOSENOTIFICATIONCONNECTION", CloseNotificationConnection);
   AddCommand("OPENDATACHANNEL", OpenDataChannel);
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
   AddCommand("CREDITGRANTREQUEST", PerformCreditGrantRequest);
   AddCommand("CREDITREQUESTREQUEST", PerformCreditRequestRequest);
   AddCommand("CREDITQUERYREQUEST", PerformCreditQueryRequest);
   AddCommand("LPTSTATUSREQUEST", PerformLPTStatusRequest);
   AddCommand("GET1284IDREQUEST", Perform1284IDRequest);
   AddCommand("SOFTRESETREQUEST", PerformSoftResetRequest);
   AddCommand("REGISTERNOTIFICATIONREQUEST", PerformRegisterNotificationRequest);
   AddCommand("DATAWRITE", PerformDataWrite);
   AddCommand("ENABLEDEBUG", EnableDebug);
   AddCommand("HELP", DisplayHelp);
}

   /* The following function Clears all commands currently in the       */
   /* command table and populates it with the Commands that are         */
   /* available for the HCR Server.                                     */
static void PopulateHCRServerCommandTable(void)
{
   /* First Clear all of the commands in the Command Table.             */
   ClearCommands();

   /* Now add all of the commands that are associated with the HCR      */
   /* Server to the Command Table.                                      */
   AddCommand("REGISTERHCRSERVER", RegisterHCRServer);
   AddCommand("UNREGISTERHCRSERVER", UnRegisterHCRServer);
   AddCommand("CONNECTREMOTENOTIFICATIONSERVER", ConnectRemoteNotificationServer);
   AddCommand("CLOSEHCRSERVERCONNECTION", CloseHCRConnection);
   AddCommand("CLOSENOTIFICATIONCONNECTION", CloseNotificationConnection);
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
   AddCommand("CREDITGRANTREPLY", PerformCreditGrantReply);
   AddCommand("CREDITREQUESTREPLY", PerformCreditRequestReply);
   AddCommand("CREDITQUERYREPLY", PerformCreditQueryReply);
   AddCommand("LPTSTATUSREPLY", PerformLPTStatusReply);
   AddCommand("GET1284IDREPLY", Perform1284IDReply);
   AddCommand("SOFTRESETREPLY", PerformSoftResetReply);
   AddCommand("REGISTERNOTIFICATIONREPLY", PerformRegisterNotificationReply);
   AddCommand("SENDNOTIFICATION", PerformNotificationRequest);
   AddCommand("DATAWRITE", PerformDataWrite);
   AddCommand("ENABLEDEBUG", EnableDebug);
   AddCommand("HELP", DisplayHelp);
}

   /* This function is responsible for taking the users input and do the*/
   /* appropriate thing with it.  First, this function get a string of  */
   /* user input, parse the user input in to command and parameters, and*/
   /* finally executing the command or display an error message if the  */
   /* input is corrupt.                                                 */
static void UserInterface_Main(void)
{
   UserCommand_t TempCommand;
   int  Result = !EXIT_CODE;
   char UserInput[MAX_COMMAND_LENGTH];

   /* Determine if we are currently running in HCR Server or HCR Client */
   /* Mode.                                                             */
   if(IsClient)
   {
      /* We are currently running in HCR Client Mode, add the           */
      /* appropriate commands to the command table and display the menu */
      /* options.                                                       */
      PopulateHCRClientCommandTable();

      DisplayHCRClientMenu();
   }
   else
   {
      /* We are not currently running in HCR Client Mode therefore we   */
      /* must be running in HCR Server Mode.  Add the appropriate       */
      /* commands to teh command table and display the menu options.    */
      PopulateHCRServerCommandTable();

      DisplayHCRServerMenu();
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
      printf("HCR>");
      fflush(stdout);

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
               }
            }
            else
               printf("Invalid Input.\r\n");
         }
      }
      else
         Result = EXIT_CODE;
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

   /* The following function is responsible for redisplaying the Menu   */
   /* options to the user.  This function returns zero on successful    */
   /* execution or a negative value on all errors.                      */
static int DisplayHelp(ParameterList_t *TempParam)
{
   /* First check to if Currently in HCR Client or HCR Server Mode.     */
   if(IsClient)
   {
      /* Currently in HCR Client Mode, display the HCR Client Menu.     */
      DisplayHCRClientMenu();
   }
   else
   {
      /* Currently in HCR Server Mode, display the HCR Server Menu.     */
      DisplayHCRServerMenu();
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

   /* The following function is responsible for setting the initial     */
   /* state of the Main Application to be a HCR Client.  This function  */
   /* returns zero on successful execution and a negative value on all  */
   /* errors.                                                           */
static int InitializeHCRClient(void)
{
   int ret_val;

   /* The program is currently running in HCR Client Mode.  Ready the   */
   /* application to support a Notification Server.  First, attempt to  */
   /* set the device to be connectable.                                 */
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
            /* The device is now connectable and discoverable so return */
            /* success.                                                 */
            ret_val = 0;
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
   /* state of the Main Application to be a HCR Server.  This function  */
   /* returns zero on successful execution and a negative value on all  */
   /* errors.                                                           */
static int InitializeHCRServer(void)
{
   int ret_val;

   /* The program is currently running in HCR Server Mode.  Ready the   */
   /* application to support a HCR Server.  First, attempt to set the   */
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
            /* The device is now connectable and discoverable return    */
            /* success to the caller.                                   */
            ret_val = 0;
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
               printf("Stack Initialization on USB Successful.\r\n");
            else
               printf("Stack Initialization on Port %d %ld (%s) Successful.\r\n", HCI_DriverInformation->DriverInformation.COMMDriverInformation.COMPortNumber, HCI_DriverInformation->DriverInformation.COMMDriverInformation.BaudRate, ((HCI_DriverInformation->DriverInformation.COMMDriverInformation.Protocol == cpBCSP)?"BCSP":"UART"));

            BluetoothStackID = Result;

            DebugID          = 0;

            ret_val          = 0;

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
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].intParam) && (NumberofValidResponses) && (TempParam->Params[0].intParam <= NumberofValidResponses) && (!COMPARE_BD_ADDR(InquiryResultList[(TempParam->Params[0].intParam - 1)], NullADDR)))
      {
         /* Check to see if General Bonding was specified.              */
         if(TempParam->NumberofParameters > 1)
            BondingType = TempParam->Params[1].intParam?btGeneral:btDedicated;
         else
            BondingType = btDedicated;

         /* Before we submit the command to the stack, we need to make  */
         /* sure that we clear out any Link Key we have stored for the  */
         /* specified device.                                           */
         DeleteLinkKey(InquiryResultList[(TempParam->Params[0].intParam - 1)]);

         /* Attempt to submit the command.                              */
         Result = GAP_Initiate_Bonding(BluetoothStackID, InquiryResultList[(TempParam->Params[0].intParam - 1)], BondingType, GAP_Event_Callback, (unsigned long)0);

         /* Check the return value of the submitted command for success.*/
         if(!Result)
         {
            /* Display a messsage indicating that Bonding was initiated */
            /* successfully.                                            */
            printf("GAP_Initiate_Bonding (%s): Function Successful.\r\n", (BondingType == btDedicated)?"Dedicated":"General");

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            /* Display a message indicating that an error occured while */
            /* initiating bonding.                                      */
            printf("GAP_Initiate_Bonding() Failure: %d.\r\n", Result);

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: Pair [Inquiry Index] [Bonding Type (0 = Dedicated, 1 = General) (optional).\r\n");

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

   /* The following function is responsible for registering a HCR       */
   /* Server.  This function returns zero on successful execution and a */
   /* negative value on all errors.                                     */
static int RegisterHCRServer(ParameterList_t *TempParam)
{
   int                           Result;
   int                           ret_val;
   HCR_SDP_1284_ID_Information_t HCR_SDP_1284_ID_Information;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that a Server isn't already registered. */
      if((!IsClient) && (!HCRID))
      {
         /* No server is currently registered.                          */

         /* Make sure that the parameters required for this function    */
         /* appear to be valid.                                         */
         if((TempParam) && (TempParam->NumberofParameters >= 2) && (TempParam->Params[0].intParam) && (TempParam->Params[1].intParam))
         {
            /* Now attempt to register a Bluetooth HCR server.          */
            Result = HCR_Open_Server(BluetoothStackID, TempParam->Params[0].intParam, TempParam->Params[1].intParam, stPrinter, HCR_Event_Callback, 0);

            if(Result > 0)
            {
               /* The server was successfully registered, note the      */
               /* Server HCR ID.                                        */
               HCRID = Result;

               printf("HCR_Open_Server: Function Successful (Control: 0x%04X, Data: 0x%04X).\r\n", TempParam->Params[0].intParam, TempParam->Params[1].intParam);

               /* Next, let's register a HCR Server SDP Record.         */

               /* Format a sample 1284 ID String.                       */
               HCR_SDP_1284_ID_Information.IEEE_1284IDLength = (Word_t)strlen(HCR_SAMPLE_1284_ID_STRING);
               HCR_SDP_1284_ID_Information.IEEE_1284IDString = (Byte_t *)HCR_SAMPLE_1284_ID_STRING;

               Result = HCR_Register_Server_SDP_Record(BluetoothStackID, HCRID, NULL, "HCR Server Service", &HCR_SDP_1284_ID_Information, "HCR Server Device", "HCR Server", NULL, &HCRServerSDPHandle);

               if(!Result)
                  printf("HCR_Register_Server_SDP_Record: Success.\r\n");
               else
                  printf("HCR_Register_Server_SDP_Record: Failure: %d.\r\n", Result);

               /* Flag the the function was successful.                 */
               ret_val = 0;
            }
            else
            {
               /* An error occurred while attempting to register the    */
               /* server.  Display an error message indicating the      */
               /* error.                                                */
               printf("HCR_Open_Server: Function Failure: %d.\r\n", Result);

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* Invalid parameters, so inform the user.                  */
            printf("Usage: RegisterHCRServer [Control Channel] [Data Channel].\r\n");

            /* One or more of the necessary parameters is/are invalid.  */
            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* A Server is already registered.                             */
         if(HCRID)
            printf("HCR_Open_Server: Server Already Registered.\r\n");
         else
            printf("HCR_Open_Server: Not in HCR Server Mode.\r\n");

         /* One or more of the necessary parameters is/are invalid.     */
         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is responsible for un-registering a        */
   /* previously registered HCR Server.  This function returns zero on  */
   /* successful execution and a negative value on all errors.          */
static int UnRegisterHCRServer(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that a Server is currently registered.  */
      if((!IsClient) && (HCRID))
      {
         /* A server is currently registered, unregister the server.    */
         Result = HCR_Close_Server(BluetoothStackID, HCRID);

         /* Next check the return value of the issued command see if it */
         /* was successful.                                             */
         if(!Result)
         {
            /* Display a message indicating that the server was         */
            /* successfully unregistered.                               */
            HCRID = 0;

            printf("HCR_Close_Server: Function Successful.\r\n");

            /* Next, let's delete any registered HCR Server SDP Record. */
            if(HCRServerSDPHandle)
               SDP_Delete_Service_Record(BluetoothStackID, HCRServerSDPHandle);

            /* Flag that there is no registered HCR Server SDP Record.  */
            HCRServerSDPHandle = 0;

            /* Flag the the function was successful.                    */
            ret_val            = 0;
         }
         else
         {
            /* An error occurred while attempting to unregister the     */
            /* server.                                                  */
            printf("HCR_Close_Server: Function Failure: %d.\r\n", Result);

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* A Server is NOT registered.                                 */
         if(HCRID)
            printf("HCR_Close_Server: No Server is Registered.\r\n");
         else
            printf("HCR_Close_Server: Not in HCR Server Mode.\r\n");

         /* One or more of the necessary parameters is/are invalid.     */
         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is responsible for registering a           */
   /* Notification Server.  This function returns zero on successful    */
   /* execution and a negative value on all errors.                     */
static int RegisterNotificationServer(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that a Server isn't already registered. */
      if((IsClient) && (!NotificationID))
      {
         /* No server is currently registered.                          */

         /* Make sure that the parameters required for this function    */
         /* appear to be valid.                                         */
         if((TempParam) && (TempParam->NumberofParameters >= 1) && (TempParam->Params[0].intParam))
         {
            /* Now attempt to register a Bluetooth HCR server.          */
            Result = HCR_Open_Notification_Server(BluetoothStackID, TempParam->Params[0].intParam, stPrinter, HCR_Event_Callback, 0);

            if(Result > 0)
            {
               /* The server was successfully registered, note the      */
               /* Notification Server ID.                               */
               NotificationID = Result;

               printf("HCR_Open_Notification_Server: Function Successful (Notification: 0x%04X).\r\n", TempParam->Params[0].intParam);

               /* Next, let's register an Notification Server SDP       */
               /* Record.                                               */
               Result = HCR_Register_Notification_Server_SDP_Record(BluetoothStackID, NotificationID, "Notification Service", &NotificationServerSDPHandle);

               if(!Result)
                  printf("HCR_Register_Notification_Server_SDP_Record: Success.\r\n");
               else
                  printf("HCR_Register_Notification_Server_SDP_Record: Failure: %d.\r\n", Result);

               /* Flag the the function was successful.                 */
               ret_val = 0;
            }
            else
            {
               /* An error occurred while attempting to register the    */
               /* server.  Display an error message indicating the      */
               /* error.                                                */
               printf("HCR_Open_Notification_Server: Function Failure: %d.\r\n", Result);

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* Invalid parameters, so inform the user.                  */
            printf("Usage: RegisterNotificationServer [Notification Channel].\r\n");

            /* One or more of the necessary parameters is/are invalid.  */
            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* A Server is NOT registered.                                 */
         if(NotificationID)
            printf("HCR_Open_Notification_Server: Server Already Registered.\r\n");
         else
            printf("HCR_Open_Notification_Server: Not in HCR Client Mode.\r\n");

         /* One or more of the necessary parameters is/are invalid.     */
         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is responsible for un-registering a        */
   /* previously registered Notification Server.  This function returns */
   /* zero on successful execution and a negative value on all errors.  */
static int UnRegisterNotificationServer(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that a Server is currently registered.  */
      if((IsClient) && (NotificationID))
      {
         /* A server is currently registered, unregister the server.    */
         Result = HCR_Close_Server(BluetoothStackID, NotificationID);

         /* Next check the return value of the issued command see if it */
         /* was successful.                                             */
         if(!Result)
         {
            /* Display a message indicating that the server was         */
            /* successfully unregistered.                               */
            NotificationID = 0;

            printf("HCR_Close_Notification_Server: Function Successful.\r\n");

            /* Next, let's delete any registered Notification Server SDP*/
            /* Record.                                                  */
            if(NotificationServerSDPHandle)
               SDP_Delete_Service_Record(BluetoothStackID, NotificationServerSDPHandle);

            /* Flag that there is no registered Notification Server SDP */
            /* Record.                                                  */
            NotificationServerSDPHandle = 0;

            /* Flag the the function was successful.                    */
            ret_val                     = 0;
         }
         else
         {
            /* An error occurred while attempting to unregister the     */
            /* server.                                                  */
            printf("HCR_Close_Notification_Server: Function Failure: %d.\r\n", Result);

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* A Server is NOT registered.                                 */
         if(HCRID)
            printf("HCR_Close_Notification_Server: No Server Registered.\r\n");
         else
            printf("HCR_Close_Notification_Server: Not in HCR Client Mode.\r\n");

         /* One or more of the necessary parameters is/are invalid.     */
         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is responsible for connecting to a Remote  */
   /* HCR Server.  This function returns zero on successful execution   */
   /* and a negative value on all errors.                               */
static int ConnectRemoteHCRServer(ParameterList_t *TempParam)
{
   int       ret_val;
   int       Result;
   BD_ADDR_t NullADDR;

   ASSIGN_BD_ADDR(NullADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Next, make sure that we do not already have a connection to a  */
      /* Remote HCR Server already.                                     */
      if((IsClient) && (!HCRID))
      {
         /* Make sure that the parameters required for this function    */
         /* appear to be valid.                                         */
         if((TempParam) && (TempParam->NumberofParameters >= 2) && (TempParam->Params[0].intParam) && (TempParam->Params[1].intParam) && (TempParam->Params[0].intParam <= NumberofValidResponses)  && (!COMPARE_BD_ADDR(InquiryResultList[(TempParam->Params[0].intParam-1)], NullADDR)))
         {
            Result = HCR_Open_Remote_Server(BluetoothStackID, InquiryResultList[(TempParam->Params[0].intParam-1)], TempParam->Params[1].intParam, HCR_Event_Callback, 0);

            /* Now check to see if the function call was successfully   */
            /* made.                                                    */
            if(Result > 0)
            {
               /* The Connect Request was successfully submitted.  The  */
               /* return value of the call is the HCR ID and is required*/
               /* for future HCR calls.                                 */
               HCRID = Result;

               printf("HCR_Open_Remote_Server: Function Successful (ID = %04X).\r\n", Result);

               /* Flag the the function was successful.                 */
               ret_val = 0;
            }
            else
            {
               /* There was an error submitting the connection request. */
               printf("HCR_Open_Remote_Server: Function Failure: %d.\r\n", Result);

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* Invalid parameters, so inform the user.                  */
            printf("Usage: ConnectRemoteHCRServer [Inquiry Index] [Control Channel].\r\n");

            /* One or more of the necessary parameters is/are invalid.  */
            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* There already exists an active HCR Client connection.       */
         /* * NOTE * This is a limitation of this test application to   */
         /*          only support a single HCR Client NOT the Profile   */
         /*          itself.                                            */
         if(HCRID)
            printf("HCR_Open_Remote_Server: Client Connection already exists.\r\n");
         else
            printf("HCR_Open_Remote_Server: Not in HCR Client Mode.\r\n");

         /* One or more of the necessary parameters is/are invalid.     */
         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is responsible for connecting to a Remote  */
   /* Notification Server.  This function returns zero on successful    */
   /* execution and a negative value on all errors.                     */
static int ConnectRemoteNotificationServer(ParameterList_t *TempParam)
{
   int       ret_val;
   int       Result;
   BD_ADDR_t NullADDR;

   ASSIGN_BD_ADDR(NullADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Next, make sure that we do not already have a connection to a  */
      /* Remote HCR Server already.                                     */
      if((!IsClient) && (!NotificationID))
      {
         /* Make sure that the parameters required for this function    */
         /* appear to be valid.                                         */
         if((TempParam) && (TempParam->NumberofParameters >= 2) && (TempParam->Params[0].intParam) && (TempParam->Params[1].intParam) && (TempParam->Params[0].intParam <= NumberofValidResponses)  && (!COMPARE_BD_ADDR(InquiryResultList[(TempParam->Params[0].intParam-1)], NullADDR)))
         {
            Result = HCR_Open_Remote_Notification_Server(BluetoothStackID, InquiryResultList[(TempParam->Params[0].intParam-1)], TempParam->Params[1].intParam, HCR_Event_Callback, 0);

            /* Now check to see if the function call was successfully   */
            /* made.                                                    */
            if(Result > 0)
            {
               /* The Connect Request was successfully submitted.  The  */
               /* return value of the call is the Notification ID and is*/
               /* required for future Notification calls.               */
               NotificationID = Result;

               printf("HCR_Open_Remote_Notification_Server: Function Successful (ID = %04X).\r\n", Result);

               /* Flag the the function was successful.                 */
               ret_val = 0;
            }
            else
            {
               /* There was an error submitting the connection request. */
               printf("HCR_Open_Remote_Notification_Server: Function Failure: %d.\r\n", Result);

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* Invalid parameters, so inform the user.                  */
            printf("Usage: ConnectRemoteNotificationServer [Inquiry Index] [Notification Channel].\r\n");

            /* One or more of the necessary parameters is/are invalid.  */
            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* There already exists an active HCR Client connection.       */
         /* * NOTE * This is a limitation of this test application to   */
         /*          only support a single Notification Client NOT the  */
         /*          Profile itself.                                    */
         if(NotificationID)
            printf("HCR_Open_Remote_Notification_Server: Notification Client Connection already exists.\r\n");
         else
            printf("HCR_Open_Remote_Notification_Server: Not in HCR Server Mode.\r\n");

         /* One or more of the necessary parameters is/are invalid.     */
         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is responsible for closing a connection to */
   /* a Remote HCR Server.  This function returns zero on successful    */
   /* execution and a negative value on all errors.                     */
static int CloseHCRConnection(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Next, make sure that we have a connection to a Remote HCR      */
      /* Client or Server.                                              */
      if(HCRID)
      {
         /* The HCR ID appears to be semi-valid.  Now try to close the  */
         /* Connection.                                                 */
         Result = HCR_Close(BluetoothStackID, HCRID);

         if(!Result)
         {
            /* The function was called successfully.  Display a message */
            /* indicating that the HCR Client/Server was successfully   */
            /* closed.                                                  */
            printf("HCR_Close: Function Successful.\r\n");

            /* Flag that we no longer have a connection.                */
            if(IsClient)
               HCRID = 0;

            /* Flag the the function was successful.                    */
            ret_val = 0;
         }
         else
         {
            /* An error occurred while attempting to close the          */
            /* Connection.                                              */
            printf("HCR_Close: Function Failure: %d.\r\n", Result);

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* The HCR ID is invalid.                                      */
         printf("HCR_Close: HCR Connection does not exist.\r\n");

         /* One or more of the necessary parameters is/are invalid.     */
         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is responsible for closing a connection to */
   /* a Remote Notification Server.  This function returns zero on      */
   /* successful execution and a negative value on all errors.          */
static int CloseNotificationConnection(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Next, make sure that we have a connection to a Notification    */
      /* Client/Server.                                                 */
      if(NotificationID)
      {
         /* The Notification ID appears to be semi-valid.  Now try to   */
         /* close the Connection.                                       */
         Result = HCR_Close(BluetoothStackID, NotificationID);

         if(!Result)
         {
            /* The function was called successfully.  Display a message */
            /* indicating that the Notification Client/Server was       */
            /* successfully closed.                                     */
            printf("HCR_Close: Function Successful.\r\n");

            /* Flag that we no longer have a connection.                */
            if(!IsClient)
               NotificationID = 0;

            /* Flag the the function was successful.                    */
            ret_val = 0;
         }
         else
         {
            /* An error occurred while attempting to close the          */
            /* Connection.                                              */
            printf("HCR_Close: Function Failure: %d.\r\n", Result);

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* The Notification ID is invalid.                             */
         printf("HCR_Close: Notification Connection does not exist.\r\n");

         /* One or more of the necessary parameters is/are invalid.     */
         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is responsible for establishing a Data     */
   /* Channel Connection on an active HCR Client Connection.  This      */
   /* function returns zero on successful execution and a negative value*/
   /* on all errors.                                                    */
static int OpenDataChannel(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Check to see if a HCR Client has already been established.     */
      if((IsClient) && (HCRID))
      {
         /* Make sure that the parameters required for this function    */
         /* appear to be valid.                                         */
         if((TempParam) && (TempParam->NumberofParameters >= 1) && (TempParam->Params[0].intParam))
         {
            /* The HCR ID and the Data Channel appears to be semi-valid.*/
            /* Now try to open the data channel.                        */
            Result = HCR_Open_Data_Channel(BluetoothStackID, HCRID, TempParam->Params[0].intParam);

            if(!Result)
            {
               /* The function was called successfully.  Display a      */
               /* message indicating that the Data Channel Establishment*/
               /* function was successfully submitted.                  */
               printf("HCR_Open_Data_Channel: Function Successful.\r\n");

               /* Flag the the function was successful.                 */
               ret_val = 0;
            }
            else
            {
               /* An error occurred while attempting to issue the Data  */
               /* Channel Establishment function.                       */
               printf("HCR_Open_Data_Channel: Function Failure: %d.\r\n", Result);

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* Invalid parameters, so inform the user.                  */
            printf("Usage: OpenDataChannel [Data Channel].\r\n");

            /* One or more of the necessary parameters is/are invalid.  */
            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* The HCR ID is invalid.                                      */
         if(!HCRID)
            printf("HCR_Open_Data_Channel: HCR Client Connection does not exist.\r\n");
         else
            printf("HCR_Open_Data_Channel: Not in HCR Client Mode.\r\n");

         /* One or more of the necessary parameters is/are invalid.     */
         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is responsible for sending a sample        */
   /* (precanned) data message over an active HCR Server or HCR Client  */
   /* Connection.  This function returns zero on successful execution   */
   /* and a negative value on all errors.                               */
   /* * NOTE * This function only exists for demonstration purposes     */
   /*          only.  Normally the data that would be sent would be     */
   /*          variable printer data (and would most likely be binary). */
   /*          This simple method is used so that the programmer can    */
   /*          see how the HCR_Data_Write() function is used.           */
static int PerformDataWrite(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Check to see if a HCR Client or HCR Server has already been    */
      /* established.                                                   */
      if(HCRID)
      {
         /* The HCR ID appears to be semi-valid.  Now try to send the   */
         /* specified data.                                             */
         Result = HCR_Data_Write(BluetoothStackID, HCRID, strlen(HCR_SAMPLE_DATA), (Byte_t *)HCR_SAMPLE_DATA);

         if(Result >= 0)
         {
            /* The function was called successfully.  Display a message */
            /* indicating that the Data Write function was successful.  */
            printf("HCR_Data_Write: Function Successful: %d.\r\n", Result);

            /* Flag the the function was successful.                    */
            ret_val = 0;
         }
         else
         {
            /* An error occurred while attempting to issue the Data     */
            /* Channel Establishment function.                          */
            printf("HCR_Data_Write: Function Failure: %d.\r\n", Result);

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* The HCR ID is invalid.                                      */
         printf("HCR_Data_Write: Invalid HCRID.\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is responsible for sending a Credit Grant  */
   /* Request PDU (with the Credits specified in the parameters) to the */
   /* remote HCR Server.  This function returns zero on successful      */
   /* execution and a negative value on all errors.                     */
static int PerformCreditGrantRequest(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Check to see if a HCR Client has already been established.     */
      if((IsClient) && (HCRID))
      {
         /* Make sure that the parameters required for this function    */
         /* appear to be valid.                                         */
         if((TempParam) && (TempParam->NumberofParameters >= 1))
         {
            /* The HCR ID and the parameter (Credit) appears to be      */
            /* semi-valid.  Now try to issue the Credit Grant Request.  */
            Result = HCR_Credit_Grant_Request(BluetoothStackID, HCRID, TempParam->Params[0].intParam);

            if(!Result)
            {
               /* The function was called successfully.  Display a      */
               /* message indicating that the Credit Grant Request      */
               /* function was successfully submitted.                  */
               printf("HCR_Credit_Grant_Request: Function Successful.\r\n");

               /* Flag the the function was successful.                 */
               ret_val = 0;
            }
            else
            {
               /* An error occurred while attempting to issue the Credit*/
               /* Grant Request function.                               */
               printf("HCR_Credit_Grant_Request: Function Failure: %d.\r\n", Result);

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* Invalid parameters, so inform the user.                  */
            printf("Usage: CreditGrantRequest [Credit].\r\n");

            /* One or more of the necessary parameters is/are invalid.  */
            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* The HCR ID is invalid.                                      */
         if(!HCRID)
            printf("HCR_Credit_Grant_Request: HCR Client Connection does not exist.\r\n");
         else
            printf("HCR_Credit_Grant_Request: Not in HCR Client Mode.\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is responsible for sending a Credit Grant  */
   /* Reply PDU (with Successful Status) to the remote HCR Client.  This*/
   /* function returns zero on successful execution and a negative value*/
   /* on all errors.                                                    */
static int PerformCreditGrantReply(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Check to see if a HCR Server has already been established.     */
      if((!IsClient) && (HCRID))
      {
         /* The HCR ID appears to be semi-valid.  Now try to issue the  */
         /* Credit Grant Reply.                                         */
         Result = HCR_Credit_Grant_Reply(BluetoothStackID, HCRID, HCR_PDU_STATUS_SUCCESS);

         if(!Result)
         {
            /* The function was called successfully.  Display a message */
            /* indicating that the Credit Grant Reply function was      */
            /* successfully submitted.                                  */
            printf("HCR_Credit_Grant_Reply: Function Successful.\r\n");

            /* Flag the the function was successful.                    */
            ret_val = 0;
         }
         else
         {
            /* An error occurred while attempting to issue the Credit   */
            /* Grant Reply function.                                    */
            printf("HCR_Credit_Grant_Reply: Function Failure: %d.\r\n", Result);

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* The HCR ID is invalid.                                      */
         if(!HCRID)
            printf("HCR_Credit_Grant_Reply: HCR Server does not exist.\r\n");
         else
            printf("HCR_Credit_Grant_Reply: Not in HCR Server Mode.\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is responsible for sending a Credit Request*/
   /* Request PDU to the remote HCR Server.  This function returns zero */
   /* on successful execution and a negative value on all errors.       */
static int PerformCreditRequestRequest(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Check to see if a HCR Client has already been established.     */
      if((IsClient) && (HCRID))
      {
         /* The HCR ID appears to be semi-valid.  Now try to issue the  */
         /* Credit Request Request.                                     */
         Result = HCR_Credit_Request_Request(BluetoothStackID, HCRID);

         if(!Result)
         {
            /* The function was called successfully.  Display a message */
            /* indicating that the Credit Request Request function was  */
            /* successfully submitted.                                  */
            printf("HCR_Credit_Request_Request: Function Successful.\r\n");

            /* Flag the the function was successful.                    */
            ret_val = 0;
         }
         else
         {
            /* An error occurred while attempting to issue the Credit   */
            /* Request Request function.                                */
            printf("HCR_Credit_Request_Request: Function Failure: %d.\r\n", Result);

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* The HCR ID is invalid.                                      */
         if(!HCRID)
            printf("HCR_Credit_Request_Request: HCR Client Connection does not exist.\r\n");
         else
            printf("HCR_Credit_Request_Request: Not in HCR Client Mode.\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is responsible for sending a Credit Request*/
   /* Reply PDU (with Successful Status and Credits specified in the    */
   /* parameters) to the remote HCR Client.  This function returns zero */
   /* on successful execution and a negative value on all errors.       */
static int PerformCreditRequestReply(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Check to see if a HCR Server has already been established.     */
      if((!IsClient) && (HCRID))
      {
         /* Make sure that the parameters required for this function    */
         /* appear to be valid.                                         */
         if((TempParam) && (TempParam->NumberofParameters >= 1))
         {
            /* The HCR ID and the parameter (Credit) appears to be      */
            /* semi-valid.  Now try to issue the Credit Request Reply.  */
            Result = HCR_Credit_Request_Reply(BluetoothStackID, HCRID, HCR_PDU_STATUS_SUCCESS, TempParam->Params[0].intParam);

            if(!Result)
            {
               /* The function was called successfully.  Display a      */
               /* message indicating that the Credit Request Reply      */
               /* function was successfully submitted.                  */
               printf("HCR_Credit_Request_Reply: Function Successful.\r\n");

               /* Flag the the function was successful.                 */
               ret_val = 0;
            }
            else
            {
               /* An error occurred while attempting to issue the Credit*/
               /* Request Reply function.                               */
               printf("HCR_Credit_Request_Reply: Function Failure: %d.\r\n", Result);

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* Invalid parameters, so inform the user.                  */
            printf("Usage: CreditRequestReply [Credit].\r\n");

            /* One or more of the necessary parameters is/are invalid.  */
            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* The HCR ID is invalid.                                      */
         if(!HCRID)
            printf("HCR_Credit_Request_Reply: HCR Server does not exist.\r\n");
         else
            printf("HCR_Credit_Request_Reply: Not in HCR Server Mode.\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is responsible for sending a Credit Query  */
   /* Request PDU to the remote HCR Server.  This function returns zero */
   /* on successful execution and a negative value on all errors.       */
static int PerformCreditQueryRequest(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Check to see if a HCR Client has already been established.     */
      if((IsClient) && (HCRID))
      {
         /* The HCR ID appears to be semi-valid.  Now try to issue the  */
         /* Credit Query Request.                                       */
         Result = HCR_Credit_Query_Request(BluetoothStackID, HCRID);

         if(!Result)
         {
            /* The function was called successfully.  Display a message */
            /* indicating that the Credit Query Request function was    */
            /* successfully submitted.                                  */
            printf("HCR_Credit_Query_Request: Function Successful.\r\n");

            /* Flag the the function was successful.                    */
            ret_val = 0;
         }
         else
         {
            /* An error occurred while attempting to issue the Credit   */
            /* Query Request function.                                  */
            printf("HCR_Credit_Query_Request: Function Failure: %d.\r\n", Result);

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* The HCR ID is invalid.                                      */
         if(!HCRID)
            printf("HCR_Credit_Query_Request: HCR Client Connection does not exist.\r\n");
         else
            printf("HCR_Credit_Query_Request: Not in HCR Client Mode.\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is responsible for sending a Credit Query  */
   /* Reply PDU (with Successful Status) to the remote HCR Client.  This*/
   /* function returns zero on successful execution and a negative value*/
   /* on all errors.                                                    */
static int PerformCreditQueryReply(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Check to see if a HCR Server has already been established.     */
      if((!IsClient) && (HCRID))
      {
         /* The HCR ID appears to be semi-valid.  Now try to issue the  */
         /* Credit Query Reply.                                         */
         Result = HCR_Credit_Query_Reply(BluetoothStackID, HCRID, HCR_PDU_STATUS_SUCCESS);

         if(!Result)
         {
            /* The function was called successfully.  Display a message */
            /* indicating that the Credit Query Reply function was      */
            /* successfully submitted.                                  */
            printf("HCR_Credit_Query_Reply: Function Successful.\r\n");

            /* Flag the the function was successful.                    */
            ret_val = 0;
         }
         else
         {
            /* An error occurred while attempting to issue the Credit   */
            /* Grant Query function.                                    */
            printf("HCR_Credit_Grant_Reply: Function Failure: %d.\r\n", Result);

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* The HCR ID is invalid.                                      */
         if(!HCRID)
            printf("HCR_Credit_Query_Reply: HCR Server does not exist.\r\n");
         else
            printf("HCR_Credit_Query_Reply: Not in HCR Server Mode.\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is responsible for sending a LPT Status    */
   /* Query Request PDU to the remote HCR Server.  This function returns*/
   /* zero on successful execution and a negative value on all errors.  */
static int PerformLPTStatusRequest(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Check to see if a HCR Client has already been established.     */
      if((IsClient) && (HCRID))
      {
         /* The HCR ID appears to be semi-valid.  Now try to issue the  */
         /* LPT Status Job Status Request.                              */
         Result = HCR_LPT_Status_Query_Request(BluetoothStackID, HCRID);

         if(!Result)
         {
            /* The function was called successfully.  Display a message */
            /* indicating that the LPT Status Query Request function was*/
            /* successfully submitted.                                  */
            printf("HCR_LPT_Status_Query_Request: Function Successful.\r\n");

            /* Flag the the function was successful.                    */
            ret_val = 0;
         }
         else
         {
            /* An error occurred while attempting to issue the LPT      */
            /* Status Query Request function.                           */
            printf("HCR_LPT_Status_Query_Request: Function Failure: %d.\r\n", Result);

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* The HCR ID is invalid.                                      */
         if(!HCRID)
            printf("HCR_LPT_Status_Query_Request: HCR Client Connection does not exist.\r\n");
         else
            printf("HCR_LPT_Status_Query_Request: Not in HCR Client Mode.\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is responsible for sending a LPT Status    */
   /* Reply PDU (with Successful Status and LPT Status specified in the */
   /* parameters) to the remote HCR Client.  This function returns zero */
   /* on successful execution and a negative value on all errors.       */
static int PerformLPTStatusReply(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Check to see if a HCR Server has already been established.     */
      if((!IsClient) && (HCRID))
      {
         /* Make sure that the parameters required for this function    */
         /* appear to be valid.                                         */
         if((TempParam) && (TempParam->NumberofParameters >= 1))
         {
            /* The HCR ID appears to be semi-valid.  Now try to issue   */
            /* the LPT Status Query Reply.                              */
            Result = HCR_LPT_Status_Query_Reply(BluetoothStackID, HCRID, HCR_PDU_STATUS_SUCCESS, (Byte_t)(TempParam->Params[0].intParam));

            if(!Result)
            {
               /* The function was called successfully.  Display a      */
               /* message indicating that the LPT Status Query Reply    */
               /* function was successfully submitted.                  */
               printf("HCR_LPT_Status_Query_Reply: Function Successful.\r\n");

               /* Flag the the function was successful.                 */
               ret_val = 0;
            }
            else
            {
               /* An error occurred while attempting to issue the LPT   */
               /* Status Query Reply function.                          */
               printf("HCR_LPT_Status_Query_Reply: Function Failure: %d.\r\n", Result);

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* Invalid parameters, so inform the user.                  */
            printf("Usage: LPTStatusReply [LPT Status Byte].\r\n");

            /* One or more of the necessary parameters is/are invalid.  */
            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* The HCR ID is invalid.                                      */
         if(!HCRID)
            printf("HCR_LPT_Status_Query_Reply: HCR Server does not exist.\r\n");
         else
            printf("HCR_LPT_Status_Query_Reply: Not in HCR Server Mode.\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is responsible for sending a 1284 ID String*/
   /* Query Request PDU to the remote HCR Server.  This function returns*/
   /* zero on successful execution and a negative value on all errors.  */
static int Perform1284IDRequest(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Check to see if a HCR Client has already been established.     */
      if((IsClient) && (HCRID))
      {
         /* Make sure that the parameters required for this function    */
         /* appear to be valid.                                         */
         if((TempParam) && (TempParam->NumberofParameters >= 1))
         {
            /* The HCR ID and the parameters appears to be semi-valid.  */
            /* Now try to issue the 1284 ID String Request.             */
            Result = HCR_IEEE_1284_ID_String_Query_Request(BluetoothStackID, HCRID, (Word_t)TempParam->Params[0].intParam, HCR_IEEE_1284_ID_STRING_MAXIMUM_LENGTH);

            if(!Result)
            {
               /* The function was called successfully.  Display a      */
               /* message indicating that the 1284 String ID Query      */
               /* Request function was successfully submitted.          */
               printf("HCR_IEEE_1284_ID_String_Query_Request: Function Successful.\r\n");

               /* Flag the the function was successful.                 */
               ret_val = 0;
            }
            else
            {
               /* An error occurred while attempting to issue the 1284  */
               /* ID String Query Request function.                     */
               printf("HCR_IEEE_1284_ID_String_Query_Request: Function Failure: %d.\r\n", Result);

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* Invalid parameters, so inform the user.                  */
            printf("Usage: Get1284IDRequest [Starting Index].\r\n");

            /* One or more of the necessary parameters is/are invalid.  */
            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* The HCR ID is invalid.                                      */
         if(!HCRID)
            printf("HCR_IEEE_1284_ID_String_Query_Request: HCR Client Connection does not exist.\r\n");
         else
            printf("HCR_IEEE_1284_ID_String_Query_Request: Not in HCR Client Mode.\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is responsible for sending a 1284 ID String*/
   /* Reply PDU (with Successful Status and hardcoded 1284 ID String    */
   /* Data) to the remote HCR Client.  This function returns zero on    */
   /* successful execution and a negative value on all errors.          */
   /* * NOTE * This function only exists for demonstration purposes     */
   /*          only.  Normally the data that would be sent would be     */
   /*          the actual 1284 ID String and would take into account the*/
   /*          requested number of bytes (received in the Request) and  */
   /*          the starting index (received in the Request).  This      */
   /*          simple method is used so that the programmer can see how */
   /*          the HCR_IEEE_1284_ID_String_Query_Reply() function is    */
   /*          used.                                                    */
static int Perform1284IDReply(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Check to see if a HCR Server has already been established.     */
      if((!IsClient) && (HCRID))
      {
         /* The HCR ID appears to be semi-valid.  Now try to issue the  */
         /* 1284 ID String Query Reply.                                 */
         Result = HCR_IEEE_1284_ID_String_Query_Reply(BluetoothStackID, HCRID, HCR_PDU_STATUS_SUCCESS, (Word_t)strlen(HCR_SAMPLE_1284_ID_STRING), (Byte_t *)HCR_SAMPLE_1284_ID_STRING);

         if(Result > 0)
         {
            /* The function was called successfully.  Display a message */
            /* indicating that the 1284 ID String Query Reply function  */
            /* was successfully submitted.                              */
            printf("HCR_IEEE_1284_ID_String_Query_Reply: Function Successful: %d.\r\n", Result);

            /* Flag the the function was successful.                    */
            ret_val = 0;
         }
         else
         {
            /* An error occurred while attempting to issue the 1284 ID  */
            /* String Query Request function.                           */
            printf("HCR_IEEE_1284_ID_String_Query_Reply: Function Failure: %d.\r\n", Result);

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* The HCR ID is invalid.                                      */
         if(!HCRID)
            printf("HCR_IEEE_1284_ID_String_Query_Reply: HCR Server does not exist.\r\n");
         else
            printf("HCR_IEEE_1284_ID_String_Query_Reply: Not in HCR Server Mode.\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is responsible for sending a Soft Reset    */
   /* Request PDU to the remote HCR Server.  This function returns zero */
   /* on successful execution and a negative value on all errors.       */
static int PerformSoftResetRequest(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Check to see if a HCR Client has already been established.     */
      if((IsClient) && (HCRID))
      {
         /* The HCR ID appears to be semi-valid.  Now try to issue the  */
         /* Soft Reset Request.                                         */
         Result = HCR_Soft_Reset_Request(BluetoothStackID, HCRID);

         if(!Result)
         {
            /* The function was called successfully.  Display a message */
            /* indicating that the Soft Reset function was successfully */
            /* submitted.                                               */
            printf("HCR_Soft_Reset: Function Successful.\r\n");

            /* Flag the the function was successful.                    */
            ret_val = 0;
         }
         else
         {
            /* An error occurred while attempting to issue the Soft     */
            /* Reset function.                                          */
            printf("HCR_Soft_Reset_Request: Function Failure: %d.\r\n", Result);

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* The HCR ID is invalid.                                      */
         if(!HCRID)
            printf("HCR_Soft_Reset_Request: HCR Client Connection does not exist.\r\n");
         else
            printf("HCR_Soft_Reset_Request: Not in HCR Client Mode.\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is responsible for sending a Soft Reset    */
   /* Reply PDU (with Successful Status) to the remote HCR Client.  This*/
   /* function returns zero on successful execution and a negative value*/
   /* on all errors.                                                    */
static int PerformSoftResetReply(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Check to see if a HCR Server has already been established.     */
      if((!IsClient) && (HCRID))
      {
         /* The HCR ID appears to be semi-valid.  Now try to issue the  */
         /* Soft Reset Reply.                                           */
         Result = HCR_Soft_Reset_Reply(BluetoothStackID, HCRID, HCR_PDU_STATUS_SUCCESS);

         if(!Result)
         {
            /* The function was called successfully.  Display a message */
            /* indicating that the Soft Reset Reply function was        */
            /* successfully submitted.                                  */
            printf("HCR_Soft_Reset_Reply: Function Successful.\r\n");

            /* Flag the the function was successful.                    */
            ret_val = 0;
         }
         else
         {
            /* An error occurred while attempting to issue the Soft     */
            /* Reset Reply function.                                    */
            printf("HCR_Soft_Reset_Reply: Function Failure: %d.\r\n", Result);

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* The HCR ID is invalid.                                      */
         if(!HCRID)
            printf("HCR_Soft_Reset_Reply: HCR Server does not exist.\r\n");
         else
            printf("HCR_Soft_Reset_Reply: Not in HCR Server Mode.\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is responsible for sending a Registration  */
   /* Request PDU to the remote HCR Server.  This function returns zero */
   /* on successful execution and a negative value on all errors.       */
static int PerformRegisterNotificationRequest(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Check to see if a HCR Client has already been established.     */
      if((IsClient) && (HCRID))
      {
         /* Make sure that the parameters required for this function    */
         /* appear to be valid.                                         */
         if((TempParam) && (TempParam->NumberofParameters >= 2))
         {
            /* The HCR ID and the parameters appears to be semi-valid.  */
            /* Now try to issue the Register Notification Request.      */
            Result = HCR_Register_Notification_Request(BluetoothStackID, HCRID, TRUE, TempParam->Params[0].intParam, TempParam->Params[1].intParam);

            if(!Result)
            {
               /* The function was called successfully.  Display a      */
               /* message indicating that the Register Notification     */
               /* function was successfully submitted.                  */
               printf("HCR_Register_Notification_Request: Function Successful.\r\n");

               /* Flag the the function was successful.                 */
               ret_val = 0;
            }
            else
            {
               /* An error occurred while attempting to issue the       */
               /* Register Notification function.                       */
               printf("HCR_Register_Notification_Request: Function Failure: %d.\r\n", Result);

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* Invalid parameters, so inform the user.                  */
            printf("Usage: RegisterNotificationRequest [Callback ID] [Callback Timeout].\r\n");

            /* One or more of the necessary parameters is/are invalid.  */
            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* The HCR ID is invalid.                                      */
         if(!HCRID)
            printf("HCR_Register_Notification_Request: HCR Client Connection does not exist.\r\n");
         else
            printf("HCR_Register_Notification_Request: Not in HCR Client Mode.\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is responsible for sending a Registration  */
   /* Notification Reply PDU (with Successful Status and Timeouts given */
   /* in the parameters) to the remote HCR Client.  This function       */
   /* returns zero on successful execution and a negative value on all  */
   /* errors.                                                           */
static int PerformRegisterNotificationReply(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Check to see if a HCR Server has already been established.     */
      if((!IsClient) && (HCRID))
      {
         /* Make sure that the parameters required for this function    */
         /* appear to be valid.                                         */
         if((TempParam) && (TempParam->NumberofParameters >= 2))
         {
            /* The HCR ID and parameters appears to be semi-valid.  Now */
            /* try to issue the Register Notification Reply.            */
            Result = HCR_Register_Notification_Reply(BluetoothStackID, HCRID, HCR_PDU_STATUS_SUCCESS, TempParam->Params[0].intParam, TempParam->Params[1].intParam);

            if(!Result)
            {
               /* The function was called successfully.  Display a      */
               /* message indicating that the Register Notificatoin     */
               /* Reply function was successfully submitted.            */
               printf("HCR_Register_Notification_Reply: Function Successful.\r\n");

               /* Flag the the function was successful.                 */
               ret_val = 0;
            }
            else
            {
               /* An error occurred while attempting to issue the       */
               /* Register Notification Reply function.                 */
               printf("HCR_Register_Notification_Reply: Function Failure: %d.\r\n", Result);

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* Invalid parameters, so inform the user.                  */
            printf("Usage: RegisterNotificationReply [Not. Timeout] [Callback Timeout].\r\n");

            /* One or more of the necessary parameters is/are invalid.  */
            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* The HCR ID is invalid.                                      */
         if(!HCRID)
            printf("HCR_Register_Notification_Reply: HCR Server does not exist.\r\n");
         else
            printf("HCR_Register_Notification_Reply: Not in HCR Server Mode.\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is responsible for sending a Notification  */
   /* Request PDU to the remote Notification Server.  This function     */
   /* returns zero on successful execution and a negative value on all  */
   /* errors.                                                           */
static int PerformNotificationRequest(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Check to see if a Notification Client has already been         */
      /* established.                                                   */
      if((!IsClient) && (NotificationID))
      {
         /* Make sure that the parameters required for this function    */
         /* appear to be valid.                                         */
         if((TempParam) && (TempParam->NumberofParameters >= 1))
         {
            /* The Notification ID and the parameters appears to be     */
            /* semi-valid.  Now try to issue the Notification Request.  */
            Result = HCR_Notification_Request(BluetoothStackID, NotificationID, TempParam->Params[0].intParam);

            if(!Result)
            {
               /* The function was called successfully.  Display a      */
               /* message indicating that the Notification function was */
               /* successfully submitted.                               */
               printf("HCR_Notification_Request: Function Successful.\r\n");

               /* Flag the the function was successful.                 */
               ret_val = 0;
            }
            else
            {
               /* An error occurred while attempting to issue the       */
               /* Notification function.                                */
               printf("HCR_Notification_Request: Function Failure: %d.\r\n", Result);

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* Invalid parameters, so inform the user.                  */
            printf("Usage: NotificationRequest [Callback ID].\r\n");

            /* One or more of the necessary parameters is/are invalid.  */
            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* The Notification ID is invalid.                             */
         if(!NotificationID)
            printf("HCR_Notification_Request: Notification Connection does not exist.\r\n");
         else
            printf("HCR_Notification_Request: Not in HCR Server Mode.\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   /* Return the result to the caller.                                  */
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

      printf("\r\nHCR>");
   }
   else
   {
      /* There was an error with one or more of the input parameters.   */
      printf("\r\n");

      printf("GAP Callback Data: Event_Data = NULL.\r\n");

      printf("\r\nHCR>");
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

   printf("\r\nHCR>");

   /* Make sure the output is displayed to the user.                    */
   fflush(stdout);
}

   /* The following function is for a HCR Event Callback.  This         */
   /* function will be called whenever a HCR Event occurs that is       */
   /* associated with the Bluetooth Stack.  This function passes to the */
   /* caller the HCR Event Data that occurred and the HCR Event Callback*/
   /* Parameter that was specified when this Callback was installed.    */
   /* The caller is free to use the contents of the HCR Event Data ONLY */
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
   /* another HCR Event will not be processed while this function call  */
   /* is outstanding).                                                  */
   /* * NOTE * This function MUST NOT Block and wait for events that    */
   /*          can only be satisfied by Receiving HCR Event Packets.  A */
   /*          Deadlock WILL occur because NO HCR Event Callbacks will  */
   /*          be issued while this function is currently outstanding.  */
static void BTPSAPI HCR_Event_Callback(unsigned int BluetoothStackID, HCR_Event_Data_t *HCREventData, unsigned long CallbackParameter)
{
   char BoardStr[13];
   char temp;

   /* First, check to see if the required parameters appear to be       */
   /* semi-valid.                                                       */
   if((BluetoothStackID) && (HCREventData != NULL))
   {
      printf("\r\n");

      /* The parameters appear to be semi-valid, now check to see what  */
      /* type the incoming event is.                                    */
      switch(HCREventData->Event_Data_Type)
      {
         case etHCR_Control_Connect_Indication:
            BD_ADDRToStr(HCREventData->Event_Data.HCR_Control_Connect_Indication_Data->BD_ADDR, BoardStr);
            printf("HCR Control Connect Indication, ID: 0x%04X, Board: %s\r\n", HCREventData->Event_Data.HCR_Control_Connect_Indication_Data->HCRID, BoardStr);
            break;
         case etHCR_Data_Connect_Indication:
            BD_ADDRToStr(HCREventData->Event_Data.HCR_Data_Connect_Indication_Data->BD_ADDR, BoardStr);
            printf("HCR Data Connect Indication, ID: 0x%04X, Board: %s\r\n", HCREventData->Event_Data.HCR_Data_Connect_Indication_Data->HCRID, BoardStr);
            break;
         case etHCR_Notification_Connect_Indication:
            BD_ADDRToStr(HCREventData->Event_Data.HCR_Notification_Connect_Indication_Data->BD_ADDR, BoardStr);
            printf("HCR Notification Connect Indication, ID: 0x%04X, Board: %s\r\n", HCREventData->Event_Data.HCR_Notification_Connect_Indication_Data->HCRID, BoardStr);
            break;
         case etHCR_Control_Connect_Confirmation:
            printf("HCR Control Connect Confirmation, ID: 0x%04X, Status 0x%04X\r\n", HCREventData->Event_Data.HCR_Control_Connect_Confirmation_Data->HCRID,
                                                                                      HCREventData->Event_Data.HCR_Control_Connect_Confirmation_Data->ConnectionStatus);


            /* Flag that there is no longer a HCR Client Connection if  */
            /* there was an error during the connection.                */
            if(HCREventData->Event_Data.HCR_Control_Connect_Confirmation_Data->ConnectionStatus != HCR_OPEN_STATUS_SUCCESS)
               HCRID = 0;
            break;
         case etHCR_Data_Connect_Confirmation:
            printf("HCR Data Connect Confirmation, ID: 0x%04X, Status 0x%04X\r\n", HCREventData->Event_Data.HCR_Data_Connect_Confirmation_Data->HCRID,
                                                                                   HCREventData->Event_Data.HCR_Data_Connect_Confirmation_Data->ConnectionStatus);
            break;
         case etHCR_Notification_Connect_Confirmation:
            printf("HCR Notification Connect Confirmation, ID: 0x%04X, Status 0x%04X\r\n", HCREventData->Event_Data.HCR_Notification_Connect_Confirmation_Data->HCRID,
                                                                                           HCREventData->Event_Data.HCR_Notification_Connect_Confirmation_Data->ConnectionStatus);

            /* Flag that there is no longer a Notification Client       */
            /* Connection if there was an error.                        */
            if(HCREventData->Event_Data.HCR_Notification_Connect_Confirmation_Data->ConnectionStatus != HCR_OPEN_STATUS_SUCCESS)
               NotificationID = 0;
            break;
         case etHCR_Control_Disconnect_Indication:
            printf("HCR Control Disconnect Indication, ID: 0x%04X\r\n", HCREventData->Event_Data.HCR_Control_Disconnect_Indication_Data->HCRID);

            /* Flag that there is no longer a HCR Client Connection.    */
            if(IsClient)
               HCRID = 0;
            break;
         case etHCR_Data_Disconnect_Indication:
            printf("HCR Data Disconnect Indication, ID: 0x%04X\r\n", HCREventData->Event_Data.HCR_Data_Disconnect_Indication_Data->HCRID);
            break;
         case etHCR_Notification_Disconnect_Indication:
            printf("HCR Notification Disconnect Indication, ID: 0x%04X\r\n", HCREventData->Event_Data.HCR_Notification_Disconnect_Indication_Data->HCRID);

            /* Flag that there is no longer a Notification Client       */
            /* Connection.                                              */
            if(!IsClient)
               NotificationID = 0;
            break;
         case etHCR_Credit_Grant_Indication:
            printf("HCR Credit Grant Indication, ID: 0x%04X\r\n", HCREventData->Event_Data.HCR_Credit_Grant_Indication_Data->HCRID);
            printf("   Credit: 0x%08lX, Total Send: 0x%08lX\r\n", HCREventData->Event_Data.HCR_Credit_Grant_Indication_Data->CreditGranted, HCREventData->Event_Data.HCR_Credit_Grant_Indication_Data->TotalSendCredit);
            break;
         case etHCR_Credit_Grant_Confirmation:
            printf("HCR Credit Grant Confirmation, ID: 0x%04X, Status: 0x%04X\r\n", HCREventData->Event_Data.HCR_Credit_Grant_Confirmation_Data->HCRID, HCREventData->Event_Data.HCR_Credit_Grant_Confirmation_Data->Status);
            printf("   Total Receive: 0x%08lX\r\n", HCREventData->Event_Data.HCR_Credit_Grant_Confirmation_Data->TotalReceiveCredit);
            break;
         case etHCR_Credit_Request_Indication:
            printf("HCR Credit Request Indication, ID: 0x%04X\r\n", HCREventData->Event_Data.HCR_Credit_Request_Indication_Data->HCRID);
            printf("   Total Receive: 0x%08lX\r\n", HCREventData->Event_Data.HCR_Credit_Request_Indication_Data->TotalReceiveCredit);
            break;
         case etHCR_Credit_Request_Confirmation:
            printf("HCR Credit Request Confirmation, ID: 0x%04X, Status: 0x%04X\r\n", HCREventData->Event_Data.HCR_Credit_Request_Confirmation_Data->HCRID, HCREventData->Event_Data.HCR_Credit_Request_Confirmation_Data->Status);
            printf("   Credit: 0x%08lX, Total Send: 0x%08lX\r\n", HCREventData->Event_Data.HCR_Credit_Request_Confirmation_Data->CreditGranted, HCREventData->Event_Data.HCR_Credit_Request_Confirmation_Data->TotalSendCredit);
            break;
         case etHCR_Credit_Return_Indication:
            printf("HCR Credit Return Indication, ID: 0x%04X\r\n", HCREventData->Event_Data.HCR_Credit_Return_Indication_Data->HCRID);
            printf("   Credit: 0x%08lX, Total Receive: 0x%08lX\r\n", HCREventData->Event_Data.HCR_Credit_Return_Indication_Data->CreditReturned, HCREventData->Event_Data.HCR_Credit_Return_Indication_Data->TotalReceiveCredit);

            /* Since the user has no way to respond to this via the User*/
            /* Interface, we will simply respond with an unsupported    */
            /* feature.                                                 */
            HCR_Credit_Request_Reply(BluetoothStackID, HCRID, HCR_PDU_STATUS_FEATURE_UNSUPPORTED, 0);
            break;
         case etHCR_Credit_Return_Confirmation:
            printf("HCR Credit Return Confirmation, ID: 0x%04X, Status: 0x%04X\r\n", HCREventData->Event_Data.HCR_Credit_Return_Confirmation_Data->HCRID, HCREventData->Event_Data.HCR_Credit_Return_Confirmation_Data->Status);
            printf("   Credit: 0x%08lX, Total Send: 0x%08lX, Total Receive: 0x%08lX\r\n", HCREventData->Event_Data.HCR_Credit_Return_Confirmation_Data->CreditReturned, HCREventData->Event_Data.HCR_Credit_Return_Confirmation_Data->TotalSendCredit, HCREventData->Event_Data.HCR_Credit_Return_Confirmation_Data->TotalReceiveCredit);
            break;
         case etHCR_Credit_Query_Indication:
            printf("HCR Credit Query Indication, ID: 0x%04X\r\n", HCREventData->Event_Data.HCR_Credit_Query_Indication_Data->HCRID);
            printf("   Query Receive: 0x%08lX, Total Receive: 0x%08lX\r\n", HCREventData->Event_Data.HCR_Credit_Query_Indication_Data->QueryReceiveCredit, HCREventData->Event_Data.HCR_Credit_Query_Indication_Data->TotalReceiveCredit);
            break;
         case etHCR_Credit_Query_Confirmation:
            printf("HCR Credit Query Confirmation, ID: 0x%04X, Status: 0x%04X\r\n", HCREventData->Event_Data.HCR_Credit_Query_Confirmation_Data->HCRID, HCREventData->Event_Data.HCR_Credit_Query_Confirmation_Data->Status);
            printf("   Query Receive: 0x%08lX, Total Receive: 0x%08lX\r\n", HCREventData->Event_Data.HCR_Credit_Query_Confirmation_Data->QueryReceiveCredit, HCREventData->Event_Data.HCR_Credit_Query_Confirmation_Data->TotalReceiveCredit);
            break;
         case etHCR_LPT_Status_Query_Indication:
            printf("HCR LPT Status Query Indication, ID: 0x%04X\r\n", HCREventData->Event_Data.HCR_LPT_Status_Query_Indication_Data->HCRID);
            break;
         case etHCR_LPT_Status_Query_Confirmation:
            printf("HCR LPT Status Query Confirmation, ID: 0x%04X, Status: 0x%04X\r\n", HCREventData->Event_Data.HCR_LPT_Status_Query_Confirmation_Data->HCRID, HCREventData->Event_Data.HCR_LPT_Status_Query_Confirmation_Data->Status);
            printf("   Value: 0x%02X\r\n", HCREventData->Event_Data.HCR_LPT_Status_Query_Confirmation_Data->LPTStatus);
            break;
         case etHCR_IEEE_1284_ID_Query_Indication:
            printf("HCR IEEE 1284ID Query Indication, ID: 0x%04X\r\n", HCREventData->Event_Data.HCR_IEEE_1284_ID_Query_Indication_Data->HCRID);
            printf("   Start: 0x%04X, Requested: 0x%04X, Total: 0x%04X\r\n", HCREventData->Event_Data.HCR_IEEE_1284_ID_Query_Indication_Data->StartingIndex, HCREventData->Event_Data.HCR_IEEE_1284_ID_Query_Indication_Data->RequestedNumberBytes, HCREventData->Event_Data.HCR_IEEE_1284_ID_Query_Indication_Data->MaximumNumberBytes);
            break;
         case etHCR_IEEE_1284_ID_Query_Confirmation:
            printf("HCR IEEE 1284ID Query Confirmation, ID: 0x%04X, Status: 0x%04X\r\n", HCREventData->Event_Data.HCR_IEEE_1284_ID_Query_Confirmation_Data->HCRID, HCREventData->Event_Data.HCR_IEEE_1284_ID_Query_Confirmation_Data->Status);
            printf("   Length: 0x%04X\r\n", HCREventData->Event_Data.HCR_IEEE_1284_ID_Query_Confirmation_Data->NumberBytes);

            /* Attempt to Display the Data to the user (note that this  */
            /* might fail if the data is binary).                       */
            printf("   Data:");

            /* Note the last character that is in the buffer.           */
            temp                                                                                                                                                                = HCREventData->Event_Data.HCR_IEEE_1284_ID_Query_Confirmation_Data->IEEE1284Data[HCREventData->Event_Data.HCR_IEEE_1284_ID_Query_Confirmation_Data->NumberBytes - 1];

            /* NULL terminate the string that was given to us.          */
            HCREventData->Event_Data.HCR_IEEE_1284_ID_Query_Confirmation_Data->IEEE1284Data[HCREventData->Event_Data.HCR_IEEE_1284_ID_Query_Confirmation_Data->NumberBytes - 1] = '\0';
            printf((char *)HCREventData->Event_Data.HCR_IEEE_1284_ID_Query_Confirmation_Data->IEEE1284Data);

            /* Replace the character that we removed.                   */
            HCREventData->Event_Data.HCR_IEEE_1284_ID_Query_Confirmation_Data->IEEE1284Data[HCREventData->Event_Data.HCR_IEEE_1284_ID_Query_Confirmation_Data->NumberBytes - 1] = temp;

            /* Print out the last character.                            */
            printf("%c\r\n", temp);
            break;
         case etHCR_Soft_Reset_Indication:
            printf("HCR Soft Reset Indication, ID: 0x%04X\r\n", HCREventData->Event_Data.HCR_Soft_Reset_Indication_Data->HCRID);
            break;
         case etHCR_Soft_Reset_Confirmation:
            printf("HCR Soft Reset Confirmation, ID: 0x%04X, Status: 0x%04X\r\n", HCREventData->Event_Data.HCR_Soft_Reset_Confirmation_Data->HCRID, HCREventData->Event_Data.HCR_Soft_Reset_Confirmation_Data->Status);
            break;
         case etHCR_Hard_Reset_Indication:
            printf("HCR Hard Reset Indication, ID: 0x%04X\r\n", HCREventData->Event_Data.HCR_Hard_Reset_Indication_Data->HCRID);

            /* Since the user has no way to respond to this via the User*/
            /* Interface, we will simply respond with an unsupported    */
            /* feature.                                                 */
            HCR_Hard_Reset_Reply(BluetoothStackID, HCRID, HCR_PDU_STATUS_FEATURE_UNSUPPORTED);
            break;
         case etHCR_Hard_Reset_Confirmation:
            printf("HCR Hard Reset Confirmation, ID: 0x%04X, Status: 0x%04X\r\n", HCREventData->Event_Data.HCR_Hard_Reset_Confirmation_Data->HCRID, HCREventData->Event_Data.HCR_Hard_Reset_Confirmation_Data->Status);
            break;
         case etHCR_Register_Notification_Indication:
            printf("HCR Register Notification Indication, ID: 0x%04X\r\n", HCREventData->Event_Data.HCR_Register_Notification_Indication_Data->HCRID);
            printf("   Register: %s, Context: 0x%08lX, Timeout: 0x%08lX\r\n", (HCREventData->Event_Data.HCR_Register_Notification_Indication_Data->Register?"TRUE":"FALSE"), HCREventData->Event_Data.HCR_Register_Notification_Indication_Data->CallbackContextID, HCREventData->Event_Data.HCR_Register_Notification_Indication_Data->CallbackTimeout);
            break;
         case etHCR_Register_Notification_Confirmation:
            printf("HCR Register Notification Confirmation, ID: 0x%04X, Status: 0x%04X\r\n", HCREventData->Event_Data.HCR_Register_Notification_Confirmation_Data->HCRID, HCREventData->Event_Data.HCR_Register_Notification_Confirmation_Data->Status);
            printf("   Not. Timeout: 0x%08lX, Callback Timeout: 0x%08lX\r\n", HCREventData->Event_Data.HCR_Register_Notification_Confirmation_Data->NotificationTimeout, HCREventData->Event_Data.HCR_Register_Notification_Confirmation_Data->CallbackTimeout);
            break;
         case etHCR_Notification_Connection_Alive_Indication:
            printf("HCR Notification Connection Alive Indication, ID: 0x%04X\r\n", HCREventData->Event_Data.HCR_Notification_Connection_Alive_Indication_Data->HCRID);

            /* Since the user has no way to respond to this via the User*/
            /* Interface, we will simply respond with an unsupported    */
            /* feature.                                                 */
            HCR_Notification_Connection_Alive_Reply(BluetoothStackID, HCRID, HCR_PDU_STATUS_FEATURE_UNSUPPORTED, 0);
            break;
         case etHCR_Notification_Connection_Alive_Confirmation:
            printf("HCR Notification Connection Alive Confirmation, ID: 0x%04X, Status: 0x%04X\r\n", HCREventData->Event_Data.HCR_Notification_Connection_Alive_Confirmation_Data->HCRID, HCREventData->Event_Data.HCR_Notification_Connection_Alive_Confirmation_Data->Status);
            printf("   Not. Timeout: 0x%08lX\r\n", HCREventData->Event_Data.HCR_Notification_Connection_Alive_Confirmation_Data->NotificationTimeout);
            break;
         case etHCR_Vendor_Specific_Data_Indication:
            printf("HCR Vendor Specific Indication, ID: 0x%04X\r\n", HCREventData->Event_Data.HCR_Vendor_Specific_Indication_Data->HCRID);
            printf("   PDU ID: 0x%04X, Length: 0x%04X\r\n", HCREventData->Event_Data.HCR_Vendor_Specific_Indication_Data->PDUID, HCREventData->Event_Data.HCR_Vendor_Specific_Indication_Data->DataLength);

            /* Since the user has no way to respond to this via the User*/
            /* Interface, we will simply respond with an unsupported    */
            /* feature.                                                 */
            HCR_Vendor_Specific_Data_Reply(BluetoothStackID, HCRID, HCR_PDU_STATUS_FEATURE_UNSUPPORTED, 0, NULL);
            break;
         case etHCR_Vendor_Specific_Data_Confirmation:
            printf("HCR Vendor Specific Indication, ID: 0x%04X, Status: 0x%04X\r\n", HCREventData->Event_Data.HCR_Vendor_Specific_Confirmation_Data->HCRID, HCREventData->Event_Data.HCR_Vendor_Specific_Confirmation_Data->Status);
            printf("   PDU ID: 0x%04X, Length: 0x%04X\r\n", HCREventData->Event_Data.HCR_Vendor_Specific_Confirmation_Data->PDUID, HCREventData->Event_Data.HCR_Vendor_Specific_Confirmation_Data->DataLength);
            break;
         case etHCR_Data_Indication:
            printf("HCR Data Indication, ID: 0x%04X\r\n", HCREventData->Event_Data.HCR_Data_Indication_Data->HCRID);
            printf("   Length: 0x%04X\r\n", HCREventData->Event_Data.HCR_Data_Indication_Data->DataLength);

            /* Attempt to Display the Data to the user (note that this  */
            /* might fail if the data is binary).                       */
            printf("   Data:");

            /* Note the last character that is in the buffer.           */
            temp                                                                                                                             = HCREventData->Event_Data.HCR_IEEE_1284_ID_Query_Confirmation_Data->IEEE1284Data[HCREventData->Event_Data.HCR_Data_Indication_Data->DataLength - 1];

            /* NULL terminate the string that was given to us.          */
            HCREventData->Event_Data.HCR_Data_Indication_Data->DataBuffer[HCREventData->Event_Data.HCR_Data_Indication_Data->DataLength - 1] = '\0';
            printf((char *)(HCREventData->Event_Data.HCR_Data_Indication_Data->DataBuffer));

            /* Replace the character that we removed.                   */
            HCREventData->Event_Data.HCR_Data_Indication_Data->DataBuffer[HCREventData->Event_Data.HCR_Data_Indication_Data->DataLength - 1] = temp;

            /* Print out the last character.                            */
            printf("%c\r\n", temp);
            break;
         case etHCR_Notification_Indication:
            printf("HCR Notification Indication, ID: 0x%04X\r\n", HCREventData->Event_Data.HCR_Notification_Indication_Data->HCRID);
            printf("   Context: 0x%08lX\r\n", HCREventData->Event_Data.HCR_Notification_Indication_Data->CallbackContextID);
            break;
         default:
            printf("Unknown HCR Event.\r\n");

            /* In the event that we received this unknown PDU we need to*/
            /* respond with an unsupported Feature Reply.               */
            HCR_Feature_Unsupported_Reply(BluetoothStackID, HCRID);
            break;
      }
   }
   else
   {
      /* There was an error with one or more of the input parameters.   */
      printf("\r\nHCR callback data: Event_Data = NULL.\r\n");
   }

   /* Output an Input Shell-type prompt.                                */
   printf("\r\nHCR>");

   /* Make sure the output is displayed to the user.                    */
   fflush(stdout);
}

   /* Main Program Entry Point.                                         */
int main(int argc, char* argv[])
{
   char                    *endptr = NULL;
   int                      ret_val;
   unsigned int             CommPortNumber;
   unsigned int             BaudRate;
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
            /* The Stack was successfully opened, next check to see if  */
            /* we are currently in Host Mode or Device Mode and         */
            /* Initialize the appropriate type.                         */
            if(IsClient)
               ret_val = InitializeHCRClient();
            else
               ret_val = InitializeHCRServer();

            /* The HCR Server or HCR Client has been initialized, now   */
            /* show the Main User Interface for that type.              */
            if(!ret_val)
               UserInterface_Main();

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

