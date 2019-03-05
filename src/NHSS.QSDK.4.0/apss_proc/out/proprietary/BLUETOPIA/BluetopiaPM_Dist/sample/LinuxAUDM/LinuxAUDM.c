/*****< linuxaudm.c >**********************************************************/
/*      Copyright 2011 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  LINUXAUDM - Simple Linux application using Bluetopia Platform Manager     */
/*              Audio Manager Application Programming Interface (API).        */
/*                                                                            */
/*  Author:  Greg Hensley                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   01/13/11  G. Hensley     Initial creation. (Based on LinuxDEVM)          */
/******************************************************************************/
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/stat.h>
#include <time.h>
#include <unistd.h>        /* Include for getpid().                           */

#include "LinuxAUDM.h"     /* Main Application Prototypes and Constants.      */

#include "SS1BTAUDM.h"     /* Audio Manager Application Programming Interface.*/
#include "SS1BTPM.h"       /* BTPM Application Programming Interface.         */

#include "AudioEncoder.h"  /* Audio Encoder sample.                           */
#include "AudioDecoder.h"  /* Audio Decoder sample.                           */

#define MAX_SUPPORTED_COMMANDS                     (96)  /* Denotes the       */
                                                         /* maximum number of */
                                                         /* User Commands that*/
                                                         /* are supported by  */
                                                         /* this application. */

#define MAX_COMMAND_LENGTH                        (128)  /* Denotes the max   */
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

#define INDENT_LENGTH                               (3)  /* Denotes the number*/
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

#define TOO_MANY_PARAMS                            (-5)  /* Denotes that there*/
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


#define WAV_FORMAT_ERROR                           (-8)  /* Denotes than an   */
                                                         /* error occurred due*/
                                                         /* to the fact that  */
                                                         /* the WAV Format    */
                                                         /* parsed from the   */
                                                         /* WAV header was    */
                                                         /* invalid or is not */
                                                         /* supported.        */

#define WAV_PCM_BUFFER_SIZE                     (10240)  /* Denotes the size  */
                                                         /* in bytes of the   */
                                                         /* Left and Right    */
                                                         /* Channel PCM Data  */
                                                         /* Buffer (Read from */
                                                         /* the WAV file).    */
                                                         /* * NOTE * This must*/
                                                         /*          be a     */
                                                         /*          multiple */
                                                         /*          of four  */
                                                         /*          (one left*/
                                                         /*          sample   */
                                                         /*          and one  */
                                                         /*          right    */
                                                         /*          sample). */

#define DEFAULT_AVRCP_COMMAND_TIMEOUT            (5000)  /* The following     */
                                                         /* timeout specifies */
                                                         /* the default       */
                                                         /* timeout to use    */
                                                         /* when waiting for  */
                                                         /* an AVRCP Command  */
                                                         /* Response from the */
                                                         /* remote device.    */

#define IGNORED_AUDIO_PACKETS_THRESHOLD           (200)  /* Denotes the number*/
                                                         /* of audio packets  */
                                                         /* not destined for  */
                                                         /* the active playing*/
                                                         /* stream to ignore  */
                                                         /* before displaying.*/

#define MAX_AUDIO_FILE_NAME_LENGTH                 (64)  /* Denotes the buffer*/
                                                         /* size for holding  */
                                                         /* the name of the   */
                                                         /* file for writing  */
                                                         /* decoded audio.    */

   /* Change the default flags based on whether we support ALSA         */
   /* streaming.                                                        */
#ifndef DISABLE_AUDIO_SINK_ALSA_OUTPUT

                                                         /* Default flags     */
                                                         /* for the audio     */
                                                         /* decoder.          */
#define DEFAULT_DECODER_CONFIG_FLAGS               (DECODER_CONFIG_FLAGS_ENABLE_ALSA_OUTPUT)

#else

                                                         /* Default flags     */
                                                         /* for the audio     */
                                                         /* decoder.          */
#define DEFAULT_DECODER_CONFIG_FLAGS               (DECODER_CONFIG_FLAGS_ENABLE_WAV_OUTPUT)

#endif

   /* The following helper macros convert an absolute volume to and from*/
   /* a percentage.                                                     */
#define ABSOLUTE_VOLUME_TO_PERCENTAGE(_x)       ((Byte_t)((((Word_t)(_x) & (Word_t)0x7F) * (Word_t)100) / (Word_t)0x7F))
#define PERCENTAGE_TO_ABSOLUTE_VOLUME(_x)       ((Byte_t)(((Word_t)(_x) * (Word_t)0x7F) / (Word_t)100))

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

   /* The following type definition represents the structure which holds*/
   /* state information regarding an Audio Stream connection.           */
typedef struct _tagAudioStream_t
{
   int                CallbackID;
   AUD_Stream_State_t StreamState;
   Event_t            StreamStateChanged;
} AudioStream_t;

   /* The following type definition represents the structure which holds*/
   /* information describing the audio payload of a WAV format file.    */
typedef struct _tagWAVInfo_t
{
   DWord_t AudioDataOffset;
   DWord_t AudioDataLength;
   Word_t  Format;
   Word_t  Channels;
   DWord_t SamplesPerSecond;
   DWord_t AverageBytesPerSecond;
   Word_t  BlockSize;
   Word_t  BitsPerSample;
   Word_t  ValidBitsPerSample;
} WAVInfo_t;

   /* The following constants are used when processing the WAV Header   */
   /* read from a WAV file.                                             */
#define WAVE_FORMAT_PCM        0x0001
#define WAVE_FORMAT_EXTENSIBLE 0xFFFE

   /* The following type definition represents the structure which holds*/
   /* parameters passed to the Playback Thread for audio playback.      */
typedef struct _tagPlaybackThreadParams_t
{
   int           CallbackID;
   BD_ADDR_t     RemoteDevice;
   int           FileDescriptor;
   WAVInfo_t     WAVInfo;
   unsigned long Offset;
   Boolean_t     Loop;
} PlaybackThreadParams_t;

   /* The following structure is used to build a table to map Pass      */
   /* Through commands to CLI inputs and AVRCP IDs.                     */
typedef struct _tagPassThroughCommand_t
{
   unsigned int  OperationID;
   char         *Display;
} PassThroughCommand_t;

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

static AudioStream_t       AudioSinkState;          /* Variable which holds the        */
                                                    /* current Stream State for the    */
                                                    /* active A2DP Sink Audio Stream.  */

static AudioStream_t       AudioSourceState;        /* Variable which holds the        */
                                                    /* current Stream State for the    */
                                                    /* active A2DP Source Audio Stream.*/

static unsigned int        AUDMRCControllerCallbackID; /* Variable which holds the     */
                                                    /* Callback ID of the currently    */
                                                    /* registered Audio Manager Remote */
                                                    /* Control Event Callback for the  */
                                                    /* Remote Control Controller role. */

static int                 AUDMRCTargetCallbackID;  /* Variable which holds the        */
                                                    /* Callback ID of the currently    */
                                                    /* registered Audio Manager Remote */
                                                    /* Control Event Callback for the  */
                                                    /* Remote Control Target role.     */

static unsigned long       FileOffset;              /* Variable which keeps track of   */
                                                    /* the location within a playing   */
                                                    /* WAV file to resume from.        */

static unsigned int        NumberCommands;          /* Variable which is used to hold  */
                                                    /* the number of Commands that are */
                                                    /* supported by this application.  */
                                                    /* Commands are added individually.*/

static CommandTable_t      CommandTable[MAX_SUPPORTED_COMMANDS]; /* Variable which is  */
                                                    /* used to hold the actual Commands*/
                                                    /* that are supported by this      */
                                                    /* application.                    */

static Byte_t              CurrentAbsoluteVolume;   /* Variable stores the current     */
                                                    /* absolute volume.                */

static Byte_t              CurrentPlayStatus;       /* Variable stores the current play*/
                                                    /* status.                         */

static BD_ADDR_t           CurrentEncodingBD_ADDR;  /* Variable which holds the current*/
                                                    /* BD_ADDR which audio is being    */
                                                    /* sent to.                        */

static BD_ADDR_t           CurrentDecodingBD_ADDR;  /* Variable which holds the current*/
                                                    /* BD_ADDR which audio is being    */
                                                    /* received from.                  */

static unsigned int        IgnoredAudioPackets;     /* Variable which tracks the number*/
                                                    /* of ignored (not decoded) audio  */
                                                    /* packets since the last time one */
                                                    /* was printed to the UI.          */

static char                AudioFileName[MAX_AUDIO_FILE_NAME_LENGTH]; /* Variable which*/
                                                    /* holds the name of the file for  */
                                                    /* decoded audio.                  */

static unsigned int        DecoderConfigFlags;      /* Variable which holds the flags  */
                                                    /* currently configured for audio  */
                                                    /* decoding.                       */

   /* The following string table is used to map the API I/O Capabilities*/
   /* values to an easily displayable string.                           */
static char *IOCapabilitiesStrings[] =
{
   "Display Only",
   "Display Yes/No",
   "Keyboard Only",
   "No Input/Output"
};

static char *AVRCPVersionStrings[] =
{
   "1.0",
   "1.3",
   "1.4",
   "1.5",
   "1.6"
};

#define NUMBER_KNOWN_AVRCP_VERSIONS                (sizeof(AVRCPVersionStrings)/sizeof(char *))

   /* The following events specify events that remote devices can       */
   /* register for on this device if this application is registered with*/
   /* AUDM for A2DP source events.                                      */
static Byte_t GetCapabilitiesSourceResponseEventIDs[] =
{
   AVRCP_EVENT_PLAYBACK_STATUS_CHANGED,
   AVRCP_EVENT_TRACK_CHANGED,
   AVRCP_EVENT_TRACK_REACHED_END,
   AVRCP_EVENT_TRACK_REACHED_START,
   AVRCP_EVENT_PLAYBACK_POS_CHANGED,
   AVRCP_EVENT_PLAYER_APPLICATION_SETTING_CHANGED,
   AVRCP_EVENT_NOW_PLAYING_CONTENT_CHANGED,
   AVRCP_EVENT_AVAILABLE_PLAYERS_CHANGED,
   AVRCP_EVENT_ADDRESSED_PLAYER_CHANGED
};

#define NUMBER_GET_CAPABILITIES_SOURCE_RESPONSE_EVENT_IDS   (sizeof(GetCapabilitiesSourceResponseEventIDs) / sizeof(GetCapabilitiesSourceResponseEventIDs[0]))

   /* The following events specify events that remote devices can       */
   /* register for on this device if this application is registered with*/
   /* AUDM for A2DP sink events.                                        */
static Byte_t GetCapabilitiesSinkResponseEventIDs[] =
{
   AVRCP_EVENT_VOLUME_CHANGED
};

#define NUMBER_GET_CAPABILITIES_SINK_RESPONSE_EVENT_IDS     (sizeof(GetCapabilitiesSinkResponseEventIDs) / sizeof(GetCapabilitiesSinkResponseEventIDs[0]))

   /* The following definition specifies the maximum number of Event IDs*/
   /* this application supports.                                        */
#define MAXIMUM_NUMBER_GET_CAPABILITIES_RESPONSE_EVENT_IDS  (NUMBER_GET_CAPABILITIES_SOURCE_RESPONSE_EVENT_IDS + NUMBER_GET_CAPABILITIES_SINK_RESPONSE_EVENT_IDS)

   /* List of common passthrough commands that may want to be used      */
   /* in this sample. Other commands can still be explicitly sent       */
   /* using their IDs, but this allows us to print out the common       */
   /* ones. This can be ammended with more IDs from AVRCP in the future */
   /* if necessary.                                                     */
static PassThroughCommand_t PassThroughCommandTable[] =
{
   { AVRCP_PASS_THROUGH_ID_PLAY,        "Play"        },
   { AVRCP_PASS_THROUGH_ID_PAUSE,       "Pause"       },
   { AVRCP_PASS_THROUGH_ID_STOP,        "Stop"        },
   { AVRCP_PASS_THROUGH_ID_VOLUME_UP,   "Volume Up"   },
   { AVRCP_PASS_THROUGH_ID_VOLUME_DOWN, "Volume Down" },
   { AVRCP_PASS_THROUGH_ID_FORWARD,     "Forward"     },
   { AVRCP_PASS_THROUGH_ID_BACKWARD,    "Backward"    },
};

#define NUMBER_PASSTHROUGH_COMMANDS             (sizeof(PassThroughCommandTable)/sizeof(PassThroughCommand_t))

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
static void StrToBD_ADDR(char *BoardStr, BD_ADDR_t *Board_Address);

static int DisplayHelp(ParameterList_t *TempParam);
static void DisplayRemoteControlEvents(void);
static void DisplayRemoteControlPlaybackStatus(Byte_t PlaybackStatus);
static void DisplayRemoteControlAttributeValue(AVRCP_Attribute_Value_ID_List_Entry_t *AttributeValue);
static void DisplayRemoteControlResponseCode(char *Prefix, Byte_t ResponseCode, char* Suffix);
static char *PassThroughToStr(unsigned int OperationID);

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

static int AUDRegisterAudioEndPoint(ParameterList_t *TempParam);
static int AUDUnRegisterAudioEndPoint(ParameterList_t *TempParam);
static int AUDRegisterRemoteControl(ParameterList_t *TempParam);
static int AUDUnRegisterRemoteControl(ParameterList_t *TempParam);
static int AUDConnectAudio(ParameterList_t *TempParam);
static int AUDDisconnectAudio(ParameterList_t *TempParam);
static int AUDConnectionRequestResponse(ParameterList_t *TempParam);
static int AUDChangeIncomingFlags(ParameterList_t *TempParam);
static int AUDPlayWAV(ParameterList_t *TempParam);
static int AUDGetStreamStatus(ParameterList_t *TempParam);
static int AUDQueryConnectedAudioDevices(ParameterList_t *TempParam);
static int AUDQueryStreamState(ParameterList_t *TempParam);
static int AUDSetStreamState(ParameterList_t *TempParam);
static int AUDQueryStreamFormat(ParameterList_t *TempParam);
static int AUDSetStreamFormat(ParameterList_t *TempParam);
static int AUDQueryStreamConfiguration(ParameterList_t *TempParam);
static int AUDSendRemoteControlPassThroughCommand(ParameterList_t *TempParam);
static int AUDConnectRemoteControl(ParameterList_t *TempParam);
static int AUDDisconnectRemoteControl(ParameterList_t *TempParam);
static int AUDQueryConnectedRemoteControls(ParameterList_t *TempParam);
static int AUDSendRemoteControlGetCapabilitiesCommand(ParameterList_t *TempParam);
static int AUDSendRemoteControlSetAbsoluteVolumeCommand(ParameterList_t *TempParam);
static int AUDSendRemoteControlRegisterNotificationCommand(ParameterList_t *TempParam);
static int AUDSendRemoteControlRegisterNotificationResponse(ParameterList_t *TempParam);
static int AUDSendRemoteControlGetTrackInformationCommand(ParameterList_t *TempParam);
static int AUDSendRemoteControlGetPlayStatusCommand(ParameterList_t *TempParam);
static int AUDQueryRemoteControlServiceInfo(ParameterList_t *TempParam);
static int AUDConfigurePlayback(ParameterList_t *TempParam);
static int AUDCurrentPlaybackConfig(ParameterList_t *TempParam);
static int AUDConnectBrowsing(ParameterList_t *TempParam);
static int AUDDisconnectBrowsing(ParameterList_t *TempParam);
static int AUDGetTotalNumberOfItems(ParameterList_t *TempParam);

static int ParseWAVHeader(int FileDescriptor, WAVInfo_t *Info);
static void *PlaybackThreadMain(void *ThreadParameter);

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

   /* BTPM Audio Manager Callback function prototype.                   */
static void BTPSAPI AUDM_Event_Callback(AUDM_Event_Data_t *EventData, void *CallbackParameter);

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

   AddCommand("AUDREGISTERAUDIOENDPOINT", AUDRegisterAudioEndPoint);
   AddCommand("AUDUNREGISTERAUDIOENDPOINT", AUDUnRegisterAudioEndPoint);
   AddCommand("AUDREGISTERREMOTECONTROL", AUDRegisterRemoteControl);
   AddCommand("AUDUNREGISTERREMOTECONTROL", AUDUnRegisterRemoteControl);
   AddCommand("AUDCONNECTAUDIO", AUDConnectAudio);
   AddCommand("AUDDISCONNECTAUDIO", AUDDisconnectAudio);
   AddCommand("AUDCONNECTIONREQUESTRESPONSE", AUDConnectionRequestResponse);
   AddCommand("AUDCHANGEINCOMINGFLAGS", AUDChangeIncomingFlags);
   AddCommand("AUDPLAYWAV", AUDPlayWAV);
   AddCommand("AUDGETSTREAMSTATUS", AUDGetStreamStatus);
   AddCommand("AUDQUERYCONNECTEDAUDIODEVICES", AUDQueryConnectedAudioDevices);
   AddCommand("AUDQUERYSTREAMSTATE", AUDQueryStreamState);
   AddCommand("AUDSETSTREAMSTATE", AUDSetStreamState);
   AddCommand("AUDQUERYSTREAMFORMAT", AUDQueryStreamFormat);
   AddCommand("AUDSETSTREAMFORMAT", AUDSetStreamFormat);
   AddCommand("AUDQUERYSTREAMCONFIGURATION", AUDQueryStreamConfiguration);
   AddCommand("AUDSENDREMOTECONTROLPASSTHROUGHCOMMAND", AUDSendRemoteControlPassThroughCommand);
   AddCommand("AUDCONNECTREMOTECONTROL", AUDConnectRemoteControl);
   AddCommand("AUDDISCONNECTREMOTECONTROL", AUDDisconnectRemoteControl);
   AddCommand("AUDQUERYCONNECTEDREMOTECONTROLS", AUDQueryConnectedRemoteControls);
   AddCommand("AUDSENDREMOTECONTROLGETCAPABILITIESCOMMAND", AUDSendRemoteControlGetCapabilitiesCommand);
   AddCommand("AUDSendRemoteControlSetAbsoluteVolumeCommand", AUDSendRemoteControlSetAbsoluteVolumeCommand);
   AddCommand("AUDSENDREMOTECONTROLREGISTERNOTIFICATIONCOMMAND", AUDSendRemoteControlRegisterNotificationCommand);
   AddCommand("AUDSENDREMOTECONTROLREGISTERNOTIFICATIONRESPONSE", AUDSendRemoteControlRegisterNotificationResponse);
   AddCommand("AUDSENDREMOTECONTROLGETTRACKINFORMATIONCOMMAND", AUDSendRemoteControlGetTrackInformationCommand);
   AddCommand("AUDSENDREMOTECONTROLGETPLAYSTATUSCOMMAND", AUDSendRemoteControlGetPlayStatusCommand);
   AddCommand("AUDQUERYREMOTECONTROLSERVICEINFO", AUDQueryRemoteControlServiceInfo);
   AddCommand("AUDCONFIGUREPLAYBACK", AUDConfigurePlayback);
   AddCommand("AUDCURRENTPLAYBACKCONFIG", AUDCurrentPlaybackConfig);
   AddCommand("AUDCONNECTBROWSING", AUDConnectBrowsing);
   AddCommand("AUDDISCONNECTBROWSING", AUDDisconnectBrowsing);
   AddCommand("AUDGETOTALNUMBEROFITEMS", AUDGetTotalNumberOfItems);

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
      printf("AUDM>");

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
      /* Retrieve the first token in the string, this should be the     */
      /* commmand.                                                      */
      TempCommand->Command = StringParser(UserInput);

      /* Flag that there are NO Parameters for this Command Parse.      */
      TempCommand->Parameters.NumberofParameters = 0;

      /* Check to see if there is a Command.                            */
      if(TempCommand->Command)
      {
         /* Initialize the return value to zero to indicate success on  */
         /* commands with no parameters.                                */
         ret_val = 0;

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

               ret_val      = TOO_MANY_PARAMS;
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
   printf("************************************************************************\r\n");
   printf("* Command Options: 1) Initialize                                       *\r\n");
   printf("*                  2) Cleanup                                          *\r\n");
   printf("*                  3) QueryDebugZoneMask                               *\r\n");
   printf("*                  4) SetDebugZoneMask                                 *\r\n");
   printf("*                  5) ShutdownService                                  *\r\n");
   printf("*                  6) RegisterEventCallback,                           *\r\n");
   printf("*                  7) UnRegisterEventCallback,                         *\r\n");
   printf("*                  8) QueryDevicePower                                 *\r\n");
   printf("*                  9) SetDevicePower                                   *\r\n");
   printf("*                  10)QueryLocalDeviceProperties                       *\r\n");
   printf("*                  11)SetLocalDeviceName                               *\r\n");
   printf("*                  12)SetLocalClassOfDevice                            *\r\n");
   printf("*                  13)SetDiscoverable                                  *\r\n");
   printf("*                  14)SetConnectable                                   *\r\n");
   printf("*                  15)SetPairable                                      *\r\n");
   printf("*                  16)StartDeviceDiscovery                             *\r\n");
   printf("*                  17)StopDeviceDiscovery                              *\r\n");
   printf("*                  18)QueryRemoteDeviceList                            *\r\n");
   printf("*                  19)QueryRemoteDeviceProperties                      *\r\n");
   printf("*                  20)AddRemoteDevice                                  *\r\n");
   printf("*                  21)DeleteRemoteDevice                               *\r\n");
   printf("*                  22)DeleteRemoteDevices                              *\r\n");
   printf("*                  23)PairWithRemoteDevice                             *\r\n");
   printf("*                  24)CancelPairWithRemoteDevice                       *\r\n");
   printf("*                  25)UnPairRemoteDevice                               *\r\n");
   printf("*                  26)QueryRemoteDeviceServices                        *\r\n");
   printf("*                  27)RegisterAuthentication                           *\r\n");
   printf("*                  28)UnRegisterAuthentication                         *\r\n");
   printf("*                  29)PINCodeResponse                                  *\r\n");
   printf("*                  30)PassKeyResponse                                  *\r\n");
   printf("*                  31)UserConfirmationResponse                         *\r\n");
   printf("*                  32)ChangeSimplePairingParameters                    *\r\n");
   printf("*                  33)AUDRegisterAudioEndPoint                         *\r\n");
   printf("*                  34)AUDUnRegisterAudioEndPoint                       *\r\n");
   printf("*                  35)AUDRegisterRemoteControl                         *\r\n");
   printf("*                  36)AUDUnRegisterRemoteControl                       *\r\n");
   printf("*                  37)AUDConnectAudio                                  *\r\n");
   printf("*                  38)AUDDisconnectAudio                               *\r\n");
   printf("*                  39)AUDConnectionRequestResponse                     *\r\n");
   printf("*                  40)AUDChangeIncomingFlags                           *\r\n");
   printf("*                  41)AUDPlayWAV                                       *\r\n");
   printf("*                  42)AUDGetStreamStatus                               *\r\n");
   printf("*                  43)AUDQueryConnectedAudioDevices                    *\r\n");
   printf("*                  44)AUDQueryStreamState                              *\r\n");
   printf("*                  45)AUDSetStreamState                                *\r\n");
   printf("*                  46)AUDQueryStreamFormat                             *\r\n");
   printf("*                  47)AUDSetStreamFormat                               *\r\n");
   printf("*                  48)AUDQueryStreamConfiguration                      *\r\n");
   printf("*                  49)AUDSendRemoteControlPassThroughCommand           *\r\n");
   printf("*                  50)AUDConnectRemoteControl                          *\r\n");
   printf("*                  51)AUDDisconnectRemoteControl                       *\r\n");
   printf("*                  52)AUDQueryConnectedRemoteControls                  *\r\n");
   printf("*                  53)AUDSendRemoteControlGetCapabilitiesCommand       *\r\n");
   printf("*                  54)AUDSendRemoteControlSetAbsoluteVolumeCommand     *\r\n");
   printf("*                  55)AUDSendRemoteControlRegisterNotificationCommand  *\r\n");
   printf("*                  56)AUDSendRemoteControlRegisterNotificationResponse *\r\n");
   printf("*                  57)AUDSendRemoteControlGetTrackInformationCommand   *\r\n");
   printf("*                  58)AUDSendRemoteControlGetPlayStatusCommand         *\r\n");
   printf("*                  59)AUDQueryRemoteControlServiceInfo                 *\r\n");
   printf("*                  60)AUDConfigurePlayback                             *\r\n");
   printf("*                  61)AUDCurrentPlaybackConfig                         *\r\n");
   printf("*                  62)AUDConnectBrowsing                               *\r\n");
   printf("*                  63)AUDDisconnectBrowsing                            *\r\n");
   printf("*                  Help, Quit.                                         *\r\n");
   printf("************************************************************************\r\n");

   return(0);
}

   /* The following function displays Event IDs used with AVRCP Get     */
   /* Capabilities and Register Notifications Commands.                 */
