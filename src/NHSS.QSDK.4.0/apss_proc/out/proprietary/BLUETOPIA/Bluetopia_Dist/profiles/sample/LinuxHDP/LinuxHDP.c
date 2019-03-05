/*****< linuxhdp.c >***********************************************************/
/*      Copyright 2010 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  LINUXHDP - Simple Linux application using HDP Profile.                    */
/*                                                                            */
/*  Author:  Tim Thomas                                                       */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/01/10  T. Thomas      Initial creation.                               */
/******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "LinuxHDP.h"      /* Main Application Prototypes and Constants.      */

#include "SS1BTPS.h"       /* Includes for the SS1 Bluetooth Protocol Stack.  */
#include "SS1BTHDP.h"      /* Includes for the SS1 HDP Profile.               */
#include "SS1BTDBG.h"      /* Includes/Constants for Bluetooth Debugging.     */

#define LOCAL_NAME_ROOT                      "LinuxHDP"  /* Root of the local */
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

#define MAX_SUPPORTED_COMMANDS                     (32)  /* Denotes the       */
                                                         /* maximum number of */
                                                         /* User Commands that*/
                                                         /* are supported by  */
                                                         /* this application. */

#define MAX_NUM_OF_PARAMETERS                       (6)  /* Denotes the max   */
                                                         /* number of         */
                                                         /* parameters a      */
                                                         /* command can have. */

#define MAX_COMMAND_LENGTH                         (64)  /* Denotes the max   */
                                                         /* buffer size used  */
                                                         /* for user commands */
                                                         /* inputted via the  */
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
                                                         /* error occurred due*/
                                                         /* to attempted      */
                                                         /* execution of a    */
                                                         /* command without a */
                                                         /* valid Bluetooth   */
                                                         /* Stack ID.         */

#define INDENT_LENGTH                               (3)  /* Denotes the number*/
                                                         /* of spaces to      */
                                                         /* indent each SDP   */
                                                         /* level.            */

#define VENDOR_ID_VALUE                        (0xAA55)  /* Vendor ID for DID */
                                                         /* Profile.          */

   /* The following defines the values that are passed to the SDP Search*/
   /* routine to define what type of search to perform.                 */
#define SDP_SEARCH_TYPE_GENERAL                 0
#define SDP_SEARCH_TYPE_HDP_PROFILE             1
#define SDP_SEARCH_TYPE_DID                     2

   /* The following constants represent the default log file names that */
   /* are used if no Log file name is specified when enabling debug.    */
#define DEFAULT_DEBUG_LOG_FILE_NAME             "LinuxHDP_ASC.log"
#define DEFAULT_DEBUG_FTS_FILE_NAME             "LinuxHDP_FTS.log"

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

/******************************************************************************/
/**  Structure to Hold Instance Information.                                 **/
/******************************************************************************/
#define MAX_MDEPS_PER_INSTANCE                  5

typedef struct _tagMDEP_Info_t
{
   Byte_t  MDEP_ID;
   Word_t  DataType;
   Byte_t  Role;
   char   *Description;
   int     DataLinkID;
} MDEP_Info_t;

typedef struct _tagInstance_Info_t
{
   Word_t      Version;
   Word_t      ControlPSM;
   Word_t      DataPSM;
   Byte_t      SupportedProcedures;
   int         NumberOfEndpoints;
   MDEP_Info_t MDEPInfo[MAX_MDEPS_PER_INSTANCE];
} Instance_Info_t;

/******************************************************************************/
/**  Extended Inquiry Response Data used to Set the local EIR Response Data. **/
/******************************************************************************/
typedef struct _tagEIR_Info_t
{
   int        NumberOfClassIDs;
   UUID_16_t *ClassIDList;
   int        DeviceNameLength;
   char      *DeviceName;
   int        VendorIDLength;
   Word_t     VendorID;
} EIR_Info_t;

#define EIR_FLAGS_ID                            0x01
#define EIR_PARTIAL_LIST_OF_16_BIT_UUIDS_ID     0x02
#define EIR_COMPLETE_LIST_OF_16_BIT_UUIDS_ID    0x03
#define EIR_PARTIAL_LIST_OF_32_BIT_UUIDS_ID     0x04
#define EIR_COMPLETE_LIST_OF_32_BIT_UUIDS_ID    0x05
#define EIR_PARTIAL_LIST_OF_128_BIT_UUIDS_ID    0x06
#define EIR_COMPLETE_LIST_OF_128_BIT_UUIDS_ID   0x07
#define EIR_PARTIAL_LOCAL_NAME_ID               0x08
#define EIR_COMPLETE_LOCAL_NAME_ID              0x09
#define EIR_TX_POWER_ID                         0x0A
#define EIR_MANUFACTURE_SPECIFIC_DATA_ID        0xFF

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

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the         */
   /* compiler as part of standard C/C++).                              */
static unsigned int        BluetoothStackID;        /* Variable which holds the Handle */
                                                    /* of the opened Bluetooth Protocol*/
                                                    /* Stack.                          */

static unsigned int        DebugID;                 /* Variable which holds the ID of  */
                                                    /* the currently enabled debugging */
                                                    /* session.                        */

static unsigned int        HDPID;                   /* Variable which holds the Handle */
                                                    /* of the active HDP Connection.   */

static int                 IsSource;                /* Variable used to indicate if the*/
                                                    /* program is to be run as an      */
                                                    /* Source or Sync Device Mode.     */

static BD_ADDR_t           InquiryResultList[MAX_INQUIRY_RESULTS];  /* Variable which  */
                                                    /* contains the inquiry result     */
                                                    /* received from the most recently */
                                                    /* preformed inquiry.              */

static int                 NumberofValidResponses;  /* Variable which holds the number */
                                                    /* of valid inquiry results within */
                                                    /* the inquiry results array.      */

static LinkKeyInfo_t       LinkKeyInfo[16];         /* Variable which holds the list of*/
                                                    /* BD_ADDR <-> Link Keys for       */
                                                    /* pairing.                        */

static int                 MCLList[MAX_INQUIRY_RESULTS];  /* Array to hold the MCLID   */
                                                    /* that is associated with a remote*/
                                                    /* BD_ADDR                         */

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

static HDP_Device_Role_t   Role;                    /* Variable to denote if the local */
                                                    /* instance is a Source or Sync.   */

static Boolean_t           SyncSupport;             /* Variable to denote if the local */
                                                    /* instance supports the Sync      */
                                                    /* Protocol.                       */

static Boolean_t           SyncMaster;              /* Variable to denote if the local */
                                                    /* instance supports the Sync      */
                                                    /* Master Role.                    */

static int                 DataLinkCount;           /* Variable to track the number of */
                                                    /* Data Links that are currently   */
                                                    /* connected.                      */

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

static char *ConnectResponseString[] =
{
   "Success",
   "Connection Timeout",
   "Connection Refused",
   "Connection Terminated",
   "Configuration Error",
   "Instance not Registered",
   "Unknown Error"
};

#define MAX_CONNECT_RESPONSE_STRING_INDEX       (sizeof(ConnectResponseString)/sizeof(char *) - 1)

static HDP_MDEP_Info_t MDEP_Info[] =
{
   { 0x01, 4100, drSink, "Sample Pulse Oximeter"         },
   { 0x02, 4103, drSink, "Sample Blood Pressure Monitor" },
   { 0x03, 4104, drSink, "Sample Thermometer"            },
   { 0x04, 4111, drSink, "Sample Weight Scale"           },
   { 0x05, 4113, drSink, "Sample Glucose Monitor"        }
} ;

#define NUM_MDEP_ENDPOINTS                      (sizeof(MDEP_Info)/sizeof(HDP_MDEP_Info_t))

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
   { "Personal Area Network", { 0x00, 0x00, 0x11, 0x15, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB } },
   { "Phonebook Access",      { 0x00, 0x00, 0x11, 0x30, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB } },
   { "SIM Access",            { 0x00, 0x00, 0x11, 0x2D, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB } },
   { "Serial Port",           { 0x00, 0x00, 0x11, 0x01, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB } },
   { "IrSYNC",                { 0x00, 0x00, 0x11, 0x04, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB } }
} ;

#define NUM_UUIDS                               (sizeof(UUIDTable)/sizeof(UUIDInfo_t))

   /* Internal function prototypes.                                     */
static void DisplayHDPMenu(void);
static void PopulateHDPCommandTable(void);

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

static int InitializeHDP(void);

static int OpenStack(HCI_DriverInformation_t *HCI_DriverInformation);
static int CloseStack(void);

static int  SetDisc(void);
static int  SetConnect(void);
static int  SetPairable(void);
static int  DeleteLinkKey(BD_ADDR_t BD_ADDR);
static void ClearConnection(int MCLID);

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

static int SDPSearch(ParameterList_t *TempParam);
static int ServiceDiscovery(ParameterList_t *TempParam);
static int RegisterHDPInstance(ParameterList_t *TempParam);
static int UnRegisterHDPInstance(ParameterList_t *TempParam);
static int ConnectRemoteInstance(ParameterList_t *TempParam);
static int DisconnectControlChannel(ParameterList_t *TempParam);
static int DataWrite(ParameterList_t *TempParam);
static int AbortRequest(ParameterList_t *TempParam);

static int CreateDataChannel(ParameterList_t *TempParam);
static int DeleteDataChannel(ParameterList_t *TempParam);
static int SyncCapabilitiesRequest(ParameterList_t *TempParam);
static int SyncSetRequest(ParameterList_t *TempParam);
static int SyncInfo(ParameterList_t *TempParam);

static void ParseDIDResponse(SDP_Service_Search_Attribute_Response_Data_t *SDPResponse);
static void ParseSDPResponse(SDP_Service_Search_Attribute_Response_Data_t *SDPResponse);
static void DisplaySDPAttributeResponse(SDP_Service_Attribute_Response_Data_t *SDPServiceAttributeResponse, unsigned int InitLevel);
static void DisplaySDPSearchAttributeResponse(SDP_Service_Search_Attribute_Response_Data_t *SDPServiceSearchAttributeResponse);
static void DisplayDataElement(SDP_Data_Element_t *SDPDataElement, unsigned int Level);

   /* Callback Function Prototypes.                                     */
static void BTPSAPI GAP_Event_Callback(unsigned int BluetoothStackID, GAP_Event_Data_t *GAPEventData, unsigned long CallbackParameter);
static void BTPSAPI SDP_Event_Callback(unsigned int BluetoothStackID, unsigned int SDPRequestID, SDP_Response_Data_t *SDP_Response_Data, unsigned long CallbackParameter);
static void BTPSAPI HDP_Event_Callback(unsigned int BluetoothStackID, HDP_Event_Data_t *HDP_Event_Data, unsigned long CallbackParameter);

   /* The following function displays the HDP Host Command Menu.        */
static void DisplayHDPMenu(void)
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
   printf("*  SDPSearch [Inquiry Index] [Type]                                 *\r\n");
   printf("*  ServiceDiscovery [Inquiry Index] [Profile Index] [UUID (Manual)] *\r\n");
   printf("*  RegisterHDPInstance [CPSM] [DPSM] [Role] {[Sync] {[Sync Master]}}*\r\n");
   printf("*  UnRegisterHDPInstance                                            *\r\n");
   printf("*  ConnectRemoteInstance [Inquiry Index] [Control PSM] [Data PSM]   *\r\n");
   printf("*  DisconnectControlChannel [MCLID]                                 *\r\n");
   printf("*  CreateDataChannel [Inquiry Index] [MDEP ID] [Mode]               *\r\n");
   printf("*  DeleteDataChannel [Inquiry Index] [Datalink ID]                  *\r\n");
   printf("*  DataWrite [DataLinkID] [Data String]                             *\r\n");
   printf("*  AbortRequest [Inquiry Index]                                     *\r\n");
   printf("*  SyncCapabilitiesRequest [Inquiry Index] [Accuracy]               *\r\n");
   printf("*  SyncSetRequest [Inquiry Index]                                   *\r\n");
   printf("*  SyncInfo [Inquiry Index]                                         *\r\n");
   printf("*  EnableDebug [Enable/Disable] [Log Type] [Log File Name]          *\r\n");
   printf("*  Help                                                             *\r\n");
   printf("*  Quit                                                             *\r\n");

   printf("*********************************************************************\r\n");
}

   /* The following function Clears all commands currently in the       */
   /* command table and populates it with the Commands that are         */
   /* available for the HDP Host.                                       */
