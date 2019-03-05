/*****< linuxbpp.c >***********************************************************/
/*      Copyright 2005 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  LINUXBPP - Basic Printing Profile Stack Application for Linux Main        */
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
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "LinuxBPP.h"      /* Main Application Prototypes and Constants.      */

#include "SS1BTPS.h"       /* Includes for the SS1 Bluetooth Protocol Stack.  */
#include "SS1BTBPP.h"      /* Includes/Constants for the BPP profile.         */
#include "SS1BTDBG.h"      /* Includes/Constants for Bluetooth Debugging.     */

#include "SS1SXMLP.h"      /* Simple XML Parser Prototypes/Constants.         */

#define LOCAL_NAME_ROOT                      "LinuxBPP"  /* Root of the local */
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

#define MAX_NUM_OF_PARAMETERS                       (5)  /* Denotes the max   */
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

   /* The following constants represent the default log file names that */
   /* are used if no Log file name is specified when enabling debug.    */
#define DEFAULT_DEBUG_LOG_FILE_NAME  "LinuxBPP_ASC.log"
#define DEFAULT_DEBUG_FTS_FILE_NAME  "LinuxBPP_FTS.log"

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

   /* Denotes the Application Function Error Value.                     */
#define APPLICATION_ERROR                           (-1)

   /* The following constant represent the canned description strings   */
   /* that are used for File Push and Send Document Client operations   */
   /* (Sender Mode).                                                    */
#define FILE_PUSH_DESCRIPTION       "File Push Description"
#define SEND_DOCUMENT_DESCRIPTION   "Send Document Description"

   /* The following bits are used to indicate which server ports are    */
   /* currently connected.                                              */
#define JOB_SERVER_PORT_CONNECTED_BIT               (0x00000001)
#define STATUS_SERVER_PORT_CONNECTED_BIT            (0x00000002)
#define REFERENCED_OBJECT_SERVER_PORT_CONNECTED_BIT (0x00000004)

   /* The following are the return values from the                      */
   /* JobServerPortGetFileData() and JobServerPortPutFileData()         */
   /* functions.                                                        */
#define FILE_DATA_FILE_IO_ERROR                                (-1)
#define FILE_DATA_FILE_IO_SUCCESS                               (0)
#define FILE_DATA_END_OF_FILE                                   (1)

   /* Denotes the constant value returned from the GetJobId() function  */
   /* is an error occurs.                                               */
#define GET_JOB_ID_ERROR                                        (-1)

   /* Denotes the constant value of the indent length per level of      */
   /* display.                                                          */
#define INDENT_LENGTH                                           (3)

   /* Denotes the constant value of the SOAP Data Element Root ID.      */
#define SOAP_DATA_ELEMENT_ROOT_ID                               (0)

   /* Denotes the constant value returned from the                      */
   /* BuildSOAPDataElementsFromSOAPDataElementList() function.          */
#define BUILD_SOAP_DATA_ELEMENTS_FROM_SOAP_DATA_ELEMENT_LIST_NO_CHILDREN (1)
#define BUILD_SOAP_DATA_ELEMENTS_FROM_SOAP_DATA_ELEMENT_LIST_ERROR (-1)

   /* Denotes the constant value returned from the                      */
   /* BuildSOAPDataElementsFromXMLFileData() function if an error       */
   /* occurs.                                                           */
#define BUILD_SOAP_DATA_ELEMENTS_FROM_XML_FILE_DATA                (-1)

#define _TAB_                                           ((char)(0x09))
#define _SPACE_                                          ((char)(' '))
#define _CARRIAGE_RETURN_                               ((char)('\r'))
#define _LINE_FEED_                                     ((char)('\n'))
#define _WHITE_SPACE_CHARACTER_                         ((char)(0xFE))

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

   /* Defines the types of service which are provided.                  */
typedef enum
{
   stPrinter,
   stSender
} ServiceType_t;

typedef enum
{
   coNone,
   coAbort,
   coFilePush,
   coGetPrinterAttributes,
   coCreateJob,
   coSendDocument,
   coGetJobAttributes,
   coCancelJob,
   coGetEvent,
   coGetReferencedObjects
} CurrentOperation_t;

   /* The following enumerated type represents all possible Process XML */
   /* States.                                                           */
typedef enum
{
   psWaitingOpeningElement,
   psWaitingClosingElement,
   psComplete,
   psError
} ProcessXMLState_t;

   /* The following structure represents the container structure for a  */
   /* single BPP SOAP Data Element Entry.                               */
typedef struct _tagBPPSOAPDataElementEntry_t
{
   unsigned int                          SOAPDataElementID;
   unsigned int                          SOAPDataElementParentID;
   unsigned int                          NameLength;
   Byte_t                               *Name;
   unsigned int                          ValueLength;
   Byte_t                               *Value;
   struct _tagBPPSOAPDataElementEntry_t *NextBPPSOAPDataElementEntryPtr;
} BPPSOAPDataElementEntry_t;

   /* The following structure represents the container structure for    */
   /* Process XML State Information.                                    */
typedef struct _tagProcessXMLStateInfo_t
{
   unsigned int               CurrentSOAPDataElementID;
   unsigned int               OpeningElementNameLength;
   Byte_t                    *OpeningElementName;
   ProcessXMLState_t          ProcessXMLState;
   BPPSOAPDataElementEntry_t *SOAPDataElementList;
} ProcessXMLStateInfo_t;

   /* The following structure represents the container structure for a  */
   /* single entry in the Response Code Table.                          */
typedef struct _tagResponseCodeEntry_t
{
   char   *ResponseCodeName;
   Byte_t  ResponseCodeValue;
} ResponseCodeEntry_t;

   /* The following buffer contains the Document Formats Supported by   */
   /* the BPP Print Server.                                             */
   /* * NOTE * Some device implementations (Samsung BPP in particular)  */
   /*          do not function correclty when the Document Formats      */
   /*          Supported is too large (this string is added to the SDP  */
   /*          record).  This is something to be aware of.  The string  */
   /*          below (the one with the preceding "_" character) is a    */
   /*          large string that can be used to copy items from as it   */
   /*          contains most of the defined formats.  The string below  */
   /*          it (the one without the preceding "_" character) can used*/
   /*          so that the sample application works correctly as-is with*/
   /*          BPP implementations that do not parse the SDP Documents  */
   /*          Format Supported Attribute.                              */
   /* * NOTE * It appears that the "magic length" that works with the   */
   /*          aforementioned Samsung BPP implementations is 180 bytes. */
   /*          when the string is longer (in bytes) than this value     */
   /*          (i.e.  181 bytes or longer) the BPP client will not print*/
   /*          to a BPP server.  When it is less than or equal to this  */
   /*          value, the BPP client appears to work fine.              */
static const char _DocumentFormatsSupported[] = "application/vnd.pwg-xhtml-print+xml:1.0,application/vnd.pwg-xhtml-print+xml:0.95,text/plain,text/x-vcard:2.1,text/plain,text/x-vcard:3.0,text/x-vcalendar:1.0,text/calendar:2.0,text/x-vmessage:1.1,application/PostScript:2,application/PostScript:3,application/vnd.hp-PCL:5E,application/vnd.hp-PCL:3C,application/PDF,image/jpeg,image/gif:89A,application/vnd.pwg-multiplexed";
static const char DocumentFormatsSupported[] = "application/vnd.pwg-xhtml-print+xml:1.0,text/plain,text/x-vcard:2.1,text/x-vcalendar:1.0,text/calendar:2.0,text/x-vmessage:1.1,image/jpeg,image/gif,application/vnd.pwg-multiplexed";

   /* The following buffer contains the Print Image Formats Supported by*/
   /* the BPP Print Server.                                             */
static const char PrintImageFormatsSupported[] = "image/jpeg";

   /* The following buffer contains the BPP 1284ID Supported by the BPP */
   /* Print Server.                                                     */
static const char Sample1284IDString[] = "Sample BPP Print Server 1284 ID String.";

   /* The following buffer contains the Character Repertoires Supported */
   /* by the BPP Print Server.                                          */
static BPP_Character_Repertoires_t CharacterRepertoiresSupported = { 0xF3, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                                     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

   /* The following table contains all possible document types by ASCII */
   /* text and their associated enumerated type.                        */
static const ResponseCodeEntry_t ResponseCodeTable[] =
{
   { "Continue",                      BPP_OBEX_RESPONSE_CONTINUE                      },
   { "Ok",                            BPP_OBEX_RESPONSE_OK                            },
   { "Created",                       BPP_OBEX_RESPONSE_CREATED                       },
   { "Accepted",                      BPP_OBEX_RESPONSE_ACCEPTED                      },
   { "Non Authoritative Information", BPP_OBEX_RESPONSE_NON_AUTHORITATIVE_INFORMATION },
   { "No Content",                    BPP_OBEX_RESPONSE_NO_CONTENT                    },
   { "Reset Content",                 BPP_OBEX_RESPONSE_RESET_CONTENT                 },
   { "Partial Content",               BPP_OBEX_RESPONSE_PARTIAL_CONTENT               },
   { "Multiple Choices",              BPP_OBEX_RESPONSE_MULTIPLE_CHOICES              },
   { "Moved Permanently",             BPP_OBEX_RESPONSE_MOVED_PERMANENTLY             },
   { "Moved Temporarily",             BPP_OBEX_RESPONSE_MOVED_TEMPORARILY             },
   { "See Other",                     BPP_OBEX_RESPONSE_SEE_OTHER                     },
   { "Not Modified",                  BPP_OBEX_RESPONSE_NOT_MODIFIED                  },
   { "Use Proxy",                     BPP_OBEX_RESPONSE_USE_PROXY                     },
   { "Bad Request",                   BPP_OBEX_RESPONSE_BAD_REQUEST                   },
   { "Unauthorized",                  BPP_OBEX_RESPONSE_UNAUTHORIZED                  },
   { "Payment Required",              BPP_OBEX_RESPONSE_PAYMENT_REQUIRED              },
   { "Forbidden",                     BPP_OBEX_RESPONSE_FORBIDDEN                     },
   { "Not Found",                     BPP_OBEX_RESPONSE_NOT_FOUND                     },
   { "Method Not Allowed",            BPP_OBEX_RESPONSE_METHOD_NOT_ALLOWED            },
   { "Not Acceptable",                BPP_OBEX_RESPONSE_NOT_ACCEPTABLE                },
   { "Proxy Authentication Required", BPP_OBEX_RESPONSE_PROXY_AUTHENTICATION_REQUIRED },
   { "Request Timeout",               BPP_OBEX_RESPONSE_REQUEST_TIMEOUT               },
   { "Conflict",                      BPP_OBEX_RESPONSE_CONFLICT                      },
   { "Gone",                          BPP_OBEX_RESPONSE_GONE                          },
   { "Length Required",               BPP_OBEX_RESPONSE_LENGTH_REQUIRED               },
   { "Precondition Failed",           BPP_OBEX_RESPONSE_PRECONDITION_FAILED           },
   { "Requested Entity Too Large",    BPP_OBEX_RESPONSE_REQUESTED_ENTITY_TOO_LARGE    },
   { "Requested URL Too Large",       BPP_OBEX_RESPONSE_REQUESTED_URL_TOO_LARGE       },
   { "Unsupported Media Type",        BPP_OBEX_RESPONSE_UNSUPPORTED_MEDIA_TYPE        },
   { "Internal Server Error",         BPP_OBEX_RESPONSE_INTERNAL_SERVER_ERROR         },
   { "Not Implemented",               BPP_OBEX_RESPONSE_NOT_IMPLEMENTED               },
   { "Bad Gateway",                   BPP_OBEX_RESPONSE_BAD_GATEWAY                   },
   { "Service Unavailable",           BPP_OBEX_RESPONSE_SERVICE_UNAVAILABLE           },
   { "Gateway Timeout",               BPP_OBEX_RESPONSE_GATEWAY_TIMEOUT               },
   { "HTTP Version Not Supported",    BPP_OBEX_RESPONSE_HTTP_VERSION_NOT_SUPPORTED    },
   { "Database Full",                 BPP_OBEX_RESPONSE_DATABASE_FULL                 },
   { "Database Locked",               BPP_OBEX_RESPONSE_DATABASE_LOCKED               }
} ;

#define NUMBER_RESPONSE_CODE_TABLE_ENTRIES              (sizeof(ResponseCodeTable)/sizeof(ResponseCodeEntry_t))

   /* The following constant strings represent the Job Id Element Name  */
   /* that exist in the SOAP Stream.                                    */
static const char JobIdElementNameValue[] = "JobId";

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the         */
   /* compiler as part of standard C/C++).                              */
static unsigned int        BluetoothStackID;        /* Variable which holds the Handle */
                                                    /* of the opened Bluetooth Protocol*/
                                                    /* Stack.                          */

static unsigned int        DebugID;                 /* Variable which holds the ID of  */
                                                    /* the currently enabled debugging */
                                                    /* session.                        */

static ServiceType_t       ServiceType;             /* Variable which holds the        */
                                                    /* current Service Type that the   */
                                                    /* application is serving as.      */

static unsigned int        BPPServerID;             /* Variable which holds the        */
                                                    /* current BPP Server ID of any    */
                                                    /* active BPP Server.              */

static unsigned long       ServerPortsConnected;    /* Variable which contains a       */
                                                    /* bit-mask of all currently       */
                                                    /* connected Server Ports.         */

static unsigned int        BPPClientID;             /* Variable which holds the        */
                                                    /* current BPP Client ID of any    */
                                                    /* active BPP Client.              */

static unsigned long       ClientPortsConnected;    /* Variable which contains a       */
                                                    /* bit-mask of all currently       */
                                                    /* connected Client Ports.         */

static DWord_t             ServiceRecordHandle;     /* Variable which holds the SDP    */
                                                    /* Service Record handle of the    */
                                                    /* active BPP Server.              */

static char                RootDirectory[1024+1];   /* Variable which contains the     */
                                                    /* Root Directory that is used to  */
                                                    /* store all received BPP          */
                                                    /* information.                    */

   /* Variables that contain BPP State Information relating to a Job    */
   /* Server.                                                           */
static char                JobServerPortCurrentFile[1024+1];
static DWord_t             JobServerPortCurrentFileIndex;
static DWord_t             JobServerPortCurrentFileSize;
static Boolean_t           JobServerPortFirstPhase;
static DWord_t             JobServerPortCurrentFileBufferIndex;
static unsigned char       JobServerPortCurrentFileBuffer[64*1024];
static unsigned int        JobServerPortValue;
static CurrentOperation_t  JobServerPortCurrentOperation;

   /* Variables that contain BPP State Information relating to a Status */
   /* Server.                                                           */
static unsigned int        StatusServerPortValue;
static CurrentOperation_t  StatusServerPortCurrentOperation;

   /* Variables that contain BPP State Information relating to an Object*/
   /* Server.                                                           */
static unsigned int        ReferencedObjectServerPortValue;
static CurrentOperation_t  ReferencedObjectsServerPortCurrentOperation;
static char                ReferencedObjectsServerPortCurrentFile[1024+1];
static DWord_t             ReferencedObjectsServerPortCurrentFileStartIndex;
static DWord_t             ReferencedObjectsServerPortCurrentFileIndex;
static DWord_t             ReferencedObjectsServerPortCurrentFileLengthToSend;
static Boolean_t           ReferencedObjectsServerPortFirstPhase;
static DWord_t             ReferencedObjectsServerPortCurrentFileBufferIndex;
static unsigned char       ReferencedObjectsServerPortCurrentFileBuffer[64*1024];

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

   /* Internal Function Prototypes.                                     */
static void DisplayMenu_Printer(void);
static void DisplayMenu_Sender(void);

static void UserInterface_Printer(void);
static void UserInterface_Sender(void);

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

static int OpenStack(HCI_DriverInformation_t *HCI_DriverInformation);
static int CloseStack(void);

static int SetDisc(void);
static int SetConnect(void);
static int SetPairable(void);
static int DeleteLinkKey(BD_ADDR_t BD_ADDR);

static int InitializePrintServer(ParameterList_t *TempParam);
static int CleanupPrintServer(ParameterList_t *TempParam);
static int OpenRemoteSenderServer(ParameterList_t *TempParam);
static int ClosePrinterConnections(ParameterList_t *TempParam);
static int GetPrinterAttributesResponse(ParameterList_t *TempParam);
static int CreateJobResponse(ParameterList_t *TempParam);
static int GetJobAttributesResponse(ParameterList_t *TempParam);
static int CancelJobResponse(ParameterList_t *TempParam);
static int GetEventResponse(ParameterList_t *TempParam);
static int GetReferencedObjects(ParameterList_t *TempParam);
static int InitializeSenderServer(ParameterList_t *TempParam);
static int CleanupSenderServer(ParameterList_t *TempParam);
static int OpenRemotePrintServer(ParameterList_t *TempParam);
static int CloseSenderConnections(ParameterList_t *TempParam);
static int FilePush(ParameterList_t *TempParam);
static int GetPrinterAttributesRequest(ParameterList_t *TempParam);
static int CreateJobRequest(ParameterList_t *TempParam);
static int GetJobAttributesRequest(ParameterList_t *TempParam);
static int CancelJobRequest(ParameterList_t *TempParam);
static int GetEventRequest(ParameterList_t *TempParam);
static int SendDocument(ParameterList_t *TempParam);
static int Abort(ParameterList_t *TempParam);

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

static int JobServerPortGetFileData(void);
static int JobServerPortPutFileData(unsigned int DataLength, unsigned char *DataBuffer);

static int ReferencedObjectServerPortGetFileData(void);
static int ReferencedObjectServerPortPutFileData(unsigned int DataLength, unsigned char *DataBuffer);
static unsigned long GetReferencedObjectServerPortFileSize(void);

static void ParseFileName(char *PathFileName, char *FileName);
static unsigned int GetTempFileName(char *PathName, char *PrefixString, char *TempFileName);

static int GetJobId(unsigned int NumberDataElements, BPP_SOAP_Data_Element_t *DataElementBuffer, DWord_t *JobId);

static unsigned int GetNextSOAPDataElementID(void);

static BPPSOAPDataElementEntry_t *AddBPPSOAPDataElementEntry(BPPSOAPDataElementEntry_t **ListHead, BPPSOAPDataElementEntry_t *EntryToAdd);
static BPPSOAPDataElementEntry_t *SearchBPPSOAPDataElementEntry(BPPSOAPDataElementEntry_t **ListHead, unsigned int SOAPDataElementID);
static BPPSOAPDataElementEntry_t *DeleteBPPSOAPDataElementEntryByParentID(BPPSOAPDataElementEntry_t **ListHead, unsigned int SOAPDataElementParentID);
static unsigned int CountBPPSOAPDataElementEntryByParentID(BPPSOAPDataElementEntry_t **ListHead, unsigned int SOAPDataElementParentID);
static void FreeBPPSOAPDataElementEntryMemory(BPPSOAPDataElementEntry_t *EntryToFree);
static void FreeBPPSOAPDataElementList(BPPSOAPDataElementEntry_t **ListHead);

static int BuildSOAPDataElementsFromXMLFileData(char *Path, unsigned char **Buffer, unsigned int *NumberDataElements, BPP_SOAP_Data_Element_t **DataElementBuffer);
static int BuildSOAPDataElementsFromSOAPDataElementList(BPPSOAPDataElementEntry_t **SOAPDataElementList, unsigned int SOAPDataElementParentID, unsigned int *NumberDataElements, BPP_SOAP_Data_Element_t **DataElementBuffer);
static void FreeSOAPDataElements(unsigned char *Buffer, unsigned int NumberDataElements, BPP_SOAP_Data_Element_t *DataElementBuffer);

static void DisplaySOAPDataElements(unsigned int NumberDataElements, BPP_SOAP_Data_Element_t *DataElementBuffer, unsigned int InitialLevel);
static void DisplaySOAPDataElement(BPP_SOAP_Data_Element_t *DataElementBuffer, unsigned int Level);

static void BTPSAPI XMLP_Event_Callback(XMLP_Event_Data_t *XMLP_Event_Data, unsigned long CallbackParameter);

static void BPPOpenPrintServer(unsigned int JobServerPort, unsigned long JobServerPortTargetsMask, unsigned int StatusServerPort);
static void BPPOpenSenderServer(unsigned int ReferencedObjectServerPort);

static void BPPOpenRemotePrintServer(BD_ADDR_t BD_ADDR, unsigned int JobServerPort, BPP_Job_Server_Port_Target_t JobServerPortTarget, unsigned int StatusServerPort);
static void BPPOpenRemoteSenderServer(BD_ADDR_t BD_ADDR, unsigned int ReferencedObjectServerPort);

static void BPPCloseServer(void);
static void BPPCloseConnection(unsigned int BPPID);

static void BPPAbortRequest(unsigned int ServerPort);

static void BPPFilePushRequest(char *MIMEMediaType, char *Path, char *FileName, char *Description);

static void BPPGetPrinterAttributesRequest(unsigned int ServerPort, unsigned int NumberDataElements, BPP_SOAP_Data_Element_t *DataElementBuffer);
static void BPPGetPrinterAttributesResponse(unsigned int ServerPort, Byte_t ResponseCode, unsigned int NumberDataElements, BPP_SOAP_Data_Element_t *DataElementBuffer);

static void BPPCreateJobRequest(unsigned int NumberDataElements, BPP_SOAP_Data_Element_t *DataElementBuffer);
static void BPPCreateJobResponse(Byte_t ResponseCode, DWord_t JobId, unsigned int NumberDataElements, BPP_SOAP_Data_Element_t *DataElementBuffer);

static void BPPSendDocumentRequest(DWord_t JobId, char *MIMEMediaType, char *Path, char *FileName, char *Description);

static void BPPGetJobAttributesRequest(unsigned int ServerPort, unsigned int NumberDataElements, BPP_SOAP_Data_Element_t *DataElementBuffer);
static void BPPGetJobAttributesResponse(unsigned int ServerPort, Byte_t ResponseCode, unsigned int NumberDataElements, BPP_SOAP_Data_Element_t *DataElementBuffer);

static void BPPCancelJobRequest(unsigned int ServerPort, unsigned int NumberDataElements, BPP_SOAP_Data_Element_t *DataElementBuffer);
static void BPPCancelJobResponse(unsigned int ServerPort, Byte_t ResponseCode, unsigned int NumberDataElements, BPP_SOAP_Data_Element_t *DataElementBuffer);

static void BPPGetEventRequest(unsigned int NumberDataElements, BPP_SOAP_Data_Element_t *DataElementBuffer);
static void BPPGetEventResponse(Byte_t ResponseCode, unsigned int NumberDataElements, BPP_SOAP_Data_Element_t *DataElementBuffer);

static void BPPGetReferencedObjectsRequest(char *ObjectIdentifier, DWord_t Offset, DWord_t Count, Boolean_t RequestObjectSize);

static void DisplaySDPAttributeResponse(SDP_Service_Attribute_Response_Data_t *SDPServiceAttributeResponse, unsigned int InitLevel);
static void DisplaySDPSearchAttributeResponse(SDP_Service_Search_Attribute_Response_Data_t *SDPServiceSearchAttributeResponse);
static void DisplayDataElement(SDP_Data_Element_t *SDPDataElement, unsigned int Level);

   /* Callback Function Prototypes.                                     */
static void BTPSAPI GAP_Event_Callback(unsigned int BluetoothStackID, GAP_Event_Data_t *GAPEventData, unsigned long CallbackParameter);
static void BTPSAPI SDP_Event_Callback(unsigned int BluetoothStackID, unsigned int SDPRequestID, SDP_Response_Data_t *SDP_Response_Data, unsigned long CallbackParameter);
static void BTPSAPI BPP_Event_Callback(unsigned int BluetoothStackID, BPP_Event_Data_t *BPPEventData, unsigned long CallbackParameter);

   /* The following function displays the Printer Command Menu.         */
static void DisplayMenu_Printer(void)
{
   /* First display the upper command bar.                              */
   printf("************************** Command Options ***************************\r\n");

   /* Next, display all of the commands.                                */
   printf("*  Inquiry                                                           *\r\n");
   printf("*  DisplayInquiryList                                                *\r\n");
   printf("*  Pair [Inquiry Index] [Bonding Type]                               *\r\n");
   printf("*  EndPairing [Inquiry Index]                                        *\r\n");
   printf("*  PINCodeResponse [PIN Code]                                        *\r\n");
   printf("*  PassKeyResponse [Numeric Passkey]                                 *\r\n");
   printf("*  UserConfirmationResponse [Confirmation Flag]                      *\r\n");
   printf("*  SetDiscoverabilityMode [Discoverability Mode]                     *\r\n");
   printf("*  SetConnectabilityMode [Connectability Mode]                       *\r\n");
   printf("*  SetPairabilityMode [Pairability Mode]                             *\r\n");
   printf("*  ChangeSimplePairingParameters [I/O Capabilities] [MITM Flag]      *\r\n");
   printf("*  GetLocalAddress                                                   *\r\n");
   printf("*  GetLocalName                                                      *\r\n");
   printf("*  SetLocalName [Local Device Name (no spaces allowed)]              *\r\n");
   printf("*  GetClassOfDevice                                                  *\r\n");
   printf("*  SetClassOfDevice [Class of Device]                                *\r\n");
   printf("*  GetRemoteName [Inquiry Index]                                     *\r\n");
   printf("*  ServiceDiscovery [Inquiry Index] [Profile Index] [UUID (Manual)]  *\r\n");
   printf("*  InitializePrintServer [Job Server Port] [Status Server Port]      *\r\n");
   printf("*  CleanupPrintServer                                                *\r\n");
   printf("*  OpenClient [Inquiry Index] [Referenced Object Server Port]        *\r\n");
   printf("*  CloseConnections [Connection Bit Mask]                            *\r\n");
   printf("*  GetPrinterAttrResponse [Port] [Response Code] [Data Element File] *\r\n");
   printf("*  CreateJobResponse [Response Code] [Data Element File]             *\r\n");
   printf("*  GetJobAttrResponse [Port] [Response Code] [Data Element File]     *\r\n");
   printf("*  CancelJobResponse [Port] [Response Code] [Data Element File]      *\r\n");
   printf("*  GetEventResponse [Response Code] [Data Element File]              *\r\n");
   printf("*  GetReferencedObjects [Object] [Offset] [Count] [Request Size]     *\r\n");
   printf("*  Abort [Port]                                                      *\r\n");
   printf("*  EnableDebug [Enable/Disable] [Log Type] [Log File Name]           *\r\n");
   printf("*  Help                                                              *\r\n");

   printf("**********************************************************************\r\n");
}

   /* The following function displays the Sender Command Menu.          */
static void DisplayMenu_Sender(void)
{
   /* First display the upper command bar.                              */
   printf("************************** Command Options ***************************\r\n");

   /* Next, display all of the commands.                                */
   printf("*  Inquiry                                                           *\r\n");
   printf("*  DisplayInquiryList                                                *\r\n");
   printf("*  Pair [Inquiry Index] [Bonding Type]                               *\r\n");
   printf("*  EndPairing [Inquiry Index]                                        *\r\n");
   printf("*  PINCodeResponse [PIN Code]                                        *\r\n");
   printf("*  PassKeyResponse [Numeric Passkey]                                 *\r\n");
   printf("*  UserConfirmationResponse [Confirmation Flag]                      *\r\n");
   printf("*  SetDiscoverabilityMode [Discoverability Mode]                     *\r\n");
   printf("*  SetConnectabilityMode [Connectability Mode]                       *\r\n");
   printf("*  SetPairabilityMode [Pairability Mode]                             *\r\n");
   printf("*  ChangeSimplePairingParameters [I/O Capabilities] [MITM Flag]      *\r\n");
   printf("*  GetLocalAddress                                                   *\r\n");
   printf("*  GetLocalName                                                      *\r\n");
   printf("*  SetLocalName [Local Device Name (no spaces allowed)]              *\r\n");
   printf("*  GetClassOfDevice                                                  *\r\n");
   printf("*  SetClassOfDevice [Class of Device]                                *\r\n");
   printf("*  GetRemoteName [Inquiry Index]                                     *\r\n");
   printf("*  ServiceDiscovery [Inquiry Index] [Profile Index] [UUID (Manual)]  *\r\n");
   printf("*  InitializeSenderServer [Referenced Objects Server Port]           *\r\n");
   printf("*  CleanupSenderServer                                               *\r\n");
   printf("*  OpenClient [Inquiry Index] [Job Server Port] [Status Server Port] *\r\n");
   printf("*  CloseConnections [Connection Bit Mask]                            *\r\n");
   printf("*  FilePush [MIME Type] [File Name]                                  *\r\n");
   printf("*  GetPrinterAttrRequest [Port] [Data Element File]                  *\r\n");
   printf("*  CreateJobRequest [Data Element File]                              *\r\n");
   printf("*  GetJobAttrRequest [Port] [Data Element File]                      *\r\n");
   printf("*  CancelJobRequest [Port] [Data Element File]                       *\r\n");
   printf("*  GetEventRequest [Data Element File]                               *\r\n");
   printf("*  SendDocument [Job ID] [MIME Type] [File Name]                     *\r\n");
   printf("*  Abort [Port]                                                      *\r\n");
   printf("*  EnableDebug [Enable/Disable] [Log Type] [Log File Name]           *\r\n");
   printf("*  Help                                                              *\r\n");

   printf("**********************************************************************\r\n");
}

   /* This function is responsible for taking the users input and do the*/
   /* appropriate thing with it.  First, this function get a string of  */
   /* user input, parse the user input in to command and parameters, and*/
   /* finally executing the command or display an error message if the  */
   /* input is corrupt.                                                 */
static void UserInterface_Printer(void)
{
   UserCommand_t TempCommand;
   int  Result = !EXIT_CODE;
   char UserInput[MAX_COMMAND_LENGTH];

   /* First let's make sure that we start on new line.                  */
   printf("\r\n");

   /* Next display the available commands.                              */
   DisplayHelp(NULL);

   ClearCommands();

   AddCommand("INITIALIZE", InitializePrintServer);
   AddCommand("CLEANUP", CleanupPrintServer);
   AddCommand("OPENCLIENT", OpenRemoteSenderServer);
   AddCommand("CLOSECONNECTIONS", ClosePrinterConnections);
   AddCommand("GETPRINTERATTRRESPONSE", GetPrinterAttributesResponse);
   AddCommand("CREATEJOBRESPONSE", CreateJobResponse);
   AddCommand("GETJOBATTRRESPONSE", GetJobAttributesResponse);
   AddCommand("CANCELJOBRESPONSE", CancelJobResponse);
   AddCommand("GETEVENTRESPONSE", GetEventResponse);
   AddCommand("GETREFERENCEDOBJECTS", GetReferencedObjects);
   AddCommand("ABORT", Abort);
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
      printf("Printer>");

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
                     /* go ahead an close any BPP connections we have   */
                     /* open.                                           */
                     if(BPPServerID)
                        CleanupPrintServer(NULL);

                     if(BPPClientID)
                        BPPCloseConnection(BPPClientID);
                     break;
               }
            }
            else
               printf("Invalid Input.\r\n");
         }
      }
      else
      {
         /* Close any open connections.                                 */
         if(BPPServerID)
            CleanupPrintServer(NULL);

         if(BPPClientID)
            BPPCloseConnection(BPPClientID);

         Result = EXIT_CODE;
      }
   }
}

   /* This function is responsible for taking the users input and do the*/
   /* appropriate thing with it.  First, this function get a string of  */
   /* user input, parse the user input in to command and parameters, and*/
   /* finally executing the command or display an error message if the  */
   /* input is corrupt.                                                 */
