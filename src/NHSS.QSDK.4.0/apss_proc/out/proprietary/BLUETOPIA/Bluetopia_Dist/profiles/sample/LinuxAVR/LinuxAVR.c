/*****< linuxavr.c >***********************************************************/
/*      Copyright 2009 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  LINUXAVR - Simple Linux application using Audio/Video Remote Control      */
/*             Profile (AVRCP).                                               */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   08/19/09  D. Lange       Initial creation.                               */
/******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "LinuxAVR.h"      /* Main Application Prototypes and Constants.      */

#include "SS1BTPS.h"       /* Includes for the SS1 Bluetooth Protocol Stack.  */
#include "SS1BTAVC.h"      /* Includes for the SS1 A/V Control Transport Prot.*/
#include "SS1BTAVR.h"      /* Includes for the SS1 A/V Remote Control Profile.*/
#include "SS1BTDBG.h"      /* Includes/Constants for Bluetooth Debugging.     */

#define LOCAL_NAME_ROOT                      "LinuxAVR"  /* Root of the local */
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

#define MAX_SUPPORTED_COMMANDS                     (35)  /* Denotes the       */
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

#define INDENT_LENGTH                               (3)  /* Denotes the number*/
                                                         /* of character      */
                                                         /* spaces to be used */
                                                         /* for indenting when*/
                                                         /* displaying SDP    */
                                                         /* Data Elements.    */

   /* The following constants represent the default log file names that */
   /* are used if no Log file name is specified when enabling debug.    */
#define DEFAULT_DEBUG_LOG_FILE_NAME  "LinuxAVR_ASC.log"
#define DEFAULT_DEBUG_FTS_FILE_NAME  "LinuxAVR_FTS.log"

   /* The following constants are used with the                         */
   /* SendVendorDependentMessage() function to specify sample Vendor    */
   /* Data to send in the command.                                      */
#define VENDOR_DATA                       "Sample Vendor Data"
#define VENDOR_DATA_LENGTH                18

   /* The following constant represents the maximum number of message   */
   /* fragments that will be supported by this application.             */
#define MAX_NUMBER_OF_FRAGMENTS           15

   /* The following constants are used with the                         */
   /* SendGetApplicationSettingsTextValueResponse() function as         */
   /* attribute value data to show how fragmentation is performed.      */
#define VALUE_TEXT                        "This is a very long text description of an attribute ID that is used to test the fragmentation feature.  Only AVRCP responses that are greater then 512 bytes must be fragmented."
#define VALUE_TEXT_LENGTH                 (sizeof(VALUE_TEXT))

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

   /* The following structure is used with the AVCTP_Profile_Info_t     */
   /* structure to track currently connected devices.                   */
typedef struct _tagDeviceList_t
{
   Boolean_t                InitiatedConnection;
   BD_ADDR_t                BD_ADDR;
   struct _tagDeviceList_t *NextDevicePtr;
} DeviceList_t;

   /* The following structure contains all state information for a      */
   /* registered AVRCP Instance.                                        */
typedef struct _tagAVCTP_Profile_Info_t
{
   int           ProfileID;
   char          ServiceName[32];
   char          ProviderName[32];
   DWord_t       ControllerSDPRecordHandle;
   DWord_t       TargetSDPRecordHandle;
   UUID_16_t     ProfileUUID;
   Boolean_t     Connected;
   DeviceList_t *DeviceList;
} AVCTP_Profile_Info_t;

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the         */
   /* compiler as part of standard C/C++).                              */
static unsigned int         BluetoothStackID;       /* Variable which holds the Handle */
                                                    /* of the opened Bluetooth Protocol*/
                                                    /* Stack.                          */

static unsigned int         DebugID;                /* Variable which holds the ID of  */
                                                    /* the currently enabled debugging */
                                                    /* session.                        */

static AVCTP_Profile_Info_t ProfileInfo;            /* Variable which holds the        */
                                                    /* current profile state of the    */
                                                    /* AVRCP Instance.                 */

static AVRCP_Response_Message_State_Info_t MessageStateInfo;   /* Variable which is    */
                                                    /* used when building responses    */
                                                    /* that could potentially be       */
                                                    /* fragmented.                     */

static BD_ADDR_t            InquiryResultList[MAX_INQUIRY_RESULTS];  /* Variable which */
                                                    /* contains the inquiry result     */
                                                    /* received from the most recently */
                                                    /* preformed inquiry.              */

static unsigned int         NumberofValidResponses; /* Variable which holds the number */
                                                    /* of valid inquiry results within */
                                                    /* the inquiry results array.      */

static LinkKeyInfo_t        LinkKeyInfo[16];        /* Variable which holds the list of*/
                                                    /* BD_ADDR <-> Link Keys for       */
                                                    /* pairing.                        */

static BD_ADDR_t            CurrentRemoteBD_ADDR;   /* Variable which holds the        */
                                                    /* current BD_ADDR of the device   */
                                                    /* which is currently pairing or   */
                                                    /* authenticating.                 */

static GAP_IO_Capability_t  IOCapability;           /* Variable which holds the        */
                                                    /* current I/O Capabilities that   */
                                                    /* are to be used for Secure Simple*/
                                                    /* Pairing.                        */

static Boolean_t            OOBSupport;             /* Variable which flags whether    */
                                                    /* or not Out of Band Secure Simple*/
                                                    /* Pairing exchange is supported.  */

static Boolean_t            MITMProtection;         /* Variable which flags whether or */
                                                    /* not Man in the Middle (MITM)    */
                                                    /* protection is to be requested   */
                                                    /* during a Secure Simple Pairing  */
                                                    /* procedure.                      */

static unsigned int         NumberCommands;         /* Variable which is used to hold  */
                                                    /* the number of Commands that are */
                                                    /* supported by this application.  */
                                                    /* Commands are added individually.*/

static CommandTable_t       CommandTable[MAX_SUPPORTED_COMMANDS]; /* Variable which is */
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
static void UserInterface(void);
static unsigned int StringToUnsignedInteger(char *StringInteger);
static char *StringParser(char *String);
static int CommandParser(UserCommand_t *TempCommand, char *UserInput);
static int CommandInterpreter(UserCommand_t *TempCommand);
static int AddCommand(char *CommandName, CommandFunction_t CommandFunction);
static CommandFunction_t FindCommand(char *Command);
static void ClearCommands(void);

static void BD_ADDRToStr(BD_ADDR_t Board_Address, char *BoardStr);
static void WriteEIRInformation(char *LocalName);

static DeviceList_t *AddDeviceListEntry(DeviceList_t **ListHead, DeviceList_t *EntryToAdd);
static DeviceList_t *SearchDeviceListEntryByBDADDR(DeviceList_t **ListHead, BD_ADDR_t BD_ADDR);
static DeviceList_t *DeleteAVCTPConnectionInfoEntry(DeviceList_t **ListHead, DeviceList_t *EntryToRemove);
static void FreeDeviceListEntryMemory(DeviceList_t *EntryToFree);
static void FreeDeviceList(DeviceList_t **ListHead);

static void UnRegisterAllProfiles(void);
static void InitializeProfileInformation(void);
static void PrintProfileInformation(void);

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

static int RegisterProfile(ParameterList_t *TempParam);
static int UnRegisterProfile(ParameterList_t *TempParam);
static int Connect(ParameterList_t *TempParam);
static int Disconnect(ParameterList_t *TempParam);
static int ProfileState(ParameterList_t *TempParam);

static int SendUnitInfoMessage(ParameterList_t *TempParam);
static int SendSubunitInfoMessage(ParameterList_t *TempParam);
static int SendPassThroughMessage(ParameterList_t *TempParam);
static int SendVendorDependentMessage(ParameterList_t *TempParam);
static int SendGetApplicationSettingsTextValueMessage(ParameterList_t *TempParam);

static int ConnectBrowsing(ParameterList_t *TempParam);
static int DisconnectBrowsing(ParameterList_t *TempParam);
static int SendBrowsingMessage(ParameterList_t *TempParam);

static void SendUnitInfoResponse(BD_ADDR_t RemoteBD_ADDR, int ProfileID, Byte_t TransactionID, Byte_t SubunitType, Byte_t SubunitID);
static void SendSubunitInfoResponse(BD_ADDR_t RemoteBD_ADDR, int ProfileID, Byte_t TransactionID, Byte_t SubunitType, Byte_t SubunitID, Byte_t PageNumber);
static void SendPassThroughResponse(BD_ADDR_t RemoteBD_ADDR, int ProfileID, Byte_t TransactionID, Byte_t SubunitType, Byte_t SubunitID, Byte_t OperationID, Boolean_t StateFlag);
static void SendVendorDependentResponse(BD_ADDR_t RemoteBD_ADDR, int ProfileID, Byte_t TransactionID, Byte_t SubunitType, Byte_t SubunitID, AVRCP_Company_ID_t CompanyID);
static void SendGetApplicationSettingsTextValueResponse(BD_ADDR_t RemoteBD_ADDR, int ProfileID, Byte_t TransactionID, Byte_t NumberEntries, AVRCP_Response_Message_State_Info_t *MessageStateInfo);

static void SendSetBrowsedPlayerResponse(BD_ADDR_t RemoteBD_ADDR, int ProfileID, Byte_t TransactionID);
static void SendAbortContinuingResponseResponse(BD_ADDR_t RemoteBD_ADDR, int ProfileID, Byte_t TransactionID);
static void SendCommandRejectResponse(BD_ADDR_t RemoteBD_ADDR, int ProfileID, Byte_t TransactionID, AVRCP_Message_Type_t MessageType, Byte_t PDU_ID, Byte_t ErrorCode);
static void SendGeneralRejectResponse(BD_ADDR_t RemoteBD_ADDR, int ProfileID, Byte_t TransactionID);

static void DisplaySDPAttributeResponse(SDP_Service_Attribute_Response_Data_t *SDPServiceAttributeResponse, unsigned int InitLevel);
static void DisplaySDPSearchAttributeResponse(SDP_Service_Search_Attribute_Response_Data_t *SDPServiceSearchAttributeResponse);
static void DisplayDataElement(SDP_Data_Element_t *SDPDataElement, unsigned int Level);

   /* BTPS Callback function prototypes.                                */
static void BTPSAPI GAP_Event_Callback(unsigned int BluetoothStackID, GAP_Event_Data_t *GAP_Event_Data, unsigned long CallbackParameter);
static void BTPSAPI SDP_Event_Callback(unsigned int BluetoothStackID, unsigned int SDPRequestID, SDP_Response_Data_t *SDP_Response_Data, unsigned long CallbackParameter);
static void BTPSAPI AVCTP_Event_Callback(unsigned int BluetoothStackID, AVCTP_Event_Data_t *AVCTPEventData, unsigned long CallbackParameter);

   /* This function is responsible for taking the input from the user   */
   /* and dispatching the appropriate Command Function.  First, this    */
   /* function retrieves a String of user input, parses the user input  */
   /* into Command and Parameters, and finally executes the Command or  */
   /* Displays an Error Message if the input is not a valid Command.    */
static void UserInterface(void)
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
   AddCommand("REGISTERPROFILE", RegisterProfile);
   AddCommand("UNREGISTERPROFILE", UnRegisterProfile);
   AddCommand("CONNECT", Connect);
   AddCommand("DISCONNECT", Disconnect);
   AddCommand("ADDBROWSINGCHANNEL", ConnectBrowsing);
   AddCommand("CLOSEBROWSINGCHANNEL", DisconnectBrowsing);
   AddCommand("PROFILESTATE", ProfileState);
   AddCommand("SENDUNITINFOMESSAGE", SendUnitInfoMessage);
   AddCommand("SENDSUBUNITINFOMESSAGE", SendSubunitInfoMessage);
   AddCommand("SENDPASSTHROUGHMESSAGE", SendPassThroughMessage);
   AddCommand("SENDVENDORDEPENDENTMESSAGE", SendVendorDependentMessage);
   AddCommand("SENDGETAPPSETTINGSVALUETEXTMESSAGE", SendGetApplicationSettingsTextValueMessage);
   AddCommand("SENDBROWSINGMESSAGE", SendBrowsingMessage);
   AddCommand("ENABLEDEBUG", EnableDebug);
   AddCommand("HELP", DisplayHelp);

   /* Make sure all Profiles are initialized to a known state.          */
   InitializeProfileInformation();

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
      printf("AVRCP>");

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
                     /* go ahead and UnRegister any Registered profiles.*/
                     UnRegisterAllProfiles();
                     break;
               }
            }
            else
               printf("Invalid Input.\r\n");
         }
      }
      else
      {
         /* If the user has request to exit we might as well go ahead   */
         /* and UnRegister any Registered profiles.                     */
         UnRegisterAllProfiles();

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

   /* The following function is a utility function that exists to add   */
   /* the specified Device List Entry to the specified Device List.     */
static DeviceList_t *AddDeviceListEntry(DeviceList_t **ListHead, DeviceList_t *EntryToAdd)
{
   BD_ADDR_t     NullADDR;
   DeviceList_t *AddedEntry = NULL;
   DeviceList_t *tmpEntry;

   ASSIGN_BD_ADDR(NullADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   /* First let's verify that values passed in are semi-valid.          */
   if((ListHead != NULL) && (EntryToAdd != NULL))
   {
      /* Make sure that the element that we are adding seems semi-valid.*/
      if((!COMPARE_BD_ADDR(EntryToAdd->BD_ADDR, NullADDR)))
      {
         /* OK, data seems semi-valid, let's allocate a new data        */
         /* structure to add to the list.                               */
         AddedEntry = (DeviceList_t *)malloc(sizeof(DeviceList_t));
         if(AddedEntry != NULL)
         {
            /* Copy All Data over.                                      */
            *AddedEntry = *EntryToAdd;

            /* Now Add it to the end of the list.                       */
            AddedEntry->NextDevicePtr = NULL;

            /* First, let's check to see if there are any elements      */
            /* already present in the List that was passed in.          */
            if((tmpEntry = *ListHead) != NULL)
            {
               /* Head Pointer was not NULL, so we will traverse the    */
               /* list until we reach the last element.                 */
               while(tmpEntry != NULL)
               {
                  if(COMPARE_BD_ADDR(tmpEntry->BD_ADDR, AddedEntry->BD_ADDR))
                  {
                     /* Entry was already added, so free the memory and */
                     /* flag an error to the caller.                    */
                     FreeDeviceListEntryMemory(AddedEntry);
                     AddedEntry = NULL;

                     /* Abort the Search.                               */
                     tmpEntry = NULL;
                  }
                  else
                  {
                     /* OK, we need to see if we are at the last element*/
                     /* of the List.  If we are, we simply break out of */
                     /* the list traversal because we know there are NO */
                     /* duplicates AND we are at the end of the list.   */
                     if(tmpEntry->NextDevicePtr != NULL)
                        tmpEntry = tmpEntry->NextDevicePtr;
                     else
                        break;
                  }
               }

               if(AddedEntry != NULL)
               {
                  /* Last element found, simply Add the entry.          */
                  tmpEntry->NextDevicePtr = AddedEntry;
               }
            }
            else
               *ListHead = AddedEntry;
         }
      }
   }
   else
      printf("AddDeviceListEntry: Invalid parameter.\r\n");

   if(AddedEntry  == NULL)
      printf("AddDeviceListEntry: Could not add Entry.\r\n");

   return(AddedEntry);
}


   /* The following function is a utility function that exists to search*/
   /* for a specific Device List Entry based on the Bluetooth Device    */
   /* Address.                                                          */
static DeviceList_t *SearchDeviceListEntryByBDADDR(DeviceList_t **ListHead, BD_ADDR_t BD_ADDR)
{
   BD_ADDR_t     NullADDR;
   DeviceList_t *FoundEntry = NULL;

   ASSIGN_BD_ADDR(NullADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   /* Let's make sure the list and Bluetooth Stack ID to search for     */
   /* appear to be valid.                                               */
   if((ListHead != NULL) && (!COMPARE_BD_ADDR(BD_ADDR, NullADDR)))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry != NULL) && (!COMPARE_BD_ADDR(BD_ADDR, FoundEntry->BD_ADDR)))
         FoundEntry = FoundEntry->NextDevicePtr;
   }

   return(FoundEntry);
}


   /* The following function is a utility function that exists to Delete*/
   /* a specific Device List Entry from the specified Device List.      */
