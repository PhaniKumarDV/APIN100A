/*****< linuxspple.c >*********************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  LINUXSPPLE - Linux Bluetooth SPP Emulation using GATT (LE) application.   */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   04/16/12  Tim Cook       Initial creation.                               */
/******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "LinuxSPPLE42.h"    /* Main Application Prototypes and Constants.    */

#include "SS1BTPS.h"       /* Includes for the SS1 Bluetooth Protocol Stack.  */
#include "SS1BTDBG.h"      /* Includes/Constants for Bluetooth Debugging.     */
#include "SS1BTGAT.h"      /* Includes for the SS1 GATT Profile.              */
#include "SS1BTGAP.h"      /* Includes for the SS1 GAP Service.               */
#include "SS1BTDIS.h"      /* Inlcudes for Device Information Service.        */

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

#define MAX_SUPPORTED_COMMANDS                     (64)  /* Denotes the       */
                                                         /* maximum number of */
                                                         /* User Commands that*/
                                                         /* are supported by  */
                                                         /* this application. */

#define MAX_COMMAND_LENGTH                         (64)  /* Denotes the max   */
                                                         /* buffer size used  */
                                                         /* for user commands */
                                                         /* input via the     */
                                                         /* User Interface.   */

#define MAX_NUM_OF_PARAMETERS                       (8)  /* Denotes the max   */
                                                         /* number of         */
                                                         /* parameters a      */
                                                         /* command can have. */

#define DEFAULT_IO_CAPABILITY      (licNoInputNoOutput)  /* Denotes the       */
                                                         /* default I/O       */
                                                         /* Capability that is*/
                                                         /* used with Pairing.*/

#define DEFAULT_MITM_PROTECTION                  (TRUE)  /* Denotes the       */
                                                         /* default value used*/
                                                         /* for Man in the    */
                                                         /* Middle (MITM)     */
                                                         /* protection used   */
                                                         /* with Secure Simple*/
                                                         /* Pairing.          */

#define DEFAULT_SECURE_CONNECTIONS               (TRUE)  /* Denotes the       */
                                                         /* default value used*/
                                                         /* for Secure        */
                                                         /* Connections.      */

#define SPPLE_DATA_BUFFER_LENGTH  (BTPS_CONFIGURATION_GATT_DEFAULT_MAXIMUM_SUPPORTED_MTU_SIZE)
                                                         /* Defines the length*/
                                                         /* of a SPPLE Data   */
                                                         /* Buffer.           */

#define SPPLE_DATA_CREDITS        (SPPLE_DATA_BUFFER_LENGTH*3) /* Defines the */
                                                         /* number of credits */
                                                         /* in an SPPLE Buffer*/

#define LED_TOGGLE_RATE_SUCCESS                   (500)  /* The LED Toggle    */
                                                         /* rate when the demo*/
                                                         /* successfully      */
                                                         /* starts up.        */

#define CONSOLE_MONITOR_RATE                       (10)  /* The rate at which */
                                                         /* the console will  */
                                                         /* be tested for     */
                                                         /* input.            */

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
                                                         /* occurred due to   */
                                                         /* attempted         */
                                                         /* execution of a    */
                                                         /* Command when a    */
                                                         /* Bluetooth Protocol*/
                                                         /* Stack has not been*/
                                                         /* opened.           */

#define UNABLE_TO_REGISTER_SERVER                  (-9)  /* Denotes that an   */
                                                         /* error occurred    */
                                                         /* when trying to    */
                                                         /* create a Serial   */
                                                         /* Port Server.      */

#define EXIT_MODE                                  (-10) /* Flags exit from   */
                                                         /* any Mode.         */

   /* The following constants represent the default log file names that */
   /* are used if no Log file name is specified when enabling debug.    */
#define DEFAULT_DEBUG_LOG_FILE_NAME                "LinuxSPPLE_ASC.log"
#define DEFAULT_DEBUG_FTS_FILE_NAME                "LinuxSPPLE_FTS.log"

   /* Determine the Name we will use for this compilation.              */
#define LE_DEMO_DEVICE_NAME                        "LinuxSPPLE42"

   /* The following type definition represents the structure which holds*/
   /* all information about the parameter, in particular the parameter  */
   /* as a string and the parameter as an unsigned int.                 */