static void UserInterface_Sender(void)
{
   UserCommand_t TempCommand;
   int  Result = !EXIT_CODE;
   char UserInput[MAX_COMMAND_LENGTH];

   /* First let's make sure that we start on new line.                  */
   printf("\r\n");

   /* Next display the available commands.                              */
   DisplayHelp(NULL);

   ClearCommands();

   AddCommand("INITIALIZE", InitializeSenderServer);
   AddCommand("CLEANUP", CleanupSenderServer);
   AddCommand("OPENCLIENT", OpenRemotePrintServer);
   AddCommand("CLOSE", CloseSenderConnections);
   AddCommand("FILEPUSH", FilePush);
   AddCommand("GETPRINTERATTRREQUEST", GetPrinterAttributesRequest);
   AddCommand("CREATEJOBREQUEST", CreateJobRequest);
   AddCommand("GETJOBATTRREQUEST", GetJobAttributesRequest);
   AddCommand("CANCELJOBREQUEST", CancelJobRequest);
   AddCommand("GETEVENTREQUEST", GetEventRequest);
   AddCommand("SENDDOCUMENT", SendDocument);
   AddCommand("ABORT", Abort);
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
      printf("Sender>");

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
                     /* go ahead an close any BPP connections we have   */
                     /* open.                                           */
                     if(BPPServerID)
                        CleanupSenderServer(NULL);

                     if(BPPClientID)
                        BPPCloseConnection(BPPClientID);
                     break;
               }
            }
            else
               printf("Invalid Input.\r\n");
         }
      }
      else
      {
         /* Close any open connections.                                 */
         if(BPPServerID)
            CleanupSenderServer(NULL);

         if(BPPClientID)
            BPPCloseConnection(BPPClientID);

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

   /* The following function is responsible for redisplaying the Menu   */
   /* options to the user.  This function returns zero on successful    */
   /* execution or a negative value on all errors.                      */
static int DisplayHelp(ParameterList_t *TempParam)
{
   /* First check to if Currently in Printer or Sender Mode.            */
   if(ServiceType == stPrinter)
   {
      /* Currently in Printer, display the Printer Menu.                */
      DisplayMenu_Printer();
   }
   else
   {
      /* Currently in Sender Mode, display the Sender Menu.             */
      DisplayMenu_Sender();
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

   /* The following function is responsible for Initializing a Basic    */
   /* Printing Profile Print Server on the Local Device.  This function */
   /* returns zero if successful and a negative value if an error       */
   /* occurred.                                                         */
static int InitializePrintServer(ParameterList_t *TempParam)
{
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      if(ServiceType == stPrinter)
      {
         /* If a BPP Server is not already opened, then we need to open */
         /* the BPP Print Server.                                       */
         if(!BPPServerID)
         {
            /* Next, let's make sure that the RFCOMM Server Ports were  */
            /* specified as parameters to this command.                 */
            if((TempParam) && (TempParam->NumberofParameters > 1) && (TempParam->Params[0].intParam) && (TempParam->Params[1].intParam))
            {
               /* Next, let's figure out the current working directory  */
               /* of the program so we can use that for our Root        */
               /* Directory.                                            */
               if(!getcwd(RootDirectory, sizeof(RootDirectory)))
               {
                  /* There was an error fetching the Directory, so let's*/
                  /* just default it to the Root Directory.             */
                  RootDirectory[0] = '\0';
               }

               /* Next we will strip off any trailing '/' the caller may*/
               /* have specified (unless it is the root of a drive).    */
               if((strlen(RootDirectory)) && (RootDirectory[strlen(RootDirectory) - 1] == '/'))
                  RootDirectory[strlen(RootDirectory) - 1] = '\0';

               /* Finally make sure the Root Directory ends in the Path */
               /* Delimiter.                                            */
               strcat(RootDirectory, "/");

               /* Note the Server Port Numbers.                         */
               JobServerPortValue    = TempParam->Params[0].intParam;
               StatusServerPortValue = TempParam->Params[1].intParam;

               /* All Parameters appear to be valid, so go ahead and    */
               /* open the BPP Print Server.                            */
               BPPOpenPrintServer(TempParam->Params[0].intParam, BPP_JOB_SERVER_PORT_TARGET_DIRECT_PRINTING_SERVICE_BIT, TempParam->Params[1].intParam);

               /* Flag success to the caller.                           */
               ret_val = 0;
            }
            else
            {
               printf("Usage: InitializePrintServer [Job Server RFCOMM Port] [Status Server RFCOMM Port].\r\n");

               /* One or more of the necessary parameters is/are        */
               /* invalid.                                              */
               ret_val = INVALID_PARAMETERS_ERROR;
            }
         }
         else
         {
            printf("BPP Server already open.\r\n");

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

   /* The following function is responsible for cleaning up any         */
   /* currently initialized Basic Printing Profile Print Server on the  */
   /* Local Device.  This function returns zero if successful and a     */
   /* negative value if an error occurred.                              */
static int CleanupPrintServer(ParameterList_t *TempParam)
{
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      if(ServiceType == stPrinter)
      {
         /* If a BPP Server is opened, then we need to close the BPP    */
         /* Server.                                                     */
         if(BPPServerID)
         {
            /* Simply go ahead and close the BPP Print Server.          */
            BPPCloseServer();

            JobServerPortValue    = 0;
            StatusServerPortValue = 0;

            /* Flag success to the caller.                              */
            ret_val               = 0;
         }
         else
         {
            printf("There is currently no BPP Print Server open.\r\n");

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

   /* The following function is responsible for opening up a remote     */
   /* connection to a remote Basic Printing Profile Sender Server.  This*/
   /* function returns zero if successful and a negative value if an    */
   /* error occurred.                                                   */
static int OpenRemoteSenderServer(ParameterList_t *TempParam)
{
   int       ret_val;
   BD_ADDR_t NullADDR;

   ASSIGN_BD_ADDR(NullADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      if(ServiceType == stPrinter)
      {
         /* If a BPP Client is not opened then we need to open a remote */
         /* connection.                                                 */
         if(!BPPClientID)
         {
            /* Make sure that all of the parameters required for this   */
            /* function appear to be at least semi-valid.               */
            if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].intParam) && (NumberofValidResponses) && (TempParam->Params[0].intParam <= NumberofValidResponses) && (!COMPARE_BD_ADDR(InquiryResultList[(TempParam->Params[0].intParam - 1)], NullADDR)))
            {
               /* Bluetooth Device appears to be valid, so check to see */
               /* if a valid Server Port was specified.                 */
               if((TempParam->NumberofParameters > 1) && (TempParam->Params[1].intParam))
               {
                  ReferencedObjectServerPortValue = TempParam->Params[1].intParam;

                  BPPOpenRemoteSenderServer(InquiryResultList[(TempParam->Params[0].intParam - 1)], TempParam->Params[1].intParam);

                  /* Flag success to the caller.                        */
                  ret_val = 0;
               }
               else
               {
                  printf("Usage: OpenClient [Inquiry Index] [Referenced Object Server RFCOMM Port].\r\n");

                  /* One or more of the necessary parameters is/are     */
                  /* invalid.                                           */
                  ret_val = INVALID_PARAMETERS_ERROR;
               }
            }
            else
            {
               printf("Usage: OpenClient [Inquiry Index] [Referenced Object Server RFCOMM Port].\r\n");

               /* One or more of the necessary parameters is/are        */
               /* invalid.                                              */
               ret_val = INVALID_PARAMETERS_ERROR;
            }
         }
         else
         {
            printf("There is already a BPP Client Connection open.\r\n");

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

   /* The following function is responsible for closing currently active*/
   /* connections to the Local Printer.  This function returns zero if  */
   /* successful and a negative value if an error occurred.             */
static int ClosePrinterConnections(ParameterList_t *TempParam)
{
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      if(ServiceType == stPrinter)
      {
         /* Make sure that all of the parameters required for this      */
         /* function appear to be at least semi-valid.                  */
         if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].intParam & 0x03))
         {
            /* Parameters appear to be valid, so attempt to close the   */
            /* specified connections.                                   */
            if(TempParam->Params[0].intParam & 0x01)
               BPPCloseConnection(BPPServerID);

            if(TempParam->Params[0].intParam & 0x02)
            {
               BPPCloseConnection(BPPClientID);

               ReferencedObjectServerPortValue = 0;
            }

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            printf("Usage: CloseConnections [Connection Bit Mask].\r\n\r\n where Connection Bit Mask:\r\n   1 - Server\r\n   2 - Client\r\n");

            /* One or more of the necessary parameters is/are invalid.  */
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

   /* The following function is responsible for issuing a Get Printer   */
   /* Attributes Response BPP Action on a Local BPP Server.  This       */
   /* function returns zero if successful and a negative value if an    */
   /* error occurred.                                                   */
static int GetPrinterAttributesResponse(ParameterList_t *TempParam)
{
   int                      ret_val;
   unsigned char           *Buffer;
   unsigned int             NumberDataElements;
   BPP_SOAP_Data_Element_t *DataElementBuffer;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      if(ServiceType == stPrinter)
      {
         /* If a BPP Server is opened, then we need to issue the Get    */
         /* Printer Attributes Response.                                */
         if(BPPServerID)
         {
            /* Next, let's make sure that a Port was specified, followed*/
            /* by a Response Code.                                      */
            if((TempParam) && (TempParam->NumberofParameters > 1) && (TempParam->Params[0].intParam))
            {
               /* Check to see if the response code indicates success.  */
               if((TempParam->Params[1].intParam == BPP_OBEX_RESPONSE_OK) || (TempParam->Params[1].intParam == BPP_OBEX_RESPONSE_CONTINUE))
               {
                  /* The response code is a successful response.        */

                  /* Port and Response Code specified, check to make    */
                  /* sure that the Response Data was specified.         */
                  if((TempParam->NumberofParameters > 2) && (strlen(TempParam->Params[2].strParam)))
                  {
                     /* The parameters appear to be at least semi-valid.*/
                     /* Next attempt to build the SOAP Data Elements    */
                     /* from the XML File Data.                         */
                     if(BuildSOAPDataElementsFromXMLFileData(TempParam->Params[2].strParam, &Buffer, &NumberDataElements, &DataElementBuffer) == 0)
                     {
                        /* The SOAP Data Elements were successfully     */
                        /* built.  Attempt to submit the Get Printer    */
                        /* Attributes Response.                         */
                        BPPGetPrinterAttributesResponse(TempParam->Params[0].intParam, (Byte_t)TempParam->Params[1].intParam, NumberDataElements, DataElementBuffer);

                        /* All through with the SOAP Data Elements, free*/
                        /* the memory.                                  */
                        FreeSOAPDataElements(Buffer, NumberDataElements, DataElementBuffer);

                        /* Flag success to the caller.                  */
                        ret_val = 0;
                     }
                     else
                     {
                        printf("Unable to Build SOAP Data Elements From XML File: %s.\r\n", TempParam->Params[2].strParam);

                        ret_val = INVALID_PARAMETERS_ERROR;
                     }
                  }
                  else
                  {
                     printf("Usage: GetPrinterAttrResponse [RFCOMM Port] [Response Code] [XML Data Element File]\r\n");

                     /* One or more of the necessary parameters is/are  */
                     /* invalid.                                        */
                     ret_val = INVALID_PARAMETERS_ERROR;
                  }
               }
               else
               {
                  /* The response code is an error response.  Attempt to*/
                  /* submit the Get Printer Attributes Response.        */
                  BPPGetPrinterAttributesResponse(TempParam->Params[0].intParam, (Byte_t)TempParam->Params[1].intParam, 0, NULL);

                  /* Flag success to the caller.                        */
                  ret_val = 0;
               }
            }
            else
            {
               printf("Usage: GetPrinterAttrResponse [RFCOMM Port] [Response Code] [XML Data Element File]\r\n");

               /* One or more of the necessary parameters is/are        */
               /* invalid.                                              */
               ret_val = INVALID_PARAMETERS_ERROR;
            }
         }
         else
         {
            printf("There is NO BPP Server Connection currently open.\r\n");

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

   /* The following function is responsible for issuing a Create Job    */
   /* Response BPP Action on a Local BPP Server.  This function returns */
   /* zero if successful and a negative value if an error occurred.     */
static int CreateJobResponse(ParameterList_t *TempParam)
{
   int                      ret_val;
   DWord_t                  JobId;
   unsigned char           *Buffer;
   unsigned int             NumberDataElements;
   BPP_SOAP_Data_Element_t *DataElementBuffer;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      if(ServiceType == stPrinter)
      {
         /* If a BPP Server is opened, then we need to issue the Create */
         /* Job Response.                                               */
         if(BPPServerID)
         {
            /* Next, let's make sure that a Response Code was specified.*/
            if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].intParam))
            {
               /* Check to see if the response code indicates success.  */
               if((TempParam->Params[0].intParam == BPP_OBEX_RESPONSE_OK) || (TempParam->Params[0].intParam == BPP_OBEX_RESPONSE_CONTINUE))
               {
                  /* The response code is a successful response.        */
                  if((TempParam->NumberofParameters > 1) && (strlen(TempParam->Params[1].strParam)))
                  {
                     /* The parameters appear to be at least semi-valid.*/
                     /* Next attempt to build the SOAP Data Elements    */
                     /* from the XML File Data.                         */
                     if(BuildSOAPDataElementsFromXMLFileData(TempParam->Params[1].strParam, &Buffer, &NumberDataElements, &DataElementBuffer) == 0)
                     {
                        /* The SOAP Data Elements were successfully     */
                        /* built.  Initialize the Job Id to an invalid  */
                        /* value.                                       */
                        JobId = BPP_JOB_ID_INVALID_VALUE;
                        GetJobId(NumberDataElements, DataElementBuffer, &JobId);

                        /* Check to see if a valid Job Id has been      */
                        /* located.                                     */
                        if(JobId != BPP_JOB_ID_INVALID_VALUE)
                        {
                           /* Attempt to submit the Create Job Response.*/
                           BPPCreateJobResponse(TempParam->Params[0].intParam, JobId, NumberDataElements, DataElementBuffer);
                        }
                        else
                           printf("Invalid Job ID.\r\n");

                        /* All through with the SOAP Data Elements, free*/
                        /* the memory.                                  */
                        FreeSOAPDataElements(Buffer, NumberDataElements, DataElementBuffer);

                        /* Flag success to the caller.                  */
                        ret_val = 0;
                     }
                     else
                     {
                        printf("Unable to Build SOAP Data Elements From XML File: %s.", TempParam->Params[1].strParam);

                        ret_val = INVALID_PARAMETERS_ERROR;
                     }
                  }
                  else
                  {
                     printf("Usage: CreateJobResponse [Response Code] [XML Data Element File]\r\n");

                     /* One or more of the necessary parameters is/are  */
                     /* invalid.                                        */
                     ret_val = INVALID_PARAMETERS_ERROR;
                  }
               }
               else
               {
                  /* The response code is an error response.  Attempt to*/
                  /* submit the Create Job Response.                    */
                  BPPCreateJobResponse((Byte_t)TempParam->Params[0].intParam, BPP_JOB_ID_INVALID_VALUE, 0, NULL);

                  /* Flag success to the caller.                        */
                  ret_val = 0;
               }
            }
            else
            {
               printf("Usage: CreateJobResponse [Response Code] [XML Data Element File]\r\n");

               /* One or more of the necessary parameters is/are        */
               /* invalid.                                              */
               ret_val = INVALID_PARAMETERS_ERROR;
            }
         }
         else
         {
            printf("There is NO BPP Server Connection currently open.\r\n");

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

   /* The following function is responsible for issuing a Get Job       */
   /* Attributes Response BPP Action on a Local BPP Server.  This       */
   /* function returns zero if successful and a negative value if an    */
   /* error occurred.                                                   */
static int GetJobAttributesResponse(ParameterList_t *TempParam)
{
   int                      ret_val;
   unsigned char           *Buffer;
   unsigned int             NumberDataElements;
   BPP_SOAP_Data_Element_t *DataElementBuffer;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      if(ServiceType == stPrinter)
      {
         /* If a BPP Server is opened, then we need to issue the Get Job*/
         /* Attributes Response.                                        */
         if(BPPServerID)
         {
            /* Next, let's make sure that a Port was specified, followed*/
            /* by a Response Code.                                      */
            if((TempParam) && (TempParam->NumberofParameters > 1) && (TempParam->Params[0].intParam))
            {
               /* Check to see if the response code indicates success.  */
               if((TempParam->Params[1].intParam == BPP_OBEX_RESPONSE_OK) || (TempParam->Params[1].intParam == BPP_OBEX_RESPONSE_CONTINUE))
               {
                  /* The response code is a successful response.        */

                  /* Port and Response Code specified, check to make    */
                  /* sure that the Response Data was specified.         */
                  if((TempParam->NumberofParameters > 2) && (strlen(TempParam->Params[2].strParam)))
                  {
                     /* The parameters appear to be at least semi-valid.*/
                     /* Next attempt to build the SOAP Data Elements    */
                     /* from the XML File Data.                         */
                     if(BuildSOAPDataElementsFromXMLFileData(TempParam->Params[2].strParam, &Buffer, &NumberDataElements, &DataElementBuffer) == 0)
                     {
                        /* The SOAP Data Elements were successfully     */
                        /* built.  Attempt to submit the Get Job        */
                        /* Attributes Response.                         */
                        BPPGetJobAttributesResponse(TempParam->Params[0].intParam, (Byte_t)TempParam->Params[1].intParam, NumberDataElements, DataElementBuffer);

                        /* All through with the SOAP Data Elements, free*/
                        /* the memory.                                  */
                        FreeSOAPDataElements(Buffer, NumberDataElements, DataElementBuffer);

                        /* Flag success to the caller.                  */
                        ret_val = 0;
                     }
                     else
                     {
                        printf("Unable to Build SOAP Data Elements From XML File: %s.\r\n", TempParam->Params[2].strParam);

                        ret_val = INVALID_PARAMETERS_ERROR;
                     }
                  }
                  else
                  {
                     printf("Usage: GetJobAttrResponse [RFCOMM Port] [Response Code] [XML Data Element File]\r\n");

                     /* One or more of the necessary parameters is/are  */
                     /* invalid.                                        */
                     ret_val = INVALID_PARAMETERS_ERROR;
                  }
               }
               else
               {
                  /* The response code is an error response.  Attempt to*/
                  /* submit the Get Job Attributes Response.            */
                  BPPGetJobAttributesResponse(TempParam->Params[0].intParam, (Byte_t)TempParam->Params[1].intParam, 0, NULL);

                  /* Flag success to the caller.                        */
                  ret_val = 0;
               }
            }
            else
            {
               printf("Usage: GetJobAttrResponse [RFCOMM Port] [Response Code] [XML Data Element File]\r\n");

               /* One or more of the necessary parameters is/are        */
               /* invalid.                                              */
               ret_val = INVALID_PARAMETERS_ERROR;
            }
         }
         else
         {
            printf("There is NO BPP Server Connection currently open.\r\n");

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

   /* The following function is responsible for issuing a Cancel Job    */
   /* Response BPP Action on a Local BPP Server.  This function returns */
   /* zero if successful and a negative value if an error occurred.     */
static int CancelJobResponse(ParameterList_t *TempParam)
{
   int                      ret_val;
   unsigned char           *Buffer;
   unsigned int             NumberDataElements;
   BPP_SOAP_Data_Element_t *DataElementBuffer;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      if(ServiceType == stPrinter)
      {
         /* If a BPP Server is opened, then we need to issue the Cancel */
         /* Job Response.                                               */
         if(BPPServerID)
         {
            /* Next, let's make sure that a Port was specified, followed*/
            /* by a Response Code.                                      */
            if((TempParam) && (TempParam->NumberofParameters > 1) && (TempParam->Params[0].intParam))
            {
               /* Check to see if the response code indicates success.  */
               if((TempParam->Params[1].intParam == BPP_OBEX_RESPONSE_OK) || (TempParam->Params[1].intParam == BPP_OBEX_RESPONSE_CONTINUE))
               {
                  /* The response code is a successful response.        */

                  /* Port and Response Code specified, check to make    */
                  /* sure that the Response Data was specified.         */
                  if((TempParam->NumberofParameters > 2) && (strlen(TempParam->Params[2].strParam)))
                  {
                     /* The parameters appear to be at least semi-valid.*/
                     /* Next attempt to build the SOAP Data Elements    */
                     /* from the XML File Data.                         */
                     if(BuildSOAPDataElementsFromXMLFileData(TempParam->Params[2].strParam, &Buffer, &NumberDataElements, &DataElementBuffer) == 0)
                     {
                        /* The SOAP Data Elements were successfully     */
                        /* built.  Attempt to submit the Get Job        */
                        /* Response.                                    */
                        BPPCancelJobResponse(TempParam->Params[0].intParam, (Byte_t)TempParam->Params[1].intParam, NumberDataElements, DataElementBuffer);

                        /* All through with the SOAP Data Elements, free*/
                        /* the memory.                                  */
                        FreeSOAPDataElements(Buffer, NumberDataElements, DataElementBuffer);

                        /* Flag success to the caller.                  */
                        ret_val = 0;
                     }
                     else
                     {
                        printf("Unable to Build SOAP Data Elements From XML File: %s.\r\n", TempParam->Params[2].strParam);

                        ret_val = INVALID_PARAMETERS_ERROR;
                     }
                  }
                  else
                  {
                     printf("Usage: CancelJobResponse [RFCOMM Port] [Response Code] [XML Data Element File]\r\n");

                     /* One or more of the necessary parameters is/are  */
                     /* invalid.                                        */
                     ret_val = INVALID_PARAMETERS_ERROR;
                  }
               }
               else
               {
                  /* The response code is an error response.  Attempt to*/
                  /* submit the Get Printer Attributes Response.        */
                  BPPCancelJobResponse(TempParam->Params[0].intParam, (Byte_t)TempParam->Params[1].intParam, 0, NULL);

                  /* Flag success to the caller.                        */
                  ret_val = 0;
               }
            }
            else
            {
               printf("Usage: CancelJobResponse [RFCOMM Port] [Response Code] [XML Data Element File]\r\n");

               /* One or more of the necessary parameters is/are        */
               /* invalid.                                              */
               ret_val = INVALID_PARAMETERS_ERROR;
            }
         }
         else
         {
            printf("There is NO BPP Server Connection currently open.\r\n");

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

   /* The following function is responsible for issuing a Get Event     */
   /* Response BPP Action on a Local BPP Server.  This function returns */
   /* zero if successful and a negative value if an error occurred.     */
static int GetEventResponse(ParameterList_t *TempParam)
{
   int                      ret_val;
   unsigned char           *Buffer;
   unsigned int             NumberDataElements;
   BPP_SOAP_Data_Element_t *DataElementBuffer;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      if(ServiceType == stPrinter)
      {
         /* If a BPP Server is opened, then we need to issue the Get    */
         /* Event Response.                                             */
         if(BPPServerID)
         {
            /* Next, let's make sure that a Response Code was specified.*/
            if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].intParam))
            {
               /* Check to see if the response code indicates success.  */
               if((TempParam->Params[0].intParam == BPP_OBEX_RESPONSE_OK) || (TempParam->Params[0].intParam == BPP_OBEX_RESPONSE_CONTINUE))
               {
                  /* The response code is a successful response.        */
                  if((TempParam->NumberofParameters > 1) && (strlen(TempParam->Params[1].strParam)))
                  {
                     /* The parameters appear to be at least semi-valid.*/
                     /* Next attempt to build the SOAP Data Elements    */
                     /* from the XML File Data.                         */
                     if(BuildSOAPDataElementsFromXMLFileData(TempParam->Params[1].strParam, &Buffer, &NumberDataElements, &DataElementBuffer) == 0)
                     {
                        /* The SOAP Data Elements were successfully     */
                        /* built.  Attempt to submit the Get Event      */
                        /* Response.                                    */
                        BPPGetEventResponse((Byte_t)TempParam->Params[0].intParam, NumberDataElements, DataElementBuffer);

                        /* All through with the SOAP Data Elements, free*/
                        /* the memory.                                  */
                        FreeSOAPDataElements(Buffer, NumberDataElements, DataElementBuffer);

                        /* Flag success to the caller.                  */
                        ret_val = 0;
                     }
                     else
                     {
                        printf("Unable to Build SOAP Data Elements From XML File: %s.", TempParam->Params[1].strParam);

                        ret_val = INVALID_PARAMETERS_ERROR;
                     }
                  }
                  else
                  {
                     printf("Usage: GetEventResponse [Response Code] [XML Data Element File]\r\n");

                     /* One or more of the necessary parameters is/are  */
                     /* invalid.                                        */
                     ret_val = INVALID_PARAMETERS_ERROR;
                  }
               }
               else
               {
                  /* The response code is an error response.  Attempt to*/
                  /* submit the Get Event Response.                     */
                  BPPGetEventResponse((Byte_t)TempParam->Params[0].intParam, 0, NULL);

                  /* Flag success to the caller.                        */
                  ret_val = 0;
               }
            }
            else
            {
               printf("Usage: GetEventResponse [Response Code] [XML Data Element File]\r\n");

               /* One or more of the necessary parameters is/are        */
               /* invalid.                                              */
               ret_val = INVALID_PARAMETERS_ERROR;
            }
         }
         else
         {
            printf("There is NO BPP Server Connection currently open.\r\n");

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

   /* The following function is responsible for issuing a Get Referenced*/
   /* Object BPP Action on a Remote BPP Server.  This function returns  */
   /* zero if successful and a negative value if an error occurred.     */
static int GetReferencedObjects(ParameterList_t *TempParam)
{
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      if(ServiceType == stPrinter)
      {
         /* If a BPP Client is opened, then we need to issue the Get    */
         /* Referenced Objects Request.                                 */
         if(BPPClientID)
         {
            /* Next, let's make sure that an Object Identifier was      */
            /* specified as a parameter to this command.                */
            if((TempParam) && (TempParam->NumberofParameters > 0) && (strlen(TempParam->Params[0].strParam)))
            {
               /* Object Identifier specfied, check to make sure the    */
               /* offset and count were specified.                      */
               if((TempParam->NumberofParameters > 2) && (TempParam->Params[2].intParam > 0))
               {
                  /* Offset and Count specified, finally issue the Get  */
                  /* Referenced Objects Request.                        */
                  BPPGetReferencedObjectsRequest(TempParam->Params[0].strParam, (DWord_t)TempParam->Params[1].intParam, (DWord_t)TempParam->Params[2].intParam, (Boolean_t)((TempParam->NumberofParameters > 3)?TempParam->Params[3].intParam:FALSE));

                  /* Flag success to the caller.                        */
                  ret_val = 0;
               }
               else
               {
                  printf("Usage: GetReferencedObjects [Object Name] [Offset] [Count] [Request Size Flag (optional)].\r\n");

                  /* One or more of the necessary parameters is/are     */
                  /* invalid.                                           */
                  ret_val = INVALID_PARAMETERS_ERROR;
               }
            }
            else
            {
               printf("Usage: GetReferencedObjects [Object Name] [Offset] [Count] [Request Size Flag (optional)].\r\n");

               /* One or more of the necessary parameters is/are        */
               /* invalid.                                              */
               ret_val = INVALID_PARAMETERS_ERROR;
            }
         }
         else
         {
            printf("There is NO BPP Client Connection currently open.\r\n");

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

   /* The following function is responsible for Initializing a Basic    */
   /* Printing Profile Sender Server on the Local Device.  This function*/
   /* returns zero if successful and a negative value if an error       */
   /* occurred.                                                         */
static int InitializeSenderServer(ParameterList_t *TempParam)
{
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      if(ServiceType == stSender)
      {
         /* If a BPP Server is not already opened, then we need to open */
         /* the BPP Sender Server.                                      */
         if(!BPPServerID)
         {
            /* Next, let's make sure that the RFCOMM Server Port was    */
            /* specified as a parameter to this command.                */
            if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].intParam))
            {
               /* Next, let's figure out the current working directory  */
               /* of the program so we can use that for our Root        */
               /* Directory.                                            */
               if(!getcwd(RootDirectory, sizeof(RootDirectory)))
               {
                  /* There was an error fetching the Directory, so let's*/
                  /* just default it to the Root Directory.             */
                  RootDirectory[0] = '\0';
               }

               /* Next we will strip off any trailing '/' the caller may*/
               /* have specified (unless it is the root of a drive).    */
               if((strlen(RootDirectory)) && (RootDirectory[strlen(RootDirectory) - 1] == '/'))
                  RootDirectory[strlen(RootDirectory) - 1] = '\0';

               /* Finally make sure the Root Directory ends in the Path */
               /* Delimiter.                                            */
               strcat(RootDirectory, "/");

               /* Go ahead and note the Port Value.                     */
               ReferencedObjectServerPortValue = TempParam->Params[0].intParam;

               /* All Parameters appear to be valid, so go ahead and    */
               /* open the BPP Sender Server.                           */
               BPPOpenSenderServer(TempParam->Params[0].intParam);

               /* Flag success to the caller.                           */
               ret_val = 0;
            }
            else
            {
               printf("Usage: InitializeSenderServer [Referenced Objects Server RFCOMM Port].\r\n");

               /* One or more of the necessary parameters is/are        */
               /* invalid.                                              */
               ret_val = INVALID_PARAMETERS_ERROR;
            }
         }
         else
         {
            printf("BPP Server already open.\r\n");

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

   /* The following function is responsible for cleaning up any         */
   /* currently initialized Basic Printing Profile Sender Server on the */
   /* Local Device.  This function returns zero if successful and a     */
   /* negative value if an error occurred.                              */
static int CleanupSenderServer(ParameterList_t *TempParam)
{
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      if(ServiceType == stSender)
      {
         /* If a BPP Server is opened, then we need to close Server.    */
         if(BPPServerID)
         {
            /* Simply go ahead and close the BPP Sender Server.         */
            BPPCloseServer();

            ReferencedObjectServerPortValue = 0;

            /* Flag success to the caller.                              */
            ret_val                         = 0;
         }
         else
         {
            printf("There is currently no BPP Sender Server open.\r\n");

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

   /* The following function is responsible for opening up a remote     */
   /* connection to a remote Basic Printing Profile Printer Server.     */
   /* This function returns zero if successful and a negative value if  */
   /* an error occurred.                                                */
static int OpenRemotePrintServer(ParameterList_t *TempParam)
{
   int       ret_val;
   BD_ADDR_t NullADDR;

   ASSIGN_BD_ADDR(NullADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      if(ServiceType == stSender)
      {
         /* If a BPP Client is not opened then we need to open a remote */
         /* connection.                                                 */
         if(!BPPClientID)
         {
            /* Make sure that all of the parameters required for this   */
            /* function appear to be at least semi-valid.               */
            if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].intParam) && (NumberofValidResponses) && (TempParam->Params[0].intParam <= NumberofValidResponses) && (!COMPARE_BD_ADDR(InquiryResultList[(TempParam->Params[0].intParam - 1)], NullADDR)))
            {
               /* Bluetooth Device appears to be valid, so check to see */
               /* if valid Server Ports were specified.                 */
               if((TempParam->NumberofParameters > 2) && (TempParam->Params[1].intParam) && (TempParam->Params[2].intParam))
               {
                  /* Note the Server Port Numbers.                      */
                  JobServerPortValue    = TempParam->Params[1].intParam;
                  StatusServerPortValue = TempParam->Params[2].intParam;

                  BPPOpenRemotePrintServer(InquiryResultList[(TempParam->Params[0].intParam - 1)], TempParam->Params[1].intParam, jtDirectPrintingService, TempParam->Params[2].intParam);

                  /* Flag success to the caller.                        */
                  ret_val = 0;
               }
               else
               {
                  printf("Usage: OpenClient [Inquiry Index] [Job Server RFCOMM Port] [Status Server RFCOMM Port].\r\n");

                  /* One or more of the necessary parameters is/are     */
                  /* invalid.                                           */
                  ret_val = INVALID_PARAMETERS_ERROR;
               }
            }
            else
            {
               printf("Usage: OpenClient [Inquiry Index] [Job Server RFCOMM Port] [Status Server RFCOMM Port].\r\n");

               /* One or more of the necessary parameters is/are        */
               /* invalid.                                              */
               ret_val = INVALID_PARAMETERS_ERROR;
            }
         }
         else
         {
            printf("There is already a BPP Client Connection open.\r\n");

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

   /* The following function is responsible for closing currently active*/
   /* connections for the Local Sender.  This function returns zero if  */
   /* successful and a negative value if an error occurred.             */
static int CloseSenderConnections(ParameterList_t *TempParam)
{
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      if(ServiceType == stSender)
      {
         /* Make sure that all of the parameters required for this      */
         /* function appear to be at least semi-valid.                  */
         if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].intParam & 0x03))
         {
            /* Parameters appear to be valid, so attempt to close the   */
            /* specified connections.                                   */
            if(TempParam->Params[0].intParam & 0x01)
               BPPCloseConnection(BPPServerID);

            if(TempParam->Params[0].intParam & 0x02)
            {
               BPPCloseConnection(BPPClientID);

               JobServerPortValue    = 0;
               StatusServerPortValue = 0;
            }

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            printf("Usage: CloseConnections [Connection Bit Mask].\r\n\r\n where Connection Bit Mask:\r\n   1 - Server\r\n   2 - Client\r\n");

            /* One or more of the necessary parameters is/are invalid.  */
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

   /* The following function is responsible for beginning a File Push   */
   /* operation on the Local Sender.  This function returns zero if     */
   /* successful and a negative value if an error occurred.             */
static int FilePush(ParameterList_t *TempParam)
{
   int  ret_val;
   char Name[256+1];

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      if(ServiceType == stSender)
      {
         /* If a BPP Client is opened, then we need to begin the File   */
         /* Push Operation.                                             */
         if(BPPClientID)
         {
            /* Next, let's make sure that a MIME Type was specified as a*/
            /* parameter to this command.                               */
            if((TempParam) && (TempParam->NumberofParameters > 0) && (strlen(TempParam->Params[0].strParam)))
            {
               /* MIME Type specified, check to make sure a File Name   */
               /* was specified.                                        */
               if((TempParam->NumberofParameters > 1) && (strlen(TempParam->Params[1].strParam)))
               {
                  /* We need to parse out the Path and File Name        */
                  /* information from the string that was specified.    */
                  ParseFileName(TempParam->Params[1].strParam, Name);

                  BPPFilePushRequest(TempParam->Params[0].strParam, TempParam->Params[1].strParam, Name, FILE_PUSH_DESCRIPTION);

                  /* Flag success to the caller.                        */
                  ret_val = 0;
               }
               else
               {
                  printf("Usage: FilePush [MIME Type] [File Name].\r\n");

                  /* One or more of the necessary parameters is/are     */
                  /* invalid.                                           */
                  ret_val = INVALID_PARAMETERS_ERROR;
               }
            }
            else
            {
               printf("Usage: FilePush [MIME Type] [File Name].\r\n");

               /* One or more of the necessary parameters is/are        */
               /* invalid.                                              */
               ret_val = INVALID_PARAMETERS_ERROR;
            }
         }
         else
         {
            printf("There is NO BPP Client Connection currently open.\r\n");

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

   /* The following function is responsible for issuing a Get Printer   */
   /* Attributes Request.  This function returns zero if successful and */
   /* a negative value if an error occurred.                            */
static int GetPrinterAttributesRequest(ParameterList_t *TempParam)
{
   int                      ret_val;
   unsigned char           *Buffer;
   unsigned int             NumberDataElements;
   BPP_SOAP_Data_Element_t *DataElementBuffer;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      if(ServiceType == stSender)
      {
         /* If a BPP Client is opened, then we need to issue a Get      */
         /* Printer Attributes Request.                                 */
         if(BPPClientID)
         {
            /* Next, let's make sure that an RFCOMM Port was specified  */
            /* as a parameter to this command.                          */
            if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].intParam))
            {
               /* Check to make sure a File Name was specified.         */
               if((TempParam->NumberofParameters > 1) && (strlen(TempParam->Params[1].strParam)))
               {
                  /* The parameters appear to be at least semi-valid.   */
                  /* Next attempt to build the SOAP Data Elements from  */
                  /* the XML File Data.                                 */
                  if(BuildSOAPDataElementsFromXMLFileData(TempParam->Params[1].strParam, &Buffer, &NumberDataElements, &DataElementBuffer) == 0)
                  {
                     /* The SOAP Data Elements were successfully built. */
                     /* Attempt to submit the Get Printer Attributes    */
                     /* Request.                                        */
                     BPPGetPrinterAttributesRequest(TempParam->Params[0].intParam, NumberDataElements, DataElementBuffer);

                     /* All through with the SOAP Data Elements, free   */
                     /* the memory.                                     */
                     FreeSOAPDataElements(Buffer, NumberDataElements, DataElementBuffer);

                     /* Flag success to the caller.                     */
                     ret_val = 0;
                  }
                  else
                  {
                     printf("Unable to Build SOAP Data Elements From XML File: %s\r\n.", TempParam->Params[1].strParam);

                     ret_val = INVALID_PARAMETERS_ERROR;
                  }
               }
               else
               {
                  printf("Usage: GetPrinterAttrRequest [RFCOMM Port] [XML Data Element File].\r\n");

                  /* One or more of the necessary parameters is/are     */
                  /* invalid.                                           */
                  ret_val = INVALID_PARAMETERS_ERROR;
               }
            }
            else
            {
               printf("Usage: GetPrinterAttrRequest [RFCOMM Port] [XML Data Element File].\r\n");

               /* One or more of the necessary parameters is/are        */
               /* invalid.                                              */
               ret_val = INVALID_PARAMETERS_ERROR;
            }
         }
         else
         {
            printf("There is NO BPP Client Connection currently open.\r\n");

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

   /* The following function is responsible for issuing a Create Job    */
   /* Request.  This function returns zero if successful and a negative */
   /* value if an error occurred.                                       */
static int CreateJobRequest(ParameterList_t *TempParam)
{
   int                      ret_val;
   unsigned char           *Buffer;
   unsigned int             NumberDataElements;
   BPP_SOAP_Data_Element_t *DataElementBuffer;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      if(ServiceType == stSender)
      {
         /* If a BPP Client is opened, then we need to issue the Create */
         /* Job Request.                                                */
         if(BPPClientID)
         {
            /* Next, let's make sure that a File Name was specified.    */
            if((TempParam) && (TempParam->NumberofParameters > 0) && (strlen(TempParam->Params[0].strParam)))
            {
               /* The parameters appear to be at least semi-valid.  Next*/
               /* attempt to build the SOAP Data Elements from the XML  */
               /* File Data.                                            */
               if(BuildSOAPDataElementsFromXMLFileData(TempParam->Params[0].strParam, &Buffer, &NumberDataElements, &DataElementBuffer) == 0)
               {
                  /* The SOAP Data Elements were successfully built.    */
                  /* Attempt to submit the Create Job Request.          */
                  BPPCreateJobRequest(NumberDataElements, DataElementBuffer);

                  /* All through with the SOAP Data Elements, free the  */
                  /* memory.                                            */
                  FreeSOAPDataElements(Buffer, NumberDataElements, DataElementBuffer);

                  /* Flag success to the caller.                        */
                  ret_val = 0;
               }
               else
               {
                  printf("Unable to Build SOAP Data Elements From XML File: %s.\r\n", TempParam->Params[0].strParam);

                  ret_val = INVALID_PARAMETERS_ERROR;
               }
            }
            else
            {
               printf("Usage: CreateJobRequest [XML Data Element File]\r\n");

               /* One or more of the necessary parameters is/are        */
               /* invalid.                                              */
               ret_val = INVALID_PARAMETERS_ERROR;
            }
         }
         else
         {
            printf("There is NO BPP Server Connection currently open.\r\n");

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

   /* The following function is responsible for issuing a Get Job       */
   /* Attributes Request.  This function returns zero if successful and */
   /* a negative value if an error occurred.                            */
static int GetJobAttributesRequest(ParameterList_t *TempParam)
{
   int                      ret_val;
   unsigned char           *Buffer;
   unsigned int             NumberDataElements;
   BPP_SOAP_Data_Element_t *DataElementBuffer;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      if(ServiceType == stSender)
      {
         /* If a BPP Client is opened, then we need to issue a Get Job  */
         /* Attributes Request.                                         */
         if(BPPClientID)
         {
            /* Next, let's make sure that an RFCOMM Port was specified  */
            /* as a parameter to this command.                          */
            if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].intParam))
            {
               /* Check to make sure a File Name was specified.         */
               if((TempParam->NumberofParameters > 1) && (strlen(TempParam->Params[1].strParam)))
               {
                  /* The parameters appear to be at least semi-valid.   */
                  /* Next attempt to build the SOAP Data Elements from  */
                  /* the XML File Data.                                 */
                  if(BuildSOAPDataElementsFromXMLFileData(TempParam->Params[1].strParam, &Buffer, &NumberDataElements, &DataElementBuffer) == 0)
                  {
                     /* The SOAP Data Elements were successfully built. */
                     /* Attempt to submit the Get Job Attributes        */
                     /* Request.                                        */
                     BPPGetJobAttributesRequest(TempParam->Params[0].intParam, NumberDataElements, DataElementBuffer);

                     /* All through with the SOAP Data Elements, free   */
                     /* the memory.                                     */
                     FreeSOAPDataElements(Buffer, NumberDataElements, DataElementBuffer);

                     /* Flag success to the caller.                     */
                     ret_val = 0;
                  }
                  else
                  {
                     printf("Unable to Build SOAP Data Elements From XML File: %s\r\n.", TempParam->Params[1].strParam);

                     ret_val = INVALID_PARAMETERS_ERROR;
                  }
               }
               else
               {
                  printf("Usage: GetJobAttrRequest [RFCOMM Port] [XML Data Element File].\r\n");

                  /* One or more of the necessary parameters is/are     */
                  /* invalid.                                           */
                  ret_val = INVALID_PARAMETERS_ERROR;
               }
            }
            else
            {
               printf("Usage: GetJobAttrRequest [RFCOMM Port] [XML Data Element File].\r\n");

               /* One or more of the necessary parameters is/are        */
               /* invalid.                                              */
               ret_val = INVALID_PARAMETERS_ERROR;
            }
         }
         else
         {
            printf("There is NO BPP Client Connection currently open.\r\n");

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

   /* The following function is responsible for issuing a Cancel Job    */
   /* Request.  This function returns zero if successful and a negative */
   /* value if an error occurred.                                       */
static int CancelJobRequest(ParameterList_t *TempParam)
{
   int                      ret_val;
   unsigned char           *Buffer;
   unsigned int             NumberDataElements;
   BPP_SOAP_Data_Element_t *DataElementBuffer;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      if(ServiceType == stSender)
      {
         /* If a BPP Client is opened, then we need to issue a Cancel   */
         /* Job Request.                                                */
         if(BPPClientID)
         {
            /* Next, let's make sure that an RFCOMM Port was specified  */
            /* as a parameter to this command.                          */
            if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].intParam))
            {
               /* Check to make sure a File Name was specified.         */
               if((TempParam->NumberofParameters > 1) && (strlen(TempParam->Params[1].strParam)))
               {
                  /* The parameters appear to be at least semi-valid.   */
                  /* Next attempt to build the SOAP Data Elements from  */
                  /* the XML File Data.                                 */
                  if(BuildSOAPDataElementsFromXMLFileData(TempParam->Params[1].strParam, &Buffer, &NumberDataElements, &DataElementBuffer) == 0)
                  {
                     /* The SOAP Data Elements were successfully built. */
                     /* Attempt to submit the Cancel Job Request.       */
                     BPPCancelJobRequest(TempParam->Params[0].intParam, NumberDataElements, DataElementBuffer);

                     /* All through with the SOAP Data Elements, free   */
                     /* the memory.                                     */
                     FreeSOAPDataElements(Buffer, NumberDataElements, DataElementBuffer);

                     /* Flag success to the caller.                     */
                     ret_val = 0;
                  }
                  else
                  {
                     printf("Unable to Build SOAP Data Elements From XML File: %s\r\n.", TempParam->Params[1].strParam);

                     ret_val = INVALID_PARAMETERS_ERROR;
                  }
               }
               else
               {
                  printf("Usage: CancelJobRequest [RFCOMM Port] [XML Data Element File].\r\n");

                  /* One or more of the necessary parameters is/are     */
                  /* invalid.                                           */
                  ret_val = INVALID_PARAMETERS_ERROR;
               }
            }
            else
            {
               printf("Usage: CancelJobRequest [RFCOMM Port] [XML Data Element File].\r\n");

               /* One or more of the necessary parameters is/are        */
               /* invalid.                                              */
               ret_val = INVALID_PARAMETERS_ERROR;
            }
         }
         else
         {
            printf("There is NO BPP Client Connection currently open.\r\n");

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

   /* The following function is responsible for issuing a Get Event     */
   /* Request.  This function returns zero if successful and a negative */
   /* value if an error occurred.                                       */
static int GetEventRequest(ParameterList_t *TempParam)
{
   int                      ret_val;
   unsigned char           *Buffer;
   unsigned int             NumberDataElements;
   BPP_SOAP_Data_Element_t *DataElementBuffer;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      if(ServiceType == stSender)
      {
         /* If a BPP Client is opened, then we need to issue the Get    */
         /* Event Request.                                              */
         if(BPPClientID)
         {
            /* Next, let's make sure that a File Name was specified.    */
            if((TempParam) && (TempParam->NumberofParameters > 0) && (strlen(TempParam->Params[0].strParam)))
            {
               /* The parameters appear to be at least semi-valid.  Next*/
               /* attempt to build the SOAP Data Elements from the XML  */
               /* File Data.                                            */
               if(BuildSOAPDataElementsFromXMLFileData(TempParam->Params[0].strParam, &Buffer, &NumberDataElements, &DataElementBuffer) == 0)
               {
                  /* The SOAP Data Elements were successfully built.    */
                  /* Attempt to submit the Get Event Request.           */
                  BPPGetEventRequest(NumberDataElements, DataElementBuffer);

                  /* All through with the SOAP Data Elements, free the  */
                  /* memory.                                            */
                  FreeSOAPDataElements(Buffer, NumberDataElements, DataElementBuffer);

                  /* Flag success to the caller.                        */
                  ret_val = 0;
               }
               else
               {
                  printf("Unable to Build SOAP Data Elements From XML File: %s.\r\n", TempParam->Params[0].strParam);

                  ret_val = INVALID_PARAMETERS_ERROR;
               }
            }
            else
            {
               printf("Usage: GetEventRequest [XML Data Element File]\r\n");

               /* One or more of the necessary parameters is/are        */
               /* invalid.                                              */
               ret_val = INVALID_PARAMETERS_ERROR;
            }
         }
         else
         {
            printf("There is NO BPP Server Connection currently open.\r\n");

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

   /* The following function is responsible for beginning a Send        */
   /* Document operation on the Local Sender.  This function returns    */
   /* zero if successful and a negative value if an error occurred.     */
static int SendDocument(ParameterList_t *TempParam)
{
   int  ret_val;
   char Name[256+1];

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      if(ServiceType == stSender)
      {
         /* If a BPP Client is opened, then we need to begin the Send   */
         /* Document Operation.                                         */
         if(BPPClientID)
         {
            /* First, let's make sure that the Job ID was specified as a*/
            /* parameter to this command.                               */
            if((TempParam) && (TempParam->NumberofParameters > 0))
            {
               /* Next, let's make sure that a MIME Type was specified  */
               /* as a parameter to this command.                       */
               if((TempParam->NumberofParameters > 1) && (strlen(TempParam->Params[1].strParam)))
               {
                  /* MIME Type specified, check to make sure a File Name*/
                  /* was specified.                                     */
                  if((TempParam->NumberofParameters > 2) && (strlen(TempParam->Params[2].strParam)))
                  {
                     /* We need to parse out the Path and File Name     */
                     /* information from the string that was specified. */
                     ParseFileName(TempParam->Params[2].strParam, Name);

                     BPPSendDocumentRequest(TempParam->Params[0].intParam, TempParam->Params[1].strParam, TempParam->Params[2].strParam, Name, SEND_DOCUMENT_DESCRIPTION);

                     /* Flag success to the caller.                     */
                     ret_val = 0;
                  }
                  else
                  {
                     printf("Usage: SendDocument [Job ID] [MIME Type] [File Name].\r\n");

                     /* One or more of the necessary parameters is/are  */
                     /* invalid.                                        */
                     ret_val = INVALID_PARAMETERS_ERROR;
                  }
               }
               else
               {
                  printf("Usage: SendDocument [Job ID] [MIME Type] [File Name].\r\n");

                  /* One or more of the necessary parameters is/are     */
                  /* invalid.                                           */
                  ret_val = INVALID_PARAMETERS_ERROR;
               }
            }
            else
            {
               printf("Usage: SendDocument [Job ID] [MIME Type] [File Name].\r\n");

               /* One or more of the necessary parameters is/are        */
               /* invalid.                                              */
               ret_val = INVALID_PARAMETERS_ERROR;
            }
         }
         else
         {
            printf("There is NO BPP Client Connection currently open.\r\n");

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
   /* BPP Action on a Remote BPP Server.  This function returns zero if */
   /* successful and a negative value if an error occurred.             */
static int Abort(ParameterList_t *TempParam)
{
   int ret_val;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* If a BPP Client is opened, then simply issue the Abort Request.*/
      if(BPPClientID)
      {
         /* Next, make sure that the correct parameters were specified. */
         if((TempParam) && (TempParam->NumberofParameters > 0))
         {
            /* Finally issue the Abort Request.                         */
            BPPAbortRequest(TempParam->Params[0].intParam);

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            printf("Usage: Abort [Server Port].\r\n\r\n where Port is the RFCOMM Server Port.\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         printf("There is NO BPP Client Connection currently open.\r\n");

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
      if((!ClientPortsConnected) && (!ServerPortsConnected))
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

   /* The following function is responsible for retrieving File Data    */
   /* that is for the Job Server Port.                                  */
static int JobServerPortGetFileData(void)
{
   int         ret_val;
   int         FileDescriptor;
   int         AmountRead;
   DWord_t     AmountToRead;
   struct stat FileStat;

   /* Check to see if the entire file has been read.                    */
   if((JobServerPortCurrentFileIndex < JobServerPortCurrentFileSize) || (JobServerPortFirstPhase))
   {
      /* The entire file has not been read, next check to make sure that*/
      /* is room in the buffer to read more of the file into.           */
      if(JobServerPortCurrentFileBufferIndex < sizeof(JobServerPortCurrentFileBuffer))
      {
         /* Next attempt to open the current file being read from.      */
         if((FileDescriptor = open(JobServerPortCurrentFile, (O_NONBLOCK | O_RDONLY), 0)) >= 0)
         {
            /* Check to see if this is the first phase.                 */
            if(JobServerPortFirstPhase)
            {
               /* This is the first phase of this operation, get the    */
               /* file size of this image.                              */
               if((!fstat(FileDescriptor, &FileStat)) && (S_ISREG(FileStat.st_mode) || S_ISLNK(FileStat.st_mode)))
                  JobServerPortCurrentFileSize = FileStat.st_size;
               else
                  JobServerPortCurrentFileSize = 0;

               JobServerPortFirstPhase = FALSE;
            }

            /* Seek to the current file location.                       */
            lseek(FileDescriptor, JobServerPortCurrentFileIndex, SEEK_SET);

            /* Calculate the amount to read based on the current buffer */
            /* space.                                                   */
            AmountToRead = sizeof(JobServerPortCurrentFileBuffer) - JobServerPortCurrentFileBufferIndex;

            /* Attempt to read some data from the file.                 */
            if((AmountRead = read(FileDescriptor, &JobServerPortCurrentFileBuffer[JobServerPortCurrentFileBufferIndex], AmountToRead)) != (unsigned int)(-1))
            {
               /* Some data was successfully read, adjust the current   */
               /* file buffer index by the amount read.                 */
               JobServerPortCurrentFileBufferIndex += AmountRead;
               JobServerPortCurrentFileIndex       += AmountRead;

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
   /* data to the Job Server Port Data File.                            */
static int JobServerPortPutFileData(unsigned int DataLength, unsigned char *DataBuffer)
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
      if((FileDescriptor = open(JobServerPortCurrentFile, ((JobServerPortFirstPhase?(O_TRUNC | O_CREAT):0) | O_NONBLOCK | O_WRONLY), (S_IRWXG | S_IRWXU | S_IRWXO))) >= 0)
      {
         /* Check to see if this is the first phase.                    */
         if(!JobServerPortFirstPhase)
         {
            /* Seek to the end of the file.                             */
            lseek(FileDescriptor, 0, SEEK_END);
         }

         /* Indicate that this is not longer the first phase.           */
         JobServerPortFirstPhase = FALSE;

         /* Attempt to write the data to the file.                      */
         if((BytesWritten = write(FileDescriptor, DataBuffer, DataLength)) != (unsigned int)(-1))
         {
            /* Adjust the amount written.                               */
            JobServerPortCurrentFileIndex += BytesWritten;

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

   /* The following function is responsible for retrieving File Data    */
   /* that is for the Referenced Object Port.                           */
static int ReferencedObjectServerPortGetFileData(void)
{
   int     ret_val;
   int     FileDescriptor;
   int     AmountRead;
   DWord_t AmountToRead;

   /* Check to see if the entire amount to be read has been read.       */
   if((ReferencedObjectsServerPortCurrentFileIndex - ReferencedObjectsServerPortCurrentFileStartIndex) < ReferencedObjectsServerPortCurrentFileLengthToSend)
   {
      /* The entire amount to be read has not been read, next check to  */
      /* make sure that is room in the buffer to read more of the file  */
      /* into.                                                          */
      if(ReferencedObjectsServerPortCurrentFileBufferIndex < sizeof(ReferencedObjectsServerPortCurrentFileBuffer))
      {
         /* Next attempt to open the current file being read from.      */
         if((FileDescriptor = open(ReferencedObjectsServerPortCurrentFile, (O_NONBLOCK | O_RDONLY), 0)) >= 0)
         {
            /* Seek to the current file location.                       */
            lseek(FileDescriptor, ReferencedObjectsServerPortCurrentFileIndex, SEEK_SET);

            /* Calculate the amount to read based on the current buffer */
            /* space and the amount left to read..                      */
            AmountToRead = ReferencedObjectsServerPortCurrentFileLengthToSend - ReferencedObjectsServerPortCurrentFileIndex;
            if(AmountToRead > (sizeof(ReferencedObjectsServerPortCurrentFileBuffer) - ReferencedObjectsServerPortCurrentFileBufferIndex))
               AmountToRead = (sizeof(ReferencedObjectsServerPortCurrentFileBuffer) - ReferencedObjectsServerPortCurrentFileBufferIndex);

            /* Attempt to read some data from the file.                 */
            if((AmountRead = read(FileDescriptor, &ReferencedObjectsServerPortCurrentFileBuffer[JobServerPortCurrentFileBufferIndex], AmountToRead)) != (unsigned int)(-1))
            {
               /* Some data was successfully read, adjust the current   */
               /* file buffer index by the amount read.                 */
               ReferencedObjectsServerPortCurrentFileBufferIndex += AmountRead;
               ReferencedObjectsServerPortCurrentFileIndex       += AmountRead;

               /* Set the return value to indicate success.             */
               ret_val = FILE_DATA_FILE_IO_SUCCESS;
            }
            else
               ret_val = FILE_DATA_FILE_IO_ERROR;

            /* Close the descriptor of the previously open file.        */
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
   /* data to the Referenced Objects Server Port Data File.             */
static int ReferencedObjectServerPortPutFileData(unsigned int DataLength, unsigned char *DataBuffer)
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
      if((FileDescriptor = open(ReferencedObjectsServerPortCurrentFile, ((ReferencedObjectsServerPortFirstPhase?(O_TRUNC | O_CREAT):0) | O_NONBLOCK | O_WRONLY), (S_IRWXG | S_IRWXU | S_IRWXO))) >= 0)
      {
         /* Check to see if this is the first phase.                    */
         if(!ReferencedObjectsServerPortFirstPhase)
         {
            /* Seek to the end of the file.                             */
            lseek(FileDescriptor, 0, SEEK_END);
         }

         /* Indicate that this is not longer the first phase.           */
         ReferencedObjectsServerPortFirstPhase = FALSE;

         /* Attempt to write the data to the file.                      */
         if((BytesWritten = write(FileDescriptor, DataBuffer, DataLength)) != (unsigned int)(-1))
         {
            /* Adjust the amount written.                               */
            ReferencedObjectsServerPortCurrentFileIndex += BytesWritten;

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

   /* The following function is responsible for determining the File    */
   /* Size of the Referenced Objects Server Port File.                  */
static unsigned long GetReferencedObjectServerPortFileSize(void)
{
   struct stat   FileStat;
   unsigned long ret_val = 0;

   /* Simply attempt to fetch the File Properties and return the size.  */
   if((!lstat(ReferencedObjectsServerPortCurrentFile, &FileStat)) && (S_ISREG(FileStat.st_mode) || S_ISLNK(FileStat.st_mode)))
      ret_val = FileStat.st_size;

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

      /* If the very last character is a path delimiter then we will    */
      /* skip it.                                                       */
      if(PathFileName[Index] == '/')
         PathFileName[Index--] = '\0';

      /* Now move backward in the string until we either run out of     */
      /* string or we find a path delimiter.                            */
      while((Index >= 0) && (PathFileName[Index] != '/'))
         Index--;

      /* Check to see if just a filename was given (no path             */
      /* information).                                                  */
      if((Index < 0) && (strlen(PathFileName)))
         strcpy(FileName, PathFileName);
      else
      {
         /* Check to see if Path delimiter was found.                   */
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

   /* The following function is a utility function that exists to build */
   /* (and return) a Temporary File Name based on the input Path and    */
   /* Prefix information.                                               */
static unsigned int GetTempFileName(char *PathName, char *PrefixString, char *TempFileName)
{
   unsigned int ret_val = 0;

   if((PathName) && (strlen(PathName)) && (PrefixString) && (TempFileName))
   {
      sprintf(TempFileName, "%s/%s%04X", PathName, PrefixString, 1);

      ret_val = 1;
   }

   return(ret_val);
}

   /* The following function is responsible for extracting the Job ID   */
   /* from the specified SOAP Elements.                                 */
static int GetJobId(unsigned int NumberDataElements, BPP_SOAP_Data_Element_t *DataElementBuffer, DWord_t *JobId)
{
   int          ret_val;
   char         TempBuffer[32];
   unsigned int Length;
   unsigned int Index;

   /* Check to make sure that the parameters passed in appear to be at  */
   /* least semi-valid.                                                 */
   if((NumberDataElements > 0) && (DataElementBuffer != NULL) && (JobId != NULL))
   {
      /* The parameters passed in appear to be at least semi-valid.     */
      /* Initialize the Job Id to an invalid value, and the return value*/
      /* to indicate an error.                                          */
      (*JobId) = BPP_JOB_ID_INVALID_VALUE;
      ret_val  = GET_JOB_ID_ERROR;

      /* Loop through each of the SOAP data elements.                   */
      for(Index=0;((Index<NumberDataElements)&&((*JobId)==BPP_JOB_ID_INVALID_VALUE));Index++)
      {
         /* Next determine the type of this SOAP Data Element.          */
         switch(DataElementBuffer[Index].ValueType)
         {
            case stTextString:
               /* This SOAP Data Element is a text string.  Check the   */
               /* Name to see if this is the Job Id SOAP Data Element.  */
               if((strlen(JobIdElementNameValue) == DataElementBuffer[Index].NameLength) && (BTPS_MemCompareI(DataElementBuffer[Index].Name, (char *)JobIdElementNameValue, DataElementBuffer[Index].NameLength) == 0))
               {
                  /* The Name matches the Job Id Element Name Value.    */
                  /* Convert the Job Id Value Data String to a Job Id   */
                  /* Value.                                             */
                  Length = DataElementBuffer[Index].ValueLength;
                  if(Length > sizeof(TempBuffer))
                     Length = sizeof(TempBuffer);

                  strncpy(TempBuffer, (char *)DataElementBuffer[Index].ValueData.TextString, Length);
                  TempBuffer[sizeof(TempBuffer)-1] = '\0';

                  (*JobId) = strtol(TempBuffer, NULL, 10);
               }
               break;
            case stSequence:
               /* This SOAP Data Element is a sequence.  In this case we*/
               /* need to search in these elements for the Job Id.  Do  */
               /* this by calling this function recursively.            */
               GetJobId(DataElementBuffer[Index].ValueLength, DataElementBuffer[Index].ValueData.DataSequence, JobId);
               break;
         }
      }

      /* Check to see if the Job Id was successfully located.           */
      if((*JobId) != BPP_JOB_ID_INVALID_VALUE)
      {
         /* The Job Id was successfully located.  Set the return value  */
         /* to indicate success.                                        */
         ret_val = 0;
      }
   }
   else
      ret_val = GET_JOB_ID_ERROR;

   return(ret_val);
}

   /* The following function is a utility function that exists to       */
   /* retrieve the next usable SOAP Data Element Entry ID.              */
static unsigned int GetNextSOAPDataElementID(void)
{
   static unsigned int NextSOAPDataElementID = 0;

   /* Increment the Counter to the next number.  Check the new number to*/
   /* see if it has gone negative (when ID is viewed as a signed        */
   /* integer).  If so, return to the first valid Number (one).         */
   NextSOAPDataElementID++;

   if(((int)NextSOAPDataElementID) < 0)
      NextSOAPDataElementID = 1;

   /* Simply return the BPP SOAP Data Element ID Number to the caller.  */
   return(NextSOAPDataElementID);
}

   /* The following function is a utility function that exists to add a */
   /* specified SOAP Data Element Entry to the specified SOAP Data      */
   /* Element List.                                                     */
static BPPSOAPDataElementEntry_t *AddBPPSOAPDataElementEntry(BPPSOAPDataElementEntry_t **ListHead, BPPSOAPDataElementEntry_t *EntryToAdd)
{
   BPPSOAPDataElementEntry_t *AddedEntry = NULL;
   BPPSOAPDataElementEntry_t *tmpEntry;

   /* First let's verify that values passed in are semi-valid.          */
   if((ListHead != NULL) && (EntryToAdd != NULL))
   {
      /* Make sure that the element that we are adding seems semi-valid.*/
      if(EntryToAdd->SOAPDataElementID)
      {
         /* OK, data seems semi-valid, let's allocate a new data        */
         /* structure to add to the list.                               */
         AddedEntry = (BPPSOAPDataElementEntry_t *)malloc(sizeof(BPPSOAPDataElementEntry_t));

         if(AddedEntry)
         {
            /* Copy All Data over.                                      */
            *AddedEntry                                = *EntryToAdd;

            /* Now Add it to the end of the list.                       */
            AddedEntry->NextBPPSOAPDataElementEntryPtr = NULL;

            /* First, let's check to see if there are any elements      */
            /* already present in the List that was passed in.          */
            if((tmpEntry = *ListHead) != NULL)
            {
               /* Head Pointer was not NULL, so we will traverse the    */
               /* list until we reach the last element.                 */
               while(tmpEntry)
               {
                  if(tmpEntry->SOAPDataElementID == AddedEntry->SOAPDataElementID)
                  {
                     /* Entry was already added, so free the memory and */
                     /* flag an error to the caller.                    */
                     FreeBPPSOAPDataElementEntryMemory(AddedEntry);
                     AddedEntry = NULL;

                     /* Abort the Search.                               */
                     tmpEntry   = NULL;
                  }
                  else
                  {
                     /* OK, we need to see if we are at the last element*/
                     /* of the List.  If we are, we simply break out of */
                     /* the list traversal because we know there are NO */
                     /* duplicates AND we are at the end of the list.   */
                     if(tmpEntry->NextBPPSOAPDataElementEntryPtr)
                        tmpEntry = tmpEntry->NextBPPSOAPDataElementEntryPtr;
                     else
                        break;
                  }
               }

               if(AddedEntry)
               {
                  /* Last element found, simply Add the entry.          */
                  tmpEntry->NextBPPSOAPDataElementEntryPtr = AddedEntry;
               }
            }
            else
               *ListHead = AddedEntry;
         }
      }
   }

   return(AddedEntry);
}

   /* The following function is a utility function that exists to search*/
   /* for (and return) a SOAP Data Element in the specified SOAP Data   */
   /* Element List that has the corresponding SOAP Data Element ID.     */
static BPPSOAPDataElementEntry_t *SearchBPPSOAPDataElementEntry(BPPSOAPDataElementEntry_t **ListHead, unsigned int SOAPDataElementID)
{
   BPPSOAPDataElementEntry_t *FoundEntry = NULL;

   /* Let's make sure the list and Basic Printing Profile SOAP Data     */
   /* Element ID to search for appear to be valid.                      */
   if((ListHead != NULL) && (SOAPDataElementID > 0))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->SOAPDataElementID != SOAPDataElementID))
         FoundEntry = FoundEntry->NextBPPSOAPDataElementEntryPtr;
   }

   return(FoundEntry);
}

   /* The following function is a utility function that exists to delete*/
   /* a SOAP Data Element from the specified SOAP Data Element List by  */
   /* the specified Parent ID.                                          */
static BPPSOAPDataElementEntry_t *DeleteBPPSOAPDataElementEntryByParentID(BPPSOAPDataElementEntry_t **ListHead, unsigned int SOAPDataElementParentID)
{
   BPPSOAPDataElementEntry_t *FoundEntry = NULL;
   BPPSOAPDataElementEntry_t *LastEntry  = NULL;

   /* Let's make sure the List to search for appear to be semi-valid.   */
   if(ListHead != NULL)
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->SOAPDataElementParentID != SOAPDataElementParentID))
      {
         LastEntry  = FoundEntry;
         FoundEntry = FoundEntry->NextBPPSOAPDataElementEntryPtr;
      }

      /* Check to see if we found the specified entry.                  */
      if(FoundEntry)
      {
         /* OK, now let's remove the entry from the list.  We have to   */
         /* check to see if the entry was the first entry in the list.  */
         if(LastEntry)
         {
            /* Entry was NOT the first entry in the list.               */
            LastEntry->NextBPPSOAPDataElementEntryPtr = FoundEntry->NextBPPSOAPDataElementEntryPtr;
         }
         else
            *ListHead = FoundEntry->NextBPPSOAPDataElementEntryPtr;

         FoundEntry->NextBPPSOAPDataElementEntryPtr = NULL;
      }
   }

   return(FoundEntry);
}

   /* The following function is a utility function that exists to count */
   /* the number SOAP Data Elements that have the specified Parent ID.  */
static unsigned int CountBPPSOAPDataElementEntryByParentID(BPPSOAPDataElementEntry_t **ListHead, unsigned int SOAPDataElementParentID)
{
   unsigned int               Count = 0;
   BPPSOAPDataElementEntry_t *CurrentEntry;

   /* Let's make sure the List to search for appear to be semi-valid.   */
   if(ListHead != NULL)
   {
      /* Now, let's search the list until we find the number of entries.*/
      CurrentEntry = *ListHead;

      while(CurrentEntry != NULL)
      {
         if(CurrentEntry->SOAPDataElementParentID == SOAPDataElementParentID)
            Count++;

         CurrentEntry = CurrentEntry->NextBPPSOAPDataElementEntryPtr;
      }
   }

   return(Count);
}

   /* The following function is a utility function that exists to free  */
   /* the specified SOAP Element Entry.                                 */
static void FreeBPPSOAPDataElementEntryMemory(BPPSOAPDataElementEntry_t *EntryToFree)
{
   if(EntryToFree)
      free(EntryToFree);
}

   /* The following function is a utility function that exists to free  */
   /* the specified SOAP Data Element List (and free all resources it   */
   /* occupies).                                                        */
static void FreeBPPSOAPDataElementList(BPPSOAPDataElementEntry_t **ListHead)
{
   BPPSOAPDataElementEntry_t *EntryToFree;
   BPPSOAPDataElementEntry_t *tmpEntry;

   if(ListHead)
   {
      /* Simply traverse the list and free every element present.       */
      EntryToFree = *ListHead;

      while(EntryToFree)
      {
         tmpEntry    = EntryToFree;
         EntryToFree = EntryToFree->NextBPPSOAPDataElementEntryPtr;

         FreeBPPSOAPDataElementEntryMemory(tmpEntry);
      }

      /* Make sure the List appears to be empty.                        */
      *ListHead = NULL;
   }
}

   /* The following function is a utility function that exists to parse */
   /* the specified XML File into a SOAP Data Element List.             */
static int BuildSOAPDataElementsFromXMLFileData(char *Path, unsigned char **Buffer, unsigned int *NumberDataElements, BPP_SOAP_Data_Element_t **DataElementBuffer)
{
   int                    ret_val;
   int                    Result;
   int                    FileDescriptor;
   struct stat            FileStat;
   unsigned char         *FileData;
   unsigned long          FileLength;
   unsigned long          LengthRead;
   ProcessXMLStateInfo_t  ProcessXMLStateInfo;

   /* First check to make sure that the parameters passed in appear to  */
   /* be at least semi-valid.                                           */
   if((Path != NULL) && (strlen(Path) > 0) && (Buffer != NULL) && (NumberDataElements != NULL) && (DataElementBuffer != NULL))
   {
      /* The parameters specified appear to be at least semi-valid.     */
      /* Next attempt to open the XML file specified.                   */
      if((FileDescriptor = open(Path, (O_NONBLOCK | O_RDONLY), 0)) >= 0)
      {
         /* The file was successfully opened, next attempt to get the   */
         /* length of the file.                                         */
         if((!fstat(FileDescriptor, &FileStat)) && (S_ISREG(FileStat.st_mode) || S_ISLNK(FileStat.st_mode)))
            FileLength = FileStat.st_size;
         else
            FileLength = 0;

         if((FileLength != 0xFFFFFFFF) && (FileLength != 0))
         {
            /* Next attempt to allocate a buffer of the appropriate size*/
            /* to read the file data into.                              */
            if((FileData = (unsigned char *)malloc(FileLength)) != NULL)
            {
               /* The file data buffer was successfully allocated, next */
               /* attempt to read the file data into the buffer.        */
               if(((LengthRead = read(FileDescriptor, FileData, FileLength)) != (unsigned long)(-1)) && (LengthRead == FileLength))
               {
                  /* Next initialize the process XML state information. */
                  ProcessXMLStateInfo.CurrentSOAPDataElementID = SOAP_DATA_ELEMENT_ROOT_ID;
                  ProcessXMLStateInfo.ProcessXMLState          = psWaitingOpeningElement;
                  ProcessXMLStateInfo.OpeningElementNameLength = 0;
                  ProcessXMLStateInfo.OpeningElementName       = NULL;
                  ProcessXMLStateInfo.SOAPDataElementList      = NULL;

                  /* Now attempt to build the SOAP Data Element List by */
                  /* parsing the XML File Data.                         */
                  if(XMLP_Parse_Buffer(FileLength, FileData, XMLP_Event_Callback, (unsigned long)&ProcessXMLStateInfo) == 0)
                  {
                     /* The SOAP Stream appears to have been parsed,    */
                     /* check the Process XML State to see if it appears*/
                     /* to have been successfully processed.            */
                     if(ProcessXMLStateInfo.ProcessXMLState == psComplete)
                     {
                        /* Next attempt to build the API's SOAP Data    */
                        /* Element Buffer from the SOAP Data Element    */
                        /* List.                                        */
                        Result = BuildSOAPDataElementsFromSOAPDataElementList(&ProcessXMLStateInfo.SOAPDataElementList, SOAP_DATA_ELEMENT_ROOT_ID, NumberDataElements, DataElementBuffer);
                        if(Result == 0)
                        {
                           /* SOAP Data Elements successfully built, set*/
                           /* the return value to indicate success.     */
                           ret_val = 0;

                           /* Set the Buffer parameter to the File Data */
                           /* Buffer allocated in this function.        */
                           (*Buffer) = FileData;
                        }
                        else
                        {
                           /* Check to see if there is no child data.   */
                           if(Result == BUILD_SOAP_DATA_ELEMENTS_FROM_SOAP_DATA_ELEMENT_LIST_NO_CHILDREN)
                           {
                              /* There is no child data to be built.    */
                              /* Set the Number Data Elements to zero,  */
                              /* and the Data Element Buffer to NULL to */
                              /* indicate this.                         */
                              (*NumberDataElements) = 0;
                              (*DataElementBuffer)  = NULL;

                              /* Set the return value to indicate       */
                              /* success.                               */
                              ret_val = 0;
                           }
                           else
                           {
                              /* An error occurred while attempting to  */
                              /* build the SOAP Data Elements from the  */
                              /* SOAP Data Element List.  Set the return*/
                              /* value to indicate an error has         */
                              /* occurred.                              */
                              ret_val = BUILD_SOAP_DATA_ELEMENTS_FROM_XML_FILE_DATA;
                           }
                        }
                     }
                     else
                        ret_val = BUILD_SOAP_DATA_ELEMENTS_FROM_XML_FILE_DATA;
                  }
                  else
                     ret_val = BUILD_SOAP_DATA_ELEMENTS_FROM_XML_FILE_DATA;
               }
               else
                  ret_val = BUILD_SOAP_DATA_ELEMENTS_FROM_XML_FILE_DATA;

               /* All through with the SOAP Data Element List.  Free the*/
               /* memory associated with this list.                     */
               /* ** NOTE ** This is done even if the                   */
               /*            XMLP_Parse_Buffer() function fails do to   */
               /*            the fact that some list elements may have  */
               /*            been added be for the failure occurred.    */
               FreeBPPSOAPDataElementList(&ProcessXMLStateInfo.SOAPDataElementList);

               /* Check to see if an error occurred during the building */
               /* of the SOAP Data Elements.                            */
               if(ret_val == BUILD_SOAP_DATA_ELEMENTS_FROM_XML_FILE_DATA)
               {
                  /* An error occurred free the memory associated with  */
                  /* the File Data being processed.                     */
                  free(FileData);
               }
            }
            else
               ret_val = BUILD_SOAP_DATA_ELEMENTS_FROM_XML_FILE_DATA;
         }
         else
            ret_val = BUILD_SOAP_DATA_ELEMENTS_FROM_XML_FILE_DATA;

         /* Close the descriptor of the previously open file.           */
         close(FileDescriptor);
      }
      else
         ret_val = BUILD_SOAP_DATA_ELEMENTS_FROM_XML_FILE_DATA;
   }
   else
      ret_val = BUILD_SOAP_DATA_ELEMENTS_FROM_XML_FILE_DATA;

   return(ret_val);
}

   /* The following function is a utility function that exists to build */
   /* SOAP Data Elements from a SOAP Data Element List.                 */
static int BuildSOAPDataElementsFromSOAPDataElementList(BPPSOAPDataElementEntry_t **SOAPDataElementList, unsigned int SOAPDataElementParentID, unsigned int *NumberDataElements, BPP_SOAP_Data_Element_t **DataElementBuffer)
{
   int                        ret_val;
   int                        Result;
   unsigned int               Index;
   unsigned int               Count;
   unsigned int               NumberEntries;
   BPP_SOAP_Data_Element_t   *DataElementBufferPtr;
   BPP_SOAP_Data_Element_t   *TempDataElementBufferPtr;
   BPPSOAPDataElementEntry_t *SOAPDataElementEntryPtr;

   /* First make sure that the parameters passed in appear to be at     */
   /* least semi-valid.                                                 */
   if((SOAPDataElementList != NULL) && (NumberDataElements != NULL) && (DataElementBuffer != NULL))
   {
      /* The parameters passed in appear to be a least semi-valid.      */
      /* Initialize the parameters to a known state.                    */
      (*NumberDataElements) = 0;
      (*DataElementBuffer)  = NULL;

      /* Next determine the number of entries in the list with the      */
      /* specified Parent ID.                                           */
      if((Count = CountBPPSOAPDataElementEntryByParentID(SOAPDataElementList, SOAPDataElementParentID)) > 0)
      {
         /* Next attempt to allocate the number of entries with the     */
         /* specified Parent ID.                                        */
         if((DataElementBufferPtr = malloc((sizeof(BPP_SOAP_Data_Element_t)*Count))) != NULL)
         {
            /* The memory was successfully allocated, set the return    */
            /* value to indicate success.                               */
            ret_val = 0;

            /* Loop until each of the SOAP Data Element Entries are     */
            /* processed.                                               */
            for(Index=0;((ret_val==0)&&(Index<Count));Index++)
            {
               /* Remove SOAP Data Element entry from the list.         */
               if((SOAPDataElementEntryPtr = DeleteBPPSOAPDataElementEntryByParentID(SOAPDataElementList, SOAPDataElementParentID)) != NULL)
               {
                  /* Set the Data Element Entry Buffers Name information*/
                  /* to the Name information located in the SOAP Data   */
                  /* Element Entry.                                     */
                  DataElementBufferPtr[Index].NameLength = SOAPDataElementEntryPtr->NameLength;
                  DataElementBufferPtr[Index].Name       = SOAPDataElementEntryPtr->Name;

                  /* Next check to see if this SOAP Data Element Entry  */
                  /* has Value information associated with it.          */
                  if((SOAPDataElementEntryPtr->ValueLength > 0) && (SOAPDataElementEntryPtr->Value != NULL))
                  {
                     /* This SOAP Data Element Entry has Value          */
                     /* information associated with it.  Set the Value  */
                     /* information in the Data Element Entry Buffer and*/
                     /* set this Data Element Buffer Entry Value Type to*/
                     /* be a Text String Type.                          */
                     DataElementBufferPtr[Index].ValueType            = stTextString;
                     DataElementBufferPtr[Index].ValueLength          = SOAPDataElementEntryPtr->ValueLength;
                     DataElementBufferPtr[Index].ValueData.TextString = SOAPDataElementEntryPtr->Value;
                  }
                  else
                  {
                     /* This SOAP Data Element Entry does not have value*/
                     /* information, therefore this Data Element Buffer */
                     /* Entry Value Type must be a Sequence type or a   */
                     /* text string with no value data.                 */
                     NumberEntries            = 0;
                     TempDataElementBufferPtr = NULL;
                     Result = BuildSOAPDataElementsFromSOAPDataElementList(SOAPDataElementList, SOAPDataElementEntryPtr->SOAPDataElementID, &NumberEntries, &TempDataElementBufferPtr);
                     if(Result == 0)
                     {
                        /* The Sequence was successfully built, set the */
                        /* Data Element Buffer Entry Value Type to be a */
                        /* Sequence and populate the associated entries */
                        /* appropriately.                               */
                        DataElementBufferPtr[Index].ValueType              = stSequence;
                        DataElementBufferPtr[Index].ValueLength            = NumberEntries;
                        DataElementBufferPtr[Index].ValueData.DataSequence = TempDataElementBufferPtr;
                     }
                     else
                     {
                        /* Check to see if this is a text string type   */
                        /* element with no value data.                  */
                        if(Result == BUILD_SOAP_DATA_ELEMENTS_FROM_SOAP_DATA_ELEMENT_LIST_NO_CHILDREN)
                        {
                           /* This is a test string type element with   */
                           /* not value data.  Set the Value information*/
                           /* in the Data Element Entry Buffer to NULL  */
                           /* and set this Data Element Buffer Entry    */
                           /* Value Type to be a Text String Type.      */
                           DataElementBufferPtr[Index].ValueType            = stTextString;
                           DataElementBufferPtr[Index].ValueLength          = 0;
                           DataElementBufferPtr[Index].ValueData.TextString = NULL;
                        }
                        else
                        {
                           /* An error occurred while attempting to     */
                           /* build the sequence.  Set the return value */
                           /* to indicate that an error occurred.       */
                           ret_val = BUILD_SOAP_DATA_ELEMENTS_FROM_SOAP_DATA_ELEMENT_LIST_ERROR;
                        }
                     }
                  }

                  /* All through with the SOAP Data Element Entry.  Free*/
                  /* the memory that is associated with it.             */
                  FreeBPPSOAPDataElementEntryMemory(SOAPDataElementEntryPtr);
               }
               else
               {
                  /* An error occurred while attempting to remove the   */
                  /* the SOAP Data Element Entry from the list.  Set the*/
                  /* return value to indicates that an error occurred.  */
                  ret_val = BUILD_SOAP_DATA_ELEMENTS_FROM_SOAP_DATA_ELEMENT_LIST_ERROR;
               }
            }

            /* Check to see if an error occurred while processing the   */
            /* SOAP Data Element Entries.                               */
            if(ret_val != 0)
            {
               /* An error occurred while attempting to build the Data  */
               /* Element Entry Buffer.  Free any previously allocated  */
               /* resources.                                            */
               free(DataElementBufferPtr);
            }
            else
            {
               /* The Data Element Entry Buffer was successfully built. */
               /* Set the passed in parameters with the built Data      */
               /* Element Entry Buffer Information.                     */
               (*NumberDataElements) = Count;
               (*DataElementBuffer)  = DataElementBufferPtr;
            }
         }
         else
            ret_val = BUILD_SOAP_DATA_ELEMENTS_FROM_SOAP_DATA_ELEMENT_LIST_ERROR;
      }
      else
      {
         /* No entries with the specified Parent ID were located, this  */
         /* must be an empty tag.  Set the return value to indicate that*/
         /* no children of this element were located.                   */
         ret_val = BUILD_SOAP_DATA_ELEMENTS_FROM_SOAP_DATA_ELEMENT_LIST_NO_CHILDREN;
      }
   }
   else
      ret_val = BUILD_SOAP_DATA_ELEMENTS_FROM_SOAP_DATA_ELEMENT_LIST_ERROR;

   return(ret_val);
}

   /* The following function is a utility function that exists to free  */
   /* the specified SOAP Data Elements Information.                     */
static void FreeSOAPDataElements(unsigned char *Buffer, unsigned int NumberDataElements, BPP_SOAP_Data_Element_t *DataElementBuffer)
{
   unsigned int Index;

   /* Check to see if the Number Data Elements and Data Element Buffer  */
   /* parameters appear to be at least semi-valid.                      */
   if((NumberDataElements > 0) && (DataElementBuffer != NULL))
   {
      /* The parameters passed in appear to be a least semi-valid.  Next*/
      /* loop through each of the data elements.                        */
      for(Index=0;Index<NumberDataElements;Index++)
      {
         /* Check to see if this entry a sequence type.                 */
         if(DataElementBuffer[Index].ValueType == stSequence)
         {
            /* This entry is a sequence type entry, in this case we need*/
            /* to free its SOAP Data Elements as well.  Do this by      */
            /* recursively calling the FreeSOAPDataElements() function. */
            FreeSOAPDataElements(NULL, DataElementBuffer[Index].ValueLength, DataElementBuffer[Index].ValueData.DataSequence);
         }
      }

      /* Finally, free the memory associated with this Data Element     */
      /* Buffer.                                                        */
      free(DataElementBuffer);
   }

   /* Check to see if the Buffer parameter appears to be at least       */
   /* semi-valid.                                                       */
   if(Buffer != NULL)
   {
      /* The buffer parameter appears to be at least semi-valid, free   */
      /* the resources associated with this parameter.                  */
      free(Buffer);
   }
}

   /* The following function is a utility function that exists to       */
   /* display the SOAP Data Elements that are contained in the specified*/
   /* SOAP Data Element List.                                           */
static void DisplaySOAPDataElements(unsigned int NumberDataElements, BPP_SOAP_Data_Element_t *DataElementBuffer, unsigned int InitialLevel)
{
   unsigned int Index;

   /* First make sure that the parameters passed in appear to be at     */
   /* least semi-valid.                                                 */
   if((NumberDataElements > 0) && (DataElementBuffer != NULL))
   {
      /* The parameters passed in appear to be a least semi-valid.  Next*/
      /* loop through each of the data elements.                        */
      for(Index=0;Index<NumberDataElements;Index++)
      {
         /* Display the data associated with this data element.         */
         DisplaySOAPDataElement(&DataElementBuffer[Index], InitialLevel);
      }
   }
}

   /* The following function is a utility function that exists to       */
   /* display an individual SOAP Element.                               */
static void DisplaySOAPDataElement(BPP_SOAP_Data_Element_t *DataElementBuffer, unsigned int Level)
{
   char         Buffer[512];
   unsigned int Index;

   /* First make sure that the parameters passed in appear to be at     */
   /* least semi-valid.                                                 */
   if(DataElementBuffer != NULL)
   {
      /* The parameters passed in appear to be a least semi-valid.      */
      /* print the name of this data element.                           */
      snprintf(Buffer, sizeof(Buffer), "%*sName: %.*s", Level*INDENT_LENGTH, "", DataElementBuffer->NameLength, (char *)DataElementBuffer->Name);
      Buffer[sizeof(Buffer)-1] = '\0';
      printf("%s\r\n", Buffer);

      /* Next determine this data elements value type.                  */
      switch(DataElementBuffer->ValueType)
      {
         case stTextString:
            /* This data element value type is a text string.  Check to */
            /* see if there is value data associated with this element. */
            /* It could have been an empty element.                     */
            if((DataElementBuffer->ValueLength > 0) && (DataElementBuffer->ValueData.TextString != NULL))
            {
               /* There is value data associated with this data element.*/
               /* Display the data associated with this data element.   */
               snprintf(Buffer, sizeof(Buffer), "%*sValue: %.*s", (Level+1)*INDENT_LENGTH, "", DataElementBuffer->ValueLength, DataElementBuffer->ValueData.TextString);
               Buffer[sizeof(Buffer)-1] = '\0';
               printf("%s\r\n", Buffer);
            }
            else
            {
               /* There is not value data associated with this data     */
               /* element.  Display a message indicating this.          */
               snprintf(Buffer, sizeof(Buffer), "%*sValue: (Empty)", (Level+1)*INDENT_LENGTH, "");
               Buffer[sizeof(Buffer)-1] = '\0';
               printf("%s\r\n", Buffer);
            }
            break;
         case stSequence:
            /* This data element value type is a sequence.  Loop through*/
            /* each of the data elements in this sequence.              */
            for(Index=0;Index<DataElementBuffer->ValueLength;Index++)
            {
               /* Display the data associated with this data element.   */
               DisplaySOAPDataElement(&(DataElementBuffer->ValueData.DataSequence[Index]), Level+1);
            }
            break;
      }
   }
}

   /* The following function is used to process Events that occur       */
   /* parsing XML data.                                                 */
static void BTPSAPI XMLP_Event_Callback(XMLP_Event_Data_t *XMLP_Event_Data, unsigned long CallbackParameter)
{
   unsigned int               Index;
   ProcessXMLStateInfo_t     *ProcessXMLStateInfoPtr = (ProcessXMLStateInfo_t *)CallbackParameter;
   BPPSOAPDataElementEntry_t  SOAPDataElementEntry;
   BPPSOAPDataElementEntry_t *SOAPDataElementEntryPtr;

   /* First make sure that the parameters passed in appear to be at     */
   /* least semi-valid.                                                 */
   if((XMLP_Event_Data != NULL) && (ProcessXMLStateInfoPtr != NULL))
   {
      switch(XMLP_Event_Data->Event_Data_Type)
      {
         case etXMLP_Start_Element:
            /* An XML Start Element was located, next determine the     */
            /* current Process XML State.                               */
            switch(ProcessXMLStateInfoPtr->ProcessXMLState)
            {
               case psWaitingOpeningElement:
                  /* Currently in the waiting opening element state.    */
                  /* Check to make sure that the Element Name appears to*/
                  /* be at least semi-valid.                            */
                  if((XMLP_Event_Data->Event_Data.XMLP_Start_Element_Data->ElementNameLength > 0) && (XMLP_Event_Data->Event_Data.XMLP_Start_Element_Data->ElementName != NULL))
                  {
                     /* The Element name appears to be at least         */
                     /* semi-valid.  Store the XML Name for this element*/
                     /* to the Process XML State Information for later  */
                     /* use.                                            */
                     ProcessXMLStateInfoPtr->OpeningElementNameLength = XMLP_Event_Data->Event_Data.XMLP_Start_Element_Data->ElementNameLength;
                     ProcessXMLStateInfoPtr->OpeningElementName       = XMLP_Event_Data->Event_Data.XMLP_Start_Element_Data->ElementName;

                     /* Advance the state to waiting for closing        */
                     /* element.                                        */
                     ProcessXMLStateInfoPtr->ProcessXMLState = psWaitingClosingElement;
                  }
                  break;
               case psWaitingClosingElement:
                  /* Currently in the waiting closing element state.    */
                  /* Check to make sure that the Element Name appears to*/
                  /* be at least semi-valid.                            */
                  if((XMLP_Event_Data->Event_Data.XMLP_Start_Element_Data->ElementNameLength > 0) && (XMLP_Event_Data->Event_Data.XMLP_Start_Element_Data->ElementName != NULL))
                  {
                     /* The Element name appears to be at least         */
                     /* semi-valid.  Next initialize a SOAP Data Element*/
                     /* Entry.                                          */
                     SOAPDataElementEntry.SOAPDataElementID              = GetNextSOAPDataElementID();
                     SOAPDataElementEntry.SOAPDataElementParentID        = ProcessXMLStateInfoPtr->CurrentSOAPDataElementID;
                     SOAPDataElementEntry.NameLength                     = XMLP_Event_Data->Event_Data.XMLP_Start_Element_Data->ElementNameLength;
                     SOAPDataElementEntry.Name                           = XMLP_Event_Data->Event_Data.XMLP_Start_Element_Data->ElementName;
                     SOAPDataElementEntry.ValueLength                    = 0;
                     SOAPDataElementEntry.Value                          = NULL;
                     SOAPDataElementEntry.NextBPPSOAPDataElementEntryPtr = NULL;

                     /* Next attempt to add the SOAP Data Element Entry */
                     /* to the SOAP Data Element List.                  */
                     if((SOAPDataElementEntryPtr = AddBPPSOAPDataElementEntry(&ProcessXMLStateInfoPtr->SOAPDataElementList, &SOAPDataElementEntry)) != NULL)
                     {
                        /* The SOAP Data Element Entry was successfully */
                        /* added to the SOAP Data Element List.  Set the*/
                        /* current SOAP Data Element ID to the newly    */
                        /* added SOAP Data Element ID since we want any */
                        /* new content to be children of this element.  */
                        ProcessXMLStateInfoPtr->CurrentSOAPDataElementID = SOAPDataElementEntryPtr->SOAPDataElementID;
                     }
                     else
                     {
                        /* An error occurred while attempting add the   */
                        /* SOAP Data Element Entry to the SOAP Data     */
                        /* Element List.  Set the Process SOAP Stream   */
                        /* State to indicate that an error has occurred.*/
                        ProcessXMLStateInfoPtr->ProcessXMLState = psError;
                     }
                  }
                  break;
               default:
                  /* We are not concerned with any other states.        */
                  break;
            }
            break;
         case etXMLP_End_Element:
            /* An XML End Element was located, next determine the       */
            /* current Process XML State.                               */
            switch(ProcessXMLStateInfoPtr->ProcessXMLState)
            {
               case psWaitingClosingElement:
                  /* Currently in the Waiting Closing Element State,    */
                  /* check to see if this is the closing element that   */
                  /* matches the opening element previously discovered. */
                  if((ProcessXMLStateInfoPtr->OpeningElementNameLength > 0) && (ProcessXMLStateInfoPtr->OpeningElementName != NULL) && (ProcessXMLStateInfoPtr->OpeningElementNameLength == XMLP_Event_Data->Event_Data.XMLP_End_Element_Data->ElementNameLength) && (BTPS_MemCompareI(XMLP_Event_Data->Event_Data.XMLP_End_Element_Data->ElementName, ProcessXMLStateInfoPtr->OpeningElementName, XMLP_Event_Data->Event_Data.XMLP_End_Element_Data->ElementNameLength) == 0))
                  {
                     /* This was a closing element that matched the     */
                     /* opening element, advance the state to indicate  */
                     /* the processing is completed.                    */
                     ProcessXMLStateInfoPtr->ProcessXMLState = psComplete;
                  }
                  else
                  {
                     /* This is not the closing XML Element that matches*/
                     /* the opening XML Element.  Attempt to locate the */
                     /* Current SOAP Data Element ID in the SOAP Data   */
                     /* Element List.                                   */
                     if((SOAPDataElementEntryPtr = SearchBPPSOAPDataElementEntry(&ProcessXMLStateInfoPtr->SOAPDataElementList, ProcessXMLStateInfoPtr->CurrentSOAPDataElementID)) != NULL)
                     {
                        /* The SOAP Data Element Entry was successfully */
                        /* located, next check to see if the Closing    */
                        /* Element Name matches the retrieved SOAP Data */
                        /* Element Entry.                               */
                        if((SOAPDataElementEntryPtr->NameLength == XMLP_Event_Data->Event_Data.XMLP_End_Element_Data->ElementNameLength) && (BTPS_MemCompareI(XMLP_Event_Data->Event_Data.XMLP_End_Element_Data->ElementName, SOAPDataElementEntryPtr->Name, XMLP_Event_Data->Event_Data.XMLP_End_Element_Data->ElementNameLength) == 0))
                        {
                           /* The Closing Element Name matches the      */
                           /* retrieved SOAP Data Element Entry.  Set   */
                           /* the Current SOAP Data Element ID to the   */
                           /* retrieved SOAP Data Elements Parent ID.   */
                           ProcessXMLStateInfoPtr->CurrentSOAPDataElementID = SOAPDataElementEntryPtr->SOAPDataElementParentID;
                        }
                        else
                        {
                           /* The Closing Element Name did not match the*/
                           /* retrieved SOAP Data Element Entry.  Set   */
                           /* the Process SOAP Stream State to indicate */
                           /* that an error has occurred.               */
                           ProcessXMLStateInfoPtr->ProcessXMLState = psError;
                        }
                     }
                     else
                     {
                        /* Unable to locate the Current SOAP Data       */
                        /* Element ID in the SOAP Data Element List.    */
                        /* Set the Process SOAP Stream State to indicate*/
                        /* that an error has occurred.                  */
                        ProcessXMLStateInfoPtr->ProcessXMLState = psError;
                     }
                  }
                  break;
               default:
                  /* We are not concerned with any other states.        */
                  break;
            }
            break;
         case etXMLP_Character:
            /* Some non-markup was located, check to see if we are in   */
            /* the correct state to process this type of event.         */
            if(ProcessXMLStateInfoPtr->ProcessXMLState == psWaitingClosingElement)
            {
               /* Next remove any leading white space that might exist  */
               /* as part of this character string.                     */
               Index = 0;
               while((Index < XMLP_Event_Data->Event_Data.XMLP_Character_Data->CharacterStringLength) && (IS_WHITE_SPACE_CHARACTER(XMLP_Event_Data->Event_Data.XMLP_Character_Data->CharacterString[Index])))
                  Index++;

               /* Now check to see if their is any character data after */
               /* removing the white space.                             */
               if(Index < XMLP_Event_Data->Event_Data.XMLP_Character_Data->CharacterStringLength)
               {
                  /* Their is still some character data left after      */
                  /* removing the leading white space.  Next attempt to */
                  /* locate the SOAP Data Element Entry being currently */
                  /* processed.                                         */
                  if((SOAPDataElementEntryPtr = SearchBPPSOAPDataElementEntry(&ProcessXMLStateInfoPtr->SOAPDataElementList, ProcessXMLStateInfoPtr->CurrentSOAPDataElementID)) != NULL)
                  {
                     /* The SOAP Data Element Entry being currently     */
                     /* processed was successfully located, store the   */
                     /* non-markup as the value length and value data   */
                     /* for this SOAP Data Element.                     */
                     SOAPDataElementEntryPtr->ValueLength = (XMLP_Event_Data->Event_Data.XMLP_Character_Data->CharacterStringLength - Index);
                     SOAPDataElementEntryPtr->Value       = &(XMLP_Event_Data->Event_Data.XMLP_Character_Data->CharacterString[Index]);
                  }
               }
            }
            break;
         default:
            /* Do nothing.                                              */
            break;
      }
   }
}

   /* The following function is responsible for opening a Basic Printing*/
   /* Profile Print Server.                                             */
static void BPPOpenPrintServer(unsigned int JobServerPort, unsigned long JobServerPortTargetsMask, unsigned int StatusServerPort)
{
   int           Result;
   BPP_1284_ID_t BPP1284ID;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the parameters passed to this      */
      /* function are valid.                                            */
      if((JobServerPort) && (JobServerPortTargetsMask) && (StatusServerPort))
      {
         /* Now check to make sure that a Server doesn't already exist. */
         if(!BPPServerID)
         {
            /* Now try and open a BPP Print Server.                     */
            Result = BPP_Open_Print_Server(BluetoothStackID, JobServerPort, JobServerPortTargetsMask, StatusServerPort, 0, BPP_Event_Callback, 0);

            /* Check to see if the call was executed successfully.      */
            if(Result > 0)
            {
               /* The BPP Print Server was successfully opened.  Save   */
               /* the returned result as the BPP Server ID because it   */
               /* will be used by later function calls.                 */
               BPPServerID = Result;

               printf("BPP_Open_Print_Server: Function Successful.\r\n");

               /* The Server was opened successfully, now register a SDP*/
               /* Record indicating that a Basic Printing Profile Print */
               /* Server exists.                                        */
               BPP1284ID.IEEE_1284IDLength = (Word_t)strlen((char *)Sample1284IDString);
               BPP1284ID.IEEE_1284IDString = (Byte_t *)Sample1284IDString;

               Result = BPP_Register_Print_Server_SDP_Record(BluetoothStackID, BPPServerID, "BPP Print Server", (char *)DocumentFormatsSupported, CharacterRepertoiresSupported, (char *)PrintImageFormatsSupported, &BPP1284ID, &ServiceRecordHandle);

               /* Check the result of the above function call for       */
               /* success.                                              */
               if(!Result)
               {
                  /* Display a message indicating that the SDP record   */
                  /* for the Basic Printing Profile Print Server was    */
                  /* registered successfully.                           */
                  printf("BPP_Register_Print_Server_SDP_Record: Function Successful.\r\n");
               }
               else
               {
                  /* Display an Error Message and make sure the Current */
                  /* Server SDP Handle is invalid, and close the BPP    */
                  /* Print Server we just opened because we weren't able*/
                  /* to register a SDP Record.                          */
                  printf("BPP_Register_Print_Server_SDP_Record: Function Failure.\r\n");
                  ServiceRecordHandle = 0;

                  /* Now try and close the opened Server.               */
                  Result = BPP_Close_Server(BluetoothStackID, BPPServerID);
                  BPPServerID = 0;

                  /* Next check the return value of the issued command  */
                  /* see if it was successful.                          */
                  if(!Result)
                  {
                     /* Display a message indicating that the Server was*/
                     /* successfully closed.                            */
                     printf("BPP_Close_Server: Function Successful.\r\n");
                  }
                  else
                  {
                     /* An error occurred while attempting to close the */
                     /* Server.                                         */
                     printf("BPP_Close_Server() Failure: %d.\r\n", Result);
                  }
               }
            }
            else
            {
               /* There was an error while trying to open the BPP Print */
               /* Server.                                               */
               printf("BPP_Open_Print_Server() Failure: %d.\r\n", Result);
            }
         }
         else
         {
            /* A Server is already open.                                */
            printf("Open Basic Printing Profile Print Server: Port already open.\r\n");
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("Open Basic Printing Profile Print Server: Invalid Parameter.\r\n");
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      printf("Stack ID Invalid.\r\n");
   }
}

   /* The following function is responsible for opening a Basic Printing*/
   /* Profile Sender Server.                                            */
static void BPPOpenSenderServer(unsigned int ReferencedObjectServerPort)
{
   int Result;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the parameters passed to this      */
      /* function are valid.                                            */
      if(ReferencedObjectServerPort)
      {
         /* Now check to make sure that a Server doesn't already exist. */
         if(!BPPServerID)
         {
            /* Now try and open a BPP Sender Server.                    */
            Result = BPP_Open_Sender_Server(BluetoothStackID, ReferencedObjectServerPort, BPP_Event_Callback, 0);

            /* Check to see if the call was executed successfully.      */
            if(Result > 0)
            {
               /* The BPP Sender Server was successfully opened.  Save  */
               /* the returned result as the BPP Server ID because it   */
               /* will be used by later function calls.                 */
               BPPServerID = Result;

               printf("BPP_Open_Sender_Server: Function Successful.\r\n");

               /* The Server was opened successfully, now register a SDP*/
               /* Record indicating that a Basic Printing Profile Sender*/
               /* Server exists.                                        */
               Result = BPP_Register_Sender_Server_Referenced_Objects_SDP_Record(BluetoothStackID, BPPServerID, "BPP Sender Server", &ServiceRecordHandle);

               /* Check the result of the above function call for       */
               /* success.                                              */
               if(!Result)
               {
                  /* Display a message indicating that the SDP record   */
                  /* for the Basic Printing Profile Sender Server was   */
                  /* registered successfully.                           */
                  printf("BPP_Register_Sender_Server_Referenced_Objects_SDP_Record: Function Successful.\r\n");
               }
               else
               {
                  /* Display an Error Message and make sure the Current */
                  /* Server SDP Handle is invalid, and close the BPP    */
                  /* Sender Server we just opened because we weren't    */
                  /* able to register a SDP Record.                     */
                  printf("BPP_Register_Sender_Server_Referenced_Objects_SDP_Record: Function Failure.\r\n");
                  ServiceRecordHandle = 0;

                  /* Now try and close the opened Server.               */
                  Result = BPP_Close_Server(BluetoothStackID, BPPServerID);
                  BPPServerID = 0;

                  /* Next check the return value of the issued command  */
                  /* see if it was successful.                          */
                  if(!Result)
                  {
                     /* Display a message indicating that the Server was*/
                     /* successfully closed.                            */
                     printf("BPP_Close_Server: Function Successful.\r\n");
                  }
                  else
                  {
                     /* An error occurred while attempting to close the */
                     /* Server.                                         */
                     printf("BPP_Close_Server() Failure: %d.\r\n", Result);
                  }
               }
            }
            else
            {
               /* There was an error while trying to open the BPP Sender*/
               /* Server.                                               */
               printf("BPP_Open_Sender_Server() Failure: %d.\r\n", Result);
            }
         }
         else
         {
            /* A Server is already open.                                */
            printf("Open Basic Printing Profile Sender Server: Port already open.\r\n");
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("Open Basic Printing Profile Sender Server: Invalid Parameter.\r\n");
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      printf("Stack ID Invalid.\r\n");
   }
}

   /* The following function exists to connect to a Remote Basic        */
   /* Printing Profile Print Server.                                    */
static void BPPOpenRemotePrintServer(BD_ADDR_t BD_ADDR, unsigned int JobServerPort, BPP_Job_Server_Port_Target_t JobServerPortTarget, unsigned int StatusServerPort)
{
   int       Result;
   BD_ADDR_t NullADDR;
   char      BoardStr[32];

   ASSIGN_BD_ADDR(NullADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the parameters passed to this      */
      /* function are valid.                                            */
      if((JobServerPort) && (StatusServerPort))
      {
         /* Now check to make sure that a Client doesn't already exist. */
         if(!BPPClientID)
         {
            if((!COMPARE_BD_ADDR(BD_ADDR, NullADDR)))
            {
               BD_ADDRToStr(BD_ADDR, BoardStr);

               /* The above parameters are valid, inform the user that  */
               /* the program is about to open a connection to the      */
               /* Remote Server.                                        */
               printf("Open Remote Print Server(BD_ADDR = %s, JobServerPort = %04X, StatusServerPort = %04X).\r\n", BoardStr, JobServerPort, StatusServerPort);

               Result = BPP_Open_Remote_Print_Server(BluetoothStackID, BD_ADDR, JobServerPort, JobServerPortTarget, StatusServerPort, 0, BPP_Event_Callback, 0);

               /* Now check to see if the function call was successfully*/
               /* made.                                                 */
               if(Result > 0)
               {
                  /* The Client was opened successfully.  The return    */
                  /* value of the call is the BPP Client ID and is      */
                  /* required for future Basic Printing Profile calls.  */
                  BPPClientID = Result;

                  ClientPortsConnected |= JOB_SERVER_PORT_CONNECTED_BIT;
                  if(StatusServerPort != BPP_STATUS_SERVER_PORT_INVALID_VALUE)
                     ClientPortsConnected |= STATUS_SERVER_PORT_CONNECTED_BIT;

                  printf("BPP_Open_Remote_Print_Server: Function Successful (ID = %04X).\r\n", BPPClientID);
               }
               else
               {
                  /* There was an error opening the Client.             */
                  printf("BPP_Open_Remote_Print_Server() Failure: %d.\r\n", Result);
               }
            }
            else
            {
               /* The BD_ADDR retrieved from the list is invalid.       */
               printf("Open Remote Print Server: Invalid BD_ADDR.\r\n");
            }
         }
         else
         {
            /* A Connection is already open, this program only supports */
            /* one Client at a time.                                    */
            printf("Open Remote Print Server: Port already open.\r\n");
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("Open Remote Print Server: Invalid Parameter.\r\n");
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      printf("Stack ID Invalid.\r\n");
   }
}

   /* The following function is responsible for connecting to a Remote  */
   /* Basic Printer Profile Sender Server.                              */
static void BPPOpenRemoteSenderServer(BD_ADDR_t BD_ADDR, unsigned int ReferencedObjectServerPort)
{
   int       Result;
   BD_ADDR_t NullADDR;
   char      BoardStr[32];

   ASSIGN_BD_ADDR(NullADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the parameters passed to this      */
      /* function are valid.                                            */
      if(ReferencedObjectServerPort)
      {
         /* Now check to make sure that a Client doesn't already exist. */
         if(!BPPClientID)
         {
            if((!COMPARE_BD_ADDR(BD_ADDR, NullADDR)))
            {
               BD_ADDRToStr(BD_ADDR, BoardStr);

               /* The above parameters are valid, inform the user that  */
               /* the program is about to open a connection to the      */
               /* Remote Server.                                        */
               printf("Open Remote Sender Server(BD_ADDR = %s, ReferencedObjectServerPort = %04X).\r\n", BoardStr, ReferencedObjectServerPort);

               Result = BPP_Open_Remote_Sender_Server(BluetoothStackID, BD_ADDR, ReferencedObjectServerPort, BPP_Event_Callback, 0);

               /* Now check to see if the function call was successfully*/
               /* made.                                                 */
               if(Result > 0)
               {
                  /* The Client was opened successfully.  The return    */
                  /* value of the call is the BPP Client ID and is      */
                  /* required for future Basic Printing Profile calls.  */
                  BPPClientID = Result;

                  ClientPortsConnected |= REFERENCED_OBJECT_SERVER_PORT_CONNECTED_BIT;

                  printf("BPP_Open_Remote_Sender_Server: Function Successful (ID = %04X).\r\n", BPPClientID);
               }
               else
               {
                  /* There was an error opening the Client.             */
                  printf("BPP_Open_Remote_Sender_Server() Failure: %d.\r\n", Result);
               }
            }
            else
            {
               /* The BD_ADDR retrieved from the list is invalid.       */
               printf("Open Remote Sender Server: Invalid BD_ADDR.\r\n");
            }
         }
         else
         {
            /* A Connection is already open, this program only supports */
            /* one Client at a time.                                    */
            printf("Open Remote Sender Server: Port already open.\r\n");
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("Open Remote Sender Server: Invalid Parameter.\r\n");
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      printf("Stack ID Invalid.\r\n");
   }
}

   /* The following function is responsible for closing an existing     */
   /* Basic Printing Profile Server.                                    */
static void BPPCloseServer(void)
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
         Result = BPP_Un_Register_SDP_Record(BluetoothStackID, BPPServerID, ServiceRecordHandle);

         /* Check the result of the above function call for success.    */
         if(!Result)
         {
            /* The function was called successfully.  Display a message */
            /* indicating that the SDP Record was successfully removed. */
            printf("BPP_Un_Register_SDP_Record: Function Successful.\r\n");
            ServiceRecordHandle = 0;
         }
         else
         {
            /* An error occurred while attempting to Un-Register the SDP*/
            /* Record.                                                  */
            printf("BPP_Un_Register_SDP_Record() Failure: %d.\r\n", Result);
         }
      }
      else
      {
         /* The Current Server SDP Handle is invalid so the Record must */
         /* already have been removed or never installed.               */
         printf("Invalid SDP Record Handle: BPP Close Server.\r\n");
      }

      /* Now check to see if the BPP Server ID appears to be semi-valid.*/
      /* This will indicate if a Server is open.                        */
      if(BPPServerID)
      {
         /* The Current BPPServerID appears to be valid so go ahead and */
         /* try and close the Server.                                   */
         Result = BPP_Close_Server(BluetoothStackID, BPPServerID);

         /* Check the result of the above function call for success.    */
         if(!Result)
         {
            /* The function was called successfully.  Display a message */
            /* indicating that the Server was successfully closed.      */
            printf("BPP_Close_Server: Function Successful.\r\n");

            BPPServerID          = 0;
            ServerPortsConnected = 0;

            /* Check to see if the current service type is a printer.   */
            if(ServiceType == stPrinter)
            {
               /* The current service type is printer.  Servers         */
               /* associated with this service type will use the job    */
               /* server port and the status server port.  Reset the    */
               /* current operation for these ports.                    */
               JobServerPortCurrentOperation    = coNone;
               StatusServerPortCurrentOperation = coNone;
            }
            else
            {
               /* The service type is not printer, check to see if it is*/
               /* sender.                                               */
               if(ServiceType == stSender)
               {
                  /* The current service type is sender.  Servers       */
                  /* associated with this service type will use the     */
                  /* referenced object server port.  Reset the current  */
                  /* operation for this port.                           */
                  ReferencedObjectsServerPortCurrentOperation = coNone;
               }
            }
         }
         else
         {
            /* An error occurred while attempting to close the Server.  */
            printf("BPP_Close_Server() Failure: %d.\r\n", Result);
         }
      }
      else
      {
         /* The Current BPPServerID is invalid so no Server must be     */
         /* open.                                                       */
         printf("Invalid BPPServerID: BPP Close Server.\r\n");
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      printf("Stack ID Invalid.\r\n");
   }
}

   /* The following function is responsible for closing an existing     */
   /* connection.                                                       */
static void BPPCloseConnection(unsigned int BPPID)
{
   int Result;

   /* First check to see if a valid Bluetooth Stack exists.             */
   if(BluetoothStackID)
   {
      /* Check to see if the BPPID appears to be semi-valid.  This      */
      /* parameter will only be valid if a Connection is open or a      */
      /* Server is Open.                                                */
      if(BPPID)
      {
         /* The Current BPPID appears to be semi-valid.  Now try to     */
         /* close the connection.                                       */
         Result = BPP_Close_Connection(BluetoothStackID, BPPID);

         if(!Result)
         {
            /* The function was called successfully.  Display a message */
            /* indicating that the Connection Close was successfully    */
            /* submitted.                                               */
            printf("BPP_Close_Connection: Function Successful.\r\n");

            /* Check to see if this was a call to close the BPP Client  */
            /* ID connection.                                           */
            if(BPPID == BPPClientID)
            {
               /* This was a request to close the BPP Client ID         */
               /* connection.  Reset the BPP Client ID and the Client   */
               /* Ports Connected Mask.                                 */
               BPPClientID          = 0;
               ClientPortsConnected = 0;

               /* Check to see if the current service type is a printer.*/
               if(ServiceType == stPrinter)
               {
                  /* The current service type is printer.  Client       */
                  /* connection associated with this service type will  */
                  /* use the referenced object server port.  Reset the  */
                  /* current operation for this port.                   */
                  ReferencedObjectsServerPortCurrentOperation = coNone;
               }
               else
               {
                  /* The service type is not printer, check to see if it*/
                  /* is sender.                                         */
                  if(ServiceType == stSender)
                  {
                     /* The current service type is sender.  Client     */
                     /* connection associated with this service type    */
                     /* will use the job server port and the status     */
                     /* server port.  Reset the current operation for   */
                     /* these ports.                                    */
                     JobServerPortCurrentOperation    = coNone;
                     StatusServerPortCurrentOperation = coNone;
                  }
               }
            }
            else
            {
               /* This was not a call to close the BPP Client ID        */
               /* connection, check to see if it was a call to          */
               /* disconnect the connection to the BPP Server ID.       */
               if(BPPID == BPPServerID)
               {
                  /* This was a request to close the BPP Server ID      */
                  /* connections.  Reset the Server Ports Connected     */
                  /* Mask.                                              */
                  ServerPortsConnected = 0;

                  /* Check to see if the current service type is a      */
                  /* printer.                                           */
                  if(ServiceType == stPrinter)
                  {
                     /* The current service type is printer.  Server    */
                     /* connection associated with this service type    */
                     /* will use the job server port and the status     */
                     /* server port.  Reset the current operation for   */
                     /* these ports.                                    */
                     JobServerPortCurrentOperation    = coNone;
                     StatusServerPortCurrentOperation = coNone;
                  }
                  else
                  {
                     /* The service type is not printer, check to see if*/
                     /* it is sender.                                   */
                     if(ServiceType == stSender)
                     {
                        /* The current service type is sender.  Server  */
                        /* connection associated with this service type */
                        /* will use the referenced object server port.  */
                        /* Reset the current operation for this port.   */
                        ReferencedObjectsServerPortCurrentOperation = coNone;
                     }
                  }
               }
            }
         }
         else
         {
            /* An error occurred while attempting to close the          */
            /* Connection.                                              */
            printf("BPP_Close_Connection() Failure: %d.\r\n", Result);
         }
      }
      else
      {
         /* The Current BPPID is invalid so no Connection or Server is  */
         /* open.                                                       */
         printf("Invalid BPP ID: BPP Close Connection.\r\n");
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      printf("Stack ID Invalid.\r\n");
   }
}

   /* The following function is responsible for issuing an Abort Request*/
   /* to the Remote Device.                                             */
static void BPPAbortRequest(unsigned int ServerPort)
{
   int Result;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the BPP Client ID appears to be    */
      /* semi-valid.                                                    */
      if(BPPClientID)
      {
         /* Check to make sure that the parameters passed in appear to  */
         /* be valid.                                                   */
         if(ServerPort)
         {
            /* Now submit the command to begin this operation.          */
            Result = BPP_Abort_Request(BluetoothStackID, BPPClientID, ServerPort);

            if(!Result)
            {
               /* The function was submitted successfully.              */

               if(ServerPort == JobServerPortValue)
               {
                  JobServerPortCurrentOperation = coAbort;
               }
               else
               {
                  if(ServerPort == StatusServerPortValue)
                  {
                     StatusServerPortCurrentOperation = coAbort;
                  }
                  else
                  {
                     if(ServerPort == ReferencedObjectServerPortValue)
                     {
                        ReferencedObjectsServerPortCurrentOperation = coAbort;
                     }
                  }
               }

               printf("BPP_Abort_Request: Function Successful.\r\n");
            }
            else
            {
               /* There was an error submitting the function.           */
               printf("BPP_Abort_Request() Failure: %d.\r\n", Result);
            }
         }
         else
         {
            /* One or more of the parameters passed in are invalid.     */
            printf("BPP Abort Request: Invalid parameter.\r\n");
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("BPP Abort Request: Invalid BPPID.\r\n");
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      printf("Stack ID Invalid.\r\n");
   }
}

   /* The following function is responsible for Sending a File to the   */
   /* Remote Device.                                                    */
static void BPPFilePushRequest(char *MIMEMediaType, char *Path, char *FileName, char *Description)
{
   int          Result;
   unsigned int AmountWritten;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the BPP Client ID appears to be    */
      /* semi-valid.                                                    */
      if(BPPClientID)
      {
         /* Check to make sure that the parameters passed in appear to  */
         /* be valid.                                                   */
         if((MIMEMediaType != NULL) && (strlen(MIMEMediaType) > 0) && (Path != NULL) && (strlen(Path) > 0) && (strlen(Path) < sizeof(JobServerPortCurrentFile)) && (FileName != NULL) && (strlen(FileName) > 0))
         {
            /* Check to make sure there isn't already a current         */
            /* operation.                                               */
            if(JobServerPortCurrentOperation == coNone)
            {
               /* There is currently not an on going operation.  Now    */
               /* initialize the current file variables.                */
               JobServerPortFirstPhase             = TRUE;
               JobServerPortCurrentFileIndex       = 0;
               JobServerPortCurrentFileSize        = 0;
               JobServerPortCurrentFileBufferIndex = 0;

               strcpy(JobServerPortCurrentFile, Path);

               /* Get some file data to send.                           */
               if(JobServerPortGetFileData() == FILE_DATA_FILE_IO_SUCCESS)
               {
                  Result = BPP_File_Push_Request(BluetoothStackID, BPPClientID, MIMEMediaType, FileName, Description, JobServerPortCurrentFileBufferIndex, JobServerPortCurrentFileBuffer, &AmountWritten, FALSE);

                  if(!Result)
                  {
                     /* The function was submitted successfully.        */

                     /* Update the Current Operation.                   */
                     JobServerPortCurrentOperation = coFilePush;

                     /* Adjust the current file buffer.                 */
                     if(AmountWritten)
                     {
                        memmove(JobServerPortCurrentFileBuffer, &JobServerPortCurrentFileBuffer[AmountWritten], (JobServerPortCurrentFileBufferIndex - AmountWritten));
                        JobServerPortCurrentFileBufferIndex -= AmountWritten;
                     }

                     printf("BPP_File_Push_Request: Function Successful.\r\n");
                  }
                  else
                  {
                     /* There was an error submitting the function.     */
                     printf("BPP_File_Push_Request() Failure: %d.\r\n", Result);
                  }
               }
               else
               {
                  /* Unable to get file data.                           */
                  printf("BPP File Push Request: Unable to Get File Data.\r\n");
               }
            }
            else
            {
               /* One or more of the parameters are invalid.            */
               printf("BPP File Push Request: Currently Outstanding Operation.\r\n");
            }
         }
         else
         {
            /* One or more of the parameters passed in are invalid.     */
            printf("BPP File Push Request: Invalid parameter.\r\n");
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("BPP File Push Request: Invalid BPPID.\r\n");
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      printf("Stack ID Invalid.\r\n");
   }
}

   /* The following function is responsible for sending a Get Printer   */
   /* Attributes Request to the Remote Device.                          */
static void BPPGetPrinterAttributesRequest(unsigned int ServerPort, unsigned int NumberDataElements, BPP_SOAP_Data_Element_t *DataElementBuffer)
{
   int Result;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the BPP Client ID appears to be    */
      /* semi-valid.                                                    */
      if(BPPClientID)
      {
         /* Check to make sure that the parameters passed in appear to  */
         /* be valid.                                                   */
         if((ServerPort) && ((NumberDataElements == 0) || ((NumberDataElements > 0) && (DataElementBuffer != NULL))))
         {
            /* Check to make sure there isn't already a current         */
            /* operation.                                               */
            if(((ServerPort == JobServerPortValue) && (JobServerPortCurrentOperation == coNone)) || ((ServerPort == StatusServerPortValue) && (StatusServerPortCurrentOperation == coNone)))
            {
               /* There is currently not an on going operation on the   */
               /* specified port.  Attempt to submit the Get Printer    */
               /* Attributes Request.                                   */
               Result = BPP_Get_Printer_Attributes_Request(BluetoothStackID, BPPClientID, ServerPort, NumberDataElements, DataElementBuffer);

               if(!Result)
               {
                  /* The function was submitted successfully.           */

                  /* Update the Current Operation.                      */
                  if(ServerPort == JobServerPortValue)
                  {
                     JobServerPortCurrentOperation = coGetPrinterAttributes;
                  }
                  else
                  {
                     if(ServerPort == StatusServerPortValue)
                     {
                        StatusServerPortCurrentOperation = coGetPrinterAttributes;
                     }
                  }

                  printf("BPP_Get_Printer_Attributes_Request: Function Successful.\r\n");
               }
               else
               {
                  /* There was an error submitting the function.        */
                  printf("BPP_Get_Printer_Attributes_Request() Failure: %d.\r\n", Result);
               }
            }
            else
            {
               /* One or more of the parameters are invalid.            */
               printf("BPP Get Printer Attributes Request: Currently Outstanding Operation.\r\n");
            }
         }
         else
         {
            /* One or more of the parameters passed in are invalid.     */
            printf("BPP Get Printer Attributes Request: Invalid parameter.\r\n");
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("BPP Get Printer Attributes Request: Invalid BPPID.\r\n");
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      printf("Stack ID Invalid.\r\n");
   }
}

   /* The following function is responsible for sending a Get Printer   */
   /* Attributes Response to the Remote Device.                         */
static void BPPGetPrinterAttributesResponse(unsigned int ServerPort, Byte_t ResponseCode, unsigned int NumberDataElements, BPP_SOAP_Data_Element_t *DataElementBuffer)
{
   int Result;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the BPP Server ID appears to be    */
      /* semi-valid.                                                    */
      if(BPPServerID)
      {
         /* Check to make sure that the parameters passed in appear to  */
         /* be valid.                                                   */
         if((ServerPort) && ((NumberDataElements == 0) || ((NumberDataElements > 0) && (DataElementBuffer != NULL))))
         {
            /* Check to make sure that the current operation appears to */
            /* be correct.                                              */
            if(((ServerPort == JobServerPortValue) && (JobServerPortCurrentOperation == coGetPrinterAttributes)) || ((ServerPort == StatusServerPortValue) && (StatusServerPortCurrentOperation == coGetPrinterAttributes)))
            {
               Result = BPP_Get_Printer_Attributes_Response(BluetoothStackID, BPPServerID, ServerPort, ResponseCode, NumberDataElements, DataElementBuffer);

               if(!Result)
               {
                  /* The function was submitted successfully.           */

                  /* Update the Current Operation.                      */
                  if(ServerPort == JobServerPortValue)
                  {
                     JobServerPortCurrentOperation = coNone;
                  }
                  else
                  {
                     if(ServerPort == StatusServerPortValue)
                     {
                        StatusServerPortCurrentOperation = coNone;
                     }
                  }

                  printf("BPP_Get_Printer_Attributes_Response: Function Successful.\r\n");
               }
               else
               {
                  /* There was an error submitting the function.        */
                  printf("BPP_Get_Printer_Attributes_Response() Failure: %d.\r\n", Result);
               }
            }
            else
            {
               /* One or more of the parameters are invalid.            */
               printf("BPP Get Printer Attributes Response: Invalid Current Operation.\r\n");
            }
         }
         else
         {
            /* One or more of the parameters passed in are invalid.     */
            printf("BPP Get Printer Attributes Response: Invalid parameter.\r\n");
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("BPP Get Printer Attributes Response: Invalid BPPID.\r\n");
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      printf("Stack ID Invalid.\r\n");
   }
}

   /* The following function is responsible for sending a Create Job    */
   /* Request to the Remote Device.                                     */
static void BPPCreateJobRequest(unsigned int NumberDataElements, BPP_SOAP_Data_Element_t *DataElementBuffer)
{
   int Result;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the BPP Client ID appears to be    */
      /* semi-valid.                                                    */
      if(BPPClientID)
      {
         /* Check to make sure that the parameters passed in appear to  */
         /* be valid.                                                   */
         if((NumberDataElements == 0) || ((NumberDataElements > 0) && (DataElementBuffer != NULL)))
         {
            /* Check to make sure there isn't already a current         */
            /* operation.                                               */
            if(JobServerPortCurrentOperation == coNone)
            {
               /* There is currently not an on going operation on the   */
               /* specified port.  Attempt to submit the Create Job     */
               /* Request.                                              */
               Result = BPP_Create_Job_Request(BluetoothStackID, BPPClientID, NumberDataElements, DataElementBuffer);

               if(!Result)
               {
                  /* The function was submitted successfully.           */

                  JobServerPortCurrentOperation = coCreateJob;

                  printf("BPP_Create_Job_Request: Function Successful.\r\n");
               }
               else
               {
                  /* There was an error submitting the function.        */
                  printf("BPP_Create_Job_Request() Failure: %d.\r\n", Result);
               }
            }
            else
            {
               /* One or more of the parameters are invalid.            */
               printf("BPP Create Job Request: Currently Outstanding Operation.\r\n");
            }
         }
         else
         {
            /* One or more of the parameters passed in are invalid.     */
            printf("BPP Create Job Request: Invalid parameter.\r\n");
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("BPP Create Job Request: Invalid BPPID.\r\n");
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      printf("Stack ID Invalid.\r\n");
   }
}

   /* The following function is responsible for sending a Create Job    */
   /* Response to the Remote Device.                                    */
static void BPPCreateJobResponse(Byte_t ResponseCode, DWord_t JobId, unsigned int NumberDataElements, BPP_SOAP_Data_Element_t *DataElementBuffer)
{
   int Result;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the BPP Server ID appears to be    */
      /* semi-valid.                                                    */
      if(BPPServerID)
      {
         /* Check to make sure that the parameters passed in appear to  */
         /* be valid.                                                   */
         if((NumberDataElements == 0) || ((NumberDataElements > 0) && (DataElementBuffer != NULL)))
         {
            /* Check to make sure that the current operation appears to */
            /* be correct.                                              */
            if(JobServerPortCurrentOperation == coCreateJob)
            {
               Result = BPP_Create_Job_Response(BluetoothStackID, BPPServerID, ResponseCode, JobId, NumberDataElements, DataElementBuffer);

               if(!Result)
               {
                  /* The function was submitted successfully.           */

                  /* Update the Current Operation.                      */
                  JobServerPortCurrentOperation = coNone;

                  printf("BPP_Create_Job_Response: Function Successful.\r\n");
               }
               else
               {
                  /* There was an error submitting the function.        */
                  printf("BPP_Create_Job_Response() Failure: %d.\r\n", Result);
               }
            }
            else
            {
               /* One or more of the parameters are invalid.            */
               printf("BPP Create Job Response: Invalid Current Operation.\r\n");
            }
         }
         else
         {
            /* One or more of the parameters passed in are invalid.     */
            printf("BPP Create Job Response: Invalid parameter.\r\n");
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("BPP Create Job Response: Invalid BPPID.\r\n");
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      printf("Stack ID Invalid.\r\n");
   }
}

   /* The following function is responsible for sending a Document to   */
   /* the Remote Device.                                                */
static void BPPSendDocumentRequest(DWord_t JobId, char *MIMEMediaType, char *Path, char *FileName, char *Description)
{
   int          Result;
   unsigned int AmountWritten;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the BPP Client ID appears to be    */
      /* semi-valid.                                                    */
      if(BPPClientID)
      {
         /* Check to make sure that the parameters passed in appear to  */
         /* be valid.                                                   */
         if((JobId != BPP_JOB_ID_INVALID_VALUE) && (MIMEMediaType != NULL) && (strlen(MIMEMediaType) > 0) && (Path != NULL) && (strlen(Path) > 0) && (strlen(Path) < sizeof(JobServerPortCurrentFile)) && (FileName != NULL) && (strlen(FileName) > 0))
         {
            /* Check to make sure there isn't already a current         */
            /* operation.                                               */
            if(JobServerPortCurrentOperation == coNone)
            {
               /* There is currently not an on going operation.  Now    */
               /* initialize the current file variables.                */
               JobServerPortFirstPhase             = TRUE;
               JobServerPortCurrentFileIndex       = 0;
               JobServerPortCurrentFileSize        = 0;
               JobServerPortCurrentFileBufferIndex = 0;

               strcpy(JobServerPortCurrentFile, Path);

               /* Get some file data to send.                           */
               if(JobServerPortGetFileData() == FILE_DATA_FILE_IO_SUCCESS)
               {
                  Result = BPP_Send_Document_Request(BluetoothStackID, BPPClientID, JobId, MIMEMediaType, FileName, Description, JobServerPortCurrentFileBufferIndex, JobServerPortCurrentFileBuffer, &AmountWritten, FALSE);

                  if(!Result)
                  {
                     /* The function was submitted successfully.        */

                     /* Update the Current Operation.                   */
                     JobServerPortCurrentOperation = coSendDocument;

                     /* Adjust the current file buffer.                 */
                     if(AmountWritten)
                     {
                        memmove(JobServerPortCurrentFileBuffer, &JobServerPortCurrentFileBuffer[AmountWritten], (JobServerPortCurrentFileBufferIndex - AmountWritten));
                        JobServerPortCurrentFileBufferIndex -= AmountWritten;
                     }

                     printf("BPP_Send_Document_Request: Function Successful.\r\n");
                  }
                  else
                  {
                     /* There was an error submitting the function.     */
                     printf("BPP_Send_Document_Request() Failure: %d.\r\n", Result);
                  }
               }
               else
               {
                  /* Unable to get file data.                           */
                  printf("BPP Send Document Request: Unable to Get File Data.\r\n");
               }
            }
            else
            {
               /* One or more of the parameters are invalid.            */
               printf("BPP Send Document Request: Currently Outstanding Operation.\r\n");
            }
         }
         else
         {
            /* One or more of the parameters passed in are invalid.     */
            printf("BPP Send Document Request: Invalid parameter.\r\n");
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("BPP Send Document Request: Invalid BPPID.\r\n");
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      printf("Stack ID Invalid.\r\n");
   }
}

   /* The following function is responsible for sending a Get Job       */
   /* Attributes Request to the Remote Device.                          */
static void BPPGetJobAttributesRequest(unsigned int ServerPort, unsigned int NumberDataElements, BPP_SOAP_Data_Element_t *DataElementBuffer)
{
   int Result;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the BPP Client ID appears to be    */
      /* semi-valid.                                                    */
      if(BPPClientID)
      {
         /* Check to make sure that the parameters passed in appear to  */
         /* be valid.                                                   */
         if((ServerPort) && ((NumberDataElements == 0) || ((NumberDataElements > 0) && (DataElementBuffer != NULL))))
         {
            /* Check to make sure there isn't already a current         */
            /* operation.                                               */
            if(((ServerPort == JobServerPortValue) && (JobServerPortCurrentOperation == coNone)) || ((ServerPort == StatusServerPortValue) && (StatusServerPortCurrentOperation == coNone)))
            {
               /* There is currently not an on going operation on the   */
               /* specified port.  Attempt to submit the Get Job        */
               /* Attributes Request.                                   */
               Result = BPP_Get_Job_Attributes_Request(BluetoothStackID, BPPClientID, ServerPort, NumberDataElements, DataElementBuffer);

               if(!Result)
               {
                  /* The function was submitted successfully.           */

                  /* Update the Current Operation.                      */
                  if(ServerPort == JobServerPortValue)
                  {
                     JobServerPortCurrentOperation = coGetJobAttributes;
                  }
                  else
                  {
                     if(ServerPort == StatusServerPortValue)
                     {
                        StatusServerPortCurrentOperation = coGetJobAttributes;
                     }
                  }

                  printf("BPP_Get_Job_Attributes_Request: Function Successful.\r\n");
               }
               else
               {
                  /* There was an error submitting the function.        */
                  printf("BPP_Get_Job_Attributes_Request() Failure: %d.\r\n", Result);
               }
            }
            else
            {
               /* One or more of the parameters are invalid.            */
               printf("BPP Get Job Attributes Request: Currently Outstanding Operation.\r\n");
            }
         }
         else
         {
            /* One or more of the parameters passed in are invalid.     */
            printf("BPP Get Job Attributes Request: Invalid parameter.\r\n");
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("BPP Get Job Attributes Request: Invalid BPPID.\r\n");
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      printf("Stack ID Invalid.\r\n");
   }
}

   /* The following function is responsible for sending a Get Job       */
   /* Attributes Response to the Remote Device.                         */
static void BPPGetJobAttributesResponse(unsigned int ServerPort, Byte_t ResponseCode, unsigned int NumberDataElements, BPP_SOAP_Data_Element_t *DataElementBuffer)
{
   int Result;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the BPP Server ID appears to be    */
      /* semi-valid.                                                    */
      if(BPPServerID)
      {
         /* Check to make sure that the parameters passed in appear to  */
         /* be valid.                                                   */
         if((ServerPort) && ((NumberDataElements == 0) || ((NumberDataElements > 0) && (DataElementBuffer != NULL))))
         {
            /* Check to make sure that the current operation appears to */
            /* be correct.                                              */
            if(((ServerPort == JobServerPortValue) && (JobServerPortCurrentOperation == coGetJobAttributes)) || ((ServerPort == StatusServerPortValue) && (StatusServerPortCurrentOperation == coGetJobAttributes)))
            {
               Result = BPP_Get_Job_Attributes_Response(BluetoothStackID, BPPServerID, ServerPort, ResponseCode, NumberDataElements, DataElementBuffer);

               if(!Result)
               {
                  /* The function was submitted successfully.           */

                  /* Update the Current Operation.                      */
                  if(ServerPort == JobServerPortValue)
                  {
                     JobServerPortCurrentOperation = coNone;
                  }
                  else
                  {
                     if(ServerPort == StatusServerPortValue)
                     {
                        StatusServerPortCurrentOperation = coNone;
                     }
                  }

                  printf("BPP_Get_Job_Attributes_Response: Function Successful.\r\n");
               }
               else
               {
                  /* There was an error submitting the function.        */
                  printf("BPP_Get_Job_Attributes_Response() Failure: %d.\r\n", Result);
               }
            }
            else
            {
               /* One or more of the parameters are invalid.            */
               printf("BPP Get Job Attributes Response: Invalid Current Operation.\r\n");
            }
         }
         else
         {
            /* One or more of the parameters passed in are invalid.     */
            printf("BPP Get Job Attributes Response: Invalid parameter.\r\n");
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("BPP Get Job Attributes Response: Invalid BPPID.\r\n");
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      printf("Stack ID Invalid.\r\n");
   }
}

   /* The following function is responsible for sending a Cancel Job    */
   /* Request to the Remote Device.                                     */
static void BPPCancelJobRequest(unsigned int ServerPort, unsigned int NumberDataElements, BPP_SOAP_Data_Element_t *DataElementBuffer)
{
   int Result;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the BPP Client ID appears to be    */
      /* semi-valid.                                                    */
      if(BPPClientID)
      {
         /* Check to make sure that the parameters passed in appear to  */
         /* be valid.                                                   */
         if((ServerPort) && ((NumberDataElements == 0) || ((NumberDataElements > 0) && (DataElementBuffer != NULL))))
         {
            /* Check to make sure there isn't already a current         */
            /* operation.                                               */
            if(((ServerPort == JobServerPortValue) && (JobServerPortCurrentOperation == coNone)) || ((ServerPort == StatusServerPortValue) && (StatusServerPortCurrentOperation == coNone)))
            {
               /* There is currently not an on going operation on the   */
               /* specified port.  Attempt to submit the Cancel Job     */
               /* Request.                                              */
               Result = BPP_Cancel_Job_Request(BluetoothStackID, BPPClientID, ServerPort, NumberDataElements, DataElementBuffer);

               if(!Result)
               {
                  /* The function was submitted successfully.           */

                  /* Update the Current Operation.                      */
                  if(ServerPort == JobServerPortValue)
                  {
                     JobServerPortCurrentOperation = coCancelJob;
                  }
                  else
                  {
                     if(ServerPort == StatusServerPortValue)
                     {
                        StatusServerPortCurrentOperation = coCancelJob;
                     }
                  }

                  printf("BPP_Cancel_Job_Request: Function Successful.\r\n");
               }
               else
               {
                  /* There was an error submitting the function.        */
                  printf("BPP_Cancel_Job_Request() Failure: %d.\r\n", Result);
               }
            }
            else
            {
               /* One or more of the parameters are invalid.            */
               printf("BPP Cancel Job Request: Currently Outstanding Operation.\r\n");
            }
         }
         else
         {
            /* One or more of the parameters passed in are invalid.     */
            printf("BPP Cancel Job Request: Invalid parameter.\r\n");
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("BPP Cancel Job Request: Invalid BPPID.\r\n");
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      printf("Stack ID Invalid.\r\n");
   }
}

   /* The following function is responsible for sending a Cancel Job    */
   /* Response to the Remote Device.                                    */
static void BPPCancelJobResponse(unsigned int ServerPort, Byte_t ResponseCode, unsigned int NumberDataElements, BPP_SOAP_Data_Element_t *DataElementBuffer)
{
   int Result;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the BPP Server ID appears to be    */
      /* semi-valid.                                                    */
      if(BPPServerID)
      {
         /* Check to make sure that the parameters passed in appear to  */
         /* be valid.                                                   */
         if((ServerPort) && ((NumberDataElements == 0) || ((NumberDataElements > 0) && (DataElementBuffer != NULL))))
         {
            /* Check to make sure that the current operation appears to */
            /* be correct.                                              */
            if(((ServerPort == JobServerPortValue) && (JobServerPortCurrentOperation == coCancelJob)) || ((ServerPort == StatusServerPortValue) && (StatusServerPortCurrentOperation == coCancelJob)))
            {
               Result = BPP_Cancel_Job_Response(BluetoothStackID, BPPServerID, ServerPort, ResponseCode, NumberDataElements, DataElementBuffer);

               if(!Result)
               {
                  /* The function was submitted successfully.           */

                  /* Update the Current Operation.                      */
                  if(ServerPort == JobServerPortValue)
                  {
                     JobServerPortCurrentOperation = coNone;
                  }
                  else
                  {
                     if(ServerPort == StatusServerPortValue)
                     {
                        StatusServerPortCurrentOperation = coNone;
                     }
                  }

                  printf("BPP_Cancel_Job_Response: Function Successful.\r\n");
               }
               else
               {
                  /* There was an error submitting the function.        */
                  printf("BPP_Cancel_Job_Response() Failure: %d.\r\n", Result);
               }
            }
            else
            {
               /* One or more of the parameters are invalid.            */
               printf("BPP Cancel Job Response: Invalid Current Operation.\r\n");
            }
         }
         else
         {
            /* One or more of the parameters passed in are invalid.     */
            printf("BPP Cancel Job Response: Invalid parameter.\r\n");
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("BPP Cancel Job Response: Invalid BPPID.\r\n");
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      printf("Stack ID Invalid.\r\n");
   }
}

   /* The following function is responsible for sending a Get Event     */
   /* Request to the Remote Device.                                     */
static void BPPGetEventRequest(unsigned int NumberDataElements, BPP_SOAP_Data_Element_t *DataElementBuffer)
{
   int Result;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the BPP Client ID appears to be    */
      /* semi-valid.                                                    */
      if(BPPClientID)
      {
         /* Check to make sure that the parameters passed in appear to  */
         /* be valid.                                                   */
         if((NumberDataElements == 0) || ((NumberDataElements > 0) && (DataElementBuffer != NULL)))
         {
            /* Check to make sure there isn't already a current         */
            /* operation.                                               */
            if(StatusServerPortCurrentOperation == coNone)
            {
               /* There is currently not an on going operation on the   */
               /* specified port.  Attempt to submit the Get Event      */
               /* Request.                                              */
               Result = BPP_Get_Event_Request(BluetoothStackID, BPPClientID, NumberDataElements, DataElementBuffer);

               if(!Result)
               {
                  /* The function was submitted successfully.           */
                  StatusServerPortCurrentOperation = coGetEvent;

                  printf("BPP_Get_Event_Request: Function Successful.\r\n");
               }
               else
               {
                  /* There was an error submitting the function.        */
                  printf("BPP_Get_Event_Request() Failure: %d.\r\n", Result);
               }
            }
            else
            {
               /* One or more of the parameters are invalid.            */
               printf("BPP Get Event Request: Currently Outstanding Operation.\r\n");
            }
         }
         else
         {
            /* One or more of the parameters passed in are invalid.     */
            printf("BPP Get Event Request: Invalid parameter.\r\n");
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("BPP Get Event Request: Invalid BPPID.\r\n");
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      printf("Stack ID Invalid.\r\n");
   }
}

   /* The following function is responsible for sending a Get Event     */
   /* Response to the Remote Device.                                    */
static void BPPGetEventResponse(Byte_t ResponseCode, unsigned int NumberDataElements, BPP_SOAP_Data_Element_t *DataElementBuffer)
{
   int Result;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the BPP Server ID appears to be    */
      /* semi-valid.                                                    */
      if(BPPServerID)
      {
         /* Check to make sure that the parameters passed in appear to  */
         /* be valid.                                                   */
         if((NumberDataElements == 0) || ((NumberDataElements > 0) && (DataElementBuffer != NULL)))
         {
            /* Check to make sure that the current operation appears to */
            /* be correct.                                              */
            if(StatusServerPortCurrentOperation == coGetEvent)
            {
               Result = BPP_Get_Event_Response(BluetoothStackID, BPPServerID, ResponseCode, NumberDataElements, DataElementBuffer);

               if(!Result)
               {
                  /* The function was submitted successfully.           */

                  /* Update the Current Operation.                      */
                  StatusServerPortCurrentOperation = coNone;

                  printf("BPP_Get_Event_Response: Function Successful.\r\n");
               }
               else
               {
                  /* There was an error submitting the function.        */
                  printf("BPP_Get_Event_Response() Failure: %d.\r\n", Result);
               }
            }
            else
            {
               /* One or more of the parameters are invalid.            */
               printf("BPP Get Event Response: Invalid Current Operation.\r\n");
            }
         }
         else
         {
            /* One or more of the parameters passed in are invalid.     */
            printf("BPP Get Event Response: Invalid parameter.\r\n");
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("BPP Get Event Response: Invalid BPPID.\r\n");
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      printf("Stack ID Invalid.\r\n");
   }
}

   /* The following function is responsible for sending a Get Referenced*/
   /* Objects Request to the Remote Device.                             */
static void BPPGetReferencedObjectsRequest(char *ObjectIdentifier, DWord_t Offset, DWord_t Count, Boolean_t RequestObjectSize)
{
   int Result;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now check to make sure that the BPP Client ID appears to be    */
      /* semi-valid.                                                    */
      if(BPPClientID)
      {
         /* Check to make sure that the parameters passed in appear to  */
         /* be valid.                                                   */
         if((ObjectIdentifier != NULL) && (strlen(ObjectIdentifier) > 0))
         {
            /* Check to make sure there isn't already a current         */
            /* operation.                                               */
            if(ReferencedObjectsServerPortCurrentOperation == coNone)
            {
               /* Reset the current file index and set that this is the */
               /* first phase.                                          */
               ReferencedObjectsServerPortCurrentFileIndex = 0;
               ReferencedObjectsServerPortFirstPhase       = TRUE;

               /* Set the Current File to the save as path specifed.    */
               strcpy(ReferencedObjectsServerPortCurrentFile, ObjectIdentifier);

               /* There is currently not an on going operation on the   */
               /* specified port.  Attempt to submit the Get Referenced */
               /* Object Request.                                       */
               Result = BPP_Get_Referenced_Objects_Request(BluetoothStackID, BPPClientID, ObjectIdentifier, Offset, Count, RequestObjectSize);

               if(!Result)
               {
                  /* The function was submitted successfully.           */
                  ReferencedObjectsServerPortCurrentOperation = coGetReferencedObjects;

                  printf("BPP_Get_Referenced_Objects_Request: Function Successful.\r\n");
               }
               else
               {
                  /* There was an error submitting the function.        */
                  printf("BPP_Get_Referenced_Objects_Request() Failure: %d.\r\n", Result);
               }

            }
            else
            {
               /* One or more of the parameters are invalid.            */
               printf("BPP Get Referenced Objects Request: Invalid Current Operation.\r\n");
            }
         }
         else
         {
            /* One or more of the parameters passed in are invalid.     */
            printf("BPP Get Referenced Objects Request: Invalid parameter.\r\n");
         }
      }
      else
      {
         /* One or more of the parameters are invalid.                  */
         printf("BPP Get Referenced Objects Request: Invalid BPPID.\r\n");
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

      if(ServiceType == stPrinter)
         printf("\r\nPrinter>");
      else
         printf("\r\nSender>");
   }
   else
   {
      /* There was an error with one or more of the input parameters.   */
      printf("\r\n");

      printf("GAP Callback Data: Event_Data = NULL.\r\n");

      if(ServiceType == stPrinter)
         printf("\r\nPrinter>");
      else
         printf("\r\nSender>");
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

   if(ServiceType == stPrinter)
      printf("\r\nPrinter>");
   else
      printf("\r\nSender>");

   /* Make sure the output is displayed to the user.                    */
   fflush(stdout);
}

   /* The following function is for a BPP Event Callback.  This         */
   /* function will be called whenever a BPP Client or Server Event     */
   /* occurs that is associated with the Bluetooth Stack.  This function*/
   /* passes to the caller the BPP Event Data that occurred and the BPP */
   /* Event Callback Parameter that was specified when this Callback was*/
   /* installed.  The caller is free to use the contents of the BPP     */
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
   /* (this argument holds anyway because another BPP Event will not be */
   /* processed while this function call is outstanding).               */
   /* * NOTE * This function MUST NOT Block and wait for Events that can*/
   /*          only be satisfied by Receiving BPP Event Data.  A        */
   /*          Deadlock WILL occur because NO BPP Event Callbacks will  */
   /*          be issued while this function is currently outstanding.  */
static void BTPSAPI BPP_Event_Callback(unsigned int BluetoothStackID, BPP_Event_Data_t *BPPEventData, unsigned long CallbackParameter)
{
   int          Result;
   char         FileName[512+1];
   Byte_t       ResponseCode;
   DWord_t      ObjectSize;
   char         BoardStr[32];
   unsigned int AmountWritten;

   /* Make sure that parameters passed in appear to be at least         */
   /* semi-valid.                                                       */
   if((BluetoothStackID) && (BPPEventData))
   {
      printf("\r\n");

      /* The parameters passed in appear to be at least semi-valid.     */
      /* Next determine the event that has occurred.                    */
      switch(BPPEventData->Event_Data_Type)
      {
         case etBPP_Open_Request_Indication:
            /* A Client has request to connect to the server.  Display  */
            /* the Event Information associated with this connection    */
            /* request.                                                 */
            BD_ADDRToStr(BPPEventData->Event_Data.BPP_Open_Request_Indication_Data->BD_ADDR, BoardStr);
            printf("BPP Open Request Indication, ID: %u, ServerPort %u, Board: %s.\r\n", BPPEventData->Event_Data.BPP_Open_Request_Indication_Data->BPPID,
                                                                                         BPPEventData->Event_Data.BPP_Open_Request_Indication_Data->ServerPort,
                                                                                         BoardStr);
            break;
         case etBPP_Open_Job_Port_Indication:
            /* A client has connected to the servers job port.  Display */
            /* the event information associated with this connection.   */
            BD_ADDRToStr(BPPEventData->Event_Data.BPP_Open_Job_Port_Indication_Data->BD_ADDR, BoardStr);
            printf("BPP Open Job Port Indication, ID: %u, Board: %s.\r\n", BPPEventData->Event_Data.BPP_Open_Job_Port_Indication_Data->BPPID,
                                                                           BoardStr);

            /* Determine the Job Server Port Target that was connected  */
            /* to.                                                      */
            switch(BPPEventData->Event_Data.BPP_Open_Job_Port_Indication_Data->JobServerPortTarget)
            {
               case jtDirectPrintingService:
                  printf("ServiceTarget: Direct Printing\r\n");
                  break;
               case jtReferencedPrintingService:
                  printf("ServiceTarget: Referenced Printing\r\n");
                  break;
               case jtAdministrativeUserInterfaceService:
                  printf("ServiceTarget: Administrative User Interface\r\n");
                  break;
            }

            /* Set the bit to indicate that the Job Server Port is      */
            /* connected.                                               */
            ServerPortsConnected |= JOB_SERVER_PORT_CONNECTED_BIT;
            break;
         case etBPP_Open_Status_Port_Indication:
            /* A client has connected to the servers status port.       */
            /* Display the event information associated with this       */
            /* connection.                                              */
            BD_ADDRToStr(BPPEventData->Event_Data.BPP_Open_Status_Port_Indication_Data->BD_ADDR, BoardStr);
            printf("BPP Open Status Port Indication, ID: %u, Board: %s.\r\n", BPPEventData->Event_Data.BPP_Open_Status_Port_Indication_Data->BPPID,
                                                                              BoardStr);

            /* Set the bit to indicate that the Status Server Port is   */
            /* connected.                                               */
            ServerPortsConnected |= STATUS_SERVER_PORT_CONNECTED_BIT;
            break;
         case etBPP_Open_Referenced_Object_Port_Indication:
            /* A client has connected to the servers referenced object  */
            /* port.  Display the event information associated with this*/
            /* connection.                                              */
            BD_ADDRToStr(BPPEventData->Event_Data.BPP_Open_Referenced_Object_Port_Indication_Data->BD_ADDR, BoardStr);
            printf("BPP Open Status Port Indication, ID: %u, Board: %s.\r\n", BPPEventData->Event_Data.BPP_Open_Referenced_Object_Port_Indication_Data->BPPID,
                                                                              BoardStr);

            /* Set the bit to indicate that the Referenced Server Port  */
            /* is connected.                                            */
            ServerPortsConnected |= REFERENCED_OBJECT_SERVER_PORT_CONNECTED_BIT;
            break;
         case etBPP_Open_Job_Port_Confirmation:
            /* The Client has received a Connect Confirmation, display  */
            /* relevant information.                                    */
            printf("BPP Open Job Port Confirmation, ID: %u, Status 0x%08X.\r\n", BPPEventData->Event_Data.BPP_Open_Job_Port_Confirmation_Data->BPPID,
                                                                                 (unsigned int)BPPEventData->Event_Data.BPP_Open_Job_Port_Confirmation_Data->OpenStatus);

            /* Check to see if the Open Status indicates success.       */
            if(BPPEventData->Event_Data.BPP_Open_Job_Port_Confirmation_Data->OpenStatus != BPP_OPEN_STATUS_SUCCESS)
            {
               /* The Open Status indicates that the connection was not */
               /* successfully created.  Reset the connection state     */
               /* information.                                          */
               ClientPortsConnected &= ~JOB_SERVER_PORT_CONNECTED_BIT;

               /* Check to see if the Client Ports Connected mask       */
               /* indicates that there are no more connections open.    */
               if(ClientPortsConnected == 0)
               {
                  /* There are not more client connections open         */
                  /* associated with the BPP Client ID.  Reset the BPP  */
                  /* Client ID.                                         */
                  BPPClientID = 0;
               }
            }
            break;
         case etBPP_Open_Status_Port_Confirmation:
            /* The Client has received a Connect Confirmation, display  */
            /* relevant information.                                    */
            printf("BPP Open Status Port Confirmation, ID: %u, Status 0x%08X.\r\n", BPPEventData->Event_Data.BPP_Open_Status_Port_Confirmation_Data->BPPID,
                                                                                    (unsigned int)BPPEventData->Event_Data.BPP_Open_Status_Port_Confirmation_Data->OpenStatus);

            /* Check to see if the Open Status indicates success.       */
            if(BPPEventData->Event_Data.BPP_Open_Status_Port_Confirmation_Data->OpenStatus != BPP_OPEN_STATUS_SUCCESS)
            {
               /* The Open Status indicates that the connection was not */
               /* successfully created.  Reset the connection state     */
               /* information.                                          */
               ClientPortsConnected &= ~STATUS_SERVER_PORT_CONNECTED_BIT;

               /* Check to see if the Client Ports Connected mask       */
               /* indicates that there are no more connections open.    */
               if(ClientPortsConnected == 0)
               {
                  /* There are not more client connections open         */
                  /* associated with the BPP Client ID.  Reset the BPP  */
                  /* Client ID.                                         */
                  BPPClientID = 0;
               }
            }
            break;
         case etBPP_Open_Referenced_Object_Port_Confirmation:
            /* The Client has received a Connect Confirmation, display  */
            /* relevant information.                                    */
            printf("BPP Open Referenced Object Port Confirmation, ID: %u, Status 0x%08X.\r\n", BPPEventData->Event_Data.BPP_Open_Referenced_Object_Port_Confirmation_Data->BPPID,
                                                                                               (unsigned int)BPPEventData->Event_Data.BPP_Open_Referenced_Object_Port_Confirmation_Data->OpenStatus);

            /* Check to see if the Open Status indicates success.       */
            if(BPPEventData->Event_Data.BPP_Open_Referenced_Object_Port_Confirmation_Data->OpenStatus != BPP_OPEN_STATUS_SUCCESS)
            {
               /* The Open Status indicates that the connection was not */
               /* successfully created.  Reset the connection state     */
               /* information.                                          */
               ClientPortsConnected &= ~REFERENCED_OBJECT_SERVER_PORT_CONNECTED_BIT;

               /* Check to see if the Client Ports Connected mask       */
               /* indicates that there are no more connections open.    */
               if(ClientPortsConnected == 0)
               {
                  /* There are not more client connections open         */
                  /* associated with the BPP Client ID.  Reset the BPP  */
                  /* Client ID.                                         */
                  BPPClientID = 0;
               }
            }
            break;
         case etBPP_Close_Job_Port_Indication:
            /* A Close Port Indication was received, this means that the*/
            /* remote device has Disconnected from the local device.    */
            /* Display the appropriate message.                         */
            printf("BPP Close Job Port Indication, ID: %u.\r\n", BPPEventData->Event_Data.BPP_Close_Job_Port_Indication_Data->BPPID);

            /* Check to see if this Close Indication is associated with */
            /* the BPP Server ID.                                       */
            if(BPPServerID == BPPEventData->Event_Data.BPP_Close_Job_Port_Indication_Data->BPPID)
            {
               /* This Close Indication is associated with the BPP      */
               /* Server ID, reset the bit indicating that the Job      */
               /* Server Port is connected.                             */
               ServerPortsConnected &= ~JOB_SERVER_PORT_CONNECTED_BIT;
            }
            else
            {
               /* This Close indication is not associated with the BPP  */
               /* Server ID, check to see if it is associated with the  */
               /* BPP Client ID.                                        */
               if(BPPClientID == BPPEventData->Event_Data.BPP_Close_Job_Port_Indication_Data->BPPID)
               {
                  /* This Close indication is associated with the BPP   */
                  /* Client ID.  Reset the bit indicating that the Job  */
                  /* Server Port is connected.                          */
                  ClientPortsConnected &= ~JOB_SERVER_PORT_CONNECTED_BIT;

                  /* Check to see if the Client Ports Connected mask    */
                  /* indicates that there are no more connections open. */
                  if(ClientPortsConnected == 0)
                  {
                     /* There are not more client connections open      */
                     /* associated with the BPP Client ID.  Reset the   */
                     /* BPP Client ID.                                  */
                     BPPClientID = 0;
                  }
               }
            }

            /* Reset the current operation on this port.                */
            JobServerPortCurrentOperation = coNone;
            break;
         case etBPP_Close_Status_Port_Indication:
            /* A Close Port Indication was received, this means that the*/
            /* remote device has Disconnected from the local device.    */
            /* Display the appropriate message.                         */
            printf("BPP Close Status Port Indication, ID: %u.\r\n", BPPEventData->Event_Data.BPP_Close_Status_Port_Indication_Data->BPPID);

            /* Check to see if this Close Indication is associated with */
            /* the BPP Server ID.                                       */
            if(BPPServerID == BPPEventData->Event_Data.BPP_Close_Status_Port_Indication_Data->BPPID)
            {
               /* This Close Indication is associated with the BPP      */
               /* Server ID, reset the bit indicating that the Status   */
               /* Server Port is connected.                             */
               ServerPortsConnected &= ~STATUS_SERVER_PORT_CONNECTED_BIT;
            }
            else
            {
               /* This Close indication is not associated with the BPP  */
               /* Server ID, check to see if it is associated with the  */
               /* BPP Client ID.                                        */
               if(BPPClientID == BPPEventData->Event_Data.BPP_Close_Status_Port_Indication_Data->BPPID)
               {
                  /* This Close indication is associated with the BPP   */
                  /* Client ID.  Reset the bit indicating that the      */
                  /* Status Server Port is connected.                   */
                  ClientPortsConnected &= ~STATUS_SERVER_PORT_CONNECTED_BIT;

                  /* Check to see if the Client Ports Connected mask    */
                  /* indicates that there are no more connections open. */
                  if(ClientPortsConnected == 0)
                  {
                     /* There are not more client connections open      */
                     /* associated with the BPP Client ID.  Reset the   */
                     /* BPP Client ID.                                  */
                     BPPClientID = 0;
                  }
               }
            }

            /* Reset the current operation on this port.                */
            StatusServerPortCurrentOperation = coNone;
            break;
         case etBPP_Close_Referenced_Object_Port_Indication:
            /* A Close Port Indication was received, this means that the*/
            /* remote device has Disconnected from the local device.    */
            /* Display the appropriate message.                         */
            printf("BPP Close Referenced Object Port Indication, ID: %u.\r\n", BPPEventData->Event_Data.BPP_Close_Referenced_Object_Port_Indication_Data->BPPID);

            /* Check to see if this Close Indication is associated with */
            /* the BPP Server ID.                                       */
            if(BPPServerID == BPPEventData->Event_Data.BPP_Close_Referenced_Object_Port_Indication_Data->BPPID)
            {
               /* This Close Indication is associated with the BPP      */
               /* Server ID, reset the bit indicating that the          */
               /* Referenced Object Server Port is connected.           */
               ServerPortsConnected &= ~REFERENCED_OBJECT_SERVER_PORT_CONNECTED_BIT;
            }
            else
            {
               /* This Close indication is not associated with the BPP  */
               /* Server ID, check to see if it is associated with the  */
               /* BPP Client ID.                                        */
               if(BPPClientID == BPPEventData->Event_Data.BPP_Close_Referenced_Object_Port_Indication_Data->BPPID)
               {
                  /* This Close indication is associated with the BPP   */
                  /* Client ID.  Reset the bit indicating that the      */
                  /* Referenced Object Server Port is connected.        */
                  ClientPortsConnected &= ~REFERENCED_OBJECT_SERVER_PORT_CONNECTED_BIT;

                  /* Check to see if the Client Ports Connected mask    */
                  /* indicates that there are no more connections open. */
                  if(ClientPortsConnected == 0)
                  {
                     /* There are not more client connections open      */
                     /* associated with the BPP Client ID.  Reset the   */
                     /* BPP Client ID.                                  */
                     BPPClientID = 0;
                  }
               }
            }

            /* Reset the current operation on this port.                */
            ReferencedObjectsServerPortCurrentOperation = coNone;
            break;
         case etBPP_Abort_Indication:
            /* An Abort Indication has been received, display all       */
            /* relevant information.                                    */
            printf("BPP Abort Indication, ID: %u, ServerPort: %u.\r\n", BPPEventData->Event_Data.BPP_Abort_Indication_Data->BPPID, BPPEventData->Event_Data.BPP_Abort_Indication_Data->ServerPort);

            /* Check to see which port this Event was received on.      */
            if(BPPEventData->Event_Data.BPP_Abort_Indication_Data->ServerPort == JobServerPortValue)
            {
               /* The event was received on the Job Server Port.  Update*/
               /* the current operation for this port.                  */
               JobServerPortCurrentOperation = coNone;

               /* Reset all other state variables.                      */
               JobServerPortFirstPhase             = TRUE;
               JobServerPortCurrentFile[0]         = '\0';
               JobServerPortCurrentFileIndex       = 0;
               JobServerPortCurrentFileSize        = 0;

               JobServerPortCurrentFileBufferIndex = 0;
            }
            else
            {
               if(BPPEventData->Event_Data.BPP_Abort_Indication_Data->ServerPort == StatusServerPortValue)
               {
                  /* The event was received on the Status Server Port.  */
                  /* Update the current operation for this port.        */
                  StatusServerPortCurrentOperation = coNone;
               }
               else
               {
                  if(BPPEventData->Event_Data.BPP_Abort_Indication_Data->ServerPort == ReferencedObjectServerPortValue)
                  {
                     /* The event was received on the Referenced Object */
                     /* Server Port.  Update the current operation for  */
                     /* this port.                                      */
                     ReferencedObjectsServerPortCurrentOperation = coNone;
                  }
               }
            }
            break;
         case etBPP_Abort_Confirmation:
            /* An Abort Confirmation has been received, display all     */
            /* relevant information.                                    */
            printf("BPP Abort Confirmation, ID: %u.\r\n", BPPEventData->Event_Data.BPP_Abort_Confirmation_Data->BPPID);

            /* Check to see which port this Event was received on.      */
            if(BPPEventData->Event_Data.BPP_Abort_Confirmation_Data->ServerPort == JobServerPortValue)
            {
               /* The event was received on the Job Server Port.  Update*/
               /* the current operation for this port.                  */
               JobServerPortCurrentOperation = coNone;

               /* Reset all other state variables.                      */
               JobServerPortFirstPhase             = TRUE;
               JobServerPortCurrentFile[0]         = '\0';
               JobServerPortCurrentFileIndex       = 0;
               JobServerPortCurrentFileSize        = 0;

               JobServerPortCurrentFileBufferIndex = 0;
            }
            else
            {
               if(BPPEventData->Event_Data.BPP_Abort_Confirmation_Data->ServerPort == StatusServerPortValue)
               {
                  /* The event was received on the Status Server Port.  */
                  /* Update the current operation for this port.        */
                  StatusServerPortCurrentOperation = coNone;
               }
               else
               {
                  if(BPPEventData->Event_Data.BPP_Abort_Confirmation_Data->ServerPort == ReferencedObjectServerPortValue)
                  {
                     /* The event was received on the Referenced Object */
                     /* Server Port.  Update the current operation for  */
                     /* this port.                                      */
                     ReferencedObjectsServerPortCurrentOperation = coNone;
                  }
               }
            }
            break;
         case etBPP_File_Push_Indication:
            /* A File Push Indication was received, display all relevant*/
            /* information.                                             */
            printf("BPP File Push Indication, ID: %u.\r\n", BPPEventData->Event_Data.BPP_File_Push_Indication_Data->BPPID);

            /* Initialize the Response Code to an Error Response.       */
            ResponseCode = BPP_OBEX_RESPONSE_BAD_REQUEST;

            /* Check to see if this is the first phase of the operation.*/
            if(JobServerPortCurrentOperation == coNone)
            {
               /* Check to see if the MIME Media Type appears to be     */
               /* valid.                                                */
               if(BPPEventData->Event_Data.BPP_File_Push_Indication_Data->MIMEMediaType != NULL)
               {
                  /* The MIME Media Type is valid.  Display the MIME    */
                  /* Media Type.                                        */
                  printf("    MIME Media Type: %s.\r\n", BPPEventData->Event_Data.BPP_File_Push_Indication_Data->MIMEMediaType);
               }

               /* Check to see if the File Name appears to be valid.    */
               if(BPPEventData->Event_Data.BPP_File_Push_Indication_Data->FileName != NULL)
               {
                  strncpy(FileName, BPPEventData->Event_Data.BPP_File_Push_Indication_Data->FileName, sizeof(FileName));
                  FileName[sizeof(FileName)-1] = '\0';
               }
               else
               {
                  /* A file name was not specified, generate a unique   */
                  /* file name to be used.                              */
                  if(GetTempFileName(".", "SS1", FileName) == 0)
                     FileName[0] = '\0';
               }

               if(strlen(FileName) > 0)
               {
                  /* The File Name is valid.  Display the File Name.    */
                  printf("    File Name: %s.\r\n", FileName);
               }

               /* Check to see if the Description appears to be valid.  */
               if(BPPEventData->Event_Data.BPP_File_Push_Indication_Data->Description != NULL)
               {
                  /* Display the Description Received.                  */
                  printf("    Description: %s.\r\n", BPPEventData->Event_Data.BPP_File_Push_Indication_Data->Description);
               }

               /* Check to see if a file name was specified, or         */
               /* successfully generated.                               */
               if(strlen(FileName) > 0)
               {
                  /* Check to make sure that the path to the new        */
                  /* filename can be successfully built.                */
                  if(sizeof(JobServerPortCurrentFile) >= (strlen(RootDirectory) + strlen(FileName) + 1))
                  {
                     /* Reset the current file index and set that this  */
                     /* is the first phase.                             */
                     JobServerPortCurrentFileIndex = 0;
                     JobServerPortFirstPhase       = TRUE;

                     /* Build the full path to the storage location for */
                     /* the file being received.                        */
                     strcpy(JobServerPortCurrentFile, RootDirectory);
                     strcat(JobServerPortCurrentFile, FileName);

                     /* Check to see if there is file data associated   */
                     /* with this transaction.                          */
                     if((BPPEventData->Event_Data.BPP_File_Push_Indication_Data->DataLength > 0) && (BPPEventData->Event_Data.BPP_File_Push_Indication_Data->DataBuffer != NULL))
                     {
                        /* There is file data associated with this      */
                        /* transaction.  Put the data to a file.        */
                        if(JobServerPortPutFileData(BPPEventData->Event_Data.BPP_File_Push_Indication_Data->DataLength, BPPEventData->Event_Data.BPP_File_Push_Indication_Data->DataBuffer) == FILE_DATA_FILE_IO_SUCCESS)
                        {
                           /* Data was successfully written to the file,*/
                           /* set the current operation and response    */
                           /* code.                                     */
                           JobServerPortCurrentOperation = coFilePush;
                           ResponseCode                  = BPP_OBEX_RESPONSE_CONTINUE;
                        }
                     }
                     else
                     {
                        /* No file data to write.  Set the current      */
                        /* operation and response code.                 */
                        JobServerPortCurrentOperation = coFilePush;
                        ResponseCode                  = BPP_OBEX_RESPONSE_CONTINUE;
                     }
                  }
               }
            }
            else
            {
               /* This is not the first phase of the operation, check to*/
               /* see if this is a continuation of a currently on going */
               /* operation.                                            */
               if(JobServerPortCurrentOperation == coFilePush)
               {
                  /* This is a continuation of the currently ongoing    */
                  /* operation.  Check to see if the MIME Media Type    */
                  /* appears to be valid.                               */
                  if(BPPEventData->Event_Data.BPP_File_Push_Indication_Data->MIMEMediaType != NULL)
                  {
                     /* The MIME Media Type is valid.  Display the MIME */
                     /* Media Type.                                     */
                     printf("    MIME Media Type: %s.\r\n", BPPEventData->Event_Data.BPP_File_Push_Indication_Data->MIMEMediaType);
                  }

                  /* Check to see if the File Name appears to be valid. */
                  if((BPPEventData->Event_Data.BPP_File_Push_Indication_Data->FileName != NULL) && (strlen(JobServerPortCurrentFile) > 0))
                  {
                     /* The File Name is valid.  Display the File Name. */
                     printf("    File Name: %s.\r\n", JobServerPortCurrentFile);
                  }

                  /* Check to see if the Description appears to be      */
                  /* valid.                                             */
                  if(BPPEventData->Event_Data.BPP_File_Push_Indication_Data->Description != NULL)
                  {
                     /* Display the Description Received.               */
                     printf("    Description: %s.\r\n", BPPEventData->Event_Data.BPP_File_Push_Indication_Data->Description);
                  }

                  /* Check to see if there is file data associated with */
                  /* this transaction.                                  */
                  if((BPPEventData->Event_Data.BPP_File_Push_Indication_Data->DataLength > 0) && (BPPEventData->Event_Data.BPP_File_Push_Indication_Data->DataBuffer != NULL))
                  {
                     /* There is file data associated with this         */
                     /* transaction.  Put the data to a file.           */
                     if(JobServerPortPutFileData(BPPEventData->Event_Data.BPP_File_Push_Indication_Data->DataLength, BPPEventData->Event_Data.BPP_File_Push_Indication_Data->DataBuffer) == FILE_DATA_FILE_IO_SUCCESS)
                     {
                        /* Set the response code to indicate success.   */
                        ResponseCode = BPP_OBEX_RESPONSE_CONTINUE;
                     }
                  }

                  /* Display a message indicating the total length      */
                  /* received up until this point.                      */
                  printf("    LengthReceived: %u.\r\n", (unsigned int)JobServerPortCurrentFileIndex);
               }
            }

            /* Check to see if this was the final piece of this         */
            /* tranaction.                                              */
            if(BPPEventData->Event_Data.BPP_File_Push_Indication_Data->Final)
            {
               /* This is the final piece of this transaction.  Reset   */
               /* the Current Operation and set the Response Code to    */
               /* indicate success.                                     */
               JobServerPortCurrentOperation = coNone;
               ResponseCode                  = BPP_OBEX_RESPONSE_OK;
            }

            /* Attempt to submit the response.                          */
            if((Result = BPP_File_Push_Response(BluetoothStackID, BPPEventData->Event_Data.BPP_File_Push_Indication_Data->BPPID, ResponseCode)) == 0)
               printf("BPP_File_Push_Response() Success.\r\n");
            else
            {
               /* An error occurred while attempting to submit the File */
               /* Push Response.                                        */
               printf("BPP_File_Push_Response() Failure: %d.\r\n", Result);
            }

            /* If an error occurred, or the operation was completed     */
            /* update the current operation.                            */
            if((Result != 0) || (ResponseCode != BPP_OBEX_RESPONSE_CONTINUE))
            {
               /* Reset the Current Operation.                          */
               JobServerPortCurrentOperation = coNone;
            }
            break;
         case etBPP_File_Push_Confirmation:
            /* A File Push Confirmation was received, display all       */
            /* relevant information.                                    */
            printf("BPP File Push Confirmation, ID: %u, ResponseCode: 0x%02X.\r\n", BPPEventData->Event_Data.BPP_File_Push_Confirmation_Data->BPPID, BPPEventData->Event_Data.BPP_File_Push_Confirmation_Data->ResponseCode);

            /* Check to see if this was the last phase of this          */
            /* operation.                                               */
            if(BPPEventData->Event_Data.BPP_File_Push_Confirmation_Data->ResponseCode == BPP_OBEX_RESPONSE_CONTINUE)
            {
               /* Get some more file data.                              */
               JobServerPortGetFileData();

               Result = BPP_File_Push_Request(BluetoothStackID, BPPEventData->Event_Data.BPP_File_Push_Confirmation_Data->BPPID, NULL, NULL, NULL, JobServerPortCurrentFileBufferIndex, JobServerPortCurrentFileBuffer, &AmountWritten, (Boolean_t)((JobServerPortCurrentFileBufferIndex == 0)?(TRUE):(FALSE)));

               if(!Result)
               {
                  /* The function was submitted successfully.           */
                  printf("BPP_File_Push_Request: Function Successful.\r\n");

                  /* Adjust the current file buffer.                    */
                  if(AmountWritten)
                  {
                     memmove(JobServerPortCurrentFileBuffer, &JobServerPortCurrentFileBuffer[AmountWritten], (JobServerPortCurrentFileBufferIndex - AmountWritten));
                     JobServerPortCurrentFileBufferIndex -= AmountWritten;
                  }
               }
               else
               {
                  /* There was an error submitting the function.        */
                  printf("BPP_File_Push_Request() Failure: %d.\r\n", Result);
               }
            }
            else
               Result = 0;

            /* If an error occurred, or the operation was completed     */
            /* update the current operation.                            */
            if((Result != 0) || (BPPEventData->Event_Data.BPP_File_Push_Confirmation_Data->ResponseCode != BPP_OBEX_RESPONSE_CONTINUE))
            {
               /* Reset the Current Operation.                          */
               JobServerPortCurrentOperation = coNone;
            }
            break;
         case etBPP_Get_Referenced_Objects_Indication:
            /* A Get Referenced Objects Indication was received, display*/
            /* all relevant information.                                */
            printf("BPP Get Referenced Objects Indication, ID: %u.\r\n", BPPEventData->Event_Data.BPP_Get_Referenced_Objects_Indication_Data->BPPID);

            /* Initialize the Response Code to an Error Response.       */
            ResponseCode = BPP_OBEX_RESPONSE_BAD_REQUEST;
            ObjectSize   = 0;

            /* Check to see if this is the first phase of the operation.*/
            if(ReferencedObjectsServerPortCurrentOperation == coNone)
            {
               /* Check to make sure that the requested object appears  */
               /* to be at least semi-valid.                            */
               if((BPPEventData->Event_Data.BPP_Get_Referenced_Objects_Indication_Data->ObjectIdentifier != NULL) && (strlen(BPPEventData->Event_Data.BPP_Get_Referenced_Objects_Indication_Data->ObjectIdentifier) > 0))
               {
                  /* The Object Identifier appears to be at least       */
                  /* semi-valid.                                        */
                  printf("    ObjectIdentifier: %s.\r\n", BPPEventData->Event_Data.BPP_Get_Referenced_Objects_Indication_Data->ObjectIdentifier);

                  /* Next display the requested file information.       */
                  printf("    Offset: %u.\r\n", (unsigned int)BPPEventData->Event_Data.BPP_Get_Referenced_Objects_Indication_Data->Offset);
                  printf("    Count: %u.\r\n", (unsigned int)BPPEventData->Event_Data.BPP_Get_Referenced_Objects_Indication_Data->Count);
                  printf("    RequestFileSize: %s.\r\n", (BPPEventData->Event_Data.BPP_Get_Referenced_Objects_Indication_Data->RequestFileSize)?("TRUE"):("FALSE"));

                  /* Check to make sure that the path to the new        */
                  /* filename can be successfully built.                */
                  if(sizeof(ReferencedObjectsServerPortCurrentFile) >= (strlen(RootDirectory) + strlen(BPPEventData->Event_Data.BPP_Get_Referenced_Objects_Indication_Data->ObjectIdentifier)))
                  {
                     /* Initialize the current file information.        */
                     ReferencedObjectsServerPortCurrentFileStartIndex   = BPPEventData->Event_Data.BPP_Get_Referenced_Objects_Indication_Data->Offset;
                     ReferencedObjectsServerPortCurrentFileIndex        = BPPEventData->Event_Data.BPP_Get_Referenced_Objects_Indication_Data->Offset;
                     ReferencedObjectsServerPortCurrentFileLengthToSend = BPPEventData->Event_Data.BPP_Get_Referenced_Objects_Indication_Data->Count;

                     ReferencedObjectsServerPortCurrentFileBufferIndex  = 0;

                     /* Build the full path to the storage location for */
                     /* the file being retrieved.                       */
                     strcpy(ReferencedObjectsServerPortCurrentFile, RootDirectory);
                     strcat(ReferencedObjectsServerPortCurrentFile, BPPEventData->Event_Data.BPP_Get_Referenced_Objects_Indication_Data->ObjectIdentifier);

                     /* Check to see if this is a request to get any    */
                     /* data.                                           */
                     if(BPPEventData->Event_Data.BPP_Get_Referenced_Objects_Indication_Data->Count > 0)
                     {
                        /* Attempt to get some file data to send.       */
                        if(ReferencedObjectServerPortGetFileData() == FILE_DATA_FILE_IO_SUCCESS)
                        {
                           /* Some data was successfully read from the  */
                           /* file, set the current operation and       */
                           /* response code.                            */
                           ReferencedObjectsServerPortCurrentOperation = coGetReferencedObjects;
                           ResponseCode                                = BPP_OBEX_RESPONSE_CONTINUE;

                           if(BPPEventData->Event_Data.BPP_Get_Referenced_Objects_Indication_Data->RequestFileSize)
                              ObjectSize = GetReferencedObjectServerPortFileSize();
                           else
                              ObjectSize = BPP_OBJECT_SIZE_INVALID_VALUE;
                        }
                     }
                     else
                     {
                        /* This is not a request to get data.  Instead  */
                        /* it could be a request to just get the file   */
                        /* size.  Check to see if this is a request to  */
                        /* get the file size.                           */
                        if(BPPEventData->Event_Data.BPP_Get_Referenced_Objects_Indication_Data->RequestFileSize)
                           ObjectSize = GetReferencedObjectServerPortFileSize();
                        else
                           ObjectSize = BPP_OBJECT_SIZE_INVALID_VALUE;

                        /* Simply set the Response Code to indicate     */
                        /* success.                                     */
                        ResponseCode = BPP_OBEX_RESPONSE_OK;
                     }
                  }
               }
            }
            else
            {
               /* This is not the first phase of the operation, check to*/
               /* see if this is a continuation of a currently on going */
               /* operation.                                            */
               if(ReferencedObjectsServerPortCurrentOperation == coGetReferencedObjects)
               {
                  /* Since we are not in the first phase of the         */
                  /* operation, set the object size to an invalid state */
                  /* since it only needs to be sent in the first phase  */
                  /* if it is requested.                                */
                  ObjectSize = BPP_OBJECT_SIZE_INVALID_VALUE;

                  /* Get some more file data.                           */
                  ReferencedObjectServerPortGetFileData();

                  /* Check to see if there is more file data to be sent.*/
                  if(ReferencedObjectsServerPortCurrentFileBufferIndex > 0)
                  {
                     /* There is more file data to be sent, in this case*/
                     /* the Response Code needs to be set to continue.  */
                     ResponseCode = BPP_OBEX_RESPONSE_CONTINUE;
                  }
                  else
                  {
                     /* There is no more file data to be sent, in this  */
                     /* case the Response Code needs to be set to ok.   */
                     ResponseCode = BPP_OBEX_RESPONSE_OK;
                  }
               }
            }

            /* Attempt to submit the response.                          */
            if((Result = BPP_Get_Referenced_Objects_Response(BluetoothStackID, BPPServerID, ResponseCode, ObjectSize, ReferencedObjectsServerPortCurrentFileBufferIndex, ReferencedObjectsServerPortCurrentFileBuffer, &AmountWritten)) == 0)
            {
               /* The function was submitted successfully.              */
               printf("BPP_Get_Referenced_Objects_Response() Success, CurrentFileStartIndex: %u, TotalRead: %u, AmountWritten: %u.\r\n", (unsigned int)ReferencedObjectsServerPortCurrentFileStartIndex, (unsigned int)(ReferencedObjectsServerPortCurrentFileIndex - ReferencedObjectsServerPortCurrentFileStartIndex), AmountWritten);

               /* Adjust the current file buffer.                       */
               if(AmountWritten)
               {
                  memmove(ReferencedObjectsServerPortCurrentFileBuffer, &ReferencedObjectsServerPortCurrentFileBuffer[AmountWritten], (ReferencedObjectsServerPortCurrentFileBufferIndex - AmountWritten));
                  ReferencedObjectsServerPortCurrentFileBufferIndex -= AmountWritten;
               }
            }
            else
            {
               /* There was an error submitting the function.           */
               printf("BPP_Get_Referenced_Objects_Response() Failure: %d.\r\n", Result);
            }

            /* If an error occurred, or the operation was completed     */
            /* update the current operation.                            */
            if((Result != 0) || (ResponseCode != BPP_OBEX_RESPONSE_CONTINUE))
            {
               /* Reset the Current Operation.                          */
               ReferencedObjectsServerPortCurrentOperation       = coNone;

               /* Reset the state information.                          */
               ReferencedObjectsServerPortFirstPhase             = TRUE;
               ReferencedObjectsServerPortCurrentFileIndex       = 0;
               ReferencedObjectsServerPortCurrentFileBufferIndex = 0;
            }
            break;
         case etBPP_Get_Referenced_Objects_Confirmation:
            /* A Get Referenced Objects Confirmation was received,      */
            /* display all relevant information.                        */
            printf("BPP Get Referenced Objects Confirmation, ID: %u, ResponseCode: 0x%02X, ObjectSize: %d.\r\n", BPPEventData->Event_Data.BPP_Get_Referenced_Objects_Confirmation_Data->BPPID, BPPEventData->Event_Data.BPP_Get_Referenced_Objects_Confirmation_Data->ResponseCode, (unsigned int)BPPEventData->Event_Data.BPP_Get_Referenced_Objects_Confirmation_Data->ObjectSize);

            /* Check for non-error response code.                       */
            if((BPPEventData->Event_Data.BPP_Get_Referenced_Objects_Confirmation_Data->ResponseCode == BPP_OBEX_RESPONSE_CONTINUE) || (BPPEventData->Event_Data.BPP_Get_Referenced_Objects_Confirmation_Data->ResponseCode == BPP_OBEX_RESPONSE_OK))
            {
               /* Check to see if this segment of the operation contains*/
               /* any retrieved data.                                   */
               if((BPPEventData->Event_Data.BPP_Get_Referenced_Objects_Confirmation_Data->DataLength > 0) && (BPPEventData->Event_Data.BPP_Get_Referenced_Objects_Confirmation_Data->DataBuffer != NULL))
               {
                  /* Put the new received file data to the file.        */
                  ReferencedObjectServerPortPutFileData(BPPEventData->Event_Data.BPP_Get_Referenced_Objects_Confirmation_Data->DataLength, BPPEventData->Event_Data.BPP_Get_Referenced_Objects_Confirmation_Data->DataBuffer);

                  /* Display the current file information.              */
                  printf("    DataLength: %u, CurrentFileIndex: %u.\r\n", BPPEventData->Event_Data.BPP_Get_Referenced_Objects_Confirmation_Data->DataLength, (unsigned int)ReferencedObjectsServerPortCurrentFileIndex);
               }

               /* Check to see if this was the last phase of this       */
               /* operation.                                            */
               if(BPPEventData->Event_Data.BPP_Get_Referenced_Objects_Confirmation_Data->ResponseCode == BPP_OBEX_RESPONSE_CONTINUE)
               {
                  /* The response code indicates there is more to the   */
                  /* object being received.  Request the next piece of  */
                  /* the object.                                        */
                  if((Result = BPP_Get_Referenced_Objects_Request(BluetoothStackID, BPPEventData->Event_Data.BPP_Get_Referenced_Objects_Confirmation_Data->BPPID, NULL, 0, BPP_COUNT_MINIMUM_VALUE, FALSE)) == 0)
                  {
                     /* The function was submitted successfully.        */
                     printf("BPP_Get_Referenced_Objects_Request: Function Successful.\r\n");
                  }
                  else
                  {
                     /* There was an error submitting the function.     */
                     printf("BPP_Get_Referenced_Objects_Request() Failure: %d.\r\n", Result);
                  }
               }
               else
                  Result = 1;
            }
            else
               Result = 0;

            /* If an error occurred, or the operation was completed     */
            /* update the current operation.                            */
            if((Result != 0) || (BPPEventData->Event_Data.BPP_Get_Referenced_Objects_Confirmation_Data->ResponseCode != BPP_OBEX_RESPONSE_CONTINUE))
            {
               /* Reset the Current Operation.                          */
               ReferencedObjectsServerPortCurrentOperation = coNone;

               /* Reset the state variables.                            */
               ReferencedObjectsServerPortCurrentFile[0]   = '\0';
               ReferencedObjectsServerPortCurrentFileIndex = 0;
            }
            break;
         case etBPP_Get_Printer_Attributes_Indication:
            /* A Get Printer Attributes Indication was received, display*/
            /* all relevant information.                                */
            printf("BPP Get Printer Attributes Indication, ID: %u, ServerPort: %u.\r\n", BPPEventData->Event_Data.BPP_Get_Printer_Attributes_Indication_Data->BPPID, BPPEventData->Event_Data.BPP_Get_Printer_Attributes_Indication_Data->ServerPort);

            /* Next check to see if the Data Elements associated with   */
            /* this event appear to be at least semi-valid.             */
            if((BPPEventData->Event_Data.BPP_Get_Printer_Attributes_Indication_Data->NumberDataElements > 0) && (BPPEventData->Event_Data.BPP_Get_Printer_Attributes_Indication_Data->DataElementBuffer != NULL))
            {
               /* The data elements associated with this event appear to*/
               /* be at least semi-valid.  Attempt to display the data  */
               /* elements.                                             */
               printf("   NumberDataElements: %u.\r\n", BPPEventData->Event_Data.BPP_Get_Printer_Attributes_Indication_Data->NumberDataElements);

               DisplaySOAPDataElements(BPPEventData->Event_Data.BPP_Get_Printer_Attributes_Indication_Data->NumberDataElements, BPPEventData->Event_Data.BPP_Get_Printer_Attributes_Indication_Data->DataElementBuffer, 0);
            }
            else
            {
               /* The data elements associated with this event do not   */
               /* appear to be at least semi-valid.  Display a message  */
               /* indicating this error.                                */
               printf("   NumberDataElements: %u.\r\n", BPPEventData->Event_Data.BPP_Get_Printer_Attributes_Indication_Data->NumberDataElements);
            }

            /* Check to see which port this Event was received on.      */
            if(BPPEventData->Event_Data.BPP_Get_Printer_Attributes_Indication_Data->ServerPort == JobServerPortValue)
            {
               /* The event was received on the Job Server Port.  Update*/
               /* the current operation for this port.                  */
               JobServerPortCurrentOperation = coGetPrinterAttributes;
            }
            else
            {
               if(BPPEventData->Event_Data.BPP_Get_Printer_Attributes_Indication_Data->ServerPort == StatusServerPortValue)
               {
                  /* The event was received on the Status Server Port.  */
                  /* Update the current operation for this port.        */
                  StatusServerPortCurrentOperation = coGetPrinterAttributes;
               }
            }
            break;
         case etBPP_Get_Printer_Attributes_Confirmation:
            /* A Get Printer Attributes Confirmation was received,      */
            /* display all relevant information.                        */
            printf("BPP Get Printer Attributes Confirmation, ID: %u, ServerPort: %u, ResponseCode: 0x%02X.\r\n", BPPEventData->Event_Data.BPP_Get_Printer_Attributes_Confirmation_Data->BPPID, BPPEventData->Event_Data.BPP_Get_Printer_Attributes_Confirmation_Data->ServerPort, BPPEventData->Event_Data.BPP_Get_Printer_Attributes_Confirmation_Data->ResponseCode);

            /* Next check to see if the Data Elements associated with   */
            /* this event appear to be at least semi-valid.             */
            if((BPPEventData->Event_Data.BPP_Get_Printer_Attributes_Confirmation_Data->NumberDataElements > 0) && (BPPEventData->Event_Data.BPP_Get_Printer_Attributes_Confirmation_Data->DataElementBuffer != NULL))
            {
               /* The data elements associated with this event appear to*/
               /* be at least semi-valid.  Attempt to display the data  */
               /* elements.                                             */
               printf("   NumberDataElements: %u.\r\n", BPPEventData->Event_Data.BPP_Get_Printer_Attributes_Confirmation_Data->NumberDataElements);

               DisplaySOAPDataElements(BPPEventData->Event_Data.BPP_Get_Printer_Attributes_Confirmation_Data->NumberDataElements, BPPEventData->Event_Data.BPP_Get_Printer_Attributes_Confirmation_Data->DataElementBuffer, 0);
            }
            else
            {
               /* The data elements associated with this event do not   */
               /* appear to be at least semi-valid.  Display a message  */
               /* indicating this error.                                */
               printf("   NumberDataElements: %u.\r\n", BPPEventData->Event_Data.BPP_Get_Printer_Attributes_Confirmation_Data->NumberDataElements);
            }

            /* Check to see which port this Event was received on.      */
            if(BPPEventData->Event_Data.BPP_Get_Printer_Attributes_Confirmation_Data->ServerPort == JobServerPortValue)
            {
               /* The event was received on the Job Server Port.  Update*/
               /* the current operation for this port.                  */
               JobServerPortCurrentOperation = coNone;
            }
            else
            {
               if(BPPEventData->Event_Data.BPP_Get_Printer_Attributes_Confirmation_Data->ServerPort == StatusServerPortValue)
               {
                  /* The event was received on the Status Server Port.  */
                  /* Update the current operation for this port.        */
                  StatusServerPortCurrentOperation = coNone;
               }
            }
            break;
         case etBPP_Create_Job_Indication:
            /* A Create Job Indication was received, display all        */
            /* relevant information.                                    */
            printf("BPP Create Job Indication, ID: %u.\r\n", BPPEventData->Event_Data.BPP_Create_Job_Indication_Data->BPPID);

            /* Next check to see if the Data Elements associated with   */
            /* this event appear to be at least semi-valid.             */
            if((BPPEventData->Event_Data.BPP_Create_Job_Indication_Data->NumberDataElements > 0) && (BPPEventData->Event_Data.BPP_Create_Job_Indication_Data->DataElementBuffer != NULL))
            {
               /* The data elements associated with this event appear to*/
               /* be at least semi-valid.  Attempt to display the data  */
               /* elements.                                             */
               printf("   NumberDataElements: %u.\r\n", BPPEventData->Event_Data.BPP_Create_Job_Indication_Data->NumberDataElements);

               DisplaySOAPDataElements(BPPEventData->Event_Data.BPP_Create_Job_Indication_Data->NumberDataElements, BPPEventData->Event_Data.BPP_Create_Job_Indication_Data->DataElementBuffer, 0);
            }
            else
            {
               /* The data elements associated with this event do not   */
               /* appear to be at least semi-valid.  Display a message  */
               /* indicating this error.                                */
               printf("   NumberDataElements: %u.\r\n", BPPEventData->Event_Data.BPP_Create_Job_Indication_Data->NumberDataElements);
            }

            /* The event was received on the Job Server Port.  Update   */
            /* the current operation for this port.                     */
            JobServerPortCurrentOperation = coCreateJob;
            break;
         case etBPP_Create_Job_Confirmation:
            /* A Create Job Confirmation was received, display all      */
            /* relevant information.                                    */
            printf("BPP Create Job Confirmation, ID: %u, ResponseCode: 0x%02X, JobId: %lu.\r\n", BPPEventData->Event_Data.BPP_Create_Job_Confirmation_Data->BPPID, BPPEventData->Event_Data.BPP_Create_Job_Confirmation_Data->ResponseCode, BPPEventData->Event_Data.BPP_Create_Job_Confirmation_Data->JobId);

            /* Next check to see if the Data Elements associated with   */
            /* this event appear to be at least semi-valid.             */
            if((BPPEventData->Event_Data.BPP_Create_Job_Confirmation_Data->NumberDataElements > 0) && (BPPEventData->Event_Data.BPP_Create_Job_Confirmation_Data->DataElementBuffer != NULL))
            {
               /* The data elements associated with this event appear to*/
               /* be at least semi-valid.  Attempt to display the data  */
               /* elements.                                             */
               printf("   NumberDataElements: %u.\r\n", BPPEventData->Event_Data.BPP_Create_Job_Confirmation_Data->NumberDataElements);

               DisplaySOAPDataElements(BPPEventData->Event_Data.BPP_Create_Job_Confirmation_Data->NumberDataElements, BPPEventData->Event_Data.BPP_Create_Job_Confirmation_Data->DataElementBuffer, 0);
            }
            else
            {
               /* The data elements associated with this event do not   */
               /* appear to be at least semi-valid.  Display a message  */
               /* indicating this error.                                */
               printf("   NumberDataElements: %u.\r\n", BPPEventData->Event_Data.BPP_Create_Job_Confirmation_Data->NumberDataElements);
            }

            /* The event was received on the Job Server Port.  Update   */
            /* the current operation for this port.                     */
            JobServerPortCurrentOperation = coNone;
            break;
         case etBPP_Send_Document_Indication:
            /* A Send Document Indication was received, display all     */
            /* relevant information.                                    */
            printf("BPP Send Document Indication, ID: %u.\r\n", BPPEventData->Event_Data.BPP_Send_Document_Indication_Data->BPPID);

            /* Initialize the Response Code to an Error Response.       */
            ResponseCode = BPP_OBEX_RESPONSE_BAD_REQUEST;

            /* Check to see if this is the first phase of the operation.*/
            if(JobServerPortCurrentOperation == coNone)
            {
               /* Display the JobId.                                    */
               printf("    JobId: %lu.\r\n", BPPEventData->Event_Data.BPP_Send_Document_Indication_Data->JobId);

               /* Check to see if the MIME Media Type appears to be     */
               /* valid.                                                */
               if(BPPEventData->Event_Data.BPP_Send_Document_Indication_Data->MIMEMediaType != NULL)
               {
                  /* The MIME Media Type is valid.  Display the MIME    */
                  /* Media Type.                                        */
                  printf("    MIME Media Type: %s.\r\n", BPPEventData->Event_Data.BPP_Send_Document_Indication_Data->MIMEMediaType);
               }

               /* Check to see if the File Name appears to be valid.    */
               if(BPPEventData->Event_Data.BPP_Send_Document_Indication_Data->FileName != NULL)
               {
                  strncpy(FileName, BPPEventData->Event_Data.BPP_Send_Document_Indication_Data->FileName, sizeof(FileName));
                  FileName[sizeof(FileName)-1] = '\0';
               }
               else
               {
                  /* A file name was not specified, generate a unique   */
                  /* file name to be used.                              */
                  if(GetTempFileName(".", "SS1", FileName) == 0)
                     FileName[0] = '\0';
               }

               if(strlen(FileName) > 0)
               {
                  /* The File Name is valid.  Display the File Name.    */
                  printf("    File Name: %s.\r\n", FileName);
               }

               /* Check to see if the Description appears to be valid.  */
               if(BPPEventData->Event_Data.BPP_Send_Document_Indication_Data->Description != NULL)
               {
                  /* Display the Description Received.                  */
                  printf("    Description: %s.\r\n", BPPEventData->Event_Data.BPP_Send_Document_Indication_Data->Description);
               }

               /* Check to see if a file name was specified, or         */
               /* successfully generated.                               */
               if(strlen(FileName) > 0)
               {
                  /* Check to make sure that the path to the new        */
                  /* filename can be successfully built.                */
                  if(sizeof(JobServerPortCurrentFile) >= (strlen(RootDirectory) + strlen(FileName) + 1))
                  {
                     /* Reset the current file index and set that this  */
                     /* is the first phase.                             */
                     JobServerPortCurrentFileIndex = 0;
                     JobServerPortFirstPhase       = TRUE;

                     /* Build the full path to the storage location for */
                     /* the file being received.                        */
                     strcpy(JobServerPortCurrentFile, RootDirectory);
                     strcat(JobServerPortCurrentFile, FileName);

                     /* Check to see if there is file data associated   */
                     /* with this transaction.                          */
                     if((BPPEventData->Event_Data.BPP_Send_Document_Indication_Data->DataLength > 0) && (BPPEventData->Event_Data.BPP_Send_Document_Indication_Data->DataBuffer != NULL))
                     {
                        /* There is file data associated with this      */
                        /* transaction.  Put the data to a file.        */
                        if(JobServerPortPutFileData(BPPEventData->Event_Data.BPP_Send_Document_Indication_Data->DataLength, BPPEventData->Event_Data.BPP_Send_Document_Indication_Data->DataBuffer) == FILE_DATA_FILE_IO_SUCCESS)
                        {
                           /* Data was successfully written to the file,*/
                           /* set the current operation and response    */
                           /* code.                                     */
                           JobServerPortCurrentOperation = coSendDocument;
                           ResponseCode                  = BPP_OBEX_RESPONSE_CONTINUE;
                        }
                     }
                     else
                     {
                        /* No file data to write.  Set the current      */
                        /* operation and response code.                 */
                        JobServerPortCurrentOperation = coSendDocument;
                        ResponseCode                  = BPP_OBEX_RESPONSE_CONTINUE;
                     }
                  }
               }
            }
            else
            {
               /* This is not the first phase of the operation, check to*/
               /* see if this is a continuation of a currently on going */
               /* operation.                                            */
               if(JobServerPortCurrentOperation == coSendDocument)
               {
                  /* This is a continuation of the currently ongoing    */
                  /* operation.  Check to see if the MIME Media Type    */
                  /* appears to be valid.                               */
                  if(BPPEventData->Event_Data.BPP_Send_Document_Indication_Data->MIMEMediaType != NULL)
                  {
                     /* The MIME Media Type is valid.  Display the MIME */
                     /* Media Type.                                     */
                     printf("    MIME Media Type: %s.\r\n", BPPEventData->Event_Data.BPP_Send_Document_Indication_Data->MIMEMediaType);
                  }

                  /* Check to see if the File Name appears to be valid. */
                  if((BPPEventData->Event_Data.BPP_Send_Document_Indication_Data->FileName != NULL) && (strlen(JobServerPortCurrentFile) > 0))
                  {
                     /* The File Name is valid.  Display the File Name. */
                     printf("    File Name: %s.\r\n", JobServerPortCurrentFile);
                  }

                  /* Check to see if the Description appears to be      */
                  /* valid.                                             */
                  if(BPPEventData->Event_Data.BPP_Send_Document_Indication_Data->Description != NULL)
                  {
                     /* Display the Description Received.               */
                     printf("    Description: %s.\r\n", BPPEventData->Event_Data.BPP_Send_Document_Indication_Data->Description);
                  }

                  /* Check to see if there is file data associated with */
                  /* this transaction.                                  */
                  if((BPPEventData->Event_Data.BPP_Send_Document_Indication_Data->DataLength > 0) && (BPPEventData->Event_Data.BPP_Send_Document_Indication_Data->DataBuffer != NULL))
                  {
                     /* There is file data associated with this         */
                     /* transaction.  Put the data to a file.           */
                     if(JobServerPortPutFileData(BPPEventData->Event_Data.BPP_Send_Document_Indication_Data->DataLength, BPPEventData->Event_Data.BPP_Send_Document_Indication_Data->DataBuffer) == FILE_DATA_FILE_IO_SUCCESS)
                     {
                        /* Set the response code to indicate success.   */
                        ResponseCode = BPP_OBEX_RESPONSE_CONTINUE;
                     }
                  }

                  /* Display a message indicating the total length      */
                  /* received up until this point.                      */
                  printf("    LengthReceived: %u.\r\n", (unsigned int)JobServerPortCurrentFileIndex);
               }
            }

            /* Check to see if this was the final piece of this         */
            /* tranaction.                                              */
            if(BPPEventData->Event_Data.BPP_Send_Document_Indication_Data->Final)
            {
               /* This is the final piece of this transaction.  Reset   */
               /* the Current Operation and set the Response Code to    */
               /* indicate success.                                     */
               JobServerPortCurrentOperation = coNone;
               ResponseCode                  = BPP_OBEX_RESPONSE_OK;
            }

            /* Attempt to submit the response.                          */
            if((Result = BPP_Send_Document_Response(BluetoothStackID, BPPEventData->Event_Data.BPP_Send_Document_Indication_Data->BPPID, ResponseCode)) == 0)
               printf("BPP_Send_Document_Response() Success.\r\n");
            else
            {
               /* An error occurred while attempting to submit the File */
               /* Push Response.                                        */
               printf("BPP_Send_Document_Response() Failure: %d.\r\n", Result);
            }

            /* If an error occurred, or the operation was completed     */
            /* update the current operation.                            */
            if((Result != 0) || (ResponseCode != BPP_OBEX_RESPONSE_CONTINUE))
            {
               /* Reset the Current Operation.                          */
               JobServerPortCurrentOperation = coNone;
            }
            break;
         case etBPP_Send_Document_Confirmation:
            /* A Send Document Confirmation was received, display all   */
            /* relevant information.                                    */
            printf("BPP Send Document Confirmation, ID: %u, ResponseCode: 0x%02X.\r\n", BPPEventData->Event_Data.BPP_Send_Document_Confirmation_Data->BPPID, BPPEventData->Event_Data.BPP_Send_Document_Confirmation_Data->ResponseCode);

            /* Check to see if this was the last phase of this          */
            /* operation.                                               */
            if(BPPEventData->Event_Data.BPP_Send_Document_Confirmation_Data->ResponseCode == BPP_OBEX_RESPONSE_CONTINUE)
            {
               /* Get some more file data.                              */
               JobServerPortGetFileData();

               Result = BPP_Send_Document_Request(BluetoothStackID, BPPEventData->Event_Data.BPP_Send_Document_Confirmation_Data->BPPID, 0, NULL, NULL, NULL, JobServerPortCurrentFileBufferIndex, JobServerPortCurrentFileBuffer, &AmountWritten, (Boolean_t)((JobServerPortCurrentFileBufferIndex == 0)?(TRUE):(FALSE)));

               if(!Result)
               {
                  /* The function was submitted successfully.           */
                  printf("BPP_Send_Document_Request: Function Successful.\r\n");

                  /* Adjust the current file buffer.                    */
                  if(AmountWritten)
                  {
                     memmove(JobServerPortCurrentFileBuffer, &JobServerPortCurrentFileBuffer[AmountWritten], (JobServerPortCurrentFileBufferIndex - AmountWritten));
                     JobServerPortCurrentFileBufferIndex -= AmountWritten;
                  }
               }
               else
               {
                  /* There was an error submitting the function.        */
                  printf("BPP_Send_Document_Request() Failure: %d.\r\n", Result);
               }
            }
            else
               Result = 0;

            /* If an error occurred, or the operation was completed     */
            /* update the current operation.                            */
            if((Result != 0) || (BPPEventData->Event_Data.BPP_Send_Document_Confirmation_Data->ResponseCode != BPP_OBEX_RESPONSE_CONTINUE))
            {
               /* Reset the Current Operation.                          */
               JobServerPortCurrentOperation = coNone;
            }
            break;
         case etBPP_Get_Job_Attributes_Indication:
            /* A Get Job Attributes Indication was received, display all*/
            /* relevant information.                                    */
            printf("BPP Get Job Attributes Indication, ID: %u, ServerPort: %u.\r\n", BPPEventData->Event_Data.BPP_Get_Job_Attributes_Indication_Data->BPPID, BPPEventData->Event_Data.BPP_Get_Job_Attributes_Indication_Data->ServerPort);

            /* Next check to see if the Data Elements associated with   */
            /* this event appear to be at least semi-valid.             */
            if((BPPEventData->Event_Data.BPP_Get_Job_Attributes_Indication_Data->NumberDataElements > 0) && (BPPEventData->Event_Data.BPP_Get_Job_Attributes_Indication_Data->DataElementBuffer != NULL))
            {
               /* The data elements associated with this event appear to*/
               /* be at least semi-valid.  Attempt to display the data  */
               /* elements.                                             */
               printf("   NumberDataElements: %u.\r\n", BPPEventData->Event_Data.BPP_Get_Job_Attributes_Indication_Data->NumberDataElements);

               DisplaySOAPDataElements(BPPEventData->Event_Data.BPP_Get_Job_Attributes_Indication_Data->NumberDataElements, BPPEventData->Event_Data.BPP_Get_Job_Attributes_Indication_Data->DataElementBuffer, 0);
            }
            else
            {
               /* The data elements associated with this event do not   */
               /* appear to be at least semi-valid.  Display a message  */
               /* indicating this error.                                */
               printf("   NumberDataElements: %u.\r\n", BPPEventData->Event_Data.BPP_Get_Job_Attributes_Indication_Data->NumberDataElements);
            }

            /* Check to see which port this Event was received on.      */
            if(BPPEventData->Event_Data.BPP_Get_Job_Attributes_Indication_Data->ServerPort == JobServerPortValue)
            {
               /* The event was received on the Job Server Port.  Update*/
               /* the current operation for this port.                  */
               JobServerPortCurrentOperation = coGetJobAttributes;
            }
            else
            {
               if(BPPEventData->Event_Data.BPP_Get_Job_Attributes_Indication_Data->ServerPort == StatusServerPortValue)
               {
                  /* The event was received on the Status Server Port.  */
                  /* Update the current operation for this port.        */
                  StatusServerPortCurrentOperation = coGetJobAttributes;
               }
            }
            break;
         case etBPP_Get_Job_Attributes_Confirmation:
            /* A Get Job Attributes Confirmation was received, display  */
            /* all relevant information.                                */
            printf("BPP Get Job Attributes Confirmation, ID: %u, ServerPort: %u, ResponseCode: 0x%02X.\r\n", BPPEventData->Event_Data.BPP_Get_Printer_Attributes_Confirmation_Data->BPPID, BPPEventData->Event_Data.BPP_Get_Job_Attributes_Confirmation_Data->ServerPort, BPPEventData->Event_Data.BPP_Get_Job_Attributes_Confirmation_Data->ResponseCode);

            /* Next check to see if the Data Elements associated with   */
            /* this event appear to be at least semi-valid.             */
            if((BPPEventData->Event_Data.BPP_Get_Job_Attributes_Confirmation_Data->NumberDataElements > 0) && (BPPEventData->Event_Data.BPP_Get_Job_Attributes_Confirmation_Data->DataElementBuffer != NULL))
            {
               /* The data elements associated with this event appear to*/
               /* be at least semi-valid.  Attempt to display the data  */
               /* elements.                                             */
               printf("   NumberDataElements: %u.\r\n", BPPEventData->Event_Data.BPP_Get_Job_Attributes_Confirmation_Data->NumberDataElements);

               DisplaySOAPDataElements(BPPEventData->Event_Data.BPP_Get_Job_Attributes_Confirmation_Data->NumberDataElements, BPPEventData->Event_Data.BPP_Get_Job_Attributes_Confirmation_Data->DataElementBuffer, 0);
            }
            else
            {
               /* The data elements associated with this event do not   */
               /* appear to be at least semi-valid.  Display a message  */
               /* indicating this error.                                */
               printf("   NumberDataElements: %u.\r\n", BPPEventData->Event_Data.BPP_Get_Job_Attributes_Confirmation_Data->NumberDataElements);
            }

            /* Check to see which port this Event was received on.      */
            if(BPPEventData->Event_Data.BPP_Get_Job_Attributes_Confirmation_Data->ServerPort == JobServerPortValue)
            {
               /* The event was received on the Job Server Port.  Update*/
               /* the current operation for this port.                  */
               JobServerPortCurrentOperation = coNone;
            }
            else
            {
               if(BPPEventData->Event_Data.BPP_Get_Job_Attributes_Confirmation_Data->ServerPort == StatusServerPortValue)
               {
                  /* The event was received on the Status Server Port.  */
                  /* Update the current operation for this port.        */
                  StatusServerPortCurrentOperation = coNone;
               }
            }
            break;
         case etBPP_Cancel_Job_Indication:
            /* A Cancel Job Indication was received, display all        */
            /* relevant information.                                    */
            printf("BPP Cancel Job Indication, ID: %u, ServerPort: %u.\r\n", BPPEventData->Event_Data.BPP_Cancel_Job_Indication_Data->BPPID, BPPEventData->Event_Data.BPP_Cancel_Job_Indication_Data->ServerPort);

            /* Next check to see if the Data Elements associated with   */
            /* this event appear to be at least semi-valid.             */
            if((BPPEventData->Event_Data.BPP_Cancel_Job_Indication_Data->NumberDataElements > 0) && (BPPEventData->Event_Data.BPP_Cancel_Job_Indication_Data->DataElementBuffer != NULL))
            {
               /* The data elements associated with this event appear to*/
               /* be at least semi-valid.  Attempt to display the data  */
               /* elements.                                             */
               printf("   NumberDataElements: %u.\r\n", BPPEventData->Event_Data.BPP_Cancel_Job_Indication_Data->NumberDataElements);

               DisplaySOAPDataElements(BPPEventData->Event_Data.BPP_Cancel_Job_Indication_Data->NumberDataElements, BPPEventData->Event_Data.BPP_Cancel_Job_Indication_Data->DataElementBuffer, 0);
            }
            else
            {
               /* The data elements associated with this event do not   */
               /* appear to be at least semi-valid.  Display a message  */
               /* indicating this error.                                */
               printf("   NumberDataElements: %u.\r\n", BPPEventData->Event_Data.BPP_Cancel_Job_Indication_Data->NumberDataElements);
            }

            /* Check to see which port this Event was received on.      */
            if(BPPEventData->Event_Data.BPP_Get_Job_Attributes_Indication_Data->ServerPort == JobServerPortValue)
            {
               /* The event was received on the Job Server Port.  Update*/
               /* the current operation for this port.                  */
               JobServerPortCurrentOperation = coCancelJob;
            }
            else
            {
               if(BPPEventData->Event_Data.BPP_Get_Job_Attributes_Indication_Data->ServerPort == StatusServerPortValue)
               {
                  /* The event was received on the Status Server Port.  */
                  /* Update the current operation for this port.        */
                  StatusServerPortCurrentOperation = coCancelJob;
               }
            }
            break;
         case etBPP_Cancel_Job_Confirmation:
            /* A Cancel Job Confirmation was received, display all      */
            /* relevant information.                                    */
            printf("BPP Cancel Job Attributes Confirmation, ID: %u, ServerPort: %u, ResponseCode: 0x%02X.\r\n", BPPEventData->Event_Data.BPP_Cancel_Job_Confirmation_Data->BPPID, BPPEventData->Event_Data.BPP_Cancel_Job_Confirmation_Data->ServerPort, BPPEventData->Event_Data.BPP_Cancel_Job_Confirmation_Data->ResponseCode);

            /* Next check to see if the Data Elements associated with   */
            /* this event appear to be at least semi-valid.             */
            if((BPPEventData->Event_Data.BPP_Cancel_Job_Confirmation_Data->NumberDataElements > 0) && (BPPEventData->Event_Data.BPP_Cancel_Job_Confirmation_Data->DataElementBuffer != NULL))
            {
               /* The data elements associated with this event appear to*/
               /* be at least semi-valid.  Attempt to display the data  */
               /* elements.                                             */
               printf("   NumberDataElements: %u.\r\n", BPPEventData->Event_Data.BPP_Cancel_Job_Confirmation_Data->NumberDataElements);

               DisplaySOAPDataElements(BPPEventData->Event_Data.BPP_Cancel_Job_Confirmation_Data->NumberDataElements, BPPEventData->Event_Data.BPP_Cancel_Job_Confirmation_Data->DataElementBuffer, 0);
            }
            else
            {
               /* The data elements associated with this event do not   */
               /* appear to be at least semi-valid.  Display a message  */
               /* indicating this error.                                */
               printf("   NumberDataElements: %u.\r\n", BPPEventData->Event_Data.BPP_Cancel_Job_Confirmation_Data->NumberDataElements);
            }

            /* Check to see which port this Event was received on.      */
            if(BPPEventData->Event_Data.BPP_Get_Job_Attributes_Confirmation_Data->ServerPort == JobServerPortValue)
            {
               /* The event was received on the Job Server Port.  Update*/
               /* the current operation for this port.                  */
               JobServerPortCurrentOperation = coNone;
            }
            else
            {
               if(BPPEventData->Event_Data.BPP_Get_Job_Attributes_Confirmation_Data->ServerPort == StatusServerPortValue)
               {
                  /* The event was received on the Status Server Port.  */
                  /* Update the current operation for this port.        */
                  StatusServerPortCurrentOperation = coNone;
               }
            }
            break;
         case etBPP_Get_Event_Indication:
            /* A Get Event Indication was received, display all relevant*/
            /* information.                                             */
            printf("BPP Get Event Indication, ID: %u.\r\n", BPPEventData->Event_Data.BPP_Get_Event_Indication_Data->BPPID);

            /* Next check to see if the Data Elements associated with   */
            /* this event appear to be at least semi-valid.             */
            if((BPPEventData->Event_Data.BPP_Get_Event_Indication_Data->NumberDataElements > 0) && (BPPEventData->Event_Data.BPP_Get_Event_Indication_Data->DataElementBuffer != NULL))
            {
               /* The data elements associated with this event appear to*/
               /* be at least semi-valid.  Attempt to display the data  */
               /* elements.                                             */
               printf("   NumberDataElements: %u.\r\n", BPPEventData->Event_Data.BPP_Get_Event_Indication_Data->NumberDataElements);

               DisplaySOAPDataElements(BPPEventData->Event_Data.BPP_Get_Event_Indication_Data->NumberDataElements, BPPEventData->Event_Data.BPP_Get_Event_Indication_Data->DataElementBuffer, 0);
            }
            else
            {
               /* The data elements associated with this event do not   */
               /* appear to be at least semi-valid.  Display a message  */
               /* indicating this error.                                */
               printf("   NumberDataElements: %u.\r\n", BPPEventData->Event_Data.BPP_Get_Event_Indication_Data->NumberDataElements);
            }

            /* The event was received on the Status Server Port.  Update*/
            /* the current operation for this port.                     */
            StatusServerPortCurrentOperation = coGetEvent;
            break;
         case etBPP_Get_Event_Confirmation:
            /* A Get Event Confirmation was received, display all       */
            /* relevant information.                                    */
            printf("BPP Get Event Confirmation, ID: %u, ResponseCode: 0x%02X.\r\n", BPPEventData->Event_Data.BPP_Get_Event_Confirmation_Data->BPPID, BPPEventData->Event_Data.BPP_Get_Event_Confirmation_Data->ResponseCode);

            /* Next check to see if the Data Elements associated with   */
            /* this event appear to be at least semi-valid.             */
            if((BPPEventData->Event_Data.BPP_Get_Event_Confirmation_Data->NumberDataElements > 0) && (BPPEventData->Event_Data.BPP_Get_Event_Confirmation_Data->DataElementBuffer != NULL))
            {
               /* The data elements associated with this event appear to*/
               /* be at least semi-valid.  Attempt to display the data  */
               /* elements.                                             */
               printf("   NumberDataElements: %u.\r\n", BPPEventData->Event_Data.BPP_Get_Event_Confirmation_Data->NumberDataElements);

               DisplaySOAPDataElements(BPPEventData->Event_Data.BPP_Get_Event_Confirmation_Data->NumberDataElements, BPPEventData->Event_Data.BPP_Get_Event_Confirmation_Data->DataElementBuffer, 0);
            }
            else
            {
               /* The data elements associated with this event do not   */
               /* appear to be at least semi-valid.  Display a message  */
               /* indicating this error.                                */
               printf("   NumberDataElements: %u.\r\n", BPPEventData->Event_Data.BPP_Get_Event_Confirmation_Data->NumberDataElements);
            }

            /* The event was received on the Status Server Port.  Update*/
            /* the current operation for this port.                     */
            StatusServerPortCurrentOperation = coNone;
            break;
         default:
            /* Do Nothing.                                              */
            break;
      }

      if(ServiceType == stPrinter)
         printf("\r\nPrinter>");
      else
         printf("\r\nSender>");

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
         ServiceType = strtol(argv[2], &endptr, 10)?stSender:stPrinter;

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
                     /* Printer or Sender Mode.                         */
                     if(ServiceType == stPrinter)
                     {
                        /* The Device is now Connectable, Discoverable, */
                        /* and Pairable so start the User Interface.    */
                        UserInterface_Printer();
                     }
                     else
                     {
                        /* The Device is now Connectable, Discoverable, */
                        /* and Pairable so start the User Interface.    */
                        UserInterface_Sender();
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
         printf("Parameter Error (Expecting = [USB = 0, UART = 1, BCSP = 2 Flag] [Printer/Sender Flag (0 = Printer)] [IF !USB [Comm Port or Device File] [Baud Rate]])\r\n");
      }
   }
   else
   {
      /* An invalid number of parameters was entered on the Command     */
      /* Line.                                                          */
      printf("Parameter Error (Expecting = [USB = 0, UART = 1, BCSP = 2 Flag] [Printer/Sender Flag (0 = Printer)] [IF !USB [Comm Port or Device File] [Baud Rate]])\r\n");
   }

   return 0;
}

