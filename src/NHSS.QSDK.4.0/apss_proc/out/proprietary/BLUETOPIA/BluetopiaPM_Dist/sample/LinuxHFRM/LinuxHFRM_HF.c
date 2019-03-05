/*****< linuxhfrm_hf.c >*******************************************************/
/*      Copyright 2011 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  LINUXHFRM_HF - Simple Linux application using Bluetopia Platform Manager  */
/*                 Hands Free Profile (HFRE) Manager Application Programming  */
/*                 Interface (API) - Hands Free Role only.                    */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/20/11  T. Cook        Initial creation. (Based on LinuxHIDM)          */
/******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include <unistd.h>        /* Include for getpid().                           */

#include "LinuxHFRM_HF.h"  /* Main Application Prototypes and Constants.      */

#include "SS1BTHFRM.h"     /* HFRE Manager Application Programming Interface. */
#include "SS1BTPM.h"       /* BTPM Application Programming Interface.         */

#define HANDSFREE_PROFILE_HANDS_FREE_UUID             0x111E
#define HANDSFREE_PROFILE_AUDIO_GATEWAY_UUID          0x111F
#define RFCOMM_PROTOCOL_UUID                          0x0003
#define SDP_ATTRIBUTE_ID_PRIMARY_LANGUAGE_BASE_VALUE  0x100

#define MAX_SUPPORTED_COMMANDS                     (128) /* Denotes the       */
                                                         /* maximum number of */
                                                         /* User Commands that*/
                                                         /* are supported by  */
                                                         /* this application. */

#define MAX_COMMAND_LENGTH                        (256)  /* Denotes the max   */
                                                         /* buffer size used  */
                                                         /* for user commands */
                                                         /* input via the     */
                                                         /* User Interface.   */

#define MAX_NUM_OF_PARAMETERS                       (5)  /* Denotes the max   */
                                                         /* number of         */
                                                         /* parameters a      */
                                                         /* command can have. */

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

#define INDENT_LENGTH                                 3  /* Denotes the number*/
                                                         /* of character      */
                                                         /* spaces to be used */
                                                         /* for indenting when*/
                                                         /* displaying SDP    */
                                                         /* Data Elements.    */


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

#define PLATFORM_MANAGER_NOT_INITIALIZED_ERROR     (-7)  /* Denotes that an   */
                                                         /* error occurred due*/
                                                         /* to the fact that  */
                                                         /* the Platform      */
                                                         /* Manager has not   */
                                                         /* been initialized. */

   /* The following enumeration is used to specify the current Audio    */
   /* Test Mode.                                                        */
typedef enum
{
   atmNone,
   atmLoopback,
   atmTestTone
} AudioTestMode_t;

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

#define MAX_SDP_RECORD_ARRAY_LIST      20

   /* Holds information on a sequence.                                  */
typedef struct _tagSeq_Info_t
{
   Word_t UUID_ID;
   Word_t Value;
} Seq_Info_t;

   /* The following structure is used to hold information located in a  */
   /* single SDP Service Record.  The structure is tailored to hold     */
   /* information about the profiles that are supported with this       */
   /* version of DISC.                                                  */
typedef struct _tagSDP_Record_t
{
   Byte_t     NumServiceClass;
   Word_t     ServiceClass[MAX_SDP_RECORD_ARRAY_LIST];
   Byte_t     NumProtocols;
   Seq_Info_t Protocol[MAX_SDP_RECORD_ARRAY_LIST];
   Byte_t     NumAdditionalProtocols;
   Seq_Info_t AdditionalProtocol[MAX_SDP_RECORD_ARRAY_LIST];
   Byte_t     NumProfiles;
   Seq_Info_t Profile[MAX_SDP_RECORD_ARRAY_LIST];
   Word_t     SupportedFeatures;
   Word_t     ServiceNameLength;
   Byte_t    *ServiceName;
   Word_t     ServiceDescLength;
   Byte_t    *ServiceDesc;
   Word_t     ProviderNameLength;
   Byte_t    *ProviderName;
} SDP_Record_t;

   /* The following defines the SDP Information that pertains to the    */
   /* Hands Free Profile.                                               */
typedef struct _tagHFRE_Info_t
{
   Byte_t ServerChannel;
   Word_t ProfileVersion;
   Word_t SupportedFeatures;
} HFRE_Info_t;

   /* The following defines the structure that contains information     */
   /* about a specific profile supported on the remote device.  The     */
   /* Profile Identifier value identifies the Profile Information       */
   /* structure that is used to access the information about the        */
   /* profile.                                                          */
typedef struct _tagProfile_Info_t
{
   Word_t       ServiceNameLength;
   Byte_t      *ServiceName;
   Word_t       ServiceDescLength;
   Byte_t      *ServiceDesc;
   Word_t       ServiceProviderLength;
   Byte_t      *ServiceProvider;
   HFRE_Info_t  HFREInfo;
} HFRE_Profile_Info_t;

#define PROFILE_INFO_DATA_SIZE                  (sizeof(HFRE_Profile_Info_t))


   /* The following type definition represents the structure which holds*/
   /* information for the mapping of Supported Features bitmasks to     */
   /* their string representation.                                      */
typedef struct _tagBitmaskMap_t
{
   unsigned int  Mask;
   char         *String;
} BitmaskMap_t;

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the         */
   /* compiler as part of standard C/C++).                              */
static Boolean_t           Initialized;             /* Variable which is used to hold  */
                                                    /* the current state of the        */
                                                    /* Bluetopia Platform Manager      */
                                                    /* Initialization.                 */

static unsigned int        DEVMCallbackID;          /* Variable which holds the        */
                                                    /* Callback ID of the currently    */
                                                    /* registered Device Manager       */
                                                    /* Callback ID.                    */

static unsigned int        AuthenticationCallbackID;/* Variable which holds the        */
                                                    /* current Authentication Callback */
                                                    /* ID that is assigned from the    */
                                                    /* Device Manager when the local   */
                                                    /* client registers for            */
                                                    /* Authentication.                 */

static unsigned int        HFREventCallbackID;      /* Variable which holds the        */
                                                    /* current HFR Event Callback ID   */
                                                    /* that is assigned from the HFR   */
                                                    /* Manager when the local client   */
                                                    /* registers for HFR Events.       */

static unsigned int        HFRDataCallbackID;       /* Variable which holds the        */
                                                    /* current HFR Data Callback ID    */
                                                    /* that is assigned from the HFR   */
                                                    /* Manager when the local client   */
                                                    /* registers for HFR Data Events.  */

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

static unsigned int        AudioDataCount;          /* Variable which is used to       */
                                                    /* control the rate at which Audio */
                                                    /* Data Event information is       */
                                                    /* printed to the console.         */

static Boolean_t           AudioData8BitFormat;     /* Variable which flags whether or */
                                                    /* not the SCO Data Format is 8 bit*/
                                                    /* PCM (TRUE) or 16 bit PCM        */
                                                    /* (FALSE).                        */
                                                    /* * NOTE * This variable defaults */
                                                    /*          to 0 (FALSE) to        */
                                                    /*          indicate the data      */
                                                    /*          format is 16 bits,     */
                                                    /*          which is the typical   */
                                                    /*          data format unless the */
                                                    /*          Bluetooth device is    */
                                                    /*          re-configured to use 8 */
                                                    /*          bits.                  */

static AudioTestMode_t     AudioTestMode;           /* Variable which holds the        */
                                                    /* current SCO Audio Data Test     */
                                                    /* Mode.                           */

static unsigned int        AudioDataToneIndex;      /* Variable which holds the        */
                                                    /* current memory pointer index    */
                                                    /* into the SCO Tone data array    */
                                                    /* (either 8 or 16 bit) of the     */
                                                    /* current SCO data (used with     */
                                                    /* SCO Test Mode 1KHz Sine Wave).  */

   /* The following string table is used to map the API I/O Capabilities*/
   /* values to an easily displayable string.                           */
static char *IOCapabilitiesStrings[] =
{
   "Display Only",
   "Display Yes/No",
   "Keyboard Only",
   "No Input/Output"
};

   /* The following typedef represents a buffer large enough to hold a  */
   /* connection type string.                                           */
typedef char ConnectionTypeStr_t[16];

static BitmaskMap_t AudioGatewayFeatures[] = {
   { HFRE_THREE_WAY_CALLING_SUPPORTED_BIT,              "THREE_WAY_CALLING"           },
   { HFRE_AG_SOUND_ENHANCEMENT_SUPPORTED_BIT,           "SOUND_ENHANCEMENT"           },
   { HFRE_AG_VOICE_RECOGNITION_SUPPORTED_BIT,           "VOICE_RECOGNITION"           },
   { HFRE_INBAND_RINGING_SUPPORTED_BIT,                 "INBAND_RINGING"              },
   { HFRE_VOICE_TAGS_SUPPORTED_BIT,                     "VOICE_TAGS"                  },
   { HFRE_REJECT_CALL_SUPPORT_BIT,                      "REJECT_CALL"                 },
   { HFRE_AG_ENHANCED_CALL_STATUS_SUPPORTED_BIT,        "ENHANCED_CALL_STATUS"        },
   { HFRE_AG_ENHANCED_CALL_CONTROL_SUPPORTED_BIT,       "ENHANCED_CALL_CONTROL"       },
   { HFRE_AG_EXTENDED_ERROR_RESULT_CODES_SUPPORTED_BIT, "EXTENDED_ERROR_RESULT_CODES" }
} ;

#define NUM_AUDIO_GATEWAY_FEATURES              (sizeof(AudioGatewayFeatures)/sizeof(BitmaskMap_t))

static BitmaskMap_t MultipartyFeatures[] = {
   { HFRE_RELEASE_ALL_HELD_CALLS,                          "RELEASE_ALL_HELD_CALLS"                          },
   { HFRE_RELEASE_ALL_ACTIVE_CALLS_ACCEPT_WAITING_CALL,    "RELEASE_ALL_ACTIVE_CALLS_ACCEPT_WAITING_CALL"    },
   { HFRE_PLACE_ALL_ACTIVE_CALLS_ON_HOLD_ACCEPT_THE_OTHER, "PLACE_ALL_ACTIVE_CALLS_ON_HOLD_ACCEPT_THE_OTHER" },
   { HFRE_ADD_A_HELD_CALL_TO_CONVERSATION,                 "ADD_A_HELD_CALL_TO_CONVERSATION"                 },
   { HFRE_CONNECT_TWO_CALLS_DISCONNECT_SUBSCRIBER,         "CONNECT_TWO_CALLS_DISCONNECT_SUBSCRIBER"         },
   { HFRE_RELEASE_SPECIFIED_ACTIVE_CALL_ONLY,              "RELEASE_SPECIFIED_ACTIVE_CALL_ONLY"              },
   { HFRE_REQUEST_PRIVATE_CONSULTATION_MODE,               "REQUEST_PRIVATE_CONSULTATION_MODE"               }
} ;

#define NUM_MULTIPARTY_FEATURES                 (sizeof(MultipartyFeatures)/sizeof(BitmaskMap_t))

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
static Byte_t AudioDataTone_1KHz_16Bit[] =
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

static void ConnectionTypeToStr(HFRM_Connection_Type_t ConnectionType, ConnectionTypeStr_t ConnectionTypeStr);

static void StrToBD_ADDR(char *BoardStr, BD_ADDR_t *Board_Address);

static int DisplayHelp(ParameterList_t *TempParam);

static int Initialize(ParameterList_t *TempParam);
static int Cleanup(ParameterList_t *TempParam);
static int RegisterEventCallback(ParameterList_t *TempParam);
static int UnRegisterEventCallback(ParameterList_t *TempParam);
static int SetDevicePower(ParameterList_t *TempParam);
static int QueryDevicePower(ParameterList_t *TempParam);
static int SetLocalRemoteDebugZoneMask(ParameterList_t *TempParam);
static int QueryLocalRemoteDebugZoneMask(ParameterList_t *TempParam);
static int ShutdownService(ParameterList_t *TempParam);
static int QueryLocalDeviceProperties(ParameterList_t *TempParam);
static int SetLocalDeviceName(ParameterList_t *TempParam);
static int SetLocalClassOfDevice(ParameterList_t *TempParam);
static int SetDiscoverable(ParameterList_t *TempParam);
static int SetConnectable(ParameterList_t *TempParam);
static int SetPairable(ParameterList_t *TempParam);
static int StartDeviceDiscovery(ParameterList_t *TempParam);
static int StopDeviceDiscovery(ParameterList_t *TempParam);
static int QueryRemoteDeviceList(ParameterList_t *TempParam);
static int QueryRemoteDeviceProperties(ParameterList_t *TempParam);
static int AddRemoteDevice(ParameterList_t *TempParam);
static int DeleteRemoteDevice(ParameterList_t *TempParam);
static int DeleteRemoteDevices(ParameterList_t *TempParam);
static int PairWithRemoteDevice(ParameterList_t *TempParam);
static int CancelPairWithRemoteDevice(ParameterList_t *TempParam);
static int UnPairRemoteDevice(ParameterList_t *TempParam);
static int QueryRemoteDeviceServices(ParameterList_t *TempParam);
static int RegisterAuthentication(ParameterList_t *TempParam);
static int UnRegisterAuthentication(ParameterList_t *TempParam);

static int ChangeSimplePairingParameters(ParameterList_t *TempParam);
static int PINCodeResponse(ParameterList_t *TempParam);
static int PassKeyResponse(ParameterList_t *TempParam);
static int UserConfirmationResponse(ParameterList_t *TempParam);

   /* Audio Gateway/Hands Free Common Functions.                        */
static int ManageAudioConnection(ParameterList_t *TempParam);
static int HFRMSetupAudioConnection(BD_ADDR_t BD_ADDR);
static int HFRMReleaseAudioConnection(BD_ADDR_t BD_ADDR);
static int HFRMConnectDevice(ParameterList_t *TempParam);
static int HFRMDisconnectDevice(ParameterList_t *TempParam);
static int HFRMConnectionRequestResponse(ParameterList_t *TempParam);
static int HFRMDisableRemoteEchoCancelationNoiseReduction(ParameterList_t *TempParam);
static int HFRMSetRemoteSpeakerGain(ParameterList_t *TempParam);
static int HFRMSetRemoteMicrophoneGain(ParameterList_t *TempParam);
static int QueryHandsFreeServices(ParameterList_t *TempParam);
static int ChangeIncomingConnectionFlags(ParameterList_t *TempParam);

   /* Hands-Free Functions.                                             */
static int HFRMEnableRemoteIndicatorEventNotification(ParameterList_t *TempParam);
static int HFRMEnableRemoteCallWaitingNotification(ParameterList_t *TempParam);
static int HFRMEnableRemoteCallLineIdentificationNotification(ParameterList_t *TempParam);

static int HFRMQueryRemoteIndicatorsStatus(ParameterList_t *TempParam);
static int HFRMQueryRemoteCallHoldingMultipartyServiceSupport(ParameterList_t *TempParam);
static int HFRMSendCallHoldingMultipartySelection(ParameterList_t *TempParam);
static int HFRMDialPhoneNumber(ParameterList_t *TempParam);
static int HFRMDialPhoneNumberFromMemory(ParameterList_t *TempParam);
static int HFRMRedialLastPhoneNumber(ParameterList_t *TempParam);
static int HFRMAnswerIncomingCall(ParameterList_t *TempParam);
static int HFRMTransmitDTMFCode(ParameterList_t *TempParam);
static int HFRMSetRemoteVoiceRecognitionActivation(ParameterList_t *TempParam);
static int HFRMVoiceTagRequest(ParameterList_t *TempParam);
static int HFRMHangUpCall(ParameterList_t *TempParam);

static int HFRMQueryOperator(ParameterList_t *TempParam);
static int HFRMSetOperatorFormat(ParameterList_t *TempParam);
static int HFRMEnableExtendedErrorReporting(ParameterList_t *TempParam);
static int HFRMQuerySubscriberNumber(ParameterList_t *TempParam);
static int HFRMSendResponseAndHold(ParameterList_t *TempParam);
static int HFRMQueryResponseAndHold(ParameterList_t *TempParam);
static int HFRMQueryRemoteCallList(ParameterList_t *TempParam);
static int HFRMSendArbitraryCommand(ParameterList_t *TempParam);
static int HFRMQuerySCOConnectionHandle(ParameterList_t *TempParam);

static int HFRRegisterEventCallback(ParameterList_t *TempParam);
static int HFRUnRegisterEventCallback(ParameterList_t *TempParam);
static int HFRRegisterDataCallback(ParameterList_t *TempParam);
static int HFRUnRegisterDataCallback(ParameterList_t *TempParam);

static int SetSCOTestMode(ParameterList_t *TempParam);

static Word_t ExtractUUID_ID(SDP_Data_Element_t *DataElementPtr);
static Boolean_t ParseAudioGatewayServiceClass(SDP_Record_t *SDPRecordPtr);
static Boolean_t ProcessHandsFreeServiceRecord(SDP_Record_t *SDPRecordPtr, HFRE_Profile_Info_t *ProfileInfoPtr);
static Boolean_t ParseHandsFreeSDPAttributeResponse(SDP_Service_Attribute_Response_Data_t *SDPServiceAttributeResponse, HFRE_Profile_Info_t *ProfileInfo);

static int GetHandsFreeServiceInformation(BD_ADDR_t BD_ADDR, Boolean_t ForceUpdate, unsigned int *PortNumber);
static Boolean_t ParseHandsFreeData(DEVM_Parsed_SDP_Data_t *ParsedSDPData, HFRE_Profile_Info_t *ProfileInfo);
static void DisplayParsedServiceData(DEVM_Parsed_SDP_Data_t *ParsedSDPData);
static void DisplaySDPAttributeResponse(SDP_Service_Attribute_Response_Data_t *SDPServiceAttributeResponse, unsigned int InitLevel);
static void DisplayDataElement(SDP_Data_Element_t *SDPDataElement, unsigned int Level);

static void DisplayLocalDeviceProperties(unsigned long UpdateMask, DEVM_Local_Device_Properties_t *LocalDeviceProperties);
static void DisplayRemoteDeviceProperties(unsigned long UpdateMask, DEVM_Remote_Device_Properties_t *RemoteDeviceProperties);

   /* BTPM Server Un-Registration Callback function prototype.          */
void BTPSAPI ServerUnRegistrationCallback(void *CallbackParameter);

   /* BTPM Local Device Manager Callback function prototype.            */
static void BTPSAPI DEVM_Event_Callback(DEVM_Event_Data_t *EventData, void *CallbackParameter);

   /* BTPM Local Device Manager Authentication Callback function        */
   /* prototype.                                                        */
static void BTPSAPI DEVM_Authentication_Callback(DEVM_Authentication_Information_t *AuthenticationRequestInformation, void *CallbackParameter);

   /* BTPM HFRE Manager Callback function prototype.                    */
static void BTPSAPI HFRM_Event_Callback(HFRM_Event_Data_t *EventData, void *CallbackParameter);

   /* This function is responsible for taking the input from the user   */
   /* and dispatching the appropriate Command Function.  First, this    */
   /* function retrieves a String of user input, parses the user input  */
   /* into Command and Parameters, and finally executes the Command or  */
   /* Displays an Error Message if the input is not a valid Command.    */