typedef struct _tagParameter_t
{
   char     *strParam;
   SDWord_t  intParam;
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

   /* Structure used to hold all of the GAP LE Parameters.              */
typedef struct _tagGAPLE_Parameters_t
{
   GAP_LE_Connectability_Mode_t ConnectableMode;
   GAP_Discoverability_Mode_t   DiscoverabilityMode;
   GAP_LE_IO_Capability_t       IOCapability;
   Boolean_t                    MITMProtection;
   Boolean_t                    SecureConnections;
   Boolean_t                    OOBDataPresent;
} GAPLE_Parameters_t;

#define GAPLE_PARAMETERS_DATA_SIZE                       (sizeof(GAPLE_Parameters_t))

   /* The following structure holds status information about a send     */
   /* process.                                                          */
typedef struct _tagSend_Info_t
{
   DWord_t BytesToSend;
   DWord_t BytesSent;
} Send_Info_t;

   /* The following defines the format of a SPPLE Data Buffer.          */
typedef struct __tagSPPLE_Data_Buffer_t
{
   unsigned int  InIndex;
   unsigned int  OutIndex;
   unsigned int  BytesFree;
   unsigned int  BufferSize;
   Byte_t        Buffer[SPPLE_DATA_BUFFER_LENGTH*3];
} SPPLE_Data_Buffer_t;

   /* The following structure represents the information we will store  */
   /* on a Discovered GAP Service.                                      */
typedef struct _tagGAPS_Client_Info_t
{
   Word_t DeviceNameHandle;
   Word_t DeviceAppearanceHandle;
} GAPS_Client_Info_t;

   /* The following structure holds information on known Device         */
   /* Appearance Values.                                                */
typedef struct _tagGAPS_Device_Appearance_Mapping_t
{
   Word_t  Appearance;
   char   *String;
} GAPS_Device_Appearance_Mapping_t;

#define DIS_MAN_NAME_HANDLE_OFFSET                 0
#define DIS_MODEL_NUM_HANDLE_OFFSET                1
#define DIS_SERIAL_NUM_HANDLE_OFFSET               2
#define DIS_HARDWARE_REV_HANDLE_OFFSET             3
#define DIS_FIRMWARE_REV_HANDLE_OFFSET             4
#define DIS_SOFTWARE_REV_HANDLE_OFFSET             5
#define DIS_SYSTEM_ID_HANDLE_OFFSET                6
#define DIS_IEEE_CERT_HANDLE_OFFSET                7
#define DIS_PNP_ID_HANDLE_OFFSET                   8

   /* The following structure holds information on known Device         */
   /* Information Service handles.                                      */
typedef struct _tagDIS_Client_Info_t
{
   Word_t ManufacturerNameHandle;
   Word_t ModelNumberHandle;
   Word_t SerialNumberHandle;
   Word_t HardwareRevisionHandle;
   Word_t FirmwareRevisionHandle;
   Word_t SoftwareRevisionHandle;
   Word_t SystemIDHandle;
   Word_t IEEE11073CertHandle;
   Word_t PnPIDHandle;
} DIS_Client_Info_t;

   /* The following structure for a Master is used to hold a list of    */
   /* information on all paired devices. For slave we will not use this */
   /* structure.                                                        */
typedef struct _tagDeviceInfo_t
{
   Word_t                   Flags;
   Byte_t                   EncryptionKeySize;
   GAP_LE_Address_Type_t    ConnectionAddressType;
   BD_ADDR_t                ConnectionBD_ADDR;
   GAP_LE_Address_Type_t    LastKnownResolvableAddressType;
   BD_ADDR_t                LastKnownResolvableAddress;
   GAP_LE_Address_Type_t    PeerAddressType;
   BD_ADDR_t                PeerAddress;
   Encryption_Key_t         PeerIRK;
   Long_Term_Key_t          LTK;
   Random_Number_t          Rand;
   Word_t                   EDIV;
   unsigned int             TransmitCredits;
   SPPLE_Data_Buffer_t      ReceiveBuffer;
   SPPLE_Data_Buffer_t      TransmitBuffer;
   int                      DISHandleIndex;
   DIS_Client_Info_t        DISClientInfo;
   GAPS_Client_Info_t       GAPSClientInfo;
   SPPLE_Client_Info_t      ClientInfo;
   SPPLE_Server_Info_t      ServerInfo;
   struct _tagDeviceInfo_t *NextDeviceInfoPtr;
} DeviceInfo_t;

#define DEVICE_INFO_DATA_SIZE                            (sizeof(DeviceInfo_t))

   /* Defines the bitmask flags that may be set in the DeviceInfo_t     */
   /* structure.                                                        */
#define DEVICE_INFO_FLAGS_LTK_VALID                         0x0001
#define DEVICE_INFO_FLAGS_SPPLE_SERVER                      0x0002
#define DEVICE_INFO_FLAGS_SPPLE_CLIENT                      0x0004
#define DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_OUTSTANDING     0x0008
#define DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_DIS             0x0020
#define DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_GAPS            0x0040
#define DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_SPPLE           0x0080
#define DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_IDLE            0x0100
#define DEVICE_INFO_FLAGS_LINK_ENCRYPTED                    0x0200
#define DEVICE_INFO_FLAGS_PEER_IDENTITY_INFO_VALID          0x0400
#define DEVICE_INFO_FLAGS_LAST_RESOLVABLE_VALID             0x0800
#define DEVICE_INFO_FLAGS_DEVICE_IN_RESOLVING_LIST          0x1000
#define DEVICE_INFO_FLAGS_DEVICE_IN_WHITE_LIST              0x2000

   /* User to represent a structure to hold a BD_ADDR return from       */
   /* BD_ADDRToStr.                                                     */
typedef char BoardStr_t[16];

                        /* The Encryption Root Key should be generated  */
                        /* in such a way as to guarantee 128 bits of    */
                        /* entropy.                                     */
static BTPSCONST Encryption_Key_t ER;

                        /* The Identity Root Key should be generated    */
                        /* in such a way as to guarantee 128 bits of    */
                        /* entropy.                                     */
static BTPSCONST Encryption_Key_t IR;

                        /* The following keys can be regenerated on the */
                        /* fly using the constant IR and ER keys and    */
                        /* are used globally, for all devices.          */
static Encryption_Key_t DHK;
static Encryption_Key_t IRK;
static Encryption_Key_t CSRK;

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the         */
   /* compiler as part of standard C/C++).                              */
static GAP_LE_Resolving_List_Entry_t  ResolvingListEntry; /* Variable to hold the last */
                                                    /* added Resolving List for use in */
                                                    /* non-directed RPA advertising.   */

static Boolean_t           SecureConnectionsOnly;   /* Boolean that tells if only      */
                                                    /* Secure Connections mode is      */
                                                    /* enabled.                        */
   
static Byte_t              SPPLEBuffer[SPPLE_DATA_BUFFER_LENGTH+1];  /* Buffer that is */
                                                    /* used for Sending/Receiving      */
                                                    /* SPPLE Service Data.             */

static unsigned int        SPPLEServiceID;          /* The following holds the SPP LE  */
                                                    /* Service ID that is returned from*/
                                                    /* GATT_Register_Service().        */

static unsigned int        DISInstanceID;           /* Holds the Instance ID for the   */
                                                    /* DIS Service.                    */

static unsigned int        GAPSInstanceID;          /* Holds the Instance ID for the   */
                                                    /* GAP Service.                    */

static GAPLE_Parameters_t  LE_Parameters;           /* Holds GAP Parameters like       */
                                                    /* Discoverability, Connectability */
                                                    /* Modes.                          */

static DeviceInfo_t       *DeviceInfoList;          /* Holds the list head for the     */
                                                    /* device info list.               */

static unsigned int        BluetoothStackID;        /* Variable which holds the Handle */
                                                    /* of the opened Bluetooth Protocol*/
                                                    /* Stack.                          */
                                                    
static BD_ADDR_t           Resolvable_BD_ADDR;      /* Holds the resolvable address of */
                                                    /* the Bluetooth device.           */

static BD_ADDR_t           ConnectionBD_ADDR;       /* Holds the BD_ADDR of the        */
                                                    /* currently connected device.     */
                                                    
static GAP_LE_Address_Type_t ConnectionAddressType; /* Holds the Address Type of the   */
                                                    /* currently connected device.     */

static unsigned int        ConnectionID;            /* Holds the Connection ID of the  */
                                                    /* currently connected device.     */

static Boolean_t           LocalDeviceIsMaster;     /* Boolean that tells if the local */
                                                    /* device is the master of the     */
                                                    /* current connection.             */

static BD_ADDR_t           CurrentRemoteBD_ADDR;    /* Variable which holds the        */
                                                    /* current BD_ADDR of the device   */
                                                    /* which is currently pairing or   */
                                                    /* authenticating.                 */

static unsigned int        NumberCommands;          /* Variable which is used to hold  */
                                                    /* the number of Commands that are */
                                                    /* supported by this application.  */
                                                    /* Commands are added individually.*/

static CommandTable_t      CommandTable[MAX_SUPPORTED_COMMANDS]; /* Variable which is  */
                                                    /* used to hold the actual Commands*/
                                                    /* that are supported by this      */
                                                    /* application.                    */

static unsigned int        DebugID;                 /* Variable which holds the ID of  */
                                                    /* the currently enabled debugging */
                                                    /* session.                        */

static Boolean_t           ScanInProgress;          /* A boolean flag to show if a scan*/
                                                    /* is in process                   */

static Send_Info_t         SendInfo;                /* Variable that contains          */
                                                    /* information about a data        */
                                                    /* transfer process.               */

static Boolean_t           LoopbackActive;          /* Variable which flags whether or */
                                                    /* not the application is currently*/
                                                    /* operating in Loopback Mode      */
                                                    /* (TRUE) or not (FALSE).          */

static Boolean_t           DisplayRawData;          /* Variable which flags whether or */
                                                    /* not the application is to       */
                                                    /* simply display the Raw Data     */
                                                    /* when it is received (when not   */
                                                    /* operating in Loopback Mode).    */

static Boolean_t           AutomaticReadActive;     /* Variable which flags whether or */
                                                    /* not the application is to       */
                                                    /* automatically read all data     */
                                                    /* as it is received.              */

static char                DisBuf[DIS_MAXIMUM_SUPPORTED_STRING+1];

   /* The following is used to map from ATT Error Codes to a printable  */
   /* string.                                                           */
static char *ErrorCodeStr[] =
{
   "ATT_PROTOCOL_ERROR_CODE_NO_ERROR",
   "ATT_PROTOCOL_ERROR_CODE_INVALID_HANDLE",
   "ATT_PROTOCOL_ERROR_CODE_READ_NOT_PERMITTED",
   "ATT_PROTOCOL_ERROR_CODE_WRITE_NOT_PERMITTED",
   "ATT_PROTOCOL_ERROR_CODE_INVALID_PDU",
   "ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_AUTHENTICATION",
   "ATT_PROTOCOL_ERROR_CODE_REQUEST_NOT_SUPPORTED",
   "ATT_PROTOCOL_ERROR_CODE_INVALID_OFFSET",
   "ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_AUTHORIZATION",
   "ATT_PROTOCOL_ERROR_CODE_PREPARE_QUEUE_FULL",
   "ATT_PROTOCOL_ERROR_CODE_ATTRIBUTE_NOT_FOUND",
   "ATT_PROTOCOL_ERROR_CODE_ATTRIBUTE_NOT_LONG",
   "ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_ENCRYPTION_KEY_SIZE",
   "ATT_PROTOCOL_ERROR_CODE_INVALID_ATTRIBUTE_VALUE_LENGTH",
   "ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR",
   "ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_ENCRYPTION",
   "ATT_PROTOCOL_ERROR_CODE_UNSUPPORTED_GROUP_TYPE",
   "ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_RESOURCES"
} ;

#define NUMBER_GATT_ERROR_CODES     (sizeof(ErrorCodeStr)/sizeof(char *))

   /* The following array is used to map Device Appearance Values to    */
   /* strings.                                                          */
static GAPS_Device_Appearance_Mapping_t AppearanceMappings[] =
{
   { GAP_DEVICE_APPEARENCE_VALUE_UNKNOWN,                        "Unknown"                   },
   { GAP_DEVICE_APPEARENCE_VALUE_GENERIC_PHONE,                  "Generic Phone"             },
   { GAP_DEVICE_APPEARENCE_VALUE_GENERIC_COMPUTER,               "Generic Computer"          },
   { GAP_DEVICE_APPEARENCE_VALUE_GENERIC_WATCH,                  "Generic Watch"             },
   { GAP_DEVICE_APPEARENCE_VALUE_SPORTS_WATCH,                   "Sports Watch"              },
   { GAP_DEVICE_APPEARENCE_VALUE_GENERIC_CLOCK,                  "Generic Clock"             },
   { GAP_DEVICE_APPEARENCE_VALUE_GENERIC_DISPLAY,                "Generic Display"           },
   { GAP_DEVICE_APPEARENCE_VALUE_GENERIC_GENERIC_REMOTE_CONTROL, "Generic Remote Control"    },
   { GAP_DEVICE_APPEARENCE_VALUE_GENERIC_EYE_GLASSES,            "Eye Glasses"               },
   { GAP_DEVICE_APPEARENCE_VALUE_GENERIC_TAG,                    "Generic Tag"               },
   { GAP_DEVICE_APPEARENCE_VALUE_GENERIC_KEYRING,                "Generic Keyring"           },
   { GAP_DEVICE_APPEARENCE_VALUE_GENERIC_MEDIA_PLAYER,           "Generic Media Player"      },
   { GAP_DEVICE_APPEARENCE_VALUE_GENERIC_BARCODE_SCANNER,        "Generic Barcode Scanner"   },
   { GAP_DEVICE_APPEARENCE_VALUE_GENERIC_THERMOMETER,            "Generic Thermometer"       },
   { GAP_DEVICE_APPEARENCE_VALUE_THERMOMETER_EAR,                "Ear Thermometer"           },
   { GAP_DEVICE_APPEARENCE_VALUE_GENERIC_HEART_RATE_SENSOR,      "Generic Heart Rate Sensor" },
   { GAP_DEVICE_APPEARENCE_VALUE_BELT_HEART_RATE_SENSOR,         "Belt Heart Rate Sensor"    },
   { GAP_DEVICE_APPEARENCE_VALUE_GENERIC_BLOOD_PRESSURE,         "Generic Blood Pressure"    },
   { GAP_DEVICE_APPEARENCE_VALUE_BLOOD_PRESSURE_ARM,             "Blood Pressure: ARM"       },
   { GAP_DEVICE_APPEARENCE_VALUE_BLOOD_PRESSURE_WRIST,           "Blood Pressure: Wrist"     },
   { GAP_DEVICE_APPEARENCE_VALUE_HUMAN_INTERFACE_DEVICE,         "Human Interface Device"    },
   { GAP_DEVICE_APPEARENCE_VALUE_HID_KEYBOARD,                   "HID Keyboard"              },
   { GAP_DEVICE_APPEARENCE_VALUE_HID_MOUSE,                      "HID Mouse"                 },
   { GAP_DEVICE_APPEARENCE_VALUE_HID_JOYSTICK,                   "HID Joystick"              },
   { GAP_DEVICE_APPEARENCE_VALUE_HID_GAMEPAD,                    "HID Gamepad"               },
   { GAP_DEVICE_APPEARENCE_VALUE_HID_DIGITIZER_TABLET,           "HID Digitizer Tablet"      },
   { GAP_DEVICE_APPEARENCE_VALUE_HID_CARD_READER,                "HID Card Reader"           },
   { GAP_DEVICE_APPEARENCE_VALUE_HID_DIGITAL_PEN,                "HID Digitizer Pen"         },
   { GAP_DEVICE_APPEARENCE_VALUE_HID_BARCODE_SCANNER,            "HID Bardcode Scanner"      },
   { GAP_DEVICE_APPEARENCE_VALUE_GENERIC_GLUCOSE_METER,          "Generic Glucose Meter"     }
} ;

#define NUMBER_OF_APPEARANCE_MAPPINGS     (sizeof(AppearanceMappings)/sizeof(GAPS_Device_Appearance_Mapping_t))

   /* The following string table is used to map HCI Version information */
   /* to an easily displayable version string.                          */
static BTPSCONST char *HCIVersionStrings[] =
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
static BTPSCONST char *IOCapabilitiesStrings[] =
{
   "Display Only",
   "Display Yes/No",
   "Keyboard Only",
   "No Input/Output",
   "Keyboard/Display"
} ;

   /* The following defines a data sequence that will be used to        */
   /* generate message data.                                            */
static char  DataStr[]  = "~!@#$%^&*()_+`1234567890-=:;\"'<>?,./@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]`abcdefghijklmnopqrstuvwxyz{|}<>\r\n";
static int   DataStrLen = (sizeof(DataStr)-1);

   /*********************************************************************/
   /**                     SPPLE Service Table                         **/
   /*********************************************************************/

   /* The SPPLE Service Declaration UUID.                               */
static BTPSCONST GATT_Primary_Service_128_Entry_t SPPLE_Service_UUID =
{
   SPPLE_SERVICE_UUID_CONSTANT
} ;

   /* The Tx Characteristic Declaration.                                */
static BTPSCONST GATT_Characteristic_Declaration_128_Entry_t SPPLE_Tx_Declaration =
{
   GATT_CHARACTERISTIC_PROPERTIES_NOTIFY,
   SPPLE_TX_CHARACTERISTIC_UUID_CONSTANT
} ;

   /* The Tx Characteristic Value.                                      */
static BTPSCONST GATT_Characteristic_Value_128_Entry_t  SPPLE_Tx_Value =
{
   SPPLE_TX_CHARACTERISTIC_UUID_CONSTANT,
   0,
   NULL
} ;

   /* The Tx Credits Characteristic Declaration.                        */
static BTPSCONST GATT_Characteristic_Declaration_128_Entry_t SPPLE_Tx_Credits_Declaration =
{
   (GATT_CHARACTERISTIC_PROPERTIES_READ|GATT_CHARACTERISTIC_PROPERTIES_WRITE_WITHOUT_RESPONSE|GATT_CHARACTERISTIC_PROPERTIES_WRITE),
   SPPLE_TX_CREDITS_CHARACTERISTIC_UUID_CONSTANT
} ;

   /* The Tx Credits Characteristic Value.                              */
static BTPSCONST GATT_Characteristic_Value_128_Entry_t SPPLE_Tx_Credits_Value =
{
   SPPLE_TX_CREDITS_CHARACTERISTIC_UUID_CONSTANT,
   0,
   NULL
} ;

   /* The SPPLE RX Characteristic Declaration.                          */
static BTPSCONST GATT_Characteristic_Declaration_128_Entry_t SPPLE_Rx_Declaration =
{
   (GATT_CHARACTERISTIC_PROPERTIES_WRITE_WITHOUT_RESPONSE),
   SPPLE_RX_CHARACTERISTIC_UUID_CONSTANT
} ;

   /* The SPPLE RX Characteristic Value.                                */
static BTPSCONST GATT_Characteristic_Value_128_Entry_t  SPPLE_Rx_Value =
{
   SPPLE_RX_CHARACTERISTIC_UUID_CONSTANT,
   0,
   NULL
} ;

   /* The SPPLE Rx Credits Characteristic Declaration.                  */
static BTPSCONST GATT_Characteristic_Declaration_128_Entry_t SPPLE_Rx_Credits_Declaration =
{
   (GATT_CHARACTERISTIC_PROPERTIES_READ|GATT_CHARACTERISTIC_PROPERTIES_NOTIFY),
   SPPLE_RX_CREDITS_CHARACTERISTIC_UUID_CONSTANT
};

   /* The SPPLE Rx Credits Characteristic Value.                        */
static BTPSCONST GATT_Characteristic_Value_128_Entry_t SPPLE_Rx_Credits_Value =
{
   SPPLE_RX_CREDITS_CHARACTERISTIC_UUID_CONSTANT,
   0,
   NULL
};

   /* Client Characteristic Configuration Descriptor.                   */
static GATT_Characteristic_Descriptor_16_Entry_t Client_Characteristic_Configuration =
{
   GATT_CLIENT_CHARACTERISTIC_CONFIGURATION_BLUETOOTH_UUID_CONSTANT,
   GATT_CLIENT_CHARACTERISTIC_CONFIGURATION_LENGTH,
   NULL
};

   /* The following defines the SPPLE service that is registered with   */
   /* the GATT_Register_Service function call.                          */
   /* * NOTE * This array will be registered with GATT in the call to   */
   /*          GATT_Register_Service.                                   */
BTPSCONST GATT_Service_Attribute_Entry_t SPPLE_Service[] =
{
   { GATT_ATTRIBUTE_FLAGS_READABLE,          aetPrimaryService128,            (Byte_t *)&SPPLE_Service_UUID                  },
   { GATT_ATTRIBUTE_FLAGS_READABLE,          aetCharacteristicDeclaration128, (Byte_t *)&SPPLE_Tx_Declaration                },
   { 0,                                      aetCharacteristicValue128,       (Byte_t *)&SPPLE_Tx_Value                      },
   { GATT_ATTRIBUTE_FLAGS_READABLE_WRITABLE, aetCharacteristicDescriptor16,   (Byte_t *)&Client_Characteristic_Configuration },
   { GATT_ATTRIBUTE_FLAGS_READABLE,          aetCharacteristicDeclaration128, (Byte_t *)&SPPLE_Tx_Credits_Declaration        },
   { GATT_ATTRIBUTE_FLAGS_READABLE_WRITABLE, aetCharacteristicValue128,       (Byte_t *)&SPPLE_Tx_Credits_Value              },
   { GATT_ATTRIBUTE_FLAGS_READABLE,          aetCharacteristicDeclaration128, (Byte_t *)&SPPLE_Rx_Declaration                },
   { GATT_ATTRIBUTE_FLAGS_WRITABLE,          aetCharacteristicValue128,       (Byte_t *)&SPPLE_Rx_Value                      },
   { GATT_ATTRIBUTE_FLAGS_READABLE,          aetCharacteristicDeclaration128, (Byte_t *)&SPPLE_Rx_Credits_Declaration        },
   { GATT_ATTRIBUTE_FLAGS_READABLE,          aetCharacteristicValue128,       (Byte_t *)&SPPLE_Rx_Credits_Value              },
   { GATT_ATTRIBUTE_FLAGS_READABLE_WRITABLE, aetCharacteristicDescriptor16,   (Byte_t *)&Client_Characteristic_Configuration }
} ;

#define SPPLE_SERVICE_ATTRIBUTE_COUNT               (sizeof(SPPLE_Service)/sizeof(GATT_Service_Attribute_Entry_t))

#define SPPLE_TX_CHARACTERISTIC_ATTRIBUTE_OFFSET               2
#define SPPLE_TX_CHARACTERISTIC_CCD_ATTRIBUTE_OFFSET           3
#define SPPLE_TX_CREDITS_CHARACTERISTIC_ATTRIBUTE_OFFSET       5
#define SPPLE_RX_CHARACTERISTIC_ATTRIBUTE_OFFSET               7
#define SPPLE_RX_CREDITS_CHARACTERISTIC_ATTRIBUTE_OFFSET       9
#define SPPLE_RX_CREDITS_CHARACTERISTIC_CCD_ATTRIBUTE_OFFSET   10

   /*********************************************************************/
   /**                    END OF SERVICE TABLE                         **/
   /*********************************************************************/

   /* Internal function prototypes.                                     */
static Boolean_t CreateNewDeviceInfoEntry(DeviceInfo_t **ListHead, GAP_LE_Address_Type_t ConnectionAddressType, BD_ADDR_t ConnectionBD_ADDR);
static DeviceInfo_t *SearchDeviceInfoEntryByBD_ADDR(DeviceInfo_t **ListHead, BD_ADDR_t BD_ADDR);
static DeviceInfo_t *DeleteDeviceInfoEntry(DeviceInfo_t **ListHead, BD_ADDR_t BD_ADDR);
static void FreeDeviceInfoEntryMemory(DeviceInfo_t *EntryToFree);
static void FreeDeviceInfoList(DeviceInfo_t **ListHead);

static void UserInterface(void);
static unsigned int StringToUnsignedInteger(char *StringInteger);
static char *StringParser(char *String);
static int CommandParser(UserCommand_t *TempCommand, char *Input);
static int CommandInterpreter(UserCommand_t *TempCommand);
static int AddCommand(char *CommandName, CommandFunction_t CommandFunction);
static CommandFunction_t FindCommand(char *Command);
static void ClearCommands(void);

static void BD_ADDRToStr(BD_ADDR_t Board_Address, BoardStr_t BoardStr);
static void StrToBD_ADDR(char *BoardStr, BD_ADDR_t *Board_Address);

static void DisplayIOCapabilities(void);
static void DisplayAdvertisingData(GAP_LE_Advertising_Data_t *Advertising_Data);
static void DisplayPairingInformation(GAP_LE_Extended_Pairing_Capabilities_t *Extended_Pairing_Capabilities);
static void DisplayLegacyPairingInformation(GAP_LE_Pairing_Capabilities_t *Pairing_Capabilities);
static void DisplayUUID(GATT_UUID_t *UUID);
static void DisplayPrompt(void);
static void DisplayUsage(char *UsageString);
static void DisplayFunctionError(char *Function,int Status);

static int OpenStack(HCI_DriverInformation_t *HCI_DriverInformation);
static int CloseStack(void);

static int SetDisc(void);
static int SetConnect(void);
static int SetPairable(void);

static void DumpAppearanceMappings(void);
static Boolean_t AppearanceToString(Word_t Appearance, char **String);
static Boolean_t AppearanceIndexToAppearance(unsigned int Index, Word_t *Appearance);

static void DISPopulateHandles(DIS_Client_Info_t *ClientInfo, GATT_Service_Discovery_Indication_Data_t *ServiceInfo);
static void GAPSPopulateHandles(GAPS_Client_Info_t *ClientInfo, GATT_Service_Discovery_Indication_Data_t *ServiceInfo);
static void SPPLEPopulateHandles(SPPLE_Client_Info_t *ClientInfo, GATT_Service_Discovery_Indication_Data_t *ServiceInfo);

static unsigned int AddDataToBuffer(SPPLE_Data_Buffer_t *DataBuffer, unsigned int DataLength, Byte_t *Data);
static unsigned int RemoveDataFromBuffer(SPPLE_Data_Buffer_t *DataBuffer, unsigned int BufferLength, Byte_t *Buffer);
static void InitializeBuffer(SPPLE_Data_Buffer_t *DataBuffer);

static int EnableDisableNotificationsIndications(Word_t ClientConfigurationHandle, Word_t ClientConfigurationValue, GATT_Client_Event_Callback_t ClientEventCallback);

static unsigned int FillBufferWithString(SPPLE_Data_Buffer_t *DataBuffer, unsigned *CurrentBufferLength, unsigned int MaxLength, Byte_t *Buffer);

static void SendProcess(DeviceInfo_t *DeviceInfo);
static void SendCredits(DeviceInfo_t *DeviceInfo, unsigned int DataLength);
static void ReceiveCreditEvent(DeviceInfo_t *DeviceInfo, unsigned int Credits);
static Boolean_t SendData(DeviceInfo_t *DeviceInfo, unsigned int DataLength, Byte_t *Data);
static void DataIndicationEvent(DeviceInfo_t *DeviceInfo, unsigned int DataLength, Byte_t *Data);
static int ReadData(DeviceInfo_t *DeviceInfo, unsigned int BufferLength, Byte_t *Buffer);

static int StartScan(unsigned int BluetoothStackID, Boolean_t UseWhitelist, GAP_LE_Address_Type_t LocalAddressType);
static int StopScan(unsigned int BluetoothStackID);

static int ConnectLEDevice(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR, GAP_LE_Address_Type_t PeerAddressType, GAP_LE_Address_Type_t LocalAddressType, Boolean_t UseWhiteList);
static int DisconnectLEDevice(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR);

static void ConfigureCapabilities(GAP_LE_Extended_Pairing_Capabilities_t *Capabilities);
static int SendPairingRequest(BD_ADDR_t BD_ADDR, Boolean_t ConnectionMaster);
static int SlavePairingRequestResponse(BD_ADDR_t BD_ADDR, Boolean_t AcceptRequest);
static int EncryptionInformationRequestResponse(BD_ADDR_t BD_ADDR, Byte_t KeySize, GAP_LE_Authentication_Response_Information_t *GAP_LE_Authentication_Response_Information);

static int DisplayHelp(ParameterList_t *TempParam);
static int UpdateConnectionParams(ParameterList_t *TempParam);
static int SetDiscoverabilityMode(ParameterList_t *TempParam);
static int SetConnectabilityMode(ParameterList_t *TempParam);
static int SetPairabilityMode(ParameterList_t *TempParam);
static int ChangePairingParameters(ParameterList_t *TempParam);
static int LEPassKeyResponse(ParameterList_t *TempParam);
static int LEQueryEncryption(ParameterList_t *TempParam);
static int LESetPasskey(ParameterList_t *TempParam);
static int GetLocalAddress(ParameterList_t *TempParam);

static int AdvertiseLE(ParameterList_t *TempParam);

static int StartScanning(ParameterList_t *TempParam);
static int StopScanning(ParameterList_t *TempParam);

static int ConnectLE(ParameterList_t *TempParam);
static int DisconnectLE(ParameterList_t *TempParam);
static int CancelConnect(ParameterList_t *TempParam);

static int PairLE(ParameterList_t *TempParam);
static int UnpairLE(ParameterList_t *TempParam);

static int DiscoverDIS(ParameterList_t *TempParam);
static int DiscoverGAPS(ParameterList_t *TempParam);
static int ReadLocalName(ParameterList_t *TempParam);
static int SetLocalName(ParameterList_t *TempParam);
static int ReadRemoteName(ParameterList_t *TempParam);
static int ReadLocalAppearance(ParameterList_t *TempParam);
static int SetLocalAppearance(ParameterList_t *TempParam);
static int ReadRemoteAppearance(ParameterList_t *TempParam);

static int EnableDebug(ParameterList_t *TempParam);

static int DiscoverSPPLE(ParameterList_t *TempParam);
static int RegisterSPPLE(ParameterList_t *TempParam);
static int UnregisterSPPLE(ParameterList_t *TempParam);
static int ConfigureSPPLE(ParameterList_t *TempParam);
static int SendDataCommand(ParameterList_t *TempParam);
static int ReadDataCommand(ParameterList_t *TempParam);

static int GetGATTMTU(ParameterList_t *TempParam);
static int SetGATTMTU(ParameterList_t *TempParam);

static int SetDataLength(ParameterList_t *TempParam);
static int ReadMaxDataLength(ParameterList_t *TempParam);
static int QueryDefaultDataLength(ParameterList_t *TempParam);
static int SetDefaultDataLength(ParameterList_t *TempParam);
static int UpdateP256Key(ParameterList_t *TempParam);
// xxx static int GenerateDHKey(ParameterList_t *TempParam);
static int AddDeviceResolvingList(ParameterList_t *TempParam);
static int RemoveDeviceResolvingList(ParameterList_t *TempParam);
static int ReadResolvingListSize(ParameterList_t *TempParam);
static int ReadPeerResolvableAddress(ParameterList_t *TempParam);
static int ReadLocalResolvableAddress(ParameterList_t *TempParam);
static int SetAddressResolutionEnable(ParameterList_t *TempParam);
static int SetRPATimeout(ParameterList_t *TempParam);
static int AddDeviceWhiteList(ParameterList_t *TempParam);
static int RemoveDeviceWhiteList(ParameterList_t *TempParam);
static int ReadWhiteListSize(ParameterList_t *TempParam);

static int Loopback(ParameterList_t *TempParam);
static int DisplayRawModeData(ParameterList_t *TempParam);
static int AutomaticReadMode(ParameterList_t *TempParam);

   /* BTPS Callback function prototypes.                                */
static void BTPSAPI HCI_Event_Callback(unsigned int BluetoothStackID, HCI_Event_Data_t *HCI_Event_Data, unsigned long CallbackParameter);
static void BTPSAPI GAP_LE_Event_Callback(unsigned int BluetoothStackID,GAP_LE_Event_Data_t *GAP_LE_Event_Data, unsigned long CallbackParameter);
static void BTPSAPI GATT_ServerEventCallback(unsigned int BluetoothStackID, GATT_Server_Event_Data_t *GATT_ServerEventData, unsigned long CallbackParameter);
static void BTPSAPI GATT_ClientEventCallback_SPPLE(unsigned int BluetoothStackID, GATT_Client_Event_Data_t *GATT_Client_Event_Data, unsigned long CallbackParameter);
static void BTPSAPI GATT_ClientEventCallback_DIS(unsigned int BluetoothStackID, GATT_Client_Event_Data_t *GATT_Client_Event_Data, unsigned long CallbackParameter);
static void BTPSAPI GATT_ClientEventCallback_GAPS(unsigned int BluetoothStackID, GATT_Client_Event_Data_t *GATT_Client_Event_Data, unsigned long CallbackParameter);
static void BTPSAPI GATT_Connection_Event_Callback(unsigned int BluetoothStackID, GATT_Connection_Event_Data_t *GATT_Connection_Event_Data, unsigned long CallbackParameter);
static void BTPSAPI GATT_Service_Discovery_Event_Callback(unsigned int BluetoothStackID, GATT_Service_Discovery_Event_Data_t *GATT_Service_Discovery_Event_Data, unsigned long CallbackParameter);

   /* The following function adds the specified Entry to the specified  */
   /* List.  This function allocates and adds an entry to the list that */
   /* has the same attributes as parameters to this function.  This     */
   /* function will return FALSE if NO Entry was added.  This can occur */
   /* if the element passed in was deemed invalid or the actual List    */
   /* Head was invalid.                                                 */
   /* ** NOTE ** This function does not insert duplicate entries into   */
   /*            the list.  An element is considered a duplicate if the */
   /*            Connection BD_ADDR.  When this occurs, this function   */
   /*            returns NULL.                                          */
static Boolean_t CreateNewDeviceInfoEntry(DeviceInfo_t **ListHead, GAP_LE_Address_Type_t ConnectionAddressType, BD_ADDR_t ConnectionBD_ADDR)
{
   Boolean_t     ret_val = FALSE;
   DeviceInfo_t *DeviceInfoPtr;

   /* Verify that the passed in parameters seem semi-valid.             */
   if((ListHead) && (!COMPARE_NULL_BD_ADDR(ConnectionBD_ADDR)))
   {
      /* Allocate the memory for the entry.                             */
      if((DeviceInfoPtr = BTPS_AllocateMemory(sizeof(DeviceInfo_t))) != NULL)
      {
         /* Initialize the entry.                                       */
         BTPS_MemInitialize(DeviceInfoPtr, 0, sizeof(DeviceInfo_t));
         DeviceInfoPtr->ConnectionAddressType = ConnectionAddressType;
         DeviceInfoPtr->ConnectionBD_ADDR     = ConnectionBD_ADDR;

         ret_val = BSC_AddGenericListEntry_Actual(ekBD_ADDR_t, BTPS_STRUCTURE_OFFSET(DeviceInfo_t, ConnectionBD_ADDR), BTPS_STRUCTURE_OFFSET(DeviceInfo_t, NextDeviceInfoPtr), (void **)(ListHead), (void *)(DeviceInfoPtr));
         if(!ret_val)
         {
            /* Failed to add to list so we should free the memory that  */
            /* we allocated for the entry.                              */
            BTPS_FreeMemory(DeviceInfoPtr);
         }
      }
   }

   return(ret_val);
}

   /* The following function searches the specified List for the        */
   /* specified Connection BD_ADDR.  This function returns NULL if      */
   /* either the List Head is invalid, the BD_ADDR is invalid, or the   */
   /* Connection BD_ADDR was NOT found.                                 */
static DeviceInfo_t *SearchDeviceInfoEntryByBD_ADDR(DeviceInfo_t **ListHead, BD_ADDR_t BD_ADDR)
{
   DeviceInfo_t *ret_val = NULL;

   /* Verify that the list head is invalid.                             */
   if(ListHead)
   {
      /* Search for the entry by the connection addresss.               */
      if((ret_val = BSC_SearchGenericListEntry(ekBD_ADDR_t, (void *)(&BD_ADDR), BTPS_STRUCTURE_OFFSET(DeviceInfo_t, ConnectionBD_ADDR), BTPS_STRUCTURE_OFFSET(DeviceInfo_t, NextDeviceInfoPtr), (void **)(ListHead))) == NULL)
      {
         /* Search for the entry by the peer address.                   */
         if((ret_val = BSC_SearchGenericListEntry(ekBD_ADDR_t, (void *)(&BD_ADDR), BTPS_STRUCTURE_OFFSET(DeviceInfo_t, PeerAddress), BTPS_STRUCTURE_OFFSET(DeviceInfo_t, NextDeviceInfoPtr), (void **)(ListHead))) == NULL)
         {
            /* Search for the entry by resolving the resolvable private */
            /* address using the stored IRK in each entry.              */
            if(GAP_LE_TEST_RESOLVABLE_ADDRESS_BITS(BD_ADDR))
            {
               ret_val = *ListHead;
               while(ret_val)
               {
                  /* Make sure the peer information is valid.           */
                  if(ret_val->Flags & DEVICE_INFO_FLAGS_PEER_IDENTITY_INFO_VALID)
                  {
                     /* Resolve this entry.                             */
                     if(GAP_LE_Resolve_Address(BluetoothStackID, &(ret_val->PeerIRK), BD_ADDR))
                     {
                        /* If this entry resolves the address then we do*/
                        /* not need to continue trying to resolve other */
                        /* entries.                                     */
                        break;
                     }
                  }

                  ret_val = ret_val->NextDeviceInfoPtr;
               }
            }
         }
      }
   }

   return(ret_val);
}

   /* The following function searches the specified Key Info List for   */
   /* the specified BD_ADDR and removes it from the List.  This function*/
   /* returns NULL if either the List Head is invalid, the BD_ADDR is   */
   /* invalid, or the specified Entry was NOT present in the list.  The */
   /* entry returned will have the Next Entry field set to NULL, and    */
   /* the caller is responsible for deleting the memory associated with */
   /* this entry by calling the FreeKeyEntryMemory() function.          */
static DeviceInfo_t *DeleteDeviceInfoEntry(DeviceInfo_t **ListHead, BD_ADDR_t BD_ADDR)
{
   return(BSC_DeleteGenericListEntry(ekBD_ADDR_t, (void *)(&BD_ADDR), BTPS_STRUCTURE_OFFSET(DeviceInfo_t, ConnectionBD_ADDR), BTPS_STRUCTURE_OFFSET(DeviceInfo_t, NextDeviceInfoPtr), (void **)(ListHead)));
}

   /* This function frees the specified Key Info Information member     */
   /* memory.                                                           */
static void FreeDeviceInfoEntryMemory(DeviceInfo_t *EntryToFree)
{
   BSC_FreeGenericListEntryMemory((void *)(EntryToFree));
}

   /* The following function deletes (and frees all memory) every       */
   /* element of the specified Key Info List. Upon return of this       */
   /* function, the Head Pointer is set to NULL.                        */
static void FreeDeviceInfoList(DeviceInfo_t **ListHead)
{
   BSC_FreeGenericListEntryList((void **)(ListHead), BTPS_STRUCTURE_OFFSET(DeviceInfo_t, NextDeviceInfoPtr));
}

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

   /* Display the available commands.                                   */
   DisplayHelp(NULL);

   /* Clear the installed command.                                      */
   ClearCommands();

   /* Install the commands relevant for this UI.                        */
   AddCommand("UPDATECONNECTIONPARAMS", UpdateConnectionParams);
   AddCommand("SETDISCOVERABILITYMODE", SetDiscoverabilityMode);
   AddCommand("SETCONNECTABILITYMODE", SetConnectabilityMode);
   AddCommand("SETPAIRABILITYMODE", SetPairabilityMode);
   AddCommand("CHANGEPAIRINGPARAMETERS", ChangePairingParameters);
   AddCommand("GETLOCALADDRESS", GetLocalAddress);
   AddCommand("ADVERTISELE", AdvertiseLE);
   AddCommand("STARTSCANNING", StartScanning);
   AddCommand("STOPSCANNING", StopScanning);
   AddCommand("CONNECTLE", ConnectLE);
   AddCommand("DISCONNECTLE", DisconnectLE);
   AddCommand("CANCELCONNECT", CancelConnect);
   AddCommand("PAIRLE", PairLE);
   AddCommand("UNPAIRLE", UnpairLE);
   AddCommand("LEPASSKEYRESPONSE", LEPassKeyResponse);
   AddCommand("QUERYENCRYPTIONMODE", LEQueryEncryption);
   AddCommand("SETPASSKEY", LESetPasskey);
   AddCommand("SENDDATA", SendDataCommand);
   AddCommand("READDATA", ReadDataCommand);
   AddCommand("LOOPBACK", Loopback);
   AddCommand("DISCOVERSPPLE", DiscoverSPPLE);
   AddCommand("REGISTERSPPLE", RegisterSPPLE);
   AddCommand("UNREGISTERSPPLE", UnregisterSPPLE);
   AddCommand("CONFIGURESPPLE", ConfigureSPPLE);
   AddCommand("DISCOVERDIS", DiscoverDIS);
   AddCommand("DISCOVERGAPS", DiscoverGAPS);
   AddCommand("GETLOCALNAME", ReadLocalName);
   AddCommand("SETLOCALNAME", SetLocalName);
   AddCommand("GETREMOTENAME", ReadRemoteName);
   AddCommand("GETLOCALAPPEARANCE", ReadLocalAppearance);
   AddCommand("SETLOCALAPPEARANCE", SetLocalAppearance);
   AddCommand("GETREMOTEAPPEARANCE", ReadRemoteAppearance);
   AddCommand("ENABLEDEBUG", EnableDebug);
   AddCommand("DISPLAYRAWMODEDATA", DisplayRawModeData);
   AddCommand("AUTOMATICREADMODE", AutomaticReadMode);
   AddCommand("GETMTU", GetGATTMTU);
   AddCommand("SETMTU", SetGATTMTU);
   
   AddCommand("SETDATALENGTH", SetDataLength);
   AddCommand("READMAXDATALENGTH", ReadMaxDataLength);
   AddCommand("QUERYDEFAULTDATALENGTH", QueryDefaultDataLength);
   AddCommand("SETDEFAULTDATALENGTH", SetDefaultDataLength);
   AddCommand("UPDATEP256KEY", UpdateP256Key);
// xxx   AddCommand("GENERATEDHKEY", GenerateDHKey);
   AddCommand("ADDRESOLVINGLIST", AddDeviceResolvingList);
   AddCommand("REMOVERESOLVINGLIST", RemoveDeviceResolvingList);
   AddCommand("READRESOLVINGLISTSIZE", ReadResolvingListSize);
   AddCommand("READPEERRPA", ReadPeerResolvableAddress);
   AddCommand("READLOCALRPA", ReadLocalResolvableAddress);
   AddCommand("SETADDRESSRESOLUTIONENABLE", SetAddressResolutionEnable);
   AddCommand("SETRPATIMEOUT", SetRPATimeout);
   AddCommand("ADDWHITELIST", AddDeviceWhiteList);
   AddCommand("REMOVEWHITELIST", RemoveDeviceWhiteList);
   AddCommand("READWHITELISTSIZE", ReadWhiteListSize);
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
      DisplayPrompt();

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
                     break;
               }
            }
            else
               printf("Invalid Input.\r\n");
         }
      }
      else
      {
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
   if((StringInteger) && (BTPS_StringLength(StringInteger)))
   {
      /* Initialize the variable.                                       */
      Index = 0;

      /* Next check to see if this is a hexadecimal number.             */
      if(BTPS_StringLength(StringInteger) > 2)
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
               if(((Index + 1) < BTPS_StringLength(StringInteger)) && (StringInteger[Index+1] >= '0') && (StringInteger[Index+1] <= '9'))
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
               if(((Index + 1) < BTPS_StringLength(StringInteger)) && (((StringInteger[Index+1] >= '0') && (StringInteger[Index+1] <= '9')) || ((StringInteger[Index+1] >= 'a') && (StringInteger[Index+1] <= 'f')) || ((StringInteger[Index+1] >= 'A') && (StringInteger[Index+1] <= 'F'))))
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
   if((String) && (BTPS_StringLength(String)))
   {
      /* The string appears to be at least semi-valid.  Search for the  */
      /* first space character and replace it with a NULL terminating   */
      /* character.                                                     */
      for(Index=0, ret_val=String;Index < BTPS_StringLength(String);Index++)
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
static int CommandParser(UserCommand_t *TempCommand, char *Input)
{
   int            ret_val;
   int            StringLength;
   char          *LastParameter;
   unsigned int   Count         = 0;

   /* Before proceeding make sure that the passed parameters appear to  */
   /* be at least semi-valid.                                           */
   if((TempCommand) && (Input) && (BTPS_StringLength(Input)))
   {
      /* First get the initial string length.                           */
      StringLength = BTPS_StringLength(Input);

      /* Retrieve the first token in the string, this should be the     */
      /* command.                                                       */
      TempCommand->Command = StringParser(Input);

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
         Input        += BTPS_StringLength(TempCommand->Command) + 1;
         StringLength  = BTPS_StringLength(Input);

         /* There was an available command, now parse out the parameters*/
         while((StringLength > 0) && ((LastParameter = StringParser(Input)) != NULL))
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
               Input        += BTPS_StringLength(LastParameter) + 1;
               StringLength -= BTPS_StringLength(LastParameter) + 1;

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
   /* A negative return value implies that command was not found and is */
   /* invalid.                                                          */
static int CommandInterpreter(UserCommand_t *TempCommand)
{
   int               Index;
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
      for(Index = 0; Index < BTPS_StringLength(TempCommand->Command); Index++)
      {
         if((TempCommand->Command[Index] >= 'a') && (TempCommand->Command[Index] <= 'z'))
            TempCommand->Command[Index] -= ('a' - 'A');
      }

      /* Check to see if the command which was entered was exit.        */
      if(memcmp(TempCommand->Command, "QUIT", strlen("QUIT")) != 0)
      {
         /* The command entered is not exit so search for command in    */
         /* table.                                                      */
         if((CommandFunction = FindCommand(TempCommand->Command)) != NULL)
         {
            /* The command was found in the table so call the command.  */
            ret_val = (*CommandFunction)(&TempCommand->Parameters);
            if(!ret_val)
            {
               /* Return success to the caller.                         */
               ret_val = 0;
            }
            else
            {
               if((ret_val != EXIT_CODE) && (ret_val != EXIT_MODE))
                  ret_val = FUNCTION_ERROR;
            }
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
   /* problematically add Commands the Global (to this module) Command  */
   /* Table.  The Command Table is simply a mapping of Command Name     */
   /* (NULL terminated ASCII string) to a command function.  This       */
   /* function returns zero if successful, or a non-zero value if the   */
   /* command could not be added to the list.                           */
static int AddCommand(char *CommandName, CommandFunction_t CommandFunction)
{
   int ret_val = 0;

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
      for(Index = 0, ret_val = NULL; ((Index < NumberCommands) && (!ret_val)); Index++)
      {
         if(BTPS_MemCompare(Command, CommandTable[Index].CommandName, BTPS_StringLength(CommandTable[Index].CommandName)) == 0)
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
static void BD_ADDRToStr(BD_ADDR_t Board_Address, BoardStr_t BoardStr)
{
   BTPS_SprintF((char *)BoardStr, "0x%02X%02X%02X%02X%02X%02X", Board_Address.BD_ADDR5, Board_Address.BD_ADDR4, Board_Address.BD_ADDR3, Board_Address.BD_ADDR2, Board_Address.BD_ADDR1, Board_Address.BD_ADDR0);
}

   /* The following function is responsible for the specified string    */
   /* into data of type BD_ADDR.  The first parameter of this function  */
   /* is the BD_ADDR string to be converted to a BD_ADDR.  The second   */
   /* parameter of this function is a pointer to the BD_ADDR in which   */
   /* the converted BD_ADDR String is to be stored.                     */
static void StrToBD_ADDR(char *BoardStr, BD_ADDR_t *Board_Address)
{
   char Buffer[5];

   if((BoardStr) && (BTPS_StringLength(BoardStr) == sizeof(BD_ADDR_t)*2) && (Board_Address))
   {
      Buffer[0] = '0';
      Buffer[1] = 'x';
      Buffer[4] = '\0';

      Buffer[2] = BoardStr[0];
      Buffer[3] = BoardStr[1];
      Board_Address->BD_ADDR5 = (Byte_t)StringToUnsignedInteger(Buffer);

      Buffer[2] = BoardStr[2];
      Buffer[3] = BoardStr[3];
      Board_Address->BD_ADDR4 = (Byte_t)StringToUnsignedInteger(Buffer);

      Buffer[2] = BoardStr[4];
      Buffer[3] = BoardStr[5];
      Board_Address->BD_ADDR3 = (Byte_t)StringToUnsignedInteger(Buffer);

      Buffer[2] = BoardStr[6];
      Buffer[3] = BoardStr[7];
      Board_Address->BD_ADDR2 = (Byte_t)StringToUnsignedInteger(Buffer);

      Buffer[2] = BoardStr[8];
      Buffer[3] = BoardStr[9];
      Board_Address->BD_ADDR1 = (Byte_t)StringToUnsignedInteger(Buffer);

      Buffer[2] = BoardStr[10];
      Buffer[3] = BoardStr[11];
      Board_Address->BD_ADDR0 = (Byte_t)StringToUnsignedInteger(Buffer);
   }
   else
   {
      if(Board_Address)
         BTPS_MemInitialize(Board_Address, 0, sizeof(BD_ADDR_t));
   }
}

   /* Displays the current I/O Capabilities.                            */
static void DisplayIOCapabilities(void)
{
   printf("I/O Capabilities: %s, MITM: %s., Secure Connections: %s.\r\n", IOCapabilitiesStrings[(unsigned int)(LE_Parameters.IOCapability - licDisplayOnly)], LE_Parameters.MITMProtection?"TRUE":"FALSE", LE_Parameters.SecureConnections?"TRUE":"FALSE");
}

   /* Utility function to display advertising data.                     */
static void DisplayAdvertisingData(GAP_LE_Advertising_Data_t *Advertising_Data)
{
   unsigned int Index;
   unsigned int Index2;

   /* Verify that the input parameters seem semi-valid.                 */
   if(Advertising_Data)
   {
      for(Index = 0; Index < Advertising_Data->Number_Data_Entries; Index++)
      {
         printf("  AD Type: 0x%02X.\r\n", (unsigned int)(Advertising_Data->Data_Entries[Index].AD_Type));
         printf("  AD Length: 0x%02X.\r\n", (unsigned int)(Advertising_Data->Data_Entries[Index].AD_Data_Length));
         if(Advertising_Data->Data_Entries[Index].AD_Data_Buffer)
         {
            printf("  AD Data: ");
            for(Index2 = 0; Index2 < Advertising_Data->Data_Entries[Index].AD_Data_Length; Index2++)
            {
               printf("0x%02X ", Advertising_Data->Data_Entries[Index].AD_Data_Buffer[Index2]);
            }
            
            if(Advertising_Data->Data_Entries[Index].AD_Type == 9)
            {
               printf(" (");
               for(Index2 = 0; Index2 < Advertising_Data->Data_Entries[Index].AD_Data_Length; Index2++)
               {
                  printf("%c", Advertising_Data->Data_Entries[Index].AD_Data_Buffer[Index2]);
               }
               printf(")");
            }
            
            printf("\r\n");
         }
      }
   }
}

   /* The following function displays the pairing capabilities that is  */
   /* passed into this function.                                        */
static void DisplayPairingInformation(GAP_LE_Extended_Pairing_Capabilities_t *Extended_Pairing_Capabilities)
{
      /* Display the IO Capability.                                        */
   switch(Extended_Pairing_Capabilities->IO_Capability)
   {
      case licDisplayOnly:
         printf("   IO Capability:       lcDisplayOnly.\r\n");
         break;
      case licDisplayYesNo:
         printf("   IO Capability:       lcDisplayYesNo.\r\n");
         break;
      case licKeyboardOnly:
         printf("   IO Capability:       lcKeyboardOnly.\r\n");
         break;
      case licNoInputNoOutput:
         printf("   IO Capability:       lcNoInputNoOutput.\r\n");
         break;
      case licKeyboardDisplay:
         printf("   IO Capability:       lcKeyboardDisplay.\r\n");
         break;
   }

   printf("   Bonding Type:        %s.\r\n", (Extended_Pairing_Capabilities->Bonding_Type == lbtBonding)?"Bonding":"No Bonding");
   printf("   Secure Connections:  %s.\r\n", (Extended_Pairing_Capabilities->Flags & GAP_LE_EXTENDED_PAIRING_CAPABILITIES_FLAGS_SECURE_CONNECTIONS)?"TRUE":"FALSE");
   printf("   MITM:                %s.\r\n", (Extended_Pairing_Capabilities->Flags & GAP_LE_EXTENDED_PAIRING_CAPABILITIES_FLAGS_MITM_REQUESTED)?"TRUE":"FALSE");
   printf("   OOB:                 %s.\r\n", (Extended_Pairing_Capabilities->Flags & GAP_LE_EXTENDED_PAIRING_CAPABILITIES_FLAGS_OOB_DATA_PRESENT)?"OOB":"OOB Not Present");
   printf("   Encryption Key Size: %d.\r\n", Extended_Pairing_Capabilities->Maximum_Encryption_Key_Size);
   printf("   Sending Keys: \r\n");
   printf("      LTK:              %s.\r\n", ((Extended_Pairing_Capabilities->Sending_Keys.Encryption_Key)?"YES":"NO"));
   printf("      IRK:              %s.\r\n", ((Extended_Pairing_Capabilities->Sending_Keys.Identification_Key)?"YES":"NO"));
   printf("      CSRK:             %s.\r\n", ((Extended_Pairing_Capabilities->Sending_Keys.Signing_Key)?"YES":"NO"));
   printf("      Link Key:         %s.\r\n", ((Extended_Pairing_Capabilities->Sending_Keys.Link_Key)?"YES":"NO"));
   printf("   Receiving Keys: \r\n");
   printf("      LTK:              %s.\r\n", ((Extended_Pairing_Capabilities->Receiving_Keys.Encryption_Key)?"YES":"NO"));
   printf("      IRK:              %s.\r\n", ((Extended_Pairing_Capabilities->Receiving_Keys.Identification_Key)?"YES":"NO"));
   printf("      CSRK:             %s.\r\n", ((Extended_Pairing_Capabilities->Receiving_Keys.Signing_Key)?"YES":"NO"));
   printf("      Link Key:         %s.\r\n", ((Extended_Pairing_Capabilities->Receiving_Keys.Link_Key)?"YES":"NO"));
}

   /* The following function displays the pairing capabilities that is  */
   /* passed into this function.                                        */
static void DisplayLegacyPairingInformation(GAP_LE_Pairing_Capabilities_t *Pairing_Capabilities)
{
   /* Display the IO Capability.                                        */
   switch(Pairing_Capabilities->IO_Capability)
   {
      case licDisplayOnly:
         printf("   IO Capability:       lcDisplayOnly.\r\n");
         break;
      case licDisplayYesNo:
         printf("   IO Capability:       lcDisplayYesNo.\r\n");
         break;
      case licKeyboardOnly:
         printf("   IO Capability:       lcKeyboardOnly.\r\n");
         break;
      case licNoInputNoOutput:
         printf("   IO Capability:       lcNoInputNoOutput.\r\n");
         break;
      case licKeyboardDisplay:
         printf("   IO Capability:       lcKeyboardDisplay.\r\n");
         break;
   }

   printf("   Bonding Type:        %s.\r\n", (Pairing_Capabilities->Bonding_Type == lbtBonding)?"Bonding":"No Bonding");
   printf("   MITM:                %s.\r\n", (Pairing_Capabilities->MITM)?"TRUE":"FALSE");
   printf("   OOB:                 %s.\r\n", (Pairing_Capabilities->OOB_Present)?"OOB":"OOB Not Present");
   printf("   Encryption Key Size: %d.\r\n", Pairing_Capabilities->Maximum_Encryption_Key_Size);
   printf("   Sending Keys: \r\n");
   printf("      LTK:              %s.\r\n", ((Pairing_Capabilities->Sending_Keys.Encryption_Key)?"YES":"NO"));
   printf("      IRK:              %s.\r\n", ((Pairing_Capabilities->Sending_Keys.Identification_Key)?"YES":"NO"));
   printf("      CSRK:             %s.\r\n", ((Pairing_Capabilities->Sending_Keys.Signing_Key)?"YES":"NO"));
   printf("   Receiving Keys: \r\n");
   printf("      LTK:              %s.\r\n", ((Pairing_Capabilities->Receiving_Keys.Encryption_Key)?"YES":"NO"));
   printf("      IRK:              %s.\r\n", ((Pairing_Capabilities->Receiving_Keys.Identification_Key)?"YES":"NO"));
   printf("      CSRK:             %s.\r\n", ((Pairing_Capabilities->Receiving_Keys.Signing_Key)?"YES":"NO"));
}

   /* The following function is provided to properly print a UUID.      */
static void DisplayUUID(GATT_UUID_t *UUID)
{
   if(UUID)
   {
      if(UUID->UUID_Type == guUUID_16)
         printf("%02X%02X", UUID->UUID.UUID_16.UUID_Byte1, UUID->UUID.UUID_16.UUID_Byte0);
      else
      {
         if(UUID->UUID_Type == guUUID_128)
         {
            printf("%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X", UUID->UUID.UUID_128.UUID_Byte15, UUID->UUID.UUID_128.UUID_Byte14, UUID->UUID.UUID_128.UUID_Byte13,
                                                                                       UUID->UUID.UUID_128.UUID_Byte12, UUID->UUID.UUID_128.UUID_Byte11, UUID->UUID.UUID_128.UUID_Byte10,
                                                                                       UUID->UUID.UUID_128.UUID_Byte9,  UUID->UUID.UUID_128.UUID_Byte8,  UUID->UUID.UUID_128.UUID_Byte7,
                                                                                       UUID->UUID.UUID_128.UUID_Byte6,  UUID->UUID.UUID_128.UUID_Byte5,  UUID->UUID.UUID_128.UUID_Byte4,
                                                                                       UUID->UUID.UUID_128.UUID_Byte3,  UUID->UUID.UUID_128.UUID_Byte2,  UUID->UUID.UUID_128.UUID_Byte1,
                                                                                       UUID->UUID.UUID_128.UUID_Byte0);
         }
      }
   }

   printf(".\r\n");
}

   /* Displays the correct prompt depending on the Server/Client Mode.  */
static void DisplayPrompt(void)
{
   printf("\r\nSPPLE>");

   fflush(stdout);
}

   /* Displays a usage string..                                         */
static void DisplayUsage(char *UsageString)
{
   printf("Usage: %s.\r\n",UsageString);
}

   /* Displays a function error message.                                */
static void DisplayFunctionError(char *Function,int Status)
{
   printf("%s Failed: %d.\r\n", Function, Status);
}

   /* Displays a function success message.                              */
static void DisplayFunctionSuccess(char *Function)
{
   printf("%s success.\r\n",Function);
}

   /* The following function is responsible for opening the SS1         */
   /* Bluetooth Protocol Stack.  This function accepts a pre-populated  */
   /* HCI Driver Information structure that contains the HCI Driver     */
   /* Transport Information.  This function returns zero on successful  */
   /* execution and a negative value on all errors.                     */
static int OpenStack(HCI_DriverInformation_t *HCI_DriverInformation)
{
   int                            Result;
   int                            ret_val = 0;
   char                           BluetoothAddress[16];
   BD_ADDR_t                      BD_ADDR;
   unsigned int                   ServiceID;
   HCI_Version_t                  HCIVersion;
   GAP_LE_Resolving_List_Entry_t  RLEntry;
   unsigned int                   AddedDeviceCount;
   
   /* First check to see if the Stack has already been opened.          */
   if(!BluetoothStackID)
   {
      /* Next, makes sure that the Driver Information passed appears to */
      /* be semi-valid.                                                 */
      if(HCI_DriverInformation)
      {
         printf("\r\n");

         printf("OpenStack().\r\n");

         /* Initialize the Stack                                        */
         Result = BSC_Initialize(HCI_DriverInformation, 0);

         /* Next, check the return value of the initialization to see if*/
         /* it was successful.                                          */
         if(Result > 0)
         {
            /* The Stack was initialized successfully, inform the user  */
            /* and set the return value of the initialization function  */
            /* to the Bluetooth Stack ID.                               */
            BluetoothStackID = Result;
            printf("Bluetooth Stack ID: %d.\r\n", BluetoothStackID);

            /* Initialize the Default Pairing Parameters.               */
            LE_Parameters.IOCapability      = DEFAULT_IO_CAPABILITY;
            LE_Parameters.MITMProtection    = DEFAULT_MITM_PROTECTION;
            LE_Parameters.SecureConnections = DEFAULT_SECURE_CONNECTIONS;
            LE_Parameters.OOBDataPresent    = FALSE;

            if(!HCI_Version_Supported(BluetoothStackID, &HCIVersion))
               printf("Device Chipset: %s.\r\n", (HCIVersion <= NUM_SUPPORTED_HCI_VERSIONS)?HCIVersionStrings[HCIVersion]:HCIVersionStrings[NUM_SUPPORTED_HCI_VERSIONS]);

            /* Let's output the Bluetooth Device Address so that the    */
            /* user knows what the Device Address is.                   */
            if(!GAP_Query_Local_BD_ADDR(BluetoothStackID, &BD_ADDR))
            {
               BD_ADDRToStr(BD_ADDR, BluetoothAddress);

               printf("BD_ADDR: %s\r\n", BluetoothAddress);
            }

            /* Set the IR and ER based on the Bluetooth Address, this   */
            /* allows them to be the same across resets while different */
            /* for different devices. This should be used in testing    */
            /* only, otherwise a random number needs to be used.        */
            BTPS_MemCopy(&((Byte_t *)&IR)[0], &BD_ADDR, BD_ADDR_SIZE);
            BTPS_MemCopy(&((Byte_t *)&ER)[ENCRYPTION_KEY_SIZE - BD_ADDR_SIZE], &BD_ADDR, BD_ADDR_SIZE);

            /* Regenerate IRK, CSRK, and DHK from the constant Identity */
            /* Root Key.                                                */
            GAP_LE_Diversify_Function(BluetoothStackID, (Encryption_Key_t *)(&IR), 1, 0, &IRK);
            GAP_LE_Diversify_Function(BluetoothStackID, (Encryption_Key_t *)(&IR), 3, 0, &DHK);
            GAP_LE_Diversify_Function(BluetoothStackID, (Encryption_Key_t *)(&ER), 1, 0, &CSRK);
            
            Result = GAP_LE_Set_Address_Resolution_Enable(BluetoothStackID, TRUE);
            
            if(!Result)
            {
               printf("Address resolution enabled.\r\n");
            }
            else
            {
               printf("Address resolution could not be enabled: %d.\r\n", Result);
            }
            
            /* Generate a resolvable address and set it.                */
            if(!GAP_LE_Generate_Static_Address(BluetoothStackID, &Resolvable_BD_ADDR))
            {
               if(!GAP_LE_Set_Random_Address(BluetoothStackID, Resolvable_BD_ADDR))
               {
                  BD_ADDRToStr(Resolvable_BD_ADDR, BluetoothAddress);
                  printf("Random address set: %s.\r\n", BluetoothAddress);
               }
            }
            
            /* Set the Resolving List Entry.                            */
            BTPS_MemInitialize(&RLEntry, 0, sizeof(GAP_LE_Resolving_List_Entry_t));
            ResolvingListEntry.Local_IRK = IRK;

            /* Add the device to the resolving list.                    */
            ret_val = GAP_LE_Add_Device_To_Resolving_List(BluetoothStackID, 1, &ResolvingListEntry, &AddedDeviceCount);            

            /* Flag that no connection is currently active.             */
            ASSIGN_BD_ADDR(ConnectionBD_ADDR, 0, 0, 0, 0, 0, 0);
            ASSIGN_BD_ADDR(CurrentRemoteBD_ADDR, 0, 0, 0, 0, 0, 0);
            LocalDeviceIsMaster = FALSE;

            /* Flag that we have no Key Information in the Key List.    */
            DeviceInfoList = NULL;

            /* Initialize the GATT Service.                             */
            if((Result = GATT_Initialize(BluetoothStackID, GATT_INITIALIZATION_FLAGS_SUPPORT_LE, GATT_Connection_Event_Callback, 0)) == 0)
            {
               /* Initialize the GAPS Service.                          */
               Result = GAPS_Initialize_Service(BluetoothStackID, &ServiceID);
               if(Result > 0)
               {
                  /* Save the Instance ID of the GAP Service.           */
                  GAPSInstanceID = (unsigned int)Result;

                  /* Set the GAP Device Name and Device Appearance.     */
                  GAPS_Set_Device_Name(BluetoothStackID, GAPSInstanceID, LE_DEMO_DEVICE_NAME);
                  GAPS_Set_Device_Appearance(BluetoothStackID, GAPSInstanceID, GAP_DEVICE_APPEARENCE_VALUE_GENERIC_COMPUTER);
                        
                  /* Next, let's register for HCI Event Callbacks.      */
                  Result = HCI_Register_Event_Callback(BluetoothStackID, HCI_Event_Callback, 0);

                  if(Result >= 0)
                  {
                     /* Initialize the DIS Service.                     */
                     Result = DIS_Initialize_Service(BluetoothStackID, &ServiceID);
                     if(Result >= 0)
                     {
                        /* Save the Instance ID of the DIS Service.     */
                        DISInstanceID = (unsigned int)Result;

                        /* Set the discoverable attributes              */
                        DIS_Set_Manufacturer_Name(BluetoothStackID, DISInstanceID, "Qualcomm Inc");
                        DIS_Set_Model_Number(BluetoothStackID, DISInstanceID, "Model Bluetopia");
                        DIS_Set_Serial_Number(BluetoothStackID, DISInstanceID, "Serial Number 1234");
                        DIS_Set_Software_Revision(BluetoothStackID, DISInstanceID, "Software Rev 4.1");
                     }
                     else
                        DisplayFunctionError("DIS_Initialize_Service", Result);
                     
                     /* Return success to the caller.                   */
                     ret_val        = 0;
                  }
                  else
                     DisplayFunctionError("HCI_Register_Event_Callback", Result);
               }
               else
               {
                  /* The Stack was NOT initialized successfully, inform */
                  /* the user and set the return value of the           */
                  /* initialization function to an error.               */
                  DisplayFunctionError("GAPS_Initialize_Service", Result);

                  /* Un-registered SPP LE Service.                      */
                  GATT_Un_Register_Service(BluetoothStackID, SPPLEServiceID);

                  /* Cleanup GATT Module.                               */
                  GATT_Cleanup(BluetoothStackID);

                  BluetoothStackID = 0;

                  ret_val          = UNABLE_TO_INITIALIZE_STACK;
               }
            }
            else
            {
               /* The Stack was NOT initialized successfully, inform the*/
               /* user and set the return value of the initialization   */
               /* function to an error.                                 */
               DisplayFunctionError("GATT_Initialize", Result);

               BluetoothStackID = 0;

               ret_val          = UNABLE_TO_INITIALIZE_STACK;
            }
         }
         else
         {
            /* The Stack was NOT initialized successfully, inform the   */
            /* user and set the return value of the initialization      */
            /* function to an error.                                    */
            DisplayFunctionError("BSC_Initialize", Result);

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

   return(ret_val);
}

   /* The following function is responsible for closing the SS1         */
   /* Bluetooth Protocol Stack.  This function requires that the        */
   /* Bluetooth Protocol stack previously have been initialized via the */
   /* OpenStack() function.  This function returns zero on successful   */
   /* execution and a negative value on all errors.                     */
static int CloseStack(void)
{
   int ret_val = 0;

   /* First check to see if the Stack has been opened.                  */
   if(BluetoothStackID)
   {
      /* Cleanup DIS Service Module.                                    */
      if(DISInstanceID)
         DIS_Cleanup_Service(BluetoothStackID, DISInstanceID);

      /* Cleanup GAP Service Module.                                    */
      if(GAPSInstanceID)
         GAPS_Cleanup_Service(BluetoothStackID, GAPSInstanceID);

      /* Un-registered SPP LE Service.                                  */
      if(SPPLEServiceID)
         GATT_Un_Register_Service(BluetoothStackID, SPPLEServiceID);

      /* Cleanup GATT Module.                                           */
      GATT_Cleanup(BluetoothStackID);

      /* Simply close the Stack                                         */
      BSC_Shutdown(BluetoothStackID);

      /* Free BTPSKRNL allocated memory.                                */
      BTPS_DeInit();

      printf("Stack Shutdown.\r\n");

      /* Free the Key List.                                             */
      FreeDeviceInfoList(&DeviceInfoList);

      /* Flag that the Stack is no longer initialized.                  */
      BluetoothStackID = 0;

      /* Flag success to the caller.                                    */
      ret_val          = 0;
   }
   else
   {
      /* A valid Stack ID does not exist, inform to user.               */
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
   int ret_val = 0;

   /* First, check that a valid Bluetooth Stack ID exists.              */
   if(BluetoothStackID)
   {
      /* * NOTE * Discoverability is only applicable when we are        */
      /*          advertising so save the default Discoverability Mode  */
      /*          for later.                                            */
      LE_Parameters.DiscoverabilityMode = dmGeneralDiscoverableMode;
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
   int ret_val = 0;

   /* First, check that a valid Bluetooth Stack ID exists.              */
   if(BluetoothStackID)
   {
      /* * NOTE * Connectability is only an applicable when advertising */
      /*          so we will just save the default connectability for   */
      /*          the next time we enable advertising.                  */
      LE_Parameters.ConnectableMode = lcmConnectable;
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
   int ret_val = 0;

   /* First, check that a valid Bluetooth Stack ID exists.              */
   if(BluetoothStackID)
   {
      /* Attempt to set the attached device to be pairable.             */
      Result = GAP_LE_Set_Pairability_Mode(BluetoothStackID, lpmPairableMode_EnableExtendedEvents);

      /* Default to Secure Connections Only mode being FALSE.           */
      SecureConnectionsOnly = FALSE;

      /* Next, check the return value of the GAP Set Pairability mode   */
      /* command for successful execution.                              */
      if(!Result)
      {
         /* The device has been set to pairable mode, now register an   */
         /* Authentication Callback to handle the Authentication events */
         /* if required.                                                */
         Result = GAP_LE_Register_Remote_Authentication(BluetoothStackID, GAP_LE_Event_Callback, (unsigned long)0);

         /* Next, check the return value of the GAP Register Remote     */
         /* Authentication command for successful execution.            */
         if(Result)
         {
            /* An error occurred while trying to execute this function. */
            DisplayFunctionError("GAP_LE_Register_Remote_Authentication", Result);

            ret_val = Result;
         }
      }
      else
      {
         /* An error occurred while trying to make the device pairable. */
         DisplayFunctionError("GAP_LE_Set_Pairability_Mode", Result);

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

   /* The following function is a utility function that is used to dump */
   /* the Appearance to String Mapping Table.                           */
static void DumpAppearanceMappings(void)
{
   unsigned int Index;

   for(Index=0;Index<NUMBER_OF_APPEARANCE_MAPPINGS;++Index)
      printf("   %u = %s.\r\n", Index, AppearanceMappings[Index].String);
}

   /* The following function is used to map a Appearance Value to it's  */
   /* string representation.  This function returns TRUE on success or  */
   /* FALSE otherwise.                                                  */
static Boolean_t AppearanceToString(Word_t Appearance, char **String)
{
   Boolean_t    ret_val;
   unsigned int Index;

   /* Verify that the input parameters are semi-valid.                  */
   if(String)
   {
      for(Index=0,ret_val=FALSE;Index<NUMBER_OF_APPEARANCE_MAPPINGS;++Index)
      {
         if(AppearanceMappings[Index].Appearance == Appearance)
         {
            *String = AppearanceMappings[Index].String;
            ret_val = TRUE;
            break;
         }
      }
   }
   else
      ret_val = FALSE;

   return(ret_val);
}

   /* The following function is used to map an Index into the Appearance*/
   /* Mapping table to it's Appearance Value.  This function returns    */
   /* TRUE on success or FALSE otherwise.                               */
static Boolean_t AppearanceIndexToAppearance(unsigned int Index, Word_t *Appearance)
{
   Boolean_t ret_val;

   if((Index < NUMBER_OF_APPEARANCE_MAPPINGS) && (Appearance))
   {
      *Appearance = AppearanceMappings[Index].Appearance;
      ret_val     = TRUE;
   }
   else
      ret_val = FALSE;

   return(ret_val);
}

   /* The following function is a utility function that provides a      */
   /* mechanism of populating a Client Information structure with the   */
   /* information discovered from a GATT Discovery operation.           */
static void DISPopulateHandles(DIS_Client_Info_t *ClientInfo, GATT_Service_Discovery_Indication_Data_t *ServiceInfo)
{
   unsigned int                                  Index;
   GATT_Characteristic_Information_t            *CurrentCharacteristic;

   /* Verify that the input parameters are semi-valid.                  */
   if((ClientInfo) && (ServiceInfo) && (ServiceInfo->ServiceInformation.UUID.UUID_Type == guUUID_16) && (DIS_COMPARE_DIS_SERVICE_UUID_TO_UUID_16(ServiceInfo->ServiceInformation.UUID.UUID.UUID_16)))
   {
      /* Initialize the Client Info Structure.                          */
      BTPS_MemInitialize(ClientInfo, 0, sizeof(DIS_Client_Info_t));
      
      /* Loop through all characteristics discovered in the service     */
      /* and populate the correct entry.                                */
      CurrentCharacteristic = ServiceInfo->CharacteristicInformationList;
      if(CurrentCharacteristic)
      {
         for(Index=0;Index<ServiceInfo->NumberOfCharacteristics;Index++,CurrentCharacteristic++)
         {
            /* All DIS UUIDs are defined to be 16 bit UUIDs.            */
            if(CurrentCharacteristic->Characteristic_UUID.UUID_Type == guUUID_16)
            {
               /* Determine which characteristic this is.               */
               if(DIS_COMPARE_DIS_MANUFACTURER_NAME_UUID_TO_UUID_16(CurrentCharacteristic->Characteristic_UUID.UUID.UUID_16))
               {
                  ClientInfo->ManufacturerNameHandle = CurrentCharacteristic->Characteristic_Handle;
                  continue;
               }
               if(DIS_COMPARE_DIS_MODEL_NUMBER_UUID_TO_UUID_16(CurrentCharacteristic->Characteristic_UUID.UUID.UUID_16))
               {
                  ClientInfo->ModelNumberHandle = CurrentCharacteristic->Characteristic_Handle;
                  continue;
               }
               if(DIS_COMPARE_DIS_SERIAL_NUMBER_UUID_TO_UUID_16(CurrentCharacteristic->Characteristic_UUID.UUID.UUID_16))
               {
                  ClientInfo->SerialNumberHandle = CurrentCharacteristic->Characteristic_Handle;
                  continue;
               }
               if(DIS_COMPARE_DIS_HARDWARE_REVISION_UUID_TO_UUID_16(CurrentCharacteristic->Characteristic_UUID.UUID.UUID_16))
               {
                  ClientInfo->HardwareRevisionHandle = CurrentCharacteristic->Characteristic_Handle;
                  continue;
               }
               if(DIS_COMPARE_DIS_FIRMWARE_REVISION_UUID_TO_UUID_16(CurrentCharacteristic->Characteristic_UUID.UUID.UUID_16))
               {
                  ClientInfo->FirmwareRevisionHandle = CurrentCharacteristic->Characteristic_Handle;
                  continue;
               }
               if(DIS_COMPARE_DIS_SOFTWARE_REVISION_UUID_TO_UUID_16(CurrentCharacteristic->Characteristic_UUID.UUID.UUID_16))
               {
                  ClientInfo->SoftwareRevisionHandle = CurrentCharacteristic->Characteristic_Handle;
                  continue;
               }
               if(DIS_COMPARE_DIS_SYSTEM_ID_UUID_TO_UUID_16(CurrentCharacteristic->Characteristic_UUID.UUID.UUID_16))
               {
                  ClientInfo->SystemIDHandle = CurrentCharacteristic->Characteristic_Handle;
                  continue;
               }
               if(DIS_COMPARE_DIS_IEEE_CERTIFICATION_DATA_UUID_TO_UUID_16(CurrentCharacteristic->Characteristic_UUID.UUID.UUID_16))
               {
                  ClientInfo->IEEE11073CertHandle = CurrentCharacteristic->Characteristic_Handle;
                  continue;
               }
               if(DIS_COMPARE_DIS_PNP_ID_UUID_TO_UUID_16(CurrentCharacteristic->Characteristic_UUID.UUID.UUID_16))
               {
                  ClientInfo->PnPIDHandle = CurrentCharacteristic->Characteristic_Handle;
                  continue;
               }
            }
         }
      }
   }
}

   /* The following function is a utility function that provides a      */
   /* mechanism of populating discovered GAP Service Handles.           */
static void GAPSPopulateHandles(GAPS_Client_Info_t *ClientInfo, GATT_Service_Discovery_Indication_Data_t *ServiceInfo)
{
   unsigned int                       Index1;
   GATT_Characteristic_Information_t *CurrentCharacteristic;

   /* Verify that the input parameters are semi-valid.                  */
   if((ClientInfo) && (ServiceInfo) && (ServiceInfo->ServiceInformation.UUID.UUID_Type == guUUID_16) && (GAP_COMPARE_GAP_SERVICE_UUID_TO_UUID_16(ServiceInfo->ServiceInformation.UUID.UUID.UUID_16)))
   {
      /* Loop through all characteristics discovered in the service     */
      /* and populate the correct entry.                                */
      CurrentCharacteristic = ServiceInfo->CharacteristicInformationList;
      if(CurrentCharacteristic)
      {
         for(Index1=0;Index1<ServiceInfo->NumberOfCharacteristics;Index1++,CurrentCharacteristic++)
         {
            /* All GAP Service UUIDs are defined to be 16 bit UUIDs.    */
            if(CurrentCharacteristic->Characteristic_UUID.UUID_Type == guUUID_16)
            {
               /* Determine which characteristic this is.               */
               if(!GAP_COMPARE_GAP_DEVICE_NAME_UUID_TO_UUID_16(CurrentCharacteristic->Characteristic_UUID.UUID.UUID_16))
               {
                  if(!GAP_COMPARE_GAP_DEVICE_APPEARANCE_UUID_TO_UUID_16(CurrentCharacteristic->Characteristic_UUID.UUID.UUID_16))
                     continue;
                  else
                  {
                     ClientInfo->DeviceAppearanceHandle = CurrentCharacteristic->Characteristic_Handle;
                     continue;
                  }
               }
               else
               {
                  ClientInfo->DeviceNameHandle = CurrentCharacteristic->Characteristic_Handle;
                  continue;
               }
            }
         }
      }
   }
}

   /* The following function is a utility function that provides a      */
   /* mechanism of populating a BRSM Client Information structure with  */
   /* the information discovered from a GATT Discovery operation.       */
static void SPPLEPopulateHandles(SPPLE_Client_Info_t *ClientInfo, GATT_Service_Discovery_Indication_Data_t *ServiceInfo)
{
   Word_t                                       *ClientConfigurationHandle;
   unsigned int                                  Index1;
   unsigned int                                  Index2;
   GATT_Characteristic_Information_t            *CurrentCharacteristic;
   GATT_Characteristic_Descriptor_Information_t *CurrentDescriptor;

   /* Verify that the input parameters are semi-valid.                  */
   if((ClientInfo) && (ServiceInfo) && (ServiceInfo->ServiceInformation.UUID.UUID_Type == guUUID_128) && (SPPLE_COMPARE_SPPLE_SERVICE_UUID_TO_UUID_128(ServiceInfo->ServiceInformation.UUID.UUID.UUID_128)))
   {
      /* Loop through all characteristics discovered in the service     */
      /* and populate the correct entry.                                */
      CurrentCharacteristic = ServiceInfo->CharacteristicInformationList;
      if(CurrentCharacteristic)
      {
         for(Index1=0;Index1<ServiceInfo->NumberOfCharacteristics;Index1++,CurrentCharacteristic++)
         {
            /* All SPPLE UUIDs are defined to be 128 bit UUIDs.         */
            if(CurrentCharacteristic->Characteristic_UUID.UUID_Type == guUUID_128)
            {
               ClientConfigurationHandle = NULL;

               /* Determine which characteristic this is.               */
               if(!SPPLE_COMPARE_SPPLE_TX_UUID_TO_UUID_128(CurrentCharacteristic->Characteristic_UUID.UUID.UUID_128))
               {
                  if(!SPPLE_COMPARE_SPPLE_TX_CREDITS_UUID_TO_UUID_128(CurrentCharacteristic->Characteristic_UUID.UUID.UUID_128))
                  {
                     if(!SPPLE_COMPARE_SPPLE_RX_UUID_TO_UUID_128(CurrentCharacteristic->Characteristic_UUID.UUID.UUID_128))
                     {
                        if(!SPPLE_COMPARE_SPPLE_RX_CREDITS_UUID_TO_UUID_128(CurrentCharacteristic->Characteristic_UUID.UUID.UUID_128))
                           continue;
                        else
                        {
                           ClientInfo->Rx_Credit_Characteristic = CurrentCharacteristic->Characteristic_Handle;
                           ClientConfigurationHandle            = &(ClientInfo->Rx_Credit_Client_Configuration_Descriptor);
                        }
                     }
                     else
                     {
                        ClientInfo->Rx_Characteristic = CurrentCharacteristic->Characteristic_Handle;
                        continue;
                     }
                  }
                  else
                  {
                     ClientInfo->Tx_Credit_Characteristic = CurrentCharacteristic->Characteristic_Handle;
                     continue;
                  }
               }
               else
               {
                  ClientInfo->Tx_Characteristic = CurrentCharacteristic->Characteristic_Handle;
                  ClientConfigurationHandle     = &(ClientInfo->Tx_Client_Configuration_Descriptor);
               }

               /* Loop through the Descriptor List.                     */
               CurrentDescriptor = CurrentCharacteristic->DescriptorList;
               if((CurrentDescriptor) && (ClientConfigurationHandle))
               {
                  for(Index2=0;Index2<CurrentCharacteristic->NumberOfDescriptors;Index2++,CurrentDescriptor++)
                  {
                     if(CurrentDescriptor->Characteristic_Descriptor_UUID.UUID_Type == guUUID_16)
                     {
                        if(GATT_COMPARE_CLIENT_CHARACTERISTIC_CONFIGURATION_ATTRIBUTE_TYPE_TO_BLUETOOTH_UUID_16(CurrentDescriptor->Characteristic_Descriptor_UUID.UUID.UUID_16))
                        {
                           *ClientConfigurationHandle = CurrentDescriptor->Characteristic_Descriptor_Handle;
                           break;
                        }
                     }
                  }
               }
            }
         }
      }
   }
}

   /* The following function is a utility function that is used to add  */
   /* data (using InIndex as the buffer index) from the buffer specified*/
   /* by the DataBuffer parameter.  The second and third parameters     */
   /* specified the length of the data to add and the pointer to the    */
   /* data to add to the buffer.  This function returns the actual      */
   /* number of bytes that were added to the buffer (or 0 if none were  */
   /* added).                                                           */
static unsigned int AddDataToBuffer(SPPLE_Data_Buffer_t *DataBuffer, unsigned int DataLength, Byte_t *Data)
{
   unsigned int BytesAdded = 0;
   unsigned int Count;

   /* Verify that the input parameters are valid.                       */
   if((DataBuffer) && (DataLength) && (Data))
   {
      /* Loop while we have data AND space in the buffer.               */
      while(DataLength)
      {
         /* Get the number of bytes that can be placed in the buffer    */
         /* until it wraps.                                             */
         Count = DataBuffer->BufferSize - DataBuffer->InIndex;

         /* Determine if the number of bytes free is less than the      */
         /* number of bytes till we wrap and choose the smaller of the  */
         /* numbers.                                                    */
         Count = (DataBuffer->BytesFree < Count)?DataBuffer->BytesFree:Count;

         /* Cap the Count that we add to buffer to the length of the    */
         /* data provided by the caller.                                */
         Count = (Count > DataLength)?DataLength:Count;

         if(Count)
         {
            /* Copy the data into the buffer.                           */
            BTPS_MemCopy(&DataBuffer->Buffer[DataBuffer->InIndex], Data, Count);

            /* Update the counts.                                       */
            DataBuffer->InIndex   += Count;
            DataBuffer->BytesFree -= Count;
            DataLength            -= Count;
            BytesAdded            += Count;
            Data                  += Count;

            /* Wrap the InIndex if necessary.                           */
            if(DataBuffer->InIndex >= DataBuffer->BufferSize)
               DataBuffer->InIndex = 0;
         }
         else
            break;
      }
   }

   return(BytesAdded);
}

   /* The following function is a utility function that is used to      */
   /* removed data (using OutIndex as the buffer index) from the buffer */
   /* specified by the DataBuffer parameter The second parameter        */
   /* specifies the length of the Buffer that is pointed to by the third*/
   /* parameter.  This function returns the actual number of bytes that */
   /* were removed from the DataBuffer (or 0 if none were added).       */
   /* * NOTE * Buffer is optional and if not specified up to            */
   /*          BufferLength bytes will be deleted from the Buffer.      */
static unsigned int RemoveDataFromBuffer(SPPLE_Data_Buffer_t *DataBuffer, unsigned int BufferLength, Byte_t *Buffer)
{
   unsigned int Count;
   unsigned int BytesRemoved = 0;
   unsigned int MaxRemove;

   /* Verify that the input parameters are valid.                       */
   if((DataBuffer) && (BufferLength))
   {
      /* Loop while we have data to remove and space in the buffer to   */
      /* place it.                                                      */
      while(BufferLength)
      {
         /* Determine the number of bytes that are present in the       */
         /* buffer.                                                     */
         Count = DataBuffer->BufferSize - DataBuffer->BytesFree;
         if(Count)
         {
            /* Calculate the maximum number of bytes that I can remove  */
            /* from the buffer before it wraps.                         */
            MaxRemove = DataBuffer->BufferSize - DataBuffer->OutIndex;

            /* Cap max we can remove at the BufferLength of the caller's*/
            /* buffer.                                                  */
            MaxRemove = (MaxRemove > BufferLength)?BufferLength:MaxRemove;

            /* Cap the number of bytes I will remove in this iteration  */
            /* at the maximum I can remove or the number of bytes that  */
            /* are in the buffer.                                       */
            Count = (Count > MaxRemove)?MaxRemove:Count;

            /* Copy the data into the caller's buffer (If specified).   */
            if(Buffer)
            {
               BTPS_MemCopy(Buffer, &DataBuffer->Buffer[DataBuffer->OutIndex], Count);
               Buffer += Count;
            }

            /* Update the counts.                                       */
            DataBuffer->OutIndex  += Count;
            DataBuffer->BytesFree += Count;
            BytesRemoved          += Count;
            BufferLength          -= Count;

            /* Wrap the OutIndex if necessary.                          */
            if(DataBuffer->OutIndex >= DataBuffer->BufferSize)
               DataBuffer->OutIndex = 0;
         }
         else
            break;
      }
   }

   return(BytesRemoved);
}

   /* The following function is used to initialize the specified buffer */
   /* to the defaults.                                                  */
static void InitializeBuffer(SPPLE_Data_Buffer_t *DataBuffer)
{
   /* Verify that the input parameters are valid.                       */
   if(DataBuffer)
   {
      DataBuffer->BufferSize = SPPLE_DATA_CREDITS;
      DataBuffer->BytesFree  = SPPLE_DATA_CREDITS;
      DataBuffer->InIndex    = 0;
      DataBuffer->OutIndex   = 0;
   }
}

   /* The following function is used to enable/disable notifications on */
   /* a specified handle.  This function returns the positive non-zero  */
   /* Transaction ID of the Write Request or a negative error code.     */
static int EnableDisableNotificationsIndications(Word_t ClientConfigurationHandle, Word_t ClientConfigurationValue, GATT_Client_Event_Callback_t ClientEventCallback)
{
   int              ret_val;
   NonAlignedWord_t Buffer;

   /* Verify the input parameters.                                      */
   if((BluetoothStackID) && (ConnectionID) && (ClientConfigurationHandle))
   {
      ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&Buffer, ClientConfigurationValue);

      ret_val = GATT_Write_Request(BluetoothStackID, ConnectionID, ClientConfigurationHandle, sizeof(Buffer), &Buffer, ClientEventCallback, 0);
   }
   else
      ret_val = BTPS_ERROR_INVALID_PARAMETER;

   return(ret_val);
}

   /* The following function is a utility function that exists to fill  */
   /* the specified buffer with the DataStr that is used to send data.  */
   /* This function will fill from the CurrentBufferLength up to Max    */
   /* Length in Buffer.  CurrentBufferLength is used to return the total*/
   /* length of the buffer.  The first parameter specifies the          */
   /* DeviceInfo which is used to fill any remainder of the string so   */
   /* that there are no breaks in the pattern.  This function returns   */
   /* the number of bytes added to the transmit buffer of the specified */
   /* device.                                                           */
static unsigned int FillBufferWithString(SPPLE_Data_Buffer_t *DataBuffer, unsigned *CurrentBufferLength, unsigned int MaxLength, Byte_t *Buffer)
{
   unsigned int DataCount;
   unsigned int Length;
   unsigned int Added2Buffer = 0;

   /* Verify that the input parameter is semi-valid.                    */
   if((DataBuffer) && (CurrentBufferLength) && (MaxLength) && (Buffer))
   {
      /* Copy as much of the DataStr into the Transmit buffer as is     */
      /* possible.                                                      */
      while(*CurrentBufferLength < MaxLength)
      {
         /* Cap the data to copy at the maximum of the string length and*/
         /* the remaining amount that can be placed in the buffer.      */
         DataCount = (DataStrLen > (MaxLength-*CurrentBufferLength))?(MaxLength-*CurrentBufferLength):DataStrLen;

         /* Build the data string into the SPPLEBuffer.                 */
         BTPS_MemCopy(&Buffer[*CurrentBufferLength], DataStr, DataCount);

         /* Increment the index.                                        */
         *CurrentBufferLength += DataCount;

         /* Add whatever bytes remaining in the DataStr into the        */
         /* transmit buffer to keep the pattern consistent.             */
         Length = DataStrLen-DataCount;
         if(Length)
         {
            /* Add the bytes remaining in the string.                   */
            Added2Buffer += AddDataToBuffer(DataBuffer, Length, (Byte_t *)&DataStr[DataCount]);
         }
      }
   }

   return(Added2Buffer);
}

   /* The following function is responsible for handling a Send Process.*/
static void SendProcess(DeviceInfo_t *DeviceInfo)
{
   int          Result;
   Boolean_t    Done = FALSE;
   unsigned int TransmitIndex;
   unsigned int DataCount;
   unsigned int MaxLength;
   unsigned int SPPLEBufferLength;
   unsigned int Added2Buffer;

   /* Verify that the input parameter is semi-valid.                    */
   if(DeviceInfo)
   {
      /* Loop while we have data to send and we have not used up all    */
      /* Transmit Credits.                                              */
      TransmitIndex     = 0;
      SPPLEBufferLength = 0;
      Added2Buffer      = 0;
      while((SendInfo.BytesToSend) && (DeviceInfo->TransmitCredits) && (!Done))
      {
         /* Get the maximum length of what we can send in this          */
         /* transaction.                                                */
         MaxLength = (SendInfo.BytesToSend > DeviceInfo->TransmitCredits)?DeviceInfo->TransmitCredits:SendInfo.BytesToSend;
         MaxLength = (MaxLength > SPPLE_DATA_BUFFER_LENGTH)?SPPLE_DATA_BUFFER_LENGTH:MaxLength;

         /* If we do not have any outstanding data get some more data.  */
         if(!SPPLEBufferLength)
         {
            /* Send any buffered data first.                            */
            if(DeviceInfo->TransmitBuffer.BytesFree != DeviceInfo->TransmitBuffer.BufferSize)
            {
               /* Remove the queued data from the Transmit Buffer.      */
               SPPLEBufferLength = RemoveDataFromBuffer(&(DeviceInfo->TransmitBuffer), MaxLength, SPPLEBuffer);

               /* If we added some data to the transmit buffer decrement*/
               /* what we just removed.                                 */
               if(Added2Buffer)
                  Added2Buffer -= SPPLEBufferLength;
            }

            /* Fill up the rest of the buffer with the data string.     */
            Added2Buffer     += FillBufferWithString(&(DeviceInfo->TransmitBuffer), &SPPLEBufferLength, MaxLength, SPPLEBuffer);

            /* Set the count of data that we can send.                  */
            DataCount         = SPPLEBufferLength;

            /* Reset the Transmit Index to 0.                           */
            TransmitIndex     = 0;
         }
         else
         {
            /* Move the data that to the beginning of the buffer.       */
            BTPS_MemMove(SPPLEBuffer, &SPPLEBuffer[TransmitIndex], SPPLEBufferLength);

            /* Send any buffered data first.                            */
            if(DeviceInfo->TransmitBuffer.BytesFree != DeviceInfo->TransmitBuffer.BufferSize)
            {
               /* Remove the queued data from the Transmit Buffer.      */
               TransmitIndex = RemoveDataFromBuffer(&(DeviceInfo->TransmitBuffer), MaxLength-SPPLEBufferLength, &SPPLEBuffer[SPPLEBufferLength]);

               /* If we added some data to the transmit buffer decrement*/
               /* what we just removed.                                 */
               if(Added2Buffer)
                  Added2Buffer -= TransmitIndex;

               /* Increment the buffer length.                          */
               SPPLEBufferLength += TransmitIndex;
            }

            /* Reset the Transmit Index to 0.                           */
            TransmitIndex     = 0;

            /* Fill up the rest of the buffer with the data string.     */
            Added2Buffer += FillBufferWithString(&(DeviceInfo->TransmitBuffer), &SPPLEBufferLength, MaxLength, SPPLEBuffer);

            /* We have data to send so cap it at the maximum that can be*/
            /* transmitted.                                             */
            DataCount     = (SPPLEBufferLength > MaxLength)?MaxLength:SPPLEBufferLength;
         }

         /* Use the correct API based on device role for SPPLE.         */
         if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_SPPLE_SERVER)
         {
            /* We are acting as SPPLE Server, so notify the Tx          */
            /* Characteristic.                                          */
            if(DeviceInfo->ServerInfo.Tx_Client_Configuration_Descriptor == GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_NOTIFY_ENABLE)
               Result = GATT_Handle_Value_Notification(BluetoothStackID, SPPLEServiceID, ConnectionID, SPPLE_TX_CHARACTERISTIC_ATTRIBUTE_OFFSET, (Word_t)DataCount, SPPLEBuffer);
            else
            {
               /* Not configured for notifications so exit the loop.    */
               Done = TRUE;
            }
         }
         else
         {
            /* We are acting as SPPLE Client, so write to the Rx        */
            /* Characteristic.                                          */
            if(DeviceInfo->ClientInfo.Tx_Characteristic)
               Result = GATT_Write_Without_Response_Request(BluetoothStackID, ConnectionID, DeviceInfo->ClientInfo.Rx_Characteristic, (Word_t)DataCount, SPPLEBuffer);
            else
            {
               /* We have not discovered the Tx Characteristic, so exit */
               /* the loop.                                             */
               Done = TRUE;
            }
         }

         /* Check to see if any data was written.                       */
         if(!Done)
         {
            /* Check to see if the data was written successfully.       */
            if(Result >= 0)
            {
               /* Adjust the counters.                                  */
               SendInfo.BytesToSend        -= (unsigned int)Result;
               SendInfo.BytesSent          += (unsigned int)Result;
               TransmitIndex               += (unsigned int)Result;
               SPPLEBufferLength           -= (unsigned int)Result;
               DeviceInfo->TransmitCredits -= (unsigned int)Result;

               /* If we have no more remaining Tx Credits AND we have   */
               /* data built up to send, we need to queue this in the Tx*/
               /* Buffer.                                               */
               if((!(DeviceInfo->TransmitCredits)) && (SPPLEBufferLength))
               {
                  /* Add the remaining data to the transmit buffer.     */
                  AddDataToBuffer(&(DeviceInfo->TransmitBuffer), SPPLEBufferLength, &SPPLEBuffer[TransmitIndex]);

                  SPPLEBufferLength = 0;
               }
            }
            else
            {
               printf("SEND failed with error %d\r\n", Result);

               SendInfo.BytesToSend  = 0;
            }
         }
      }

      /* If we have added more bytes to the transmit buffer than we can */
      /* send in this process remove the extra.                         */
      if(Added2Buffer > SendInfo.BytesToSend)
         RemoveDataFromBuffer(&(DeviceInfo->TransmitBuffer), Added2Buffer-SendInfo.BytesToSend, NULL);

      /* Display a message if we have sent all required data.           */
      if((!SendInfo.BytesToSend) && (SendInfo.BytesSent))
      {
         printf("\r\nSend Complete, Sent %u.\r\n", (unsigned int)SendInfo.BytesSent);
         DisplayPrompt();

         SendInfo.BytesSent = 0;
      }
   }
}

   /* The following function is responsible for transmitting the        */
   /* specified number of credits to the remote device.                 */
static void SendCredits(DeviceInfo_t *DeviceInfo, unsigned int DataLength)
{
   NonAlignedWord_t Credits;

   /* Verify that the input parameters are semi-valid.                  */
   if((DeviceInfo) && (DataLength))
   {
      /* Format the credit packet.                                      */
      ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&Credits, DataLength);

      /* Determine how to send credits based on the role.               */
      if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_SPPLE_SERVER)
      {
         /* We are acting as a server so notify the Rx Credits          */
         /* characteristic.                                             */
         if(DeviceInfo->ServerInfo.Rx_Credit_Client_Configuration_Descriptor == GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_NOTIFY_ENABLE)
            GATT_Handle_Value_Notification(BluetoothStackID, SPPLEServiceID, ConnectionID, SPPLE_RX_CREDITS_CHARACTERISTIC_ATTRIBUTE_OFFSET, WORD_SIZE, (Byte_t *)&Credits);
      }
      else
      {
         /* We are acting as a client so send a Write Without Response  */
         /* packet to the Tx Credit Characteristic.                     */
         if(DeviceInfo->ClientInfo.Tx_Credit_Characteristic)
            GATT_Write_Without_Response_Request(BluetoothStackID, ConnectionID, DeviceInfo->ClientInfo.Tx_Credit_Characteristic, WORD_SIZE, &Credits);
      }
   }
}

   /* The following function is responsible for handling a received     */
   /* credit, event.                                                    */
static void ReceiveCreditEvent(DeviceInfo_t *DeviceInfo, unsigned int Credits)
{
   /* Verify that the input parameters are semi-valid.                  */
   if(DeviceInfo)
   {
      /* If this is a real credit event store the number of credits.    */
      DeviceInfo->TransmitCredits += Credits;

      /* Handle any active send process.                                */
      SendProcess(DeviceInfo);

      /* Send all queued data.                                          */
      SendData(DeviceInfo, 0, NULL);

      /* It is possible that we have received data queued, so call the  */
      /* Data Indication Event to handle this.                          */
      DataIndicationEvent(DeviceInfo, 0, NULL);
   }
}

   /* The following function sends the specified data to the specified  */
   /* data.  This function will queue any of the data that does not go  */
   /* out.  This function returns TRUE if all the data was sent, or     */
   /* FALSE.                                                            */
   /* * NOTE * If DataLength is 0 and Data is NULL then all queued data */
   /*          will be sent.                                            */
static Boolean_t SendData(DeviceInfo_t *DeviceInfo, unsigned int DataLength, Byte_t *Data)
{
   int          Result;
   Boolean_t    DataSent = FALSE;
   Boolean_t    Done;
   unsigned int DataCount;
   unsigned int MaxLength;
   unsigned int TransmitIndex;
   unsigned int SPPLEBufferLength;

   /* Verify that the input parameters are semi-valid.                  */
   if(DeviceInfo)
   {
      /* Loop while we have data to send and we can send it.            */
      Done              = FALSE;
      TransmitIndex     = 0;
      SPPLEBufferLength = 0;
      while(!Done)
      {
         /* Check to see if we have credits to use to transmit the data.*/
         if(DeviceInfo->TransmitCredits)
         {
            /* Get the maximum length of what we can send in this       */
            /* transaction.                                             */
            MaxLength = (SPPLE_DATA_BUFFER_LENGTH > DeviceInfo->TransmitCredits)?DeviceInfo->TransmitCredits:SPPLE_DATA_BUFFER_LENGTH;

            /* If we do not have any outstanding data get some more     */
            /* data.                                                    */
            if(!SPPLEBufferLength)
            {
               /* Send any buffered data first.                         */
               if(DeviceInfo->TransmitBuffer.BytesFree != DeviceInfo->TransmitBuffer.BufferSize)
               {
                  /* Remove the queued data from the Transmit Buffer.   */
                  SPPLEBufferLength = RemoveDataFromBuffer(&(DeviceInfo->TransmitBuffer), MaxLength, SPPLEBuffer);
               }
               else
               {
                  /* Check to see if we have data to send.              */
                  if((DataLength) && (Data))
                  {
                     /* Copy the data to send into the SPPLEBuffer.     */
                     SPPLEBufferLength = (DataLength > MaxLength)?MaxLength:DataLength;
                     BTPS_MemCopy(SPPLEBuffer, Data, SPPLEBufferLength);

                     DataLength -= SPPLEBufferLength;
                     Data       += SPPLEBufferLength;
                  }
                  else
                  {
                     /* No data queued or data left to send so exit the */
                     /* loop.                                           */
                     Done = TRUE;
                  }
               }

               /* Set the count of data that we can send.               */
               DataCount         = SPPLEBufferLength;

               /* Reset the Transmit Index to 0.                        */
               TransmitIndex     = 0;
            }
            else
            {
               /* We have data to send so cap it at the maximum that can*/
               /* be transmitted.                                       */
               DataCount = (SPPLEBufferLength > MaxLength)?MaxLength:SPPLEBufferLength;
            }

            /* Try to write data if not exiting the loop.               */
            if(!Done)
            {
               /* Use the correct API based on device role for SPPLE.   */
               if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_SPPLE_SERVER)
               {
                  /* We are acting as SPPLE Server, so notify the Tx    */
                  /* Characteristic.                                    */
                  if(DeviceInfo->ServerInfo.Tx_Client_Configuration_Descriptor == GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_NOTIFY_ENABLE)
                     Result = GATT_Handle_Value_Notification(BluetoothStackID, SPPLEServiceID, ConnectionID, SPPLE_TX_CHARACTERISTIC_ATTRIBUTE_OFFSET, (Word_t)DataCount, &SPPLEBuffer[TransmitIndex]);
                  else
                  {
                     /* Not configured for notifications so exit the    */
                     /* loop.                                           */
                     Done = TRUE;
                  }
               }
               else
               {
                  /* We are acting as SPPLE Client, so write to the Rx  */
                  /* Characteristic.                                    */
                  if(DeviceInfo->ClientInfo.Tx_Characteristic)
                     Result = GATT_Write_Without_Response_Request(BluetoothStackID, ConnectionID, DeviceInfo->ClientInfo.Rx_Characteristic, (Word_t)DataCount, &SPPLEBuffer[TransmitIndex]);
                  else
                  {
                     /* We have not discovered the Tx Characteristic, so*/
                     /* exit the loop.                                  */
                     Done = TRUE;
                  }
               }

               /* Check to see if any data was written.                 */
               if(!Done)
               {
                  /* Check to see if the data was written successfully. */
                  if(Result >= 0)
                  {
                     /* Adjust the counters.                            */
                     TransmitIndex               += (unsigned int)Result;
                     SPPLEBufferLength           -= (unsigned int)Result;
                     DeviceInfo->TransmitCredits -= (unsigned int)Result;

                     /* Flag that data was sent.                        */
                     DataSent                     = TRUE;

                     /* If we have no more remaining Tx Credits AND we  */
                     /* have data built up to send, we need to queue    */
                     /* this in the Tx Buffer.                          */
                     if((!(DeviceInfo->TransmitCredits)) && (SPPLEBufferLength))
                     {
                        /* Add the remaining data to the transmit       */
                        /* buffer.                                      */
                        AddDataToBuffer(&(DeviceInfo->TransmitBuffer), SPPLEBufferLength, &SPPLEBuffer[TransmitIndex]);

                        SPPLEBufferLength = 0;
                     }
                  }
                  else
                  {
                     printf("SEND failed with error %d\r\n", Result);

                     DataSent  = FALSE;
                  }
               }
            }
         }
         else
         {
            /* We have no transmit credits, so buffer the data.         */
            DataCount = AddDataToBuffer(&(DeviceInfo->TransmitBuffer), DataLength, Data);
            if(DataCount == DataLength)
               DataSent = TRUE;
            else
               DataSent = FALSE;

            /* Exit the loop.                                           */
            Done = TRUE;
         }
      }
   }

   return(DataSent);
}

   /* The following function is responsible for handling a data         */
   /* indication event.                                                 */
static void DataIndicationEvent(DeviceInfo_t *DeviceInfo, unsigned int DataLength, Byte_t *Data)
{
   Boolean_t    Done;
   unsigned int ReadLength;
   unsigned int Length;

   /* Verify that the input parameters are semi-valid.                  */
   if(DeviceInfo)
   {
      /* If we are automatically reading the data, go ahead and credit  */
      /* what we just received, as well as reading everything in the    */
      /* buffer.                                                        */
      if((AutomaticReadActive) || (LoopbackActive))
      {
         /* Loop until we read all of the data queued.                  */
         Done = FALSE;
         while(!Done)
         {
            /* If in loopback mode cap what we remove at the max of what*/
            /* we can send or queue.                                    */
            if(LoopbackActive)
               ReadLength = (SPPLE_DATA_BUFFER_LENGTH > (DeviceInfo->TransmitCredits + DeviceInfo->TransmitBuffer.BytesFree))?(DeviceInfo->TransmitCredits + DeviceInfo->TransmitBuffer.BytesFree):SPPLE_DATA_BUFFER_LENGTH;
            else
               ReadLength = SPPLE_DATA_BUFFER_LENGTH;

            /* Read all queued data.                                    */
            Length = ReadData(DeviceInfo, ReadLength, SPPLEBuffer);
            if(Length > 0)
            {
               /* If loopback is active, loopback the data.             */
               if(LoopbackActive)
                  SendData(DeviceInfo, Length, SPPLEBuffer);

               /* If we are displaying the data then do that here.      */
               if(DisplayRawData)
               {
                  SPPLEBuffer[Length] = '\0';
                  printf("%s", (char *)SPPLEBuffer);
                  fflush(stdout);
               }
            }
            else
               Done = TRUE;
         }

         /* Only send/display data just received if any is specified in */
         /* the call to this function.                                  */
         if((DataLength) && (Data))
         {
            /* If loopback is active, loopback the data just received.  */
            if((AutomaticReadActive) || (LoopbackActive))
            {
               /* If we are displaying the data then do that here.      */
               if(DisplayRawData)
               {
                  BTPS_MemCopy(SPPLEBuffer, Data, DataLength);
                  SPPLEBuffer[DataLength] = '\0';
                  printf("%s", (char *)SPPLEBuffer);
                  fflush(stdout);
               }

               /* Check to see if Loopback is active, if it is we will  */
               /* loopback the data we just received.                   */
               if(LoopbackActive)
               {
                  /* Only queue the data in the receive buffer that we  */
                  /* cannot send.                                       */
                  ReadLength = (DataLength > (DeviceInfo->TransmitCredits + DeviceInfo->TransmitBuffer.BytesFree))?(DeviceInfo->TransmitCredits + DeviceInfo->TransmitBuffer.BytesFree):DataLength;

                  /* Send the data.                                     */
                  if(SendData(DeviceInfo, ReadLength, Data))
                  {
                     /* Credit the data we just sent.                   */
                     SendCredits(DeviceInfo, ReadLength);

                     /* Increment what was just sent.                   */
                     DataLength -= ReadLength;
                     Data       += ReadLength;
                  }
               }
               else
               {
                  /* Loopback is not active so just credit back the data*/
                  /* we just received.                                  */
                  SendCredits(DeviceInfo, DataLength);

                  DataLength = 0;
               }

               /* If we have data left that cannot be sent, queue this  */
               /* in the receive buffer.                                */
               if((DataLength) && (Data))
               {
                  /* We are not in Loopback or Automatic Read Mode so   */
                  /* just buffer all the data.                          */
                  Length = AddDataToBuffer(&(DeviceInfo->ReceiveBuffer), DataLength, Data);
                  if(Length != DataLength)
                     printf("Receive Buffer Overflow of %u bytes", DataLength - Length);
               }
            }

            /* If we are displaying the data then do that here.         */
            if(DisplayRawData)
            {
               BTPS_MemCopy(SPPLEBuffer, Data, DataLength);
               SPPLEBuffer[DataLength] = '\0';
               printf("%s", (char *)SPPLEBuffer);
            }
         }
      }
      else
      {
         if((DataLength) && (Data))
         {
            /* Display a Data indication event.                         */
            printf("\r\nData Indication Event, Connection ID %u, Received %u bytes.\r\n", ConnectionID, DataLength);

            /* We are not in Loopback or Automatic Read Mode so just    */
            /* buffer all the data.                                     */
            Length = AddDataToBuffer(&(DeviceInfo->ReceiveBuffer), DataLength, Data);
            if(Length != DataLength)
               printf("Receive Buffer Overflow of %u bytes.\r\n", DataLength - Length);
         }
      }
   }
}

   /* The following function is used to read data from the specified    */
   /* device.  The final two parameters specify the BufferLength and the*/
   /* Buffer to read the data into.  On success this function returns   */
   /* the number of bytes read.  If an error occurs this will return a  */
   /* negative error code.                                              */
static int ReadData(DeviceInfo_t *DeviceInfo, unsigned int BufferLength, Byte_t *Buffer)
{
   int          ret_val;
   Boolean_t    Done;
   unsigned int Length;
   unsigned int TotalLength;

   /* Verify that the input parameters are semi-valid.                  */
   if((DeviceInfo) && (BufferLength) && (Buffer))
   {
      Done        = FALSE;
      TotalLength = 0;
      while(!Done)
      {
         Length = RemoveDataFromBuffer(&(DeviceInfo->ReceiveBuffer), BufferLength, Buffer);
         if(Length > 0)
         {
            BufferLength -= Length;
            Buffer       += Length;
            TotalLength   = Length;
         }
         else
            Done = TRUE;
      }

      /* Credit what we read.                                           */
      SendCredits(DeviceInfo, TotalLength);

      /* Return the total number of bytes read.                         */
      ret_val = (int)TotalLength;
   }
   else
      ret_val = BTPS_ERROR_INVALID_PARAMETER;

   return(ret_val);
}

   /* The following function is responsible for starting a scan.        */
static int StartScan(unsigned int BluetoothStackID, Boolean_t UseWhitelist, GAP_LE_Address_Type_t LocalAddressType)
{
   int Result;

   /* First, determine if the input parameters appear to be semi-valid. */
   if(BluetoothStackID)
   {
      /* Not currently scanning, go ahead and attempt to perform the    */
      /* scan.                                                          */
      Result = GAP_LE_Perform_Scan(BluetoothStackID, stActive, 10, 10, LocalAddressType, (UseWhitelist) ? fpWhiteList : fpNoFilter, TRUE, GAP_LE_Event_Callback, 0);

      if(!Result)
      {
         printf("Scan started successfully.\r\n");
      }
      else
      {
         /* Unable to start the scan.                                   */
         printf("Unable to perform scan: %d\r\n", Result);
      }
   }
   else
      Result = -1;

   return(Result);
}

   /* The following function is responsible for stopping on on-going    */
   /* scan.                                                             */
static int StopScan(unsigned int BluetoothStackID)
{
   int Result;

   /* First, determine if the input parameters appear to be semi-valid. */
   if(BluetoothStackID)
   {
      Result = GAP_LE_Cancel_Scan(BluetoothStackID);
      if(!Result)
      {
         printf("Scan stopped successfully.\r\n");
      }
      else
      {
         /* Error stopping scan.                                        */
         printf("Unable to stop scan: %d\r\n", Result);
      }
   }
   else
      Result = -1;

   return(Result);
}

   /* The following function is responsible for creating an LE          */
   /* connection to the specified Remote Device.                        */
static int ConnectLEDevice(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR, GAP_LE_Address_Type_t PeerAddressType, GAP_LE_Address_Type_t LocalAddressType, Boolean_t UseWhiteList)
{
   int                            Result;
   //unsigned int                   WhiteListChanged;
   //GAP_LE_White_List_Entry_t      WhiteListEntry;
   GAP_LE_Connection_Parameters_t ConnectionParameters;

   /* First, determine if the input parameters appear to be semi-valid. */
   if((BluetoothStackID) && (!COMPARE_NULL_BD_ADDR(BD_ADDR)))
   {
      if(COMPARE_NULL_BD_ADDR(ConnectionBD_ADDR))
      {
         /* Remove any previous entries for this device from the White  */
         /* List.                                                       */
         //WhiteListEntry.Address_Type = PeerAddressType;
         //WhiteListEntry.Address      = BD_ADDR;

         //GAP_LE_Remove_Device_From_White_List(BluetoothStackID, 1, &WhiteListEntry, &WhiteListChanged);

         //if(UseWhiteList)
            //Result = GAP_LE_Add_Device_To_White_List(BluetoothStackID, 1, &WhiteListEntry, &WhiteListChanged);
         //else
            Result = 1;

         /* If everything has been successful, up until this point, then*/
         /* go ahead and attempt the connection.                        */
         if(Result >= 0)
         {
            /* Initialize the connection parameters.                    */
            ConnectionParameters.Connection_Interval_Min    = 50;
            ConnectionParameters.Connection_Interval_Max    = 200;
            ConnectionParameters.Minimum_Connection_Length  = 0;
            ConnectionParameters.Maximum_Connection_Length  = 10000;
            ConnectionParameters.Slave_Latency              = 0;
            ConnectionParameters.Supervision_Timeout        = 20000;

            /* Everything appears correct, go ahead and attempt to make */
            /* the connection.                                          */
            Result = GAP_LE_Create_Connection(BluetoothStackID, 100, 100, Result ? fpNoFilter : fpWhiteList, (PeerAddressType == latPublic) ? latPublicIdentity : latRandomIdentity, Result ? &BD_ADDR : NULL, LocalAddressType, &ConnectionParameters, GAP_LE_Event_Callback, 0);

            if(!Result)
            {
               printf("Connection Request successful.\r\n");

               /* Note the connection information.                      */
               ConnectionBD_ADDR     = BD_ADDR;
               ConnectionAddressType = (PeerAddressType == latPublic) ? latPublicIdentity : latRandomIdentity;
            }
            else
            {
               /* Unable to create connection.                          */
               printf("Unable to create connection: %d.\r\n", Result);
            }
         }
         else
         {
            /* Unable to add device to White List.                      */
            printf("Unable to add device to White List.\r\n");
         }
      }
      else
      {
         /* Device already connected.                                   */
         printf("Device is already connected.\r\n");

         Result = -2;
      }
   }
   else
      Result = -1;

   return(Result);
}

   /* The following function is provided to allow a mechanism to        */
   /* disconnect a currently connected device.                          */
static int DisconnectLEDevice(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR)
{
   int Result;

   /* First, determine if the input parameters appear to be semi-valid. */
   if((BluetoothStackID) && (!COMPARE_NULL_BD_ADDR(BD_ADDR)))
   {
      if(!COMPARE_NULL_BD_ADDR(ConnectionBD_ADDR))
      {
         Result = GAP_LE_Disconnect(BluetoothStackID, BD_ADDR);

         if(!Result)
         {
            printf("Disconnect Request successful.\r\n");
         }
         else
         {
            /* Unable to disconnect device.                             */
            printf("Unable to disconnect device: %d.\r\n", Result);
         }
      }
      else
      {
         /* Device not connected.                                       */
         printf("Device is not connected.\r\n");

         Result = 0;
      }
   }
   else
      Result = -1;

   return(Result);
}

   /* The following function provides a mechanism to configure a        */
   /* Pairing Capabilities structure with the application's pairing     */
   /* parameters.                                                       */
static void ConfigureCapabilities(GAP_LE_Extended_Pairing_Capabilities_t *Capabilities)
{
   /* Make sure the Capabilities pointer is semi-valid.                 */
   if(Capabilities)
   {
      /* Initialize the capabilities.                                   */
      BTPS_MemInitialize(Capabilities, 0, GAP_LE_EXTENDED_PAIRING_CAPABILITIES_SIZE);

      /* Configure the Pairing Capabilities structure.                  */
      Capabilities->Bonding_Type                    = lbtBonding;
      Capabilities->IO_Capability                   = LE_Parameters.IOCapability;
      Capabilities->Maximum_Encryption_Key_Size     = GAP_LE_MAXIMUM_ENCRYPTION_KEY_SIZE;
      Capabilities->Flags                           = 0;

      /* Configure the flags for the capability structure.              */
      if(LE_Parameters.MITMProtection)
         Capabilities->Flags |= GAP_LE_EXTENDED_PAIRING_CAPABILITIES_FLAGS_MITM_REQUESTED;

      if(LE_Parameters.SecureConnections)
         Capabilities->Flags |= GAP_LE_EXTENDED_PAIRING_CAPABILITIES_FLAGS_SECURE_CONNECTIONS;

      if(LE_Parameters.OOBDataPresent)
         Capabilities->Flags |= GAP_LE_EXTENDED_PAIRING_CAPABILITIES_FLAGS_OOB_DATA_PRESENT;

      /* This application only demonstrates using Long Term Key's (LTK) */
      /* for encryption of a LE Link, however we could request and send */
      /* all possible keys here if we wanted to.                        */
      Capabilities->Receiving_Keys.Encryption_Key     = TRUE;
      Capabilities->Receiving_Keys.Identification_Key = TRUE;
      Capabilities->Receiving_Keys.Signing_Key        = TRUE;
      Capabilities->Receiving_Keys.Link_Key           = FALSE;

      Capabilities->Sending_Keys.Encryption_Key       = TRUE;
      Capabilities->Sending_Keys.Identification_Key   = TRUE;
      Capabilities->Sending_Keys.Signing_Key          = TRUE;
      Capabilities->Sending_Keys.Link_Key             = FALSE;
   }
}

   /* The following function provides a mechanism for sending a pairing */
   /* request to a device that is connected on an LE Link.              */
static int SendPairingRequest(BD_ADDR_t BD_ADDR, Boolean_t ConnectionMaster)
{
   int                                    ret_val;
   BoardStr_t                             BoardStr;
   GAP_LE_Extended_Pairing_Capabilities_t ExtendedCapabilities;

   /* Make sure a Bluetooth Stack is open.                              */
   if(BluetoothStackID)
   {
      /* Make sure the BD_ADDR is valid.                                */
      if(!COMPARE_NULL_BD_ADDR(BD_ADDR))
      {
         /* Configure the application pairing parameters.               */
         ConfigureCapabilities(&ExtendedCapabilities);

         /* Set the BD_ADDR of the device that we are attempting to pair*/
         /* with.                                                       */
         CurrentRemoteBD_ADDR = BD_ADDR;

         BD_ADDRToStr(BD_ADDR, BoardStr);
         printf("Attempting to Pair to %s.\r\n", BoardStr);

         DisplayPairingInformation(&ExtendedCapabilities);

         /* Attempt to pair to the remote device.                       */
         if(ConnectionMaster)
         {
            /* Start the pairing process.                               */
            if((ret_val = GAP_LE_Extended_Pair_Remote_Device(BluetoothStackID, BD_ADDR, &ExtendedCapabilities, GAP_LE_Event_Callback, 0)) == BTPS_ERROR_SECURE_CONNECTIONS_NOT_SUPPORTED)
            {
               /* P-256 Public Key isn't valid so do NOT allow Secure   */
               /* Connections.                                          */
               printf("No P-256 Public Key, disabling Secure Connections.\r\n");

               ExtendedCapabilities.Flags &= ~GAP_LE_EXTENDED_PAIRING_CAPABILITIES_FLAGS_SECURE_CONNECTIONS;

               /* Try this again.                                       */
               ret_val = GAP_LE_Extended_Pair_Remote_Device(BluetoothStackID, BD_ADDR, &ExtendedCapabilities, GAP_LE_Event_Callback, 0);
            }

            printf("     GAP_LE_Extended_Pair_Remote_Device returned %d.\r\n", ret_val);
         }
         else
         {
            /* As a slave we can only request that the Master start     */
            /* the pairing process.                                     */
            ret_val = GAP_LE_Extended_Request_Security(BluetoothStackID, BD_ADDR, &ExtendedCapabilities, GAP_LE_Event_Callback, 0);

            printf("     GAP_LE_Extended_Request_Security returned %d.\r\n", ret_val);
         }
      }
      else
      {
         printf("Invalid Parameters.\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      printf("Stack ID Invalid.\r\n");

      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function provides a mechanism of sending a Slave    */
   /* Pairing Response to a Master's Pairing Request.                   */
static int SlavePairingRequestResponse(BD_ADDR_t BD_ADDR, Boolean_t AcceptRequest)
{
   int                                          ret_val;
   BoardStr_t                                   BoardStr;
   GAP_LE_Authentication_Response_Information_t AuthenticationResponseData;

   /* Make sure a Bluetooth Stack is open.                              */
   if(BluetoothStackID)
   {
      BD_ADDRToStr(BD_ADDR, BoardStr);
      printf("Sending Pairing Response to %s.\r\n", BoardStr);

      /* We must be the slave if we have received a Pairing Request     */
      /* thus we will respond with our capabilities.                    */
      AuthenticationResponseData.GAP_LE_Authentication_Type = larPairingCapabilities;
      
      /* If we are accepting this request, set the authentication data  */
      /* length, otherwise set it to 0 to reject the request.           */
      if(AcceptRequest)
      {
         AuthenticationResponseData.Authentication_Data_Length = GAP_LE_EXTENDED_PAIRING_CAPABILITIES_SIZE;
      }
      else
      {
         AuthenticationResponseData.Authentication_Data_Length = 0;
      }

      /* Configure the Application Pairing Parameters.                  */
      ConfigureCapabilities(&(AuthenticationResponseData.Authentication_Data.Extended_Pairing_Capabilities));

      /* Attempt to pair to the remote device.                          */
      if((ret_val = GAP_LE_Authentication_Response(BluetoothStackID, BD_ADDR, &AuthenticationResponseData)) == BTPS_ERROR_SECURE_CONNECTIONS_NOT_SUPPORTED)
      {
         /* P-256 Public Key isn't valid so do NOT allow Secure         */
         /* Connections.                                                */
         printf("No P-256 Public Key, disabling Secure Connections.\r\n");

         AuthenticationResponseData.Authentication_Data.Extended_Pairing_Capabilities.Flags &= ~GAP_LE_EXTENDED_PAIRING_CAPABILITIES_FLAGS_SECURE_CONNECTIONS;

         /* Try this again.                                             */
         ret_val = GAP_LE_Authentication_Response(BluetoothStackID, BD_ADDR, &AuthenticationResponseData);
      }

      printf("GAP_LE_Authentication_Response returned %d.\r\n", ret_val);
   }
   else
   {
      printf("Stack ID Invalid.\r\n");

      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is provided to allow a mechanism of        */
   /* responding to a request for Encryption Information to send to a   */
   /* remote device.                                                    */
static int EncryptionInformationRequestResponse(BD_ADDR_t BD_ADDR, Byte_t KeySize, GAP_LE_Authentication_Response_Information_t *GAP_LE_Authentication_Response_Information)
{
   int    ret_val;
   Word_t LocalDiv;

   /* Make sure a Bluetooth Stack is open.                              */
   if(BluetoothStackID)
   {
      /* Make sure the input parameters are semi-valid.                 */
      if((!COMPARE_NULL_BD_ADDR(BD_ADDR)) && (GAP_LE_Authentication_Response_Information))
      {
         printf("   Calling GAP_LE_Generate_Long_Term_Key.\r\n");

         /* Generate a new LTK, EDIV and Rand tuple.                    */
         ret_val = GAP_LE_Generate_Long_Term_Key(BluetoothStackID, (Encryption_Key_t *)(&DHK), (Encryption_Key_t *)(&ER), &(GAP_LE_Authentication_Response_Information->Authentication_Data.Encryption_Information.LTK), &LocalDiv, &(GAP_LE_Authentication_Response_Information->Authentication_Data.Encryption_Information.EDIV), &(GAP_LE_Authentication_Response_Information->Authentication_Data.Encryption_Information.Rand));
         if(!ret_val)
         {
            printf("   Encryption Information Request Response.\r\n");

            /* Response to the request with the LTK, EDIV and Rand      */
            /* values.                                                  */
            GAP_LE_Authentication_Response_Information->GAP_LE_Authentication_Type                                     = larEncryptionInformation;
            GAP_LE_Authentication_Response_Information->Authentication_Data_Length                                     = GAP_LE_ENCRYPTION_INFORMATION_DATA_SIZE;
            GAP_LE_Authentication_Response_Information->Authentication_Data.Encryption_Information.Encryption_Key_Size = KeySize;

            ret_val = GAP_LE_Authentication_Response(BluetoothStackID, BD_ADDR, GAP_LE_Authentication_Response_Information);
            if(!ret_val)
            {
               printf("   GAP_LE_Authentication_Response (larEncryptionInformation) success.\r\n");
            }
            else
            {
               printf("   Error - SM_Generate_Long_Term_Key returned %d.\r\n", ret_val);
            }
         }
         else
         {
            printf("   Error - SM_Generate_Long_Term_Key returned %d.\r\n", ret_val);
         }
      }
      else
      {
         printf("Invalid Parameters.\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      printf("Stack ID Invalid.\r\n");

      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for setting the application */
   /* state to support loopback mode.  This function will return zero on*/
   /* successful execution and a negative value on errors.              */
static int Loopback(ParameterList_t *TempParam)
{
   int ret_val;

   /* First check to see if the parameters required for the execution of*/
   /* this function appear to be semi-valid.                            */
   if(BluetoothStackID)
   {
      /* Next check to see if the parameters required for the execution */
      /* of this function appear to be semi-valid.                      */
      if((TempParam) && (TempParam->NumberofParameters > 0))
      {
         if(TempParam->Params->intParam)
            LoopbackActive = TRUE;
         else
            LoopbackActive = FALSE;
      }
      else
         LoopbackActive = (LoopbackActive?FALSE:TRUE);

      /* Finally output the current Loopback state.                     */
      printf("Current Loopback Mode set to: %s.\r\n", LoopbackActive?"ACTIVE":"INACTIVE");

      /* Flag success.                                                  */
      ret_val = 0;
   }
   else
   {
      /* One or more of the necessary parameters are invalid.           */
      ret_val = INVALID_PARAMETERS_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for setting the application */
   /* state to support displaying Raw Data.  This function will return  */
   /* zero on successful execution and a negative value on errors.      */
static int DisplayRawModeData(ParameterList_t *TempParam)
{
   int ret_val;

   /* First check to see if the parameters required for the execution of*/
   /* this function appear to be semi-valid.                            */
   if(BluetoothStackID)
   {
      /* Check to see if Loopback is active.  If it is then we will not */
      /* process this command (and we will inform the user).            */
      if(!LoopbackActive)
      {
         /* Next check to see if the parameters required for the        */
         /* execution of this function appear to be semi-valid.         */
         if((TempParam) && (TempParam->NumberofParameters > 0))
         {
            if(TempParam->Params->intParam)
               DisplayRawData = TRUE;
            else
               DisplayRawData = FALSE;
         }
         else
            DisplayRawData = (DisplayRawData?FALSE:TRUE);

         /* Output the current Raw Data Display Mode state.             */
         printf("Current Raw Data Display Mode set to: %s.\r\n", DisplayRawData?"ACTIVE":"INACTIVE");

         /* Flag that the function was successful.                      */
         ret_val = 0;
      }
      else
      {
         printf("Unable to process Raw Mode Display Request when operating in Loopback Mode.\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* One or more of the necessary parameters are invalid.           */
      ret_val = INVALID_PARAMETERS_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for setting the application */
   /* state to support Automatically reading all data that is received  */
   /* through SPP.  This function will return zero on successful        */
   /* execution and a negative value on errors.                         */
static int AutomaticReadMode(ParameterList_t *TempParam)
{
   int ret_val;

   /* First check to see if the parameters required for the execution of*/
   /* this function appear to be semi-valid.                            */
   if(BluetoothStackID)
   {
      /* Check to see if Loopback is active.  If it is then we will not */
      /* process this command (and we will inform the user).            */
      if(!LoopbackActive)
      {
         /* Next check to see if the parameters required for the        */
         /* execution of this function appear to be semi-valid.         */
         if((TempParam) && (TempParam->NumberofParameters > 0))
         {
            if(TempParam->Params->intParam)
               AutomaticReadActive = TRUE;
            else
               AutomaticReadActive = FALSE;
         }
         else
            AutomaticReadActive = (AutomaticReadActive?FALSE:TRUE);

         /* Output the current Automatic Read Mode state.               */
         printf("Current Automatic Read Mode set to: %s.\r\n", AutomaticReadActive?"ACTIVE":"INACTIVE");

         /* Flag that the function was successful.                      */
         ret_val = 0;
      }
      else
      {
         printf("Unable to process Automatic Read Mode Request when operating in Loopback Mode.\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* One or more of the necessary parameters are invalid.           */
      ret_val = INVALID_PARAMETERS_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for displaying the current  */
   /* Command Options for either Serial Port Client or Serial Port      */
   /* Server.  The input parameter to this function is completely       */
   /* ignored, and only needs to be passed in because all Commands that */
   /* can be entered at the Prompt pass in the parsed information.  This*/
   /* function displays the current Command Options that are available  */
   /* and always returns zero.                                          */
static int DisplayHelp(ParameterList_t *TempParam)
{
   printf("\r\n");
   printf("******************************************************************\r\n");
   printf("* Command Options General: Help, GetLocalAddress,                *\r\n");
   printf("*                          EnableDebug, GetMTU, SetMTU           *\r\n");
   printf("* Command Options 4.2:     SetDataLength, ReadMaxDataLength      *\r\n");
   printf("*                          QueryDefaultDataLength,               *\r\n");
   printf("*                          SetDefaultDataLength,                 *\r\n");
//   printf("*                          UpdateP256Key                         *\r\n");
// xxx   printf("*                                       , GenerateDHKey          *\r\n");
   printf("*                          AddResolvingList,                     *\r\n");
   printf("*                          RemoveResolvingList,                  *\r\n");
   printf("*                          ReadResolvingListSize,                *\r\n");
   printf("*                          ReadPeerRPA, ReadLocalRPA,            *\r\n");
   printf("*                          SetAddressResolutionEnable,           *\r\n");
   printf("*                          SetRPATimeout, AddWhiteList,          *\r\n");
   printf("*                          RemoveWhiteList, ReadWhiteListSize    *\r\n");
   printf("* Command Options GAPLE:   UpdateConnectionParams,               *\r\n");
   printf("*                          SetDiscoverabilityMode,               *\r\n");
   printf("*                          SetConnectabilityMode,                *\r\n");
   printf("*                          SetPairabilityMode,                   *\r\n");
   printf("*                          ChangePairingParameters,              *\r\n");
   printf("*                          AdvertiseLE, StartScanning,           *\r\n");
   printf("*                          StopScanning, ConnectLE,              *\r\n");
   printf("*                          CancelConnect, DisconnectLE,          *\r\n");
   printf("*                          PairLE, UnpairLE,                     *\r\n");
   printf("*                          LEPasskeyResponse,                    *\r\n");
   printf("*                          QueryEncryptionMode, SetPasskey,      *\r\n");
   printf("*                          DiscoverGAPS, DiscoverDIS,            *\r\n");
   printf("*                          GetLocalName, SetLocalName,           *\r\n");
   printf("*                          GetRemoteName,                        *\r\n");
   printf("*                          SetLocalAppearance,                   *\r\n");
   printf("*                          GetLocalAppearance,                   *\r\n");
   printf("*                          GetRemoteAppearance,                  *\r\n");
   printf("* Command Options SPPLE:   DiscoverSPPLE, RegisterSPPLE,         *\r\n");
   printf("*                          UnregisterSPPLE, SendData,            *\r\n");
   printf("*                          ConfigureSPPLE, ReadData, Loopback,   *\r\n");
   printf("*                          DisplayRawModeData, AutomaticReadMode *\r\n");
   printf("******************************************************************\r\n");

   return(0);
}

   /* The following function is responsible for updating the connection */
   /* parameters.                                                       */
static int UpdateConnectionParams(ParameterList_t *TempParam)
{
   int                            ret_val;
   GAP_LE_Connection_Parameters_t ConnectionParams;
   
   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters > 5))
      {
         /* Check to see if a device is connected.                      */
         if(!COMPARE_NULL_BD_ADDR(ConnectionBD_ADDR))
         {
            ConnectionParams.Connection_Interval_Min    = TempParam->Params[0].intParam;
            ConnectionParams.Connection_Interval_Max    = TempParam->Params[1].intParam;
            ConnectionParams.Slave_Latency              = TempParam->Params[2].intParam;
            ConnectionParams.Supervision_Timeout        = TempParam->Params[3].intParam;
            ConnectionParams.Minimum_Connection_Length  = TempParam->Params[4].intParam;
            ConnectionParams.Maximum_Connection_Length  = TempParam->Params[5].intParam;
         
            ret_val = GAP_LE_Update_Connection_Parameters(BluetoothStackID, ConnectionBD_ADDR, &ConnectionParams);
         
            if(!ret_val)
            {
               printf("GAP_LE_Update_Connection_Parameters() success.\r\n");
            }
            else
            {
               printf("GAP_LE_Update_Connection_Parameters() failure: %d.\r\n", ret_val);
               
               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* There is no currently connected device.                  */
            printf("Not Connected.\r\n");

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         DisplayUsage("UpdateConnectionParams [ConnectionIntervalMin] [ConnectionIntervalMax] [SlaveLatency] [SupervisionTimeout] [MinConnectionLength] [MaxConnectionLength]");

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
   /* Discoverability Mode of the local device.  This function returns  */
   /* zero on successful execution and a negative value on all errors.  */
static int SetDiscoverabilityMode(ParameterList_t *TempParam)
{
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

         /* Set the LE Discoveryability Mode.                           */
         LE_Parameters.DiscoverabilityMode = DiscoverabilityMode;

         /* The Mode was changed successfully.                          */
         printf("Discoverability: %s.\r\n", (DiscoverabilityMode == dmNonDiscoverableMode)?"Non":((DiscoverabilityMode == dmGeneralDiscoverableMode)?"General":"Limited"));

         /* Flag success to the caller.                                 */
         ret_val = 0;
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         DisplayUsage("SetDiscoverabilityMode [Mode(0 = Non Discoverable, 1 = Limited Discoverable, 2 = General Discoverable)]");

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
   int ret_val;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 1) && (TempParam->Params[0].intParam >= 0) && (TempParam->Params[0].intParam <= 1))
      {
         /* Parameters appear to be valid, map the specified parameters */
         /* into the API specific parameters.                           */
         /* * NOTE * The Connectability Mode in LE is only applicable   */
         /*          when advertising, if a device is not advertising   */
         /*          it is not connectable.                             */
         if(TempParam->Params[0].intParam == 0)
            LE_Parameters.ConnectableMode = lcmNonConnectable;
         else
            LE_Parameters.ConnectableMode = lcmConnectable;

         /* The Mode was changed successfully.                          */
         printf("Connectability Mode: %s.\r\n", (LE_Parameters.ConnectableMode == lcmNonConnectable)?"Non Connectable":"Connectable");

         /* Flag success to the caller.                                 */
         ret_val = 0;
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         DisplayUsage("SetConnectabilityMode [(0 = NonConectable, 1 = Connectable)]");

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
   int                        Result;
   int                        ret_val;
   char                      *Mode;
   GAP_LE_Pairability_Mode_t  PairabilityMode;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].intParam >= 0) && (TempParam->Params[0].intParam <= 2))
      {
         /* Parameters appear to be valid, map the specified parameters */
         /* into the API specific parameters.                           */
         PairabilityMode = lpmNonPairableMode;
         Mode            = "lpmNonPairableMode";

         if(TempParam->Params[0].intParam == 1)
         {
            PairabilityMode       = lpmPairableMode_EnableExtendedEvents;
            Mode                  = "lpmPairableMode";
            SecureConnectionsOnly = FALSE;
         }

         if(TempParam->Params[0].intParam == 2)
         {
            PairabilityMode                 = lpmPairableMode_EnableExtendedEvents;
            Mode                            = "lpmPairableMode";
            SecureConnectionsOnly           = TRUE;
            LE_Parameters.SecureConnections = TRUE;
         }

         /* Parameters mapped, now set the Pairability Mode.            */
         Result = GAP_LE_Set_Pairability_Mode(BluetoothStackID, PairabilityMode);

         /* Next, check the return value to see if the command was      */
         /* issued successfully.                                        */
         if(Result >= 0)
         {
            /* The Mode was changed successfully.                       */
            if(!SecureConnectionsOnly)
            {
               printf("Pairability Mode Changed to %s.\r\n", Mode);
            }
            else
            {
               printf("Pairability Mode Changed to %s, Secure Connections Only.\r\n", Mode);
            }

            /* If Secure Simple Pairing has been enabled, inform the    */
            /* user of the current Secure Simple Pairing parameters.    */
            if(PairabilityMode == lpmPairableMode)
               DisplayIOCapabilities();

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            /* There was an error setting the Mode.                     */
            DisplayFunctionError("GAP_Set_Pairability_Mode", Result);

            /* Flag that an error occurred while submitting the command.*/
            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         DisplayUsage("SetPairabilityMode [Mode (0 = Non Pairable, 1 = Pairable, 2 = Pairable, Secure Connections Only]");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      printf("Invalid Stack ID.\r\n");

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
static int ChangePairingParameters(ParameterList_t *TempParam)
{
   int ret_val;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters == 3) && (TempParam->Params[0].intParam >= 0) && (TempParam->Params[0].intParam <= 4))
      {
         /* Parameters appear to be valid, map the specified parameters */
         /* into the API specific parameters.                           */
         if(TempParam->Params[0].intParam == 0)
            LE_Parameters.IOCapability = licDisplayOnly;
         else
         {
            if(TempParam->Params[0].intParam == 1)
               LE_Parameters.IOCapability = licDisplayYesNo;
            else
            {
               if(TempParam->Params[0].intParam == 2)
                  LE_Parameters.IOCapability = licKeyboardOnly;
               else
               {
                  if(TempParam->Params[0].intParam == 3)
                     LE_Parameters.IOCapability = licNoInputNoOutput;
                  else
                     LE_Parameters.IOCapability = licKeyboardDisplay;
               }
            }
         }

         /* Map the Man in the Middle (MITM) Protection valid.          */
         LE_Parameters.MITMProtection    = (Boolean_t)(TempParam->Params[1].intParam?TRUE:FALSE);

         /* Finally, map the secure connections valid.                  */
         LE_Parameters.SecureConnections = (Boolean_t)(TempParam->Params[2].intParam?TRUE:FALSE);
         
         /* Set Secure Connections Only mode to false if we have        */
         /* disabled secure connections.                                */
         if(LE_Parameters.SecureConnections == FALSE)
         {
            SecureConnectionsOnly = FALSE;
         }

         /* Inform the user of the New I/O Capabilities.                */
         DisplayIOCapabilities();

         /* Flag success to the caller.                                 */
         ret_val = 0;
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: ChangePairingParameters [I/O Capability (0 = Display Only, 1 = Display Yes/No, 2 = Keyboard Only, 3 = No Input/Output, 4 = Keyboard/Display)] [MITM Requirement (0 = No, 1 = Yes)] [Secure Connections (0 = No, 1 = Yes)].\r\n");

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
   /* Authentication Response with a Pass Key value specified via the   */
   /* input parameter.  This function returns zero on successful        */
   /* execution and a negative value on all errors.                     */
static int LEPassKeyResponse(ParameterList_t *TempParam)
{
   int                                           Result;
   int                                           ret_val;
   GAP_LE_Authentication_Response_Information_t  GAP_LE_Authentication_Response_Information;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* First, check to see if there is an on-going Pairing operation  */
      /* active.                                                        */
      if(!COMPARE_NULL_BD_ADDR(CurrentRemoteBD_ADDR))
      {
         /* Make sure that all of the parameters required for this      */
         /* function appear to be at least semi-valid.                  */
         if((TempParam) && (TempParam->NumberofParameters > 0) && (BTPS_StringLength(TempParam->Params[0].strParam) <= GAP_LE_PASSKEY_MAXIMUM_NUMBER_OF_DIGITS))
         {
            /* Parameters appear to be valid, go ahead and populate the */
            /* response structure.                                      */
            GAP_LE_Authentication_Response_Information.GAP_LE_Authentication_Type  = larPasskey;
            GAP_LE_Authentication_Response_Information.Authentication_Data_Length  = (Byte_t)(sizeof(DWord_t));
            GAP_LE_Authentication_Response_Information.Authentication_Data.Passkey = (DWord_t)(TempParam->Params[0].intParam);

            /* Submit the Authentication Response.                      */
            Result = GAP_LE_Authentication_Response(BluetoothStackID, CurrentRemoteBD_ADDR, &GAP_LE_Authentication_Response_Information);

            /* Check the return value for the submitted command for     */
            /* success.                                                 */
            if(!Result)
            {
               /* Operation was successful, inform the user.            */
               DisplayFunctionSuccess("Passkey Response");

               /* Flag success to the caller.                           */
               ret_val = 0;
            }
            else
            {
               /* Inform the user that the Authentication Response was  */
               /* not successful.                                       */
               DisplayFunctionError("GAP_LE_Authentication_Response", Result);

               ret_val = FUNCTION_ERROR;
            }

            /* Flag that there is no longer a current Authentication    */
            /* procedure in progress.                                   */
            ASSIGN_BD_ADDR(CurrentRemoteBD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
         }
         else
         {
            /* One or more of the necessary parameters is/are invalid.  */
            printf("PassKeyResponse [Numeric Passkey(0 - 999999)].\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* There is not currently an on-going authentication operation,*/
         /* inform the user of this error condition.                    */
         printf("Pass Key Authentication Response: Authentication not in progress.\r\n");

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

   /* The following function is responsible for querying the Encryption */
   /* Mode for an LE Connection.  This function returns zero on         */
   /* successful execution and a negative value on all errors.          */
static int LEQueryEncryption(ParameterList_t *TempParam)
{
   int                   ret_val;
   GAP_Encryption_Mode_t GAP_Encryption_Mode;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* First, check to see if there is an on-going Pairing operation  */
      /* active.                                                        */
      if(!COMPARE_NULL_BD_ADDR(ConnectionBD_ADDR))
      {
         /* Query the current Encryption Mode for this Connection.      */
         ret_val = GAP_LE_Query_Encryption_Mode(BluetoothStackID, ConnectionBD_ADDR, &GAP_Encryption_Mode);
         if(!ret_val)
            printf("Current Encryption Mode: %s.\r\n", (GAP_Encryption_Mode == emEnabled)?"Enabled":"Disabled");
         else
         {
            printf("Error - GAP_LE_Query_Encryption_Mode returned %d.\r\n", ret_val);

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* There is not currently an on-going authentication operation,*/
         /* inform the user of this error condition.                    */
         printf("Not Connected.\r\n");

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

   /* The following function is responsible for querying the Encryption */
   /* Mode for an LE Connection.  This function returns zero on         */
   /* successful execution and a negative value on all errors.          */
static int LESetPasskey(ParameterList_t *TempParam)
{
   int     ret_val;
   DWord_t Passkey;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Make sure that all of the parameters required for this         */
      /* function appear to be at least semi-valid.                     */
      if((TempParam) && (TempParam->NumberofParameters >= 1) && ((TempParam->Params[0].intParam == 0) || (TempParam->Params[0].intParam == 1)))
      {
         if(TempParam->Params[0].intParam == 1)
         {
            /* We are setting the passkey so make sure it is valid.     */
            if(BTPS_StringLength(TempParam->Params[1].strParam) <= GAP_LE_PASSKEY_MAXIMUM_NUMBER_OF_DIGITS)
            {
               Passkey = (DWord_t)(TempParam->Params[1].intParam);

               ret_val = GAP_LE_Set_Fixed_Passkey(BluetoothStackID, &Passkey);
               if(!ret_val)
                  printf("Fixed Passkey set to %06u.\r\n", (unsigned int)Passkey);
            }
            else
            {
               printf("Error - Invalid Passkey.\r\n");

               ret_val = INVALID_PARAMETERS_ERROR;
            }
         }
         else
         {
            /* Un-set the fixed passkey that we previously configured.  */
            ret_val = GAP_LE_Set_Fixed_Passkey(BluetoothStackID, NULL);
            if(!ret_val)
               printf("Fixed Passkey no longer configured.\r\n");
         }

         /* If GAP_LE_Set_Fixed_Passkey returned an error display this. */
         if((ret_val) && (ret_val != INVALID_PARAMETERS_ERROR))
         {
            printf("Error - GAP_LE_Set_Fixed_Passkey returned %d.\r\n", ret_val);

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("SetPasskey [(0 = UnSet Passkey, 1 = Set Fixed Passkey)] [6 Digit Passkey (optional)].\r\n");

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
   /* Device Address of the local Bluetooth Device.  This function      */
   /* returns zero on successful execution and a negative value on all  */
   /* errors.                                                           */
static int GetLocalAddress(ParameterList_t *TempParam)
{
   int        Result;
   int        ret_val;
   BD_ADDR_t  BD_ADDR;
   BoardStr_t BoardStr;

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
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for enabling LE             */
   /* Advertisements.  This function returns zero on successful         */
   /* execution and a negative value on all errors.                     */
static int AdvertiseLE(ParameterList_t *TempParam)
{
   int                                 ret_val;
   int                                 Length;
   GAP_LE_Advertising_Parameters_t     AdvertisingParameters;
   GAP_LE_Connectability_Parameters_t  ConnectabilityParameters;
   char                                NameBuffer[BTPS_CONFIGURATION_GAPS_MAXIMUM_SUPPORTED_DEVICE_NAME+1];
   union
   {
      Advertising_Data_t               AdvertisingData;
      Scan_Response_Data_t             ScanResponseData;
   } Advertisement_Data_Buffer;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters > 0) && ((TempParam->Params[0].intParam == 0) || (TempParam->NumberofParameters > 2)) && ((TempParam->Params[0].intParam == 0) || (TempParam->Params[2].intParam == 0) || (TempParam->NumberofParameters > 4)))
      {
         /* Determine whether to enable or disable Advertising.         */
         if(TempParam->Params[0].intParam == 0)
         {
            /* Disable Advertising.                                     */
            ret_val = GAP_LE_Advertising_Disable(BluetoothStackID);
            if(!ret_val)
               printf("   GAP_LE_Advertising_Disable success.\r\n");
            else
            {
               printf("   GAP_LE_Advertising_Disable returned %d.\r\n", ret_val);

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* Enable Advertising.                                      */
            /* Set the Advertising Data.                                */
            BTPS_MemInitialize(&(Advertisement_Data_Buffer.AdvertisingData), 0, sizeof(Advertising_Data_t));

            /* Set the Flags A/D Field (1 byte type and 1 byte Flags.   */
            Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[0] = 2;
            Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[1] = HCI_LE_ADVERTISING_REPORT_DATA_TYPE_FLAGS;
            Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[2] = 0;

            /* Configure the flags field based on the Discoverability   */
            /* Mode.                                                    */
            if(LE_Parameters.DiscoverabilityMode == dmGeneralDiscoverableMode)
               Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[2] = HCI_LE_ADVERTISING_FLAGS_GENERAL_DISCOVERABLE_MODE_FLAGS_BIT_MASK;
            else
            {
               if(LE_Parameters.DiscoverabilityMode == dmLimitedDiscoverableMode)
                  Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[2] = HCI_LE_ADVERTISING_FLAGS_LIMITED_DISCOVERABLE_MODE_FLAGS_BIT_MASK;
            }

            /* Write thee advertising data to the chip.                 */
            ret_val = GAP_LE_Set_Advertising_Data(BluetoothStackID, (Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[0] + 1), &(Advertisement_Data_Buffer.AdvertisingData));
            if(!ret_val)
            {
               BTPS_MemInitialize(&(Advertisement_Data_Buffer.ScanResponseData), 0, sizeof(Scan_Response_Data_t));

               Length = 0;

               /* Verify that the GAP Service is registered.            */
               if(GAPSInstanceID)
               {
                  /* Initialize the Name Buffer to all zeros.           */
                  BTPS_MemInitialize(NameBuffer, 0, sizeof(NameBuffer));

                  /* Query the Local Name.                              */
                  ret_val = GAPS_Query_Device_Name(BluetoothStackID, GAPSInstanceID, NameBuffer);
                              
                  if(!ret_val)
                  {
                     /* Set the Scan Response Data.                     */
                     Length = BTPS_StringLength(NameBuffer);
                  }
               }
   
               if(Length < (ADVERTISING_DATA_MAXIMUM_SIZE - 2))
               {
                  Advertisement_Data_Buffer.ScanResponseData.Scan_Response_Data[1] = HCI_LE_ADVERTISING_REPORT_DATA_TYPE_LOCAL_NAME_COMPLETE;
               }
               else
               {
                  Advertisement_Data_Buffer.ScanResponseData.Scan_Response_Data[1] = HCI_LE_ADVERTISING_REPORT_DATA_TYPE_LOCAL_NAME_SHORTENED;
                  Length = (ADVERTISING_DATA_MAXIMUM_SIZE - 2);
               }

               Advertisement_Data_Buffer.ScanResponseData.Scan_Response_Data[0] = (Byte_t)(1 + Length);
               BTPS_MemCopy(&(Advertisement_Data_Buffer.ScanResponseData.Scan_Response_Data[2]), NameBuffer, Length);

               ret_val = GAP_LE_Set_Scan_Response_Data(BluetoothStackID, (Advertisement_Data_Buffer.ScanResponseData.Scan_Response_Data[0] + 1), &(Advertisement_Data_Buffer.ScanResponseData));
               if(!ret_val)
               {
                  /* Set up the advertising parameters.                 */
                  AdvertisingParameters.Advertising_Channel_Map   = HCI_LE_ADVERTISING_CHANNEL_MAP_DEFAULT;
                  AdvertisingParameters.Scan_Request_Filter       = fpNoFilter;
                  AdvertisingParameters.Connect_Request_Filter    = fpNoFilter;
                  AdvertisingParameters.Advertising_Interval_Min  = 100;
                  AdvertisingParameters.Advertising_Interval_Max  = 200;
                    
                  /* Set the address type.                              */
                  if((TempParam->Params[1].intParam == 0) || (TempParam->Params[1].intParam > 2))
                  {
                     ConnectabilityParameters.Own_Address_Type = latPublic;
                  }
                  else
                  {
                     if(TempParam->Params[1].intParam == 1)
                     {
                        ConnectabilityParameters.Own_Address_Type = latRandom;
                     }
                     else
                     {
                        ConnectabilityParameters.Own_Address_Type = latResolvableFallbackPrivate;
                     }
                  }
                  
                  /* Handle whether or not to use directed advertising  */
                  if(TempParam->Params[2].intParam == 0)
                  {
                     ConnectabilityParameters.Connectability_Mode = lcmConnectable;
                     
                     /* If using an RPA, make sure to set the peer      */
                     /* address and type to that in the resolving list. */
                     if((ConnectabilityParameters.Own_Address_Type == latResolvableFallbackPrivate) && (!COMPARE_NULL_BD_ADDR(ResolvingListEntry.Peer_Identity_Address)))
                     {
                        ConnectabilityParameters.Direct_Address_Type = ResolvingListEntry.Peer_Identity_Address_Type;
                        ConnectabilityParameters.Direct_Address      = ResolvingListEntry.Peer_Identity_Address;
                     }
                     else
                     {
                        ConnectabilityParameters.Direct_Address_Type = latPublic;
                        ASSIGN_BD_ADDR(ConnectabilityParameters.Direct_Address, 0, 0, 0, 0, 0, 0);
                     }
                  }
                  else
                  {
                     ConnectabilityParameters.Connectability_Mode = lcmLowDutyCycleDirectConnectable;
                     
                     /* Convert the parameter to a Bluetooth Address.   */
                     StrToBD_ADDR(TempParam->Params[3].strParam, &ConnectabilityParameters.Direct_Address);
                     ConnectabilityParameters.Direct_Address_Type = (TempParam->Params[4].intParam ? latRandomIdentity : latPublicIdentity);
                  }

                  /* Now enable advertising.                            */
                  ret_val = GAP_LE_Advertising_Enable(BluetoothStackID, TRUE, &AdvertisingParameters, &ConnectabilityParameters, GAP_LE_Event_Callback, 0);
                  if(!ret_val)
                  {
                     printf("   GAP_LE_Advertising_Enable success.\r\n");
                  }
                  else
                  {
                     printf("   GAP_LE_Advertising_Enable returned %d.\r\n", ret_val);

                     ret_val = FUNCTION_ERROR;
                  }
               }
               else
               {
                  printf("   GAP_LE_Set_Advertising_Data(dtScanResponse) returned %d.\r\n", ret_val);

                  ret_val = FUNCTION_ERROR;
               }

            }
            else
            {
               printf("   GAP_LE_Set_Advertising_Data(dtAdvertising) returned %d.\r\n", ret_val);

               ret_val = FUNCTION_ERROR;
            }
         }
      }
      else
      {
         DisplayUsage("AdvertiseLE [(0 = Disable, 1 = Enable)] [Address Type (0 = Public, 1 = Random, 2 = RPA)] [Directed Advertising (0 = Disable, 1 = Enable)] [Directed Address (optional)] [Directed Address Type (optional) (0 = Public, 1 = Random)]");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(0);
}

   /* The following function is responsible for starting an LE scan     */
   /* procedure.  This function returns zero if successful and a        */
   /* negative value if an error occurred.                              */
static int StartScanning(ParameterList_t *TempParam)
{
   int                   ret_val;
   GAP_LE_Address_Type_t LocalAddressType;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters > 1))
      {
         if(!ScanInProgress)
         {
            ScanInProgress = TRUE;
            
            /* Set the local address type.                              */
            if((TempParam->Params[1].intParam == 0) || (TempParam->Params[1].intParam > 2))
            {
               LocalAddressType = latPublic;
            }
            else
            {
               if(TempParam->Params[1].intParam == 1)
               {
                  LocalAddressType = latRandom;
               }
               else
               {
                  LocalAddressType = latResolvableFallbackPrivate;
               }
            }

            /* Simply start scanning.                                   */
            if(!StartScan(BluetoothStackID, (Boolean_t)(TempParam->Params[0].intParam), LocalAddressType))
               ret_val = 0;
            else
               ret_val = FUNCTION_ERROR;
         }
         else
         {
            printf("\r\nScan already in progress!\r\n");

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         DisplayUsage("StartScanning [UseWhiteList (0 = FALSE, 1 = TRUE)] [Local ADDR Type (0 = Public, 1 = Random, 2 = RPA)]");

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

   /* The following function is responsible for stopping an LE scan     */
   /* procedure.  This function returns zero if successful and a        */
   /* negative value if an error occurred.                              */
static int StopScanning(ParameterList_t *TempParam)
{
   int ret_val;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      if(ScanInProgress)
      {
         ScanInProgress = FALSE;

         /* Simply stop scanning.                                       */
         if(!StopScan(BluetoothStackID))
            ret_val = 0;
         else
            ret_val = FUNCTION_ERROR;
      }
      else
      {
         printf("\r\nScan is not in progress.\r\n");

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

   /* The following function is responsible for connecting to an LE     */
   /* device.  This function returns zero if successful and a negative  */
   /* value if an error occurred.                                       */
static int ConnectLE(ParameterList_t *TempParam)
{
   int                   ret_val;
   BD_ADDR_t             BD_ADDR;
   GAP_LE_Address_Type_t LocalAddressType;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      if(ScanInProgress == TRUE)
      {
         StopScan(BluetoothStackID);
         
         ScanInProgress = FALSE;

         printf("\r\nScan stopped before making LE Connection\r\n");
      }

      /* Next, make sure that a valid device address exists.            */
      if((TempParam) && (TempParam->NumberofParameters > 1) && (TempParam->Params[0].strParam) && (BTPS_StringLength(TempParam->Params[0].strParam) == (sizeof(BD_ADDR_t)*2)))
      {
         /* Convert the parameter to a Bluetooth Device Address.        */
         StrToBD_ADDR(TempParam->Params[0].strParam, &BD_ADDR);

         if ((TempParam->Params[1].intParam >= 0) && (TempParam->Params[1].intParam <= 2))
         {
            if((TempParam->Params[2].intParam == 0) || (TempParam->Params[2].intParam > 2))
            {
               LocalAddressType = latPublic;
            }
            else
            {
               if(TempParam->Params[2].intParam == 1)
               {
                  LocalAddressType = latRandom;
               }
               else
               {
                  LocalAddressType = latResolvableFallbackPrivate;
               }
            }
         
            if(!ConnectLEDevice(BluetoothStackID, BD_ADDR, TempParam->Params[1].intParam ? latRandom : latPublic, LocalAddressType, FALSE))
               ret_val = 0;
            else
               ret_val = FUNCTION_ERROR;
         }
         else
         {
               printf("Usage: ConnectLE [BD_ADDR] [Peer ADDR Type (0 = Public, 1 = Random)] [Local ADDR Type (0 = Public, 1 = Random, 2 = RPA)].\r\n");
               ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* Invalid parameters specified so flag an error to the user.  */
         printf("Usage: ConnectLE [BD_ADDR] [Peer ADDR Type (0 = Public, 1 = Random)] [Local ADDR Type (0 = Public, 1 = Random, 2 = RPA)].\r\n");

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

   /* The following function is responsible for cancelling a pending    */
   /* connection request. This function returns zero if successful and  */
   /* a negative value if an error occurred.                            */
static int CancelConnect(ParameterList_t *TempParam)
{
   int                   ret_val;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Cancel the connection.                                         */
      ret_val = GAP_LE_Cancel_Create_Connection(BluetoothStackID);   

      if(!ret_val)
      {
         /* Clear the saved Connection BD_ADDR.                         */
         ASSIGN_BD_ADDR(ConnectionBD_ADDR, 0, 0, 0, 0, 0, 0);
         
         printf("Cancelled connection request.\r\n");
         ret_val = 0;
      }
      else
      {
         printf("GAP_LE_Cancel_Create_Connection() returned %d.\r\n", ret_val);
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

   /* The following function is responsible for disconnecting to an LE  */
   /* device.  This function returns zero if successful and a negative  */
   /* value if an error occurred.                                       */
static int DisconnectLE(ParameterList_t *TempParam)
{
   int ret_val;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Next, check to make sure we are currently connected.           */
      if(!COMPARE_NULL_BD_ADDR(ConnectionBD_ADDR))
      {
         if(!DisconnectLEDevice(BluetoothStackID, ConnectionBD_ADDR))
            ret_val = 0;
         else
            ret_val = FUNCTION_ERROR;
      }
      else
      {
         printf("Device is not connected.\r\n");

         /* Flag success to the caller.                                 */
         ret_val = 0;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is provided to allow a mechanism of        */
   /* Pairing (or requesting security if a slave) to the connected      */
   /* device.                                                           */
static int PairLE(ParameterList_t *TempParam)
{
   int ret_val;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Next, check to make sure we are currently connected.           */
      if(!COMPARE_NULL_BD_ADDR(ConnectionBD_ADDR))
      {
         if(!SendPairingRequest(ConnectionBD_ADDR, LocalDeviceIsMaster))
            ret_val = 0;
         else
            ret_val = FUNCTION_ERROR;
      }
      else
      {
         printf("Device is not connected.\r\n");

         /* Flag success to the caller.                                 */
         ret_val = 0;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is provided to allow a mechanism of        */
   /* unpairing with the connected device.                              */
static int UnpairLE(ParameterList_t *TempParam)
{
   int           ret_val;
   DeviceInfo_t *DeviceInfo;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Get the device info for the connection device.                 */
      if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
      {
         /* Clear all pairing information.                              */
         BTPS_MemInitialize(&DeviceInfo->PeerIRK, 0, sizeof(Encryption_Key_t));
         BTPS_MemInitialize(&DeviceInfo->LTK, 0, LONG_TERM_KEY_SIZE);
         BTPS_MemInitialize(&DeviceInfo->Rand, 0, RANDOM_NUMBER_DATA_SIZE);
         BTPS_MemInitialize(&DeviceInfo->EDIV, 0, WORD_SIZE);
         
         DeviceInfo->Flags &= ~(DEVICE_INFO_FLAGS_LTK_VALID);
         DeviceInfo->Flags &= ~(DEVICE_INFO_FLAGS_PEER_IDENTITY_INFO_VALID);
         
         printf("Device unpaired.\r\n");
         
         ret_val = 0;
      }
      else
      {
         printf("No Device Info.\r\n");

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

   /* The following function is responsible for performing a DIS Service*/
   /* Service Discovery Operation.  This function will return zero on   */
   /* successful execution and a negative value on errors.              */
static int DiscoverDIS(ParameterList_t *TempParam)
{
   int           ret_val;
   GATT_UUID_t   UUID;
   DeviceInfo_t *DeviceInfo;

   /* Verify that there is a connection that is established.            */
   if(ConnectionID)
   {
      /* Get the device info for the connection device.                 */
      if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
      {
         /* Verify that no service discovery is outstanding for this    */
         /* device.                                                     */
         if(!(DeviceInfo->Flags & DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_OUTSTANDING))
         {
            /* Configure the filter so that only the DIS Service is     */
            /* discovered.                                              */
            UUID.UUID_Type = guUUID_16;
            DIS_ASSIGN_DIS_SERVICE_UUID_16(UUID.UUID.UUID_16);

            BTPS_MemInitialize(&DeviceInfo->DISClientInfo, 0, sizeof(DIS_Client_Info_t));

            ret_val = GATT_Start_Service_Discovery(BluetoothStackID, ConnectionID, 1, &UUID, GATT_Service_Discovery_Event_Callback, 0);

            if(!ret_val)
            {
               printf("GATT_Service_Discovery_Start() success.\r\n");

               /* Flag that a Service Discovery Operation is            */
               /* outstanding.                                          */
               DeviceInfo->Flags          |= (DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_OUTSTANDING | DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_DIS);
               DeviceInfo->DISHandleIndex  = 0;
            }
            else
            {
               /* An error occur so just clean-up.                      */
               printf("Error - GATT_Service_Discovery_Start returned %d.\r\n", ret_val);

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            printf("Service Discovery Operation Outstanding for Device.\r\n");

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         printf("No Device Info.\r\n");

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      printf("No Connection Established\r\n");

      ret_val = FUNCTION_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for performing a GAP Service*/
   /* Service Discovery Operation.  This function will return zero on   */
   /* successful execution and a negative value on errors.              */
static int DiscoverGAPS(ParameterList_t *TempParam)
{
   int           ret_val;
   GATT_UUID_t   UUID;
   DeviceInfo_t *DeviceInfo;

   /* Verify that there is a connection that is established.            */
   if(ConnectionID)
   {
      /* Get the device info for the connection device.                 */
      if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
      {
         /* Verify that no service discovery is outstanding for this    */
         /* device.                                                     */
         if(!(DeviceInfo->Flags & DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_OUTSTANDING))
         {
            /* Configure the filter so that only the SPP LE Service is  */
            /* discovered.                                              */
            UUID.UUID_Type = guUUID_16;
            GAP_ASSIGN_GAP_SERVICE_UUID_16(UUID.UUID.UUID_16);

            ret_val = GATT_Start_Service_Discovery(BluetoothStackID, ConnectionID, 1, &UUID, GATT_Service_Discovery_Event_Callback, 0);
            if(!ret_val)
            {
               /* Display success message.                              */
               printf("GATT_Start_Service_Discovery_Handle_Range() success.\r\n");

               /* Flag that a Service Discovery Operation is            */
               /* outstanding.                                          */
               DeviceInfo->Flags |= (DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_OUTSTANDING | DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_GAPS);
            }
            else
            {
               /* An error occur so just clean-up.                      */
               printf("Error - GATT_Service_Discovery_Start returned %d.\r\n", ret_val);

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            printf("Service Discovery Operation Outstanding for Device.\r\n");

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         printf("No Device Info.\r\n");

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      printf("No Connection Established\r\n");

      ret_val = FUNCTION_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for reading the current     */
   /* Local Device Name.  This function will return zero on successful  */
   /* execution and a negative value on errors.                         */
static int ReadLocalName(ParameterList_t *TempParam)
{
   int  ret_val;
   char NameBuffer[BTPS_CONFIGURATION_GAPS_MAXIMUM_SUPPORTED_DEVICE_NAME+1];

   /* Verify that the GAP Service is registered.                        */
   if(GAPSInstanceID)
   {
      /* Initialize the Name Buffer to all zeros.                       */
      BTPS_MemInitialize(NameBuffer, 0, sizeof(NameBuffer));

      /* Query the Local Name.                                          */
      ret_val = GAPS_Query_Device_Name(BluetoothStackID, GAPSInstanceID, NameBuffer);
      if(!ret_val)
         printf("Device Name: %s.\r\n", NameBuffer);
      else
      {
         printf("Error - GAPS_Query_Device_Name returned %d.\r\n", ret_val);

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      printf("GAP Service not registered.\r\n");

      ret_val = FUNCTION_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for setting the current     */
   /* Local Device Name.  This function will return zero on successful  */
   /* execution and a negative value on errors.                         */
static int SetLocalName(ParameterList_t *TempParam)
{
   int  ret_val;

   /* Verify that the input parameters are semi-valid.                  */
   if((TempParam) && (TempParam->NumberofParameters > 0) && (BTPS_StringLength(TempParam->Params[0].strParam) > 0) && (BTPS_StringLength(TempParam->Params[0].strParam) <= BTPS_CONFIGURATION_GAPS_MAXIMUM_SUPPORTED_DEVICE_NAME))
   {
      /* Verify that the GAP Service is registered.                     */
      if(GAPSInstanceID)
      {
         /* Query the Local Name.                                       */
         ret_val = GAPS_Set_Device_Name(BluetoothStackID, GAPSInstanceID, TempParam->Params[0].strParam);
         if(!ret_val)
            printf("GAPS_Set_Device_Name success.\r\n");
         else
         {
            printf("Error - GAPS_Query_Device_Name returned %d.\r\n", ret_val);

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         printf("GAP Service not registered.\r\n");

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      printf("Usage: SetLocalName [NameString].\r\n");

      ret_val = FUNCTION_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for reading the Device Name */
   /* for the currently connected remote device.  This function will    */
   /* return zero on successful execution and a negative value on       */
   /* errors.                                                           */
static int ReadRemoteName(ParameterList_t *TempParam)
{
   int           ret_val;
   DeviceInfo_t *DeviceInfo;

   /* Verify that there is a connection that is established.            */
   if(ConnectionID)
   {
      /* Get the device info for the connection device.                 */
      if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
      {
         /* Verify that we discovered the Device Name Handle.           */
         if(DeviceInfo->GAPSClientInfo.DeviceNameHandle)
         {
            /* Attempt to read the remote device name.                  */
            ret_val = GATT_Read_Value_Request(BluetoothStackID, ConnectionID, DeviceInfo->GAPSClientInfo.DeviceNameHandle, GATT_ClientEventCallback_GAPS, (unsigned long)DeviceInfo->GAPSClientInfo.DeviceNameHandle);
            if(ret_val > 0)
            {
               printf("Attempting to read Remote Device Name.\r\n");

               ret_val = 0;
            }
            else
            {
               printf("Error - GATT_Read_Value_Request returned %d.\r\n", ret_val);

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            printf("GAP Service Device Name Handle not discovered.\r\n");

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         printf("No Device Info.\r\n");

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      printf("No Connection Established\r\n");

      ret_val = FUNCTION_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for reading the Local Device*/
   /* Appearance value.  This function will return zero on successful   */
   /* execution and a negative value on errors.                         */
static int ReadLocalAppearance(ParameterList_t *TempParam)
{
   int     ret_val;
   char   *AppearanceString;
   Word_t  Appearance;

   /* Verify that the GAP Service is registered.                        */
   if(GAPSInstanceID)
   {
      /* Query the Local Name.                                          */
      ret_val = GAPS_Query_Device_Appearance(BluetoothStackID, GAPSInstanceID, &Appearance);
      if(!ret_val)
      {
         /* Map the Appearance to a String.                             */
         if(AppearanceToString(Appearance, &AppearanceString))
            printf("Device Appearance: %s(%u).\r\n", AppearanceString, Appearance);
         else
            printf("Device Appearance: Unknown(%u).\r\n", Appearance);
      }
      else
      {
         printf("Error - GAPS_Query_Device_Appearance returned %d.\r\n", ret_val);

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      printf("GAP Service not registered.\r\n");

      ret_val = FUNCTION_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for setting the Local Device*/
   /* Appearance value.  This function will return zero on successful   */
   /* execution and a negative value on errors.                         */
static int SetLocalAppearance(ParameterList_t *TempParam)
{
   int    ret_val;
   Word_t Appearance;

   /* Verify that the input parameters are semi-valid.                  */
   if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].intParam >= 0) && (TempParam->Params[0].intParam < NUMBER_OF_APPEARANCE_MAPPINGS))
   {
      /* Verify that the GAP Service is registered.                     */
      if(GAPSInstanceID)
      {
         /* Map the Appearance Index to the GAP Appearance Value.       */
         if(AppearanceIndexToAppearance(TempParam->Params[0].intParam, &Appearance))
         {
            /* Set the Local Appearance.                                */
            ret_val = GAPS_Set_Device_Appearance(BluetoothStackID, GAPSInstanceID, Appearance);
            if(!ret_val)
               printf("GAPS_Set_Device_Appearance success.\r\n");
            else
            {
               printf("Error - GAPS_Set_Device_Appearance returned %d.\r\n", ret_val);

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            printf("Invalid Appearance Index.\r\n");

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         printf("GAP Service not registered.\r\n");

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      printf("Usage: SetLocalName [Index].\r\n");
      printf("Where Index = \r\n");
      DumpAppearanceMappings();

      ret_val = FUNCTION_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for reading the Device Name */
   /* for the currently connected remote device.  This function will    */
   /* return zero on successful execution and a negative value on       */
   /* errors.                                                           */
static int ReadRemoteAppearance(ParameterList_t *TempParam)
{
   int           ret_val;
   DeviceInfo_t *DeviceInfo;

   /* Verify that there is a connection that is established.            */
   if(ConnectionID)
   {
      /* Get the device info for the connection device.                 */
      if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
      {
         /* Verify that we discovered the Device Name Handle.           */
         if(DeviceInfo->GAPSClientInfo.DeviceAppearanceHandle)
         {
            /* Attempt to read the remote device name.                  */
            ret_val = GATT_Read_Value_Request(BluetoothStackID, ConnectionID, DeviceInfo->GAPSClientInfo.DeviceAppearanceHandle, GATT_ClientEventCallback_GAPS, (unsigned long)DeviceInfo->GAPSClientInfo.DeviceAppearanceHandle);
            if(ret_val > 0)
            {
               printf("Attempting to read Remote Device Appearance.\r\n");

               ret_val = 0;
            }
            else
            {
               printf("Error - GATT_Read_Value_Request returned %d.\r\n", ret_val);

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            printf("GAP Service Device Appearance Handle not discovered.\r\n");

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         printf("No Device Info.\r\n");

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      printf("No Connection Established\r\n");

      ret_val = FUNCTION_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for performing a SPPLE      */
   /* Service Discovery Operation.  This function will return zero on   */
   /* successful execution and a negative value on errors.              */
static int DiscoverSPPLE(ParameterList_t *TempParam)
{
   int           ret_val;
   GATT_UUID_t   UUID;
   DeviceInfo_t *DeviceInfo;

   /* Verify that there is a connection that is established.            */
   if(ConnectionID)
   {
      /* Get the device info for the connection device.                 */
      if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
      {
         /* Verify that no service discovery is outstanding for this    */
         /* device.                                                     */
         if(!(DeviceInfo->Flags & DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_OUTSTANDING))
         {
            /* Configure the filter so that only the SPP LE Service is  */
            /* discovered.                                              */
            UUID.UUID_Type = guUUID_128;
            SPPLE_ASSIGN_SPPLE_SERVICE_UUID_128(UUID.UUID.UUID_128);

            ret_val = GATT_Start_Service_Discovery(BluetoothStackID, ConnectionID, 1, &UUID, GATT_Service_Discovery_Event_Callback, 0);
            if(!ret_val)
            {
               /* Display success message.                              */
               printf("GATT_Start_Service_Discovery_Handle_Range() success.\r\n");

               /* Flag that a Service Discovery Operation is            */
               /* outstanding.                                          */
               DeviceInfo->Flags |= (DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_OUTSTANDING | DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_SPPLE);
            }
            else
            {
               /* An error occur so just clean-up.                      */
               printf("Error - GATT_Service_Discovery_Start returned %d.\r\n", ret_val);

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            printf("Service Discovery Operation Outstanding for Device.\r\n");

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         printf("No Device Info.\r\n");

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      printf("No Connection Established\r\n");

      ret_val = FUNCTION_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for registering a SPPLE     */
   /* Service.  This function will return zero on successful execution  */
   /* and a negative value on errors.                                   */
static int RegisterSPPLE(ParameterList_t *TempParam)
{
   int                           ret_val;
   GATT_Attribute_Handle_Group_t ServiceHandleGroup;

   /* Verify that there is no active connection.                        */
   if(!ConnectionID)
   {
      /* Verify that the Service is not already registered.             */
      if(!SPPLEServiceID)
      {
         /* Initialize the Service Handle Group to 0 since we do not    */
         /* require a specific location in the service table.           */
         ServiceHandleGroup.Starting_Handle = 0;
         ServiceHandleGroup.Ending_Handle   = 0;

         /* Register the SPPLE Service.                                 */
         ret_val = GATT_Register_Service(BluetoothStackID, SPPLE_SERVICE_FLAGS, SPPLE_SERVICE_ATTRIBUTE_COUNT, (GATT_Service_Attribute_Entry_t *)SPPLE_Service, &ServiceHandleGroup, GATT_ServerEventCallback, 0);
         if(ret_val > 0)
         {
            /* Display success message.                                 */
            printf("Successfully registered SPPLE Service.\r\n");

            /* Save the ServiceID of the registered service.            */
            SPPLEServiceID = (unsigned int)ret_val;

            /* Return success to the caller.                            */
            ret_val        = 0;
         }
      }
      else
      {
         printf("SPPLE Service already registered.\r\n");

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      printf("Connection currently active.\r\n");

      ret_val = FUNCTION_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for unregistering a SPPLE   */
   /* Service.  This function will return zero on successful execution  */
   /* and a negative value on errors.                                   */
static int UnregisterSPPLE(ParameterList_t *TempParam)
{
   int ret_val;

   /* Verify that there is no active connection.                        */
   if(!ConnectionID)
   {
      /* Verify that the Service is not already registered.             */
      if(SPPLEServiceID)
      {
         /* Un-registered SPP LE Service.                               */
         GATT_Un_Register_Service(BluetoothStackID, SPPLEServiceID);

         /* Display success message.                                    */
         printf("Successfully unregistered SPPLE Service.\r\n");

         /* Save the ServiceID of the registered service.               */
         SPPLEServiceID = 0;

         /* Return success to the caller.                               */
         ret_val        = 0;
      }
      else
      {
         printf("SPPLE Service not registered.\r\n");

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      printf("Connection currently active.\r\n");

      ret_val = FUNCTION_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for configure a SPPLE       */
   /* Service on a remote device.  This function will return zero on    */
   /* successful execution and a negative value on errors.              */
static int ConfigureSPPLE(ParameterList_t *TempParam)
{
   int           ret_val;
   DeviceInfo_t *DeviceInfo;

   /* Verify that there is a connection that is established.            */
   if(ConnectionID)
   {
      /* Get the device info for the connection device.                 */
      if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
      {
         /* Verify that we are not already acting as a Server.          */
         if(!(DeviceInfo->Flags & DEVICE_INFO_FLAGS_SPPLE_SERVER))
         {
            /* Determine if a service discovery operation has been      */
            /* previously done.                                         */
            if(SPPLE_CLIENT_INFORMATION_VALID(DeviceInfo->ClientInfo))
            {
               printf("SPPLE Service found on remote device, attempting to read Transmit Credits, and configured CCCDs.\r\n");
   
               /* Send the Initial Credits to the remote device.        */
               SendCredits(DeviceInfo, DeviceInfo->ReceiveBuffer.BytesFree);
   
               /* Enable Notifications on the proper characteristics.   */
               EnableDisableNotificationsIndications(DeviceInfo->ClientInfo.Rx_Credit_Client_Configuration_Descriptor, GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_NOTIFY_ENABLE, GATT_ClientEventCallback_SPPLE);
               EnableDisableNotificationsIndications(DeviceInfo->ClientInfo.Tx_Client_Configuration_Descriptor, GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_NOTIFY_ENABLE, GATT_ClientEventCallback_SPPLE);
   
               /* Flag that we are an SPPLE Client.                     */
               DeviceInfo->Flags |= DEVICE_INFO_FLAGS_SPPLE_CLIENT;
               ret_val            = 0;
            }
            else
            {
               printf("No SPPLE Service discovered on device.\r\n");
   
               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            printf("Already operating as a Server\n");
   
            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         printf("No Device Info.\r\n");

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      printf("No Connection Established\r\n");

      ret_val = FUNCTION_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for sending a number of     */
   /* characters to a remote device to which a connection exists.  The  */
   /* function receives a parameter that indicates the number of byte to*/
   /* be transferred.  This function will return zero on successful     */
   /* execution and a negative value on errors.                         */
static int SendDataCommand(ParameterList_t *TempParam)
{
   DeviceInfo_t *DeviceInfo;

   /* Make sure that all of the parameters required for this function   */
   /* appear to be at least semi-valid.                                 */
   if((TempParam) && (TempParam->NumberofParameters >= 1) && (TempParam->Params[0].intParam > 0))
   {
      /* Verify that there is a connection that is established.         */
      if(ConnectionID)
      {
         /* Check to see if we are sending to another port.             */
         if(!SendInfo.BytesToSend)
         {
            /* Get the device info for the connection device.           */
            if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
            {
               /* Verify that we are a Client or Server.                */
               if(DeviceInfo->Flags & (DEVICE_INFO_FLAGS_SPPLE_CLIENT | DEVICE_INFO_FLAGS_SPPLE_SERVER))
               {
                  /* Get the count of the number of bytes to send.      */
                  SendInfo.BytesToSend  = (DWord_t)TempParam->Params[0].intParam;
                  SendInfo.BytesSent    = 0;
   
                  /* Kick start the send process.                          */
                  SendProcess(DeviceInfo);
               }
               else
                  printf("SPPLE has not been configured\n");
            }
            else
               printf("No Device Info.\r\n");
         }
         else
            printf("Send Currently in progress.\r\n");
      }
      else
         printf("No Connection Established\r\n");
   }
   else
      DisplayUsage("SEND [Number of Bytes to send]\r\n");

   return(0);
}

   /* The following function is responsible for reading data sent by a  */
   /* remote device to which a connection exists.  This function will   */
   /* return zero on successful execution and a negative value on       */
   /* errors.                                                           */
static int ReadDataCommand(ParameterList_t *TempParam)
{
   Byte_t       *Ptr;
   Boolean_t     Done;
   unsigned int  Temp;
   DeviceInfo_t *DeviceInfo;

   /* Verify that there is a connection that is established.            */
   if(ConnectionID)
   {
      /* Get the device info for the connection device.                 */
      if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
      {
         /* Verify that we are a Client or Server.                      */
         if(DeviceInfo->Flags & (DEVICE_INFO_FLAGS_SPPLE_CLIENT | DEVICE_INFO_FLAGS_SPPLE_SERVER))
         {
            /* Determine the number of bytes we are going to read.      */
            Temp = DeviceInfo->ReceiveBuffer.BufferSize - DeviceInfo->ReceiveBuffer.BytesFree;
   
            printf("Read: %u.\r\n", Temp);
   
            /* Loop and read all of the data.                           */
            Done = FALSE;
            while(!Done)
            {
               /* Read the data.                                        */
               Temp = ReadData(DeviceInfo, SPPLE_DATA_BUFFER_LENGTH, SPPLEBuffer);
               if(Temp > 0)
               {
                  /* Display the data.                                  */
                  Ptr = SPPLEBuffer;
   
                  while(Temp > 0)
                  {
                     printf("%02X ", *Ptr);
                     Ptr++;
                     Temp--;
                  }
               }
               else
                  Done = TRUE;
            }
         }
         else
            printf("SPPLE has not been configured\n");
      }
      else
         printf("No Device Info.\r\n");
   }
   else
      printf("No Connection Established\r\n");

   printf("\r\n");

   return(0);
}

   /* The following function is responsible for querying the GATT MTU.  */
   /* This function returns zero if successful and a negative value if  */
   /* an error occurred.                                                */
static int GetGATTMTU(ParameterList_t *TempParam)
{
   int    ret_val;
   Word_t MTU;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Simply query the Maximum Supported MTU from the GATT layer.    */
      if((ret_val = GATT_Query_Maximum_Supported_MTU(BluetoothStackID, &MTU)) == 0)
         printf("Maximum GATT MTU: %u.\r\n", (unsigned int)MTU);
      else
      {
         printf("Error - GATT_Query_Maximum_Supported_MTU() %d.\r\n", ret_val);

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

   /* The following function is responsible for setting the GATT MTU.   */
   /* This function returns zero if successful and a negative value if  */
   /* an error occurred.                                                */
static int SetGATTMTU(ParameterList_t *TempParam)
{
   int ret_val;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Bluetooth Stack is initialized, go ahead and check to see if   */
      /* the parameters are valid.                                      */
      if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].intParam >= ATT_PROTOCOL_MTU_MINIMUM_LE) && (TempParam->Params[0].intParam <= ATT_PROTOCOL_MTU_MAXIMUM))
      {
         /* Simply set the Maximum Supported MTU to the GATT layer.     */
         if((ret_val = GATT_Change_Maximum_Supported_MTU(BluetoothStackID, (Word_t)TempParam->Params[0].intParam)) == 0)
            printf("GATT_Change_Maximum_Supported_MTU() success, new GATT MTU: %u.\r\n", (unsigned int)TempParam->Params[0].intParam);
         else
         {
            printf("Error - GATT_Change_Maximum_Supported_MTU() %d.\r\n", ret_val);

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* Invalid parameters specified so flag an error to the user.  */
         printf("Usage: SetMTU [GATT MTU (>= %u, <= %u)].\r\n", (unsigned int)ATT_PROTOCOL_MTU_MINIMUM_LE, (unsigned int)ATT_PROTOCOL_MTU_MAXIMUM);

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

// xxx
static int SetDataLength(ParameterList_t *TempParam)
{
   int ret_val;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Bluetooth Stack is initialized, go ahead and check to see if   */
      /* the parameters are valid.                                      */
      if((TempParam) && (TempParam->NumberofParameters > 1))
      {
         /* Set the data length.                                        */
         ret_val = GAP_LE_Set_Data_Length(BluetoothStackID, ConnectionBD_ADDR, TempParam->Params[0].intParam, TempParam->Params[1].intParam);
         
         if(ret_val == 0)
         {
            printf("GAP_LE_Set_Data_Length() success: %d\r\n", ret_val);
         }
         else
         {
            printf(" Error - GAP_LE_Set_Data_Length() %d.\r\n", ret_val);
            
            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* Invalid parameters specified so flag an error to the user.  */
         printf("Usage: SetDataLength [TxOctets] [TxTime].\r\n");

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

// xxx
static int ReadMaxDataLength(ParameterList_t *TempParam)
{
   int    ret_val;
   Byte_t StatusResult;
   Word_t MaxTxOctets;
   Word_t MaxTxTime;
   Word_t MaxRxOctets;
   Word_t MaxRxTime;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Read the maximum data length.                                  */
      ret_val = HCI_LE_Read_Maximum_Data_Length(BluetoothStackID, &StatusResult, &MaxTxOctets, &MaxTxTime, &MaxRxOctets, &MaxRxTime);
      
      if(ret_val == 0)
      {
         if(StatusResult == 0)
         {
            printf("HCI_LE_Read_Maximum_Data_Length Command Success: 0x%02x\r\n", StatusResult);
            printf("   MaxTxOctets:  %d\r\n", MaxTxOctets);
            printf("   MaxTxTime:    %d\r\n", MaxTxTime);
            printf("   MaxRxOctets:  %d\r\n", MaxRxOctets);
            printf("   MaxRxTime:    %d\r\n", MaxRxTime);
         }
         else
         {
            printf("HCI_LE_Read_Maximum_Data_Length Command Failure: 0x%02x\r\n", StatusResult);
         }
      }
      else
      {
         printf(" Error - HCI_LE_Read_Maximum_Data_Length() %d.\r\n", ret_val);
         
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

// xxx
static int QueryDefaultDataLength(ParameterList_t *TempParam)
{
   int    ret_val;
   Word_t MaxTxOctets;
   Word_t MaxTxTime;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Read the suggested default data length.                        */
      ret_val = GAP_LE_Query_Default_Data_Length(BluetoothStackID, &MaxTxOctets, &MaxTxTime);
      
      if(ret_val == 0)
      {
         printf("GAP_LE_Query_Default_Data_Length() success: %d\r\n", ret_val);
         printf("   SuggestedMaxTxOctets: %d\r\n", MaxTxOctets);
         printf("   SuggestedMaxTxTime: %d\r\n", MaxTxTime);
      }
      else
      {
         printf(" Error - GAP_LE_Query_Default_Data_Length() %d.\r\n", ret_val);
         
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

// xxx
static int SetDefaultDataLength(ParameterList_t *TempParam)
{
   int ret_val;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Bluetooth Stack is initialized, go ahead and check to see if   */
      /* the parameters are valid.                                      */
      if((TempParam) && (TempParam->NumberofParameters > 1))
      {
         /* Write the suggested default data length.                    */
         ret_val = GAP_LE_Set_Default_Data_Length(BluetoothStackID, TempParam->Params[0].intParam, TempParam->Params[1].intParam);
         
         if(ret_val == 0)
         {
            printf("GAP_LE_Set_Default_Data_Length() success: %d\r\n", ret_val);
         }
         else
         {
            printf(" Error - GAP_LE_Set_Default_Data_Length() %d.\r\n", ret_val);
            
            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* Invalid parameters specified so flag an error to the user.  */
         printf("Usage: SetDefaultDataLength [TxOctets] [TxTime].\r\n");

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

// xxx
static int UpdateP256Key(ParameterList_t *TempParam)
{
   int ret_val;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Update the local P256 public key.                              */
      ret_val = GAP_LE_Update_Local_P256_Public_Key(BluetoothStackID);
      
      if(ret_val == 0)
      {
         printf("GAP_LE_Update_Local_P256_Public_Key() success: %d\r\n", ret_val);
      }
      else
      {
         printf(" Error - GAP_LE_Update_Local_P256_Public_Key() %d.\r\n", ret_val);
         
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

// xxx static int GenerateDHKey(ParameterList_t *TempParam);

// xxx
static int AddDeviceResolvingList(ParameterList_t *TempParam)
{
   int                            ret_val;
   unsigned int                   AddedDeviceCount;
   DeviceInfo_t                  *DeviceInfo;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
      {
         if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_PEER_IDENTITY_INFO_VALID)
         {
            if(!(DeviceInfo->Flags & DEVICE_INFO_FLAGS_DEVICE_IN_RESOLVING_LIST))
            {
               ResolvingListEntry.Peer_Identity_Address_Type = DeviceInfo->PeerAddressType;
               ResolvingListEntry.Peer_Identity_Address      = DeviceInfo->PeerAddress;
               ResolvingListEntry.Peer_IRK                   = DeviceInfo->PeerIRK;
               ResolvingListEntry.Local_IRK                  = IRK;
   
               /* Add the device to the resolving list.                 */
               ret_val = GAP_LE_Add_Device_To_Resolving_List(BluetoothStackID, 1, &ResolvingListEntry, &AddedDeviceCount);
               
               if(ret_val == 0)
               {
                  printf("GAP_LE_Add_Device_To_Resolving_List() success: %d\r\n", ret_val);
                  printf("   Devices added: %d.\r\n", AddedDeviceCount);
                  
                  DeviceInfo->Flags |= DEVICE_INFO_FLAGS_DEVICE_IN_RESOLVING_LIST;
               }
               else
               {
                  printf(" Error - GAP_LE_Add_Device_To_Resolving_List() %d.\r\n", ret_val);
                  ret_val = FUNCTION_ERROR;
               }
            }
            else
            {
               printf("Device already in resolving list.\r\n");
               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            printf("No peer identity information stored.\r\n");
            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         printf("No device information found for address.\r\n");
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

// xxx
static int RemoveDeviceResolvingList(ParameterList_t *TempParam)
{
   int                            ret_val;
   unsigned int                   RemovedDeviceCount;
   DeviceInfo_t                  *DeviceInfo;
   GAP_LE_Resolving_List_Entry_t  ListEntry;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
      {
         if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_DEVICE_IN_RESOLVING_LIST)
         {
            ListEntry.Peer_Identity_Address_Type = DeviceInfo->PeerAddressType;
            ListEntry.Peer_Identity_Address      = DeviceInfo->PeerAddress;
            ListEntry.Peer_IRK                   = DeviceInfo->PeerIRK;
            ListEntry.Local_IRK                  = IRK;
         
            /* Remove the device from the resolving list.               */
            ret_val = GAP_LE_Remove_Device_From_Resolving_List(BluetoothStackID, 1, &ListEntry, &RemovedDeviceCount);
            
            if(ret_val == 0)
            {
               /* Clear the last resolving list entry.                  */
               ResolvingListEntry.Peer_Identity_Address_Type = latPublic;
               ASSIGN_BD_ADDR(ResolvingListEntry.Peer_Identity_Address, 0, 0, 0, 0, 0, 0);

               printf("GAP_LE_Remove_Device_From_Resolving_List() success: %d\r\n", ret_val);
               printf("   Devices removed: %d", RemovedDeviceCount);
               
               DeviceInfo->Flags &= ~DEVICE_INFO_FLAGS_DEVICE_IN_RESOLVING_LIST;
            }
            else
            {
               printf(" Error - GAP_LE_Remove_Device_From_Resolving_List() %d.\r\n", ret_val);
               
               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            printf("Device not in resolving list.\r\n");
            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         printf("No device information found for address.\r\n");
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

// xxx
static int ReadResolvingListSize(ParameterList_t *TempParam)
{
   int          ret_val;
   unsigned int ResolvingListSize;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Read the resolving list size.                                  */
      ret_val = GAP_LE_Read_Resolving_List_Size(BluetoothStackID, &ResolvingListSize);
      
      if(ret_val == 0)
      {
         printf("GAP_LE_Read_Resolving_List_Size() success: %d\r\n", ret_val);
         printf("   Resolving list size: %d", ResolvingListSize);
      }
      else
      {
         printf(" Error - GAP_LE_Read_Resolving_List_Size() %d.\r\n", ret_val);
         
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

// xxx
static int ReadPeerResolvableAddress(ParameterList_t *TempParam)
{
   int           ret_val;
   Byte_t        StatusResult;
   BD_ADDR_t     AddressResult;
   DeviceInfo_t *DeviceInfo;
   BoardStr_t    BoardStr;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
      {
         if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_PEER_IDENTITY_INFO_VALID)
         {
            if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_DEVICE_IN_RESOLVING_LIST)
            {
               /* Read the peer address.                                         */
               ret_val = HCI_LE_Read_Peer_Resolvable_Address(BluetoothStackID, DeviceInfo->PeerAddressType, DeviceInfo->PeerAddress, &StatusResult, &AddressResult);
               
               if(ret_val == 0)
               {
                  if(StatusResult == 0)
                  {
                     printf("HCI_LE_Read_Peer_Resolvable_Address Command Success: 0x%02x\r\n", StatusResult);
                     BD_ADDRToStr(AddressResult, BoardStr);
                     printf("   Peer Resolvable Address: %s.\r\n", BoardStr);
                  }
                  else
                  {
                     printf("HCI_LE_Read_Peer_Resolvable_Address Command Failure: 0x%02x\r\n", StatusResult);
                  }
               }
               else
               {
                  printf(" Error - HCI_LE_Read_Peer_Resolvable_Address() %d.\r\n", ret_val);
                  
                  ret_val = FUNCTION_ERROR;
               }
            }
            else
            {
               printf("Device not in resolving list.\r\n");
               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            printf("No peer identity information stored.\r\n");
            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         printf("No device information found for address.\r\n");
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

// xxx
static int ReadLocalResolvableAddress(ParameterList_t *TempParam)
{
   int           ret_val;
   Byte_t        StatusResult;
   BD_ADDR_t     AddressResult;
   DeviceInfo_t *DeviceInfo;
   BoardStr_t    BoardStr;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
      {
         if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_PEER_IDENTITY_INFO_VALID)
         {
            if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_DEVICE_IN_RESOLVING_LIST)
            {
               /* Read the peer address.                                         */
               ret_val = HCI_LE_Read_Local_Resolvable_Address(BluetoothStackID, DeviceInfo->PeerAddressType, DeviceInfo->PeerAddress, &StatusResult, &AddressResult);
               
               if(ret_val == 0)
               {
                  if(StatusResult == 0)
                  {
                     printf("HCI_LE_Read_Local_Resolvable_Address Command Success: 0x%02x\r\n", StatusResult);
                     BD_ADDRToStr(AddressResult, BoardStr);
                     printf("   Local Resolvable Address: %s.\r\n", BoardStr);
                  }
                  else
                  {
                     printf("HCI_LE_Read_Local_Resolvable_Address Command Failure: 0x%02x\r\n", StatusResult);
                  }
               }
               else
               {
                  printf(" Error - HCI_LE_Read_Local_Resolvable_Address() %d.\r\n", ret_val);
                  
                  ret_val = FUNCTION_ERROR;
               }
            }
            else
            {
               printf("Device not in resolving list.\r\n");
               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            printf("No peer identity information stored.\r\n");
            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         printf("No device information found for address.\r\n");
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

// xxx
static int SetAddressResolutionEnable(ParameterList_t *TempParam)
{
   int ret_val;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Bluetooth Stack is initialized, go ahead and check to see if   */
      /* the parameters are valid.                                      */
      if((TempParam) && (TempParam->NumberofParameters > 0))
      {
         /* Enable/disable address resolution.                          */
         ret_val = GAP_LE_Set_Address_Resolution_Enable(BluetoothStackID, TempParam->Params[0].intParam);
         
         if(ret_val == 0)
         {
            printf("GAP_LE_Set_Address_Resolution_Enable() success: %d\r\n", ret_val);
         }
         else
         {
            printf(" Error - GAP_LE_Set_Address_Resolution_Enable() %d.\r\n", ret_val);
            
            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* Invalid parameters specified so flag an error to the user.  */
         printf("Usage: SetAddressResolutionEnable [(0 = Disable, 1 = Enable)].\r\n");

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

// xxx
static int SetRPATimeout(ParameterList_t *TempParam)
{
   int ret_val;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Bluetooth Stack is initialized, go ahead and check to see if   */
      /* the parameters are valid.                                      */
      if((TempParam) && (TempParam->NumberofParameters > 0))
      {
         /* Set the resolvable private address timeout.                 */
         ret_val = GAP_LE_Set_Resolvable_Private_Address_Timeout(BluetoothStackID, TempParam->Params[0].intParam);
         
         if(ret_val == 0)
         {
            printf("GAP_LE_Set_Resolvable_Private_Address_Timeout() success: %d\r\n", ret_val);
         }
         else
         {
            printf(" Error - GAP_LE_Set_Resolvable_Private_Address_Timeout() %d.\r\n", ret_val);
            
            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* Invalid parameters specified so flag an error to the user.  */
         printf("Usage: SetRPATimeout [Timeout (seconds)].\r\n");

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

// xxx
static int AddDeviceWhiteList(ParameterList_t *TempParam)
{
   int                        ret_val;
   unsigned int               AddedDeviceCount;
   DeviceInfo_t              *DeviceInfo;
   GAP_LE_White_List_Entry_t  WhiteListEntry;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
      {
         if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_PEER_IDENTITY_INFO_VALID)
         {
            if(!(DeviceInfo->Flags & DEVICE_INFO_FLAGS_DEVICE_IN_WHITE_LIST))
            {
               WhiteListEntry.Address_Type = DeviceInfo->PeerAddressType;
               WhiteListEntry.Address      = DeviceInfo->PeerAddress;
   
               /* Add the device to the white list.                 */
               ret_val = GAP_LE_Add_Device_To_White_List(BluetoothStackID, 1, &WhiteListEntry, &AddedDeviceCount);
               
               if(ret_val == 0)
               {
                  printf("GAP_LE_Add_Device_To_White_List() success: %d\r\n", ret_val);
                  printf("   Devices added: %d\r\n", AddedDeviceCount);
                  
                  DeviceInfo->Flags |= DEVICE_INFO_FLAGS_DEVICE_IN_WHITE_LIST;
               }
               else
               {
                  printf(" Error - GAP_LE_Add_Device_To_White_List() %d.\r\n", ret_val);
                  ret_val = FUNCTION_ERROR;
               }
            }
            else
            {
               printf("Device already in white list.\r\n");
               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            printf("No peer identity information stored.\r\n");
            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         printf("No device information found for address.\r\n");
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

// xxx
static int RemoveDeviceWhiteList(ParameterList_t *TempParam)
{
   int                        ret_val;
   unsigned int               RemovedDeviceCount;
   DeviceInfo_t              *DeviceInfo;
   GAP_LE_White_List_Entry_t  WhiteListEntry;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
      {
         if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_PEER_IDENTITY_INFO_VALID)
         {
            if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_DEVICE_IN_WHITE_LIST)
            {
               WhiteListEntry.Address_Type = DeviceInfo->PeerAddressType;
               WhiteListEntry.Address      = DeviceInfo->PeerAddress;
            
               /* Remove the device from the White list.                     */
               ret_val = GAP_LE_Remove_Device_From_White_List(BluetoothStackID, 1, &WhiteListEntry, &RemovedDeviceCount);
               
               if(ret_val == 0)
               {
                  printf("GAP_LE_Remove_Device_From_White_List() success: %d\r\n", ret_val);
                  printf("   Devices removed: %d", RemovedDeviceCount);
                  
                  DeviceInfo->Flags &= ~DEVICE_INFO_FLAGS_DEVICE_IN_WHITE_LIST;
               }
               else
               {
                  printf(" Error - GAP_LE_Remove_Device_From_White_List() %d.\r\n", ret_val);
                  
                  ret_val = FUNCTION_ERROR;
               }
            }
            else
            {
               printf("Device not in white list.\r\n");
               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            printf("No peer identity information stored.\r\n");
            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         printf("No device information found for address.\r\n");
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

// xxx
static int ReadWhiteListSize(ParameterList_t *TempParam)
{
   int          ret_val;
   unsigned int WhiteListSize;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Read the white list size.                                      */
      ret_val = GAP_LE_Read_White_List_Size(BluetoothStackID, &WhiteListSize);
      
      if(ret_val == 0)
      {
         printf("GAP_LE_Read_White_List_Size() success: %d\r\n", ret_val);
         printf("   White list size: %d", WhiteListSize);
      }
      else
      {
         printf(" Error - GAP_LE_Read_White_List_Size() %d.\r\n", ret_val);
         
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

   /* ***************************************************************** */
   /*                         Event Callbacks                           */
   /* ***************************************************************** */
   
   /* The following function is for the HCI Event Callback.  This       */
   /* function will be called whenever a Callback has been registered   */
   /* for the specified HCI Action that is associated with the Bluetooth*/
   /* Stack.  This function passes to the caller the HCI Event Data of  */
   /* the specified Event and the HCI Event Callback Parameter that was */
   /* specified when this Callback was installed.  The caller is free to*/
   /* use the contents of the HCI Event Data ONLY in the context of this*/
   /* callback.  If the caller requires the Data for a longer period of */
   /* time, then the callback function MUST copy the data into another  */
   /* Data Buffer.  This function is guaranteed NOT to be invoked more  */
   /* than once simultaneously for the specified installed callback     */
   /* (i.e. this function DOES NOT have be reentrant).  It Needs to be  */
   /* noted however, that if the same Callback is installed more than   */
   /* once, then the callbacks will be called serially.  Because of     */
   /* this, the processing in this function should be as efficient as   */
   /* possible.  It should also be noted that this function is called in*/
   /* the Thread Context of a Thread that the User does NOT own.        */
   /* Therefore, processing in this function should be as efficient as  */
   /* possible (this argument holds anyway because other HCI Events will*/
   /* not be processed while this function call is outstanding).        */
   /* * NOTE * This function MUST NOT Block and wait for events that    */
   /*          can only be satisfied by Receiving other HCI Events.  A  */
   /*          Deadlock WILL occur because NO HCI Event Callbacks will  */
   /*          be issued while this function is currently outstanding.  */
static void BTPSAPI HCI_Event_Callback(unsigned int BluetoothStackID, HCI_Event_Data_t *HCI_Event_Data, unsigned long CallbackParameter)
{
#if 0
   Boolean_t                 DisplayPrompt = TRUE;
   HCI_LE_Meta_Event_Data_t *LE_Event_Data;
   BoardStr_t                BoardStr;
   unsigned int              Index;
   
   if((BluetoothStackID) && (HCI_Event_Data))
   {
      switch(HCI_Event_Data->Event_Data_Type)
      {
         case etVendor_Specific_Debug_Event:
            
            if((HCI_Event_Data->Event_Data_Size >= 7) && (((Byte_t *)HCI_Event_Data->Event_Data.HCI_Unknown_Event_Data)[4] == 99))
            {
               printf("\r\n");
            
               for(Index = 6; Index < HCI_Event_Data->Event_Data_Size; Index++)
               {
                  printf("%c", ((Byte_t *)HCI_Event_Data->Event_Data.HCI_Unknown_Event_Data)[Index]);
               }
               
               printf("\r\n");
            }
            
            
            break;
         case etLE_Meta_Event:
            //printf("\r\n");
            //printf("etLE_Meta_Event.\r\n");
            
            /* Get the event data.                                      */
            LE_Event_Data = HCI_Event_Data->Event_Data.HCI_LE_Meta_Event_Data;
            
            switch(LE_Event_Data->LE_Event_Data_Type)
            {
               case meData_Length_Change_Event:
                  /* Handle the meData_Length_Change_Event.             */
                  printf("\r\nmeData_Length_Change_Event\r\n");
                  printf("   MaxTxOctets: %d\r\n", LE_Event_Data->Event_Data.HCI_LE_Data_Length_Change_Event_Data.MaxTxOctets);
                  printf("   MaxTxTime: %d\r\n", LE_Event_Data->Event_Data.HCI_LE_Data_Length_Change_Event_Data.MaxTxTime);
                  printf("   MaxRxOctets: %d\r\n", LE_Event_Data->Event_Data.HCI_LE_Data_Length_Change_Event_Data.MaxRxOctets);
                  printf("   MaxRxTime: %d\r\n", LE_Event_Data->Event_Data.HCI_LE_Data_Length_Change_Event_Data.MaxRxTime);
                  
                  break;
               case meRead_Local_P256_Public_Key_Complete_Event:
                  /* Handle the meRead_Local_P256_Public_Key_Complete_Event.             */
                  printf("\r\nmeRead_Local_P256_Public_Key_Complete_Event\r\n");
                  printf("   Status: %d\r\n", LE_Event_Data->Event_Data.HCI_LE_Read_Local_P256_Public_Key_Complete_Event_Data.Status);
                  
                  break;
               case meGenerate_DHKey_Complete_Event:
                  /* Handle the meGenerate_DHKey_Complete_Event.             */
                  printf("\r\nmeGenerate_DHKey_Complete_Event\r\n");
                  printf("   Status: %d\r\n", LE_Event_Data->Event_Data.HCI_LE_Generate_DHKey_Complete_Event_Data.Status);
                  
                  break;
               case meEnhanced_Connection_Complete_Event:
                  /* Handle the meEnhanced_Connection_Complete_Event.             */
                  printf("\r\nmeEnhanced_Connection_Complete_Event\r\n");
                  printf("   Status:            %d\r\n", LE_Event_Data->Event_Data.HCI_LE_Enhanced_Connection_Complete_Event_Data.Status);
                  BD_ADDRToStr(LE_Event_Data->Event_Data.HCI_LE_Enhanced_Connection_Complete_Event_Data.Peer_Address, BoardStr);
                  printf("   Peer Address:      %s\r\n", BoardStr);
                     
                  switch(LE_Event_Data->Event_Data.HCI_LE_Enhanced_Connection_Complete_Event_Data.Peer_Address_Type)
                  {
                     case latPublic:
                        printf("   Peer Address Type: Public.\r\n");
                        break;
                     case latRandom:
                        printf("   Peer Address Type: Random.\r\n");
                     case latPublicIdentity:
                        break;
                        printf("   Peer Address Type: Public Identity.\r\n");
                        break;
                     case latRandomIdentity:
                        printf("   Peer Address Type: Random Identity.\r\n");
                        break;
                     default:
                        printf("   Peer Address Type: Unknown.\r\n");
                  }
                  
                  BD_ADDRToStr(LE_Event_Data->Event_Data.HCI_LE_Enhanced_Connection_Complete_Event_Data.Local_Resolvable_Private_Address, BoardStr);
                  printf("   Local RPA:         %s\r\n", BoardStr);
                  BD_ADDRToStr(LE_Event_Data->Event_Data.HCI_LE_Enhanced_Connection_Complete_Event_Data.Peer_Resolvable_Private_Address, BoardStr);
                  printf("   Peer RPA:          %s\r\n", BoardStr);
                  
// xxx BD_ADDR_t Local_Resolvable_Private_Address;
// xxx BD_ADDR_t Peer_Resolvable_Private_Address;
                  
                  break;
               case meDirect_Advertising_Report_Event:
                  /* Handle the meDirect_Advertising_Report_Event.             */
                  printf("\r\nmeDirect_Advertising_Report_Event\r\n");
                  
// xxx
                  
                  break;
               default:
                  DisplayPrompt = FALSE;
                  break;
            }
            
            if(DisplayPrompt)
               printf("\r\nHCI>");
            
            break;
         default:
            break;
      }
   }

   /* Make sure the output is displayed to the user.                    */
   fflush(stdout);
   
#endif
}

   /* The following function is for the GAP LE Event Receive Data       */
   /* Callback.  This function will be called whenever a Callback has   */
   /* been registered for the specified GAP LE Action that is associated*/
   /* with the Bluetooth Stack.  This function passes to the caller the */
   /* GAP LE Event Data of the specified Event and the GAP LE Event     */
   /* Callback Parameter that was specified when this Callback was      */
   /* installed.  The caller is free to use the contents of the GAP LE  */
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
   /* (this argument holds anyway because other GAP Events will not be  */
   /* processed while this function call is outstanding).               */
   /* * NOTE * This function MUST NOT Block and wait for Events that can*/
   /*          only be satisfied by Receiving a Bluetooth Event         */
   /*          Callback.  A Deadlock WILL occur because NO Bluetooth    */
   /*          Callbacks will be issued while this function is currently*/
   /*          outstanding.                                             */
static void BTPSAPI GAP_LE_Event_Callback(unsigned int BluetoothStackID, GAP_LE_Event_Data_t *GAP_LE_Event_Data, unsigned long CallbackParameter)
{
   int                                           Result;
   Word_t                                        EDIV;
   BoardStr_t                                    BoardStr;
   unsigned int                                  Index;
   DeviceInfo_t                                 *DeviceInfo;
   Random_Number_t                               RandomNumber;
   Long_Term_Key_t                               GeneratedLTK;
   BD_ADDR_t                                     LocalBD_ADDR;
   GAP_LE_Security_Information_t                 GAP_LE_Security_Information;
   GAP_LE_Advertising_Report_Data_t             *DeviceEntryPtr;
   GAP_LE_Direct_Advertising_Report_Data_t      *DirectDeviceEntryPtr;
   GAP_LE_Authentication_Event_Data_t           *Authentication_Event_Data;
   GAP_LE_Authentication_Response_Information_t  GAP_LE_Authentication_Response_Information;
   GAP_LE_Connection_Parameters_t                ConnectionParams;

   /* Verify that all parameters to this callback are Semi-Valid.       */
   if((BluetoothStackID) && (GAP_LE_Event_Data))
   {
      switch(GAP_LE_Event_Data->Event_Data_Type)
      {
         case etLE_Direct_Advertising_Report:
            printf("\r\netLE_Direct_Advertising_Report with size %d.\r\n",(int)GAP_LE_Event_Data->Event_Data_Size);
            printf("%d Responses.\r\n",GAP_LE_Event_Data->Event_Data.GAP_LE_Direct_Advertising_Report_Event_Data->Number_Device_Entries);

            for(Index = 0; Index < GAP_LE_Event_Data->Event_Data.GAP_LE_Direct_Advertising_Report_Event_Data->Number_Device_Entries; Index++)
            {
               DirectDeviceEntryPtr = &(GAP_LE_Event_Data->Event_Data.GAP_LE_Direct_Advertising_Report_Event_Data->Direct_Advertising_Data[Index]);
               
               /* Display the Address Type.                             */
               switch(DirectDeviceEntryPtr->Address_Type)
               {
                  case latPublic:
                     printf("  Address Type:        Public.\r\n");
                     break;
                  case latRandom:
                     printf("  Address Type:        Random.\r\n");
                     break;
                  case latPublicIdentity:
                     printf("  Address Type:        Public Identity.\r\n");
                     break;
                  case latRandomIdentity:
                     printf("  Address Type:        Random Identity.\r\n");
                     break;
                  default:
                     printf("  Address Type:        Unknown.\r\n");
               }
               
               /* Display the Device Address.                           */
               printf("  Address:             0x%02X%02X%02X%02X%02X%02X.\r\n", DirectDeviceEntryPtr->BD_ADDR.BD_ADDR5, DirectDeviceEntryPtr->BD_ADDR.BD_ADDR4, DirectDeviceEntryPtr->BD_ADDR.BD_ADDR3, DirectDeviceEntryPtr->BD_ADDR.BD_ADDR2, DirectDeviceEntryPtr->BD_ADDR.BD_ADDR1, DirectDeviceEntryPtr->BD_ADDR.BD_ADDR0);
               
               /* Display the Address Type.                             */
               switch(DirectDeviceEntryPtr->Direct_Address_Type)
               {
                  case latPublic:
                     printf("  Direct Address Type: Public.\r\n");
                     break;
                  case latRandom:
                     printf("  Direct Address Type: Random.\r\n");
                     break;
                  case latPublicIdentity:
                     printf("  Direct Address Type: Public Identity.\r\n");
                     break;
                  case latRandomIdentity:
                     printf("  Direct Address Type: Random Identity.\r\n");
                     break;
                  default:
                     printf("  Direct Address Type: Unknown.\r\n");
               }
               
               /* Display the Direct Device Address.                    */
               printf("  Direct Address:      0x%02X%02X%02X%02X%02X%02X.\r\n", DirectDeviceEntryPtr->Direct_BD_ADDR.BD_ADDR5, DirectDeviceEntryPtr->Direct_BD_ADDR.BD_ADDR4, DirectDeviceEntryPtr->Direct_BD_ADDR.BD_ADDR3, DirectDeviceEntryPtr->Direct_BD_ADDR.BD_ADDR2, DirectDeviceEntryPtr->Direct_BD_ADDR.BD_ADDR1, DirectDeviceEntryPtr->Direct_BD_ADDR.BD_ADDR0);

               printf("  RSSI:                %d.\r\n", (int)DirectDeviceEntryPtr->RSSI);

               /* If this is a resolvable private address we need to try*/
               /* to resolve the address to make sure that we do not    */
               /* already have an entry in the scan list.               */
               /* * NOTE * This may happen if we have enabled address   */
               /*          translation and added the remote device to   */
               /*          the device list previously.  Due to the      */
               /*          controller, the advertising report may not be*/
               /*          translated in the resolving list by the      */
               /*          controller at the time this advertising      */
               /*          report is received so we need to make sure   */
               /*          that we don't add a duplicate entry.         */
               if(GAP_LE_TEST_RESOLVABLE_ADDRESS_BITS(DirectDeviceEntryPtr->BD_ADDR))
               {
                  /* Attempt to resolve this RPA.                       */
                  DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, DirectDeviceEntryPtr->BD_ADDR);
                  if(DeviceInfo)
                  {
                     /* Check to see if the Peer Identity Information is*/
                     /* valid.                                          */
                     if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_PEER_IDENTITY_INFO_VALID)
                     {
                        /* Update the Last Known RPA for this device.   */
                        DeviceInfo->LastKnownResolvableAddressType  = DirectDeviceEntryPtr->Address_Type;
                        DeviceInfo->LastKnownResolvableAddress      = DirectDeviceEntryPtr->BD_ADDR;

                        DeviceInfo->Flags                          |= DEVICE_INFO_FLAGS_LAST_RESOLVABLE_VALID;

                        /* Print the resolved address.                  */
                        BD_ADDRToStr(DeviceInfo->PeerAddress, BoardStr);
                        printf("  Peer Address:        %s (%s)\r\n", BoardStr, ((DeviceInfo->PeerAddressType == latPublic) ? "Public" : "Static"));
                     }

                  }
               }
               else
                  DeviceInfo = NULL;
            }
            
            break;
         case etLE_Advertising_Report:
            printf("\r\netLE_Advertising_Report with size %d.\r\n",(int)GAP_LE_Event_Data->Event_Data_Size);
            printf("%d Responses.\r\n",GAP_LE_Event_Data->Event_Data.GAP_LE_Advertising_Report_Event_Data->Number_Device_Entries);

            for(Index = 0; Index < GAP_LE_Event_Data->Event_Data.GAP_LE_Advertising_Report_Event_Data->Number_Device_Entries; Index++)
            {
               DeviceEntryPtr = &(GAP_LE_Event_Data->Event_Data.GAP_LE_Advertising_Report_Event_Data->Advertising_Data[Index]);

               /* Display the packet type for the device                */
               switch(DeviceEntryPtr->Advertising_Report_Type)
               {
                  case rtConnectableUndirected:
                     printf("  Advertising Type: rtConnectableUndirected.\r\n");
                     break;
                  case rtConnectableDirected:
                     printf("  Advertising Type: rtConnectableDirected.\r\n");
                     break;
                  case rtScannableUndirected:
                     printf("  Advertising Type: rtScannableUndirected.\r\n");
                     break;
                  case rtNonConnectableUndirected:
                     printf("  Advertising Type: rtNonConnectableUndirected.\r\n");
                     break;
                  case rtScanResponse:
                     printf("  Advertising Type: rtScanResponse.\r\n");
                     break;
               }

               /* Display the Address Type.                             */
               switch(DeviceEntryPtr->Address_Type)
               {
                  case latPublic:
                     printf("  Address Type: Public.\r\n");
                     break;
                  case latRandom:
                     printf("  Address Type: Random.\r\n");
                     break;
                  case latPublicIdentity:
                     printf("  Address Type: Public Identity.\r\n");
                     break;
                  case latRandomIdentity:
                     printf("  Address Type: Random Identity.\r\n");
                     break;
                  default:
                     printf("  Address Type: Unknown.\r\n");
               }

               /* Display the Device Address.                           */
               printf("  Address: 0x%02X%02X%02X%02X%02X%02X.\r\n", DeviceEntryPtr->BD_ADDR.BD_ADDR5, DeviceEntryPtr->BD_ADDR.BD_ADDR4, DeviceEntryPtr->BD_ADDR.BD_ADDR3, DeviceEntryPtr->BD_ADDR.BD_ADDR2, DeviceEntryPtr->BD_ADDR.BD_ADDR1, DeviceEntryPtr->BD_ADDR.BD_ADDR0);
               printf("  RSSI: %d.\r\n", (int)DeviceEntryPtr->RSSI);
               printf("  Data Length: %d.\r\n", DeviceEntryPtr->Raw_Report_Length);

               /* If this is a resolvable private address we need to try*/
               /* to resolve the address to make sure that we do not    */
               /* already have an entry in the scan list.               */
               /* * NOTE * This may happen if we have enabled address   */
               /*          translation and added the remote device to   */
               /*          the device list previously.  Due to the      */
               /*          controller, the advertising report may not be*/
               /*          translated in the resolving list by the      */
               /*          controller at the time this advertising      */
               /*          report is received so we need to make sure   */
               /*          that we don't add a duplicate entry.         */
               if(GAP_LE_TEST_RESOLVABLE_ADDRESS_BITS(DeviceEntryPtr->BD_ADDR))
               {
                  /* Attempt to resolve this RPA.                       */
                  DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, DeviceEntryPtr->BD_ADDR);
                  if(DeviceInfo)
                  {
                     /* Check to see if the Peer Identity Information is*/
                     /* valid.                                          */
                     if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_PEER_IDENTITY_INFO_VALID)
                     {
                        /* Update the Last Known RPA for this device.   */
                        DeviceInfo->LastKnownResolvableAddressType  = DeviceEntryPtr->Address_Type;
                        DeviceInfo->LastKnownResolvableAddress      = DeviceEntryPtr->BD_ADDR;

                        DeviceInfo->Flags                          |= DEVICE_INFO_FLAGS_LAST_RESOLVABLE_VALID;

                        /* Print the resolved address.                  */
                        BD_ADDRToStr(DeviceInfo->PeerAddress, BoardStr);
                        printf("  Peer Address:      %s (%s)\r\n", BoardStr, ((DeviceInfo->PeerAddressType == latPublic) ? "Public" : "Static"));
                     }

                  }
               }
               else
                  DeviceInfo = NULL;
                  
               DisplayAdvertisingData(&(DeviceEntryPtr->Advertising_Data));
            }
            break;
         case etLE_Connection_Complete:
            printf("\r\netLE_Connection_Complete with size %d.\r\n",(int)GAP_LE_Event_Data->Event_Data_Size);

            if(GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data)
            {
               BD_ADDRToStr(GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Peer_Address, BoardStr);

               printf("   Status:       0x%02X.\r\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Status);
               printf("   Role:         %s.\r\n", (GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Master)?"Master":"Slave");

               /* Display the correct address type.                     */
               switch(GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Peer_Address_Type)
               {
                  case latPublic:
                     printf("   Address Type: Public.\r\n");
                     break;
                  case latRandom:
                     printf("   Address Type: Random.\r\n");
                     break;
                  case latPublicIdentity:
                     printf("   Address Type: Public Identity.\r\n");
                     break;
                  case latRandomIdentity:
                     printf("   Address Type: Random Identity.\r\n");
                     break;
                  default:
                     printf("   Address Type: Unknown.\r\n");
                     break;
               }

               printf("   BD_ADDR:      %s.\r\n", BoardStr);

               if(GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Status == HCI_ERROR_CODE_NO_ERROR)
               {
                  ConnectionBD_ADDR   = GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Peer_Address;
                  LocalDeviceIsMaster = GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Master;

                  /* Make sure that no entry already exists.            */
                  if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Peer_Address)) == NULL)
                  {
                     /* No entry exists so create one.                  */
                     if(!CreateNewDeviceInfoEntry(&DeviceInfoList, GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Peer_Address_Type, GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Peer_Address))
                     {
                        printf("Failed to add device to Device Info List.\r\n");
                     }
                  }
                  else
                  {
                     /* If the incoming connection address does not     */
                     /* match the connection address in the device      */
                     /* information then we need to update it.          */
                     if(!COMPARE_BD_ADDR(ConnectionBD_ADDR, DeviceInfo->ConnectionBD_ADDR))
                     {
                        printf("Updating connection address.\r\n");
                        DeviceInfo->ConnectionBD_ADDR     = ConnectionBD_ADDR;
                        DeviceInfo->ConnectionAddressType = GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Peer_Address_Type;
                     }

                     /* If the incoming connection address is a         */
                     /* resolvable private address and it does not match*/
                     /* the last known resolvable address then we need  */
                     /* to update it.                                   */
                     if((GAP_LE_TEST_RESOLVABLE_ADDRESS_BITS(ConnectionBD_ADDR)) && (!COMPARE_BD_ADDR(ConnectionBD_ADDR, DeviceInfo->LastKnownResolvableAddress)))
                     {
                        printf("Updating last known resolvable address.\r\n");
                        DeviceInfo->LastKnownResolvableAddress      = ConnectionBD_ADDR;
                        DeviceInfo->LastKnownResolvableAddressType  = GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Peer_Address_Type;
                        DeviceInfo->Flags                          |= DEVICE_INFO_FLAGS_LAST_RESOLVABLE_VALID;
                     }

                     /* If we are the Master of the connection we will  */
                     /* attempt to Re-Establish Security if a LTK for   */
                     /* this device exists (i.e.  we previously paired).*/
                     if(LocalDeviceIsMaster)
                     {
                        /* Re-Establish Security if there is a LTK that */
                        /* is stored for this device.                   */
                        if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_LTK_VALID)
                        {
                           /* Re-Establish Security with this LTK.      */
                           printf("Attempting to Re-Establish Security.\r\n");

                           /* Attempt to re-establish security to this  */
                           /* device.                                   */
                           GAP_LE_Security_Information.Local_Device_Is_Master                                      = TRUE;
                           BTPS_MemCopy(&(GAP_LE_Security_Information.Security_Information.Master_Information.LTK), &(DeviceInfo->LTK), LONG_TERM_KEY_SIZE);
                           BTPS_MemCopy(&(GAP_LE_Security_Information.Security_Information.Master_Information.Rand), &(DeviceInfo->Rand), RANDOM_NUMBER_DATA_SIZE);

                           GAP_LE_Security_Information.Security_Information.Master_Information.EDIV                = DeviceInfo->EDIV;
                           GAP_LE_Security_Information.Security_Information.Master_Information.Encryption_Key_Size = DeviceInfo->EncryptionKeySize;

                           Result = GAP_LE_Reestablish_Security(BluetoothStackID, ConnectionBD_ADDR, &GAP_LE_Security_Information, GAP_LE_Event_Callback, 0);
                           if(Result)
                           {
                              printf("GAP_LE_Reestablish_Security returned %d.\r\n", Result);
                           }
                        }
                     }
                  }
               }
            }
            break;
         case etLE_Disconnection_Complete:
            printf("\r\netLE_Disconnection_Complete with size %d.\r\n", (int)GAP_LE_Event_Data->Event_Data_Size);
            if(GAP_LE_Event_Data->Event_Data.GAP_LE_Disconnection_Complete_Event_Data)
            {
               printf("   Status: 0x%02X.\r\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Disconnection_Complete_Event_Data->Status);
               printf("   Reason: 0x%02X.\r\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Disconnection_Complete_Event_Data->Reason);

               BD_ADDRToStr(GAP_LE_Event_Data->Event_Data.GAP_LE_Disconnection_Complete_Event_Data->Peer_Address, BoardStr);
               printf("   BD_ADDR: %s.\r\n", BoardStr);

               /* Clear the Send Information for LE.                    */
               SendInfo.BytesToSend = 0;
               SendInfo.BytesSent   = 0;

               /* Check to see if the device info is present in the     */
               /* list.                                                 */
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
               {
                  /* Flag that no service discovery operation is        */
                  /* outstanding for this device.                       */
                  DeviceInfo->Flags &= ~DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_OUTSTANDING;

                  /* Re-initialize the Transmit and Receive Buffers, as */
                  /* well as the transmit credits.                      */
                  InitializeBuffer(&(DeviceInfo->TransmitBuffer));
                  InitializeBuffer(&(DeviceInfo->ReceiveBuffer));

                  /* Clear the CCCDs stored for this device.            */
                  DeviceInfo->ServerInfo.Rx_Credit_Client_Configuration_Descriptor = 0;
                  DeviceInfo->ServerInfo.Tx_Client_Configuration_Descriptor        = 0;

                  /* Clear the Transmit Credits count.                  */
                  DeviceInfo->TransmitCredits = 0;

                  /* If this device is not paired, then delete it.  The */
                  /* link will be encrypted if the device is paired.    */
                  if(!(DeviceInfo->Flags & DEVICE_INFO_FLAGS_LINK_ENCRYPTED))
                  {
                     if((DeviceInfo = DeleteDeviceInfoEntry(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
                        FreeDeviceInfoEntryMemory(DeviceInfo);
                  }
                  else
                  {
                     /* Flag that the Link is no longer encrypted since */
                     /* we have disconnected.                           */
                     DeviceInfo->Flags &= ~DEVICE_INFO_FLAGS_LINK_ENCRYPTED;
                  }
               }

               /* Clear the saved Connection BD_ADDR.                   */
               ASSIGN_BD_ADDR(ConnectionBD_ADDR, 0, 0, 0, 0, 0, 0);
               LocalDeviceIsMaster = FALSE;
            }
            break;
         case etLE_Data_Length_Change:
            printf("\r\netLE_Data_Length_Change with size %d.\r\n", (int)GAP_LE_Event_Data->Event_Data_Size);
            if(GAP_LE_Event_Data->Event_Data.GAP_LE_Data_Length_Change_Event_Data)
            {
               BD_ADDRToStr(GAP_LE_Event_Data->Event_Data.GAP_LE_Data_Length_Change_Event_Data->BD_ADDR, BoardStr);
               printf("   BD_ADDR: %s.\r\n", BoardStr);
               printf("   Max Tx Octets: %d\r\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Data_Length_Change_Event_Data->MaxTxOctets);
               printf("   Max Tx Time:   %d\r\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Data_Length_Change_Event_Data->MaxTxTime);
               printf("   Max Rx Octets: %d\r\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Data_Length_Change_Event_Data->MaxRxOctets);
               printf("   Max Rx Time:   %d\r\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Data_Length_Change_Event_Data->MaxRxTime);
            }
            break;
         case etLE_Encryption_Change:
            printf("\r\netLE_Encryption_Change with size %d.\r\n",(int)GAP_LE_Event_Data->Event_Data_Size);

            /* Search for the device entry to see flag if the link is   */
            /* encrypted.                                               */
            if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, GAP_LE_Event_Data->Event_Data.GAP_LE_Encryption_Change_Event_Data->BD_ADDR)) != NULL)
            {
               /* Check to see if the encryption change was successful. */
               if((GAP_LE_Event_Data->Event_Data.GAP_LE_Encryption_Change_Event_Data->Encryption_Change_Status == HCI_ERROR_CODE_NO_ERROR) && (GAP_LE_Event_Data->Event_Data.GAP_LE_Encryption_Change_Event_Data->Encryption_Mode == emEnabled))
                  DeviceInfo->Flags |= DEVICE_INFO_FLAGS_LINK_ENCRYPTED;
               else
                  DeviceInfo->Flags &= ~DEVICE_INFO_FLAGS_LINK_ENCRYPTED;
            }
            break;
         case etLE_Encryption_Refresh_Complete:
            printf("\r\netLE_Encryption_Refresh_Complete with size %d.\r\n", (int)GAP_LE_Event_Data->Event_Data_Size);

            /* Search for the device entry to see flag if the link is   */
            /* encrypted.                                               */
            if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, GAP_LE_Event_Data->Event_Data.GAP_LE_Encryption_Refresh_Complete_Event_Data->BD_ADDR)) != NULL)
            {
               /* Check to see if the refresh was successful.           */
               if(GAP_LE_Event_Data->Event_Data.GAP_LE_Encryption_Refresh_Complete_Event_Data->Status == HCI_ERROR_CODE_NO_ERROR)
                  DeviceInfo->Flags |= DEVICE_INFO_FLAGS_LINK_ENCRYPTED;
               else
                  DeviceInfo->Flags &= ~DEVICE_INFO_FLAGS_LINK_ENCRYPTED;
            }
            break;
         case etLE_Authentication:
            printf("\r\netLE_Authentication with size %d.\r\n", (int)GAP_LE_Event_Data->Event_Data_Size);

            /* Make sure the authentication event data is valid before  */
            /* continuing.                                              */
            if((Authentication_Event_Data = GAP_LE_Event_Data->Event_Data.GAP_LE_Authentication_Event_Data) != NULL)
            {
               if(((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, Authentication_Event_Data->BD_ADDR)) != NULL) && (!COMPARE_NULL_BD_ADDR(DeviceInfo->PeerAddress)))
               {
                  BD_ADDRToStr(DeviceInfo->PeerAddress, BoardStr);
               }
               else
               {
                  BD_ADDRToStr(Authentication_Event_Data->BD_ADDR, BoardStr);
               }

               switch(Authentication_Event_Data->GAP_LE_Authentication_Event_Type)
               {
                  case latLongTermKeyRequest:
                     printf("    latKeyRequest: \r\n");
                     printf("      BD_ADDR: %s.\r\n", BoardStr);

                     /* Initialize the authentication response data to  */
                     /* indicate no LTK present (if we find or          */
                     /* re-generate the LTK we will update this         */
                     /* structure accordingly).                         */
                     GAP_LE_Authentication_Response_Information.GAP_LE_Authentication_Type = larLongTermKey;
                     GAP_LE_Authentication_Response_Information.Authentication_Data_Length = 0;

                     /* Initialize some variables to determine if this  */
                     /* is a request for a Long Term Key generated via  */
                     /* Secure Connections (which we must store and can */
                     /* NOT re-generate).                               */
                     BTPS_MemInitialize(&RandomNumber, 0, sizeof(RandomNumber));
                     EDIV = 0;

                     /* Check to see if this is a request for a SC      */
                     /* generated Long Term Key.                        */
                     if((Authentication_Event_Data->Authentication_Event_Data.Long_Term_Key_Request.EDIV == EDIV) && (COMPARE_RANDOM_NUMBER(Authentication_Event_Data->Authentication_Event_Data.Long_Term_Key_Request.Rand, RandomNumber)))
                     {
                        /* Search for the entry for this slave to store */
                        /* the information into.                        */
                        if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, Authentication_Event_Data->BD_ADDR)) != NULL)
                        {
                           /* Check to see if the LTK is valid.         */
                           if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_LTK_VALID)
                           {
                              /* Respond with the stored Long Term Key. */
                              GAP_LE_Authentication_Response_Information.Authentication_Data_Length                                        = GAP_LE_LONG_TERM_KEY_INFORMATION_DATA_SIZE;
                              GAP_LE_Authentication_Response_Information.Authentication_Data.Long_Term_Key_Information.Encryption_Key_Size = DeviceInfo->EncryptionKeySize;

                              BTPS_MemCopy(&(GAP_LE_Authentication_Response_Information.Authentication_Data.Long_Term_Key_Information.Long_Term_Key), &(DeviceInfo->LTK), LONG_TERM_KEY_SIZE);
                           }
                        }
                     }
                     else
                     {
                        /* The other side of a connection is requesting    */
                        /* that we start encryption. Thus we should        */
                        /* regenerate LTK for this connection and send it  */
                        /* to the chip.                                    */
                        Result = GAP_LE_Regenerate_Long_Term_Key(BluetoothStackID, (Encryption_Key_t *)(&DHK), (Encryption_Key_t *)(&ER), Authentication_Event_Data->Authentication_Event_Data.Long_Term_Key_Request.EDIV, &(Authentication_Event_Data->Authentication_Event_Data.Long_Term_Key_Request.Rand), &GeneratedLTK);
                        if(!Result)
                        {
                           printf("      GAP_LE_Regenerate_Long_Term_Key Success.\r\n");

                           /* Respond with the Re-Generated Long Term Key. */
                           GAP_LE_Authentication_Response_Information.GAP_LE_Authentication_Type                                        = larLongTermKey;
                           GAP_LE_Authentication_Response_Information.Authentication_Data_Length                                        = GAP_LE_LONG_TERM_KEY_INFORMATION_DATA_SIZE;
                           GAP_LE_Authentication_Response_Information.Authentication_Data.Long_Term_Key_Information.Encryption_Key_Size = GAP_LE_MAXIMUM_ENCRYPTION_KEY_SIZE;
                           GAP_LE_Authentication_Response_Information.Authentication_Data.Long_Term_Key_Information.Long_Term_Key       = GeneratedLTK;
                        }
                        else
                        {
                           printf("      GAP_LE_Regenerate_Long_Term_Key returned %d.\r\n",Result);
                        }
                     }

                     /* Send the Authentication Response.               */
                     Result = GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information);
                     if(Result)
                     {
                        printf("      GAP_LE_Authentication_Response returned %d.\r\n",Result);
                     }
                     break;
                  case latSecurityRequest:
                     /* Display the data for this event.                */
                     /* * NOTE * This is only sent from Slave to Master.*/
                     /*          Thus we must be the Master in this     */
                     /*          connection.                            */
                     printf("    latSecurityRequest:.\r\n");
                     printf("      BD_ADDR: %s.\r\n", BoardStr);
                     printf("      Bonding Type: %s.\r\n", ((Authentication_Event_Data->Authentication_Event_Data.Security_Request.Bonding_Type == lbtBonding)?"Bonding":"No Bonding"));
                     printf("      MITM: %s.\r\n", ((Authentication_Event_Data->Authentication_Event_Data.Security_Request.MITM)?"YES":"NO"));

                     /* Determine if we have previously paired with the */
                     /* device. If we have paired we will attempt to    */
                     /* re-establish security using a previously        */
                     /* exchanged LTK.                                  */
                     if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, Authentication_Event_Data->BD_ADDR)) != NULL)
                     {
                        /* Determine if a Valid Long Term Key is stored */
                        /* for this device.                             */
                        if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_LTK_VALID)
                        {
                           printf("Attempting to Re-Establish Security.\r\n");

                           /* Attempt to re-establish security to this  */
                           /* device.                                   */
                           GAP_LE_Security_Information.Local_Device_Is_Master                                      = TRUE;

                           BTPS_MemCopy(&(GAP_LE_Security_Information.Security_Information.Master_Information.LTK), &(DeviceInfo->LTK), LONG_TERM_KEY_SIZE);
                           BTPS_MemCopy(&(GAP_LE_Security_Information.Security_Information.Master_Information.Rand), &(DeviceInfo->Rand), RANDOM_NUMBER_DATA_SIZE);

                           GAP_LE_Security_Information.Security_Information.Master_Information.EDIV                = DeviceInfo->EDIV;
                           GAP_LE_Security_Information.Security_Information.Master_Information.Encryption_Key_Size = DeviceInfo->EncryptionKeySize;

                           Result = GAP_LE_Reestablish_Security(BluetoothStackID, ConnectionBD_ADDR, &GAP_LE_Security_Information, GAP_LE_Event_Callback, 0);

                           if(Result)
                           {
                              printf("GAP_LE_Reestablish_Security returned %d.\r\n",Result);
                           }
                        }
                        else
                        {
                           CurrentRemoteBD_ADDR = Authentication_Event_Data->BD_ADDR;

                           /* We do not have a stored Link Key for this */
                           /* device so go ahead and pair to this       */
                           /* device.                                   */
                           SendPairingRequest(Authentication_Event_Data->BD_ADDR, TRUE);
                        }
                     }
                     else
                     {
                        CurrentRemoteBD_ADDR = Authentication_Event_Data->BD_ADDR;

                        /* There is no Key Info Entry for this device   */
                        /* so we will just treat this as a slave        */
                        /* request and initiate pairing.                */
                        SendPairingRequest(Authentication_Event_Data->BD_ADDR, TRUE);
                     }

                     break;
                  case latPairingRequest:
                     CurrentRemoteBD_ADDR = Authentication_Event_Data->BD_ADDR;

                     printf("Pairing Request: %s.\r\n", BoardStr);
                     DisplayLegacyPairingInformation(&(Authentication_Event_Data->Authentication_Event_Data.Pairing_Request));

                     /* This is a pairing request. Respond with a       */
                     /* Pairing Response.                               */
                     /* * NOTE * This is only sent from Master to Slave.*/
                     /*          Thus we must be the Slave in this      */
                     /*          connection.                            */

                     /* Send the Pairing Response if we are not in      */
                     /* Secure Connections Only mode.                   */
                     if(!SecureConnectionsOnly)
                     {
                        SlavePairingRequestResponse(Authentication_Event_Data->BD_ADDR, TRUE);
                     }
                     else
                     {      
                        printf("Rejecting non-Secure Connections pairing request.\r\n");
                        SlavePairingRequestResponse(Authentication_Event_Data->BD_ADDR, FALSE);
                     }
                     
                     break;
                  case latExtendedPairingRequest:
                     CurrentRemoteBD_ADDR = Authentication_Event_Data->BD_ADDR;

                     printf("Extended Pairing Request: %s.\r\n", BoardStr);
                     DisplayPairingInformation(&(Authentication_Event_Data->Authentication_Event_Data.Extended_Pairing_Request));

                     /* This is a pairing request. Respond with a       */
                     /* Pairing Response.                               */
                     /* * NOTE * This is only sent from Master to Slave.*/
                     /*          Thus we must be the Slave in this      */
                     /*          connection.                            */

                     /* Send the Pairing Response.                      */
                     if(!(SecureConnectionsOnly) || (Authentication_Event_Data->Authentication_Event_Data.Extended_Pairing_Request.Flags & GAP_LE_EXTENDED_PAIRING_CAPABILITIES_FLAGS_SECURE_CONNECTIONS))
                     {
                        SlavePairingRequestResponse(Authentication_Event_Data->BD_ADDR, TRUE);
                     }
                     else
                     {
                        /* Reject the pairing request if we are in      */
                        /* Secure Connections Only mode and this is not */
                        /* a Secure Connections pairing request.        */
                        printf("Rejecting non-Secure Connections pairing request.\r\n");
                        
                        SlavePairingRequestResponse(Authentication_Event_Data->BD_ADDR, FALSE);
                     }
                     break;
                  case latConfirmationRequest:
                     printf("latConfirmationRequest.\r\n");

                     /* Check to see what type of confirmation request  */
                     /* this is.                                        */
                     switch(Authentication_Event_Data->Authentication_Event_Data.Confirmation_Request.Request_Type)
                     {
                        case crtNone:
                           /* Handle the just works request.            */
                           GAP_LE_Authentication_Response_Information.GAP_LE_Authentication_Type = larConfirmation;

                           /* By setting the Authentication_Data_Length */
                           /* to any NON-ZERO value we are informing the*/
                           /* GAP LE Layer that we are accepting Just   */
                           /* Works Pairing.                            */
                           if(!SecureConnectionsOnly)
                           {
                              GAP_LE_Authentication_Response_Information.Authentication_Data_Length = DWORD_SIZE;

                              /* Handle this differently based on the local*/
                              /* IO Caps.                                  */
                              switch(LE_Parameters.IOCapability)
                              {
                                 case licNoInputNoOutput:
                                    printf("Invoking Just Works.\r\n");
                                    
                                    Result = GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information);
                                    if(Result)
                                    {
                                       printf("GAP_LE_Authentication_Response returned %d.\r\n",Result);
                                    }
                                    break;
                                 case licDisplayOnly:
                                    printf("Confirmation of Pairing.\r\n");

                                    GAP_LE_Authentication_Response_Information.Authentication_Data.Passkey = Authentication_Event_Data->Authentication_Event_Data.Confirmation_Request.Display_Passkey;

                                    /* Submit the Authentication Response. */
                                    if((Result = GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information)) != 0)
                                       DisplayFunctionError("GAP_LE_Authentication_Response", Result);
                                    break;
                                 default:
                                    printf("Confirmation of Pairing.\r\n");

                                    /* Submit the Authentication Response. */
                                    if((Result = GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information)) != 0)
                                       DisplayFunctionError("GAP_LE_Authentication_Response", Result);
                                    break;
                              }
                           }
                           else
                           {
                              /* Otherwise, reject the request.         */
                              GAP_LE_Authentication_Response_Information.Authentication_Data_Length = 0;
                              
                              printf("Rejecting non-Secure Connections confirmation request.\r\n");
                              
                              Result = GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information);
                              if(Result)
                              {
                                 printf("GAP_LE_Authentication_Response returned %d.\r\n",Result);
                              }
                           }
                           break;
                        case crtPasskey:
                           /* Inform the user to call the appropriate   */
                           /* command.                                  */
                           printf("Call LEPasskeyResponse [PASSCODE].\r\n");
                           break;
                        case crtDisplay:
                           printf("Passkey: %06u.\r\n", (unsigned int)(Authentication_Event_Data->Authentication_Event_Data.Confirmation_Request.Display_Passkey));
                           break;
                        default:
                           /* This application doesn't support OOB and  */
                           /* Secure Connections request types will be  */
                           /* handled by the ExtendedConfirmationRequest*/
                           /* event.  So we will simply inform the user.*/
                           printf("Authentication method not supported.\r\n");
                           break;
                     }
                     break;
                  case latExtendedConfirmationRequest:
                     printf("latExtendedConfirmationRequest.\r\n");

                     printf("   Secure Connections:     %s.\r\n", (Authentication_Event_Data->Authentication_Event_Data.Extended_Confirmation_Request.Flags & GAP_LE_EXTENDED_CONFIRMATION_REQUEST_FLAGS_SECURE_CONNECTIONS)?"YES":"NO");
                     printf("   Just Works Pairing:     %s.\r\n", (Authentication_Event_Data->Authentication_Event_Data.Extended_Confirmation_Request.Flags & GAP_LE_EXTENDED_CONFIRMATION_REQUEST_FLAGS_JUST_WORKS_PAIRING)?"YES":"NO");
                     printf("   Keypress Notifications: %s.\r\n", (Authentication_Event_Data->Authentication_Event_Data.Extended_Confirmation_Request.Flags & GAP_LE_EXTENDED_CONFIRMATION_REQUEST_FLAGS_KEYPRESS_NOTIFICATIONS_REQUESTED)?"YES":"NO");

                     /* Check that if we are in Secure Connections Only    */
                     /* mode that Secure Connections is supported.         */
                     if(!(SecureConnectionsOnly) || (Authentication_Event_Data->Authentication_Event_Data.Extended_Confirmation_Request.Flags & GAP_LE_EXTENDED_CONFIRMATION_REQUEST_FLAGS_SECURE_CONNECTIONS))
                     {
                        /* Check to see what type of confirmation request  */
                        /* this is.                                        */
                        switch(Authentication_Event_Data->Authentication_Event_Data.Extended_Confirmation_Request.Request_Type)
                        {
                           case crtNone:
                              /* Handle the just works request.            */
                              GAP_LE_Authentication_Response_Information.GAP_LE_Authentication_Type = larConfirmation;

                              /* By setting the Authentication_Data_Length */
                              /* to any NON-ZERO value we are informing the*/
                              /* GAP LE Layer that we are accepting Just   */
                              /* Works Pairing.                            */
                              GAP_LE_Authentication_Response_Information.Authentication_Data_Length = DWORD_SIZE;

                              /* Handle this differently based on the local*/
                              /* IO Caps.                                  */
                              switch(LE_Parameters.IOCapability)
                              {
                                 case licNoInputNoOutput:
                                    printf("Invoking Just Works.\r\n");

                                    /* Just Accept Just Works Pairing.     */
                                    Result = GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information);
                                    if(Result)
                                    {
                                       printf("GAP_LE_Authentication_Response returned %d.\r\n", Result);
                                    }
                                    break;
                                 case licDisplayOnly:
                                    printf("Confirmation of Pairing.\r\n");

                                    GAP_LE_Authentication_Response_Information.Authentication_Data.Passkey = Authentication_Event_Data->Authentication_Event_Data.Confirmation_Request.Display_Passkey;

                                    /* Submit the Authentication Response. */
                                    if((Result = GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information)) != 0)
                                       DisplayFunctionError("GAP_LE_Authentication_Response", Result);
                                    break;
                                 default:
                                    printf("Confirmation of Pairing.\r\n");

                                    /* Submit the Authentication Response. */
                                    if((Result = GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information)) != 0)
                                       DisplayFunctionError("GAP_LE_Authentication_Response", Result);
                                    break;
                              }
                              break;
                           case crtPasskey:
                              /* Inform the user to call the appropriate   */
                              /* command.                                  */
                              printf("Call LEPasskeyResponse [PASSKEY].\r\n");
                              break;
                           case crtDisplay:
                              printf("Passkey: %06u.\r\n", Authentication_Event_Data->Authentication_Event_Data.Extended_Confirmation_Request.Display_Passkey);

                              GAP_LE_Authentication_Response_Information.GAP_LE_Authentication_Type  = larPasskey;
                              GAP_LE_Authentication_Response_Information.Authentication_Data_Length  = (Byte_t)(sizeof(DWord_t));
                              GAP_LE_Authentication_Response_Information.Authentication_Data.Passkey = Authentication_Event_Data->Authentication_Event_Data.Extended_Confirmation_Request.Display_Passkey;

                              /* Since this is in an extended confirmation */
                              /* request we need to respond to the display */
                              /* request.                                  */
                              if((Result = GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information)) != 0)
                                 DisplayFunctionError("GAP_LE_Authentication_Response", Result);
                              break;
                           case crtDisplayYesNo:
                              /* Handle the Display Yes/No request.        */
                              GAP_LE_Authentication_Response_Information.GAP_LE_Authentication_Type = larConfirmation;

                              /* By setting the Authentication_Data_Length */
                              /* to any NON-ZERO value we are informing the*/
                              /* GAP LE Layer that we are accepting Just   */
                              /* Works Pairing.                            */
                              GAP_LE_Authentication_Response_Information.Authentication_Data_Length = DWORD_SIZE;

                              /* Check to see if this is Just Works or     */
                              /* Numeric Comparison.                       */
                              if(Authentication_Event_Data->Authentication_Event_Data.Extended_Confirmation_Request.Flags & GAP_LE_EXTENDED_CONFIRMATION_REQUEST_FLAGS_JUST_WORKS_PAIRING)
                              {
                                 /* Handle this differently based on the   */
                                 /* local IO Caps.                         */
                                 switch(LE_Parameters.IOCapability)
                                 {
                                    case licNoInputNoOutput:
                                       printf("Invoking Just Works.\r\n");

                                       /* Just Accept Just Works Pairing.  */
                                       Result = GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information);
                                       if(Result)
                                       {
                                          printf("GAP_LE_Authentication_Response returned %d.\r\n", Result);
                                       }
                                       break;
                                    case licDisplayOnly:
                                       printf("Confirmation of Pairing.\r\n");

                                       GAP_LE_Authentication_Response_Information.Authentication_Data.Passkey = Authentication_Event_Data->Authentication_Event_Data.Confirmation_Request.Display_Passkey;

                                       /* Submit the Authentication        */
                                       /* Response.                        */
                                       if((Result = GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information)) != 0)
                                          DisplayFunctionError("GAP_LE_Authentication_Response", Result);
                                       break;
                                    default:
                                       printf("Confirmation of Pairing.\r\n");

                                       /* Submit the Authentication        */
                                       /* Response.                        */
                                       if((Result = GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information)) != 0)
                                          DisplayFunctionError("GAP_LE_Authentication_Response", Result);
                                       break;
                                 }
                              }
                              else
                              {
                                 /* This is numeric comparison so go ahead */
                                 /* and display the numeric value to       */
                                 /* confirm.                               */
                                 printf("Confirmation Value: %ld\r\n", (unsigned long)Authentication_Event_Data->Authentication_Event_Data.Extended_Confirmation_Request.Display_Passkey);

                                 /* Submit the Authentication Response.    */
                                 if((Result = GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information)) != 0)
                                    DisplayFunctionError("GAP_LE_Authentication_Response", Result);
                                 break;
                              }
                              break;
                           default:
                              /* This application doesn't support OOB so we*/
                              /* will simply inform the user.              */
                              printf("Authentication method not supported.\r\n");
                              break;
                        }
                     }
                     else
                     {
                        /* Set the authentication data length to 0 to   */
                        /* reject the request.                          */
                        GAP_LE_Authentication_Response_Information.Authentication_Data_Length = 0;
                        
                        printf("Rejecting non-Secure Connections confirmation request.\r\n");
                              
                        /* Submit the Authentication Response.          */
                        if((Result = GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information)) != 0)
                           DisplayFunctionError("GAP_LE_Authentication_Response", Result);
                     }
                     break;
                  case latSecurityEstablishmentComplete:
                     printf("Security Re-Establishment Complete: %s.\r\n", BoardStr);
                     printf("                            Status: 0x%02X.\r\n", Authentication_Event_Data->Authentication_Event_Data.Security_Establishment_Complete.Status);
                     break;
                  case latPairingStatus:
                     ASSIGN_BD_ADDR(CurrentRemoteBD_ADDR, 0, 0, 0, 0, 0, 0);

                     printf("Pairing Status: %s.\r\n", BoardStr);
                     printf("        Status: 0x%02X.\r\n", Authentication_Event_Data->Authentication_Event_Data.Pairing_Status.Status);

                     if(Authentication_Event_Data->Authentication_Event_Data.Pairing_Status.Status == GAP_LE_PAIRING_STATUS_NO_ERROR)
                     {
                        printf("        Key Size: %d.\r\n", Authentication_Event_Data->Authentication_Event_Data.Pairing_Status.Negotiated_Encryption_Key_Size);
                     }
                     else
                     {
                        /* Failed to pair so delete the key entry for   */
                        /* this device and disconnect the link.         */
                        if((DeviceInfo = DeleteDeviceInfoEntry(&DeviceInfoList, Authentication_Event_Data->BD_ADDR)) != NULL)
                           FreeDeviceInfoEntryMemory(DeviceInfo);

                        /* Disconnect the Link.                         */
                        GAP_LE_Disconnect(BluetoothStackID, Authentication_Event_Data->BD_ADDR);
                     }
                     break;
                  case latEncryptionInformationRequest:
                     printf("Encryption Information Request %s.\r\n", BoardStr);

                     /* Generate new LTK, EDIV and Rand and respond with*/
                     /* them.                                           */
                     EncryptionInformationRequestResponse(Authentication_Event_Data->BD_ADDR, Authentication_Event_Data->Authentication_Event_Data.Encryption_Request_Information.Encryption_Key_Size, &GAP_LE_Authentication_Response_Information);
                     break;
                  case latEncryptionInformation:
                     /* Display the information from the event.         */
                     printf(" Encryption Information from RemoteDevice: %s.\r\n", BoardStr);
                     printf("    Key Size: %d.\r\n", Authentication_Event_Data->Authentication_Event_Data.Encryption_Information.Encryption_Key_Size);

                     /* Search for the entry for this slave to store the*/
                     /* information into.                               */
                     if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, Authentication_Event_Data->BD_ADDR)) != NULL)
                     {
                        BTPS_MemCopy(&(DeviceInfo->LTK), &(Authentication_Event_Data->Authentication_Event_Data.Encryption_Information.LTK), sizeof(DeviceInfo->LTK));
                        DeviceInfo->EDIV              = Authentication_Event_Data->Authentication_Event_Data.Encryption_Information.EDIV;
                        BTPS_MemCopy(&(DeviceInfo->Rand), &(Authentication_Event_Data->Authentication_Event_Data.Encryption_Information.Rand), sizeof(DeviceInfo->Rand));
                        DeviceInfo->EncryptionKeySize = Authentication_Event_Data->Authentication_Event_Data.Encryption_Information.Encryption_Key_Size;
                        DeviceInfo->Flags            |= DEVICE_INFO_FLAGS_LTK_VALID;
                     }
                     else
                     {
                        printf("No Key Info Entry for this device.\r\n");
                     }
                     break;
                  case latIdentityInformation:
                     printf("Identity Information from Remote Device: %s.\r\n", BoardStr);
                     
                     if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, Authentication_Event_Data->BD_ADDR)) == NULL)
                        CreateNewDeviceInfoEntry(&DeviceInfoList, Authentication_Event_Data->Authentication_Event_Data.Identity_Information.Address_Type, Authentication_Event_Data->BD_ADDR);
                     
                     if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, Authentication_Event_Data->BD_ADDR)) != NULL)
                     {
                        if(!(DeviceInfo->Flags & DEVICE_INFO_FLAGS_PEER_IDENTITY_INFO_VALID))
                        {
                           if(!COMPARE_BD_ADDR(Authentication_Event_Data->BD_ADDR, Authentication_Event_Data->Authentication_Event_Data.Identity_Information.Address))
                           {
                              DeviceInfo->LastKnownResolvableAddress  = Authentication_Event_Data->BD_ADDR;
                              DeviceInfo->Flags                      |= DEVICE_INFO_FLAGS_LAST_RESOLVABLE_VALID;
                           }
                           
                           DeviceInfo->PeerAddress      = Authentication_Event_Data->Authentication_Event_Data.Identity_Information.Address;
                           DeviceInfo->PeerAddressType  = Authentication_Event_Data->Authentication_Event_Data.Identity_Information.Address_Type;
                           DeviceInfo->PeerIRK          = Authentication_Event_Data->Authentication_Event_Data.Identity_Information.IRK;
                           DeviceInfo->Flags           |= DEVICE_INFO_FLAGS_PEER_IDENTITY_INFO_VALID;
                        }
                        
                        BD_ADDRToStr(DeviceInfo->PeerAddress, BoardStr);
                        printf("   Peer Address:      %s\r\n", BoardStr);
                        printf("   Peer Address Type: %s\r\n", (DeviceInfo->PeerAddressType) ? "latRandom" : "latPublic");
                        
                        if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_LAST_RESOLVABLE_VALID)
                        {
                        BD_ADDRToStr(DeviceInfo->LastKnownResolvableAddress, BoardStr);
                        printf("   Last RPA:          %s\r\n", BoardStr);
                        }
                     }
                     
                     break;
                  case latIdentityInformationRequest:
                     printf("Identity Information Request from Remote Device: %s.\r\n", BoardStr); 
                     
                     GAP_Query_Local_BD_ADDR(BluetoothStackID, &LocalBD_ADDR);
                     
                     GAP_LE_Authentication_Response_Information.GAP_LE_Authentication_Type                            = larIdentityInformation;
                     GAP_LE_Authentication_Response_Information.Authentication_Data_Length                            = GAP_LE_IDENTITY_INFORMATION_DATA_SIZE;
                     GAP_LE_Authentication_Response_Information.Authentication_Data.Identity_Information.IRK          = IRK;
                     GAP_LE_Authentication_Response_Information.Authentication_Data.Identity_Information.Address_Type = latPublicIdentity;
                     GAP_LE_Authentication_Response_Information.Authentication_Data.Identity_Information.Address      = LocalBD_ADDR;
                     
                     GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information);
                     
                     break;
                  case latSigningInformation:
                     printf("Signing Information from Remote Device: %s.\r\n", BoardStr);
                     break;
                  case latSigningInformationRequest:
                     printf("Signing Information Request from Remote Device: %s.\r\n", BoardStr); 
                     
                     GAP_Query_Local_BD_ADDR(BluetoothStackID, &LocalBD_ADDR);
                     
                     GAP_LE_Authentication_Response_Information.GAP_LE_Authentication_Type                            = larSigningInformation;
                     GAP_LE_Authentication_Response_Information.Authentication_Data_Length                            = GAP_LE_SIGNING_INFORMATION_DATA_SIZE;
                     GAP_LE_Authentication_Response_Information.Authentication_Data.Signing_Information.CSRK          = CSRK;
                     
                     GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information);
                     
                     break;
                  default:
                     printf("Unhandled event: %d.\r\n", Authentication_Event_Data->GAP_LE_Authentication_Event_Type);
                     break;
               }
            }
            else
               printf("DATA NULL!.\r\n");
            break;
         case etLE_Connection_Parameter_Update_Request:
            printf("\r\netLE_Connection_Parameter_Update_Request\r\n");
            
            if(GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Update_Request_Event_Data)
            {
               BD_ADDRToStr(GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Update_Request_Event_Data->BD_ADDR, BoardStr);
               printf("   BD_ADDR:                     %s\r\n", BoardStr);
               printf("   Connection Interval Minimum: %d\r\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Update_Request_Event_Data->Conn_Interval_Min);
               printf("   Connection Interval Maximum: %d\r\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Update_Request_Event_Data->Conn_Interval_Max);
               printf("   Slave Latency:               %d\r\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Update_Request_Event_Data->Slave_Latency);
               printf("   Supervision Timeout:         %d\r\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Update_Request_Event_Data->Conn_Supervision_Timeout);
               
               ConnectionParams.Connection_Interval_Min    = GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Update_Request_Event_Data->Conn_Interval_Min;
               ConnectionParams.Connection_Interval_Max    = GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Update_Request_Event_Data->Conn_Interval_Max;
               ConnectionParams.Slave_Latency              = GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Update_Request_Event_Data->Slave_Latency;
               ConnectionParams.Supervision_Timeout        = GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Update_Request_Event_Data->Conn_Supervision_Timeout;
               ConnectionParams.Minimum_Connection_Length  = 0;
               ConnectionParams.Maximum_Connection_Length  = 10000;
               
               GAP_LE_Connection_Parameter_Update_Response(BluetoothStackID, GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Update_Request_Event_Data->BD_ADDR, TRUE, &ConnectionParams);
            }
            
            break;
         case etLE_Connection_Parameter_Update_Response:
            printf("\r\netLE_Connection_Parameter_Update_Response\r\n");
            
            if(GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Update_Response_Event_Data)
            {
               BD_ADDRToStr(GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Update_Response_Event_Data->BD_ADDR, BoardStr);
               printf("   BD_ADDR:  %s\r\n", BoardStr);
               printf("   Accepted: %s\r\n", (GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Update_Response_Event_Data->Accepted ? "TRUE" : "FALSE"));
            }
            
            break;
         case etLE_Connection_Parameter_Updated:
            printf("\r\netLE_Connection_Parameter_Updated\r\n");
            
            if(GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Updated_Event_Data)
            {
               BD_ADDRToStr(GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Updated_Event_Data->BD_ADDR, BoardStr);
               printf("   BD_ADDR:             %s\r\n", BoardStr);
               printf("   Status:              %d\r\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Updated_Event_Data->Status);
               printf("   Connection Interval: %d\r\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Updated_Event_Data->Current_Connection_Parameters.Connection_Interval);
               printf("   Slave Latency:       %d\r\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Updated_Event_Data->Current_Connection_Parameters.Slave_Latency);
               printf("   Supervision Timeout: %d\r\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Updated_Event_Data->Current_Connection_Parameters.Supervision_Timeout);
            }
            
            break;
            break;
         default:
            break;
      }

      /* Display the command prompt.                                    */
      DisplayPrompt();
   }
}

   /* The following function is for an GATT Server Event Callback.  This*/
   /* function will be called whenever a GATT Request is made to the    */
   /* server who registers this function that cannot be handled         */
   /* internally by GATT.  This function passes to the caller the GATT  */
   /* Server Event Data that occurred and the GATT Server Event Callback*/
   /* Parameter that was specified when this Callback was installed.    */
   /* The caller is free to use the contents of the GATT Server Event   */
   /* Data ONLY in the context of this callback.  If the caller requires*/
   /* the Data for a longer period of time, then the callback function  */
   /* MUST copy the data into another Data Buffer.  This function is    */
   /* guaranteed NOT to be invoked more than once simultaneously for the*/
   /* specified installed callback (i.e.  this function DOES NOT have be*/
   /* reentrant).  It Needs to be noted however, that if the same       */
   /* Callback is installed more than once, then the callbacks will be  */
   /* called serially.  Because of this, the processing in this function*/
   /* should be as efficient as possible.  It should also be noted that */
   /* this function is called in the Thread Context of a Thread that the*/
   /* User does NOT own.  Therefore, processing in this function should */
   /* be as efficient as possible (this argument holds anyway because   */
   /* another GATT Event (Server/Client or Connection) will not be      */
   /* processed while this function call is outstanding).               */
   /* * NOTE * This function MUST NOT Block and wait for Events that can*/
   /*          only be satisfied by Receiving a Bluetooth Event         */
   /*          Callback.  A Deadlock WILL occur because NO Bluetooth    */
   /*          Callbacks will be issued while this function is currently*/
   /*          outstanding.                                             */
static void BTPSAPI GATT_ServerEventCallback(unsigned int BluetoothStackID, GATT_Server_Event_Data_t *GATT_ServerEventData, unsigned long CallbackParameter)
{
   Byte_t        Temp[2];
   Word_t        Value;
   Word_t        PreviousValue;
   Word_t        AttributeOffset;
   Word_t        AttributeLength;
   DeviceInfo_t *DeviceInfo;

   /* Verify that all parameters to this callback are Semi-Valid.       */
   if((BluetoothStackID) && (GATT_ServerEventData))
   {
      /* Grab the device for the currently connected device.            */
      if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
      {
         switch(GATT_ServerEventData->Event_Data_Type)
         {
            case etGATT_Server_Read_Request:
               /* Verify that the Event Data is valid.                  */
               if(GATT_ServerEventData->Event_Data.GATT_Read_Request_Data)
               {
                  if(GATT_ServerEventData->Event_Data.GATT_Read_Request_Data->AttributeValueOffset == 0)
                  {
                     /* Determine which request this read is coming for.*/
                     switch(GATT_ServerEventData->Event_Data.GATT_Read_Request_Data->AttributeOffset)
                     {
                        case SPPLE_TX_CHARACTERISTIC_CCD_ATTRIBUTE_OFFSET:
                           ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(Temp, DeviceInfo->ServerInfo.Tx_Client_Configuration_Descriptor);
                           break;
                        case SPPLE_TX_CREDITS_CHARACTERISTIC_ATTRIBUTE_OFFSET:
                           ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(Temp, DeviceInfo->TransmitCredits);
                           break;
                        case SPPLE_RX_CREDITS_CHARACTERISTIC_ATTRIBUTE_OFFSET:
                           ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(Temp, DeviceInfo->ReceiveBuffer.BytesFree);
                           break;
                        case SPPLE_RX_CREDITS_CHARACTERISTIC_CCD_ATTRIBUTE_OFFSET:
                           ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(Temp, DeviceInfo->ServerInfo.Rx_Credit_Client_Configuration_Descriptor);
                           break;
                     }

                     GATT_Read_Response(BluetoothStackID, GATT_ServerEventData->Event_Data.GATT_Read_Request_Data->TransactionID, WORD_SIZE, Temp);
                  }
                  else
                     GATT_Error_Response(BluetoothStackID, GATT_ServerEventData->Event_Data.GATT_Read_Request_Data->TransactionID, GATT_ServerEventData->Event_Data.GATT_Read_Request_Data->AttributeOffset, ATT_PROTOCOL_ERROR_CODE_ATTRIBUTE_NOT_LONG);
               }
               else
                  printf("Invalid Read Request Event Data.\r\n");
               break;
            case etGATT_Server_Write_Request:
               /* Verify that the Event Data is valid.                  */
               if(GATT_ServerEventData->Event_Data.GATT_Write_Request_Data)
               {
                  if(GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->AttributeValueOffset == 0)
                  {
                     /* Cache the Attribute Offset.                     */
                     AttributeOffset = GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->AttributeOffset;
                     AttributeLength = GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->AttributeValueLength;
                     
                     /* Check to see if this write is OK for our role.  */
                     if((AttributeOffset == SPPLE_TX_CHARACTERISTIC_CCD_ATTRIBUTE_OFFSET) || (AttributeOffset == SPPLE_RX_CREDITS_CHARACTERISTIC_CCD_ATTRIBUTE_OFFSET))
                     {
                        /* Check to see if we know if we are the Client */
                        /* or Server.                                   */
                        if(!(DeviceInfo->Flags & (DEVICE_INFO_FLAGS_SPPLE_CLIENT | DEVICE_INFO_FLAGS_SPPLE_SERVER)))
                        {
                           /* We will be the Server for this device.    */
                           DeviceInfo->Flags |= DEVICE_INFO_FLAGS_SPPLE_SERVER;
                        }
                        else
                        {
                           /* This indicates that we are acting as a       */
                           /* Server.  Error the request if we are a Client*/
                           if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_SPPLE_CLIENT)
                           {
                              /* Cause the Request to be invalid.          */
                              AttributeLength = 0;
                           }
                        }
                     }
                     
                     /* Verify that the value is of the correct length. */
                     if((AttributeOffset == SPPLE_RX_CHARACTERISTIC_ATTRIBUTE_OFFSET) || ((AttributeLength) && (AttributeLength <= WORD_SIZE)))
                     {
                        /* Since the value appears valid go ahead and   */
                        /* accept the write request.                    */
                        GATT_Write_Response(BluetoothStackID, GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->TransactionID);

                        /* If this is not a write to the Rx             */
                        /* Characteristic we will read the data here.   */
                        if(AttributeOffset != SPPLE_RX_CHARACTERISTIC_ATTRIBUTE_OFFSET)
                        {
                           if(GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->AttributeValueLength == WORD_SIZE)
                              Value = READ_UNALIGNED_WORD_LITTLE_ENDIAN(GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->AttributeValue);
                           else
                              Value = READ_UNALIGNED_BYTE_LITTLE_ENDIAN(GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->AttributeValue);
                        }
                        else
                           Value = 0;

                        /* Determine which attribute this write request */
                        /* is for.                                      */
                        switch(AttributeOffset)
                        {
                           case SPPLE_TX_CHARACTERISTIC_CCD_ATTRIBUTE_OFFSET:
                              /* Client has updated the Tx CCCD.  Now we*/
                              /* need to check if we have any data to   */
                              /* send.                                  */
                              DeviceInfo->ServerInfo.Tx_Client_Configuration_Descriptor = Value;

                              /* If may be possible for transmit queued */
                              /* data now.  So fake a Receive Credit    */
                              /* event with 0 as the received credits.  */
                              ReceiveCreditEvent(DeviceInfo, 0);
                              break;
                           case SPPLE_TX_CREDITS_CHARACTERISTIC_ATTRIBUTE_OFFSET:
                              /* Client has sent updated credits.       */
                              ReceiveCreditEvent(DeviceInfo, Value);
                              break;
                           case SPPLE_RX_CHARACTERISTIC_ATTRIBUTE_OFFSET:
                              /* Client has sent data, so we should     */
                              /* handle this as a data indication event.*/
                              DataIndicationEvent(DeviceInfo, GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->AttributeValueLength, GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->AttributeValue);

                              if((!AutomaticReadActive) && (!LoopbackActive))
                                 DisplayPrompt();
                              break;
                           case SPPLE_RX_CREDITS_CHARACTERISTIC_CCD_ATTRIBUTE_OFFSET:
                              /* Cache the previous CCD Value.          */
                              PreviousValue = DeviceInfo->ServerInfo.Rx_Credit_Client_Configuration_Descriptor;

                              /* Note the updated Rx CCCD Value.        */
                              DeviceInfo->ServerInfo.Rx_Credit_Client_Configuration_Descriptor = Value;

                              /* If we were not previously configured   */
                              /* for notifications send the initial     */
                              /* credits to the device.                 */
                              if(PreviousValue != GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_NOTIFY_ENABLE)
                              {
                                 /* Send the initial credits to the     */
                                 /* device.                             */
                                 SendCredits(DeviceInfo, DeviceInfo->ReceiveBuffer.BytesFree);
                              }
                              break;
                        }
                     }
                     else
                        GATT_Error_Response(BluetoothStackID, GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->TransactionID, GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->AttributeOffset, ATT_PROTOCOL_ERROR_CODE_INVALID_ATTRIBUTE_VALUE_LENGTH);
                  }
                  else
                     GATT_Error_Response(BluetoothStackID, GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->TransactionID, GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->AttributeOffset, ATT_PROTOCOL_ERROR_CODE_ATTRIBUTE_NOT_LONG);
               }
               else
                  printf("Invalid Write Request Event Data.\r\n");
               break;
            default:
               break;
         }
      }
   }
   else
   {
      /* There was an error with one or more of the input parameters.   */
      printf("\r\n");

      printf("GATT Callback Data: Event_Data = NULL.\r\n");

      DisplayPrompt();
   }
}

   /* The following function is for an GATT Client Event Callback.  This*/
   /* function will be called whenever a GATT Response is received for a*/
   /* request that was made when this function was registered.  This    */
   /* function passes to the caller the GATT Client Event Data that     */
   /* occurred and the GATT Client Event Callback Parameter that was    */
   /* specified when this Callback was installed.  The caller is free to*/
   /* use the contents of the GATT Client Event Data ONLY in the context*/
   /* of this callback.  If the caller requires the Data for a longer   */
   /* period of time, then the callback function MUST copy the data into*/
   /* another Data Buffer.  This function is guaranteed NOT to be       */
   /* invoked more than once simultaneously for the specified installed */
   /* callback (i.e.  this function DOES NOT have be reentrant).  It    */
   /* Needs to be noted however, that if the same Callback is installed */
   /* more than once, then the callbacks will be called serially.       */
   /* Because of this, the processing in this function should be as     */
   /* efficient as possible.  It should also be noted that this function*/
   /* is called in the Thread Context of a Thread that the User does NOT*/
   /* own.  Therefore, processing in this function should be as         */
   /* efficient as possible (this argument holds anyway because another */
   /* GATT Event (Server/Client or Connection) will not be processed    */
   /* while this function call is outstanding).                         */
   /* * NOTE * This function MUST NOT Block and wait for Events that can*/
   /*          only be satisfied by Receiving a Bluetooth Event         */
   /*          Callback.  A Deadlock WILL occur because NO Bluetooth    */
   /*          Callbacks will be issued while this function is currently*/
   /*          outstanding.                                             */
static void BTPSAPI GATT_ClientEventCallback_SPPLE(unsigned int BluetoothStackID, GATT_Client_Event_Data_t *GATT_Client_Event_Data, unsigned long CallbackParameter)
{
   Word_t        Credits;
   BoardStr_t    BoardStr;
   DeviceInfo_t *DeviceInfo;

   /* Verify that all parameters to this callback are Semi-Valid.       */
   if((BluetoothStackID) && (GATT_Client_Event_Data))
   {
      /* Determine the event that occurred.                             */
      switch(GATT_Client_Event_Data->Event_Data_Type)
      {
         case etGATT_Client_Error_Response:
            if(GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data)
            {
               printf("\r\nError Response.\r\n");
               BD_ADDRToStr(GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->RemoteDevice, BoardStr);
               printf("Connection ID:   %u.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ConnectionID);
               printf("Transaction ID:  %u.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->TransactionID);
               printf("Connection Type: %s.\r\n", (GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ConnectionType == gctLE)?"LE":"BR/EDR");
               printf("BD_ADDR:         %s.\r\n", BoardStr);
               printf("Error Type:      %s.\r\n", (GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorType == retErrorResponse)?"Response Error":"Response Timeout");

               /* Only print out the rest if it is valid.               */
               if(GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorType == retErrorResponse)
               {
                  printf("Request Opcode:  0x%02X.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->RequestOpCode);
                  printf("Request Handle:  0x%04X.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->RequestHandle);
                  printf("Error Code:      0x%02X.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorCode);
                  printf("Error Mesg:      %s.\r\n", ErrorCodeStr[GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorCode]);
               }
            }
            else
               printf("Error - Null Error Response Data.\r\n");
            break;
         case etGATT_Client_Read_Response:
            if(GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data)
            {
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->RemoteDevice)) != NULL)
               {
                  if((Word_t)CallbackParameter == DeviceInfo->ClientInfo.Rx_Credit_Characteristic)
                  {
                     if(GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength == WORD_SIZE)
                     {
                        /* Display the credits we just received.        */
                        Credits = READ_UNALIGNED_WORD_LITTLE_ENDIAN(GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValue);
                        printf("\r\nReceived %u Initial Credits.\r\n", Credits);

                        /* We have received the initial credits from the*/
                        /* device so go ahead and handle a Receive      */
                        /* Credit Event.                                */
                        ReceiveCreditEvent(DeviceInfo, Credits);
                     }
                  }
               }
            }
            else
               printf("\r\nError - Null Read Response Data.\r\n");
            break;
         case etGATT_Client_Exchange_MTU_Response:
            if(GATT_Client_Event_Data->Event_Data.GATT_Exchange_MTU_Response_Data)
            {
               printf("\r\nExchange MTU Response.\r\n");
               BD_ADDRToStr(GATT_Client_Event_Data->Event_Data.GATT_Exchange_MTU_Response_Data->RemoteDevice, BoardStr);
               printf("Connection ID:   %u.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Exchange_MTU_Response_Data->ConnectionID);
               printf("Transaction ID:  %u.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Exchange_MTU_Response_Data->TransactionID);
               printf("Connection Type: %s.\r\n", (GATT_Client_Event_Data->Event_Data.GATT_Exchange_MTU_Response_Data->ConnectionType == gctLE)?"LE":"BR/EDR");
               printf("BD_ADDR:         %s.\r\n", BoardStr);
               printf("GATT MTU:        %u.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Exchange_MTU_Response_Data->ServerMTU);
            }
            else
               printf("\r\nError - Null Write Response Data.\r\n");
            break;
         case etGATT_Client_Write_Response:
            if(GATT_Client_Event_Data->Event_Data.GATT_Write_Response_Data)
            {
               printf("\r\nWrite Response.\r\n");
               BD_ADDRToStr(GATT_Client_Event_Data->Event_Data.GATT_Write_Response_Data->RemoteDevice, BoardStr);
               printf("Connection ID:   %u.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Write_Response_Data->ConnectionID);
               printf("Transaction ID:  %u.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Write_Response_Data->TransactionID);
               printf("Connection Type: %s.\r\n", (GATT_Client_Event_Data->Event_Data.GATT_Write_Response_Data->ConnectionType == gctLE)?"LE":"BR/EDR");
               printf("BD_ADDR:         %s.\r\n", BoardStr);
               printf("Bytes Written:   %u.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Write_Response_Data->BytesWritten);
            }
            else
               printf("\r\nError - Null Write Response Data.\r\n");
            break;
         default:
            break;
      }

      /* Print the command line prompt.                                 */
      DisplayPrompt();
   }
   else
   {
      /* There was an error with one or more of the input parameters.   */
      printf("\r\n");

      printf("GATT Callback Data: Event_Data = NULL.\r\n");

      DisplayPrompt();
   }
}

   /* The following function is for an GATT Client Event Callback.  This*/
   /* function will be called whenever a GATT Response is received for a*/
   /* request that was made when this function was registered.  This    */
   /* function passes to the caller the GATT Client Event Data that     */
   /* occurred and the GATT Client Event Callback Parameter that was    */
   /* specified when this Callback was installed.  The caller is free to*/
   /* use the contents of the GATT Client Event Data ONLY in the context*/
   /* of this callback.  If the caller requires the Data for a longer   */
   /* period of time, then the callback function MUST copy the data into*/
   /* another Data Buffer.  This function is guaranteed NOT to be       */
   /* invoked more than once simultaneously for the specified installed */
   /* callback (i.e.  this function DOES NOT have be reentrant).  It    */
   /* Needs to be noted however, that if the same Callback is installed */
   /* more than once, then the callbacks will be called serially.       */
   /* Because of this, the processing in this function should be as     */
   /* efficient as possible.  It should also be noted that this function*/
   /* is called in the Thread Context of a Thread that the User does NOT*/
   /* own.  Therefore, processing in this function should be as         */
   /* efficient as possible (this argument holds anyway because another */
   /* GATT Event (Server/Client or Connection) will not be processed    */
   /* while this function call is outstanding).                         */
   /* * NOTE * This function MUST NOT Block and wait for Events that can*/
   /*          only be satisfied by Receiving a Bluetooth Event         */
   /*          Callback.  A Deadlock WILL occur because NO Bluetooth    */
   /*          Callbacks will be issued while this function is currently*/
   /*          outstanding.                                             */
static void BTPSAPI GATT_ClientEventCallback_DIS(unsigned int BluetoothStackID, GATT_Client_Event_Data_t *GATT_Client_Event_Data, unsigned long CallbackParameter)
{
   int           ret_val;
   Word_t        Handle;
   BoardStr_t    BoardStr;
   Boolean_t     ShowPrompt = FALSE;
   DeviceInfo_t *DeviceInfo = (DeviceInfo_t *)CallbackParameter;

   /* Verify that all parameters to this callback are Semi-Valid.       */
   if((BluetoothStackID) && (GATT_Client_Event_Data) && (DeviceInfo))
   {
      /* Determine the event that occurred.                             */
      switch(GATT_Client_Event_Data->Event_Data_Type)
      {
         case etGATT_Client_Error_Response:
            if(GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data)
            {
               printf("\r\nError Response.\r\n");
               BD_ADDRToStr(GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->RemoteDevice, BoardStr);
               printf("Connection ID:   %u.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ConnectionID);
               printf("Transaction ID:  %u.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->TransactionID);
               printf("Connection Type: %s.\r\n", (GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ConnectionType == gctLE)?"LE":"BR/EDR");
               printf("BD_ADDR:         %s.\r\n", BoardStr);
               printf("Error Type:      %s.\r\n", (GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorType == retErrorResponse)?"Response Error":"Response Timeout");

               /* Only print out the rest if it is valid.               */
               if(GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorType == retErrorResponse)
               {
                  printf("Request Opcode:  0x%02X.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->RequestOpCode);
                  printf("Request Handle:  0x%04X.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->RequestHandle);
                  printf("Error Code:      0x%02X.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorCode);

                  if(GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorCode < NUMBER_GATT_ERROR_CODES)
                  {
                     printf("Error Mesg:      %s.\r\n", ErrorCodeStr[GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorCode]);
                  }
                  else
                  {
                     printf("Error Mesg:      Unknown.\r\n");
                  }
               }
            }
            else
               printf("\nError - Null Error Response Data.\r\n");
            ShowPrompt = TRUE;
            break;
         case etGATT_Client_Read_Response:
            if(GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data)
            {
               /* Copy the result data to a local buffer and terminate  */
               /* the data with a NULL.                                 */
               if(GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength)
               {
                  BTPS_MemCopy(DisBuf, GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValue, GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength);
                  DisBuf[GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength] = 0;
               }
               else
                  DisBuf[0] = 0;
                     
               switch(DeviceInfo->DISHandleIndex)
               {
                  case DIS_MAN_NAME_HANDLE_OFFSET:
                     printf("Manufacturer Name: %s\n", (DisBuf[0])?DisBuf:"NULL");
                     break;
                  case DIS_MODEL_NUM_HANDLE_OFFSET:
                     printf("     Model Number: %s\n", (DisBuf[0])?DisBuf:"NULL");
                     break;
                  case DIS_SERIAL_NUM_HANDLE_OFFSET:
                     printf("    Serial Number: %s\n", (DisBuf[0])?DisBuf:"NULL");
                     break;
                  case DIS_HARDWARE_REV_HANDLE_OFFSET:
                     printf("Hardware Revision: %s\n", (DisBuf[0])?DisBuf:"NULL");
                     break;
                  case DIS_FIRMWARE_REV_HANDLE_OFFSET:
                     printf("Firmware Revision: %s\n", (DisBuf[0])?DisBuf:"NULL");
                     break;
                  case DIS_SOFTWARE_REV_HANDLE_OFFSET:
                     printf("Software Revision: %s\n", (DisBuf[0])?DisBuf:"NULL");
                     break;
                  case DIS_SYSTEM_ID_HANDLE_OFFSET:
                     printf("System ID - ManID: 0x%02X%02X%02X%02X%02X\n", DisBuf[0], DisBuf[1], DisBuf[2], DisBuf[3], DisBuf[4]);
                     printf("              OUI: 0x%02X%02X%02X\n", DisBuf[5], DisBuf[6], DisBuf[7]);
                     break;
                  case DIS_IEEE_CERT_HANDLE_OFFSET:
                     printf("   IEEE Cert Data: %s\n", (DisBuf[0])?DisBuf:"NULL");
                     break;
                  case DIS_PNP_ID_HANDLE_OFFSET:
                     printf("  PnP ID - Source: %d\n", DisBuf[0]);
                     printf("              VID: 0x%04X\n", READ_UNALIGNED_WORD_LITTLE_ENDIAN(&DisBuf[1]));
                     printf("              PID: 0x%04X\n", READ_UNALIGNED_WORD_LITTLE_ENDIAN(&DisBuf[3]));
                     printf("          Version: 0x%04X\n", READ_UNALIGNED_WORD_LITTLE_ENDIAN(&DisBuf[5]));
                     break;
               }

               Handle = 0;
               while(!Handle)
               {
                  DeviceInfo->DISHandleIndex++;
                  if(DeviceInfo->DISHandleIndex <= DIS_PNP_ID_HANDLE_OFFSET)
                     Handle  = ((Word_t *)&DeviceInfo->DISClientInfo)[DeviceInfo->DISHandleIndex];
                  else
                     break;
               }
               if(!Handle)
               {
                  ShowPrompt                  = TRUE;
                  DeviceInfo->DISHandleIndex  = 0;
                  DeviceInfo->Flags          &= ~DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_IDLE;
                  printf("\nDIS Search Complete\n");
               }
               else
               {
                  ret_val = GATT_Read_Value_Request(BluetoothStackID, GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->ConnectionID, Handle, GATT_ClientEventCallback_DIS, (unsigned long)DeviceInfo);
                  if(ret_val < 0)
                  {
                     printf("\nError reading remote DIS information\n");
                     ShowPrompt                  = TRUE;
                     DeviceInfo->DISHandleIndex  = 0;
                     DeviceInfo->Flags          &= ~DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_IDLE;
                  }
               }
            }
            break;
         default:
            break;
      }

      /* Print the command line prompt.                                 */
      if(ShowPrompt)
         DisplayPrompt();
   }
   else
   {
      /* There was an error with one or more of the input parameters.   */
      printf("\r\n");

      printf("GATT Callback Data: Event_Data = NULL.\r\n");

      DisplayPrompt();
   }
}

   /* The following function is for an GATT Client Event Callback.  This*/
   /* function will be called whenever a GATT Response is received for a*/
   /* request that was made when this function was registered.  This    */
   /* function passes to the caller the GATT Client Event Data that     */
   /* occurred and the GATT Client Event Callback Parameter that was    */
   /* specified when this Callback was installed.  The caller is free to*/
   /* use the contents of the GATT Client Event Data ONLY in the context*/
   /* of this callback.  If the caller requires the Data for a longer   */
   /* period of time, then the callback function MUST copy the data into*/
   /* another Data Buffer.  This function is guaranteed NOT to be       */
   /* invoked more than once simultaneously for the specified installed */
   /* callback (i.e.  this function DOES NOT have be reentrant).  It    */
   /* Needs to be noted however, that if the same Callback is installed */
   /* more than once, then the callbacks will be called serially.       */
   /* Because of this, the processing in this function should be as     */
   /* efficient as possible.  It should also be noted that this function*/
   /* is called in the Thread Context of a Thread that the User does NOT*/
   /* own.  Therefore, processing in this function should be as         */
   /* efficient as possible (this argument holds anyway because another */
   /* GATT Event (Server/Client or Connection) will not be processed    */
   /* while this function call is outstanding).                         */
   /* * NOTE * This function MUST NOT Block and wait for Events that can*/
   /*          only be satisfied by Receiving a Bluetooth Event         */
   /*          Callback.  A Deadlock WILL occur because NO Bluetooth    */
   /*          Callbacks will be issued while this function is currently*/
   /*          outstanding.                                             */
static void BTPSAPI GATT_ClientEventCallback_GAPS(unsigned int BluetoothStackID, GATT_Client_Event_Data_t *GATT_Client_Event_Data, unsigned long CallbackParameter)
{
   char         *NameBuffer;
   Word_t        Appearance;
   BoardStr_t    BoardStr;
   DeviceInfo_t *DeviceInfo;

   /* Verify that all parameters to this callback are Semi-Valid.       */
   if((BluetoothStackID) && (GATT_Client_Event_Data))
   {
      /* Determine the event that occurred.                             */
      switch(GATT_Client_Event_Data->Event_Data_Type)
      {
         case etGATT_Client_Error_Response:
            if(GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data)
            {
               printf("\r\nError Response.\r\n");
               BD_ADDRToStr(GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->RemoteDevice, BoardStr);
               printf("Connection ID:   %u.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ConnectionID);
               printf("Transaction ID:  %u.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->TransactionID);
               printf("Connection Type: %s.\r\n", (GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ConnectionType == gctLE)?"LE":"BR/EDR");
               printf("BD_ADDR:         %s.\r\n", BoardStr);
               printf("Error Type:      %s.\r\n", (GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorType == retErrorResponse)?"Response Error":"Response Timeout");

               /* Only print out the rest if it is valid.               */
               if(GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorType == retErrorResponse)
               {
                  printf("Request Opcode:  0x%02X.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->RequestOpCode);
                  printf("Request Handle:  0x%04X.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->RequestHandle);
                  printf("Error Code:      0x%02X.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorCode);
                  printf("Error Mesg:      %s.\r\n", ErrorCodeStr[GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorCode]);
               }
            }
            else
               printf("Error - Null Error Response Data.\r\n");
            break;
         case etGATT_Client_Read_Response:
            if(GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data)
            {
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->RemoteDevice)) != NULL)
               {
                  if((Word_t)CallbackParameter == DeviceInfo->GAPSClientInfo.DeviceNameHandle)
                  {
                     /* Display the remote device name.                 */
                     if((NameBuffer = (char *)BTPS_AllocateMemory(GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength+1)) != NULL)
                     {
                        BTPS_MemInitialize(NameBuffer, 0, GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength+1);
                        BTPS_MemCopy(NameBuffer, GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValue, GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength);

                        printf("\r\nRemote Device Name: %s.\r\n", NameBuffer);

                        BTPS_FreeMemory(NameBuffer);
                     }
                  }
                  else
                  {
                     if((Word_t)CallbackParameter == DeviceInfo->GAPSClientInfo.DeviceAppearanceHandle)
                     {
                        if(GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength == GAP_DEVICE_APPEARENCE_VALUE_LENGTH)
                        {
                           Appearance = READ_UNALIGNED_WORD_LITTLE_ENDIAN(GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValue);
                           if(AppearanceToString(Appearance, &NameBuffer))
                              printf("\r\nRemote Device Appearance: %s(%u).\r\n", NameBuffer, Appearance);
                           else
                              printf("\r\nRemote Device Appearance: Unknown(%u).\r\n", Appearance);
                        }
                        else
                           printf("Invalid Remote Appearance Value Length %u.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength);
                     }
                  }
               }
            }
            else
               printf("\r\nError - Null Read Response Data.\r\n");
            break;
         default:
            break;
      }

      /* Print the command line prompt.                                 */
      DisplayPrompt();
   }
   else
   {
      /* There was an error with one or more of the input parameters.   */
      printf("\r\n");

      printf("GATT Callback Data: Event_Data = NULL.\r\n");

      DisplayPrompt();
   }
}

   /* The following function is for an GATT Connection Event Callback.  */
   /* This function is called for GATT Connection Events that occur on  */
   /* the specified Bluetooth Stack.  This function passes to the caller*/
   /* the GATT Connection Event Data that occurred and the GATT         */
   /* Connection Event Callback Parameter that was specified when this  */
   /* Callback was installed.  The caller is free to use the contents of*/
   /* the GATT Client Event Data ONLY in the context of this callback.  */
   /* If the caller requires the Data for a longer period of time, then */
   /* the callback function MUST copy the data into another Data Buffer.*/
   /* This function is guaranteed NOT to be invoked more than once      */
   /* simultaneously for the specified installed callback (i.e.  this   */
   /* function DOES NOT have be reentrant).  It Needs to be noted       */
   /* however, that if the same Callback is installed more than once,   */
   /* then the callbacks will be called serially.  Because of this, the */
   /* processing in this function should be as efficient as possible.   */
   /* It should also be noted that this function is called in the Thread*/
   /* Context of a Thread that the User does NOT own.  Therefore,       */
   /* processing in this function should be as efficient as possible    */
   /* (this argument holds anyway because another GATT Event            */
   /* (Server/Client or Connection) will not be processed while this    */
   /* function call is outstanding).                                    */
   /* * NOTE * This function MUST NOT Block and wait for Events that can*/
   /*          only be satisfied by Receiving a Bluetooth Event         */
   /*          Callback.  A Deadlock WILL occur because NO Bluetooth    */
   /*          Callbacks will be issued while this function is currently*/
   /*          outstanding.                                             */
static void BTPSAPI GATT_Connection_Event_Callback(unsigned int BluetoothStackID, GATT_Connection_Event_Data_t *GATT_Connection_Event_Data, unsigned long CallbackParameter)
{
   Word_t        Credits;
   Word_t        MTU;
   Boolean_t     SuppressResponse = FALSE;
   BoardStr_t    BoardStr;
   DeviceInfo_t *DeviceInfo;

   /* Verify that all parameters to this callback are Semi-Valid.       */
   if((BluetoothStackID) && (GATT_Connection_Event_Data))
   {
      /* Determine the Connection Event that occurred.                  */
      switch(GATT_Connection_Event_Data->Event_Data_Type)
      {
         case etGATT_Connection_Device_Connection:
            if(GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data)
            {
               /* Save the Connection ID for later use.                 */
               ConnectionID = GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data->ConnectionID;

               printf("\r\netGATT_Connection_Device_Connection with size %u:\r\n", GATT_Connection_Event_Data->Event_Data_Size);
               BD_ADDRToStr(GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data->RemoteDevice, BoardStr);
               printf("   Connection ID:   %u.\r\n", GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data->ConnectionID);
               printf("   Connection Type: %s.\r\n", ((GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data->ConnectionType == gctLE)?"LE":"BR/EDR"));
               printf("   Remote Device:   %s.\r\n", BoardStr);
               printf("   GATT MTU:        %u.\r\n", GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data->MTU);

               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data->RemoteDevice)) != NULL)
               {

                  /* Clear the SPPLE Role Flag.                         */
                  DeviceInfo->Flags &= ~DEVICE_INFO_FLAGS_SPPLE_SERVER;

                  /* Initialize the Transmit and Receive Buffers.       */
                  InitializeBuffer(&(DeviceInfo->ReceiveBuffer));
                  InitializeBuffer(&(DeviceInfo->TransmitBuffer));

                  if(!LocalDeviceIsMaster)
                  {
                     /* Flag that we will act as the Server.            */
                     DeviceInfo->Flags |= DEVICE_INFO_FLAGS_SPPLE_SERVER;

                     /* Send the Initial Credits if the Rx Credit CCD is*/
                     /* already configured (for a bonded device this    */
                     /* could be the case).                             */
                     SendCredits(DeviceInfo, DeviceInfo->ReceiveBuffer.BytesFree);
                  }
                  else
                  {
                     /* Attempt to update the MTU to the maximum        */
                     /* supported.                                      */
                     if(!GATT_Query_Maximum_Supported_MTU(BluetoothStackID, &MTU))
                     {
                        GATT_Exchange_MTU_Request(BluetoothStackID, ConnectionID, MTU, GATT_ClientEventCallback_SPPLE, 0);
                     }
                  }
               }
            }
            else
               printf("Error - Null Connection Data.\r\n");
            break;
         case etGATT_Connection_Device_Disconnection:
            if(GATT_Connection_Event_Data->Event_Data.GATT_Device_Disconnection_Data)
            {
               /* Clear the Connection ID.                              */
               ConnectionID = 0;

               printf("\r\netGATT_Connection_Device_Disconnection with size %u: \r\n", GATT_Connection_Event_Data->Event_Data_Size);
               BD_ADDRToStr(GATT_Connection_Event_Data->Event_Data.GATT_Device_Disconnection_Data->RemoteDevice, BoardStr);
               printf("   Connection ID:   %u.\r\n", GATT_Connection_Event_Data->Event_Data.GATT_Device_Disconnection_Data->ConnectionID);
               printf("   Connection Type: %s.\r\n", ((GATT_Connection_Event_Data->Event_Data.GATT_Device_Disconnection_Data->ConnectionType == gctLE)?"LE":"BR/EDR"));
               printf("   Remote Device:   %s.\r\n", BoardStr);
            }
            else
               printf("Error - Null Disconnection Data.\r\n");
            break;
         case etGATT_Connection_Device_Connection_MTU_Update:
            if(GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_MTU_Update_Data)
            {
               printf("\r\netGATT_Connection_Device_Connection_MTU_Update.\r\n");
               BD_ADDRToStr(GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_MTU_Update_Data->RemoteDevice, BoardStr);
               printf("Connection ID:   %u.\r\n", GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_MTU_Update_Data->ConnectionID);
               printf("Connection Type: %s.\r\n", (GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_MTU_Update_Data->ConnectionType == gctLE) ? "LE" : "BR/EDR");
               printf("BD_ADDR:         %s.\r\n", BoardStr);
               printf("GATT MTU:        %u.\r\n", GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_MTU_Update_Data->MTU);
            }
            break;
         case etGATT_Connection_Server_Notification:
            if(GATT_Connection_Event_Data->Event_Data.GATT_Server_Notification_Data)
            {
               /* Find the Device Info for the device that has sent us  */
               /* the notification.                                     */
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, GATT_Connection_Event_Data->Event_Data.GATT_Server_Notification_Data->RemoteDevice)) != NULL)
               {
                  /* Determine the characteristic that is being         */
                  /* notified.                                          */
                  if(GATT_Connection_Event_Data->Event_Data.GATT_Server_Notification_Data->AttributeHandle == DeviceInfo->ClientInfo.Rx_Credit_Characteristic)
                  {
                     /* Verify that the length of the Rx Credit         */
                     /* Notification is correct.                        */
                     if(GATT_Connection_Event_Data->Event_Data.GATT_Server_Notification_Data->AttributeValueLength == WORD_SIZE)
                     {
                        Credits = READ_UNALIGNED_WORD_LITTLE_ENDIAN(GATT_Connection_Event_Data->Event_Data.GATT_Server_Notification_Data->AttributeValue);

                        /* Handle the received credits event.           */
                        ReceiveCreditEvent(DeviceInfo, Credits);

                        /* Suppress the command prompt.                 */
                        SuppressResponse   = TRUE;
                     }
                  }
                  else
                  {
                     if(GATT_Connection_Event_Data->Event_Data.GATT_Server_Notification_Data->AttributeHandle == DeviceInfo->ClientInfo.Tx_Characteristic)
                     {
                        /* This is a Tx Characteristic Event.  So call  */
                        /* the function to handle the data indication   */
                        /* event.                                       */
                        DataIndicationEvent(DeviceInfo, GATT_Connection_Event_Data->Event_Data.GATT_Server_Notification_Data->AttributeValueLength, GATT_Connection_Event_Data->Event_Data.GATT_Server_Notification_Data->AttributeValue);

                        /* If we are not looping back or doing automatic*/
                        /* reads we will display the prompt.            */
                        if((!AutomaticReadActive) && (!LoopbackActive))
                           SuppressResponse = FALSE;
                        else
                           SuppressResponse = TRUE;
                     }
                  }
               }
            }
            else
               printf("Error - Null Server Notification Data.\r\n");
            break;
         default:
            break;
      }

      /* Print the command line prompt.                                 */
      if(!SuppressResponse)
         DisplayPrompt();
   }
   else
   {
      /* There was an error with one or more of the input parameters.   */
      printf("\r\n");

      printf("GATT Connection Callback Data: Event_Data = NULL.\r\n");

      DisplayPrompt();
   }
}

   /* The following function is for an GATT Discovery Event Callback.   */
   /* This function will be called whenever a GATT Service is discovered*/
   /* or a previously started service discovery process is completed.   */
   /* This function passes to the caller the GATT Discovery Event Data  */
   /* that occurred and the GATT Client Event Callback Parameter that   */
   /* was specified when this Callback was installed.  The caller is    */
   /* free to use the contents of the GATT Discovery Event Data ONLY in */
   /* the context of this callback.  If the caller requires the Data for*/
   /* a longer period of time, then the callback function MUST copy the */
   /* data into another Data Buffer.  This function is guaranteed NOT to*/
   /* be invoked more than once simultaneously for the specified        */
   /* installed callback (i.e.  this function DOES NOT have be          */
   /* reentrant).  It Needs to be noted however, that if the same       */
   /* Callback is installed more than once, then the callbacks will be  */
   /* called serially.  Because of this, the processing in this function*/
   /* should be as efficient as possible.  It should also be noted that */
   /* this function is called in the Thread Context of a Thread that the*/
   /* User does NOT own.  Therefore, processing in this function should */
   /* be as efficient as possible (this argument holds anyway because   */
   /* another GATT Discovery Event will not be processed while this     */
   /* function call is outstanding).                                    */
   /* * NOTE * This function MUST NOT Block and wait for Events that can*/
   /*          only be satisfied by Receiving a Bluetooth Event         */
   /*          Callback.  A Deadlock WILL occur because NO Bluetooth    */
   /*          Callbacks will be issued while this function is currently*/
   /*          outstanding.                                             */
static void BTPSAPI GATT_Service_Discovery_Event_Callback(unsigned int BluetoothStackID, GATT_Service_Discovery_Event_Data_t *GATT_Service_Discovery_Event_Data, unsigned long CallbackParameter)
{
   int           ret_val;
   Boolean_t     ShowPrompt = TRUE;
   DeviceInfo_t *DeviceInfo;

   if((BluetoothStackID) && (GATT_Service_Discovery_Event_Data))
   {
      if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
      {
         switch(GATT_Service_Discovery_Event_Data->Event_Data_Type)
         {
            case etGATT_Service_Discovery_Indication:
               /* Verify the event data.                                */
               if(GATT_Service_Discovery_Event_Data->Event_Data.GATT_Service_Discovery_Indication_Data)
               {
                  printf("\r\n");
                  printf("Service 0x%04X - 0x%04X, UUID: ", GATT_Service_Discovery_Event_Data->Event_Data.GATT_Service_Discovery_Indication_Data->ServiceInformation.Service_Handle, GATT_Service_Discovery_Event_Data->Event_Data.GATT_Service_Discovery_Indication_Data->ServiceInformation.End_Group_Handle);
                  DisplayUUID(&(GATT_Service_Discovery_Event_Data->Event_Data.GATT_Service_Discovery_Indication_Data->ServiceInformation.UUID));
                  printf("\r\n");

                  /* Attempt to populate the handles for the DIS        */
                  /* Service.                                           */
                  DISPopulateHandles(&(DeviceInfo->DISClientInfo), GATT_Service_Discovery_Event_Data->Event_Data.GATT_Service_Discovery_Indication_Data);

                  /* Attempt to populate the handles for the GAP        */
                  /* Service.                                           */
                  GAPSPopulateHandles(&(DeviceInfo->GAPSClientInfo), GATT_Service_Discovery_Event_Data->Event_Data.GATT_Service_Discovery_Indication_Data);

                  /* Attempt to populate the handles for the SPPLE      */
                  /* Service.                                           */
                  SPPLEPopulateHandles(&(DeviceInfo->ClientInfo), GATT_Service_Discovery_Event_Data->Event_Data.GATT_Service_Discovery_Indication_Data);
               }
               break;
            case etGATT_Service_Discovery_Complete:
               /* Verify the event data.                                */
               if(GATT_Service_Discovery_Event_Data->Event_Data.GATT_Service_Discovery_Complete_Data)
               {
                  printf("\r\n");
                  printf("Service Discovery Operation Complete, Status 0x%02X.\r\n", GATT_Service_Discovery_Event_Data->Event_Data.GATT_Service_Discovery_Complete_Data->Status);

                  /* Flag that no service discovery operation is     */
                  /* outstanding for this device.                    */
                  if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_DIS)
                  {
                     /* Start to Query the Handles discovered.       */
                     printf("\nQuery Remote PnP Information\n\n");
                     ShowPrompt = FALSE;
                     
                     /* Dispatch a Callback to query the handles.    */
                     ret_val = GATT_Read_Value_Request(BluetoothStackID, GATT_Service_Discovery_Event_Data->Event_Data.GATT_Service_Discovery_Complete_Data->ConnectionID, DeviceInfo->DISClientInfo.ManufacturerNameHandle, GATT_ClientEventCallback_DIS, (unsigned long)DeviceInfo);
                     if(ret_val < 0)
                     {
                        printf("Error Reading DIS Attribute\n");
                        DeviceInfo->DISHandleIndex  = 0;
                        DeviceInfo->Flags          &= ~DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_IDLE;
                     }
                  }
                  if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_GAPS)
                  {
                     DeviceInfo->Flags &= ~DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_IDLE;
                  }
                  if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_SPPLE)
                  {
                     DeviceInfo->Flags &= ~DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_IDLE;
                     if(SPPLE_CLIENT_INFORMATION_VALID(DeviceInfo->ClientInfo))
                        printf("\r\nValid SPPLE Service Found.\r\n");
                     else
                        printf("\r\nNo SPPLE Service Found.\r\n");
                  }
               }
               break;
         }

         if(ShowPrompt)
            DisplayPrompt();
      }
   }
   else
   {
      /* There was an error with one or more of the input parameters.   */
      printf("\r\n");

      printf("GATT Callback Data: Event_Data = NULL.\r\n");

      DisplayPrompt();
   }
}

   /* Main Program Entry Point.                                         */
int main(int argc, char* argv[])
{
   char                    *endptr = NULL;
   int                      Result   = 0;
   unsigned int             CommPortNumber;
   unsigned int             BaudRate;
   HCI_COMM_Protocol_t      Protocol = cpUART_RTS_CTS;
   HCI_DriverInformation_t  HCI_DriverInformation;
   HCI_DriverInformation_t *HCI_DriverInformationPtr;

   /* Initialize some defaults.                                         */
   LoopbackActive      = FALSE;
   DisplayRawData      = FALSE;
   AutomaticReadActive = FALSE;

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
         case BCSP_PARAMETER_VALUE:
            Protocol = cpBCSP;
            /* Let case fall through.                                   */
         case UART_PARAMETER_VALUE:
            /* The Transport selected was UART, check to see if the     */
            /* number of parameters entered on the command line is      */
            /* correct.                                                 */
            if(argc == NUM_EXPECTED_PARAMETERS_UART)
            {
               /* The correct number of parameters were entered, convert*/
               /* the command line parameters to the proper format.     */
               BaudRate       = strtol(argv[3], &endptr, 10);
               if((argv[2][0] >= '0') && (argv[2][0] <= '9'))
               {
                  CommPortNumber = strtol(argv[2], &endptr, 10);
                  HCI_DRIVER_SET_COMM_INFORMATION(&HCI_DriverInformation, CommPortNumber, BaudRate, Protocol);
               }
               else
               {
                  HCI_DRIVER_SET_EXTENDED_COMM_INFORMATION_DEVICE_NAME(&HCI_DriverInformation, -1, BaudRate, Protocol, 0, argv[2]);
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
                     /* Start the User Interface.                       */
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
         printf("Parameter Error (Expecting = [USB = 0, UART = 1, BCSP = 2 Flag] [IF !USB [Comm Port or Device Name] [Baud Rate]])\r\n");
      }
   }
   else
   {
      /* An invalid number of parameters was entered on the Command     */
      /* Line.                                                          */
      printf("Parameter Error (Expecting = [USB = 0, UART = 1, BCSP = 2 Flag] [IF !USB [Comm Port or Device Name] [Baud Rate]])\r\n");
   }

   return 0;
}