static DeviceList_t *DeleteAVCTPConnectionInfoEntry(DeviceList_t **ListHead, DeviceList_t *EntryToRemove)
{
   DeviceList_t *FoundEntry = NULL;
   DeviceList_t *LastEntry  = NULL;

   /* Let's make sure the List appears to be semi-valid.                */
   if(ListHead != NULL)
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry != NULL) && (FoundEntry != EntryToRemove))
      {
         LastEntry  = FoundEntry;
         FoundEntry = FoundEntry->NextDevicePtr;
      }

      /* Check to see if we found the specified entry.                  */
      if(FoundEntry != NULL)
      {
         /* OK, now let's remove the entry from the list.  We have to   */
         /* check to see if the entry was the first entry in the list.  */
         if(LastEntry != NULL)
         {
            /* Entry was NOT the first entry in the list.               */
            LastEntry->NextDevicePtr = FoundEntry->NextDevicePtr;
         }
         else
            *ListHead = FoundEntry->NextDevicePtr;

         FoundEntry->NextDevicePtr = NULL;
      }
   }

   return(FoundEntry);
}

   /* The following function is a utility function that exists to free  */
   /* all resources/memory of a specific Device List Entry.             */
static void FreeDeviceListEntryMemory(DeviceList_t *EntryToFree)
{
   if(EntryToFree != NULL)
      free(EntryToFree);
}

   /* The following function is a utility function that exists to remove*/
   /* (and free all resources) every entry from the specified Device    */
   /* List.                                                             */
static void FreeDeviceList(DeviceList_t **ListHead)
{
   DeviceList_t *EntryToFree;
   DeviceList_t *tmpEntry;

   if(ListHead != NULL)
   {
      /* Simply traverse the list and free every element present.       */
      EntryToFree = *ListHead;

      while(EntryToFree != NULL)
      {
         tmpEntry    = EntryToFree;
         EntryToFree = EntryToFree->NextDevicePtr;

         FreeDeviceListEntryMemory(tmpEntry);
      }

      /* Make sure the List appears to be empty.                        */
      *ListHead = NULL;
   }
}

   /* The following function is responsible for UnRegistering all       */
   /* registered Profiles.                                              */
static void UnRegisterAllProfiles(void)
{
   if(BluetoothStackID)
   {
      /* If the Profile is Registered, go ahead and Un-register it.     */
      if(ProfileInfo.ProfileID)
         AVCTP_UnRegister_Profile(BluetoothStackID, ProfileInfo.ProfileID);

      /* If any SDP Records are present, go ahead and delete them from  */
      /* the SDP Database.                                              */
      if(ProfileInfo.ControllerSDPRecordHandle)
      {
         AVRCP_Un_Register_SDP_Record(BluetoothStackID, ProfileInfo.ControllerSDPRecordHandle);
      }

      if(ProfileInfo.TargetSDPRecordHandle)
      {
         AVRCP_Un_Register_SDP_Record(BluetoothStackID, ProfileInfo.TargetSDPRecordHandle);
      }

      /* Free the Device List.                                          */
      FreeDeviceList(&(ProfileInfo.DeviceList));

      /* Make sure the Profile state reflects that the profile is now   */
      /* un-registered.                                                 */
      ProfileInfo.ProfileID                 = 0;
      ProfileInfo.Connected                 = FALSE;
      ProfileInfo.TargetSDPRecordHandle     = 0;
      ProfileInfo.ControllerSDPRecordHandle = 0;
   }
}

   /* The following function is responsible for initializing all        */
   /* supported Profile Instances supported by this application to      */
   /* known, default, values.                                           */
static void InitializeProfileInformation(void)
{
   BTPS_MemInitialize(&ProfileInfo, 0, sizeof(ProfileInfo));

   /* Populate the Profile UUID for this profile.                       */
   SDP_ASSIGN_AUDIO_VIDEO_REMOTE_CONTROL_PROFILE_UUID_16(ProfileInfo.ProfileUUID);

   strcpy(ProfileInfo.ServiceName, "AVRCP");
   strcpy(ProfileInfo.ProviderName, "Stonestreet One");
}

   /* The following function is responsible for printing out Profile    */
   /* Instance Information for the AVRCP Profile.                       */