static void PopulateHDPCommandTable(void)
{
   /* First Clear all of the commands in the Command Table.             */
   ClearCommands();

   /* Now add all of the commands that are associated with the HDP Host */
   /* to the Command Table.                                             */
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
   AddCommand("SDPSEARCH", SDPSearch);
   AddCommand("SERVICEDISCOVERY", ServiceDiscovery);
   AddCommand("REGISTERHDPINSTANCE", RegisterHDPInstance);
   AddCommand("UNREGISTERHDPINSTANCE", UnRegisterHDPInstance);
   AddCommand("CONNECTREMOTEINSTANCE", ConnectRemoteInstance);
   AddCommand("DISCONNECTCONTROLCHANNEL", DisconnectControlChannel);
   AddCommand("CREATEDATACHANNEL", CreateDataChannel);
   AddCommand("DELETEDATACHANNEL", DeleteDataChannel);
   AddCommand("DATAWRITE", DataWrite);
   AddCommand("ABORTREQUEST", AbortRequest);
   AddCommand("SYNCCAPABILITIESREQUEST", SyncCapabilitiesRequest);
   AddCommand("SYNCSETREQUEST", SyncSetRequest);
   AddCommand("SYNCINFO", SyncInfo);
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

   /* Add the appropriate commands to the command table and display the */
   /* menu options.                                                     */
   PopulateHDPCommandTable();
   DisplayHDPMenu();

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
      printf("HDP>");
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
                  case EXIT_CODE:
                     /* If the user has request to exit we might as well*/
                     /* go ahead an close any Remote HDP Connection to a*/
                     /* Server that we have open.                       */
                     if(HDPID)
                        UnRegisterHDPInstance(NULL);
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
      if((memcmp(TempCommand->Command, "QUIT", strlen("QUIT")) != 0) && (memcmp(TempCommand->Command, "EXIT", strlen("EXIT")) != 0))
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
   /* Display the HDP Command Menu.                                     */
   DisplayHDPMenu();

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
   /* state of the Main Application to be a HDP Host.  This function    */
   /* returns zero on successful execution and a negative value on all  */
   /* errors.                                                           */
static int InitializeHDP(void)
{
   int ret_val;

   /* The program is currently running in Host mode.  Ready the device  */
   /* to be a HDP Host Server.  First, attempt to set the device to be  */
   /* connectable.                                                      */
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
            /* Initialize the list on remote connections and initialize */
            /* the HDP profile.                                         */
            memset(MCLList, 0, sizeof(MCLList));
            HDPID   = 0;
            ret_val = HDP_Initialize(BluetoothStackID);
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

   /* The following function is responsible for adding information to   */
   /* the SDP record to support the Device Identification Profile.      */
static int AddDeviceIDInformation(void)
{
   int                ret_val;
   DWord_t            SDPRecordHandle;
   SDP_UUID_Entry_t   UUIDEntry;
   SDP_Data_Element_t SDPDataElement;

   /* Create a Service Record with the PNP UUID.                        */
   UUIDEntry.SDP_Data_Element_Type = deUUID_16;
   SDP_ASSIGN_PNP_INFORMATION_UUID_16(UUIDEntry.UUID_Value.UUID_16);
   ret_val = SDP_Create_Service_Record(BluetoothStackID, 1, &UUIDEntry);
   if(ret_val)
   {
      /* Save the Handle to this Service Record.                        */
      SDPRecordHandle = ret_val;

      /* Add the Profile Version Attribute.                             */
      SDPDataElement.SDP_Data_Element_Type                  = deUnsignedInteger2Bytes;
      SDPDataElement.SDP_Data_Element_Length                = WORD_SIZE;
      SDPDataElement.SDP_Data_Element.UnsignedInteger2Bytes = 0x0103;
      ret_val = SDP_Add_Attribute(BluetoothStackID, SDPRecordHandle, SDP_ATTRIBUTE_ID_DID_SPECIFICATION_ID, &SDPDataElement);
      if(!ret_val)
      {
         /* Add the Vendor ID Attribute.                                */
         SDPDataElement.SDP_Data_Element_Type                  = deUnsignedInteger2Bytes;
         SDPDataElement.SDP_Data_Element_Length                = WORD_SIZE;
         SDPDataElement.SDP_Data_Element.UnsignedInteger2Bytes = VENDOR_ID_VALUE;
         ret_val = SDP_Add_Attribute(BluetoothStackID, SDPRecordHandle, SDP_ATTRIBUTE_ID_DID_VENDOR_ID, &SDPDataElement);
         if(!ret_val)
         {
            /* Add the Product ID Attribute.                            */
            SDPDataElement.SDP_Data_Element_Type                  = deUnsignedInteger2Bytes;
            SDPDataElement.SDP_Data_Element_Length                = WORD_SIZE;
            SDPDataElement.SDP_Data_Element.UnsignedInteger2Bytes = 0x0001;
            ret_val = SDP_Add_Attribute(BluetoothStackID, SDPRecordHandle, SDP_ATTRIBUTE_ID_DID_PRODUCT_ID, &SDPDataElement);
            if(!ret_val)
            {
               /* Add the Product Version ID Attribute.                 */
               SDPDataElement.SDP_Data_Element_Type                  = deUnsignedInteger2Bytes;
               SDPDataElement.SDP_Data_Element_Length                = WORD_SIZE;
               SDPDataElement.SDP_Data_Element.UnsignedInteger2Bytes = 0x0112;
               ret_val = SDP_Add_Attribute(BluetoothStackID, SDPRecordHandle, SDP_ATTRIBUTE_ID_DID_VERSION, &SDPDataElement);
               if(!ret_val)
               {
                  /* Add the Primary Record ID Attribute.               */
                  SDPDataElement.SDP_Data_Element_Type    = deBoolean;
                  SDPDataElement.SDP_Data_Element_Length  = BYTE_SIZE;
                  SDPDataElement.SDP_Data_Element.Boolean = TRUE;
                  ret_val = SDP_Add_Attribute(BluetoothStackID, SDPRecordHandle, SDP_ATTRIBUTE_ID_DID_PRIMARY_RECORD, &SDPDataElement);
                  if(!ret_val)
                  {
                     /* Add the Primary Record ID Attribute.            */
                     SDPDataElement.SDP_Data_Element_Type                  = deUnsignedInteger2Bytes;
                     SDPDataElement.SDP_Data_Element_Length                = WORD_SIZE;
                     SDPDataElement.SDP_Data_Element.UnsignedInteger2Bytes = 0x0001;
                     SDP_Add_Attribute(BluetoothStackID, SDPRecordHandle, SDP_ATTRIBUTE_ID_DID_VENDOR_ID_SOURCE, &SDPDataElement);
                  }
               }
            }
         }
      }
   }
   else
      printf("Failed to all Device Identification Record to the SDP Database.\r\n");

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
   int                        ret_val;
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

            /* Set the Class of Device for Medical.                     */
            ASSIGN_CLASS_OF_DEVICE(ClassOfDevice, 0x00, 0x00, 0x00);
            SET_MAJOR_SERVICE_CLASS(ClassOfDevice, HCI_LMP_CLASS_OF_DEVICE_SERVICE_CLASS_CAPTURING_BIT);
            SET_MAJOR_DEVICE_CLASS(ClassOfDevice, HCI_LMP_CLASS_OF_DEVICE_MAJOR_DEVICE_CLASS_HEALTH);
            SET_MINOR_DEVICE_CLASS(ClassOfDevice, HCI_LMP_CLASS_OF_DEVICE_MINOR_DEVICE_HEALTH_UNCLASSIFIED);
            GAP_Set_Class_Of_Device(BluetoothStackID, ClassOfDevice);

            /* Add information to support the Device Identification     */
            /* Profile.                                                 */
            AddDeviceIDInformation();

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
      /* Attempt to set the attached device to be pairable using Secured*/
      /* Simple Pairing.                                                */
      ret_val = GAP_Set_Pairability_Mode(BluetoothStackID, pmPairableMode);
      if(!ret_val)
         printf("GAP_Set_Pairability_Mode(pmPairableMode_EnableSecureSimplePairing).\r\n");
      else
      {
         /* The local device may not support SSP, so set legacy pairing */
         /* mode.                                                       */
         ret_val = GAP_Set_Pairability_Mode(BluetoothStackID, pmPairableMode);
         if(!ret_val)
            printf("GAP_Set_Pairability_Mode(pmPairableMode).\r\n");
      }

      /* Next, check the return value of the GAP Set Pairability mode   */
      /* command for successful execution.                              */
      if(!ret_val)
      {
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

   /* The following function is responsible for clearing the Connection */
   /* information from the connection list.                             */
static void ClearConnection(int MCLID)
{
   int Index;

   /* Verify that the Connection ID is valid.                           */
   if(MCLID)
   {
      /* Search the list for a match of the Connection ID.              */
      for(Index=0; Index < MAX_INQUIRY_RESULTS; Index++)
      {
         if(MCLList[Index] == MCLID)
         {
            /* Clear the ID from the list.                              */
            MCLList[Index] = 0;
            break;
         }
      }
   }
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
         printf("Return Value is %d GAP_Perform_Inquiry() FAILURE.\n", ret_val);
   }
   else
      ret_val = INVALID_STACK_ID_ERROR;

   return(ret_val);
}

   /* The following function is a utility function that exists to       */
   /* display the current Inquiry List (with Indexes).  This is useful  */
   /* in case the user has forgotten what Inquiry Index a particular    */
   /* Bluetooth Device was located in.  This function returns zero on   */
   /* successful execution and a negative value on all errors.          */
static int DisplayInquiryList(ParameterList_t *TempParam)
{
   int  ret_val;
   char BoardStr[13];
   int  Index;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Simply display all of the items in the Inquiry List.           */
      printf("Inquiry List: %d Devices%s\r\n\r\n", NumberofValidResponses, NumberofValidResponses?":":".");

      for(Index=0;Index<NumberofValidResponses;Index++)
      {
         BD_ADDRToStr(InquiryResultList[Index], BoardStr);

         printf("%sInquiry Result: %d, %s.\r\n", (MCLList[Index])?"*":" ", (Index+1), BoardStr);
      }

      if(NumberofValidResponses)
         printf("\r\n");

      /* All finished, flag success to the caller.                      */
      ret_val = 0;
   }
   else
      ret_val = INVALID_STACK_ID_ERROR;

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
      ret_val = INVALID_STACK_ID_ERROR;

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
         printf("Usage: SetConnectabilityMode [Mode (0 = Non Connectable, 1 = Connectable)].\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
      ret_val = INVALID_STACK_ID_ERROR;

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
      ret_val = INVALID_STACK_ID_ERROR;

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

         /* Finally map the Man in the Middle (MITM) Protection valid.  */
         MITMProtection = (Boolean_t)(TempParam->Params[1].intParam?TRUE:FALSE);

         /* Inform the user of the New I/O Capabilities.                */
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
      ret_val = INVALID_STACK_ID_ERROR;

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
      if(!HDPID)
      {
         /* There is no currently active connection, make sure that all */
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
               /* Display a message indicating that Bonding was         */
               /* initiated successfully.                               */
               printf("GAP_Initiate_Bonding (%s): Function Successful.\r\n", (BondingType == btDedicated)?"Dedicated":"General");

               /* Flag success to the caller.                           */
               ret_val = 0;
            }
            else
            {
               /* Display a message indicating that an error occurred   */
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
      ret_val = INVALID_STACK_ID_ERROR;

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
            /* Display a message indicating that the End bonding was    */
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
            /* Display a message indicating that an error occurred while*/
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
      ret_val = INVALID_STACK_ID_ERROR;

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
      ret_val = INVALID_STACK_ID_ERROR;

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
      ret_val = INVALID_STACK_ID_ERROR;

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
      ret_val = INVALID_STACK_ID_ERROR;

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
         /* Display a message indicating that an error occurred while   */
         /* attempting to query the Local Device Address.               */
         printf("GAP_Query_Local_BD_ADDR() Failure: %d.\r\n", Result);

         ret_val = FUNCTION_ERROR;
      }
   }
   else
      ret_val = INVALID_STACK_ID_ERROR;

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
            printf("Local Device Name set to: %s.\r\n", TempParam->Params[0].strParam);

            /* Rewrite extended inquiry information.                    */
            WriteEIRInformation(TempParam->Params[0].strParam);

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            /* Display a message indicating that an error occurred while*/
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
      ret_val = INVALID_STACK_ID_ERROR;

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
         /* Display a message indicating that an error occurred while   */
         /* attempting to query the Local Device Name.                  */
         printf("GAP_Query_Local_Device_Name() Failure: %d.\r\n", Result);

         ret_val = FUNCTION_ERROR;
      }
   }
   else
      ret_val = INVALID_STACK_ID_ERROR;

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
            printf("Set Class of Device to 0x%02X%02X%02X.\r\n", Class_of_Device.Class_of_Device0, Class_of_Device.Class_of_Device1, Class_of_Device.Class_of_Device2);

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            /* Display a message indicating that an error occurred while*/
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
      ret_val = INVALID_STACK_ID_ERROR;

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
         /* Display a message indicating that an error occurred while   */
         /* attempting to query the Local Class of Device.              */
         printf("GAP_Query_Class_Of_Device() Failure: %d.\r\n", Result);

         ret_val = FUNCTION_ERROR;
      }
   }
   else
      ret_val = INVALID_STACK_ID_ERROR;

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
            /* Display a message indicating that Remote Name request was*/
            /* initiated successfully.                                  */
            printf("GAP_Query_Remote_Device_Name: Function Successful.\r\n");

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            /* Display a message indicating that an error occurred while*/
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
      ret_val = INVALID_STACK_ID_ERROR;

   return(ret_val);
}

   /* The following function is responsible for the submission of an SDP*/
   /* request for a remote device.  There are 3 types of searches that  */
   /* can be performed: A search of all of the records, a search of the */
   /* HDP records, and a search of the Device Identification record.    */
   /* This function returns zero on successful execution and a negative */
   /* value on all errors.                                              */