static void DisplayRemoteControlEvents(void)
{
   printf("Event IDs (AVRCP Version >= 1.3):\r\n");
   printf("   AVRCP_EVENT_PLAYBACK_STATUS_CHANGED:            0x%02X\r\n", AVRCP_EVENT_PLAYBACK_STATUS_CHANGED);
   printf("   AVRCP_EVENT_TRACK_CHANGED:                      0x%02X\r\n", AVRCP_EVENT_TRACK_CHANGED);
   printf("   AVRCP_EVENT_TRACK_REACHED_END:                  0x%02X\r\n", AVRCP_EVENT_TRACK_REACHED_END);
   printf("   AVRCP_EVENT_TRACK_REACHED_START:                0x%02X\r\n", AVRCP_EVENT_TRACK_REACHED_START);
   printf("   AVRCP_EVENT_PLAYBACK_POS_CHANGED:               0x%02X\r\n", AVRCP_EVENT_PLAYBACK_POS_CHANGED);
   printf("   AVRCP_EVENT_BATT_STATUS_CHANGED:                0x%02X\r\n", AVRCP_EVENT_BATT_STATUS_CHANGED);
   printf("   AVRCP_EVENT_SYSTEM_STATUS_CHANGED:              0x%02X\r\n", AVRCP_EVENT_SYSTEM_STATUS_CHANGED);
   printf("   AVRCP_EVENT_PLAYER_APPLICATION_SETTING_CHANGED: 0x%02X\r\n", AVRCP_EVENT_PLAYER_APPLICATION_SETTING_CHANGED);
   printf("Event IDs (AVRCP Version >= 1.4):\r\n");
   printf("   AVRCP_EVENT_NOW_PLAYING_CONTENT_CHANGED:        0x%02X\r\n", AVRCP_EVENT_NOW_PLAYING_CONTENT_CHANGED);
   printf("   AVRCP_EVENT_AVAILABLE_PLAYERS_CHANGED:          0x%02X\r\n", AVRCP_EVENT_AVAILABLE_PLAYERS_CHANGED);
   printf("   AVRCP_EVENT_ADDRESSED_PLAYER_CHANGED:           0x%02X\r\n", AVRCP_EVENT_ADDRESSED_PLAYER_CHANGED);
   printf("   AVRCP_EVENT_UIDS_CHANGED:                       0x%02X\r\n", AVRCP_EVENT_UIDS_CHANGED);
   printf("   AVRCP_EVENT_VOLUME_CHANGED:                     0x%02X\r\n", AVRCP_EVENT_VOLUME_CHANGED);
}

   /* The following function displays the remote control playback status*/
   /* in the terminal.                                                  */
static void DisplayRemoteControlPlaybackStatus(Byte_t PlaybackStatus)
{
   char *Status;

   switch(PlaybackStatus)
   {
      case AVRCP_PLAY_STATUS_STATUS_STOPPED:
         Status = "Stopped";
         break;
      case AVRCP_PLAY_STATUS_STATUS_PLAYING:
         Status = "Playing";
         break;
      case AVRCP_PLAY_STATUS_STATUS_PAUSED:
         Status = "Paused";
         break;
      case AVRCP_PLAY_STATUS_STATUS_FWD_SEEK:
         Status = "Forward Seek";
         break;
      case AVRCP_PLAY_STATUS_STATUS_REV_SEEK:
         Status = "Reverse Seek";
         break;
      case AVRCP_PLAY_STATUS_STATUS_ERROR:
         Status = "Error";
         break;
      default:
         Status = "Unknown";
         break;
   }

   printf("%s", Status);
}

   /* The following function display a remote control response code.    */
static void DisplayRemoteControlResponseCode(char *Prefix, Byte_t ResponseCode, char* Suffix)
{
   char *ResponseCodeStr;

   if(!Prefix)
      Prefix = "";

   if(!Suffix)
      Suffix = "";

   switch(ResponseCode)
   {
      case AVRCP_RESPONSE_NOT_IMPLEMENTED: ResponseCodeStr = "Not Implemented";  break;
      case AVRCP_RESPONSE_ACCEPTED:        ResponseCodeStr = "Accepted";         break;
      case AVRCP_RESPONSE_REJECTED:        ResponseCodeStr = "Rejected";         break;
      case AVRCP_RESPONSE_IN_TRANSITION:   ResponseCodeStr = "In Transition";    break;
      case AVRCP_RESPONSE_STABLE:          ResponseCodeStr = "Stable";           break;
      case AVRCP_RESPONSE_CHANGED:         ResponseCodeStr = "Changed";          break;
      case AVRCP_RESPONSE_INTERIM:         ResponseCodeStr = "Interim";          break;
      default:                             ResponseCodeStr = "Unknown Response"; break;
   }

   printf("%s0x%02X (%s)%s", Prefix, ResponseCode, ResponseCodeStr, Suffix);
}

   /* The following function displays a Remote Control Attribute Value. */
static void DisplayRemoteControlAttributeValue(AVRCP_Attribute_Value_ID_List_Entry_t *AttributeValue)
{
   char *Temp;

   if(AttributeValue)
   {
      Temp = NULL;

      switch(AttributeValue->AttributeID)
      {
         case AVRCP_PLAYER_SETTING_ATTRIBUTE_ID_ILLEGAL:
            printf("Illegal Attribute ID");
            break;
         case AVRCP_PLAYER_SETTING_ATTRIBUTE_ID_EQUALIZER_ON_OFF_STATUS:
            printf("Equalizer ");
            switch(AttributeValue->ValueID)
            {
               case AVRCP_PLAYER_SETTING_ATTRIBUTE_VALUE_ID_EQUALIZER_STATUS_OFF: Temp = "Off";      break;
               case AVRCP_PLAYER_SETTING_ATTRIBUTE_VALUE_ID_EQUALIZER_STATUS_ON:  Temp = "On";       break;
               default:                                                           Temp = "Reserved"; break;
            }
            break;
         case AVRCP_PLAYER_SETTING_ATTRIBUTE_ID_REPEAT_MODE_STATUS:
            printf("Repeat ");
            switch(AttributeValue->ValueID)
            {
               case AVRCP_PLAYER_SETTING_VALUE_ID_REPEAT_MODE_STATUS_OFF:          Temp = "Off";          break;
               case AVRCP_PLAYER_SETTING_VALUE_ID_REPEAT_MODE_STATUS_SINGLE_TRACK: Temp = "Single Track"; break;
               case AVRCP_PLAYER_SETTING_VALUE_ID_REPEAT_MODE_STATUS_ALL_TRACKS:   Temp = "All Tracks";   break;
               case AVRCP_PLAYER_SETTING_VALUE_ID_REPEAT_MODE_STATUS_GROUP:        Temp = "Group";        break;
               default:                                                            Temp = "Reserved";     break;
            }
            break;
         case AVRCP_PLAYER_SETTING_ATTRIBUTE_ID_SHUFFLE_ON_OFF_STATUS:
            printf("Shuffle ");
            switch(AttributeValue->ValueID)
            {
               case AVRCP_PLAYER_SETTING_VALUE_ID_SHUFFLE_STATUS_OFF:        Temp = "Off";        break;
               case AVRCP_PLAYER_SETTING_VALUE_ID_SHUFFLE_STATUS_ALL_TRACKS: Temp = "All Tracks"; break;
               case AVRCP_PLAYER_SETTING_VALUE_ID_SHUFFLE_STATUS_GROUP:      Temp = "Group";      break;
               default:                                                      Temp = "Reserved";   break;
            }
            break;
         case AVRCP_PLAYER_SETTING_ATTRIBUTE_ID_SCAN_ON_OFF_STATUS:
            printf("Scan ");
            switch(AttributeValue->ValueID)
            {
               case AVRCP_PLAYER_SETTING_VALUE_ID_SCAN_STATUS_OFF:        Temp = "Off";        break;
               case AVRCP_PLAYER_SETTING_VALUE_ID_SCAN_STATUS_ALL_TRACKS: Temp = "All Tracks"; break;
               case AVRCP_PLAYER_SETTING_VALUE_ID_SCAN_STATUS_GROUP:      Temp = "Group";      break;
               default:                                                   Temp = "Reserved";   break;
            }
            break;
         default:
            printf("Reserved");
            break;
      }

      if(Temp)
         printf("(%s)", Temp);
   }
}

   /* The following function is a utility to get a displayable string   */
   /* for known Pass Through commands.                                  */