static void PrintProfileInformation(void)
{
   char          BoardStr[13];
   DeviceList_t *DeviceListEntry;

   printf("Profile ID: %d\r\n", ProfileInfo.ProfileID);

   printf("Connected: %s\r\n", ProfileInfo.Connected?"TRUE":"FALSE");

   DeviceListEntry = ProfileInfo.DeviceList;

   if(!DeviceListEntry)
      printf("No Devices in the devices List.\r\n");
   else
   {
      while(DeviceListEntry != NULL)
      {
         BD_ADDRToStr(DeviceListEntry->BD_ADDR, BoardStr);

         printf("Remote Device: %s ", BoardStr);
         printf("Initiated: %d\r\n", DeviceListEntry->InitiatedConnection);

         DeviceListEntry = DeviceListEntry->NextDevicePtr;
      }
   }

   printf("UUID: 0x%02X%02X\r\n", ProfileInfo.ProfileUUID.UUID_Byte1, ProfileInfo.ProfileUUID.UUID_Byte0);

   printf("SDP Handle (C): %08lX\r\n", ProfileInfo.ControllerSDPRecordHandle);
   printf("SDP Handle (T): %08lX\r\n", ProfileInfo.TargetSDPRecordHandle);

   printf("ServiceName: %s\r\n", ProfileInfo.ServiceName);
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

            /* Go ahead and initialize AVCTP since the stack            */
            /* initialization was successful.                           */
            if((ret_val = AVCTP_Initialize(BluetoothStackID)) == 0)
            {
               printf("AVCTP_Initialize: Success.\r\n");

               /* Enable Browse Channel support.                        */
               if((ret_val = AVCTP_Enable_Browsing_Channel_Support(BluetoothStackID)) == 0)
                  printf("AVCTP_Enable_Browsing_Channel_Support: Success.\r\n");
               else
                  printf("AVCTP_Enable_Browsing_Channel_Support: Failure. Error: %d\r\n", ret_val);
            }
            else
            {
               printf("AVCTP_Initialize: Failure: %d\r\n", ret_val);

               /* Shutdown the Bluetooth Protocol Stack that was        */
               /* initialized.                                          */
               BSC_Shutdown(BluetoothStackID);

               /* Flag an error to the caller (and flag that the        */
               /* Bluetooth Stack has not been initialized).            */
               BluetoothStackID = 0;

               ret_val          = UNABLE_TO_INITIALIZE_STACK;
            }
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
      /* First, close down the AVCTP.                                   */
      AVCTP_Cleanup(BluetoothStackID);

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
   printf("******************************************************************\r\n");
   printf("* Command Options: Inquiry, DisplayInquiryList, Pair,            *\r\n");
   printf("*                  EndPairing, PINCodeResponse, PassKeyResponse, *\r\n");
   printf("*                  UserConfirmationResponse,                     *\r\n");
   printf("*                  SetDiscoverabilityMode, SetConnectabilityMode,*\r\n");
   printf("*                  SetPairabilityMode, ServiceDiscovery          *\r\n");
   printf("*                  ChangeSimplePairingParameters,                *\r\n");
   printf("*                  GetLocalAddress, GetLocalName, SetLocalName,  *\r\n");
   printf("*                  GetClassOfDevice, SetClassOfDevice,           *\r\n");
   printf("*                  GetRemoteName, RegisterProfile,               *\r\n");
   printf("*                  UnRegisterProfile, Connect, Disconnect,       *\r\n");
   printf("*                  AddBrowsingChannel, CloseBrowsingChannel,     *\r\n");
   printf("*                  ProfileState, SendUnitInfoMessage,            *\r\n");
   printf("*                  SendSubunitInfoMessage,                       *\r\n");
   printf("*                  SendPassThroughMessage,                       *\r\n");
   printf("*                  SendVendorDependentMessage,                   *\r\n");
   printf("*                  SendGetAppSettingsValueTextMessage,           *\r\n");
   printf("*                  SendBrowsingMessage, EnableDebug,             *\r\n");
   printf("*                  Help, Quit.                                   *\r\n");
   printf("******************************************************************\r\n");

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
      if((!ProfileInfo.ProfileID) || (!((ProfileInfo.ProfileID) && (ProfileInfo.Connected))))
      {
         /* There are no currently connected profiles, make sure that   */
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

   /* The following function is responsible for registering an AVCTP    */
   /* Profile on the local device.  This function returns zero if       */
   /* successful and a negative value if an error occurred.             */
static int RegisterProfile(ParameterList_t *TempParam)
{
   int       ret_val;
   int       Result;
   DWord_t   RecordHandle;
   Boolean_t IsTarget;
   Boolean_t IsController;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the Profile Instance passed to this*/
      /* function is valid.                                             */
      if((TempParam) && (TempParam->NumberofParameters >= 1) && (TempParam->Params[0].intParam & 0x03))
      {
         /* Now check to make sure that a Profile isn't already         */
         /* registered.                                                 */
         if(!ProfileInfo.ProfileID)
         {
            /* Attempt to register the profile                          */

            Result = AVCTP_Register_Profile(BluetoothStackID, ProfileInfo.ProfileUUID, AVCTP_Event_Callback, 0);

            /* Now check to see if the function call was successfully   */
            /* made.                                                    */
            if(Result > 0)
            {
               printf("AVCTP_Register_Profile: Function Success.\r\n");

               /* Profile added, go ahead and initialize the Profile    */
               /* Information.                                          */
               ret_val = 0;

               /* Determine the Roles of this Profile.                  */
               if(TempParam->Params[0].intParam & 0x01)
                  IsController = TRUE;
               else
                  IsController = FALSE;

               if(TempParam->Params[0].intParam & 0x02)
                  IsTarget = TRUE;
               else
                  IsTarget = FALSE;

               /* Update the Service Name to reflect Controller/Target  */
               /* settings.                                             */
               sprintf(ProfileInfo.ServiceName, "%s%s%s%s", "AVRCP", (IsController?" Controller":""), (((IsController) && (IsTarget))?" And":""), (IsTarget?" Target":""));

               /* Note the Profile ID of the newly added Profile.       */
               ProfileInfo.ProfileID = Result;

               /* Profile Added, go ahead and add the SDP Record.       */
               if((ProfileInfo.ProfileID) && (IsController))
               {
                  if(!AVRCP_Register_SDP_Record(BluetoothStackID, TRUE, ProfileInfo.ServiceName, ProfileInfo.ProviderName, (Word_t)(SDP_AVRCP_SUPPORTED_FEATURES_TARGET_CATEGORY_1 | SDP_AVRCP_SUPPORTED_FEATURES_TARGET_CATEGORY_2), &RecordHandle))
                  {
                     ProfileInfo.ControllerSDPRecordHandle = RecordHandle;
                     printf("Controller SDP Record Added.\r\n");
                  }
                  else
                  {
                     /* Error adding the SDP Record to the Database, so */
                     /* go ahead unregister the Profile and notify the  */
                     /* user.                                           */
                     AVCTP_UnRegister_Profile(BluetoothStackID, ProfileInfo.ProfileID);

                     if(ProfileInfo.TargetSDPRecordHandle)
                        AVRCP_Un_Register_SDP_Record(BluetoothStackID, ProfileInfo.TargetSDPRecordHandle);

                     ProfileInfo.ControllerSDPRecordHandle = 0;
                     ProfileInfo.TargetSDPRecordHandle     = 0;
                     ProfileInfo.ProfileID                 = 0;

                     printf("Unable to register SDP Record for Controller Role.  Profile Not Registered.");

                     /* Flag an error to the caller.                    */
                     ret_val = FUNCTION_ERROR;
                  }
               }

               /* Add a Target SDP Record if profile supports Target    */
               /* Role.                                                 */
               if((ProfileInfo.ProfileID) && (IsTarget))
               {
                  if(!AVRCP_Register_SDP_Record(BluetoothStackID, FALSE, ProfileInfo.ServiceName, ProfileInfo.ProviderName, (Word_t)(SDP_AVRCP_SUPPORTED_FEATURES_TARGET_CATEGORY_1 | SDP_AVRCP_SUPPORTED_FEATURES_TARGET_CATEGORY_2), &RecordHandle))
                  {
                     ProfileInfo.TargetSDPRecordHandle = RecordHandle;
                     printf("Target SDP Record Added.\r\n");
                  }
                  else
                  {
                     /* Error adding the SDP Record to the Database, so */
                     /* go ahead unregister the Profile and notify the  */
                     /* user.                                           */
                     AVCTP_UnRegister_Profile(BluetoothStackID, ProfileInfo.ProfileID);

                     if(ProfileInfo.ControllerSDPRecordHandle)
                        AVRCP_Un_Register_SDP_Record(BluetoothStackID, ProfileInfo.ControllerSDPRecordHandle);

                     ProfileInfo.ControllerSDPRecordHandle = 0;
                     ProfileInfo.TargetSDPRecordHandle     = 0;
                     ProfileInfo.ProfileID                 = 0;

                     printf("Unable to register SDP Record for Target Role. Profile Not Registered.\r\n");

                     /* Flag an error to the caller.                    */
                     ret_val = FUNCTION_ERROR;
                  }
               }
            }
            else
            {
               /* There was an error attempting to register the Profile.*/
               printf("AVCTP_Register_Profile: Function Failure: %d.\r\n", Result);

               ret_val = Result;
            }
         }
         else
         {
            /* Instance is already present.                             */
            printf("Unable to Register Profile.  Profile is already Registered.\r\n");

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         printf("Usage: RegisterProfile [Controller/Target Bit Mask (Controller = 0x01, Target = 0x02)].\r\n");

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

   /* The following function is responsible for un-registering a        */
   /* previously registered AVCTP Profile on the local device.  This    */
   /* function returns zero if successful and a negative value if an    */
   /* error occurred.                                                   */
static int UnRegisterProfile(ParameterList_t *TempParam)
{
   int ret_val;
   int Result;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that a Profile exists at this instance. */
      if(ProfileInfo.ProfileID)
      {
         /* Try to Un-register the Profile.                             */
         Result = AVCTP_UnRegister_Profile(BluetoothStackID, ProfileInfo.ProfileID);

         /* Now check to see if the function call was successfully made.*/
         if(!Result)
         {
            /* Successful, go ahead and flag that the Profile is no     */
            /* longer active.                                           */
            ProfileInfo.ProfileID = 0;
            ProfileInfo.Connected = FALSE;

            if(ProfileInfo.ControllerSDPRecordHandle)
            {
               AVCTP_UnRegister_Profile_SDP_Record(BluetoothStackID, ProfileInfo.ControllerSDPRecordHandle);

               ProfileInfo.ControllerSDPRecordHandle = 0;
            }

            if(ProfileInfo.TargetSDPRecordHandle)
            {
               AVCTP_UnRegister_Profile_SDP_Record(BluetoothStackID, ProfileInfo.TargetSDPRecordHandle);

               ProfileInfo.TargetSDPRecordHandle = 0;
            }

            FreeDeviceList(&(ProfileInfo.DeviceList));

            printf("AVCTP_UnRegister_Profile: Function Success.\r\n");

            ret_val = 0;
         }
         else
         {
            /* There was an error attempting to Un-register the Profile.*/
            printf("AVCTP_UnRegister_Profile: Function Failure: %d.\r\n", Result);

            ret_val = Result;
         }
      }
      else
      {
         printf("Unable to Un-register Profile.  Profile is not Registered..\r\n");

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

   /* The following function is responsible for connecting a local AVCTP*/
   /* Profile instance to a remote AVCTP Profile instance.  This        */
   /* function returns zero if successful and a negative value if an    */
   /* error occurred.                                                   */
static int Connect(ParameterList_t *TempParam)
{
   int       ret_val;
   int       Result;
   BD_ADDR_t NullADDR;

   ASSIGN_BD_ADDR(NullADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the Profile Instance passed to this*/
      /* function is valid.                                             */
      if((TempParam) && (TempParam->NumberofParameters >= 1) && (TempParam->Params[0].intParam <= NumberofValidResponses) && (!COMPARE_BD_ADDR(InquiryResultList[(TempParam->Params[0].intParam - 1)], NullADDR)))
      {
         /* Now check to make sure that the Profile exists.             */
         if(ProfileInfo.ProfileID)
         {
            /* Try to Disconnect the Profile.                           */
            Result = AVCTP_Connect_Device(BluetoothStackID, ProfileInfo.ProfileID, InquiryResultList[TempParam->Params[0].intParam - 1]);

            /* Now check to see if the function call was successfully   */
            /* made.                                                    */
            if(!Result)
            {
               /* The Disconnect Request was successfully submitted.    */
               printf("AVCTP_Connect_Device: Function Successful.\r\n");

               ret_val = 0;
            }
            else
            {
               /* There was an error attempting to disconnect the       */
               /* Profile.                                              */
               printf("AVCTP_Connect_Device: Function Failure: %d.\r\n", Result);

               ret_val = Result;
            }
         }
         else
         {
            /* Profile is not registered.                               */
            printf("No Profile Registered.\r\n");

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         printf("Usage: Connect [Inquiry Index].\r\n");

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

   /* The following function is responsible for disconnecting a local   */
   /* AVCTP Profile instance from a remote AVCTP Profile instance.  This*/
   /* function returns zero if successful and a negative value if an    */
   /* error occurred.                                                   */
static int Disconnect(ParameterList_t *TempParam)
{
   int           ret_val;
   int           Result;
   BD_ADDR_t     NullADDR;
   DeviceList_t *DeviceListEntry;

   ASSIGN_BD_ADDR(NullADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the Profile Instance passed to this*/
      /* function is valid.                                             */
      if((TempParam) && (TempParam->NumberofParameters >= 1) && (TempParam->Params[0].intParam <= NumberofValidResponses) && (!COMPARE_BD_ADDR(InquiryResultList[(TempParam->Params[0].intParam - 1)], NullADDR)))
      {
         /* Now check to make sure that a Profile exists at this        */
         /* instance AND it is connected.                               */
         if((ProfileInfo.ProfileID) && (ProfileInfo.Connected))
         {
            /* Try to Disconnect the Profile.                           */
            Result = AVCTP_Close_Connection(BluetoothStackID, ProfileInfo.ProfileID, InquiryResultList[TempParam->Params[0].intParam - 1]);

            /* Now check to see if the function call was successfully   */
            /* made.                                                    */
            if(!Result)
            {
               /* The Disconnect Request was successfully submitted.    */
               printf("AVCTP_Close_Connection: Function Successful.\r\n");

               /* Delete the Device entry from the Device List.         */
               if((DeviceListEntry = SearchDeviceListEntryByBDADDR(&(ProfileInfo.DeviceList), InquiryResultList[TempParam->Params[0].intParam - 1])) != NULL)
               {
                  if(DeleteAVCTPConnectionInfoEntry(&(ProfileInfo.DeviceList), DeviceListEntry) != NULL)
                     FreeDeviceListEntryMemory(DeviceListEntry);

                  if(!ProfileInfo.DeviceList)
                     ProfileInfo.Connected = FALSE;
               }

               ret_val = 0;
            }
            else
            {
               /* There was an error attempting to disconnect the       */
               /* Profile.                                              */
               printf("AVCTP_Close_Connection: Function Failure: %d.\r\n", Result);

               ret_val = Result;
            }
         }
         else
         {
            /* Instance is either not present or not currently          */
            /* connected.                                               */
            printf("No Profile Registered or not Profile Not Connected.\r\n");

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         printf("Usage: Disconnect [Inquiry Index].\r\n");

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

   /* The following function is responsible for connecting a local AVCTP*/
   /* Profile Browsing Channel instance to a remote AVCTP Profile       */
   /* Browsing Channel instance.  This function returns zero if         */
   /* successful and a negative value if an error occurred.             */
static int ConnectBrowsing(ParameterList_t *TempParam)
{
   int       ret_val;
   int       Result;
   BD_ADDR_t NullADDR;

   ASSIGN_BD_ADDR(NullADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the Profile Instance passed to this*/
      /* function is valid.                                             */
      if((TempParam) && (TempParam->NumberofParameters >= 1) && (TempParam->Params[0].intParam <= NumberofValidResponses) && (!COMPARE_BD_ADDR(InquiryResultList[(TempParam->Params[0].intParam - 1)], NullADDR)))
      {
         /* Now check to make sure that a Profile exists at this        */
         /* instance AND it is connected.                               */
         if((ProfileInfo.ProfileID) && (ProfileInfo.Connected))
         {
            /* Try to Connect the Browsing Channel.                     */
            Result = AVCTP_Connect_Browsing_Channel(BluetoothStackID, ProfileInfo.ProfileID, InquiryResultList[TempParam->Params[0].intParam - 1], L2CAP_MAXIMUM_SUPPORTED_STACK_MTU);

            /* Now check to see if the function call was successfully   */
            /* made.                                                    */
            if(!Result)
            {
               /* The Browsing Channel Connect Request was successfully */
               /* submitted.                                            */
               printf("AVCTP_Connect_Browsing_Channel: Function Successful.\r\n");

               ret_val = 0;
            }
            else
            {
               /* There was an error attempting to connect the Browsing */
               /* Channel.                                              */
               printf("AVCTP_Connect_Browsing_Channel: Function Failure: %d.\r\n", Result);

               ret_val = Result;
            }
         }
         else
         {
            /* Instance is either not present or not currently          */
            /* connected.                                               */
            printf("No Profile Instance present at Instance Index: %d.\r\n", TempParam->Params[0].intParam);

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         printf("Usage: AddBrowsingChannel [Inquiry Index].\r\n");

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

   /* The following function is responsible for disconnecting a local   */
   /* AVCTP Browsing Channel Profile instance from a remote AVCTP       */
   /* Profile instance.  This function returns zero if successful and a */
   /* negative value if an error occurred.                              */
static int DisconnectBrowsing(ParameterList_t *TempParam)
{
   int       ret_val;
   int       Result;
   BD_ADDR_t NullADDR;

   ASSIGN_BD_ADDR(NullADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the Profile Instance passed to this*/
      /* function is valid.                                             */
      if((TempParam) && (TempParam->NumberofParameters >= 1) && (TempParam->Params[0].intParam <= NumberofValidResponses) && (!COMPARE_BD_ADDR(InquiryResultList[(TempParam->Params[0].intParam - 1)], NullADDR)))
      {
         /* Now check to make sure that a Profile exists at this        */
         /* instance AND it is connected.                               */
         if((ProfileInfo.ProfileID) && (ProfileInfo.Connected))
         {
            /* Try to Disconnect the Browsing Channel.                  */
            Result = AVCTP_Close_Browsing_Channel(BluetoothStackID, ProfileInfo.ProfileID, InquiryResultList[TempParam->Params[0].intParam - 1]);

            /* Now check to see if the function call was successfully   */
            /* made.                                                    */
            if(!Result)
            {
               /* The Disconnect Request was successfully submitted.    */
               printf("AVCTP_Close_Browsing_Channel: Function Successful.\r\n");

               ret_val = 0;
            }
            else
            {
               /* There was an error attempting to disconnect the       */
               /* Browsing Channel.                                     */
               printf("AVCTP_Close_Browsing_Channel: Function Failure: %d.\r\n", Result);

               ret_val = Result;
            }
         }
         else
         {
            /* Instance is either not present or not currently          */
            /* connected.                                               */
            printf("No Profile Instance present or Profile not connected at Instance Index: %d.\r\n", TempParam->Params[0].intParam);

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         printf("Usage: CloseBrowsingChannel [Inquiry Index].\r\n");

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

   /* The following function is responsible for displaying current AVCTP*/
   /* Profile instance state information.  This function returns zero if*/
   /* successful and a negative value if an error occurred.             */
static int ProfileState(ParameterList_t *TempParam)
{
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Simply output the Profile Information.                         */
      PrintProfileInformation();

      ret_val = 0;
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      printf("Stack ID Invalid.\r\n");

      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for sending an AVRCP Unit   */
   /* Info Message to a remote AVCTP Profile instance.  This function   */
   /* returns zero if successful and a negative value if an error       */
   /* occurred.                                                         */
static int SendUnitInfoMessage(ParameterList_t *TempParam)
{
   int           ret_val;
   int           Status;
   BD_ADDR_t     NullADDR;
   unsigned char Buffer[48];

   ASSIGN_BD_ADDR(NullADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the Profile Instance passed to this*/
      /* function is valid.                                             */
      if((TempParam) && (TempParam->NumberofParameters >= 2) && (TempParam->Params[0].intParam <= NumberofValidResponses) && (!COMPARE_BD_ADDR(InquiryResultList[(TempParam->Params[0].intParam - 1)], NullADDR)))
      {
         /* Now check to make sure that the Profile exists at this      */
         /* instance.                                                   */
         if((ProfileInfo.ProfileID) && (ProfileInfo.Connected))
         {
            /* Format up a UnitInfo Command (for demonstration          */
            /* purposes).                                               */
            Status = AVRCP_Format_Unit_Info_Command(BluetoothStackID, sizeof(Buffer), Buffer);
            printf("AVRCP_Format_Unit_Info_Command: %d\r\n", Status);
            if(Status > 0)
            {
               /* Attempt to send the data.                             */
               if((Status = AVCTP_Send_Message(BluetoothStackID, ProfileInfo.ProfileID, InquiryResultList[(TempParam->Params[0].intParam - 1)], (Byte_t)(TempParam->Params[1].intParam & 0x0F), FALSE, Status, Buffer)) != 0)
               {
                  printf("Send Message Failure Error: %d.\r\n", Status);

                  ret_val = Status;
               }
               else
               {
                  printf("Send Message Successful.\r\n");

                  ret_val = 0;
               }
            }
            else
            {
               printf("AVRCP_Format_Unit_Info_Command: %d.\r\n", Status);

               ret_val = Status;
            }
         }
         else
         {
            /* Instance is either not present or not currently          */
            /* connected.                                               */
            printf("No Profile Instance present or Profile not connected.\r\n");

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         printf("Usage: SendUnitInfoMessage [Inquiry Index] [Transaction ID (0 <= 15)].\r\n");

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

   /* The following function is responsible for sending an AVRCP Subunit*/
   /* Info Message to a remote AVCTP Profile instance.  This function   */
   /* returns zero if successful and a negative value if an error       */
   /* occurred.                                                         */
static int SendSubunitInfoMessage(ParameterList_t *TempParam)
{
   int                               ret_val;
   int                               Status;
   BD_ADDR_t                         NullADDR;
   unsigned char                     Buffer[48];
   AVRCP_Subunit_Info_Command_Data_t SubunitInfoCommandData;

   ASSIGN_BD_ADDR(NullADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the Profile Instance passed to this*/
      /* function is valid.                                             */
      if((TempParam) && (TempParam->NumberofParameters >= 2) && (TempParam->Params[0].intParam <= NumberofValidResponses) && (!COMPARE_BD_ADDR(InquiryResultList[(TempParam->Params[0].intParam - 1)], NullADDR)))
      {
         /* Now check to make sure that the Profile exists at this      */
         /* instance.                                                   */
         if((ProfileInfo.ProfileID) && (ProfileInfo.Connected))
         {
            /* Format up a UnitInfo Command (for demonstration          */
            /* purposes).                                               */
            SubunitInfoCommandData.SubunitType = AVRCP_SUBUNIT_TYPE_UNIT;
            SubunitInfoCommandData.SubunitID   = AVRCP_SUBUNIT_ID_IGNORE;
            SubunitInfoCommandData.Page        = 0;

            /* Format the Command from the Command Data.                */
            Status = AVRCP_Format_Subunit_Info_Command(BluetoothStackID, &SubunitInfoCommandData, sizeof(Buffer), Buffer);
            printf("AVRCP_Format_Subunit_Info_Command: %d\r\n", Status);
            if(Status > 0)
            {
               /* Attempt to send the data.                             */
               if((Status = AVCTP_Send_Message(BluetoothStackID, ProfileInfo.ProfileID, InquiryResultList[(TempParam->Params[0].intParam - 1)], (Byte_t)(TempParam->Params[1].intParam & 0x0F), FALSE, Status, Buffer)) != 0)
               {
                  printf("Send Message Failure Error: %d.\r\n", Status);

                  ret_val = Status;
               }
               else
               {
                  printf("Send Message Successful.\r\n");

                  ret_val = 0;
               }
            }
            else
            {
               printf("AVRCP_Format_Subunit_Info_Command: %d.\r\n", Status);

               ret_val = Status;
            }
         }
         else
         {
            /* Instance is either not present or not currently          */
            /* connected.                                               */
            printf("No Profile Instance present or Profile not connected.\r\n");

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         printf("Usage: SendSubunitInfoMessage [Inquiry Index] [Transaction ID (0 <= 15)].\r\n");

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

   /* The following function is responsible for sending an AVRCP        */
   /* Pass-through Message to a remote AVCTP Profile instance.  This    */
   /* function returns zero if successful and a negative value if an    */
   /* error occurred.                                                   */
static int SendPassThroughMessage(ParameterList_t *TempParam)
{
   int                               ret_val;
   int                               Status;
   Byte_t                            OperationID;
   BD_ADDR_t                         NullADDR;
   unsigned char                     Buffer[48];
   AVRCP_Pass_Through_Command_Data_t PassThroughCommandData;

   ASSIGN_BD_ADDR(NullADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the Profile Instance passed to this*/
      /* function is valid.                                             */
      if((TempParam) && (TempParam->NumberofParameters >= 3) && (TempParam->Params[0].intParam <= NumberofValidResponses) && (!COMPARE_BD_ADDR(InquiryResultList[(TempParam->Params[0].intParam - 1)], NullADDR)))
      {
         /* Determine the Pass-through Operation that was specified.    */
         Status = 0;
         switch(TempParam->Params[2].intParam)
         {
            case 0:
               /* Pause.                                                */
               OperationID = AVRCP_PASS_THROUGH_ID_PAUSE;

               printf("Pause Pass-through Command specified.\r\n");
               break;
            case 1:
               /* Play.                                                 */
               OperationID = AVRCP_PASS_THROUGH_ID_PLAY;

               printf("Play Pass-through Command specified.\r\n");
               break;
            case 2:
               /* Stop.                                                 */
               OperationID = AVRCP_PASS_THROUGH_ID_STOP;

               printf("Stop Pass-through Command specified.\r\n");
               break;
            case 3:
               /* Volume Up.                                            */
               OperationID = AVRCP_PASS_THROUGH_ID_VOLUME_UP;

               printf("Volume Up Pass-through Command specified.\r\n");
               break;
            case 4:
               /* Volume Down.                                          */
               OperationID = AVRCP_PASS_THROUGH_ID_VOLUME_DOWN;

               printf("Volume Down Pass-through Command specified.\r\n");
               break;
            case 5:
               /* Specify Command.                                      */
               if((TempParam->NumberofParameters >= 4) && (TempParam->Params[3].intParam) && (TempParam->Params[4].intParam != 0xFF))
               {
                  OperationID = (Byte_t)TempParam->Params[3].intParam;

                  printf("Specific Pass-through Command specified: 0x%02X.\r\n", OperationID);
               }
               else
                  Status = INVALID_PARAMETERS_ERROR;
               break;
            default:
               Status = INVALID_PARAMETERS_ERROR;

               printf("Unknown Pass-through Command Option specified: %d\r\n", TempParam->Params[3].intParam);
               break;
         }

         if(!Status)
         {
            /* Now check to make sure that the Profile exists at this   */
            /* instance.                                                */
            if((ProfileInfo.ProfileID) && (ProfileInfo.Connected))
            {
               /* Format up a Pass-through Command.                     */
               PassThroughCommandData.CommandType         = AVRCP_CTYPE_CONTROL;
               PassThroughCommandData.SubunitType         = AVRCP_SUBUNIT_TYPE_PANEL;
               PassThroughCommandData.SubunitID           = AVRCP_SUBUNIT_ID_INSTANCE_0;
               PassThroughCommandData.OperationID         = OperationID;
               PassThroughCommandData.StateFlag           = FALSE;
               PassThroughCommandData.OperationDataLength = 0;
               PassThroughCommandData.OperationData       = NULL;

               /* Format the Command from the Command Data.             */
               Status = AVRCP_Format_Pass_Through_Command(BluetoothStackID, &PassThroughCommandData, sizeof(Buffer), Buffer);
               printf("AVRCP_Format_Pass_Through_Command: %d\r\n", Status);
               if(Status > 0)
               {
                  /* Attempt to send the data.                          */
                  if((Status = AVCTP_Send_Message(BluetoothStackID, ProfileInfo.ProfileID, InquiryResultList[(TempParam->Params[0].intParam - 1)], (Byte_t)(TempParam->Params[1].intParam & 0x0F), FALSE, Status, Buffer)) != 0)
                  {
                     printf("Send Message Failure Error: %d.\r\n", Status);

                     ret_val = Status;
                  }
                  else
                  {
                     printf("Send Message Successful.\r\n");

                     ret_val = 0;
                  }
               }
               else
               {
                  printf("AVRCP_Format_Pass_Through_Command: %d.\r\n", Status);

                  ret_val = Status;
               }
            }
            else
            {
               /* Instance is either not present or not currently       */
               /* connected.                                            */
               printf("No Profile Instance present or Profile not connected.\r\n");

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            printf("Usage: SendPassThroughMessage [Inquiry Index] [Transaction ID (0 <= 15)] [Passthrough Command (0 = Pause, 1 = Play, 2 = Stop, 3 = Vol. Up, 4 = Vol. Down, 5 = Specify Command)] [Specify Command Value (only used with Specify Command)].\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         printf("Usage: SendPassThroughMessage [Inquiry Index] [Transaction ID (0 <= 15)] [Passthrough Command (0 = Pause, 1 = Play, 2 = Stop, 3 = Vol. Up, 4 = Vol. Down, 5 = Specify Command)] [Specify Command Value (only used with Specify Command)].\r\n");

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

   /* The following function is responsible for sending an AVRCP Vendor */
   /* Dependent Message to a remote AVCTP Profile instance.  This       */
   /* function returns zero if successful and a negative value if an    */
   /* error occurred.                                                   */
static int SendVendorDependentMessage(ParameterList_t *TempParam)
{
   int                                           ret_val;
   int                                           Status;
   BD_ADDR_t                                     NullADDR;
   unsigned char                                 Buffer[48];
   AVRCP_Vendor_Dependent_Generic_Command_Data_t VendorDependentGenericCommandData;

   ASSIGN_BD_ADDR(NullADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the Profile Instance passed to this*/
      /* function is valid.                                             */
      if((TempParam) && (TempParam->NumberofParameters >= 2) && (TempParam->Params[0].intParam <= NumberofValidResponses) && (!COMPARE_BD_ADDR(InquiryResultList[(TempParam->Params[0].intParam - 1)], NullADDR)))
      {
         /* Now check to make sure that the Profile exists at this      */
         /* instance.                                                   */
         if((ProfileInfo.ProfileID) && (ProfileInfo.Connected))
         {
            /* Format up a Vendor Dependent Generic Command (for        */
            /* demonstration purposes).                                 */
            VendorDependentGenericCommandData.CommandType = AVRCP_CTYPE_NOTIFY;
            VendorDependentGenericCommandData.SubunitType = AVRCP_SUBUNIT_TYPE_VENDOR_SPECIFIC;
            VendorDependentGenericCommandData.SubunitID   = AVRCP_SUBUNIT_ID_IGNORE;
            ASSIGN_AVRCP_COMPANY_ID(VendorDependentGenericCommandData.CompanyID, 0x12, 0x33, 0x21);

            VendorDependentGenericCommandData.DataLength = VENDOR_DATA_LENGTH;
            VendorDependentGenericCommandData.DataBuffer = (Byte_t *)VENDOR_DATA;

            /* Format the Command from the Command Data.                */
            Status = AVRCP_Format_Vendor_Dependent_Generic_Command(BluetoothStackID, &VendorDependentGenericCommandData, sizeof(Buffer), (unsigned char *)Buffer);
            printf("AVRCP_Format_Vendor_Dependent_Generic_Command: %d\r\n", Status);
            if(Status > 0)
            {
               /* Attempt to send the data.                             */
               if((Status = AVCTP_Send_Message(BluetoothStackID, ProfileInfo.ProfileID, InquiryResultList[(TempParam->Params[0].intParam - 1)], (Byte_t)(TempParam->Params[1].intParam & 0x0F), FALSE, Status, Buffer)) != 0)
               {
                  printf("Send Message Failure Error: %d.\r\n", Status);

                  ret_val = Status;
               }
               else
               {
                  printf("Send Message Successful.\r\n");

                  ret_val = 0;
               }
            }
            else
            {
               printf("AVRCP_Format_Vendor_Dependent_Generic_Command: %d.\r\n", Status);

               ret_val = Status;
            }
         }
         else
         {
            /* Instance is either not present or not currently          */
            /* connected.                                               */
            printf("No Profile Instance present or Profile not connected.\r\n");

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         printf("Usage: SendVendorDependentMessage [Inquiry Index] [Transaction ID (0 <= 15)].\r\n");

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

   /* The following function is responsible for sending an AVRCP Get    */
   /* Application Settings Text Value Message to a remote AVCTP Profile */
   /* instance.  This function returns zero if successful and a negative*/
   /* value if an error occurred.                                       */
static int SendGetApplicationSettingsTextValueMessage(ParameterList_t *TempParam)
{
   int                                                            ret_val;
   int                                                            Status;
   Byte_t                                                         ValueIDList[6];
   BD_ADDR_t                                                      NullADDR;
   unsigned char                                                  Buffer[48];
   AVRCP_Get_Player_Application_Setting_Value_Text_Command_Data_t GetPlayerApplicationSettingValueTextCommandData;

   ASSIGN_BD_ADDR(NullADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the Profile Instance passed to this*/
      /* function is valid.                                             */
      if((TempParam) && (TempParam->NumberofParameters >= 2) && (TempParam->Params[0].intParam <= NumberofValidResponses) && (!COMPARE_BD_ADDR(InquiryResultList[(TempParam->Params[0].intParam - 1)], NullADDR)))
      {
         /* Determine the correct Operation ID.                         */

         /* Now check to make sure that the Profile exists at this      */
         /* instance.                                                   */
         if((ProfileInfo.ProfileID) && (ProfileInfo.Connected))
         {
            /* Format up a Get Player Application Setting Value Text    */
            /* Generic Command (for demonstration purposes).            */
            GetPlayerApplicationSettingValueTextCommandData.AttributeID    = AVRCP_PLAYER_SETTING_ATTRIBUTE_ID_SHUFFLE_ON_OFF_STATUS;
            GetPlayerApplicationSettingValueTextCommandData.NumberValueIDs = 3;
            GetPlayerApplicationSettingValueTextCommandData.ValueIDList    = ValueIDList;
            GetPlayerApplicationSettingValueTextCommandData.ValueIDList[0] = AVRCP_PLAYER_SETTING_VALUE_ID_SHUFFLE_STATUS_OFF;
            GetPlayerApplicationSettingValueTextCommandData.ValueIDList[1] = AVRCP_PLAYER_SETTING_VALUE_ID_SHUFFLE_STATUS_ALL_TRACKS;
            GetPlayerApplicationSettingValueTextCommandData.ValueIDList[2] = AVRCP_PLAYER_SETTING_VALUE_ID_SHUFFLE_STATUS_GROUP;

            /* Format a Get Application Settings Command data           */
            Status = AVRCP_Format_Get_Player_Application_Setting_Value_Text_Command(BluetoothStackID, &GetPlayerApplicationSettingValueTextCommandData, sizeof(Buffer), (unsigned char *)Buffer);
            printf("AVRCP_Format_Get_Player_Application_Setting_Value_Text_Command: %d\r\n", Status);
            if(Status > 0)
            {
               /* Attempt to send the data.                             */
               if((Status = AVCTP_Send_Message(BluetoothStackID, ProfileInfo.ProfileID, InquiryResultList[(TempParam->Params[0].intParam - 1)], (Byte_t)(TempParam->Params[1].intParam & 0x0F), FALSE, Status, Buffer)) != 0)
               {
                  printf("Send Message Failure Error: %d.\r\n", Status);

                  ret_val = Status;
               }
               else
               {
                  printf("Send Message Successful.\r\n");

                  ret_val = 0;
               }
            }
            else
            {
               printf("AVRCP_Format_Vendor_Dependent_Generic_Command: %d.\r\n", Status);

               ret_val = Status;
            }
         }
         else
         {
            /* Instance is either not present or not currently          */
            /* connected.                                               */
            printf("No Profile Instance present or Profile not connected.\r\n");

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         printf("Usage: SendGetAppSettingsValueTextMessage [Inquiry Index] [Transaction ID (0 <= 15)].\r\n");

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

   /* The following function is responsible for sending AVCTP Browsing  */
   /* Channel Message Data to a remote AVCTP Profile instance (on the   */
   /* Browsing Channel).  This function returns zero if successful and a*/
   /* negative value if an error occurred.                              */
static int SendBrowsingMessage(ParameterList_t *TempParam)
{
   int                                     ret_val;
   int                                     Status;
   BD_ADDR_t                               NullADDR;
   unsigned char                           Buffer[48];
   AVRCP_Set_Browsed_Player_Command_Data_t SetBrowsedPlayerCommandData;

   ASSIGN_BD_ADDR(NullADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the Profile Instance passed to this*/
      /* function is valid.                                             */
      if((TempParam) && (TempParam->NumberofParameters >= 2) && (TempParam->Params[0].intParam <= NumberofValidResponses) && (!COMPARE_BD_ADDR(InquiryResultList[(TempParam->Params[0].intParam - 1)], NullADDR)))
      {
         /* Now check to make sure that the Profile exists at this      */
         /* instance.                                                   */
         if((ProfileInfo.ProfileID) && (ProfileInfo.Connected))
         {
            /* Format up a Set Browsed Player Command (for demonstration*/
            /* purposes).                                               */
            SetBrowsedPlayerCommandData.PlayerID = 0x1234;
            Status = AVRCP_Format_Set_Browsed_Player_Command(BluetoothStackID, &SetBrowsedPlayerCommandData, sizeof(Buffer), (unsigned char *)Buffer);
            printf("AVRCP_Format_Set_Browsed_Player_Command: %d\r\n", Status);
            if(Status > 0)
            {
               /* Attempt to send the data.                             */
               if((Status = AVCTP_Send_Browsing_Channel_Message(BluetoothStackID, ProfileInfo.ProfileID, InquiryResultList[(TempParam->Params[0].intParam - 1)], (Byte_t)(TempParam->Params[1].intParam & 0x0F), FALSE, Status, Buffer)) != 0)
               {
                  printf("Send Browsing Channel Message Failure Error: %d.\r\n", Status);

                  ret_val = Status;
               }
               else
               {
                  printf("Send Browsing Channel Message Successful.\r\n");

                  ret_val = 0;
               }
            }
            else
            {
               printf("AVRCP_Format_Set_Browsed_Player_Command: %d.\r\n", Status);

               ret_val = Status;
            }
         }
         else
         {
            /* Instance is either not present or not currently          */
            /* connected.                                               */
            printf("No Profile Instance present or Profile not connected.\r\n");

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         printf("Usage: SendBrowsingMessage [Inquiry Index] [Transaction ID (0 <= 15)].\r\n");

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

   /* The following function is reponsible for generating and sending a */
   /* Unit Info Response to the remote AVRCP Device.                    */
static void SendUnitInfoResponse(BD_ADDR_t RemoteBD_ADDR, int ProfileID, Byte_t TransactionID, Byte_t SubunitType, Byte_t SubunitID)
{
   int                             ret_val;
   Byte_t                          Data[AVRCP_UNIT_INFO_RESPONSE_SIZE];
   AVRCP_Unit_Info_Response_Data_t UnitInfoResponseData;

   /* Assign the parameters to the Info Response Data Structure.        */
   UnitInfoResponseData.ResponseCode = AVRCP_RESPONSE_STABLE;
   UnitInfoResponseData.SubunitType  = SubunitType;
   UnitInfoResponseData.SubunitID    = SubunitID;
   UnitInfoResponseData.UnitType     = AVRCP_SUBUNIT_TYPE_PANEL;
   UnitInfoResponseData.Unit         = 0;
   ASSIGN_AVRCP_COMPANY_ID(UnitInfoResponseData.CompanyID, 0x12, 0x33, 0x21);

   /* Format the Response into the supplied Buffer.                     */
   ret_val = AVRCP_Format_Unit_Info_Response(BluetoothStackID, &UnitInfoResponseData, AVRCP_UNIT_INFO_RESPONSE_SIZE, Data);
   if(ret_val > 0)
   {
      /* Attempt to send the response data.                             */
      if((ret_val = AVCTP_Send_Message(BluetoothStackID, ProfileID, RemoteBD_ADDR, (Byte_t)TransactionID, TRUE, ret_val, Data)) != 0)
         printf("Send message failure Error: %d.\r\n", ret_val);
      else
         printf("Send Message Successful.\r\n");
   }
   else
      printf("Format Unit Info Response failed with error %d.\r\n", ret_val);
}

   /* The following function is reponsible for generating and sending a */
   /* Subunit Info Response to the remote AVRCP Device.                 */
static void SendSubunitInfoResponse(BD_ADDR_t RemoteBD_ADDR, int ProfileID, Byte_t TransactionID, Byte_t SubunitType, Byte_t SubunitID, Byte_t PageNumber)
{
   int                                ret_val;
   Byte_t                             Data[AVRCP_SUBUNIT_INFO_RESPONSE_SIZE];
   AVRCP_Subunit_Table_List_Entry_t   ListEntry;
   AVRCP_Subunit_Info_Response_Data_t SubunitInfoResponseData;

   /* Assign the parameters to the Subunit Info Response Data Structure.*/
   SubunitInfoResponseData.ResponseCode         = AVRCP_RESPONSE_STABLE;
   SubunitInfoResponseData.SubunitType          = SubunitType;
   SubunitInfoResponseData.SubunitID            = SubunitID;
   SubunitInfoResponseData.NumberSubunitEntries = 1;
   SubunitInfoResponseData.Page                 = PageNumber;
   SubunitInfoResponseData.SubunitTable         = &ListEntry;
   ListEntry.SubunitType                        = AVRCP_SUBUNIT_TYPE_PANEL;
   ListEntry.MaxSubunitID                       = 0;

   /* Format the Data into a Response Packet.                           */
   ret_val = AVRCP_Format_Subunit_Info_Response(BluetoothStackID, &SubunitInfoResponseData, AVRCP_SUBUNIT_INFO_RESPONSE_SIZE, Data);
   if(ret_val > 0)
   {
      /* Attempt to send the data.                                      */
      if((ret_val = AVCTP_Send_Message(BluetoothStackID, ProfileID, RemoteBD_ADDR, (Byte_t)TransactionID, TRUE, ret_val, Data)) != 0)
         printf("Send message failure Error: %d.\r\n", ret_val);
      else
         printf("Send Message Successful.\r\n");
   }
   else
      printf("Format Subunit Info Response failed with error %d.\r\n", ret_val);
}

   /* The following function is reponsible for generating and sending a */
   /* Pass-through Response to the remote AVRCP Device.                 */
static void SendPassThroughResponse(BD_ADDR_t RemoteBD_ADDR, int ProfileID, Byte_t TransactionID, Byte_t SubunitType, Byte_t SubunitID, Byte_t OperationID, Boolean_t StateFlag)
{
   int                                 ret_val;
   Byte_t                              Data[AVRCP_CALCULATE_PASS_THROUGH_RESPONSE_SIZE(0)];
   AVRCP_Pass_Through_Response_Data_t  PassThroughResponseData;

   /* Assign the parameters to the Pass Through Info Response Data      */
   /* Structure.                                                        */
   PassThroughResponseData.ResponseCode        = AVRCP_RESPONSE_ACCEPTED;
   PassThroughResponseData.SubunitType         = SubunitType;
   PassThroughResponseData.SubunitID           = SubunitID;
   PassThroughResponseData.OperationID         = OperationID;
   PassThroughResponseData.StateFlag           = StateFlag;
   PassThroughResponseData.OperationDataLength = 0;
   PassThroughResponseData.OperationData       = NULL;

   /* Format the Data into a Response Packet.                           */
   ret_val = AVRCP_Format_Pass_Through_Response(BluetoothStackID, &PassThroughResponseData, AVRCP_CALCULATE_PASS_THROUGH_RESPONSE_SIZE(0), Data);
   if(ret_val > 0)
   {
      /* Attempt to send the data.                                      */
      if((ret_val = AVCTP_Send_Message(BluetoothStackID, ProfileID, RemoteBD_ADDR, (Byte_t)TransactionID, TRUE, ret_val, Data)) != 0)
         printf("Send message failure Error: %d.\r\n", ret_val);
      else
         printf("Send Message Successful.\r\n");
   }
   else
      printf("Format Pass Through Response failed with error %d.\r\n", ret_val);
}

   /* The following function is reponsible for generating and sending a */
   /* Vendor Dependent Response to the remote AVRCP Device.             */
static void SendVendorDependentResponse(BD_ADDR_t RemoteBD_ADDR, int ProfileID, Byte_t TransactionID, Byte_t SubunitType, Byte_t SubunitID, AVRCP_Company_ID_t CompanyID)
{
   int                                            ret_val;
   Byte_t                                         Data[AVRCP_CALCULATE_VENDOR_DEPENDENT_RESPONSE_SIZE(0)];
   AVRCP_Vendor_Dependent_Generic_Response_Data_t VendorDependentResponseData;

   /* Assign the parameters to the Vendor Dependent Response Data       */
   /* Structure.                                                        */
   VendorDependentResponseData.ResponseCode = AVRCP_RESPONSE_ACCEPTED;
   VendorDependentResponseData.SubunitType  = SubunitType;
   VendorDependentResponseData.SubunitID    = SubunitID;
   VendorDependentResponseData.CompanyID    = CompanyID;
   VendorDependentResponseData.DataLength   = 0;
   VendorDependentResponseData.DataBuffer   = NULL;

   /* Format the Data into a Response Packet.                           */
   ret_val = AVRCP_Format_Vendor_Dependent_Generic_Response(BluetoothStackID, &VendorDependentResponseData, AVRCP_CALCULATE_VENDOR_DEPENDENT_RESPONSE_SIZE(0), Data);
   if(ret_val > 0)
   {
      /* Attempt to send the data.                                      */
      if((ret_val = AVCTP_Send_Message(BluetoothStackID, ProfileID, RemoteBD_ADDR, (Byte_t)TransactionID, TRUE, ret_val, Data)) != 0)
         printf("Send message failure Error: %d.\r\n", ret_val);
      else
         printf("Send Message Successful.\r\n");
   }
   else
      printf("Format Vendor Dependent Generic Response failed with error %d.\r\n", ret_val);
}

   /* The following function is reponsible for generating and sending a */
   /* Get Application Settings Text Value Response to the remote AVRCP  */
   /* Device.                                                           */
static void SendGetApplicationSettingsTextValueResponse(BD_ADDR_t RemoteBD_ADDR, int ProfileID, Byte_t TransactionID, Byte_t NumberEntries, AVRCP_Response_Message_State_Info_t *MessageStateInfo)
{
   int                                                                    i, ret_val;
   Byte_t                                                                 Data[AVRCP_MAXIMUM_NON_BROWSING_MESSAGE_SIZE];
   static Byte_t                                                          ValueText[VALUE_TEXT_LENGTH];
   static AVRCP_Value_Text_List_Entry_t                                   ValueList[3];
   static AVRCP_Get_Player_Application_Setting_Value_Text_Response_Data_t GetPlayerApplicationSettingValueTextResponseData;

   /* Check to see if this is the first time that this has been called. */
   if(!MessageStateInfo->Offset)
   {
      /* Assign the parameters to the Application Settings Text Value   */
      /* Response Data Structure.                                       */
      GetPlayerApplicationSettingValueTextResponseData.ResponseCode           = AVRCP_RESPONSE_ACCEPTED;
      GetPlayerApplicationSettingValueTextResponseData.NumberValueTextEntries = NumberEntries;
      GetPlayerApplicationSettingValueTextResponseData.ValueTextEntryList     = (AVRCP_Value_Text_List_Entry_t *)ValueList;

      /* Copy the long text to a buffer.                                */
      BTPS_MemCopy(ValueText, VALUE_TEXT, VALUE_TEXT_LENGTH);

      for(i=0;i<NumberEntries;i++)
      {
         ValueList[i].ValueID           = (Byte_t)(i+1);
         ValueList[i].ValueStringLength = VALUE_TEXT_LENGTH;
         ValueList[i].ValueStringData   = ValueText;
         ValueList[i].CharacterSet      = SDP_UTF_8_CHARACTER_ENCODING;
      }
   }

   /* Initialize the State Info Structure.                              */
   MessageStateInfo->Complete = FALSE;

   /* Format the Data into a Response Packet.  Note that since this     */
   /* can/will be fragmented, all of the information that is used for   */
   /* this command must be in static variables.                         */
   ret_val = AVRCP_Format_Get_Player_Application_Setting_Value_Text_Response(BluetoothStackID, MessageStateInfo, &GetPlayerApplicationSettingValueTextResponseData, sizeof(Data), Data);
   if(ret_val > 0)
   {
      /* Attempt to send the data.                                      */
      if((ret_val = AVCTP_Send_Message(BluetoothStackID, ProfileID, RemoteBD_ADDR, (Byte_t)TransactionID, TRUE, ret_val, Data)) != 0)
         printf("Send message failure Error: %d.\r\n", ret_val);
      else
      {
         printf("Offset %d Complete %d\r\n", MessageStateInfo->Offset, MessageStateInfo->Complete);

         printf("Send Message Successful.\r\n");
      }
   }
   else
      printf("Format Get Application Setting Text Value Response failed with error %d.\r\n", ret_val);
}

   /* The following function is reponsible for generating and sending a */
   /* Set Browsed Player Response (Browsing Channel) to the remote AVRCP*/
   /* Device.                                                           */
static void SendSetBrowsedPlayerResponse(BD_ADDR_t RemoteBD_ADDR, int ProfileID, Byte_t TransactionID)
{
   int                                       Result;
   Byte_t                                    Buffer[128];
   AVRCP_Folder_Name_List_Entry_t            FolderNameList[2];
   AVRCP_Set_Browsed_Player_Response_Data_t  SetBrowsedPlayerResponseData;

   /* Assign the parameters to the Set Browsed Player Response Data     */
   /* Structure.                                                        */
   SetBrowsedPlayerResponseData.Status         = AVRCP_COMMAND_ERROR_STATUS_CODE_COMPLETE_NO_ERROR;
   SetBrowsedPlayerResponseData.UIDCounter     = 0x1234;
   SetBrowsedPlayerResponseData.NumberItems    = 0x77665544;
   SetBrowsedPlayerResponseData.CharacterSet   = SDP_UTF_8_CHARACTER_ENCODING;
   SetBrowsedPlayerResponseData.FolderDepth    = 2;
   SetBrowsedPlayerResponseData.FolderNameList = FolderNameList;

   FolderNameList[0].FolderName       = (Byte_t *)"Main Folder";
   FolderNameList[0].FolderNameLength = (Word_t)(strlen((char *)FolderNameList[0].FolderName) + 1);

   FolderNameList[1].FolderName       = (Byte_t *)"Sub Folder";
   FolderNameList[1].FolderNameLength = (Word_t)(strlen((char *)FolderNameList[1].FolderName) + 1);

   Result = AVRCP_Format_Set_Browsed_Player_Response(BluetoothStackID, AVRCP_MINIMUM_BROWSING_CHANNEL_MTU, &SetBrowsedPlayerResponseData, sizeof(Buffer), (unsigned char *)Buffer);
   if(Result > 0)
   {
      /* Attempt to send the data.                                      */
      if((Result = AVCTP_Send_Browsing_Channel_Message(BluetoothStackID, ProfileID, RemoteBD_ADDR, (Byte_t)TransactionID, TRUE, Result, Buffer)) != 0)
         printf("Send Browsing Channel Message Failure Error: %d.\r\n", Result);
      else
         printf("Send Browsing Channel Message Successful (Set Browsed Player Response).\r\n");
   }
   else
      printf("AVRCP_Format_Set_Browsed_Player_Response() failed with error %d.\r\n", Result);
}

   /* The following function is reponsible for generating and sending an*/
   /* Abort Continuing Response Response to the remote AVRCP Device.    */
static void SendAbortContinuingResponseResponse(BD_ADDR_t RemoteBD_ADDR, int ProfileID, Byte_t TransactionID)
{
   int                                             Result;
   Byte_t                                          Buffer[128];
   AVRCP_Abort_Continuing_Response_Response_Data_t AbortContinuingResponseResponseData;

   /* Assign the parameters to the Abort Continuing Response Response   */
   /* Data Structure.                                                   */
   AbortContinuingResponseResponseData.ResponseCode = AVRCP_RESPONSE_ACCEPTED;

   Result = AVRCP_Format_Abort_Continuing_Response_Response(BluetoothStackID, &AbortContinuingResponseResponseData, sizeof(Buffer), (unsigned char *)Buffer);
   if(Result > 0)
   {
      /* Attempt to send the data.                                      */
      if((Result = AVCTP_Send_Message(BluetoothStackID, ProfileID, RemoteBD_ADDR, (Byte_t)TransactionID, TRUE, Result, Buffer)) != 0)
         printf("Send Abort Continuing Response Response Message Failure Error: %d.\r\n", Result);
      else
         printf("Send Abort Continuing Response Response Message Successful (Command Reject).\r\n");
   }
   else
      printf("AVRCP_Format_Abort_Continuing_Response_Response() failed with error %d.\r\n", Result);
}

   /* The following function is reponsible for generating and sending a */
   /* Command Reject Response to the remote AVRCP Device.               */
static void SendCommandRejectResponse(BD_ADDR_t RemoteBD_ADDR, int ProfileID, Byte_t TransactionID, AVRCP_Message_Type_t MessageType, Byte_t PDU_ID, Byte_t ErrorCode)
{
   int                                   Result;
   Byte_t                                Buffer[128];
   AVRCP_Command_Reject_Response_Data_t  CommandRejectResponseData;

   /* Assign the parameters to the Command Reject Response Data         */
   /* Structure.                                                        */
   CommandRejectResponseData.MessageType  = MessageType;
   CommandRejectResponseData.ResponseCode = AVRCP_RESPONSE_REJECTED;
   CommandRejectResponseData.PDU_ID       = PDU_ID;
   CommandRejectResponseData.ErrorCode    = ErrorCode;

   Result = AVRCP_Format_Command_Reject_Response(BluetoothStackID, &CommandRejectResponseData, sizeof(Buffer), (unsigned char *)Buffer);
   if(Result > 0)
   {
      /* Attempt to send the data.                                      */
      if((Result = AVCTP_Send_Message(BluetoothStackID, ProfileID, RemoteBD_ADDR, (Byte_t)TransactionID, TRUE, Result, Buffer)) != 0)
         printf("Send Command Reject Response Message Failure Error: %d.\r\n", Result);
      else
         printf("Send Command Reject Response Message Successful (Command Reject).\r\n");
   }
   else
      printf("AVRCP_Format_Command_Reject_Response() failed with error %d.\r\n", Result);
}

   /* The following function is reponsible for generating and sending a */
   /* General Reject Response (Browsing Channel) to the remote AVRCP    */
   /* Device.                                                           */
static void SendGeneralRejectResponse(BD_ADDR_t RemoteBD_ADDR, int ProfileID, Byte_t TransactionID)
{
   int                                   Result;
   Byte_t                                Buffer[128];
   AVRCP_General_Reject_Response_Data_t  GeneralRejectResponseData;

   /* Assign the parameters to the General Reject Response Data         */
   /* Structure.                                                        */
   GeneralRejectResponseData.RejectReason = AVRCP_COMMAND_ERROR_STATUS_CODE_INVALID_COMMAND;

   Result = AVRCP_Format_General_Reject_Response(BluetoothStackID, &GeneralRejectResponseData, sizeof(Buffer), (unsigned char *)Buffer);
   if(Result > 0)
   {
      /* Attempt to send the data.                                      */
      if((Result = AVCTP_Send_Browsing_Channel_Message(BluetoothStackID, ProfileID, RemoteBD_ADDR, (Byte_t)TransactionID, TRUE, Result, Buffer)) != 0)
         printf("Send Browsing Channel Message Failure Error: %d.\r\n", Result);
      else
         printf("Send Browsing Channel Message Successful (General Reject).\r\n");
   }
   else
      printf("AVRCP_Format_General_Reject_Response() failed with error %d.\r\n", Result);
}

   /* The following function is a utility function that exists to       */
   /* display and parse the specified AVRCP Message data.  In the case  */
   /* of a Request (specified by fourth parameter), this function will  */
   /* automatically respond with the correct response.                  */
static void HandleReceivedMessage(BD_ADDR_t RemoteBD_ADDR, int ProfileID, Byte_t TransactionID, Byte_t MessageType, unsigned int DataLength, Byte_t *DataBuffer)
{
   int                                               i, ret_val;
   long                                              Result;
   Byte_t                                            CType;
   Byte_t                                            ResponseCode;
   Byte_t                                            SubunitType;
   Byte_t                                            SubunitID;
   Byte_t                                            UnitType;
   Byte_t                                            Unit;
   Byte_t                                            PageNumber;
   Byte_t                                            SubunitInstanceType;
   Byte_t                                            SubunitInstanceID;
   Byte_t                                            OperationID;
   Byte_t                                            OperationDataLength;
   Byte_t                                            ID;
   Byte_t                                           *Ptr;
   Byte_t                                            PassThroughBuffer[AVRCP_CALCULATE_PASS_THROUGH_RESPONSE_HEADER_SIZE];
   Byte_t                                            ContinueData[AVRCP_REQUEST_CONTINUING_RESPONSE_COMMAND_PDU_SIZE];
   Boolean_t                                         StateFlag;
   static int                                        ListIndex = 0;
   static int                                        Entries;
   AVRCP_Company_ID_t                                CompanyID;
   AVRCP_Message_Information_t                      *Message;
   AVRCP_Value_Text_List_Entry_t                    *ValueListPtr;
   AVRCP_Pass_Through_Command_Data_t                 PassThroughCommandData;
   static AVRCP_Message_Information_t                MessageList[MAX_NUMBER_OF_FRAGMENTS];
   AVRCP_Request_Continuing_Response_Command_Data_t  RequestContinuingResponseCommandData;

   /* Decode the Message that was received.                             */
   Message = &MessageList[ListIndex];
   AVRCP_Decode_Message(BluetoothStackID, FALSE, (Boolean_t)(MessageType == AVCTP_MESSAGE_TYPE_RESPONSE), DataLength, DataBuffer, Message);

   /* Check to see if this all of the message has been received.        */
   if(Message->FinalMessage)
   {
      /* Verify that this is not the last part of a fragmented message. */
      if(Message->MessageType != amtFragmentedMessage)
      {
         /* Check to see if this is a response to a command.            */
         if(Message->Response)
         {
            printf("\r\nRESPONSE Message Received.\r\n");

            /* Process the received message response.                   */
            switch(Message->MessageType)
            {
               case amtUnitInfo:
                  /* Print out the Unit Information.                    */
                  ResponseCode  = Message->MessageInformation.UnitInfoResponseData.ResponseCode;
                  SubunitType   = Message->MessageInformation.UnitInfoResponseData.SubunitType;
                  SubunitID     = Message->MessageInformation.UnitInfoResponseData.SubunitID;

                  printf("\r\nResponse Code: 0x%02X  \r\nSubunit Type: 0x%02X \r\nSubunitID: 0x%02X\r\n", ResponseCode, SubunitType, SubunitID);

                  UnitType      = Message->MessageInformation.UnitInfoResponseData.UnitType;
                  Unit          = Message->MessageInformation.UnitInfoResponseData.Unit;
                  CompanyID     = Message->MessageInformation.UnitInfoResponseData.CompanyID;

                  printf("\r\nUNIT INFO RESPONSE => Unit Type: 0x%02X  Unit: 0x%02X CompanyID: 0x%02X%02X%02X\r\n", UnitType, Unit, CompanyID.CompanyID0, CompanyID.CompanyID1, CompanyID.CompanyID2);
                  break;
               case amtSubunitInfo:
                  /* Print out the Subunit Information.                 */
                  ResponseCode  = Message->MessageInformation.SubunitInfoResponseData.ResponseCode;
                  SubunitType   = Message->MessageInformation.SubunitInfoResponseData.SubunitType;
                  SubunitID     = Message->MessageInformation.SubunitInfoResponseData.SubunitID;

                  printf("\r\nResponse Code: 0x%02X  \r\nSubunit Type: 0x%02X \r\nSubunitID: 0x%02X\r\n", ResponseCode, SubunitType, SubunitID);

                  PageNumber    = Message->MessageInformation.SubunitInfoResponseData.Page;
                  Entries       = Message->MessageInformation.SubunitInfoResponseData.NumberSubunitEntries;
                  printf("\r\nSUBUNIT INFO RESPONSE => Page Number: %d, containing %d Subunit entries\r\n", PageNumber, Entries);

                  for(i=0;i<Entries;i++)
                  {
                     SubunitInstanceType = Message->MessageInformation.SubunitInfoResponseData.SubunitTable[i].SubunitType;
                     SubunitInstanceID   = Message->MessageInformation.SubunitInfoResponseData.SubunitTable[i].MaxSubunitID;

                     printf("%d) Subunit Instance Type: 0x%02X Subunit Instance ID: 0x%02X\r\n", i+1, SubunitInstanceType, SubunitInstanceID);
                  }
                  break;
               case amtPassThrough:
                  /* Print out the pass through Information.            */
                  ResponseCode  = Message->MessageInformation.PassThroughResponseData.ResponseCode;
                  SubunitType   = Message->MessageInformation.PassThroughResponseData.SubunitType;
                  SubunitID     = Message->MessageInformation.PassThroughResponseData.SubunitID;

                  printf("\r\nResponse Code: 0x%02X  \r\nSubunit Type: 0x%02X \r\nSubunitID: 0x%02X\r\n", ResponseCode, SubunitType, SubunitID);

                  OperationID         = Message->MessageInformation.PassThroughResponseData.OperationID;
                  StateFlag           = Message->MessageInformation.PassThroughResponseData.StateFlag;
                  OperationDataLength = Message->MessageInformation.PassThroughResponseData.OperationDataLength;

                  printf("\r\nPASS THROUGH RESPONSE => OperationID: 0x%02X State Flag: 0x%02X OperationDataLength: 0x%02X\r\n",  OperationID, StateFlag, OperationDataLength);

                  /* If we have receive the Passthrough Response and    */
                  /* with the button pressed state as Down, we will     */
                  /* automatically go ahead and send another one with   */
                  /* button pressed state as Up.                        */
                  if(!StateFlag)
                  {
                     PassThroughCommandData.CommandType         = AVRCP_CTYPE_CONTROL;
                     PassThroughCommandData.SubunitType         = SubunitType;
                     PassThroughCommandData.SubunitID           = SubunitID;
                     PassThroughCommandData.OperationID         = OperationID;
                     PassThroughCommandData.StateFlag           = TRUE;
                     PassThroughCommandData.OperationDataLength = 0;
                     PassThroughCommandData.OperationData       = NULL;

                     /* Format the Command from the Command Data.       */
                     ret_val = AVRCP_Format_Pass_Through_Command(BluetoothStackID, &PassThroughCommandData, sizeof(PassThroughBuffer), PassThroughBuffer);
                     if(ret_val > 0)
                     {
                        /* Attempt to send the data.                    */
                        if((ret_val = AVCTP_Send_Message(BluetoothStackID, ProfileID, RemoteBD_ADDR, (Byte_t)((TransactionID + 1) & 0x0F), FALSE, ret_val, PassThroughBuffer)) != 0)
                           printf("Send message failure Error: %d.\r\n", ret_val);
                        else
                           printf("Send Message Successful.\r\n");
                     }
                     else
                        printf("Format Command Message failed with error %d.\r\n", ret_val);
                  }
                  break;
               case amtVendorDependent_Generic:
                  /* Print out the vendor Specific Information.         */
                  ResponseCode  = Message->MessageInformation.VendorDependentGenericResponseData.ResponseCode;
                  SubunitType   = Message->MessageInformation.VendorDependentGenericResponseData.SubunitType;
                  SubunitID     = Message->MessageInformation.VendorDependentGenericResponseData.SubunitID;

                  printf("\r\nResponse Code: 0x%02X  \r\nSubunit Type: 0x%02X \r\nSubunitID: 0x%02X\r\n", ResponseCode, SubunitType, SubunitID);

                  CompanyID  = Message->MessageInformation.VendorDependentGenericResponseData.CompanyID;
                  DataLength = Message->MessageInformation.VendorDependentGenericResponseData.DataLength;
                  Ptr        = Message->MessageInformation.VendorDependentGenericResponseData.DataBuffer;

                  printf("\r\nVENDOR SPECIFIC RESPONSE => Vendor Specific Data of length %d for CompanyID: 0x%02X%02X%02X\r\n", DataLength, CompanyID.CompanyID0, CompanyID.CompanyID1, CompanyID.CompanyID2);

                  /* Display and Vendor Specific Information.           */
                  if(DataLength)
                  {
                     for(i=0;i<(int)DataLength;i++,Ptr++)
                     {
                        printf("0x%02X ", *Ptr);
                        if((((i+1) % 8) == 0) || (i == (int)(DataLength-1)))
                           printf("\r\n");
                     }
                     printf("\r\n");
                  }
                  break;
               case amtGetPlayerApplicationSettingValueText:
                  /* Print out the Capabilies Information.              */
                  ResponseCode = Message->MessageInformation.GetPlayerApplicationSettingValueTextResponseData.ResponseCode;
                  Entries      = Message->MessageInformation.GetPlayerApplicationSettingValueTextResponseData.NumberValueTextEntries;
                  ValueListPtr = Message->MessageInformation.GetPlayerApplicationSettingValueTextResponseData.ValueTextEntryList;

                  printf("\r\nResponse Code: 0x%02X  \r\nEntries: %d\r\n", ResponseCode, Entries);

                  for(i=0;i<Entries;i++,ValueListPtr++)
                  {
                     printf("\r\nValue ID: 0x%02X Value Text Length: %d\r\n", ValueListPtr->ValueID, ValueListPtr->ValueStringLength);
                     printf((char *)ValueListPtr->ValueStringData);
                     printf("\r\n");
                  }
                  break;
               case amtCommandRejectResponse:
                  printf("\r\nResponse Code: 0x%02X  \r\n Message Type: 0x%02X  \r\n PDU_ID: 0x%02X \r\n Error Code: 0x%02X\r\n", Message->MessageInformation.CommandRejectResponseData.ResponseCode, Message->MessageInformation.CommandRejectResponseData.MessageType, Message->MessageInformation.CommandRejectResponseData.PDU_ID, Message->MessageInformation.CommandRejectResponseData.ErrorCode);
                  break;
               default:
                  if(Message->MessageType == amtUnknown)
                     printf("\r\nUnknown Message Type, PDU_ID: 0x%02X, Parameter Length: 0x%04X\r\n", Message->MessageInformation.UnknownCommandData.PDU_ID, Message->MessageInformation.UnknownCommandData.ParameterLength);
                  else
                     printf("\r\nUnhandled Message Type %d\r\n", Message->MessageType);
                  break;
            }
         }
         else
         {
            printf("\r\nCOMMAND Message Received.\r\n");

            switch(Message->MessageType)
            {
               case amtUnitInfo:
                  CType       = Message->MessageInformation.UnitInfoCommandData.CommandType;
                  SubunitType = Message->MessageInformation.UnitInfoCommandData.SubunitType;
                  SubunitID   = Message->MessageInformation.UnitInfoCommandData.SubunitID;

                  printf("Command Type: 0x%02X \r\nSubunit Type: 0x%02X \r\nSubunitID: 0x%02X\r\n", CType, SubunitType, SubunitID);

                  UnitType  = Message->MessageInformation.UnitInfoCommandData.UnitType;
                  Unit      = Message->MessageInformation.UnitInfoCommandData.Unit;

                  /* Print out the Unit Information.                    */
                  printf("\r\nUNIT INFO COMMAND => Unit Type: 0x%02X  Unit: 0x%02X\r\n\r\n", UnitType, Unit);

                  /* Send a response to the Unit Info Command.          */
                  SendUnitInfoResponse(RemoteBD_ADDR, ProfileID, TransactionID, SubunitType, SubunitID);
                  break;
               case amtSubunitInfo:
                  CType       = Message->MessageInformation.SubunitInfoCommandData.CommandType;
                  SubunitType = Message->MessageInformation.SubunitInfoCommandData.SubunitType;
                  SubunitID   = Message->MessageInformation.SubunitInfoCommandData.SubunitID;

                  printf("Command Type: 0x%02X \r\nSubunit Type: 0x%02X \r\nSubunitID: 0x%02X\r\n", CType, SubunitType, SubunitID);

                  /* Print out the Subunit Information.                 */
                  PageNumber    = Message->MessageInformation.SubunitInfoCommandData.Page;

                  printf("\r\nSUBUNIT INFO COMMAND => Page Number: %d\r\n", PageNumber);

                  /* Send a response to the Subunit Info Command.       */
                  SendSubunitInfoResponse(RemoteBD_ADDR, ProfileID, TransactionID, SubunitType, SubunitID, PageNumber);
                  break;
               case amtPassThrough:
                  CType       = Message->MessageInformation.PassThroughCommandData.CommandType;
                  SubunitType = Message->MessageInformation.PassThroughCommandData.SubunitType;
                  SubunitID   = Message->MessageInformation.PassThroughCommandData.SubunitID;

                  printf("Command Type: 0x%02X \r\nSubunit Type: 0x%02X \r\nSubunitID: 0x%02X\r\n", CType, SubunitType, SubunitID);

                  OperationID         = Message->MessageInformation.PassThroughCommandData.OperationID;
                  StateFlag           = Message->MessageInformation.PassThroughCommandData.StateFlag;
                  OperationDataLength = Message->MessageInformation.PassThroughCommandData.OperationDataLength;
                  DataBuffer          = Message->MessageInformation.PassThroughCommandData.OperationData;

                  printf("PASS THROUGH COMMAND => OperationID: 0x%02X StateFlag: 0x%02X OperationDataLength: 0x%02X\r\n",  OperationID, StateFlag, OperationDataLength);

                  /* Display and Passthrough Information.               */
                  if(OperationDataLength)
                  {
                     for(i=0;i<(int)OperationDataLength;i++)
                     {
                        printf("0x%02X ", *(DataBuffer+i));
                        if((((i+1) % 8) == 0) || (i == (int)(DataLength-1)))
                           printf("\r\n");
                     }
                     printf("\r\n");
                  }

                  /* Send a response to the Passthrough Command.        */
                  SendPassThroughResponse(RemoteBD_ADDR, ProfileID, TransactionID, SubunitType, SubunitID, OperationID, StateFlag);
                  break;
               case amtVendorDependent_Generic:
                  CType       = Message->MessageInformation.VendorDependentGenericCommandData.CommandType;
                  SubunitType = Message->MessageInformation.VendorDependentGenericCommandData.SubunitType;
                  SubunitID   = Message->MessageInformation.VendorDependentGenericCommandData.SubunitID;

                  printf("Command Type: 0x%02X \r\nSubunit Type: 0x%02X \r\nSubunitID: 0x%02X\r\n", CType, SubunitType, SubunitID);

                  CompanyID  = Message->MessageInformation.VendorDependentGenericCommandData.CompanyID;
                  DataLength = Message->MessageInformation.VendorDependentGenericCommandData.DataLength;
                  DataBuffer = Message->MessageInformation.VendorDependentGenericCommandData.DataBuffer;

                  printf("VENDOR SPECIFIC COMMAND => %d byte of Vender Data for Company ID: 0x%02X%02X%02X\r\n", DataLength, CompanyID.CompanyID0, CompanyID.CompanyID1, CompanyID.CompanyID2);

                  /* Display and Vendor Specific Information.           */
                  if(DataLength)
                  {
                     for(i=0;i<(int)DataLength;i++)
                     {
                        printf("0x%02X ", *(DataBuffer+i));
                        if((((i+1) % 8) == 0) || (i == (int)(DataLength-1)))
                           printf("\r\n");
                     }
                     printf("\r\n");
                  }

                  /* Send a response to the Vendor Generic Command.     */
                  SendVendorDependentResponse(RemoteBD_ADDR, ProfileID, TransactionID, SubunitType, SubunitID, CompanyID);
                  break;
               case amtGetPlayerApplicationSettingValueText:
                  /* Print out the Capabilies Information.              */
                  ID      = Message->MessageInformation.GetPlayerApplicationSettingValueTextCommandData.AttributeID;
                  Entries = Message->MessageInformation.GetPlayerApplicationSettingValueTextCommandData.NumberValueIDs;

                  printf("GET PLAYER APPLICATION SETTINGS VALUE TEXT COMMAND\r\nID: %02X Number of values: %d\r\n", ID, Entries);

                  /* Send the Response for the Application Settings     */
                  /* Value Text.                                        */
                  MessageStateInfo.Offset   = 0;
                  SendGetApplicationSettingsTextValueResponse(RemoteBD_ADDR, ProfileID, TransactionID, (Byte_t)Entries, &MessageStateInfo);
                  break;
               case amtRequestContinuingResponse:
                  /* We have received a command to continue with a      */
                  /* fragmented message.                                */
                  switch(Message->MessageInformation.RequestContinuingResponseCommandData.MessageType)
                  {
                     case amtGetPlayerApplicationSettingValueText:
                        printf("Request Continue.\r\n");

                        SendGetApplicationSettingsTextValueResponse(RemoteBD_ADDR, ProfileID, TransactionID, (Byte_t)Entries, &MessageStateInfo);
                        break;
                     case amtGetCapabilities:
                     case amtGetCurrentPlayerApplicationSettingValue:
                     case amtGetPlayerApplicationSettingAttributeText:
                     case amtGetElementAttributes:
                     default:
                        /* These commands are not currently supported by*/
                        /* this sample application.  Change the event   */
                        /* type to halt the fragmentation process and   */
                        /* cleanup any allocted data.                   */
                        Message->MessageType = 0;

                        /* We do not support this, go ahead and reject  */
                        /* the request.                                 */
                        SendCommandRejectResponse(RemoteBD_ADDR, ProfileID, TransactionID, amtRequestContinuingResponse, 0, AVRCP_COMMAND_ERROR_STATUS_CODE_INVALID_COMMAND);
                        break;
                  }
                  break;
               case amtAbortContinuingResponse:
                  /* Nothing really to do except halt the fragmentation */
                  /* process.                                           */
                  Message->MessageType = 0;

                  SendAbortContinuingResponseResponse(RemoteBD_ADDR, ProfileID, TransactionID);
                  break;
               default:
                  /* We do not support this, go ahead and reject the    */
                  /* request.                                           */
                  if(Message->MessageType == amtUnknown)
                  {
                     printf("\r\nUnknown Message Type, PDU_ID: 0x%02X, Parameter Length: 0x%04X\r\n", Message->MessageInformation.UnknownCommandData.PDU_ID, Message->MessageInformation.UnknownCommandData.ParameterLength);

                     SendCommandRejectResponse(RemoteBD_ADDR, ProfileID, TransactionID, amtUnknown, Message->MessageInformation.UnknownCommandData.PDU_ID, AVRCP_COMMAND_ERROR_STATUS_CODE_INVALID_COMMAND);
                  }
                  else
                  {
                     printf("\r\nUnhandled Message Type %d\r\n", Message->MessageType);

                     SendCommandRejectResponse(RemoteBD_ADDR, ProfileID, TransactionID, Message->MessageType, 0, AVRCP_COMMAND_ERROR_STATUS_CODE_INVALID_COMMAND);
                  }
                  break;
            }
         }

         /* Free any memory that was alloced during the decoding        */
         /* operation.                                                  */
         AVRCP_Free_Decoded_Message(BluetoothStackID, Message);
      }
      else
      {
         /* We have recevied the last fragment of the message so we will*/
         /* increment ListIndex and use this as a count of the number of*/
         /* fragments that we have collected.                           */
         ListIndex++;

         /* We have received the last fragment of a fragmented message. */
         /* We need to allocate a buffer that is large enough to        */
         /* reconstruct the entire original message.  To determine the  */
         /* amount of data needed, we will call the function with a NULL*/
         /* buffer and the value that is returned will be the amount    */
         /* that is needed.                                             */
         Result = AVRCP_Rebuild_Fragmented_Message(BluetoothStackID, ListIndex, MessageList, 0, NULL);
         printf("AVRCP_Rebuild_Fragmented_Message Memory Required: %ld from %d Fragments\r\n", Result, ListIndex);

         if(Result > 0)
         {
            /* Allocate a buffer for the message.                       */
            DataLength = (unsigned int)Result;
            DataBuffer = BTPS_AllocateMemory(DataLength);
            if(DataBuffer)
            {
               /* Perform the Actual rebuilding of the message into the */
               /* buffer.                                               */
               Result = AVRCP_Rebuild_Fragmented_Message(BluetoothStackID, ListIndex, MessageList, DataLength, DataBuffer);
               if(Result > 0)
               {
                  /* Free the memory that was allocated for the fragments.    */
                  AVRCP_Free_Fragmented_Message_List(BluetoothStackID, ListIndex, MessageList);
                  ListIndex = 0;

                  /* We have now rebuilt the fragments into an original */
                  /* message.  Make a recursive call to process this    */
                  /* message.  Increment the List Index so that the next*/
                  /* call can use a new entry in the list.              */
                  HandleReceivedMessage(RemoteBD_ADDR, ProfileID, TransactionID, MessageType, DataLength, DataBuffer);
               }
               else
               {
                  /* We need to cleanup the data that has been          */
                  /* allocated.                                         */
                  AVRCP_Free_Fragmented_Message_List(BluetoothStackID, ListIndex, MessageList);
                  ListIndex = 0;
               }

               /* Free the data that was allocated.                     */
               BTPS_FreeMemory(DataBuffer);
            }
            else
            {
               /* We failed to allocate the memory that we need.        */
               printf("ERROR: Failed to allocate memory.\r\n");
            }
         }
      }
   }
   else
   {
      /* The only time that we should be here is for a fragmented       */
      /* message so verify that this is a fragmented message.           */
      if(Message->MessageType == amtFragmentedMessage)
      {
         /* Increment the count of the number of fragments that have    */
         /* beed recevied.                                              */
         printf("Received Fragmented Message.\r\n");
         ListIndex++;

         switch(Message->MessageInformation.MessageFragmentData.PDU_ID)
         {
            case AVRCP_SPECIFIC_AV_C_COMMAND_PDU_ID_GET_CAPABILITIES:
               RequestContinuingResponseCommandData.MessageType = amtGetCapabilities;
               break;
            case AVRCP_SPECIFIC_AV_C_COMMAND_PDU_ID_GET_CURRENT_PLAYER_SETTING_VALUE:
               RequestContinuingResponseCommandData.MessageType = amtGetCurrentPlayerApplicationSettingValue;
               break;
            case AVRCP_SPECIFIC_AV_C_COMMAND_PDU_ID_GET_PLAYER_SETTING_ATTRIBUTE_TEXT:
               RequestContinuingResponseCommandData.MessageType = amtGetPlayerApplicationSettingAttributeText;
               break;
            case AVRCP_SPECIFIC_AV_C_COMMAND_PDU_ID_GET_PLAYER_SETTING_VALUE_TEXT:
               RequestContinuingResponseCommandData.MessageType = amtGetPlayerApplicationSettingValueText;
               break;
            case AVRCP_SPECIFIC_AV_C_COMMAND_PDU_ID_GET_ELEMENT_ATTRIBUTES:
               RequestContinuingResponseCommandData.MessageType = amtGetElementAttributes;
               break;
            default:
               /* Set the Message Type to an Invalid Value.             */
               RequestContinuingResponseCommandData.MessageType = 0;
               break;

         }

         /* Send a request to continue.                                 */
         ret_val = AVRCP_Format_Request_Continuing_Response_Command(BluetoothStackID, &RequestContinuingResponseCommandData, AVRCP_REQUEST_CONTINUING_RESPONSE_COMMAND_PDU_SIZE, ContinueData);
         if(ret_val > 0)
         {
            /* Attempt to send the data.                                */
            if((ret_val = AVCTP_Send_Message(BluetoothStackID, ProfileID, RemoteBD_ADDR, (Byte_t)TransactionID, FALSE, ret_val, ContinueData)) != 0)
            {
               printf("Send message failure Error: %d.\r\n", ret_val);

               /* Free the memory that was allocated for the fragments. */
               AVRCP_Free_Fragmented_Message_List(BluetoothStackID, ListIndex, MessageList);
            }
            else
               printf("Send Continue Request Successful.\r\n");
         }
      }
      else
         printf("ERROR: Non-Complete Message Type %d received\r\n", Message->MessageType);
   }
}

   /* The following function is a utility function that exists to       */
   /* display and parse the specified AVRCP Browsing Message data.  In  */
   /* the case of a Request (specified by fourth parameter), this       */
   /* function will automatically respond with the correct response.    */
static void HandleReceivedBrowsingMessage(BD_ADDR_t RemoteBD_ADDR, int ProfileID, Byte_t TransactionID, Byte_t MessageType, unsigned int DataLength, Byte_t *DataBuffer)
{
   int                         Result;
   unsigned int                Index;
   unsigned int                Index1;
   unsigned int                Index2;
   AVRCP_Message_Information_t Msg;

   /* Decode the Message that was received.                             */
   Result = AVRCP_Decode_Message(BluetoothStackID, TRUE, (Boolean_t)(MessageType == AVCTP_MESSAGE_TYPE_RESPONSE), DataLength, DataBuffer, &Msg);
   printf("AVRCP_Decode_Message: %d\r\n", Result);

   if(!Result)
   {
      /* Check to see if this is a response to a command.               */
      if(Msg.Response)
      {
         printf("\r\nBROWSING RESPONSE Message Received.\r\n");

         /* Process the received message response.                      */
         switch(Msg.MessageType)
         {
            case amtSetBrowsedPlayer:
               printf("Set Browsed Player\r\n");
               printf("Status: %02X, UID Counter: %04X, NumberItems: %08lX\r\n", Msg.MessageInformation.SetBrowsedPlayerResponseData.Status, Msg.MessageInformation.SetBrowsedPlayerResponseData.UIDCounter, Msg.MessageInformation.SetBrowsedPlayerResponseData.NumberItems);
               printf("Char Set: %04X, Folder Depth: %02X\r\n", Msg.MessageInformation.SetBrowsedPlayerResponseData.CharacterSet, Msg.MessageInformation.SetBrowsedPlayerResponseData.FolderDepth);

               for(Index=0;Index<(unsigned int)Msg.MessageInformation.SetBrowsedPlayerResponseData.FolderDepth;Index++)
               {
                  printf("(%04X ", Msg.MessageInformation.SetBrowsedPlayerResponseData.FolderNameList[Index].FolderNameLength);

                  for(Index1=0;Index1<(unsigned int)Msg.MessageInformation.SetBrowsedPlayerResponseData.FolderNameList[Index].FolderNameLength;Index1++)
                     printf("%c", Msg.MessageInformation.SetBrowsedPlayerResponseData.FolderNameList[Index].FolderName[Index1]);

                  printf(")\r\n");
               }
               break;
            case amtChangePath:
               printf("Change Path\r\n");
               printf("Status : %02X, Number Items: %08lX\r\n", Msg.MessageInformation.ChangePathResponseData.Status, Msg.MessageInformation.ChangePathResponseData.NumberItems);
               break;
            case amtGetItemAttributes:
               printf("Get Item Attributes\r\n");
               printf("Status: %02X, Number Attributes: %02X\r\n", Msg.MessageInformation.GetItemAttributesResponseData.Status, Msg.MessageInformation.GetItemAttributesResponseData.NumberAttributes);

               for(Index=0;Index<(unsigned int)Msg.MessageInformation.GetItemAttributesResponseData.NumberAttributes;Index++)
               {
                  printf("(%08lX, %04X, %04X : \r\n", Msg.MessageInformation.GetItemAttributesResponseData.AttributeValueList[Index].AttributeID,
                                                      Msg.MessageInformation.GetItemAttributesResponseData.AttributeValueList[Index].CharacterSet,
                                                      Msg.MessageInformation.GetItemAttributesResponseData.AttributeValueList[Index].AttributeValueLength);

                  for(Index1=0;Index1<(unsigned int)Msg.MessageInformation.GetItemAttributesResponseData.AttributeValueList[Index].AttributeValueLength;Index1++)
                     printf("%c", Msg.MessageInformation.GetItemAttributesResponseData.AttributeValueList[Index].AttributeValueData[Index1]);

                  printf(")\r\n");
               }
               break;
            case amtSearch:
               printf("Search\r\n");
               printf("Status: %02X, UID Counter: %04X, Number Items: %08lX\r\n", Msg.MessageInformation.SearchResponseData.Status, Msg.MessageInformation.SearchResponseData.UIDCounter, Msg.MessageInformation.SearchResponseData.NumberItems);
               break;
            case amtGeneralReject:
               printf("General Reject\r\n");
               printf("Reason: %02X\r\n", Msg.MessageInformation.GeneralRejectResponseData.RejectReason);
               break;
            case amtGetFolderItems:
               printf("Get Folder Items\r\n");
               printf("Status: %02X, UID Counter: %04X, Number Items: %04X\r\n", Msg.MessageInformation.GetFolderItemsResponseData.Status, Msg.MessageInformation.GetFolderItemsResponseData.UIDCounter, Msg.MessageInformation.GetFolderItemsResponseData.NumberItems);

               for(Index=0;Index<(unsigned int)Msg.MessageInformation.GetFolderItemsResponseData.NumberItems;Index++)
               {
                  switch(Msg.MessageInformation.GetFolderItemsResponseData.ItemList[Index].ItemType)
                  {
                     case AVRCP_BROWSABLE_ITEM_TYPE_MEDIA_PLAYER_ITEM:
                        printf("Index: %d, Media Player Item\r\n", Index);

                        printf("  Player ID: %04X\r\n", Msg.MessageInformation.GetFolderItemsResponseData.ItemList[Index].ItemData.MediaPlayerItemData.PlayerID);
                        printf("  Major Player Type: %02X\r\n", Msg.MessageInformation.GetFolderItemsResponseData.ItemList[Index].ItemData.MediaPlayerItemData.MajorPlayerType);
                        printf("  Player Sub Type: %08lX\r\n", Msg.MessageInformation.GetFolderItemsResponseData.ItemList[Index].ItemData.MediaPlayerItemData.PlayerSubType);
                        printf("  Play Status: %02X\r\n", Msg.MessageInformation.GetFolderItemsResponseData.ItemList[Index].ItemData.MediaPlayerItemData.PlayStatus);
                        printf("  Features Mask: ");
                        for(Index1=0;Index1<sizeof(AVRCP_Media_Player_Item_Features_Mask_t);Index1++)
                           printf("%02X", ((Byte_t *)&(Msg.MessageInformation.GetFolderItemsResponseData.ItemList[Index].ItemData.MediaPlayerItemData.FeaturesMask))[Index1]);
                        printf("\r\n");
                        printf("  Character Set: %04X\r\n", Msg.MessageInformation.GetFolderItemsResponseData.ItemList[Index].ItemData.MediaPlayerItemData.CharacterSet);
                        printf("  Displayable Name Length: %04X\r\n", Msg.MessageInformation.GetFolderItemsResponseData.ItemList[Index].ItemData.MediaPlayerItemData.DisplayableNameLength);

                        if(Msg.MessageInformation.GetFolderItemsResponseData.ItemList[Index].ItemData.MediaPlayerItemData.DisplayableNameLength)
                        {
                           printf("  Displayable Name: ");

                           for(Index1=0;Index1<(unsigned int)Msg.MessageInformation.GetFolderItemsResponseData.ItemList[Index].ItemData.MediaPlayerItemData.DisplayableNameLength;Index1++)
                              printf("%c", Msg.MessageInformation.GetFolderItemsResponseData.ItemList[Index].ItemData.MediaPlayerItemData.DisplayableName[Index1]);

                           printf("\r\n");
                        }
                        break;
                     case AVRCP_BROWSABLE_ITEM_TYPE_FOLDER_ITEM:
                        printf("Index: %d, Folder Item\r\n", Index);

                        printf("  Folder UID: %08lX%08lX\r\n", ((DWord_t *)&(Msg.MessageInformation.GetFolderItemsResponseData.ItemList[Index].ItemData.FolderItemData.FolderUID))[1], ((DWord_t *)&(Msg.MessageInformation.GetFolderItemsResponseData.ItemList[Index].ItemData.FolderItemData.FolderUID))[0]);
                        printf("  Folder Type: %02X\r\n", Msg.MessageInformation.GetFolderItemsResponseData.ItemList[Index].ItemData.FolderItemData.FolderType);
                        printf("  Is Playable: %02X\r\n", Msg.MessageInformation.GetFolderItemsResponseData.ItemList[Index].ItemData.FolderItemData.IsPlayable);
                        printf("  Character Set: %04X\r\n", Msg.MessageInformation.GetFolderItemsResponseData.ItemList[Index].ItemData.FolderItemData.CharacterSet);
                        printf("  Displayable Name Length: %04X\r\n", Msg.MessageInformation.GetFolderItemsResponseData.ItemList[Index].ItemData.FolderItemData.DisplayableNameLength);

                        if(Msg.MessageInformation.GetFolderItemsResponseData.ItemList[Index].ItemData.FolderItemData.DisplayableNameLength)
                        {
                           printf("  Displayable Name: ");

                           for(Index1=0;Index1<(unsigned int)Msg.MessageInformation.GetFolderItemsResponseData.ItemList[Index].ItemData.FolderItemData.DisplayableNameLength;Index1++)
                              printf("%c", Msg.MessageInformation.GetFolderItemsResponseData.ItemList[Index].ItemData.FolderItemData.DisplayableName[Index1]);

                           printf("\r\n");
                        }
                        break;
                     case AVRCP_BROWSABLE_ITEM_TYPE_MEDIA_ELEMENT_ITEM:
                        printf("Index: %d, Media Element Item\r\n", Index);

                        printf("  UID: %08lX%08lX\r\n", ((DWord_t *)&(Msg.MessageInformation.GetFolderItemsResponseData.ItemList[Index].ItemData.MediaElementItemData.UID))[1], ((DWord_t *)&(Msg.MessageInformation.GetFolderItemsResponseData.ItemList[Index].ItemData.MediaElementItemData.UID))[0]);
                        printf("  Media Type: %02X\r\n", Msg.MessageInformation.GetFolderItemsResponseData.ItemList[Index].ItemData.MediaElementItemData.MediaType);
                        printf("  Character Set: %04X\r\n", Msg.MessageInformation.GetFolderItemsResponseData.ItemList[Index].ItemData.MediaElementItemData.CharacterSet);
                        printf("  Displayable Name Length: %04X\r\n", Msg.MessageInformation.GetFolderItemsResponseData.ItemList[Index].ItemData.MediaElementItemData.DisplayableNameLength);

                        if(Msg.MessageInformation.GetFolderItemsResponseData.ItemList[Index].ItemData.MediaElementItemData.DisplayableNameLength)
                        {
                           printf("  Displayable Name: ");

                           for(Index1=0;Index1<(unsigned int)Msg.MessageInformation.GetFolderItemsResponseData.ItemList[Index].ItemData.MediaElementItemData.DisplayableNameLength;Index1++)
                              printf("%c", Msg.MessageInformation.GetFolderItemsResponseData.ItemList[Index].ItemData.MediaElementItemData.DisplayableName[Index1]);

                           printf("\r\n");
                        }

                        printf("  Number Attributes: %02X\r\n", Msg.MessageInformation.GetFolderItemsResponseData.ItemList[Index].ItemData.MediaElementItemData.NumberAttributes);

                        if(Msg.MessageInformation.GetFolderItemsResponseData.ItemList[Index].ItemData.MediaElementItemData.NumberAttributes)
                        {
                           for(Index1=0;Index1<(unsigned int)Msg.MessageInformation.GetFolderItemsResponseData.ItemList[Index].ItemData.MediaElementItemData.NumberAttributes;Index1++)
                           {
                              printf("    Attribute ID: %08lX\r\n", Msg.MessageInformation.GetFolderItemsResponseData.ItemList[Index].ItemData.MediaElementItemData.ElementAttributeList[Index1].AttributeID);
                              printf("    Character Set: %04X\r\n", Msg.MessageInformation.GetFolderItemsResponseData.ItemList[Index].ItemData.MediaElementItemData.ElementAttributeList[Index1].CharacterSet);
                              printf("    Attribute Value Length: %04X\r\n", Msg.MessageInformation.GetFolderItemsResponseData.ItemList[Index].ItemData.MediaElementItemData.ElementAttributeList[Index1].AttributeValueLength);

                              if(Msg.MessageInformation.GetFolderItemsResponseData.ItemList[Index].ItemData.MediaElementItemData.ElementAttributeList[Index1].AttributeValueLength)
                              {
                                 printf("  Attribute Value Data: ");

                                 for(Index2=0;Index2<(unsigned int)Msg.MessageInformation.GetFolderItemsResponseData.ItemList[Index].ItemData.MediaElementItemData.ElementAttributeList[Index1].AttributeValueLength;Index2++)
                                    printf("%c", Msg.MessageInformation.GetFolderItemsResponseData.ItemList[Index].ItemData.MediaElementItemData.ElementAttributeList[Index1].AttributeValueData[Index2]);

                                 printf("\r\n");
                              }
                           }
                        }
                        break;
                     default:
                        printf("Index: %d, Unknown Item Type: %02X\r\n", Index, Msg.MessageInformation.GetFolderItemsResponseData.ItemList[Index].ItemType);
                        break;
                  }
               }
               break;
            default:
               if(Msg.MessageType == amtUnknown)
                  printf("Unknown Message Type, PDU ID: 0x%02X, Length: 0x%04X\r\n", Msg.MessageInformation.UnknownCommandData.PDU_ID, Msg.MessageInformation.UnknownCommandData.ParameterLength);
               else
                  printf("Unhandled Message Type %d\r\n", Msg.MessageType);
               break;
         }
      }
      else
      {
         printf("\r\nBROWSING COMMAND Message Received.\r\n");

         switch(Msg.MessageType)
         {
            case amtSetBrowsedPlayer:
               printf("Set Browsed Player\r\n");
               printf("Player ID: %04X\r\n", Msg.MessageInformation.SetBrowsedPlayerCommandData.PlayerID);
               break;
            case amtChangePath:
               printf("Change Path\r\n");
               printf("UID Counter: %04X, Direction: %02X, Folder UID: %08lX%08lX\r\n", Msg.MessageInformation.ChangePathCommandData.UIDCounter, Msg.MessageInformation.ChangePathCommandData.Direction, ((DWord_t *)&(Msg.MessageInformation.ChangePathCommandData.FolderUID))[1], ((DWord_t *)&(Msg.MessageInformation.ChangePathCommandData.FolderUID))[0]);
               break;
            case amtGetItemAttributes:
               printf("Get Item Attributes\r\n");
               printf("Scope: %02X, UID: %08lX%08lX, UID Counter: %04X\r\n", Msg.MessageInformation.GetItemAttributesCommandData.Scope, ((DWord_t *)&(Msg.MessageInformation.GetItemAttributesCommandData.UID))[1], ((DWord_t *)&(Msg.MessageInformation.GetItemAttributesCommandData.UID))[0], Msg.MessageInformation.GetItemAttributesCommandData.UIDCounter);
               printf("Number Attribute ID: %02X\r\n", Msg.MessageInformation.GetItemAttributesCommandData.NumberAttributeIDs);

               for(Index=0;Index<(unsigned int)Msg.MessageInformation.GetItemAttributesCommandData.NumberAttributeIDs;Index++)
                  printf("%08lX ", Msg.MessageInformation.GetItemAttributesCommandData.AttributeIDList[Index]);
               printf("\r\n");
               break;
            case amtSearch:
               printf("Search\r\n");
               printf("Character Set: %04X, Length: %04X\r\n", Msg.MessageInformation.SearchCommandData.CharacterSet, Msg.MessageInformation.SearchCommandData.Length);

               if(Msg.MessageInformation.SearchCommandData.Length)
               {
                  printf("Search String: ");

                  for(Index1=0;Index1<(unsigned int)Msg.MessageInformation.SearchCommandData.Length;Index1++)
                     printf("%c", Msg.MessageInformation.SearchCommandData.SearchString[Index1]);

                  printf("\r\n");
               }
               break;
            case amtGetFolderItems:
               printf("Get Folder Items\r\n");
               printf("Scope: %02X, Start: %08lX, End: %08lX\r\n", Msg.MessageInformation.GetFolderItemsCommandData.Scope, Msg.MessageInformation.GetFolderItemsCommandData.StartItem, Msg.MessageInformation.GetFolderItemsCommandData.EndItem);
               printf("Number Attribute ID: %02X\r\n", Msg.MessageInformation.GetFolderItemsCommandData.NumberAttributeIDs);

               if(Msg.MessageInformation.GetFolderItemsCommandData.NumberAttributeIDs != AVRCP_GET_FOLDER_ITEMS_ATTRIBUTE_COUNT_NO_ATTRIBUTES)
               {
                  for(Index=0;Index<(unsigned int)Msg.MessageInformation.GetFolderItemsCommandData.NumberAttributeIDs;Index++)
                     printf("%08lX ", Msg.MessageInformation.GetFolderItemsCommandData.AttributeIDList[Index]);

                  printf("\r\n");
               }
               break;
            default:
               if(Msg.MessageType == amtUnknown)
                  printf("Unknown Message Type, PDU ID: 0x%02X, Length: 0x%04X\r\n", Msg.MessageInformation.UnknownCommandData.PDU_ID, Msg.MessageInformation.UnknownCommandData.ParameterLength);
               else
                  printf("Unhandled Message Type %d\r\n", Msg.MessageType);
               break;
         }

         /* For demonstration purposes we will respond only to the Set  */
         /* Browsed Player Command and reject all the other ones.       */
         if(Msg.MessageType == amtSetBrowsedPlayer)
            SendSetBrowsedPlayerResponse(RemoteBD_ADDR, ProfileID, TransactionID);
         else
            SendGeneralRejectResponse(RemoteBD_ADDR, ProfileID, TransactionID);
      }

      /* Free any memory that was alloced during the decoding operation.*/
      AVRCP_Free_Decoded_Message(BluetoothStackID, &Msg);
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

      printf("\r\nAVRCP>");
   }
   else
   {
      /* There was an error with one or more of the input parameters.   */
      printf("\r\n");

      printf("GAP Callback Data: Event_Data = NULL.\r\n");

      printf("\r\nAVRCP>");
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

   printf("\r\nAVRCP>");

   /* Make sure the output is displayed to the user.                    */
   fflush(stdout);
}

   /* The following function represents the AVCTP Profile Event         */
   /* Callback.  This function will be called whenever an AVCTP Profile */
   /* Event occurs that is associated with the specified Bluetooth Stack*/
   /* ID.  This function takes as its parameters the Bluetooth Stack ID,*/
   /* the AVCTP Event Data that occurred and the AVCTP Event Callback   */
   /* Parameter that was specified when this Callback was installed.    */
   /* The caller is free to use the contents of the AVCTP Event Data    */
   /* ONLY in the context of this callback.  If the caller requires the */
   /* Data for a longer period of time, then the callback function MUST */
   /* copy the data into another Data Buffer.  This function is         */
   /* guaranteed NOT to be invoked more than once simultaneously for the*/
   /* specified installed callback (i.e. this function DOES NOT have be */
   /* reentrant).  It Needs to be noted however, that if the same       */
   /* Callback is installed more than once, then the callbacks will be  */
   /* called serially.  Because of this, the processing in this function*/
   /* should be as efficient as possible.  It should also be noted that */
   /* this function is called in the Thread Context of a Thread that the*/
   /* User does NOT own.  Therefore, processing in this function should */
   /* be as efficient as possible (this argument holds anyway because   */
   /* another AVCTP Profile Event will not be processed while this      */
   /* function call is outstanding).                                    */
   /* * NOTE * This function MUST NOT Block and wait for events that can*/
   /*          only be satisfied by Receiving other AVCTP Events.  A    */
   /*          Deadlock WILL occur because NO AVCTP Event Callbacks will*/
   /*          be issued while this function is currently outstanding.  */
static void BTPSAPI AVCTP_Event_Callback(unsigned int BluetoothStackID, AVCTP_Event_Data_t *AVCTPEventData, unsigned long CallbackParameter)
{
   char          BoardStr[13];
   Byte_t       *DataBuffer;
   unsigned int  DataLength;
   DeviceList_t  DeviceListEntry;
   DeviceList_t *DeviceListEntryPtr;

   if((BluetoothStackID) && (AVCTPEventData))
   {
      printf("\r\n");

      switch(AVCTPEventData->Event_Data_Type)
      {
         case etAVCTP_Connect_Indication:
            ProfileInfo.Connected               = TRUE;

            DeviceListEntry.BD_ADDR             = AVCTPEventData->Event_Data.AVCTP_Connect_Indication_Data->BD_ADDR;
            DeviceListEntry.InitiatedConnection = 0;

            AddDeviceListEntry(&(ProfileInfo.DeviceList), &DeviceListEntry);

            BD_ADDRToStr(DeviceListEntry.BD_ADDR, BoardStr);

            printf("Connect Indication Received for %d, %s\r\n", ProfileInfo.ProfileID, BoardStr);
            break;
         case etAVCTP_Browsing_Channel_Connect_Indication:
            BD_ADDRToStr(AVCTPEventData->Event_Data.AVCTP_Browsing_Channel_Connect_Indication_Data->BD_ADDR, BoardStr);

            printf("Browsing Channel Connect Indication Received for %d, %s, MTU: %d\r\n", AVCTPEventData->Event_Data.AVCTP_Browsing_Channel_Connect_Indication_Data->AVCTPProfileID, BoardStr, AVCTPEventData->Event_Data.AVCTP_Browsing_Channel_Connect_Indication_Data->MTU);
            break;
         case etAVCTP_Connect_Confirmation:
            if(AVCTPEventData->Event_Data.AVCTP_Connect_Confirmation_Data->Status == 0)
            {
               ProfileInfo.Connected               = TRUE;

               DeviceListEntry.BD_ADDR             = AVCTPEventData->Event_Data.AVCTP_Connect_Confirmation_Data->BD_ADDR;
               DeviceListEntry.InitiatedConnection = 1;

               AddDeviceListEntry(&(ProfileInfo.DeviceList), &DeviceListEntry);
            }

            BD_ADDRToStr(AVCTPEventData->Event_Data.AVCTP_Connect_Confirmation_Data->BD_ADDR, BoardStr);

            printf("Connect Confirmation Received for %d, %s: Status: %d\r\n", ProfileInfo.ProfileID, BoardStr, AVCTPEventData->Event_Data.AVCTP_Connect_Confirmation_Data->Status);
            break;
         case etAVCTP_Browsing_Channel_Connect_Confirmation:
            BD_ADDRToStr(AVCTPEventData->Event_Data.AVCTP_Browsing_Channel_Connect_Confirmation_Data->BD_ADDR, BoardStr);

            printf("Browsing Channel Connect Confirmation Received for %d, %s: Status: %d, MTU: %d\r\n", AVCTPEventData->Event_Data.AVCTP_Browsing_Channel_Connect_Confirmation_Data->AVCTPProfileID, BoardStr, AVCTPEventData->Event_Data.AVCTP_Browsing_Channel_Connect_Confirmation_Data->Status, AVCTPEventData->Event_Data.AVCTP_Browsing_Channel_Connect_Confirmation_Data->MTU);
            break;
         case etAVCTP_Disconnect_Indication:
            /* Delete the entry from the list.                          */
            DeviceListEntryPtr = SearchDeviceListEntryByBDADDR(&(ProfileInfo.DeviceList), AVCTPEventData->Event_Data.AVCTP_Disconnect_Indication_Data->BD_ADDR);

            if(DeleteAVCTPConnectionInfoEntry(&(ProfileInfo.DeviceList),DeviceListEntryPtr) != NULL)
               FreeDeviceListEntryMemory(DeviceListEntryPtr);

            if(!ProfileInfo.DeviceList)
               ProfileInfo.Connected = FALSE;

            BD_ADDRToStr(AVCTPEventData->Event_Data.AVCTP_Disconnect_Indication_Data->BD_ADDR, BoardStr);

            printf("Disconnect Indication Received for %d, %s\r\n", ProfileInfo.ProfileID, BoardStr);
            break;
         case etAVCTP_Browsing_Channel_Disconnect_Indication:
            BD_ADDRToStr(AVCTPEventData->Event_Data.AVCTP_Browsing_Channel_Disconnect_Indication_Data->BD_ADDR, BoardStr);

            printf("Browsing Channel Disconnect Indication Received for %d, %s\r\n", AVCTPEventData->Event_Data.AVCTP_Browsing_Channel_Disconnect_Indication_Data->AVCTPProfileID, BoardStr);
            break;
         case etAVCTP_Message_Indication:
            BD_ADDRToStr(AVCTPEventData->Event_Data.AVCTP_Message_Indication_Data->BD_ADDR, BoardStr);
            printf("Message Received from %s  Profile ID: %d Message Type: %d  TransactionID: %d\r\n", BoardStr, AVCTPEventData->Event_Data.AVCTP_Message_Indication_Data->AVCTPProfileID, AVCTPEventData->Event_Data.AVCTP_Message_Indication_Data->MessageType, AVCTPEventData->Event_Data.AVCTP_Message_Indication_Data->TransactionID);

            /* Check to see if a Valid Request was made.                */
            if(!AVCTPEventData->Event_Data.AVCTP_Message_Indication_Data->InvalidProfileID)
            {
               DataBuffer = AVCTPEventData->Event_Data.AVCTP_Message_Indication_Data->DataBuffer;
               DataLength = AVCTPEventData->Event_Data.AVCTP_Message_Indication_Data->DataLength;

               HandleReceivedMessage(AVCTPEventData->Event_Data.AVCTP_Message_Indication_Data->BD_ADDR, AVCTPEventData->Event_Data.AVCTP_Message_Indication_Data->AVCTPProfileID, AVCTPEventData->Event_Data.AVCTP_Message_Indication_Data->TransactionID, AVCTPEventData->Event_Data.AVCTP_Message_Indication_Data->MessageType, DataLength, DataBuffer);
            }
            else
               printf("Request not supported: ProfileID: 0x%02X\r\n", AVCTPEventData->Event_Data.AVCTP_Message_Indication_Data->InvalidProfileID);
            break;
         case etAVCTP_Browsing_Channel_Message_Indication:
            BD_ADDRToStr(AVCTPEventData->Event_Data.AVCTP_Browsing_Channel_Message_Indication_Data->BD_ADDR, BoardStr);

            printf("Browsing Message Received from %d, %s. Response: %d.\r\n", AVCTPEventData->Event_Data.AVCTP_Browsing_Channel_Message_Indication_Data->AVCTPProfileID, BoardStr, AVCTPEventData->Event_Data.AVCTP_Browsing_Channel_Message_Indication_Data->MessageType);

            printf("TrID: %d IPID: %d Length: %d\r\n", AVCTPEventData->Event_Data.AVCTP_Browsing_Channel_Message_Indication_Data->TransactionID, AVCTPEventData->Event_Data.AVCTP_Browsing_Channel_Message_Indication_Data->InvalidProfileID, AVCTPEventData->Event_Data.AVCTP_Browsing_Channel_Message_Indication_Data->DataLength);

            DataBuffer = AVCTPEventData->Event_Data.AVCTP_Browsing_Channel_Message_Indication_Data->DataBuffer;
            DataLength = AVCTPEventData->Event_Data.AVCTP_Browsing_Channel_Message_Indication_Data->DataLength;

            HandleReceivedBrowsingMessage(AVCTPEventData->Event_Data.AVCTP_Browsing_Channel_Message_Indication_Data->BD_ADDR, AVCTPEventData->Event_Data.AVCTP_Browsing_Channel_Message_Indication_Data->AVCTPProfileID, AVCTPEventData->Event_Data.AVCTP_Browsing_Channel_Message_Indication_Data->TransactionID, AVCTPEventData->Event_Data.AVCTP_Browsing_Channel_Message_Indication_Data->MessageType, DataLength, DataBuffer);
            break;
         default:
            printf("Profile Unknown AVCTP Event.\r\n");
            break;
      }

      printf("AVRCP>");

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
               BaudRate       = strtol(argv[3], &endptr, 10);

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

      /* Check to see if the HCI_Driver Information Strucuture was      */
      /* successfully setup.                                            */
      if(HCI_DriverInformationPtr)
      {
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
                     UserInterface();
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

