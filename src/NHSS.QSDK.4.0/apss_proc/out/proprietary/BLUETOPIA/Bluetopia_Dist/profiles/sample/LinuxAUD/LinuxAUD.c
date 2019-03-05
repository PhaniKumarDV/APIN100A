/*****< linuxaud.c >***********************************************************/
/*      Copyright 2011 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  LINUXAUD - Simple Linux application using Audio Profile Sub-System        */
/*                                                                            */
/*  Author:  Matt Seabold                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   07/05/11  M. Seabold     Initial creation.                               */
/******************************************************************************/
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>

#include "LinuxAUD.h"      /* Main Application Prototypes and Constants.      */

#include "SS1BTPS.h"       /* Includes for the SS1 Bluetooth Protocol Stack.  */
#include "SS1BTAUD.h"      /* Includes for the SS1 Audio Profile Sub-System.  */
#include "SS1BTDBG.h"      /* Includes/Constants for Bluetooth Debugging.     */

#include "AudioEncoder.h"  /* Audio Encoder sample.                           */
#include "AudioDecoder.h"  /* Audio Decoder sample.                           */

#define LOCAL_NAME_ROOT                      "LinuxAUD"  /* Root of the local */
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

#define MAX_SUPPORTED_COMMANDS                     (45)  /* Denotes the       */
                                                         /* maximum number of */
                                                         /* User Commands that*/
                                                         /* are supported by  */
                                                         /* this application. */

#define MAX_COMMAND_LENGTH                         (64)  /* Denotes the max   */
                                                         /* buffer size used  */
                                                         /* for user commands */
                                                         /* input via the     */
                                                         /* User Interface.   */

#define MAX_NUM_OF_PARAMETERS                       (4)  /* Denotes the max   */
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

#define ENDPOINT_TYPE_SRC                           (1)  /* Flags which denote*/
                                                         /* which type of     */
#define ENDPOINT_TYPE_SNK                           (2)  /* endpoints the app */
                                                         /* is initialized as.*/

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

#define WAV_FORMAT_ERROR                           (-9)  /* Denotes than an   */
                                                         /* error occurred due*/
                                                         /* to the fact that  */
                                                         /* the WAV Format    */
                                                         /* parsed from the   */
                                                         /* WAV header was    */
                                                         /* invalid or is not */
                                                         /* supported.        */

#define WAV_PCM_BUFFER_SIZE                       10240  /* Denotes the size  */
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

#define INDENT_LENGTH                                 3  /* Denotes the number*/
                                                         /* of character      */
                                                         /* spaces to be used */
                                                         /* for indenting when*/
                                                         /* displaying SDP    */
                                                         /* Data Elements.    */

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

#define DEFAULT_ABSOLUTE_VOLUME                    (0x7F/2) /* 50%.           */

                                                         /* Macro to convert  */
                                                         /* a volume percent  */
                                                         /* to a scaled AVRCP */
                                                         /* value.            */
#define ABSOLUTE_VOLUME_TO_PERCENTAGE(_x)          ((_x * 100)/0x7F)

                                                         /* Macro to convert  */
                                                         /* a scaled AVRCP    */
                                                         /* volume value to a */
                                                         /* percentage.       */
#define ABSOLUTE_VOLUME_FROM_PERCENTAGE(_x)        ((_x * 0x7F)/100) 

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

   /* The following constants represent the default log file names that */
   /* are used if no Log file name is specified when enabling debug.    */
#define DEFAULT_DEBUG_LOG_FILE_NAME  "LinuxAUD_ASC.log"
#define DEFAULT_DEBUG_FTS_FILE_NAME  "LinuxAUD_FTS.log"

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
   unsigned int  BluetoothStackID;
   int           FileDescriptor;
   WAVInfo_t     WAVInfo;
   unsigned long Offset;
   Boolean_t     Loop;
} PlaybackThreadParams_t;

   /* The following structure is used to build a table to map integer   */
   /* keys to string representations.                                   */
typedef struct _tagIntToStrMap_t
{
   unsigned int  IntKey;
   char         *StrVal;
} IntToStrMap_t;

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the         */
   /* compiler as part of standard C/C++).                              */
static unsigned int        EndPointType;            /* Variable used to indicate if the*/
                                                    /* program is to be run SRC, SNK,  */
                                                    /* or SRC and SNK mode.            */

static Boolean_t           Initialized;             /* Variable used to denote whether */
                                                    /* the AUD modules has been        */
                                                    /* initialized.                    */

static Boolean_t           Connection;              /* Variable used to indicate       */
                                                    /* whether the module currently has*/
                                                    /* a connection.                   */

static Boolean_t           RemoteControlConnection; /* Variable used to indicate       */
                                                    /* whether the module currently has*/
                                                    /* an on-going remote control      */
                                                    /* connection.                     */

static unsigned int        BluetoothStackID;        /* Variable which holds the Handle */
                                                    /* of the opened Bluetooth Protocol*/
                                                    /* Stack.                          */

static unsigned int        DebugID;                 /* Variable which holds the ID of  */
                                                    /* the currently enabled debugging */
                                                    /* session.                        */

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

static BD_ADDR_t           CurrentSNK_BD_ADDR;      /* Variable which holds the current*/
                                                    /* BD_ADDR of the device that is   */
                                                    /* connected to a current SNK      */
                                                    /* endpoint.                       */

static BD_ADDR_t           CurrentSRC_BD_ADDR;      /* Variable which holds the current*/
                                                    /* BD_ADDR of the device that is   */
                                                    /* connected to a current SNK      */
                                                    /* endpoint.                       */

static BD_ADDR_t           CurrentRC_BD_ADDR;       /* Variable which holds the current*/
                                                    /* BD_ADDR of the device that is   */
                                                    /* connected to a current RC       */
                                                    /* endpoint.                       */

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

unsigned long              FileOffset;              /* Variable which keeps track of   */
                                                    /* the location within a plaing WAV*/
                                                    /* file from which to resume.      */

static unsigned int        NumberCommands;          /* Variable which is used to hold  */
                                                    /* the number of Commands that are */
                                                    /* supported by this application.  */
                                                    /* Commands are added individually.*/

static CommandTable_t      CommandTable[MAX_SUPPORTED_COMMANDS]; /* Variable which is  */
                                                    /* used to hold the actual Commands*/
                                                    /* that are supported by this      */
                                                    /* application.                    */

static Event_t             SourceStreamStateChangedEvent; /* Event indicating that a   */
                                                    /* Stream State Changed event has  */
                                                    /* been received for the SRC       */
                                                    /* endpoint.                       */

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

static Boolean_t           SinkStreamStarted;       /* Variable which flags whether we */
                                                    /* are currently streaming data as */
                                                    /* a sink.                         */

static Byte_t              CurrentAbsVolume;        /* Variable which tracks the last  */
                                                    /* notified Absolute Volume.       */

static Boolean_t           VolumeChangedRegistered; /* Variable which tracks whether a */
                                                    /* remote source has register for  */
                                                    /* Volume Changed Notifications.   */

static AUD_Stream_Format_t AudioSRCSupportedFormats[] =
{
   { 44100, 2, AUD_STREAM_FORMAT_FLAGS_CODEC_TYPE_SBC },
   { 48000, 2, AUD_STREAM_FORMAT_FLAGS_CODEC_TYPE_SBC },
   { 48000, 1, AUD_STREAM_FORMAT_FLAGS_CODEC_TYPE_SBC },
   { 44100, 1, AUD_STREAM_FORMAT_FLAGS_CODEC_TYPE_SBC }
} ;

#define NUM_SRC_SUPPORTED_FORMATS      (sizeof(AudioSRCSupportedFormats)/sizeof(AUD_Stream_Format_t))

static AUD_Stream_Format_t AudioSNKSupportedFormats[] =
{
   { 44100, 2, AUD_STREAM_FORMAT_FLAGS_CODEC_TYPE_SBC },
   { 48000, 2, AUD_STREAM_FORMAT_FLAGS_CODEC_TYPE_SBC },
   { 48000, 1, AUD_STREAM_FORMAT_FLAGS_CODEC_TYPE_SBC },
   { 44100, 1, AUD_STREAM_FORMAT_FLAGS_CODEC_TYPE_SBC }
} ;

#define NUM_SNK_SUPPORTED_FORMATS      (sizeof(AudioSNKSupportedFormats)/sizeof(AUD_Stream_Format_t))

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

   /* List of common passthrough commands that may want to be used      */
   /* in this sample. Other commands can still be explicitly sent       */
   /* using their IDs, but this allows us to print out the common       */
   /* ones. This can be ammended with more IDs from AVRCP in the future */
   /* if necessary.                                                     */
static IntToStrMap_t PassThroughCommandTable[] = 
{
   { AVRCP_PASS_THROUGH_ID_PLAY,        "Play"        },
   { AVRCP_PASS_THROUGH_ID_PAUSE,       "Pause"       },
   { AVRCP_PASS_THROUGH_ID_STOP,        "Stop"        },
   { AVRCP_PASS_THROUGH_ID_VOLUME_UP,   "Volume Up"   },
   { AVRCP_PASS_THROUGH_ID_VOLUME_DOWN, "Volume Down" },
   { AVRCP_PASS_THROUGH_ID_FORWARD,     "Forward"     },
   { AVRCP_PASS_THROUGH_ID_BACKWARD,    "Backward"    },
};

#define NUMBER_PASSTHROUGH_COMMANDS             (sizeof(PassThroughCommandTable)/sizeof(IntToStrMap_t))

   /* List of AVRCP Response Codes.                                     */
static IntToStrMap_t AVRResponseCodeTable[] =
{
   { AVRCP_RESPONSE_NOT_IMPLEMENTED, "Not Implemented" },
   { AVRCP_RESPONSE_ACCEPTED,        "Accepted"        },
   { AVRCP_RESPONSE_REJECTED,        "Rejected"        },
   { AVRCP_RESPONSE_IN_TRANSITION,   "In Transition"   },
   { AVRCP_RESPONSE_STABLE,          "Stable"          },
   { AVRCP_RESPONSE_CHANGED,         "Changed"         },
   { AVRCP_RESPONSE_INTERIM,         "Interim"         }
};

#define NUMBER_AVR_RESPONSE_CODES               (sizeof(AVRResponseCodeTable)/sizeof(IntToStrMap_t))

   /* List of AVRCP Event IDs.                                          */
static IntToStrMap_t AVREventIDTable[] =
{
   { AVRCP_EVENT_VOLUME_CHANGED, "Volume Changed" }
};

#define NUMBER_AVR_EVENT_IDS                    (sizeof(AVREventIDTable)/sizeof(IntToStrMap_t))

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
static char *IntKeyToStr(unsigned int MapSize, IntToStrMap_t *Map, unsigned int Key);

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

static int OpenRequestResponse(ParameterList_t *TempParam);
static int OpenRemoteStream(ParameterList_t *TempParam);
static int CloseStream(ParameterList_t *TempParam);
static int OpenRemoteControl(ParameterList_t *TempParam);
static int CloseRemoteControl(ParameterList_t *TempParam);
static int PlayWAV(ParameterList_t *TempParam);
static int ChangeStreamState(ParameterList_t *TempParam);
static int QueryStreamState(ParameterList_t *TempParam);
static int ChangeStreamFormat(ParameterList_t *TempParam);
static int QueryStreamFormat(ParameterList_t *TempParam);
static int QuerySupportedFormats(ParameterList_t *TempParam);
static int QueryStreamConfiguration(ParameterList_t *TempParam);
static int ChangeConnectionMode(ParameterList_t *TempParam);
static int QueryConnectionMode(ParameterList_t *TempParam);
static int SendPassThroughCommand(ParameterList_t *TempParam);
static int ConfigurePlayback(ParameterList_t *TempParam);
static int CurrentPlaybackConfig(ParameterList_t *TempParam);
static int NotifyAbsoluteVolume(ParameterList_t *TempParam);
static int RegisterVolumeNotification(ParameterList_t *TempParam);
static int SetAbsoluteVolume(ParameterList_t *TempParam);