static int SDPSearch(ParameterList_t *TempParam)
{
   int                           ret_val;
   int                           Type;
   int                           Index;
   BD_ADDR_t                     NullADDR;
   char                          BoardStr[13];
   SDP_UUID_Entry_t              SDP_UUID_Entry;
   SDP_Attribute_ID_List_Entry_t AttributeIDList;

   ASSIGN_BD_ADDR(NullADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 2) && (TempParam->Params[0].intParam) && (NumberofValidResponses) && (TempParam->Params[0].intParam <= NumberofValidResponses) && (!COMPARE_BD_ADDR(InquiryResultList[(TempParam->Params[0].intParam - 1)], NullADDR)))
      {
         Index = (TempParam->Params[0].intParam - 1);
         Type  = TempParam->Params[1].intParam;

         BD_ADDRToStr(InquiryResultList[Index], BoardStr);

         /* Assign the appropriate UUIDs to search for.                 */
         SDP_UUID_Entry.SDP_Data_Element_Type = deUUID_16;
         AttributeIDList.Attribute_Range      = TRUE;
         if(Type == SDP_SEARCH_TYPE_DID)
         {
            SDP_ASSIGN_PNP_INFORMATION_UUID_16(SDP_UUID_Entry.UUID_Value.UUID_16);
            AttributeIDList.Start_Attribute_ID = 0x0200;
            AttributeIDList.End_Attribute_ID   = 0x0300;
         }
         else
         {
            if(Type == SDP_SEARCH_TYPE_HDP_PROFILE)
            {
               SDP_ASSIGN_HEALTH_DEVICE_SERVICE_CLASS_UUID_16(SDP_UUID_Entry.UUID_Value.UUID_16);
            }
            else
            {
               SDP_ASSIGN_L2CAP_UUID_16(SDP_UUID_Entry.UUID_Value.UUID_16);
            }

            AttributeIDList.Start_Attribute_ID = 1;
            AttributeIDList.End_Attribute_ID   = 0x302;
         }

         /* Submit the SDP Request.                                     */
         ret_val = SDP_Service_Search_Attribute_Request(BluetoothStackID, InquiryResultList[Index], 1, &SDP_UUID_Entry, 1, &AttributeIDList, SDP_Event_Callback, Type);
         if(ret_val > 0)
         {
            printf("SDP Search In Progress for %s\r\n", BoardStr);
            ret_val = 0;
         }
         else
         {
            printf("SDP Search Request Failure %d\r\n", ret_val);

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         printf("Usage: SDPSearch [Inquiry Index] [Type 0=ALL, 1=HDP, 2=DID]\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
      ret_val = INVALID_STACK_ID_ERROR;

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
         Result = SDP_Service_Search_Attribute_Request(BluetoothStackID, InquiryResultList[(TempParam->Params[0].intParam - 1)], 1, &SDPUUIDEntry, 1, &AttributeID, SDP_Event_Callback, SDP_SEARCH_TYPE_GENERAL);

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

   /* The following function is responsible for registering an HDP      */
   /* instance.  The function receives the Control PSM, Data PSM, Role  */
   /* and the supported features.  This function returns zero on        */
   /* successful execution and a negative value on all errors.          */
static int RegisterHDPInstance(ParameterList_t *TempParam)
{
   int                i;
   int                ret_val;
   Word_t             ControlPSM;
   Word_t             DataPSM;
   DWord_t            ServiceRecordHandle;
   int                NumberOfEndpoints;
   Byte_t             SupportedProcedures;
   HDP_Device_Role_t  Role;
   HDP_MDEP_Info_t   *HDPMDEPInfo;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 3))
      {
         /* Verify that and instance is not already registered.         */
         if(!HDPID)
         {
            /* Extract the data from the parameters that were passed in.*/
            ControlPSM = (Word_t)TempParam->Params[0].intParam;
            DataPSM    = (Word_t)TempParam->Params[1].intParam;
            Role       = (HDP_Device_Role_t)TempParam->Params[2].intParam;

            /* Verify that the PSM values specified are valid values.   */
            printf("Control %04X Data %04X Role %d\r\n", ControlPSM, DataPSM, Role);
            if((L2CAP_PSM_VALID_PSM(ControlPSM)) && (L2CAP_PSM_VALID_PSM(DataPSM)) && (ControlPSM != DataPSM))
            {
               /* Check to see if the supported procedures include the  */
               /* Sync Profile.                                         */
               SupportedProcedures  = 0;
               if(TempParam->NumberofParameters >= 4)
               {
                  if(TempParam->Params[3].intParam)
                  {
                     SyncSupport          = TRUE;
                     SupportedProcedures |= (Byte_t)(MCAP_SUPPORTED_PROCEDURE_CLOCK_SYNC_PROTOCOL);
                     if((TempParam->NumberofParameters >= 5) && (TempParam->Params[4].intParam))
                     {
                        SyncMaster           = TRUE;
                        SupportedProcedures |= (Byte_t)(MCAP_SUPPORTED_PROCEDURE_CLOCK_SYNC_PROTOCOL_MASTER_ROLE);
                     }
                  }
               }

               /* Attempt to register an HDP Instance.                  */
               ret_val = HDP_Register_Instance(BluetoothStackID, ControlPSM, DataPSM, SupportedProcedures, HDP_Event_Callback, 0);
               if(ret_val > 0)
               {
                  /* Save the handle for the HDP Instance.              */
                  HDPID             = ret_val;
                  HDPMDEPInfo       = MDEP_Info;
                  IsSource          = (Boolean_t)(Role == drSource);
                  NumberOfEndpoints = NUM_MDEP_ENDPOINTS;
                  printf("HDP Instance Registered with ID %d.\r\n", HDPID);

                  /* Attempt to register endpoints for this instance.   */
                  for(i=0; i < NumberOfEndpoints; i++)
                  {
                     /* Set the Source/Sink type base on the currently  */
                     /* role.                                           */
                     HDPMDEPInfo->MDEP_Role = (Role == drSink)?drSink:drSource;
                     ret_val = HDP_Register_Endpoint(BluetoothStackID, HDPID, HDPMDEPInfo, HDP_Event_Callback, HDPID);
                     if(ret_val >= 0)
                     {
                        /* Add the Device Specialization ID to the MDEP */
                        /* List.                                        */
                        printf("Endpoint type %04X as %s with MDEP ID %02X Registered\r\n", HDPMDEPInfo->MDEP_DataType, ((HDPMDEPInfo->MDEP_Role)?"Sink":"Source"), HDPMDEPInfo->MDEP_ID);
                     }
                     else
                        printf("Failed to register endpoint. Error %d\r\n", ret_val);

                     HDPMDEPInfo++;
                  }

                  /* Register the SDP record that contains all of the   */
                  /* endpoints that were registered.                    */
                  ret_val = HDP_Register_SDP_Record(BluetoothStackID, HDPID, "HDP Sample", "Stonestreet One", &ServiceRecordHandle);
                  if(!ret_val)
                     printf("SDP Service Record Successfully added.\r\n");
                  else
                     printf("SDP Service Record failed with error %d.\r\n", ret_val);

                  /* We will manually accept each connection.           */
                  HDP_Set_Connection_Mode(BluetoothStackID, HDPID, hcmManualAccept);
               }
               else
                  printf("HDP Register Instance failed with %d\r\n", ret_val);
            }
            else
            {
               printf("Invalid PSM Value. %04X %04X\r\n", ControlPSM, DataPSM);

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            printf("Instance is already registered\r\n");

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         printf("RegisterHDPInstance [Control PSM] [Data PSM] [Role 0=Source,1=Sync] {{Sync Support 0-No,1-Yes} {Sync Master 0-No,1-Yes}}\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
      ret_val = INVALID_STACK_ID_ERROR;

   return(ret_val);
}

   /* The following function is responsible for UnRegisering the HDP    */
   /* Instance.  This function returns zero on successful execution and */
   /* a negative value on all errors.                                   */
static int UnRegisterHDPInstance(ParameterList_t *TempParam)
{
   int ret_val;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Next, make sure that an HDP Instance has been registered.      */
      if(HDPID)
      {
         ret_val = HDP_UnRegister_Instance(BluetoothStackID, HDPID);
         if(!ret_val)
         {
            printf("HDP Instance %d UnRegistered.\r\n", HDPID);
            HDPID = 0;
         }
         else
            printf("HDP UnRegister Instance failed with %d\r\n", ret_val);
      }
      else
      {
         printf("No Instance currently registered.\r\n");

         ret_val = FUNCTION_ERROR;
      }
   }
   else
      ret_val = INVALID_STACK_ID_ERROR;

   return(ret_val);
}

   /* The following function is responsible for making a HDP Control    */
   /* Channel connection to a remote HDP Device.  This function returns */
   /* zero on successful execution and a negative value on all errors.  */
static int ConnectRemoteInstance(ParameterList_t *TempParam)
{
   int       ret_val;
   int       Index;
   Word_t    ControlPSM;
   Word_t    DataPSM;
   BD_ADDR_t NullADDR;
   char      BoardStr[13];

   ASSIGN_BD_ADDR(NullADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Next, make sure that an HDP Instance has been registered.      */
      if(HDPID)
      {
         /* There is no currently active connection, make sure that all */
         /* of the parameters required for this function appear to be at*/
         /* least semi-valid.                                           */
         if((TempParam) && (TempParam->NumberofParameters >= 3) && (TempParam->Params[0].intParam) && (NumberofValidResponses) && (TempParam->Params[0].intParam <= NumberofValidResponses) && (!COMPARE_BD_ADDR(InquiryResultList[(TempParam->Params[0].intParam - 1)], NullADDR)))
         {
            Index = (TempParam->Params[0].intParam - 1);
            BD_ADDRToStr(InquiryResultList[Index], BoardStr);

            /* Check to see if we are already connect to this device.   */
            if(!MCLList[Index])
            {
               ControlPSM = (Word_t)TempParam->Params[1].intParam;
               DataPSM    = (Word_t)TempParam->Params[2].intParam;

               if((L2CAP_PSM_VALID_PSM(ControlPSM)) && (L2CAP_PSM_VALID_PSM(DataPSM)) && (ControlPSM != DataPSM))
               {
                  /* The above parameters are valid, inform the user    */
                  /* that the program is about to End Bonding with the  */
                  /* Remote Device.                                     */
                  printf("Connect to %s\r\n", BoardStr);
                  ret_val = HDP_Connect_Remote_Instance(BluetoothStackID, HDPID, InquiryResultList[Index], ControlPSM, DataPSM);
                  if(ret_val > 0)
                  {
                     /* Associate the MCLID with the BD_ADDR that is    */
                     /* being connected.                                */
                     MCLList[Index] = ret_val;

                     printf("HDP_Connect_Remote_Instance success. MCLID: %d\r\n", ret_val);

                     ret_val = 0;

                  }
                  else
                     printf("HDP_Connect_Remote_Instance failed with %d\r\n", ret_val);
               }
               else
               {
                  printf("Invalid PSM Value. %04X %04X\r\n", ControlPSM, DataPSM);

                  ret_val = FUNCTION_ERROR;
               }
            }
            else
            {
               printf("Control Channel to remote device already exists\r\n");

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* One or more of the necessary parameters is/are invalid.  */
            printf("Usage: ConnectRemoteInstance [Inquiry Index] [Control PSM] [Data PSM].\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         printf("Local Instance not registered\n");

         ret_val = FUNCTION_ERROR;
      }
   }
   else
      ret_val = INVALID_STACK_ID_ERROR;

   return(ret_val);
}

   /* The following function is responsible for disconnecting the HDP   */
   /* Control Channel to a remote device.  This function returns zero on*/
   /* successful execution and a negative value on all errors.          */
static int DisconnectControlChannel(ParameterList_t *TempParam)
{
   int       ret_val;
   int       Index;
   char      BoardStr[13];
   BD_ADDR_t NullADDR;

   ASSIGN_BD_ADDR(NullADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Next, make sure that an HDP Instance has been registered.      */
      if(HDPID)
      {
         /* There is no currently active connection, make sure that all */
         /* of the parameters required for this function appear to be at*/
         /* least semi-valid.                                           */
         if((TempParam) && (TempParam->NumberofParameters >= 1) && (TempParam->Params[0].intParam) && (NumberofValidResponses) && (TempParam->Params[0].intParam <= NumberofValidResponses) && (!COMPARE_BD_ADDR(InquiryResultList[(TempParam->Params[0].intParam - 1)], NullADDR)))
         {
            Index = (TempParam->Params[0].intParam - 1);
            BD_ADDRToStr(InquiryResultList[Index], BoardStr);

            /* Check to see if we are already connect to this device.   */
            if(MCLList[Index])
            {
               /* Request that the control channel be disconnected.     */
               ret_val = HDP_Close_Connection(BluetoothStackID, MCLList[Index]);
               if(!ret_val)
               {
                  /* Clear the Control Channel information from the     */
                  /* list.                                              */
                  MCLList[Index] = 0;
                  printf("HDP_Close_Connection success.  MCLID: %d\r\n", MCLList[Index]);
               }
               else
                  printf("HDP_Close_Connection failed with %d\r\n", ret_val);
            }
            else
            {
               printf("No Connection exists to Device %s\r\n", BoardStr);

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            printf("Usage: DisconnectControlChannel [Inquiry Index].\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         printf("Local Instance not registered\n");

         ret_val = FUNCTION_ERROR;
      }
   }
   else
      ret_val = INVALID_STACK_ID_ERROR;

   return(ret_val);
}

   /* The following function is responsible for the creation of a Data  */
   /* Channel between two HDP instances.  This function returns zero on */
   /* successful execution and a negative value on all errors.          */
static int CreateDataChannel(ParameterList_t *TempParam)
{
   int                       ret_val;
   int                       Index;
   Byte_t                    MDEP_ID;
   HDP_Device_Role_t         Role;
   HDP_Channel_Mode_t        Mode;
   HDP_Channel_Config_Info_t ConfigInfo;
   char                      BoardStr[13];
   BD_ADDR_t                 NullADDR;

   ASSIGN_BD_ADDR(NullADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Next, make sure that an HDP Instance has been registered.      */
      if(HDPID)
      {
         /* There is no currently active connection, make sure that all */
         /* of the parameters required for this function appear to be at*/
         /* least semi-valid.                                           */
         if((TempParam) && (TempParam->NumberofParameters >= 3) && (TempParam->Params[0].intParam) && (NumberofValidResponses) && (TempParam->Params[0].intParam <= NumberofValidResponses) && (!COMPARE_BD_ADDR(InquiryResultList[(TempParam->Params[0].intParam - 1)], NullADDR)))
         {
            /* Determine the address of the remote HDP Device.          */
            Index = (TempParam->Params[0].intParam - 1);
            BD_ADDRToStr(InquiryResultList[Index], BoardStr);

            /* Check to see if we are already connect to this device.   */
            if(MCLList[Index])
            {
               /* Extract and initialize the parameters for the         */
               /* requested data channel.                               */
               MDEP_ID                             = (Byte_t)TempParam->Params[1].intParam;
               Mode                                = (HDP_Channel_Mode_t)TempParam->Params[2].intParam;
               Role                                = (HDP_Device_Role_t)((IsSource)?drSource:drSink);

               ConfigInfo.FCSMode                  = (HDP_FCS_Mode_t)((IsSource)?fcsEnabled:fcsNoPreference);
               ConfigInfo.MaxTxPacketSize          = 2048;
               ConfigInfo.TxSegmentSize            = 256;
               ConfigInfo.NumberOfTxSegmentBuffers = 10;

               /* Send the request for the data channel.                */
               ret_val = HDP_Create_Data_Channel_Request(BluetoothStackID, MCLList[Index], (Byte_t)MDEP_ID, Role, (HDP_Channel_Mode_t)Mode, &ConfigInfo);
               if(ret_val > 0)
               {
                  printf("Create Data Channel Request success.  DataLinkID: %d MDEP ID:%d Mode: %d\r\n", ret_val, MDEP_ID, Mode);
                  ret_val = 0;
               }
               else
                  printf("HDP_Create_Data_Channel_Request failed with %d\r\n", ret_val);
            }
            else
            {
               printf("No Connection exists to Device %s\r\n", BoardStr);

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            printf("Usage: CreateDataChannel [Inquiry Index] [MDEP ID] [Mode 0=No Preference,1=Reliable,2=Streaming].\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         printf("Local Instance not registered\n");

         ret_val = FUNCTION_ERROR;
      }
   }
   else
      ret_val = INVALID_STACK_ID_ERROR;

   return(ret_val);
}

   /* The following function is responsible for the deletion and        */
   /* disconnection of a Data Channel.  This function returns zero on   */
   /* successful execution and a negative value on all errors.          */
static int DeleteDataChannel(ParameterList_t *TempParam)
{
   int                       ret_val;
   int                       Index;
   int                       DataLinkID;
   char                      BoardStr[13];
   BD_ADDR_t                 NullADDR;

   ASSIGN_BD_ADDR(NullADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Next, make sure that an HDP Instance has been registered.      */
      if(HDPID)
      {
         /* There is no currently active connection, make sure that all */
         /* of the parameters required for this function appear to be at*/
         /* least semi-valid.                                           */
         if((TempParam) && (TempParam->NumberofParameters >= 2) && (TempParam->Params[0].intParam) && (NumberofValidResponses) && (TempParam->Params[0].intParam <= NumberofValidResponses) && (!COMPARE_BD_ADDR(InquiryResultList[(TempParam->Params[0].intParam - 1)], NullADDR)))
         {
            Index = (TempParam->Params[0].intParam - 1);
            BD_ADDRToStr(InquiryResultList[Index], BoardStr);

            /* Check to see if we are already connect to this device.   */
            if(MCLList[Index])
            {
               /* Extract the DataLinkID that is to be deleted.  If the */
               /* DataLinkID is Zero, then all DataLinks will be        */
               /* deleted.                                              */
               DataLinkID = (TempParam->Params[1].intParam)?TempParam->Params[1].intParam:DATA_LINK_ALL_ID;
               ret_val    = HDP_Delete_Data_Channel(BluetoothStackID, MCLList[Index], DataLinkID);
               if(ret_val > 0)
               {
                  printf("Delete Data Channel Request success.  DataLinkID: %d\r\n", TempParam->Params[1].intParam);
                  ret_val = 0;
               }
               else
                  printf("HDP_Delete_Data_Channel_Request failed with %d\r\n", ret_val);
            }
            else
            {
               printf("No Connection exists to Device %s\r\n", BoardStr);

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            printf("Usage: DeleteDataChannel [Inquiry Index] [Datalink ID, 0=All].\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         printf("Local Instance not registered\n");

         ret_val = FUNCTION_ERROR;
      }
   }
   else
      ret_val = INVALID_STACK_ID_ERROR;

   return(ret_val);
}

   /* The following function is responsible for the sending of data on a*/
   /* specified Data Channel.  This function returns zero on successful */
   /* execution and a negative value on all errors.                     */
static int DataWrite(ParameterList_t *TempParam)
{
   int ret_val;
   int DataLinkID;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Next, make sure that an HDP Instance has been registered.      */
      if(HDPID)
      {
         /* There is no currently active connection, make sure that all */
         /* of the parameters required for this function appear to be at*/
         /* least semi-valid.                                           */
         if((TempParam) && (TempParam->NumberofParameters >= 2))
         {
            /* Extract the DataLinkID of the channel to send the data   */
            /* on.                                                      */
            DataLinkID = TempParam->Params[0].intParam;
            ret_val    = HDP_Write_Data(BluetoothStackID, DataLinkID, strlen(TempParam->Params[1].strParam), (Byte_t *)TempParam->Params[1].strParam);
            if(!ret_val)
            {
               printf("Data Write Request successfully sent %s \r\n", TempParam->Params[1].strParam);
               ret_val = 0;
            }
            else
               printf("HDP_Write_Data failed with %d\r\n", ret_val);
         }
         else
         {
            printf("Usage: WriteData [Datalink ID] [Message String].\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         printf("Local Instance not registered\n");

         ret_val = FUNCTION_ERROR;
      }
   }
   else
      ret_val = INVALID_STACK_ID_ERROR;

   return(ret_val);
}

   /* The following function is responsible for the sending an Abort    */
   /* Request on a specified Control Channel.  This function returns    */
   /* zero on successful execution and a negative value on all errors.  */
static int AbortRequest(ParameterList_t *TempParam)
{
   int       ret_val;
   int       Index;
   char      BoardStr[13];
   BD_ADDR_t NullADDR;

   ASSIGN_BD_ADDR(NullADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Next, make sure that an HDP Instance has been registered.      */
      if(HDPID)
      {
         /* There is no currently active connection, make sure that all */
         /* of the parameters required for this function appear to be at*/
         /* least semi-valid.                                           */
         if((TempParam) && (TempParam->NumberofParameters >= 1) && (TempParam->Params[0].intParam) && (NumberofValidResponses) && (TempParam->Params[0].intParam <= NumberofValidResponses) && (!COMPARE_BD_ADDR(InquiryResultList[(TempParam->Params[0].intParam - 1)], NullADDR)))
         {
            /* Get the Address of the device on which the Control       */
            /* Channel resides.                                         */
            Index = (TempParam->Params[0].intParam - 1);
            BD_ADDRToStr(InquiryResultList[Index], BoardStr);

            /* Check to see if we are already connect to this device.   */
            if(MCLList[Index])
            {
               /* Send the Abort of the associated Control Channel..    */
               ret_val = HDP_Abort_Data_Channel_Request(BluetoothStackID, MCLList[Index]);

               if(!ret_val)
                  printf("Abort Data Channel Request success\r\n");
               else
                  printf("HDP_Abort_Data_Channel_Request failed with %d\r\n", ret_val);
            }
            else
            {
               printf("No Connection exists to Device %s\r\n", BoardStr);

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            printf("Usage: AbortRequest [Inquiry Index].\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         printf("Local Instance not registered\n");

         ret_val = FUNCTION_ERROR;
      }
   }
   else
      ret_val = INVALID_STACK_ID_ERROR;

   return(ret_val);
}

   /* The following function is responsible for the sending an Sync     */
   /* Capabilities request to a remote device that supports the Sync    */
   /* Protocol.  This function returns zero on successful execution and */
   /* a negative value on all errors.                                   */
static int SyncCapabilitiesRequest(ParameterList_t *TempParam)
{
   int       ret_val;
   int       Index;
   char      BoardStr[13];
   Word_t    Accuracy;
   BD_ADDR_t NullADDR;

   ASSIGN_BD_ADDR(NullADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Next, make sure that an HDP Instance has been registered.      */
      if(HDPID)
      {
         /* Verify that this device supports the Sync Master Role.      */
         if(SyncMaster)
         {
            /* There is no currently active connection, make sure that  */
            /* all of the parameters required for this function appear  */
            /* to be at least semi-valid.                               */
            if((TempParam) && (TempParam->NumberofParameters >= 2) && (TempParam->Params[0].intParam) && (NumberofValidResponses) && (TempParam->Params[0].intParam <= NumberofValidResponses) && (!COMPARE_BD_ADDR(InquiryResultList[(TempParam->Params[0].intParam - 1)], NullADDR)))
            {
               /* Get the address of the remote device that is to       */
               /* receive the request.                                  */
               Index    = (TempParam->Params[0].intParam - 1);
               Accuracy = (Word_t)TempParam->Params[1].intParam;
               BD_ADDRToStr(InquiryResultList[Index], BoardStr);

               /* Check to see if we are already connect to this device.*/
               if(MCLList[Index])
               {
                  /* Send the request on the associated Control Channel.*/
                  ret_val = HDP_Sync_Capabilities_Request(BluetoothStackID, MCLList[Index], Accuracy);
                  if(!ret_val)
                  {
                     printf("HDP_Sync_Capabilities_Request success.\r\n");
                  }
                  else
                  {
                     printf("HDP_Sync_Capabilities_Request failed with %d\r\n", ret_val);
                  }
               }
               else
               {
                  printf("No Connection exists to Device %s\r\n", BoardStr);

                  ret_val = FUNCTION_ERROR;
               }
            }
            else
            {
               printf("Usage: SyncCapabilitiesRequest [Inquiry Index] [Accuracy (ppm)].\r\n");

               ret_val = INVALID_PARAMETERS_ERROR;
            }
         }
         else
         {
            printf("Local Instance does not support the Sync-Master Role\n");

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         printf("No HDP Instance registered\r\n");

         ret_val = FUNCTION_ERROR;
      }
   }
   else
      ret_val = INVALID_STACK_ID_ERROR;

   return(ret_val);
}

   /* The following function is responsible for the sending an Sync Set */
   /* Request to a remote device that supports the Sync Protocol.  This */
   /* function returns zero on successful execution and a negative value*/
   /* on all errors.                                                    */
static int SyncSetRequest(ParameterList_t *TempParam)
{
   int       ret_val;
   int       Index;
   char      BoardStr[13];
   BD_ADDR_t NullADDR;

   ASSIGN_BD_ADDR(NullADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Next, make sure that an HDP Instance has been registered.      */
      if(HDPID)
      {
         /* Verify that this device supports the Sync Master Role.      */
         if(SyncMaster)
         {
            /* There is no currently active connection, make sure that  */
            /* all of the parameters required for this function appear  */
            /* to be at least semi-valid.                               */
            if((TempParam) && (TempParam->NumberofParameters >= 1) && (TempParam->Params[0].intParam) && (NumberofValidResponses) && (TempParam->Params[0].intParam <= NumberofValidResponses) && (!COMPARE_BD_ADDR(InquiryResultList[(TempParam->Params[0].intParam - 1)], NullADDR)))
            {
               /* Get the address of the remote device that is to       */
               /* receive the request.                                  */
               Index    = (TempParam->Params[0].intParam - 1);
               BD_ADDRToStr(InquiryResultList[Index], BoardStr);

               /* Check to see if we are already connect to this device.*/
               if(MCLList[Index])
               {
                  /* Send the request on the associated Control Channel.*/
                  ret_val = HDP_Sync_Set_Request(BluetoothStackID, HDPID, TRUE, CLOCK_SYNC_NOW, 0);
                  if(!ret_val)
                  {
                     printf("HDP_Sync_Set_Request success.\r\n");
                  }
                  else
                  {
                     printf("HDP_Sync_Set_Request failed with %d\r\n", ret_val);
                  }
               }
               else
               {
                  printf("No Connection exists to Device %s\r\n", BoardStr);

                  ret_val = FUNCTION_ERROR;
               }
            }
            else
            {
               printf("Usage: SyncSetRequest [Inquiry Index].\r\n");

               ret_val = INVALID_PARAMETERS_ERROR;
            }
         }
         else
         {
            printf("Local Instance does not support the Sync-Master Role\n");

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         printf("No HDP Instance registered\r\n");

         ret_val = FUNCTION_ERROR;
      }
   }
   else
      ret_val = INVALID_STACK_ID_ERROR;

   return(ret_val);
}

   /* The following function is responsible for the sending an Sync     */
   /* Information Event to a remote device that supports the Sync       */
   /* Protocol.  This function returns zero on successful execution and */
   /* a negative value on all errors.                                   */
static int SyncInfo(ParameterList_t *TempParam)
{
   int       ret_val;
   int       Index;
   char      BoardStr[13];
   BD_ADDR_t NullADDR;
   Word_t    Accuracy;
   DWord_t   ClockValue;

   ASSIGN_BD_ADDR(NullADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Next, make sure that an HDP Instance has been registered.      */
      if(HDPID)
      {
         /* Verify that this device supports the Sync Role.             */
         if(SyncSupport)
         {
            /* There is no currently active connection, make sure that  */
            /* all of the parameters required for this function appear  */
            /* to be at least semi-valid.                               */
            if((TempParam) && (TempParam->NumberofParameters >= 1) && (TempParam->Params[0].intParam) && (NumberofValidResponses) && (TempParam->Params[0].intParam <= NumberofValidResponses) && (!COMPARE_BD_ADDR(InquiryResultList[(TempParam->Params[0].intParam - 1)], NullADDR)))
            {
               /* Get the address of the remote device that is to       */
               /* receive the request.                                  */
               Index    = (TempParam->Params[0].intParam - 1);
               BD_ADDRToStr(InquiryResultList[Index], BoardStr);

               /* Check to see if we are already connect to this device.*/
               if(MCLList[Index])
               {
                  /* Get the Current Bluetooth Clock Value from the     */
                  /* local Bluetooth device.                            */
                  ret_val = HDP_Sync_Get_Bluetooth_Clock_Value(BluetoothStackID, HDPID, &ClockValue, &Accuracy);
                  if(!ret_val)
                  {
                     /* Send the event on the associated Control        */
                     /* Channel.                                        */
                     ret_val = HDP_Sync_Info_Indication(BluetoothStackID, HDPID, ClockValue, 1000, 1);
                     if(!ret_val)
                     {
                        printf("HDP_Sync_Info_Indication success.\r\n");
                     }
                     else
                     {
                        printf("HDP_Sync_Info_Indication failed with %d\r\n", ret_val);
                     }
                  }
                  else
                     printf("Attempt to read Bluetooth Clock Failed with %d\r\n", ret_val);
               }
               else
               {
                  printf("No Connection exists to Device %s\r\n", BoardStr);

                  ret_val = FUNCTION_ERROR;
               }
            }
            else
            {
               printf("Usage: SyncInfo [Inquiry Index].\r\n");

               ret_val = INVALID_PARAMETERS_ERROR;
            }
         }
         else
         {
            printf("Local Instance does not support the Sync Role\n");

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         printf("No HDP Instance registered\r\n");

         ret_val = FUNCTION_ERROR;
      }
   }
   else
      ret_val = INVALID_STACK_ID_ERROR;

   return(ret_val);
}

   /* The following function is responsible for displaying information  */
   /* about the DID information that is contained in the SDP response   */
   /* data that is supplied.                                            */
static void ParseDIDResponse(SDP_Service_Search_Attribute_Response_Data_t *SDPResponse)
{
   int                                    NumRecords;
   int                                    NumAttributes;
   Word_t                                 WordValue;
   SDP_Service_Attribute_Response_Data_t *ServiceRecord;
   SDP_Service_Attribute_Value_Data_t    *Attribute;
   SDP_Data_Element_t                    *DataElement;

   /* Verify that the pointer is valid.                                 */
   if(SDPResponse)
   {
      /* Initialize the Data Structure.                                 */
      NumRecords    = SDPResponse->Number_Service_Records;
      ServiceRecord = SDPResponse->SDP_Service_Attribute_Response_Data;

      /* We searched for the DID UUID, so each record that was located  */
      /* is an DID record.                                              */
      while(NumRecords--)
      {
         /* Get the number of attributes that are in the record and set */
         /* a pointer to the first attribute.                           */
         NumAttributes = ServiceRecord->Number_Attribute_Values;
         Attribute     = ServiceRecord->SDP_Service_Attribute_Value_Data;
         while(NumAttributes--)
         {
            /* Get a pointer to the first data element for the current  */
            /* attribute.                                               */
            DataElement = Attribute->SDP_Data_Element;
            switch(Attribute->Attribute_ID)
            {
               case SDP_ATTRIBUTE_ID_DID_SPECIFICATION_ID:
                  /* Verify that the data element is what we expected.  */
                  if(DataElement->SDP_Data_Element_Type == deUnsignedInteger2Bytes)
                  {
                     WordValue = DataElement->SDP_Data_Element.UnsignedInteger2Bytes;
                     printf("Device ID Profile Version: %d.%d\r\n", (Byte_t)(WordValue >> 8), (Byte_t)WordValue);
                  }
                  break;
               case SDP_ATTRIBUTE_ID_DID_VENDOR_ID:
                  /* Verify that the data element is what we expected.  */
                  if(DataElement->SDP_Data_Element_Type == deUnsignedInteger2Bytes)
                  {
                     WordValue = DataElement->SDP_Data_Element.UnsignedInteger2Bytes;
                     printf("Vendor ID Value: 0x%04X\r\n", WordValue);
                  }
                  break;
               case SDP_ATTRIBUTE_ID_DID_PRODUCT_ID:
                  /* Verify that the data element is what we expected.  */
                  if(DataElement->SDP_Data_Element_Type == deUnsignedInteger2Bytes)
                  {
                     WordValue = DataElement->SDP_Data_Element.UnsignedInteger2Bytes;
                     printf("Vendor Porrect ID: 0x%04X\r\n", WordValue);
                  }
                  break;
               case SDP_ATTRIBUTE_ID_DID_VERSION:
                  /* Verify that the data element is what we expected.  */
                  if(DataElement->SDP_Data_Element_Type == deUnsignedInteger2Bytes)
                  {
                     WordValue = DataElement->SDP_Data_Element.UnsignedInteger2Bytes;
                     printf("Vendor Product Version: %d.%d.%d\r\n", (Byte_t)(WordValue >> 8), ((Byte_t)WordValue >> 4), (WordValue & 0x000F));
                  }
                  break;
               case SDP_ATTRIBUTE_ID_DID_PRIMARY_RECORD:
                  /* Verify that the data element is what we expected.  */
                  if(DataElement->SDP_Data_Element_Type == deBoolean)
                  {
                     WordValue = DataElement->SDP_Data_Element.Boolean;
                     printf("Primary Record: %s\r\n", (WordValue)?"Yes":"No");
                  }
                  break;
               case SDP_ATTRIBUTE_ID_DID_VENDOR_ID_SOURCE:
                  /* Verify that the data element is what we expected.  */
                  if(DataElement->SDP_Data_Element_Type == deUnsignedInteger2Bytes)
                  {
                     WordValue = DataElement->SDP_Data_Element.UnsignedInteger2Bytes;
                     printf("Vendor ID Source: %s", (WordValue == 1)?"Bluetooth SIG":"USB Implementer's Forum");
                  }
                  break;
            }
            /* Advance to the next Attribute in the sequence.           */
            Attribute++;
         }
         /* Advance to the next Service Record.                         */
         ServiceRecord++;
      }

   }
}

   /* The following function is responsible for displaying information  */
   /* about the HDP information that is contained in the SDP response   */
   /* data that is supplied.                                            */
static void ParseSDPResponse(SDP_Service_Search_Attribute_Response_Data_t *SDPResponse)
{
   int                                    NumRecords;
   int                                    NumAttributes;
   int                                    NumEndpoints;
   MDEP_Info_t                            MDEPInfo;
   Instance_Info_t                        InstanceInfo;
   SDP_Service_Attribute_Response_Data_t *ServiceRecord;
   SDP_Service_Attribute_Value_Data_t    *Attribute;
   SDP_Data_Element_t                    *DataElement1;
   SDP_Data_Element_t                    *DataElement2;
   SDP_Data_Element_t                    *DataElement3;

   /* Verify that the pointer is valid.                                 */
   if(SDPResponse)
   {
      /* Initialize the Data Structure.                                 */
      BTPS_MemInitialize(&InstanceInfo, 0, sizeof(Instance_Info_t));
      NumRecords    = SDPResponse->Number_Service_Records;
      ServiceRecord = SDPResponse->SDP_Service_Attribute_Response_Data;

      /* We searched for HDP UUID, so each record that was located is an*/
      /* HDP Instance.                                                  */
      while(NumRecords--)
      {
         /* Get the number of attributes that are in the record and set */
         /* a pointer to the first attribute.                           */
         NumAttributes = ServiceRecord->Number_Attribute_Values;
         Attribute     = ServiceRecord->SDP_Service_Attribute_Value_Data;
         while(NumAttributes--)
         {
            /* Get a pointer to the first data element for the current  */
            /* attribute.                                               */
            DataElement1 = Attribute->SDP_Data_Element;
            switch(Attribute->Attribute_ID)
            {
               case SDP_ATTRIBUTE_ID_PROTOCOL_DESCRIPTOR_LIST:
                  /* Verify that the data element is what we expected.  */
                  if((DataElement1->SDP_Data_Element_Type == deSequence) && (DataElement1->SDP_Data_Element_Length == 2))
                  {
                     /* The Protocol Descriptor List is a sequence of 2 */
                     /* Data Element sequences, where the 1st sequence  */
                     /* identifies L2CAP and the registered L2CAP PSM.  */
                     DataElement1 = DataElement1->SDP_Data_Element.SDP_Data_Element_Sequence;
                     if((DataElement1->SDP_Data_Element_Type == deSequence) && (DataElement1->SDP_Data_Element_Length == 2))
                     {
                        /* Get a pointer to the first data element and  */
                        /* skip over the L2CAP UUID element.            */
                        DataElement2 = DataElement1->SDP_Data_Element.SDP_Data_Element_Sequence;
                        DataElement2++;
                        if(DataElement2->SDP_Data_Element_Type == deUnsignedInteger2Bytes)
                        {
                           /* Read the PSM Value for the Control        */
                           /* Channel.                                  */
                           InstanceInfo.ControlPSM = DataElement2->SDP_Data_Element.UnsignedInteger2Bytes;
                           printf("Control PSM: %04X\r\n", InstanceInfo.ControlPSM);
                        }
                     }
                     /* The second sequence contains the UUID of the    */
                     /* MCAP Control Channel and the MCAP Version.      */
                     DataElement1++;
                     if((DataElement1->SDP_Data_Element_Type == deSequence) && (DataElement1->SDP_Data_Element_Length == 2))
                     {
                        /* Get a pointer to the first data element and  */
                        /* skip over the MCAP Control Channel UUID      */
                        /* element.                                     */
                        DataElement2 = DataElement1->SDP_Data_Element.SDP_Data_Element_Sequence;
                        DataElement2++;
                        if(DataElement2->SDP_Data_Element_Type == deUnsignedInteger2Bytes)
                        {
                           /* Read the MCAP Version Information.        */
                           InstanceInfo.Version = DataElement2->SDP_Data_Element.UnsignedInteger2Bytes;
                           printf("Version: %04X\r\n", InstanceInfo.Version);
                        }
                     }
                  }
                  break;
               case SDP_ATTRIBUTE_ID_ADDITIONAL_PROTOCOL_DESCRIPTOR_LISTS:
                  /* Verify that the data element is what we expected.  */
                  if((DataElement1->SDP_Data_Element_Type == deSequence) && (DataElement1->SDP_Data_Element_Length == 1))
                  {
                     /* The Additional Protocol Descriptor List is a    */
                     /* sequence of 1 Data Element sequence, where the  */
                     /* sequence identifies L2CAP and the registered    */
                     /* L2CAP PSM.                                      */
                     DataElement1 = DataElement1->SDP_Data_Element.SDP_Data_Element_Sequence;
                     if((DataElement1->SDP_Data_Element_Type == deSequence) && (DataElement1->SDP_Data_Element_Length == 2))
                     {
                        /* Get a pointer to the first data element and  */
                        /* skip over the L2CAP UUID element.            */
                        DataElement2 = DataElement1->SDP_Data_Element.SDP_Data_Element_Sequence;
                        if((DataElement2->SDP_Data_Element_Type == deSequence) && (DataElement2->SDP_Data_Element_Length == 2))
                        {
                           DataElement3 = DataElement2->SDP_Data_Element.SDP_Data_Element_Sequence;
                           if(DataElement3->SDP_Data_Element_Type == deUUID_16)
                           {
                              DataElement3++;
                              if(DataElement3->SDP_Data_Element_Type == deUnsignedInteger2Bytes)
                              {
                                 /* Read the PSM Value for the Control  */
                                 /* Channel.                            */
                                 InstanceInfo.DataPSM = DataElement3->SDP_Data_Element.UnsignedInteger2Bytes;
                                 printf("Data PSM: %04X\r\n", InstanceInfo.DataPSM);
                              }
                           }
                        }
                     }
                  }
                  break;
               case SDP_ATTRIBUTE_ID_HDP_SUPPORTED_FEATURES:
                  /* Verify that the data element is what we expected.  */
                  if((DataElement1->SDP_Data_Element_Type == deSequence) && (DataElement1->SDP_Data_Element_Length >= 1))
                  {
                     /* Set a pointer to the first endpoint info        */
                     /* structure and set the number of endpoints that  */
                     /* are listed in the sequence.                     */
                     NumEndpoints = DataElement1->SDP_Data_Element_Length;

                     /* Set a pointer to the first sequence of endpoint */
                     /* information.                                    */
                     DataElement1 = DataElement1->SDP_Data_Element.SDP_Data_Element_Sequence;
                     while(NumEndpoints--)
                     {
                        /* Each endpoint should have 3 or 4 entries.    */
                        if((DataElement1->SDP_Data_Element_Type == deSequence) && (DataElement1->SDP_Data_Element_Length >= 3))
                        {
                           /* Get a pointer to the first data element.  */
                           DataElement2 = DataElement1->SDP_Data_Element.SDP_Data_Element_Sequence;
                           if(DataElement2->SDP_Data_Element_Type == deUnsignedInteger1Byte)
                           {
                              /* Verify and save the MDEP ID.           */
                              MDEPInfo.MDEP_ID = DataElement2->SDP_Data_Element.UnsignedInteger1Byte;
                              DataElement2++;
                              if(DataElement2->SDP_Data_Element_Type == deUnsignedInteger2Bytes)
                              {
                                /* Verify and save the Data Type ID.    */
                                 MDEPInfo.DataType = DataElement2->SDP_Data_Element.UnsignedInteger2Bytes;
                                 DataElement2++;
                                 if(DataElement2->SDP_Data_Element_Type == deUnsignedInteger1Byte)
                                {
                                   /* Verify and save the device Role.  */
                                    MDEPInfo.Role = DataElement2->SDP_Data_Element.UnsignedInteger1Byte;
                                    printf("MDEP_ID: %d DataType: %04X Role:%d\r\n", MDEPInfo.MDEP_ID, MDEPInfo.DataType, MDEPInfo.Role);
                                 }
                              }
                           }
                        }
                        /* Advance to the next data endpoint sequence   */
                        /* and info structure.                          */
                        DataElement1++;
                     }
                  }
                  break;
               case SDP_ATTRIBUTE_ID_HDP_MCAP_SUPPORTED_PROCEDURES:
                  /* Verify that the data element is what we expected.  */
                  if(DataElement1->SDP_Data_Element_Type == deUnsignedInteger1Byte)
                     InstanceInfo.SupportedProcedures = DataElement1->SDP_Data_Element.UnsignedInteger1Byte;
                  printf("Supported Procedures %02X\r\n", InstanceInfo.SupportedProcedures);
                  printf("Reconnect Initiation Support: %s\r\n", (InstanceInfo.SupportedProcedures & MCAP_SUPPORTED_PROCEDURE_RECONNECT_INITIATION)?"Yes":"No");
                  printf("Reconnect Acceptance Support: %s\r\n", (InstanceInfo.SupportedProcedures & MCAP_SUPPORTED_PROCEDURE_RECONNECT_ACCEPTANCE)?"Yes":"No");
                  printf("Sync Slave Support: %s\r\n", (InstanceInfo.SupportedProcedures & MCAP_SUPPORTED_PROCEDURE_CLOCK_SYNC_PROTOCOL)?"Yes":"No");
                  printf("Sync Master Support: %s\r\n", (InstanceInfo.SupportedProcedures & MCAP_SUPPORTED_PROCEDURE_CLOCK_SYNC_PROTOCOL_MASTER_ROLE)?"Yes":"No");
                  break;

            }
            /* Advance to the next Attribute in the sequence.           */
            Attribute++;
         }
         /* Advance to the next Service Record.                         */
         ServiceRecord++;
      }
   }
}

   /* The following function is responsible for displaying information  */
   /* about the Attributes that is specified in the SDP response.  The  */
   /* information is indented as specified by the InitLevel Parameter.  */
static void DisplaySDPAttributeResponse(SDP_Service_Attribute_Response_Data_t *SDPServiceAttributeResponse, unsigned int InitLevel)
{
   Word_t Index;

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
      printf("No SDP Attributes Found\r\n");
}

   /* The following function is responsible for processing the SDP      */
   /* Service Search Attribute Response data.                           */
static void DisplaySDPSearchAttributeResponse(SDP_Service_Search_Attribute_Response_Data_t *SDPServiceSearchAttributeResponse)
{
   Word_t Index;

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

   /* The following function is responsible for displaying information  */
   /* about the SDP Data Type that is specified.  The information is    */
   /* indented as specified by the Level Parameter.                     */
static void DisplayDataElement(SDP_Data_Element_t *SDPDataElement, unsigned int Level)
{
   char         Buffer[256];
   unsigned int Index;

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
         printf("%*s Type: Unsigned Int = 0x%08lX\r\n", (Level*INDENT_LENGTH), "", SDPDataElement->SDP_Data_Element.UnsignedInteger4Bytes);
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
         printf("%*s Type: Signed Int = 0x%04X", (Level*INDENT_LENGTH), "", SDPDataElement->SDP_Data_Element.SignedInteger2Bytes);
         break;
      case deSignedInteger4Bytes:
         /* Display the Signed Integer (4 Bytes) Type.                  */
         printf("%*s Type: Signed Int = 0x%08lX\r\n", (Level*INDENT_LENGTH), "", SDPDataElement->SDP_Data_Element.SignedInteger4Bytes);
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
   int                               ret_val;
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
                  ret_val = GAP_Authentication_Response(BluetoothStackID, GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device, &GAP_Authentication_Information);

                  /* Check the result of the submitted command.         */
                  if(!ret_val)
                     printf("GAP_Authentication_Response() Success.\r\n");
                  else
                     printf("GAP_Authentication_Response() Failure: %d.\r\n", ret_val);
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
                  /* Check to see if there was an error.  If so, then   */
                  /* Flag that there is no longer a current             */
                  /* Authentication procedure in progress.              */
                  ASSIGN_BD_ADDR(CurrentRemoteBD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

                  /* Delete any stored link key.                        */
                  if(GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Authentication_Event_Data.Authentication_Status)
                  {
                     for(Index=0;Index<(sizeof(LinkKeyInfo)/sizeof(LinkKeyInfo_t));Index++)
                     {
                        if(COMPARE_BD_ADDR(LinkKeyInfo[Index].BD_ADDR, GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device))
                        {
                           LinkKeyInfo[Index].BD_ADDR = CurrentRemoteBD_ADDR;
                           break;
                        }
                     }
                  }
                  break;
               case atLinkKeyCreation:
                  /* A link key creation event occurred, first display  */
                  /* the remote device that caused this event.          */
                  BD_ADDRToStr(GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device, BoardStr);
                  printf("atLinkKeyCreation: %s\r\n", BoardStr);

                  /* The BD_ADDR of the remote device has been displayed*/
                  /* now display the link key being created.            */
                  printf("Link Key: 0x\r\n");

                  for(Index = 0;Index<sizeof(Link_Key_t);Index++)
                     printf("%02X", ((Byte_t *)(&(GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Authentication_Event_Data.Link_Key_Info.Link_Key)))[Index]);

                  printf("\r\n");

                  /* Now store the link Key in either a free location OR*/
                  /* over the old key location.                         */
                  ASSIGN_BD_ADDR(NULL_BD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

                  for(Index=0,ret_val=-1;Index<(sizeof(LinkKeyInfo)/sizeof(LinkKeyInfo_t));Index++)
                  {
                     if(COMPARE_BD_ADDR(LinkKeyInfo[Index].BD_ADDR, GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device))
                        break;
                     else
                     {
                        if((ret_val == (-1)) && (COMPARE_BD_ADDR(LinkKeyInfo[Index].BD_ADDR, NULL_BD_ADDR)))
                           ret_val = Index;
                     }
                  }

                  /* If we didn't find a match, see if we found an empty*/
                  /* location.                                          */
                  if(Index == (sizeof(LinkKeyInfo)/sizeof(LinkKeyInfo_t)))
                     Index = ret_val;

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
                  ret_val = GAP_Authentication_Response(BluetoothStackID, GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device, &GAP_Authentication_Information);

                  /* Check the result of the submitted command.         */
                  /* Check the result of the submitted command.         */
                  if(!ret_val)
                     printf("GAP_Authentication_Response() Success.\r\n");
                  else
                     printf("GAP_Authentication_Response() Failure: %d.\r\n", ret_val);
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

                     ret_val = GAP_Authentication_Response(BluetoothStackID, GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device, &GAP_Authentication_Information);

                     if(!ret_val)
                        printf("GAP_Authentication_Response() Success.\r\n");
                     else
                        printf("GAP_Authentication_Response() Failure: %d.\r\n", ret_val);

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

                  ret_val = GAP_Authentication_Response(BluetoothStackID, GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device, &GAP_Authentication_Information);

                  if(!ret_val)
                     printf("GAP_Authentication_Response() Success.\r\n");
                  else
                     printf("GAP_Authentication_Response() Failure: %d.\r\n", ret_val);
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
         case etEncryption_Change_Result:
            break;
         default:
            /* An unknown/unexpected GAP event was received.            */
            printf("Unknown/Unhandled GAP Event: %d.\n", GAP_Event_Data->Event_Data_Type);
            break;
      }

      printf("\r\nHDP>");
   }
   else
   {
      /* There was an error with one or more of the input parameters.   */
      printf("\r\n");

      printf("GAP Callback Data: Event_Data = NULL.\r\n");

      printf("\r\nHDP>");
   }

   /* Make sure the output is displayed to the user.                    */
   fflush(stdout);
}

   /* The following function is for the SDP Event Data Callback.  This  */
   /* function will be called whenever an SDP Result has been received  */
   /* by the device that is associated with the Bluetooth Stack.  This  */
   /* function passes to the caller the SDP Event Data of the specified */
   /* Event and the SDP Event Callback Parameter that was specified when*/
   /* the SDP request was made.  The caller is free to use the contents */
   /* of the SDP Event Data ONLY in the context of this callback.  If   */
   /* the caller requires the Data for a longer period of time, then the*/
   /* callback function MUST copy the data into another Data Buffer.    */
   /* This function is guaranteed NOT to be invoked more than once      */
   /* simultaneously for the specified installed callback (i.e.  this   */
   /* function DOES NOT have be reentrant).  It Needs to be noted       */
   /* however, that if the same Callback is installed more than once,   */
   /* then the callbacks will be called serially.  Because of this, the */
   /* processing in this function should be as efficient as possible.   */
   /* It should also be noted that this function is called in the Thread*/
   /* Context of a Thread that the User does NOT own.  Therefore,       */
   /* processing in this function should be as efficient as possible    */
   /* (this argument holds anyway because other SDP Events will not be  */
   /* processed while this function call is outstanding).               */
   /* * NOTE * This function MUST NOT Block and wait for events that can*/
   /*          only be satisfied by Receiving other SDP Events.  A      */
   /*          Deadlock WILL occur because NO SDP Event Callbacks will  */
   /*          be issued while this function is currently outstanding.  */
static void BTPSAPI SDP_Event_Callback(unsigned int BluetoothStackID, unsigned int SDPRequestID, SDP_Response_Data_t *SDP_Response_Data, unsigned long CallbackParameter)
{
   int ret_val = 0;

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
            printf("SDP Timeout Received (Size = 0x%04X).\r\n", sizeof(SDP_Response_Data_t));
            break;
         case rdConnectionError:
            /* A SDP Connection Error was received, display a message   */
            /* indicating this.                                         */
            printf("SDP Connection Error Received (Size = 0x%04X).\r\n", sizeof(SDP_Response_Data_t));
            break;
         case rdErrorResponse:
            /* A SDP error response was received, display all relevant  */
            /* information regarding this event.                        */
            printf("SDP Error Response Received (Size = 0x%04X) - Error Code: %d.\r\n", sizeof(SDP_Response_Data_t), SDP_Response_Data->SDP_Response_Data.SDP_Error_Response_Data.Error_Code);
            break;
         case rdServiceSearchAttributeResponse:
            /* A SDP Service Search Attribute Response was received,    */
            /* display all relevant information regarding this event    */
            printf("SDP Service Search Attribute Response Received (Size = 0x%04X)\r\n", sizeof(SDP_Response_Data_t));
            switch(CallbackParameter)
            {
               case SDP_SEARCH_TYPE_GENERAL:
                  DisplaySDPSearchAttributeResponse(&SDP_Response_Data->SDP_Response_Data.SDP_Service_Search_Attribute_Response_Data);
                  break;
               case SDP_SEARCH_TYPE_HDP_PROFILE:
                  ParseSDPResponse(&SDP_Response_Data->SDP_Response_Data.SDP_Service_Search_Attribute_Response_Data);
                  break;
               case SDP_SEARCH_TYPE_DID:
                  ParseDIDResponse(&SDP_Response_Data->SDP_Response_Data.SDP_Service_Search_Attribute_Response_Data);
                  break;
            }
            break;
         default:
            /* An unknown/unexpected SDP event was received.            */
            printf("Unknown SDP Event.\r\n");
            break;
      }

      /* Check the return value of any function that might have been    */
      /* executed in the callback.                                      */
      if(ret_val)
      {
         /* There was an error while executing the function.            */
         printf("Error %d.\r\n", ret_val);
      }
   }
   else
   {
      /* There was an error with one or more of the input parameters.   */
      printf("SDP callback data: Response_Data = NULL.\r\n");
   }
   /* Output an Input Shell-type prompt.                                */
   printf("\r\nHDP>");

   /* Make sure the output is displayed to the user.                    */
   fflush(stdout);
}

   /* The following function is for the HDP Event Data Callback.  This  */
   /* function will be called whenever an HDP Events has been received  */
   /* by the device that is associated with the Bluetooth Stack.  This  */
   /* function passes to the caller the HDP Event Data of the specified */
   /* Event and the HDP Event Callback Parameter that was specified when*/
   /* the HDP Instance was registered.  The caller is free to use the   */
   /* contents of the HDP Event Data ONLY in the context of this        */
   /* callback.  If the caller requires the Data for a longer period of */
   /* time, then the callback function MUST copy the data into another  */
   /* Data Buffer.  This function is guaranteed NOT to be invoked more  */
   /* than once simultaneously for the specified installed callback     */
   /* (i.e.  this function DOES NOT have be reentrant).  It Needs to be */
   /* noted however, that if the same Callback is installed more than   */
   /* once, then the callbacks will be called serially.  Because of     */
   /* this, the processing in this function should be as efficient as   */
   /* possible.  It should also be noted that this function is called in*/
   /* the Thread Context of a Thread that the User does NOT own.        */
   /* Therefore, processing in this function should be as efficient as  */
   /* possible (this argument holds anyway because other HDP Events will*/
   /* not be processed while this function call is outstanding).        */
   /* * NOTE * This function MUST NOT Block and wait for events that can*/
   /*          only be satisfied by Receiving other HDP Events.  A      */
   /*          Deadlock WILL occur because NO HDP Event Callbacks will  */
   /*          be issued while this function is currently outstanding.  */
static void BTPSAPI HDP_Event_Callback(unsigned int BluetoothStackID, HDP_Event_Data_t *HDP_Event_Data, unsigned long CallbackParameter)
{
   int                       Index;
   int                       Status;
   unsigned int              HDPID;
   unsigned int              MCLID;
   int                       DataLinkID;
   Word_t                    DataLength;
   Byte_t                    ResponseCode;
   Word_t                    RequiredAccuracy;
   Byte_t                    MDEPID;
   BD_ADDR_t                 BD_ADDR;
   HDP_Channel_Mode_t        Mode;
   HDP_Channel_Config_Info_t ConfigInfo;
   DWord_t                   ClockValue;
   Word_t                    Accuracy;

   if((BluetoothStackID) && (HDP_Event_Data))
   {
      switch(HDP_Event_Data->Event_Data_Type)
      {
         case etHDP_Connect_Request_Indication:
            printf("etHDP_Connect_Request_Indication");
            HDPID   = HDP_Event_Data->Event_Data.HDP_Connect_Request_Indication_Data->HDPInstanceID;
            MCLID   = HDP_Event_Data->Event_Data.HDP_Connect_Request_Indication_Data->MCLID;
            BD_ADDR = HDP_Event_Data->Event_Data.HDP_Control_Connect_Indication_Data->BD_ADDR;
            printf("etHDP_Connect_Request_Indication on Instance %d, MCLID %d\r\n", HDPID, MCLID);
            HDP_Connect_Request_Response(BluetoothStackID, HDPID, MCLID, TRUE);
            for(Index = 0; Index < NumberofValidResponses; Index++)
            {
               if(COMPARE_BD_ADDR(InquiryResultList[Index], BD_ADDR))
               {
                  break;
               }
            }
            if(Index == NumberofValidResponses)
            {
               InquiryResultList[Index] = BD_ADDR;
               NumberofValidResponses++;
            }
            break;
         case etHDP_Control_Connect_Indication:
            MCLID   = HDP_Event_Data->Event_Data.HDP_Control_Connect_Indication_Data->MCLID;
            BD_ADDR = HDP_Event_Data->Event_Data.HDP_Control_Connect_Indication_Data->BD_ADDR;
            for(Index = 0; Index < NumberofValidResponses; Index++)
            {
               if(COMPARE_BD_ADDR(InquiryResultList[Index], BD_ADDR))
               {
                  /* Associate the MCLID with the BD_ADDR of the remote */
                  /* device.                                            */
                  MCLList[Index] = MCLID;
                  break;
               }
            }
            DataLinkCount = 0;
            printf("etHDP_Control_Connect_Indication: MCLID %d\r\n", MCLID);
            break;
         case etHDP_Control_Connect_Confirmation:
            MCLID  = HDP_Event_Data->Event_Data.HDP_Control_Connect_Confirmation_Data->MCLID;
            Status = HDP_Event_Data->Event_Data.HDP_Control_Connect_Confirmation_Data->Status;
            printf("Connection Status: %04X (%s)\r\n", Status, (Status <= MAX_CONNECT_RESPONSE_STRING_INDEX)?ConnectResponseString[Status]:"Unsupported Response");
            if(Status)
               ClearConnection(MCLID);
            break;
         case etHDP_Control_Disconnect_Indication:
            MCLID = HDP_Event_Data->Event_Data.HDP_Control_Disconnect_Indication_Data->MCLID;
            printf("Disconnect Indication: MCLID %d\r\n", MCLID);
            break;
         case etHDP_Control_Create_Data_Link_Indication:
            printf("etHDP_Control_Create_Data_Link_Indication\r\n");
            MCLID      = HDP_Event_Data->Event_Data.HDP_Control_Create_Data_Link_Indication_Data->MCLID;
            DataLinkID = HDP_Event_Data->Event_Data.HDP_Control_Create_Data_Link_Indication_Data->DataLinkID;
            Mode       = HDP_Event_Data->Event_Data.HDP_Control_Create_Data_Link_Indication_Data->ChannelMode;
            MDEPID     = HDP_Event_Data->Event_Data.HDP_Control_Create_Data_Link_Indication_Data->MDEPID;
            printf("   MCLID: %d\r\n", MCLID);
            printf("   DataLinkID: %d\r\n", DataLinkID);
            printf("   MDEPID: %d\r\n", MDEPID);
            printf("   Mode: %d\r\n", Mode);

            ConfigInfo.FCSMode                  = fcsEnabled;
            ConfigInfo.MaxTxPacketSize          = 2048;
            ConfigInfo.TxSegmentSize            = 256;
            ConfigInfo.NumberOfTxSegmentBuffers = 10;

            if((Role == drSource) && (Mode == cmNoPreference))
            {
               /* Create a reliable channel on the first connection and */
               /* a streaming channel on each additional channel.       */
               if(DataLinkCount)
                  Mode = cmStreaming;
               else
                  Mode = cmReliable;

               ResponseCode = HDP_RESPONSE_CODE_SUCCESS;
            }
            else
            {
               if((Role == drSink) && (Mode != cmNoPreference))
               {
                  if((DataLinkCount) || ((!DataLinkCount) && (Mode == cmReliable)))
                     ResponseCode = HDP_RESPONSE_CODE_SUCCESS;
                  else
                     ResponseCode = HDP_RESPONSE_CODE_CONFIGURATION_REJECTED;
               }
               else
                  ResponseCode = HDP_RESPONSE_CODE_CONFIGURATION_REJECTED;
            }
            printf("ResponseCode %d\r\n", ResponseCode);

            if(ResponseCode == HDP_RESPONSE_CODE_SUCCESS)
            {
               DataLinkCount++;
               HDP_Create_Data_Channel_Response(BluetoothStackID, DataLinkID, ResponseCode, Mode, &ConfigInfo);
            }
            else
               HDP_Create_Data_Channel_Response(BluetoothStackID, DataLinkID, ResponseCode, Mode, NULL);
            break;
         case etHDP_Control_Create_Data_Link_Confirmation:
            ResponseCode = HDP_Event_Data->Event_Data.HDP_Control_Create_Data_Link_Confirmation_Data->ResponseCode;
            if(ResponseCode == HDP_RESPONSE_CODE_SUCCESS)
               DataLinkCount++;
            printf("etHDP_Control_Create_Data_Link_Confirmation Result: %02X\r\n", ResponseCode);
            break;
         case etHDP_Control_Abort_Data_Link_Indication:
            printf("etHDP_Control_Abort_Data_Link_Indication\r\n");
            break;
         case etHDP_Control_Abort_Data_Link_Confirmation:
            printf("etHDP_Control_Abort_Data_Link_Confirmation\r\n");
            break;
         case etHDP_Control_Delete_Data_Link_Indication:
            DataLinkID = HDP_Event_Data->Event_Data.HDP_Control_Delete_Data_Link_Indication_Data->DataLinkID;
            if(DataLinkID == DATA_LINK_ALL_ID)
               printf("etHDP_Control_Delete_Data_Link_Indication for all Data Links\r\n");
            else
               printf("etHDP_Control_Delete_Data_Link_Indication for DatatLinkID %d\r\n", DataLinkID);
            break;
         case etHDP_Control_Delete_Data_Link_Confirmation:
            ResponseCode = HDP_Event_Data->Event_Data.HDP_Control_Delete_Data_Link_Confirmation_Data->ResponseCode;
            DataLinkID   = HDP_Event_Data->Event_Data.HDP_Control_Delete_Data_Link_Confirmation_Data->DataLinkID;
            printf("etHDP_Control_Delete_Data_Link_Confirmation for DataLinkID %d\r\n", DataLinkID);
            printf("  Response code %d\r\n", ResponseCode);
            break;
         case etHDP_Data_Link_Connect_Indication:
            printf("etHDP_Data_Link_Connect_Indication\r\n");
            DataLinkID = HDP_Event_Data->Event_Data.HDP_Data_Link_Connect_Indication_Data->DataLinkID;
            printf("Connection for DataLinkID %d\r\n", DataLinkID);
            break;
         case etHDP_Data_Link_Connect_Confirmation:
            printf("etHDP_Data_Link_Connect_Confirmation\r\n");
            Status     = HDP_Event_Data->Event_Data.HDP_Data_Link_Connect_Confirmation_Data->Status;
            DataLinkID = HDP_Event_Data->Event_Data.HDP_Data_Link_Connect_Confirmation_Data->DataLinkID;
            if(Status == HDP_OPEN_STATUS_SUCCESS)
               printf("Connection Complete for DataLinkID %d\r\n", DataLinkID);
            else
               printf("Connection Failed for DataLinkID %d with error %d\r\n", DataLinkID, Status);
            break;
         case etHDP_Data_Link_Disconnect_Indication:
            printf("etHDP_Data_Link_Disconnect_Indication\r\n");
            MCLID      = HDP_Event_Data->Event_Data.HDP_Data_Link_Disconnect_Indication_Data->MCLID;
            DataLinkID = HDP_Event_Data->Event_Data.HDP_Data_Link_Disconnect_Indication_Data->DataLinkID;
            printf("Data Channel Disconnect for MCLID %d DataLinkID %d\r\n", MCLID, DataLinkID);
            break;
         case etHDP_Data_Link_Data_Indication:
            printf("etHDP_Data_Link_Data_Indication\r\n");
            DataLinkID = HDP_Event_Data->Event_Data.HDP_Data_Link_Data_Indication_Data->DataLinkID;
            DataLength = HDP_Event_Data->Event_Data.HDP_Data_Link_Data_Indication_Data->DataLength;
            printf("%d bytes received on DataLinkID %d\r\n", DataLength, DataLinkID);
            break;
         case etHDP_Sync_Capabilities_Indication:
            printf("etHDP_Sync_Capabilities_Indication\r\n");
            MCLID            = HDP_Event_Data->Event_Data.HDP_Sync_Capabilities_Indication_Data->MCLID;
            RequiredAccuracy = HDP_Event_Data->Event_Data.HDP_Sync_Capabilities_Indication_Data->RequiredAcuracy;
            printf("RequiredAccuracy: %d\r\n", RequiredAccuracy);
            HDP_Sync_Capabilities_Response(BluetoothStackID, MCLID, 1, 1, 313, 250, HDP_RESPONSE_CODE_SUCCESS);
            break;
         case etHDP_Sync_Capabilities_Confirmation:
            printf("etHDP_Sync_Capabilities_Confirmation\r\n");
            if(HDP_Event_Data->Event_Data.HDP_Sync_Capabilities_Confirmation_Data->ResponseCode == HDP_RESPONSE_CODE_SUCCESS)
            {
               printf("MCLID: %d\r\n", HDP_Event_Data->Event_Data.HDP_Sync_Capabilities_Confirmation_Data->MCLID);
               printf("Lead Time: %d\r\n", HDP_Event_Data->Event_Data.HDP_Sync_Capabilities_Confirmation_Data->SyncLeadTime);
               printf("Resolution: %d\r\n", HDP_Event_Data->Event_Data.HDP_Sync_Capabilities_Confirmation_Data->AccessResolution);
               printf("Timestamp Resolution: %d\r\n", HDP_Event_Data->Event_Data.HDP_Sync_Capabilities_Confirmation_Data->NativeResolution);
               printf("Timestamp Accuracy: %d\r\n", HDP_Event_Data->Event_Data.HDP_Sync_Capabilities_Confirmation_Data->NativeAccuracy);
            }
            else
               printf("Failed with Response Code: %d\r\n", HDP_Event_Data->Event_Data.HDP_Sync_Capabilities_Confirmation_Data->ResponseCode);
            break;
         case etHDP_Sync_Set_Indication:
            printf("etHDP_Sync_Set_Indication\r\n");
            MCLID = HDP_Event_Data->Event_Data.HDP_Sync_Set_Indication_Data->MCLID;
            printf("MCLID: %d\r\n", MCLID);
            printf("Clock Sync Time: %08lX\r\n", HDP_Event_Data->Event_Data.HDP_Sync_Set_Indication_Data->ClockSyncTime);
            printf("Timestamp Sync Time: %08lX%08lx\r\n", (DWord_t)(HDP_Event_Data->Event_Data.HDP_Sync_Set_Indication_Data->TimestampSyncTime >> 32), (DWord_t)HDP_Event_Data->Event_Data.HDP_Sync_Set_Indication_Data->TimestampSyncTime);
            printf("Update Request: %d\r\n", HDP_Event_Data->Event_Data.HDP_Sync_Set_Indication_Data->UpdateInformationRequest);
            HDP_Sync_Get_Bluetooth_Clock_Value(BluetoothStackID, MCLID, &ClockValue, &Accuracy);
            HDP_Sync_Set_Response(BluetoothStackID, MCLID, ClockValue, 1, 2, HDP_RESPONSE_CODE_SUCCESS);
            break;
         case etHDP_Sync_Set_Confirmation:
            printf("etHDP_Sync_Set_Confirmation\r\n");
            if(HDP_Event_Data->Event_Data.HDP_Sync_Set_Confirmation_Data->ResponseCode == HDP_RESPONSE_CODE_SUCCESS)
            {
               printf("MCLID: %d\r\n", HDP_Event_Data->Event_Data.HDP_Sync_Set_Confirmation_Data->MCLID);
               printf("Clock Sync Time: %08lX\r\n", HDP_Event_Data->Event_Data.HDP_Sync_Set_Confirmation_Data->ClockSyncTime);
               printf("Timestamp Sync Time: %08lX%08lx\r\n", (DWord_t)(HDP_Event_Data->Event_Data.HDP_Sync_Set_Confirmation_Data->TimestampSyncTime >> 32), (DWord_t)HDP_Event_Data->Event_Data.HDP_Sync_Set_Confirmation_Data->TimestampSyncTime);
               printf("Timestamp Accuracy: %d\r\n", HDP_Event_Data->Event_Data.HDP_Sync_Set_Confirmation_Data->TimestampSampleAccuracy);
            }
            else
               printf("Failed with Response Code: %d\r\n", HDP_Event_Data->Event_Data.HDP_Sync_Set_Confirmation_Data->ResponseCode);
            break;
         case etHDP_Sync_Info_Indication:
            printf("etHDP_Sync_Info_Indication\r\n");
            break;
         default:
            printf("Unsupported Event %d\r\n", HDP_Event_Data->Event_Data_Type);
            break;
      }
   }
   /* Output an Input Shell-type prompt.                                */
   printf("\r\nHDP>");

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
               BaudRate = strtol(argv[3], &endptr, 10);

               /* Either a port number or a device file can be used.    */
               if((argv[2][0] >= '0') && (argv[2][0] <= '9'))
               {
                  CommPortNumber = strtol(argv[2], &endptr, 10);
                  HCI_DRIVER_SET_COMM_INFORMATION(&HCI_DriverInformation, CommPortNumber, BaudRate, cpUART);
               }
               else
               {
                  HCI_DRIVER_SET_EXTENDED_COMM_INFORMATION_DEVICE_NAME(&HCI_DriverInformation, -1, BaudRate, cpUART, 0, argv[2]);
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
               BaudRate = strtol(argv[3], &endptr, 10);

               /* Either a port number or a device file can be used.    */
               if((argv[2][0] >= '0') && (argv[2][0] <= '9'))
               {
                  CommPortNumber = strtol(argv[2], &endptr, 10);
                  HCI_DRIVER_SET_COMM_INFORMATION(&HCI_DriverInformation, CommPortNumber, BaudRate, cpBCSP);
               }
               else
               {
                  HCI_DRIVER_SET_EXTENDED_COMM_INFORMATION_DEVICE_NAME(&HCI_DriverInformation, -1, BaudRate, cpBCSP, 0, argv[2]);
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
            /* The Stack was successfully opened, next register an HDP  */
            /* Instance.                                                */
            ret_val = InitializeHDP();

            /* The Host or Device has been initialized, now show the    */
            /* Main User Interface for that type.                       */
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
         printf("Parameter Error (Expecting = [USB = 0, UART = 1, BCSP = 2 Flag] [IF !USB [Comm Port or Device File] [Baud Rate]])\r\n");
      }
   }
   else
   {
      /* An invalid number of parameters was entered on the Command     */
      /* Line.                                                          */
      printf("Parameter Error (Expecting = [USB = 0, UART = 1, BCSP = 2 Flag] [IF !USB [Comm Port or Device File] [Baud Rate]])\r\n");
   }

   return 0;
}