static char *PassThroughToStr(unsigned int OperationID)
{
   char         *ret_val;
   unsigned int  Index;

   ret_val = "Other";

   for(Index = 0; Index < NUMBER_PASSTHROUGH_COMMANDS; Index++)
   {
      if(PassThroughCommandTable[Index].OperationID == OperationID)
      {
         ret_val = PassThroughCommandTable[Index].Display;
         break;
      }
   }

   return(ret_val);
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

      /* Nothing to do other than to clean up the Bluetopia Platform    */
      /* Manager Service and flag that it is no longer initialized.     */
      BTPM_Cleanup();

      Initialized    = FALSE;
      DEVMCallbackID = 0;

      ret_val        = 0;
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

                  for(Index=0;Index<Result;Index++)
                  {
                     BD_ADDRToStr(BD_ADDRList[Index], Buffer);

                     printf("%2d. %s\r\n", (Index+1), Buffer);
                  }
               }

               /* Flag success.                                         */
               ret_val = 0;
            }
            else
            {
               /* Error attempting to query Remote Device List, inform  */
               /* the user and flag an error.                           */
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
      if((TempParam) && (TempParam->NumberofParameters) && (TempParam->Params[0].strParam) && (strlen(TempParam->Params[0].strParam)))
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
      if((TempParam) && (TempParam->NumberofParameters) && (TempParam->Params[0].strParam) && (strlen(TempParam->Params[0].strParam)))
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
      if((TempParam) && (TempParam->NumberofParameters) && (TempParam->Params[0].strParam) && (strlen(TempParam->Params[0].strParam)))
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
      if((TempParam) && (TempParam->NumberofParameters) && (TempParam->Params[0].strParam) && (strlen(TempParam->Params[0].strParam)))
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
      if((TempParam) && (TempParam->NumberofParameters) && (TempParam->Params[0].strParam) && (strlen(TempParam->Params[0].strParam)))
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
      if((TempParam) && (TempParam->NumberofParameters) && (TempParam->Params[0].strParam) && (strlen(TempParam->Params[0].strParam)))
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
      if((TempParam) && (TempParam->NumberofParameters > 1) && (TempParam->Params[0].strParam) && (strlen(TempParam->Params[0].strParam)))
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
                  if((Result) && (ServiceData))
                  {
                     printf("Returned Service Data (%d Bytes):\r\n", Result);

                     for(Index=0;Index<Result;Index++)
                        printf("%02X", ServiceData[Index]);

                     printf("\r\n");

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
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for Registering an AUDM Data*/
   /* Event Callback. This function returns zero if successful and a    */
   /* negative value if an error occurred.                              */
static int AUDRegisterAudioEndPoint(ParameterList_t *TempParam)
{
   int               Result;
   int               ret_val;
   int               CallbackID;
   AudioStream_t    *StreamState;
   AUD_Stream_Type_t StreamType;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 1) && (TempParam->Params[0].intParam <= 1))
      {
         /* Parameters appear to be valid, determine the requested      */
         /* Stream Type.                                                */
         if(TempParam->Params[0].intParam)
         {
            StreamType  = astSNK;
            StreamState = &AudioSinkState;
         }
         else
         {
            StreamType  = astSRC;
            StreamState = &AudioSourceState;
         }

         /* Check to see if there is already a Callback Registered for  */
         /* this Stream Type.                                           */
         CallbackID = StreamState->CallbackID;

         if(CallbackID <= 0)
         {
            if((Result = AUDM_Register_Data_Event_Callback(StreamType, AUDM_Event_Callback, StreamState)) > 0)
            {
               printf("AUDM_Register_Data_Event_Callback() Success: %d.\r\n", Result);

               printf("Audio %s Callback Registered.\r\n", ((StreamType == astSRC)?"Source":"Sink"));

               StreamState->CallbackID = Result;

               /* Flag success.                                         */
               ret_val                 = 0;
            }
            else
            {
               /* Error Registering for Authentication, inform the user */
               /* and flag an error.                                    */
               printf("AUDM_Register_Data_Event_Callback() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            printf("Audio %s Callback already registered.\r\n", ((StreamType == astSRC)?"Source":"Sink"));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: AUDRegisterAudioEndPoint [Local Stream Type (0 = Source, 1 = Sink)].\r\n");

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

   /* The following function is responsible for Un-Registering an       */
   /* existing AUDM Data Event Callback for either the Audio Source or  */
   /* Audio Sink role. This function returns zero if successful and a   */
   /* negative value if an error occurred.                              */
static int AUDUnRegisterAudioEndPoint(ParameterList_t *TempParam)
{
   int ret_val;
   int CallbackID;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 1) && (TempParam->Params[0].intParam <= 1))
      {
         /* Parameters appear to be valid.                              */

         /* This API call cannot fail, so identify the appropriate      */
         /* Callback ID, then go ahead and clear out the existing ID.   */
         if(TempParam->Params[0].intParam)
         {
            CallbackID                = AudioSinkState.CallbackID;
            AudioSinkState.CallbackID = 0;
         }
         else
         {
            CallbackID                  = AudioSourceState.CallbackID;
            AudioSourceState.CallbackID = 0;
         }

         /* Check whether the Callback was actually registered.         */
         if(CallbackID > 0)
         {
            /* Perform the actual un-registration.                      */
            AUDM_Un_Register_Data_Event_Callback(CallbackID);

            printf("AUDM_Un_Register_Data_Event_Callback() Success.\r\n");
         }
         else
            printf("AUDM_Un_Register_Data_Event_Callback() Failure: Callback not registered.\r\n");

         /* Flag success.                                               */
         ret_val = 0;
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: AUDUnRegisterAudioEndPoint [Local Stream Type (0 = Source, 1 = Sink)].\r\n");

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

   /* The following function is responsible for Registering an AUDM     */
   /* Remote Control Event Callback for either the Controller or Target */
   /* role. This function returns zero if successful and a negative     */
   /* value if an error occurred.                                       */
static int AUDRegisterRemoteControl(ParameterList_t *TempParam)
{
   int ret_val;
   int Result;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 1) && (TempParam->Params[0].intParam <= 1))
      {
         /* Parameters appear to be valid, determine which Service Type */
         /* was requested.                                              */
         if(TempParam->Params[0].intParam)
         {
            /* "Controller" Service Type.                               */

            /* Make sure we're not already registered.                  */
            if(!AUDMRCControllerCallbackID)
            {
               Result = AUDM_Register_Remote_Control_Event_Callback(AUDM_REGISTER_REMOTE_CONTROL_DATA_SERVICE_TYPE_CONTROLLER, AUDM_Event_Callback, NULL);
               if(Result > 0)
                  AUDMRCControllerCallbackID = Result;
            }
            else
            {
               printf("Remote Control Controller Callback already registered.\r\n");

               Result = 0;
            }
         }
         else
         {
            /* "Target" Service Type.                                   */

            /* Make sure we're not already registered.                  */
            if(!AUDMRCTargetCallbackID)
            {
               Result = AUDM_Register_Remote_Control_Event_Callback(AUDM_REGISTER_REMOTE_CONTROL_DATA_SERVICE_TYPE_TARGET, AUDM_Event_Callback, NULL);
               if(Result > 0)
                  AUDMRCTargetCallbackID = Result;
            }
            else
            {
               printf("Remote Control Target Callback already registered.\r\n");

               Result = 0;
            }
         }

         if(Result > 0)
         {
            printf("AUDM_Register_Remote_Control_Event_Callback() Success: %d.\r\n", Result);

            /* Flag success.                                            */
            ret_val = 0;
         }
         else
         {
            /* Error Registering for Authentication, inform the user and*/
            /* flag an error.                                           */
            printf("AUDM_Register_Remote_Control_Event_Callback() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: AUDRegisterRemoteControl [Service Type (0 = Target, 1 = Controller)].\r\n");

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

   /* The following function is responsible for Un-Registering an       */
   /* existing AUDM Remote Control Event Callback for either the        */
   /* Controller or Target role. This function returns zero if          */
   /* successful and a negative value if an error occurred.             */
static int AUDUnRegisterRemoteControl(ParameterList_t *TempParam)
{
   int          ret_val;
   unsigned int CallbackID;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 1) && (TempParam->Params[0].intParam <= 1))
      {
         /* Parameters appear to be valid, map the specified parameters */
         /* into the API specific parameters.                           */
         if(TempParam->Params[0].intParam)
         {
            CallbackID                 = AUDMRCControllerCallbackID;
            AUDMRCControllerCallbackID = 0;
         }
         else
         {
            CallbackID             = AUDMRCTargetCallbackID;
            AUDMRCTargetCallbackID = 0;
         }

         if(CallbackID > 0)
         {
            AUDM_Un_Register_Remote_Control_Event_Callback(CallbackID);

            printf("AUDM_Un_Register_Remote_Control_Event_Callback() Success.\r\n");
         }
         else
            printf("AUDM_Un_Register_Remote_Control_Event_Callback() Failure: Callback not registered.\r\n");

         /* Flag success.                                               */
         ret_val = 0;
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: AUDUnRegisterRemoteControl [Service Type (0 = Target, 1 = Controller)].\r\n");

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

   /* The following function is responsible for Initiating an Audio     */
   /* (A2DP) Connection with a Remote Endpoint (Sink or Source). This   */
   /* function returns zero if successful and a negative value if an    */
   /* error occurred.                                                   */
static int AUDConnectAudio(ParameterList_t *TempParam)
{
   int               ret_val;
   int               Result;
   void             *CallbackParameter;
   BD_ADDR_t         RemoteDeviceAddress;
   unsigned long     StreamFlags;
   AUD_Stream_Type_t StreamType;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && ((TempParam->NumberofParameters == 2) || ((TempParam->NumberofParameters >= 3) && (TempParam->Params[2].intParam <= 3))) && (TempParam->Params[1].intParam <= 1) && (TempParam->Params[0].strParam))
      {
         /* Parameters appear to be valid, map the specified parameters */
         /* into the API specific parameters.                           */
         StrToBD_ADDR(TempParam->Params[0].strParam, &RemoteDeviceAddress);
         StreamType = ((TempParam->Params[1].intParam == 0)?astSRC:astSNK);

         if(TempParam->NumberofParameters == 3)
            StreamFlags = (TempParam->Params[2].intParam);
         else
            StreamFlags = 0;

         if(StreamType == astSNK)
            CallbackParameter = &AudioSinkState;
         else
            CallbackParameter = &AudioSourceState;

         Result = AUDM_Connect_Audio_Stream(RemoteDeviceAddress, StreamType, StreamFlags, AUDM_Event_Callback, CallbackParameter, NULL);

         if(!Result)
         {
            printf("AUDM_Connect_Audio_Stream() Success: %d.\r\n", Result);

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            /* Error in the API call. Inform the user and flag an error.*/
            printf("AUDM_Connect_Audio_Stream() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: AUDConnectAudio [BD_ADDR] [Local Stream Type (0 = Source, 1 = Sink)] [Flags (Optional - 1:Authentication, 2:Encryption)].\r\n");

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

   /* The following function is responsible for Disconnecting an        */
   /* established Audio (A2DP) connection. This function returns zero if*/
   /* successful and a negative value if an error occurred.             */
static int AUDDisconnectAudio(ParameterList_t *TempParam)
{
   int       ret_val;
   int       Result;
   BD_ADDR_t BD_ADDR;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 2) && (TempParam->Params[0].strParam) && (TempParam->Params[1].intParam <= 1))
      {
         StrToBD_ADDR(TempParam->Params[0].strParam, &BD_ADDR);
         Result = AUDM_Disconnect_Audio_Stream(BD_ADDR, ((TempParam->Params[1].intParam == 0)?astSRC:astSNK));

         if(!Result)
         {
            printf("AUDM_Disconnect_Audio_Stream() Success: %d.\r\n", Result);

            /* Flag success to the caller.                              */
            ret_val = 0;

            /* Note the change in state and inform the Audio Thread if  */
            /* it is playing.                                           */
            AudioSourceState.StreamState = astStreamStopped;

            BTPS_SetEvent(AudioSourceState.StreamStateChanged);
         }
         else
         {
            /* Error in the API call. Inform the user and flag an error.*/
            printf("AUDM_Disconnect_Audio_Stream() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: AUDDisconnectAudio [BD_ADDR] [Local Stream Type (0 = Source, 1 = Sink)].\r\n");

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

   /* The following function is responsible for Responding to a         */
   /* Connection Request from a Remote Device for either Audio or Remote*/
   /* Control connections. This function returns zero if successful and */
   /* a negative value if an error occurred.                            */
static int AUDConnectionRequestResponse(ParameterList_t *TempParam)
{
   int                           Result;
   int                           ret_val;
   Boolean_t                     Accept;
   BD_ADDR_t                     RemoteDeviceAddress;
   AUD_Connection_Request_Type_t RequestType;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 3) && (TempParam->Params[0].intParam <= 1) && (TempParam->Params[1].strParam) && (TempParam->Params[2].intParam <= 1))
      {
         RequestType = ((TempParam->Params[0].intParam == 0)?acrStream:acrRemoteControl);
         Accept      = ((TempParam->Params[2].intParam == 0)?FALSE:TRUE);

         StrToBD_ADDR(TempParam->Params[1].strParam, &RemoteDeviceAddress);

         Result = AUDM_Connection_Request_Response(RequestType, RemoteDeviceAddress, Accept);

         if(!Result)
         {
            printf("AUDM_Connection_Request_Response() Success: %d.\r\n", Result);

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            /* Error in the API call. Inform the user and flag an error.*/
            printf("AUDM_Connection_Request_Response() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: AUDConnectionRequestResponse [Request Type (0 = Audio, 1 = Remote Control)] [BD_ADDR] [Accept? (0/1)].\r\n");

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

   /* The following function is responsible for changing the incoming   */
   /* Audio Manager Connection Flags.  This function returns zero if    */
   /* successful and a negative value if an error occurred.             */
static int AUDChangeIncomingFlags(ParameterList_t *TempParam)
{
   int ret_val;
   int Result;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 1))
      {
         /* Parameters appear to be valid, map the specified parameters */
         /* into the API specific parameters.                           */
         Result = AUDM_Change_Incoming_Connection_Flags((unsigned long)(TempParam->Params[0].intParam));

         if(!Result)
         {
            printf("AUDM_Change_Incoming_Connection_Flags() Success: %d.\r\n", Result);

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            /* Error in the API call. Inform the user and flag an error.*/
            printf("AUDM_Change_Incoming_Connection_Flags() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: AUDChangeIncomingFlags [Flags (1: Authorization, 2: Authentication, 4: Encryption)].\r\n");

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

   /* The following function is responsible for Loading and Playing a   */
   /* WAV-format audio file over an established connection to an Audio  */
   /* Sink. This function returns zero if successful and a negative     */
   /* value if an error occurred.                                       */
static int AUDPlayWAV(ParameterList_t *TempParam)
{
   int                     Result;
   int                     ret_val;
   int                     FileDescriptor;
   int                     CallbackID;
   BD_ADDR_t               RemoteDeviceAddress;
   WAVInfo_t               WAVInfo;
   AUD_Stream_Format_t     StreamFormat;
   PlaybackThreadParams_t *PlaybackThreadParams;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      if(COMPARE_NULL_BD_ADDR(CurrentEncodingBD_ADDR))
      {
         /* Make sure that all of the parameters required for this      */
         /* function appear to be at least semi-valid.                  */
         if((TempParam) && (TempParam->NumberofParameters >= 2) && (TempParam->Params[0].strParam) && (TempParam->Params[1].strParam))
         {
            StrToBD_ADDR(TempParam->Params[0].strParam, &RemoteDeviceAddress);

            /* Before attempting to open the specified WAV file,        */
            /* determine if there is an Audio Source Endpoint registered*/
            /* with this client.                                        */
            CallbackID = AudioSourceState.CallbackID;

            if(CallbackID > 0)
            {
               errno = 0;

               if((FileDescriptor = open((TempParam->Params[1].strParam), (O_RDONLY))) >= 0)
               {
                  if(ParseWAVHeader(FileDescriptor, &WAVInfo) == 0)
                  {
                     printf("Parsed WAV File:\r\n");
                     printf("    AudioDataOffset:       %ld\r\n", (long)WAVInfo.AudioDataOffset);
                     printf("    AudioDataLength:       %ld\r\n", (long)WAVInfo.AudioDataLength);
                     printf("    Format:                %hd\r\n", WAVInfo.Format);
                     printf("    Channels:              %hd\r\n", WAVInfo.Channels);
                     printf("    SamplesPerSecond:      %ld\r\n", (long)WAVInfo.SamplesPerSecond);
                     printf("    AverageBytesPerSecond: %ld\r\n", (long)WAVInfo.AverageBytesPerSecond);
                     printf("    BlockSize:             %hd\r\n", WAVInfo.BlockSize);
                     printf("    BitsPerSample:         %hd\r\n", WAVInfo.BitsPerSample);
                     printf("    ValidBitsPerSample:    %hd\r\n", WAVInfo.ValidBitsPerSample);
                     printf("\r\n");

                     if((WAVInfo.Format == WAVE_FORMAT_PCM) && (WAVInfo.BitsPerSample == 16))
                     {
                        /* WAV file appears good, now check the active  */
                        /* outgoing audio connection.                   */
                        if((Result = AUDM_Query_Audio_Stream_Format(RemoteDeviceAddress, astSRC, &StreamFormat)) == 0)
                        {
                           if((StreamFormat.SampleFrequency == WAVInfo.SamplesPerSecond) && (StreamFormat.NumberChannels == WAVInfo.Channels))
                           {
                              if(((Result = AUDM_Change_Audio_Stream_State(RemoteDeviceAddress, astSRC, astStreamStarted)) == 0) || (Result == BTPM_ERROR_CODE_AUDIO_STREAM_STATE_IS_ALREADY_STARTED))
                              {
                                 if((PlaybackThreadParams = (PlaybackThreadParams_t *)malloc(sizeof(PlaybackThreadParams_t))) != NULL)
                                 {
                                    /* Initialize the audio playback    */
                                    /* thread.                          */
                                    PlaybackThreadParams->CallbackID     = CallbackID;
                                    PlaybackThreadParams->RemoteDevice   = RemoteDeviceAddress;
                                    PlaybackThreadParams->FileDescriptor = dup(FileDescriptor);
                                    PlaybackThreadParams->WAVInfo        = WAVInfo;

                                    /* Check to see if an offset was    */
                                    /* specified.                       */
                                    if(TempParam->NumberofParameters > 2)
                                       PlaybackThreadParams->Offset = (TempParam->Params[2].intParam)?FileOffset:0;
                                    else
                                       PlaybackThreadParams->Offset = 0;

                                    /* Check to see if we should loop. */
                                    if(TempParam->NumberofParameters > 3)
                                       PlaybackThreadParams->Loop = (Boolean_t)(TempParam->Params[3].intParam);
                                    else
                                       PlaybackThreadParams->Loop = FALSE;

                                    printf("Starting Playback Thread.\r\n");

                                    if(BTPS_CreateThread(PlaybackThreadMain, 16384, PlaybackThreadParams))
                                    {
                                       /* Flag success to the caller.   */
                                       ret_val = 0;

                                       CurrentEncodingBD_ADDR = RemoteDeviceAddress;
                                    }
                                    else
                                    {
                                       printf("Unable to start Playback Thread.\r\n");

                                       close(PlaybackThreadParams->FileDescriptor);
                                       free(PlaybackThreadParams);

                                       ret_val = FUNCTION_ERROR;
                                    }
                                 }
                                 else
                                 {
                                    printf("Unable to allocate memory for Playback Thread creation.\r\n");

                                    ret_val = FUNCTION_ERROR;
                                 }
                              }
                              else
                              {
                                 printf("Unable to begin playback: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

                                 ret_val = FUNCTION_ERROR;
                              }
                           }
                           else
                           {
                              printf("Format mismatch: WAV audio does not match stream format:\r\n");
                              printf("    Frequency: %lu\r\n", StreamFormat.SampleFrequency);
                              printf("    Channels : %u\r\n",  StreamFormat.NumberChannels);

                              ret_val = FUNCTION_ERROR;
                           }
                        }
                        else
                        {
                           printf("AUDM_Query_Audio_Stream_Format() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

                           ret_val = FUNCTION_ERROR;
                        }
                     }
                     else
                     {
                        if(WAVInfo.Format != WAVE_FORMAT_PCM)
                           printf("WAV file audio data is not in the PCM format.\r\n");
                        else
                           printf("WAV file must contain 16-bit PCM samples for A2DP/SBC.\r\n");

                        ret_val = FUNCTION_ERROR;
                     }
                  }
                  else
                  {
                     printf("Unable to parse WAV file header.\r\n");

                     ret_val = FUNCTION_ERROR;
                  }

                  /* We're done with out copy of the file descriptor,   */
                  /* now, so close it.                                  */
                  close(FileDescriptor);
               }
               else
               {
                  printf("Unable to open WAV file (errno: %d, %s).\r\n", errno, strerror(errno));

                  ret_val = FUNCTION_ERROR;
               }
            }
            else
            {
               printf("Register an Audio Source Callback, first. Example:\r\n    AUDRegisterAudioEndPoint 0\r\n");

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* One or more of the necessary parameters is/are invalid.  */
            printf("Usage: AUDPlayWAV [BD_ADDR] [WAV File Name] [Restart/Resume - 0/1 (Optional)] [Loop - 1=TRUE (Optional)].\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         printf("Audio playback already in progress on another stream.\r\n");

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

   /* The following function is responsible for Querying the current    */
   /* State of an Audio Stream, including the Stream's State, Format,   */
   /* and Connection Status. This function returns zero if successful   */
   /* and a negative value if an error occurred.                        */
static int AUDGetStreamStatus(ParameterList_t *TempParam)
{
   int                 Result;
   int                 ret_val;
   BD_ADDR_t           RemoteDeviceAddress;
   AUD_Stream_Type_t   StreamType;
   AUD_Stream_State_t  StreamState;
   AUD_Stream_Format_t StreamFormat;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 2) && (TempParam->Params[0].strParam) && (TempParam->Params[1].intParam <= 1))
      {
         StrToBD_ADDR(TempParam->Params[0].strParam, &RemoteDeviceAddress);
         StreamType = ((TempParam->Params[1].intParam == 0)?astSRC:astSNK);

         /* The stream is connected. Next, get the stream info.         */
         if((Result = AUDM_Query_Audio_Stream_State(RemoteDeviceAddress, StreamType, &StreamState)) == 0)
         {
            printf("AUDM_Query_Audio_Stream_State() Success: %d.\r\n", Result);

            if((Result = AUDM_Query_Audio_Stream_Format(RemoteDeviceAddress, StreamType, &StreamFormat)) == 0)
            {
               printf("AUDM_Query_Audio_Stream_Format() Success: %d.\r\n", Result);

               printf("    Stream   : Connected\r\n");
               printf("    Address  : %s\r\n",  TempParam->Params[0].strParam);
               printf("    State    : %s\r\n",  (StreamState == astStreamStopped)?"Stopped":"Playing");
               printf("    Frequency: %lu\r\n", StreamFormat.SampleFrequency);
               printf("    Channels : %u\r\n",  StreamFormat.NumberChannels);
               printf("    Flags    : %lu\r\n", StreamFormat.FormatFlags);

               /* Flag success to the caller.                           */
               ret_val = 0;
            }
            else
            {
               /* Error in the API call.  Inform the user and flag an   */
               /* error.                                                */
               printf("AUDM_Query_Audio_Stream_Format() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* Error in the API call.  Inform the user and flag an      */
            /* error.                                                   */
            printf("AUDM_Query_Audio_Stream_State() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: AUDGetStreamStatus [BD_ADDR] [Local Stream Type (0 = Source, 1 = Sink)].\r\n");

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

   /* The following function is responsible for querying the list of    */
   /* connected audio devices.  This function returns zero if successful*/
   /* and a negative value if an error occurred.                        */
static int AUDQueryConnectedAudioDevices(ParameterList_t *TempParam)
{
   int           ret_val;
   int           Result;
   char          Buffer[32];
   BD_ADDR_t    *RemoteDeviceAddressList;
   unsigned int  TotalConnected;
   unsigned int  Index;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      if((TempParam) && (TempParam->NumberofParameters >= 1) && (TempParam->Params[0].intParam <= 1))
      {
         /* First query the number of connected devices.                */
         if((Result = AUDM_Query_Audio_Connected_Devices((TempParam->Params[0].intParam == 0)?astSRC:astSNK, 0, NULL, &TotalConnected)) >= 0)
         {
            /* Allocate memory for the entire list of connected audio   */
            /* devices.                                                 */
            if((TotalConnected) && ((RemoteDeviceAddressList = BTPS_AllocateMemory(TotalConnected*sizeof(BD_ADDR_t))) != NULL))
            {
               /* Get the list of connected devices.                    */
               if((Result = AUDM_Query_Audio_Connected_Devices((TempParam->Params[0].intParam == 0)?astSRC:astSNK, TotalConnected, RemoteDeviceAddressList, &TotalConnected)) >= 0)
               {
                  printf("Total Connected Devices: %u.\r\n", TotalConnected);

                  for(Index=0;Index<(unsigned int)Result;Index++)
                  {
                     BD_ADDRToStr(RemoteDeviceAddressList[Index], Buffer);

                     printf("%u: %s.\r\n", (Index+1), Buffer);
                  }

                  /* Return success to the caller.                      */
                  ret_val = 0;
               }
               else
               {
                  /* Error in the API call.  Inform the user and flag an*/
                  /* error.                                             */
                  printf("AUDM_Query_Audio_Connected_Devices() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

                  ret_val = FUNCTION_ERROR;
               }

               /* Free the allocate memory.                             */
               BTPS_FreeMemory(RemoteDeviceAddressList);
            }
            else
            {
               if(TotalConnected)
               {
                  printf("%s", "Failed to allocate memory\r\n");

                  ret_val = FUNCTION_ERROR;
               }
               else
               {
                  printf("%s", "Total Connected Remote Controls: 0.\r\n");

                  ret_val = 0;
               }
            }
         }
         else
         {
            /* Error in the API call.  Inform the user and flag an      */
            /* error.                                                   */
            printf("AUDM_Query_Audio_Connected_Devices() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: AUDQueryConnectedAudioDevices [Local Stream Type (0 = Source, 1 = Sink)].\r\n");

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

   /* The following function is responsible for querying the current    */
   /* Stream State of an existing Audio connection.  This function      */
   /* returns zero if successful and a negative value if an error       */
   /* occurred.                                                         */
static int AUDQueryStreamState(ParameterList_t *TempParam)
{
   int                Result;
   int                ret_val;
   BD_ADDR_t          BD_ADDR;
   AUD_Stream_Type_t  StreamType;
   AUD_Stream_State_t StreamState;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 2) && (TempParam->Params[0].strParam) && (TempParam->Params[1].intParam <= 1))
      {
         /* Note the specified parameters.                              */
         StrToBD_ADDR(TempParam->Params[0].strParam, &BD_ADDR);
         StreamType = ((TempParam->Params[1].intParam == 0)?astSRC:astSNK);

         Result = AUDM_Query_Audio_Stream_State(BD_ADDR, StreamType, &StreamState);

         if(!Result)
         {
            printf("AUDM_Query_Audio_Stream_State() Success: %d.\r\n", Result);

            printf("    State: %s\r\n",  (StreamState == astStreamStopped)?"Stopped":"Playing");

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            /* Error in the API call. Inform the user and flag an error.*/
            printf("AUDM_Query_Audio_Stream_State() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: AUDQueryStreamState [BD_ADDR] [Local Stream Type (0 = Source, 1 = Sink)].\r\n");

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

   /* The following function is responsible for Setting the Stream State*/
   /* of an existing Audio connection. This function returns zero if    */
   /* successful and a negative value if an error occurred.             */
static int AUDSetStreamState(ParameterList_t *TempParam)
{
   int                Result;
   int                ret_val;
   BD_ADDR_t          BD_ADDR;
   AUD_Stream_Type_t  StreamType;
   AUD_Stream_State_t StreamState;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 3) && (TempParam->Params[0].strParam) && (TempParam->Params[1].intParam <= 1) && (TempParam->Params[2].intParam <= 1))
      {
         StrToBD_ADDR(TempParam->Params[0].strParam, &BD_ADDR);
         StreamType  = ((TempParam->Params[1].intParam == 0)?astSRC:astSNK);
         StreamState = ((TempParam->Params[2].intParam == 0)?astStreamStopped:astStreamStarted);

         Result = AUDM_Change_Audio_Stream_State(BD_ADDR, StreamType, StreamState);

         if(!Result)
         {
            printf("AUDM_Change_Audio_Stream_State() Success: %d.\r\n", Result);

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            /* Error in the API call. Inform the user and flag an error.*/
            printf("AUDM_Change_Audio_Stream_State() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: AUDSetStreamState [BD_ADDR] [Local Stream Type (0 = Source, 1 = Sink)] [Stream State (0 = Stop, 1 = Start)].\r\n");

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

   /* The following function is responsible for querying the current    */
   /* Stream Format of an existing Audio connection.  This function     */
   /* returns zero if successful and a negative value if an error       */
   /* occurred.                                                         */
static int AUDQueryStreamFormat(ParameterList_t *TempParam)
{
   int                 Result;
   int                 ret_val;
   BD_ADDR_t           BD_ADDR;
   AUD_Stream_Type_t   StreamType;
   AUD_Stream_Format_t StreamFormat;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 2) && (TempParam->Params[0].strParam) && (TempParam->Params[1].intParam <= 1))
      {
         /* Note the specified parameters.                              */
         StrToBD_ADDR(TempParam->Params[0].strParam, &BD_ADDR);
         StreamType = ((TempParam->Params[1].intParam == 0)?astSRC:astSNK);

         Result = AUDM_Query_Audio_Stream_Format(BD_ADDR, StreamType, &StreamFormat);

         if(!Result)
         {
            printf("AUDM_Query_Audio_Stream_Format(%d) Success: %d.\r\n", StreamType, Result);

            printf("    Frequency: %lu\r\n", StreamFormat.SampleFrequency);
            printf("    Channels : %u\r\n", StreamFormat.NumberChannels);
            printf("    Flags    : %lu\r\n", StreamFormat.FormatFlags);

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            /* Error in the API call. Inform the user and flag an error.*/
            printf("AUDM_Query_Audio_Stream_Format() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: AUDQueryStreamFormat [BD_ADDR] [Local Stream Type (0 = Source, 1 = Sink)].\r\n");

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

   /* The following function is responsible for Setting the Stream      */
   /* Format of an existing Audio connection. This function returns zero*/
   /* if successful and a negative value if an error occurred.          */
static int AUDSetStreamFormat(ParameterList_t *TempParam)
{
   int                 Result;
   int                 ret_val;
   BD_ADDR_t           BD_ADDR;
   AUD_Stream_Type_t   StreamType;
   AUD_Stream_Format_t StreamFormat;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 4) && (TempParam->Params[0].strParam) && (TempParam->Params[1].intParam <= 1))
      {
         /* Note the specified parameters.                              */
         StrToBD_ADDR(TempParam->Params[0].strParam, &BD_ADDR);

         StreamType                   = ((TempParam->Params[1].intParam == 0)?astSRC:astSNK);

         StreamFormat.SampleFrequency = (unsigned long)(TempParam->Params[2].intParam);
         StreamFormat.NumberChannels  = (unsigned int)(TempParam->Params[3].intParam);
         StreamFormat.FormatFlags     = (unsigned long)0;

         Result = AUDM_Change_Audio_Stream_Format(BD_ADDR, StreamType, &StreamFormat);

         if(!Result)
         {
            printf("AUDM_Change_Audio_Stream_Format() Success: %d.\r\n", Result);

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            /* Error in the API call. Inform the user and flag an error.*/
            printf("AUDM_Change_Audio_Stream_Format() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: AUDSetStreamFormat [BD_ADDR] [Local Stream Type (0 = Source, 1 = Sink)] [Sample Rate] [Number Channels].\r\n");

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

   /* The following function is responsible for querying the current    */
   /* Stream Configuration of an existing Audio connection.  This       */
   /* function returns zero if successful and a negative value if an    */
   /* error occurred.                                                   */
static int AUDQueryStreamConfiguration(ParameterList_t *TempParam)
{
   int                        Result;
   int                        ret_val;
   BD_ADDR_t                  BD_ADDR;
   unsigned int               Index;
   AUD_Stream_Type_t          StreamType;
   AUD_Stream_Configuration_t StreamConfiguration;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 2) && (TempParam->Params[0].strParam) && (TempParam->Params[1].intParam <= 1))
      {
         StrToBD_ADDR(TempParam->Params[0].strParam, &BD_ADDR);
         StreamType  = ((TempParam->Params[1].intParam == 0)?astSRC:astSNK);

         Result = AUDM_Query_Audio_Stream_Configuration(BD_ADDR, StreamType, &StreamConfiguration);

         if(!Result)
         {
            printf("AUDM_Query_Audio_Stream_Configuration() Success: %d.\r\n", Result);

            printf("    Media MTU        : %u\r\n", StreamConfiguration.MediaMTU);
            printf("    Frequency        : %lu\r\n", StreamConfiguration.StreamFormat.SampleFrequency);
            printf("    Channels         : %u\r\n", StreamConfiguration.StreamFormat.NumberChannels);
            printf("    Flags            : %lu\r\n", StreamConfiguration.StreamFormat.FormatFlags);
            printf("    Codec Type       : %u\r\n", StreamConfiguration.MediaCodecType);
            printf("    Codec Info Length: %u\r\n", StreamConfiguration.MediaCodecInfoLength);
            printf("    Codec Information:");

            for(Index=0;Index<StreamConfiguration.MediaCodecInfoLength;Index++)
               printf(" 0x%02X", StreamConfiguration.MediaCodecInformation[Index]);

            printf("\r\n");

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            /* Error in the API call. Inform the user and flag an error.*/
            printf("AUDM_Query_Audio_Stream_Configuration() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: AUDQueryStreamConfiguration [BD_ADDR] [Local Stream Type (0 = Source, 1 = Sink)].\r\n");

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

   /* The following function is responsible for Sending a Remote Control*/
   /* (AVRCP) Pass-Through Command to a connected AVRCP Target. This    */
   /* function returns zero if successful and a negative value if an    */
   /* error occurred.                                                   */
static int AUDSendRemoteControlPassThroughCommand(ParameterList_t *TempParam)
{
   int                               Result;
   int                               ret_val;
   BD_ADDR_t                         RemoteDevice;
   unsigned int                      Index;
   unsigned long                     Timeout;
   AUD_Remote_Control_Command_Data_t CommandData;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      if(AUDMRCControllerCallbackID > 0)
      {
         /* Make sure that all of the parameters required for this      */
         /* function appear to be at least semi-valid.                  */
         if((TempParam) && (TempParam->NumberofParameters >= 3) && (TempParam->Params[0].strParam) && (TempParam->Params[2].intParam <= NUMBER_PASSTHROUGH_COMMANDS))
         {
            StrToBD_ADDR(TempParam->Params[0].strParam, &RemoteDevice);

            Timeout = TempParam->Params[1].intParam;

            /* Determine the Pass-through Operation that was specified. */
            Result = 0;

            if(TempParam->Params[2].intParam < NUMBER_PASSTHROUGH_COMMANDS)
            {
               CommandData.MessageData.PassThroughCommandData.OperationID = PassThroughCommandTable[TempParam->Params[2].intParam].OperationID;
               printf("%s Pass-through Command specified.\r\n", PassThroughCommandTable[TempParam->Params[2].intParam].Display);
            }
            else
            {
               /* Specify Command.                                      */
               if((TempParam->NumberofParameters >= 4) && (TempParam->Params[3].intParam) && (TempParam->Params[3].intParam != 0xFF))
               {
                  CommandData.MessageData.PassThroughCommandData.OperationID = (Byte_t)TempParam->Params[3].intParam;

                  printf("Specific Pass-through Command specified: 0x%02X.\r\n", CommandData.MessageData.PassThroughCommandData.OperationID);
               }
               else
                  Result = INVALID_PARAMETERS_ERROR;
            }

            if(!Result)
            {
               /* Format up a Pass-through Command. With the StateFlag  */
               /* member set to FALSE, this will be the Button PRESSED  */
               /* event.                                                */
               CommandData.MessageType                                            = amtPassThrough;
               CommandData.MessageData.PassThroughCommandData.CommandType         = AVRCP_CTYPE_CONTROL;
               CommandData.MessageData.PassThroughCommandData.SubunitType         = AVRCP_SUBUNIT_TYPE_PANEL;
               CommandData.MessageData.PassThroughCommandData.SubunitID           = AVRCP_SUBUNIT_ID_INSTANCE_0;
               CommandData.MessageData.PassThroughCommandData.StateFlag           = FALSE;
               CommandData.MessageData.PassThroughCommandData.OperationDataLength = 0;
               CommandData.MessageData.PassThroughCommandData.OperationData       = NULL;

               if((Result = AUDM_Send_Remote_Control_Command(AUDMRCControllerCallbackID, RemoteDevice, Timeout, &CommandData)) >= 0)
               {
                  printf("Remote Control Command Sent: %s, Transaction ID = %d.\r\n", ((CommandData.MessageData.PassThroughCommandData.StateFlag == FALSE)?"PRESSED":"RELEASED"), Result);

                  /* Switch StateFlag to TRUE so we can send the Button */
                  /* RELEASED event.                                    */
                  CommandData.MessageData.PassThroughCommandData.StateFlag = TRUE;

                  if((Result = AUDM_Send_Remote_Control_Command(AUDMRCControllerCallbackID, RemoteDevice, Timeout, &CommandData)) >= 0)
                  {
                     printf("Remote Control Command Sent: %s, Transaction ID = %d.\r\n", ((CommandData.MessageData.PassThroughCommandData.StateFlag == FALSE)?"PRESSED":"RELEASED"), Result);

                     /* Flag success to the caller.                     */
                     ret_val = 0;
                  }
                  else
                  {
                     /* Error in the API call. Inform the user and flag */
                     /* an error.                                       */
                     printf("AUDM_Send_Remote_Control_Command() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

                     ret_val = FUNCTION_ERROR;
                  }
               }
               else
               {
                  /* Error in the API call. Inform the user and flag an */
                  /* error.                                             */
                  printf("AUDM_Send_Remote_Control_Command() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

                  ret_val = FUNCTION_ERROR;
               }
            }
            else
            {
               /* One or more of the necessary parameters is/are        */
               /* invalid.                                              */
               printf("Usage: AUDSendRemoteControlPassThroughCommand [BD_ADDR] [Timeout (ms)] [Passthrough Command] [Explicit ID (When Explicit Command ID is selected)].\r\n");
               printf("   Possible Commands:\r\n");

               for(Index = 0; Index < NUMBER_PASSTHROUGH_COMMANDS; Index++)
                  printf("      %2u - %s.\r\n", Index, PassThroughCommandTable[Index].Display);

               printf("      %2u - Explicit Command ID.\r\n", (unsigned int)NUMBER_PASSTHROUGH_COMMANDS);

               ret_val = INVALID_PARAMETERS_ERROR;
            }
         }
         else
         {
            /* One or more of the necessary parameters is/are invalid.  */
            printf("Usage: AUDSendRemoteControlPassThroughCommand [BD_ADDR] [Timeout (ms)] [Passthrough Command] [Explicit ID (When Explicit Command ID is selected)].\r\n");
            printf("   Possible Commands:\r\n");

            for(Index = 0; Index < NUMBER_PASSTHROUGH_COMMANDS; Index++)
               printf("      %2u - %s.\r\n", Index, PassThroughCommandTable[Index].Display);

            printf("      %2u - Explicit Command ID.\r\n", (unsigned int)NUMBER_PASSTHROUGH_COMMANDS);

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         printf("Register a Remote Control Callback (in the Controller Role), first. Example:\r\n    AUDRegisterRemoteControl 1\r\n");

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

   /* The following function is responsible for Initiating an Remote    */
   /* Control connection to a remote device.  This function returns zero*/
   /* if successful and a negative value if an error occurred.          */
static int AUDConnectRemoteControl(ParameterList_t *TempParam)
{
   int            ret_val;
   int            Result;
   BD_ADDR_t      RemoteDeviceAddress;
   unsigned long  ConnectionFlags;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && ((TempParam->NumberofParameters == 1) || ((TempParam->NumberofParameters >= 2) && (TempParam->Params[1].intParam <= 3))) && (TempParam->Params[0].strParam))
      {
         /* Parameters appear to be valid, map the specified parameters */
         /* into the API specific parameters.                           */
         StrToBD_ADDR(TempParam->Params[0].strParam, &RemoteDeviceAddress);

         if(TempParam->NumberofParameters == 2)
            ConnectionFlags = (TempParam->Params[1].intParam);
         else
            ConnectionFlags = 0;

         Result = AUDM_Connect_Remote_Control(RemoteDeviceAddress, ConnectionFlags, AUDM_Event_Callback, NULL, NULL);

         if(!Result)
         {
            printf("AUDM_Connect_Remote_Control() Success: %d, Flags 0x%08X.\r\n", Result, (unsigned int)ConnectionFlags);

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            /* Error in the API call. Inform the user and flag an error.*/
            printf("AUDM_Connect_Remote_Control() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: AUDConnectRemoteControl [BD_ADDR] [Flags (Optional - 1:Authentication, 2:Encryption)].\r\n");

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

   /* The following function is responsible for Disconnecting an        */
   /* established Remote Control connection.  This function returns zero*/
   /* if successful and a negative value if an error occurred.          */
static int AUDDisconnectRemoteControl(ParameterList_t *TempParam)
{
   int       ret_val;
   int       Result;
   BD_ADDR_t RemoteDeviceAddress;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 1) && (TempParam->Params[0].strParam))
      {
         /* Parameters appear to be valid, map the specified parameters */
         /* into the API specific parameters.                           */
         StrToBD_ADDR(TempParam->Params[0].strParam, &RemoteDeviceAddress);

         Result = AUDM_Disconnect_Remote_Control(RemoteDeviceAddress);

         if(!Result)
         {
            printf("AUDM_Disconnect_Remote_Control() Success: %d.\r\n", Result);

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            /* Error in the API call. Inform the user and flag an error.*/
            printf("AUDM_Disconnect_Remote_Control() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: AUDDisconnectRemoteControl [BD_ADDR].\r\n");

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

   /* The following function is responsible for querying the list of    */
   /* connected Remote Control devices.  This function returns zero if  */
   /* successful and a negative value if an error occurred.             */
static int AUDQueryConnectedRemoteControls(ParameterList_t *TempParam)
{
   int           ret_val;
   int           Result;
   char          Buffer[32];
   BD_ADDR_t    *RemoteDeviceAddressList;
   unsigned int  TotalConnected;
   unsigned int  Index;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* First query the number of connected devices.                   */
      if((Result = AUDM_Query_Remote_Control_Connected_Devices(0, NULL, &TotalConnected)) >= 0)
      {
         /* Allocate memory for the entire list of connected remote     */
         /* control devices.                                            */
         if((TotalConnected) && ((RemoteDeviceAddressList = BTPS_AllocateMemory(TotalConnected*sizeof(BD_ADDR_t))) != NULL))
         {
            /* Get the list of connected devices.                       */
            if((Result = AUDM_Query_Remote_Control_Connected_Devices(TotalConnected, RemoteDeviceAddressList, &TotalConnected)) >= 0)
            {
               printf("Total Connected Remote Controls: %u.\r\n", TotalConnected);

               for(Index=0;Index<(unsigned int)Result;Index++)
               {
                  BD_ADDRToStr(RemoteDeviceAddressList[Index], Buffer);

                  printf("%u: %s.\r\n", (Index+1), Buffer);
               }

               /* Return success to the caller.                         */
               ret_val = 0;
            }
            else
            {
               /* Error in the API call.  Inform the user and flag an   */
               /* error.                                                */
               printf("AUDM_Query_Remote_Control_Connected_Devices() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

               ret_val = FUNCTION_ERROR;
            }

            /* Free the allocate memory.                                */
            BTPS_FreeMemory(RemoteDeviceAddressList);
         }
         else
         {
            if(TotalConnected)
            {
               printf("%s", "Failed to allocate memory\r\n");

               ret_val = FUNCTION_ERROR;
            }
            else
            {
               printf("%s", "Total Connected Remote Controls: 0.\r\n");

               ret_val = 0;
            }
         }
      }
      else
      {
         /* Error in the API call.  Inform the user and flag an error.  */
         printf("AUDM_Query_Remote_Control_Connected_Devices() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

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

   /* The following function is responsible for Sending a Remote Control*/
   /* (AVRCP) Get Capabilities Command to a connected AVRCP Target.     */
   /* This function returns zero if successful and a negative value if  */
   /* an error occurred.                                                */
   /* * NOTE * Both the local and remote device must support an AVRCP   */
   /*          Version greater than or equal to 1.3 to use this command.*/
static int AUDSendRemoteControlGetCapabilitiesCommand(ParameterList_t *TempParam)
{
   int                               Result;
   int                               ret_val;
   BD_ADDR_t                         RemoteDevice;
   AUD_Remote_Control_Command_Data_t CommandData;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      if(AUDMRCControllerCallbackID > 0)
      {
         /* Make sure that all of the parameters required for this      */
         /* function appear to be at least semi-valid.                  */
         if((TempParam) && (TempParam->NumberofParameters >= 1) && (TempParam->Params[0].strParam))
         {
            StrToBD_ADDR(TempParam->Params[0].strParam, &RemoteDevice);

            /* Format a Get Capabilities command.                       */
            CommandData.MessageType                                         = amtGetCapabilities;
            CommandData.MessageData.GetCapabilitiesCommandData.CapabilityID = AVRCP_GET_CAPABILITIES_CAPABILITY_ID_EVENTS_SUPPORTED;

            /* Try to Send the Message.                                 */
            if((Result = AUDM_Send_Remote_Control_Command(AUDMRCControllerCallbackID, RemoteDevice, DEFAULT_AVRCP_COMMAND_TIMEOUT, &CommandData)) >= 0)
            {
               printf("Remote Control Command Sent: Transaction ID = %d\r\n", Result);

               /* Flag success to the caller.                           */
               ret_val = 0;
            }
            else
            {
               /* Error in the API call. Inform the user and flag an    */
               /* error.                                                */
               printf("AUDM_Send_Remote_Control_Command() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* One or more of the necessary parameters is/are invalid.  */
            printf("Usage: AUDSendRemoteControlGetCapabilitiesCommand [BD_ADDR]\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         printf("Register a Remote Control Callback (in the Controller Role), first. Example:\r\n    AUDRegisterRemoteControl 1\r\n");

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

   /* The following function is responsible for Sending a Remote Control*/
   /* (AVRCP) Get Capabilities Command to a connected AVRCP Target.     */
   /* This function returns zero if successful and a negative value if  */
   /* an error occurred.                                                */
   /* * NOTE * Both the local and remote device must support an AVRCP   */
   /*          Version greater than or equal to 1.4 to use this command.*/
   /* * NOTE * Per the AVRCP 1.4 Specification, "This command is used to*/
   /*          set an absolute volume to be used by the rendering       */
   /*          device" i.e. this command can only be executed source    */
   /*          devices.  Sink devices can notify the source of volume   */
   /*          changes using notifications if the source has registered */
   /*          for volume changed notifications.                        */
static int AUDSendRemoteControlSetAbsoluteVolumeCommand(ParameterList_t *TempParam)
{
   int                               Result;
   int                               ret_val;
   BD_ADDR_t                         RemoteDevice;
   AUD_Remote_Control_Command_Data_t CommandData;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Check if this a application is registered as an AVRCP          */
      /* Controller.                                                    */
      if(AUDMRCControllerCallbackID > 0)
      {
         /* Only sourcing devices can send absolute volume commands,    */
         /* check if this application is registered as an A2DP source.  */
         if(AudioSourceState.CallbackID)
         {
            /* Make sure that all of the parameters required for this   */
            /* function appear to be at least semi-valid.               */
            if((TempParam) && (TempParam->NumberofParameters >= 2) && (TempParam->Params[0].strParam))
            {
               StrToBD_ADDR(TempParam->Params[0].strParam, &RemoteDevice);

               /* Format a Get Capabilities command.                    */
               CommandData.MessageType                                             = amtSetAbsoluteVolume;
               CommandData.MessageData.SetAbsoluteVolumeCommandData.AbsoluteVolume = (Byte_t)PERCENTAGE_TO_ABSOLUTE_VOLUME(TempParam->Params[1].intParam);

               /* Try to Send the Message.                              */
               if((Result = AUDM_Send_Remote_Control_Command(AUDMRCControllerCallbackID, RemoteDevice, DEFAULT_AVRCP_COMMAND_TIMEOUT, &CommandData)) >= 0)
               {
                  printf("Remote Control Command Sent: Transaction ID = %d\r\n", Result);

                  /* Flag success to the caller.                        */
                  ret_val = 0;
               }
               else
               {
                  /* Error in the API call. Inform the user and flag an */
                  /* error.                                             */
                  printf("AUDM_Send_Remote_Control_Command() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

                  ret_val = FUNCTION_ERROR;
               }
            }
            else
            {
               /* One or more of the necessary parameters is/are        */
               /* invalid.                                              */
               printf("Usage: AUDSendRemoteControlSetAbsoluteVolumeCommand [BD_ADDR] [Volume (%%)]\r\n");

               ret_val = INVALID_PARAMETERS_ERROR;
            }
         }
         else
         {
            printf("Error: Only source devices can send Set Absolute Volume Commands.\r\n"
                   "       To register for source events use: AUDRegisterAudioEndPoint 0\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         printf("Register a Remote Control Callback (in the Controller Role), first. Example:\r\n    AUDRegisterRemoteControl 1\r\n");

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

   /* The following function is responsible for Sending a Remote Control*/
   /* (AVRCP) Register Notification Command to a connected AVRCP Target.*/
   /* This function returns zero if successful and a negative value if  */
   /* an error occurred.                                                */
   /* * NOTE * Both the local and remote device must support an AVRCP   */
   /*          Version greater than or equal to 1.3 to use this command.*/
static int AUDSendRemoteControlRegisterNotificationCommand(ParameterList_t *TempParam)
{
   int                               ret_val;
   int                               Result;
   Byte_t                            EventID;
   BD_ADDR_t                         RemoteDevice;
   AUD_Remote_Control_Command_Data_t CommandData;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      if(AUDMRCControllerCallbackID > 0)
      {
         /* Make sure that all of the parameters required for this      */
         /* function appear to be at least semi-valid.                  */
         if((TempParam) && (TempParam->NumberofParameters >= 2) && (TempParam->Params[0].strParam) && (TempParam->Params[1].intParam >= AVRCP_EVENT_PLAYBACK_STATUS_CHANGED) && (TempParam->Params[1].intParam <= AVRCP_EVENT_VOLUME_CHANGED))
         {
            /* Note the input parameters.                               */
            StrToBD_ADDR(TempParam->Params[0].strParam, &RemoteDevice);
            EventID = TempParam->Params[1].intParam;

            /* Zero the command data structure.                         */
            BTPS_MemInitialize(&CommandData, 0x00, sizeof(CommandData));

            /* Build the response.                                      */
            CommandData.MessageType                                         = amtRegisterNotification;
            CommandData.MessageData.RegisterNotificationCommandData.EventID = EventID;

            if(EventID == AVRCP_EVENT_PLAYBACK_POS_CHANGED)
            {
               /* Set the refresh interval to 1 second.                 */
               CommandData.MessageData.RegisterNotificationCommandData.PlaybackInterval = 1;
            }

            /* Try to Send the Message.                                 */
            if((Result = AUDM_Send_Remote_Control_Command(AUDMRCControllerCallbackID, RemoteDevice, DEFAULT_AVRCP_COMMAND_TIMEOUT, &CommandData)) >= 0)
            {
               printf("Remote Control Command Sent: Transaction ID = %d\r\n", Result);

               /* Flag success to the caller.                           */
               ret_val = 0;
            }
            else
            {
               /* Error in the API call. Inform the user and flag an    */
               /* error.                                                */
               printf("AUDM_Send_Remote_Control_Command() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* One or more of the necessary parameters is/are invalid.  */
            printf("Usage: AUDSendRemoteControlRegisterNotificationCommand [BD_ADDR] [Event ID]\r\n");
            DisplayRemoteControlEvents();

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         printf("Register a Remote Control Callback (in the Controller Role), first. Example:\r\n    AUDRegisterRemoteControl 1\r\n");

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

   /* The following function is responsible for Sending a Remote Control*/
   /* (AVRCP) Register Notification Response to a connected AVRCP       */
   /* Target.  This function returns zero if successful and a negative  */
   /* value if an error occurred.                                       */
   /* * NOTE * Both the local and remote device must support an AVRCP   */
   /*          Version greater than or equal to 1.3 to use this command.*/
static int AUDSendRemoteControlRegisterNotificationResponse(ParameterList_t *TempParam)
{
   int                                          ret_val;
   int                                          Result;
   unsigned int                                 TransactionID;
   unsigned int                                 NumberAdditionalParameters;
   unsigned int                                 AdditionalParametersStartIndex;
   Byte_t                                       EventID;
   Boolean_t                                    DisplayUsage;
   BD_ADDR_t                                    RemoteDevice;
   AUD_Remote_Control_Response_Data_t           ResponseData;
   AVRCP_Register_Notification_Response_Data_t *RegisterNotification;
   AVRCP_Attribute_Value_ID_List_Entry_t        AttributeValueIDEntry;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      if(AUDMRCControllerCallbackID > 0)
      {
         DisplayUsage                   = FALSE;

         AdditionalParametersStartIndex = 3;

         /* Make sure that all of the parameters required for this      */
         /* function appear to be at least semi-valid.                  */
         if((TempParam) && (TempParam->NumberofParameters >= AdditionalParametersStartIndex) && (TempParam->Params[0].strParam) && (TempParam->Params[2].intParam >= AVRCP_EVENT_PLAYBACK_STATUS_CHANGED) && (TempParam->Params[2].intParam <= AVRCP_EVENT_VOLUME_CHANGED))
         {
            /* Note the input parameters.                               */
            StrToBD_ADDR(TempParam->Params[0].strParam, &RemoteDevice);
            TransactionID   = (unsigned int)TempParam->Params[1].intParam;
            EventID         = (Byte_t)TempParam->Params[2].intParam;

            Result          = 0;

            switch(EventID)
            {
               case AVRCP_EVENT_PLAYBACK_STATUS_CHANGED:
                  NumberAdditionalParameters = 1;
                  break;
               case AVRCP_EVENT_TRACK_CHANGED:
                  NumberAdditionalParameters = 1;
                  break;
               case AVRCP_EVENT_TRACK_REACHED_END:
                  NumberAdditionalParameters = 0;
                  break;
               case AVRCP_EVENT_TRACK_REACHED_START:
                  NumberAdditionalParameters = 0;
                  break;
               case AVRCP_EVENT_PLAYBACK_POS_CHANGED:
                  NumberAdditionalParameters = 1;
                  break;
               case AVRCP_EVENT_BATT_STATUS_CHANGED:
                  NumberAdditionalParameters = 1;
                  break;
               case AVRCP_EVENT_SYSTEM_STATUS_CHANGED:
                  NumberAdditionalParameters = 1;
                  break;
               case AVRCP_EVENT_PLAYER_APPLICATION_SETTING_CHANGED:
                  NumberAdditionalParameters = 2;
                  break;
               case AVRCP_EVENT_NOW_PLAYING_CONTENT_CHANGED:
                  NumberAdditionalParameters = 0;
                  break;
               case AVRCP_EVENT_AVAILABLE_PLAYERS_CHANGED:
                  NumberAdditionalParameters = 0;
                  break;
               case AVRCP_EVENT_ADDRESSED_PLAYER_CHANGED:
                  NumberAdditionalParameters = 2;
                  break;
               case AVRCP_EVENT_UIDS_CHANGED:
                  NumberAdditionalParameters = 1;
                  break;
               case AVRCP_EVENT_VOLUME_CHANGED:
                  NumberAdditionalParameters = 1;
                  break;
               default:
                  NumberAdditionalParameters = 0;
                  break;
            }

            if((!Result) && (TempParam->NumberofParameters >= (AdditionalParametersStartIndex + NumberAdditionalParameters)))
            {
               /* Zero the response structures.                         */
               BTPS_MemInitialize(&ResponseData, 0x00, sizeof(ResponseData));
               BTPS_MemInitialize(&AttributeValueIDEntry, 0x00, sizeof(AttributeValueIDEntry));

               RegisterNotification = &ResponseData.MessageData.RegisterNotificationResponseData;

               switch(EventID)
               {
                  case AVRCP_EVENT_PLAYBACK_STATUS_CHANGED:
                     RegisterNotification->NotificationData.PlaybackStatusChangedData.PlayStatus                        = (Byte_t)TempParam->Params[AdditionalParametersStartIndex].intParam;
                     CurrentPlayStatus                                                                                  = RegisterNotification->NotificationData.PlaybackStatusChangedData.PlayStatus;
                     break;
                  case AVRCP_EVENT_TRACK_CHANGED:
                     RegisterNotification->NotificationData.TrackChangedData.Identifier                                 = (QWord_t)TempParam->Params[AdditionalParametersStartIndex].intParam;
                     break;
                  case AVRCP_EVENT_PLAYBACK_POS_CHANGED:
                     RegisterNotification->NotificationData.PlaybackPosChangedData.PlaybackPosition                     = (DWord_t)TempParam->Params[AdditionalParametersStartIndex].intParam;
                     break;
                  case AVRCP_EVENT_BATT_STATUS_CHANGED:
                     RegisterNotification->NotificationData.BattStatusChangedData.BatteryStatus                         = (Byte_t)TempParam->Params[AdditionalParametersStartIndex].intParam;
                     break;
                  case AVRCP_EVENT_SYSTEM_STATUS_CHANGED:
                     RegisterNotification->NotificationData.SystemStatusChangedData.SystemStatus                        = (Byte_t)TempParam->Params[AdditionalParametersStartIndex].intParam;
                     break;
                  case AVRCP_EVENT_PLAYER_APPLICATION_SETTING_CHANGED:
                     AttributeValueIDEntry.AttributeID                                                                  = (Byte_t)TempParam->Params[AdditionalParametersStartIndex].intParam;
                     AttributeValueIDEntry.ValueID                                                                      = (Byte_t)TempParam->Params[AdditionalParametersStartIndex + 1].intParam;
                     RegisterNotification->NotificationData.PlayerApplicationSettingChangedData.NumberAttributeValueIDs = 1;
                     RegisterNotification->NotificationData.PlayerApplicationSettingChangedData.AttributeValueIDList    = &AttributeValueIDEntry;
                     break;
                  case AVRCP_EVENT_ADDRESSED_PLAYER_CHANGED:
                     RegisterNotification->NotificationData.AddressedPlayerChangedData.MediaPlayerID                    = (Word_t)TempParam->Params[AdditionalParametersStartIndex].intParam;
                     RegisterNotification->NotificationData.AddressedPlayerChangedData.UIDCounter                       = (Word_t)TempParam->Params[AdditionalParametersStartIndex + 1].intParam;
                     break;
                  case AVRCP_EVENT_UIDS_CHANGED:
                     RegisterNotification->NotificationData.UIDsChangedData.UIDCounter                                  = (Word_t)TempParam->Params[AdditionalParametersStartIndex].intParam;
                     break;
                  case AVRCP_EVENT_VOLUME_CHANGED:
                     RegisterNotification->NotificationData.VolumeChangedData.AbsoluteVolume                            = (Byte_t)PERCENTAGE_TO_ABSOLUTE_VOLUME(TempParam->Params[AdditionalParametersStartIndex].intParam);
                     CurrentAbsoluteVolume                                                                              = RegisterNotification->NotificationData.VolumeChangedData.AbsoluteVolume;
                     break;
                  default:
                     break;
               }

               /* Build the response.                                   */
               ResponseData.MessageType           = amtRegisterNotification;
               RegisterNotification->EventID      = EventID;
               RegisterNotification->ResponseCode = AVRCP_RESPONSE_CHANGED;

               /* Try to Send the Message.                              */
               if((Result = AUDM_Send_Remote_Control_Response(AUDMRCTargetCallbackID, RemoteDevice, TransactionID, &ResponseData)) >= 0)
               {
                  printf("Success: Remote Control Changed Notification Response Sent.\r\n");

                  /* Flag success to the caller.                        */
                  ret_val = 0;
               }
               else
               {
                  /* Error in the API call. Inform the user and flag an */
                  /* error.                                             */
                  printf("AUDM_Send_Remote_Control_Response() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

                  ret_val = FUNCTION_ERROR;
               }
            }
            else
            {
               DisplayUsage = TRUE;

               ret_val      = FUNCTION_ERROR;
            }
         }
         else
         {
            DisplayUsage = TRUE;

            ret_val      = INVALID_PARAMETERS_ERROR;
         }

         if(DisplayUsage)
         {
            /* One or more of the necessary parameters is/are invalid.  */
            printf("Usage: AUDSendRemoteControlRegisterNotificationResponse [BD_ADDR] [Transaction ID] [Event ID] [Notification Parameters]\r\n");
            printf("Absolute Volume Example: AUDSendRemoteControlRegisterNotificationResponse 010203040506 12 0x0D 75\r\n");
            DisplayRemoteControlEvents();
         }
      }
      else
      {
         printf("Register a Remote Control Callback (in the Controller Role), first. Example:\r\n    AUDRegisterRemoteControl 1\r\n");

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

   /* The following function is responsible for Sending a Remote Control*/
   /* (AVRCP) Get Element Attributes Command to retrieve the currently  */
   /* playing track information from a connected AVRCP Target.  This    */
   /* function returns zero if successful and a negative value if an    */
   /* error occurred.                                                   */
   /* * NOTE * Both the local and remote device must support an AVRCP   */
   /*          Version greater than or equal to 1.3 to use this command.*/
static int AUDSendRemoteControlGetTrackInformationCommand(ParameterList_t *TempParam)
{
   int                               ret_val;
   int                               Result;
   unsigned int                      Index;
   Boolean_t                         DisplayUsage;
   BD_ADDR_t                         RemoteDevice;
   AUD_Remote_Control_Command_Data_t CommandData;
   unsigned int                      AttributeIDsIndex;
   unsigned int                      AttributeIDsCount;
   DWord_t                           AttributeIDs[AVRCP_MEDIA_ATTRIBUTE_ID_PLAYING_TIME_MS];

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      if(AUDMRCControllerCallbackID > 0)
      {
         DisplayUsage      = FALSE;

         AttributeIDsIndex = 1;

         /* Make sure that all of the parameters required for this      */
         /* function appear to be at least semi-valid.                  */
         if((TempParam) && (TempParam->NumberofParameters > AttributeIDsIndex) && (TempParam->Params[0].strParam))
         {
            /* Note the input parameters.                               */
            StrToBD_ADDR(TempParam->Params[0].strParam, &RemoteDevice);

            Result            = 0;
            AttributeIDsCount = 0;

            if(TempParam->Params[AttributeIDsIndex].intParam == 0)
            {
               CommandData.MessageData.GetElementAttributesCommandData.AttributeIDList = NULL;
            }
            else
            {
               for(Index = AttributeIDsIndex; Index < ((unsigned int)TempParam->NumberofParameters); Index++)
               {
                  if((TempParam->Params[Index].intParam >= AVRCP_MEDIA_ATTRIBUTE_ID_TITLE_OF_MEDIA) && (TempParam->Params[Index].intParam <= AVRCP_MEDIA_ATTRIBUTE_ID_PLAYING_TIME_MS))
                  {
                     AttributeIDs[AttributeIDsCount++] = (DWord_t)TempParam->Params[Index].intParam;
                  }
                  else
                  {
                     Result = INVALID_PARAMETERS_ERROR;

                     break;
                  }
               }

               if(!Result)
               {
                  CommandData.MessageData.GetElementAttributesCommandData.AttributeIDList = AttributeIDs;
               }
            }

            if(!Result)
            {
               /* Build the response.                                   */
               CommandData.MessageType                                                  = amtGetElementAttributes;
               CommandData.MessageData.GetElementAttributesCommandData.Identifier       = AVRCP_ELEMENT_IDENTIFIER_PLAYING;
               CommandData.MessageData.GetElementAttributesCommandData.NumberAttributes = AttributeIDsCount;

               /* Try to Send the Message.                              */
               if((Result = AUDM_Send_Remote_Control_Command(AUDMRCControllerCallbackID, RemoteDevice, DEFAULT_AVRCP_COMMAND_TIMEOUT, &CommandData)) >= 0)
               {
                  printf("Remote Control Command Sent: Transaction ID = %d\r\n", Result);

                  /* Flag success to the caller.                        */
                  ret_val = 0;
               }
               else
               {
                  /* Error in the API call. Inform the user and flag an */
                  /* error.                                             */
                  printf("AUDM_Send_Remote_Control_Command() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

                  ret_val = FUNCTION_ERROR;
               }
            }
            else
            {
               DisplayUsage = TRUE;

               ret_val      = FUNCTION_ERROR;
            }
         }
         else
         {
            DisplayUsage = TRUE;

            ret_val      = INVALID_PARAMETERS_ERROR;
         }

         if(DisplayUsage)
         {
            /* One or more of the necessary parameters is/are invalid.  */
            printf("Usage: AUDSendRemoteControlGetTrackInformationCommand [BD_ADDR] [[GetAll = 0] || [[Attribute ID] [Attribute ID] ...]]\r\n");
            printf("Get All Attributes Example: AUDSendRemoteControlGetTrackInformationCommand 010203040506 0\r\n");
            printf("Attribute IDs:\r\n");
            printf("   AVRCP_MEDIA_ATTRIBUTE_ID_ILLEGAL:               0x%08X\r\n", AVRCP_MEDIA_ATTRIBUTE_ID_ILLEGAL);
            printf("   AVRCP_MEDIA_ATTRIBUTE_ID_TITLE_OF_MEDIA:        0x%08X\r\n", AVRCP_MEDIA_ATTRIBUTE_ID_TITLE_OF_MEDIA);
            printf("   AVRCP_MEDIA_ATTRIBUTE_ID_NAME_OF_ARTIST:        0x%08X\r\n", AVRCP_MEDIA_ATTRIBUTE_ID_NAME_OF_ARTIST);
            printf("   AVRCP_MEDIA_ATTRIBUTE_ID_NAME_OF_ALBUM:         0x%08X\r\n", AVRCP_MEDIA_ATTRIBUTE_ID_NAME_OF_ALBUM);
            printf("   AVRCP_MEDIA_ATTRIBUTE_ID_NUMBER_OF_MEDIA:       0x%08X\r\n", AVRCP_MEDIA_ATTRIBUTE_ID_NUMBER_OF_MEDIA);
            printf("   AVRCP_MEDIA_ATTRIBUTE_ID_TOTAL_NUMBER_OF_MEDIA: 0x%08X\r\n", AVRCP_MEDIA_ATTRIBUTE_ID_TOTAL_NUMBER_OF_MEDIA);
            printf("   AVRCP_MEDIA_ATTRIBUTE_ID_GENRE:                 0x%08X\r\n", AVRCP_MEDIA_ATTRIBUTE_ID_GENRE);
            printf("   AVRCP_MEDIA_ATTRIBUTE_ID_PLAYING_TIME_MS:       0x%08X\r\n", AVRCP_MEDIA_ATTRIBUTE_ID_PLAYING_TIME_MS);
         }
      }
      else
      {
         printf("Register a Remote Control Callback (in the Controller Role), first. Example:\r\n    AUDRegisterRemoteControl 1\r\n");

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

   /* The following function is responsible for Sending a Remote Control*/
   /* (AVRCP) Get Playback Status Command to a connected AVRCP Target.  */
   /* This function returns zero if successful and a negative value if  */
   /* an error occurred.                                                */
   /* * NOTE * Both the local and remote device must support an AVRCP   */
   /*          Version greater than or equal to 1.3 to use this command.*/
static int AUDSendRemoteControlGetPlayStatusCommand(ParameterList_t *TempParam)
{
   int                               Result;
   int                               ret_val;
   BD_ADDR_t                         RemoteDevice;
   AUD_Remote_Control_Command_Data_t CommandData;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Check if this application is registered for AVRCP Controller   */
      /* events.                                                        */
      if(AUDMRCControllerCallbackID > 0)
      {
         /* Make sure that all of the parameters required for this      */
         /* function appear to be at least semi-valid.                  */
         if((TempParam) && (TempParam->NumberofParameters >= 1) && (TempParam->Params[0].strParam))
         {
            StrToBD_ADDR(TempParam->Params[0].strParam, &RemoteDevice);

            /* Format a Get Capabilities command.                       */
            CommandData.MessageType = amtGetPlayStatus;

            /* Try to Send the Message.                                 */
            if((Result = AUDM_Send_Remote_Control_Command(AUDMRCControllerCallbackID, RemoteDevice, DEFAULT_AVRCP_COMMAND_TIMEOUT, &CommandData)) >= 0)
            {
               printf("Remote Control Command Sent: Transaction ID = %d\r\n", Result);

               /* Flag success to the caller.                           */
               ret_val = 0;
            }
            else
            {
               /* Error in the API call. Inform the user and flag an    */
               /* error.                                                */
               printf("AUDM_Send_Remote_Control_Command() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* One or more of the necessary parameters is/are invalid.  */
            printf("Usage: AUDSendRemoteControlGetPlayStatusCommand [BD_ADDR]\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         printf("Register a Remote Control Callback (in the Controller Role), first. Example:\r\n    AUDRegisterRemoteControl 1\r\n");

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

   /* The following function is responsible for query the AVRCP service */
   /* information of a remote device.  This function returns zero if    */
   /* successful and a negative value if an error occurred.             */
static int AUDQueryRemoteControlServiceInfo(ParameterList_t *TempParam)
{
   int                                 Result;
   int                                 ret_val;
   BD_ADDR_t                           RemoteDevice;
   AUDM_Remote_Control_Services_Info_t ServicesInfo;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 1) && (TempParam->Params[0].strParam))
      {
         StrToBD_ADDR(TempParam->Params[0].strParam, &RemoteDevice);

         Result = AUDM_Query_Remote_Control_Services_Info(RemoteDevice, &ServicesInfo);

         if(Result == 0)
         {
            printf("AUDM_Query_Remote_Control_Services_Info() Success.\r\n");
            printf("   Flags: %08X\r\n", (unsigned int)ServicesInfo.ServiceFlags);

            if(ServicesInfo.ServiceFlags & AUDM_REMOTE_CONTROL_SERVICES_FLAGS_CONTROLLER_ROLE_SUPPORTED)
            {
               printf("    Controller Role Info:\r\n");

               if(ServicesInfo.ControllerInfo.Version < NUMBER_KNOWN_AVRCP_VERSIONS)
                  printf("        Version:  %s\r\n", AVRCPVersionStrings[ServicesInfo.ControllerInfo.Version]);
               else
                  printf("        Version:  Unknown (%u)\r\n", ServicesInfo.ControllerInfo.Version);

               printf("        Features: %04X\r\n", ServicesInfo.ControllerInfo.SupportedFeaturesFlags);
            }

            if(ServicesInfo.ServiceFlags & AUDM_REMOTE_CONTROL_SERVICES_FLAGS_TARGET_ROLE_SUPPORTED)
            {
               printf("    Target Role Info:\r\n");

               if(ServicesInfo.TargetInfo.Version < NUMBER_KNOWN_AVRCP_VERSIONS)
                  printf("        Version:  %s\r\n", AVRCPVersionStrings[ServicesInfo.TargetInfo.Version]);
               else
                  printf("        Version:  Unknown (%u)\r\n", ServicesInfo.TargetInfo.Version);

               printf("        Features: %04X\r\n", ServicesInfo.TargetInfo.SupportedFeaturesFlags);
            }

            ret_val = 0;
         }
         else
         {
            printf("AUDM_Query_Remote_Control_Services_Info() Failure: %d, %s\r\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: AUDQueryRemoteControlServiceInfo [BD_ADDR]\r\n");

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

static int AUDConfigurePlayback(ParameterList_t *TempParam)
{
   int   ret_val;
   char *NamePtr;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 1))
      {
         if(COMPARE_NULL_BD_ADDR(CurrentDecodingBD_ADDR))
         {
            DecoderConfigFlags = TempParam->Params[0].intParam;

            if(DecoderConfigFlags & DECODER_CONFIG_FLAGS_ENABLE_WAV_OUTPUT)
            {
               if((TempParam->NumberofParameters >= 2) && (TempParam->Params[1].strParam))
                  NamePtr = TempParam->Params[1].strParam;
               else
                  NamePtr = DECODER_DEFAULT_WAV_FILE_NAME;

               strncpy(AudioFileName, NamePtr, MAX_AUDIO_FILE_NAME_LENGTH);
               AudioFileName[MAX_AUDIO_FILE_NAME_LENGTH-1] = '\0';
            }

            printf("Audio Playback Configured. Enable Features:\r\n");

            if(DecoderConfigFlags & DECODER_CONFIG_FLAGS_ENABLE_WAV_OUTPUT)
               printf("   Wave File Output. Filename: %s\r\n", AudioFileName);

#ifndef DISABLE_AUDIO_SINK_ALSA_OUTPUT

            if(DecoderConfigFlags & DECODER_CONFIG_FLAGS_ENABLE_ALSA_OUTPUT)
               printf("   Alsa Streaming.\r\n");

#endif

            if(!DecoderConfigFlags)
               printf("   None.\r\n");

            ret_val = 0;
         }
         else
         {
            /* We can't change the config while we are already decoding.*/
            printf("Cannot modify configuration while playback is in progress.\r\n");

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: AUDConfigurePlayback [Config Flags] [WAV File Name (Optional if WAV output enabled. \"audio.wav\" by default)]\r\n");
         printf("   Possible Config Flags:\r\n");
         printf("      0x%08X - Enable WAV File Output\r\n", DECODER_CONFIG_FLAGS_ENABLE_WAV_OUTPUT);

#ifndef DISABLE_AUDIO_SINK_ALSA_OUTPUT

         printf("      0x%08X - Enable ALSA Streaming\r\n", DECODER_CONFIG_FLAGS_ENABLE_ALSA_OUTPUT);

#endif

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

static int AUDCurrentPlaybackConfig(ParameterList_t *TempParam)
{
   int ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      ret_val = 0;

      printf("Current Playback Configuration:\r\n");

      if(!DecoderConfigFlags)
         printf("   None.\r\n");
      else
      {
         if(DecoderConfigFlags & DECODER_CONFIG_FLAGS_ENABLE_WAV_OUTPUT)
            printf("   Wave File Output. Filename: %s.\r\n", AudioFileName);

#ifndef DISABLE_AUDIO_SINK_ALSA_OUTPUT

         if(DecoderConfigFlags & DECODER_CONFIG_FLAGS_ENABLE_ALSA_OUTPUT)
            printf("   Alsa Streaming.\r\n");

#endif

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

static int AUDConnectBrowsing(ParameterList_t *TempParam)
{
   int       ret_val;
   int       Result;
   BD_ADDR_t RemoteDeviceAddress;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 1) && (TempParam->Params[0].strParam))
      {
         /* Parameters appear to be valid, map the specified parameters */
         /* into the API specific parameters.                           */
         StrToBD_ADDR(TempParam->Params[0].strParam, &RemoteDeviceAddress);

         Result = AUDM_Connect_Remote_Control_Browsing(RemoteDeviceAddress, 0, AUDM_Event_Callback, NULL, NULL);

         if(!Result)
         {
            printf("AUDM_Connect_Remote_Control_Browsing() Success: %d.\r\n", Result);

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            /* Error in the API call. Inform the user and flag an error.*/
            printf("AUDM_Connect_Remote_Control_Browsing() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: AUDConnectBrowsing [BD_ADDR].\r\n");

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

static int AUDDisconnectBrowsing(ParameterList_t *TempParam)
{
   int       ret_val;
   int       Result;
   BD_ADDR_t RemoteDeviceAddress;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 1) && (TempParam->Params[0].strParam))
      {
         /* Parameters appear to be valid, map the specified parameters */
         /* into the API specific parameters.                           */
         StrToBD_ADDR(TempParam->Params[0].strParam, &RemoteDeviceAddress);

         Result = AUDM_Disconnect_Remote_Control_Browsing(RemoteDeviceAddress);

         if(!Result)
         {
            printf("AUDM_Disconnect_Remote_Control_Browsing() Success: %d.\r\n", Result);

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            /* Error in the API call. Inform the user and flag an error.*/
            printf("AUDM_Disconnect_Remote_Control_Browsing() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: AUDDisconnectBrowsing [BD_ADDR].\r\n");

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

static int AUDGetTotalNumberOfItems(ParameterList_t *TempParam)
{
   int                               ret_val;
   int                               Result;
   BD_ADDR_t                         RemoteDeviceAddress;
   AUD_Remote_Control_Command_Data_t CommandData;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure we are registered as a controller. */
      if(AUDMRCControllerCallbackID > 0)
      {
         /* Make sure that all of the parameters required for this      */
         /* function appear to be at least semi-valid.                  */
         if((TempParam) && (TempParam->NumberofParameters >= 1) && (TempParam->Params[0].strParam))
         {
            /* Parameters appear to be valid, map the specified         */
            /* parameters into the API specific parameters.             */
            StrToBD_ADDR(TempParam->Params[0].strParam, &RemoteDeviceAddress);

            CommandData.MessageType                                        = amtGetTotalNumberOfItems;
            CommandData.MessageData.GetTotalNumberOfItemsCommandData.Scope = AVRCP_NAVIGATION_SCOPE_MEDIA_PLAYER_LIST;

            Result = AUDM_Send_Remote_Control_Command(AUDMRCControllerCallbackID, RemoteDeviceAddress, 500, &CommandData);

            if(!Result)
            {
               printf("AUDM_Send_Remote_Control_Command() Success: %d.\r\n", Result);

               /* Flag success to the caller.                           */
               ret_val = 0;
            }
            else
            {
               /* Error in the API call. Inform the user and flag an    */
               /* error.                                                */
               printf("AUDM_Send_Remote_Control_Command() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* One or more of the necessary parameters is/are invalid.     */
            printf("Usage: AUDDisconnectBrowsing [BD_ADDR].\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         printf("Register a Remote Control Callback (in the Controller Role), first. Example:\r\n    AUDRegisterRemoteControl 1\r\n");

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

   /* The following function is a utility function that exists to parse */
   /* a WAV audio file header and extract information relevant to       */
   /* interpreting the audio payload of the file. This function returns */
   /* zero on success or a negative value if the file cannot be parsed. */
static int ParseWAVHeader(int FileDescriptor, WAVInfo_t *Info)
{
   int         ret_val;
   Byte_t      Buffer[64];
   Byte_t      SubFormat[16];
   Word_t      FormatTag;
   Word_t      Channels;
   Word_t      BlockAlign;
   Word_t      BitsPerSample;
   Word_t      FmtExtensionSize;
   Word_t      ValidBitsPerSample;
   DWord_t     FmtChunkSize;
   DWord_t     SamplesPerSec;
   DWord_t     AvgBytesPerSec;
   struct stat FileStat;

   ret_val = 0;

   if((FileDescriptor >= 0) && (Info))
   {
      /* Parse the RIFF Header.                                         */
      if((read(FileDescriptor, Buffer, 8) == 8) && (memcmp(Buffer, "RIFF", 4) == 0))
      {
         if((read(FileDescriptor, Buffer, 4) == 4) && (memcmp(Buffer, "WAVE", 4) == 0))
         {
            if((read(FileDescriptor, Buffer, 8) == 8) && (memcmp(Buffer, "fmt ", 4) == 0))
            {
               FmtChunkSize = READ_UNALIGNED_DWORD_LITTLE_ENDIAN(&Buffer[4]);

               if((FmtChunkSize == 16) || (FmtChunkSize == 18) || (FmtChunkSize == 40))
               {
                  if(read(FileDescriptor, Buffer, FmtChunkSize) == FmtChunkSize)
                  {
                     FormatTag      = READ_UNALIGNED_WORD_LITTLE_ENDIAN(&Buffer[0]);
                     Channels       = READ_UNALIGNED_WORD_LITTLE_ENDIAN(&Buffer[2]);
                     SamplesPerSec  = READ_UNALIGNED_DWORD_LITTLE_ENDIAN(&Buffer[4]);
                     AvgBytesPerSec = READ_UNALIGNED_DWORD_LITTLE_ENDIAN(&Buffer[8]);
                     BlockAlign     = READ_UNALIGNED_WORD_LITTLE_ENDIAN(&Buffer[12]);
                     BitsPerSample  = READ_UNALIGNED_WORD_LITTLE_ENDIAN(&Buffer[14]);

                     if(FmtChunkSize > 16)
                     {
                        FmtExtensionSize = READ_UNALIGNED_WORD_LITTLE_ENDIAN(&Buffer[16]);

                        if((FmtExtensionSize == 22) && (FmtChunkSize == 40))
                        {
                           ValidBitsPerSample = READ_UNALIGNED_WORD_LITTLE_ENDIAN(&Buffer[18]);

                           /* dwChannelMask (4 bytes, bitmask):         */
                           /* indicates the speakers in common          */
                           /* multi-channel configurations to which each*/
                           /* audio channel should correspond.  This is */
                           /* ignored since we only support 1- and      */
                           /* 2-channel audio.                          */

                           memcpy(SubFormat, &Buffer[24], 16);
                        }
                        else
                        {
                           if((FmtExtensionSize == 0) && (FmtChunkSize == 18))
                           {
                              ValidBitsPerSample = ((BlockAlign * 8) / Channels);

                              memset(SubFormat, 0, 16);
                           }
                           else
                           {
                              printf("Invalid combination of Format Chunk Size and Format Extension Size (%ld, %hd)\r\n", (long)FmtChunkSize, FmtExtensionSize);

                              ret_val = WAV_FORMAT_ERROR;
                           }
                        }
                     }
                     else
                     {
                        ValidBitsPerSample = ((BlockAlign * 8) / Channels);

                        memset(SubFormat, 0, 16);
                     }
                  }
                  else
                  {
                     printf("Unable to read Format Chunk.\r\n");

                     ret_val = WAV_FORMAT_ERROR;
                  }
               }
               else
               {
                  printf("Format header specifies invalid Chunk Size (%ld)\r\n", (long)FmtChunkSize);

                  ret_val = WAV_FORMAT_ERROR;
               }
            }
            else
            {
               printf("Unable to read Format header.\r\n");

               ret_val = WAV_FORMAT_ERROR;
            }
         }
         else
         {
            printf("Unable to read WAVE header.\r\n");

            ret_val = WAV_FORMAT_ERROR;
         }
      }
      else
      {
         printf("Unable to read RIFF header.\r\n");

         ret_val = WAV_FORMAT_ERROR;
      }

      /* Identify the start of the Audio Data, if no other problems have*/
      /* been encountered.                                              */
      if(!ret_val)
      {
         /* Scan Chunk Headers (ID + Size) until we find the one tagged */
         /* with the "data" ID.                                         */
         while((read(FileDescriptor, Buffer, 8) == 8) && (memcmp(Buffer, "data", 4) != 0))
         {
            errno = 0;

            if(lseek(FileDescriptor, READ_UNALIGNED_DWORD_LITTLE_ENDIAN(&Buffer[4]), SEEK_CUR) < 0)
            {
               printf("Error seeking through WAV file for Data Chunk: %d, %s\r\n", errno, strerror(errno));

               ret_val = WAV_FORMAT_ERROR;
            }
         }

         Info->AudioDataOffset = lseek(FileDescriptor, 0, SEEK_CUR);
         Info->AudioDataLength = READ_UNALIGNED_DWORD_LITTLE_ENDIAN(&Buffer[4]);

         /* Make sure the file is big enough to justify the reported    */
         /* length of the Data Chunk.                                   */
         errno = 0;
         if(fstat(FileDescriptor, &FileStat) == 0)
         {
            if(FileStat.st_size >= (Info->AudioDataOffset + Info->AudioDataLength))
            {
               if(FormatTag == WAVE_FORMAT_EXTENSIBLE)
                  Info->Format = READ_UNALIGNED_WORD_LITTLE_ENDIAN(SubFormat);
               else
                  Info->Format = FormatTag;

               Info->Channels              = Channels;
               Info->SamplesPerSecond      = SamplesPerSec;
               Info->AverageBytesPerSecond = AvgBytesPerSec;
               Info->BlockSize             = BlockAlign;
               Info->BitsPerSample         = BitsPerSample;
               Info->ValidBitsPerSample    = ((ValidBitsPerSample <= BitsPerSample)?ValidBitsPerSample:BitsPerSample);

               /* WAV file successfully parsed and validated.           */
               ret_val = 0;
            }
            else
            {
               printf("WAV file too short: Header & Data requires %lu bytes, but file is only %lu bytes long\r\n", (unsigned long)(Info->AudioDataOffset + Info->AudioDataLength), (unsigned long)FileStat.st_size);

               ret_val = WAV_FORMAT_ERROR;
            }
         }
         else
         {
            printf("Error reading file size: %d, %s\r\n", errno, strerror(errno));

            ret_val = WAV_FORMAT_ERROR;
         }
      }
   }
   else
   {
      printf("Bad file descriptor.\r\n");

      ret_val = INVALID_PARAMETERS_ERROR;
   }

   return(ret_val);
}

   /* The following function is provided to be used as an entry         */
   /* point for a thread which streams PCM audio data from an open      */
   /* file stream. The function expects to receive a pointer to a       */
   /* PlaybackThreadParameter_t structure and always returns NULL.      */
static void *PlaybackThreadMain(void *ThreadParameter)
{
   int                     Result;
   off_t                   StartOffset;
   Byte_t                 *Buffer;
   unsigned int            BufferLength;
   unsigned int            BufferRemaining;
   unsigned int            FileRemaining;
   unsigned int            ExpectedCount;
   unsigned int            BytesPerFrame;
   unsigned long           ReadCount;
   unsigned long           OffsetCount;
   PlaybackThreadParams_t *Params;

   if((Params = (PlaybackThreadParams_t *)ThreadParameter) != NULL)
   {
      BytesPerFrame   = Params->WAVInfo.BlockSize;
      FileRemaining   = Params->WAVInfo.AudioDataLength;

      OffsetCount     = Params->Offset;

      ReadCount       = 0;

      BufferLength    = WAV_PCM_BUFFER_SIZE;
      BufferRemaining = 0;

      if((Buffer = (Byte_t *)malloc(BufferLength)) != NULL)
      {
         /* Set up our playback and SBC encoder states.                 */
         if(InitializeAudioEncoder(Params->CallbackID, Params->WAVInfo.SamplesPerSecond, Params->WAVInfo.Channels, Params->RemoteDevice) == 0)
         {
            /* Note the current position of the file so we know the     */
            /* beginning of the Audio Data if we need to loop playback. */
            StartOffset = lseek(Params->FileDescriptor, 0, SEEK_CUR);

            printf("Total audio data stream size: %u\r\n", FileRemaining);

            Result = 0;

            /* Go ahead and read up to the offset to resume playing (if */
            /* specified).                                              */
            if(OffsetCount > FileRemaining)
               OffsetCount = FileRemaining;

            ExpectedCount = ((FileRemaining < BufferLength)?FileRemaining:BufferLength);

            while((OffsetCount > 0) && ((Result = read(Params->FileDescriptor, Buffer, ExpectedCount)) > 0))
            {
               OffsetCount   -= Result;
               FileRemaining -= Result;
            }

            Result = 0;
            while(((BufferRemaining > 0) || (FileRemaining > 0)) && (Result >= 0))
            {
               /* Check to see if the Stream State changed.             */
               if(BTPS_WaitEvent(AudioSourceState.StreamStateChanged, 0))
               {
                  BTPS_ResetEvent(AudioSourceState.StreamStateChanged);

                  if(AudioSourceState.StreamState == astStreamStopped)
                     break;
               }

               /* There is more data available (in the buffer or in the */
               /* file), so build up a new SBC packet now. We do this   */
               /* ahead of time so the packet can be sent as soon as    */
               /* we're ready for it.                                   */

               /* Check whether the buffer is exhausted. If so, refill  */
               /* it with data from the audio file.                     */
               if(BufferRemaining == 0)
               {
                  /* We have to ensure that we read whole frames (where */
                  /* one frame is a set of one sample for each channel),*/
                  /* so we'll keep reading until we either run out of   */
                  /* data in the file or we hit our target amount. The  */
                  /* target amount is the size of the buffer or what's  */
                  /* left of the file. Since the Buffer is always sized */
                  /* for a whole number of frames, the only way we'll   */
                  /* end up off the mark is if the file doesn't contain */
                  /* a whole number of frames. This would only happen   */
                  /* with a malformed WAV, but we can correct for it by */
                  /* padding the data.                                  */

                  /* Read as much as we can, given the limits of the    */
                  /* Buffer size and what's left in the file.           */
                  Result        = 0;
                  ExpectedCount = ((FileRemaining < BufferLength)?FileRemaining:BufferLength);
                  while((ExpectedCount > 0) && ((Result = read(Params->FileDescriptor, Buffer, ExpectedCount)) > 0))
                  {
                     /* We were able to read some data. Keep track of   */
                     /* how much we have left to read and how much we've*/
                     /* read so far.                                    */
                     ExpectedCount   -= Result;
                     BufferRemaining += Result;

                     ReadCount += Result;
                  }

                  /* Update how many bytes remain to be read from the   */
                  /* file.                                              */
                  if(BufferRemaining > 0)
                     FileRemaining -= BufferRemaining;

                  /* Now check the results of the reading loop.         */
                  if(Result <= 0)
                  {
                     /* We couldn't read everything we wanted to, either*/
                     /* because we ran out of data in the file or there */
                     /* was an error accessing it. We might need to     */
                     /* correct for this to ensure we have a whole      */
                     /* number of frames.                               */
                     if(BufferRemaining % BytesPerFrame)
                     {
                        memset(&Buffer[BufferRemaining], 0, (BytesPerFrame - (BufferRemaining % BytesPerFrame)));

                        BufferRemaining += (BytesPerFrame - (BufferRemaining % BytesPerFrame));
                     }
                  }
               }

               if(Result >= 0)
               {
                  Result = SendAudioData(Buffer, BufferRemaining, Params->RemoteDevice);
                  /* We have processed the buffer, reset the size       */
                  BufferRemaining = 0;
               }

               /* If we reached the end of the file and looping is      */
               /* enabled, attempt to seek back to the beginning of the */
               /* audio data.                                           */
               if((!FileRemaining) && (Params->Loop))
               {
                  if(lseek(Params->FileDescriptor, StartOffset, SEEK_SET) >= 0)
                     FileRemaining = Params->WAVInfo.AudioDataLength;
               }
            }

            if(FileRemaining == 0)
               FileOffset = 0;
            else
               FileOffset = Params->Offset + ReadCount;

            CleanupAudioEncoder();
         }
         else
            printf("Unable to configure SBC Encoder.\r\n");

         free(Buffer);
      }
      else
         printf("Unable to allocate memory for audio data buffer.\r\n");

      close(Params->FileDescriptor);

      free(Params);
   }

   ASSIGN_BD_ADDR(CurrentEncodingBD_ADDR, 0, 0, 0, 0, 0, 0);

   printf("Playback Thread done.\r\n");

   printf("AUDM>");

   fflush(stdout);

   return(NULL);
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

      if((!UpdateMask) || (UpdateMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_SNIFF_STATE))
      {
         if(RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_LINK_CURRENTLY_SNIFF_MODE)
            printf("Sniff State : TRUE (%u ms)\r\n", RemoteDeviceProperties->SniffInterval);
         else
            printf("Sniff State : FALSE\r\n");
      }

      if((!UpdateMask) || (UpdateMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_SERVICES_STATE))
         printf("Serv. Known  : %s\r\n", (RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_SERVICES_KNOWN)?"TRUE":"FALSE");
   }
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

   /* The following function handles processing a Remote Control Command*/
   /* Indication.                                                       */
static void ProcessRemoteControlCommandIndication(BD_ADDR_t RemoteDeviceBD_ADDR, unsigned int TransactionID, AUD_Remote_Control_Command_Data_t *CommandData)
{
   char                               *Message;
   char                               *ResponseMessage;
   char                               *RespondWith;
   int                                 Result;
   unsigned int                        Index;
   unsigned int                        AVRCPCapabilitiesCount;
   AVRCP_Capability_Info_t             AVRCPCapabilities[MAXIMUM_NUMBER_GET_CAPABILITIES_RESPONSE_EVENT_IDS];
   AUD_Remote_Control_Response_Data_t  ResponseData;
   Boolean_t                           SupportedEvent;

   switch(CommandData->MessageType)
   {
      case amtUnknown:
         Message = "Unknown";
         break;
      case amtUnitInfo:
         Message = "UnitInfo";
         break;
      case amtSubunitInfo:
         Message = "SubunitInfo";
         break;
      case amtPassThrough:
         Message = "PassThrough";
         break;
      case amtVendorDependent_Generic:
         Message = "VendorDependent_Generic";
         break;
      case amtBrowsingChannel_Generic:
         Message = "BrowsingChannel_Generic";
         break;
      case amtFragmentedMessage:
         Message = "FragmentedMessage";
         break;
      case amtGroupNavigation:
         Message = "GroupNavigation";
         break;
      case amtGetCapabilities:
         Message = "GetCapabilities";
         break;
      case amtListPlayerApplicationSettingAttributes:
         Message = "ListPlayerApplicationSettingAttributes";
         break;
      case amtListPlayerApplicationSettingValues:
         Message = "ListPlayerApplicationSettingValues";
         break;
      case amtGetCurrentPlayerApplicationSettingValue:
         Message = "GetCurrentPlayerApplicationSettingValue";
         break;
      case amtSetPlayerApplicationSettingValue:
         Message = "SetPlayerApplicationSettingValue";
         break;
      case amtGetPlayerApplicationSettingAttributeText:
         Message = "GetPlayerApplicationSettingAttributeText";
         break;
      case amtGetPlayerApplicationSettingValueText:
         Message = "GetPlayerApplicationSettingValueText";
         break;
      case amtInformDisplayableCharacterSet:
         Message = "InformDisplayableCharacterSet";
         break;
      case amtInformBatteryStatusOfCT:
         Message = "InformBatteryStatusOfCT";
         break;
      case amtGetElementAttributes:
         Message = "GetElementAttributes";
         break;
      case amtGetPlayStatus:
         Message = "GetPlayStatus";
         break;
      case amtRegisterNotification:
         Message = "RegisterNotification";
         break;
      case amtRequestContinuingResponse:
         Message = "RequestContinuingResponse";
         break;
      case amtAbortContinuingResponse:
         Message = "AbortContinuingResponse";
         break;
      case amtSetAbsoluteVolume:
         Message = "SetAbsoluteVolume";
         break;
      case amtCommandRejectResponse:
         Message = "CommandRejectResponse";
         break;
      case amtSetAddressedPlayer:
         Message = "SetAddressedPlayer";
         break;
      case amtPlayItem:
         Message = "PlayItem";
         break;
      case amtAddToNowPlaying:
         Message = "AddToNowPlaying";
         break;
      case amtSetBrowsedPlayer:
         Message = "SetBrowsedPlayer";
         break;
      case amtChangePath:
         Message = "ChangePath";
         break;
      case amtGetItemAttributes:
         Message = "GetItemAttributes";
         break;
      case amtSearch:
         Message = "Search";
         break;
      case amtGetFolderItems:
         Message = "GetFolderItems";
         break;
      case amtGeneralReject:
         Message = "GeneralReject";
         break;
      case amtGetTotalNumberOfItems:
         Message = "GetTotalNumberOfItems";
         break;
      default:
         Message = "Unknown";
         break;
   }

   printf("    Message Type : %s\r\n", Message);

   RespondWith     = NULL;
   ResponseMessage = NULL;

   /* Print additional information for select commands.                 */
   switch(CommandData->MessageType)
   {
      case amtPassThrough:
         printf("       Command Type: %u\r\n", CommandData->MessageData.PassThroughCommandData.CommandType);
         printf("       Subunit Type: %u\r\n", CommandData->MessageData.PassThroughCommandData.SubunitType);
         printf("       Subunit ID  : %u\r\n", CommandData->MessageData.PassThroughCommandData.SubunitID);
         printf("       Operation ID: %s (%u)\r\n", PassThroughToStr(CommandData->MessageData.PassThroughCommandData.OperationID), CommandData->MessageData.PassThroughCommandData.OperationID);
         printf("       State Flag  : %s\r\n", (CommandData->MessageData.PassThroughCommandData.StateFlag == FALSE)?"FALSE":"TRUE");
         printf("       Data Length : %u\r\n", CommandData->MessageData.PassThroughCommandData.OperationDataLength);

         ResponseData.MessageType                                             = amtPassThrough;
         ResponseData.MessageData.PassThroughResponseData.ResponseCode        = AVRCP_RESPONSE_ACCEPTED;
         ResponseData.MessageData.PassThroughResponseData.SubunitType         = CommandData->MessageData.PassThroughCommandData.SubunitType;
         ResponseData.MessageData.PassThroughResponseData.SubunitID           = CommandData->MessageData.PassThroughCommandData.SubunitID;
         ResponseData.MessageData.PassThroughResponseData.OperationID         = CommandData->MessageData.PassThroughCommandData.OperationID;
         ResponseData.MessageData.PassThroughResponseData.StateFlag           = CommandData->MessageData.PassThroughCommandData.StateFlag;
         ResponseData.MessageData.PassThroughResponseData.OperationDataLength = 0;
         ResponseData.MessageData.PassThroughResponseData.OperationData       = NULL;

         ResponseMessage                                                      = "Pass Through";
         break;
      case amtGetCapabilities:
         switch(CommandData->MessageData.GetCapabilitiesCommandData.CapabilityID)
         {
            case AVRCP_GET_CAPABILITIES_CAPABILITY_ID_COMPANY_ID:
               Message = "Company ID";
               break;
            case AVRCP_GET_CAPABILITIES_CAPABILITY_ID_EVENTS_SUPPORTED:
               Message     = "Events Supported";
               break;
            default:
               Message = "Unknown";
         }
         printf("    Capability ID: %s\r\n", Message);

         /* Check if this application is registered for AVRCP Target    */
         /* Events and the Capability ID of this message is a Get       */
         /* Capabilities Request.                                       */
         if((AUDMRCTargetCallbackID) && (CommandData->MessageData.GetCapabilitiesCommandData.CapabilityID == AVRCP_GET_CAPABILITIES_CAPABILITY_ID_EVENTS_SUPPORTED))
         {
            AVRCPCapabilitiesCount = 0;

            /* Check if the application is registered for A2DP source   */
            /* events.                                                  */
            if(AudioSourceState.CallbackID)
            {
               /* The application is registered for A2DP source events, */
               /* add the source Event IDs to the response.             */
               for(Index=0;Index<NUMBER_GET_CAPABILITIES_SOURCE_RESPONSE_EVENT_IDS;Index++)
               {
                  AVRCPCapabilities[AVRCPCapabilitiesCount++].CapabilityInfo.EventID = GetCapabilitiesSourceResponseEventIDs[Index];
               }
            }

            /* Check if the application is registered for A2DP sink     */
            /* events.                                                  */
            if(AudioSinkState.CallbackID)
            {
               /* The application is registered for A2DP sink events,   */
               /* add the sink Event IDs to the response.               */
               for(Index=0;Index<NUMBER_GET_CAPABILITIES_SINK_RESPONSE_EVENT_IDS;Index++)
               {
                  AVRCPCapabilities[AVRCPCapabilitiesCount++].CapabilityInfo.EventID = GetCapabilitiesSinkResponseEventIDs[Index];
               }
            }

            /* Prepare the response data.                               */
            ResponseData.MessageType                                                = amtGetCapabilities;
            ResponseData.MessageData.GetCapabilitiesResponseData.CapabilityID       = AVRCP_GET_CAPABILITIES_CAPABILITY_ID_EVENTS_SUPPORTED;
            ResponseData.MessageData.GetCapabilitiesResponseData.NumberCapabilities = AVRCPCapabilitiesCount;
            ResponseData.MessageData.GetCapabilitiesResponseData.CapabilityInfoList = AVRCPCapabilities;
            ResponseData.MessageData.GetCapabilitiesResponseData.ResponseCode       = AVRCP_RESPONSE_STABLE;

            ResponseMessage                                                         = "Supported Capabilities";
         }

         break;
      case amtRegisterNotification:
         switch(CommandData->MessageData.RegisterNotificationCommandData.EventID)
         {
            case AVRCP_EVENT_PLAYBACK_STATUS_CHANGED:
               Message = "AVRCP_EVENT_PLAYBACK_STATUS_CHANGED";
               ResponseData.MessageData.RegisterNotificationResponseData.NotificationData.PlaybackStatusChangedData.PlayStatus = CurrentPlayStatus;
               break;
            case AVRCP_EVENT_TRACK_CHANGED:
               Message = "AVRCP_EVENT_TRACK_CHANGED";
               break;
            case AVRCP_EVENT_TRACK_REACHED_END:
               Message = "AVRCP_EVENT_TRACK_REACHED_END";
               break;
            case AVRCP_EVENT_TRACK_REACHED_START:
               Message = "AVRCP_EVENT_TRACK_REACHED_START";
               break;
            case AVRCP_EVENT_PLAYBACK_POS_CHANGED:
               Message = "AVRCP_EVENT_PLAYBACK_POS_CHANGED";
               break;
            case AVRCP_EVENT_BATT_STATUS_CHANGED:
               Message = "AVRCP_EVENT_BATT_STATUS_CHANGED";
               break;
            case AVRCP_EVENT_SYSTEM_STATUS_CHANGED:
               Message = "AVRCP_EVENT_SYSTEM_STATUS_CHANGED";
               break;
            case AVRCP_EVENT_PLAYER_APPLICATION_SETTING_CHANGED:
               Message = "AVRCP_EVENT_PLAYER_APPLICATION_SETTING_CHANGED";
               break;
            case AVRCP_EVENT_NOW_PLAYING_CONTENT_CHANGED:
               Message = "AVRCP_EVENT_NOW_PLAYING_CONTENT_CHANGED";
               break;
            case AVRCP_EVENT_AVAILABLE_PLAYERS_CHANGED:
               Message = "AVRCP_EVENT_AVAILABLE_PLAYERS_CHANGED";
               break;
            case AVRCP_EVENT_ADDRESSED_PLAYER_CHANGED:
               Message = "AVRCP_EVENT_ADDRESSED_PLAYER_CHANGED";
               break;
            case AVRCP_EVENT_UIDS_CHANGED:
               Message = "AVRCP_EVENT_UIDS_CHANGED";
               break;
            case AVRCP_EVENT_VOLUME_CHANGED:
               Message = "AVRCP_EVENT_VOLUME_CHANGED";
               ResponseData.MessageData.RegisterNotificationResponseData.NotificationData.VolumeChangedData.AbsoluteVolume = CurrentAbsoluteVolume;
               break;
            default:
               Message = "Unknown";
               break;
         }

         printf("    Event        : %s\r\n", Message);

         /* Check if this application is registered for AVRCP Target    */
         /* Events.                                                     */
         if(AUDMRCTargetCallbackID)
         {
            /* The application is registered for AVRCP Target Events,   */
            /* next determine if this is a supported event.             */
            SupportedEvent = FALSE;

            /* Check if this application is registered for source       */
            /* events.                                                  */
            if(AudioSourceState.CallbackID)
            {
               /* The application is registered for source events, next */
               /* check if this event is supported.                     */
               for(Index=0;Index<NUMBER_GET_CAPABILITIES_SOURCE_RESPONSE_EVENT_IDS;Index++)
               {
                  if(CommandData->MessageData.RegisterNotificationCommandData.EventID == GetCapabilitiesSourceResponseEventIDs[Index])
                  {
                     SupportedEvent = TRUE;
                     break;
                  }
               }
            }

            /* Check if this application is registered for sink events. */
            if((!SupportedEvent) && (AudioSinkState.CallbackID))
            {
               /* The application is registered for sink events, next   */
               /* check if this event is supported.                     */
               for(Index=0;Index<NUMBER_GET_CAPABILITIES_SINK_RESPONSE_EVENT_IDS;Index++)
               {
                  if(CommandData->MessageData.RegisterNotificationCommandData.EventID == GetCapabilitiesSinkResponseEventIDs[Index])
                  {
                     SupportedEvent = TRUE;
                     break;
                  }
               }
            }

            /* Check if this event was found to be a supported event.   */
            if(SupportedEvent)
            {
               /* This is a supported event, populate the response.     */
               ResponseData.MessageType                                               = amtRegisterNotification;
               ResponseData.MessageData.RegisterNotificationResponseData.EventID      = CommandData->MessageData.RegisterNotificationCommandData.EventID;
               ResponseData.MessageData.RegisterNotificationResponseData.ResponseCode = AVRCP_RESPONSE_INTERIM;

               ResponseMessage = "Register Notification Interim";
               RespondWith     = "Send Changed Notification using AUDSendRemoteControlRegisterNotificationResponse.\r\n";
            }
            else
            {
               /* This is not a supported event, populate a Command     */
               /* Reject Response.                                      */
               ResponseData.MessageType                                        = amtCommandRejectResponse;
               ResponseData.MessageData.CommandRejectResponseData.ErrorCode    = 0;
               ResponseData.MessageData.CommandRejectResponseData.MessageType  = amtRegisterNotification;
               ResponseData.MessageData.CommandRejectResponseData.ResponseCode = AVRCP_RESPONSE_NOT_IMPLEMENTED;

               ResponseMessage = "Register Notification Command Reject";
            }
         }
         break;
      case amtSetAbsoluteVolume:
         printf("    Volume       : %d%%\r\n", ABSOLUTE_VOLUME_TO_PERCENTAGE(CommandData->MessageData.SetAbsoluteVolumeCommandData.AbsoluteVolume));

         /* Populate the Set Absolute Volume Response.                  */
         CurrentAbsoluteVolume                                                 = CommandData->MessageData.SetAbsoluteVolumeCommandData.AbsoluteVolume;
         ResponseData.MessageType                                              = amtSetAbsoluteVolume;
         ResponseData.MessageData.SetAbsoluteVolumeResponseData.AbsoluteVolume = CurrentAbsoluteVolume;
         ResponseData.MessageData.SetAbsoluteVolumeResponseData.ResponseCode   = AVRCP_RESPONSE_ACCEPTED;

         ResponseMessage = "Set Absolute Volume";

         break;
      case amtGetTotalNumberOfItems:
         printf("    Scope:       : %u.\r\n", CommandData->MessageData.GetTotalNumberOfItemsCommandData.Scope);

         ResponseData.MessageType                                                 = amtGetTotalNumberOfItems;
         ResponseData.MessageData.GetTotalNumberOfItemsResponseData.Status        = AVRCP_COMMAND_ERROR_STATUS_CODE_COMPLETE_NO_ERROR;
         ResponseData.MessageData.GetTotalNumberOfItemsResponseData.UIDCounter    = 1;
         ResponseData.MessageData.GetTotalNumberOfItemsResponseData.NumberOfItems = 13;

         ResponseMessage                                                          = "Get Total Number of Items";
         break;
      default:
         break;
   }

   if(ResponseMessage)
   {
      /* Try to Send the Message.                                       */
      if((Result = AUDM_Send_Remote_Control_Response(AUDMRCTargetCallbackID, RemoteDeviceBD_ADDR, TransactionID, &ResponseData)) >= 0)
      {
         printf("Status: %s Response Successfully Sent.\r\n", ResponseMessage);

         if(RespondWith)
            printf("%s\r\n", RespondWith);
      }
      else
      {
         /* Error in the API call. Inform the user and flag an error.   */
         printf("Error: Could Not Send %s Response, AUDM_Send_Remote_Control_Response() Failure: %d, %s.\r\n", ResponseMessage, Result, ERR_ConvertErrorCodeToString(Result));
      }
   }
}

   /* The following function handles processing a Remote Control Command*/
   /* Confirmation.                                                     */
static void ProcessRemoteControlCommandConfirmation(AUD_Remote_Control_Response_Data_t *ResponseData)
{
   char                                 *Message;
   unsigned int                          Index;
   char                                  TempArr[150];
   Boolean_t                             Success;
   AVRCP_Element_Attribute_List_Entry_t *AttributeList;

   switch(ResponseData->MessageType)
   {
      case amtUnknown:
         Message = "Unknown";
         break;
      case amtUnitInfo:
         Message = "UnitInfo";
         break;
      case amtSubunitInfo:
         Message = "SubunitInfo";
         break;
      case amtPassThrough:
         Message = "PassThrough";
         break;
      case amtVendorDependent_Generic:
         Message = "VendorDependent_Generic";
         break;
      case amtBrowsingChannel_Generic:
         Message = "BrowsingChannel_Generic";
         break;
      case amtFragmentedMessage:
         Message = "FragmentedMessage";
         break;
      case amtGroupNavigation:
         Message = "GroupNavigation";
         break;
      case amtGetCapabilities:
         Message = "GetCapabilities";
         break;
      case amtListPlayerApplicationSettingAttributes:
         Message = "ListPlayerApplicationSettingAttributes";
         break;
      case amtListPlayerApplicationSettingValues:
         Message = "ListPlayerApplicationSettingValues";
         break;
      case amtGetCurrentPlayerApplicationSettingValue:
         Message = "GetCurrentPlayerApplicationSettingValue";
         break;
      case amtSetPlayerApplicationSettingValue:
         Message = "SetPlayerApplicationSettingValue";
         break;
      case amtGetPlayerApplicationSettingAttributeText:
         Message = "GetPlayerApplicationSettingAttributeText";
         break;
      case amtGetPlayerApplicationSettingValueText:
         Message = "GetPlayerApplicationSettingValueText";
         break;
      case amtInformDisplayableCharacterSet:
         Message = "InformDisplayableCharacterSet";
         break;
      case amtInformBatteryStatusOfCT:
         Message = "InformBatteryStatusOfCT";
         break;
      case amtGetElementAttributes:
         Message = "GetElementAttributes";
         break;
      case amtGetPlayStatus:
         Message = "GetPlayStatus";
         break;
      case amtRegisterNotification:
         Message = "RegisterNotification";
         break;
      case amtRequestContinuingResponse:
         Message = "RequestContinuingResponse";
         break;
      case amtAbortContinuingResponse:
         Message = "AbortContinuingResponse";
         break;
      case amtSetAbsoluteVolume:
         Message = "SetAbsoluteVolume";
         break;
      case amtCommandRejectResponse:
         Message = "CommandRejectResponse";
         break;
      case amtSetAddressedPlayer:
         Message = "SetAddressedPlayer";
         break;
      case amtPlayItem:
         Message = "PlayItem";
         break;
      case amtAddToNowPlaying:
         Message = "AddToNowPlaying";
         break;
      case amtSetBrowsedPlayer:
         Message = "SetBrowsedPlayer";
         break;
      case amtChangePath:
         Message = "ChangePath";
         break;
      case amtGetItemAttributes:
         Message = "GetItemAttributes";
         break;
      case amtSearch:
         Message = "Search";
         break;
      case amtGetFolderItems:
         Message = "GetFolderItems";
         break;
      case amtGeneralReject:
         Message = "GeneralReject";
         break;
      case amtGetTotalNumberOfItems:
         Message = "GetTotalNumberOfItems";
         break;
      default:
         Message = "Unknown";
         break;
   }

   printf("    Message Type : %s\r\n", Message);

   switch(ResponseData->MessageType)
   {
      case amtPassThrough:
         DisplayRemoteControlResponseCode("       Response Code: ", ResponseData->MessageData.PassThroughResponseData.ResponseCode, "\r\n");
         printf("       Subunit Type : %u\r\n", ResponseData->MessageData.PassThroughResponseData.SubunitType);
         printf("       Subunit ID   : %u\r\n", ResponseData->MessageData.PassThroughResponseData.SubunitID);
         printf("       Operation ID: %s (%u)\r\n", PassThroughToStr(ResponseData->MessageData.PassThroughResponseData.OperationID), ResponseData->MessageData.PassThroughResponseData.OperationID);
         printf("       State Flag   : %s\r\n", (ResponseData->MessageData.PassThroughResponseData.StateFlag == FALSE)?"FALSE":"TRUE");
         printf("       Data Length  : %u\r\n", ResponseData->MessageData.PassThroughResponseData.OperationDataLength);
         break;
      case amtGetCapabilities:
         DisplayRemoteControlResponseCode("    Response Code: ", ResponseData->MessageData.GetCapabilitiesResponseData.ResponseCode, "\r\n");

         printf("    Supported Events:\r\n");
         for(Index=0;Index<ResponseData->MessageData.GetCapabilitiesResponseData.NumberCapabilities;Index++)
         {
            switch(ResponseData->MessageData.GetCapabilitiesResponseData.CapabilityInfoList[Index].CapabilityInfo.EventID)
            {
               case AVRCP_EVENT_PLAYBACK_STATUS_CHANGED:
                  printf("       AVRCP_EVENT_PLAYBACK_STATUS_CHANGED:            0x%02X\r\n", ResponseData->MessageData.GetCapabilitiesResponseData.CapabilityInfoList[Index].CapabilityInfo.EventID);
                  break;
               case AVRCP_EVENT_TRACK_CHANGED:
                  printf("       AVRCP_EVENT_TRACK_CHANGED:                      0x%02X\r\n", ResponseData->MessageData.GetCapabilitiesResponseData.CapabilityInfoList[Index].CapabilityInfo.EventID);
                  break;
               case AVRCP_EVENT_TRACK_REACHED_END:
                  printf("       AVRCP_EVENT_TRACK_REACHED_END:                  0x%02X\r\n", ResponseData->MessageData.GetCapabilitiesResponseData.CapabilityInfoList[Index].CapabilityInfo.EventID);
                  break;
               case AVRCP_EVENT_TRACK_REACHED_START:
                  printf("       AVRCP_EVENT_TRACK_REACHED_START:                0x%02X\r\n", ResponseData->MessageData.GetCapabilitiesResponseData.CapabilityInfoList[Index].CapabilityInfo.EventID);
                  break;
               case AVRCP_EVENT_PLAYBACK_POS_CHANGED:
                  printf("       AVRCP_EVENT_PLAYBACK_POS_CHANGED:               0x%02X\r\n", ResponseData->MessageData.GetCapabilitiesResponseData.CapabilityInfoList[Index].CapabilityInfo.EventID);
                  break;
               case AVRCP_EVENT_BATT_STATUS_CHANGED:
                  printf("       AVRCP_EVENT_BATT_STATUS_CHANGED:                0x%02X\r\n", ResponseData->MessageData.GetCapabilitiesResponseData.CapabilityInfoList[Index].CapabilityInfo.EventID);
                  break;
               case AVRCP_EVENT_SYSTEM_STATUS_CHANGED:
                  printf("       AVRCP_EVENT_SYSTEM_STATUS_CHANGED:              0x%02X\r\n", ResponseData->MessageData.GetCapabilitiesResponseData.CapabilityInfoList[Index].CapabilityInfo.EventID);
                  break;
               case AVRCP_EVENT_PLAYER_APPLICATION_SETTING_CHANGED:
                  printf("       AVRCP_EVENT_PLAYER_APPLICATION_SETTING_CHANGED: 0x%02X\r\n", ResponseData->MessageData.GetCapabilitiesResponseData.CapabilityInfoList[Index].CapabilityInfo.EventID);
                  break;
               case AVRCP_EVENT_NOW_PLAYING_CONTENT_CHANGED:
                  printf("       AVRCP_EVENT_NOW_PLAYING_CONTENT_CHANGED:        0x%02X\r\n", ResponseData->MessageData.GetCapabilitiesResponseData.CapabilityInfoList[Index].CapabilityInfo.EventID);
                  break;
               case AVRCP_EVENT_AVAILABLE_PLAYERS_CHANGED:
                  printf("       AVRCP_EVENT_AVAILABLE_PLAYERS_CHANGED:          0x%02X\r\n", ResponseData->MessageData.GetCapabilitiesResponseData.CapabilityInfoList[Index].CapabilityInfo.EventID);
                  break;
               case AVRCP_EVENT_ADDRESSED_PLAYER_CHANGED:
                  printf("       AVRCP_EVENT_ADDRESSED_PLAYER_CHANGED:           0x%02X\r\n", ResponseData->MessageData.GetCapabilitiesResponseData.CapabilityInfoList[Index].CapabilityInfo.EventID);
                  break;
               case AVRCP_EVENT_UIDS_CHANGED:
                  printf("       AVRCP_EVENT_UIDS_CHANGED:                       0x%02X\r\n", ResponseData->MessageData.GetCapabilitiesResponseData.CapabilityInfoList[Index].CapabilityInfo.EventID);
                  break;
               case AVRCP_EVENT_VOLUME_CHANGED:
                  printf("       AVRCP_EVENT_VOLUME_CHANGED:                     0x%02X\r\n", ResponseData->MessageData.GetCapabilitiesResponseData.CapabilityInfoList[Index].CapabilityInfo.EventID);
                  break;
               default:
                  printf("       Unknown Event ID:                               0x%02X\r\n", ResponseData->MessageData.GetCapabilitiesResponseData.CapabilityInfoList[Index].CapabilityInfo.EventID);
                  break;
            }
         }
         printf("Register for event notification using: AUDSendRemoteControlRegisterNotificationCommand.\r\n");
         break;
      case amtGetElementAttributes:
         AttributeList = ResponseData->MessageData.GetElementAttributesResponseData.AttributeList;

         if(ResponseData->MessageData.GetElementAttributesResponseData.ResponseCode == AVRCP_RESPONSE_STABLE)
         {
            printf("    Attributes:\r\n");

            for(Index = 0; Index < ResponseData->MessageData.GetElementAttributesResponseData.NumberAttributes; Index ++)
            {
               Success = TRUE;

               switch(AttributeList[Index].AttributeID)
               {
                  case AVRCP_MEDIA_ATTRIBUTE_ID_ILLEGAL:               printf("       Illegal Attribute\r\n"); Success = FALSE;    break;
                  case AVRCP_MEDIA_ATTRIBUTE_ID_TITLE_OF_MEDIA:        printf("       Title:             ");                       break;
                  case AVRCP_MEDIA_ATTRIBUTE_ID_NAME_OF_ARTIST:        printf("       Artist:            ");                       break;
                  case AVRCP_MEDIA_ATTRIBUTE_ID_NAME_OF_ALBUM:         printf("       Album:             ");                       break;
                  case AVRCP_MEDIA_ATTRIBUTE_ID_NUMBER_OF_MEDIA:       printf("       Track Number:      ");                       break;
                  case AVRCP_MEDIA_ATTRIBUTE_ID_TOTAL_NUMBER_OF_MEDIA: printf("       Total Num Tracks:  ");                       break;
                  case AVRCP_MEDIA_ATTRIBUTE_ID_GENRE:                 printf("       Genre:             ");                       break;
                  case AVRCP_MEDIA_ATTRIBUTE_ID_PLAYING_TIME_MS:       printf("       Track Length (ms): ");                       break;
                  default:                                             printf("       Unknown Attribute\r\n"); Success = FALSE;    break;
               }

               if(Success)
               {
                  if(AttributeList[Index].AttributeValueLength < sizeof(TempArr))
                  {
                     /* Copy the string and then null terminate it.     */
                     BTPS_MemCopy(TempArr, AttributeList[Index].AttributeValueData, AttributeList[Index].AttributeValueLength);
                     TempArr[AttributeList[Index].AttributeValueLength] = 0;
                  }
                  else
                  {
                     /* The string won't fit in the array, copy and     */
                     /* truncate the string.                            */
                     BTPS_MemCopy(TempArr, AttributeList[Index].AttributeValueData, (sizeof(TempArr) - 1));
                     TempArr[sizeof(TempArr) - 1] = 0;
                  }

                  printf("%s\r\n", TempArr);
               }
            }
         }
         else
         {
            DisplayRemoteControlResponseCode("    Response Code: ", ResponseData->MessageData.GetElementAttributesResponseData.ResponseCode, " [Error: Response != AVRCP_RESPONSE_STABLE]\r\n");
         }
         break;
      case amtGetPlayStatus:
         DisplayRemoteControlResponseCode("    Response Code: ", ResponseData->MessageData.GetPlayStatusResponseData.ResponseCode, "\r\n");
         printf("    Status       : ");
         DisplayRemoteControlPlaybackStatus(ResponseData->MessageData.GetPlayStatusResponseData.PlayStatus);
         printf("\r\n");

         printf("    Song Length  : ");
         if(ResponseData->MessageData.GetPlayStatusResponseData.SongLength != 0xFFFFFFFF)
            printf("%lu:%02lu\r\n", (unsigned long)((ResponseData->MessageData.GetPlayStatusResponseData.SongLength / 1000) / 60), (unsigned long)((ResponseData->MessageData.GetPlayStatusResponseData.SongLength / 1000) % 60));
         else
            printf("0x%lX (Not Supported)\r\n", (unsigned long)ResponseData->MessageData.GetPlayStatusResponseData.SongLength);

         printf("    Song Position: ");
         if(ResponseData->MessageData.GetPlayStatusResponseData.SongPosition != 0xFFFFFFFF)
            printf("%lu:%02lu\r\n", (unsigned long)((ResponseData->MessageData.GetPlayStatusResponseData.SongPosition / 1000) / 60), (unsigned long)((ResponseData->MessageData.GetPlayStatusResponseData.SongPosition / 1000) % 60));
         else
            printf("0x%lX (Not Supported)\r\n", (unsigned long)ResponseData->MessageData.GetPlayStatusResponseData.SongPosition);
         break;
      case amtRegisterNotification:
         DisplayRemoteControlResponseCode("    Response Code: ", ResponseData->MessageData.RegisterNotificationResponseData.ResponseCode, NULL);
         if(ResponseData->MessageData.RegisterNotificationResponseData.ResponseCode == AVRCP_RESPONSE_INTERIM)
            printf(" [Actual Notification Pending]");
         printf("\r\n");

         switch(ResponseData->MessageData.RegisterNotificationResponseData.EventID)
         {
            case AVRCP_EVENT_PLAYBACK_STATUS_CHANGED:
               Message = "AVRCP_EVENT_PLAYBACK_STATUS_CHANGED";
               if((ResponseData->MessageData.RegisterNotificationResponseData.ResponseCode == AVRCP_RESPONSE_CHANGED) || (ResponseData->MessageData.RegisterNotificationResponseData.ResponseCode == AVRCP_RESPONSE_INTERIM))
               {
                  CurrentAbsoluteVolume = ResponseData->MessageData.RegisterNotificationResponseData.NotificationData.VolumeChangedData.AbsoluteVolume;
               }
               break;
            case AVRCP_EVENT_TRACK_CHANGED:
               Message = "AVRCP_EVENT_TRACK_CHANGED";
               break;
            case AVRCP_EVENT_TRACK_REACHED_END:
               Message = "AVRCP_EVENT_TRACK_REACHED_END";
               break;
            case AVRCP_EVENT_TRACK_REACHED_START:
               Message = "AVRCP_EVENT_TRACK_REACHED_START";
               break;
            case AVRCP_EVENT_PLAYBACK_POS_CHANGED:
               Message = "AVRCP_EVENT_PLAYBACK_POS_CHANGED";
               break;
            case AVRCP_EVENT_BATT_STATUS_CHANGED:
               Message = "AVRCP_EVENT_BATT_STATUS_CHANGED";
               break;
            case AVRCP_EVENT_SYSTEM_STATUS_CHANGED:
               Message = "AVRCP_EVENT_SYSTEM_STATUS_CHANGED";
               break;
            case AVRCP_EVENT_PLAYER_APPLICATION_SETTING_CHANGED:
               Message = "AVRCP_EVENT_PLAYER_APPLICATION_SETTING_CHANGED";
               break;
            case AVRCP_EVENT_NOW_PLAYING_CONTENT_CHANGED:
               Message = "AVRCP_EVENT_NOW_PLAYING_CONTENT_CHANGED";
               break;
            case AVRCP_EVENT_AVAILABLE_PLAYERS_CHANGED:
               Message = "AVRCP_EVENT_AVAILABLE_PLAYERS_CHANGED";
               break;
            case AVRCP_EVENT_ADDRESSED_PLAYER_CHANGED:
               Message = "AVRCP_EVENT_ADDRESSED_PLAYER_CHANGED";
               break;
            case AVRCP_EVENT_UIDS_CHANGED:
               Message = "AVRCP_EVENT_UIDS_CHANGED";
               break;
            case AVRCP_EVENT_VOLUME_CHANGED:
               Message = "AVRCP_EVENT_VOLUME_CHANGED";
               if((ResponseData->MessageData.RegisterNotificationResponseData.ResponseCode == AVRCP_RESPONSE_CHANGED) || (ResponseData->MessageData.RegisterNotificationResponseData.ResponseCode == AVRCP_RESPONSE_INTERIM))
               {
                  CurrentAbsoluteVolume = ResponseData->MessageData.RegisterNotificationResponseData.NotificationData.VolumeChangedData.AbsoluteVolume;
               }
               break;
            default:
               Message = "Unknown";
               break;
         }

         printf("    Event        : %s\r\n", Message);

         switch(ResponseData->MessageData.RegisterNotificationResponseData.EventID)
         {
            case AVRCP_EVENT_PLAYBACK_STATUS_CHANGED:
               if((ResponseData->MessageData.RegisterNotificationResponseData.ResponseCode == AVRCP_RESPONSE_CHANGED) || (ResponseData->MessageData.RegisterNotificationResponseData.ResponseCode == AVRCP_RESPONSE_INTERIM))
               {
                  CurrentPlayStatus = ResponseData->MessageData.RegisterNotificationResponseData.NotificationData.PlaybackStatusChangedData.PlayStatus;
                  printf("    Play Status  : ");
                  DisplayRemoteControlPlaybackStatus(CurrentPlayStatus);
                  printf("\r\n");
               }
               break;
            case AVRCP_EVENT_PLAYER_APPLICATION_SETTING_CHANGED:
               if((ResponseData->MessageData.RegisterNotificationResponseData.ResponseCode == AVRCP_RESPONSE_CHANGED) || (ResponseData->MessageData.RegisterNotificationResponseData.ResponseCode == AVRCP_RESPONSE_INTERIM))
               {
                  printf("    # Attributes : %d\r\n", ResponseData->MessageData.RegisterNotificationResponseData.NotificationData.PlayerApplicationSettingChangedData.NumberAttributeValueIDs);
                  fflush(stdout);
                  for(Index=0;Index<(ResponseData->MessageData.RegisterNotificationResponseData.NotificationData.PlayerApplicationSettingChangedData.NumberAttributeValueIDs);Index++)
                  {
                     printf("    Attribute %d  : ", Index + 1);
                     fflush(stdout);
                     DisplayRemoteControlAttributeValue(&(ResponseData->MessageData.RegisterNotificationResponseData.NotificationData.PlayerApplicationSettingChangedData.AttributeValueIDList[Index]));
                     printf("\r\n");
                     fflush(stdout);
                  }
               }
               break;
            case AVRCP_EVENT_VOLUME_CHANGED:
               if((ResponseData->MessageData.RegisterNotificationResponseData.ResponseCode == AVRCP_RESPONSE_CHANGED) || (ResponseData->MessageData.RegisterNotificationResponseData.ResponseCode == AVRCP_RESPONSE_INTERIM))
               {
                  CurrentAbsoluteVolume = ResponseData->MessageData.RegisterNotificationResponseData.NotificationData.VolumeChangedData.AbsoluteVolume;
                  printf("    Volume       : %d%%\r\n", ABSOLUTE_VOLUME_TO_PERCENTAGE(CurrentAbsoluteVolume));
               }
               break;
            default:
               break;
         }

         if(ResponseData->MessageData.RegisterNotificationResponseData.ResponseCode == AVRCP_RESPONSE_CHANGED)
         {
            printf("Register Notification Command Complete. Re-register for this notification using: AUDSendRemoteControlRegisterNotificationCommand.\r\n");
         }
         break;
      case amtGetTotalNumberOfItems:
         printf("    Status: 0x%02X.\r\n", ResponseData->MessageData.GetTotalNumberOfItemsResponseData.Status);
         printf("    UIDCounter: %u.\r\n", (unsigned int)ResponseData->MessageData.GetTotalNumberOfItemsResponseData.UIDCounter);
         printf("    Total Items: %u.\r\n", (unsigned int)ResponseData->MessageData.GetTotalNumberOfItemsResponseData.NumberOfItems);
         break;
      default:
         break;
   }
}

   /* This is a utility function to process events in which the         */
   /* stream state changes and make appropriate actions for the         */
   /* encoder/decoder.                                                  */
static void ProcessStreamStateChangeEvent(BD_ADDR_t RemoteDeviceAddress, AUD_Stream_Type_t Type, AUD_Stream_State_t State, AudioStream_t *Stream)
{
   if(Type == astSNK)
   {
      if(State == astStreamStopped)
      {
         /* If this is the device that took control of streaming, clean */
         /* it up.                                                      */
         if(COMPARE_BD_ADDR(CurrentDecodingBD_ADDR, RemoteDeviceAddress))
         {
            printf("\r\nCleaning up Audio Decoder for stream.\r\n");
            CleanupAudioDecoder();
            ASSIGN_BD_ADDR(CurrentDecodingBD_ADDR, 0, 0, 0, 0, 0, 0);
         }
      }
      else
      {
         if(DecoderConfigFlags)
         {
            /* If we do not currently have a stream playing, bring the  */
            /* the decoder for this stream.                             */
            if(COMPARE_NULL_BD_ADDR(CurrentDecodingBD_ADDR))
            {
               printf("\r\nInitializing playback for stream.\r\n");
               CurrentDecodingBD_ADDR = RemoteDeviceAddress;
               InitializeAudioDecoder(DecoderConfigFlags, CurrentDecodingBD_ADDR, AudioFileName);
            }
            else
            {
               printf("\r\nPlayback already intialized for another stream.\r\n");
            }
         }
         else
         {
            printf("No playback mode configured\n");
         }
      }
   }
   else
   {

      /* If the Playback Thread is running, signal to it that the state */
      /* has changed.                                                   */
      /* * NOTE * It is possible that this event handler is the         */
      /*          Remote Control Event Handler, in which case the       */
      /*          Callback Parameter will be NULL.                      */
      if((Stream) && (COMPARE_BD_ADDR(CurrentEncodingBD_ADDR, RemoteDeviceAddress)) && (State == astStreamStopped))
      {
         /* Update the cached state and flag the notification event for */
         /* the change.                                                 */
         Stream->StreamState = State;

         BTPS_SetEvent(Stream->StreamStateChanged);
      }
   }
}

   /* The following function is the Callback function that is installed */
   /* to be notified when any local IPC connection to the server has    */
   /* been lost.  This case only occurs when the Server exits.  This    */
   /* callback allows the application mechanism to be notified so that  */
   /* all resources can be cleaned up (i.e.  call BTPM_Cleanup().       */
void BTPSAPI ServerUnRegistrationCallback(void *CallbackParameter)
{
   printf("Server has been Un-Registered.\r\n");

   printf("AUDM>");

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

   printf("AUDM>");

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

            /* Inform the user of the Remote I/O Capabilities.          */
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

   printf("AUDM>");

   /* Make sure the output is displayed to the user.                    */
   fflush(stdout);
}

   /* BTPM Audio Manager Callback function prototype.                   */
static void BTPSAPI AUDM_Event_Callback(AUDM_Event_Data_t *EventData, void *CallbackParameter)
{
   char      *Message;
   char       Buffer[32];
   Boolean_t  DisplayPrompt = TRUE;

   if(EventData)
   {
      switch(EventData->EventType)
      {
         case aetIncomingConnectionRequest:
            printf("\nAUDM: Incoming Connection Request.\r\n");

            BD_ADDRToStr((EventData->EventData.IncomingConnectionRequestEventData.RemoteDeviceAddress), Buffer);

            printf("    Remote Address: %s\r\n", Buffer);
            printf("    Request Type:   %s\r\n", ((EventData->EventData.IncomingConnectionRequestEventData.RequestType == acrStream)?"Audio Stream":"Remote Control"));

            printf("Respond with the AUDConnectionRequestResponse command.\r\n");
            break;
         case aetAudioStreamConnected:
            printf("\nAUDM: Audio Stream Connected.\r\n");

            BD_ADDRToStr((EventData->EventData.AudioStreamConnectedEventData.RemoteDeviceAddress), Buffer);

            printf("    Type     : %s\r\n",  (EventData->EventData.AudioStreamConnectedEventData.StreamType == astSNK)?"Sink":"Source");
            printf("    Address  : %s\r\n",  Buffer);
            printf("    MTU      : %u\r\n",  EventData->EventData.AudioStreamConnectedEventData.MediaMTU);
            printf("    Frequency: %lu\r\n", EventData->EventData.AudioStreamConnectedEventData.StreamFormat.SampleFrequency);
            printf("    Channels : %u\r\n",  EventData->EventData.AudioStreamConnectedEventData.StreamFormat.NumberChannels);
            printf("    Flags    : %lu\r\n", EventData->EventData.AudioStreamConnectedEventData.StreamFormat.FormatFlags);

            /* Check if a callback parameter was specified.             */
            if(CallbackParameter)
            {
               /* Reset the state of the Stream State Changed Event.    */
               ((AudioStream_t *)CallbackParameter)->StreamState = astStreamStopped;

               BTPS_ResetEvent(((AudioStream_t *)CallbackParameter)->StreamStateChanged);
            }

            break;
         case aetAudioStreamConnectionStatus:
            printf("\nAUDM: Audio Stream Connection Status: ");

            BD_ADDRToStr((EventData->EventData.AudioStreamConnectionStatusEventData.RemoteDeviceAddress), Buffer);

            switch(EventData->EventData.AudioStreamConnectionStatusEventData.ConnectionStatus)
            {
               case AUDM_STREAM_CONNECTION_STATUS_SUCCESS:
                  Message = "Success";
                  break;
               case AUDM_STREAM_CONNECTION_STATUS_FAILURE_TIMEOUT:
                  Message = "Timed out";
                  break;
               case AUDM_STREAM_CONNECTION_STATUS_FAILURE_REFUSED:
                  Message = "Refused";
                  break;
               case AUDM_STREAM_CONNECTION_STATUS_FAILURE_SECURITY:
                  Message = "Security";
                  break;
               case AUDM_STREAM_CONNECTION_STATUS_FAILURE_DEVICE_POWER_OFF:
                  Message = "Device Power Off";
                  break;
               case AUDM_STREAM_CONNECTION_STATUS_FAILURE_UNKNOWN:
               default:
                  Message = "Unknown Error";
                  break;
            }

            printf("%u, %s.\r\n", EventData->EventData.AudioStreamConnectionStatusEventData.ConnectionStatus, Message);
            printf("    Address  : %s\r\n",  Buffer);
            printf("    Type     : %s\r\n",  (EventData->EventData.AudioStreamConnectionStatusEventData.StreamType == astSNK)?"Sink":"Source");

            if(EventData->EventData.AudioStreamConnectionStatusEventData.ConnectionStatus == AUDM_STREAM_CONNECTION_STATUS_SUCCESS)
            {
               printf("    MTU      : %u\r\n",  EventData->EventData.AudioStreamConnectionStatusEventData.MediaMTU);
               printf("    Frequency: %lu\r\n", EventData->EventData.AudioStreamConnectionStatusEventData.StreamFormat.SampleFrequency);
               printf("    Channels : %u\r\n",  EventData->EventData.AudioStreamConnectionStatusEventData.StreamFormat.NumberChannels);
               printf("    Flags    : %lu\r\n", EventData->EventData.AudioStreamConnectionStatusEventData.StreamFormat.FormatFlags);

               /* Check if a callback parameter was specified.          */
               if(CallbackParameter)
               {
                  /* Reset the state of the Stream State Changed Event. */
                  ((AudioStream_t *)CallbackParameter)->StreamState = astStreamStopped;

                  BTPS_ResetEvent(((AudioStream_t *)CallbackParameter)->StreamStateChanged);
               }
            }
            break;
         case aetAudioStreamDisconnected:
            printf("\nAUDM: Audio Stream Disconnected: ");

            BD_ADDRToStr((EventData->EventData.AudioStreamDisconnectedEventData.RemoteDeviceAddress), Buffer);

            if(EventData->EventData.AudioStreamDisconnectedEventData.StreamType == astSNK)
               printf("Sink.\r\n");
            else
               printf("Source.\r\n");

            /* If the Playback Thread is running, signal to it that the */
            /* state has changed.                                       */
            /* * NOTE * It is possible that this event handler is the   */
            /*          Remote Control Event Handler, in which case the */
            /*          Callback Parameter will be NULL.                */
            if(CallbackParameter)
            {
               /* Update the cached state and flag the notification     */
               /* event for the change.                                 */
               ((AudioStream_t *)CallbackParameter)->StreamState = astStreamStopped;

               BTPS_SetEvent(((AudioStream_t *)CallbackParameter)->StreamStateChanged);
            }
            break;
         case aetAudioStreamStateChanged:
            printf("\nAUDM: Audio Stream State Changed.\r\n");

            BD_ADDRToStr((EventData->EventData.AudioStreamStateChangedEventData.RemoteDeviceAddress), Buffer);

            printf("    Address: %s\r\n",  Buffer);
            printf("    Type   : %s\r\n", (EventData->EventData.AudioStreamStateChangedEventData.StreamType == astSNK)?"Sink":"Source");
            printf("    State  : %s\r\n", (EventData->EventData.AudioStreamStateChangedEventData.StreamState == astStreamStopped)?"Stopped":"Playing");

            ProcessStreamStateChangeEvent(
                  EventData->EventData.AudioStreamStateChangedEventData.RemoteDeviceAddress,
                  EventData->EventData.AudioStreamStateChangedEventData.StreamType,
                  EventData->EventData.AudioStreamStateChangedEventData.StreamState,
                  (AudioStream_t *)CallbackParameter);

            break;
         case aetChangeAudioStreamStateStatus:
            printf("\nAUDM: Change Audio Stream State Status: %s\r\n", ((EventData->EventData.ChangeAudioStreamStateStatusEventData.Successful == FALSE)?"Failure":"Success"));

            BD_ADDRToStr((EventData->EventData.ChangeAudioStreamStateStatusEventData.RemoteDeviceAddress), Buffer);

            printf("    Address: %s\r\n",  Buffer);
            printf("    Type   : %s\r\n", (EventData->EventData.ChangeAudioStreamStateStatusEventData.StreamType == astSNK)?"Sink":"Source");
            printf("    State  : %s\r\n", (EventData->EventData.ChangeAudioStreamStateStatusEventData.StreamState == astStreamStopped)?"Stopped":"Playing");

            ProcessStreamStateChangeEvent(
                  EventData->EventData.ChangeAudioStreamStateStatusEventData.RemoteDeviceAddress,
                  EventData->EventData.ChangeAudioStreamStateStatusEventData.StreamType,
                  EventData->EventData.ChangeAudioStreamStateStatusEventData.StreamState,
                  (AudioStream_t *)CallbackParameter);

            break;
         case aetAudioStreamFormatChanged:
            printf("\nAUDM: Audio Stream Format Changed.\r\n");

            BD_ADDRToStr((EventData->EventData.AudioStreamFormatChangedEventData.RemoteDeviceAddress), Buffer);

            printf("    Address  : %s\r\n",  Buffer);
            printf("    Type     : %s\r\n", (EventData->EventData.AudioStreamFormatChangedEventData.StreamType == astSNK)?"Sink":"Source");
            printf("    Frequency: %lu\r\n", EventData->EventData.AudioStreamFormatChangedEventData.StreamFormat.SampleFrequency);
            printf("    Channels : %u\r\n", EventData->EventData.AudioStreamFormatChangedEventData.StreamFormat.NumberChannels);
            printf("    Flags    : %lu\r\n", EventData->EventData.AudioStreamFormatChangedEventData.StreamFormat.FormatFlags);
            break;
         case aetChangeAudioStreamFormatStatus:
            printf("\nAUDM: Change Audio Stream Format Status: %s\r\n", ((EventData->EventData.ChangeAudioStreamFormatStatusEventData.Successful == FALSE)?"Failure":"Success"));

            BD_ADDRToStr((EventData->EventData.ChangeAudioStreamFormatStatusEventData.RemoteDeviceAddress), Buffer);

            printf("    Address  : %s\r\n",  Buffer);
            printf("    Type     : %s\r\n", (EventData->EventData.ChangeAudioStreamFormatStatusEventData.StreamType == astSNK)?"Sink":"Source");
            printf("    Frequency: %lu\r\n", EventData->EventData.ChangeAudioStreamFormatStatusEventData.StreamFormat.SampleFrequency);
            printf("    Channels : %u\r\n", EventData->EventData.ChangeAudioStreamFormatStatusEventData.StreamFormat.NumberChannels);
            printf("    Flags    : %lu\r\n", EventData->EventData.ChangeAudioStreamFormatStatusEventData.StreamFormat.FormatFlags);
            break;
         case aetEncodedAudioStreamData:
            /* If this is currently the active decoding stream, pass the*/
            /* audio data to the decoder.                               */
            if(COMPARE_BD_ADDR(CurrentDecodingBD_ADDR, EventData->EventData.EncodedAudioStreamDataEventData.RemoteDeviceAddress))
               ProcessAudioData(EventData->EventData.EncodedAudioStreamDataEventData.RawAudioDataFrame, EventData->EventData.EncodedAudioStreamDataEventData.RawAudioDataFrameLength);
            else
            {
               if(IgnoredAudioPackets >= IGNORED_AUDIO_PACKETS_THRESHOLD)
               {
                  printf("\r\nEncoded Audio Data Received for non-playing thread:\r\n");

                  BD_ADDRToStr(EventData->EventData.EncodedAudioStreamDataEventData.RemoteDeviceAddress, Buffer);

                  printf("    Address:     %s\r\n", Buffer);
                  printf("    Data Length: %u\r\n", EventData->EventData.EncodedAudioStreamDataEventData.RawAudioDataFrameLength);

                  IgnoredAudioPackets = 0;
               }
               else
                  IgnoredAudioPackets++;
            }

            DisplayPrompt = FALSE;
            break;
         case aetRemoteControlConnected:
            printf("\nAUDM: Remote Control Connected.\r\n");

            BD_ADDRToStr((EventData->EventData.RemoteControlConnectedEventData.RemoteDeviceAddress), Buffer);

            printf("    Address: %s\r\n", Buffer);
            break;
         case aetRemoteControlConnectionStatus:
            printf("\nAUDM: Remote Control Connection Status: ");

            switch(EventData->EventData.RemoteControlConnectionStatusEventData.ConnectionStatus)
            {
               case AUDM_REMOTE_CONTROL_CONNECTION_STATUS_SUCCESS:
                  Message = "Success";
                  break;
               case AUDM_REMOTE_CONTROL_CONNECTION_STATUS_FAILURE_TIMEOUT:
                  Message = "Timed out";
                  break;
               case AUDM_REMOTE_CONTROL_CONNECTION_STATUS_FAILURE_REFUSED:
                  Message = "Refused";
                  break;
               case AUDM_REMOTE_CONTROL_CONNECTION_STATUS_FAILURE_SECURITY:
                  Message = "Security";
                  break;
               case AUDM_REMOTE_CONTROL_CONNECTION_STATUS_FAILURE_DEVICE_POWER_OFF:
                  Message = "Device Power Off";
                  break;
               case AUDM_REMOTE_CONTROL_CONNECTION_STATUS_FAILURE_UNKNOWN:
               default:
                  Message = "Unknown Error";
                  break;
            }

            printf("%u, %s.\r\n", EventData->EventData.RemoteControlConnectionStatusEventData.ConnectionStatus, Message);
            break;
         case aetRemoteControlDisconnected:
            printf("\nAUDM: Remote Control Disconnected.\r\n");

            BD_ADDRToStr((EventData->EventData.RemoteControlDisconnectedEventData.RemoteDeviceAddress), Buffer);

            switch(EventData->EventData.RemoteControlDisconnectedEventData.DisconnectReason)
            {
               case adrRemoteDeviceDisconnect:
                  Message = "Disconnection Requested";
                  break;
               case adrRemoteDeviceLinkLoss:
                  Message = "Link Lost";
                  break;
               case adrRemoteDeviceTimeout:
                  Message = "Connection Timeout";
                  break;
               default:
                  Message = "Unknown Reason";
                  break;
            }
            printf("    Address: %s\r\n", Buffer);
            printf("    Reason : %s\r\n", Message);
            break;
         case aetRemoteControlCommandIndication:
            printf("\nAUDM: Remote Control Command Indication.\r\n");
            BD_ADDRToStr((EventData->EventData.RemoteControlCommandIndicationEventData.RemoteDeviceAddress), Buffer);
            printf("    Address      : %s\r\n", Buffer);
            printf("    TransactionID: %u\r\n", EventData->EventData.RemoteControlCommandIndicationEventData.TransactionID);
            ProcessRemoteControlCommandIndication(EventData->EventData.RemoteControlCommandIndicationEventData.RemoteDeviceAddress, EventData->EventData.RemoteControlCommandIndicationEventData.TransactionID, &(EventData->EventData.RemoteControlCommandIndicationEventData.RemoteControlCommandData));
            break;
         case aetRemoteControlCommandConfirmation:
            printf("\nAUDM: Remote Control Command Confirmation.\r\n");
            BD_ADDRToStr((EventData->EventData.RemoteControlCommandConfirmationEventData.RemoteDeviceAddress), Buffer);
            printf("    Address      : %s\r\n", Buffer);
            printf("    TransactionID: %u\r\n", EventData->EventData.RemoteControlCommandConfirmationEventData.TransactionID);
            printf("    Status       : %d (%s)\r\n", EventData->EventData.RemoteControlCommandConfirmationEventData.Status, ERR_ConvertErrorCodeToString(EventData->EventData.RemoteControlCommandConfirmationEventData.Status));
            if(!(EventData->EventData.RemoteControlCommandConfirmationEventData.Status))
               ProcessRemoteControlCommandConfirmation(&(EventData->EventData.RemoteControlCommandConfirmationEventData.RemoteControlResponseData));
            break;
         case aetRemoteControlBrowsingConnected:
            printf("\nAUDM: Remote Control Browsing Connected.\r\n");

            BD_ADDRToStr((EventData->EventData.RemoteControlBrowsingConnectedEventData.RemoteDeviceAddress), Buffer);

            printf("    Address: %s\r\n", Buffer);
            break;
         case aetRemoteControlBrowsingConnectionStatus:
            printf("\nAUDM: Remote Control Browsing Connection Status: ");

            switch(EventData->EventData.RemoteControlBrowsingConnectionStatusEventData.ConnectionStatus)
            {
               case AUDM_REMOTE_CONTROL_CONNECTION_STATUS_SUCCESS:
                  Message = "Success";
                  break;
               case AUDM_REMOTE_CONTROL_CONNECTION_STATUS_FAILURE_TIMEOUT:
                  Message = "Timed out";
                  break;
               case AUDM_REMOTE_CONTROL_CONNECTION_STATUS_FAILURE_REFUSED:
                  Message = "Refused";
                  break;
               case AUDM_REMOTE_CONTROL_CONNECTION_STATUS_FAILURE_SECURITY:
                  Message = "Security";
                  break;
               case AUDM_REMOTE_CONTROL_CONNECTION_STATUS_FAILURE_DEVICE_POWER_OFF:
                  Message = "Device Power Off";
                  break;
               case AUDM_REMOTE_CONTROL_CONNECTION_STATUS_FAILURE_UNKNOWN:
               default:
                  Message = "Unknown Error";
                  break;
            }

            printf("%u, %s.\r\n", EventData->EventData.RemoteControlBrowsingConnectionStatusEventData.ConnectionStatus, Message);
            break;
         case aetRemoteControlBrowsingDisconnected:
            printf("\nAUDM: Remote Control Browsing Disconnected.\r\n");

            BD_ADDRToStr((EventData->EventData.RemoteControlBrowsingDisconnectedEventData.RemoteDeviceAddress), Buffer);
            printf("    Address: %s\r\n", Buffer);
            break;
         default:
            printf("\nAUDM: Unknown Audio Manager Event Received: 0x%08X, Length: 0x%08X.\r\n", (unsigned int)EventData->EventType, EventData->EventLength);
            break;
      }
   }
   else
      printf("\r\nAUDM Event Data is NULL.\r\n");

   if(DisplayPrompt)
   {
      printf("AUDM>");

      /* Make sure the output is displayed to the user.                 */
      fflush(stdout);
   }
}

   /* Main Program Entry Point.                                         */
int main(int argc, char *argv[])
{
   /* Initialize the default Secure Simple Pairing parameters.          */
   IOCapability                        = DEFAULT_IO_CAPABILITY;
   OOBSupport                          = FALSE;
   MITMProtection                      = DEFAULT_MITM_PROTECTION;

   /* Go ahead and initialize the control Events for each of the Audio  */
   /* Streams.                                                          */
   AudioSourceState.StreamStateChanged = BTPS_CreateEvent(FALSE);

   AudioSinkState.StreamStateChanged   = BTPS_CreateEvent(FALSE);

   /* Setup the default playback parameters.                            */
   DecoderConfigFlags = DEFAULT_DECODER_CONFIG_FLAGS;
   strncpy(AudioFileName, DECODER_DEFAULT_WAV_FILE_NAME, MAX_AUDIO_FILE_NAME_LENGTH);
   AudioFileName[MAX_AUDIO_FILE_NAME_LENGTH-1] = '\0';

   if((AudioSourceState.StreamStateChanged) && (AudioSinkState.StreamStateChanged))
   {
      /* Nothing really to do here aside from running the main          */
      /* application code.                                              */
      UserInterface();
   }
   else
      printf("\r\nError: Unable to create Events for Stream Endpoint Signalling.\r\n");

   /* Clean up any Events created for the Audio Streams.                */
   if(AudioSourceState.StreamStateChanged)
      BTPS_CloseEvent(AudioSourceState.StreamStateChanged);

   if(AudioSinkState.StreamStateChanged)
      BTPS_CloseEvent(AudioSinkState.StreamStateChanged);

   /* Return success.                                                   */
   return(0);
}