static int ParseWAVHeader(int FileDescriptor, WAVInfo_t *Info);
static void *PlaybackThreadMain(void *ThreadParameter);

static void DisplaySDPAttributeResponse(SDP_Service_Attribute_Response_Data_t *SDPServiceAttributeResponse, unsigned int InitLevel);
static void DisplaySDPSearchAttributeResponse(SDP_Service_Search_Attribute_Response_Data_t *SDPServiceSearchAttributeResponse);
static void DisplayDataElement(SDP_Data_Element_t *SDPDataElement, unsigned int Level);

static void ProcessRemoteControlCommandIndication(AUD_Remote_Control_Command_Indication_Data_t *CommandData);
static void ProcessRemoteControlCommandConfirmation(AUD_Remote_Control_Command_Confirmation_Data_t *ResponseData);

   /* BTPS Callback function prototypes.                                */
static void BTPSAPI GAP_Event_Callback(unsigned int BluetoothStackID, GAP_Event_Data_t *GAP_Event_Data, unsigned long CallbackParameter);
static void BTPSAPI SDP_Event_Callback(unsigned int BluetoothStackID, unsigned int SDPRequestID, SDP_Response_Data_t *SDP_Response_Data, unsigned long CallbackParameter);
static void BTPSAPI AUD_Event_Callback(unsigned int BluetoothStackID, AUD_Event_Data_t *AUD_Event_Data, unsigned long CallbackParameter);

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
   AddCommand("OPENREQUESTRESPONSE", OpenRequestResponse);
   AddCommand("OPENREMOTESTREAM", OpenRemoteStream);
   AddCommand("CLOSESTREAM", CloseStream);
   AddCommand("OPENREMOTECONTROL", OpenRemoteControl);
   AddCommand("CLOSEREMOTECONTROL", CloseRemoteControl);
   AddCommand("PLAYWAV", PlayWAV);
   AddCommand("CHANGESTREAMSTATE", ChangeStreamState);
   AddCommand("QUERYSTREAMSTATE", QueryStreamState);
   AddCommand("CHANGESTREAMFORMAT", ChangeStreamFormat);
   AddCommand("QUERYSTREAMFORMAT", QueryStreamFormat);
   AddCommand("QUERYSUPPORTEDFORMATS", QuerySupportedFormats);
   AddCommand("QUERYSTREAMCONFIG", QueryStreamConfiguration);
   AddCommand("CHANGECONNECTIONMODE", ChangeConnectionMode);
   AddCommand("QUERYCONNECTIONMODE", QueryConnectionMode);
   AddCommand("SENDPASSTHROUGHCOMMAND", SendPassThroughCommand);
   AddCommand("CONFIGUREPLAYBACK", ConfigurePlayback);
   AddCommand("CURRENTPLAYBACKCONFIG", CurrentPlaybackConfig);
   AddCommand("NOTIFYABSOLUTEVOLUME", NotifyAbsoluteVolume);
   AddCommand("REGISTERVOLUMENOTIFICATION", RegisterVolumeNotification);
   AddCommand("SETABSOLUTEVOLUME", SetAbsoluteVolume);
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
      printf("AUD>");

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
                     /* go ahead an close any streams we have opened.   */
                     AUD_Close_Stream(BluetoothStackID, CurrentSRC_BD_ADDR, astSRC);
                     AUD_Close_Stream(BluetoothStackID, CurrentSNK_BD_ADDR, astSNK);
                     break;
               }
            }
            else
               printf("Invalid Input.\r\n");
         }
      }
      else
      {
         /* Clean up Audio profile sub-system.                          */
         AUD_Un_Initialize(BluetoothStackID);

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

   /* The following function is a utility to get a displayable string   */
   /* for known integer key values.                                     */
static char *IntKeyToStr(unsigned int MapSize, IntToStrMap_t *Map, unsigned int Key)
{
   char         *ret_val;
   unsigned int  Index;

   ret_val = "Unknown";

   for(Index = 0; Index < MapSize; Index++)
   {
      if(Map[Index].IntKey == Key)
      {
         ret_val = Map[Index].StrVal;
         break;
      }
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

            BluetoothStackID              = Result;

            DebugID                       = 0;

            /* Initialize the default Secure Simple Pairing parameters. */
            IOCapability                  = DEFAULT_IO_CAPABILITY;
            OOBSupport                    = FALSE;
            MITMProtection                = DEFAULT_MITM_PROTECTION;

            SourceStreamStateChangedEvent = BTPS_CreateEvent(FALSE);

            DecoderConfigFlags = DEFAULT_DECODER_CONFIG_FLAGS;
            strncpy(AudioFileName, DECODER_DEFAULT_WAV_FILE_NAME, MAX_AUDIO_FILE_NAME_LENGTH);
            AudioFileName[MAX_AUDIO_FILE_NAME_LENGTH-1] = '\0';

            CurrentAbsVolume = DEFAULT_ABSOLUTE_VOLUME;

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
      /* Clean up Audio profile sub-system.                             */
      AUD_Un_Initialize(BluetoothStackID);

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

   /* The following function is responsible for initializing the Audio  */
   /* Manager with the proper endpoints noted in the EndPointType flag. */
static int Initialize(void)
{
   int                                      ret_val;
   AUD_Initialization_Info_t                InitializationInfo;
   AUD_Remote_Control_Role_Info_t           TargetRemoteControlRoleInfo;
   AUD_Remote_Control_Role_Info_t           ControllerRemoteControlRoleInfo;
   AUD_Stream_Initialization_Info_t         SRCStreamInitializationInfo;
   AUD_Stream_Initialization_Info_t         SNKStreamInitializationInfo;
   AUD_Remote_Control_Initialization_Info_t RemoteControlInitializationInfo;

   /* First, check to make sure that a valid Bluetooth Stack ID exists. */
   if(BluetoothStackID)
   {
      /* Next, check to make sure that the Audio Manager has not already*/
      /* been initialized.                                              */
      if(!Initialized)
      {
         /* Audio has not been initialized, now let's attempt to        */
         /* initialize it.                                              */
         memset(&InitializationInfo, 0, sizeof(InitializationInfo));

         /* Initialize that we are supporting AVRCP 1.0.                */
         memset(&RemoteControlInitializationInfo, 0, sizeof(RemoteControlInitializationInfo));

         RemoteControlInitializationInfo.SupportedVersion   = apvVersion1_4;
         InitializationInfo.RemoteControlInitializationInfo = &RemoteControlInitializationInfo;

         /* First, format up the Initialization parameters.             */
         if(EndPointType & ENDPOINT_TYPE_SRC)
         {
            /* Initialize the Stream Information.                       */
            SRCStreamInitializationInfo.EndpointSDPDescription       = "A2DP Source";
            SRCStreamInitializationInfo.NumberSupportedStreamFormats = NUM_SRC_SUPPORTED_FORMATS;

            BTPS_MemCopy(SRCStreamInitializationInfo.StreamFormat, AudioSRCSupportedFormats, sizeof(AudioSRCSupportedFormats));

            /* Finally add the Stream and AVRCP Initialization to the   */
            /* main Initialization structure.                           */
            InitializationInfo.SRCInitializationInfo                           = &SRCStreamInitializationInfo;
         }

         if(EndPointType & ENDPOINT_TYPE_SNK)
         {
            /* Initialize the Stream Information.                       */
            SNKStreamInitializationInfo.EndpointSDPDescription       = "A2DP Sink";
            SNKStreamInitializationInfo.NumberSupportedStreamFormats = NUM_SNK_SUPPORTED_FORMATS;

            BTPS_MemCopy(SNKStreamInitializationInfo.StreamFormat, AudioSNKSupportedFormats, sizeof(AudioSNKSupportedFormats));

            /* Finally add the Stream and AVRCP Initialization to the   */
            /* main Initialization structure.                           */
            InitializationInfo.SNKInitializationInfo                               = &SNKStreamInitializationInfo;
         }

         /* Next, initialize the AVRCP Remote Control Target            */
         /* Information.                                                */
         TargetRemoteControlRoleInfo.SupportedFeaturesFlags = (SDP_AVRCP_SUPPORTED_FEATURES_TARGET_CATEGORY_1 | SDP_AVRCP_SUPPORTED_FEATURES_TARGET_CATEGORY_2);
         TargetRemoteControlRoleInfo.ProviderName           = "Stonestreet One";
         TargetRemoteControlRoleInfo.ServiceName            = "AVRCP Target";

         /* Next, initialize the AVRCP Remote Control Controller        */
         /* Information.                                                */
         ControllerRemoteControlRoleInfo.SupportedFeaturesFlags = (SDP_AVRCP_SUPPORTED_FEATURES_CONTROLLER_CATEGORY_1 | SDP_AVRCP_SUPPORTED_FEATURES_CONTROLLER_CATEGORY_2);
         ControllerRemoteControlRoleInfo.ProviderName           = "Stonestreet One";
         ControllerRemoteControlRoleInfo.ServiceName            = "AVRCP Controller";

         InitializationInfo.RemoteControlInitializationInfo->TargetRoleInfo = &TargetRemoteControlRoleInfo;
         InitializationInfo.RemoteControlInitializationInfo->ControllerRoleInfo = &ControllerRemoteControlRoleInfo;

         /* Everything has been initialized, now attemp to initialize   */
         /* the Audio Manager.                                          */
         ret_val = AUD_Initialize(BluetoothStackID, &InitializationInfo, AUD_Event_Callback, 0);
         if(!ret_val)
         {
            printf("AUD_Initialize(): Success: %s.\r\n", ((EndPointType & ENDPOINT_TYPE_SRC) && (EndPointType & ENDPOINT_TYPE_SNK))?"Source and Sink":(EndPointType & ENDPOINT_TYPE_SRC)?"Source":"Sink");

            /* Flag that there are currently no connected SRC/SNK       */
            /* devices.                                                 */
            ASSIGN_BD_ADDR(CurrentSNK_BD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
            ASSIGN_BD_ADDR(CurrentSRC_BD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

            Initialized = TRUE;
         }
         else
         {
            printf("AUD_Initialize(): Failure: %d\r\n", ret_val);
         }
      }
      else
         ret_val = -1;
   }
   else
      ret_val = -1;

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
   /* Command Options for the Audio profile sub-system.  The input      */
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
   printf("*                  SetPairabilityMode,                           *\r\n");
   printf("*                  ChangeSimplePairingParameters,                *\r\n");
   printf("*                  GetLocalAddress, GetLocalName, SetLocalName,  *\r\n");
   printf("*                  GetClassOfDevice, SetClassOfDevice,           *\r\n");
   printf("*                  GetRemoteName, ServiceDiscovery,              *\r\n");
   printf("* Common Audio:                                                  *\r\n");
   printf("*                  OpenRequestResponse, OpenRemoteStream,        *\r\n");
   printf("*                  CloseStream, OpenRemoteControl,               *\r\n");
   printf("*                  CloseRemoteControl, ChangeStreamState,        *\r\n");
   printf("*                  QueryStreamState, ChangeStreamFormat,         *\r\n");
   printf("*                  QueryStreamFormat, QuerySupportedFormats,     *\r\n");
   printf("*                  QueryStreamConfig, ChangeConnectionMode,      *\r\n");

   if(EndPointType & ENDPOINT_TYPE_SRC)
   {
      printf("* Audio Source:                                                  *\r\n");   
      printf("*                  PlayWAV, RegisterVolumeNotification,          *\r\n");
      printf("*                  SetAbsoluteVolume,                            *\r\n");
   }
   if(EndPointType & ENDPOINT_TYPE_SNK)
   {
      printf("* Audio Sink:                                                    *\r\n");   
      printf("*                  SendPassThroughCommand, ConfigurePlayback,    *\r\n");
      printf("*                  CurrentPlaybackConfig, NotifyAbsoluteVolume,  *\r\n");
   }
   printf("*                                                                *\r\n");
   printf("*                  EnableDebug, Help, Quit.                      *\r\n");

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
      if(!Connection)
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

   /* The following function is responsible for responding to a request */
   /* to connect from a remote Bluetooth Device.  This function returns */
   /* zero on successful execution and a negative value on all errors.  */
static int OpenRequestResponse(ParameterList_t *TempParam)
{
   int       Result;
   int       ret_val;
   BD_ADDR_t NullADDR;

   ASSIGN_BD_ADDR(NullADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* First, check to see if there is an on-going connection request */
      /* operation active.                                              */
      if(!COMPARE_BD_ADDR(CurrentRemoteBD_ADDR, NullADDR))
      {
         /* Make sure that all of the parameters required for this      */
         /* function appear to be at least semi-valid.                  */
         if((TempParam) && (TempParam->NumberofParameters > 1) && (TempParam->Params[0].intParam >= 0) && (TempParam->Params[0].intParam <= 1))
         {
            /* Parameters appear to be valid.                           */

            /* Attempt to submit the response.                          */
            Result = AUD_Open_Request_Response(BluetoothStackID, CurrentRemoteBD_ADDR, TempParam->Params[0].intParam?acrRemoteControl:acrStream, (Boolean_t)TempParam->Params[1].intParam);

            if(!Result)
            {
               /* Function was successful, inform the user.             */
               printf("AUD_Open_Request_Response(%s): Successful.\r\n", TempParam->Params[1].intParam?"TRUE":"FALSE");

               /* Flag that there is no longer a current connection     */
               /* procedure in progress.                                */
               ASSIGN_BD_ADDR(CurrentRemoteBD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

               /* Flag success to the caller.                           */
               ret_val = 0;
            }
            else
            {
               /* Function failed, inform the user.                     */
               printf("AUD_Open_Request_Response: Failure: %d.\r\n", Result);

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* One of more of the parameters is/are invalid.            */
            printf("Usage: OpenRequestResponse [Type (0=Stream, 1=Remote Control)] [Reject/Accept (0/1)].\r\n");
            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* There is not currently an on-going authentication operation,*/
         /* inform the user of this error condition.                    */
         printf("Unable to issue Open Request Response: Connection Request is not currently in progress.\r\n");

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

   /* The following function is responsible for opening a stream        */
   /* endpoint on a remote Bluetooth Device. This function returns zero */
   /* on successful execution and a negative value on all errors.       */
static int OpenRemoteStream(ParameterList_t *TempParam)
{
   int       Result;
   int       ret_val;
   BD_ADDR_t NullADDR;

   ASSIGN_BD_ADDR(NullADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   /* First, check that a valid Bluetooth Stack ID exists.              */
   if(BluetoothStackID)
   {
      /* Now, check that the input parameters appear to be at least     */
      /* semi-valid.                                                    */
      if((TempParam) && (TempParam->NumberofParameters > 1) && (TempParam->Params[0].intParam) && (NumberofValidResponses) && (TempParam->Params[0].intParam <= NumberofValidResponses) && (!COMPARE_BD_ADDR(InquiryResultList[(TempParam->Params[0].intParam - 1)], NullADDR)))
      {
         /* Attempt to open the remote stream.                          */
         Result = AUD_Open_Remote_Stream(BluetoothStackID, InquiryResultList[(TempParam->Params[0].intParam - 1)], (TempParam->Params[1].intParam)?astSRC:astSNK);

         if(!Result)
         {
            /* Function was successful, inform the user.                */
            printf("AUD_Open_Remote_Stream(%s): Successful.\r\n", TempParam->Params[1].intParam?"SRC":"SNK");

            /* Note the currently connected SRC/SNK BD_ADDR.            */
            if(TempParam->Params[1].intParam)
               CurrentSRC_BD_ADDR = InquiryResultList[(TempParam->Params[0].intParam - 1)];
            else
               CurrentSNK_BD_ADDR = InquiryResultList[(TempParam->Params[0].intParam - 1)];

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            /* Function failed, inform the user.                        */
            printf("AUD_Open_Remote_Stream(): Failure: %d.\r\n", Result);

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One of more of the parameters is/are invalid.               */
         printf("Usage: OpenRemoteStream [Inquiry Index] [Local Stream Type (0=SNK, 1=SRC)].\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
      ret_val = INVALID_STACK_ID_ERROR;

   return(ret_val);
}

   /* The following function is responsible for closing an opened       */
   /* stream.  This function returns zero on successful execution and a */
   /* negative value on all errors.                                     */
static int CloseStream(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First, check that a valid Bluetooth Stack ID exists.              */
   if(BluetoothStackID)
   {
      /* Now, check that the input parameters appear to be at least     */
      /* semi-valid.                                                    */
      if((TempParam) && (TempParam->NumberofParameters > 0))
      {
         /* Attempt to close the stream.                                */
         Result = AUD_Close_Stream(BluetoothStackID, (TempParam->Params[0].intParam)?CurrentSRC_BD_ADDR:CurrentSNK_BD_ADDR, (TempParam->Params[0].intParam)?astSRC:astSNK);

         if(!Result)
         {
            /* Function was successful, inform the user.                */
            printf("AUD_Close_Stream(%s): Successful.\r\n", TempParam->Params[0].intParam?"SRC":"SNK");

            /* Since the SRC/SNK is no longer connected, go ahead and   */
            /* flag it as such.                                         */
            if(TempParam->Params[0].intParam)
            {
               ASSIGN_BD_ADDR(CurrentSRC_BD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
            }
            else
            {
               ASSIGN_BD_ADDR(CurrentSNK_BD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
               SinkStreamStarted = FALSE;
            }

            /* Flag success to the caller.                              */
            ret_val    = 0;

            Connection = FALSE;
         }
         else
         {
            /* Function failed, inform the user.                        */
            printf("AUD_Close_Stream(): Failure: %d.\r\n", Result);

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One of more of the parameters is/are invalid.               */
         printf("Usage: CloseStream [Local Stream Type (0=SNK, 1=SRC)].\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
      ret_val = INVALID_STACK_ID_ERROR;

   return(ret_val);
}

   /* The following function is responsible for opening a remote control*/
   /* connection to a remote Bluetooth Device.  This function returns   */
   /* zero on successful execution and a negative value on all errors.  */
static int OpenRemoteControl(ParameterList_t *TempParam)
{
   int       Result;
   int       ret_val;
   BD_ADDR_t NullADDR;

   ASSIGN_BD_ADDR(NullADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   /* First, check that a valid Bluetooth Stack ID exists.              */
   if(BluetoothStackID)
   {
      /* Now, check that the input parameters appear to be at least     */
      /* semi-valid.                                                    */
      if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].intParam) && (NumberofValidResponses) && (TempParam->Params[0].intParam <= NumberofValidResponses) && (!COMPARE_BD_ADDR(InquiryResultList[(TempParam->Params[0].intParam - 1)], NullADDR)))
      {
         /* Attempt to open the remote control connection.              */
         Result = AUD_Open_Remote_Control(BluetoothStackID, InquiryResultList[(TempParam->Params[0].intParam - 1)]);

         if(!Result)
         {
            /* Function was successful, inform the user.                */
            printf("AUD_Open_Remote_Control(): Successful.\r\n");

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            /* Function failed, inform the user.                        */
            printf("AUD_Open_Remote_Control(): Failure: %d.\r\n", Result);

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One of more of the parameters is/are invalid.               */
         printf("Usage: OpenRemoteControl [Inquiry Index].\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
      ret_val = INVALID_STACK_ID_ERROR;

   return(ret_val);
}

   /* The following function is responsible for closing an opened remote*/
   /* control connection.  This function returns zero on successful     */
   /* execution and a negative value on all errors.                     */
static int CloseRemoteControl(ParameterList_t *TempParam)
{
   int       Result;
   int       ret_val;
   BD_ADDR_t NullADDR;

   ASSIGN_BD_ADDR(NullADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   /* First, check that a valid Bluetooth Stack ID exists.              */
   if(BluetoothStackID)
   {
      /* Now, check that the input parameters appear to be at least     */
      /* semi-valid.                                                    */
      if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].intParam) && (NumberofValidResponses) && (TempParam->Params[0].intParam <= NumberofValidResponses) && (!COMPARE_BD_ADDR(InquiryResultList[(TempParam->Params[0].intParam - 1)], NullADDR)))
      {
         /* Attempt to close the remote control connection.             */
         Result = AUD_Close_Remote_Control(BluetoothStackID, InquiryResultList[(TempParam->Params[0].intParam - 1)]);

         if(!Result)
         {
            /* Function was successful, inform the user.                */
            printf("AUD_Close_Remote_Control(): Successful.\r\n");

            /* Flag success to the caller.                              */
            ret_val                 = 0;

            RemoteControlConnection = FALSE;
            VolumeChangedRegistered = FALSE;
         }
         else
         {
            /* Function failed, inform the user.                        */
            printf("AUD_Close_Remote_Control(): Failure: %d.\r\n", Result);

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One of more of the parameters is/are invalid.               */
         printf("Usage: CloseRemoteControl [Inquiry Index].\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
      ret_val = INVALID_STACK_ID_ERROR;

   return(ret_val);
}

   /* The following function is responsible for Loading and Playing a   */
   /* WAV-format audio file over an established connection to an Audio  */
   /* Sink. This function returns zero if successful and a negative     */
   /* value if an error occurred.                                       */
static int PlayWAV(ParameterList_t *TempParam)
{
   int                     Result;
   int                     ret_val;
   int                     FileDescriptor;
   WAVInfo_t               WAVInfo;
   AUD_Stream_State_t      StreamState;
   AUD_Stream_Format_t     StreamFormat;
   PlaybackThreadParams_t *PlaybackThreadParams;

   /* First, check to make sure that we have already been initialized.  */
   if(BluetoothStackID)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 1) && (TempParam->Params[0].strParam))
      {
         /* Before attempting to open the specified WAV file, determine */
         /* if there is an Audio Source Endpoint registered with this   */
         /* client.                                                     */
         if(EndPointType & ENDPOINT_TYPE_SRC)
         {
            errno = 0;

            if((FileDescriptor = open((TempParam->Params[0].strParam), (O_RDONLY))) >= 0)
            {
               if(ParseWAVHeader(FileDescriptor, &WAVInfo) == 0)
               {
                  printf("Parsed WAV File:\r\n");
                  printf("    AudioDataOffset:       %ld\r\n", WAVInfo.AudioDataOffset);
                  printf("    AudioDataLength:       %ld\r\n", WAVInfo.AudioDataLength);
                  printf("    Format:                %hd\r\n", WAVInfo.Format);
                  printf("    Channels:              %hd\r\n", WAVInfo.Channels);
                  printf("    SamplesPerSecond:      %ld\r\n", WAVInfo.SamplesPerSecond);
                  printf("    AverageBytesPerSecond: %ld\r\n", WAVInfo.AverageBytesPerSecond);
                  printf("    BlockSize:             %hd\r\n", WAVInfo.BlockSize);
                  printf("    BitsPerSample:         %hd\r\n", WAVInfo.BitsPerSample);
                  printf("    ValidBitsPerSample:    %hd\r\n", WAVInfo.ValidBitsPerSample);
                  printf("\r\n");

                  if((WAVInfo.Format == WAVE_FORMAT_PCM) && (WAVInfo.BitsPerSample == 16))
                  {
                     /* WAV file appears good, now check the active     */
                     /* outgoing audio connection. If the stream status */
                     /* can be retrieved successfully, the stream must  */
                     /* be connected.                                   */
                     if((Result = AUD_Query_Stream_State(BluetoothStackID, CurrentSRC_BD_ADDR, astSRC, &StreamState)) == 0)
                     {
                        if((Result = AUD_Query_Stream_Format(BluetoothStackID, CurrentSRC_BD_ADDR, astSRC, &StreamFormat)) == 0)
                        {
                           if((StreamFormat.SampleFrequency == WAVInfo.SamplesPerSecond) && (StreamFormat.NumberChannels == WAVInfo.Channels))
                           {
                              BTPS_ResetEvent(SourceStreamStateChangedEvent);

                              Result = AUD_Change_Stream_State(BluetoothStackID, CurrentSRC_BD_ADDR, astSRC, astStreamStarted);

                              if(((Result == 0) && (BTPS_WaitEvent(SourceStreamStateChangedEvent, 10000))) || (Result == BTAUD_ERROR_STREAM_STATE_ALREADY_CURRENT))
                              {
                                 BTPS_ResetEvent(SourceStreamStateChangedEvent);

                                 if((PlaybackThreadParams = (PlaybackThreadParams_t *)malloc(sizeof(PlaybackThreadParams_t))) != NULL)
                                 {
                                    /* Initialize the audio playback    */
                                    /* thread.                          */
                                    PlaybackThreadParams->BluetoothStackID = BluetoothStackID;
                                    PlaybackThreadParams->FileDescriptor   = dup(FileDescriptor);
                                    PlaybackThreadParams->WAVInfo          = WAVInfo;

                                    /* Check to see if an offset was    */
                                    /* specified.                       */
                                    if(TempParam->NumberofParameters > 1)
                                       PlaybackThreadParams->Offset = (TempParam->Params[1].intParam)?FileOffset:0;
                                    else
                                       PlaybackThreadParams->Offset = 0;

                                    /* Check to see if we should loop. */
                                    if(TempParam->NumberofParameters > 2)
                                       PlaybackThreadParams->Loop = (Boolean_t)(TempParam->Params[2].intParam);
                                    else
                                       PlaybackThreadParams->Loop = FALSE;

                                    printf("Starting Playback Thread.\r\n");

                                    if(BTPS_CreateThread(PlaybackThreadMain, 16384, PlaybackThreadParams))
                                    {
                                       /* Flag success to the caller.   */
                                       ret_val = 0;
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
                                 printf("Unable to begin playback: %d.\r\n", Result);

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
                           printf("AUDM_Query_Audio_Stream_Format() Failure: %d.\r\n", Result);

                           ret_val = FUNCTION_ERROR;
                        }
                     }
                     else
                     {
                        printf("Not connected as an Audio Source.\r\n");

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

               /* We're done with out copy of the file descriptor, now, */
               /* so close it.                                          */
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
            printf("Playing audio is only supported in the Source role.\r\n");

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: PlayWAV [WAV File Name] [Restart/Resume - 0/1 (Optional)] [Loop - 1=TRUE (Optional)].\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
      ret_val = INVALID_STACK_ID_ERROR;

   return(ret_val);
}

   /* The following function is responsible for changing the state of an*/
   /* open stream.  This function returns zero on successful execution  */
   /* and a negative value on all errors.                               */
static int ChangeStreamState(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First, check that a valid Bluetooth Stack ID exists.              */
   if(BluetoothStackID)
   {
      /* Now, check that the input parameters appear to be at least     */
      /* semi-valid.                                                    */
      if((TempParam) && (TempParam->NumberofParameters > 1))
      {
         /* Attempt to change the stream state.                         */
         Result = AUD_Change_Stream_State(BluetoothStackID, (TempParam->Params[0].intParam)?CurrentSRC_BD_ADDR:CurrentSNK_BD_ADDR, (TempParam->Params[0].intParam)?astSRC:astSNK, (TempParam->Params[1].intParam)?astStreamStarted:astStreamStopped);

         if(!Result)
         {
            /* Function was successful, inform the user.                */
            printf("AUD_Change_Stream_State(%s): Successful.\r\n", TempParam->Params[1].intParam?"Started":"Stopped");

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            /* Function failed, inform the user.                        */
            printf("AUD_Change_Stream_State(): Failure: %d.\r\n", Result);

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One of more of the parameters is/are invalid.               */
         printf("Usage: ChangeStreamState [Local Stream Type (0=SNK, 1=SRC)] [Stream State (0=Stopped, 1=Started)].\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
      ret_val = INVALID_STACK_ID_ERROR;

   return(ret_val);
}

   /* The following function is responsible for determining and         */
   /* displaying an open stream's current state.  This function returns */
   /* zero on successful execution and a negative value on all errors.  */
static int QueryStreamState(ParameterList_t *TempParam)
{
   int                Result;
   int                ret_val;
   AUD_Stream_State_t StreamState;

   /* First, check that a valid Bluetooth Stack ID exists.              */
   if(BluetoothStackID)
   {
      /* Now, check that the input parameters appear to be at least     */
      /* semi-valid.                                                    */
      if((TempParam) && (TempParam->NumberofParameters > 0))
      {
         /* Attempt to query the stream state.                          */
         Result = AUD_Query_Stream_State(BluetoothStackID, (TempParam->Params[0].intParam)?CurrentSRC_BD_ADDR:CurrentSNK_BD_ADDR, (TempParam->Params[0].intParam)?astSRC:astSNK, &StreamState);

         if(!Result)
         {
            /* Function was successful, inform the user.                */
            printf("AUD_Query_Stream_State(): Successful.\r\n");
            printf("    State: %s.\r\n", (StreamState == astStreamStopped)?"Stopped":"Started");

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            /* Function failed, inform the user.                        */
            printf("AUD_Query_Stream_State(): Failure: %d.\r\n", Result);

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One of more of the parameters is/are invalid.               */
         printf("Usage: QueryStreamState [Local Stream Type (0=SNK, 1=SRC)].\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
      ret_val = INVALID_STACK_ID_ERROR;

   return(ret_val);
}

   /* The following function is responsible for changing the format of a*/
   /* currently open stream.  The Supported Formats Index should be     */
   /* determined by the displayed list from a call to                   */
   /* QuerySupportedFormats.  This function returns zero on successful  */
   /* execution and a negative value on all errors.                     */
static int ChangeStreamFormat(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First, check that a valid Bluetooth Stack ID exists.              */
   if(BluetoothStackID)
   {
      /* Now, check that the input parameters appear to be at least     */
      /* semi-valid.                                                    */
      if((TempParam) && (TempParam->NumberofParameters > 1) && (TempParam->Params[1].intParam) && ((!(TempParam->Params[0].intParam) && (NUM_SNK_SUPPORTED_FORMATS) && (TempParam->Params[1].intParam <= NUM_SNK_SUPPORTED_FORMATS)) || ((TempParam->Params[0].intParam) && (NUM_SRC_SUPPORTED_FORMATS) && (TempParam->Params[1].intParam <= NUM_SRC_SUPPORTED_FORMATS))))
      {
         /* Attempt to change the stream format.                        */
         Result = AUD_Change_Stream_Format(BluetoothStackID, (TempParam->Params[0].intParam)?CurrentSRC_BD_ADDR:CurrentSNK_BD_ADDR, (TempParam->Params[0].intParam)?astSRC:astSNK, (TempParam->Params[0].intParam)?&AudioSRCSupportedFormats[(TempParam->Params[1].intParam - 1)]:&AudioSNKSupportedFormats[(TempParam->Params[1].intParam - 1)]);

         if(!Result)
         {
            /* Function was successful, inform the user.                */
            printf("AUD_Change_Stream_Format(%s): Successful.\r\n", TempParam->Params[0].intParam?"SRC":"SNK");

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            /* Function failed, inform the user.                        */
            printf("AUD_Change_Stream_Format(): Failure: %d.\r\n", Result);

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One of more of the parameters is/are invalid.               */
         printf("Usage: ChangeStreamFormat [Local Stream Type (0=SNK, 1=SRC)] [Supported Formats Index].\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
      ret_val = INVALID_STACK_ID_ERROR;

   return(ret_val);
}

   /* The following function is responsible for determining and         */
   /* displaying the format of a currently opened stream.  This function*/
   /* returns zero on successful execution and a negative value on all  */
   /* errors.                                                           */
static int QueryStreamFormat(ParameterList_t *TempParam)
{
   int                 Result;
   int                 ret_val;
   AUD_Stream_Format_t StreamFormat;

   /* First, check that a valid Bluetooth Stack ID exists.              */
   if(BluetoothStackID)
   {
      /* Now, check that the input parameters appear to be at least     */
      /* semi-valid.                                                    */
      if((TempParam) && (TempParam->NumberofParameters > 0))
      {
         /* Attempt to query the stream format.                         */
         Result = AUD_Query_Stream_Format(BluetoothStackID, (TempParam->Params[0].intParam)?CurrentSRC_BD_ADDR:CurrentSNK_BD_ADDR, (TempParam->Params[0].intParam)?astSRC:astSNK, &StreamFormat);

         if(!Result)
         {
            /* Function was successful, inform the user.                */
            printf("AUD_Query_Stream_Format(%s): Successful.\r\n", TempParam->Params[0].intParam?"SRC":"SNK");
            printf("    Format: %u, %u.\r\n", (unsigned int)StreamFormat.SampleFrequency, (unsigned int)StreamFormat.NumberChannels);

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            /* Function failed, inform the user.                        */
            printf("AUD_Query_Stream_Format(): Failure: %d.\r\n", Result);

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One of more of the parameters is/are invalid.               */
         printf("Usage: QueryStreamFormat [Local Stream Type (0=SNK, 1=SRC)].\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
      ret_val = INVALID_STACK_ID_ERROR;

   return(ret_val);
}

   /* The following function is responsible for displaying the local    */
   /* supported formats to the user.  The indices displayed should be   */
   /* used with the ChangeStreamFormat function.  This function returns */
   /* zero on successful execution and a negative value on all errors.  */
static int QuerySupportedFormats(ParameterList_t *TempParam)
{
   int Index;
   int ret_val;

   /* First, check that a valid Bluetooth Stack ID exists.              */
   if(BluetoothStackID)
   {
      if(EndPointType & ENDPOINT_TYPE_SNK)
      {
         printf("Supported Sink Formats:\r\n");
         for(Index=0;Index<NUM_SNK_SUPPORTED_FORMATS;Index++)
            printf("%d: {%u, %u}\r\n", Index+1, (unsigned int)AudioSNKSupportedFormats[Index].SampleFrequency, (unsigned int)AudioSNKSupportedFormats[Index].NumberChannels);
      }

      if(EndPointType & ENDPOINT_TYPE_SRC)
      {
         printf("\r\nSupported Source Formats:\r\n");
         for(Index=0;Index<NUM_SRC_SUPPORTED_FORMATS;Index++)
            printf("%d: {%u, %u}\r\n", Index+1, (unsigned int)AudioSRCSupportedFormats[Index].SampleFrequency, (unsigned int)AudioSRCSupportedFormats[Index].NumberChannels);
      }

      /* Flag success to the caller.                                    */
      ret_val = 0;
   }
   else
      ret_val = INVALID_STACK_ID_ERROR;

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for determining and         */
   /* displaying the configuration of a currently opened stream.  This  */
   /* function returns zero on successful execution and a negative value*/
   /* on all errors.                                                    */
static int QueryStreamConfiguration(ParameterList_t *TempParam)
{
   int                        Result;
   int                        ret_val;
   int                        Index;
   AUD_Stream_Configuration_t StreamConfiguration;

   /* First, check that a valid Bluetooth Stack ID exists.              */
   if(BluetoothStackID)
   {
      /* Now, check that the input parameters appear to be at least     */
      /* semi-valid.                                                    */
      if((TempParam) && (TempParam->NumberofParameters > 0))
      {
         /* Attempt to query the stream configuration.                  */
         Result = AUD_Query_Stream_Configuration(BluetoothStackID, (TempParam->Params[0].intParam)?CurrentSRC_BD_ADDR:CurrentSNK_BD_ADDR, (TempParam->Params[0].intParam)?astSRC:astSNK, &StreamConfiguration);

         if(!Result)
         {
            /* Function was successful, inform the user.                */
            printf("AUD_Query_Stream_Configuration(%s): Successful.\r\n", TempParam->Params[0].intParam?"SRC":"SNK");
            printf("    Format:              %u, %u.\r\n", (unsigned int)StreamConfiguration.StreamFormat.SampleFrequency, (unsigned int)StreamConfiguration.StreamFormat.NumberChannels);
            printf("    MTU:                 %d.\n", StreamConfiguration.MediaMTU);
            printf("    Codec Type (Length): %d (%d).\r\n", StreamConfiguration.MediaCodecType, StreamConfiguration.MediaCodecInfoLength);
            printf("    Codec Info:          0x");

            for(Index=0;Index<StreamConfiguration.MediaCodecInfoLength;Index++)
               printf("%02X", (unsigned int)(StreamConfiguration.MediaCodecInformation[Index] & 0xFF));

            printf(".\r\n");

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            /* Function failed, inform the user.                        */
            printf("AUD_Query_Stream_Configuration(): Failure: %d.\r\n", Result);

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One of more of the parameters is/are invalid.               */
         printf("Usage: QueryStreamConfig [Local Stream Type (0=SNK, 1=SRC)].\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
      ret_val = INVALID_STACK_ID_ERROR;

   return(ret_val);
}

   /* The following function is responsible for changing how incoming   */
   /* connections are handled.  This function returns zero on successful*/
   /* execution and a negative value on all errors.                     */
static int ChangeConnectionMode(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First, check that a valid Bluetooth Stack ID exists.              */
   if(BluetoothStackID)
   {
      /* Now, check that the input parameters appear to be at least     */
      /* semi-valid.                                                    */
      if((TempParam) && (TempParam->NumberofParameters > 0))
      {
         /* Attempt to set the connection mode.                         */
         Result = AUD_Set_Server_Connection_Mode(BluetoothStackID, (!(TempParam->Params[0].intParam))?ausAutomaticAccept:(TempParam->Params[0].intParam == 1)?ausAutomaticReject:ausManualAccept);

         if(!Result)
         {
            /* Function was successful, inform the user.                */
            printf("AUD_Set_Server_Connection_Mode(): Successful.\r\n");

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            /* Function failed, inform the user.                        */
            printf("AUD_Set_Server_Connection_Mode(): Failure: %d.\r\n", Result);

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One of more of the parameters is/are invalid.               */
         printf("Usage: ChangeConnectionMode [Connection Mode (0=Automatic Accept, 1=Automatic Reject, 2=Manual Accept)].\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
      ret_val = INVALID_STACK_ID_ERROR;

   return(ret_val);
}

   /* The following function is responsible for determing and displaying*/
   /* how incoming connections are handled.  This function returns zero */
   /* on successful execution and a negative value on all errors.       */
static int QueryConnectionMode(ParameterList_t *TempParam)
{
   int                          Result;
   int                          ret_val;
   AUD_Server_Connection_Mode_t ServerConnectionMode;

   /* First, check that a valid Bluetooth Stack ID exists.              */
   if(BluetoothStackID)
   {
      /* Attempt to get the connection mode.                            */
      Result = AUD_Get_Server_Connection_Mode(BluetoothStackID, &ServerConnectionMode);

      if(!Result)
      {
         /* Function was successful, inform the user.                   */
         printf("AUD_Get_Server_Connection_Mode(): Successful.\r\n");
         printf("    Connection Mode: %s.\r\n", (ServerConnectionMode == ausAutomaticAccept)?"Automatic Accept":(ServerConnectionMode == ausAutomaticReject)?"Automatic Reject":"Manual Accept");

         /* Flag success to the caller.                                 */
         ret_val = 0;
      }
      else
      {
         /* Function failed, inform the user.                           */
         printf("AUD_Get_Server_Connection_Mode(): Failure: %d.\r\n", Result);

         ret_val = FUNCTION_ERROR;
      }
   }
   else
      ret_val = INVALID_STACK_ID_ERROR;

   return(ret_val);
}

   /* The following function is responsible for sending Remote Control  */
   /* Pass Through Commands.  This function returns zero on successful  */
   /* execution and a negative value on all errors.                     */
static int SendPassThroughCommand(ParameterList_t *TempParam)
{
   int                               ret_val;
   int                               Result;
   unsigned int                      Index;
   Byte_t                            OperationID;
   AUD_Remote_Control_Command_Data_t RemoteControlCommandData;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now, check that the input parameters appear to be at least     */
      /* semi-valid.                                                    */
      if((TempParam) && (TempParam->NumberofParameters >= 2) && (TempParam->Params[1].intParam <= NUMBER_PASSTHROUGH_COMMANDS))
      {
         /* Now check to make sure that a Remote Control session is     */
         /* connected.                                                  */
         if(RemoteControlConnection)
         {
            /* Now let's parse the data that was specified.             */
            Result = 0;

            if(TempParam->Params[1].intParam < NUMBER_PASSTHROUGH_COMMANDS)
            {
               OperationID = PassThroughCommandTable[TempParam->Params[1].intParam].IntKey;
               printf("%s Pass-through Command specified.\r\n", PassThroughCommandTable[TempParam->Params[1].intParam].StrVal);
            }
            else
            {
               /* Specify Command.                                      */
               if((TempParam->NumberofParameters >= 3) && (TempParam->Params[2].intParam) && (TempParam->Params[2].intParam != 0xFF))
               {
                  OperationID = TempParam->Params[2].intParam;

                  printf("Specific Pass-through Command specified: 0x%02X.\r\n", OperationID);
               }
               else
                  Result = INVALID_PARAMETERS_ERROR;
            }

            if(!Result)
            {
               /* Format up a Pass-through Command. StateFlag is FALSE  */
               /* for "Button Down".                                    */
               RemoteControlCommandData.MessageType                                            = amtPassThrough;
               RemoteControlCommandData.MessageData.PassThroughCommandData.CommandType         = AVRCP_CTYPE_CONTROL;
               RemoteControlCommandData.MessageData.PassThroughCommandData.SubunitType         = AVRCP_SUBUNIT_TYPE_PANEL;
               RemoteControlCommandData.MessageData.PassThroughCommandData.SubunitID           = AVRCP_SUBUNIT_ID_INSTANCE_0;
               RemoteControlCommandData.MessageData.PassThroughCommandData.OperationID         = OperationID;
               RemoteControlCommandData.MessageData.PassThroughCommandData.StateFlag           = FALSE;
               RemoteControlCommandData.MessageData.PassThroughCommandData.OperationDataLength = 0;
               RemoteControlCommandData.MessageData.PassThroughCommandData.OperationData       = NULL;

               /* Try to Send the Message.                              */
               if((Result = AUD_Send_Remote_Control_Command(BluetoothStackID, CurrentRC_BD_ADDR, &RemoteControlCommandData, TempParam->Params[0].intParam)) <= 0)
               {
                  /* There was an error attempting to Send the Message. */
                  printf("AUD_Send_Remote_Control_Command: Function Failure: %d.\r\n", Result);

                  ret_val = Result;
               }
               else
               {
                  /* The Send Message Request was successfully          */
                  /* submitted.                                         */
                  printf("AUD_Send_Remote_Control_Command Function Successful: Button %s, Transaction ID = %d\r\n", ((RemoteControlCommandData.MessageData.PassThroughCommandData.StateFlag == FALSE) ? "DOWN" : "UP"), Result);

                  /* Follow up with the associated "Button Up" command. */
                  RemoteControlCommandData.MessageData.PassThroughCommandData.StateFlag = TRUE;

                  /* Try to Send the Message.                           */
                  if((Result = AUD_Send_Remote_Control_Command(BluetoothStackID, CurrentRC_BD_ADDR, &RemoteControlCommandData, TempParam->Params[0].intParam)) <= 0)
                  {
                     /* There was an error attempting to Send the       */
                     /* Message.                                        */
                     printf("AUD_Send_Remote_Control_Command: Function Failure: %d.\r\n", Result);

                     ret_val = Result;
                  }
                  else
                  {
                     /* The Send Message Request was successfully       */
                     /* submitted.                                      */
                     printf("AUD_Send_Remote_Control_Command Function Successful: Button %s, Transaction ID = %d\r\n", ((RemoteControlCommandData.MessageData.PassThroughCommandData.StateFlag == FALSE) ? "DOWN" : "UP"), Result);

                     ret_val = 0;
                  }
               }
            }
            else
            {
               printf("Usage: SendPassThroughCommand [Timeout (ms)] [Passthrough Command] [Explicit ID (When Explicit Command ID is selected)].\r\n");
               printf("   Possible Commands:\r\n");

               for(Index = 0; Index < NUMBER_PASSTHROUGH_COMMANDS; Index++)
                  printf("      %2u - %s.\r\n", Index, PassThroughCommandTable[Index].StrVal);

               printf("      %2u - Explicit Command ID.\r\n", (unsigned int)NUMBER_PASSTHROUGH_COMMANDS);

               ret_val = Result;
            }
         }
         else
         {
            /* Profile is either not Registered or not connected.       */
            printf("No Profile Registered or Profile not connected.\r\n");

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         printf("Usage: SendPassThroughCommand [Timeout (ms)] [Passthrough Command] [Explicit ID (When Explicit Command ID is selected)].\r\n");
         printf("   Possible Commands:\r\n");

         for(Index = 0; Index < NUMBER_PASSTHROUGH_COMMANDS; Index++)
            printf("      %2u - %s.\r\n", Index, PassThroughCommandTable[Index].StrVal);

         printf("      %2u - Explicit Command ID.\r\n", (unsigned int)NUMBER_PASSTHROUGH_COMMANDS);


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

   /* This function is used to configure how incoming audio data is     */
   /* procsesed as a Sink. It returns zero on success or a negative     */
   /* error code if there was an error.                                 */
static int ConfigurePlayback(ParameterList_t *TempParam)
{
   int   ret_val;
   char *NamePtr;

   /* First, check to make sure that we have already been initialized.  */
   if(BluetoothStackID)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 1))
      {
         if(!SinkStreamStarted)
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
         printf("Usage: ConfigurePlayback [Config Flags] [WAV File Name (Optional if WAV output enabled. \"audio.wav\" by default)]\r\n");
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
      /* No valid Bluetooth Stack ID exists.                            */
      printf("Stack ID Invalid.\r\n");

      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* This function prints the current playback configuration.          */
static int CurrentPlaybackConfig(ParameterList_t *TempParam)
{
   int ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(BluetoothStackID)
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
      /* No valid Bluetooth Stack ID exists.                            */
      printf("Stack ID Invalid.\r\n");

      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* This function is used to send an Absolute Volume notification to a*/
   /* remote Source. It returns zero if successful or a negative error  */
   /* code if there was an error.                                       */
static int NotifyAbsoluteVolume(ParameterList_t *TempParam)
{
   int                                ret_val;
   AUD_Remote_Control_Response_Data_t ResponseData;

   /* First, check to make sure that we have already been initialized.  */
   if(BluetoothStackID)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 2) && (TempParam->Params[1].intParam <= 100))
      {
         /* Make sure the remote side has registered for this event.    */
         if(VolumeChangedRegistered)
         {
            /* Format the notification event.                           */
            ResponseData.MessageType                                                                                    = amtRegisterNotification;
            ResponseData.MessageData.RegisterNotificationResponseData.ResponseCode                                      = AVRCP_RESPONSE_CHANGED;
            ResponseData.MessageData.RegisterNotificationResponseData.EventID                                           = AVRCP_EVENT_VOLUME_CHANGED;
            ResponseData.MessageData.RegisterNotificationResponseData.NotificationData.VolumeChangedData.AbsoluteVolume = (Byte_t)ABSOLUTE_VOLUME_FROM_PERCENTAGE(TempParam->Params[1].intParam);

            /* Now simply send the notification response.               */
            ret_val = AUD_Send_Remote_Control_Response(BluetoothStackID, CurrentRC_BD_ADDR, TempParam->Params[0].intParam, &ResponseData);

            if(!ret_val)
            {
               printf("AUD_Send_Remote_Control_Response() Successful.\r\n");

               CurrentAbsVolume        = ResponseData.MessageData.RegisterNotificationResponseData.NotificationData.VolumeChangedData.AbsoluteVolume;
               VolumeChangedRegistered = FALSE;
            }
            else
            {
               printf("AUD_Send_Remote_Control_Response() Error: %d.r\n", ret_val);

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            printf("Remote side has not registered for volume changed notifications.\r\n");

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: NotifyAbsoluteVolume [TransactionID] [Volume (0-100)]\r\n");

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

   /* The function is used to register for volume changed notifications */
   /* from a remote Sink.                                               */
static int RegisterVolumeNotification(ParameterList_t *TempParam)
{
   int                               ret_val;
   AUD_Remote_Control_Command_Data_t CommandData;

   /* First, check to make sure that we have already been initialized.  */
   if(BluetoothStackID)
   {
      /* Now make sure a Remote Control is connected.                   */
      if(RemoteControlConnection)
      {
         CommandData.MessageType                                         = amtRegisterNotification;
         CommandData.MessageData.RegisterNotificationCommandData.EventID = AVRCP_EVENT_VOLUME_CHANGED;

         /* Now send the Register Notification command.                 */
         ret_val = AUD_Send_Remote_Control_Command(BluetoothStackID, CurrentRC_BD_ADDR, &CommandData, 500); 

         if(ret_val > 0)
         {
            printf("AUD_Send_Remote_Control_Command() Successful. TransactionID: %d.\r\n", ret_val);

            ret_val = 0;
         }
         else
         {
            printf("AUD_Send_Remote_Control_Command() Error: %d.\r\n", ret_val);

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         printf("Remote control not connected.\r\n");

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

   /* This function is used to set the absolute volume on a remote Sink.*/
static int SetAbsoluteVolume(ParameterList_t *TempParam)
{
   int                               ret_val;
   AUD_Remote_Control_Command_Data_t CommandData;

   /* First, check to make sure that we have already been initialized.  */
   if(BluetoothStackID)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 2) && (TempParam->Params[0].intParam <= 100))
      {
         /* Make sure the remote side has registered for this event.    */
         if(RemoteControlConnection)
         {
            /* Format the Command.                                      */
            CommandData.MessageType                                             = amtSetAbsoluteVolume;
            CommandData.MessageData.SetAbsoluteVolumeCommandData.AbsoluteVolume = ABSOLUTE_VOLUME_FROM_PERCENTAGE(TempParam->Params[0].intParam);

            /* Now simply send the command.                             */
            ret_val = AUD_Send_Remote_Control_Command(BluetoothStackID, CurrentRC_BD_ADDR, &CommandData, TempParam->Params[1].intParam);

            if(ret_val >= 0)
            {
               printf("AUD_Send_Remote_Control_Command() Successful: %d.\r\n", ret_val);

               ret_val = 0;
            }
            else
            {
               printf("AUD_Send_Remote_Control_Command() Error: %d.r\n", ret_val);

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            printf("Remote Control not connected.\r\n");

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: SetAbsoluteVolume [Volume (0-100)] [Timeout (ms)]\r\n");

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
                              printf("Invalid combination of Format Chunk Size and Format Extension Size (%ld, %hd)\r\n", FmtChunkSize, FmtExtensionSize);

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
                  printf("Format header specifies invalid Chunk Size (%ld)\r\n", FmtChunkSize);

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
               printf("WAV file too short: Header & Data requires %lu bytes, but file is only %lu bytes long\r\n", (Info->AudioDataOffset + Info->AudioDataLength), FileStat.st_size);

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
   AUD_Stream_State_t      StreamState;
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
         if(InitializeAudioEncoder(Params->BluetoothStackID, CurrentSRC_BD_ADDR, Params->WAVInfo.SamplesPerSecond, Params->WAVInfo.Channels) == 0)
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
               if(BTPS_WaitEvent(SourceStreamStateChangedEvent, 0))
               {
                  BTPS_ResetEvent(SourceStreamStateChangedEvent);

                  Result = AUD_Query_Stream_State(Params->BluetoothStackID, CurrentSRC_BD_ADDR, astSRC, &StreamState);

                  if(((!Result) && (StreamState == astStreamStopped)) || (Result == BTAUD_ERROR_STREAM_NOT_CONNECTED) || (Result == BTAUD_ERROR_STREAM_NOT_INITIALIZED))
                     break;
                  else
                     Result = 0;
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
                  Result = SendAudioData(CurrentSRC_BD_ADDR, Buffer, BufferRemaining);

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

            CleanupAudioDecoder();

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

   printf("Playback Thread done.\r\n");

   printf("AUDM>");

   fflush(stdout);

   return(NULL);
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

   /* This function is a utility to process incoming Remote Control     */
   /* Commands. It parses commands known to this application, prints    */
   /* relavent data, and responds accordingly.                          */
static void ProcessRemoteControlCommandIndication(AUD_Remote_Control_Command_Indication_Data_t *CommandData)
{
   AVRCP_Capability_Info_t Capability;
   AUD_Remote_Control_Response_Data_t ResponseData;

   switch(CommandData->RemoteControlCommandData.MessageType)
   {
      case amtPassThrough:
         printf("\r\nPass Through Command.\r\n");
         printf("   Operation ID: %s (%u).\r\n", IntKeyToStr(NUMBER_PASSTHROUGH_COMMANDS, PassThroughCommandTable, CommandData->RemoteControlCommandData.MessageData.PassThroughCommandData.OperationID), CommandData->RemoteControlCommandData.MessageData.PassThroughCommandData.OperationID);
         printf("   Pressed:      %s.\r\n", CommandData->RemoteControlCommandData.MessageData.PassThroughCommandData.StateFlag?"RELEASED":"PRESSED");

         ResponseData.MessageType                                             = amtPassThrough;
         ResponseData.MessageData.PassThroughResponseData.ResponseCode        = AVRCP_RESPONSE_ACCEPTED;
         ResponseData.MessageData.PassThroughResponseData.SubunitType         = CommandData->RemoteControlCommandData.MessageData.PassThroughCommandData.SubunitType;
         ResponseData.MessageData.PassThroughResponseData.SubunitID           = CommandData->RemoteControlCommandData.MessageData.PassThroughCommandData.SubunitID;
         ResponseData.MessageData.PassThroughResponseData.OperationID         = CommandData->RemoteControlCommandData.MessageData.PassThroughCommandData.OperationID;
         ResponseData.MessageData.PassThroughResponseData.StateFlag           = CommandData->RemoteControlCommandData.MessageData.PassThroughCommandData.StateFlag;
         ResponseData.MessageData.PassThroughResponseData.OperationDataLength = 0;
         ResponseData.MessageData.PassThroughResponseData.OperationData       = NULL;

         break;
      case amtGetCapabilities:
         printf("\r\nGet Capabilities Command.\r\n");
         if(CommandData->RemoteControlCommandData.MessageData.GetCapabilitiesCommandData.CapabilityID == AVRCP_GET_CAPABILITIES_CAPABILITY_ID_EVENTS_SUPPORTED)
         {
            printf("   Responding with supported events.\r\n");

            Capability.CapabilityInfo.EventID                                       = AVRCP_EVENT_VOLUME_CHANGED;

            ResponseData.MessageType                                                = amtGetCapabilities;
            ResponseData.MessageData.GetCapabilitiesResponseData.ResponseCode       = AVRCP_RESPONSE_STABLE;
            ResponseData.MessageData.GetCapabilitiesResponseData.CapabilityID       = AVRCP_GET_CAPABILITIES_CAPABILITY_ID_EVENTS_SUPPORTED;
            ResponseData.MessageData.GetCapabilitiesResponseData.NumberCapabilities = 1;
            ResponseData.MessageData.GetCapabilitiesResponseData.CapabilityInfoList = &Capability;

         }
         else
         {
            printf("   Company ID Request not supported.\r\n");

            ResponseData.MessageType                                        = amtCommandRejectResponse;
            ResponseData.MessageData.CommandRejectResponseData.ResponseCode = AVRCP_RESPONSE_NOT_IMPLEMENTED;
            ResponseData.MessageData.CommandRejectResponseData.MessageType  = CommandData->RemoteControlCommandData.MessageType;
            ResponseData.MessageData.CommandRejectResponseData.PDU_ID       = 0;
            ResponseData.MessageData.CommandRejectResponseData.ErrorCode    = AVRCP_COMMAND_ERROR_STATUS_CODE_INVALID_COMMAND;
         }
         break;
      case amtRegisterNotification:
         printf("\r\nRegister Notification Command.\r\n");

         if(CommandData->RemoteControlCommandData.MessageData.RegisterNotificationCommandData.EventID == AVRCP_EVENT_VOLUME_CHANGED)
         {
            printf("   Volume Change Event Registered.\r\n");

            ResponseData.MessageType                                                                                    = amtRegisterNotification;
            ResponseData.MessageData.RegisterNotificationResponseData.ResponseCode                                      = AVRCP_RESPONSE_INTERIM;
            ResponseData.MessageData.RegisterNotificationResponseData.EventID                                           = AVRCP_EVENT_VOLUME_CHANGED;
            ResponseData.MessageData.RegisterNotificationResponseData.NotificationData.VolumeChangedData.AbsoluteVolume = CurrentAbsVolume;

            /* Note that the notification is registered.                */
            VolumeChangedRegistered = TRUE;
         }
         else
         {
            printf("   Event Type %u not supported.\r\n", CommandData->RemoteControlCommandData.MessageData.RegisterNotificationCommandData.EventID);

            ResponseData.MessageType                                        = amtCommandRejectResponse;
            ResponseData.MessageData.CommandRejectResponseData.ResponseCode = AVRCP_RESPONSE_NOT_IMPLEMENTED;
            ResponseData.MessageData.CommandRejectResponseData.MessageType  = CommandData->RemoteControlCommandData.MessageType;
            ResponseData.MessageData.CommandRejectResponseData.PDU_ID       = 0;
            ResponseData.MessageData.CommandRejectResponseData.ErrorCode    = AVRCP_COMMAND_ERROR_STATUS_CODE_INVALID_COMMAND;
         }
         break;
      case amtSetAbsoluteVolume:
         printf("\r\nSet Absolute Volume.\r\n");
         printf("   Volume: %u%% (0x%02X).\r\n", ABSOLUTE_VOLUME_TO_PERCENTAGE(CommandData->RemoteControlCommandData.MessageData.SetAbsoluteVolumeCommandData.AbsoluteVolume), CommandData->RemoteControlCommandData.MessageData.SetAbsoluteVolumeCommandData.AbsoluteVolume);

         CurrentAbsVolume                                                      = CommandData->RemoteControlCommandData.MessageData.SetAbsoluteVolumeCommandData.AbsoluteVolume;

         ResponseData.MessageType                                              = amtSetAbsoluteVolume;
         ResponseData.MessageData.SetAbsoluteVolumeResponseData.ResponseCode   = AVRCP_RESPONSE_ACCEPTED;
         ResponseData.MessageData.SetAbsoluteVolumeResponseData.AbsoluteVolume = CurrentAbsVolume;
         break;
      default:
         /* For now we only care about pass through.           */
         printf("\r\nUnknown Message Type.\r\n");

         ResponseData.MessageType                                        = amtCommandRejectResponse;
         ResponseData.MessageData.CommandRejectResponseData.ResponseCode = AVRCP_RESPONSE_NOT_IMPLEMENTED;
         ResponseData.MessageData.CommandRejectResponseData.MessageType  = CommandData->RemoteControlCommandData.MessageType;
         ResponseData.MessageData.CommandRejectResponseData.PDU_ID       = 0;
         ResponseData.MessageData.CommandRejectResponseData.ErrorCode    = AVRCP_COMMAND_ERROR_STATUS_CODE_INVALID_COMMAND;
   }

   AUD_Send_Remote_Control_Response(BluetoothStackID, CommandData->BD_ADDR, CommandData->TransactionID, &ResponseData);
}

   /* This function is a utility to process Remote Control Responses. It*/
   /* handles responses to commands known to this application and prints*/
   /* any relevant data.                                                */
static void ProcessRemoteControlCommandConfirmation(AUD_Remote_Control_Command_Confirmation_Data_t *ResponseData)
{
   if(ResponseData)
   {
      printf("\r\n");

      switch(ResponseData->RemoteControlResponseData.MessageType)
      {
         case amtPassThrough:
            printf("Passthrough Response.\r\n");
            printf("   Response Code: %s (%u).\r\n", IntKeyToStr(NUMBER_AVR_RESPONSE_CODES, AVRResponseCodeTable, ResponseData->RemoteControlResponseData.MessageData.PassThroughResponseData.ResponseCode), ResponseData->RemoteControlResponseData.MessageData.PassThroughResponseData.ResponseCode);
            break;
         case amtRegisterNotification:
            printf("Register Notification Response.\r\n");
            printf("   Response Code: %s (%u).\r\n", IntKeyToStr(NUMBER_AVR_RESPONSE_CODES, AVRResponseCodeTable, ResponseData->RemoteControlResponseData.MessageData.RegisterNotificationResponseData.ResponseCode), ResponseData->RemoteControlResponseData.MessageData.RegisterNotificationResponseData.ResponseCode);
            printf("   EventID:       %s (%u).\r\n", IntKeyToStr(NUMBER_AVR_EVENT_IDS, AVREventIDTable, ResponseData->RemoteControlResponseData.MessageData.RegisterNotificationResponseData.EventID), ResponseData->RemoteControlResponseData.MessageData.RegisterNotificationResponseData.EventID);

            if(ResponseData->RemoteControlResponseData.MessageData.RegisterNotificationResponseData.EventID == AVRCP_EVENT_VOLUME_CHANGED)
            {
               printf("   Volume:        %u%% (0x%02X).\r\n", ABSOLUTE_VOLUME_TO_PERCENTAGE(ResponseData->RemoteControlResponseData.MessageData.RegisterNotificationResponseData.NotificationData.VolumeChangedData.AbsoluteVolume), ResponseData->RemoteControlResponseData.MessageData.RegisterNotificationResponseData.NotificationData.VolumeChangedData.AbsoluteVolume);
            }
            break;
         case amtSetAbsoluteVolume:
            printf("Set Absolute Volume Resposne.\r\n");
            printf("   Response Code: %s (%u).\r\n", IntKeyToStr(NUMBER_AVR_RESPONSE_CODES, AVRResponseCodeTable, ResponseData->RemoteControlResponseData.MessageData.SetAbsoluteVolumeResponseData.ResponseCode), ResponseData->RemoteControlResponseData.MessageData.SetAbsoluteVolumeResponseData.ResponseCode);
            printf("   Volume Set:    %u%% (0x%02X).\r\n", ABSOLUTE_VOLUME_TO_PERCENTAGE(ResponseData->RemoteControlResponseData.MessageData.SetAbsoluteVolumeResponseData.AbsoluteVolume), ResponseData->RemoteControlResponseData.MessageData.SetAbsoluteVolumeResponseData.AbsoluteVolume); 
            break;
         default:
            printf("Unknown Message Type.\r\n");
      }
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

      printf("\r\nAUD>");
   }
   else
   {
      /* There was an error with one or more of the input parameters.   */
      printf("\r\n");

      printf("GAP Callback Data: Event_Data = NULL.\r\n");

      printf("\r\nAUD>");
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

   printf("\r\nAUD>");

   /* Make sure the output is displayed to the user.                    */
   fflush(stdout);
}

   /* The following function is for the AUD Event Receive Data Callback.*/
   /* This function will be called whenever a Callback has been         */
   /* registered for the specified AUD Action that is associated with   */
   /* the Bluetooth Stack.  This function passes to the caller the AUD  */
   /* Event Data of the specified Event and the AUD Event Callback      */
   /* Parameter that was specified when this Callback was installed.    */
   /* The caller is free to use the contents of the AUD Event Data ONLY */
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
   /* other AUD Events will not be processed while this function call is*/
   /* outstanding).                                                     */
   /* * NOTE * This function MUST NOT Block and wait for events that    */
   /*          can only be satisfied by Receiving other AUD Events.  A  */
   /*          Deadlock WILL occur because NO AUD Event Callbacks will  */
   /*          be issued while this function is currently outstanding.  */
static void BTPSAPI AUD_Event_Callback(unsigned int BluetoothStackID, AUD_Event_Data_t *AUD_Event_Data, unsigned long CallbackParameter)
{
   char       BoardStr[13];
   Boolean_t  DisplayPrompt = TRUE;

   if((BluetoothStackID) && (AUD_Event_Data))
   {
      if(AUD_Event_Data->Event_Data_Type != etAUD_Encoded_Audio_Data_Indication)
         printf("\r\n");

      switch(AUD_Event_Data->Event_Data_Type)
      {
         case etAUD_Open_Request_Indication:
            printf("AUD Open Request Indication, Type: %s\r\n", (AUD_Event_Data->Event_Data.AUD_Open_Request_Indication_Data->ConnectionRequestType == acrStream)?"Audio Stream":"Remote Control");

            BD_ADDRToStr(AUD_Event_Data->Event_Data.AUD_Open_Request_Indication_Data->BD_ADDR, BoardStr);
            printf("BD_ADDR: %s\r\n", BoardStr);

            printf("\r\nRespond with the command: OpenRequestResponse\r\n");

            /* Note the current Remote BD_ADDR that is requesting the   */
            /* connection.                                              */
            CurrentRemoteBD_ADDR = AUD_Event_Data->Event_Data.AUD_Open_Request_Indication_Data->BD_ADDR;
            break;
         case etAUD_Stream_Open_Indication:
            /* A local stream endpoint has been opened, display the     */
            /* information.                                             */
            printf("AUD Stream Open Indication, Type: %s.\r\n", (AUD_Event_Data->Event_Data.AUD_Stream_Open_Indication_Data->StreamType == astSNK)?"Sink":"Source");

            BD_ADDRToStr(AUD_Event_Data->Event_Data.AUD_Stream_Open_Indication_Data->BD_ADDR, BoardStr);
            printf("BD_ADDR:  %s.\r\n", BoardStr);
            printf("MediaMTU: %u.\r\n", AUD_Event_Data->Event_Data.AUD_Stream_Open_Indication_Data->MediaMTU);
            printf("Format:   %u, %u, 0x%08X.\r\n", (unsigned int)AUD_Event_Data->Event_Data.AUD_Stream_Open_Indication_Data->StreamFormat.SampleFrequency, (unsigned int)AUD_Event_Data->Event_Data.AUD_Stream_Open_Indication_Data->StreamFormat.NumberChannels, (unsigned int)AUD_Event_Data->Event_Data.AUD_Stream_Open_Indication_Data->StreamFormat.FormatFlags);

            /* Note the currently connected SRC/SNK BD_ADDR.            */
            if(AUD_Event_Data->Event_Data.AUD_Stream_Open_Indication_Data->StreamType == astSNK)
               CurrentSNK_BD_ADDR = AUD_Event_Data->Event_Data.AUD_Stream_Open_Indication_Data->BD_ADDR;
            else
               CurrentSRC_BD_ADDR = AUD_Event_Data->Event_Data.AUD_Stream_Open_Indication_Data->BD_ADDR;

            Connection = TRUE;
            break;
         case etAUD_Stream_Open_Confirmation:
            /* An OpenRemote request has been completed, display the    */
            /* information.                                             */
            printf("AUD Stream Open Confirmation, Type: %s.\r\n", (AUD_Event_Data->Event_Data.AUD_Stream_Open_Confirmation_Data->StreamType == astSNK)?"Sink":"Source");

            printf("Status:   %u.\r\n", AUD_Event_Data->Event_Data.AUD_Stream_Open_Confirmation_Data->OpenStatus);
            printf("MediaMTU: %u.\r\n", AUD_Event_Data->Event_Data.AUD_Stream_Open_Confirmation_Data->MediaMTU);
            printf("Format:   %u, %u, 0x%08X.\r\n", (unsigned int)AUD_Event_Data->Event_Data.AUD_Stream_Open_Confirmation_Data->StreamFormat.SampleFrequency, (unsigned int)AUD_Event_Data->Event_Data.AUD_Stream_Open_Confirmation_Data->StreamFormat.NumberChannels, (unsigned int)AUD_Event_Data->Event_Data.AUD_Stream_Open_Confirmation_Data->StreamFormat.FormatFlags);

            if(!AUD_Event_Data->Event_Data.AUD_Stream_Open_Confirmation_Data->OpenStatus)
               Connection = TRUE;
            else
            {
               /* Error, make sure we clear the currently connected     */
               /* SRC/SNK BD_ADDR.                                      */
               if(AUD_Event_Data->Event_Data.AUD_Stream_Open_Confirmation_Data->StreamType == astSNK)
               {
                  ASSIGN_BD_ADDR(CurrentSNK_BD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
               }
               else
               {
                  ASSIGN_BD_ADDR(CurrentSRC_BD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
               }
            }
            break;
         case etAUD_Stream_Close_Indication:
            /* A local stream endpoint has been closed, display the     */
            /* information.                                             */
            printf("AUD Stream Close Indication, Type: %s.\r\n", (AUD_Event_Data->Event_Data.AUD_Stream_Close_Indication_Data->StreamType == astSNK)?"Sink":"Source");

            BD_ADDRToStr(AUD_Event_Data->Event_Data.AUD_Stream_Close_Indication_Data->BD_ADDR, BoardStr);
            printf("BD_ADDR: %s.\r\n", BoardStr);
            printf("Reason:  %s.\r\n", (AUD_Event_Data->Event_Data.AUD_Stream_Close_Indication_Data->DisconnectReason == adrRemoteDeviceDisconnect)?"Disconnect":(AUD_Event_Data->Event_Data.AUD_Stream_Close_Indication_Data->DisconnectReason == adrRemoteDeviceLinkLoss)?"Link Loss":"Timeout");

            Connection = FALSE;

            /* Stream is closed.  Make sure we clear any existing       */
            /* SRC/SNK BD_ADDR that was stored.                         */
            if(AUD_Event_Data->Event_Data.AUD_Stream_Close_Indication_Data->StreamType == astSNK)
            {
               ASSIGN_BD_ADDR(CurrentSNK_BD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
               SinkStreamStarted = FALSE;
            }
            else
            {
               ASSIGN_BD_ADDR(CurrentSRC_BD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
            }

            if(AUD_Event_Data->Event_Data.AUD_Stream_Close_Indication_Data->StreamType == astSRC)
               BTPS_SetEvent(SourceStreamStateChangedEvent);
            break;
         case etAUD_Remote_Control_Open_Indication:
            BD_ADDRToStr(AUD_Event_Data->Event_Data.AUD_Remote_Control_Open_Indication_Data->BD_ADDR, BoardStr);

            printf("Open Remote Control Indication: %s.\r\n", BoardStr);

            RemoteControlConnection = TRUE;
            CurrentRC_BD_ADDR       = AUD_Event_Data->Event_Data.AUD_Remote_Control_Open_Indication_Data->BD_ADDR;
            break;
         case etAUD_Remote_Control_Open_Confirmation:
            BD_ADDRToStr(AUD_Event_Data->Event_Data.AUD_Remote_Control_Open_Confirmation_Data->BD_ADDR, BoardStr);

            printf("Open Remote Control Confirmation: %s, %d.\r\n", BoardStr, AUD_Event_Data->Event_Data.AUD_Remote_Control_Open_Confirmation_Data->OpenStatus);

            if(!AUD_Event_Data->Event_Data.AUD_Remote_Control_Open_Confirmation_Data->OpenStatus)
            {
               RemoteControlConnection = TRUE;
               CurrentRC_BD_ADDR       = AUD_Event_Data->Event_Data.AUD_Remote_Control_Open_Confirmation_Data->BD_ADDR;
            }
            else
               ASSIGN_BD_ADDR(CurrentRC_BD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
            break;
         case etAUD_Remote_Control_Close_Indication:
            BD_ADDRToStr(AUD_Event_Data->Event_Data.AUD_Remote_Control_Close_Indication_Data->BD_ADDR, BoardStr);

            printf("Close Remote Control Indication: %s, %d.\r\n", BoardStr, AUD_Event_Data->Event_Data.AUD_Remote_Control_Close_Indication_Data->DisconnectReason);

            RemoteControlConnection = FALSE;
            VolumeChangedRegistered = FALSE;
            ASSIGN_BD_ADDR(CurrentRC_BD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
            break;
         case etAUD_Stream_State_Change_Indication:
            /* A local stream endpoint has had the state changed,       */
            /* display the information.                                 */
            printf("AUD Stream State Change Indication, Type: %s.\r\n", (AUD_Event_Data->Event_Data.AUD_Stream_State_Change_Indication_Data->StreamType == astSNK)?"Sink":"Source");

            BD_ADDRToStr(AUD_Event_Data->Event_Data.AUD_Stream_State_Change_Indication_Data->BD_ADDR, BoardStr);
            printf("BD_ADDR: %s.\r\n", BoardStr);
            printf("State:   %s.\r\n", (AUD_Event_Data->Event_Data.AUD_Stream_State_Change_Indication_Data->StreamState == astStreamStarted)?"Started":"Stopped");

            if(AUD_Event_Data->Event_Data.AUD_Stream_State_Change_Indication_Data->StreamState == astStreamStopped)
            {
               SinkStreamStarted = FALSE;
               CleanupAudioDecoder();
            }
            else
            {
               SinkStreamStarted = TRUE;
               InitializeAudioDecoder(BluetoothStackID, DecoderConfigFlags, CurrentSNK_BD_ADDR, AudioFileName);
            }

            /* Signal that the state has changed to the playback thread.*/
            if(AUD_Event_Data->Event_Data.AUD_Stream_State_Change_Indication_Data->StreamType == astSRC)
               BTPS_SetEvent(SourceStreamStateChangedEvent);
            break;
         case etAUD_Stream_State_Change_Confirmation:
            /* A request to change the stream state has returned,       */
            /* display the information.                                 */
            printf("AUD Stream State Change Confirmation, Type: %s.\r\n", (AUD_Event_Data->Event_Data.AUD_Stream_State_Change_Confirmation_Data->StreamType == astSNK)?"Sink":"Source");

            BD_ADDRToStr(AUD_Event_Data->Event_Data.AUD_Stream_State_Change_Confirmation_Data->BD_ADDR, BoardStr);
            printf("BD_ADDR:    %s.\r\n", BoardStr);
            printf("State:      %s.\r\n", (AUD_Event_Data->Event_Data.AUD_Stream_State_Change_Confirmation_Data->StreamState == astStreamStarted)?"Started":"Stopped");
            printf("Successful: %s.\r\n", AUD_Event_Data->Event_Data.AUD_Stream_State_Change_Confirmation_Data->Successful?"TRUE":"FALSE");

            if(AUD_Event_Data->Event_Data.AUD_Stream_State_Change_Confirmation_Data->Successful)
               SinkStreamStarted = (AUD_Event_Data->Event_Data.AUD_Stream_State_Change_Confirmation_Data->StreamState == astStreamStarted);

            if(AUD_Event_Data->Event_Data.AUD_Stream_State_Change_Confirmation_Data->StreamType == astSRC)
               BTPS_SetEvent(SourceStreamStateChangedEvent);
            break;
         case etAUD_Stream_Format_Change_Indication:
            /* A local stream endpoint has had the format changed,      */
            /* display the information.                                 */
            printf("AUD Stream Format Change Indication, Type: %s.\r\n", (AUD_Event_Data->Event_Data.AUD_Stream_Format_Change_Indication_Data->StreamType == astSNK)?"Sink":"Source");

            BD_ADDRToStr(AUD_Event_Data->Event_Data.AUD_Stream_Format_Change_Indication_Data->BD_ADDR, BoardStr);
            printf("BD_ADDR: %s.\r\n", BoardStr);
            printf("Format:  %u, %u. 0x%08X.\r\n", (unsigned int)AUD_Event_Data->Event_Data.AUD_Stream_Format_Change_Indication_Data->StreamFormat.SampleFrequency, (unsigned int)AUD_Event_Data->Event_Data.AUD_Stream_Format_Change_Indication_Data->StreamFormat.NumberChannels, (unsigned int)AUD_Event_Data->Event_Data.AUD_Stream_Format_Change_Indication_Data->StreamFormat.FormatFlags);
            break;
         case etAUD_Stream_Format_Change_Confirmation:
            /* A request to change the stream format has returned,      */
            /* display the information.                                 */
            printf("AUD Stream Format Change Confirmation, Type: %s.\r\n", (AUD_Event_Data->Event_Data.AUD_Stream_Format_Change_Confirmation_Data->StreamType == astSNK)?"Sink":"Source");

            BD_ADDRToStr(AUD_Event_Data->Event_Data.AUD_Stream_Format_Change_Confirmation_Data->BD_ADDR, BoardStr);
            printf("BD_ADDR:    %s.\r\n", BoardStr);
            printf("Format:     %u, %u, 0x%08X.\r\n", (unsigned int)AUD_Event_Data->Event_Data.AUD_Stream_Format_Change_Confirmation_Data->StreamFormat.SampleFrequency, (unsigned int)AUD_Event_Data->Event_Data.AUD_Stream_Format_Change_Confirmation_Data->StreamFormat.NumberChannels, (unsigned int)AUD_Event_Data->Event_Data.AUD_Stream_Format_Change_Confirmation_Data->StreamFormat.FormatFlags);
            printf("Successful: %s.\r\n", AUD_Event_Data->Event_Data.AUD_Stream_Format_Change_Confirmation_Data->Successful?"TRUE":"FALSE");
            break;
         case etAUD_Encoded_Audio_Data_Indication:
            /* Encoded Audio Data has been received. Either process the */
            /* data or just notify the user if there is no processing   */
            /* enabled.                                                 */
            if(DecoderConfigFlags)
            {
               /* Pass the audio data to the decoder.                   */
               ProcessAudioData(AUD_Event_Data->Event_Data.AUD_Encoded_Audio_Data_Indication_Data->RawAudioDataFrame, AUD_Event_Data->Event_Data.AUD_Encoded_Audio_Data_Indication_Data->RawAudioDataFrameLength);

               DisplayPrompt = FALSE;
            }
            else
            {
               if(IgnoredAudioPackets >= IGNORED_AUDIO_PACKETS_THRESHOLD) 
               {
                  printf("\r\nAUD Encoded Audio Data Indication.\r\n");

                  BD_ADDRToStr(AUD_Event_Data->Event_Data.AUD_Encoded_Audio_Data_Indication_Data->BD_ADDR, BoardStr);
                  printf("BD_ADDR: %s.\r\n", BoardStr);
                  printf("Length:  %d.\r\n", AUD_Event_Data->Event_Data.AUD_Encoded_Audio_Data_Indication_Data->RawAudioDataFrameLength);

                  IgnoredAudioPackets = 0;
               }
               else
               {
                  ++IgnoredAudioPackets;
                  DisplayPrompt = FALSE;
               }
            }

            break;
         case etAUD_Signalling_Channel_Open_Indication:
            BD_ADDRToStr(AUD_Event_Data->Event_Data.AUD_Signalling_Channel_Open_Indication_Data->BD_ADDR, BoardStr);
            printf("Signalling Channel Open Indication: %s\r\n", BoardStr);
            break;
         case etAUD_Signalling_Channel_Close_Indication:
            BD_ADDRToStr(AUD_Event_Data->Event_Data.AUD_Signalling_Channel_Close_Indication_Data->BD_ADDR, BoardStr);
            printf("Signalling Channel Close Indication: %s\r\n", BoardStr);
            break;
         case etAUD_Remote_Control_Command_Indication:
            BD_ADDRToStr(AUD_Event_Data->Event_Data.AUD_Remote_Control_Command_Indication_Data->BD_ADDR, BoardStr);
            printf("Remote Control Command Indication.\r\n");
            printf("BD_ADDR:        %s.\r\n", BoardStr); 
            printf("Transaction ID: %u.\r\n", AUD_Event_Data->Event_Data.AUD_Remote_Control_Command_Indication_Data->TransactionID);

            ProcessRemoteControlCommandIndication(AUD_Event_Data->Event_Data.AUD_Remote_Control_Command_Indication_Data);
            break;
         case etAUD_Remote_Control_Command_Confirmation:
            BD_ADDRToStr(AUD_Event_Data->Event_Data.AUD_Remote_Control_Command_Confirmation_Data->BD_ADDR, BoardStr);
            printf("Remote Control Command Confirmation.\r\n");
            printf("BD_ADDR:        %s.\r\n", BoardStr);
            printf("Transaction ID: %u.\r\n", AUD_Event_Data->Event_Data.AUD_Remote_Control_Command_Confirmation_Data->TransactionID);
            printf("Status:         %u.\r\n", AUD_Event_Data->Event_Data.AUD_Remote_Control_Command_Confirmation_Data->ConfirmationStatus);

            ProcessRemoteControlCommandConfirmation(AUD_Event_Data->Event_Data.AUD_Remote_Control_Command_Confirmation_Data);
            break;
         default:
            /* An unknown/unexpected Audio event was received.          */
            printf("Unknown/Unhandled Audio Event: %d.\n", AUD_Event_Data->Event_Data_Type);
            break;
      }

      if(DisplayPrompt)
      {
         printf("\r\nAUD>");

         /* Make sure the output is displayed to the user.              */
         fflush(stdout);
      }
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
         EndPointType = strtol(argv[2], &endptr, 10);

         /* Verify that at least a single supported Endpoint type was   */
         /* specified.                                                  */
         if(EndPointType & (ENDPOINT_TYPE_SNK | ENDPOINT_TYPE_SRC))
         {
            /* Try to Open the stack and check if it was successful.    */
            if(!OpenStack(HCI_DriverInformationPtr))
            {
               /* First attempt to set the Device to be Connectable.    */
               Result = SetConnect();

               /* Next, check to see if the Device was successfully made*/
               /* Connectable.                                          */
               if(!Result)
               {
                  /* Now that the device is Connectable attempt to make */
                  /* it Discoverable.                                   */
                  Result = SetDisc();

                  /* Next, check to see if the Device was successfully  */
                  /* made Discoverable.                                 */
                  if(!Result)
                  {
                     /* Now that the device is discoverable attempt to  */
                     /* make it pairable.                               */
                     Result = SetPairable();

                     if(!Result)
                     {
                        /* The Device is now Connectable, Discoverable, */
                        /* and Pairable so open up the Audio SRC and/or */
                        /* Audio SNK.                                   */
                        if(!Initialize())
                           UserInterface();
                     }
                  }
               }

               /* Close the Bluetooth Stack.                            */
               CloseStack();
            }
            else
            {
               /* There was an error while attempting to open the Stack.*/
               printf("Unable to open the stack.\r\n");
            }
         }
         else
         {
            /* Endpoint type that was specified is incorrect.           */
            printf("Parameter Error (Expecting = [USB = 0, UART = 1, BCSP = 2 Flag] [EndPoint Flags (1=SRC 2=SNK)] [IF !USB [Comm Port or Device File] [Baud Rate]])\r\n");
         }
      }
      else
      {
         /* One or more of the Command Line parameters appear to be     */
         /* invalid.                                                    */
         printf("Parameter Error (Expecting = [USB = 0, UART = 1, BCSP = 2 Flag] [EndPoint Flags (1=SRC 2=SNK)] [IF !USB [Comm Port or Device File] [Baud Rate]])\r\n");
      }
   }
   else
   {
      /* An invalid number of parameters was entered on the Command     */
      /* Line.                                                          */
      printf("Parameter Error (Expecting = [USB = 0, UART = 1, BCSP = 2 Flag] [EndPoint Flags (1=SRC 2=SNK)] [IF !USB [Comm Port or Device File] [Baud Rate]])\r\n");
   }

   return(0);
}