static void UserInterface(void)
{
   UserCommand_t TempCommand;
   int  Result = !EXIT_CODE;
   char UserInput[MAX_COMMAND_LENGTH];

   /* First let's make sure that we start on new line.                  */
   printf("\r\n");

   /* Next display the available commands.                              */
   DisplayHelp(NULL);

   ClearCommands();

   AddCommand("INITIALIZE", Initialize);
   AddCommand("CLEANUP", Cleanup);
   AddCommand("QUERYDEBUGZONEMASK", QueryLocalRemoteDebugZoneMask);
   AddCommand("SETDEBUGZONEMASK", SetLocalRemoteDebugZoneMask);
   AddCommand("SHUTDOWNSERVICE", ShutdownService);
   AddCommand("REGISTEREVENTCALLBACK", RegisterEventCallback);
   AddCommand("UNREGISTEREVENTCALLBACK", UnRegisterEventCallback);
   AddCommand("QUERYDEVICEPOWER", QueryDevicePower);
   AddCommand("SETDEVICEPOWER", SetDevicePower);
   AddCommand("QUERYLOCALDEVICEPROPERTIES", QueryLocalDeviceProperties);
   AddCommand("SETLOCALDEVICENAME", SetLocalDeviceName);
   AddCommand("SETLOCALCLASSOFDEVICE", SetLocalClassOfDevice);
   AddCommand("SETDISCOVERABLE", SetDiscoverable);
   AddCommand("SETCONNECTABLE", SetConnectable);
   AddCommand("SETPAIRABLE", SetPairable);
   AddCommand("STARTDEVICEDISCOVERY", StartDeviceDiscovery);
   AddCommand("STOPDEVICEDISCOVERY", StopDeviceDiscovery);
   AddCommand("QUERYREMOTEDEVICELIST", QueryRemoteDeviceList);
   AddCommand("QUERYREMOTEDEVICEPROPERTIES", QueryRemoteDeviceProperties);
   AddCommand("ADDREMOTEDEVICE", AddRemoteDevice);
   AddCommand("DELETEREMOTEDEVICE", DeleteRemoteDevice);
   AddCommand("DELETEREMOTEDEVICES", DeleteRemoteDevices);
   AddCommand("PAIRWITHREMOTEDEVICE", PairWithRemoteDevice);
   AddCommand("CANCELPAIRWITHREMOTEDEVICE", CancelPairWithRemoteDevice);
   AddCommand("UNPAIRREMOTEDEVICE", UnPairRemoteDevice);
   AddCommand("QUERYREMOTEDEVICESERVICES", QueryRemoteDeviceServices);
   AddCommand("REGISTERAUTHENTICATION", RegisterAuthentication);
   AddCommand("UNREGISTERAUTHENTICATION", UnRegisterAuthentication);
   AddCommand("PINCODERESPONSE", PINCodeResponse);
   AddCommand("PASSKEYRESPONSE", PassKeyResponse);
   AddCommand("USERCONFIRMATIONRESPONSE", UserConfirmationResponse);
   AddCommand("CHANGESIMPLEPAIRINGPARAMETERS", ChangeSimplePairingParameters);

   /* HFRE & AG Common Commands.                                        */
   AddCommand("CONNECTDEVICE", HFRMConnectDevice);
   AddCommand("DISCONNECTDEVICE", HFRMDisconnectDevice);
   AddCommand("CONNECTIONREQUESTRESPONSE", HFRMConnectionRequestResponse);
   AddCommand("MANAGEAUDIO", ManageAudioConnection);
   AddCommand("DISABLEREMOTESOUNDENHANCEMENT", HFRMDisableRemoteEchoCancelationNoiseReduction);
   AddCommand("SETSPEAKERGAIN", HFRMSetRemoteSpeakerGain);
   AddCommand("SETMICROPHONEGAIN", HFRMSetRemoteMicrophoneGain);
   AddCommand("QUERYHANDSFREESERVICES", QueryHandsFreeServices);
   AddCommand("CHANGEINCOMINGCONNECTIONFLAGS", ChangeIncomingConnectionFlags);

   /* HFRM Commands.                                                    */
   AddCommand("ENABLEREMOTEINDICATORNOTIFICATION", HFRMEnableRemoteIndicatorEventNotification);
   AddCommand("ENABLEREMOTECALLWAITINGNOTIFICATION", HFRMEnableRemoteCallWaitingNotification);
   AddCommand("ENABLEREMOTECALLERIDNOTIFICATION", HFRMEnableRemoteCallLineIdentificationNotification);
   AddCommand("QUERYREMOTEINDICATORSTATUS", HFRMQueryRemoteIndicatorsStatus);
   AddCommand("QUERYREMOTECALLHOLDSUPPORT", HFRMQueryRemoteCallHoldingMultipartyServiceSupport);
   AddCommand("SENDCALLHOLDSELECTION", HFRMSendCallHoldingMultipartySelection);
   AddCommand("DIALNUMBER", HFRMDialPhoneNumber);
   AddCommand("MEMORYDIAL", HFRMDialPhoneNumberFromMemory);
   AddCommand("REDIALLASTNUMBER", HFRMRedialLastPhoneNumber);
   AddCommand("ANSWERCALL", HFRMAnswerIncomingCall);
   AddCommand("HANGUPCALL", HFRMHangUpCall);
   AddCommand("SENDDTMF", HFRMTransmitDTMFCode);
   AddCommand("SETVOICERECOGNITIONACTIVATION", HFRMSetRemoteVoiceRecognitionActivation);
   AddCommand("SENDVOICETAGREQUEST", HFRMVoiceTagRequest);
   AddCommand("QUERYOPERATOR", HFRMQueryOperator);
   AddCommand("SETOPERATORFORMAT", HFRMSetOperatorFormat);
   AddCommand("ENABLEERRORREPORTS", HFRMEnableExtendedErrorReporting);
   AddCommand("QUERYPHONENUMBER", HFRMQuerySubscriberNumber);
   AddCommand("QUERYRESPHOLD", HFRMQueryResponseAndHold);
   AddCommand("SENDRESPHOLD", HFRMSendResponseAndHold);
   AddCommand("QUERYCALLLIST", HFRMQueryRemoteCallList);
   AddCommand("SENDARBITRARYCOMMAND", HFRMSendArbitraryCommand);
   AddCommand("QUERYSCOCONNECTIONHANDLE", HFRMQuerySCOConnectionHandle);
   AddCommand("HFRREGISTEREVENTCALLBACK", HFRRegisterEventCallback);
   AddCommand("HFRUNREGISTEREVENTCALLBACK", HFRUnRegisterEventCallback);
   AddCommand("HFRREGISTERDATACALLBACK", HFRRegisterDataCallback);
   AddCommand("HFRUNREGISTERDATACALLBACK", HFRUnRegisterDataCallback);

   AddCommand("SETSCOTESTMODE", SetSCOTestMode);

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
      printf("Hands Free>");

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
      for(Index=0,ret_val=String; Index < strlen(String); Index++)
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

      /* Check to see if there is a Command                             */
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
      for(i=0; i<strlen(TempCommand->Command); i++)
         TempCommand->Command[i] = toupper(TempCommand->Command[i]);

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
      /* Special shortcut: If it's simply a one or two digit number,    */
      /* convert the command directly based on the Command Index.       */
      if((strlen(Command) == 1) && (Command[0] >= '1') && (Command[0] <= '9'))
      {
         Index = atoi(Command);

         if(Index < NumberCommands)
            ret_val = CommandTable[Index - 1].CommandFunction;
         else
            ret_val = NULL;
      }
      else
      {
         if((strlen(Command) == 2) && (Command[0] >= '0') && (Command[0] <= '9') && (Command[1] >= '0') && (Command[1] <= '9'))
         {
            Index = atoi(Command);

            if(Index < NumberCommands)
               ret_val = CommandTable[Index?(Index-1):Index].CommandFunction;
            else
               ret_val = NULL;
         }
         else
         {
            /* Now loop through each element in the table to see if     */
            /* there is a match.                                        */
            for(Index=0,ret_val=NULL; ((Index<NumberCommands) && (!ret_val)); Index++)
            {
               if((strlen(Command) == strlen(CommandTable[Index].CommandName)) && (memcmp(Command, CommandTable[Index].CommandName, strlen(CommandTable[Index].CommandName)) == 0))
                  ret_val = CommandTable[Index].CommandFunction;
            }
         }
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

   /* The following function is responsible for converting a HFRM       */
   /* Connection Type to a string.  The first parameter to this function*/
   /* is the ConnectionType to be converted to a string.  The second    */
   /* parameter of this function is a pointer to a string in which the  */
   /* converted Connection Type is to be stored.                        */
static void ConnectionTypeToStr(HFRM_Connection_Type_t ConnectionType, ConnectionTypeStr_t ConnectionTypeStr)
{
   if(ConnectionType == hctHandsFree)
      sprintf(ConnectionTypeStr, "hctHandsFree");
   else
   {
      if(ConnectionType == hctAudioGateway)
         sprintf(ConnectionTypeStr, "hctAudioGateway");
      else
         sprintf(ConnectionTypeStr, "Unknown");
   }
}

   /* The following function is responsible for the specified string    */
   /* into data of type BD_ADDR.  The first parameter of this function  */
   /* is the BD_ADDR string to be converted to a BD_ADDR.  The second   */
   /* parameter of this function is a pointer to the BD_ADDR in which   */
   /* the converted BD_ADDR String is to be stored.                     */
static void StrToBD_ADDR(char *BoardStr, BD_ADDR_t *Board_Address)
{
   unsigned int Address[sizeof(BD_ADDR_t)];

   if((BoardStr) && (strlen(BoardStr) == sizeof(BD_ADDR_t)*2) && (Board_Address))
   {
      sscanf(BoardStr, "%02X%02X%02X%02X%02X%02X", &(Address[5]), &(Address[4]), &(Address[3]), &(Address[2]), &(Address[1]), &(Address[0]));

      Board_Address->BD_ADDR5 = (Byte_t)Address[5];
      Board_Address->BD_ADDR4 = (Byte_t)Address[4];
      Board_Address->BD_ADDR3 = (Byte_t)Address[3];
      Board_Address->BD_ADDR2 = (Byte_t)Address[2];
      Board_Address->BD_ADDR1 = (Byte_t)Address[1];
      Board_Address->BD_ADDR0 = (Byte_t)Address[0];
   }
   else
   {
      if(Board_Address)
         BTPS_MemInitialize(Board_Address, 0, sizeof(BD_ADDR_t));
   }
}


   /* The following function is responsible for displaying the current  */
   /* Command Options for this Device Manager Sample Application.  The  */
   /* input parameter to this function is completely ignored, and only  */
   /* needs to be passed in because all Commands that can be entered at */
   /* the Prompt pass in the parsed information.  This function displays*/
   /* the current Command Options that are available and always returns */
   /* zero.                                                             */
static int DisplayHelp(ParameterList_t *TempParam)
{
   /* Note the order they are listed here *MUST* match the order in     */
   /* which then are added to the Command Table.                        */
   printf("******************************************************************\r\n");

   /* Common command options.                                           */
   printf("* Command Options: 1) Initialize                                 *\r\n");
   printf("*                  2) Cleanup                                    *\r\n");
   printf("*                  3) QueryDebugZoneMask                         *\r\n");
   printf("*                  4) SetDebugZoneMask                           *\r\n");
   printf("*                  5) ShutdownService                            *\r\n");
   printf("*                  6) RegisterEventCallback,                     *\r\n");
   printf("*                  7) UnRegisterEventCallback,                   *\r\n");
   printf("*                  8) QueryDevicePower                           *\r\n");
   printf("*                  9) SetDevicePower                             *\r\n");
   printf("*                  10)QueryLocalDeviceProperties                 *\r\n");
   printf("*                  11)SetLocalDeviceName                         *\r\n");
   printf("*                  12)SetLocalClassOfDevice                      *\r\n");
   printf("*                  13)SetDiscoverable                            *\r\n");
   printf("*                  14)SetConnectable                             *\r\n");
   printf("*                  15)SetPairable                                *\r\n");
   printf("*                  16)StartDeviceDiscovery                       *\r\n");
   printf("*                  17)StopDeviceDiscovery                        *\r\n");
   printf("*                  18)QueryRemoteDeviceList                      *\r\n");
   printf("*                  19)QueryRemoteDeviceProperties                *\r\n");
   printf("*                  20)AddRemoteDevice                            *\r\n");
   printf("*                  21)DeleteRemoteDevice                         *\r\n");
   printf("*                  22)DeleteRemoteDevices                        *\r\n");
   printf("*                  23)PairWithRemoteDevice                       *\r\n");
   printf("*                  24)CancelPairWithRemoteDevice                 *\r\n");
   printf("*                  25)UnPairRemoteDevice                         *\r\n");
   printf("*                  26)QueryRemoteDeviceServices                  *\r\n");
   printf("*                  27)RegisterAuthentication                     *\r\n");
   printf("*                  28)UnRegisterAuthentication                   *\r\n");
   printf("*                  29)PINCodeResponse                            *\r\n");
   printf("*                  30)PassKeyResponse                            *\r\n");
   printf("*                  31)UserConfirmationResponse                   *\r\n");
   printf("*                  32)ChangeSimplePairingParameters              *\r\n");

   /* General Hands-Free commands.                                      */
   printf("*                  33)ConnectDevice                              *\r\n");
   printf("*                  34)DisconnectDevice                           *\r\n");
   printf("*                  35)ConnectionRequestResponse                  *\r\n");
   printf("*                  36)ManageAudio                                *\r\n");
   printf("*                  37)DisableRemoteSoundEnhancement              *\r\n");
   printf("*                  38)SetSpeakerGain                             *\r\n");
   printf("*                  39)SetMicrophoneGain                          *\r\n");
   printf("*                  40)QueryHandsFreeServices                     *\r\n");
   printf("*                  41)ChangeIncomingConnectionFlags              *\r\n");

   /* Hands-Free remote notifications.                                  */
   printf("*                  42)EnableRemoteIndicatorNotification          *\r\n");
   printf("*                  43)EnableRemoteCallWaitingNotification        *\r\n");
   printf("*                  44)EnableRemoteCallerIDNotification           *\r\n");
   printf("*                  45)QueryRemoteIndicatorStatus                 *\r\n");

   /* Hands-Free phone call commands.                                   */
   printf("*                  46)QueryRemoteCallHoldSupport                 *\r\n");
   printf("*                  47)SendCallHoldSelection                      *\r\n");
   printf("*                  48)DialNumber                                 *\r\n");
   printf("*                  49)MemoryDial                                 *\r\n");
   printf("*                  50)ReDialLastNumber                           *\r\n");
   printf("*                  51)AnswerCall                                 *\r\n");
   printf("*                  52)HangupCall                                 *\r\n");
   printf("*                  53)SendDTMF                                   *\r\n");

   /* Miscellaneous Hands-Free commands.                                */
   printf("*                  54)SetVoiceRecognitionActivation              *\r\n");
   printf("*                  55)SendVoiceTagRequest                        *\r\n");
   printf("*                  56)QueryOperator                              *\r\n");
   printf("*                  57)SetOperatorFormat                          *\r\n");
   printf("*                  58)EnableErrorReports                         *\r\n");
   printf("*                  59)QueryPhoneNumber                           *\r\n");
   printf("*                  60)QueryRespHold                              *\r\n");
   printf("*                  61)SendRespHold                               *\r\n");
   printf("*                  62)QueryCallList                              *\r\n");
   printf("*                  63)SendArbitraryCommand                       *\r\n");
   printf("*                  64)QuerySCOConnectionHandle                   *\r\n");

   /* Hands-Free callback registration commands.                        */
   printf("*                  65)HFRRegisterEventCallback                   *\r\n");
   printf("*                  66)HFRUnRegisterEventCallback                 *\r\n");
   printf("*                  67)HFRRegisterDataCallback                    *\r\n");
   printf("*                  68)HFRUnRegisterDataCallback                  *\r\n");

   /* Application configuration commands.                               */
   printf("*                  69)SetSCOTestMode                             *\r\n");

   printf("*                  Help, Quit.                                   *\r\n");
   printf("******************************************************************\r\n");

   return(0);
}

   /* The following function is responsible for Initializing the        */
   /* Bluetopia Platform Manager Framework.  This function returns      */
   /* zero if successful and a negative value if an error occurred.     */
static int Initialize(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First, check to make sure that we are not already initialized.    */
   if(!Initialized)
   {
      /* Determine if the user would like to Register an Event Callback */
      /* with the calling of this command.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 1))
      {
         /* Now actually initialize the Platform Manager Service.       */
         Result = BTPM_Initialize((unsigned long)getpid(), NULL, ServerUnRegistrationCallback, NULL);

         if(!Result)
         {
            /* Initialization successful, go ahead and inform the user  */
            /* that it was successful and flag that the Platform Manager*/
            /* has been initialized.                                    */
            printf("BTPM_Initialize() Success: %d.\r\n", Result);

            /* If the caller would like to Register an Event Callback   */
            /* then we will do that at this time.                       */
            if(TempParam->Params[0].intParam)
            {
               if((Result = DEVM_RegisterEventCallback(DEVM_Event_Callback, NULL)) > 0)
               {
                  printf("DEVM_RegisterEventCallback() Success: %d.\r\n", Result);

                  /* Note the Callback ID and flag success.             */
                  DEVMCallbackID = (unsigned int)Result;

                  Initialized    = TRUE;

                  ret_val        = 0;
               }
               else
               {
                  /* Error registering the Callback, inform user and    */
                  /* flag an error.                                     */
                  printf("DEVM_RegisterEventCallback() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

                  ret_val = FUNCTION_ERROR;

                  /* Since there was an error, go ahead and clean up the*/
                  /* library.                                           */
                  BTPM_Cleanup();
               }
            }
            else
            {
               /* Nothing more to do, simply flag success to the caller.*/
               Initialized = TRUE;

               ret_val     = 0;
            }
         }
         else
         {
            /* Error initializing Platform Manager, inform the user.    */
            printf("BTPM_Initialize() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         printf("Usage: Initialize [0/1 - Register for Events].\r\n");

         /* One or more of the necessary parameters is/are invalid.     */
         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Already initialized, flag an error.                            */
      printf("Initialization Failure: Already initialized.\r\n");

      ret_val = FUNCTION_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for Cleaning up/Shutting    */
   /* down the Bluetopia Platform Manager Framework.  This function     */
   /* returns zero if successful and a negative value if an error       */
   /* occurred.                                                         */
static int Cleanup(ParameterList_t *TempParam)
{
   int ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* If there was an Event Callback Registered, then we need to     */
      /* un-register it.                                                */
      if(DEVMCallbackID)
         DEVM_UnRegisterEventCallback(DEVMCallbackID);

      if(AuthenticationCallbackID)
         DEVM_UnRegisterAuthentication(AuthenticationCallbackID);

      if(HFREventCallbackID)
         HFRM_Un_Register_Event_Callback(HFREventCallbackID);

      if(HFRDataCallbackID)
         HFRM_Un_Register_Data_Event_Callback(HFRDataCallbackID);

      /* Nothing to do other than to clean up the Bluetopia Platform    */
      /* Manager Service and flag that it is no longer initialized.     */
      BTPM_Cleanup();

      Initialized              = FALSE;
      DEVMCallbackID           = 0;
      AuthenticationCallbackID = 0;
      HFREventCallbackID       = 0;
      HFRDataCallbackID        = 0;
      ASSIGN_BD_ADDR(CurrentRemoteBD_ADDR, 0, 0, 0, 0, 0, 0);

      ret_val                  = 0;
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for Registering a Local     */
   /* Device Manager Callback with the Bluetopia Platform Manager       */
   /* Framework.  This function returns zero if successful and a        */
   /* negative value if an error occurred.                              */
static int RegisterEventCallback(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* If there is an Event Callback Registered, then we need to flag */
      /* an error.                                                      */
      if(!DEVMCallbackID)
      {
         /* Callback has not been registered, go ahead and attempt to   */
         /* register it.                                                */
         if((Result = DEVM_RegisterEventCallback(DEVM_Event_Callback, NULL)) > 0)
         {
            printf("DEVM_RegisterEventCallback() Success: %d.\r\n", Result);

            /* Note the Callback ID and flag success.                   */
            DEVMCallbackID = (unsigned int)Result;

            ret_val        = 0;
         }
         else
         {
            /* Error registering the Callback, inform user and flag an  */
            /* error.                                                   */
            printf("DEVM_RegisterEventCallback() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* Callback already registered, go ahead and notify the user.  */
         printf("Device Manager Event Callback already registered.\r\n");

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for Un-Registering a Local  */
   /* Device Manager Callback that has previously been registered with  */
   /* the Bluetopia Platform Manager Framework.  This function returns  */
   /* zero if successful and a negative value if an error occurred.     */
static int UnRegisterEventCallback(ParameterList_t *TempParam)
{
   int ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Next, check to make sure that there is an Event Callback       */
      /* already registered.                                            */
      if(DEVMCallbackID)
      {
         /* Callback has been registered, go ahead and attempt to       */
         /* un-register it.                                             */
         DEVM_UnRegisterEventCallback(DEVMCallbackID);

         printf("DEVM_UnRegisterEventCallback() Success.\r\n");

         /* Flag that there is no longer a Callback registered.         */
         DEVMCallbackID = 0;

         ret_val        = 0;
      }
      else
      {
         /* Callback already registered, go ahead and notify the user.  */
         printf("Device Manager Event Callback is not registered.\r\n");

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for Setting the Device Power*/
   /* of the Local Device.  This function returns zero if successful and*/
   /* a negative value if an error occurred.                            */
static int SetDevicePower(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First, check to make sure that we are not already initialized.    */
   if(Initialized)
   {
      /* Determine if the user would like to Power On or Off the Local  */
      /* Device with the calling of this command.                       */
      if((TempParam) && (TempParam->NumberofParameters >= 1))
      {
         /* Now actually Perform the command.                           */
         if(TempParam->Params[0].intParam)
            Result = DEVM_PowerOnDevice();
         else
            Result = DEVM_PowerOffDevice();

         if(!Result)
         {
            /* Device Power request was successful, go ahead and inform */
            /* the User.                                                */
            printf("DEVM_Power%sDevice() Success: %d.\r\n", TempParam->Params[0].intParam?"On":"Off", Result);

            /* Return success to the caller.                            */
            ret_val = 0;
         }
         else
         {
            /* Error Powering On/Off the device, inform the user.       */
            printf("DEVM_Power%sDevice() Failure: %d, %s.\r\n", TempParam->Params[0].intParam?"On":"Off", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         printf("Usage: SetDevicePower [0/1 - Power Off/Power On].\r\n");

         /* One or more of the necessary parameters is/are invalid.     */
         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = FUNCTION_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for querying the current    */
   /* device power for the local device.  This function returns zero if */
   /* successful and a negative value if an error occurred.             */
static int QueryDevicePower(ParameterList_t *TempParam)
{
   int       ret_val;
   Boolean_t Result;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Framework has been initialized, go ahead and query the current */
      /* Power state.                                                   */
      Result = DEVM_QueryDevicePowerState();

      if(Result >= 0)
         printf("DEVM_QueryDevicePowerState() Success: %s.\r\n", Result?"On":"Off");
      else
         printf("DEVM_QueryDevicePowerState() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

      /* Flag success to the caller.                                    */
      ret_val = 0;
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for Setting the Debug Zone  */
   /* Mask (either Local or Remote).  This function returns zero if     */
   /* successful and a negative value if an error occurred.             */
static int SetLocalRemoteDebugZoneMask(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First, check to make sure that we are not already initialized.    */
   if(Initialized)
   {
      /* Determine if the user would like to set the Local Library Debug*/
      /* Mask or the Remote Services Debug Mask.                        */
      if((TempParam) && (TempParam->NumberofParameters >= 2))
      {
         /* Now actually Perform the command.                           */
         Result = BTPM_SetDebugZoneMask((Boolean_t)((TempParam->Params[0].intParam)?TRUE:FALSE), (unsigned long)TempParam->Params[1].intParam);

         if(!Result)
         {
            /* Set Debug Zone Mask request was successful, go ahead and */
            /* inform the User.                                         */
            printf("BTPM_SetDebugZoneMask(%s) Success: 0x%08lX.\r\n", TempParam->Params[0].intParam?"Remote":"Local", (unsigned long)TempParam->Params[1].intParam);

            /* Return success to the caller.                            */
            ret_val = 0;
         }
         else
         {
            /* Error Querying Debug Zone Mask, inform the user.         */
            printf("BTPM_SetDebugZoneMask(%s) Failure: %d, %s.\r\n", TempParam->Params[0].intParam?"Remote":"Local", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         printf("Usage: SetDebugZoneMask [0/1 - Local/Service] [Debug Zone Mask].\r\n");

         /* One or more of the necessary parameters is/are invalid.     */
         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = FUNCTION_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for Querying the current    */
   /* Debug Zone Mask (either Local or Remote).  This function returns  */
   /* zero if successful and a negative value if an error occurred.     */
static int QueryLocalRemoteDebugZoneMask(ParameterList_t *TempParam)
{
   int           Result;
   int           ret_val;
   unsigned long DebugZoneMask;

   /* First, check to make sure that we are not already initialized.    */
   if(Initialized)
   {
      /* Determine if the user would like to query the Local Library    */
      /* Debug Mask or the Remote Services Debug Mask.                  */
      if((TempParam) && (TempParam->NumberofParameters >= 1))
      {
         /* Now actually Perform the command.                           */
         Result = BTPM_QueryDebugZoneMask((Boolean_t)((TempParam->Params[0].intParam)?TRUE:FALSE), (TempParam->NumberofParameters > 1)?TempParam->Params[1].intParam:0, &DebugZoneMask);

         if(!Result)
         {
            /* Query Debug Zone Mask request was successful, go ahead   */
            /* and inform the User.                                     */
            printf("BTPM_QueryDebugZoneMask(%s) Success: 0x%08lX.\r\n", TempParam->Params[0].intParam?"Remote":"Local", DebugZoneMask);

            /* Return success to the caller.                            */
            ret_val = 0;
         }
         else
         {
            /* Error Querying Debug Zone Mask, inform the user.         */
            printf("BTPM_QueryDebugZoneMask(%s, %d) Failure: %d, %s.\r\n", TempParam->Params[0].intParam?"Remote":"Local", (TempParam->NumberofParameters > 1)?TempParam->Params[1].intParam:0, Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         printf("Usage: QueryDebugZoneMask [0/1 - Local/Service] [Page Number - optional, default 0].\r\n");

         /* One or more of the necessary parameters is/are invalid.     */
         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = FUNCTION_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for shutting down the remote*/
   /* server.  This function returns zero if successful and a negative  */
   /* value if an error occurred.                                       */
static int ShutdownService(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Initialized, go ahead and attempt to shutdown the server.      */
      if((Result = BTPM_ShutdownService()) == 0)
      {
         printf("BTPM_ShutdownService() Success: %d.\r\n", Result);

         /* Flag success.                                               */
         ret_val = 0;
      }
      else
      {
         /* Error shutting down the service, inform the user and flag an*/
         /* error.                                                      */
         printf("BTPM_ShutdownService() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for displaying the current  */
   /* local device properties of the server.  This function returns zero*/
   /* if successful and a negative value if an error occurred.          */
static int QueryLocalDeviceProperties(ParameterList_t *TempParam)
{
   int                            Result;
   int                            ret_val;
   DEVM_Local_Device_Properties_t LocalDeviceProperties;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Initialized, go ahead and query the Local Device Properties.   */
      if((Result = DEVM_QueryLocalDeviceProperties(&LocalDeviceProperties)) >= 0)
      {
         printf("DEVM_QueryLocalDeviceProperties() Success: %d.\r\n", Result);

         /* Next, go ahead and display the properties.                  */
         DisplayLocalDeviceProperties(0, &LocalDeviceProperties);

         /* Flag success.                                               */
         ret_val = 0;
      }
      else
      {
         /* Error querying the Local Device Properties, inform the user */
         /* and flag an error.                                          */
         printf("DEVM_QueryLocalDeviceProperties() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for setting the Local Name  */
   /* Device Property of the local device.  This function returns zero  */
   /* if successful and a negative value if an error occurred.          */
static int SetLocalDeviceName(ParameterList_t *TempParam)
{
   int                            Result;
   int                            ret_val;
   DEVM_Local_Device_Properties_t LocalDeviceProperties;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      BTPS_MemInitialize(&LocalDeviceProperties, 0, sizeof(LocalDeviceProperties));

      if((TempParam) && (TempParam->NumberofParameters))
      {
         LocalDeviceProperties.DeviceNameLength = strlen(TempParam->Params[0].strParam);
         strcpy(LocalDeviceProperties.DeviceName, TempParam->Params[0].strParam);
      }

      printf("Attempting to set Device Name to: \"%s\".\r\n", LocalDeviceProperties.DeviceName);

      if((Result = DEVM_UpdateLocalDeviceProperties(DEVM_UPDATE_LOCAL_DEVICE_PROPERTIES_DEVICE_NAME, &LocalDeviceProperties)) >= 0)
      {
         printf("DEVM_UpdateLocalDeviceProperties() Success: %d.\r\n", Result);

         /* Flag success.                                               */
         ret_val = 0;
      }
      else
      {
         /* Error updating the Local Device Properties, inform the user */
         /* and flag an error.                                          */
         printf("DEVM_UpdateLocalDeviceProperties() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for setting the Local Class */
   /* of Device Device Property of the local device.  This function     */
   /* returns zero if successful and a negative value if an error       */
   /* occurred.                                                         */
static int SetLocalClassOfDevice(ParameterList_t *TempParam)
{
   int                            Result;
   int                            ret_val;
   DEVM_Local_Device_Properties_t LocalDeviceProperties;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      BTPS_MemInitialize(&LocalDeviceProperties, 0, sizeof(LocalDeviceProperties));

      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters))
      {
         ASSIGN_CLASS_OF_DEVICE(LocalDeviceProperties.ClassOfDevice, (Byte_t)((TempParam->Params[0].intParam) & 0xFF), (Byte_t)(((TempParam->Params[0].intParam) >> 8) & 0xFF), (Byte_t)(((TempParam->Params[0].intParam) >> 16) & 0xFF));

         printf("Attempting to set Class Of Device to: 0x%02X%02X%02X.\r\n", LocalDeviceProperties.ClassOfDevice.Class_of_Device0, LocalDeviceProperties.ClassOfDevice.Class_of_Device1, LocalDeviceProperties.ClassOfDevice.Class_of_Device2);

         if((Result = DEVM_UpdateLocalDeviceProperties(DEVM_UPDATE_LOCAL_DEVICE_PROPERTIES_CLASS_OF_DEVICE, &LocalDeviceProperties)) >= 0)
         {
            printf("DEVM_UpdateLocalDeviceProperties() Success: %d.\r\n", Result);

            /* Flag success.                                            */
            ret_val = 0;
         }
         else
         {
            /* Error updating the Local Device Properties, inform the   */
            /* user and flag an error.                                  */
            printf("DEVM_UpdateLocalDeviceProperties() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: SetLocalClassOfDevice [Class of Device].\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for setting the Local       */
   /* Discoverability Mode Device Property of the local device.  This   */
   /* function returns zero if successful and a negative value if an    */
   /* error occurred.                                                   */
static int SetDiscoverable(ParameterList_t *TempParam)
{
   int                            Result;
   int                            ret_val;
   DEVM_Local_Device_Properties_t LocalDeviceProperties;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      BTPS_MemInitialize(&LocalDeviceProperties, 0, sizeof(LocalDeviceProperties));

      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters > 0))
      {
         LocalDeviceProperties.DiscoverableMode = (Boolean_t)TempParam->Params[0].intParam;

         if(TempParam->NumberofParameters > 1)
            LocalDeviceProperties.DiscoverableModeTimeout = TempParam->Params[1].intParam;

         if(TempParam->Params[0].intParam)
         {
            if(LocalDeviceProperties.DiscoverableModeTimeout)
               printf("Attempting to set Discoverability Mode: Limited (%d Seconds).\r\n", LocalDeviceProperties.DiscoverableModeTimeout);
            else
               printf("Attempting to set Discoverability Mode: General.\r\n");
         }
         else
            printf("Attempting to set Discoverability Mode: None.\r\n");

         if((Result = DEVM_UpdateLocalDeviceProperties(DEVM_UPDATE_LOCAL_DEVICE_PROPERTIES_DISCOVERABLE_MODE, &LocalDeviceProperties)) >= 0)
         {
            printf("DEVM_UpdateLocalDeviceProperties() Success: %d.\r\n", Result);

            /* Flag success.                                            */
            ret_val = 0;
         }
         else
         {
            /* Error updating the Local Device Properties, inform the   */
            /* user and flag an error.                                  */
            printf("DEVM_UpdateLocalDeviceProperties() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: SetDiscoverable [Enable/Disable] [Timeout (Enable only)].\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for setting the Local       */
   /* Connectability Mode Device Property of the local device.  This    */
   /* function returns zero if successful and a negative value if an    */
   /* error occurred.                                                   */
static int SetConnectable(ParameterList_t *TempParam)
{
   int                            Result;
   int                            ret_val;
   DEVM_Local_Device_Properties_t LocalDeviceProperties;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      BTPS_MemInitialize(&LocalDeviceProperties, 0, sizeof(LocalDeviceProperties));

      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters > 0))
      {
         LocalDeviceProperties.ConnectableMode = (Boolean_t)TempParam->Params[0].intParam;

         if(TempParam->NumberofParameters > 1)
            LocalDeviceProperties.ConnectableModeTimeout = TempParam->Params[1].intParam;

         if(TempParam->Params[0].intParam)
         {
            if(LocalDeviceProperties.ConnectableModeTimeout)
               printf("Attempting to set Connectability Mode: Connectable (%d Seconds).\r\n", LocalDeviceProperties.ConnectableModeTimeout);
            else
               printf("Attempting to set Connectability Mode: Connectable.\r\n");
         }
         else
            printf("Attempting to set Connectability Mode: Non-Connectable.\r\n");

         if((Result = DEVM_UpdateLocalDeviceProperties(DEVM_UPDATE_LOCAL_DEVICE_PROPERTIES_CONNECTABLE_MODE, &LocalDeviceProperties)) >= 0)
         {
            printf("DEVM_UpdateLocalDeviceProperties() Success: %d.\r\n", Result);

            /* Flag success.                                            */
            ret_val = 0;
         }
         else
         {
            /* Error updating the Local Device Properties, inform the   */
            /* user and flag an error.                                  */
            printf("DEVM_UpdateLocalDeviceProperties() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: SetConnectable [Enable/Disable] [Timeout (Enable only)].\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for setting the Local       */
   /* Pairability Mode Device Property of the local device.  This       */
   /* function returns zero if successful and a negative value if an    */
   /* error occurred.                                                   */
static int SetPairable(ParameterList_t *TempParam)
{
   int                            Result;
   int                            ret_val;
   DEVM_Local_Device_Properties_t LocalDeviceProperties;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      BTPS_MemInitialize(&LocalDeviceProperties, 0, sizeof(LocalDeviceProperties));

      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters > 0))
      {
         LocalDeviceProperties.PairableMode = (Boolean_t)TempParam->Params[0].intParam;

         if(TempParam->NumberofParameters > 1)
            LocalDeviceProperties.PairableModeTimeout = TempParam->Params[1].intParam;

         if(TempParam->Params[0].intParam)
         {
            if(LocalDeviceProperties.PairableModeTimeout)
               printf("Attempting to set Pairability Mode: Pairable (%d Seconds).\r\n", LocalDeviceProperties.PairableModeTimeout);
            else
               printf("Attempting to set Pairability Mode: Pairable.\r\n");
         }
         else
            printf("Attempting to set Pairability Mode: Non-Pairable.\r\n");

         if((Result = DEVM_UpdateLocalDeviceProperties(DEVM_UPDATE_LOCAL_DEVICE_PROPERTIES_PAIRABLE_MODE, &LocalDeviceProperties)) >= 0)
         {
            printf("DEVM_UpdateLocalDeviceProperties() Success: %d.\r\n", Result);

            /* Flag success.                                            */
            ret_val = 0;
         }
         else
         {
            /* Error updating the Local Device Properties, inform the   */
            /* user and flag an error.                                  */
            printf("DEVM_UpdateLocalDeviceProperties() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: SetPairable [Enable/Disable] [Timeout (Enable only)].\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for starting a Device       */
   /* Discovery on the local device.  This function returns zero if     */
   /* successful and a negative value if an error occurred.             */
static int StartDeviceDiscovery(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters))
      {
         if(TempParam->Params[0].intParam)
            printf("Attempting to Start Discovery (%d Seconds).\r\n", TempParam->Params[0].intParam);
         else
            printf("Attempting to Start Discovery (INDEFINITE).\r\n");

         if((Result = DEVM_StartDeviceDiscovery(TempParam->Params[0].intParam)) >= 0)
         {
            printf("DEVM_StartDeviceDiscovery() Success: %d.\r\n", Result);

            /* Flag success.                                            */
            ret_val = 0;
         }
         else
         {
            /* Error attempting to start Device Discovery, inform the   */
            /* user and flag an error.                                  */
            printf("DEVM_StartDeviceDiscovery() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: StartDeviceDiscovery [Duration].\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for stopping a Device       */
   /* Discovery on the local device.  This function returns zero if     */
   /* successful and a negative value if an error occurred.             */
static int StopDeviceDiscovery(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Initialized, go ahead and attempt to stop Device Discovery.    */
      if((Result = DEVM_StopDeviceDiscovery()) >= 0)
      {
         printf("DEVM_StopDeviceDiscovery() Success: %d.\r\n", Result);

         /* Flag success.                                               */
         ret_val = 0;
      }
      else
      {
         /* Error stopping Device Discovery, inform the user and flag an*/
         /* error.                                                      */
         printf("DEVM_StopDeviceDiscovery() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for Querying the Remote     */
   /* Device Address List.  This function returns zero if successful and*/
   /* a negative value if an error occurred.                            */
static int QueryRemoteDeviceList(ParameterList_t *TempParam)
{
   int                Result;
   int                ret_val;
   char               Buffer[32];
   BD_ADDR_t         *BD_ADDRList;
   unsigned int       Index;
   unsigned int       Filter;
   unsigned int       TotalNumberDevices;
   Class_of_Device_t  ClassOfDevice;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters))
      {
         printf("Attempting Query %d Devices.\r\n", TempParam->Params[0].intParam);

         if(TempParam->Params[0].intParam)
            BD_ADDRList = (BD_ADDR_t *)BTPS_AllocateMemory(sizeof(BD_ADDR_t)*TempParam->Params[0].intParam);
         else
            BD_ADDRList = NULL;

         if((!TempParam->Params[0].intParam) || ((TempParam->Params[0].intParam) && (BD_ADDRList)))
         {
            /* If there were any optional parameters specified, go ahead*/
            /* and note them.                                           */
            if(TempParam->NumberofParameters > 1)
               Filter = TempParam->Params[1].intParam;
            else
               Filter = 0;

            if(TempParam->NumberofParameters > 2)
            {
               ASSIGN_CLASS_OF_DEVICE(ClassOfDevice, (Byte_t)((TempParam->Params[2].intParam) & 0xFF), (Byte_t)(((TempParam->Params[2].intParam) >> 8) & 0xFF), (Byte_t)(((TempParam->Params[2].intParam) >> 16) & 0xFF));
            }
            else
            {
               ASSIGN_CLASS_OF_DEVICE(ClassOfDevice, 0, 0, 0);
            }

            if((Result = DEVM_QueryRemoteDeviceList(Filter, ClassOfDevice, TempParam->Params[0].intParam, BD_ADDRList, &TotalNumberDevices)) >= 0)
            {
               printf("DEVM_QueryRemoteDeviceList() Success: %d, Total Number Devices: %d.\r\n", Result, TotalNumberDevices);

               if((Result) && (BD_ADDRList))
               {
                  printf("Returned device list (%d Entries):\r\n", Result);

                  for(Index = 0; Index < Result; Index++)
                  {
                     BD_ADDRToStr(BD_ADDRList[Index], Buffer);

                     printf("%2d. %s\r\n", (Index + 1), Buffer);
                  }
               }

               /* Flag success.                                         */
               ret_val = 0;
            }
            else
            {
               /* Error attempting to start Device Discovery, inform the*/
               /* user and flag an error.                               */
               printf("DEVM_QueryRemoteDeviceList() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

               ret_val = FUNCTION_ERROR;
            }

            /* Free any memory that was allocated.                      */
            if(BD_ADDRList)
               BTPS_FreeMemory(BD_ADDRList);
         }
         else
         {
            /* Unable to allocate memory for List.                      */
            printf("Unable to allocate memory for %d Devices.\r\n", TempParam->Params[0].intParam);

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: QueryRemoteDeviceList [Number of Devices] [Filter (Optional)] [COD Filter (Optional)].\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for Querying the Remote     */
   /* Device Properties of a specific Remote Device.  This function     */
   /* returns zero if successful and a negative value if an error       */
   /* occurred.                                                         */
static int QueryRemoteDeviceProperties(ParameterList_t *TempParam)
{
   int                             Result;
   int                             ret_val;
   BD_ADDR_t                       BD_ADDR;
   Boolean_t                       ForceUpdate;
   DEVM_Remote_Device_Properties_t RemoteDeviceProperties;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters))
      {
         /* Convert the parameter to a Bluetooth Device Address.        */
         StrToBD_ADDR(TempParam->Params[0].strParam, &BD_ADDR);

         if(TempParam->NumberofParameters > 1)
            ForceUpdate = (Boolean_t)(TempParam->Params[1].intParam?TRUE:FALSE);
         else
            ForceUpdate = FALSE;

         printf("Attempting to Query Device Properties: %s, ForceUpdate: %s.\r\n", TempParam->Params[0].strParam, ForceUpdate?"TRUE":"FALSE");

         if((Result = DEVM_QueryRemoteDeviceProperties(BD_ADDR, ForceUpdate, &RemoteDeviceProperties)) >= 0)
         {
            printf("DEVM_QueryRemoteDeviceProperties() Success: %d.\r\n", Result);

            /* Display the Remote Device Properties.                    */
            DisplayRemoteDeviceProperties(0, &RemoteDeviceProperties);

            /* Flag success.                                            */
            ret_val = 0;
         }
         else
         {
            /* Error Querying Remote Device, inform the user and flag an*/
            /* error.                                                   */
            printf("DEVM_QueryRemoteDeviceProperties() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: QueryRemoteDeviceProperties [BD_ADDR] [Force Update].\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for Adding the specified    */
   /* Remote Device.  This function returns zero if successful and a    */
   /* negative value if an error occurred.                              */
static int AddRemoteDevice(ParameterList_t *TempParam)
{
   int                                   Result;
   int                                   ret_val;
   BD_ADDR_t                             BD_ADDR;
   Class_of_Device_t                     ClassOfDevice;
   DEVM_Remote_Device_Application_Data_t ApplicationData;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters))
      {
         /* Convert the parameter to a Bluetooth Device Address.        */
         StrToBD_ADDR(TempParam->Params[0].strParam, &BD_ADDR);

         /* Check to see if a Class of Device was specified.            */
         if(TempParam->NumberofParameters > 1)
         {
            ASSIGN_CLASS_OF_DEVICE(ClassOfDevice, (Byte_t)((TempParam->Params[1].intParam) & 0xFF), (Byte_t)(((TempParam->Params[1].intParam) >> 8) & 0xFF), (Byte_t)(((TempParam->Params[1].intParam) >> 16) & 0xFF));
         }
         else
         {
            ASSIGN_CLASS_OF_DEVICE(ClassOfDevice, 0, 0, 0);
         }

         printf("Attempting to Add Device: %s.\r\n", TempParam->Params[0].strParam);

         /* Check to see if Application Information was specified.      */
         if(TempParam->NumberofParameters > 2)
         {
            ApplicationData.FriendlyNameLength = strlen(TempParam->Params[2].strParam);

            strcpy(ApplicationData.FriendlyName, TempParam->Params[2].strParam);

            ApplicationData.ApplicationInfo = 0;

            if(TempParam->NumberofParameters > 3)
               ApplicationData.ApplicationInfo = TempParam->Params[3].intParam;
         }

         if((Result = DEVM_AddRemoteDevice(BD_ADDR, ClassOfDevice, (TempParam->NumberofParameters > 2)?&ApplicationData:NULL)) >= 0)
         {
            printf("DEVM_AddRemoteDevice() Success: %d.\r\n", Result);

            /* Flag success.                                            */
            ret_val = 0;
         }
         else
         {
            /* Error Adding Remote Device, inform the user and flag an  */
            /* error.                                                   */
            printf("DEVM_AddRemoteDevice() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: AddRemoteDevice [BD_ADDR] [[COD (Optional)] [Friendly Name (Optional)] [Application Info (Optional)]].\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for Deleting the specified  */
   /* Remote Device.  This function returns zero if successful and a    */
   /* negative value if an error occurred.                              */
static int DeleteRemoteDevice(ParameterList_t *TempParam)
{
   int       Result;
   int       ret_val;
   BD_ADDR_t BD_ADDR;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters))
      {
         /* Convert the parameter to a Bluetooth Device Address.        */
         StrToBD_ADDR(TempParam->Params[0].strParam, &BD_ADDR);

         printf("Attempting to Delete Device: %s.\r\n", TempParam->Params[0].strParam);

         if((Result = DEVM_DeleteRemoteDevice(BD_ADDR)) >= 0)
         {
            printf("DEVM_DeleteRemoteDevice() Success: %d.\r\n", Result);

            /* Flag success.                                            */
            ret_val = 0;
         }
         else
         {
            /* Error Deleting Remote Device, inform the user and flag an*/
            /* error.                                                   */
            printf("DEVM_DeleteRemoteDevice() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: DeleteRemoteDevice [BD_ADDR].\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for Deleting the specified  */
   /* Remote Devices.  This function returns zero if successful and a   */
   /* negative value if an error occurred.                              */
static int DeleteRemoteDevices(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters))
      {
         printf("Attempting to Delete Remote Devices, Filter %d.\r\n", TempParam->Params[0].intParam);

         if((Result = DEVM_DeleteRemoteDevices(TempParam->Params[0].intParam)) >= 0)
         {
            printf("DEVM_DeleteRemoteDevices() Success: %d.\r\n", Result);

            /* Flag success.                                            */
            ret_val = 0;
         }
         else
         {
            /* Error Deleting Remote Devices, inform the user and flag  */
            /* an error.                                                */
            printf("DEVM_DeleteRemoteDevices() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: DeleteRemoteDevices [Device Delete Filter].\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for Pairing the specified   */
   /* Remote Device.  This function returns zero if successful and a    */
   /* negative value if an error occurred.                              */
static int PairWithRemoteDevice(ParameterList_t *TempParam)
{
   int       Result;
   int       ret_val;
   BD_ADDR_t BD_ADDR;
   Boolean_t Force;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters))
      {
         /* Convert the parameter to a Bluetooth Device Address.        */
         StrToBD_ADDR(TempParam->Params[0].strParam, &BD_ADDR);

         if(TempParam->NumberofParameters > 1)
            Force = (Boolean_t)(TempParam->Params[1].intParam?1:0);
         else
            Force = (Boolean_t)0;

         printf("Attempting to Pair With Remote Device: %s (Force = %d).\r\n", TempParam->Params[0].strParam, Force);

         if((Result = DEVM_PairWithRemoteDevice(BD_ADDR, Force)) >= 0)
         {
            printf("DEVM_PairWithRemoteDevice() Success: %d.\r\n", Result);

            /* Flag success.                                            */
            ret_val = 0;
         }
         else
         {
            /* Error Pairing with Remote Device, inform the user and    */
            /* flag an error.                                           */
            printf("DEVM_PairWithRemoteDevice() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: PairWithRemoteDevice [BD_ADDR] [Force Pair (Optional)].\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for Cancelling an on-going  */
   /* Pairing operation with the specified Remote Device.  This function*/
   /* returns zero if successful and a negative value if an error       */
   /* occurred.                                                         */
static int CancelPairWithRemoteDevice(ParameterList_t *TempParam)
{
   int       Result;
   int       ret_val;
   BD_ADDR_t BD_ADDR;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters))
      {
         /* Convert the parameter to a Bluetooth Device Address.        */
         StrToBD_ADDR(TempParam->Params[0].strParam, &BD_ADDR);

         printf("Attempting to Cancel Pair With Remote Device: %s.\r\n", TempParam->Params[0].strParam);

         if((Result = DEVM_CancelPairWithRemoteDevice(BD_ADDR)) >= 0)
         {
            printf("DEVM_CancelPairWithRemoteDevice() Success: %d.\r\n", Result);

            /* Flag success.                                            */
            ret_val = 0;
         }
         else
         {
            /* Error Cancelling Pairing with Remote Device, inform the  */
            /* user and flag an error.                                  */
            printf("DEVM_CancelPairWithRemoteDevice() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: CancelPairWithRemoteDevice [BD_ADDR].\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for Un-Pairing the specified*/
   /* Remote Device.  This function returns zero if successful and a    */
   /* negative value if an error occurred.                              */
static int UnPairRemoteDevice(ParameterList_t *TempParam)
{
   int       Result;
   int       ret_val;
   BD_ADDR_t BD_ADDR;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters))
      {
         /* Convert the parameter to a Bluetooth Device Address.        */
         StrToBD_ADDR(TempParam->Params[0].strParam, &BD_ADDR);

         printf("Attempting to Un-Pair Remote Device: %s.\r\n", TempParam->Params[0].strParam);

         if((Result = DEVM_UnPairRemoteDevice(BD_ADDR, 0)) >= 0)
         {
            printf("DEVM_UnPairRemoteDevice() Success: %d.\r\n", Result);

            /* Flag success.                                            */
            ret_val = 0;
         }
         else
         {
            /* Error Un-Pairing with Remote Device, inform the user and */
            /* flag an error.                                           */
            printf("DEVM_UnPairRemoteDevice() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: UnPairRemoteDevice [BD_ADDR].\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for Querying the Remote     */
   /* Device Services of the specified Remote Device.  This function    */
   /* returns zero if successful and a negative value if an error       */
   /* occurred.                                                         */
static int QueryRemoteDeviceServices(ParameterList_t *TempParam)
{
   int                     Result;
   int                     ret_val;
   BD_ADDR_t               BD_ADDR;
   unsigned int            Index;
   unsigned int            TotalServiceSize;
   unsigned char          *ServiceData;
   DEVM_Parsed_SDP_Data_t  ParsedSDPData;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters > 1))
      {
         /* Initialize success.                                         */
         ret_val = 0;

         /* Convert the parameter to a Bluetooth Device Address.        */
         StrToBD_ADDR(TempParam->Params[0].strParam, &BD_ADDR);

         printf("Attempting Query Remote Device %s For Services.\r\n", TempParam->Params[0].strParam);

         if(!TempParam->Params[1].intParam)
         {
            /* Caller has requested to actually retrieve it locally,    */
            /* determine how many bytes were requested.                 */
            if(TempParam->NumberofParameters > 2)
               ServiceData = (unsigned char *)BTPS_AllocateMemory(TempParam->Params[2].intParam);
            else
               ret_val = INVALID_PARAMETERS_ERROR;
         }
         else
            ServiceData = NULL;

         if(!ret_val)
         {
            if((TempParam->Params[1].intParam) || ((!TempParam->Params[1].intParam) && (ServiceData)))
            {
               if((Result = DEVM_QueryRemoteDeviceServices(BD_ADDR, (Boolean_t)TempParam->Params[1].intParam, TempParam->Params[1].intParam?0:TempParam->Params[2].intParam, ServiceData, &TotalServiceSize)) >= 0)
               {
                  printf("DEVM_QueryRemoteDeviceServices() Success: %d, Total Number Service Bytes: %d.\r\n", Result, (TempParam->Params[1].intParam)?0:TotalServiceSize);

                  /* Now convert the Raw Data to parsed data.           */
                  if(Result)
                  {
                     if(ServiceData)
                     {
                        printf("Returned Service Data (%d Bytes):\r\n", Result);

                        for(Index = 0; Index < ((unsigned int)Result); Index++)
                           printf("%02X", ServiceData[Index]);

                        printf("\r\n");
                     }

                     Result = DEVM_ConvertRawSDPStreamToParsedSDPData(Result, ServiceData, &ParsedSDPData);

                     if(!Result)
                     {
                        /* Success, Display the Parsed Data.            */
                        DisplayParsedServiceData(&ParsedSDPData);

                        /* All finished with the parsed data, so free   */
                        /* it.                                          */
                        DEVM_FreeParsedSDPData(&ParsedSDPData);
                     }
                     else
                        printf("DEVM_ConvertRawSDPStreamToParsedSDPData() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));
                  }

                  /* Flag success.                                      */
                  ret_val = 0;
               }
               else
               {
                  /* Error attempting to query Services, inform the user*/
                  /* and flag an error.                                 */
                  printf("DEVM_QueryRemoteDeviceServices() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

                  ret_val = FUNCTION_ERROR;
               }

               /* Free any memory that was allocated.                   */
               if(ServiceData)
                  BTPS_FreeMemory(ServiceData);
            }
            else
            {
               /* Unable to allocate memory for List.                   */
               printf("Unable to allocate memory for %d Service Bytes.\r\n", TempParam->Params[2].intParam);

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* One or more of the necessary parameters is/are invalid.  */
            printf("Usage: QueryRemoteDeviceServices [BD_ADDR] [Force Update] [Bytes to Query (specified if Force is 0)].\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: QueryRemoteDeviceServices [BD_ADDR] [Force Update] [Bytes to Query (specified if Force is 0)].\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for Registering the Local   */
   /* Client to receive (and process) Authentication Events.  This      */
   /* function returns zero if successful and a negative value if an    */
   /* error occurred.                                                   */
static int RegisterAuthentication(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Initialized, go ahead and attempt to Register for              */
      /* Authentication.                                                */
      if((Result = DEVM_RegisterAuthentication(DEVM_Authentication_Callback, NULL)) >= 0)
      {
         printf("DEVM_RegisterAuthentication() Success: %d.\r\n", Result);

         /* Note the Authentication Callback ID.                        */
         AuthenticationCallbackID = (unsigned int)Result;

         /* Flag success.                                               */
         ret_val                  = 0;
      }
      else
      {
         /* Error Registering for Authentication, inform the user and   */
         /* flag an error.                                              */
         printf("DEVM_RegisterAuthentication() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for Un-Registering the Local*/
   /* Client to receive (and process) Authentication Events.  This      */
   /* function returns zero if successful and a negative value if an    */
   /* error occurred.                                                   */
static int UnRegisterAuthentication(ParameterList_t *TempParam)
{
   int ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Initialized, go ahead and attempt to Register for              */
      /* Authentication.                                                */
      DEVM_UnRegisterAuthentication(AuthenticationCallbackID);

      printf("DEVM_UnRegisterAuthentication() Success.\r\n");

      /* Clear the Authentication Callback ID.                          */
      AuthenticationCallbackID = 0;

      /* Flag success.                                                  */
      ret_val                  = 0;
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for issuing a GAP           */
   /* Authentication Response with a PIN Code value specified via the   */
   /* input parameter.  This function returns zero on successful        */
   /* execution and a negative value on all errors.                     */
static int PINCodeResponse(ParameterList_t *TempParam)
{
   int                               Result;
   int                               ret_val;
   BD_ADDR_t                         NullADDR;
   PIN_Code_t                        PINCode;
   DEVM_Authentication_Information_t AuthenticationResponseInformation;

   ASSIGN_BD_ADDR(NullADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
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
            BTPS_MemInitialize(&AuthenticationResponseInformation, 0, sizeof(AuthenticationResponseInformation));

            AuthenticationResponseInformation.BD_ADDR                    = CurrentRemoteBD_ADDR;
            AuthenticationResponseInformation.AuthenticationAction       = DEVM_AUTHENTICATION_ACTION_PIN_CODE_RESPONSE;
            AuthenticationResponseInformation.AuthenticationDataLength   = (Byte_t)(strlen(TempParam->Params[0].strParam));

            AuthenticationResponseInformation.AuthenticationData.PINCode = PINCode;

            /* Submit the Authentication Response.                      */
            Result = DEVM_AuthenticationResponse(AuthenticationCallbackID, &AuthenticationResponseInformation);

            /* Check the return value for the submitted command for     */
            /* success.                                                 */
            if(!Result)
            {
               /* Operation was successful, inform the user.            */
               printf("DEVM_AuthenticationResponse(), Pin Code Response Success.\r\n");

               /* Flag success to the caller.                           */
               ret_val = 0;
            }
            else
            {
               /* Inform the user that the Authentication Response was  */
               /* not successful.                                       */
               printf("DEVM_AuthenticationResponse() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

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
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for issuing a GAP           */
   /* Authentication Response with a Pass Key value specified via the   */
   /* input parameter.  This function returns zero on successful        */
   /* execution and a negative value on all errors.                     */
static int PassKeyResponse(ParameterList_t *TempParam)
{
   int                               Result;
   int                               ret_val;
   BD_ADDR_t                         NullADDR;
   DEVM_Authentication_Information_t AuthenticationResponseInformation;

   ASSIGN_BD_ADDR(NullADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
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
            BTPS_MemInitialize(&AuthenticationResponseInformation, 0, sizeof(AuthenticationResponseInformation));

            AuthenticationResponseInformation.BD_ADDR                    = CurrentRemoteBD_ADDR;
            AuthenticationResponseInformation.AuthenticationAction       = DEVM_AUTHENTICATION_ACTION_PASSKEY_RESPONSE;
            AuthenticationResponseInformation.AuthenticationDataLength   = sizeof(AuthenticationResponseInformation.AuthenticationData.Passkey);

            AuthenticationResponseInformation.AuthenticationData.Passkey = (DWord_t)(TempParam->Params[0].intParam);

            /* Submit the Authentication Response.                      */
            Result = DEVM_AuthenticationResponse(AuthenticationCallbackID, &AuthenticationResponseInformation);

            /* Check the return value for the submitted command for     */
            /* success.                                                 */
            if(!Result)
            {
               /* Operation was successful, inform the user.            */
               printf("DEVM_AuthenticationResponse(), Passkey Response Success.\r\n");

               /* Flag success to the caller.                           */
               ret_val = 0;
            }
            else
            {
               /* Inform the user that the Authentication Response was  */
               /* not successful.                                       */
               printf("DEVM_AuthenticationResponse() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

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
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for issuing a GAP           */
   /* Authentication Response with a User Confirmation value specified  */
   /* via the input parameter.  This function returns zero on successful*/
   /* execution and a negative value on all errors.                     */
static int UserConfirmationResponse(ParameterList_t *TempParam)
{
   int                               Result;
   int                               ret_val;
   BD_ADDR_t                         NullADDR;
   DEVM_Authentication_Information_t AuthenticationResponseInformation;

   ASSIGN_BD_ADDR(NullADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
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
            BTPS_MemInitialize(&AuthenticationResponseInformation, 0, sizeof(AuthenticationResponseInformation));

            AuthenticationResponseInformation.BD_ADDR                         = CurrentRemoteBD_ADDR;
            AuthenticationResponseInformation.AuthenticationAction            = DEVM_AUTHENTICATION_ACTION_USER_CONFIRMATION_RESPONSE;
            AuthenticationResponseInformation.AuthenticationDataLength        = sizeof(AuthenticationResponseInformation.AuthenticationData.Confirmation);

            AuthenticationResponseInformation.AuthenticationData.Confirmation = (Boolean_t)(TempParam->Params[0].intParam?TRUE:FALSE);

            /* Submit the Authentication Response.                      */
            Result = DEVM_AuthenticationResponse(AuthenticationCallbackID, &AuthenticationResponseInformation);

            /* Check the return value for the submitted command for     */
            /* success.                                                 */
            if(!Result)
            {
               /* Operation was successful, inform the user.            */
               printf("DEVM_AuthenticationResponse(), User Confirmation Response Success.\r\n");

               /* Flag success to the caller.                           */
               ret_val = 0;
            }
            else
            {
               /* Inform the user that the Authentication Response was  */
               /* not successful.                                       */
               printf("DEVM_AuthenticationResponse() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

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
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
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

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 2) && (TempParam->Params[0].intParam <= 3))
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
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for setting up or releasing */
   /* an audio connection.  This function returns zero on successful    */
   /* execution and a negative value on all errors.                     */
static int ManageAudioConnection(ParameterList_t *TempParam)
{
   int       ret_val;
   BD_ADDR_t BD_ADDR;

   /* Verify that the HFR Event Callback has been registered.           */
   if(HFREventCallbackID)
   {
      /* make sure the passed in parameters appears to be semi-valid.   */
      if((TempParam) && (TempParam->NumberofParameters > 1))
      {
         /* Convert the parameter to a Bluetooth Device Address.        */
         StrToBD_ADDR(TempParam->Params[0].strParam, &BD_ADDR);

         /* Check to see if this is a request to setup an audio         */
         /* connection or disconnect an audio connection.               */
         if(TempParam->Params[1].intParam)
         {
            /* This is a request to setup an audio connection, call the */
            /* Setup Audio Connection function.                         */
            ret_val = HFRMSetupAudioConnection(BD_ADDR);
         }
         else
         {
            /* This is a request to disconnect an audio connection, call*/
            /* the Release Audio Connection function.                   */
            ret_val = HFRMReleaseAudioConnection(BD_ADDR);
         }
      }
      else
      {
         /* The required parameter is invalid.                          */
         printf("Usage: ManageAudio [BD_ADDR] [Release = 0, Setup = 1].\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* One or more of the necessary parameters is/are invalid.        */
      printf("HFR Event Callback MUST be registered before making this call.\r\n");

      ret_val = INVALID_PARAMETERS_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for Setting up an Audio     */
   /* Connection.  This function returns zero on successful execution   */
   /* and a negative value on all errors.                               */
static int HFRMSetupAudioConnection(BD_ADDR_t BD_ADDR)
{
   int Result;
   int ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* The Port ID appears to be a semi-valid value.  Now submit the  */
      /* command.                                                       */
      Result  = HFRM_Setup_Audio_Connection(HFREventCallbackID, hctHandsFree, BD_ADDR);

      /* Set the return value of this function equal to the Result of   */
      /* the function call.                                             */
      ret_val = Result;

      /* Now check to see if the command was submitted successfully.    */
      if(!Result)
      {
         /* The function was submitted successfully.                    */
         printf("HFRM_Setup_Audio_Connection: Function Successful.\r\n");
      }
      else
      {
         /* There was an error submitting the function.                 */
         printf("HFRM_Setup_Audio_Connection() Failure: %d.\r\n", Result);
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for Releasing an existing   */
   /* Audio Connection.  This function returns zero on successful       */
   /* execution and a negative value on all errors.                     */
static int HFRMReleaseAudioConnection(BD_ADDR_t BD_ADDR)
{
   int Result;
   int ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* The Port ID appears to be a semi-valid value.  Now submit the  */
      /* command.                                                       */
      Result  = HFRM_Release_Audio_Connection(HFREventCallbackID, hctHandsFree, BD_ADDR);

      /* Set the return value of this function equal to the Result of   */
      /* the function call.                                             */
      ret_val = Result;

      /* Now check to see if the command was submitted successfully.    */
      if(!Result)
      {
         /* The function was submitted successfully.                    */
         printf("HFRM_Release_Audio_Connection: Function Successful.\r\n");
      }
      else
      {
         /* There was an error submitting the function.                 */
         printf("HFRM_Release_Audio_Connection() Failure: %d.\r\n", Result);
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for establishing a Hands    */
   /* Free connection.  This function returns zero on successful        */
   /* execution and a negative value on all errors.                     */
static int HFRMConnectDevice(ParameterList_t *TempParam)
{
   int           Result;
   int           ret_val;
   BD_ADDR_t     BD_ADDR;
   unsigned int  ServerPortNumber;
   unsigned long ConnectionFlags;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Verify that the HFR Event Callback has been registered.        */
      if(HFREventCallbackID)
      {
         /* The port ID appears to be at least semi-valid, now check to */
         /* make sure the passed in parameters appears to be semi-valid.*/
         if((TempParam) && (TempParam->NumberofParameters > 0))
         {
            /* Convert the parameter to a Bluetooth Device Address.     */
            StrToBD_ADDR(TempParam->Params[0].strParam, &BD_ADDR);

            /* Determine if a port number was specified, if one is not  */
            /* specified we will query the device manager.              */
            if((TempParam->NumberofParameters >= 2) && (TempParam->Params[1].intParam))
            {
               ServerPortNumber = TempParam->Params[1].intParam;

               Result           = 0;
            }
            else
            {
               /* Port number was not specified so query the Port Number*/
               /* for the specified BD_ADDR.                            */
               Result = GetHandsFreeServiceInformation(BD_ADDR, FALSE, &ServerPortNumber);
            }

            if(!Result)
            {
               /* Determine if Connection Flags were specified.         */
               if(TempParam->NumberofParameters >= 3)
                  ConnectionFlags = (unsigned long)TempParam->Params[2].intParam;
               else
                  ConnectionFlags = 0;

               /* Call the function to connect the remote device.       */
               Result = HFRM_Connect_Remote_Device(hctHandsFree, BD_ADDR, ServerPortNumber, ConnectionFlags, HFRM_Event_Callback, NULL, NULL);

               /* Set the return value of this function equal to the    */
               /* Result of the function call.                          */
               ret_val = Result;

               if(!Result)
               {
                  /* The function was submitted successfully.           */
                  printf("HFRM_Connect_Remote_Device: Function Successful.\r\n");
               }
               else
               {
                  /* There was an error submitting the function.        */
                  printf("HFRM_Connect_Remote_Device() Failure: %d (%s).\r\n", Result, ERR_ConvertErrorCodeToString(Result));
               }
            }
            else
            {
               printf("Failed to find the Port Number on the specified remote device. (Did you do SDP)?\r\n");

               /* Set the return value of this function equal to the    */
               /* Result of the function call.                          */
               ret_val = Result;
            }
         }
         else
         {
            /* One or more of the necessary parameters is/are invalid.  */
            printf("Usage: Connect [BD_ADDR] [Port Number (optional)] [Connection Flags (optional - 0 = No Flags, 1 = Authentication, 2 = Encryption, 3 = Authentication/Encryption)].\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("HFR Event Callback MUST be registered before making this call.\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for disconnecting a Hands   */
   /* Free connection.  This function returns zero on successful        */
   /* execution and a negative value on all errors.                     */
static int HFRMDisconnectDevice(ParameterList_t *TempParam)
{
   int       Result;
   int       ret_val;
   BD_ADDR_t BD_ADDR;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Verify that the HFR Event Callback has been registered.        */
      if(HFREventCallbackID)
      {
         /* The port ID appears to be at least semi-valid, now check to */
         /* make sure the passed in parameters appears to be semi-valid.*/
         if((TempParam) && (TempParam->NumberofParameters > 0))
         {
            /* Convert the parameter to a Bluetooth Device Address.     */
            StrToBD_ADDR(TempParam->Params[0].strParam, &BD_ADDR);

            /* Disconnect the Device.                                   */
            Result = HFRM_Disconnect_Device(hctHandsFree, BD_ADDR);

            /* Set the return value of this function equal to the Result*/
            /* of the function call.                                    */
            ret_val = Result;

            if(!Result)
            {
               /* The function was submitted successfully.              */
               printf("HFRM_Disconnect_Device: Function Successful.\r\n");
            }
            else
            {
               /* There was an error submitting the function.           */
               printf("HFRM_Disconnect_Device() Failure: %d (%s).\r\n", Result, ERR_ConvertErrorCodeToString(Result));
            }
         }
         else
         {
            /* One or more of the necessary parameters is/are invalid.  */
            printf("Usage: Disconnect [BD_ADDR].\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("HFR Event Callback MUST be registered before making this call.\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for sending a Connection    */
   /* Request Response to a Hands Free connection request.  This        */
   /* function returns zero on successful execution and a negative value*/
   /* on all errors.                                                    */
static int HFRMConnectionRequestResponse(ParameterList_t *TempParam)
{
   int       Result;
   int       ret_val;
   BD_ADDR_t BD_ADDR;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Verify that the HFR Event Callback has been registered.        */
      if(HFREventCallbackID)
      {
         /* The port ID appears to be at least semi-valid, now check to */
         /* make sure the passed in parameters appears to be semi-valid.*/
         if((TempParam) && (TempParam->NumberofParameters > 1))
         {
            /* Convert the parameter to a Bluetooth Device Address.     */
            StrToBD_ADDR(TempParam->Params[0].strParam, &BD_ADDR);

            /* Send the Connection Request Response to the Device.      */
            Result = HFRM_Connection_Request_Response(hctHandsFree, BD_ADDR, (Boolean_t)(TempParam->Params[1].intParam));

            /* Set the return value of this function equal to the Result*/
            /* of the function call.                                    */
            ret_val = Result;

            if(!Result)
            {
               /* The function was submitted successfully.              */
               printf("HFRM_Connection_Request_Response: Function Successful.\r\n");
            }
            else
            {
               /* There was an error submitting the function.           */
               printf("HFRM_Connection_Request_Response() Failure: %d (%s).\r\n", Result, ERR_ConvertErrorCodeToString(Result));
            }
         }
         else
         {
            /* One or more of the necessary parameters is/are invalid.  */
            printf("Usage: ConnectRequestResponse [BD_ADDR] [Accept (0 = Reject, 1 = Accept)].\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("HFR Event Callback MUST be registered before making this call.\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for disabling Sound         */
   /* Enhancement on the Remote Device.  This function returns zero on  */
   /* successful execution and a negative value on all errors.          */
static int HFRMDisableRemoteEchoCancelationNoiseReduction(ParameterList_t *TempParam)
{
   int       Result;
   int       ret_val;
   BD_ADDR_t BD_ADDR;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Verify that the HFR Event Callback has been registered.        */
      if(HFREventCallbackID)
      {
         /* The port ID appears to be at least semi-valid, now check to */
         /* make sure the passed in parameters appears to be semi-valid.*/
         if((TempParam) && (TempParam->NumberofParameters > 0))
         {
            /* Convert the parameter to a Bluetooth Device Address.     */
            StrToBD_ADDR(TempParam->Params[0].strParam, &BD_ADDR);

            /* The Port ID appears to be is a semi-valid value.  Now    */
            /* submit the command.                                      */
            Result  = HFRM_Disable_Remote_Echo_Cancellation_Noise_Reduction(HFREventCallbackID, hctHandsFree, BD_ADDR);

            /* Set the return value of this function equal to the Result*/
            /* of the function call.                                    */
            ret_val = Result;

            if(!Result)
            {
               /* The function was submitted successfully.              */
               printf("HFRM_Disable_Remote_Echo_Cancellation_Noise_Reduction: Function Successful.\r\n");
            }
            else
            {
               /* There was an error submitting the function.           */
               printf("HFRM_Disable_Remote_Echo_Cancellation_Noise_Reduction() Failure: %d (%s).\r\n", Result, ERR_ConvertErrorCodeToString(Result));
            }
         }
         else
         {
            /* One or more of the necessary parameters is/are invalid.  */
            printf("Usage: DisableRemoteEchoCancelationNoiseReduction [BD_ADDR].\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("HFR Event Callback MUST be registered before making this call.\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for setting the Speaker Gain*/
   /* on Remote Device.  This function returns zero on successful       */
   /* execution and a negative value on all errors.                     */
static int HFRMSetRemoteSpeakerGain(ParameterList_t *TempParam)
{
   int       Result;
   int       ret_val;
   BD_ADDR_t BD_ADDR;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Verify that the HFR Event Callback has been registered.        */
      if(HFREventCallbackID)
      {
         /* The Port ID appears to be semi-valid, now check the Speaker */
         /* Gain.                                                       */
         if((TempParam) && (TempParam->NumberofParameters > 1) && (TempParam->Params[1].intParam <= HFRE_SPEAKER_GAIN_MAXIMUM))
         {
            /* Convert the parameter to a Bluetooth Device Address.     */
            StrToBD_ADDR(TempParam->Params[0].strParam, &BD_ADDR);

            /* The Speaker Gain is a valid value.  Now submit the       */
            /* command.                                                 */
            Result  = HFRM_Set_Remote_Speaker_Gain(HFREventCallbackID, hctHandsFree, BD_ADDR, TempParam->Params[1].intParam);

            /* Set the return value of this function equal to the Result*/
            /* of the function call.                                    */
            ret_val = Result;

            if(!Result)
            {
               /* The function was submitted successfully.              */
               printf("HFRE_Set_Remote_Speaker_Gain: Function Successful.\r\n");
            }
            else
            {
               /* There was an error submitting the function.           */
               printf("HFRM_Set_Remote_Voice_Recognition_Activation() Failure: %d (%s).\r\n", Result, ERR_ConvertErrorCodeToString(Result));
            }
         }
         else
         {
            /* The Speaker Gain parameter is invalid.                   */
            printf("Usage: SetSpeakerGain [BD_ADDR] [0 <= SpeakerGain <= 15].\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("HFR Event Callback MUST be registered before making this call.\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for setting the Microphone  */
   /* Gain on Remote Device.  This function returns zero on successful  */
   /* execution and a negative value on all errors.                     */
static int HFRMSetRemoteMicrophoneGain(ParameterList_t *TempParam)
{
   int       Result;
   int       ret_val;
   BD_ADDR_t BD_ADDR;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Verify that the HFR Event Callback has been registered.        */
      if(HFREventCallbackID)
      {
         /* The Port ID appears to be semi-valid, now check the         */
         /* Microphone Gain.                                            */
         if((TempParam) && (TempParam->NumberofParameters > 1) && (TempParam->Params[1].intParam <= HFRE_MICROPHONE_GAIN_MAXIMUM))
         {
            /* Convert the parameter to a Bluetooth Device Address.     */
            StrToBD_ADDR(TempParam->Params[0].strParam, &BD_ADDR);

            /* The Microphone Gain is a valid value.  Now submit the    */
            /* command.                                                 */
            Result  = HFRM_Set_Remote_Microphone_Gain(HFREventCallbackID, hctHandsFree, BD_ADDR, TempParam->Params[1].intParam);

            /* Set the return value of this function equal to the Result*/
            /* of the function call.                                    */
            ret_val = Result;

            if(!Result)
            {
               /* The function was submitted successfully.              */
               printf("HFRM_Set_Remote_Microphone_Gain(): Function Successful.\r\n");
            }
            else
            {
               /* There was an error submitting the function.           */
               printf("HFRM_Set_Remote_Microphone_Gain() Failure: %d (%s).\r\n", Result, ERR_ConvertErrorCodeToString(Result));
            }
         }
         else
         {
            /* The Microphone Gain parameter is invalid.                */
            printf("Usage: SetMicrophoneGain [BD_ADDR] [0 <= MicrophoneGain <= 15].\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("HFR Event Callback MUST be registered before making this call.\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for Querying the Remote     */
   /* Device Services of the specified Remote Device.  This function    */
   /* returns zero if successful and a negative value if an error       */
   /* occurred.                                                         */
static int QueryHandsFreeServices(ParameterList_t *TempParam)
{
   int       ret_val;
   BD_ADDR_t BD_ADDR;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters > 1))
      {
         /* Initialize success.                                         */
         ret_val = 0;

         /* Convert the parameter to a Bluetooth Device Address.        */
         StrToBD_ADDR(TempParam->Params[0].strParam, &BD_ADDR);

         printf("Attempting Query Remote Device %s For Services.\r\n", TempParam->Params[0].strParam);

         ret_val = GetHandsFreeServiceInformation(BD_ADDR, TempParam->Params[1].intParam, NULL);
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: QueryHandsFreeServices [BD_ADDR] [Force Update].\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is used to change the incoming connection  */
   /* flags.                                                            */
static int ChangeIncomingConnectionFlags(ParameterList_t *TempParam)
{
   int ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters > 0))
      {
         /* Change the incoming connection flags.                       */
         ret_val = HFRM_Change_Incoming_Connection_Flags(hctHandsFree, (unsigned long)TempParam->Params[0].intParam);
         if(!ret_val)
         {
            /* The function was submitted successfully.                 */
            printf("HFRM_Change_Incoming_Connection_Flags(): Function Successful.\r\n");
         }
         else
         {
            /* There was an error submitting the function.              */
            printf("HFRM_Change_Incoming_Connection_Flags() Failure: %d (%s).\r\n", ret_val, ERR_ConvertErrorCodeToString(ret_val));
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: ChangeIncomingConnectionFlags [Connection Flags].\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for enabling or disabling   */
   /* remote indicator event notification on the remote audio gateway.  */
   /* This function returns zero on successful execution and a negative */
   /* value on all errors.                                              */
static int HFRMEnableRemoteIndicatorEventNotification(ParameterList_t *TempParam)
{
   int       Result;
   int       ret_val;
   BD_ADDR_t BD_ADDR;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Verify that the HFR Event Callback has been registered.        */
      if(HFREventCallbackID)
      {
         /* The port ID appears to be at least semi-valid, now check to */
         /* make sure the passed in parameters appears to be semi-valid.*/
         if((TempParam) && (TempParam->NumberofParameters > 1))
         {
            /* Convert the parameter to a Bluetooth Device Address.     */
            StrToBD_ADDR(TempParam->Params[0].strParam, &BD_ADDR);

            /* The Port ID appears to be is a semi-valid value.  Now    */
            /* submit the command.                                      */
            Result  = HFRM_Enable_Remote_Indicator_Event_Notification(HFREventCallbackID, BD_ADDR, (TempParam->Params[1].intParam)?TRUE:FALSE);

            /* Set the return value of this function equal to the Result*/
            /* of the function call.                                    */
            ret_val = Result;

            if(!Result)
            {
               /* The function was submitted successfully.              */
               printf("HFRM_Enable_Remote_Indicator_Event_Notification: Function Successful.\r\n");
            }
            else
            {
               /* There was an error submitting the function.           */
               printf("HFRM_Enable_Remote_Indicator_Event_Notification() Failure: %d (%s).\r\n", Result, ERR_ConvertErrorCodeToString(Result));
            }
         }
         else
         {
            /* One or more of the necessary parameters is/are invalid.  */
            printf("Usage: EnableRemoteIndicatorNotification [BD_ADDR] [Disable = 0, Enable = 1].\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("HFR Event Callback MUST be registered before making this call.\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for enabling or disabling   */
   /* Call Waiting Notification on the remote audio gateway.  This      */
   /* function returns zero on successful execution and a negative value*/
   /* on all errors.                                                    */
static int HFRMEnableRemoteCallWaitingNotification(ParameterList_t *TempParam)
{
   int       Result;
   int       ret_val;
   BD_ADDR_t BD_ADDR;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Verify that the HFR Event Callback has been registered.        */
      if(HFREventCallbackID)
      {
         /* The port ID appears to be at least semi-valid, now check to */
         /* make sure the passed in parameters appears to be semi-valid.*/
         if((TempParam) && (TempParam->NumberofParameters > 1))
         {
            /* Convert the parameter to a Bluetooth Device Address.     */
            StrToBD_ADDR(TempParam->Params[0].strParam, &BD_ADDR);

            /* The Port ID appears to be is a semi-valid value.  Now    */
            /* submit the command.                                      */
            Result  = HFRM_Enable_Remote_Call_Waiting_Notification(HFREventCallbackID, BD_ADDR, (TempParam->Params[1].intParam)?TRUE:FALSE);

            /* Set the return value of this function equal to the Result*/
            /* of the function call.                                    */
            ret_val = Result;

            if(!Result)
            {
               /* The function was submitted successfully.              */
               printf("HFRM_Enable_Remote_Call_Waiting_Notification: Function Successful.\r\n");
            }
            else
            {
               /* There was an error submitting the function.           */
               printf("HFRM_Enable_Remote_Call_Waiting_Notification() Failure: %d (%s).\r\n", Result, ERR_ConvertErrorCodeToString(Result));
            }
         }
         else
         {
            /* One or more of the necessary parameters is/are invalid.  */
            printf("Usage: EnableRemoteCallWaitingNotifcation [BD_ADDR] [Disable = 0, Enable = 1].\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("HFR Event Callback MUST be registered before making this call.\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for enabling or disabling   */
   /* Remote Call Line Identification Notification on the Remote Audio  */
   /* Gateway.  This function returns zero on successful execution and a*/
   /* negative value on all errors.                                     */
static int HFRMEnableRemoteCallLineIdentificationNotification(ParameterList_t *TempParam)
{
   int       Result;
   int       ret_val;
   BD_ADDR_t BD_ADDR;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Verify that the HFR Event Callback has been registered.        */
      if(HFREventCallbackID)
      {
         /* The port ID appears to be at least semi-valid, now check to */
         /* make sure the passed in parameters appears to be semi-valid.*/
         if((TempParam) && (TempParam->NumberofParameters > 1))
         {
            /* Convert the parameter to a Bluetooth Device Address.     */
            StrToBD_ADDR(TempParam->Params[0].strParam, &BD_ADDR);

            /* The Port ID appears to be is a semi-valid value.  Now    */
            /* submit the command.                                      */
            Result  = HFRM_Enable_Remote_Call_Line_Identification_Notification(HFREventCallbackID, BD_ADDR, (TempParam->Params[1].intParam)?TRUE:FALSE);

            /* Set the return value of this function equal to the Result*/
            /* of the function call.                                    */
            ret_val = Result;

            if(!Result)
            {
               /* The function was submitted successfully.              */
               printf("HFRM_Enable_Remote_Call_Line_Identification_Notification: Function Successful.\r\n");
            }
            else
            {
               /* There was an error submitting the function.           */
               printf("HFRM_Enable_Remote_Call_Line_Identification_Notification() Failure: %d (%s).\r\n", Result, ERR_ConvertErrorCodeToString(Result));
            }
         }
         else
         {
            /* One or more of the necessary parameters is/are invalid.  */
            printf("Usage: EnableRemoteCallerIDNotification [BD_ADDR] [Disable = 0, Enable = 1].\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("HFR Event Callback MUST be registered before making this call.\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for Querying the Current    */
   /* Indicator Status of the Remote Audio Gateway.  This function      */
   /* returns zero on successful execution and a negative value on all  */
   /* errors.                                                           */
static int HFRMQueryRemoteIndicatorsStatus(ParameterList_t *TempParam)
{
   int       Result;
   int       ret_val;
   BD_ADDR_t BD_ADDR;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Verify that the HFR Event Callback has been registered.        */
      if(HFREventCallbackID)
      {
         /* The port ID appears to be at least semi-valid, now check to */
         /* make sure the passed in parameters appears to be semi-valid.*/
         if((TempParam) && (TempParam->NumberofParameters > 0))
         {
            /* Convert the parameter to a Bluetooth Device Address.     */
            StrToBD_ADDR(TempParam->Params[0].strParam, &BD_ADDR);

            /* The Port ID appears to be a semi-valid value.  Now submit*/
            /* the command.                                             */
            Result  = HFRM_Query_Remote_Control_Indicator_Status(HFREventCallbackID, BD_ADDR);

            /* Set the return value of this function equal to the Result*/
            /* of the function call.                                    */
            ret_val = Result;

            /* Now check to see if the command was submitted            */
            /* successfully.                                            */
            if(!Result)
            {
               /* The function was submitted successfully.              */
               printf("HFRM_Query_Remote_Control_Indicator_Status: Function Successful.\r\n");
            }
            else
            {
               /* There was an error submitting the function.           */
               printf("HFRM_Query_Remote_Control_Indicator_Status() Failure: %d (%s).\r\n", Result, ERR_ConvertErrorCodeToString(Result));
            }
         }
         else
         {
            /* One or more of the necessary parameters is/are invalid.  */
            printf("Usage: QueryRemoteControlIndicators [BD_ADDR].\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("HFR Event Callback MUST be registered before making this call.\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for Querying the Call       */
   /* Holding and Mutliparty Services Supported by the Remote Audio     */
   /* Gateway.  This function returns zero on successful execution and a*/
   /* negative value on all errors.                                     */
static int HFRMQueryRemoteCallHoldingMultipartyServiceSupport(ParameterList_t *TempParam)
{
   int       Result;
   int       ret_val;
   BD_ADDR_t BD_ADDR;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Verify that the HFR Event Callback has been registered.        */
      if(HFREventCallbackID)
      {
         /* The port ID appears to be at least semi-valid, now check to */
         /* make sure the passed in parameters appears to be semi-valid.*/
         if((TempParam) && (TempParam->NumberofParameters > 0))
         {
            /* Convert the parameter to a Bluetooth Device Address.     */
            StrToBD_ADDR(TempParam->Params[0].strParam, &BD_ADDR);

            /* The Port ID appears to be a semi-valid value.  Now submit*/
            /* the command.                                             */
            Result  = HFRM_Query_Remote_Call_Holding_Multiparty_Service_Support(HFREventCallbackID, BD_ADDR);

            /* Set the return value of this function equal to the Result*/
            /* of the function call.                                    */
            ret_val = Result;

            /* Now check to see if the command was submitted            */
            /* successfully.                                            */
            if(!Result)
            {
               /* The function was submitted successfully.              */
               printf("HFRM_Query_Remote_Call_Holding_Multiparty_Service_Support: Function Successful.\r\n");
            }
            else
            {
               /* There was an error submitting the function.           */
               printf("HFRM_Query_Remote_Call_Holding_Multiparty_Service_Support() Failure: %d (%s).\r\n", Result, ERR_ConvertErrorCodeToString(Result));
            }
         }
         else
         {
            /* One or more of the necessary parameters is/are invalid.  */
            printf("Usage: QueryRemoteCallHoldingMultipartyServiceSupport [BD_ADDR].\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("HFR Event Callback MUST be registered before making this call.\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for sending a Call Holding  */
   /* and Mutliparty Service Selection to the Remote Audio Gateway.     */
   /* This function returns zero on successful execution and a negative */
   /* value on all errors.                                              */
static int HFRMSendCallHoldingMultipartySelection(ParameterList_t *TempParam)
{
   int                                       Result;
   int                                       ret_val;
   int                                       Index;
   BD_ADDR_t                                 BD_ADDR;
   HFRE_Call_Hold_Multiparty_Handling_Type_t Selection;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Verify that the HFR Event Callback has been registered.        */
      if(HFREventCallbackID)
      {
         /* The port ID appears to be at least semi-valid, now check to */
         /* make sure the passed in parameters appears to be semi-valid.*/
         if((TempParam) && (TempParam->NumberofParameters > 1))
         {
            /* Convert the parameter to a Bluetooth Device Address.     */
            StrToBD_ADDR(TempParam->Params[0].strParam, &BD_ADDR);

            /* If no index parameter was on the command line, make sure */
            /* we have a zero value for Index, and use the first        */
            /* parameter as HFRE_Call_Hold_Multiparty_Handling_Type_t   */
            /* selection value.                                         */
            if(TempParam->NumberofParameters < 3)
            {
               Index     = 0;

               Selection = TempParam->Params[1].intParam;
            }
            else
            {
               /* If we have an index parameter, we have to advise the  */
               /* profile that this is either a release specified call  */
               /* or a private consultation with the specified call Our */
               /* enum values allow us to use the first parameter as a  */
               /* selection with an offset value.                       */
               Index     = TempParam->Params[2].intParam;

               Selection = TempParam->Params[1].intParam + chConnectTwoCallsAndDisconnect;
            }

            /* Now submit the command.                                  */
            Result  = HFRM_Send_Call_Holding_Multiparty_Selection(HFREventCallbackID, BD_ADDR, Selection, Index);

            /* Set the return value of this function equal to the Result*/
            /* of the function call.                                    */
            ret_val = Result;

            if(!Result)
            {
               /* The function was submitted successfully.              */
               printf("HFRM_Send_Call_Holding_Multiparty_Selection: Function Successful.\r\n");
            }
            else
            {
               /* There was an error submitting the function.           */
               printf("HFRM_Send_Call_Holding_Multiparty_Selection() Failure: %d (%s).\r\n", Result, ERR_ConvertErrorCodeToString(Result));
            }
         }
         else
         {
            /* One or more of the necessary parameters is/are invalid.  */
            printf("Usage: SendCallHoldSelection [BD_ADDR] [Call Hold Selection] [Optional Index].\r\n");
            printf("       Call Hold Selection 0=Release All Held Calls.\r\n");
            printf("       Call Hold Selection 1=Release All Active Calls, Accept Waiting Call.\r\n");
            printf("       Call Hold Selection 2=Place All Calls on Hold, Accept Waiting Call.\r\n");
            printf("       Call Hold Selection 3=Add Held Call to Conversation.\r\n");
            printf("       Call Hold Selection 4=Connect Two Calls and Disconnect.\r\n");
            printf("       Call Hold Selection 5=Release Specified Call, Accept Waiting Call.\r\n");
            printf("       Call Hold Selection 6=Private Consultation with Specified Call.\r\n");
            printf("       Optional Index = Specified call for CHLD Selection values 5 or 6\r          (defaults to zero).\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("HFR Event Callback MUST be registered before making this call.\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for sending the command to  */
   /* dial the specified phone number on the Remote Audio Gateway.  This*/
   /* function returns zero on successful execution and a negative value*/
   /* on all errors.                                                    */
static int HFRMDialPhoneNumber(ParameterList_t *TempParam)
{
   int       Result;
   int       ret_val;
   BD_ADDR_t BD_ADDR;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Verify that the HFR Event Callback has been registered.        */
      if(HFREventCallbackID)
      {
         /* The Port ID appears to be at least semi-valid, now check the*/
         /* required parameters for this command.                       */
         if((TempParam) && (TempParam->NumberofParameters > 1) && (TempParam->Params[1].strParam) && (strlen(TempParam->Params[1].strParam)))
         {
            /* Convert the parameter to a Bluetooth Device Address.     */
            StrToBD_ADDR(TempParam->Params[0].strParam, &BD_ADDR);

            /* The parameters appear to be is a semi-valid value.  Now  */
            /* submit the command.                                      */
            Result  = HFRM_Dial_Phone_Number(HFREventCallbackID, BD_ADDR, TempParam->Params[1].strParam);

            /* Set the return value of this function equal to the Result*/
            /* of the function call.                                    */
            ret_val = Result;

            if(!Result)
            {
               /* The function was submitted successfully.              */
               printf("HFRM_Dial_Phone_Number: Function Successful.\r\n");
            }
            else
            {
               /* There was an error submitting the function.           */
               printf("HFRM_Dial_Phone_Number() Failure: %d (%s).\r\n", Result, ERR_ConvertErrorCodeToString(Result));
            }
         }
         else
         {
            /* The Phone Number parameter is invalid.                   */
            printf("Usage: DialNumber [BD_ADDR] [Phone Number].\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("HFR Event Callback MUST be registered before making this call.\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for sending the command to  */
   /* dial a phone number from memory on the Remote Audio Gateway.  This*/
   /* function returns zero on successful execution and a negative value*/
   /* on all errors.                                                    */
static int HFRMDialPhoneNumberFromMemory(ParameterList_t *TempParam)
{
   int       Result;
   int       ret_val;
   BD_ADDR_t BD_ADDR;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Verify that the HFR Event Callback has been registered.        */
      if(HFREventCallbackID)
      {
         /* The Port ID appears to be at least semi-valid, now check the*/
         /* required parameters for this command.                       */
         if((TempParam) && (TempParam->NumberofParameters > 1))
         {
            /* Convert the parameter to a Bluetooth Device Address.     */
            StrToBD_ADDR(TempParam->Params[0].strParam, &BD_ADDR);

            /* The parameters appear to be is a semi-valid value.  Now  */
            /* submit the command.                                      */
            Result  = HFRM_Dial_Phone_Number_From_Memory(HFREventCallbackID, BD_ADDR, TempParam->Params[1].intParam);

            /* Set the return value of this function equal to the Result*/
            /* of the function call.                                    */
            ret_val = Result;

            if(!Result)
            {
               /* The function was submitted successfully.              */
               printf("HFRM_Dial_Phone_Number_From_Memory: Function Successful.\r\n");
            }
            else
            {
               /* There was an error submitting the function.           */
               printf("HFRM_Dial_Phone_Number_From_Memory() Failure: %d (%s).\r\n", Result, ERR_ConvertErrorCodeToString(Result));
            }
         }
         else
         {
            /* The Memory Location parameter is invalid.                */
            printf("Usage: MemoryDial [BD_ADDR] [MemoryLocation].\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("HFR Event Callback MUST be registered before making this call.\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for sending the command to  */
   /* dial the last number dialed on the Remote Audio Gateway.  This    */
   /* function returns zero on successful execution and a negative value*/
   /* on all errors.                                                    */
static int HFRMRedialLastPhoneNumber(ParameterList_t *TempParam)
{
   int       Result;
   int       ret_val;
   BD_ADDR_t BD_ADDR;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Verify that the HFR Event Callback has been registered.        */
      if(HFREventCallbackID)
      {
         /* The Port ID appears to be at least semi-valid, now check the*/
         /* required parameters for this command.                       */
         if((TempParam) && (TempParam->NumberofParameters > 0))
         {
            /* Convert the parameter to a Bluetooth Device Address.     */
            StrToBD_ADDR(TempParam->Params[0].strParam, &BD_ADDR);

            /* The Port ID appears to be a semi-valid value.  Now submit*/
            /* the command.                                             */
            Result  = HFRM_Redial_Last_Phone_Number(HFREventCallbackID, BD_ADDR);

            /* Set the return value of this function equal to the Result*/
            /* of the function call.                                    */
            ret_val = Result;

            /* Now check to see if the command was submitted            */
            /* successfully.                                            */
            if(!Result)
            {
               /* The function was submitted successfully.              */
               printf("HFRM_Redial_Last_Phone_Number: Function Successful.\r\n");
            }
            else
            {
               /* There was an error submitting the function.           */
               printf("HFRM_Redial_Last_Phone_Number() Failure: %d (%s).\r\n", Result, ERR_ConvertErrorCodeToString(Result));
            }
         }
         else
         {
            /* The Memory Location parameter is invalid.                */
            printf("Usage: ReDial [BD_ADDR].\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("HFR Event Callback MUST be registered before making this call.\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for sending the command to  */
   /* Anwser an Incoming Call on the Remote Audio Gateway.  This        */
   /* function returns zero on successful execution and a negative value*/
   /* on all errors.                                                    */
static int HFRMAnswerIncomingCall(ParameterList_t *TempParam)
{
   int       Result;
   int       ret_val;
   BD_ADDR_t BD_ADDR;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Verify that the HFR Event Callback has been registered.        */
      if(HFREventCallbackID)
      {
         /* The Port ID appears to be at least semi-valid, now check the*/
         /* required parameters for this command.                       */
         if((TempParam) && (TempParam->NumberofParameters > 0))
         {
            /* Convert the parameter to a Bluetooth Device Address.     */
            StrToBD_ADDR(TempParam->Params[0].strParam, &BD_ADDR);

            /* The Port ID appears to be a semi-valid value.  Now submit*/
            /* the command.                                             */
            Result  = HFRM_Answer_Incoming_Call(HFREventCallbackID, BD_ADDR);

            /* Set the return value of this function equal to the Result*/
            /* of the function call.                                    */
            ret_val = Result;

            /* Now check to see if the command was submitted            */
            /* successfully.                                            */
            if(!Result)
            {
               /* The function was submitted successfully.              */
               printf("HFRM_Answer_Incoming_Call: Function Successful.\r\n");
            }
            else
            {
               /* There was an error submitting the function.           */
               printf("HFRM_Answer_Incoming_Call() Failure: %d (%s).\r\n", Result, ERR_ConvertErrorCodeToString(Result));
            }
         }
         else
         {
            /* The Memory Location parameter is invalid.                */
            printf("Usage: AnswerCall [BD_ADDR].\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("HFR Event Callback MUST be registered before making this call.\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for sending a DTMF Code to  */
   /* the Remote Audio Gateway.  This function returns zero on          */
   /* successful execution and a negative value on all errors.          */
static int HFRMTransmitDTMFCode(ParameterList_t *TempParam)
{
   int       Result;
   int       ret_val;
   BD_ADDR_t BD_ADDR;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Verify that the HFR Event Callback has been registered.        */
      if(HFREventCallbackID)
      {
         /* The Port ID appears to be semi-valid, now check the DTMF    */
         /* Code.                                                       */
         if((TempParam) && (TempParam->NumberofParameters > 1) && (TempParam->Params[1].strParam) && (strlen(TempParam->Params[1].strParam)))
         {
            /* Convert the parameter to a Bluetooth Device Address.     */
            StrToBD_ADDR(TempParam->Params[0].strParam, &BD_ADDR);

            /* The DTMF Code is a semi-valid value.  Now submit the     */
            /* command.                                                 */
            Result  = HFRM_Transmit_DTMF_Code(HFREventCallbackID, BD_ADDR, TempParam->Params[1].strParam[0]);

            /* Set the return value of this function equal to the Result*/
            /* of the function call.                                    */
            ret_val = Result;

            if(!Result)
            {
               /* The function was submitted successfully.              */
               printf("HFRM_Transmit_DTMF_Code(): Function Successful.\r\n");
            }
            else
            {
               /* There was an error submitting the function.           */
               printf("HFRM_Transmit_DTMF_Code() Failure: %d (%s).\r\n", Result, ERR_ConvertErrorCodeToString(Result));
            }
         }
         else
         {
            /* The DTMF Code parameter is invalid.                      */
            printf("Usage: SendDTMF [BD_ADDR] [DTMF Code].\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("HFR Event Callback MUST be registered before making this call.\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for deactivating Voice      */
   /* Recognition Activation on the Audio Gateway and for change the    */
   /* Voice Recognition Activation state on the Hands Free Unit.  This  */
   /* function returns zero on successful execution and a negative value*/
   /* on all errors.                                                    */
static int HFRMSetRemoteVoiceRecognitionActivation(ParameterList_t *TempParam)
{
   int       ret_val;
   int       Result;
   BD_ADDR_t BD_ADDR;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Verify that the HFR Event Callback has been registered.        */
      if(HFREventCallbackID)
      {
         /* The Port ID appears to be at least semi-valid, now check the*/
         /* required parameters for this command.                       */
         if((TempParam) && (TempParam->NumberofParameters > 1))
         {
            /* Convert the parameter to a Bluetooth Device Address.     */
            StrToBD_ADDR(TempParam->Params[0].strParam, &BD_ADDR);

            /* The Port ID appears to be is a semi-valid value.  Now    */
            /* submit the command.                                      */
            Result  = HFRM_Set_Remote_Voice_Recognition_Activation(HFREventCallbackID, hctHandsFree, BD_ADDR, (Boolean_t)TempParam->Params[1].intParam);

            /* Set the return value of this function equal to the Result*/
            /* of the function call.                                    */
            ret_val = Result;

            if(!Result)
            {
               /* The function was submitted successfully.              */
               printf("HFRM_Set_Remote_Voice_Recognition_Activation: Function Successful.\r\n");
            }
            else
            {
               /* There was an error submitting the function.           */
               printf("HFRM_Set_Remote_Voice_Recognition_Activation() Failure: %d (%s).\r\n", Result, ERR_ConvertErrorCodeToString(Result));
            }
         }
         else
         {
            /* The required parameter is invalid.                       */
            printf("Usage: SetVoiceRecognitionActivation [BD_ADDR] [Disable = 0, Enable = 1].\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("HFR Event Callback MUST be registered before making this call.\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for sending a Voice Tag     */
   /* Request to the Remote Audio Gateway.  This function returns zero  */
   /* on successful execution and a negative value on all errors.       */
static int HFRMVoiceTagRequest(ParameterList_t *TempParam)
{
   int       Result;
   int       ret_val;
   BD_ADDR_t BD_ADDR;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Verify that the HFR Event Callback has been registered.        */
      if(HFREventCallbackID)
      {
         /* The Port ID appears to be at least semi-valid, now check the*/
         /* required parameters for this command.                       */
         if((TempParam) && (TempParam->NumberofParameters > 0))
         {
            /* Convert the parameter to a Bluetooth Device Address.     */
            StrToBD_ADDR(TempParam->Params[0].strParam, &BD_ADDR);

            /* The Port ID appears to be a semi-valid value.  Now submit*/
            /* the command.                                             */
            Result  = HFRM_Voice_Tag_Request(HFREventCallbackID, BD_ADDR);

            /* Set the return value of this function equal to the Result*/
            /* of the function call.                                    */
            ret_val = Result;

            /* Now check to see if the command was submitted            */
            /* successfully.                                            */
            if(!Result)
            {
               /* The function was submitted successfully.              */
               printf("HFRM_Voice_Tag_Request: Function Successful.\r\n");
            }
            else
            {
               /* There was an error submitting the function.           */
               printf("HFRM_Voice_Tag_Request() Failure: %d (%s).\r\n", Result, ERR_ConvertErrorCodeToString(Result));
            }
         }
         else
         {
            /* The required parameter is invalid.                       */
            printf("Usage: VoiceTagRequest [BD_ADDR] [Disable = 0, Enable = 1].\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("HFR Event Callback MUST be registered before making this call.\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for sending the command to  */
   /* HangUp ongoing calls or reject incoming calls on the Remote Audio */
   /* Gateway.  This function returns zero on successful execution and a*/
   /* negative value on all errors.                                     */
static int HFRMHangUpCall(ParameterList_t *TempParam)
{
   int       Result;
   int       ret_val;
   BD_ADDR_t BD_ADDR;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Verify that the HFR Event Callback has been registered.        */
      if(HFREventCallbackID)
      {
         /* The Port ID appears to be at least semi-valid, now check the*/
         /* required parameters for this command.                       */
         if((TempParam) && (TempParam->NumberofParameters > 0))
         {
            /* Convert the parameter to a Bluetooth Device Address.     */
            StrToBD_ADDR(TempParam->Params[0].strParam, &BD_ADDR);

            /* The Port ID appears to be a semi-valid value.  Now submit*/
            /* the command.                                             */
            Result  = HFRM_Hang_Up_Call(HFREventCallbackID, BD_ADDR);

            /* Set the return value of this function equal to the Result*/
            /* of the function call.                                    */
            ret_val = Result;

            /* Now check to see if the command was submitted            */
            /* successfully.                                            */
            if(!Result)
            {
               /* The function was submitted successfully.              */
               printf("HFRM_Hang_Up_Call: Function Successful.\r\n");
            }
            else
            {
               /* There was an error submitting the function.           */
               printf("HFRM_Hang_Up_Call() Failure: %d (%s).\r\n", Result, ERR_ConvertErrorCodeToString(Result));
            }
         }
         else
         {
            /* The required parameter is invalid.                       */
            printf("Usage: HangUp [BD_ADDR].\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("HFR Event Callback MUST be registered before making this call.\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for sending the command to  */
   /* Query the current Operator selection on the Remote Audio Gateway  */
   /* Device.  This function returns zero on successful execution and a */
   /* negative value on all errors.                                     */
static int HFRMQueryOperator(ParameterList_t *TempParam)
{
   int       Result;
   int       ret_val;
   BD_ADDR_t BD_ADDR;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Verify that the HFR Event Callback has been registered.        */
      if(HFREventCallbackID)
      {
         /* The Port ID appears to be at least semi-valid, now check the*/
         /* required parameters for this command.                       */
         if((TempParam) && (TempParam->NumberofParameters > 0))
         {
            /* Convert the parameter to a Bluetooth Device Address.     */
            StrToBD_ADDR(TempParam->Params[0].strParam, &BD_ADDR);

            /* The Port ID appears to be is a semi-valid value.  Now    */
            /* submit the command.                                      */
            Result = HFRM_Query_Remote_Network_Operator_Selection(HFREventCallbackID, BD_ADDR);

            /* Set the return value of this function equal to the Result*/
            /* of the function call.                                    */
            ret_val = Result;

            /* Now check to see if the command was submitted            */
            /* successfully.                                            */
            if(!Result)
            {
               /* The function was submitted successfully.              */
               printf("HFRM_Query_Remote_Network_Operator_Selection: Function Successful.\r\n");
            }
            else
            {
               /* There was an error submitting the function.           */
               printf("HFRM_Query_Remote_Network_Operator_Selection() Failure: %d (%s).\r\n", Result, ERR_ConvertErrorCodeToString(Result));
            }
         }
         else
         {
            /* The required parameter is invalid.                       */
            printf("Usage: QueryNetworkOperator [BD_ADDR].\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("HFR Event Callback MUST be registered before making this call.\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for sending the command to  */
   /* Set the Operator Format on the Remote Audio Gateway Device.  This */
   /* function returns zero on successful execution and a negative value*/
   /* on all errors.                                                    */
static int HFRMSetOperatorFormat(ParameterList_t *TempParam)
{
   int       Result;
   int       ret_val;
   BD_ADDR_t BD_ADDR;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Verify that the HFR Event Callback has been registered.        */
      if(HFREventCallbackID)
      {
         /* The Port ID appears to be at least semi-valid, now check the*/
         /* required parameters for this command.                       */
         if((TempParam) && (TempParam->NumberofParameters > 0))
         {
            /* Convert the parameter to a Bluetooth Device Address.     */
            StrToBD_ADDR(TempParam->Params[0].strParam, &BD_ADDR);

            /* The Port ID appears to be is a semi-valid value.  Now    */
            /* submit the command.                                      */
            Result = HFRM_Set_Network_Operator_Selection_Format(HFREventCallbackID, BD_ADDR);

            /* Set the return value of this function equal to the Result*/
            /* of the function call.                                    */
            ret_val = Result;

            /* Now check to see if the command was submitted            */
            /* successfully.                                            */
            if(!Result)
            {
               /* The function was submitted successfully.              */
               printf("HFRM_Set_Network_Operator_Selection_Format: Function Successful.\r\n");
            }
            else
            {
               /* There was an error submitting the function.           */
               printf("HFRM_Set_Network_Operator_Selection_Format() Failure: %d (%s).\r\n", Result, ERR_ConvertErrorCodeToString(Result));
            }
         }
         else
         {
            /* The required parameter is invalid.                       */
            printf("Usage: SetNetworkOperator [BD_ADDR].\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("HFR Event Callback MUST be registered before making this call.\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for sending the command to  */
   /* Enable extended Error Reporting (+CMEE) on the Remote Audio       */
   /* Gateway Device.  This function returns zero on successful         */
   /* execution and a negative value on all errors.                     */
static int HFRMEnableExtendedErrorReporting(ParameterList_t *TempParam)
{
   int       ret_val;
   int       Result;
   BD_ADDR_t BD_ADDR;
   Boolean_t EnableExtendedErrorResults;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Verify that the HFR Event Callback has been registered.        */
      if(HFREventCallbackID)
      {
         /* The port ID appears to be at least semi-valid, now check to */
         /* make sure the passed in parameters appears to be semi-valid.*/
         if((TempParam) && (TempParam->NumberofParameters > 1))
         {
            /* Convert the parameter to a Bluetooth Device Address.     */
            StrToBD_ADDR(TempParam->Params[0].strParam, &BD_ADDR);

            /* Determine the selection type.                            */
            switch(TempParam->Params[1].intParam)
            {
               case 0:
                  EnableExtendedErrorResults = FALSE;
                  break;
               default:
                  EnableExtendedErrorResults = TRUE;
                  break;
            }

            /* Looks like we have a valid enable/disable flag also.  Now*/
            /* submit the command.                                      */
            Result = HFRM_Enable_Remote_Extended_Error_Result(HFREventCallbackID, BD_ADDR, EnableExtendedErrorResults);

            /* Set the return value of this function equal to the Result*/
            /* of the function call.                                    */
            ret_val = Result;

            /* Now check to see if the command was submitted            */
            /* successfully.                                            */
            if(!Result)
            {
               /* The function was submitted successfully.              */
               printf("HFRM_Enable_Remote_Extended_Error_Result: Function Successful.\r\n");
            }
            else
            {
               /* There was an error submitting the function.           */
               printf("HFRM_Enable_Remote_Extended_Error_Result() Failure: %d (%s).\r\n", Result, ERR_ConvertErrorCodeToString(Result));
            }
         }
         else
         {
            printf("Usage: EnableErrorReports [BD_ADDR] [FALSE = 0 | TRUE = 1]\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("HFR Event Callback MUST be registered before making this call.\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for sending the command to  */
   /* Request the Subscriber Number(s) from the Remote Audio Gateway    */
   /* Device.  This function returns zero on successful execution and a */
   /* negative value on all errors.                                     */
static int HFRMQuerySubscriberNumber(ParameterList_t *TempParam)
{
   int       ret_val;
   int       Result;
   BD_ADDR_t BD_ADDR;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Verify that the HFR Event Callback has been registered.        */
      if(HFREventCallbackID)
      {
         /* The port ID appears to be at least semi-valid, now check to */
         /* make sure the passed in parameters appears to be semi-valid.*/
         if((TempParam) && (TempParam->NumberofParameters > 0))
         {
            /* Convert the parameter to a Bluetooth Device Address.     */
            StrToBD_ADDR(TempParam->Params[0].strParam, &BD_ADDR);

            /* The Port ID appears to be is a semi-valid value.  Now    */
            /* submit the command.                                      */
            Result = HFRM_Query_Subscriber_Number_Information(HFREventCallbackID, BD_ADDR);

            /* Set the return value of this function equal to the Result*/
            /* of the function call.                                    */
            ret_val = Result;

            /* Now check to see if the command was submitted            */
            /* successfully.                                            */
            if(!Result)
            {
               /* The function was submitted successfully.              */
               printf("HFRM_Query_Subscriber_Number_Information: Function Successful.\r\n");
            }
            else
            {
               /* There was an error submitting the function.           */
               printf("HFRM_Query_Subscriber_Number_Information() Failure: %d (%s).\r\n", Result, ERR_ConvertErrorCodeToString(Result));
            }
         }
         else
         {
            printf("Usage: QuerySubscriberNumber [BD_ADDR]\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("HFR Event Callback MUST be registered before making this call.\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for sending the command to  */
   /* query the Response and Hold Status from the Remote Audio Gateway  */
   /* Device.  This function returns zero on successful execution and a */
   /* negative value on all errors.                                     */
static int HFRMQueryResponseAndHold(ParameterList_t *TempParam)
{
   int       Result;
   int       ret_val;
   BD_ADDR_t BD_ADDR;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Verify that the HFR Event Callback has been registered.        */
      if(HFREventCallbackID)
      {
         /* The port ID appears to be at least semi-valid, now check to */
         /* make sure the passed in parameters appears to be semi-valid.*/
         if((TempParam) && (TempParam->NumberofParameters > 0))
         {
            /* Convert the parameter to a Bluetooth Device Address.     */
            StrToBD_ADDR(TempParam->Params[0].strParam, &BD_ADDR);

            /* The Port ID appears to be is a semi-valid value.  Now    */
            /* submit the command.                                      */
            Result = HFRM_Query_Response_Hold_Status(HFREventCallbackID, BD_ADDR);

            /* Set the return value of this function equal to the Result*/
            /* of the function call.                                    */
            ret_val = Result;

            /* Now check to see if the command was submitted            */
            /* successfully.                                            */
            if(!Result)
            {
               /* The function was submitted successfully.              */
               printf("HFRM_Query_Response_Hold_Status: Function Successful.\r\n");
            }
            else
            {
               /* There was an error submitting the function.           */
               printf("HFRM_Query_Response_Hold_Status() Failure: %d (%s).\r\n", Result, ERR_ConvertErrorCodeToString(Result));
            }
         }
         else
         {
            printf("Usage: QueryResponseHoldStatus [BD_ADDR]\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("HFR Event Callback MUST be registered before making this call.\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for sending the Response and*/
   /* Hold Command (+BTRH) to the remote device.  This function returns */
   /* zero on successful execution and a negative value on all errors.  */
static int HFRMSendResponseAndHold(ParameterList_t *TempParam)
{
   int       Result;
   int       ret_val;
   int       CallState;
   BD_ADDR_t BD_ADDR;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Verify that the HFR Event Callback has been registered.        */
      if(HFREventCallbackID)
      {
         /* make sure the passed in parameters appears to be semi-valid.*/
         if((TempParam) && (TempParam->NumberofParameters > 1))
         {
            /* Convert the parameter to a Bluetooth Device Address.     */
            StrToBD_ADDR(TempParam->Params[0].strParam, &BD_ADDR);

            /* If we have a parameter, use it.                          */
            CallState = TempParam->Params[1].intParam;

            /* HF role needs to send the AT command version.            */
            Result = HFRM_Set_Incoming_Call_State(HFREventCallbackID, BD_ADDR, (HFRE_Call_State_t)CallState);

            /* Set the return value of this function equal to the Result*/
            /* of the function call.                                    */
            ret_val = Result;

            /* Now check to see if the command was submitted            */
            /* successfully.                                            */
            if(!Result)
            {
               /* The function was submitted successfully.              */
               printf("HFRM_Set_Incoming_Call_State: Function Successful.\r\n");
            }
            else
            {
               /* There was an error submitting the function.           */
               printf("HFRM_Set_Incoming_Call_State() Failure: %d (%s).\r\n", Result, ERR_ConvertErrorCodeToString(Result));
            }
         }
         else
         {
            printf("Usage: SendRespHold [BD_ADDR] [CallState].\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("HFR Event Callback MUST be registered before making this call.\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for sending the command to  */
   /* request the current calls list (+CLCC) from the Remote Audio      */
   /* Gateway Device.  This function returns zero on successful         */
   /* execution and a negative value on all errors.                     */
static int HFRMQueryRemoteCallList(ParameterList_t *TempParam)
{
   int       ret_val;
   int       Result;
   BD_ADDR_t BD_ADDR;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Verify that the HFR Event Callback has been registered.        */
      if(HFREventCallbackID)
      {
         /* make sure the passed in parameters appears to be semi-valid.*/
         if((TempParam) && (TempParam->NumberofParameters > 0))
         {
            /* Convert the parameter to a Bluetooth Device Address.     */
            StrToBD_ADDR(TempParam->Params[0].strParam, &BD_ADDR);

            /* The Port ID appears to be is a semi-valid value.  Now    */
            /* submit the command.                                      */
            Result = HFRM_Query_Remote_Current_Calls_List(HFREventCallbackID, BD_ADDR);

            /* Set the return value of this function equal to the Result*/
            /* of the function call.                                    */
            ret_val = Result;

            /* Now check to see if the command was submitted            */
            /* successfully.                                            */
            if(!Result)
            {
               /* The function was submitted successfully.              */
               printf("HFRM_Query_Remote_Current_Calls_List: Function Successful.\r\n");
            }
            else
            {
               /* There was an error submitting the function.           */
               printf("HFRM_Query_Remote_Current_Calls_List() Failure: %d (%s).\r\n", Result, ERR_ConvertErrorCodeToString(Result));
            }
         }
         else
         {
            /* One or more of the parameters are invalid.               */
            printf("Usage: QueryCallList [BD_ADDR].\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("HFR Event Callback MUST be registered before making this call.\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for sending an arbitrary    */
   /* command to the Remote Audio Gateway Device. This function returns */
   /* zero on successful execution and a negative value on all errors.  */
static int HFRMSendArbitraryCommand(ParameterList_t *TempParam)
{
   int          ret_val;
   int          Result;
   char        *Command;
   BD_ADDR_t    BD_ADDR;
   unsigned int CommandLength;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Verify that the HFR Event Callback has been registered.        */
      if(HFREventCallbackID)
      {
         /* Make sure the passed in parameters appears to be semi-valid.*/
         if((TempParam) && (TempParam->NumberofParameters >= 2) && (BTPS_StringLength(TempParam->Params[1].strParam) > 0))
         {
            /* Convert the parameter to a Bluetooth Device Address.     */
            StrToBD_ADDR(TempParam->Params[0].strParam, &BD_ADDR);

            /* We have already checked that the command string has      */
            /* non-zero length. Now, check that it is terminated in a   */
            /* carriage-return.                                         */
            CommandLength = BTPS_StringLength(TempParam->Params[1].strParam);
            if(TempParam->Params[1].strParam[CommandLength - 1] == '\r')
               Command = TempParam->Params[1].strParam;
            else
            {
               if((Command = (char *)BTPS_AllocateMemory(CommandLength+2)) != NULL)
               {
                  BTPS_StringCopy(Command, TempParam->Params[1].strParam);
                  Command[CommandLength]   = '\r';
                  Command[CommandLength+1] = '\0';
               }
            }

            if(Command)
            {
               /* The Port ID appears to be is a semi-valid value. Now  */
               /* submit the command.                                   */
               Result = HFRM_Send_Arbitrary_Command(HFREventCallbackID, BD_ADDR, Command);

               if(Command != TempParam->Params[1].strParam)
                  BTPS_FreeMemory(Command);

               /* Set the return value of this function equal to the    */
               /* Result of the function call.                          */
               ret_val = Result;

               /* Now check to see if the command was submitted         */
               /* successfully.                                         */
               if(!Result)
               {
                  /* The function was submitted successfully.           */
                  printf("HFRM_Send_Arbitrary_Command: Function Successful.\r\n");
               }
               else
               {
                  /* There was an error submitting the function.        */
                  printf("HFRM_Send_Arbitrary_Command: Failure: %d (%s).\r\n", Result, ERR_ConvertErrorCodeToString(Result));
               }
            }
            else
            {
               /* We were unable to obtain the command. The only way    */
               /* this could happen is if the user command was not      */
               /* suffixed with a CR and the buffer allocation failed.  */
               printf("Error allocating buffer space for command.\r\n");

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* One or more of the parameters are invalid.               */
            printf("Usage: SendArbitraryCommand [BD_ADDR] [AT Command String].\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("HFR Event Callback MUST be registered before making this call.\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for sending an arbitrary    */
   /* command to the Remote Audio Gateway Device. This function returns */
   /* zero on successful execution and a negative value on all errors.  */
static int HFRMQuerySCOConnectionHandle(ParameterList_t *TempParam)
{
   int       ret_val;
   int       Result;
   Word_t    SCOHandle;
   BD_ADDR_t BD_ADDR;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Verify that the HFR Event Callback has been registered.        */
      if(HFREventCallbackID)
      {
         /* make sure the passed in parameters appears to be semi-valid.*/
         if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].strParam))
         {
            /* Convert the parameter to a Bluetooth Device Address.     */
            StrToBD_ADDR(TempParam->Params[0].strParam, &BD_ADDR);

            /* The Port ID appears to be is a semi-valid value.  Now    */
            /* submit the command.                                      */
            Result = HFRM_Query_SCO_Connection_Handle(HFREventCallbackID, hctHandsFree, BD_ADDR, &SCOHandle);

            /* Set the return value of this function equal to the Result*/
            /* of the function call.                                    */
            ret_val = Result;

            /* Now check to see if the command was submitted            */
            /* successfully.                                            */
            if(!Result)
            {
               /* The function was submitted successfully.              */
               printf("HFRM_Query_SCO_Connection_Handle(): Function Successful. Handle: %u.\r\n", (unsigned int)SCOHandle);
            }
            else
            {
               /* There was an error submitting the function.           */
               printf("HFRM_Query_SCO_Connection_Handle() Failure: %d (%s).\r\n", Result, ERR_ConvertErrorCodeToString(Result));
            }
         }
         else
         {
            /* One or more of the parameters are invalid.               */
            printf("Usage: QuerySCOConnectionHandle [BD_ADDR].\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("HFR Control Event Callback MUST be registered before making this call.\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for registering with the HFR*/
   /* Manager to receive HFR Events.  This function returns zero if     */
   /* successful and a negative value if an error occurred.             */
static int HFRRegisterEventCallback(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* If there is an Event Callback Registered, then we need to flag */
      /* an error.                                                      */
      if(!HFREventCallbackID)
      {
         /* make sure the passed in parameters appears to be semi-valid.*/
         if((TempParam) && (TempParam->NumberofParameters > 0))
         {
            /* Callback has not been registered, go ahead and attempt to*/
            /* register it.                                             */
            Result = HFRM_Register_Event_Callback(hctHandsFree, (Boolean_t)TempParam->Params[0].intParam, HFRM_Event_Callback, NULL);
            if(Result > 0)
            {
               printf("HFRM_Register_Event_Callback() Success: %d.\r\n", Result);

               /* Note the Callback ID and flag success.                */
               HFREventCallbackID = (unsigned int)Result;

               ret_val            = 0;
            }
            else
            {
               /* Error registering the Callback, inform user and flag  */
               /* an error.                                             */
               printf("HFRM_Register_Event_Callback() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* One or more of the parameters are invalid.               */
            printf("Usage: HFRRegisterEventCallback [(0 = Non-Control Callback, 1 = Control Callback)].\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* Callback already registered, go ahead and notify the user.  */
         printf("HFR Manager Event Callback already registered.\r\n");

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for Un-registering a        */
   /* previously registered HFR Event Registration with the HFR Manager.*/
   /* This function returns zero if successful and a negative value if  */
   /* an error occurred.                                                */
static int HFRUnRegisterEventCallback(ParameterList_t *TempParam)
{
   int ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Next, check to make sure that there is an Event Callback       */
      /* already registered.                                            */
      if(HFREventCallbackID)
      {
         /* Callback has been registered, go ahead and attempt to       */
         /* un-register it.                                             */
         HFRM_Un_Register_Event_Callback(HFREventCallbackID);

         printf("HFRM_Un_Register_Event_Callback() Success.\r\n");

         /* Flag that there is no longer a Callback registered.         */
         HFREventCallbackID = 0;

         ret_val            = 0;
      }
      else
      {
         /* Callback NOT registered, go ahead and notify the user.      */
         printf("HFR Manager Event Callback is not registered.\r\n");

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for registering with the HFR*/
   /* Manager to receive HFR Data Events (and to allow the local client */
   /* the ability to send SCO audio data).  This function returns zero  */
   /* if successful and a negative value if an error occurred.          */
static int HFRRegisterDataCallback(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* If there is an Data Callback Registered, then we need to flag  */
      /* an error.                                                      */
      if(!HFRDataCallbackID)
      {
         /* Callback has not been registered, go ahead and attempt to   */
         /* register it.                                                */
         Result = HFRM_Register_Data_Event_Callback(hctHandsFree, HFRM_Event_Callback, NULL);
         if(Result > 0)
         {
            printf("HFRM_Register_Data_Event_Callback() Success: %d.\r\n", Result);

            /* Note the Callback ID and flag success.                   */
            HFRDataCallbackID = (unsigned int)Result;

            ret_val           = 0;
         }
         else
         {
            /* Error registering the Callback, inform user and flag an  */
            /* error.                                                   */
            printf("HFRM_Register_Data_Event_Callback() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* Callback already registered, go ahead and notify the user.  */
         printf("HFR Manager Data Callback already registered.\r\n");

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for Un-registering a        */
   /* previously registered HFR Data Event Registration with the HFR    */
   /* Manager.  This function returns zero if successful and a negative */
   /* value if an error occurred.                                       */
static int HFRUnRegisterDataCallback(ParameterList_t *TempParam)
{
   int ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Next, check to make sure that there is an Data Callback already*/
      /* registered.                                                    */
      if(HFRDataCallbackID)
      {
         /* Callback has been registered, go ahead and attempt to       */
         /* un-register it.                                             */
         HFRM_Un_Register_Data_Event_Callback(HFRDataCallbackID);

         printf("HFRM_Un_Register_Data_Event_Callback() Success.\r\n");

         /* Flag that there is no longer a Callback registered.         */
         HFRDataCallbackID = 0;

         ret_val           = 0;
      }
      else
      {
         /* Callback NOT registered, go ahead and notify the user.      */
         printf("HFR Manager Data Callback is not registered.\r\n");

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
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

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Next, check to make sure that a valid Data format was          */
      /* specified.                                                     */
      if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].intParam <= 2))
      {
         /* Parameters appear to be valid.  Go ahead and set the new    */
         /* test mode.                                                  */
         if(TempParam->Params[0].intParam == 2)
            AudioTestMode = atmLoopback;
         else
         {
            if(TempParam->Params[0].intParam == 1)
            {
               AudioTestMode      = atmTestTone;

               /* Reset the SCO Tone Index.                             */
               AudioDataToneIndex = 0;
            }
            else
               AudioTestMode = atmNone;
         }

         printf("SCO Test Mode Set to: %s.\r\n", (AudioTestMode == atmNone)?"None":((AudioTestMode == atmTestTone)?"1 KHz Tone":"Loopback"));

         /* Flag success to the caller.                                 */
         ret_val = 0;
      }
      else
      {
         /* Invalid parameters specified so flag an error to the user.  */
         printf("Usage: SetSCOTestMode [Mode (None = 0, 1KHz Tone = 1, Loopback = 2)].\r\n");

         /* Flag that invalid parameters were specified.                */
         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is a utility function that exists to       */
   /* display either the entire Local Device Property information (first*/
   /* parameter is zero) or portions that have changed.                 */
static void DisplayLocalDeviceProperties(unsigned long UpdateMask, DEVM_Local_Device_Properties_t *LocalDeviceProperties)
{
   char Buffer[64];

   if(LocalDeviceProperties)
   {
      /* First, display any information that is not part of any update  */
      /* mask.                                                          */
      if(!UpdateMask)
      {
         BD_ADDRToStr(LocalDeviceProperties->BD_ADDR, Buffer);

         printf("BD_ADDR:      %s\r\n", Buffer);
         printf("HCI Ver:      0x%04X\r\n", (Word_t)LocalDeviceProperties->HCIVersion);
         printf("HCI Rev:      0x%04X\r\n", (Word_t)LocalDeviceProperties->HCIRevision);
         printf("LMP Ver:      0x%04X\r\n", (Word_t)LocalDeviceProperties->LMPVersion);
         printf("LMP Sub Ver:  0x%04X\r\n", (Word_t)LocalDeviceProperties->LMPSubVersion);
         printf("Device Man:   0x%04X (%s)\r\n", (Word_t)LocalDeviceProperties->DeviceManufacturer, DEVM_ConvertManufacturerNameToString(LocalDeviceProperties->DeviceManufacturer));
         printf("Device Flags: 0x%08lX\r\n", LocalDeviceProperties->LocalDeviceFlags);
      }

      if((!UpdateMask) || (UpdateMask & DEVM_LOCAL_DEVICE_PROPERTIES_CHANGED_CLASS_OF_DEVICE))
         printf("COD:          0x%02X%02X%02X\r\n", LocalDeviceProperties->ClassOfDevice.Class_of_Device0, LocalDeviceProperties->ClassOfDevice.Class_of_Device1, LocalDeviceProperties->ClassOfDevice.Class_of_Device2);

      if((!UpdateMask) || (UpdateMask & DEVM_LOCAL_DEVICE_PROPERTIES_CHANGED_DEVICE_NAME))
         printf("Device Name:  %s\r\n", (LocalDeviceProperties->DeviceNameLength)?LocalDeviceProperties->DeviceName:"");

      if((!UpdateMask) || (UpdateMask & DEVM_LOCAL_DEVICE_PROPERTIES_CHANGED_DISCOVERABLE_MODE))
         printf("Disc. Mode:   %s, 0x%08X\r\n", LocalDeviceProperties->DiscoverableMode?"TRUE ":"FALSE", LocalDeviceProperties->DiscoverableModeTimeout);

      if((!UpdateMask) || (UpdateMask & DEVM_LOCAL_DEVICE_PROPERTIES_CHANGED_CONNECTABLE_MODE))
         printf("Conn. Mode:   %s, 0x%08X\r\n", LocalDeviceProperties->ConnectableMode?"TRUE ":"FALSE", LocalDeviceProperties->ConnectableModeTimeout);

      if((!UpdateMask) || (UpdateMask & DEVM_LOCAL_DEVICE_PROPERTIES_CHANGED_PAIRABLE_MODE))
         printf("Pair. Mode:   %s, 0x%08X\r\n", LocalDeviceProperties->PairableMode?"TRUE ":"FALSE", LocalDeviceProperties->PairableModeTimeout);
   }
}

   /* The following function is a utility function that exists to       */
   /* display either the entire Remote Device Property information      */
   /* (first parameter is zero) or portions that have changed.          */
static void DisplayRemoteDeviceProperties(unsigned long UpdateMask, DEVM_Remote_Device_Properties_t *RemoteDeviceProperties)
{
   char Buffer[64];

   if(RemoteDeviceProperties)
   {
      /* First, display any information that is not part of any update  */
      /* mask.                                                          */
      BD_ADDRToStr(RemoteDeviceProperties->BD_ADDR, Buffer);

      printf("BD_ADDR:       %s\r\n", Buffer);

      if((!UpdateMask) || (UpdateMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_CLASS_OF_DEVICE))
         printf("COD:           0x%02X%02X%02X\r\n", RemoteDeviceProperties->ClassOfDevice.Class_of_Device0, RemoteDeviceProperties->ClassOfDevice.Class_of_Device1, RemoteDeviceProperties->ClassOfDevice.Class_of_Device2);

      if((!UpdateMask) || (UpdateMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_DEVICE_NAME))
         printf("Device Name:   %s\r\n", (RemoteDeviceProperties->DeviceNameLength)?RemoteDeviceProperties->DeviceName:"");

      if((!UpdateMask) || (UpdateMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_DEVICE_FLAGS))
         printf("Device Flags:  0x%08lX\r\n", RemoteDeviceProperties->RemoteDeviceFlags);

      if((!UpdateMask) || (UpdateMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_RSSI))
         printf("RSSI:          %d\r\n", RemoteDeviceProperties->RSSI);

      if((!UpdateMask) && (RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_CURRENTLY_CONNECTED))
         printf("Trans. Power:  %d\r\n", RemoteDeviceProperties->TransmitPower);

      if((!UpdateMask) || ((UpdateMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_APPLICATION_DATA) && (RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_APPLICATION_DATA_VALID)))
      {
         printf("Friendly Name: %s\r\n", (RemoteDeviceProperties->ApplicationData.FriendlyNameLength)?RemoteDeviceProperties->ApplicationData.FriendlyName:"");

         printf("App. Info:   : %08lX\r\n", RemoteDeviceProperties->ApplicationData.ApplicationInfo);
      }

      if((!UpdateMask) || (UpdateMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_PAIRING_STATE))
         printf("Paired State : %s\r\n", (RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_CURRENTLY_PAIRED)?"TRUE":"FALSE");

      if((!UpdateMask) || (UpdateMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_CONNECTION_STATE))
         printf("Connect State: %s\r\n", (RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_CURRENTLY_CONNECTED)?"TRUE":"FALSE");

      if((!UpdateMask) || (UpdateMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_ENCRYPTION_STATE))
         printf("Encrypt State: %s\r\n", (RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_LINK_CURRENTLY_ENCRYPTED)?"TRUE":"FALSE");

      if((!UpdateMask) || (UpdateMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_SERVICES_STATE))
         printf("Serv. Known  : %s\r\n", (RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_SERVICES_KNOWN)?"TRUE":"FALSE");
   }
}

   /* The following function is used to extract a UUID value and convert*/
   /* it to a Word Value that represents the 16 Bit UUID value.         */
static Word_t ExtractUUID_ID(SDP_Data_Element_t *DataElementPtr)
{
   Word_t UUID = 0;

   /* Verify that the value passed in appears valid.                    */
   if(DataElementPtr)
   {
      /* Verify that this parameter is a UUID.                          */
      switch(DataElementPtr->SDP_Data_Element_Type)
      {
         case deUUID_128:
            UUID = (Word_t)((DataElementPtr->SDP_Data_Element.UUID_128.UUID_Byte2 << 8) + DataElementPtr->SDP_Data_Element.UUID_128.UUID_Byte3);
            break;
         case deUUID_32:
            UUID = (Word_t)((DataElementPtr->SDP_Data_Element.UUID_32.UUID_Byte2 << 8) + DataElementPtr->SDP_Data_Element.UUID_32.UUID_Byte3);
            break;
         case deUUID_16:
            UUID = (Word_t)((DataElementPtr->SDP_Data_Element.UUID_16.UUID_Byte0 << 8) + DataElementPtr->SDP_Data_Element.UUID_16.UUID_Byte1);
         default:
            break;
      }
   }

   return(UUID);
}

   /* The following function is used to scan the information that was   */
   /* extracted from an SDP Service Record and determine if this is a   */
   /* Hands Free Audio Gateway Service Record.                          */
static Boolean_t ParseAudioGatewayServiceClass(SDP_Record_t *SDPRecordPtr)
{
   Word_t       *ServiceClass;
   Boolean_t     ret_val = FALSE;
   unsigned int  Index1;

   /* Verify that the parameter passed in appears valid.                */
   if(SDPRecordPtr)
   {
      /* Scan through the Service Class UUIDs that we found.            */
      Index1       = SDPRecordPtr->NumServiceClass;
      ServiceClass = SDPRecordPtr->ServiceClass;

      while((Index1--) && (!ret_val))
      {
         /* Check for a Service Class match of the profile.             */
         if(*ServiceClass == HANDSFREE_PROFILE_AUDIO_GATEWAY_UUID)
            ret_val = TRUE;

         ServiceClass++;
      }
   }

   return(ret_val);
}

   /* The following function is used to process the information that has*/
   /* been extracted from one SDP Service Record.  The information will */
   /* be examined to determine if the record is a Hands Free profile    */
   /* record.  If a Hands Free record is located, the information is    */
   /* extracted from the SDP Record and placed into the appropriate     */
   /* fields of the Profile Information structure.                      */
static Boolean_t ProcessHandsFreeServiceRecord(SDP_Record_t *SDPRecordPtr, HFRE_Profile_Info_t *ProfileInfoPtr)
{
   Boolean_t ret_val = FALSE;

   /* Verify that the parameter passed in appears valid.                */
   if((SDPRecordPtr) && (ProfileInfoPtr))
   {
      /* Initialize the Profile Info Structure.                         */
      BTPS_MemInitialize(ProfileInfoPtr, 0, PROFILE_INFO_DATA_SIZE);

      /* Determine the Profile that is defined by this record.          */
      ret_val = ParseAudioGatewayServiceClass(SDPRecordPtr);
      if(ret_val)
      {
         /* Set the information that is common to all profiles.         */
         ProfileInfoPtr->ServiceDescLength     = SDPRecordPtr->ServiceDescLength;
         ProfileInfoPtr->ServiceDesc           = SDPRecordPtr->ServiceDesc;
         ProfileInfoPtr->ServiceNameLength     = SDPRecordPtr->ServiceNameLength;
         ProfileInfoPtr->ServiceName           = SDPRecordPtr->ServiceName;
         ProfileInfoPtr->ServiceProviderLength = SDPRecordPtr->ProviderNameLength;
         ProfileInfoPtr->ServiceProvider       = SDPRecordPtr->ProviderName;

         /* Verify that there are at lease 2 protocol entries and that  */
         /* the second protocol indicates RFCOMM.                       */
         if((SDPRecordPtr->NumProtocols >= 2) && (SDPRecordPtr->Protocol[1].UUID_ID == RFCOMM_PROTOCOL_UUID))
         {
            ProfileInfoPtr->HFREInfo.ServerChannel = (Byte_t)SDPRecordPtr->Protocol[1].Value;

            /* Verify that there are at lease 1 Profile entries.        */
            if((SDPRecordPtr->NumProfiles >= 1) && ((SDPRecordPtr->Profile[0].UUID_ID == HANDSFREE_PROFILE_HANDS_FREE_UUID) || (SDPRecordPtr->Profile[0].UUID_ID == HANDSFREE_PROFILE_AUDIO_GATEWAY_UUID)))
            {
               ProfileInfoPtr->HFREInfo.ProfileVersion = SDPRecordPtr->Profile[0].Value;

               /* Mask the Special Features bits and save to the Flags  */
               /* Value.                                                */
               ProfileInfoPtr->HFREInfo.SupportedFeatures = (Word_t)(SDPRecordPtr->SupportedFeatures);
            }
            else
               ret_val = FALSE;
         }
         else
            ret_val = FALSE;
      }
   }

   return(ret_val);
}

   /* The following function is used to process an SDP Service Record   */
   /* and determine if it is a Hands Free record and process the        */
   /* appropriate fields if it is.  This function returns TRUE if       */
   /* successful or FALSE otherwise.                                    */
static Boolean_t ParseHandsFreeSDPAttributeResponse(SDP_Service_Attribute_Response_Data_t *SDPServiceAttributeResponse, HFRE_Profile_Info_t *ProfileInfo)
{
   unsigned int        Index;
   unsigned int        Index1;
   Boolean_t           ret_val = FALSE;
   SDP_Record_t        SDPRecordInfo;
   SDP_Data_Element_t *DataElement1;
   SDP_Data_Element_t *DataElement2;
   SDP_Data_Element_t *DataElement3;

   /* Verify that the input parameters are semi-valid.                  */
   if((SDPServiceAttributeResponse) && (ProfileInfo))
   {
      /* First, check to make sure that there were Attributes returned. */
      if(SDPServiceAttributeResponse->Number_Attribute_Values)
      {
         /* Clear the SDP Record Info structure.                        */
         BTPS_MemInitialize(&SDPRecordInfo, 0, sizeof(SDP_Record_t));

         /* Loop through all returned SDP Attribute Values.             */
         for(Index1 = 0; Index1 < SDPServiceAttributeResponse->Number_Attribute_Values; Index1++)
         {
            /* Get a pointer to the first data element for the current  */
            /* attribute.                                               */
            DataElement1 = SDPServiceAttributeResponse->SDP_Service_Attribute_Value_Data[Index1].SDP_Data_Element;
            switch(SDPServiceAttributeResponse->SDP_Service_Attribute_Value_Data[Index1].Attribute_ID)
            {
               case SDP_ATTRIBUTE_ID_SERVICE_CLASS_ID_LIST:
                  /* Verify that the data element is what we expected.  */
                  if(DataElement1->SDP_Data_Element_Type == deSequence)
                  {
                     /* Get a pointer to the first data element and skip*/
                     /* over the L2CAP UUID element.                    */
                     DataElement2 = DataElement1->SDP_Data_Element.SDP_Data_Element_Sequence;
                     Index        = DataElement1->SDP_Data_Element_Length;
                     while((Index--) && (SDPRecordInfo.NumServiceClass < MAX_SDP_RECORD_ARRAY_LIST))
                     {
                        /* Extract a UUID_16_t from the Data Element;   */
                        SDPRecordInfo.ServiceClass[SDPRecordInfo.NumServiceClass] = ExtractUUID_ID(DataElement2);

                        /* Increment the number of Service Classes that */
                        /* have been processed.                         */
                        SDPRecordInfo.NumServiceClass++;

                        /* Advance to the next Data Element.            */
                        DataElement2++;
                     }
                  }
                  break;
               case SDP_ATTRIBUTE_ID_PROTOCOL_DESCRIPTOR_LIST:
                  /* Verify that the data element is what we expected.  */
                  if(DataElement1->SDP_Data_Element_Type == deSequence)
                  {
                     /* The Protocol Descriptor List is a sequence of   */
                     /* Data Element sequences, where the 1st sequence  */
                     /* identifies L2CAP and the registered L2CAP PSM.  */
                     DataElement2 = DataElement1->SDP_Data_Element.SDP_Data_Element_Sequence;
                     Index        = DataElement1->SDP_Data_Element_Length;

                     while((Index--) && (SDPRecordInfo.NumProtocols < MAX_SDP_RECORD_ARRAY_LIST))
                     {
                        /* Verify that this element is a Data Element   */
                        /* Sequence.                                    */
                        if(DataElement2->SDP_Data_Element_Type == deSequence)
                        {
                           /* Get the start of the next sequence.       */
                           DataElement3 = DataElement2->SDP_Data_Element.SDP_Data_Element_Sequence;

                           /* Extract a UUID_16_t from the Data Element */
                           SDPRecordInfo.Protocol[SDPRecordInfo.NumProtocols].UUID_ID = ExtractUUID_ID(DataElement3);

                           /* If the Length was greater than 1, then    */
                           /* there is an additional Byte or Word that  */
                           /* follows.                                  */
                           if(DataElement2->SDP_Data_Element_Length > 1)
                           {
                              /* Advance to the next element and get the*/
                              /* value that follows.                    */
                              DataElement3++;
                              if(SDPRecordInfo.NumProtocols < MAX_SDP_RECORD_ARRAY_LIST)
                              {
                                 if(DataElement3->SDP_Data_Element_Type == deUnsignedInteger2Bytes)
                                    SDPRecordInfo.Protocol[SDPRecordInfo.NumProtocols].Value = DataElement3->SDP_Data_Element.UnsignedInteger2Bytes;
                                 else
                                    SDPRecordInfo.Protocol[SDPRecordInfo.NumProtocols].Value = DataElement3->SDP_Data_Element.UnsignedInteger1Byte;
                              }
                           }

                           /* Increment the number of protocols that    */
                           /* have been processed.                      */
                           SDPRecordInfo.NumProtocols++;
                        }
                        DataElement2++;
                     }
                  }
                  break;
               case SDP_ATTRIBUTE_ID_ADDITIONAL_PROTOCOL_DESCRIPTOR_LISTS:
                  /* Verify that the data element is what we expected.  */
                  if(DataElement1->SDP_Data_Element_Type == deSequence)
                  {
                     /* The Additional Protocol Descriptor List is a    */
                     /* sequence of Additional Protocol Descriptor      */
                     /* Lists.  Since we are only going to process the  */
                     /* 1st list, then increment to the first list      */
                     /* sequence.                                       */
                     DataElement1 = DataElement1->SDP_Data_Element.SDP_Data_Element_Sequence;
                     if(DataElement1->SDP_Data_Element_Type == deSequence)
                     {
                        DataElement2 = DataElement1->SDP_Data_Element.SDP_Data_Element_Sequence;
                        Index        = DataElement1->SDP_Data_Element_Length;
                        while((Index--) && (SDPRecordInfo.NumAdditionalProtocols < MAX_SDP_RECORD_ARRAY_LIST))
                        {
                           /* Verify that this element is a Data Element*/
                           /* Sequence.                                 */
                           if(DataElement2->SDP_Data_Element_Type == deSequence)
                           {
                              /* Get the start of the next sequence.    */
                              DataElement3 = DataElement2->SDP_Data_Element.SDP_Data_Element_Sequence;

                              /* Extract a UUID_16_t from the Data      */
                              /* Element                                */
                              SDPRecordInfo.AdditionalProtocol[SDPRecordInfo.NumAdditionalProtocols].UUID_ID = ExtractUUID_ID(DataElement3);

                              /* If the Length was greater than 1, then */
                              /* there is an additional Byte or Word    */
                              /* that follows.                          */
                              if(DataElement2->SDP_Data_Element_Length > 1)
                              {
                                 /* Advance to the next element and get */
                                 /* the value that follows.             */
                                 DataElement3++;

                                 if(SDPRecordInfo.NumAdditionalProtocols < MAX_SDP_RECORD_ARRAY_LIST)
                                 {
                                    if(DataElement3->SDP_Data_Element_Type == deUnsignedInteger2Bytes)
                                       SDPRecordInfo.AdditionalProtocol[SDPRecordInfo.NumAdditionalProtocols].Value = DataElement3->SDP_Data_Element.UnsignedInteger2Bytes;
                                    else
                                       SDPRecordInfo.AdditionalProtocol[SDPRecordInfo.NumAdditionalProtocols].Value = DataElement3->SDP_Data_Element.UnsignedInteger1Byte;
                                 }
                              }

                              /* Increment the number of protocols that */
                              /* have been processed.                   */
                              SDPRecordInfo.NumAdditionalProtocols++;
                           }
                           DataElement2++;
                        }
                     }
                  }
                  break;
               case SDP_ATTRIBUTE_ID_BLUETOOTH_PROFILE_DESCRIPTOR_LIST:
                  /* Verify that the data element is what we expected.  */
                  if(DataElement1->SDP_Data_Element_Type == deSequence)
                  {
                     /* The Profile Descriptor List is a sequence of    */
                     /* Data Element sequences, where the 1st element in*/
                     /* each sequence identifies the Profile followed by*/
                     /* the Profile Version.                            */
                     DataElement2 = DataElement1->SDP_Data_Element.SDP_Data_Element_Sequence;
                     Index        = DataElement1->SDP_Data_Element_Length;
                     while((Index--) && (SDPRecordInfo.NumProfiles < MAX_SDP_RECORD_ARRAY_LIST))
                     {
                        /* Verify that this element is a Data Element   */
                        /* Sequence.                                    */
                        if(DataElement2->SDP_Data_Element_Type == deSequence)
                        {
                           /* Get the start of the next sequence.       */
                           DataElement3 = DataElement2->SDP_Data_Element.SDP_Data_Element_Sequence;

                           /* Extract a UUID_16_t from the Data Element */
                           SDPRecordInfo.Profile[SDPRecordInfo.NumProfiles].UUID_ID = ExtractUUID_ID(DataElement3);

                           /* Verify that the version information is    */
                           /* present.                                  */
                           if(DataElement2->SDP_Data_Element_Length > 1)
                           {
                              /* Advance to the next element and get the*/
                              /* value that follows.                    */
                              DataElement3++;

                              if(SDPRecordInfo.NumProfiles < MAX_SDP_RECORD_ARRAY_LIST)
                              {
                                 if(DataElement3->SDP_Data_Element_Type == deUnsignedInteger2Bytes)
                                    SDPRecordInfo.Profile[SDPRecordInfo.NumProfiles].Value = DataElement3->SDP_Data_Element.UnsignedInteger2Bytes;
                              }
                           }

                           /* Increment the number of profiles that have*/
                           /* been processed.                           */
                           SDPRecordInfo.NumProfiles++;
                        }
                        DataElement2++;
                     }
                  }
                  break;
               case (SDP_ATTRIBUTE_ID_PRIMARY_LANGUAGE_BASE_VALUE+SDP_ATTRIBUTE_OFFSET_ID_SERVICE_NAME):
                  /* Verify that the data element is what we expected.  */
                  if(DataElement1->SDP_Data_Element_Type == deTextString)
                  {
                     SDPRecordInfo.ServiceName       = DataElement1->SDP_Data_Element.TextString;
                     SDPRecordInfo.ServiceNameLength = (Word_t)DataElement1->SDP_Data_Element_Length;
                  }
                  break;
               case (SDP_ATTRIBUTE_ID_PRIMARY_LANGUAGE_BASE_VALUE+SDP_ATTRIBUTE_OFFSET_ID_SERVICE_DESCRIPTION):
                  /* Verify that the data element is what we expected.  */
                  if(DataElement1->SDP_Data_Element_Type == deTextString)
                  {
                     SDPRecordInfo.ServiceDesc       = DataElement1->SDP_Data_Element.TextString;
                     SDPRecordInfo.ServiceDescLength = (Word_t)DataElement1->SDP_Data_Element_Length;
                  }
                  break;
               case (SDP_ATTRIBUTE_ID_PRIMARY_LANGUAGE_BASE_VALUE+SDP_ATTRIBUTE_OFFSET_ID_PROVIDER_NAME):
                  /* Verify that the data element is what we expected.  */
                  if(DataElement1->SDP_Data_Element_Type == deTextString)
                  {
                     SDPRecordInfo.ProviderName       = DataElement1->SDP_Data_Element.TextString;
                     SDPRecordInfo.ProviderNameLength = (Word_t)DataElement1->SDP_Data_Element_Length;
                  }
                  break;
               case SDP_ATTRIBUTE_ID_SUPPORTED_FEATURES:
                  /* Verify that the data element is what we expected.  */
                  if(DataElement1->SDP_Data_Element_Type == deUnsignedInteger2Bytes)
                     SDPRecordInfo.SupportedFeatures = DataElement1->SDP_Data_Element.UnsignedInteger2Bytes;
                  break;
               default:
                  break;
            }
         }

         ret_val = ProcessHandsFreeServiceRecord(&SDPRecordInfo, ProfileInfo);
      }
   }

   return(ret_val);
}

   /* The following function is used to get the Hands Free Service      */
   /* information from a specified remote device.                       */
static int GetHandsFreeServiceInformation(BD_ADDR_t BD_ADDR, Boolean_t ForceUpdate, unsigned int *PortNumber)
{
   int                     ret_val;
   Boolean_t               ServiceLocated;
   unsigned int            TotalServiceSize;
   unsigned char          *ServiceData;
   HFRE_Profile_Info_t     HFREProfileInfo;
   DEVM_Parsed_SDP_Data_t  ParsedSDPData;

   /* Determine the total number of bytes that are stored for this      */
   if(!(ret_val = DEVM_QueryRemoteDeviceServices(BD_ADDR, ForceUpdate, 0, NULL, &TotalServiceSize)))
   {
      /* Determine if we are being asked to display the Service         */
      /* Information.                                                   */
      if(!ForceUpdate)
      {
         /* Determine if there is any service data for this device.     */
         if(TotalServiceSize)
         {
            /* Allocate memory to hold all of the service data.         */
            if((ServiceData = BTPS_AllocateMemory(TotalServiceSize)) != NULL)
            {
               /* Query the remote device services.                     */
               if((ret_val = DEVM_QueryRemoteDeviceServices(BD_ADDR, ForceUpdate, TotalServiceSize, ServiceData, &TotalServiceSize)) >= 0)
               {
                  /* Convert the raw SDP stream to a parsed SDP record. */
                  ret_val = DEVM_ConvertRawSDPStreamToParsedSDPData(ret_val, ServiceData, &ParsedSDPData);
                  if(!ret_val)
                  {
                     /* Determine if a Hands Free Audio gateway service */
                     /* is located on the remote device.                */
                     ServiceLocated = ParseHandsFreeData(&ParsedSDPData, &HFREProfileInfo);
                     if(ServiceLocated)
                     {
                        if((HFREProfileInfo.ServiceNameLength) && (HFREProfileInfo.ServiceNameLength < TotalServiceSize))
                        {
                           BTPS_MemCopy(ServiceData, HFREProfileInfo.ServiceName, HFREProfileInfo.ServiceNameLength);
                           ServiceData[HFREProfileInfo.ServiceNameLength] = '\0';
                           printf("Service Name: %s.\r\n", ServiceData);
                        }

                        if((HFREProfileInfo.ServiceProviderLength) && (HFREProfileInfo.ServiceProviderLength < TotalServiceSize))
                        {
                           BTPS_MemCopy(ServiceData, HFREProfileInfo.ServiceProvider, HFREProfileInfo.ServiceProviderLength);
                           ServiceData[HFREProfileInfo.ServiceProviderLength] = '\0';
                           printf("Service Provider: %s.\r\n", ServiceData);
                        }

                        if((HFREProfileInfo.ServiceDescLength) && (HFREProfileInfo.ServiceDescLength < TotalServiceSize))
                        {
                           BTPS_MemCopy(ServiceData, HFREProfileInfo.ServiceDesc, HFREProfileInfo.ServiceDescLength);
                           ServiceData[HFREProfileInfo.ServiceDescLength] = '\0';
                           printf("Service Description: %s.\r\n", ServiceData);
                        }

                        printf("RFCOMM Port Number: 0x%02X.\r\n", HFREProfileInfo.HFREInfo.ServerChannel);
                        printf("HFRE Profile Version: 0x%04X.\r\n", HFREProfileInfo.HFREInfo.ProfileVersion);
                        printf("HFRE Supported Features: 0x%04X.\r\n", HFREProfileInfo.HFREInfo.SupportedFeatures);

                        /* Return the port number if requested.         */
                        if(PortNumber)
                           *PortNumber = (unsigned int)HFREProfileInfo.HFREInfo.ServerChannel;
                     }
                     else
                        printf("Hands Free Audio Gateway Server not located on remote device.\r\n");

                     /* All finished with the parsed data, so free it.  */
                     DEVM_FreeParsedSDPData(&ParsedSDPData);
                  }
                  else
                  {
                     printf("DEVM_ConvertRawSDPStreamToParsedSDPData() Failure: %d, %s.\r\n", ret_val, ERR_ConvertErrorCodeToString(ret_val));

                     ret_val = FUNCTION_ERROR;
                  }
               }
               else
               {
                  /* Error attempting to query Services, inform the user*/
                  /* and flag an error.                                 */
                  printf("DEVM_QueryRemoteDeviceServices() Failure: %d, %s.\r\n", ret_val, ERR_ConvertErrorCodeToString(ret_val));

                  ret_val = FUNCTION_ERROR;
               }

               /* Free the previously allocated memory.                 */
               BTPS_FreeMemory(ServiceData);
            }
            else
            {
               printf("Failed to allocate memory for the service data for %u bytes.\r\n", TotalServiceSize);

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            printf("Total Service Size is 0.\r\n");

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         printf("Querying services for remote device.\r\n");

         ret_val = 0;
      }
   }
   else
   {
      /* Error attempting to query Services, inform the user and flag an*/
      /* error.                                                         */
      printf("DEVM_QueryRemoteDeviceServices() Failure: %d, %s.\r\n", ret_val, ERR_ConvertErrorCodeToString(ret_val));

      ret_val = FUNCTION_ERROR;
   }

   return(ret_val);
}

   /* The following function is used to parse a SDP data stream into a  */
   /* Hands Free Audio profile information structure structure.  This   */
   /* function returns TRUE if a Hands Free record was parsed from the  */
   /* SDP data stream or FALSE otherwise                                */
static Boolean_t ParseHandsFreeData(DEVM_Parsed_SDP_Data_t *ParsedSDPData, HFRE_Profile_Info_t *ProfileInfo)
{
   Boolean_t    ret_val = FALSE;
   unsigned int Index;

   /* Verify that the input parameters are semi-valid.                  */
   if((ParsedSDPData) && (ProfileInfo))
   {
      /* Loop through all returned SDP Service Records.                 */
      for(Index=0;(Index<ParsedSDPData->NumberServiceRecords)&&(!ret_val);Index++)
      {
         /* Call Display SDPAttributeResponse for all SDP Service       */
         /* Records received.                                           */
         ret_val = ParseHandsFreeSDPAttributeResponse(&(ParsedSDPData->SDPServiceAttributeResponseData[Index]), ProfileInfo);
      }
   }

   return(ret_val);
}

   /* The following function is responsible for displaying the contents */
   /* of Parsed Remote Device Services Data to the display.             */
static void DisplayParsedServiceData(DEVM_Parsed_SDP_Data_t *ParsedSDPData)
{
   unsigned int Index;

   /* First, check to see if Service Records were returned.             */
   if((ParsedSDPData) && (ParsedSDPData->NumberServiceRecords))
   {
      /* Loop through all returned SDP Service Records.                 */
      for(Index=0; Index<ParsedSDPData->NumberServiceRecords; Index++)
      {
         /* First display the number of SDP Service Records we are      */
         /* currently processing.                                       */
         printf("Service Record: %u:\r\n", (Index + 1));

         /* Call Display SDPAttributeResponse for all SDP Service       */
         /* Records received.                                           */
         DisplaySDPAttributeResponse(&(ParsedSDPData->SDPServiceAttributeResponseData[Index]), 1);
      }
   }
   else
      printf("No SDP Service Records Found.\r\n");
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
         printf("%*s Type: Unsigned Int = 0x%02X%02X%02X%02X%02X%02X%02X%02X\r\n", (Level*INDENT_LENGTH), "",
                                                                                 SDPDataElement->SDP_Data_Element.UnsignedInteger8Bytes[7],
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
         printf("%*s Type: Unsigned Int = 0x%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X\r\n", (Level*INDENT_LENGTH), "",
                                                                                                                 SDPDataElement->SDP_Data_Element.UnsignedInteger16Bytes[15],
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
         printf("%*s Type: Signed Int = 0x%02X%02X%02X%02X%02X%02X%02X%02X\r\n", (Level*INDENT_LENGTH), "",
                                                                               SDPDataElement->SDP_Data_Element.SignedInteger8Bytes[7],
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
         printf("%*s Type: Signed Int = 0x%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X\r\n", (Level*INDENT_LENGTH), "",
                                                                                                               SDPDataElement->SDP_Data_Element.SignedInteger16Bytes[15],
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
         printf("%*s Type: UUID_16 = 0x%02X%02X\r\n", (Level*INDENT_LENGTH), "",
                                                    SDPDataElement->SDP_Data_Element.UUID_16.UUID_Byte0,
                                                    SDPDataElement->SDP_Data_Element.UUID_16.UUID_Byte1);
         break;
      case deUUID_32:
         printf("%*s Type: UUID_32 = 0x%02X%02X%02X%02X\r\n", (Level*INDENT_LENGTH), "",
                                                            SDPDataElement->SDP_Data_Element.UUID_32.UUID_Byte0,
                                                            SDPDataElement->SDP_Data_Element.UUID_32.UUID_Byte1,
                                                            SDPDataElement->SDP_Data_Element.UUID_32.UUID_Byte2,
                                                            SDPDataElement->SDP_Data_Element.UUID_32.UUID_Byte3);
         break;
      case deUUID_128:
         printf("%*s Type: UUID_128 = 0x%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X\r\n", (Level*INDENT_LENGTH), "",
                                                                                                             SDPDataElement->SDP_Data_Element.UUID_128.UUID_Byte0,
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

   /* The following function is the Callback function that is installed */
   /* to be notified when any local IPC connection to the server has    */
   /* been lost.  This case only occurs when the Server exits.  This    */
   /* callback allows the application mechanism to be notified so that  */
   /* all resources can be cleaned up (i.e.  call BTPM_Cleanup().       */
void BTPSAPI ServerUnRegistrationCallback(void *CallbackParameter)
{
   printf("Server has been Un-Registered, Everything must be re-initialized.\r\n");

   printf("Hands Free>");

   /* Clean up everything.                                              */
   Cleanup(NULL);

   /* Make sure the output is displayed to the user.                    */
   fflush(stdout);
}

   /* The following function is the Device Manager Event Callback       */
   /* function that is Registered with the Device Manager.  This        */
   /* callback is responsible for processing all Device Manager Events. */
static void BTPSAPI DEVM_Event_Callback(DEVM_Event_Data_t *EventData, void *CallbackParameter)
{
   char Buffer[32];

   if(EventData)
   {
      printf("\r\n");

      switch(EventData->EventType)
      {
         case detDevicePoweredOn:
            printf("Device Powered On.\r\n");
            break;
         case detDevicePoweringOff:
            printf("Device Powering Off Event, Timeout: 0x%08X.\r\n", EventData->EventData.DevicePoweringOffEventData.PoweringOffTimeout);
            break;
         case detDevicePoweredOff:
            printf("Device Powered Off.\r\n");
            break;
         case detLocalDevicePropertiesChanged:
            printf("Local Device Properties Changed.\r\n");

            DisplayLocalDeviceProperties(EventData->EventData.LocalDevicePropertiesChangedEventData.ChangedMemberMask, &(EventData->EventData.LocalDevicePropertiesChangedEventData.LocalDeviceProperties));
            break;
         case detDeviceDiscoveryStarted:
            printf("Device Discovery Started.\r\n");
            break;
         case detDeviceDiscoveryStopped:
            printf("Device Discovery Stopped.\r\n");
            break;
         case detRemoteDeviceFound:
            printf("Remote Device Found.\r\n");

            DisplayRemoteDeviceProperties(0, &(EventData->EventData.RemoteDeviceFoundEventData.RemoteDeviceProperties));
            break;
         case detRemoteDeviceDeleted:
            BD_ADDRToStr(EventData->EventData.RemoteDeviceDeletedEventData.RemoteDeviceAddress, Buffer);

            printf("Remote Device Deleted: %s.\r\n", Buffer);
            break;
         case detRemoteDevicePropertiesChanged:
            printf("Remote Device Properties Changed.\r\n");

            DisplayRemoteDeviceProperties(EventData->EventData.RemoteDevicePropertiesChangedEventData.ChangedMemberMask, &(EventData->EventData.RemoteDevicePropertiesChangedEventData.RemoteDeviceProperties));
            break;
         case detRemoteDevicePropertiesStatus:
            BD_ADDRToStr(EventData->EventData.RemoteDevicePropertiesStatusEventData.RemoteDeviceProperties.BD_ADDR, Buffer);

            printf("Remote Device Properties Status: %s, %s.\r\n", Buffer, EventData->EventData.RemoteDevicePropertiesStatusEventData.Success?"SUCCESS":"FAILURE");

            DisplayRemoteDeviceProperties(0, &(EventData->EventData.RemoteDevicePropertiesStatusEventData.RemoteDeviceProperties));
            break;
         case detRemoteDeviceServicesStatus:
            BD_ADDRToStr(EventData->EventData.RemoteDeviceServicesStatusEventData.RemoteDeviceAddress, Buffer);

            printf("Remote Device %s Services Status: %s, %s.\r\n", Buffer, (EventData->EventData.RemoteDeviceServicesStatusEventData.StatusFlags & DEVM_REMOTE_DEVICE_SERVICES_STATUS_FLAGS_LOW_ENERGY)?"LE":"BR/EDR", (EventData->EventData.RemoteDeviceServicesStatusEventData.StatusFlags & DEVM_REMOTE_DEVICE_SERVICES_STATUS_FLAGS_SUCCESS)?"SUCCESS":"FAILURE");
            break;
         case detRemoteDevicePairingStatus:
            BD_ADDRToStr(EventData->EventData.RemoteDevicePairingStatusEventData.RemoteDeviceAddress, Buffer);

            printf("Remote Device Pairing Status: %s, %s (0x%02X)\r\n", Buffer, (EventData->EventData.RemoteDevicePairingStatusEventData.Success)?"SUCCESS":"FAILURE", EventData->EventData.RemoteDevicePairingStatusEventData.AuthenticationStatus);
            break;
         case detRemoteDeviceAuthenticationStatus:
            BD_ADDRToStr(EventData->EventData.RemoteDeviceAuthenticationStatusEventData.RemoteDeviceAddress, Buffer);

            printf("Remote Device Authentication Status: %s, %d (%s)\r\n", Buffer, EventData->EventData.RemoteDeviceAuthenticationStatusEventData.Status, (EventData->EventData.RemoteDeviceAuthenticationStatusEventData.Status)?ERR_ConvertErrorCodeToString(EventData->EventData.RemoteDeviceAuthenticationStatusEventData.Status):"SUCCESS");
            break;
         case detRemoteDeviceEncryptionStatus:
            BD_ADDRToStr(EventData->EventData.RemoteDeviceEncryptionStatusEventData.RemoteDeviceAddress, Buffer);

            printf("Remote Device Encryption Status: %s, %d (%s)\r\n", Buffer, EventData->EventData.RemoteDeviceEncryptionStatusEventData.Status, (EventData->EventData.RemoteDeviceEncryptionStatusEventData.Status)?ERR_ConvertErrorCodeToString(EventData->EventData.RemoteDeviceEncryptionStatusEventData.Status):"SUCCESS");
            break;
         case detRemoteDeviceConnectionStatus:
            BD_ADDRToStr(EventData->EventData.RemoteDeviceConnectionStatusEventData.RemoteDeviceAddress, Buffer);

            printf("Remote Device Connection Status: %s, %d (%s)\r\n", Buffer, EventData->EventData.RemoteDeviceConnectionStatusEventData.Status, (EventData->EventData.RemoteDeviceConnectionStatusEventData.Status)?ERR_ConvertErrorCodeToString(EventData->EventData.RemoteDeviceConnectionStatusEventData.Status):"SUCCESS");
            break;
         default:
            printf("Unknown Device Manager Event Received: 0x%08X, Length: 0x%08X.\r\n", (unsigned int)EventData->EventType, EventData->EventLength);
            break;
      }
   }
   else
      printf("\r\nDEVM Event Data is NULL.\r\n");

   printf("Hands Free>");

   /* Make sure the output is displayed to the user.                    */
   fflush(stdout);
}

   /* The following function is the Device Manager Authentication Event */
   /* Callback function that is Registered with the Device Manager.     */
   /* This callback is responsible for processing all Device Manager    */
   /* Authentication Request Events.                                    */
static void BTPSAPI DEVM_Authentication_Callback(DEVM_Authentication_Information_t *AuthenticationRequestInformation, void *CallbackParameter)
{
   int                               Result;
   char                              Buffer[32];
   DEVM_Authentication_Information_t AuthenticationResponseInformation;

   if(AuthenticationRequestInformation)
   {
      printf("\r\n");

      BD_ADDRToStr(AuthenticationRequestInformation->BD_ADDR, Buffer);

      printf("Authentication Request received for %s.\r\n", Buffer);

      switch(AuthenticationRequestInformation->AuthenticationAction)
      {
         case DEVM_AUTHENTICATION_ACTION_PIN_CODE_REQUEST:
            printf("PIN Code Request.\r\n.");

            /* Note the current Remote BD_ADDR that is requesting the   */
            /* PIN Code.                                                */
            CurrentRemoteBD_ADDR = AuthenticationRequestInformation->BD_ADDR;

            /* Inform the user that they will need to respond with a PIN*/
            /* Code Response.                                           */
            printf("\r\nRespond with the command: PINCodeResponse\r\n");
            break;
         case DEVM_AUTHENTICATION_ACTION_USER_CONFIRMATION_REQUEST:
            printf("User Confirmation Request.\r\n");

            /* Note the current Remote BD_ADDR that is requesting the   */
            /* User Confirmation.                                       */
            CurrentRemoteBD_ADDR = AuthenticationRequestInformation->BD_ADDR;

            if(IOCapability != icDisplayYesNo)
            {
               /* Invoke Just works.                                    */

               printf("\r\nAuto Accepting: %lu\r\n", (unsigned long)AuthenticationRequestInformation->AuthenticationData.Passkey);

               BTPS_MemInitialize(&AuthenticationResponseInformation, 0, sizeof(AuthenticationResponseInformation));

               AuthenticationResponseInformation.BD_ADDR                         = AuthenticationRequestInformation->BD_ADDR;
               AuthenticationResponseInformation.AuthenticationAction            = DEVM_AUTHENTICATION_ACTION_USER_CONFIRMATION_RESPONSE;
               AuthenticationResponseInformation.AuthenticationDataLength        = sizeof(AuthenticationResponseInformation.AuthenticationData.Confirmation);

               AuthenticationResponseInformation.AuthenticationData.Confirmation = (Boolean_t)TRUE;

               if((Result = DEVM_AuthenticationResponse(AuthenticationCallbackID, &AuthenticationResponseInformation)) >= 0)
                  printf("DEVM_AuthenticationResponse() Success.\r\n");
               else
                  printf("DEVM_AuthenticationResponse() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

               /* Flag that there is no longer a current Authentication */
               /* procedure in progress.                                */
               ASSIGN_BD_ADDR(CurrentRemoteBD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
            }
            else
            {
               printf("User Confirmation: %lu\r\n", (unsigned long)AuthenticationRequestInformation->AuthenticationData.Passkey);

               /* Inform the user that they will need to respond with a */
               /* PIN Code Response.                                    */
               printf("\r\nRespond with the command: UserConfirmationResponse\r\n");
            }
            break;
         case DEVM_AUTHENTICATION_ACTION_PASSKEY_REQUEST:
            printf("Passkey Request.\r\n");

            /* Note the current Remote BD_ADDR that is requesting the   */
            /* Passkey.                                                 */
            CurrentRemoteBD_ADDR = AuthenticationRequestInformation->BD_ADDR;

            /* Inform the user that they will need to respond with a    */
            /* Passkey Response.                                        */
            printf("\r\nRespond with the command: PassKeyResponse\r\n");
            break;
         case DEVM_AUTHENTICATION_ACTION_PASSKEY_INDICATION:
            printf("PassKey Indication.\r\n");

            printf("PassKey: %lu\r\n", (unsigned long)AuthenticationRequestInformation->AuthenticationData.Passkey);
            break;
         case DEVM_AUTHENTICATION_ACTION_KEYPRESS_INDICATION:
            printf("Keypress Indication.\r\n");

            printf("Keypress: %d\r\n", (int)AuthenticationRequestInformation->AuthenticationData.Keypress);
            break;
         case DEVM_AUTHENTICATION_ACTION_OUT_OF_BAND_DATA_REQUEST:
            printf("Out of Band Data Request.\r\n");

            /* This application does not support OOB data so respond    */
            /* with a data length of Zero to force a negative reply.    */
            BTPS_MemInitialize(&AuthenticationResponseInformation, 0, sizeof(AuthenticationResponseInformation));

            AuthenticationResponseInformation.BD_ADDR                  = AuthenticationRequestInformation->BD_ADDR;
            AuthenticationResponseInformation.AuthenticationAction     = DEVM_AUTHENTICATION_ACTION_OUT_OF_BAND_DATA_RESPONSE;
            AuthenticationResponseInformation.AuthenticationDataLength = 0;

            if((Result = DEVM_AuthenticationResponse(AuthenticationCallbackID, &AuthenticationResponseInformation)) >= 0)
               printf("DEVM_AuthenticationResponse() Success.\r\n");
            else
               printf("DEVM_AuthenticationResponse() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));
            break;
         case DEVM_AUTHENTICATION_ACTION_IO_CAPABILITIES_REQUEST:
            printf("I/O Capability Request.\r\n");

            /* Respond with the currently configured I/O Capabilities.  */
            BTPS_MemInitialize(&AuthenticationResponseInformation, 0, sizeof(AuthenticationResponseInformation));

            AuthenticationResponseInformation.BD_ADDR                                                    = AuthenticationRequestInformation->BD_ADDR;
            AuthenticationResponseInformation.AuthenticationAction                                       = DEVM_AUTHENTICATION_ACTION_IO_CAPABILITIES_RESPONSE;
            AuthenticationResponseInformation.AuthenticationDataLength                                   = sizeof(AuthenticationResponseInformation.AuthenticationData.IOCapabilities);

            AuthenticationResponseInformation.AuthenticationData.IOCapabilities.IO_Capability            = (GAP_IO_Capability_t)IOCapability;
            AuthenticationResponseInformation.AuthenticationData.IOCapabilities.MITM_Protection_Required = MITMProtection;
            AuthenticationResponseInformation.AuthenticationData.IOCapabilities.OOB_Data_Present         = OOBSupport;

            if((Result = DEVM_AuthenticationResponse(AuthenticationCallbackID, &AuthenticationResponseInformation)) >= 0)
               printf("DEVM_AuthenticationResponse() Success.\r\n");
            else
               printf("DEVM_AuthenticationResponse() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));
            break;
         case DEVM_AUTHENTICATION_ACTION_IO_CAPABILITIES_RESPONSE:
            printf("I/O Capability Response.\r\n");

            /* Inform the user of the Remote I/O Capablities.           */
            printf("Remote I/O Capabilities: %s, MITM Protection: %s.\r\n", IOCapabilitiesStrings[AuthenticationRequestInformation->AuthenticationData.IOCapabilities.IO_Capability], AuthenticationRequestInformation->AuthenticationData.IOCapabilities.MITM_Protection_Required?"TRUE":"FALSE");
            break;
         case DEVM_AUTHENTICATION_ACTION_AUTHENTICATION_STATUS_RESULT:
            printf("Authentication Status.\r\n");

            printf("Status: %d\r\n", AuthenticationRequestInformation->AuthenticationData.AuthenticationStatus);
            break;
         case DEVM_AUTHENTICATION_ACTION_AUTHENTICATION_START:
            printf("Authentication Start.\r\n");
            break;
         case DEVM_AUTHENTICATION_ACTION_AUTHENTICATION_END:
            printf("Authentication End.\r\n");
            break;
         default:
            printf("Unknown Device Manager Authentication Event Received: 0x%08X, Length: 0x%08X.\r\n", (unsigned int)AuthenticationRequestInformation->AuthenticationAction, AuthenticationRequestInformation->AuthenticationDataLength);
            break;
      }
   }
   else
      printf("\r\nDEVM Authentication Request Data is NULL.\r\n");

   printf("Hands Free>");

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
static void BTPSAPI HFRM_Event_Callback(HFRM_Event_Data_t *EventData, void *CallbackParameter)
{
   char                BoardStr[13];
   unsigned int        Index;
   ConnectionTypeStr_t ConnectionStr;
   Byte_t              AudioData[256];

   /* First, check to see if the required parameters appear to be       */
   /* semi-valid.                                                       */
   if(EventData != NULL)
   {
      if(EventData->EventType != hetHFRAudioData)
         printf("\r\n");

      /* The parameters appear to be semi-valid, now check to see what  */
      /* type the incoming event is.                                    */
      switch(EventData->EventType)
      {
         case hetHFRIncomingConnectionRequest:
            /* A Client is attempting to connect to the Server, display */
            /* the BD_ADDR of the connecting device.                    */
            BD_ADDRToStr(EventData->EventData.IncomingConnectionRequestEventData.RemoteDeviceAddress, BoardStr);
            ConnectionTypeToStr(EventData->EventData.IncomingConnectionRequestEventData.ConnectionType, ConnectionStr);

            printf("hetHFRIncomingConnectionRequest, BD_ADDR: %s, Type: %s.\r\n", BoardStr, ConnectionStr);
            printf("Respond with the command: ConnectionRequestResponse\r\n");
            break;
         case hetHFRConnected:
            /* A Client has connected to the Server, display the BD_ADDR*/
            /* of the connecting device.                                */
            BD_ADDRToStr(EventData->EventData.ConnectedEventData.RemoteDeviceAddress, BoardStr);
            ConnectionTypeToStr(EventData->EventData.ConnectedEventData.ConnectionType, ConnectionStr);
            printf("hetHFRConnected, BD_ADDR: %s, Type: %s.\r\n", BoardStr, ConnectionStr);
            break;
         case hetHFRDisconnected:
            /* A Client has connected to the Server, display the BD_ADDR*/
            /* of the connecting device.                                */
            BD_ADDRToStr(EventData->EventData.DisconnectedEventData.RemoteDeviceAddress, BoardStr);
            ConnectionTypeToStr(EventData->EventData.DisconnectedEventData.ConnectionType, ConnectionStr);
            printf("hetHFRDisconnected, BD_ADDR: %s, Type: %s, Reason: %u.\r\n", BoardStr, ConnectionStr, EventData->EventData.DisconnectedEventData.DisconnectReason);
            break;
         case hetHFRConnectionStatus:
            /* The Client received a Connect Confirmation, display      */
            /* relevant information.                                    */
            BD_ADDRToStr(EventData->EventData.ConnectionStatusEventData.RemoteDeviceAddress, BoardStr);
            ConnectionTypeToStr(EventData->EventData.ConnectionStatusEventData.ConnectionType, ConnectionStr);
            printf("hetHFRConnectionStatus, BD_ADDR: %s, Type: %s, Status: %u.\r\n", BoardStr, ConnectionStr, EventData->EventData.ConnectionStatusEventData.ConnectionStatus);
            break;
         case hetHFRServiceLevelConnectionEstablished:
            BD_ADDRToStr(EventData->EventData.ServiceLevelConnectionEstablishedEventData.RemoteDeviceAddress, BoardStr);
            ConnectionTypeToStr(EventData->EventData.ServiceLevelConnectionEstablishedEventData.ConnectionType, ConnectionStr);

            /* A Open Service Level Indication was received, display    */
            /* relevant information.                                    */
            printf("hetHFRServiceLevelConnectionEstablished, BD_ADDR: %s, Type: %s, RemoteSupportedFeaturesValid: %s\n",
                                                             BoardStr,
                                                             ConnectionStr,
                                                             (EventData->EventData.ServiceLevelConnectionEstablishedEventData.RemoteSupportedFeaturesValid)?"TRUE":"FALSE");
            printf("RemoteSupportedFeatures:\n   ");

            for(Index=0;Index<NUM_AUDIO_GATEWAY_FEATURES;Index++)
            {
               if(EventData->EventData.ServiceLevelConnectionEstablishedEventData.RemoteSupportedFeatures & AudioGatewayFeatures[Index].Mask)
                  printf("%s ", AudioGatewayFeatures[Index].String);
            }

            printf("\nRemoteCallHoldMultipartySupport:\n   ");
            for(Index=0;Index<NUM_MULTIPARTY_FEATURES;Index++)
            {
               if(EventData->EventData.ServiceLevelConnectionEstablishedEventData.RemoteCallHoldMultipartySupport & MultipartyFeatures[Index].Mask)
                  printf("%s ", MultipartyFeatures[Index].String);
            }

            printf("\n");

            break;
         case hetHFRAudioConnected:
            /* A Client has connected to the Server, display the BD_ADDR*/
            /* of the connecting device.                                */
            BD_ADDRToStr(EventData->EventData.AudioConnectedEventData.RemoteDeviceAddress, BoardStr);
            ConnectionTypeToStr(EventData->EventData.AudioConnectedEventData.ConnectionType, ConnectionStr);
            printf("hetHFRAudioConnected, BD_ADDR: %s, Type: %s.\r\n", BoardStr, ConnectionStr);

            /* Flag that we would like the Audio Data Indication message*/
            /* to be displayed on the first reception of Audio Data.    */
            AudioDataCount = 1;
            break;
         case hetHFRAudioDisconnected:
            /* A Client has connected to the Server, display the BD_ADDR*/
            /* of the connecting device.                                */
            BD_ADDRToStr(EventData->EventData.AudioDisconnectedEventData.RemoteDeviceAddress, BoardStr);
            ConnectionTypeToStr(EventData->EventData.AudioDisconnectedEventData.ConnectionType, ConnectionStr);
            printf("hetHFRAudioDisconnected, BD_ADDR: %s, Type: %s\r\n", BoardStr, ConnectionStr);

            /* Flag that we would like the Audio Data Indication message*/
            /* to be displayed on the first reception of Audio Data.    */
            AudioDataCount = 0;
            break;
         case hetHFRAudioConnectionStatus:
            /* The Client received a Connect Confirmation, display      */
            /* relevant information.                                    */
            BD_ADDRToStr(EventData->EventData.AudioConnectionStatusEventData.RemoteDeviceAddress, BoardStr);
            ConnectionTypeToStr(EventData->EventData.AudioConnectionStatusEventData.ConnectionType, ConnectionStr);
            printf("hetHFRAudioConnectionStatus, BD_ADDR: %s, Type: %s, Successfull: %s.\r\n", BoardStr, ConnectionStr, (EventData->EventData.AudioConnectionStatusEventData.Successful?"YES":"NO"));
            break;
         case hetHFRAudioData:
            /* To avoid flooding the user with Audio Data Messages, only*/
            /* print out a message every second.  The number below is   */
            /* loosely based on the fact that the Bluetooth             */
            /* Specification mentions 3ms per SCO Packet, which is what */
            /* most chips appear to use.  So we arrive at the 'magical' */
            /* number below.                                            */
            AudioDataCount--;
            if(!AudioDataCount)
            {
               /* An Audio Data Indication was received, display the    */
               /* relevant information.                                 */
               BD_ADDRToStr(EventData->EventData.AudioDataEventData.RemoteDeviceAddress, BoardStr);
               ConnectionTypeToStr(EventData->EventData.AudioDataEventData.ConnectionType, ConnectionStr);
               printf("\r\n");
               printf("hetHFRAudioData, BD_ADDR: %s, Type: %s, Length 0x%04X: %02X%02X%02X%02X.\r\n", BoardStr, ConnectionStr, EventData->EventData.AudioDataEventData.AudioDataLength, EventData->EventData.AudioDataEventData.AudioData[0], EventData->EventData.AudioDataEventData.AudioData[1], EventData->EventData.AudioDataEventData.AudioData[2], EventData->EventData.AudioDataEventData.AudioData[3]);

               /* Check if a the packet status flag is set.             */
               if(EventData->EventData.AudioDataEventData.AudioDataFlags & HFRM_AUDIO_DATA_FLAGS_PACKET_STATUS_MASK)
               {
                  /* A packet status flag is set, determine which flag  */
                  /* it is.                                             */
                  switch(EventData->EventData.AudioDataEventData.AudioDataFlags & HFRM_AUDIO_DATA_FLAGS_PACKET_STATUS_MASK)
                  {
                     case HFRM_AUDIO_DATA_FLAGS_PACKET_STATUS_POSSIBLY_INVALID_DATA:
                        printf("Packet Status: Possibly Invalid Data\r\n");
                        break;
                     case HFRM_AUDIO_DATA_FLAGS_PACKET_STATUS_NO_DATA_RECEIVED:
                        printf("Packet Status: No Data Received\r\n");
                        break;
                     case HFRM_AUDIO_DATA_FLAGS_PACKET_STATUS_DATA_PARTIALLY_LOST:
                        printf("Packet Status: Data Partially Lost\r\n");
                        break;
                     default:
                        printf("Packet Status: Unknown Flag 0x%08lX\r\n", EventData->EventData.AudioDataEventData.AudioDataFlags);
                        break;
                  }
               }

               printf("Hands Free>");

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
            if(AudioTestMode == atmLoopback)
            {
               /* Simply loop the data back.                            */
               HFRM_Send_Audio_Data(EventData->EventData.AudioDataEventData.DataEventsHandlerID, EventData->EventData.AudioDataEventData.ConnectionType, EventData->EventData.AudioDataEventData.RemoteDeviceAddress, EventData->EventData.AudioDataEventData.AudioDataLength, EventData->EventData.AudioDataEventData.AudioData);
            }
            else
            {
               if(AudioTestMode == atmTestTone)
               {
                  /* Build the Audio Data Test 1KHz tone into our local */
                  /* Audio Buffer.                                      */
                  for(Index=0;Index<(unsigned int)EventData->EventData.AudioDataEventData.AudioDataLength;Index++)
                  {
                     if(AudioData8BitFormat)
                     {
                        AudioData[Index]    = AudioDataTone_1KHz_8Bit[AudioDataToneIndex++];

                        AudioDataToneIndex %= sizeof(AudioDataTone_1KHz_8Bit);
                     }
                     else
                     {
                        AudioData[Index]    = AudioDataTone_1KHz_16Bit[AudioDataToneIndex++];

                        AudioDataToneIndex %= sizeof(AudioDataTone_1KHz_16Bit);
                     }
                  }

                  /* Data has been formatted, go ahead and send the     */
                  /* data.                                              */
                  HFRM_Send_Audio_Data(EventData->EventData.AudioDataEventData.DataEventsHandlerID, EventData->EventData.AudioDataEventData.ConnectionType, EventData->EventData.AudioDataEventData.RemoteDeviceAddress, EventData->EventData.AudioDataEventData.AudioDataLength, AudioData);
               }
            }
            break;
         case hetHFRControlIndicatorStatusIndication:
            BD_ADDRToStr(EventData->EventData.ControlIndicatorStatusIndicationEventData.RemoteDeviceAddress, BoardStr);

            /* A Control Indicator Status Indication was received,      */
            /* display all relevant information.                        */
            switch(EventData->EventData.ControlIndicatorStatusIndicationEventData.ControlIndicatorEntry.ControlIndicatorType)
            {
               case ciBoolean:
                  printf("hetHFRControlIndicatorStatusIndication, BD_ADDR: %s, Description: %s, Value: %s.\r\n", BoardStr,
                                                                                                                 EventData->EventData.ControlIndicatorStatusIndicationEventData.ControlIndicatorEntry.IndicatorDescription,
                                                                                                                 (EventData->EventData.ControlIndicatorStatusIndicationEventData.ControlIndicatorEntry.Control_Indicator_Data.ControlIndicatorBooleanType.CurrentIndicatorValue)?"TRUE":"FALSE");
                  break;
               case ciRange:
                  printf("hetHFRControlIndicatorStatusIndication, BD_ADDR: %s, Description: %s, Value: %u.\r\n", BoardStr,
                                                                                                                 EventData->EventData.ControlIndicatorStatusIndicationEventData.ControlIndicatorEntry.IndicatorDescription,
                                                                                                                 EventData->EventData.ControlIndicatorStatusIndicationEventData.ControlIndicatorEntry.Control_Indicator_Data.ControlIndicatorRangeType.CurrentIndicatorValue);
                  break;
            }
            break;
         case hetHFRControlIndicatorStatusConfirmation:
            BD_ADDRToStr(EventData->EventData.ControlIndicatorStatusConfirmationEventData.RemoteDeviceAddress, BoardStr);

            /* A Control Indicator Status Confirmation was received,    */
            /* display all relevant information.                        */
            switch(EventData->EventData.ControlIndicatorStatusConfirmationEventData.ControlIndicatorEntry.ControlIndicatorType)
            {
               case ciBoolean:
                  printf("hetHFRControlIndicatorStatusConfirmation, BD_ADDR: %s, Description: %s, Value: %s.\r\n", BoardStr,
                                                                                                                   EventData->EventData.ControlIndicatorStatusConfirmationEventData.ControlIndicatorEntry.IndicatorDescription,
                                                                                                                   (EventData->EventData.ControlIndicatorStatusConfirmationEventData.ControlIndicatorEntry.Control_Indicator_Data.ControlIndicatorBooleanType.CurrentIndicatorValue)?"TRUE":"FALSE");
                  break;
               case ciRange:
                  printf("hetHFRControlIndicatorStatusConfirmation, BD_ADDR: %s, Description: %s, Value: %u.\r\n", BoardStr,
                                                                                                                   EventData->EventData.ControlIndicatorStatusConfirmationEventData.ControlIndicatorEntry.IndicatorDescription,
                                                                                                                   EventData->EventData.ControlIndicatorStatusConfirmationEventData.ControlIndicatorEntry.Control_Indicator_Data.ControlIndicatorRangeType.CurrentIndicatorValue);
                  break;
            }
            break;
         case hetHFRCallHoldMultipartySupportConfirmation:
            BD_ADDRToStr(EventData->EventData.CallHoldMultipartySupportConfirmationEventData.RemoteDeviceAddress, BoardStr);

            /* A Call Hold and Multiparty Support Confirmation was      */
            /* received, display all relevant information.              */
            printf("hetHFRCallHoldMultipartySupportConfirmation, BD_ADDR: %s, Support Mask Valid: %s, Support Mask: 0x%08lX.\r\n", BoardStr,
                                                                                                                                   (EventData->EventData.CallHoldMultipartySupportConfirmationEventData.CallHoldSupportMaskValid)?"Yes":"No",
                                                                                                                                   EventData->EventData.CallHoldMultipartySupportConfirmationEventData.CallHoldSupportMask);
            break;
         case hetHFRCallWaitingNotificationIndication:
            BD_ADDRToStr(EventData->EventData.CallWaitingNotificationIndicationEventData.RemoteDeviceAddress, BoardStr);

            /* A Call Waiting Notification Indication was received,     */
            /* display all relevant information.                        */
            printf("hetHFRCallWaitingNotificationIndication, BD_ADDR: %s, Phone Number %s.\r\n", BoardStr,
                                                                                                 (EventData->EventData.CallWaitingNotificationIndicationEventData.PhoneNumber)?EventData->EventData.CallWaitingNotificationIndicationEventData.PhoneNumber:"<None>");
            break;
         case hetHFRCallLineIdentificationNotificationIndication:
            BD_ADDRToStr(EventData->EventData.CallLineIdentificationNotificationIndicationEventData.RemoteDeviceAddress, BoardStr);

            /* A Call Line Identification Notification Indication was   */
            /* received, display all relevant information.              */
            printf("hetHFRCallLineIdentificationNotificationIndication, BD_ADDR: %s, Phone Number %s.\r\n", BoardStr,
                                                                                                            (EventData->EventData.CallLineIdentificationNotificationIndicationEventData.PhoneNumber)?EventData->EventData.CallLineIdentificationNotificationIndicationEventData.PhoneNumber:"<None>");
            break;
         case hetHFRRingIndication:
            BD_ADDRToStr(EventData->EventData.RingIndicationEventData.RemoteDeviceAddress, BoardStr);

            /* A Ring Indication was received, display all relevant     */
            /* information.                                             */
            printf("hetHFRRingIndication, BD_ADDR: %s.\r\n", BoardStr);
            break;
         case hetHFRSpeakerGainIndication:
            BD_ADDRToStr(EventData->EventData.SpeakerGainIndicationEventData.RemoteDeviceAddress, BoardStr);
            ConnectionTypeToStr(EventData->EventData.SpeakerGainIndicationEventData.ConnectionType, ConnectionStr);

            /* A Ring Indication was received, display all relevant     */
            /* information.                                             */
            printf("hetHFRSpeakerGainIndication, BD_ADDR: %s, Type: %s, Speaker Gain: %u.\r\n", BoardStr, ConnectionStr, EventData->EventData.SpeakerGainIndicationEventData.SpeakerGain);
            break;
         case hetHFRMicrophoneGainIndication:
            BD_ADDRToStr(EventData->EventData.MicrophoneGainIndicationEventData.RemoteDeviceAddress, BoardStr);
            ConnectionTypeToStr(EventData->EventData.MicrophoneGainIndicationEventData.ConnectionType, ConnectionStr);

            /* A Ring Indication was received, display all relevant     */
            /* information.                                             */
            printf("hetHFRMicrophoneGainIndication, BD_ADDR: %s, Type: %s, Microphone Gain: %u.\r\n", BoardStr, ConnectionStr, EventData->EventData.MicrophoneGainIndicationEventData.MicrophoneGain);
            break;
         case hetHFRInBandRingToneSettingIndication:
            BD_ADDRToStr(EventData->EventData.InBandRingToneSettingIndicationEventData.RemoteDeviceAddress, BoardStr);

            /* An InBand Ring Tone Setting Indication was received,     */
            /* display all relevant information.                        */
            printf("hetHFRInBandRingToneSettingIndication, BD_ADDR: %s, Enabled: %s.\r\n", BoardStr,
                                                                                           (EventData->EventData.InBandRingToneSettingIndicationEventData.Enabled)?"TRUE":"FALSE");
            break;
         case hetHFRVoiceTagRequestConfirmation:
            BD_ADDRToStr(EventData->EventData.VoiceTagRequestConfirmationEventData.RemoteDeviceAddress, BoardStr);

            /* A Voice Tag Request Confirmation was received, display   */
            /* all relevant information.                                */
            if(EventData->EventData.VoiceTagRequestConfirmationEventData.PhoneNumber)
            {
               printf("hetHFRVoiceTagRequestConfirmation, BD_ADDR: %s, Phone Number %s.\r\n", BoardStr,
                                                                                              EventData->EventData.VoiceTagRequestConfirmationEventData.PhoneNumber);
            }
            else
            {
               printf("hetHFRVoiceTagRequestConfirmation, BD_ADDR: %s, Requeset Rejected.\r\n", BoardStr);
            }
            break;
         case hetHFRCommandResult:
            BD_ADDRToStr(EventData->EventData.CommandResultEventData.RemoteDeviceAddress, BoardStr);

            /* An Command Confirmation was received, display the        */
            /* relevant information.                                    */
            printf("hetHFRCommandResult, BD_ADDR: %s, Type %d Code %d.\r\n", BoardStr, EventData->EventData.CommandResultEventData.ResultType, EventData->EventData.CommandResultEventData.ResultValue);
            break;
         case hetHFRCurrentCallsListConfirmation:
            BD_ADDRToStr(EventData->EventData.CurrentCallsListConfirmationEventData.RemoteDeviceAddress, BoardStr);

            printf("hetHFRCurrentCallsListConfirmation, BD_ADDR: %s.\r\n", BoardStr);
            printf("+CLCC: [%d] Dir: %d Status: %d Mode: %d Multi: %c Format: %d Num: %s Name: %s\r\n", EventData->EventData.CurrentCallsListConfirmationEventData.CurrentCallListEntry.Index,
                                                                                                        EventData->EventData.CurrentCallsListConfirmationEventData.CurrentCallListEntry.CallDirection,
                                                                                                        EventData->EventData.CurrentCallsListConfirmationEventData.CurrentCallListEntry.CallStatus,
                                                                                                        EventData->EventData.CurrentCallsListConfirmationEventData.CurrentCallListEntry.CallMode,
                                                                                                        ((EventData->EventData.CurrentCallsListConfirmationEventData.CurrentCallListEntry.Multiparty)?('Y'):('N')),
                                                                                                        EventData->EventData.CurrentCallsListConfirmationEventData.CurrentCallListEntry.NumberFormat,
                                                                                                        EventData->EventData.CurrentCallsListConfirmationEventData.CurrentCallListEntry.PhoneNumber?EventData->EventData.CurrentCallsListConfirmationEventData.CurrentCallListEntry.PhoneNumber:"<None>",
                                                                                                        EventData->EventData.CurrentCallsListConfirmationEventData.CurrentCallListEntry.PhonebookName?EventData->EventData.CurrentCallsListConfirmationEventData.CurrentCallListEntry.PhonebookName:"<None>");


            break;
         case hetHFRNetworkOperatorSelectionConfirmation:
            BD_ADDRToStr(EventData->EventData.NetworkOperatorSelectionConfirmationEventData.RemoteDeviceAddress, BoardStr);

            printf("hetHFRNetworkOperatorSelectionConfirmation, BD_ADDR: %s.\r\n", BoardStr);
            printf("+COPS: Mode: %d Network: %s\r\n", EventData->EventData.NetworkOperatorSelectionConfirmationEventData.NetworkMode,
                                                      (EventData->EventData.NetworkOperatorSelectionConfirmationEventData.NetworkOperator)?EventData->EventData.NetworkOperatorSelectionConfirmationEventData.NetworkOperator:"<None>");
            break;
         case hetHFRSubscriberNumberInformationConfirmation:
            BD_ADDRToStr(EventData->EventData.SubscriberNumberInformationConfirmationEventData.RemoteDeviceAddress, BoardStr);

            printf("hetHFRSubscriberNumberInformationConfirmation, BD_ADDR: %s.\r\n", BoardStr);
            printf("+CNUM: SvcType: %d Format: %d Num: %s\r\n", EventData->EventData.SubscriberNumberInformationConfirmationEventData.SubscriberNumberInformation.ServiceType,
                                                                EventData->EventData.SubscriberNumberInformationConfirmationEventData.SubscriberNumberInformation.NumberFormat,
                                                                EventData->EventData.SubscriberNumberInformationConfirmationEventData.SubscriberNumberInformation.PhoneNumber);
            break;
         case hetHFRResponseHoldStatusConfirmation:
            BD_ADDRToStr(EventData->EventData.ResponseHoldStatusConfirmationEventData.RemoteDeviceAddress, BoardStr);

            printf("hetHFRResponseHoldStatusConfirmation, BD_ADDR: %s, CallState: %d.\r\n", BoardStr, EventData->EventData.ResponseHoldStatusConfirmationEventData.CallState);
            break;
         case hetHFRIncomingCallStateConfirmation:
            BD_ADDRToStr(EventData->EventData.IncomingCallStateConfirmationEventData.RemoteDeviceAddress, BoardStr);

            printf("hetHFRIncomingCallStateConfirmation, BD_ADDR: %s, CallState: %d.\r\n", BoardStr, EventData->EventData.IncomingCallStateConfirmationEventData.CallState);
            break;
         case hetHFRArbitraryResponseIndication:
            BD_ADDRToStr(EventData->EventData.ArbitraryResponseIndicationEventData.RemoteDeviceAddress, BoardStr);

            printf("hetHFRArbitraryResponseIndication, BD_ADDR: %s, ResponseData: %s\r\n", BoardStr, EventData->EventData.ArbitraryResponseIndicationEventData.ResponseData);
            break;
         case hetHFRVoiceRecognitionIndication:
            BD_ADDRToStr(EventData->EventData.VoiceRecognitionIndicationEventData.RemoteDeviceAddress, BoardStr);

            printf("hetHFRVoiceRecognitionIndication, BD_ADDR: %s, Enabled: %s\r\n", BoardStr, EventData->EventData.VoiceRecognitionIndicationEventData.VoiceRecognitionActive?"TRUE":"FALSE");

            HFRM_Send_Terminating_Response(HFREventCallbackID, EventData->EventData.VoiceRecognitionIndicationEventData.RemoteDeviceAddress, erOK, 0);
            break;
         default:
            /* An unknown/unexpected HFRE event was received.           */
            printf("\r\nUnknown HFRM Event Received: %d.\r\n", EventData->EventType);
            break;

   /* Audio Gateway Events.                                             */
#if 0

         case etHFRE_Call_Hold_Multiparty_Selection_Indication:
            /* A Call Hold and Multipart Selection was received, display*/
            /* all relevant information.                                */
            printf("HFRE Call Hold Multiparty Selection Indication, ID: 0x%04X, Selection: 0x%04X.\r\n", EventData->Event_Data.HFRE_Call_Hold_Multiparty_Selection_Indication_Data->HFREPortID,
                                                                                                         EventData->Event_Data.HFRE_Call_Hold_Multiparty_Selection_Indication_Data->CallHoldMultipartyHandling);

            /* Send an OK terminating response code.                    */
            HFRE_Send_Terminating_Response(BluetoothStackID, EventData->Event_Data.HFRE_Call_Hold_Multiparty_Selection_Indication_Data->HFREPortID, erOK, 0);
            break;
         case etHFRE_Call_Waiting_Notification_Activation_Indication:
            /* A Call Waiting Notification Activation was received,     */
            /* display all relevant information.                        */
            printf("HFRE Call Waiting Notification Activation Indication, ID: 0x%04X, Enabled: %s.\r\n", EventData->Event_Data.HFRE_Call_Waiting_Notification_Activation_Indication_Data->HFREPortID,
                                                                                                         (EventData->Event_Data.HFRE_Call_Waiting_Notification_Activation_Indication_Data->Enabled)?"TRUE":"FALSE");

            /* Send an OK terminating response code.                    */
            HFRE_Send_Terminating_Response(BluetoothStackID, EventData->Event_Data.HFRE_Call_Waiting_Notification_Activation_Indication_Data->HFREPortID, erOK, 0);
            break;
         case etHFRE_Call_Line_Identification_Notification_Activation_Indication:
            /* A Call Line Identification Notification Activation was   */
            /* received, display all relevant information.              */
            printf("HFRE Call Line Identification Notification Activation Indication, ID: 0x%04X, Enabled: %s.\r\n", EventData->Event_Data.HFRE_Call_Line_Identification_Notification_Activation_Indication_Data->HFREPortID,
                                                                                                                     (EventData->Event_Data.HFRE_Call_Line_Identification_Notification_Activation_Indication_Data->Enabled)?"TRUE":"FALSE");

            /* Send an OK terminating response code.                    */
            HFRE_Send_Terminating_Response(BluetoothStackID, EventData->Event_Data.HFRE_Call_Line_Identification_Notification_Activation_Indication_Data->HFREPortID, erOK, 0);
            break;
         case etHFRE_Disable_Sound_Enhancement_Indication:
            /* A Disable Sound Enhancement Indication was received,     */
            /* display all relevant information.                        */
            printf("HFRE Disable Sound Enhancement Indication, ID: 0x%04X.\r\n", EventData->Event_Data.HFRE_Disable_Sound_Enhancement_Indication_Data->HFREPortID);

            /* Send an OK terminating response code.                    */
            HFRE_Send_Terminating_Response(BluetoothStackID, EventData->Event_Data.HFRE_Disable_Sound_Enhancement_Indication_Data->HFREPortID, erOK, 0);
            break;
         case etHFRE_Dial_Phone_Number_Indication:
            /* A Dial Phone Number Indication was received, display all */
            /* relevant information.                                    */
            printf("HFRE Dial Phone Number Indication, ID: 0x%04X, Phone Number %s.\r\n", EventData->Event_Data.HFRE_Dial_Phone_Number_Indication_Data->HFREPortID,
                                                                                          EventData->Event_Data.HFRE_Dial_Phone_Number_Indication_Data->PhoneNumber);

            /* Send an OK terminating response code.                    */
            HFRE_Send_Terminating_Response(BluetoothStackID, EventData->Event_Data.HFRE_Dial_Phone_Number_Indication_Data->HFREPortID, erOK, 0);
            break;
         case etHFRE_Dial_Phone_Number_From_Memory_Indication:
            /* A Dial Phone Number from Memory Indication was received, */
            /* display all relevant information.                        */
            printf("HFRE Dial Phone Number From Memory Indication, ID: 0x%04X, Memory Location 0x%04X.\r\n", EventData->Event_Data.HFRE_Dial_Phone_Number_From_Memory_Indication_Data->HFREPortID,
                                                                                                             EventData->Event_Data.HFRE_Dial_Phone_Number_From_Memory_Indication_Data->MemoryLocation);

            /* Send an OK terminating response code.                    */
            HFRE_Send_Terminating_Response(BluetoothStackID, EventData->Event_Data.HFRE_Dial_Phone_Number_From_Memory_Indication_Data->HFREPortID, erOK, 0);
            break;
         case etHFRE_ReDial_Last_Phone_Number_Indication:
            /* A ReDial Last Phone Number Indication was received,      */
            /* display all relevant information.                        */
            printf("HFRE ReDial Last Phone Number Indication, ID: 0x%04X.\r\n", EventData->Event_Data.HFRE_ReDial_Last_Phone_Number_Indication_Data->HFREPortID);

            /* Send an OK terminating response code.                    */
            HFRE_Send_Terminating_Response(BluetoothStackID, EventData->Event_Data.HFRE_ReDial_Last_Phone_Number_Indication_Data->HFREPortID, erOK, 0);
            break;
         case etHFRE_Generate_DTMF_Tone_Indication:
            /* A Generate DTMF Tone Indication was received, display all*/
            /* relevant information.                                    */
            printf("HFRE Generate DTMF Tone Indication, ID: 0x%04X, DTMF Code %c.\r\n", EventData->Event_Data.HFRE_Generate_DTMF_Tone_Indication_Data->HFREPortID,
                                                                                        EventData->Event_Data.HFRE_Generate_DTMF_Tone_Indication_Data->DTMFCode);

            /* Send an OK terminating response code.                    */
            HFRE_Send_Terminating_Response(BluetoothStackID, EventData->Event_Data.HFRE_Generate_DTMF_Tone_Indication_Data->HFREPortID, erOK, 0);
            break;
         case etHFRE_Answer_Call_Indication:
            /* A Answer Call Indication was received, display all       */
            /* relevant information.                                    */
            printf("HFRE Answer Call Indication, ID: 0x%04X.\r\n", EventData->Event_Data.HFRE_Answer_Call_Indication_Data->HFREPortID);

            /* Send an OK terminating response code.                    */
            HFRE_Send_Terminating_Response(BluetoothStackID, EventData->Event_Data.HFRE_Answer_Call_Indication_Data->HFREPortID, erOK, 0);
            break;
         case etHFRE_Voice_Recognition_Notification_Indication:
            /* A Voice Recognition Notification Indication was received,*/
            /* display all relevant information.                        */
            printf("HFRE Voice Recognition Notification Indication, ID: 0x%04X, Active: %s.\r\n", EventData->Event_Data.HFRE_Voice_Recognition_Notification_Indication_Data->HFREPortID,
                                                                                                  (EventData->Event_Data.HFRE_Voice_Recognition_Notification_Indication_Data->VoiceRecognitionActive)?"TRUE":"FALSE");

            /* Send an OK terminating response code.                    */
            HFRE_Send_Terminating_Response(BluetoothStackID, EventData->Event_Data.HFRE_Voice_Recognition_Notification_Indication_Data->HFREPortID, erOK, 0);
            break;
         case etHFRE_Speaker_Gain_Indication:
            /* A Speaker Gain Indication was received, display all      */
            /* relevant information.                                    */
            printf("HFRE Speaker Gain Indication, ID: 0x%04X, Speaker Gain 0x%04X.\r\n", EventData->Event_Data.HFRE_Speaker_Gain_Indication_Data->HFREPortID,
                                                                                         EventData->Event_Data.HFRE_Speaker_Gain_Indication_Data->SpeakerGain);
            /* Send an OK terminating response code.                    */
            HFRE_Send_Terminating_Response(BluetoothStackID, EventData->Event_Data.HFRE_Speaker_Gain_Indication_Data->HFREPortID, erOK, 0);
            break;
         case etHFRE_Microphone_Gain_Indication:
            /* A Microphone Gain Indication was received, display all   */
            /* relevant information.                                    */
            printf("HFRE Microphone Gain Indication, ID: 0x%04X, Microphone Gain 0x%04X.\r\n", EventData->Event_Data.HFRE_Microphone_Gain_Indication_Data->HFREPortID,
                                                                                               EventData->Event_Data.HFRE_Microphone_Gain_Indication_Data->MicrophoneGain);

            /* Send an OK terminating response code.                    */
            HFRE_Send_Terminating_Response(BluetoothStackID, EventData->Event_Data.HFRE_Microphone_Gain_Indication_Data->HFREPortID, erOK, 0);
            break;
         case etHFRE_Voice_Tag_Request_Indication:
            /* A Voice Tag Request Indication was received, display all */
            /* relevant information.                                    */
            printf("HFRE Voice Tag Request Indication, ID: 0x%04X.\r\n", EventData->Event_Data.HFRE_Voice_Tag_Request_Indication_Data->HFREPortID);
            break;
         case etHFRE_Hang_Up_Indication:
            /* A Hang-Up Indication was received, display all relevant  */
            /* information.                                             */
            printf("HFRE Hang Up Indication, ID: 0x%04X.\r\n", EventData->Event_Data.HFRE_Hang_Up_Indication_Data->HFREPortID);

            /* Send an OK terminating response code.                    */
            HFRE_Send_Terminating_Response(BluetoothStackID, EventData->Event_Data.HFRE_Hang_Up_Indication_Data->HFREPortID, erOK, 0);
            break;
         case etHFRE_Network_Operator_Selection_Format_Indication:
            printf("HFRE Network Operator Selection Format Indication, ID: 0x%04X Format: %d.\r\n", EventData->Event_Data.HFRE_Network_Operator_Selection_Format_Indication_Data->HFREPortID, EventData->Event_Data.HFRE_Network_Operator_Selection_Format_Indication_Data->Format);

            /* Send an OK terminating response code.                    */
            HFRE_Send_Terminating_Response(BluetoothStackID, EventData->Event_Data.HFRE_Network_Operator_Selection_Format_Indication_Data->HFREPortID, erOK, 0);
            break;
         case etHFRE_Extended_Error_Result_Activation_Indication:
            printf("HFRE Extended Error Result Activation Indication, ID: 0x%04X. Enabled: %c\r\n", EventData->Event_Data.HFRE_Extended_Error_Result_Activation_Indication_Data->HFREPortID, ((EventData->Event_Data.HFRE_Extended_Error_Result_Activation_Indication_Data->Enabled==TRUE)?('Y'):('N')));

            /* Send an OK terminating response code.                    */
            HFRE_Send_Terminating_Response(BluetoothStackID, EventData->Event_Data.HFRE_Extended_Error_Result_Activation_Indication_Data->HFREPortID, erOK, 0);
            break;
         case etHFRE_Response_Hold_Status_Indication:
            printf("HFRE Response Hold Status Indication, ID: 0x%04X.\r\n", EventData->Event_Data.HFRE_Response_Hold_Status_Indication_Data->HFREPortID);

            /* Send an OK terminating response code.                    */
            HFRE_Send_Terminating_Response(BluetoothStackID, EventData->Event_Data.HFRE_Response_Hold_Status_Indication_Data->HFREPortID, erOK, 0);
            break;
         case etHFRE_Current_Calls_List_Indication:
            printf("etHFRE Current Calls List Indication, ID: 0x%04X.\r\n", EventData->Event_Data.HFRE_Current_Calls_List_Indication_Data->HFREPortID);
            break;
         case etHFRE_Network_Operator_Selection_Indication:
            BD_ADDRToStr(EventData->EventData.CurrentCallsListConfirmationEventData.RemoteDeviceAddress, BoardStr);

            printf("HFRE Network Operator Selection Indication, ID: 0x%04X.\r\n", EventData->Event_Data.HFRE_Network_Operator_Selection_Indication_Data->HFREPortID);
            break;
         case etHFRE_Subscriber_Number_Information_Indication:
            printf("HFRE Subscriber Number Information Indication, ID: 0x%04X.\r\n", EventData->Event_Data.HFRE_Subscriber_Number_Information_Indication_Data->HFREPortID);
            break;
         case etHFRE_Incoming_Call_State_Indication:
            printf("HFRE Incoming Call State Indication, ID: 0x%04X CallState: %d.\r\n", EventData->Event_Data.HFRE_Incoming_Call_State_Indication_Data->HFREPortID, EventData->Event_Data.HFRE_Incoming_Call_State_Indication_Data->CallState);
            break;

#endif

      }

      /* Output an Input Shell-type prompt.                             */
      if(EventData->EventType != hetHFRAudioData)
      {
         printf("Hands Free>");

         /* Make sure the output is displayed to the user.              */
         fflush(stdout);
      }
   }
   else
   {
      /* There was an error with one or more of the input parameters.   */
      printf("\r\nHFRM callback data: Event_Data = NULL.\r\n");
   }
}

   /* Main Program Entry Point.                                         */
int main(int argc, char *argv[])
{
   /* Initialize the default Secure Simple Pairing parameters.          */
   IOCapability   = DEFAULT_IO_CAPABILITY;
   OOBSupport     = FALSE;
   MITMProtection = DEFAULT_MITM_PROTECTION;

   /* Nothing really to do here aside from running the main application */
   /* code.                                                             */
   UserInterface();

   /* Return success.                                                   */
   return(0);
}
