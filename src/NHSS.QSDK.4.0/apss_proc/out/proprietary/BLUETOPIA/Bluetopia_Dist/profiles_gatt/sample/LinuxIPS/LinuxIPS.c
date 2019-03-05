/*
* Copyright (c) 2016 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
/*****< linuxips.c >***********************************************************/
/*      Copyright 2015 - 2016 Qualcomm Technologies, Inc.                     */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  LINUXIPS - Linux Bluetooth Indoor Positioning Service using GATT          */
/*             (LE/BREDR) sample application.                                 */
/*                                                                            */
/*  Author:  Ryan McCord                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/03/15  R. McCord      Initial creation.                               */
/******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "LinuxIPS.h"            /* Application Header.                       */
#include "SS1BTPS.h"             /* Main SS1 BT Stack Header.                 */
#include "SS1BTDBG.h"            /* BTPS Debug Header.                        */
#include "SS1BTGAT.h"            /* Main SS1 GATT Header.                     */
#include "SS1BTGAP.h"            /* Main SS1 GAP Service Header.              */
#include "SS1BTDIS.h"            /* Main SS1 DIS Service Header.              */
#include "SS1BTIPS.h"            /* Main SS1 IPS Service Header.              */
#include "math.h"                /* Math header included for encoding.        */

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

#define MAX_SUPPORTED_COMMANDS                     (60)  /* Denotes the       */
                                                         /* maximum number of */
                                                         /* User Commands that*/
                                                         /* are supported by  */
                                                         /* this application. */

#define MAX_COMMAND_LENGTH                         (128) /* Denotes the max   */
                                                         /* buffer size used  */
                                                         /* for user commands */
                                                         /* input via the     */
                                                         /* User Interface.   */

#define MAX_INQUIRY_RESULTS                        (32)  /* Denotes the max   */
                                                         /* number of inquiry */
                                                         /* results.          */

#define MAX_NUM_OF_PARAMETERS                       (5)  /* Denotes the max   */
                                                         /* number of         */
                                                         /* parameters a      */
                                                         /* command can have. */

#define DEFAULT_IO_CAPABILITY       (licNoInputNoOutput) /* Denotes the       */
                                                         /* default I/O       */
                                                         /* Capability that is*/
                                                         /* used with Pairing.*/

#define DEFAULT_MITM_PROTECTION                 (TRUE)   /* Denotes the       */
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

#define PRINT_SERVICE_DISCOVERY_HANDLES            (FALSE)/* Enables printing  */
                                                          /* of the UUID's     */
                                                          /* during service    */
                                                          /* discovery. May    */
                                                          /* be enabled to aid */
                                                          /* with debugging and*/
                                                          /* PTS testing.      */

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
                                                         /* create a IPS     */
                                                         /* Server.           */

#define EXIT_MODE                                  (-10) /* Flags exit from   */
                                                         /* any Mode.         */

#define UNABLE_TO_ALLOCATE_MEMORY                  (-11) /* Denotes that we   */
                                                         /* failed because we */
                                                         /* couldn't allocate */
                                                         /* memory.           */

#define IPS_MAXIMUM_NUMBER_OF_CHARACTERISTICS      (9)   /* The number of    */
                                                         /* characteristics  */
                                                         /* supported by the */
                                                         /* service.         */

   /* The following constants represent the default log file names that */
   /* are used if no Log file name is specified when enabling debug.    */
#define DEFAULT_DEBUG_LOG_FILE_NAME                "LinuxIPS_ASC.log"
#define DEFAULT_DEBUG_FTS_FILE_NAME                "LinuxIPS_FTS.log"

   /* Determine the Name we will use for this compilation.              */
#define LE_DEMO_DEVICE_NAME                        "LinuxIPS"

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

   /* The following enumerated type definition defines the different    */
   /* types of service discovery that can be performed.                 */
typedef enum
{
   sdDIS,
   sdGAPS,
   sdIPS,
} Service_Discovery_Type_t;

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

   /* The following enumeration defines the possible DIS attribute      */
   /* handle types that may be read on a remote DIS Server.             */
typedef enum _tagDIS_Characteristic_Type_t
{
   dctManufacturerName,
   dctModelNumber,
   dctSerialNumber,
   dctHardwareRevision,
   dctFirmwareRevision,
   dctSoftwareRevision,
   dctSystemID,
   dctIEEE11073Cert,
   dctPnPID
} DIS_Characteristic_Type_t;

   /* The following structure holds information on known Device         */
   /* Information Service attribute handles.  The RequestType contains  */
   /* the type of DIS attribute handle that we are reading and MUST be  */
   /* set before a read request is made.  A buffer is also contained to */
   /* hold the read string.                                             */
typedef struct _tagDIS_Client_Info_t
{
   Word_t                     ManufacturerNameHandle;
   Word_t                     ModelNumberHandle;
   Word_t                     SerialNumberHandle;
   Word_t                     HardwareRevisionHandle;
   Word_t                     FirmwareRevisionHandle;
   Word_t                     SoftwareRevisionHandle;
   Word_t                     SystemIDHandle;
   Word_t                     IEEE11073CertHandle;
   Word_t                     PnPIDHandle;

   DIS_Characteristic_Type_t  RequestType;
   char                       Buffer[DIS_MAXIMUM_SUPPORTED_STRING+1];
} DIS_Client_Info_t;

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

   /* The following structure for a Master is used to hold a list of    */
   /* information on all paired devices. For slave we will not use this */
   /* structure.                                                        */
typedef struct _tagDeviceInfo_t
{
   Word_t                     Flags;
   Byte_t                     EncryptionKeySize;
   GAP_LE_Address_Type_t      ConnectionAddressType;
   BD_ADDR_t                  ConnectionBD_ADDR;
   Link_Key_t                 LinkKey;
   Long_Term_Key_t            LTK;
   Random_Number_t            Rand;
   Word_t                     EDIV;
   DIS_Client_Info_t          DISClientInfo;
   GAPS_Client_Info_t         GAPSClientInfo;
   IPS_Client_Information_t   IPSClientInfo;
   char                       LocationName[BTPS_CONFIGURATION_IPS_MAXIMUM_SUPPORTED_STRING_LENGTH+1];
   unsigned int               LocationNameOffset;
   unsigned int               LocationNameLength;
   struct _tagDeviceInfo_t   *NextDeviceInfoPtr;
} DeviceInfo_t;

#define DEVICE_INFO_DATA_SIZE  (sizeof(DeviceInfo_t))

   /* Defines the bitmask flags that may be set in the DeviceInfo_t     */
   /* structure.                                                        */
#define DEVICE_INFO_FLAGS_LTK_VALID                         0x0001
#define DEVICE_INFO_FLAGS_LINK_KEY_VALID                    0x0002
#define DEVICE_INFO_FLAGS_LINK_ENCRYPTED                    0x0004
#define DEVICE_INFO_FLAGS_BR_EDR_CONNECTED                  0x0008
#define DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_OUTSTANDING     0x0010
#define DEVICE_INFO_FLAGS_DIS_SERVICE_DISCOVERY_COMPLETE    0x0020
#define DEVICE_INFO_FLAGS_GAPS_SERVICE_DISCOVERY_COMPLETE   0x0040
#define DEVICE_INFO_FLAGS_IPS_SERVICE_DISCOVERY_COMPLETE    0x0080
#define DEVICE_INFO_FLAGS_RELIABLE_WRITE_MODE_ENABLED       0x0100

   /* User to represent a structure to hold a BD_ADDR return from       */
   /* BD_ADDRToStr.                                                     */
typedef char BoardStr_t[16];

                        /* The Encryption Root Key should be generated  */
                        /* in such a way as to guarantee 128 bits of    */
                        /* entropy.                                     */
static BTPSCONST Encryption_Key_t ER = {0x28, 0xBA, 0xE1, 0x37, 0x13, 0xB2, 0x20, 0x45, 0x16, 0xB2, 0x19, 0xD0, 0x80, 0xEE, 0x4A, 0x51};

                        /* The Identity Root Key should be generated    */
                        /* in such a way as to guarantee 128 bits of    */
                        /* entropy.                                     */
static BTPSCONST Encryption_Key_t IR = {0x41, 0x09, 0xA0, 0x88, 0x09, 0x6B, 0x70, 0xC0, 0x95, 0x23, 0x3C, 0x8C, 0x48, 0xFC, 0xC9, 0xFE};

                        /* The following keys can be regenerated on the */
                        /* fly using the constant IR and ER keys and    */
                        /* are used globally, for all devices.          */
static Encryption_Key_t DHK;
static Encryption_Key_t IRK;

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the         */
   /* compiler as part of standard C/C++).                              */
static unsigned int        IPSInstanceID;
                                                    /* The following holds the IPS     */
                                                    /* Instance IDs that are returned  */
                                                    /* from IPS_Initialize_XXX().      */

static unsigned int        GAPSInstanceID;          /* Holds the Instance ID for the   */
                                                    /* GAP Service.                    */

static unsigned int        DISInstanceID;           /* Holds the Instance ID for the   */
                                                    /* DIS Service.                    */

static GAPLE_Parameters_t  LE_Parameters;           /* Holds GAP Parameters like       */
                                                    /* Discoverability, Connectability */
                                                    /* Modes.                          */

static DeviceInfo_t       *DeviceInfoList;          /* Holds the list head for the     */
                                                    /* device info list.               */

static unsigned int        BluetoothStackID;        /* Variable which holds the Handle */
                                                    /* of the opened Bluetooth Protocol*/
                                                    /* Stack.                          */

static BD_ADDR_t           ConnectionBD_ADDR;       /* Holds the BD_ADDR of the        */
                                                    /* currently connected device.     */

static BD_ADDR_t           CurrentRemoteBD_ADDR;    /* Variable which holds the        */
                                                    /* current BD_ADDR of the device   */
                                                    /* which is currently pairing or   */
                                                    /* authenticating.                 */

static unsigned int        ConnectionID;            /* Holds the Connection ID of the  */
                                                    /* currently connected device.     */

static Boolean_t           LocalDeviceIsMaster;     /* Boolean that tells if the local */
                                                    /* device is the master of the     */
                                                    /* current connection.             */

static BD_ADDR_t           InquiryResultList[MAX_INQUIRY_RESULTS];
                                                    /* Variable which                  */
                                                    /* contains the inquiry result     */
                                                    /* received from the most recently */
                                                    /* preformed inquiry.              */

static unsigned int        NumberofValidResponses;  /* Variable which holds the number */
                                                    /* of valid inquiry results within */
                                                    /* the inquiry results array.      */

static Link_Key_t          CreatedLinkKey;          /* Variable which holds the created*/
                                                    /* Link Keys from pairing to be    */
                                                    /* assigned to the device          */
                                                    /* information.                    */

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

static CommandTable_t      CommandTable[MAX_SUPPORTED_COMMANDS];
                                                    /* Variable which is               */
                                                    /* used to hold the actual Commands*/
                                                    /* that are supported by this      */
                                                    /* application.                    */

static unsigned int        DebugID;                 /* Variable which holds the ID of  */
                                                    /* the currently enabled debugging */
                                                    /* session.                        */

static Boolean_t           ScanInProgress;          /* A boolean flag to show if a scan*/
                                                    /* is in process                   */

static Word_t              ReadMultipleRequestHandles[9];
                                                    /* Array to hold all the handles   */
                                                    /* for a read multiple request.    */

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

#define NUMBER_OF_ERROR_CODES     (sizeof(ErrorCodeStr)/sizeof(char *))

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
static void DisplayRawAdvertisingData(GAP_LE_Advertising_Data_Entry_t *Entry);
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

static int StartScan(unsigned int BluetoothStackID);
static int StopScan(unsigned int BluetoothStackID);

static int ConnectLEDevice(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR, GAP_LE_Address_Type_t AddressType, Boolean_t UseWhiteList);
static int DisconnectLEDevice(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR);

static void ConfigureCapabilities(GAP_LE_Extended_Pairing_Capabilities_t *Capabilities);
static int SendPairingRequest(BD_ADDR_t BD_ADDR, Boolean_t ConnectionMaster);
static int SlavePairingRequestResponse(BD_ADDR_t BD_ADDR);
static int EncryptionInformationRequestResponse(BD_ADDR_t BD_ADDR, Byte_t KeySize, GAP_LE_Authentication_Response_Information_t *GAP_LE_Authentication_Response_Information);

static int DisplayHelp(ParameterList_t *TempParam);
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

static int Connect(ParameterList_t *TempParam);
static int Disconnect(ParameterList_t *TempParam);

static int PairLE(ParameterList_t *TempParam);

static int EnableDebug(ParameterList_t *TempParam);
static int GetGATTMTU(ParameterList_t *TempParam);
static int SetGATTMTU(ParameterList_t *TempParam);

   /* Device Information Service Functions and Commands.                */
static int DiscoverDIS(ParameterList_t *TempParam);
static void DISPopulateHandles(DIS_Client_Info_t *ClientInfo, GATT_Service_Discovery_Indication_Data_t *ServiceInfo);
static int ReadDISCharacteristic(ParameterList_t *TempParam);

   /* Generic Access Profile Service Functions and Commands.            */
static int DiscoverGAPS(ParameterList_t *TempParam);
static void GAPSPopulateHandles(GAPS_Client_Info_t *ClientInfo, GATT_Service_Discovery_Indication_Data_t *ServiceInfo);
static int ReadLocalName(ParameterList_t *TempParam);
static int SetLocalName(ParameterList_t *TempParam);
static int ReadRemoteName(ParameterList_t *TempParam);
static int ReadLocalAppearance(ParameterList_t *TempParam);
static int SetLocalAppearance(ParameterList_t *TempParam);
static int ReadRemoteAppearance(ParameterList_t *TempParam);

static void DumpAppearanceMappings(void);
static Boolean_t AppearanceToString(Word_t Appearance, char **String);
static Boolean_t AppearanceIndexToAppearance(unsigned int Index, Word_t *Appearance);

   /* Indoor Positioning Service Function Commands                      */
static int RegisterIPS(ParameterList_t *TempParam);
static int UnregisterIPS(ParameterList_t *TempParam);
static void InitializeCharacteristicData(void);
static int DiscoverIPS(ParameterList_t *TempParam);
static void IPSPopulateHandles(DeviceInfo_t *DeviceInfo, GATT_Service_Discovery_Indication_Data_t *ServiceDiscoveryData);
static void StoreDescriptorHandles(GATT_Characteristic_Information_t *CharacteristicInfoPtr, Word_t *AttributeHandle);

static int ReadIndoorPositioningConfiguration(ParameterList_t *TempParam);
static int WriteIndoorPositioningConfiguration(ParameterList_t *TempParam);
static int ReadLatitude(ParameterList_t *TempParam);
static int WriteLatitude(ParameterList_t *TempParam);
static int ReadLongitude(ParameterList_t *TempParam);
static int WriteLongitude(ParameterList_t *TempParam);
static int ReadLocalNorthCoordinate(ParameterList_t *TempParam);
static int WriteLocalNorthCoordinate(ParameterList_t *TempParam);
static int ReadLocalEastCoordinate(ParameterList_t *TempParam);
static int WriteLocalEastCoordinate(ParameterList_t *TempParam);
static int ReadFloorNumber(ParameterList_t *TempParam);
static int WriteFloorNumber(ParameterList_t *TempParam);
static int ReadAltitude(ParameterList_t *TempParam);
static int WriteAltitude(ParameterList_t *TempParam);
static int ReadUncertainty(ParameterList_t *TempParam);
static int WriteUncertainty(ParameterList_t *TempParam);
static int ReadLocationName(ParameterList_t *TempParam);
static int WriteLocationName(ParameterList_t *TempParam);
static int ReadMultipleCharacteristics(ParameterList_t *TempParam);
static int ExecuteWriteRequest(ParameterList_t *TempParam);
static int DecodeDisplayIPCResponse(Byte_t *Value, Word_t ValueLength);
static int DecodeDisplayLatitudeResponse(Byte_t *Value, Word_t ValueLength);
static int DecodeDisplayLongitudeResponse(Byte_t *Value, Word_t ValueLength);
static int DecodeDisplayLocalNorthCoordinateResponse(Byte_t *Value, Word_t ValueLength);
static int DecodeDisplayLocalEastCoordinateResponse(Byte_t *Value, Word_t ValueLength);
static int DecodeDisplayFloorNumberResponse(Byte_t *Value, Word_t ValueLength);
static int DecodeDisplayAltitudeResponse(Byte_t *Value, Word_t ValueLength);
static int DecodeDisplayUncertaintyResponse(Byte_t *Value, Word_t ValueLength);
static int DecodeDisplayIPSAdvertisingData(GAP_LE_Advertising_Data_Entry_t *Data_Entry);
static void EncodeLatitude(SDWord_t *Latitude);
static void DecodeLatitude(SDWord_t *Latitude);
static void EncodeLongitude(SDWord_t *Longitude);
static void DecodeLongitude(SDWord_t *Longitude);
static void DisplayUncertaintyCharacteristic(Byte_t Uncertainty);
static void DisplayIPSCharacteristicType(IPS_Characteristic_Type_t Type);
static void DisplayUpdatedCharacteristics(Word_t Flags);
static void DisplayWriteIPCUsage(void);
static void DisplayLatitudeUsage(void);
static void DisplayLongitudeUsage(void);
static void DisplayLocalNorthCoordinateUsage(void);
static void DisplayLocalEastCoordinateUsage(void);
static void DisplayFloorNumberUsage(void);
static void DisplayAltitudeUsage(void);
static void DisplayUncertaintyUsage(void);
static void DisplayLocationNameUsage(void);

   /* BTPS Callback function prototypes.                                */
static void BTPSAPI GAP_Event_Callback(unsigned int BluetoothStackID, GAP_Event_Data_t *GAP_Event_Data, unsigned long CallbackParameter);
static void BTPSAPI GAP_LE_Event_Callback(unsigned int BluetoothStackID, GAP_LE_Event_Data_t *GAP_LE_Event_Data, unsigned long CallbackParameter);
static void BTPSAPI IPS_EventCallback(unsigned int BluetoothStackID, IPS_Event_Data_t *IPS_Event_Data, unsigned long CallbackParameter);
static void BTPSAPI GATT_ClientEventCallback_IPS(unsigned int BluetoothStackID, GATT_Client_Event_Data_t *GATT_Client_Event_Data, unsigned long CallbackParameter);
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
         BTPS_MemCopy(&(DeviceInfoPtr->ConnectionBD_ADDR), &ConnectionBD_ADDR, sizeof(ConnectionBD_ADDR));

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
   return(BSC_SearchGenericListEntry(ekBD_ADDR_t, (void *)(&BD_ADDR), BTPS_STRUCTURE_OFFSET(DeviceInfo_t, ConnectionBD_ADDR), BTPS_STRUCTURE_OFFSET(DeviceInfo_t, NextDeviceInfoPtr), (void **)(ListHead)));
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
   AddCommand("SETDISCOVERABILITYMODE", SetDiscoverabilityMode);
   AddCommand("SETCONNECTABILITYMODE", SetConnectabilityMode);
   AddCommand("SETPAIRABILITYMODE", SetPairabilityMode);
   AddCommand("CHANGEPAIRINGPARAMETERS", ChangePairingParameters);
   AddCommand("GETLOCALADDRESS", GetLocalAddress);
   AddCommand("ADVERTISELE", AdvertiseLE);
   AddCommand("STARTSCANNING", StartScanning);
   AddCommand("STOPSCANNING", StopScanning);
   AddCommand("CONNECT", Connect);
   AddCommand("DISCONNECT", Disconnect);
   AddCommand("PAIRLE", PairLE);
   AddCommand("LEPASSKEYRESPONSE", LEPassKeyResponse);
   AddCommand("QUERYENCRYPTIONMODE", LEQueryEncryption);
   AddCommand("SETPASSKEY", LESetPasskey);
   AddCommand("DISCOVERDIS", DiscoverDIS);
   AddCommand("READDIS", ReadDISCharacteristic);
   AddCommand("DISCOVERGAPS", DiscoverGAPS);
   AddCommand("GETLOCALNAME", ReadLocalName);
   AddCommand("SETLOCALNAME", SetLocalName);
   AddCommand("GETREMOTENAME", ReadRemoteName);
   AddCommand("GETLOCALAPPEARANCE", ReadLocalAppearance);
   AddCommand("SETLOCALAPPEARANCE", SetLocalAppearance);
   AddCommand("GETREMOTEAPPEARANCE", ReadRemoteAppearance);
   AddCommand("ENABLEDEBUG", EnableDebug);
   AddCommand("REGISTERIPS", RegisterIPS);
   AddCommand("UNREGISTERIPS", UnregisterIPS);
   AddCommand("DISCOVERIPS", DiscoverIPS);
   AddCommand("READIPC", ReadIndoorPositioningConfiguration);
   AddCommand("WRITEIPC", WriteIndoorPositioningConfiguration);
   AddCommand("READLATITUDE", ReadLatitude);
   AddCommand("WRITELATITUDE", WriteLatitude);
   AddCommand("READLONGITUDE", ReadLongitude);
   AddCommand("WRITELONGITUDE", WriteLongitude);
   AddCommand("READLOCALNORTH", ReadLocalNorthCoordinate);
   AddCommand("WRITELOCALNORTH", WriteLocalNorthCoordinate);
   AddCommand("READLOCALEAST", ReadLocalEastCoordinate);
   AddCommand("WRITELOCALEAST", WriteLocalEastCoordinate);
   AddCommand("READFLOORNUMBER", ReadFloorNumber);
   AddCommand("WRITEFLOORNUMBER", WriteFloorNumber);
   AddCommand("READALTITUDE", ReadAltitude);
   AddCommand("WRITEALTITUDE", WriteAltitude);
   AddCommand("READUNCERTAINTY", ReadUncertainty);
   AddCommand("WRITEUNCERTAINTY", WriteUncertainty);
   AddCommand("READLOCATIONNAME", ReadLocationName);
   AddCommand("WRITELOCATIONNAME", WriteLocationName);
   AddCommand("READMULTIPLE", ReadMultipleCharacteristics);
   AddCommand("EXECUTEWRITE",  ExecuteWriteRequest);
   AddCommand("GETMTU", GetGATTMTU);
   AddCommand("SETMTU", SetGATTMTU);
   AddCommand("HELP", DisplayHelp);

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
   /* strings to their unsigned integer equivalent.  This function can  */
   /* handle leading and tailing white space, however it does not handle*/
   /* signed or comma delimited values.  This function takes as its     */
   /* input the string which is to be converted.  The function returns  */
   /* zero if an error occurs otherwise it returns the value parsed from*/
   /* the string passed as the input parameter.                         */
static unsigned int StringToUnsignedInteger(char *StringInteger)
{
   int          IsHex;
   unsigned int Index;
   Boolean_t    SignedNumber = FALSE;
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
         /* Check if this is a signed number.                           */
         if(StringInteger[0] == '-')
         {
            SignedNumber = TRUE;
         }

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
                  if(SignedNumber)
                  {
                     /* Go ahead and make the number signed.            */
                     ret_val *= -1;
                  }

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
   /* programmatic add Commands the Global (to this module) Command     */
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
   int                              Result;
   unsigned int                     Index;
   GAP_LE_Advertising_Data_Entry_t *Advertising_Data_Entry;
   Byte_t                           Flags;
   GATT_UUID_t                      UUID;

      /* Verify that the input parameters seem semi-valid.              */
   if(Advertising_Data)
   {
      /* Loop and display all the advertising data entries.             */
      for(Index = 0; Index < Advertising_Data->Number_Data_Entries; Index++)
      {
         /* Store a pointer to the advertising data entry.              */
         Advertising_Data_Entry = &(Advertising_Data->Data_Entries[Index]);

         /* Print the default information.                              */
         printf("\r\nAdvertising data entry:\r\n");

         /* We will print default advertising report data types that    */
         /* this sample uses.  Other types will be displayed in their   */
         /* raw format.                                                 */
         switch(Advertising_Data->Data_Entries[Index].AD_Type)
         {
            case HCI_LE_ADVERTISING_REPORT_DATA_TYPE_FLAGS:
               printf("   AD Type:   Flags.\r\n");
               printf("   AD Length: 0x%02X.\r\n", (unsigned int)(Advertising_Data_Entry->AD_Data_Length));
               DisplayRawAdvertisingData(&(Advertising_Data->Data_Entries[Index]));

               /* Make sure the flags are present to display.           */
               if(Advertising_Data_Entry->AD_Data_Length >= NON_ALIGNED_BYTE_SIZE)
               {
                  /* Store the Flags.                                   */
                  Flags = READ_UNALIGNED_BYTE_LITTLE_ENDIAN(Advertising_Data_Entry->AD_Data_Buffer);

                  printf("\r\n   Flags:\r\n");
                  if(Flags == 0)
                  {
                     printf("      No Flags\r\n");
                     break;
                  }

                  if(Flags & HCI_LE_ADVERTISING_FLAGS_LIMITED_DISCOVERABLE_MODE_FLAGS_BIT_MASK)
                  {
                     printf("      Limited Discoverable Mode\r\n");
                  }

                  if(Flags & HCI_LE_ADVERTISING_FLAGS_GENERAL_DISCOVERABLE_MODE_FLAGS_BIT_MASK)
                  {
                     printf("      General Discoverable Mode\r\n");
                  }

                  if(Flags & HCI_LE_ADVERTISING_FLAGS_BR_EDR_NOT_SUPPORTED_FLAGS_BIT_MASK)
                  {
                     printf("      BR/EDR Not Supported\r\n");
                  }

                  if(Flags & HCI_LE_ADVERTISING_FLAGS_SIMULTANEOUS_LE_BR_EDR_TO_SAME_DEVICE_CONTROLLER_BIT_MASK)
                  {
                     printf("      Simultaneous LE-BR/EDR To Same Device Controller\r\n");
                  }

                  if(Flags & HCI_LE_ADVERTISING_FLAGS_SIMULTANEOUS_LE_BR_EDR_TO_SAME_DEVICE_HOST_BIT_MASK)
                  {
                     printf("      Simultaneous LE-BR/EDR To Same Host\r\n");
                  }
               }
               break;
            case HCI_LE_ADVERTISING_REPORT_DATA_TYPE_16_BIT_SERVICE_UUID_COMPLETE:
               printf("   AD Type:   16 Bit Service UUID Complete.\r\n");
               printf("   AD Length: 0x%02X.\r\n", (unsigned int)(Advertising_Data_Entry->AD_Data_Length));
               DisplayRawAdvertisingData(Advertising_Data_Entry);

               /* We will check for the IPS Service data.  Make sure the*/
               /* mandatory UUID is present.                            */
               if(Advertising_Data_Entry->AD_Data_Length >= NON_ALIGNED_WORD_SIZE)
               {
                  /* Store the type.                                    */
                  UUID.UUID_Type = guUUID_16;

                  /* Check if this is the IPS Service data.             */
                  if(IPS_COMPARE_IPS_SERVICE_UUID_TO_UUID_16(*((UUID_16_t *)&(Advertising_Data_Entry->AD_Data_Buffer[0]))))
                  {
                     /* Decode the UUID.                                */
                     ASSIGN_SDP_UUID_16(UUID.UUID.UUID_16, Advertising_Data_Entry->AD_Data_Buffer[0], Advertising_Data_Entry->AD_Data_Buffer[1]);

                     /* Display the UUID.                               */
                     printf("   UUID:      0x");
                     DisplayUUID(&UUID);
                  }
               }
               break;
            case HCI_LE_ADVERTISING_REPORT_DATA_TYPE_LOCAL_NAME_SHORTENED:
               printf("   AD Type:   Local Name Shortened.\r\n");
               printf("   AD Length: 0x%02X.\r\n", (unsigned int)(Advertising_Data_Entry->AD_Data_Length));
               DisplayRawAdvertisingData(Advertising_Data_Entry);

               if(Advertising_Data_Entry->AD_Data_Length >= NON_ALIGNED_BYTE_SIZE)
               {
                  printf("   Shortened Device Name: %s", Advertising_Data_Entry->AD_Data_Buffer);
               }
               break;
            case HCI_LE_ADVERTISING_REPORT_DATA_TYPE_LOCAL_NAME_COMPLETE:
               printf("   AD Type:   Local Name Complete.\r\n");
               printf("   AD Length: 0x%02X.\r\n", (unsigned int)(Advertising_Data_Entry->AD_Data_Length));
               DisplayRawAdvertisingData(Advertising_Data_Entry);

               if(Advertising_Data_Entry->AD_Data_Length >= NON_ALIGNED_BYTE_SIZE)
               {
                  printf("   Complete Device Name: %s\r\n", Advertising_Data_Entry->AD_Data_Buffer);
               }
               break;
            case IPS_ADVERTISING_REPORT_DATA_TYPE:
               printf("   AD Type:   IPS Advertising Report Data.\r\n");
               printf("   AD Length: 0x%02X.\r\n", (unsigned int)(Advertising_Data_Entry->AD_Data_Length));
               DisplayRawAdvertisingData(Advertising_Data_Entry);

               /* Decode and display the data.                          */
               if((Result = DecodeDisplayIPSAdvertisingData(Advertising_Data_Entry)) != 0)
                  DisplayFunctionError("DecodeDisplayIPSAdvertisingData", Result);
               break;
            default:
               printf("   AD Type:   0x%02X.\r\n", (unsigned int)(Advertising_Data_Entry->AD_Type));
               printf("   AD Length: 0x%02X.\r\n", (unsigned int)(Advertising_Data_Entry->AD_Data_Length));

               /* Simply call the internal function to display the raw  */
               /* data.                                                 */
               DisplayRawAdvertisingData(&(Advertising_Data->Data_Entries[Index]));
               break;
         }
      }
   }
}

   /* The following function is a helper function to display the raw    */
   /* data for an advertising data entry.                               */
static void DisplayRawAdvertisingData(GAP_LE_Advertising_Data_Entry_t *Entry)
{
   unsigned int Index;

   if(Entry)
   {
      if((Entry->AD_Data_Length) && (Entry->AD_Data_Buffer))
      {
         printf("   AD Data:   ");
         for(Index = 0; Index < Entry->AD_Data_Length; Index++)
         {
            printf("0x%02X ", Entry->AD_Data_Buffer[Index]);
         }
         printf("\r\n");
      }
   }
}

   /* The following function displays the pairing capabilities that is  */
   /* passed into this function.                                        */
static void DisplayPairingInformation(GAP_LE_Extended_Pairing_Capabilities_t *Extended_Pairing_Capabilities)
{
      /* Display the IO Capability.                                     */
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

   printf("\r\n");
}

   /* Displays the correct prompt depending on the Server/Client Mode.  */
static void DisplayPrompt(void)
{
   printf("\r\nLinuxIPS>");

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

   /* The following function is responsible for opening the SS1         */
   /* Bluetooth Protocol Stack.  This function accepts a pre-populated  */
   /* HCI Driver Information structure that contains the HCI Driver     */
   /* Transport Information.  This function returns zero on successful  */
   /* execution and a negative value on all errors.                     */
static int OpenStack(HCI_DriverInformation_t *HCI_DriverInformation)
{
   int           Result;
   int           ret_val = 0;
   char          BluetoothAddress[16];
   BD_ADDR_t     BD_ADDR;
   unsigned int  ServiceID;
   HCI_Version_t HCIVersion;

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

            /* Flag that no connection is currently active.             */
            ASSIGN_BD_ADDR(ConnectionBD_ADDR, 0, 0, 0, 0, 0, 0);
            ASSIGN_BD_ADDR(CurrentRemoteBD_ADDR, 0, 0, 0, 0, 0, 0);
            LocalDeviceIsMaster = FALSE;

            /* Regenerate IRK and DHK from the constant Identity Root   */
            /* Key.                                                     */
            GAP_LE_Diversify_Function(BluetoothStackID, (Encryption_Key_t *)(&IR), 1,0, &IRK);
            GAP_LE_Diversify_Function(BluetoothStackID, (Encryption_Key_t *)(&IR), 3, 0, &DHK);

            /* Flag that we have no Key Information in the Key List.    */
            DeviceInfoList = NULL;

            /* Initialize the GATT Service.                             */
            if(!(Result = GATT_Initialize(BluetoothStackID, (GATT_INITIALIZATION_FLAGS_SUPPORT_LE | GATT_INITIALIZATION_FLAGS_SUPPORT_BR_EDR), GATT_Connection_Event_Callback, 0)))
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

                  /* Initialize the DIS Service.                        */
                  Result = DIS_Initialize_Service(BluetoothStackID, &ServiceID);
                  if(Result > 0)
                  {
                     /* Save the Instance ID of the GAP Service.        */
                     DISInstanceID = (unsigned int)Result;

                     /* Set the discoverable attributes                 */
                     DIS_Set_Manufacturer_Name(BluetoothStackID, DISInstanceID, BTPS_VERSION_COMPANY_NAME_STRING);
                     DIS_Set_Model_Number(BluetoothStackID, DISInstanceID, BTPS_VERSION_VERSION_STRING);
                     DIS_Set_Serial_Number(BluetoothStackID, DISInstanceID, BTPS_VERSION_VERSION_STRING);

                     /* Return success to the caller.                   */
                     ret_val        = 0;
                  }
               }
               else
               {
                  /* The Stack was NOT initialized successfully, inform */
                  /* the user and set the return value of the           */
                  /* initialization function to an error.               */
                  DisplayFunctionError("GAPS_Initialize_Service", Result);

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
      /* Cleanup GAP Service Module.                                    */
      if(GAPSInstanceID)
      {
         GAPS_Cleanup_Service(BluetoothStackID, GAPSInstanceID);

         GAPSInstanceID = 0;
      }

      /* Cleanup IPS Service.                                           */
      if(IPSInstanceID)
      {
         IPS_Cleanup_Service(BluetoothStackID, IPSInstanceID);
      }

      /* Cleanup DIS Service Module.                                    */
      if(DISInstanceID)
      {
         DIS_Cleanup_Service(BluetoothStackID, DISInstanceID);

         DISInstanceID = 0;
      }

      /* Cleanup GATT Module.                                           */
      GATT_Cleanup(BluetoothStackID);

      /* Simply close the Stack                                         */
      BSC_Shutdown(BluetoothStackID);

      /* Free BTPSKRNL allocated memory.                                */
      BTPS_DeInit();

      printf("Stack Shutdown.\r\n");

      /* Free the device information List.                              */
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

      /* A semi-valid Bluetooth Stack ID exists, now attempt to set the */
      /* attached Devices Discoverablity Mode to General.               */
      ret_val = GAP_Set_Discoverability_Mode(BluetoothStackID, dmGeneralDiscoverableMode, 0);
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

      /* A semi-valid Bluetooth Stack ID exists, now attempt to set the */
      /* attached Devices Connectablity Mode to Connectable.            */
      ret_val = GAP_Set_Connectability_Mode(BluetoothStackID, cmConnectableMode);
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
      Result = GAP_LE_Set_Pairability_Mode(BluetoothStackID, lpmPairableMode);

      /* Next, check the return value of the GAP Set Pairability mode   */
      /* command for successful execution.                              */
      if(!Result)
      {
         /* The device has been set to pairable mode, now register an   */
         /* Authentication Callback to handle the Authentication events */
         /* if required.                                                */
         Result = GAP_LE_Register_Remote_Authentication(BluetoothStackID, GAP_LE_Event_Callback, (unsigned long)0);

         /* A semi-valid Bluetooth Stack ID exists, now attempt to set  */
         /* the attached Devices Connectablity Mode to Connectable.     */
         if(!(ret_val = GAP_Set_Pairability_Mode(BluetoothStackID, pmPairableMode_EnableSecureSimplePairing)))
         {
            /* The device has been set to pairable mode, now register an*/
            /* Authentication Callback to handle the Authentication     */
            /* events if required.                                      */
            if((ret_val = GAP_Register_Remote_Authentication(BluetoothStackID, GAP_Event_Callback, (unsigned long)0)))
               DisplayFunctionError("GAP_Register_Remote_Authentication", ret_val);
         }
         else
            DisplayFunctionError("GAP_Set_Pairability_Mode", ret_val);

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

   /* The following function is responsible for starting a scan.        */
static int StartScan(unsigned int BluetoothStackID)
{
   int Result;

   /* First, determine if the input parameters appear to be semi-valid. */
   if(BluetoothStackID)
   {
      /* Not currently scanning, go ahead and attempt to perform the    */
      /* scan.                                                          */
      Result = GAP_LE_Perform_Scan(BluetoothStackID, stActive, 10, 10, latPublic, fpNoFilter, TRUE, GAP_LE_Event_Callback, 0);

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
static int ConnectLEDevice(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR, GAP_LE_Address_Type_t AddressType, Boolean_t UseWhiteList)
{
   int                            Result;
   unsigned int                   WhiteListChanged;
   GAP_LE_White_List_Entry_t      WhiteListEntry;
   GAP_LE_Connection_Parameters_t ConnectionParameters;

   /* First, determine if the input parameters appear to be semi-valid. */
   if((BluetoothStackID) && (!COMPARE_NULL_BD_ADDR(BD_ADDR)))
   {
      if(COMPARE_NULL_BD_ADDR(ConnectionBD_ADDR))
      {
         /* Remove any previous entries for this device from the White  */
         /* List.                                                       */
         WhiteListEntry.Address_Type = AddressType;
         WhiteListEntry.Address      = BD_ADDR;

         GAP_LE_Remove_Device_From_White_List(BluetoothStackID, 1, &WhiteListEntry, &WhiteListChanged);

         if(UseWhiteList)
            Result = GAP_LE_Add_Device_To_White_List(BluetoothStackID, 1, &WhiteListEntry, &WhiteListChanged);
         else
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
            Result = GAP_LE_Create_Connection(BluetoothStackID, 100, 100, Result ? fpNoFilter : fpWhiteList, AddressType, Result ? &BD_ADDR : NULL, latPublic, &ConnectionParameters, GAP_LE_Event_Callback, 0);

            if(!Result)
            {
               printf("Connection Request successful.\r\n");

               /* Note the connection information.                      */
               ConnectionBD_ADDR = BD_ADDR;
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
      Capabilities->Receiving_Keys.Identification_Key = FALSE;
      Capabilities->Receiving_Keys.Signing_Key        = FALSE;
      Capabilities->Receiving_Keys.Link_Key           = FALSE;

      Capabilities->Sending_Keys.Encryption_Key       = TRUE;
      Capabilities->Sending_Keys.Identification_Key   = FALSE;
      Capabilities->Sending_Keys.Signing_Key          = FALSE;
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
               /* Since Secure Connections isn't supported go ahead and */
               /* disable our request for Secure Connections and        */
               /* re-submit our request.                                */
               printf("Secure Connections not supported, disabling Secure Connections.\r\n");

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
static int SlavePairingRequestResponse(BD_ADDR_t BD_ADDR)
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
      AuthenticationResponseData.Authentication_Data_Length = GAP_LE_EXTENDED_PAIRING_CAPABILITIES_SIZE;

      /* Configure the Application Pairing Parameters.                  */
      ConfigureCapabilities(&(AuthenticationResponseData.Authentication_Data.Extended_Pairing_Capabilities));

      /* Attempt to pair to the remote device.                          */
      if((ret_val = GAP_LE_Authentication_Response(BluetoothStackID, BD_ADDR, &AuthenticationResponseData)) == BTPS_ERROR_SECURE_CONNECTIONS_NOT_SUPPORTED)
      {
         /* Since Secure Connections isn't supported go ahead and       */
         /* disable our request for Secure Connections and re-submit our*/
         /* request.                                                    */
         printf("Secure Connections not supported, disabling Secure Connections.\r\n");

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
               DisplayFunctionError("GAP_LE_Authentication_Response", ret_val);
            }
         }
         else
         {
            DisplayFunctionError("GAP_LE_Generate_Long_Term_Key", ret_val);
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
   printf("* IPS Sample Application                                         *\r\n");
   printf("******************************************************************\r\n");
   printf("* Command Options General: Help, Quit, GetLocalAddress,          *\r\n");
   printf("*                          EnableDebug, GetMTU, SetMTU           *\r\n");
   printf("* Command Options GAPLE:   SetDiscoverabilityMode,               *\r\n");
   printf("*                          SetConnectabilityMode,                *\r\n");
   printf("*                          SetPairabilityMode,                   *\r\n");
   printf("*                          ChangePairingParameters,              *\r\n");
   printf("*                          AdvertiseLE, StartScanning,           *\r\n");
   printf("*                          StopScanning, Connect,                *\r\n");
   printf("*                          Disconnect, PairLE,                   *\r\n");
   printf("*                          LEPasskeyResponse,                    *\r\n");
   printf("*                          QueryEncryptionMode, SetPasskey,      *\r\n");
   printf("* Command Options DIS:     DiscoverDIS, ReadDIS,                 *\r\n");
   printf("* Command Options GAPS:    DiscoverGAPS, GetLocalName,           *\r\n");
   printf("*                          SetLocalName, GetRemoteName,          *\r\n");
   printf("*                          SetLocalAppearance,                   *\r\n");
   printf("*                          GetLocalAppearance,                   *\r\n");
   printf("*                          GetRemoteAppearance,                  *\r\n");
   printf("* Command Options IPS:                                           *\r\n");
   printf("*                Server:   RegisterIPS,                          *\r\n");
   printf("*                          UnregisterIPS,                        *\r\n");
   printf("*                Client:   DiscoverIPS,                          *\r\n");
   printf("*                          ReadMultiple, ExecuteWrite,           *\r\n");
   printf("*                Both:     ReadIPC, WriteIPC,                    *\r\n");
   printf("*                          ReadLatitude, WriteLatitude,          *\r\n");
   printf("*                          ReadLongitude, WriteLongitude,        *\r\n");
   printf("*                          ReadLocalNorth, WriteLocalNorth,      *\r\n");
   printf("*                          ReadLocalEast, WriteLocalEast,        *\r\n");
   printf("*                          ReadFloorNumber, WriteFloorNumber,    *\r\n");
   printf("*                          ReadAltitude, WriteAltitude,          *\r\n");
   printf("*                          ReadUncertainty, WriteUncertainty,    *\r\n");
   printf("*                          ReadLocationName, WriteLocationName   *\r\n");
   printf("******************************************************************\r\n");

   return(0);
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
      if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].intParam >= 0) && (TempParam->Params[0].intParam <= 1))
      {
         /* Parameters appear to be valid, map the specified parameters */
         /* into the API specific parameters.                           */
         PairabilityMode = lpmNonPairableMode;
         Mode            = "lpmNonPairableMode";

         if(TempParam->Params[0].intParam == 1)
         {
            PairabilityMode = lpmPairableMode;
            Mode            = "lpmPairableMode";
         }

         /* Parameters mapped, now set the Pairability Mode.            */
         Result = GAP_LE_Set_Pairability_Mode(BluetoothStackID, PairabilityMode);

         /* Next, check the return value to see if the command was      */
         /* issued successfully.                                        */
         if(Result >= 0)
         {
            /* The Mode was changed successfully.                       */
            printf("Pairability Mode Changed to %s.\r\n", Mode);

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
         DisplayUsage("SetPairabilityMode [Mode (0 = Non Pairable, 1 = Pairable]");

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
               printf("Passkey Response Success.");

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
   union
   {
      Advertising_Data_t               AdvertisingData;
      Scan_Response_Data_t             ScanResponseData;
   } Advertisement_Data_Buffer;
   Byte_t                              BroadcastData[IPS_MAXIMUM_BROADCAST_DATA_SIZE];
   Byte_t                              BroadcastDataLength = IPS_MAXIMUM_BROADCAST_DATA_SIZE;
   unsigned int                        AdvertisingLength;

   /* First, check that valid Bluetooth Stack ID exists.  And that we   */
   /* are not already connected.                                        */
   /* * NOTE * The check for the ConnectionID has been stripped from the*/
   /*          sample application.  This is present in other samples,   */
   /*          however IPS needs to be able to advertise non-connectable*/
   /*          advertisements after being connected.                    */
   if(BluetoothStackID)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].intParam >= 0) && (TempParam->Params[0].intParam <= 1))
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

            if(IPSInstanceID)
            {
               /* Advertise the Indoor Positioning Service(1 byte type  */
               /* and 2 bytes UUID)                                     */
               Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[3] = 3;
               Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[4] = HCI_LE_ADVERTISING_REPORT_DATA_TYPE_16_BIT_SERVICE_UUID_COMPLETE;
               IPS_ASSIGN_IPS_SERVICE_UUID_16(&(Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[5]));

               /* Format the IPS broadcast data.                        */
               /* * NOTE * The BroadcastDataLength will contain the size*/
               /*          of the Broadcast data on output.             */
               if((ret_val = IPS_Format_Broadcasting_Data(BluetoothStackID, IPSInstanceID, &BroadcastDataLength, BroadcastData)) == 0)
               {
                  /* Simply copy the formatted broadcasting data into   */
                  /* the advertising data.                              */
                  /* * NOTE * The index starts at 7 to account for the  */
                  /*          IPS UUID inserted at indexes 5 and 6.     */
                  BTPS_MemCopy((Advertisement_Data_Buffer.AdvertisingData.Advertising_Data + 7), BroadcastData, BroadcastDataLength);
               }
               else
               {
                  DisplayFunctionError("IPS_Format_Broadcasting_Data",  ret_val);
               }
            }

            /* Set the advertising data length.                         */
            /* * NOTE * +2 for the UUID size.                           */
            AdvertisingLength  = Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[0];
            AdvertisingLength += Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[3];
            AdvertisingLength += 2;
            AdvertisingLength += BroadcastDataLength;

            /* Write the advertising data to the chip.                  */
            ret_val = GAP_LE_Set_Advertising_Data(BluetoothStackID, AdvertisingLength, &(Advertisement_Data_Buffer.AdvertisingData));
            if(!ret_val)
            {
               BTPS_MemInitialize(&(Advertisement_Data_Buffer.ScanResponseData), 0, sizeof(Scan_Response_Data_t));

               /* Set the Scan Response Data.                           */
               Length = BTPS_StringLength(LE_DEMO_DEVICE_NAME);
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
               BTPS_MemCopy(&(Advertisement_Data_Buffer.ScanResponseData.Scan_Response_Data[2]),LE_DEMO_DEVICE_NAME,Length);

               ret_val = GAP_LE_Set_Scan_Response_Data(BluetoothStackID, (Advertisement_Data_Buffer.ScanResponseData.Scan_Response_Data[0] + 1), &(Advertisement_Data_Buffer.ScanResponseData));
               if(!ret_val)
               {
                  /* Set up the advertising parameters.                 */
                  AdvertisingParameters.Advertising_Channel_Map   = HCI_LE_ADVERTISING_CHANNEL_MAP_DEFAULT;
                  AdvertisingParameters.Scan_Request_Filter       = fpNoFilter;
                  AdvertisingParameters.Connect_Request_Filter    = fpNoFilter;
                  AdvertisingParameters.Advertising_Interval_Min  = 100;
                  AdvertisingParameters.Advertising_Interval_Max  = 200;

                  /* Configure the Connectability Parameters.           */
                  if(!ConnectionID)
                  {
                     /* * NOTE * Since we do not ever put ourselves to  */
                     /*          be direct connectable then we will set */
                     /*          the DirectAddress to all 0s.           */
                     ConnectabilityParameters.Connectability_Mode  = LE_Parameters.ConnectableMode;
                     ConnectabilityParameters.Own_Address_Type     = latPublic;
                     ConnectabilityParameters.Direct_Address_Type  = latPublic;
                     ASSIGN_BD_ADDR(ConnectabilityParameters.Direct_Address, 0, 0, 0, 0, 0, 0);
                  }
                  else
                  {
                     /* * NOTE * Since we do not ever put ourselves to  */
                     /*          be direct connectable then we will set */
                     /*          the DirectAddress to all 0s.           */
                     ConnectabilityParameters.Connectability_Mode  = lcmNonConnectable;
                     ConnectabilityParameters.Own_Address_Type     = latPublic;
                     ConnectabilityParameters.Direct_Address_Type  = latPublic;
                     ASSIGN_BD_ADDR(ConnectabilityParameters.Direct_Address, 0, 0, 0, 0, 0, 0);
                  }

                  /* Now enable advertising.                            */
                  /* * NOTE * We will only enabled the scan response if */
                  /*          this is a connectable advertisement.      */
                  ret_val = GAP_LE_Advertising_Enable(BluetoothStackID, (!ConnectionID) ? TRUE : FALSE, &AdvertisingParameters, &ConnectabilityParameters, GAP_LE_Event_Callback, 0);
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
         DisplayUsage("AdvertiseLE [(0 = Disable, 1 = Enable)]");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      printf("\r\nInvalid BluetoothStackID.\r\n");
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for starting an LE scan     */
   /* procedure.  This function returns zero if successful and a        */
   /* negative value if an error occurred.                              */
static int StartScanning(ParameterList_t *TempParam)
{
   int ret_val;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      if(!ScanInProgress)
      {
         ScanInProgress = TRUE;

         /* Simply start scanning.                                      */
         if(!StartScan(BluetoothStackID))
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

   /* The following function is responsible for a creating a connection */
   /* to a specified remote device.  This function returns zero if      */
   /* successful or a negative integer indicating that an error occured.*/
static int Connect(ParameterList_t *TempParam)
{
   int       ret_val = FUNCTION_ERROR;
   BD_ADDR_t BD_ADDR;

   /* First, check that a valid Bluetooth Stack ID exists.              */
   if(BluetoothStackID)
   {
      /* Next, make sure that the parameters are valid.                 */
      if((TempParam) && (TempParam->NumberofParameters >= 1) && (TempParam->NumberofParameters < 3) && (TempParam->Params[0].strParam) && (BTPS_StringLength(TempParam->Params[0].strParam) == (sizeof(BD_ADDR_t)*2)))
      {
         /* Convert the parameter to a Bluetooth Device Address.        */
         StrToBD_ADDR(TempParam->Params[0].strParam, &BD_ADDR);

         /* If there are two parameters then the user has specified an  */
         /* address type and we will connect over LE.                   */
         if(TempParam->NumberofParameters == 2)
         {
            if(!ConnectLEDevice(BluetoothStackID, BD_ADDR, (GAP_LE_Address_Type_t)TempParam->Params[1].intParam, FALSE))
               ret_val = 0;
            else
               ret_val = FUNCTION_ERROR;
         }
         else
         {
            /* If there is only one parameter then this must be a BR/EDR*/
            /* connection.                                              */
            if(TempParam->NumberofParameters == 1)
            {
               /* Call GATT_Connect to connect BR/EDR over GATT.        */
               if((ret_val = GATT_Connect(BluetoothStackID, BD_ADDR, GATT_Connection_Event_Callback, 0)) > 0)
                  ret_val = 0;
               else
               {
                  DisplayFunctionError("GATT_Connect", ret_val);
                  ret_val = FUNCTION_ERROR;
               }
            }
            else
               printf("Usage: Connect [BD_ADDR] [Address Type(LE Only)](Optional).\r\n");
         }
      }
      else
      {
         /* Invalid parameters specified so flag an error to the user.  */
         printf("Usage: Connect [BR/EDR BD_ADDR].\r\n");
         printf("       Connect [LE BD_ADDR] [Local Address Type(0=PUBLIC, 1=RANDOM)].\r\n");

         /* Flag that an error occurred while submitting the command.   */
         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }

   return ret_val;
}

   /* The following function is responsible for disconnecting a remote  */
   /* device device.  This function returns zero if successful and a    */
   /* negative value if an error occurred.                              */
static int Disconnect(ParameterList_t *TempParam)
{
   int           ret_val = FUNCTION_ERROR;
   DeviceInfo_t *DeviceInfo;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Next, check to make sure we are currently connected.           */
      if(!COMPARE_NULL_BD_ADDR(ConnectionBD_ADDR))
      {
         /* Get the device information so we can determine what         */
         /* transport we are connected over.                            */
         if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
         {
            if(!(DeviceInfo->Flags & DEVICE_INFO_FLAGS_BR_EDR_CONNECTED))
            {
               /* Disconnect over LE.                                   */
               if(!DisconnectLEDevice(BluetoothStackID, ConnectionBD_ADDR))
                  ret_val = 0;
               else
                  ret_val = FUNCTION_ERROR;
            }
            else
            {
               /* Disconnect over BR/EDR.                               */
               if((ret_val = GATT_Disconnect(BluetoothStackID, ConnectionID)) == 0)
                  ret_val = 0;
               else
                  ret_val = FUNCTION_ERROR;
            }
         }
         else
            printf("Disconnect: No device info.\r\n");
      }
      else
         printf("Device is not connected.\r\n");
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

            ret_val = GATT_Start_Service_Discovery(BluetoothStackID, ConnectionID, 1, &UUID, GATT_Service_Discovery_Event_Callback, sdDIS);
            if(!ret_val)
            {
               printf("\r\nGATT_Service_Discovery_Start() success.\r\n");

               /* Flag that a Service Discovery Operation is            */
               /* outstanding.                                          */
               DeviceInfo->Flags |= (DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_OUTSTANDING);
            }
            else
            {
               /* An error occur so just clean-up.                      */
               printf("\r\nError - GATT_Service_Discovery_Start returned %d.\r\n", ret_val);

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            printf("\r\nService Discovery Operation Outstanding for Device.\r\n");

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         printf("\r\nNo Device Info.\r\n");

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      printf("\r\nNo Connection Established\r\n");

      ret_val = FUNCTION_ERROR;
   }

   return(ret_val);
}

   /* The following function is a utility function that provides a      */
   /* mechanism of populating a Client Information structure with the   */
   /* information discovered from a GATT Discovery operation.           */
static void DISPopulateHandles(DIS_Client_Info_t *ClientInfo, GATT_Service_Discovery_Indication_Data_t *ServiceInfo)
{
   unsigned int                       Index;
   GATT_Characteristic_Information_t *CurrentCharacteristic;

   /* Verify that the input parameters are semi-valid.                  */
   if((ClientInfo) && (ServiceInfo) && (ServiceInfo->ServiceInformation.UUID.UUID_Type == guUUID_16) && (DIS_COMPARE_DIS_SERVICE_UUID_TO_UUID_16(ServiceInfo->ServiceInformation.UUID.UUID.UUID_16)))
   {
      /* Loop through all characteristics discovered in the service     */
      /* and populate the correct entry.                                */
      CurrentCharacteristic = ServiceInfo->CharacteristicInformationList;
      if(CurrentCharacteristic)
      {
         for(Index=0; Index<ServiceInfo->NumberOfCharacteristics; Index++, CurrentCharacteristic++)
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

               /* Always print warnings about unknown DIS               */
               /* Characteristics .                                     */
               printf("\r\nWarning - Unknown DIS characteristic.\r\n");
               printf("\r\nCharacteristic Handle: 0x%04X\r\n", CurrentCharacteristic->Characteristic_Handle);
               printf("   Properties:         0x%02X\r\n", CurrentCharacteristic->Characteristic_Properties);
               printf("   UUID:               0x");
               DisplayUUID(&(CurrentCharacteristic->Characteristic_UUID));
            }
         }
      }
   }
}

   /* The following function is responsible for issuing a command to    */
   /* read a DIS characteristic value on a remote DIS Server.           */
static int ReadDISCharacteristic(ParameterList_t *TempParam)
{
   int                        ret_val = 0;
   DIS_Characteristic_Type_t  Type;
   DeviceInfo_t              *DeviceInfo;
   Word_t                     AttributeHandle = 0;

   if(BluetoothStackID)
   {
      /* Make sure the parameters are semi-valid.                       */
      if((TempParam) && (TempParam->NumberofParameters == 1) && ((TempParam->Params[0].intParam >= dctManufacturerName) && (TempParam->Params[0].intParam <= dctPnPID)))
      {
         /* Make sure we are connected.                                 */
         if(ConnectionID)
         {
            /* Store the parameter.                                     */
            Type = (DIS_Characteristic_Type_t)TempParam->Params[0].intParam;

            /* Get the device information.                              */
            if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
            {
               /* Make sure service discovery has been performed for    */
               /* DIS.                                                  */
               if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_DIS_SERVICE_DISCOVERY_COMPLETE)
               {
                  /* Set the handle depending on the attribute handle   */
                  /* type requested.                                    */
                  switch(Type)
                  {
                     case dctManufacturerName:
                        AttributeHandle = DeviceInfo->DISClientInfo.ManufacturerNameHandle;
                        break;
                     case dctModelNumber:
                        AttributeHandle = DeviceInfo->DISClientInfo.ModelNumberHandle;
                        break;
                     case dctSerialNumber:
                        AttributeHandle = DeviceInfo->DISClientInfo.SerialNumberHandle;
                        break;
                     case dctHardwareRevision:
                        AttributeHandle = DeviceInfo->DISClientInfo.HardwareRevisionHandle;
                        break;
                     case dctFirmwareRevision:
                        AttributeHandle = DeviceInfo->DISClientInfo.FirmwareRevisionHandle;
                        break;
                     case dctSoftwareRevision:
                        AttributeHandle = DeviceInfo->DISClientInfo.SoftwareRevisionHandle;
                        break;
                     case dctSystemID:
                        AttributeHandle = DeviceInfo->DISClientInfo.SystemIDHandle;
                        break;
                     case dctIEEE11073Cert:
                        AttributeHandle = DeviceInfo->DISClientInfo.IEEE11073CertHandle;
                        break;
                     case dctPnPID:
                        AttributeHandle = DeviceInfo->DISClientInfo.PnPIDHandle;
                        break;
                     default:
                        printf("\r\nWarning - Unknown DIS attribute handle type.\r\n");
                        break;
                  }

                  /* Make sure the handle is valid from service         */
                  /* discovery.                                         */
                  if(AttributeHandle)
                  {
                     /* Save the request type so we can handle the read */
                     /* response.                                       */
                     DeviceInfo->DISClientInfo.RequestType = Type;

                     /* Send the GATT read value request.               */
                     /* * NOTE * We will not save the transactionID     */
                     /*          returned by this function, which we    */
                     /*          could use to cancel the request.       */
                     if((ret_val = GATT_Read_Value_Request(BluetoothStackID, ConnectionID, AttributeHandle, GATT_ClientEventCallback_DIS, AttributeHandle)) > 0)
                     {
                        printf("\r\nGATT Read Value Request sent:\r\n");
                        printf("   TransactionID:     %d\r\n",     ret_val);
                        printf("   Attribute Handle:  0x%04X\r\n", AttributeHandle);
                     }
                     else
                        DisplayFunctionError("GATT_Read_Value_Request", ret_val);

                     /* Simply return success to the caller.            */
                     ret_val = 0;
                  }
                  else
                     printf("Invalid attribute handle.\r\n");
               }
               else
                  printf("\r\nService discovery has not been performed.\r\n");
            }
            else
               printf("\r\nNo device information.\r\n");
         }
      }
      else
      {
         printf("Usage: ReadDIS [Characteristic Type [0-8]]\r\n");
         printf("\r\n Where (Characteristic Type) is:\r\n");
         printf("  0  = Manufacturer Name\r\n");
         printf("  1  = Model Number\r\n");
         printf("  2  = Serial Number\r\n");
         printf("  3  = Hardware Revision\r\n");
         printf("  4  = Firmware Revision\r\n");
         printf("  5  = Software Revision\r\n");
         printf("  6  = SystemID\r\n");
         printf("  7  = IEEE 11073 Cert\r\n");
         printf("  8  = PNP ID\r\n");
      }
   }
   else
      printf("\r\nInvalid BluetoothStackID.\r\n");


   return (ret_val);
}

   /* The following function is responsible for performing a GAP Service*/
   /* Service Discovery Operation.  This function will return zero on   */
   /* successful execution and a negative value on errors.              */
static int DiscoverGAPS(ParameterList_t *TempParam)
{
   int                            ret_val;
   GATT_UUID_t                    UUID[1];
   DeviceInfo_t                  *DeviceInfo;
   GATT_Attribute_Handle_Group_t  DiscoveryHandleRange;

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
            /* Configure the filter so that only the GAP Service is     */
            /* discovered.                                              */
            UUID[0].UUID_Type = guUUID_16;
            GAP_ASSIGN_GAP_SERVICE_UUID_16(UUID[0].UUID.UUID_16);

            BTPS_MemInitialize(&DiscoveryHandleRange, 0, sizeof(DiscoveryHandleRange));

            /* Start the service discovery process.                     */
            if((TempParam->NumberofParameters >= 2) && (TempParam->Params[0].intParam) && (TempParam->Params[1].intParam) && (TempParam->Params[0].intParam <= TempParam->Params[1].intParam))
            {
               DiscoveryHandleRange.Starting_Handle = TempParam->Params[0].intParam;
               DiscoveryHandleRange.Ending_Handle   = TempParam->Params[1].intParam;

               ret_val = GATT_Start_Service_Discovery_Handle_Range(BluetoothStackID, ConnectionID, &DiscoveryHandleRange, (sizeof(UUID)/sizeof(GATT_UUID_t)), UUID, GATT_Service_Discovery_Event_Callback, sdGAPS);
            }
            else
               ret_val = GATT_Start_Service_Discovery(BluetoothStackID, ConnectionID, (sizeof(UUID)/sizeof(GATT_UUID_t)), UUID, GATT_Service_Discovery_Event_Callback, sdGAPS);

            if(!ret_val)
            {
               /* Display success message.                              */
               if(DiscoveryHandleRange.Starting_Handle == 0)
                  printf("GATT_Service_Discovery_Start() success.\r\n");
               else
                  printf("GATT_Start_Service_Discovery_Handle_Range() success.\r\n");

               /* Flag that a Service Discovery Operation is            */
               /* outstanding.                                          */
               DeviceInfo->Flags |= DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_OUTSTANDING;
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

   /* The following function is a utility function that provides a      */
   /* mechanism of populating discovered GAP Service Handles.           */
static void GAPSPopulateHandles(GAPS_Client_Info_t *ClientInfo, GATT_Service_Discovery_Indication_Data_t *ServiceInfo)
{
   unsigned int                       Index1;
   GATT_Characteristic_Information_t *CurrentCharacteristic;

   /* Verify that the input parameters are semi-valid.                  */
   if((ClientInfo) && (ServiceInfo) && (ServiceInfo->ServiceInformation.UUID.UUID_Type == guUUID_16) && (GAP_COMPARE_GAP_SERVICE_UUID_TO_UUID_16(ServiceInfo->ServiceInformation.UUID.UUID.UUID_16)))
   {
      /* Loop through all characteristics discovered in the service and */
      /* populate the correct entry.                                    */
      CurrentCharacteristic = ServiceInfo->CharacteristicInformationList;
      if(CurrentCharacteristic)
      {
         for(Index1 = 0; Index1 < ServiceInfo->NumberOfCharacteristics; Index1++, CurrentCharacteristic++)
         {
            /* All GAP Service UUIDs are defined to be 16 bit UUIDs.    */
            if(CurrentCharacteristic->Characteristic_UUID.UUID_Type == guUUID_16)
            {
               /* Determine which characteristic this is.               */
               if(GAP_COMPARE_GAP_DEVICE_NAME_UUID_TO_UUID_16(CurrentCharacteristic->Characteristic_UUID.UUID.UUID_16))
               {
                  ClientInfo->DeviceNameHandle = CurrentCharacteristic->Characteristic_Handle;
                  continue;
               }

               if(GAP_COMPARE_GAP_DEVICE_APPEARANCE_UUID_TO_UUID_16(CurrentCharacteristic->Characteristic_UUID.UUID.UUID_16))
               {
                  ClientInfo->DeviceAppearanceHandle = CurrentCharacteristic->Characteristic_Handle;
                  continue;
               }

               /* Always print warnings about unknown GAPS              */
               /* Characteristics .                                     */
               printf("\r\nWarning - Unknown GAPS characteristic.\r\n");
               printf("\r\nCharacteristic Handle: 0x%04X\r\n", CurrentCharacteristic->Characteristic_Handle);
               printf("   Properties:         0x%02X\r\n", CurrentCharacteristic->Characteristic_Properties);
               printf("   UUID:               0x");
               DisplayUUID(&(CurrentCharacteristic->Characteristic_UUID));
            }
         }
      }
   }
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

   /* The following function is responsible for registering a BAP       */
   /* Service.  This function will return zero on successful execution  */
   /* and a negative value on errors.                                   */
static int RegisterIPS(ParameterList_t *TempParam)
{
   int                    ret_val = 0;
   IPS_Initialize_Data_t  InitializeData;

   if((TempParam) && (TempParam->NumberofParameters == 3))
   {
      /* Verify that there is no active connection.                     */
      if(!ConnectionID)
      {
         /* Verify that the Service is not already registered.          */
         if(!IPSInstanceID)
         {
            /* Initialize the Initialize Information to make sure there */
            /* is no unexpected behaviour.                              */
            BTPS_MemInitialize(&InitializeData, 0, IPS_INITIALIZE_DATA_SIZE);

            /* We will enable the specified service features..          */
            InitializeData.IPS_Characteristic_Flags          = (unsigned int)TempParam->Params[0].intParam;
            InitializeData.IPS_Characteristic_Property_Flags = (unsigned int)TempParam->Params[1].intParam;
            InitializeData.Enable_Authorized_Device_Write    = (Boolean_t)TempParam->Params[2].intParam;


            /* Initialize the service.                                  */
            ret_val = IPS_Initialize_Service(BluetoothStackID, (unsigned int)IPS_SERVICE_FLAGS_DUAL_MODE, &InitializeData, IPS_EventCallback, 0, &IPSInstanceID);
            if((ret_val > 0) && (IPSInstanceID > 0))
            {
               /* Display success message.                              */
               printf("Successfully registered IPS Service, IPSInstanceID = %u.\r\n", ret_val);

               /* Save the ServiceID of the registered service.         */
               IPSInstanceID = (unsigned int)ret_val;

               /* We can now go ahead and initialize the                */
               /* characteristics.                                      */
               InitializeCharacteristicData();

               /* Simply return success to the caller.                  */
               ret_val = 0;
            }
            else
            {
               DisplayFunctionError("IPS_Initialize_Service", ret_val);
            }
         }
         else
         {
            printf("\r\nIPS is already registered.\r\n");
         }
      }
      else
      {
         printf("\r\nConnection currently active.\r\n");
      }
   }
   else
   {
      /* Invalid parameters specified so flag an error to the user.     */
      printf("Usage: RegisterIPS [IPS_Characteristic_Flags (UINT16)] [IPS_Characteristic_Property_Flags (UINT16)] [Enable Authorized Device Write (Boolean)]\r\n");
      printf("\r\nWhere IPS_Characteristic_Flags (bitmask) are:\r\n");
      printf("      0x01 = Support Local Coordinates.\r\n");
      printf("      0x02 = Support Floor Number.\r\n");
      printf("      0x04 = Support Altitude.\r\n");
      printf("      0x08 = Support Uncertainty.\r\n");
      printf("      0x10 = Support Location Name.\r\n");
      printf("\r\nWhere IPS_Characteristic_Property_Flags (bitmask) are:\r\n");
      printf("      0x01 = Support Write (Mandatory for other writes).\r\n");
      printf("      0x02 = Support Write Without Response.\r\n");
      printf("      0x04 = Support Reliable Write.\r\n");
   }

   return(ret_val);
}

   /* The following function is responsible for unregistering a BAP     */
   /* Service.  This function will return zero on successful execution  */
   /* and a negative value on errors.                                   */
static int UnregisterIPS(ParameterList_t *TempParam)
{
   int ret_val = FUNCTION_ERROR;

   /* Verify that a service is registered.                              */
   if(IPSInstanceID)
   {
      /* If there is a connected device, then first disconnect it.      */
      if(!COMPARE_NULL_BD_ADDR(ConnectionBD_ADDR))
         DisconnectLEDevice(BluetoothStackID, ConnectionBD_ADDR);

      /* Unregister the IPS Service with GATT.                          */
      ret_val = IPS_Cleanup_Service(BluetoothStackID, IPSInstanceID);
      if(ret_val == 0)
      {
         /* Display success message.                                    */
         printf("Successfully unregistered IPS Service InstanceID %u.\r\n", IPSInstanceID);

         /* Clear the InstanceID.                                       */
         IPSInstanceID = 0;
      }
      else
         DisplayFunctionError("IPS_Cleanup_Service", ret_val);
   }
   else
      printf("IPS Service not registered.\r\n");

   return(ret_val);
}

   /* The following function is responsible for initialzing the         */
   /* characteristic data after IPS has been registered.  Since we      */
   /* enabled all characteristics we will initialize all values.        */
static void InitializeCharacteristicData(void)
{
   int    Result;
   Byte_t Config;

   /* Make sure the parameters are semi-valid.                          */
   if(BluetoothStackID)
   {
      if(IPSInstanceID)
      {
         /* Set the IPC.                                                */
         /* * NOTE * We will simply invert the reserved bits so         */
         /*          everything is enabled by default.  We will force   */
         /*          global coordinates to be used by default.          */
         Config = (Byte_t)(~IPS_INDOOR_POSITIONING_CONFIG_RESERVED);
         Config &= ~IPS_INDOOR_POSITIONING_CONFIG_LOCAL_COORDINATES_USED;
         if((Result = IPS_Set_Indoor_Positioning_Configuration(BluetoothStackID, IPSInstanceID, Config)) != 0)
            DisplayFunctionError("IPS_Set_Indoor_Positioning_Configuration", Result);

         /* * NOTE * When the IPS is registered, Mandatory and supported*/
         /*          optional characteristics will be initialized to    */
         /*          their NOT_CONFIGURED values, however we will do the*/
         /*          same here for demonstrational purposes.            */
         /*          Characteristics that do not have a NOT_CONFIGURED  */
         /*          value will be initialized to zero or empty string. */

         /* Set the Latitude.                                           */
         if((Result = IPS_Set_Latitude(BluetoothStackID, IPSInstanceID, IPS_LATITUDE_NOT_CONFIGURED)) != 0)
            DisplayFunctionError("IPS_Set_Latitude", Result);

         /* Set the Longitude.                                          */
         if((Result = IPS_Set_Longitude(BluetoothStackID, IPSInstanceID, IPS_LONGITUDE_NOT_CONFIGURED)) != 0)
            DisplayFunctionError("IPS_Set_Longitude", Result);

         /* Set the Local North Coordinate.                             */
         if((Result = IPS_Set_Local_North_Coordinate(BluetoothStackID, IPSInstanceID, IPS_LOCAL_COORDINATE_NOT_CONFIGURED)) != 0)
            DisplayFunctionError("IPS_Set_Local_North_Coordinate", Result);

         /* Set the Local East Coordinate.                              */
         if((Result = IPS_Set_Local_East_Coordinate(BluetoothStackID, IPSInstanceID, IPS_LOCAL_COORDINATE_NOT_CONFIGURED)) != 0)
            DisplayFunctionError("IPS_Set_Local_East_Coordinate", Result);

         /* Set the Floor Number.                                       */
         if((Result = IPS_Set_Floor_Number(BluetoothStackID, IPSInstanceID, IPS_FLOOR_NUMBER_NOT_CONFIGURED)) != 0)
            DisplayFunctionError("IPS_Set_Floor_Number", Result);

         /* Set the Altitude.                                           */
         if((Result = IPS_Set_Altitude(BluetoothStackID, IPSInstanceID, IPS_ALTITUDE_NOT_CONFIGURED)) != 0)
            DisplayFunctionError("IPS_Set_Altitude", Result);

         /* Set the Uncertainty.                                        */
         if((Result = IPS_Set_Uncertainty(BluetoothStackID, IPSInstanceID, 0x06)) != 0)
            DisplayFunctionError("IPS_Set_Uncertainty", Result);


         /* Set the Location Name.                                      */
         if((Result = IPS_Set_Location_Name(BluetoothStackID, IPSInstanceID, "Office")) != 0)
            DisplayFunctionError("IPS_Set_Location_Name", Result);
      }
      else
         printf("\r\nIPS is not registered.\r\n");
   }
   else
      printf("\r\nInvalid BluetoothStackID.\r\n");
}

   /* The following function is responsible for performing a BAP        */
   /* Service Discovery Operation.  This function will return zero on   */
   /* successful execution and a negative value on errors.              */
static int DiscoverIPS(ParameterList_t *TempParam)
{
   DeviceInfo_t *DeviceInfo;
   GATT_UUID_t   UUID;
   int           ret_val = FUNCTION_ERROR;

   /* Verify that we are not configured as a server                     */
   if(!IPSInstanceID)
   {
      /* Verify that there is a connection that is established.         */
      if(ConnectionID)
      {
         /* Get the device info for the connection device.              */
         if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
         {
            /* Verify that no service discovery is outstanding for this */
            /* device.                                                  */
            if(!(DeviceInfo->Flags & DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_OUTSTANDING))
            {
               /* Configure the filter so that only the IPS Service is  */
               /* discovered.                                           */
               UUID.UUID_Type = guUUID_16;
               IPS_ASSIGN_IPS_SERVICE_UUID_16(&(UUID.UUID.UUID_16));

               /* Start the service discovery process.                  */
               ret_val = GATT_Start_Service_Discovery(BluetoothStackID, ConnectionID, (sizeof(UUID)/sizeof(GATT_UUID_t)), &UUID, GATT_Service_Discovery_Event_Callback, sdIPS);
               if(!ret_val)
               {
                  /* Display success message.                           */
                  printf("GATT_Service_Discovery_Start success.\r\n");

                  /* Flag that a Service Discovery Operation is         */
                  /* outstanding.                                       */
                  DeviceInfo->Flags |= DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_OUTSTANDING;
               }
               else
                  DisplayFunctionError("GATT_Service_Discovery_Start", ret_val);
            }
            else
               printf("Service Discovery Operation Outsanding for Device.\r\n");
         }
         else
            printf("No Device Info.\r\n");
      }
      else
         printf("No Connection Established\r\n");
   }
   else
      printf("Only the client can discover IPS.\r\n");

   return(ret_val);
}

   /* The following function is a utility function that provides a      */
   /* mechanism of populating a IPS Client Information structure with   */
   /* the information discovered from a GATT Discovery operation.       */
static void IPSPopulateHandles(DeviceInfo_t *DeviceInfo, GATT_Service_Discovery_Indication_Data_t *ServiceDiscoveryData)
{
   unsigned int                       Index;
   GATT_Characteristic_Information_t *CharacteristicInfoPtr;

   /* Verify that the input parameters are semi-valid.                  */
   if((DeviceInfo) && (ServiceDiscoveryData) && (ServiceDiscoveryData->ServiceInformation.UUID.UUID_Type == guUUID_16) && (IPS_COMPARE_IPS_SERVICE_UUID_TO_UUID_16(ServiceDiscoveryData->ServiceInformation.UUID.UUID.UUID_16)))
   {
      /* Loop through all characteristics discovered in the service and */
      /* populate the correct entry.                                    */
      CharacteristicInfoPtr = ServiceDiscoveryData->CharacteristicInformationList;
      if(CharacteristicInfoPtr)
      {
         for(Index = 0; Index < ServiceDiscoveryData->NumberOfCharacteristics; Index++)
         {
            /* Print the characteristic information discovered if       */
            /* needed.                                                  */
            if(PRINT_SERVICE_DISCOVERY_HANDLES)
            {
               printf("%d. Characteristic Handle: 0x%04X\r\n", Index+1, CharacteristicInfoPtr[Index].Characteristic_Handle);
               printf("       Properties:        0x%02X\r\n", CharacteristicInfoPtr[Index].Characteristic_Properties);
               printf("       UUID:              0x");
               DisplayUUID(&(CharacteristicInfoPtr[Index].Characteristic_UUID));
            }

            /* All IPS UUIDs are defined to be 16 bit UUIDs.            */
            if(CharacteristicInfoPtr[Index].Characteristic_UUID.UUID_Type == guUUID_16)
            {
               /* If this is the indoor positioning configuration       */
               /* characteristic.                                       */
               if(IPS_COMPARE_INDOOR_POSITIONING_CONFIGURATION_UUID_TO_UUID_16(CharacteristicInfoPtr[Index].Characteristic_UUID.UUID.UUID_16))
               {
                  /* Store the handle.                                  */
                  DeviceInfo->IPSClientInfo.IPS_Indoor_Positioning_Configuration = CharacteristicInfoPtr[Index].Characteristic_Handle;

                  /* Verify that the mandatory read property is         */
                  /* supported.                                         */
                  if(!(CharacteristicInfoPtr[Index].Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
                     printf("\r\nWarning - Mandatory read property of the Indoor Positioning Configuration characteristic not supported!\r\n");

                  /* Store the descriptor handles for this              */
                  /* characteristic.                                    */
                  StoreDescriptorHandles(&(CharacteristicInfoPtr[Index]), &(DeviceInfo->IPSClientInfo.IPS_Indoor_Positioning_Configuration_Extended_Properties));

                  /* Check the next Characteristic.                     */
                  continue;
               }

               /* If this is the Latitude characteristic.               */
               if(IPS_COMPARE_LATITUDE_UUID_TO_UUID_16(CharacteristicInfoPtr[Index].Characteristic_UUID.UUID.UUID_16))
               {
                  /* Store the handle.                                  */
                  DeviceInfo->IPSClientInfo.IPS_Latitude = CharacteristicInfoPtr[Index].Characteristic_Handle;

                  /* Verify that the mandatory read property is         */
                  /* supported.                                         */
                  if(!(CharacteristicInfoPtr[Index].Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
                     printf("\r\nWarning - Mandatory read property of the Indoor Positioning Configuration characteristic not supported!\r\n");

                  /* Store the descriptor handles for this              */
                  /* characteristic.                                    */
                  StoreDescriptorHandles(&(CharacteristicInfoPtr[Index]), &(DeviceInfo->IPSClientInfo.IPS_Latitude_Extended_Properties));

                  /* Check the next Characteristic.                     */
                  continue;
               }


               /* If this is the Longitude characteristic.              */
               if(IPS_COMPARE_LONGITUDE_UUID_TO_UUID_16(CharacteristicInfoPtr[Index].Characteristic_UUID.UUID.UUID_16))
               {
                  /* Store the handle.                                  */
                  DeviceInfo->IPSClientInfo.IPS_Longitude = CharacteristicInfoPtr[Index].Characteristic_Handle;

                  /* Verify that the mandatory read property is         */
                  /* supported.                                         */
                  if(!(CharacteristicInfoPtr[Index].Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
                     printf("\r\nWarning - Mandatory read property of the Indoor Positioning Configuration characteristic not supported!\r\n");

                  /* Store the descriptor handles for this              */
                  /* characteristic.                                    */
                  StoreDescriptorHandles(&(CharacteristicInfoPtr[Index]), &(DeviceInfo->IPSClientInfo.IPS_Longitude_Extended_Properties));

                  /* Check the next Characteristic.                     */
                  continue;
               }

               /* If this is the Local North Coordinate characteristic. */
               if(IPS_COMPARE_LOCAL_NORTH_COORDINATE_UUID_TO_UUID_16(CharacteristicInfoPtr[Index].Characteristic_UUID.UUID.UUID_16))
               {
                  /* Store the handle.                                  */
                  DeviceInfo->IPSClientInfo.IPS_Local_North_Coordinate = CharacteristicInfoPtr[Index].Characteristic_Handle;

                  /* Verify that the mandatory read property is         */
                  /* supported.                                         */
                  if(!(CharacteristicInfoPtr[Index].Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
                     printf("\r\nWarning - Mandatory read property of the Indoor Positioning Configuration characteristic not supported!\r\n");

                  /* Store the descriptor handles for this              */
                  /* characteristic.                                    */
                  StoreDescriptorHandles(&(CharacteristicInfoPtr[Index]), &(DeviceInfo->IPSClientInfo.IPS_Local_North_Coordinate_Extended_Properties));

                  /* Check the next Characteristic.                     */
                  continue;
               }

               /* If this is the Local East Coordinate characteristic.  */
               if(IPS_COMPARE_LOCAL_EAST_COORDINATE_UUID_TO_UUID_16(CharacteristicInfoPtr[Index].Characteristic_UUID.UUID.UUID_16))
               {
                  /* Store the handle.                                  */
                  DeviceInfo->IPSClientInfo.IPS_Local_East_Coordinate = CharacteristicInfoPtr[Index].Characteristic_Handle;

                  /* Verify that the mandatory read property is         */
                  /* supported.                                         */
                  if(!(CharacteristicInfoPtr[Index].Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
                     printf("\r\nWarning - Mandatory read property of the Indoor Positioning Configuration characteristic not supported!\r\n");

                  /* Store the descriptor handles for this              */
                  /* characteristic.                                    */
                  StoreDescriptorHandles(&(CharacteristicInfoPtr[Index]), &(DeviceInfo->IPSClientInfo.IPS_Local_East_Coordinate_Extended_Properties));

                  /* Check the next Characteristic.                     */
                  continue;
               }

               /* If this is the Floor Number characteristic.           */
               if(IPS_COMPARE_FLOOR_NUMBER_UUID_TO_UUID_16(CharacteristicInfoPtr[Index].Characteristic_UUID.UUID.UUID_16))
               {
                  /* Store the handle.                                  */
                  DeviceInfo->IPSClientInfo.IPS_Floor_Number = CharacteristicInfoPtr[Index].Characteristic_Handle;

                  /* Verify that the mandatory read property is         */
                  /* supported.                                         */
                  if(!(CharacteristicInfoPtr[Index].Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
                     printf("\r\nWarning - Mandatory read property of the Indoor Positioning Configuration characteristic not supported!\r\n");

                  /* Store the descriptor handles for this              */
                  /* characteristic.                                    */
                  StoreDescriptorHandles(&(CharacteristicInfoPtr[Index]), &(DeviceInfo->IPSClientInfo.IPS_Floor_Number_Extended_Properties));

                  /* Check the next Characteristic.                     */
                  continue;
               }

               /* If this is the Altitude characteristic.               */
               if(IPS_COMPARE_ALTITUDE_UUID_TO_UUID_16(CharacteristicInfoPtr[Index].Characteristic_UUID.UUID.UUID_16))
               {
                  /* Store the handle.                                  */
                  DeviceInfo->IPSClientInfo.IPS_Altitude = CharacteristicInfoPtr[Index].Characteristic_Handle;

                  /* Verify that the mandatory read property is         */
                  /* supported.                                         */
                  if(!(CharacteristicInfoPtr[Index].Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
                     printf("\r\nWarning - Mandatory read property of the Indoor Positioning Configuration characteristic not supported!\r\n");

                  /* Store the descriptor handles for this              */
                  /* characteristic.                                    */
                  StoreDescriptorHandles(&(CharacteristicInfoPtr[Index]), &(DeviceInfo->IPSClientInfo.IPS_Altitude_Extended_Properties));

                  /* Check the next Characteristic.                     */
                  continue;
               }

               /* If this is the Uncertainty characteristic.            */
               if(IPS_COMPARE_UNCERTAINTY_UUID_TO_UUID_16(CharacteristicInfoPtr[Index].Characteristic_UUID.UUID.UUID_16))
               {
                  /* Store the handle.                                  */
                  DeviceInfo->IPSClientInfo.IPS_Uncertainty = CharacteristicInfoPtr[Index].Characteristic_Handle;

                  /* Verify that the mandatory read property is         */
                  /* supported.                                         */
                  if(!(CharacteristicInfoPtr[Index].Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
                     printf("\r\nWarning - Mandatory read property of the Indoor Positioning Configuration characteristic not supported!\r\n");

                  /* Store the descriptor handles for this              */
                  /* characteristic.                                    */
                  StoreDescriptorHandles(&(CharacteristicInfoPtr[Index]), &(DeviceInfo->IPSClientInfo.IPS_Uncertainty_Extended_Properties));

                  /* Check the next Characteristic.                     */
                  continue;
               }

               /* If this is the Location Name characteristic.          */
               if(IPS_COMPARE_LOCATION_NAME_UUID_TO_UUID_16(CharacteristicInfoPtr[Index].Characteristic_UUID.UUID.UUID_16))
               {
                  /* Store the handle.                                  */
                  DeviceInfo->IPSClientInfo.IPS_Location_Name = CharacteristicInfoPtr[Index].Characteristic_Handle;

                  /* Verify that the mandatory read property is         */
                  /* supported.                                         */
                  if(!(CharacteristicInfoPtr[Index].Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
                     printf("\r\nWarning - Mandatory read property of the Indoor Positioning Configuration characteristic not supported!\r\n");

                  /* Store the descriptor handles for this              */
                  /* characteristic.                                    */
                  StoreDescriptorHandles(&(CharacteristicInfoPtr[Index]), &(DeviceInfo->IPSClientInfo.IPS_Location_Name_Extended_Properties));

                  /* Check the next Characteristic.                     */
                  continue;
               }

               /* Always print warnings about unknown IPS               */
               /* Characteristics .                                     */
               printf("\r\nWarning - Unknown IPS Characteristic.\r\n");
               printf("\r\nCharacteristic Handle: 0x%04X\r\n", CharacteristicInfoPtr[Index].Characteristic_Handle);
               printf("   Properties:         0x%02X\r\n", CharacteristicInfoPtr[Index].Characteristic_Properties);
               printf("   UUID:               0x");
               DisplayUUID(&(CharacteristicInfoPtr[Index].Characteristic_UUID));
            }
            else
               printf("\r\nWarning - Characteristic not a 16 bit UUID.\r\n");
         }
      }
   }
}

   /* The following function is a helper function for                   */
   /* StoreAttributeHandles() to populate the descriptor handles for an */
   /* IPS Charactersitic.                                               */
static void StoreDescriptorHandles(GATT_Characteristic_Information_t *CharacteristicInfoPtr, Word_t *AttributeHandle)
{
   unsigned int                                  Index;
   GATT_Characteristic_Descriptor_Information_t *DescriptorInfoPtr;

   /* Save a pointer to the descriptor list for this Characteristic.    */
   DescriptorInfoPtr = CharacteristicInfoPtr->DescriptorList;

   /* Populate the optional characteristic descriptors.                 */
   for(Index = 0; Index < CharacteristicInfoPtr[Index].NumberOfDescriptors; Index++)
   {
      /* Print the characteristic descriptor information if needed.     */
      if(PRINT_SERVICE_DISCOVERY_HANDLES)
      {
         printf("\r\n");
         printf("%d. Descriptor Handle:     0x%04X\r\n", Index+1, DescriptorInfoPtr[Index].Characteristic_Descriptor_Handle);
         printf("       UUID:              0x");

         DisplayUUID(&(DescriptorInfoPtr[Index].Characteristic_Descriptor_UUID));
         if((Index+1) == CharacteristicInfoPtr->NumberOfDescriptors)
            printf("\r\n");
      }

      /* Make sure the the UUID is a 16 bit UUID.                       */
      if(DescriptorInfoPtr->Characteristic_Descriptor_UUID.UUID_Type == guUUID_16)
      {
         /* Check for the Extended Properties descriptor.               */
         if(GATT_COMPARE_CHARACTERISTIC_EXTENDED_PROPERTIES_ATTRIBUTE_TYPE_TO_BLUETOOTH_UUID_16(DescriptorInfoPtr[Index].Characteristic_Descriptor_UUID.UUID.UUID_16))
         {
            /* Store the handle                                         */
            *AttributeHandle = DescriptorInfoPtr[Index].Characteristic_Descriptor_Handle;

            /* Get the next descriptor.                                 */
            continue;
         }

         /* Always print warnings for unknown characteristic            */
         /* descriptors.                                                */
         printf("\r\nWarning - Unknown IPS Characteristic Descriptor.\r\n");
         printf("\r\nDescriptor Handle: 0x%04X\r\n", DescriptorInfoPtr[Index].Characteristic_Descriptor_Handle);
         printf("   UUID:           0x");
         DisplayUUID(&(DescriptorInfoPtr[Index].Characteristic_Descriptor_UUID));
      }
      else
         printf("\r\nWarning - Descriptor not a 16 bit UUID.\r\n");
   }
}

   /* The following function is responsible for reading the specified   */
   /* characteristic.  If this function is called by an IPS Server then */
   /* the characteristic will be read directly.  If this function is    */
   /* called by the IPS Client then a GATT read request will be issued. */
   /* The GATT request will be received by the registered callback when */
   /* the request is sent.                                              */
static int ReadIndoorPositioningConfiguration(ParameterList_t *TempParam)
{
   int           ret_val;
   DeviceInfo_t *DeviceInfo;
   Word_t        AttributeHandle;
   Byte_t        IndoorPositioningConfig;

   /* Make sure the stack is initialized.                               */
   if(BluetoothStackID)
   {
      /* Check if this is the IPS Server.                               */
      if(IPSInstanceID)
      {
         /* Let's call the API to query the characteristic.             */
         if((ret_val = IPS_Query_Indoor_Positioning_Configuration(BluetoothStackID, IPSInstanceID, &IndoorPositioningConfig)) == 0)
         {
            /* Print the characteristic.                                */
            printf("\r\nIndoor Positioning Configuration:\r\n");
            printf("   Value:   0x%02X\r\n", IndoorPositioningConfig);
         }
         else
            DisplayFunctionError("IPS_Query_Indoor_Positioning_Configuration", ret_val);
      }
      else
      {
         /* We must be the client so let's make sure we are connected.  */
         if(ConnectionID)
         {
            /* Get the device information.                              */
            if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList,  ConnectionBD_ADDR)) != NULL)
            {
               /* Make sure we have performed service discovery so we   */
               /* have the attribute handle offset to request.          */
               if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_IPS_SERVICE_DISCOVERY_COMPLETE)
               {
                  /* Make sure we have the desired handle.  May not     */
                  /* exist if this is an optional characteristic.       */
                  /* * NOTE * This handle is also used in the GATT read */
                  /*          request as the CallbackParameter, which we*/
                  /*          will use to identity the characteristic   */
                  /*          for the read response in the              */
                  /*          GATT_ClientEventCallback_IPS callback.    */
                  if(DeviceInfo->IPSClientInfo.IPS_Indoor_Positioning_Configuration)
                  {
                     /* Store the handle.                               */
                     AttributeHandle = DeviceInfo->IPSClientInfo.IPS_Indoor_Positioning_Configuration;

                     /* Send the GATT read request.                     */
                     /* * NOTE * We will not save the transactionID     */
                     /*          returned by this function, which we    */
                     /*          could use to cancel the request.       */
                     if((ret_val = GATT_Read_Value_Request(BluetoothStackID, ConnectionID, AttributeHandle, GATT_ClientEventCallback_IPS, AttributeHandle)) > 0)
                     {
                        printf("\r\nGATT Read Value Request sent:\r\n");
                        printf("   TransactionID:     %d\r\n",     ret_val);
                        printf("   Attribute Handle:  0x%04X\r\n", AttributeHandle);
                     }
                     else
                        DisplayFunctionError("GATT_Read_Value_Request", ret_val);
                  }
                  else
                     printf("\r\nCharacteristic handle has not been discovered.\r\n");
               }
               else
                  printf("\r\nService discovery has not been completed.\r\n");
            }
            else
               printf("\r\nNo device information.\r\n");
         }
         else
            printf("\r\nMust be connected to an IPS Server.\r\n");
      }
   }
   else
      printf("\r\nBluetooth stack has not been initialized.\r\n");

   return (0);
}

   /* The following function is responsible for writing the specified   */
   /* characteristic.  If this function is called by an IPS Server then */
   /* the characteristic will be written directly.  If this function is */
   /* called by the IPS Client then a GATT write or write without       */
   /* response request will be issued.  The GATT request will be        */
   /* received by the registered callback for the GATT write request.   */
static int WriteIndoorPositioningConfiguration(ParameterList_t *TempParam)
{
   int           ret_val;
   DeviceInfo_t *DeviceInfo;
   Word_t        AttributeHandle;
   Byte_t        IndoorPositioningConfig;
   enum
   {
      wmWrite,
      wmWriteWOResponse,
      wmReliableWrite
   } WriteMode;

   /* Make sure the stack is initialized.                               */
   if(BluetoothStackID)
   {
      /* Verify that the parameters are semi-valid.                     */
      if((TempParam) && (TempParam->NumberofParameters >= 1) && (TempParam->NumberofParameters <= 2))
      {
         /* Store the IPC.                                              */
         IndoorPositioningConfig = (Byte_t)TempParam->Params[0].intParam;

         /* Check if this is the IPS Server.                            */
         if(IPSInstanceID)
         {
            /* Let's call the API to write the characteristic.          */
            if((ret_val = IPS_Set_Indoor_Positioning_Configuration(BluetoothStackID, IPSInstanceID, IndoorPositioningConfig)) == 0)
            {
               /* Print the characteristic.                             */
               printf("\r\nIndoor Positioning Configuration:\r\n");
               printf("   Value:   0x%02X\r\n", IndoorPositioningConfig);
            }
            else
               DisplayFunctionError("IPS_Set_Indoor_Positioning_Configuration", ret_val);
         }
         else
         {
            /* Make sure the Request Type has been supplied.            */
            if(TempParam->NumberofParameters == 2)
            {
               /* Store the write mode.                                 */
               WriteMode = TempParam->Params[1].intParam;

               /* We must be the client so let's make sure we are       */
               /* connected.                                            */
               if(ConnectionID)
               {
                  /* Get the device information.                        */
                  if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList,  ConnectionBD_ADDR)) != NULL)
                  {
                     /* Make sure we have performed service discovery so*/
                     /* we have the attribute handle offset to request. */
                     if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_IPS_SERVICE_DISCOVERY_COMPLETE)
                     {
                        /* Make sure we have the desired handle.  May   */
                        /* not exist if this is an optional             */
                        /* characteristic.                              */
                        /* * NOTE * This handle is also used in the GATT*/
                        /*          read request as the                 */
                        /*          CallbackParameter, which we will use*/
                        /*          to identity the characteristic for  */
                        /*          the read response in the            */
                        /*          GATT_ClientEventCallback_IPS        */
                        /*          callback.                           */
                        if(DeviceInfo->IPSClientInfo.IPS_Indoor_Positioning_Configuration)
                        {
                           /* Store the handle.                         */
                           AttributeHandle = DeviceInfo->IPSClientInfo.IPS_Indoor_Positioning_Configuration;

                           /* Determine the GATT Write type.            */
                           switch(WriteMode)
                           {
                              case wmWrite:
                                 if(!(DeviceInfo->Flags & DEVICE_INFO_FLAGS_RELIABLE_WRITE_MODE_ENABLED))
                                 {
                                    /* Send the GATT Write Request.     */
                                    /* * NOTE * We will not save the    */
                                    /*          transactionID returned  */
                                    /*          by this function, which */
                                    /*          we could use to cancel  */
                                    /*          the request.            */
                                    if((ret_val = GATT_Write_Request(BluetoothStackID, ConnectionID, AttributeHandle, (Word_t)NON_ALIGNED_BYTE_SIZE, &IndoorPositioningConfig, GATT_ClientEventCallback_IPS, AttributeHandle)) > 0)
                                    {
                                       printf("\r\nGATT Write Request sent:\r\n");
                                       printf("   TransactionID:     %d\r\n",     ret_val);
                                       printf("   Attribute Handle:  0x%04X\r\n", AttributeHandle);
                                    }
                                    else
                                       DisplayFunctionError("GATT_Write_Request", ret_val);
                                 }
                                 else
                                    printf("\r\nReliable write in progress. Use command 'ExecWrite' to cancel.\r\n");
                                 break;
                              case wmWriteWOResponse:
                                 if(!(DeviceInfo->Flags & DEVICE_INFO_FLAGS_RELIABLE_WRITE_MODE_ENABLED))
                                 {
                                    /* Send the GATT Write Without      */
                                    /* Response Request.                */
                                    if((ret_val = GATT_Write_Without_Response_Request(BluetoothStackID, ConnectionID, AttributeHandle, (Word_t)NON_ALIGNED_BYTE_SIZE, &IndoorPositioningConfig)) > 0)
                                    {
                                       printf("\r\nGATT Write Request WO/Response sent:\r\n");
                                       printf("   Number of Bytes Written:  %u\r\n",     ret_val);
                                       printf("   Attribute Handle:         0x%04X\r\n", AttributeHandle);
                                    }
                                    else
                                       DisplayFunctionError("GATT_Write_Without_Response_Request", ret_val);
                                 }
                                 else
                                    printf("\r\nReliable write in progress. Use command 'ExecWrite' to cancel.r\n");
                                 break;
                              case wmReliableWrite:
                                 /* Send the GATT Prepared Write        */
                                 /* Request.                            */
                                 /* * NOTE * We will not save the       */
                                 /*          transactionID returned by  */
                                 /*          this function, which we    */
                                 /*          could use to cancel the    */
                                 /*          request.                   */
                                 if((ret_val = GATT_Prepare_Write_Request(BluetoothStackID, ConnectionID, AttributeHandle, (Word_t)NON_ALIGNED_BYTE_SIZE, 0, &IndoorPositioningConfig, GATT_ClientEventCallback_IPS, AttributeHandle)) > 0)
                                 {
                                    printf("\r\nGATT Prepare Write Request sent:\r\n");
                                    printf("   TransactionID:     %d\r\n",     ret_val);
                                    printf("   Attribute Handle:  0x%04X\r\n", AttributeHandle);

                                    /* Enable reliable write mode.      */
                                    /* * NOTE * This can only be        */
                                    /*          disabled by calling the */
                                    /*          ExecuteWriteRequest() to*/
                                    /*          write the request data  */
                                    /*          or cancel the request.  */
                                    DeviceInfo->Flags |= DEVICE_INFO_FLAGS_RELIABLE_WRITE_MODE_ENABLED;

                                 }
                                 else
                                    DisplayFunctionError("GATT_Prepare_Write_Request", ret_val);
                                 break;
                              default:
                                /* Do nothing.                          */
                                break;
                           }
                        }
                        else
                           printf("\r\nCharacteristic handle has not been discovered.\r\n");
                     }
                     else
                        printf("\r\nService discovery has not been completed.\r\n");
                  }
                  else
                     printf("\r\nNo device information.\r\n");
               }
               else
                  printf("\r\nMust be connected to an IPS Server.\r\n");
            }
            else
            {
               /* Display the usage.                                    */
               DisplayWriteIPCUsage();
            }
         }
      }
      else
      {
         /* Display the usage.                                          */
         DisplayWriteIPCUsage();
      }
   }
   else
      printf("\r\nBluetooth stack has not been initialized.\r\n");

   return (0);
}

   /* The following function is responsible for reading the specified   */
   /* characteristic.  If this function is called by an IPS Server then */
   /* the characteristic will be read directly.  If this function is    */
   /* called by the IPS Client then a GATT read request will be issued. */
   /* The GATT request will be received by the registered callback when */
   /* the request is sent.                                              */
static int ReadLatitude(ParameterList_t *TempParam)
{
   int           ret_val;
   DeviceInfo_t *DeviceInfo;
   Word_t        AttributeHandle;
   SDWord_t      Latitude;

   /* Make sure the stack is initialized.                               */
   if(BluetoothStackID)
   {
      /* Check if this is the IPS Server.                               */
      if(IPSInstanceID)
      {
         /* Let's call the API to query the characteristic.             */
         if((ret_val = IPS_Query_Latitude(BluetoothStackID, IPSInstanceID, &Latitude)) == 0)
         {
            printf("\r\nLatitude:\r\n");
            if(Latitude != (SDWord_t)IPS_LATITUDE_NOT_CONFIGURED)
            {
               /* Simply call the function to decode the Latitude.      */
               /* Print the value.                                      */
               DecodeLatitude(&Latitude);
               printf("   Value:   %d (degrees).\r\n", Latitude);
            }
            else
               printf("   Value:   Not Configured.\r\n");
         }
         else
            DisplayFunctionError("IPS_Query_Latitude", ret_val);
      }
      else
      {
         /* We must be the client so let's make sure we are connected.  */
         if(ConnectionID)
         {
            /* Get the device information.                              */
            if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList,  ConnectionBD_ADDR)) != NULL)
            {
               /* Make sure we have performed service discovery so we   */
               /* have the attribute handle offset to request.          */
               if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_IPS_SERVICE_DISCOVERY_COMPLETE)
               {
                  /* Make sure we have the desired handle.  May not     */
                  /* exist if this is an optional characteristic.       */
                  /* * NOTE * This handle is also used in the GATT read */
                  /*          request as the CallbackParameter, which we*/
                  /*          will use to identity the characteristic   */
                  /*          for the read response in the              */
                  /*          GATT_ClientEventCallback_IPS callback.    */
                  if(DeviceInfo->IPSClientInfo.IPS_Latitude)
                  {
                     /* Store the handle.                               */
                     AttributeHandle = DeviceInfo->IPSClientInfo.IPS_Latitude;

                     /* Send the GATT read request.                     */
                     /* * NOTE * We will not save the transactionID     */
                     /*          returned by this function, which we    */
                     /*          could use to cancel the request.       */
                     if((ret_val = GATT_Read_Value_Request(BluetoothStackID, ConnectionID, AttributeHandle, GATT_ClientEventCallback_IPS, AttributeHandle)) > 0)
                     {
                        printf("\r\nGATT Read Value Request sent:\r\n");
                        printf("   TransactionID:     %d\r\n",     ret_val);
                        printf("   Attribute Handle:  0x%04X\r\n", AttributeHandle);
                     }
                     else
                        DisplayFunctionError("GATT_Read_Value_Request", ret_val);
                  }
                  else
                     printf("\r\nCharacteristic handle has not been discovered.\r\n");
               }
               else
                  printf("\r\nService discovery has not been completed.\r\n");
            }
            else
               printf("\r\nNo device information.\r\n");
         }
         else
            printf("\r\nMust be connected to an IPS Server.\r\n");
      }
   }
   else
      printf("\r\nBluetooth stack has not been initialized.\r\n");

   return (0);
}

   /* The following function is responsible for writing the specified   */
   /* characteristic.  If this function is called by an IPS Server then */
   /* the characteristic will be written directly.  If this function is */
   /* called by the IPS Client then a GATT write or write without       */
   /* response request will be issued.  The GATT request will be        */
   /* received by the registered callback for the GATT write request.   */
static int WriteLatitude(ParameterList_t *TempParam)
{
   int           ret_val;
   DeviceInfo_t *DeviceInfo;
   Word_t        AttributeHandle;
   SDWord_t      Latitude;
   enum
   {
      wmWrite,
      wmWriteWOResponse,
      wmReliableWrite
   } WriteMode;

   /* Make sure the stack is initialized.                               */
   if(BluetoothStackID)
   {
      /* Verify that the parameters are semi-valid.                     */
      if((TempParam) && (TempParam->NumberofParameters >= 1) && (TempParam->NumberofParameters <= 2))
      {
         /* Store the Latitude.                                         */
         Latitude = (SDWord_t)TempParam->Params[0].intParam;

         /* Verify the Latitude.                                        */
         if((Latitude >= -90) && (Latitude <= 90))
         {
            /* Encode the Latitude.                                     */
            //Latitude = IPS_ENCODE_LATITUDE_CHARACTERISTIC(Latitude);
            EncodeLatitude(&Latitude);

            /* Check if this is the IPS Server.                         */
            if(IPSInstanceID)
            {
               /* Let's call the API to write the characteristic.       */
               if((ret_val = IPS_Set_Latitude(BluetoothStackID, IPSInstanceID, Latitude)) == 0)
               {
                  /* Print the characteristic.                          */
                  printf("\r\nLatitude:\r\n");
                  printf("   Value:   0x%08X\r\n", Latitude);
               }
               else
                  DisplayFunctionError("IPS_Set_Latitude", ret_val);
            }
            else
            {
               /* Make sure the Request Type has been supplied.         */
               if(TempParam->NumberofParameters == 2)
               {
                  /* Store the write mode.                              */
                  WriteMode = TempParam->Params[1].intParam;

                  /* We must be the client so let's make sure we are    */
                  /* connected.                                         */
                  if(ConnectionID)
                  {
                     /* Get the device information.                     */
                     if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList,  ConnectionBD_ADDR)) != NULL)
                     {
                        /* Make sure we have performed service discovery*/
                        /* so we have the attribute handle offset to    */
                        /* request.                                     */
                        if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_IPS_SERVICE_DISCOVERY_COMPLETE)
                        {
                           /* Make sure we have the desired handle.  May*/
                           /* not exist if this is an optional          */
                           /* characteristic.                           */
                           /* * NOTE * This handle is also used in the  */
                           /*          GATT read request as the         */
                           /*          CallbackParameter, which we will */
                           /*          use to identity the              */
                           /*          characteristic for the read      */
                           /*          response in the                  */
                           /*          GATT_ClientEventCallback_IPS     */
                           /*          callback.                        */
                           if(DeviceInfo->IPSClientInfo.IPS_Latitude)
                           {
                              /* Store the handle.                      */
                              AttributeHandle = DeviceInfo->IPSClientInfo.IPS_Latitude;

                              /* Determine the GATT Write type.         */
                              switch(WriteMode)
                              {
                                 case wmWrite:
                                    if(!(DeviceInfo->Flags & DEVICE_INFO_FLAGS_RELIABLE_WRITE_MODE_ENABLED))
                                    {
                                       /* Send the GATT Write Request.  */
                                       /* * NOTE * We will not save the */
                                       /*          transactionID        */
                                       /*          returned by this     */
                                       /*          function, which we   */
                                       /*          could use to cancel  */
                                       /*          the request.         */
                                       if((ret_val = GATT_Write_Request(BluetoothStackID, ConnectionID, AttributeHandle, (Word_t)NON_ALIGNED_SDWORD_SIZE, &Latitude, GATT_ClientEventCallback_IPS, AttributeHandle)) > 0)
                                       {
                                          printf("\r\nGATT Write Request sent:\r\n");
                                          printf("   TransactionID:     %d\r\n",     ret_val);
                                          printf("   Attribute Handle:  0x%04X\r\n", AttributeHandle);
                                       }
                                       else
                                          DisplayFunctionError("GATT_Write_Request", ret_val);
                                    }
                                    else
                                       printf("\r\nReliable write in progress. Use command 'ExecWrite' to cancel.r\n");
                                    break;
                                 case wmWriteWOResponse:
                                    if(!(DeviceInfo->Flags & DEVICE_INFO_FLAGS_RELIABLE_WRITE_MODE_ENABLED))
                                    {
                                       /* Send the GATT Write Without   */
                                       /* Response Request.             */
                                       if((ret_val = GATT_Write_Without_Response_Request(BluetoothStackID, ConnectionID, AttributeHandle, (Word_t)NON_ALIGNED_SDWORD_SIZE, &Latitude)) > 0)
                                       {
                                          printf("\r\nGATT Write Without Response Request sent:\r\n");
                                          printf("   Number of Bytes Written:  %u\r\n",     ret_val);
                                          printf("   Attribute Handle:         0x%04X\r\n", AttributeHandle);
                                       }
                                       else
                                          DisplayFunctionError("GATT_Write_Without_Response_Request", ret_val);
                                    }
                                    else
                                       printf("\r\nReliable write in progress. Use command 'ExecWrite' to cancel.r\n");
                                    break;
                                 case wmReliableWrite:
                                    /* Send the GATT Prepared Write     */
                                    /* Request.                         */
                                    /* * NOTE * We will not save the    */
                                    /*          transactionID returned  */
                                    /*          by this function, which */
                                    /*          we could use to cancel  */
                                    /*          the request.            */
                                    if((ret_val = GATT_Prepare_Write_Request(BluetoothStackID, ConnectionID, AttributeHandle, (Word_t)NON_ALIGNED_SDWORD_SIZE, 0, &Latitude, GATT_ClientEventCallback_IPS, AttributeHandle)) > 0)
                                    {
                                       printf("\r\nGATT Prepare Write Request sent:\r\n");
                                       printf("   TransactionID:     %d\r\n",     ret_val);
                                       printf("   Attribute Handle:  0x%04X\r\n", AttributeHandle);

                                       /* Enable reliable write mode.   */
                                       /* * NOTE * This can only be     */
                                       /*          disabled by calling  */
                                       /*          the                  */
                                       /*          ExecuteWriteRequest()*/
                                       /*          to write the request */
                                       /*          data or cancel the   */
                                       /*          request.             */
                                       DeviceInfo->Flags |= DEVICE_INFO_FLAGS_RELIABLE_WRITE_MODE_ENABLED;
                                    }
                                    else
                                       DisplayFunctionError("GATT_Prepare_Write_Request", ret_val);
                                    break;
                                 default:
                                   /* Do nothing.                       */
                                   break;
                              }
                           }
                           else
                              printf("\r\nCharacteristic handle has not been discovered.\r\n");
                        }
                        else
                           printf("\r\nService discovery has not been completed.\r\n");
                     }
                     else
                        printf("\r\nNo device information.\r\n");
                  }
                  else
                     printf("\r\nMust be connected to an IPS Server.\r\n");
               }
               else
               {
                  /* Display the usage.                                 */
                  DisplayLatitudeUsage();
               }
            }
         }
         else
            printf("\r\nLatitude MUST be between -90 and 90 degrees.\r\n");
      }
      else
      {
         /* Display the usage.                                          */
         DisplayLatitudeUsage();
      }
   }
   else
      printf("\r\nBluetooth stack has not been initialized.\r\n");

   return (0);
}

   /* The following function is responsible for reading the specified   */
   /* characteristic.  If this function is called by an IPS Server then */
   /* the characteristic will be read directly.  If this function is    */
   /* called by the IPS Client then a GATT read request will be issued. */
   /* The GATT request will be received by the registered callback when */
   /* the request is sent.                                              */
static int ReadLongitude(ParameterList_t *TempParam)
{
   int           ret_val;
   DeviceInfo_t *DeviceInfo;
   Word_t        AttributeHandle;
   SDWord_t      Longitude;

   /* Make sure the stack is initialized.                               */
   if(BluetoothStackID)
   {
      /* Check if this is the IPS Server.                               */
      if(IPSInstanceID)
      {
         /* Let's call the API to query the characteristic.             */
         if((ret_val = IPS_Query_Longitude(BluetoothStackID, IPSInstanceID, &Longitude)) == 0)
         {
            printf("\r\nLongitude:\r\n");
            if(Longitude != (SDWord_t)IPS_LONGITUDE_NOT_CONFIGURED)
            {
               /* Simply call the function to decode the Longitude.     */
               /* Print the value.                                      */
               DecodeLongitude(&Longitude);
               printf("   Value:   %d (degrees).\r\n", Longitude);
            }
            else
               printf("   Value:   Not Configured.\r\n");
         }
         else
            DisplayFunctionError("IPS_Query_Longitude", ret_val);
      }
      else
      {
         /* We must be the client so let's make sure we are connected.  */
         if(ConnectionID)
         {
            /* Get the device information.                              */
            if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList,  ConnectionBD_ADDR)) != NULL)
            {
               /* Make sure we have performed service discovery so we   */
               /* have the attribute handle offset to request.          */
               if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_IPS_SERVICE_DISCOVERY_COMPLETE)
               {
                  /* Make sure we have the desired handle.  May not     */
                  /* exist if this is an optional characteristic.       */
                  /* * NOTE * This handle is also used in the GATT read */
                  /*          request as the CallbackParameter, which we*/
                  /*          will use to identity the characteristic   */
                  /*          for the read response in the              */
                  /*          GATT_ClientEventCallback_IPS callback.    */
                  if(DeviceInfo->IPSClientInfo.IPS_Longitude)
                  {
                     /* Store the handle.                               */
                     AttributeHandle = DeviceInfo->IPSClientInfo.IPS_Longitude;

                     /* Send the GATT read request.                     */
                     /* * NOTE * We will not save the transactionID     */
                     /*          returned by this function, which we    */
                     /*          could use to cancel the request.       */
                     if((ret_val = GATT_Read_Value_Request(BluetoothStackID, ConnectionID, AttributeHandle, GATT_ClientEventCallback_IPS, AttributeHandle)) > 0)
                     {
                        printf("\r\nGATT Read Value Request sent:\r\n");
                        printf("   TransactionID:     %d\r\n",     ret_val);
                        printf("   Attribute Handle:  0x%04X\r\n", AttributeHandle);
                     }
                     else
                        DisplayFunctionError("GATT_Read_Value_Request", ret_val);
                  }
                  else
                     printf("\r\nCharacteristic handle has not been discovered.\r\n");
               }
               else
                  printf("\r\nService discovery has not been completed.\r\n");
            }
            else
               printf("\r\nNo device information.\r\n");
         }
         else
            printf("\r\nMust be connected to an IPS Server.\r\n");
      }
   }
   else
      printf("\r\nBluetooth stack has not been initialized.\r\n");

   return (0);
}

   /* The following function is responsible for writing the specified   */
   /* characteristic.  If this function is called by an IPS Server then */
   /* the characteristic will be written directly.  If this function is */
   /* called by the IPS Client then a GATT write or write without       */
   /* response request will be issued.  The GATT request will be        */
   /* received by the registered callback for the GATT write request.   */
static int WriteLongitude(ParameterList_t *TempParam)
{
   int           ret_val;
   DeviceInfo_t *DeviceInfo;
   Word_t        AttributeHandle;
   SDWord_t      Longitude;
   enum
   {
      wmWrite,
      wmWriteWOResponse,
      wmReliableWrite
   } WriteMode;

   /* Make sure the stack is initialized.                               */
   if(BluetoothStackID)
   {
      /* Verify that the parameters are semi-valid.                     */
      if((TempParam) && (TempParam->NumberofParameters >= 1) && (TempParam->NumberofParameters <= 2))
      {
         /* Store the Longitude.                                        */
         Longitude = (SDWord_t)TempParam->Params[0].intParam;

         /* Verify the Longitude.                                       */
         if((Longitude >= -180) && (Longitude <= 180))
         {
            /* Encode the Latitude.                                     */
            EncodeLongitude(&Longitude);

            /* Check if this is the IPS Server.                         */
            if(IPSInstanceID)
            {
               /* Let's call the API to write the characteristic.       */
               if((ret_val = IPS_Set_Longitude(BluetoothStackID, IPSInstanceID, Longitude)) == 0)
               {
                  /* Print the characteristic.                          */
                  printf("\r\nLongitude:\r\n");
                  printf("   Value:   0x%08X\r\n", Longitude);
               }
               else
                  DisplayFunctionError("IPS_Set_Longitude", ret_val);
            }
            else
            {
               /* Make sure the Request Type has been supplied.         */
               if(TempParam->NumberofParameters == 2)
               {
                  /* Store the write mode.                              */
                  WriteMode = TempParam->Params[1].intParam;

                  /* We must be the client so let's make sure we are    */
                  /* connected.                                         */
                  if(ConnectionID)
                  {
                     /* Get the device information.                     */
                     if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList,  ConnectionBD_ADDR)) != NULL)
                     {
                        /* Make sure we have performed service discovery*/
                        /* so we have the attribute handle offset to    */
                        /* request.                                     */
                        if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_IPS_SERVICE_DISCOVERY_COMPLETE)
                        {
                           /* Make sure we have the desired handle.  May*/
                           /* not exist if this is an optional          */
                           /* characteristic.                           */
                           /* * NOTE * This handle is also used in the  */
                           /*          GATT read request as the         */
                           /*          CallbackParameter, which we will */
                           /*          use to identity the              */
                           /*          characteristic for the read      */
                           /*          response in the                  */
                           /*          GATT_ClientEventCallback_IPS     */
                           /*          callback.                        */
                           if(DeviceInfo->IPSClientInfo.IPS_Longitude)
                           {
                              /* Store the handle.                      */
                              AttributeHandle = DeviceInfo->IPSClientInfo.IPS_Longitude;

                              /* Determine the GATT Write type.         */
                              switch(WriteMode)
                              {
                                 case wmWrite:
                                    if(!(DeviceInfo->Flags & DEVICE_INFO_FLAGS_RELIABLE_WRITE_MODE_ENABLED))
                                    {
                                       /* Send the GATT Write Request.  */
                                       /* * NOTE * We will not save the */
                                       /*          transactionID        */
                                       /*          returned by this     */
                                       /*          function, which we   */
                                       /*          could use to cancel  */
                                       /*          the request.         */
                                       if((ret_val = GATT_Write_Request(BluetoothStackID, ConnectionID, AttributeHandle, (Word_t)NON_ALIGNED_SDWORD_SIZE, &Longitude, GATT_ClientEventCallback_IPS, AttributeHandle)) > 0)
                                       {
                                          printf("\r\nGATT Write Request sent:\r\n");
                                          printf("   TransactionID:     %d\r\n",     ret_val);
                                          printf("   Attribute Handle:  0x%04X\r\n", AttributeHandle);
                                       }
                                       else
                                          DisplayFunctionError("GATT_Write_Request", ret_val);
                                    }
                                    else
                                       printf("\r\nReliable write in progress. Use command 'ExecWrite' to cancel.r\n");
                                    break;
                                 case wmWriteWOResponse:
                                    if(!(DeviceInfo->Flags & DEVICE_INFO_FLAGS_RELIABLE_WRITE_MODE_ENABLED))
                                    {
                                       /* Send the GATT Write Without   */
                                       /* Response Request.             */
                                       if((ret_val = GATT_Write_Without_Response_Request(BluetoothStackID, ConnectionID, AttributeHandle, (Word_t)NON_ALIGNED_SDWORD_SIZE, &Longitude)) > 0)
                                       {
                                          printf("\r\nGATT Write Without Response Request sent:\r\n");
                                          printf("   Number of Bytes Written:  %u\r\n",     ret_val);
                                          printf("   Attribute Handle:         0x%04X\r\n", AttributeHandle);
                                       }
                                       else
                                          DisplayFunctionError("GATT_Write_Without_Response_Request", ret_val);
                                    }
                                    else
                                       printf("\r\nReliable write in progress. Use command 'ExecWrite' to cancel.r\n");
                                    break;
                                 case wmReliableWrite:
                                    /* Send the GATT Prepared Write     */
                                    /* Request.                         */
                                    /* * NOTE * We will not save the    */
                                    /*          transactionID returned  */
                                    /*          by this function, which */
                                    /*          we could use to cancel  */
                                    /*          the request.            */
                                    if((ret_val = GATT_Prepare_Write_Request(BluetoothStackID, ConnectionID, AttributeHandle, (Word_t)NON_ALIGNED_SDWORD_SIZE, 0, &Longitude, GATT_ClientEventCallback_IPS, AttributeHandle)) > 0)
                                    {
                                       printf("\r\nGATT Prepare Write Request sent:\r\n");
                                       printf("   TransactionID:     %d\r\n",     ret_val);
                                       printf("   Attribute Handle:  0x%04X\r\n", AttributeHandle);

                                       /* Enable reliable write mode.   */
                                       /* * NOTE * This can only be     */
                                       /*          disabled by calling  */
                                       /*          the                  */
                                       /*          ExecuteWriteRequest()*/
                                       /*          to write the request */
                                       /*          data or cancel the   */
                                       /*          request.             */
                                       DeviceInfo->Flags |= DEVICE_INFO_FLAGS_RELIABLE_WRITE_MODE_ENABLED;
                                    }
                                    else
                                       DisplayFunctionError("GATT_Prepare_Write_Request", ret_val);
                                    break;
                                 default:
                                   /* Do nothing.                       */
                                   break;
                              }
                           }
                           else
                              printf("\r\nCharacteristic handle has not been discovered.\r\n");
                        }
                        else
                           printf("\r\nService discovery has not been completed.\r\n");
                     }
                     else
                        printf("\r\nNo device information.\r\n");
                  }
                  else
                     printf("\r\nMust be connected to an IPS Server.\r\n");
               }
               else
               {
                  /* Display the usage.                                 */
                  DisplayLongitudeUsage();
               }
            }
         }
         else
            printf("\r\nLongitude MUST be between -180 and 180 degrees.\r\n");
      }
      else
      {
         /* Display the usage.                                          */
         DisplayLongitudeUsage();
      }
   }
   else
      printf("\r\nBluetooth stack has not been initialized.\r\n");

   return (0);
}

   /* The following function is responsible for reading the specified   */
   /* characteristic.  If this function is called by an IPS Server then */
   /* the characteristic will be read directly.  If this function is    */
   /* called by the IPS Client then a GATT read request will be issued. */
   /* The GATT request will be received by the registered callback when */
   /* the request is sent.                                              */
static int ReadLocalNorthCoordinate(ParameterList_t *TempParam)
{
   int           ret_val;
   DeviceInfo_t *DeviceInfo;
   Word_t        AttributeHandle;
   SWord_t       LocalNorthCoordinate;

   /* Make sure the stack is initialized.                               */
   if(BluetoothStackID)
   {
      /* Check if this is the IPS Server.                               */
      if(IPSInstanceID)
      {
         /* Let's call the API to query the characteristic.             */
         if((ret_val = IPS_Query_Local_North_Coordinate(BluetoothStackID, IPSInstanceID, &LocalNorthCoordinate)) == 0)
         {
            /* Print the characteristic.                                */
            printf("\r\nLocal North Coordinate:\r\n");
            printf("   Value:   0x%04X\r\n", LocalNorthCoordinate);
         }
         else
         {
            /* Let's check if the characteristic is supported.          */
            if(ret_val == IPS_ERROR_FEATURE_NOT_SUPPORTED)
            {
               printf("\r\nCharacteristic is not supported.\r\n");
            }
            else
               DisplayFunctionError("IPS_Query_Local_North_Coordinate", ret_val);
         }
      }
      else
      {
         /* We must be the client so let's make sure we are connected.  */
         if(ConnectionID)
         {
            /* Get the device information.                              */
            if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList,  ConnectionBD_ADDR)) != NULL)
            {
               /* Make sure we have performed service discovery so we   */
               /* have the attribute handle offset to request.          */
               if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_IPS_SERVICE_DISCOVERY_COMPLETE)
               {
                  /* Make sure we have the desired handle.  May not     */
                  /* exist if this is an optional characteristic.       */
                  /* * NOTE * This handle is also used in the GATT read */
                  /*          request as the CallbackParameter, which we*/
                  /*          will use to identity the characteristic   */
                  /*          for the read response in the              */
                  /*          GATT_ClientEventCallback_IPS callback.    */
                  if(DeviceInfo->IPSClientInfo.IPS_Local_North_Coordinate)
                  {
                     /* Store the handle.                               */
                     AttributeHandle = DeviceInfo->IPSClientInfo.IPS_Local_North_Coordinate;

                     /* Send the GATT read request.                     */
                     /* * NOTE * We will not save the transactionID     */
                     /*          returned by this function, which we    */
                     /*          could use to cancel the request.       */
                     if((ret_val = GATT_Read_Value_Request(BluetoothStackID, ConnectionID, AttributeHandle, GATT_ClientEventCallback_IPS, AttributeHandle)) > 0)
                     {
                        printf("\r\nGATT Read Value Request sent:\r\n");
                        printf("   TransactionID:     %d\r\n",     ret_val);
                        printf("   Attribute Handle:  0x%04X\r\n", AttributeHandle);
                     }
                     else
                        DisplayFunctionError("GATT_Read_Value_Request", ret_val);
                  }
                  else
                     printf("\r\nCharacteristic handle has not been discovered.\r\n");
               }
               else
                  printf("\r\nService discovery has not been completed.\r\n");
            }
            else
               printf("\r\nNo device information.\r\n");
         }
         else
            printf("\r\nMust be connected to an IPS Server.\r\n");
      }
   }
   else
      printf("\r\nBluetooth stack has not been initialized.\r\n");

   return (0);
}

   /* The following function is responsible for writing the specified   */
   /* characteristic.  If this function is called by an IPS Server then */
   /* the characteristic will be written directly.  If this function is */
   /* called by the IPS Client then a GATT write or write without       */
   /* response request will be issued.  The GATT request will be        */
   /* received by the registered callback for the GATT write request.   */
static int WriteLocalNorthCoordinate(ParameterList_t *TempParam)
{
   int           ret_val;
   DeviceInfo_t *DeviceInfo;
   Word_t        AttributeHandle;
   SWord_t       LocalNorthCoordinate;
   enum
   {
      wmWrite,
      wmWriteWOResponse,
      wmReliableWrite
   } WriteMode;

   /* Make sure the stack is initialized.                               */
   if(BluetoothStackID)
   {
      /* Verify that the parameters are semi-valid.                     */
      if((TempParam) && (TempParam->NumberofParameters >= 1) && (TempParam->NumberofParameters <= 2))
      {
         /* Store the Local North Coordinate.                           */
         LocalNorthCoordinate = (SWord_t)TempParam->Params[0].intParam;

         /* Verify the local coordinate.                                */
         if((LocalNorthCoordinate >= IPS_ENCODED_LOCAL_COORDINATES_RANGE_LOWER) && (LocalNorthCoordinate <= IPS_ENCODED_LOCAL_COORDINATES_RANGE_UPPER))
         {
            /* Check if this is the IPS Server.                         */
            if(IPSInstanceID)
            {
               /* Let's call the API to write the characteristic.       */
               if((ret_val = IPS_Set_Local_North_Coordinate(BluetoothStackID, IPSInstanceID, LocalNorthCoordinate)) == 0)
               {
                  /* Print the characteristic.                          */
                  printf("\r\nLocal North Coordinate:\r\n");
                  printf("   Value:   0x%04X\r\n", LocalNorthCoordinate);
               }
               else
               {
                  /* Let's check if the characteristic is supported.    */
                  if(ret_val == IPS_ERROR_FEATURE_NOT_SUPPORTED)
                  {
                     printf("\r\nCharacteristic is not supported.\r\n");
                  }
                  else
                     DisplayFunctionError("IPS_Set_Local_North_Coordinate", ret_val);
               }
            }
            else
            {
               /* Make sure the Request Type has been supplied.         */
               if(TempParam->NumberofParameters == 2)
               {
                  /* Store the write mode.                              */
                  WriteMode = TempParam->Params[1].intParam;

                  /* We must be the client so let's make sure we are    */
                  /* connected.                                         */
                  if(ConnectionID)
                  {
                     /* Get the device information.                     */
                     if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList,  ConnectionBD_ADDR)) != NULL)
                     {
                        /* Make sure we have performed service discovery*/
                        /* so we have the attribute handle offset to    */
                        /* request.                                     */
                        if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_IPS_SERVICE_DISCOVERY_COMPLETE)
                        {
                           /* Make sure we have the desired handle.  May*/
                           /* not exist if this is an optional          */
                           /* characteristic.                           */
                           /* * NOTE * This handle is also used in the  */
                           /*          GATT read request as the         */
                           /*          CallbackParameter, which we will */
                           /*          use to identity the              */
                           /*          characteristic for the read      */
                           /*          response in the                  */
                           /*          GATT_ClientEventCallback_IPS     */
                           /*          callback.                        */
                           if(DeviceInfo->IPSClientInfo.IPS_Local_North_Coordinate)
                           {
                              /* Store the handle.                      */
                              AttributeHandle = DeviceInfo->IPSClientInfo.IPS_Local_North_Coordinate;

                              /* Determine the GATT Write type.         */
                              switch(WriteMode)
                              {
                                 case wmWrite:
                                    if(!(DeviceInfo->Flags & DEVICE_INFO_FLAGS_RELIABLE_WRITE_MODE_ENABLED))
                                    {
                                       /* Send the GATT Write Request.  */
                                       /* * NOTE * We will not save the */
                                       /*          transactionID        */
                                       /*          returned by this     */
                                       /*          function, which we   */
                                       /*          could use to cancel  */
                                       /*          the request.         */
                                       if((ret_val = GATT_Write_Request(BluetoothStackID, ConnectionID, AttributeHandle, (Word_t)NON_ALIGNED_SWORD_SIZE, &LocalNorthCoordinate, GATT_ClientEventCallback_IPS, AttributeHandle)) > 0)
                                       {
                                          printf("\r\nGATT Write Request sent:\r\n");
                                          printf("   TransactionID:     %d\r\n",     ret_val);
                                          printf("   Attribute Handle:  0x%04X\r\n", AttributeHandle);
                                       }
                                       else
                                          DisplayFunctionError("GATT_Write_Request", ret_val);
                                    }
                                    else
                                       printf("\r\nReliable write in progress. Use command 'ExecWrite' to cancel.r\n");
                                    break;
                                 case wmWriteWOResponse:
                                    if(!(DeviceInfo->Flags & DEVICE_INFO_FLAGS_RELIABLE_WRITE_MODE_ENABLED))
                                    {
                                       /* Send the GATT Write Without   */
                                       /* Response Request.             */
                                       if((ret_val = GATT_Write_Without_Response_Request(BluetoothStackID, ConnectionID, AttributeHandle, (Word_t)NON_ALIGNED_SWORD_SIZE, &LocalNorthCoordinate)) > 0)
                                       {
                                          printf("\r\nGATT Write Without Response Request sent:\r\n");
                                          printf("   Number of Bytes Written:  %u\r\n",     ret_val);
                                          printf("   Attribute Handle:         0x%04X\r\n", AttributeHandle);
                                       }
                                       else
                                          DisplayFunctionError("GATT_Write_Without_Response_Request", ret_val);
                                    }
                                    else
                                       printf("\r\nReliable write in progress. Use command 'ExecWrite' to cancel.r\n");
                                    break;
                                 case wmReliableWrite:
                                    /* Send the GATT Prepared Write     */
                                    /* Request.                         */
                                    /* * NOTE * We will not save the    */
                                    /*          transactionID returned  */
                                    /*          by this function, which */
                                    /*          we could use to cancel  */
                                    /*          the request.            */
                                    if((ret_val = GATT_Prepare_Write_Request(BluetoothStackID, ConnectionID, AttributeHandle, (Word_t)NON_ALIGNED_SWORD_SIZE, 0, &LocalNorthCoordinate, GATT_ClientEventCallback_IPS, AttributeHandle)) > 0)
                                    {
                                       printf("\r\nGATT Prepare Write Request sent:\r\n");
                                       printf("   TransactionID:     %d\r\n",     ret_val);
                                       printf("   Attribute Handle:  0x%04X\r\n", AttributeHandle);

                                       /* Enable reliable write mode.   */
                                       /* * NOTE * This can only be     */
                                       /*          disabled by calling  */
                                       /*          the                  */
                                       /*          ExecuteWriteRequest()*/
                                       /*          to write the request */
                                       /*          data or cancel the   */
                                       /*          request.             */
                                       DeviceInfo->Flags |= DEVICE_INFO_FLAGS_RELIABLE_WRITE_MODE_ENABLED;
                                    }
                                    else
                                       DisplayFunctionError("GATT_Prepare_Write_Request", ret_val);
                                    break;
                                 default:
                                   /* Do nothing.                       */
                                   break;
                              }
                           }
                           else
                              printf("\r\nCharacteristic handle has not been discovered.\r\n");
                        }
                        else
                           printf("\r\nService discovery has not been completed.\r\n");
                     }
                     else
                        printf("\r\nNo device information.\r\n");
                  }
                  else
                     printf("\r\nMust be connected to an IPS Server.\r\n");
               }
               else
               {
                  /* Display the usage.                                 */
                  DisplayLocalNorthCoordinateUsage();
               }
            }
         }
         else
            printf("\r\nLocal North Coordinate is not between -32767 to 32767 decimeters.\r\n");
      }
      else
      {
         /* Display the usage.                                          */
         DisplayLocalNorthCoordinateUsage();
      }
   }
   else
      printf("\r\nBluetooth stack has not been initialized.\r\n");

   return (0);
}

   /* The following function is responsible for reading the specified   */
   /* characteristic.  If this function is called by an IPS Server then */
   /* the characteristic will be read directly.  If this function is    */
   /* called by the IPS Client then a GATT read request will be issued. */
   /* The GATT request will be received by the registered callback when */
   /* the request is sent.                                              */
static int ReadLocalEastCoordinate(ParameterList_t *TempParam)
{
   int           ret_val;
   DeviceInfo_t *DeviceInfo;
   Word_t        AttributeHandle;
   SWord_t       LocalEastCoordinate;

   /* Make sure the stack is initialized.                               */
   if(BluetoothStackID)
   {
      /* Check if this is the IPS Server.                               */
      if(IPSInstanceID)
      {
         /* Let's call the API to query the characteristic.             */
         if((ret_val = IPS_Query_Local_East_Coordinate(BluetoothStackID, IPSInstanceID, &LocalEastCoordinate)) == 0)
         {
            /* Print the characteristic.                                */
            printf("\r\nLocal East Coordinate:\r\n");
            printf("   Value:   0x%04X\r\n", LocalEastCoordinate);
         }
         else
         {
            /* Let's check if the characteristic is supported.          */
            if(ret_val == IPS_ERROR_FEATURE_NOT_SUPPORTED)
            {
               printf("\r\nCharacteristic is not supported.\r\n");
            }
            else
               DisplayFunctionError("IPS_Query_Local_East_Coordinate", ret_val);
         }
      }
      else
      {
         /* We must be the client so let's make sure we are connected.  */
         if(ConnectionID)
         {
            /* Get the device information.                              */
            if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList,  ConnectionBD_ADDR)) != NULL)
            {
               /* Make sure we have performed service discovery so we   */
               /* have the attribute handle offset to request.          */
               if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_IPS_SERVICE_DISCOVERY_COMPLETE)
               {
                  /* Make sure we have the desired handle.  May not     */
                  /* exist if this is an optional characteristic.       */
                  /* * NOTE * This handle is also used in the GATT read */
                  /*          request as the CallbackParameter, which we*/
                  /*          will use to identity the characteristic   */
                  /*          for the read response in the              */
                  /*          GATT_ClientEventCallback_IPS callback.    */
                  if(DeviceInfo->IPSClientInfo.IPS_Local_East_Coordinate)
                  {
                     /* Store the handle.                               */
                     AttributeHandle = DeviceInfo->IPSClientInfo.IPS_Local_East_Coordinate;

                     /* Send the GATT read request.                     */
                     /* * NOTE * We will not save the transactionID     */
                     /*          returned by this function, which we    */
                     /*          could use to cancel the request.       */
                     if((ret_val = GATT_Read_Value_Request(BluetoothStackID, ConnectionID, AttributeHandle, GATT_ClientEventCallback_IPS, AttributeHandle)) > 0)
                     {
                        printf("\r\nGATT Read Value Request sent:\r\n");
                        printf("   TransactionID:     %d\r\n",     ret_val);
                        printf("   Attribute Handle:  0x%04X\r\n", AttributeHandle);
                     }
                     else
                        DisplayFunctionError("GATT_Read_Value_Request", ret_val);
                  }
                  else
                     printf("\r\nCharacteristic handle has not been discovered.\r\n");
               }
               else
                  printf("\r\nService discovery has not been completed.\r\n");
            }
            else
               printf("\r\nNo device information.\r\n");
         }
         else
            printf("\r\nMust be connected to an IPS Server.\r\n");
      }
   }
   else
      printf("\r\nBluetooth stack has not been initialized.\r\n");

   return (0);
}

   /* The following function is responsible for writing the specified   */
   /* characteristic.  If this function is called by an IPS Server then */
   /* the characteristic will be written directly.  If this function is */
   /* called by the IPS Client then a GATT write or write without       */
   /* response request will be issued.  The GATT request will be        */
   /* received by the registered callback for the GATT write request.   */
static int WriteLocalEastCoordinate(ParameterList_t *TempParam)
{
   int           ret_val;
   DeviceInfo_t *DeviceInfo;
   Word_t        AttributeHandle;
   SWord_t       LocalEastCoordinate;
   enum
   {
      wmWrite,
      wmWriteWOResponse,
      wmReliableWrite
   } WriteMode;

   /* Make sure the stack is initialized.                               */
   if(BluetoothStackID)
   {
      /* Verify that the parameters are semi-valid.                     */
      if((TempParam) && (TempParam->NumberofParameters >= 1) && (TempParam->NumberofParameters <= 2))
      {
         /* Store the Local East Coordintate.                           */
         LocalEastCoordinate = (SWord_t)TempParam->Params[0].intParam;

         /* Verify the local coordinate.                                */
         if((LocalEastCoordinate >= IPS_ENCODED_LOCAL_COORDINATES_RANGE_LOWER) && (LocalEastCoordinate <= IPS_ENCODED_LOCAL_COORDINATES_RANGE_UPPER))
         {
            /* Check if this is the IPS Server.                         */
            if(IPSInstanceID)
            {
               /* Let's call the API to write the characteristic.       */
               if((ret_val = IPS_Set_Local_East_Coordinate(BluetoothStackID, IPSInstanceID, LocalEastCoordinate)) == 0)
               {
                  /* Print the characteristic.                          */
                  printf("\r\nLocal East Coordinate:\r\n");
                  printf("   Value:   0x%04X\r\n", LocalEastCoordinate);
               }
               else
               {
                  /* Let's check if the characteristic is supported.    */
                  if(ret_val == IPS_ERROR_FEATURE_NOT_SUPPORTED)
                  {
                     printf("\r\nCharacteristic is not supported.\r\n");
                  }
                  else
                     DisplayFunctionError("IPS_Set_Local_East_Coordinate", ret_val);
               }
            }
            else
            {
               /* Make sure the Request Type has been supplied.         */
               if(TempParam->NumberofParameters == 2)
               {
                  /* Store the write mode.                              */
                  WriteMode = TempParam->Params[1].intParam;

                  /* We must be the client so let's make sure we are    */
                  /* connected.                                         */
                  if(ConnectionID)
                  {
                     /* Get the device information.                     */
                     if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList,  ConnectionBD_ADDR)) != NULL)
                     {
                        /* Make sure we have performed service discovery*/
                        /* so we have the attribute handle offset to    */
                        /* request.                                     */
                        if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_IPS_SERVICE_DISCOVERY_COMPLETE)
                        {
                           /* Make sure we have the desired handle.  May*/
                           /* not exist if this is an optional          */
                           /* characteristic.                           */
                           /* * NOTE * This handle is also used in the  */
                           /*          GATT read request as the         */
                           /*          CallbackParameter, which we will */
                           /*          use to identity the              */
                           /*          characteristic for the read      */
                           /*          response in the                  */
                           /*          GATT_ClientEventCallback_IPS     */
                           /*          callback.                        */
                           if(DeviceInfo->IPSClientInfo.IPS_Local_East_Coordinate)
                           {
                              /* Store the handle.                      */
                              AttributeHandle = DeviceInfo->IPSClientInfo.IPS_Local_East_Coordinate;

                              /* Determine the GATT Write type.         */
                              switch(WriteMode)
                              {
                                 case wmWrite:
                                    if(!(DeviceInfo->Flags & DEVICE_INFO_FLAGS_RELIABLE_WRITE_MODE_ENABLED))
                                    {
                                       /* Send the GATT Write Request.  */
                                       /* * NOTE * We will not save the */
                                       /*          transactionID        */
                                       /*          returned by this     */
                                       /*          function, which we   */
                                       /*          could use to cancel  */
                                       /*          the request.         */
                                       if((ret_val = GATT_Write_Request(BluetoothStackID, ConnectionID, AttributeHandle, (Word_t)NON_ALIGNED_SWORD_SIZE, &LocalEastCoordinate, GATT_ClientEventCallback_IPS, AttributeHandle)) > 0)
                                       {
                                          printf("\r\nGATT Write Request sent:\r\n");
                                          printf("   TransactionID:     %d\r\n",     ret_val);
                                          printf("   Attribute Handle:  0x%04X\r\n", AttributeHandle);
                                       }
                                       else
                                          DisplayFunctionError("GATT_Write_Request", ret_val);
                                    }
                                    else
                                       printf("\r\nReliable write in progress. Use command 'ExecWrite' to cancel.r\n");
                                    break;
                                 case wmWriteWOResponse:
                                    if(!(DeviceInfo->Flags & DEVICE_INFO_FLAGS_RELIABLE_WRITE_MODE_ENABLED))
                                    {
                                       /* Send the GATT Write Without   */
                                       /* Response Request.             */
                                       if((ret_val = GATT_Write_Without_Response_Request(BluetoothStackID, ConnectionID, AttributeHandle, (Word_t)NON_ALIGNED_SWORD_SIZE, &LocalEastCoordinate)) > 0)
                                       {
                                          printf("\r\nGATT Write Without Response Request sent:\r\n");
                                          printf("   Number of Bytes Written:  %u\r\n",     ret_val);
                                          printf("   Attribute Handle:         0x%04X\r\n", AttributeHandle);
                                       }
                                       else
                                          DisplayFunctionError("GATT_Write_Without_Response_Request", ret_val);
                                    }
                                    else
                                       printf("\r\nReliable write in progress. Use command 'ExecWrite' to cancel.r\n");
                                    break;
                                 case wmReliableWrite:
                                    /* Send the GATT Prepared Write     */
                                    /* Request.                         */
                                    /* * NOTE * We will not save the    */
                                    /*          transactionID returned  */
                                    /*          by this function, which */
                                    /*          we could use to cancel  */
                                    /*          the request.            */
                                    if((ret_val = GATT_Prepare_Write_Request(BluetoothStackID, ConnectionID, AttributeHandle, (Word_t)NON_ALIGNED_SWORD_SIZE, 0, &LocalEastCoordinate, GATT_ClientEventCallback_IPS, AttributeHandle)) > 0)
                                    {
                                       printf("\r\nGATT Prepare Write Request sent:\r\n");
                                       printf("   TransactionID:     %d\r\n",     ret_val);
                                       printf("   Attribute Handle:  0x%04X\r\n", AttributeHandle);

                                       /* Enable reliable write mode.   */
                                       /* * NOTE * This can only be     */
                                       /*          disabled by calling  */
                                       /*          the                  */
                                       /*          ExecuteWriteRequest()*/
                                       /*          to write the request */
                                       /*          data or cancel the   */
                                       /*          request.             */
                                       DeviceInfo->Flags |= DEVICE_INFO_FLAGS_RELIABLE_WRITE_MODE_ENABLED;
                                    }
                                    else
                                       DisplayFunctionError("GATT_Prepare_Write_Request", ret_val);
                                    break;
                                 default:
                                   /* Do nothing.                       */
                                   break;
                              }
                           }
                           else
                              printf("\r\nCharacteristic handle has not been discovered.\r\n");
                        }
                        else
                           printf("\r\nService discovery has not been completed.\r\n");
                     }
                     else
                        printf("\r\nNo device information.\r\n");
                  }
                  else
                     printf("\r\nMust be connected to an IPS Server.\r\n");
               }
               else
               {
                  /* Display the usage.                                 */
                  DisplayLocalEastCoordinateUsage();
               }
            }
         }
         else
            printf("\r\nLocal East Coordinate is not between -32767 to 32767 decimeters.\r\n");
      }
      else
      {
         /* Display the usage.                                          */
         DisplayLocalEastCoordinateUsage();
      }
   }
   else
      printf("\r\nBluetooth stack has not been initialized.\r\n");

   return (0);
}

   /* The following function is responsible for reading the specified   */
   /* characteristic.  If this function is called by an IPS Server then */
   /* the characteristic will be read directly.  If this function is    */
   /* called by the IPS Client then a GATT read request will be issued. */
   /* The GATT request will be received by the registered callback when */
   /* the request is sent.                                              */
static int ReadFloorNumber(ParameterList_t *TempParam)
{
   int           ret_val;
   DeviceInfo_t *DeviceInfo;
   Word_t        AttributeHandle;
   Byte_t        FloorNumber;

   /* Make sure the stack is initialized.                               */
   if(BluetoothStackID)
   {
      /* Check if this is the IPS Server.                               */
      if(IPSInstanceID)
      {
         /* Let's call the API to query the characteristic.             */
         if((ret_val = IPS_Query_Floor_Number(BluetoothStackID, IPSInstanceID, &FloorNumber)) == 0)
         {
            /* Decode and display the value.                            */
            printf("\r\nFloor Number:\r\n");
            switch(FloorNumber)
            {
               case 253:
                  FloorNumber = 0;
                  printf("   Value:   %d\r\n", FloorNumber);
                  break;
               case 254:
                  FloorNumber = 1;
                  printf("   Value:   %d\r\n", FloorNumber);
                  break;
               case IPS_FLOOR_NUMBER_NOT_CONFIGURED:
                  printf("   Value:   Not Configured\r\n");
                  break;
               default:
                  FloorNumber = FloorNumber - 20;
                  printf("   Value:   %d\r\n", FloorNumber);
                  break;
            }
         }
         else
         {
            /* Let's check if the characteristic is supported.          */
            if(ret_val == IPS_ERROR_FEATURE_NOT_SUPPORTED)
            {
               printf("\r\nCharacteristic is not supported.\r\n");
            }
            else
               DisplayFunctionError("IPS_Query_Floor_Number", ret_val);
         }
      }
      else
      {
         /* We must be the client so let's make sure we are connected.  */
         if(ConnectionID)
         {
            /* Get the device information.                              */
            if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList,  ConnectionBD_ADDR)) != NULL)
            {
               /* Make sure we have performed service discovery so we   */
               /* have the attribute handle offset to request.          */
               if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_IPS_SERVICE_DISCOVERY_COMPLETE)
               {
                  /* Make sure we have the desired handle.  May not     */
                  /* exist if this is an optional characteristic.       */
                  /* * NOTE * This handle is also used in the GATT read */
                  /*          request as the CallbackParameter, which we*/
                  /*          will use to identity the characteristic   */
                  /*          for the read response in the              */
                  /*          GATT_ClientEventCallback_IPS callback.    */
                  if(DeviceInfo->IPSClientInfo.IPS_Floor_Number)
                  {
                     /* Store the handle.                               */
                     AttributeHandle = DeviceInfo->IPSClientInfo.IPS_Floor_Number;

                     /* Send the GATT read request.                     */
                     /* * NOTE * We will not save the transactionID     */
                     /*          returned by this function, which we    */
                     /*          could use to cancel the request.       */
                     if((ret_val = GATT_Read_Value_Request(BluetoothStackID, ConnectionID, AttributeHandle, GATT_ClientEventCallback_IPS, AttributeHandle)) > 0)
                     {
                        printf("\r\nGATT Read Value Request sent:\r\n");
                        printf("   TransactionID:     %d\r\n",     ret_val);
                        printf("   Attribute Handle:  0x%04X\r\n", AttributeHandle);
                     }
                     else
                        DisplayFunctionError("GATT_Read_Value_Request", ret_val);
                  }
                  else
                     printf("\r\nCharacteristic handle has not been discovered.\r\n");
               }
               else
                  printf("\r\nService discovery has not been completed.\r\n");
            }
            else
               printf("\r\nNo device information.\r\n");
         }
         else
            printf("\r\nMust be connected to an IPS Server.\r\n");
      }
   }
   else
      printf("\r\nBluetooth stack has not been initialized.\r\n");

   return (0);
}

   /* The following function is responsible for writing the specified   */
   /* characteristic.  If this function is called by an IPS Server then */
   /* the characteristic will be written directly.  If this function is */
   /* called by the IPS Client then a GATT write or write without       */
   /* response request will be issued.  The GATT request will be        */
   /* received by the registered callback for the GATT write request.   */
static int WriteFloorNumber(ParameterList_t *TempParam)
{
   int           ret_val;
   DeviceInfo_t *DeviceInfo;
   Word_t        AttributeHandle;
   Byte_t        FloorNumber;
   enum
   {
      wmWrite,
      wmWriteWOResponse,
      wmReliableWrite
   } WriteMode;

   /* Make sure the stack is initialized.                               */
   if(BluetoothStackID)
   {
      /* Verify that the parameters are semi-valid.                     */
      if((TempParam) && (TempParam->NumberofParameters >= 1) && (TempParam->NumberofParameters <= 2))
      {
         /* Verify the floor number.                                    */
         /* * NOTE * The ground floor is one so we will bump zero to 1. */
         if((TempParam->Params[0].intParam >= -20) && (TempParam->Params[0].intParam <= 232))
         {
            /* Encode the Floor Number.  We will combine floors 0 and 1 */
            /* as the ground floor and set it to 254, which designates  */
            /* the ground floor as 1.                                   */
            if((TempParam->Params[0].intParam == 0) || (TempParam->Params[0].intParam == 1))
               FloorNumber = 254;
            else
               FloorNumber = (Byte_t)(TempParam->Params[0].intParam + 20);

            /* Check if this is the IPS Server.                         */
            if(IPSInstanceID)
            {
               /* Let's call the API to write the characteristic.       */
               if((ret_val = IPS_Set_Floor_Number(BluetoothStackID, IPSInstanceID, FloorNumber)) == 0)
               {
                  /* Print the characteristic.                          */
                  printf("\r\nFloor Number:\r\n");
                  printf("   Value:   0x%02X\r\n", FloorNumber);
               }
               else
               {
                  /* Let's check if the characteristic is supported.    */
                  if(ret_val == IPS_ERROR_FEATURE_NOT_SUPPORTED)
                  {
                     printf("\r\nCharacteristic is not supported.\r\n");
                  }
                  else
                     DisplayFunctionError("IPS_Set_Floor_Number", ret_val);
               }
            }
            else
            {
               /* Make sure the Request Type has been supplied.         */
               if(TempParam->NumberofParameters == 2)
               {
                  /* Store the write mode.                              */
                  WriteMode = TempParam->Params[1].intParam;

                  /* We must be the client so let's make sure we are    */
                  /* connected.                                         */
                  if(ConnectionID)
                  {
                     /* Get the device information.                     */
                     if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList,  ConnectionBD_ADDR)) != NULL)
                     {
                        /* Make sure we have performed service discovery*/
                        /* so we have the attribute handle offset to    */
                        /* request.                                     */
                        if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_IPS_SERVICE_DISCOVERY_COMPLETE)
                        {
                           /* Make sure we have the desired handle.  May*/
                           /* not exist if this is an optional          */
                           /* characteristic.                           */
                           /* * NOTE * This handle is also used in the  */
                           /*          GATT read request as the         */
                           /*          CallbackParameter, which we will */
                           /*          use to identity the              */
                           /*          characteristic for the read      */
                           /*          response in the                  */
                           /*          GATT_ClientEventCallback_IPS     */
                           /*          callback.                        */
                           if(DeviceInfo->IPSClientInfo.IPS_Floor_Number)
                           {
                              /* Store the handle.                      */
                              AttributeHandle = DeviceInfo->IPSClientInfo.IPS_Floor_Number;

                              /* Determine the GATT Write type.         */
                              switch(WriteMode)
                              {
                                 case wmWrite:
                                    if(!(DeviceInfo->Flags & DEVICE_INFO_FLAGS_RELIABLE_WRITE_MODE_ENABLED))
                                    {
                                       /* Send the GATT Write Request.  */
                                       /* * NOTE * We will not save the */
                                       /*          transactionID        */
                                       /*          returned by this     */
                                       /*          function, which we   */
                                       /*          could use to cancel  */
                                       /*          the request.         */
                                       if((ret_val = GATT_Write_Request(BluetoothStackID, ConnectionID, AttributeHandle, (Word_t)NON_ALIGNED_BYTE_SIZE, &FloorNumber, GATT_ClientEventCallback_IPS, AttributeHandle)) > 0)
                                       {
                                          printf("\r\nGATT Write Request sent:\r\n");
                                          printf("   TransactionID:     %d\r\n",     ret_val);
                                          printf("   Attribute Handle:  0x%04X\r\n", AttributeHandle);
                                       }
                                       else
                                          DisplayFunctionError("GATT_Write_Request", ret_val);
                                    }
                                    else
                                       printf("\r\nReliable write in progress. Use command 'ExecWrite' to cancel.r\n");
                                    break;
                                 case wmWriteWOResponse:
                                    if(!(DeviceInfo->Flags & DEVICE_INFO_FLAGS_RELIABLE_WRITE_MODE_ENABLED))
                                    {
                                       /* Send the GATT Write Without   */
                                       /* Response Request.             */
                                       if((ret_val = GATT_Write_Without_Response_Request(BluetoothStackID, ConnectionID, AttributeHandle, (Word_t)NON_ALIGNED_BYTE_SIZE, &FloorNumber)) > 0)
                                       {
                                          printf("\r\nGATT Write Without Response Request sent:\r\n");
                                          printf("   Number of Bytes Written:  %u\r\n",     ret_val);
                                          printf("   Attribute Handle:         0x%04X\r\n", AttributeHandle);
                                       }
                                       else
                                          DisplayFunctionError("GATT_Write_Without_Response_Request", ret_val);
                                    }
                                    else
                                       printf("\r\nReliable write in progress. Use command 'ExecWrite' to cancel.r\n");
                                    break;
                                 case wmReliableWrite:
                                    /* Send the GATT Prepared Write     */
                                    /* Request.                         */
                                    /* * NOTE * We will not save the    */
                                    /*          transactionID returned  */
                                    /*          by this function, which */
                                    /*          we could use to cancel  */
                                    /*          the request.            */
                                    if((ret_val = GATT_Prepare_Write_Request(BluetoothStackID, ConnectionID, AttributeHandle, (Word_t)NON_ALIGNED_BYTE_SIZE, 0, &FloorNumber, GATT_ClientEventCallback_IPS, AttributeHandle)) > 0)
                                    {
                                       printf("\r\nGATT Prepare Write Request sent:\r\n");
                                       printf("   TransactionID:     %d\r\n",     ret_val);
                                       printf("   Attribute Handle:  0x%04X\r\n", AttributeHandle);

                                       /* Enable reliable write mode.   */
                                       /* * NOTE * This can only be     */
                                       /*          disabled by calling  */
                                       /*          the                  */
                                       /*          ExecuteWriteRequest()*/
                                       /*          to write the request */
                                       /*          data or cancel the   */
                                       /*          request.             */
                                       DeviceInfo->Flags |= DEVICE_INFO_FLAGS_RELIABLE_WRITE_MODE_ENABLED;
                                    }
                                    else
                                       DisplayFunctionError("GATT_Prepare_Write_Request", ret_val);
                                    break;
                                 default:
                                   /* Do nothing.                       */
                                   break;
                              }
                           }
                           else
                              printf("\r\nCharacteristic handle has not been discovered.\r\n");
                        }
                        else
                           printf("\r\nService discovery has not been completed.\r\n");
                     }
                     else
                        printf("\r\nNo device information.\r\n");
                  }
                  else
                     printf("\r\nMust be connected to an IPS Server.\r\n");
               }
               else
               {
                  /* Display the usage.                                 */
                  DisplayFloorNumberUsage();
               }
            }
         }
         else
            printf("\r\nFloor Number is not between 1 and 232.\r\n");
      }
      else
      {
         /* Display the usage.                                          */
         DisplayFloorNumberUsage();
      }
   }
   else
      printf("\r\nBluetooth stack has not been initialized.\r\n");

   return (0);
}

   /* The following function is responsible for reading the specified   */
   /* characteristic.  If this function is called by an IPS Server then */
   /* the characteristic will be read directly.  If this function is    */
   /* called by the IPS Client then a GATT read request will be issued. */
   /* The GATT request will be received by the registered callback when */
   /* the request is sent.                                              */
static int ReadAltitude(ParameterList_t *TempParam)
{
   int           ret_val;
   DeviceInfo_t *DeviceInfo;
   Word_t        AttributeHandle;
   SDWord_t      Altitude;

   /* Make sure the stack is initialized.                               */
   if(BluetoothStackID)
   {
      /* Check if this is the IPS Server.                               */
      if(IPSInstanceID)
      {
         /* Let's call the API to query the characteristic.             */
         if((ret_val = IPS_Query_Altitude(BluetoothStackID, IPSInstanceID, (Word_t *)&Altitude)) == 0)
         {
            /* Print the characteristic.                                */
            printf("\r\nAltitude:\r\n");

            if(Altitude != (SDWord_t)IPS_ALTITUDE_NOT_CONFIGURED)
            {
               /* Decode and display the Altitude.                      */
               Altitude = Altitude - 1000;
               printf("   Value:   %d (decimeters)\r\n", Altitude);
            }
            else
               printf("   Value:   Not Configured.\r\n");
         }
         else
         {
            /* Let's check if the characteristic is supported.          */
            if(ret_val == IPS_ERROR_FEATURE_NOT_SUPPORTED)
            {
               printf("\r\nCharacteristic is not supported.\r\n");
            }
            else
               DisplayFunctionError("IPS_Query_Altitude", ret_val);
         }
      }
      else
      {
         /* We must be the client so let's make sure we are connected.  */
         if(ConnectionID)
         {
            /* Get the device information.                              */
            if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList,  ConnectionBD_ADDR)) != NULL)
            {
               /* Make sure we have performed service discovery so we   */
               /* have the attribute handle offset to request.          */
               if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_IPS_SERVICE_DISCOVERY_COMPLETE)
               {
                  /* Make sure we have the desired handle.  May not     */
                  /* exist if this is an optional characteristic.       */
                  /* * NOTE * This handle is also used in the GATT read */
                  /*          request as the CallbackParameter, which we*/
                  /*          will use to identity the characteristic   */
                  /*          for the read response in the              */
                  /*          GATT_ClientEventCallback_IPS callback.    */
                  if(DeviceInfo->IPSClientInfo.IPS_Altitude)
                  {
                     /* Store the handle.                               */
                     AttributeHandle = DeviceInfo->IPSClientInfo.IPS_Altitude;

                     /* Send the GATT read request.                     */
                     /* * NOTE * We will not save the transactionID     */
                     /*          returned by this function, which we    */
                     /*          could use to cancel the request.       */
                     if((ret_val = GATT_Read_Value_Request(BluetoothStackID, ConnectionID, AttributeHandle, GATT_ClientEventCallback_IPS, AttributeHandle)) > 0)
                     {
                        printf("\r\nGATT Read Value Request sent:\r\n");
                        printf("   TransactionID:     %d\r\n",     ret_val);
                        printf("   Attribute Handle:  0x%04X\r\n", AttributeHandle);
                     }
                     else
                        DisplayFunctionError("GATT_Read_Value_Request", ret_val);
                  }
                  else
                     printf("\r\nCharacteristic handle has not been discovered.\r\n");
               }
               else
                  printf("\r\nService discovery has not been completed.\r\n");
            }
            else
               printf("\r\nNo device information.\r\n");
         }
         else
            printf("\r\nMust be connected to an IPS Server.\r\n");
      }
   }
   else
      printf("\r\nBluetooth stack has not been initialized.\r\n");

   return (0);
}

   /* The following function is responsible for writing the specified   */
   /* characteristic.  If this function is called by an IPS Server then */
   /* the characteristic will be written directly.  If this function is */
   /* called by the IPS Client then a GATT write or write without       */
   /* response request will be issued.  The GATT request will be        */
   /* received by the registered callback for the GATT write request.   */
static int WriteAltitude(ParameterList_t *TempParam)
{
   int           ret_val;
   DeviceInfo_t *DeviceInfo;
   Word_t        AttributeHandle;
   Word_t        Altitude;
   enum
   {
      wmWrite,
      wmWriteWOResponse,
      wmReliableWrite
   } WriteMode;

   /* Make sure the stack is initialized.                               */
   if(BluetoothStackID)
   {
      /* Verify that the parameters are semi-valid.                     */
      if((TempParam) && (TempParam->NumberofParameters >= 1) && (TempParam->NumberofParameters <= 2))
      {
         /* Verify the Altitude.                                        */
         if((TempParam->Params[0].intParam >= -1000) && (TempParam->Params[0].intParam <= 64534))
         {
            /* Encode the Altitude.                                     */
            Altitude = (Word_t)(TempParam->Params[0].intParam + 1000);

            /* Check if this is the IPS Server.                         */
            if(IPSInstanceID)
            {
               /* Let's call the API to write the characteristic.       */
               if((ret_val = IPS_Set_Altitude(BluetoothStackID, IPSInstanceID, Altitude)) == 0)
               {
                  /* Print the characteristic.                          */
                  printf("\r\nAltitude:\r\n");
                  printf("   Value:   0x%04X\r\n", Altitude);
               }
               else
               {
                  /* Let's check if the characteristic is supported.    */
                  if(ret_val == IPS_ERROR_FEATURE_NOT_SUPPORTED)
                  {
                     printf("\r\nCharacteristic is not supported.\r\n");
                  }
                  else
                     DisplayFunctionError("IPS_Set_Altitude", ret_val);
               }
            }
            else
            {
               /* Make sure the Request Type has been supplied.         */
               if(TempParam->NumberofParameters == 2)
               {
                  /* Store the write mode.                              */
                  WriteMode = TempParam->Params[1].intParam;

                  /* We must be the client so let's make sure we are    */
                  /* connected.                                         */
                  if(ConnectionID)
                  {
                     /* Get the device information.                     */
                     if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList,  ConnectionBD_ADDR)) != NULL)
                     {
                        /* Make sure we have performed service discovery*/
                        /* so we have the attribute handle offset to    */
                        /* request.                                     */
                        if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_IPS_SERVICE_DISCOVERY_COMPLETE)
                        {
                           /* Make sure we have the desired handle.  May*/
                           /* not exist if this is an optional          */
                           /* characteristic.                           */
                           /* * NOTE * This handle is also used in the  */
                           /*          GATT read request as the         */
                           /*          CallbackParameter, which we will */
                           /*          use to identity the              */
                           /*          characteristic for the read      */
                           /*          response in the                  */
                           /*          GATT_ClientEventCallback_IPS     */
                           /*          callback.                        */
                           if(DeviceInfo->IPSClientInfo.IPS_Altitude)
                           {
                              /* Store the handle.                      */
                              AttributeHandle = DeviceInfo->IPSClientInfo.IPS_Altitude;

                              /* Determine the GATT Write type.         */
                              switch(WriteMode)
                              {
                                 case wmWrite:
                                    if(!(DeviceInfo->Flags & DEVICE_INFO_FLAGS_RELIABLE_WRITE_MODE_ENABLED))
                                    {
                                       /* Send the GATT Write Request.  */
                                       /* * NOTE * We will not save the */
                                       /*          transactionID        */
                                       /*          returned by this     */
                                       /*          function, which we   */
                                       /*          could use to cancel  */
                                       /*          the request.         */
                                       if((ret_val = GATT_Write_Request(BluetoothStackID, ConnectionID, AttributeHandle, (Word_t)NON_ALIGNED_WORD_SIZE, &Altitude, GATT_ClientEventCallback_IPS, AttributeHandle)) > 0)
                                       {
                                          printf("\r\nGATT Write Request sent:\r\n");
                                          printf("   TransactionID:     %d\r\n",     ret_val);
                                          printf("   Attribute Handle:  0x%04X\r\n", AttributeHandle);
                                       }
                                       else
                                          DisplayFunctionError("GATT_Write_Request", ret_val);
                                    }
                                    else
                                       printf("\r\nReliable write in progress. Use command 'ExecWrite' to cancel.r\n");
                                    break;
                                 case wmWriteWOResponse:
                                    if(!(DeviceInfo->Flags & DEVICE_INFO_FLAGS_RELIABLE_WRITE_MODE_ENABLED))
                                    {
                                       /* Send the GATT Write Without   */
                                       /* Response Request.             */
                                       if((ret_val = GATT_Write_Without_Response_Request(BluetoothStackID, ConnectionID, AttributeHandle, (Word_t)NON_ALIGNED_WORD_SIZE, &Altitude)) > 0)
                                       {
                                          printf("\r\nGATT Write Without Response Request sent:\r\n");
                                          printf("   Number of Bytes Written:  %u\r\n",     ret_val);
                                          printf("   Attribute Handle:         0x%04X\r\n", AttributeHandle);
                                       }
                                       else
                                          DisplayFunctionError("GATT_Write_Without_Response_Request", ret_val);
                                    }
                                    else
                                       printf("\r\nReliable write in progress. Use command 'ExecWrite' to cancel.r\n");
                                    break;
                                 case wmReliableWrite:
                                    /* Send the GATT Prepared Write     */
                                    /* Request.                         */
                                    /* * NOTE * We will not save the    */
                                    /*          transactionID returned  */
                                    /*          by this function, which */
                                    /*          we could use to cancel  */
                                    /*          the request.            */
                                    if((ret_val = GATT_Prepare_Write_Request(BluetoothStackID, ConnectionID, AttributeHandle, (Word_t)NON_ALIGNED_WORD_SIZE, 0, &Altitude, GATT_ClientEventCallback_IPS, AttributeHandle)) > 0)
                                    {
                                       printf("\r\nGATT Prepare Write Request sent:\r\n");
                                       printf("   TransactionID:     %d\r\n",     ret_val);
                                       printf("   Attribute Handle:  0x%04X\r\n", AttributeHandle);

                                       /* Enable reliable write mode.   */
                                       /* * NOTE * This can only be     */
                                       /*          disabled by calling  */
                                       /*          the                  */
                                       /*          ExecuteWriteRequest()*/
                                       /*          to write the request */
                                       /*          data or cancel the   */
                                       /*          request.             */
                                       DeviceInfo->Flags |= DEVICE_INFO_FLAGS_RELIABLE_WRITE_MODE_ENABLED;
                                    }
                                    else
                                       DisplayFunctionError("GATT_Prepare_Write_Request", ret_val);
                                    break;
                                 default:
                                   /* Do nothing.                       */
                                   break;
                              }
                           }
                           else
                              printf("\r\nCharacteristic handle has not been discovered.\r\n");
                        }
                        else
                           printf("\r\nService discovery has not been completed.\r\n");
                     }
                     else
                        printf("\r\nNo device information.\r\n");
                  }
                  else
                     printf("\r\nMust be connected to an IPS Server.\r\n");
               }
               else
               {
                  /* Display the usage.                                 */
                  DisplayAltitudeUsage();
               }
            }
         }
         else
            printf("\r\nAltitude not between -1000 to 64534 decimenters.\r\n");
      }
      else
      {
        /* Display the usage.                                           */
         DisplayAltitudeUsage();
      }
   }
   else
      printf("\r\nBluetooth stack has not been initialized.\r\n");

   return (0);
}

   /* The following function is responsible for reading the specified   */
   /* characteristic.  If this function is called by an IPS Server then */
   /* the characteristic will be read directly.  If this function is    */
   /* called by the IPS Client then a GATT read request will be issued. */
   /* The GATT request will be received by the registered callback when */
   /* the request is sent.                                              */
static int ReadUncertainty(ParameterList_t *TempParam)
{
   int           ret_val;
   DeviceInfo_t *DeviceInfo;
   Word_t        AttributeHandle;
   Byte_t        Uncertainty;

   /* Make sure the stack is initialized.                               */
   if(BluetoothStackID)
   {
      /* Check if this is the IPS Server.                               */
      if(IPSInstanceID)
      {
         /* Let's call the API to query the characteristic.             */
         if((ret_val = IPS_Query_Uncertainty(BluetoothStackID, IPSInstanceID, &Uncertainty)) == 0)
         {
            /* Print the characteristic.                                */
            printf("\r\nUncertainty:\r\n");
            DisplayUncertaintyCharacteristic(Uncertainty);
         }
         else
         {
            /* Let's check if the characteristic is supported.          */
            if(ret_val == IPS_ERROR_FEATURE_NOT_SUPPORTED)
            {
               printf("\r\nCharacteristic is not supported.\r\n");
            }
            else
               DisplayFunctionError("IPS_Query_Uncertainty", ret_val);
         }
      }
      else
      {
         /* We must be the client so let's make sure we are connected.  */
         if(ConnectionID)
         {
            /* Get the device information.                              */
            if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList,  ConnectionBD_ADDR)) != NULL)
            {
               /* Make sure we have performed service discovery so we   */
               /* have the attribute handle offset to request.          */
               if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_IPS_SERVICE_DISCOVERY_COMPLETE)
               {
                  /* Make sure we have the desired handle.  May not     */
                  /* exist if this is an optional characteristic.       */
                  /* * NOTE * This handle is also used in the GATT read */
                  /*          request as the CallbackParameter, which we*/
                  /*          will use to identity the characteristic   */
                  /*          for the read response in the              */
                  /*          GATT_ClientEventCallback_IPS callback.    */
                  if(DeviceInfo->IPSClientInfo.IPS_Uncertainty)
                  {
                     /* Store the handle.                               */
                     AttributeHandle = DeviceInfo->IPSClientInfo.IPS_Uncertainty;

                     /* Send the GATT read request.                     */
                     /* * NOTE * We will not save the transactionID     */
                     /*          returned by this function, which we    */
                     /*          could use to cancel the request.       */
                     if((ret_val = GATT_Read_Value_Request(BluetoothStackID, ConnectionID, AttributeHandle, GATT_ClientEventCallback_IPS, AttributeHandle)) > 0)
                     {
                        printf("\r\nGATT Read Value Request sent:\r\n");
                        printf("   TransactionID:     %d\r\n",     ret_val);
                        printf("   Attribute Handle:  0x%04X\r\n", AttributeHandle);
                     }
                     else
                        DisplayFunctionError("GATT_Read_Value_Request", ret_val);
                  }
                  else
                     printf("\r\nCharacteristic handle has not been discovered.\r\n");
               }
               else
                  printf("\r\nService discovery has not been completed.\r\n");
            }
            else
               printf("\r\nNo device information.\r\n");
         }
         else
            printf("\r\nMust be connected to an IPS Server.\r\n");
      }
   }
   else
      printf("\r\nBluetooth stack has not been initialized.\r\n");

   return (0);
}

   /* The following function is responsible for writing the specified   */
   /* characteristic.  If this function is called by an IPS Server then */
   /* the characteristic will be written directly.  If this function is */
   /* called by the IPS Client then a GATT write or write without       */
   /* response request will be issued.  The GATT request will be        */
   /* received by the registered callback for the GATT write request.   */
static int WriteUncertainty(ParameterList_t *TempParam)
{
   int           ret_val;
   DeviceInfo_t *DeviceInfo;
   Word_t        AttributeHandle;
   Word_t        Uncertainty;
   enum
   {
      wmWrite,
      wmWriteWOResponse,
      wmReliableWrite
   } WriteMode;

   /* Make sure the stack is initialized.                               */
   if(BluetoothStackID)
   {
      /* Verify that the parameters are semi-valid.                     */
      if((TempParam) && (TempParam->NumberofParameters >= 3) && (TempParam->NumberofParameters <= 4))
      {
         /* Set the uncertainty bit mask.                               */
         /* * NOTE * The IPS Server should ignore requests to update the*/
         /*          Update Time bits of the bit mask.                  */
         Uncertainty = (Byte_t)TempParam->Params[0].intParam;
         IPS_SET_UNCERTAINTY_UPDATE_TIME_BITS(Uncertainty, (Byte_t)TempParam->Params[1].intParam);
         IPS_SET_UNCERTAINTY_PRECISION_BITS(Uncertainty, (Byte_t)TempParam->Params[2].intParam);

         /* Check if this is the IPS Server.                            */
         if(IPSInstanceID)
         {
            /* Let's call the API to write the characteristic.          */
            if((ret_val = IPS_Set_Uncertainty(BluetoothStackID, IPSInstanceID, Uncertainty)) == 0)
            {
               /* Print the characteristic.                             */
               printf("\r\nUncertainty:\r\n");
               printf("   Value:   0x%02X\r\n", Uncertainty);
            }
            else
            {
               /* Let's check if the characteristic is supported.       */
               if(ret_val == IPS_ERROR_FEATURE_NOT_SUPPORTED)
               {
                  printf("\r\nCharacteristic is not supported.\r\n");
               }
               else
                  DisplayFunctionError("IPS_Set_Uncertainty", ret_val);
            }
         }
         else
         {
            /* Make sure the Request Type has been supplied.            */
            if(TempParam->NumberofParameters == 4)
            {
               /* Store the write mode.                                 */
               WriteMode = TempParam->Params[3].intParam;

               /* We must be the client so let's make sure we are       */
               /* connected.                                            */
               if(ConnectionID)
               {
                  /* Get the device information.                        */
                  if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList,  ConnectionBD_ADDR)) != NULL)
                  {
                     /* Make sure we have performed service discovery so*/
                     /* we have the attribute handle offset to request. */
                     if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_IPS_SERVICE_DISCOVERY_COMPLETE)
                     {
                        /* Make sure we have the desired handle.  May   */
                        /* not exist if this is an optional             */
                        /* characteristic.                              */
                        /* * NOTE * This handle is also used in the GATT*/
                        /*          read request as the                 */
                        /*          CallbackParameter, which we will use*/
                        /*          to identity the characteristic for  */
                        /*          the read response in the            */
                        /*          GATT_ClientEventCallback_IPS        */
                        /*          callback.                           */
                        if(DeviceInfo->IPSClientInfo.IPS_Uncertainty)
                        {
                           /* Store the handle.                         */
                           AttributeHandle = DeviceInfo->IPSClientInfo.IPS_Uncertainty;

                           /* Determine the GATT Write type.            */
                           switch(WriteMode)
                           {
                              case wmWrite:
                                 if(!(DeviceInfo->Flags & DEVICE_INFO_FLAGS_RELIABLE_WRITE_MODE_ENABLED))
                                 {
                                    /* Send the GATT Write Request.     */
                                    /* * NOTE * We will not save the    */
                                    /*          transactionID returned  */
                                    /*          by this function, which */
                                    /*          we could use to cancel  */
                                    /*          the request.            */
                                    if((ret_val = GATT_Write_Request(BluetoothStackID, ConnectionID, AttributeHandle, (Word_t)NON_ALIGNED_BYTE_SIZE, &Uncertainty, GATT_ClientEventCallback_IPS, AttributeHandle)) > 0)
                                    {
                                       printf("\r\nGATT Write Request sent:\r\n");
                                       printf("   TransactionID:     %d\r\n",     ret_val);
                                       printf("   Attribute Handle:  0x%04X\r\n", AttributeHandle);
                                    }
                                    else
                                       DisplayFunctionError("GATT_Write_Request", ret_val);
                                 }
                                 else
                                    printf("\r\nReliable write in progress. Use command 'ExecWrite' to cancel.r\n");
                                 break;
                              case wmWriteWOResponse:
                                 if(!(DeviceInfo->Flags & DEVICE_INFO_FLAGS_RELIABLE_WRITE_MODE_ENABLED))
                                 {
                                    /* Send the GATT Write Without      */
                                    /* Response Request.                */
                                    if((ret_val = GATT_Write_Without_Response_Request(BluetoothStackID, ConnectionID, AttributeHandle, (Word_t)NON_ALIGNED_BYTE_SIZE, &Uncertainty)) > 0)
                                    {
                                       printf("\r\nGATT Write Without Response Request sent:\r\n");
                                       printf("   Number of Bytes Written:  %u\r\n",     ret_val);
                                       printf("   Attribute Handle:         0x%04X\r\n", AttributeHandle);
                                    }
                                    else
                                       DisplayFunctionError("GATT_Write_Without_Response_Request", ret_val);
                                 }
                                 else
                                    printf("\r\nReliable write in progress. Use command 'ExecWrite' to cancel.r\n");
                                 break;
                              case wmReliableWrite:
                                 /* Send the GATT Prepared Write        */
                                 /* Request.                            */
                                 /* * NOTE * We will not save the       */
                                 /*          transactionID returned by  */
                                 /*          this function, which we    */
                                 /*          could use to cancel the    */
                                 /*          request.                   */
                                 if((ret_val = GATT_Prepare_Write_Request(BluetoothStackID, ConnectionID, AttributeHandle, (Word_t)NON_ALIGNED_BYTE_SIZE, 0, &Uncertainty, GATT_ClientEventCallback_IPS, AttributeHandle)) > 0)
                                 {
                                    printf("\r\nGATT Prepare Write Request sent:\r\n");
                                    printf("   TransactionID:     %d\r\n",     ret_val);
                                    printf("   Attribute Handle:  0x%04X\r\n", AttributeHandle);

                                    /* Enable reliable write mode.      */
                                    /* * NOTE * This can only be        */
                                    /*          disabled by calling the */
                                    /*          ExecuteWriteRequest() to*/
                                    /*          write the request data  */
                                    /*          or cancel the request.  */
                                    DeviceInfo->Flags |= DEVICE_INFO_FLAGS_RELIABLE_WRITE_MODE_ENABLED;
                                 }
                                 else
                                    DisplayFunctionError("GATT_Prepare_Write_Request", ret_val);
                                 break;
                              default:
                                /* Do nothing.                          */
                                break;
                           }
                        }
                        else
                           printf("\r\nCharacteristic handle has not been discovered.\r\n");
                     }
                     else
                        printf("\r\nService discovery has not been completed.\r\n");
                  }
                  else
                     printf("\r\nNo device information.\r\n");
               }
               else
                  printf("\r\nMust be connected to an IPS Server.\r\n");
            }
            else
            {
               /* Display the usage.                                    */
               DisplayUncertaintyUsage();
            }
         }
      }
      else
      {
         /* Display the usage.                                          */
         DisplayUncertaintyUsage();
      }
   }
   else
      printf("\r\nBluetooth stack has not been initialized.\r\n");

   return (0);
}

   /* The following function is responsible for reading the specified   */
   /* characteristic.  If this function is called by an IPS Server then */
   /* the characteristic will be read directly.  If this function is    */
   /* called by the IPS Client then a GATT read request will be issued. */
   /* The GATT request will be received by the registered callback when */
   /* the request is sent.                                              */
static int ReadLocationName(ParameterList_t *TempParam)
{
   int           ret_val;
   DeviceInfo_t *DeviceInfo;
   Word_t        AttributeHandle;
   char          LocationName[BTPS_CONFIGURATION_IPS_MAXIMUM_SUPPORTED_STRING_LENGTH+1];
   enum
   {
      rmRead,
      rmReadLong
   } ReadMode;

   /* Make sure the stack is initialized.                               */
   if(BluetoothStackID)
   {
      /* Check if this is the IPS Server.                               */
      if(IPSInstanceID)
      {
         /* Let's call the API to query the characteristic.             */
         /* * NOTE * We will add one to account for the NULL terminator */
         /*          that will automatically be set by this function.   */
         if((ret_val = IPS_Query_Location_Name(BluetoothStackID, IPSInstanceID, (unsigned int)(BTPS_CONFIGURATION_IPS_MAXIMUM_SUPPORTED_STRING_LENGTH+1), LocationName)) == 0)
         {
            /* Print the characteristic.                                */
            printf("\r\nLocation Name:\r\n");
            printf("   Value:   %s\r\n", LocationName);
         }
         else
         {
            /* Let's check if the characteristic is supported.          */
            if(ret_val == IPS_ERROR_FEATURE_NOT_SUPPORTED)
            {
               printf("\r\nCharacteristic is not supported.\r\n");
            }
            else
            {
               if(ret_val != IPS_ERROR_LOCATION_NAME_NOT_CONFIGURED)
               {
                  DisplayFunctionError("IPS_Query_Location_Name", ret_val);
               }
               else
               {
                  printf("\r\nLocation Name:\r\n");
                  printf("   Value:   %s\r\n", "Not Configured.");
               }
            }
         }
      }
      else
      {
         /* Make sure the parameters are semi-valid.                    */
         if((TempParam) && (TempParam->NumberofParameters == 1))
         {
            /* Store the parameters.                                    */
            ReadMode = TempParam->Params[0].intParam;

            /* We must be the client so let's make sure we are          */
            /* connected.                                               */
            if(ConnectionID)
            {
               /* Get the device information.                           */
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
               {
                  /* Make sure we have performed service discovery so we*/
                  /* have the attribute handle offset to request.       */
                  if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_IPS_SERVICE_DISCOVERY_COMPLETE)
                  {
                     /* Make sure we have the desired handle.  May not  */
                     /* exist if this is an optional characteristic.    */
                     /* * NOTE * This handle is also used in the GATT   */
                     /*          read request as the CallbackParameter, */
                     /*          which we will use to identity the      */
                     /*          characteristic for the read response in*/
                     /*          the GATT_ClientEventCallback_IPS       */
                     /*          callback.                              */
                     if(DeviceInfo->IPSClientInfo.IPS_Location_Name)
                     {
                        /* Store the handle.                            */
                        AttributeHandle = DeviceInfo->IPSClientInfo.IPS_Location_Name;

                        switch(ReadMode)
                        {
                           case rmRead:
                              /* Reset the location name offset.        */
                              DeviceInfo->LocationNameOffset = 0;

                              /* Send the GATT read request.            */
                              /* * NOTE * We will not save the          */
                              /*          transactionID returned by this*/
                              /*          function, which we could use  */
                              /*          to cancel the request.        */
                              if((ret_val = GATT_Read_Value_Request(BluetoothStackID, ConnectionID, AttributeHandle, GATT_ClientEventCallback_IPS, AttributeHandle)) > 0)
                              {
                                 printf("\r\nGATT Read Value Request sent:\r\n");
                                 printf("   TransactionID:     %d\r\n",     ret_val);
                                 printf("   Attribute Handle:  0x%04X\r\n", AttributeHandle);
                              }
                              else
                                 DisplayFunctionError("GATT_Read_Value_Request", ret_val);
                              break;
                           case rmReadLong:
                              /* Send the GATT read long request.       */
                              /* * NOTE * We will not save the          */
                              /*          transactionID returned by this*/
                              /*          function, which we could use  */
                              /*          to cancel the request.        */
                              if((ret_val = GATT_Read_Long_Value_Request(BluetoothStackID, ConnectionID, AttributeHandle, DeviceInfo->LocationNameOffset, GATT_ClientEventCallback_IPS, AttributeHandle)) > 0)
                              {
                                 printf("\r\nGATT Read Long Value Request sent:\r\n");
                                 printf("   TransactionID:     %d\r\n",     ret_val);
                                 printf("   Attribute Handle:  0x%04X\r\n", AttributeHandle);
                              }
                              else
                                 DisplayFunctionError("GATT_Read_Value_Request", ret_val);
                              break;
                           default:
                              printf("\r\nInvalid request type.\r\n");
                        }

                     }
                     else
                        printf("\r\nCharacteristic handle has not been discovered.\r\n");
                  }
                  else
                     printf("\r\nService discovery has not been completed.\r\n");
               }
               else
                  printf("\r\nNo device information.\r\n");
            }
            else
               printf("\r\nMust be connected to an IPS Server.\r\n");
         }
         else
         {
            /* Display the usage.                                       */
            printf("\r\nUsage: ReadLocationName (Client ONLY)[GATT Request Type(Boolean)]\r\n");
            printf("\r\nWhere GATT Request Type is:\r\n");
            printf("\r\n   Request Type:\r\n");
            printf("      0 = Read.\r\n");
            printf("      1 = Read Long.\r\n");
            printf("\r\nNOTE: To read a long User Description descriptor, the standard read MUST be used first, followed by read longs for subsequent requests.\r\n");
         }
      }
   }
   else
      printf("\r\nBluetooth stack has not been initialized.\r\n");

   return (0);
}

   /* The following function is responsible for reading the specified   */
   /* characteristic.  If this function is called by an IPS Server then */
   /* the characteristic will be read directly.  If this function is    */
   /* called by the IPS Client then a GATT read request will be issued. */
   /* The GATT request will be received by the registered callback when */
   /* the request is sent.                                              */
static int WriteLocationName(ParameterList_t *TempParam)
{
   int           ret_val;
   DeviceInfo_t *DeviceInfo;
   Word_t        AttributeHandle;
   char          LocationName[BTPS_CONFIGURATION_IPS_MAXIMUM_SUPPORTED_STRING_LENGTH];
   Byte_t        StringLength;
   Word_t        MTU;
   enum
   {
      wmWrite,
      wmWriteWOResponse,
      wmReliableWrite
   } WriteMode;

   /* Make sure the stack is initialized.                               */
   if(BluetoothStackID)
   {
      /* Verify that the parameters are semi-valid.                     */
      if((TempParam) && (TempParam->NumberofParameters >= 1) && (TempParam->NumberofParameters <= 2))
      {
         /* Make sure the string length is valid.  We will not support  */
         /* strings that are over                                       */
         /* BTPS_CONFIGURATION_IPS_MAXIMUM_SUPPORTED_STRING_LENGTH.     */
         if((BTPS_StringLength(TempParam->Params[0].strParam)+1) <= BTPS_CONFIGURATION_IPS_MAXIMUM_SUPPORTED_STRING_LENGTH)
         {
            /* Store the parameters.                                    */
            /* * NOTE * We will account for the NULL terminator in the  */
            /*          length.                                         */
            BTPS_StringCopy(LocationName, TempParam->Params[0].strParam);
            StringLength = BTPS_StringLength(LocationName)+1;

            /* Check if this is the IPS Server.                         */
            if(IPSInstanceID)
            {
               /* Let's call the API to write the characteristic.       */
               if((ret_val = IPS_Set_Location_Name(BluetoothStackID, IPSInstanceID, LocationName)) == 0)
               {
                  /* Print the characteristic.                          */
                  printf("\r\nLocation Name:\r\n");
                  printf("   Value:   %s\r\n", LocationName);
               }
               else
               {
                  /* Let's check if the characteristic is supported.    */
                  if(ret_val == IPS_ERROR_FEATURE_NOT_SUPPORTED)
                  {
                     printf("\r\nCharacteristic is not supported.\r\n");
                  }
                  else
                     DisplayFunctionError("IPS_Set_Location_Name", ret_val);
               }
            }
            else
            {
               /* Make sure the Request Type has been supplied.         */
               if(TempParam->NumberofParameters == 2)
               {
                  /* Store the write mode.                              */
                  WriteMode = TempParam->Params[1].intParam;

                  /* We must be the client so let's make sure we are    */
                  /* connected.                                         */
                  if(ConnectionID)
                  {
                     /* Get the device information.                     */
                     if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList,  ConnectionBD_ADDR)) != NULL)
                     {
                        /* Make sure we have performed service discovery*/
                        /* so we have the attribute handle offset to    */
                        /* request.                                     */
                        if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_IPS_SERVICE_DISCOVERY_COMPLETE)
                        {
                           /* Make sure we have the desired handle.  May*/
                           /* not exist if this is an optional          */
                           /* characteristic.                           */
                           /* * NOTE * This handle is also used in the  */
                           /*          GATT read request as the         */
                           /*          CallbackParameter, which we will */
                           /*          use to identity the              */
                           /*          characteristic for the read      */
                           /*          response in the                  */
                           /*          GATT_ClientEventCallback_IPS     */
                           /*          callback.                        */
                           if(DeviceInfo->IPSClientInfo.IPS_Location_Name)
                           {
                              /* Store the handle.                      */
                              AttributeHandle = DeviceInfo->IPSClientInfo.IPS_Location_Name;

                              /* Determine the GATT Write type.         */
                              switch(WriteMode)
                              {
                                 case wmWrite:
                                    if(!(DeviceInfo->Flags & DEVICE_INFO_FLAGS_RELIABLE_WRITE_MODE_ENABLED))
                                    {
                                       /* Query the GATT Connection MTU */
                                       /* to determine how we should    */
                                       /* handle the write request.     */
                                       if((ret_val = GATT_Query_Connection_MTU(BluetoothStackID, ConnectionID, &MTU)) == 0)
                                       {
                                          /* Determine the type of      */
                                          /* write.                     */
                                          if(StringLength <= (MTU-3))
                                          {
                                             /* Send the GATT Write     */
                                             /* Request.                */
                                             /* * NOTE * We will not    */
                                             /*          save the       */
                                             /*          transactionID  */
                                             /*          returned by    */
                                             /*          this function, */
                                             /*          which we could */
                                             /*          use to cancel  */
                                             /*          the request.   */
                                             if((ret_val = GATT_Write_Request(BluetoothStackID, ConnectionID, AttributeHandle, (Word_t)StringLength, LocationName, GATT_ClientEventCallback_IPS, AttributeHandle)) > 0)
                                             {
                                                printf("\r\nGATT Write Request sent:\r\n");
                                                printf("   TransactionID:     %d\r\n",     ret_val);
                                                printf("   Attribute Handle:  0x%04X\r\n", AttributeHandle);
                                             }
                                             else
                                                DisplayFunctionError("GATT_Write_Request", ret_val);
                                          }
                                          else
                                          {
                                             /* Store the string so we  */
                                             /* can send the rest of it */
                                             /* when we receive the     */
                                             /* prepare write response. */
                                             BTPS_StringCopy(DeviceInfo->LocationName, LocationName);

                                             /* Initialize the offset to*/
                                             /* zero and the size of the*/
                                             /* string so we know when  */
                                             /* to stop.                */
                                             /* * NOTE * We will update */
                                             /*          the offset once*/
                                             /*          we have        */
                                             /*          verified the   */
                                             /*          response.      */
                                             DeviceInfo->LocationNameOffset = 0;
                                             DeviceInfo->LocationNameLength = StringLength;

                                             /* Send the first GATT     */
                                             /* Prepared Write Request. */
                                             /* We will send the next   */
                                             /* request when we receive */
                                             /* the prepared response.  */
                                             /* * NOTE * We will not    */
                                             /*          save the       */
                                             /*          transactionID  */
                                             /*          returned by    */
                                             /*          this function, */
                                             /*          which we could */
                                             /*          use to cancel  */
                                             /*          the request.   */
                                             if((ret_val = GATT_Prepare_Write_Request(BluetoothStackID, ConnectionID, AttributeHandle, (Word_t)StringLength, 0, LocationName, GATT_ClientEventCallback_IPS, AttributeHandle)) > 0)
                                             {
                                                printf("\r\nGATT Prepare Write Request sent:\r\n");
                                                printf("   TransactionID:     %d\r\n",     ret_val);
                                                printf("   Attribute Handle:  0x%04X\r\n", AttributeHandle);
                                                printf("   Expected Length:   %u\r\n",     StringLength);
                                             }
                                             else
                                                DisplayFunctionError("GATT_Prepare_Write_Request", ret_val);
                                          }
                                       }
                                       else
                                          DisplayFunctionError("GATT_Query_Connection_MTU", ret_val);
                                    }
                                    else
                                       printf("\r\nReliable write in progress. Use command 'ExecWrite' to cancel.r\n");
                                    break;
                                 case wmWriteWOResponse:
                                    if(!(DeviceInfo->Flags & DEVICE_INFO_FLAGS_RELIABLE_WRITE_MODE_ENABLED))
                                    {
                                       /* Send the GATT Write Without   */
                                       /* Response Request.             */
                                       if((ret_val = GATT_Write_Without_Response_Request(BluetoothStackID, ConnectionID, AttributeHandle, (Word_t)StringLength, LocationName)) > 0)
                                       {
                                          printf("\r\nGATT Write Without Response Request sent:\r\n");
                                          printf("   Number of Bytes Written:  %u\r\n",     ret_val);
                                          printf("   Attribute Handle:         0x%04X\r\n", AttributeHandle);
                                       }
                                       else
                                          DisplayFunctionError("GATT_Write_Without_Response_Request", ret_val);
                                    }
                                    else
                                       printf("\r\nReliable write in progress. Use command 'ExecWrite' to cancel.r\n");
                                    break;
                                 case wmReliableWrite:
                                    /* Store the string so we can send  */
                                    /* the rest of it when we receive   */
                                    /* the prepare write response.      */
                                    BTPS_StringCopy(DeviceInfo->LocationName, LocationName);

                                    /* Initialize the offset to zero and*/
                                    /* the size of the string so we know*/
                                    /* when to stop.                    */
                                    /* * NOTE * We will update the      */
                                    /*          offset once we have     */
                                    /*          verified the response.  */
                                    DeviceInfo->LocationNameOffset = 0;
                                    DeviceInfo->LocationNameLength = StringLength;

                                    /* Send the GATT Prepared Write     */
                                    /* Request.                         */
                                    /* * NOTE * We will not save the    */
                                    /*          transactionID returned  */
                                    /*          by this function, which */
                                    /*          we could use to cancel  */
                                    /*          the request.            */
                                    if((ret_val = GATT_Prepare_Write_Request(BluetoothStackID, ConnectionID, AttributeHandle, (Word_t)StringLength, 0, LocationName, GATT_ClientEventCallback_IPS, AttributeHandle)) > 0)
                                    {
                                       printf("\r\nGATT Prepare Write Request sent:\r\n");
                                       printf("   TransactionID:     %d\r\n",     ret_val);
                                       printf("   Attribute Handle:  0x%04X\r\n", AttributeHandle);
                                       printf("   Expected Length:   %u\r\n",     StringLength);

                                       /* Enable reliable write mode.   */
                                       /* * NOTE * This can only be     */
                                       /*          disabled by calling  */
                                       /*          the                  */
                                       /*          ExecuteWriteRequest()*/
                                       /*          to write the request */
                                       /*          data or cancel the   */
                                       /*          request.             */
                                       DeviceInfo->Flags |= DEVICE_INFO_FLAGS_RELIABLE_WRITE_MODE_ENABLED;
                                    }
                                    else
                                       DisplayFunctionError("GATT_Prepare_Write_Request", ret_val);
                                    break;
                                 default:
                                   /* Do nothing.                       */
                                   break;
                              }
                           }
                           else
                              printf("\r\nCharacteristic handle has not been discovered.\r\n");
                        }
                        else
                           printf("\r\nService discovery has not been completed.\r\n");
                     }
                     else
                        printf("\r\nNo device information.\r\n");
                  }
                  else
                     printf("\r\nMust be connected to an IPS Server.\r\n");
               }
               else
               {
                  /* Display the usage.                                 */
                  DisplayLocationNameUsage();
               }
            }
         }
         else
            printf("\r\nString is too big. Must be less than or equal to %u characters (including NULL terminator).\r\n", (unsigned int)BTPS_CONFIGURATION_IPS_MAXIMUM_SUPPORTED_STRING_LENGTH);
      }
      else
      {
         /* Display the usage.                                          */
         DisplayLocationNameUsage();
      }
   }
   else
      printf("\r\nBluetooth stack has not been initialized.\r\n");

   return (0);
}

   /* The following function is responsible for reading multiple        */
   /* characteristics.  This function may only be called by an IPS      */
   /* Client.  If this function is called by the IPS Client then a GATT */
   /* read multiple request will be issued.  The GATT Read Multiple     */
   /* response will be received by the registered callback when the     */
   /* request is sent.                                                  */
static int ReadMultipleCharacteristics(ParameterList_t *TempParam)
{
   int           ret_val;
   DeviceInfo_t *DeviceInfo;
   Word_t        Characteristics;
   Word_t        AttributeHandleCtr = 0;
   Word_t        AttributeHandleList[IPS_MAXIMUM_NUMBER_OF_CHARACTERISTICS];
   Boolean_t     SendRequest = TRUE;
   unsigned int  Index;

   /* Check to make sure the parameters are semi-valid.                 */
   if((TempParam) && (TempParam->NumberofParameters == 1))
   {
      /* Make sure the stack is initialized.                            */
      if(BluetoothStackID)
      {
         /* Check if this is the IPS Client.                            */
         if(!IPSInstanceID)
         {
            /* We must be the client so let's make sure we are          */
            /* connected.                                               */
            if(ConnectionID)
            {
               /* Get the device information.                           */
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList,  ConnectionBD_ADDR)) != NULL)
               {
                  /* Make sure we have performed service discovery so we*/
                  /* have the attribute handle offset to request.       */
                  if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_IPS_SERVICE_DISCOVERY_COMPLETE)
                  {
                     /* Store the parameters.                           */
                     Characteristics = (Word_t)TempParam->Params[0].intParam;

                     /* Build the attribute handle list.                */
                     if(Characteristics != 0)
                     {
                        /* Make sure the client read multiple request   */
                        /* handles has all handles set to zero.  Only   */
                        /* valid handles are expected when we receive   */
                        /* the response.                                */
                        BTPS_MemInitialize(&ReadMultipleRequestHandles, 0, sizeof(ReadMultipleRequestHandles));

                        /* Check for the IPC.                           */
                        if(Characteristics & IPS_CHARACTERISTIC_INDOOR_POSITIONING_CONFIG)
                        {
                           /* Verify we have discovered the             */
                           /* characteristic's handle.                  */
                           if(DeviceInfo->IPSClientInfo.IPS_Indoor_Positioning_Configuration)
                           {
                              /* Store the handle.                      */
                              AttributeHandleList[AttributeHandleCtr++]                     = DeviceInfo->IPSClientInfo.IPS_Indoor_Positioning_Configuration;
                              ReadMultipleRequestHandles[ictIndoorPositioningConfiguration] = DeviceInfo->IPSClientInfo.IPS_Indoor_Positioning_Configuration;
                           }
                           else
                           {
                              printf("\r\nIPC handle has not been discovered.\r\n");
                              SendRequest = FALSE;
                           }
                        }

                        /* Check for the Latitude.                      */
                        if(Characteristics & IPS_CHARACTERISTIC_LATITUDE)
                        {
                           /* Verify we have discovered the             */
                           /* characteristic's handle.                  */
                           if(DeviceInfo->IPSClientInfo.IPS_Latitude)
                           {
                              /* Store the handle.                      */
                              AttributeHandleList[AttributeHandleCtr++] = DeviceInfo->IPSClientInfo.IPS_Latitude;
                              ReadMultipleRequestHandles[ictLatitude]   = DeviceInfo->IPSClientInfo.IPS_Latitude;
                           }
                           else
                           {
                              printf("\r\nLatitude handle has not been discovered.\r\n");
                              SendRequest = FALSE;
                           }
                        }

                        /* Check for the Longitude.                     */
                        if(Characteristics & IPS_CHARACTERISTIC_LONGITUDE)
                        {
                           /* Verify we have discovered the             */
                           /* characteristic's handle.                  */
                           if(DeviceInfo->IPSClientInfo.IPS_Longitude)
                           {
                              /* Store the handle.                      */
                              AttributeHandleList[AttributeHandleCtr++] = DeviceInfo->IPSClientInfo.IPS_Longitude;
                              ReadMultipleRequestHandles[ictLongitude]  = DeviceInfo->IPSClientInfo.IPS_Longitude;
                           }
                           else
                           {
                              printf("\r\nLongitude handle has not been discovered.\r\n");
                              SendRequest = FALSE;
                           }
                        }

                        /* Check for the Local North Coordinate.        */
                        if(Characteristics & IPS_CHARACTERISTIC_LOCAL_NORTH_COORDINATE)
                        {
                           /* Verify we have discovered the             */
                           /* characteristic's handle.                  */
                           if(DeviceInfo->IPSClientInfo.IPS_Local_North_Coordinate)
                           {
                              /* Store the handle.                      */
                              AttributeHandleList[AttributeHandleCtr++]           = DeviceInfo->IPSClientInfo.IPS_Local_North_Coordinate;
                              ReadMultipleRequestHandles[ictLocalNorthCoordinate] = DeviceInfo->IPSClientInfo.IPS_Local_North_Coordinate;
                           }
                           else
                           {
                              printf("\r\nLocal North Coordinate handle has not been discovered.\r\n");
                              SendRequest = FALSE;
                           }
                        }

                        /* Check for the Local East Coordinate.         */
                        if(Characteristics & IPS_CHARACTERISTIC_LOCAL_EAST_COORDINATE)
                        {
                           /* Verify we have discovered the             */
                           /* characteristic's handle.                  */
                           if(DeviceInfo->IPSClientInfo.IPS_Local_East_Coordinate)
                           {
                              /* Store the handle.                      */
                              AttributeHandleList[AttributeHandleCtr++]          = DeviceInfo->IPSClientInfo.IPS_Local_East_Coordinate;
                              ReadMultipleRequestHandles[ictLocalEastCoordinate] = DeviceInfo->IPSClientInfo.IPS_Local_East_Coordinate;
                           }
                           else
                           {
                              printf("\r\nLocal East Coordinate handle has not been discovered.\r\n");
                              SendRequest = FALSE;
                           }
                        }

                        /* Check for the Floor Number.                  */
                        if(Characteristics & IPS_CHARACTERISTIC_FLOOR_NUMBER)
                        {
                           /* Verify we have discovered the             */
                           /* characteristic's handle.                  */
                           if(DeviceInfo->IPSClientInfo.IPS_Floor_Number)
                           {
                              /* Store the handle.                      */
                              AttributeHandleList[AttributeHandleCtr++]  = DeviceInfo->IPSClientInfo.IPS_Floor_Number;
                              ReadMultipleRequestHandles[ictFloorNumber] = DeviceInfo->IPSClientInfo.IPS_Floor_Number;
                           }
                           else
                           {
                              printf("\r\nFloor Number has not been discovered.\r\n");
                              SendRequest = FALSE;
                           }
                        }

                        /* Check for the Altitude.                      */
                        if(Characteristics & IPS_CHARACTERISTIC_ALTITUDE)
                        {
                           /* Verify we have discovered the             */
                           /* characteristic's handle.                  */
                           if(DeviceInfo->IPSClientInfo.IPS_Altitude)
                           {
                              /* Store the handle.                      */
                              AttributeHandleList[AttributeHandleCtr++] = DeviceInfo->IPSClientInfo.IPS_Altitude;
                              ReadMultipleRequestHandles[ictAltitude]   = DeviceInfo->IPSClientInfo.IPS_Altitude;
                           }
                           else
                           {
                              printf("\r\nAltitude has not been discovered.\r\n");
                              SendRequest = FALSE;
                           }
                        }

                        /* Check for the Uncertainty.                   */
                        if(Characteristics & IPS_CHARACTERISTIC_UNCERTAINTY)
                        {
                           /* Verify we have discovered the             */
                           /* characteristic's handle.                  */
                           if(DeviceInfo->IPSClientInfo.IPS_Uncertainty)
                           {
                              /* Store the handle.                      */
                              AttributeHandleList[AttributeHandleCtr++]  = DeviceInfo->IPSClientInfo.IPS_Uncertainty;
                              ReadMultipleRequestHandles[ictUncertainty] = DeviceInfo->IPSClientInfo.IPS_Uncertainty;
                           }
                           else
                           {
                              printf("\r\nUncertainty has not been discovered.\r\n");
                              SendRequest = FALSE;
                           }
                        }

                        /* Check for the Location Name.                 */
                        if(Characteristics & IPS_CHARACTERISTIC_LOCATION_NAME)
                        {
                           /* Verify we have discovered the             */
                           /* characteristic's handle.                  */
                           if(DeviceInfo->IPSClientInfo.IPS_Location_Name)
                           {
                              /* Store the handle.                      */
                              AttributeHandleList[AttributeHandleCtr++]   = DeviceInfo->IPSClientInfo.IPS_Location_Name;
                              ReadMultipleRequestHandles[ictLocationName] = DeviceInfo->IPSClientInfo.IPS_Uncertainty;
                           }
                           else
                           {
                              printf("\r\nLocation Name has not been discovered.\r\n");
                              SendRequest = FALSE;
                           }
                        }

                        if(SendRequest)
                        {
                           /* Send the GATT read multiple request.      */
                           /* * NOTE * We will not save the             */
                           /*          transactionID returned by this   */
                           /*          function, which we could use to  */
                           /*          cancel the request.              */
                           /* * NOTE * We will set the CallbackParameter*/
                           /*          to zero since it is not needed   */
                           /*          like a standard read request.    */
                           if((ret_val = GATT_Read_Multiple_Values_Request(BluetoothStackID, ConnectionID, AttributeHandleCtr, AttributeHandleList, GATT_ClientEventCallback_IPS, 0)) > 0)
                           {
                              printf("\r\nGATT Read Multiple Values Request sent:\r\n");
                              printf("   TransactionID:     %d\r\n", ret_val);
                              printf("   Handle List:\r\n");

                              /* Display the handles.                   */
                              for(Index = 0; Index < (unsigned int)AttributeHandleCtr; Index++)
                              {
                                 printf("      %u. Handle:  0x%04X\r\n", Index+1, AttributeHandleList[Index]);
                              }
                           }
                           else
                              DisplayFunctionError("GATT_Read_Multiple_Values_Request", ret_val);
                        }
                     }
                     else
                     {
                        /* Display the usage.                           */
                        printf("\r\nUsage: ReadMultiple [Characteristics(UINT16 Bitmask)]\r\n");
                        printf("\r\nWhere the Characteristics has the following bit values (cannot be zero):\r\n");
                        printf("      0x0001 = Indoor Positioning Configuration.\r\n");
                        printf("      0x0002 = Latitude.\r\n");
                        printf("      0x0004 = Longitude.\r\n");
                        printf("      0x0008 = Local North Coordinate.\r\n");
                        printf("      0x0010 = Local East Coordinate.\r\n");
                        printf("      0x0020 = Floor Number.\r\n");
                        printf("      0x0040 = Altitude.\r\n");
                        printf("      0x0080 = Uncertainty.\r\n");
                        printf("      0x0100 = Location Name.\r\n");
                     }
                  }
                  else
                     printf("\r\nService discovery has not been completed.\r\n");
               }
               else
                  printf("\r\nNo device information.\r\n");
            }
            else
               printf("\r\nMust be connected to an IPS Server.\r\n");
         }
         else
            printf("\r\nMust be an IPS client.\r\n");
      }
      else
         printf("\r\nBluetooth stack has not been initialized.\r\n");
   }
   else
   {
      /* Display the usage.                                             */
      printf("\r\nUsage: ReadMultiple [Characteristics(UINT16 Bitmask)]\r\n");
      printf("\r\nWhere the Characteristics has the following bit values (cannot be zero):\r\n");
      printf("      0x0001 = Indoor Positioning Configuration.\r\n");
      printf("      0x0002 = Latitude.\r\n");
      printf("      0x0004 = Longitude.\r\n");
      printf("      0x0008 = Local North Coordinate.\r\n");
      printf("      0x0010 = Local East Coordinate.\r\n");
      printf("      0x0020 = Floor Number.\r\n");
      printf("      0x0040 = Altitude.\r\n");
      printf("      0x0080 = Uncertainty.\r\n");
      printf("      0x0100 = Location Name.\r\n");
   }

   return (0);
}

   /* The following function is responsible for executing a reliable    */
   /* write request.  This function may only be called by an IPS Client.*/
   /* This function may only be called if a reliable write is in        */
   /* progress.  Any characteristic write command can enable the        */
   /* reliable write mode.  Once triggered only this function is able to*/
   /* stop it.  This will instruct the IPS Server to store all the      */
   /* prepared data that has previuosly been written or delete all the  */
   /* data if the request is canceled.                                  */
static int ExecuteWriteRequest(ParameterList_t *TempParam)
{
   int           ret_val;
   DeviceInfo_t *DeviceInfo;

   /* Check to make sure the parameters are semi-valid.                 */
   if((TempParam) && (TempParam->NumberofParameters == 1))
   {
      /* Make sure the stack is initialized.                            */
      if(BluetoothStackID)
      {
         /* Check if this is the IPS Client.                            */
         if(!IPSInstanceID)
         {
            /* We must be the client so let's make sure we are          */
            /* connected.                                               */
            if(ConnectionID)
            {
               /* Get the device information.                           */
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList,  ConnectionBD_ADDR)) != NULL)
               {
                  /* Make sure we have performed service discovery so we*/
                  /* have the attribute handle offset to request.       */
                  if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_IPS_SERVICE_DISCOVERY_COMPLETE)
                  {
                     /* Send the GATT Execute Write request.            */
                     /* * NOTE * We will not save the transactionID     */
                     /*          returned by this function, which we    */
                     /*          could use to cancel the request.       */
                     if((ret_val = GATT_Execute_Write_Request(BluetoothStackID, ConnectionID, (Boolean_t)TempParam->Params[0].intParam, GATT_ClientEventCallback_IPS, 0)) > 0)
                     {
                        printf("\r\nGATT Execute Write Request sent:\r\n");
                        printf("   TransactionID:  %d\r\n", ret_val);
                        printf("   Cancel Write:   %s\r\n", (Boolean_t)TempParam->Params[0].intParam ? "Yes" : "No");
                     }
                     else
                        DisplayFunctionError("GATT_Execute_Write_Request", ret_val);

                     /* Disable reliable write mode.                    */
                     DeviceInfo->Flags &= ~DEVICE_INFO_FLAGS_RELIABLE_WRITE_MODE_ENABLED;
                  }
                  else
                     printf("\r\nService discovery has not been completed.\r\n");
               }
               else
                  printf("\r\nNo device information.\r\n");
            }
            else
               printf("\r\nMust be connected to an IPS Server.\r\n");
         }
         else
            printf("\r\nMust be an IPS client.\r\n");
      }
      else
         printf("\r\nBluetooth stack has not been initialized.\r\n");
   }
   else
   {
      /* Display the usage.                                             */
      printf("\r\nUsage: ExecWrite [Cancel Write(0=False, 1=TRUE)]\r\n");
   }

   return (0);
}

   /* The following function is used to decode and display the IPC      */
   /* characteristic for read responses.                                */
static int DecodeDisplayIPCResponse(Byte_t *Value, Word_t ValueLength)
{
   int     ret_val = 0;
   Byte_t  IPC;

   /* Display the characteristic name.                                  */
   printf("\r\nIndoor Positioning Configuration:\r\n");

   /* Verify the length.                                                */
   if(ValueLength >= NON_ALIGNED_BYTE_SIZE)
   {
      /* Decode the Characteristic.                                     */
      IPC = READ_UNALIGNED_BYTE_LITTLE_ENDIAN(Value);

      /* Print the characteristic.                                      */
      printf("   Value:   0x%02X\r\n", IPC);
   }
   else
   {
      /* We only need to care about the return value if this is the GATT*/
      /* read multiple response since we do not want to continue with   */
      /* the next characteristic.                                       */
      printf("   Value:   Invalid Length.\r\n");
      ret_val = FUNCTION_ERROR;
   }
   printf("\r\n");

   return ret_val;
}

   /* The following function is used to decode and display the Latitude */
   /* characteristic for read responses.                                */
static int DecodeDisplayLatitudeResponse(Byte_t *Value, Word_t ValueLength)
{
   int       ret_val = 0;
   SDWord_t  Latitude;

   /* Display the characteristic name.                                  */
   printf("\r\nLatitude:\r\n");

   /* Verify the length.                                                */
   if(ValueLength >= NON_ALIGNED_SDWORD_SIZE)
   {
      /* Decode the Characteristic.                                     */
      Latitude = READ_UNALIGNED_DWORD_LITTLE_ENDIAN(Value);

      /* Print the characteristic.                                      */
      if(Latitude != (SDWord_t)IPS_LATITUDE_NOT_CONFIGURED)
      {
         /* Decode and display the value.                               */
         DecodeLatitude(&Latitude);
         printf("   Value:   %d (degrees)\r\n", Latitude);
      }
      else
         printf("   Value:   Not Configured.\r\n");
   }
   else
   {
      /* We only need to care about the return value if this is the GATT*/
      /* read multiple response since we do not want to continue with   */
      /* the next characteristic.                                       */
      printf("   Value:   Invalid Length.\r\n");
      ret_val = FUNCTION_ERROR;
   }
   printf("\r\n");

   return ret_val;
}

   /* The following function is used to decode and display the Longitude*/
   /* characteristic for read responses.                                */
static int DecodeDisplayLongitudeResponse(Byte_t *Value, Word_t ValueLength)
{
   int       ret_val = 0;
   SDWord_t  Longitude;

   /* Display the characteristic name.                                  */
   printf("\r\nLongitude:\r\n");

   /* Verify the length.                                                */
   if(ValueLength >= NON_ALIGNED_SDWORD_SIZE)
   {
      /* Decode the Characteristic.                                     */
      Longitude = READ_UNALIGNED_DWORD_LITTLE_ENDIAN(Value);

      /* Print the characteristic.                                      */
      if(Longitude != (SDWord_t)IPS_LONGITUDE_NOT_CONFIGURED)
      {
         /* Decode and display the value.                               */
         DecodeLongitude(&Longitude);
         printf("   Value:   %d (degrees)\r\n", Longitude);
      }
      else
         printf("   Value:   Not Configured.\r\n");
   }
   else
   {
      /* We only need to care about the return value if this is the GATT*/
      /* read multiple response since we do not want to continue with   */
      /* the next characteristic.                                       */
      printf("   Value:   Invalid Length.\r\n");
      ret_val = FUNCTION_ERROR;
   }
   printf("\r\n");

   return ret_val;
}

   /* The following function is used to decode and display the Local    */
   /* East Coordinate characteristic for read responses.                */
static int DecodeDisplayLocalNorthCoordinateResponse(Byte_t *Value, Word_t ValueLength)
{
   int       ret_val = 0;
   SDWord_t  LocalNorthCoordinte;

   /* Display the characteristic name.                                  */
   printf("\r\nLocal North Coordinate:\r\n");

   /* Verify the length.                                                */
   if(ValueLength >= NON_ALIGNED_SWORD_SIZE)
   {
      /* Decode the Characteristic.                                     */
      LocalNorthCoordinte = READ_UNALIGNED_WORD_LITTLE_ENDIAN(Value);

      /* Print the characteristic.                                      */
      if(LocalNorthCoordinte != (SDWord_t)IPS_LOCAL_COORDINATE_NOT_CONFIGURED)
      {
         /* Display the value.                                          */
         printf("   Value:   %d (decimeters)\r\n", LocalNorthCoordinte);
      }
      else
         printf("   Value:   Not Configured.\r\n");
   }
   else
   {
      /* We only need to care about the return value if this is the GATT*/
      /* read multiple response since we do not want to continue with   */
      /* the next characteristic.                                       */
      printf("   Value:   Invalid Length.\r\n");
      ret_val = FUNCTION_ERROR;
   }
   printf("\r\n");

   return ret_val;
}

   /* The following function is used to decode and display the Local    */
   /* North Coordinate characteristic for read responses.               */
static int DecodeDisplayLocalEastCoordinateResponse(Byte_t *Value, Word_t ValueLength)
{
   int       ret_val = 0;
   SDWord_t  LocalEastCoordinte;

   /* Display the characteristic name.                                  */
   printf("\r\nLocal East Coordinate:\r\n");

   /* Verify the length.                                                */
   if(ValueLength >= NON_ALIGNED_SWORD_SIZE)
   {
      /* Decode the Characteristic.                                     */
      LocalEastCoordinte = READ_UNALIGNED_WORD_LITTLE_ENDIAN(Value);

      /* Print the characteristic.                                      */
      if(LocalEastCoordinte != (SDWord_t)IPS_LOCAL_COORDINATE_NOT_CONFIGURED)
      {
         /* Display the value.                                          */
         printf("   Value:   %d (decimeters)\r\n", LocalEastCoordinte);
      }
      else
         printf("   Value:   Not Configured.\r\n");
   }
   else
   {
      /* We only need to care about the return value if this is the GATT*/
      /* read multiple response since we do not want to continue with   */
      /* the next characteristic.                                       */
      printf("   Value:   Invalid Length.\r\n");
      ret_val = FUNCTION_ERROR;
   }
   printf("\r\n");

   return ret_val;
}

   /* The following function is used to decode and display the Floor    */
   /* Number characteristic for read responses.                         */
static int DecodeDisplayFloorNumberResponse(Byte_t *Value, Word_t ValueLength)
{
   int      ret_val = 0;
   SWord_t  FloorNumber;

   /* Display the characteristic name.                                  */
   printf("\r\nFloor Number:\r\n");

   /* Verify the length.                                                */
   if(ValueLength >= NON_ALIGNED_BYTE_SIZE)
   {
      /* Decode the Characteristic.                                     */
      FloorNumber = READ_UNALIGNED_BYTE_LITTLE_ENDIAN(Value);

      /* Decode and display the value.                                  */
      switch(FloorNumber)
      {
         case 253:
            FloorNumber = 0;
            printf("   Value:   %d\r\n", FloorNumber);
            break;
         case 254:
            FloorNumber = 1;
            printf("   Value:   %d\r\n", FloorNumber);
            break;
         case IPS_FLOOR_NUMBER_NOT_CONFIGURED:
            printf("   Value:   Not Configured\r\n");
            break;
         default:
            FloorNumber = FloorNumber - 20;
            printf("   Value:   %d\r\n", FloorNumber);
            break;
      }
   }
   else
   {
      /* We only need to care about the return value if this is the GATT*/
      /* read multiple response since we do not want to continue with   */
      /* the next characteristic.                                       */
      printf("   Value:   Invalid Length.\r\n");
      ret_val = FUNCTION_ERROR;
   }
   printf("\r\n");

   return ret_val;
}

   /* The following function is used to decode and display the Altitude */
   /* characteristic for read responses.                                */
static int DecodeDisplayAltitudeResponse(Byte_t *Value, Word_t ValueLength)
{
   int       ret_val = 0;
   SDWord_t  Altitude;

   /* Display the characteristic name.                                  */
   printf("\r\nAltitude:\r\n");

   /* Verify the length.                                                */
   if(ValueLength >= NON_ALIGNED_WORD_SIZE)
   {
      /* Decode the Characteristic.                                     */
      Altitude = READ_UNALIGNED_WORD_LITTLE_ENDIAN(Value);

      /* Print the characteristic.                                      */
      if(Altitude != (SDWord_t)IPS_ALTITUDE_NOT_CONFIGURED)
      {
         /* Decode and display the Altitude.                            */
         Altitude = Altitude - 1000;
         printf("   Value:   %d (decimeters)\r\n", Altitude);
      }
      else
         printf("   Value:   Not Configured.\r\n");
   }
   else
   {
      /* We only need to care about the return value if this is the GATT*/
      /* read multiple response since we do not want to continue with   */
      /* the next characteristic.                                       */
      printf("   Value:   Invalid Length.\r\n");
      ret_val = FUNCTION_ERROR;
   }
   printf("\r\n");

   return ret_val;
}

   /* The following function is used to decode and display the          */
   /* Uncertainty characteristic for read responses.                    */
static int DecodeDisplayUncertaintyResponse(Byte_t *Value, Word_t ValueLength)
{
   int     ret_val = 0;
   Byte_t  Uncertainty;

   /* Display the characteristic name.                                  */
   printf("\r\nUncertainty:\r\n");

   /* Verify the length.                                                */
   if(ValueLength >= NON_ALIGNED_BYTE_SIZE)
   {
      /* Decode the Characteristic.                                     */
      Uncertainty = READ_UNALIGNED_BYTE_LITTLE_ENDIAN(Value);

      /* Display the value.                                             */
      DisplayUncertaintyCharacteristic(Uncertainty);
   }
   else
   {
      /* We only need to care about the return value if this is the GATT*/
      /* read multiple response since we do not want to continue with   */
      /* the next characteristic.                                       */
      printf("   Value:   Invalid Length.\r\n");
      ret_val = FUNCTION_ERROR;
   }
   printf("\r\n");

   return ret_val;
}

   /* The following function is responsible for decoding and displaying */
   /* the                                                               */
static int DecodeDisplayIPSAdvertisingData(GAP_LE_Advertising_Data_Entry_t *Data_Entry)
{
   int                   ret_val = 0;
   IPS_Broadcast_Data_t  BroadcastData;

   if(Data_Entry)
   {
      /* Only display the data if it exists.                            */
      /* * NOTE * Only the AD type and length will exist if the IPC is  */
      /*          zero.                                                 */
      if(Data_Entry->AD_Data_Length)
      {
         /* Decode the data.                                            */
         if((ret_val = IPS_Decode_Broadcasting_Data(Data_Entry->AD_Data_Length, Data_Entry->AD_Data_Buffer, &BroadcastData)) == 0)
         {
            /* Print the Flags.                                         */
            printf("\r\nFlags:\r\n");
            printf("   Value:   0x%02X\r\n", BroadcastData.Flags);

            /* Decode the values and display the data based on the Flags*/
            /* field.                                                   */
            if(BroadcastData.Flags & IPS_INDOOR_POSITIONING_CONFIG_COORDINATES_PRESENT)
            {
               /* Determine the type to display.                        */
               if(BroadcastData.Flags & IPS_INDOOR_POSITIONING_CONFIG_LOCAL_COORDINATES_USED)
               {
                  /* Print the Local North Coordindate.                 */
                  printf("\r\nLocal North Coordinate:\r\n");
                  if(BroadcastData.LocalNorthCoordinate != (SWord_t)IPS_LOCAL_COORDINATE_NOT_CONFIGURED)
                  {
                     /* Display the value.                              */
                     printf("   Value:   %d (decimeters)\r\n", BroadcastData.LocalNorthCoordinate);
                  }
                  else
                     printf("   Value:   Not Configured.\r\n");

                  /* Print the Local East Coordindate.                  */
                  printf("\r\nLocal East Coordinate:\r\n");
                  if(BroadcastData.LocalEastCoordinate != (SWord_t)IPS_LOCAL_COORDINATE_NOT_CONFIGURED)
                  {
                     /* Display the value.                              */
                     printf("   Value:   %d (decimeters)\r\n", BroadcastData.LocalEastCoordinate);
                  }
                  else
                     printf("   Value:   Not Configured.\r\n");
               }
               else
               {
                  /* Print the Latitude.                                */
                  printf("\r\nLatitude:\r\n");
                  if(BroadcastData.Latitude != (SDWord_t)IPS_LATITUDE_NOT_CONFIGURED)
                  {
                     /* Decode and display the value.                   */
                     DecodeLatitude(&BroadcastData.Latitude);
                     printf("   Value:   %d (degrees)\r\n", BroadcastData.Latitude);
                  }
                  else
                     printf("   Value:   Not Configured.\r\n");

                  /* Print the Longitude.                               */
                  printf("\r\nLongitude:\r\n");
                  if(BroadcastData.Longitude != (SDWord_t)IPS_LONGITUDE_NOT_CONFIGURED)
                  {
                     /* Decode and display the value.                   */
                     DecodeLongitude(&BroadcastData.Longitude);
                     printf("   Value:   %d (degrees)\r\n", BroadcastData.Longitude);
                  }
                  else
                     printf("   Value:   Not Configured.\r\n");
               }
            }

            /* If Tx Power is present.                                  */
            if(BroadcastData.Flags & IPS_INDOOR_POSITIONING_CONFIG_TX_POWER_PRESENT)
            {
               /* Print the Tx Power.                                   */
               printf("\r\nTx Power:\r\n");
               printf("   Value:   %u (dBm)\r\n", BroadcastData.TxPower);
            }

            /* If Floor Number is present.                              */
            if(BroadcastData.Flags & IPS_INDOOR_POSITIONING_CONFIG_FLOOR_NUMBER_PRESENT)
            {
               /* Print the Altitude.                                   */
               printf("\r\nFloor Number:\r\n");

               /* Decode and display the Altitude.                      */
               switch(BroadcastData.FloorNumber)
               {
                  case 253:
                     BroadcastData.FloorNumber = 0;
                     printf("   Value:   %d\r\n", BroadcastData.FloorNumber);
                     break;
                  case 254:
                     BroadcastData.FloorNumber = 1;
                     printf("   Value:   %d\r\n", BroadcastData.FloorNumber);
                     break;
                  case IPS_FLOOR_NUMBER_NOT_CONFIGURED:
                     printf("   Value:   Not Configured\r\n");
                     break;
                  default:
                     BroadcastData.FloorNumber = BroadcastData.FloorNumber - 20;
                     printf("   Value:   %d\r\n", BroadcastData.FloorNumber);
                     break;
               }
            }

            /* If Altitude is present.                                  */
            if(BroadcastData.Flags & IPS_INDOOR_POSITIONING_CONFIG_ALTITUDE_PRESENT)
            {
               /* Print the Altitude.                                   */
               printf("\r\nAltitude:\r\n");
               if(BroadcastData.Altitude != (SWord_t)IPS_ALTITUDE_NOT_CONFIGURED)
               {
                  /* Decode and display the Altitude.                   */
                  BroadcastData.Altitude = BroadcastData.Altitude - 1000;
                  printf("   Value:   %d (decimeters)\r\n", BroadcastData.Altitude);
               }
               else
                  printf("   Value:   Not Configured.\r\n");
            }

            /* If Uncertainty is present.                               */
            if(BroadcastData.Flags & IPS_INDOOR_POSITIONING_CONFIG_UNCERTAINTY_PRESENT)
            {
               /* Print the Uncertainty.                                */
               printf("\r\nUncertainty:\r\n");
               DisplayUncertaintyCharacteristic(BroadcastData.Uncertainty);
            }

            /* If Location Name is availbale.                           */
            if(BroadcastData.Flags & IPS_INDOOR_POSITIONING_CONFIG_LOCATION_NAME_AVAILABLE)
            {
               /* Print the Uncertainty.                                */
               printf("\r\nLocation Name available.\r\n");
            }
         }
         else
            DisplayFunctionError("IPS_Decode_Broadcasting_Data", ret_val);
      }
   }
   else
      ret_val = INVALID_PARAMETERS_ERROR;

   return ret_val;
}

   /* The following function is responsible for encoding the latitude.  */
static void EncodeLatitude(SDWord_t *Latitude)
{
   SQWord_t Result;

   if(Latitude)
   {
      /* Perform the encoding.                                          */
      Result = (SQWord_t)floor(((*Latitude)/90.0)*(pow(2.0, 31.0)));

      /* Check if we are out of bounds.  If we are we need to set to    */
      /* those values.                                                  */
      /* * NOTE * Values hardcoded in decimal.                          */
      if(Result > 2147483647)
      {
         *Latitude = IPS_ENCODED_LATITUDE_UPPER_BOUND;
      }
      else
      {
         if(Result < -2147483647)
         {
            *Latitude = IPS_ENCODED_LATITUDE_LOWER_BOUND;
         }
         else
         {
            *Latitude = (SDWord_t)Result;
         }
      }
   }
}

   /* The following function is responsible for decoding the latitude.  */
static void DecodeLatitude(SDWord_t *Latitude)
{
   if(Latitude)
   {
      /* If -90 degress was encoded then we went out of bounds due to   */
      /* 2's complement numbers (-2^31+1 is one less than the lowest    */
      /* possible number).  We will simply account for the added one    */
      /* here by subtracting one.                                       */
      if(*Latitude == IPS_ENCODED_LATITUDE_LOWER_BOUND)
      {
         /* Subtract one.                                               */
         *Latitude -= 1;
      }

      /* Perform the decoding.                                          */
      *Latitude = (SDWord_t)ceil((((SQWord_t)(*Latitude))*90)/(pow(2.0, 31.0)));
   }
}

   /* The following function is responsible for encoding the longitude. */
static void EncodeLongitude(SDWord_t *Longitude)
{
   SQWord_t Result;

   if(Longitude)
   {
      /* Perform the encoding.                                          */
      Result = (SQWord_t)floor(((*Longitude)/180.0)*(pow(2.0, 31.0)));

      /* Check if we are out of bounds.  If we are we need to set to    */
      /* those values.                                                  */
      /* * NOTE * Values hardcoded in decimal.                          */
      if(Result > 2147483647)
      {
         *Longitude = IPS_ENCODED_LONGITUDE_UPPER_BOUND;
      }
      else
      {
         if(Result < -2147483647)
         {
            *Longitude = IPS_ENCODED_LONGITUDE_LOWER_BOUND;
         }
         else
         {
            *Longitude = (SDWord_t)Result;
         }
      }
   }
}

   /* The following function is responsible for decoding the longitude. */
static void DecodeLongitude(SDWord_t *Longitude)
{
   if(Longitude)
   {
      /* If -180 degress was encoded then we went out of bounds due to  */
      /* 2's complement numbers (-2^31+1 is one less than the lowest    */
      /* possible number).  We will simply account for the added one    */
      /* here by subtracting one.                                       */
      if(*Longitude == IPS_ENCODED_LONGITUDE_LOWER_BOUND)
      {
         /* Subtract one.                                               */
         *Longitude -= 1;
      }

      /* Perform the decoding.                                          */
      *Longitude = (SDWord_t)ceil((((SQWord_t)(*Longitude))*180)/(pow(2.0, 31.0)));
   }
}

   /* The following function is a helper function to print off the      */
   /* Uncertainty characteristic bit fields.                            */
static void DisplayUncertaintyCharacteristic(Byte_t Uncertainty)
{
   printf("   Value:        0x%02X\r\n", Uncertainty);
   printf("   Location:     %s\r\n", (Uncertainty & IPS_UNCERTAINTY_MOBILE) ? "Mobile" : "Stationary");
   printf("   Update Time:  ");

   /* Only switch on the update time bits.                              */
   switch(IPS_DECODE_UNCERTAINTY_UPDATE_TIME_BITS(Uncertainty))
   {
      case IPS_UNCERTAINTY_UPDATE_TIME_UP_TO_3_SEC:
         printf("Up to 3 seconds.\r\n");
         break;
      case IPS_UNCERTAINTY_UPDATE_TIME_UP_TO_4_SEC:
         printf("Up to 4 seconds.\r\n");
         break;
      case IPS_UNCERTAINTY_UPDATE_TIME_UP_TO_6_SEC:
         printf("Up to 6 seconds.\r\n");
         break;
      case IPS_UNCERTAINTY_UPDATE_TIME_UP_TO_12_SEC:
         printf("Up to 12 seconds.\r\n");
         break;
      case IPS_UNCERTAINTY_UPDATE_TIME_UP_TO_28_SEC:
         printf("Up to 28 seconds.\r\n");
         break;
      case IPS_UNCERTAINTY_UPDATE_TIME_UP_TO_89_SEC:
         printf("Up to 89 seconds.\r\n");
         break;
      case IPS_UNCERTAINTY_UPDATE_TIME_UP_TO_426_SEC:
         printf("Up to 426 seconds.\r\n");
         break;
      case IPS_UNCERTAINTY_UPDATE_TIME_UP_TO_3541_SEC:
         printf("Up to 3541 seconds.\r\n");
         break;
      default:
         printf("Invalid.\r\n");
         break;
   }

   printf("   Precision:    ");

   /* Only switch on the precision bits.                                */
   switch(IPS_DECODE_UNCERTAINTY_PRECISION_BITS(Uncertainty))
   {
      case IPS_UNCERTAINTY_PRECISION_LESS_THAN_POINT_1_M:
         printf("Less than .1 meter.\r\n");
         break;
      case IPS_UNCERTAINTY_PRECISION_POINT_1_TO_1_M:
         printf(".1 to 1 meter.\r\n");
         break;
      case IPS_UNCERTAINTY_PRECISION_1_TO_2_M:
         printf("1 to 2 meters.\r\n");
         break;
      case IPS_UNCERTAINTY_PRECISION_2_TO_5_M:
         printf("2 to 5 meters.\r\n");
         break;
      case IPS_UNCERTAINTY_PRECISION_5_TO_10_M:
         printf("5 to 10 meters.\r\n");
         break;
      case IPS_UNCERTAINTY_PRECISION_10_TO_50_M:
         printf("10 to 50 meters.\r\n");
         break;
      case IPS_UNCERTAINTY_PRECISION_GREATER_THAN_50_M:
         printf("Greater than 50 meters.\r\n");
         break;
      default:
         printf("Invalid.\r\n");
         break;
   }
}

   /* The following function is responsible for display the IPS         */
   /* Characteristic Type.                                              */
static void DisplayIPSCharacteristicType(IPS_Characteristic_Type_t Type)
{
   switch(Type)
   {
      case ictIndoorPositioningConfiguration:
         printf("Indoor Positioning Configuration.\r\n");
         break;
      case ictLatitude:
         printf("Latitude.\r\n");
         break;
      case ictLongitude:
         printf("Longitude.\r\n");
         break;
      case ictLocalNorthCoordinate:
         printf("Local North Coordinate.\r\n");
         break;
      case ictLocalEastCoordinate:
         printf("Local East Coordinate.\r\n");
         break;
      case ictFloorNumber:
         printf("Floor Number.\r\n");
         break;
      case ictAltitude:
         printf("Altitude.\r\n");
         break;
      case ictUncertainty:
         printf("Uncertainty.\r\n");
         break;
      case ictLocationName:
         printf("Location Name.\r\n");
         break;
      default:
         /* Just here to prevent warnings.                              */
         break;
   }
}

   /* The following function is a helper function to display the        */
   /* characteristics that were read or update when IPS Events are      */
   /* received.                                                         */
static void DisplayUpdatedCharacteristics(Word_t Flags)
{
   unsigned int Index;
   Word_t       BitPosition;

   printf("   Characteristic(s):    0x%04X\r\n", Flags);

   /* * NOTE * We will start at one so the first bit position is set.   */
   for(Index = 0, BitPosition = 1; Index < IPS_MAXIMUM_NUMBER_OF_CHARACTERISTICS; Index++, BitPosition <<= 1)
   {
      /* If the characteristic specified by the bit mask Flags is       */
      /* present let's print the characteristic type.                   */
      switch(Flags & BitPosition)
      {
         case IPS_CHARACTERISTIC_INDOOR_POSITIONING_CONFIG:
            printf("                         Indoor Positioning Config.\r\n");
            break;
         case IPS_CHARACTERISTIC_LATITUDE:
            printf("                         Latitude.\r\n");
            break;
         case IPS_CHARACTERISTIC_LONGITUDE:
            printf("                         Longitude.\r\n");
            break;
         case IPS_CHARACTERISTIC_LOCAL_NORTH_COORDINATE:
            printf("                         Local North Coordinate.\r\n");
            break;
         case IPS_CHARACTERISTIC_LOCAL_EAST_COORDINATE:
            printf("                         Local East Coordinate.\r\n");
            break;
         case IPS_CHARACTERISTIC_FLOOR_NUMBER:
            printf("                         Floor Number.\r\n");
            break;
         case IPS_CHARACTERISTIC_ALTITUDE:
            printf("                         Altitude.\r\n");
            break;
         case IPS_CHARACTERISTIC_UNCERTAINTY:
            printf("                         Uncertainty.\r\n");
            break;
         case IPS_CHARACTERISTIC_LOCATION_NAME:
            printf("                         Location Name.\r\n");
            break;
         default:
            /* Don't print anything if the characteristic has been      */
            /* updated.                                                 */
            break;
      }
   }
}

   /* The following function displays the usage for the Write Indoor    */
   /* Positioning Configuration command.                                */
static void DisplayWriteIPCUsage(void)
{
   /* Display the usage.                                                */
   printf("\r\nUsage: WriteIPC [Configuration(UINT8)] (Client ONLY)[GATT Request Type(Boolean)]\r\n");
   printf("\r\nWhere Configuration and GATT Request Type values are:\r\n");
   printf("\r\n   Configuration (bitmask):\r\n");
   printf("      0x01 = Coordinates Present.\r\n");
   printf("      0x02 = Local Coordinates Used.\r\n");
   printf("      0x04 = Tx Power Present.\r\n");
   printf("      0x08 = Altitude Present.\r\n");
   printf("      0x10 = Floor Number Present.\r\n");
   printf("      0x20 = Uncertainty Present.\r\n");
   printf("      0x40 = Location Name Present.\r\n");
   printf("\r\n   Request Type:\r\n");
   printf("      0 = Write.\r\n");
   printf("      1 = Write Without Response.\r\n");
   printf("      2 = Reliable Write.\r\n");
}

   /* The following function displays the usage for the Write Latitude  */
   /* command.                                                          */
static void DisplayLatitudeUsage(void)
{
   /* Display the usage.                                                */
   printf("\r\nUsage: WriteLatitude [Latitude (-90 to 90 degrees(SINT32))] (Client ONLY)[GATT Request Type(Boolean)]\r\n");
   printf("\r\nWhere GATT Request Type is:\r\n");
   printf("\r\n   Request Type:\r\n");
   printf("      0 = Write.\r\n");
   printf("      1 = Write Without Response.\r\n");
   printf("      2 = Reliable Write.\r\n");
}

   /* The following function displays the usage for the Write Longitude */
   /* command.                                                          */
static void DisplayLongitudeUsage(void)
{
   /* Display the usage.                                                */
   printf("\r\nUsage: WriteLongitude [Longitude (-180 to 180 degrees(SINT32))] (Client ONLY)[GATT Request Type(Boolean)]\r\n");
   printf("\r\nWhere GATT Request Type is:\r\n");
   printf("\r\n   Request Type:\r\n");
   printf("      0 = Write.\r\n");
   printf("      1 = Write Without Response.\r\n");
   printf("      2 = Reliable Write.\r\n");
}

   /* The following function displays the usage for the Write Local     */
   /* North Coordinate command.                                         */
static void DisplayLocalNorthCoordinateUsage(void)
{
   /* Display the usage.                                                */
   printf("\r\nUsage: WriteLocalNorth [Local North Coordinate (-32767 to 32767 decimeters SINT16)] (Client ONLY)[GATT Request Type(Boolean)]\r\n");
   printf("\r\nWhere GATT Request Type is:\r\n");
   printf("\r\n   Request Type:\r\n");
   printf("      0 = Write.\r\n");
   printf("      1 = Write Without Response.\r\n");
   printf("      2 = Reliable Write.\r\n");
}

   /* The following function displays the usage for the Write Local East*/
   /* Coordinate command.                                               */
static void DisplayLocalEastCoordinateUsage(void)
{
   /* Display the usage.                                                */
   printf("\r\nUsage: WriteLocalEast [Local East Coordinate (-32767 to 32767 decimeters SINT16)] (Client ONLY)[GATT Request Type(Boolean)]\r\n");
   printf("\r\nWhere GATT Request Type is:\r\n");
   printf("\r\n   Request Type:\r\n");
   printf("      0 = Write.\r\n");
   printf("      1 = Write Without Response.\r\n");
   printf("      2 = Reliable Write.\r\n");
}

   /* The following function displays the usage for the Write Floor     */
   /* Number command.                                                   */
static void DisplayFloorNumberUsage(void)
{
   /* Display the usage.                                                */
   printf("\r\nUsage: WriteFloorNumber [Floor Number(-20 to 232 UINT8)] (Client ONLY)[GATT Request Type(Boolean)]\r\n");
   printf("\r\nWhere GATT Request Type is:\r\n");
   printf("\r\n   Request Type:\r\n");
   printf("      0 = Write.\r\n");
   printf("      1 = Write Without Response.\r\n");
   printf("      2 = Reliable Write.\r\n");
   printf("\r\nNote - Floor numbers 0,1 will be encoded as ground floor.\r\n");
}

   /* The following function displays the usage for the Write Altitude  */
   /* command.                                                          */
static void DisplayAltitudeUsage(void)
{
   /* Display the usage.                                                */
   printf("\r\nUsage: WriteAltitude [Altitude(-1000 to 64534 decimenters UINT16)] (Client ONLY)[GATT Request Type(Boolean)]\r\n");
   printf("\r\nWhere GATT Request Type is:\r\n");
   printf("\r\n   Request Type:\r\n");
   printf("      0 = Write.\r\n");
   printf("      1 = Write Without Response.\r\n");
   printf("      2 = Reliable Write.\r\n");
}

   /* The following function displays the usage for the Write           */
   /* Uncertainty command.                                              */
static void DisplayUncertaintyUsage(void)
{
   /* Display the usage.                                                */
   printf("\r\nUsage: WriteUncertainty [Location(UINT8)] [Update Time(UINT8)] [Precision(UINT8)] (Client ONLY)[GATT Request Type(Boolean)]\r\n");
   printf("\r\nWhere Location, Update Time, Precision, and GATT Request Type are:\r\n");
   printf("\r\n   Location (bitmask):\r\n");
   printf("      0x01 = Mobile location.\r\n");
   printf("\r\n   Update Time (Only valid if Location is Mobile):\r\n");
   printf("      0 = Up to 3 seconds.\r\n");
   printf("      1 = Up to 4 seconds.\r\n");
   printf("      2 = Up to 6 seconds.\r\n");
   printf("      3 = Up to 12 seconds.\r\n");
   printf("      4 = Up to 28 seconds.\r\n");
   printf("      5 = Up to 89 seconds.\r\n");
   printf("      6 = Up to 426 seconds.\r\n");
   printf("      7 = Up to 3541 seconds.\r\n");
   printf("\r\n   Precision:\r\n");
   printf("      0 = Less than 0.1 meter.\r\n");
   printf("      1 = 0.1 to 1 meter.\r\n");
   printf("      2 = 1 to 2 meters.\r\n");
   printf("      3 = 2 to 5 meters.\r\n");
   printf("      4 = 5 to 10 meters.\r\n");
   printf("      5 = 10 to 50 meters.\r\n");
   printf("      6 = Greater than 50 meters.\r\n");
   printf("\r\nWhere GATT Request Type is:\r\n");
   printf("\r\n   Request Type:\r\n");
   printf("      0 = Write.\r\n");
   printf("      1 = Write Without Response.\r\n");
   printf("      2 = Reliable Write.\r\n");
}

   /* The following function displays the usage for the Write Location  */
   /* Name command.                                                     */
static void DisplayLocationNameUsage(void)
{
   /* Display the usage.                                                */
   printf("\r\nUsage: WriteLocationName [Location Name(UTF8)] (Client ONLY)[GATT Request Type(Boolean)]\r\n");
   printf("\r\nWhere GATT Request Type is:\r\n");
   printf("\r\n   Request Type:\r\n");
   printf("      0 = Write.\r\n");
   printf("      1 = Write Without Response.\r\n");
   printf("      2 = Reliable Write.\r\n");
}

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
   Boolean_t                         OOB_Data;
   Boolean_t                         MITM;
   GAP_IO_Capability_t               RemoteIOCapability;
   GAP_Inquiry_Event_Data_t         *GAP_Inquiry_Event_Data;
   GAP_Remote_Name_Event_Data_t     *GAP_Remote_Name_Event_Data;
   GAP_Authentication_Information_t  GAP_Authentication_Information;
   DeviceInfo_t                     *DeviceInfo;

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
               printf("GAP_Inquiry_Result: %d Found.\r\n", GAP_Inquiry_Event_Data->Number_Devices);

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

                     printf("GAP Inquiry Result: %d, %s.\r\n", (Index+1), BoardStr);
                  }

                  NumberofValidResponses = GAP_Inquiry_Event_Data->Number_Devices;
               }
            }
            break;
         case etEncryption_Change_Result:
            /* Intentional fall through case.                           */
         case etEncryption_Refresh_Complete:
            /* Next convert the BD_ADDR to a string.                    */
            BD_ADDRToStr(GAP_Event_Data->Event_Data.GAP_Encryption_Mode_Event_Data->Remote_Device, BoardStr);

            /* Display this GAP Encryption Change Result.               */
            printf("GAP Encryption Change: %s.\r\n", BoardStr);
            printf("   Encryption Mode: 0x%02X\r\n", GAP_Event_Data->Event_Data.GAP_Encryption_Mode_Event_Data->Encryption_Mode);
            printf("   Status:          0x%02X\r\n", GAP_Event_Data->Event_Data.GAP_Encryption_Mode_Event_Data->Encryption_Change_Status);

            /* * NOTE * Since these events are received before we set   */
            /*          the device information in                       */
            /*          GATT_Connection_Event_Callback(), we will wait  */
            /*          to flag that encryption has been enabled in the */
            /*          device information until we are successfully    */
            /*          connected over GATT.  The device information is */
            /*          set later since we do not receive a connection  */
            /*          event for BR/EDR, which is not the case for LE. */
            /*          Since the encryption checks for this dual mode  */
            /*          sample use the device information, we need to   */
            /*          make sure we flag it for BR/EDR connections.    */
            break;
         case etInquiry_Entry_Result:
            /* Next convert the BD_ADDR to a string.                    */
            BD_ADDRToStr(GAP_Event_Data->Event_Data.GAP_Inquiry_Entry_Event_Data->BD_ADDR, BoardStr);

            /* Display this GAP Inquiry Entry Result.                   */
            printf("GAP Inquiry Entry: %s.\r\n", BoardStr);
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

                  /* See if we can find the stored device information   */
                  /* for the remote device.                             */
                  if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device)) != NULL)
                  {
                     /* If the link key is valid.                       */
                     if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_LINK_KEY_VALID)
                     {
                        /* Link Key information stored, go ahead and    */
                        /* respond with the stored Link Key.            */
                        GAP_Authentication_Information.Authentication_Data_Length   = sizeof(Link_Key_t);
                        GAP_Authentication_Information.Authentication_Data.Link_Key = DeviceInfo->LinkKey;
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

                  /* Store the link key so that we can assign it to the */
                  /* device information that will be created in the GATT*/
                  /* Connection Event Callback(): when the              */
                  /* etGATT_Connection_Device_Connection event is       */
                  /* received.                                          */
                  CreatedLinkKey = GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Authentication_Event_Data.Link_Key_Info.Link_Key;

                  /* Display the link key that has being created.       */
                  printf("Link Key: 0x");

                  for(Index = 0;Index<sizeof(Link_Key_t);Index++)
                     printf("%02X", ((Byte_t *)(&(CreatedLinkKey)))[Index]);
                  printf("\r\n");

                  break;
               case atIOCapabilityRequest:
                  BD_ADDRToStr(GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device, BoardStr);
                  printf("atIOCapabilityRequest: %s\r\n", BoardStr);

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
                     printf("\r\nAuto Accepting: %u\r\n", GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Authentication_Event_Data.Numeric_Value);

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
                     printf("User Confirmation: %u\r\n", GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Authentication_Event_Data.Numeric_Value);

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

                  printf("Passkey Value: %u\r\n", GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Authentication_Event_Data.Numeric_Value);
                  break;
               case atKeypressNotification:
                  BD_ADDRToStr(GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device, BoardStr);
                  printf("atKeypressNotification: %s\r\n", BoardStr);

                  printf("Keypress: %d\r\n", (int)GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Authentication_Event_Data.Keypress_Type);
                  break;
               case atSecureSimplePairingComplete:
                  BD_ADDRToStr(GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device, BoardStr);
                  printf("atSecureSimplePairingComplete: %s\r\n", BoardStr);
               default:
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
            break;
      }

      DisplayPrompt();
   }
   else
   {
      /* There was an error with one or more of the input parameters.   */
      printf("\r\n");

      printf("GAP Callback Data: Event_Data = NULL.\r\n");

      DisplayPrompt();
   }

   /* Make sure the output is displayed to the user.                    */
   fflush(stdout);
}

   /* ***************************************************************** */
   /*                         Event Callbacks                           */
   /* ***************************************************************** */

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
   GAP_LE_Security_Information_t                 GAP_LE_Security_Information;
   GAP_LE_Advertising_Report_Data_t             *DeviceEntryPtr;
   GAP_LE_Authentication_Event_Data_t           *Authentication_Event_Data;
   GAP_LE_Authentication_Response_Information_t  GAP_LE_Authentication_Response_Information;

   /* Verify that all parameters to this callback are Semi-Valid.       */
   if((BluetoothStackID) && (GAP_LE_Event_Data))
   {
      switch(GAP_LE_Event_Data->Event_Data_Type)
      {
         case etLE_Advertising_Report:
            printf("etLE_Advertising_Report with size %d.\r\n",(int)GAP_LE_Event_Data->Event_Data_Size);
            printf("  %d Responses.\r\n",GAP_LE_Event_Data->Event_Data.GAP_LE_Advertising_Report_Event_Data->Number_Device_Entries);

            for(Index = 0; Index < GAP_LE_Event_Data->Event_Data.GAP_LE_Advertising_Report_Event_Data->Number_Device_Entries; Index++)
            {
               DeviceEntryPtr = &(GAP_LE_Event_Data->Event_Data.GAP_LE_Advertising_Report_Event_Data->Advertising_Data[Index]);

               /* Display the packet type for the device                */
               switch(DeviceEntryPtr->Advertising_Report_Type)
               {
                  case rtConnectableUndirected:
                     printf("  Advertising Type: %s.\r\n", "rtConnectableUndirected");
                     break;
                  case rtConnectableDirected:
                     printf("  Advertising Type: %s.\r\n", "rtConnectableDirected");
                     break;
                  case rtScannableUndirected:
                     printf("  Advertising Type: %s.\r\n", "rtScannableUndirected");
                     break;
                  case rtNonConnectableUndirected:
                     printf("  Advertising Type: %s.\r\n", "rtNonConnectableUndirected");
                     break;
                  case rtScanResponse:
                     printf("  Advertising Type: %s.\r\n", "rtScanResponse");
                     break;
               }

               /* Display the Address Type.                             */
               if(DeviceEntryPtr->Address_Type == latPublic)
               {
                  printf("  Address Type: %s.\r\n","atPublic");
               }
               else
               {
                  printf("  Address Type: %s.\r\n","atRandom");
               }

               /* Display the Device Address.                           */
               printf("  Address: 0x%02X%02X%02X%02X%02X%02X.\r\n", DeviceEntryPtr->BD_ADDR.BD_ADDR5, DeviceEntryPtr->BD_ADDR.BD_ADDR4, DeviceEntryPtr->BD_ADDR.BD_ADDR3, DeviceEntryPtr->BD_ADDR.BD_ADDR2, DeviceEntryPtr->BD_ADDR.BD_ADDR1, DeviceEntryPtr->BD_ADDR.BD_ADDR0);
               printf("  RSSI: 0x%02X.\r\n", DeviceEntryPtr->RSSI);
               printf("  Data Length: %d.\r\n", DeviceEntryPtr->Raw_Report_Length);

               DisplayAdvertisingData(&(DeviceEntryPtr->Advertising_Data));
            }
            break;
         case etLE_Connection_Complete:
            printf("etLE_Connection_Complete with size %d.\r\n",(int)GAP_LE_Event_Data->Event_Data_Size);

            if(GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data)
            {
               BD_ADDRToStr(GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Peer_Address, BoardStr);

               printf("   Status:       0x%02X.\r\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Status);
               printf("   Role:         %s.\r\n", (GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Master)?"Master":"Slave");
               printf("   Address Type: %s.\r\n", (GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Peer_Address_Type == latPublic)?"Public":"Random");
               printf("   BD_ADDR:      %s.\r\n", BoardStr);

               if(GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Status == HCI_ERROR_CODE_NO_ERROR)
               {
                  ConnectionBD_ADDR   = GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Peer_Address;
                  LocalDeviceIsMaster = GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Master;

                  /* Make sure that no entry already exists.            */
                  if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) == NULL)
                  {
                     /* No entry exists so create one.                  */
                     if(!CreateNewDeviceInfoEntry(&DeviceInfoList, GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Peer_Address_Type, ConnectionBD_ADDR))
                        printf("Failed to add device to Device Info List.\r\n");
                  }
                  else
                  {
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
                           BTPS_MemCopy(&(GAP_LE_Security_Information.Security_Information.Master_Information.LTK), &(DeviceInfo->LTK), sizeof(DeviceInfo->LTK));
                           GAP_LE_Security_Information.Security_Information.Master_Information.EDIV                = DeviceInfo->EDIV;
                           BTPS_MemCopy(&(GAP_LE_Security_Information.Security_Information.Master_Information.Rand), &(DeviceInfo->Rand), sizeof(DeviceInfo->Rand));
                           GAP_LE_Security_Information.Security_Information.Master_Information.Encryption_Key_Size = DeviceInfo->EncryptionKeySize;

                           Result = GAP_LE_Reestablish_Security(BluetoothStackID, ConnectionBD_ADDR, &GAP_LE_Security_Information, GAP_LE_Event_Callback, 0);
                           if(Result)
                           {
                              DisplayFunctionError("GAP_LE_Reestablish_Security", Result);
                           }
                        }
                     }
                  }
               }
            }
            break;
         case etLE_Disconnection_Complete:
            printf("etLE_Disconnection_Complete with size %d.\r\n", (int)GAP_LE_Event_Data->Event_Data_Size);

            if(GAP_LE_Event_Data->Event_Data.GAP_LE_Disconnection_Complete_Event_Data)
            {
               printf("   Status: 0x%02X.\r\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Disconnection_Complete_Event_Data->Status);
               printf("   Reason: 0x%02X.\r\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Disconnection_Complete_Event_Data->Reason);

               BD_ADDRToStr(GAP_LE_Event_Data->Event_Data.GAP_LE_Disconnection_Complete_Event_Data->Peer_Address, BoardStr);
               printf("   BD_ADDR: %s.\r\n", BoardStr);

               /* Check to see if the device info is present in the     */
               /* list.                                                 */
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
               {
                  /* Flag that no service discovery operation is        */
                  /* outstanding for this device.                       */
                  DeviceInfo->Flags &= ~DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_OUTSTANDING;

                  /* If this device is not paired , then delete it.  The*/
                  /* link will be encrypted if the device is paired.    */
                  if((!(DeviceInfo->Flags & DEVICE_INFO_FLAGS_LINK_ENCRYPTED)) || (!(DeviceInfo->Flags & DEVICE_INFO_FLAGS_LTK_VALID)))
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
               else
                  printf("Warning - Disconnect from unknown device.\r\n");

               /* Clear the saved Connection BD_ADDR.                   */
               ASSIGN_BD_ADDR(ConnectionBD_ADDR, 0, 0, 0, 0, 0, 0);
               LocalDeviceIsMaster = FALSE;
            }
            break;
         case etLE_Encryption_Change:
            printf("etLE_Encryption_Change with size %d.\r\n",(int)GAP_LE_Event_Data->Event_Data_Size);

            if(GAP_LE_Event_Data->Event_Data.GAP_LE_Encryption_Change_Event_Data)
            {
               /* Search for the device entry to see flag if the link is*/
               /* encrypted.                                            */
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, GAP_LE_Event_Data->Event_Data.GAP_LE_Encryption_Change_Event_Data->BD_ADDR)) != NULL)
               {
                  /* Check to see if the encryption change was          */
                  /* successful.                                        */
                  if((GAP_LE_Event_Data->Event_Data.GAP_LE_Encryption_Change_Event_Data->Encryption_Change_Status == HCI_ERROR_CODE_NO_ERROR) && (GAP_LE_Event_Data->Event_Data.GAP_LE_Encryption_Change_Event_Data->Encryption_Mode == emEnabled))
                     DeviceInfo->Flags |= DEVICE_INFO_FLAGS_LINK_ENCRYPTED;
                  else
                     DeviceInfo->Flags &= ~DEVICE_INFO_FLAGS_LINK_ENCRYPTED;
               }
            }
            break;
         case etLE_Encryption_Refresh_Complete:
            printf("etLE_Encryption_Refresh_Complete with size %d.\r\n", (int)GAP_LE_Event_Data->Event_Data_Size);

            if(GAP_LE_Event_Data->Event_Data.GAP_LE_Encryption_Refresh_Complete_Event_Data)
            {
               /* Search for the device entry to see flag if the link is*/
               /* encrypted.                                            */
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, GAP_LE_Event_Data->Event_Data.GAP_LE_Encryption_Refresh_Complete_Event_Data->BD_ADDR)) != NULL)
               {
                  /* Check to see if the refresh was successful.        */
                  if(GAP_LE_Event_Data->Event_Data.GAP_LE_Encryption_Refresh_Complete_Event_Data->Status == HCI_ERROR_CODE_NO_ERROR)
                     DeviceInfo->Flags |= DEVICE_INFO_FLAGS_LINK_ENCRYPTED;
                  else
                     DeviceInfo->Flags &= ~DEVICE_INFO_FLAGS_LINK_ENCRYPTED;
               }
            }
            break;
         case etLE_Authentication:
            printf("etLE_Authentication with size %d.\r\n", (int)GAP_LE_Event_Data->Event_Data_Size);

            /* Make sure the authentication event data is valid before  */
            /* continuing.                                              */
            if((Authentication_Event_Data = GAP_LE_Event_Data->Event_Data.GAP_LE_Authentication_Event_Data) != NULL)
            {
               BD_ADDRToStr(Authentication_Event_Data->BD_ADDR, BoardStr);

               switch(Authentication_Event_Data->GAP_LE_Authentication_Event_Type)
               {
                  case latLongTermKeyRequest:
                     printf("latKeyRequest: \r\n");
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
                              printf("      Stored LTK valid.\r\n");

                              /* Respond with the stored Long Term Key. */
                              GAP_LE_Authentication_Response_Information.Authentication_Data_Length                                        = GAP_LE_LONG_TERM_KEY_INFORMATION_DATA_SIZE;
                              GAP_LE_Authentication_Response_Information.Authentication_Data.Long_Term_Key_Information.Encryption_Key_Size = DeviceInfo->EncryptionKeySize;

                              BTPS_MemCopy(&(GAP_LE_Authentication_Response_Information.Authentication_Data.Long_Term_Key_Information.Long_Term_Key), &(DeviceInfo->LTK), LONG_TERM_KEY_SIZE);
                           }
                        }
                     }
                     else
                     {
                        printf("      Stored LTK missing.\r\n");

                        /* The other side of a connection is requesting */
                        /* that we start encryption.  Thus we should    */
                        /* regenerate LTK for this connection and send  */
                        /* it to the chip.                              */
                        Result = GAP_LE_Regenerate_Long_Term_Key(BluetoothStackID, (Encryption_Key_t *)(&DHK), (Encryption_Key_t *)(&ER), Authentication_Event_Data->Authentication_Event_Data.Long_Term_Key_Request.EDIV, &(Authentication_Event_Data->Authentication_Event_Data.Long_Term_Key_Request.Rand), &GeneratedLTK);
                        if(!Result)
                        {
                           printf("      GAP_LE_Regenerate_Long_Term_Key Success.\r\n");

                           /* Respond with the Re-Generated Long Term   */
                           /* Key.                                      */
                           GAP_LE_Authentication_Response_Information.GAP_LE_Authentication_Type                                        = larLongTermKey;
                           GAP_LE_Authentication_Response_Information.Authentication_Data_Length                                        = GAP_LE_LONG_TERM_KEY_INFORMATION_DATA_SIZE;
                           GAP_LE_Authentication_Response_Information.Authentication_Data.Long_Term_Key_Information.Encryption_Key_Size = GAP_LE_MAXIMUM_ENCRYPTION_KEY_SIZE;
                           GAP_LE_Authentication_Response_Information.Authentication_Data.Long_Term_Key_Information.Long_Term_Key       = GeneratedLTK;
                        }
                        else
                           DisplayFunctionError("GAP_LE_Regenerate_Long_Term_Key", Result);
                     }

                     /* Send the Authentication Response.               */
                     Result = GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information);
                     if(Result)
                     {
                        printf("GAP_LE_Authentication_Response returned %d.\r\n",Result);
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
                           BTPS_MemCopy(&(GAP_LE_Security_Information.Security_Information.Master_Information.LTK), &(DeviceInfo->LTK), sizeof(DeviceInfo->LTK));
                           GAP_LE_Security_Information.Security_Information.Master_Information.EDIV                = DeviceInfo->EDIV;
                           BTPS_MemCopy(&(GAP_LE_Security_Information.Security_Information.Master_Information.Rand), &(DeviceInfo->Rand), sizeof(DeviceInfo->Rand));
                           GAP_LE_Security_Information.Security_Information.Master_Information.Encryption_Key_Size = DeviceInfo->EncryptionKeySize;

                           Result = GAP_LE_Reestablish_Security(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Security_Information, GAP_LE_Event_Callback, 0);
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

                     /* Send the Pairing Response.                      */
                     SlavePairingRequestResponse(Authentication_Event_Data->BD_ADDR);
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
                     SlavePairingRequestResponse(Authentication_Event_Data->BD_ADDR);
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
                           GAP_LE_Authentication_Response_Information.Authentication_Data_Length = DWORD_SIZE;

                           /* Handle this differently based on the local*/
                           /* IO Caps.                                  */
                           switch(LE_Parameters.IOCapability)
                           {
                              case licNoInputNoOutput:
                                 printf("Invoking Just Works.\r\n");

                                 /* By setting the                      */
                                 /* Authentication_Data_Length to any   */
                                 /* NON-ZERO value we are informing the */
                                 /* GAP LE Layer that we are accepting  */
                                 /* Just Works Pairing.                 */

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
                  default:
                     break;
               }
            }
            break;
         default:
            break;
      }

      /* Display the command prompt.                                    */
      DisplayPrompt();
   }
}

   /* The following is a IPS Server Event Callback.  This function will */
   /* be called whenever an IPS Server Profile Event occurs that is     */
   /* associated with the specified Bluetooth Stack ID.  This function  */
   /* passes to the caller the Bluetooth Stack ID, the IPS Event Data   */
   /* that occurred and the IPS Event Callback Parameter that was       */
   /* specified when this Callback was The caller is free to installed. */
   /* use the contents of the IPS Event Data ONLY in the context If the */
   /* caller requires the Data for a longer period of of this callback. */
   /* time, then the callback function MUST copy the data into another  */
   /* Data Buffer This function is guaranteed NOT to be invoked more    */
   /* than once simultaneously for the specified this function DOES NOT */
   /* have be re-entrant).  It needs to be installed callback (i.e.     */
   /* noted however, that if the same Callback is installed more than   */
   /* once, then the callbacks will Because of be called serially.      */
   /* this, the processing in this function should be as It should also */
   /* be noted that this function is called in efficient as possible.   */
   /* the Thread Context of a Thread that the User does NOT own.        */
   /* Therefore, processing in this function should be as efficient as  */
   /* possible (this argument holds anyway because another IPS Event    */
   /* will not be processed while this function call is outstanding).   */
   /* ** NOTE ** This function MUST NOT Block and wait for events that  */
   /*            can only be satisfied by Receiving IPS Event Packets.  */
   /*            A Deadlock WILL occur because NO IPS Event Callbacks   */
   /*            will be issued while this function is currently        */
   /*            outstanding.                                           */
static void BTPSAPI IPS_EventCallback(unsigned int BluetoothStackID, IPS_Event_Data_t *IPS_Event_Data, unsigned long CallbackParameter)
{
   int                       Result;
   BD_ADDR_t                 RemoteDevice;
   BoardStr_t                BoardStr;
   unsigned int              InstanceID;
   unsigned int              ConnectionID;
   unsigned int              TransactionID;
   GATT_Connection_Type_t    ConnectionType;
   DeviceInfo_t             *DeviceInfo;

   /* Verify that all parameters to this callback are Semi-Valid.       */
   if((BluetoothStackID) && (IPS_Event_Data))
   {
      /* Determine the IPS Event type.                                  */
      switch(IPS_Event_Data->Event_Data_Type)
      {
         case etIPS_Server_Characteristic_Read:
            printf("\r\netIPS_Server_Characteristic_Read with size %u.\r\n", IPS_Event_Data->Event_Data_Size);
            if(IPS_Event_Data->Event_Data.IPS_Server_Characteristic_Read_Data)
            {
               /* Print event information.                              */
               RemoteDevice    = IPS_Event_Data->Event_Data.IPS_Server_Characteristic_Read_Data->RemoteDevice;
               BD_ADDRToStr(RemoteDevice, BoardStr);
               InstanceID      = IPS_Event_Data->Event_Data.IPS_Server_Characteristic_Read_Data->InstanceID;
               ConnectionID    = IPS_Event_Data->Event_Data.IPS_Server_Characteristic_Read_Data->ConnectionID;
               TransactionID   = IPS_Event_Data->Event_Data.IPS_Server_Characteristic_Read_Data->TransactionID;
               ConnectionType  = IPS_Event_Data->Event_Data.IPS_Server_Characteristic_Read_Data->ConnectionType;

               printf("   Instance ID:          %u.\r\n", InstanceID);
               printf("   Connection ID:        %u.\r\n", ConnectionID);
               printf("   Transaction ID:       %u.\r\n", TransactionID);
               printf("   Connection Type:      %s.\r\n", ((ConnectionType == gctLE)?"LE":"BR/EDR"));
               printf("   Remote Device:        %s.\r\n", BoardStr);
               printf("   Characteristic Type:  ");

               /* Display the IPS Characteristic type.                  */
               DisplayIPSCharacteristicType(IPS_Event_Data->Event_Data.IPS_Server_Characteristic_Read_Data->Type);
            }
            break;
         case etIPS_Server_Characteristic_Updated:
            printf("\r\netIPS_Server_Characteristic_Updated with size %u.\r\n", IPS_Event_Data->Event_Data_Size);
            if(IPS_Event_Data->Event_Data.IPS_Server_Characteristic_Updated_Data)
            {
               /* Print event information.                              */
               RemoteDevice    = IPS_Event_Data->Event_Data.IPS_Server_Characteristic_Updated_Data->RemoteDevice;
               BD_ADDRToStr(RemoteDevice, BoardStr);
               InstanceID      = IPS_Event_Data->Event_Data.IPS_Server_Characteristic_Updated_Data->InstanceID;
               ConnectionID    = IPS_Event_Data->Event_Data.IPS_Server_Characteristic_Updated_Data->ConnectionID;
               TransactionID   = IPS_Event_Data->Event_Data.IPS_Server_Characteristic_Updated_Data->TransactionID;
               ConnectionType  = IPS_Event_Data->Event_Data.IPS_Server_Characteristic_Updated_Data->ConnectionType;

               printf("   Instance ID:          %u.\r\n", InstanceID);
               printf("   Connection ID:        %u.\r\n", ConnectionID);
               printf("   Transaction ID:       %u.\r\n", TransactionID);
               printf("   Connection Type:      %s.\r\n", ((ConnectionType == gctLE)?"LE":"BR/EDR"));
               printf("   Remote Device:        %s.\r\n", BoardStr);

               /* Display the updated characteristics.                  */
               DisplayUpdatedCharacteristics(IPS_Event_Data->Event_Data.IPS_Server_Characteristic_Updated_Data->Characteristic);
            }
            break;
         case etIPS_Server_Authorization_Request:
            printf("\r\netIPS_Server_Authorization_Request with size %u.\r\n", IPS_Event_Data->Event_Data_Size);
            if(IPS_Event_Data->Event_Data.IPS_Server_Authorization_Request_Data)
            {
               /* Print event information.                              */
               RemoteDevice    = IPS_Event_Data->Event_Data.IPS_Server_Authorization_Request_Data->RemoteDevice;
               BD_ADDRToStr(RemoteDevice, BoardStr);
               InstanceID      = IPS_Event_Data->Event_Data.IPS_Server_Authorization_Request_Data->InstanceID;
               ConnectionID    = IPS_Event_Data->Event_Data.IPS_Server_Authorization_Request_Data->ConnectionID;
               TransactionID   = IPS_Event_Data->Event_Data.IPS_Server_Authorization_Request_Data->TransactionID;
               ConnectionType  = IPS_Event_Data->Event_Data.IPS_Server_Authorization_Request_Data->ConnectionType;

               printf("   Instance ID:          %u.\r\n", InstanceID);
               printf("   Connection ID:        %u.\r\n", ConnectionID);
               printf("   Transaction ID:       %u.\r\n", TransactionID);
               printf("   Connection Type:      %s.\r\n", ((ConnectionType == gctLE)?"LE":"BR/EDR"));
               printf("   Remote Device:        %s.\r\n", BoardStr);
               printf("   Characteristic Type:  ");

               /* Display the IPS Characteristic type.                  */
               DisplayIPSCharacteristicType(IPS_Event_Data->Event_Data.IPS_Server_Authorization_Request_Data->Type);

               /* Get the device information.                           */
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList,  RemoteDevice)) != NULL)
               {
                  /* For simplicitly this sample application will just  */
                  /* use whether we are encrypted or not to             */
                  /* automatically authorize requests.                  */
                  if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_LINK_ENCRYPTED)
                  {
                     /* Send the authorized response.                   */
                     if((Result = IPS_Authorization_Request_Response(BluetoothStackID, InstanceID, TransactionID, FALSE)) != 0)
                        DisplayFunctionError("IPS_Authorization_Request_Response", Result);
                  }
                  else
                  {
                     /* Send the unauthorized response.                 */
                     if((Result = IPS_Authorization_Request_Response(BluetoothStackID, InstanceID, TransactionID, TRUE)) != 0)
                        DisplayFunctionError("IPS_Authorization_Request_Response", Result);
                  }
               }
               else
                  printf("\r\nNo device information.\r\n");
            }
            break;
         default:
            printf("\r\nUnknown IPS Event\r\n");
            break;
      }
   }
   else
   {
      /* There was an error with one or more of the input parameters.   */
      printf("\r\nGATT Callback Data: Event_Data = NULL.\r\n");
   }

   DisplayPrompt();
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
static void BTPSAPI GATT_ClientEventCallback_IPS(unsigned int BluetoothStackID, GATT_Client_Event_Data_t *GATT_Client_Event_Data, unsigned long CallbackParameter)
{
   /* Common                                                            */
   int                        Result;
   unsigned int               Index;

   /* GATT ESS Client Event Callback data types.                        */
   BoardStr_t                 BoardStr;
   BD_ADDR_t                  RemoteDevice;
   DeviceInfo_t              *DeviceInfo;
   unsigned int               ConnectionID;
   unsigned int               TransactionID;
   GATT_Connection_Type_t     ConnectionType;
   GATT_Request_Error_Type_t  ErrorType;
   Word_t                     AttributeHandle;
   Word_t                     BytesWritten;
   Word_t                     ValueOffset;
   Word_t                     ValueLength;
   Byte_t                    *Value;

   /* IPS Response data types.                                          */
   Word_t                     MTU;
   Word_t                     CurrentOffset;
   Word_t                     ExpectedLength;
   Word_t                     PreparedWriteLength;

   Word_t                     Extended_Properties;
   unsigned int               Counter = 1;

   /* Verify that all parameters to this callback are Semi-Valid.       */
   if((BluetoothStackID) && (GATT_Client_Event_Data))
   {
      /* Determine the event that occurred.                             */
      switch(GATT_Client_Event_Data->Event_Data_Type)
      {
         case etGATT_Client_Error_Response:
            printf("\r\netGATT_Client_Error_Response.\r\n");
            if(GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data)
            {
               /* Store the event data.                                 */
               ConnectionID   = GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ConnectionID;
               RemoteDevice   = GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->RemoteDevice;
               TransactionID  = GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->TransactionID;
               ConnectionType = GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ConnectionType;
               ErrorType      = GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorType;

               printf("   Connection ID:    %u.\r\n", ConnectionID);
               printf("   Transaction ID:   %u.\r\n", TransactionID);
               printf("   Connection Type:  %s.\r\n", (ConnectionType == gctLE)?"LE":"BR/EDR");
               BD_ADDRToStr(RemoteDevice, BoardStr);
               printf("   BD_ADDR:          %s.\r\n", BoardStr);
               printf("   Error Type:       %s.\r\n", (ErrorType == retErrorResponse)? "Response Error" : "Response Timeout");

               /* Only print out the rest if it is valid.               */
               if(GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorType == retErrorResponse)
               {
                  printf("   Request Opcode:   0x%02X.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->RequestOpCode);
                  printf("   Request Handle:   0x%04X.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->RequestHandle);
                  printf("   Error Code:       ");

                  switch(GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorCode)
                  {
                     case IPS_ERROR_CODE_INVALID_VALUE:
                        printf("Invalid value.\r\n");
                        break;
                     case ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_AUTHORIZATION:
                        printf("Insufficient Authorization.\r\n");
                        break;
                     case ATT_PROTOCOL_ERROR_CODE_PREPARE_QUEUE_FULL:
                        printf("Prepare Queue Full.\r\n");
                        break;
                     case ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR:
                        printf("Unlikely error.\r\n");
                        break;
                     default:
                        printf("0x%02X.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorCode);
                        break;
                  }
               }
            }
            else
               printf("Error - Null Error Response Data.\r\n");
            break;
         case etGATT_Client_Read_Response:
            printf("\r\netGATT_Client_Read_Response.\r\n");
            if(GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data)
            {
               /* Store the event data.                                 */
               ConnectionID   = GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->ConnectionID;
               RemoteDevice   = GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->RemoteDevice;
               TransactionID  = GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->TransactionID;
               ConnectionType = GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->ConnectionType;
               ValueLength    = GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength;
               Value          = GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValue;

               /* Print the event data.                                 */
               printf("   Connection ID:    %u.\r\n", ConnectionID);
               printf("   Transaction ID:   %u.\r\n", TransactionID);
               printf("   Connection Type:  %s.\r\n", (ConnectionType == gctLE)?"LE":"BR/EDR");
               BD_ADDRToStr(RemoteDevice, BoardStr);
               printf("   BD_ADDR:          %s.\r\n", BoardStr);
               printf("   Handle:           0x%04X.\r\n", (Word_t)CallbackParameter);
               printf("   Value Length:     %u.\r\n", ValueLength);

               /* If we know about this device and a callback parameter */
               /* exists, then check if we know what read response this */
               /* is.                                                   */
               if((Value) && (ValueLength != 0) && (CallbackParameter != 0))
               {
                  /* Make sure we can get the device information.       */
                  if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, RemoteDevice)) != NULL)
                  {
                     /* Check for the Indoor Positioning Characteristic.*/
                     if(DeviceInfo->IPSClientInfo.IPS_Indoor_Positioning_Configuration == CallbackParameter)
                     {
                        /* Call the internal function to decode and     */
                        /* display the characteristic.                  */
                        DecodeDisplayIPCResponse(Value, ValueLength);
                     }
                     else
                     {
                        /* Check for the Latitude Characteristic.       */
                        if(DeviceInfo->IPSClientInfo.IPS_Latitude == CallbackParameter)
                        {
                           /* Call the internal function to decode and  */
                           /* display the characteristic.               */
                           DecodeDisplayLatitudeResponse(Value, ValueLength);
                        }
                        else
                        {
                           /* Check for the Longitude Characteristic.   */
                           if(DeviceInfo->IPSClientInfo.IPS_Longitude == CallbackParameter)
                           {
                              /* Call the internal function to decode   */
                              /* and display the characteristic.        */
                              DecodeDisplayLongitudeResponse(Value, ValueLength);
                           }
                           else
                           {
                              /* Check for the Local North Coordinate   */
                              /* Characteristic.                        */
                              if(DeviceInfo->IPSClientInfo.IPS_Local_North_Coordinate == CallbackParameter)
                              {
                                 /* Call the internal function to decode*/
                                 /* and display the characteristic.     */
                                 DecodeDisplayLocalNorthCoordinateResponse(Value, ValueLength);
                              }
                              else
                              {
                                 /* Check for the Local East Coordinate */
                                 /* Characteristic.                     */
                                 if(DeviceInfo->IPSClientInfo.IPS_Local_East_Coordinate == CallbackParameter)
                                 {
                                    /* Call the internal function to    */
                                    /* decode and display the           */
                                    /* characteristic.                  */
                                    DecodeDisplayLocalEastCoordinateResponse(Value, ValueLength);
                                 }
                                 else
                                 {
                                    /* Check for the Floor Number       */
                                    /* Characteristic.                  */
                                    if(DeviceInfo->IPSClientInfo.IPS_Floor_Number == CallbackParameter)
                                    {
                                       /* Call the internal function to */
                                       /* decode and display the        */
                                       /* characteristic.               */
                                       DecodeDisplayFloorNumberResponse(Value, ValueLength);
                                    }
                                    else
                                    {
                                       /* Check for the Altitude        */
                                       /* Characteristic.               */
                                       if(DeviceInfo->IPSClientInfo.IPS_Altitude == CallbackParameter)
                                       {
                                          /* Call the internal function */
                                          /* to decode and display the  */
                                          /* characteristic.            */
                                          DecodeDisplayAltitudeResponse(Value, ValueLength);
                                       }
                                       else
                                       {
                                          /* Check for the Uncertainty  */
                                          /* Characteristic.            */
                                          if(DeviceInfo->IPSClientInfo.IPS_Uncertainty == CallbackParameter)
                                          {
                                             /* Call the internal       */
                                             /* function to decode and  */
                                             /* display the             */
                                             /* characteristic.         */
                                             DecodeDisplayUncertaintyResponse(Value, ValueLength);
                                          }
                                          else
                                          {
                                             /* Check for the Location  */
                                             /* Name Characteristic.    */
                                             if(DeviceInfo->IPSClientInfo.IPS_Location_Name == CallbackParameter)
                                             {
                                                /* Make sure the length */
                                                /* is semi-valid.       */
                                                if(ValueLength >= NON_ALIGNED_BYTE_SIZE)
                                                {
                                                   printf("\r\nLocation Name:\r\n");
                                                   printf("   Value:   %s\r\n", (char *)Value);

                                                   /* Update the offset */
                                                   /* in case a read    */
                                                   /* long needs to be  */
                                                   /* issued.           */
                                                   DeviceInfo->LocationNameOffset += ValueLength;
                                                }
                                                else
                                                   printf("   Value:   Invalid Length.\r\n");
                                             }
                                             else
                                             {
                                                /* Check the for the    */
                                                /* extended properties  */
                                                /* descriptor.          */
                                                if((DeviceInfo->IPSClientInfo.IPS_Indoor_Positioning_Configuration_Extended_Properties == CallbackParameter) ||
                                                   (DeviceInfo->IPSClientInfo.IPS_Latitude_Extended_Properties == CallbackParameter) ||
                                                   (DeviceInfo->IPSClientInfo.IPS_Longitude_Extended_Properties == CallbackParameter) ||
                                                   (DeviceInfo->IPSClientInfo.IPS_Local_North_Coordinate_Extended_Properties == CallbackParameter) ||
                                                   (DeviceInfo->IPSClientInfo.IPS_Local_East_Coordinate_Extended_Properties == CallbackParameter) ||
                                                   (DeviceInfo->IPSClientInfo.IPS_Floor_Number_Extended_Properties == CallbackParameter) ||
                                                   (DeviceInfo->IPSClientInfo.IPS_Altitude_Extended_Properties == CallbackParameter) ||
                                                   (DeviceInfo->IPSClientInfo.IPS_Uncertainty_Extended_Properties == CallbackParameter) ||
                                                   (DeviceInfo->IPSClientInfo.IPS_Location_Name_Extended_Properties == CallbackParameter))
                                                {
                                                   printf("\r\nExtended Properties:\r\n");

                                                   /* Verify the length.*/
                                                   if(ValueLength >= NON_ALIGNED_WORD_SIZE)
                                                   {
                                                      /* Decode the     */
                                                      /* Extended       */
                                                      /* Properties     */
                                                      /* Characteristic */
                                                      /* Descriptor.    */
                                                      Extended_Properties = READ_UNALIGNED_WORD_LITTLE_ENDIAN(Value);

                                                      /* Print the      */
                                                      /* cdescriptor.   */
                                                      printf("   Value:   0x%04X\r\n", Extended_Properties);
                                                   }
                                                   else
                                                      printf("   Invalid Length.\r\n");
                                                }
                                                else
                                                {
                                                   printf("\r\nWarning - Unknown Characteristic Read Response.\r\n");
                                                }
                                             }
                                          }
                                       }
                                    }
                                 }
                              }
                           }
                        }
                     }
                  }
                  else
                     printf("\r\nError - Unknown device.\r\n");
               }
               else
                  printf("\r\nInvalid event data/parameters.\r\n");
            }
            else
               printf("\r\nError - Null Read Response Data.\r\n");
            break;
         case etGATT_Client_Read_Long_Response:
            printf("\r\netGATT_Client_Read_Long_Response.\r\n");
            if(GATT_Client_Event_Data->Event_Data.GATT_Read_Long_Response_Data)
            {
               /* Store the event data.                                 */
               ConnectionID   = GATT_Client_Event_Data->Event_Data.GATT_Read_Long_Response_Data->ConnectionID;
               RemoteDevice   = GATT_Client_Event_Data->Event_Data.GATT_Read_Long_Response_Data->RemoteDevice;
               TransactionID  = GATT_Client_Event_Data->Event_Data.GATT_Read_Long_Response_Data->TransactionID;
               ConnectionType = GATT_Client_Event_Data->Event_Data.GATT_Read_Long_Response_Data->ConnectionType;
               ValueLength    = GATT_Client_Event_Data->Event_Data.GATT_Read_Long_Response_Data->AttributeValueLength;
               Value          = GATT_Client_Event_Data->Event_Data.GATT_Read_Long_Response_Data->AttributeValue;

               /* Print the event data.                                 */
               printf("   Connection ID:    %u.\r\n", ConnectionID);
               printf("   Transaction ID:   %u.\r\n", TransactionID);
               printf("   Connection Type:  %s.\r\n", (ConnectionType == gctLE)?"LE":"BR/EDR");
               BD_ADDRToStr(RemoteDevice, BoardStr);
               printf("   BD_ADDR:          %s.\r\n", BoardStr);
               printf("   Handle:           0x%04X.\r\n", (Word_t)CallbackParameter);
               printf("   Value Length:     %u.\r\n", ValueLength);

               /* If we know about this device and a callback parameter */
               /* exists, then check if we know what read response this */
               /* is.                                                   */
               if((Value) && (ValueLength != 0) && (CallbackParameter != 0))
               {
                  /* Make sure we can get the device information.       */
                  if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, RemoteDevice)) != NULL)
                  {
                     /* The only read long response can be for the      */
                     /* location name so we will check for its handle.  */
                     if(DeviceInfo->IPSClientInfo.IPS_Location_Name == CallbackParameter)
                     {
                        /* Check to make sure the value length does not */
                        /* exceed the maximum size of the Location Name */
                        /* Queue that the client can hold.              */
                        if((DeviceInfo->LocationNameOffset + ValueLength) <= (unsigned int)BTPS_CONFIGURATION_IPS_MAXIMUM_SUPPORTED_STRING_LENGTH)
                        {
                           /* Store the data.                           */
                           BTPS_MemCopy((DeviceInfo->LocationName + DeviceInfo->LocationNameOffset), Value, ValueLength);

                           /* Update the offset.                        */
                           DeviceInfo->LocationNameOffset += ValueLength;

                           /* Make sure we don't go past the end of the */
                           /* string if we receive an incomplete string.*/
                           DeviceInfo->LocationName[DeviceInfo->LocationNameOffset] = '\0';

                           /* Display the Location Name.                */
                           printf("\r\nLocation Name:\r\n");
                           printf("   Value:   %s\r\n", DeviceInfo->LocationName);
                        }
                        else
                           printf("\r\nError - Length exceeds size of Location Name.\r\n");
                     }
                     else
                        printf("\r\nError - Invalid handle.\r\n");
                  }
                  else
                     printf("\r\nError - Unknown device.\r\n");
               }
               else
                  printf("\r\nInvalid event data/parameters.\r\n");
            }
            else
               printf("\r\nError - Null Read Long Response Data.\r\n");
            break;
         case etGATT_Client_Read_Multiple_Response:
            printf("\r\netGATT_Client_Read_Multiple_Response.\r\n");
            if(GATT_Client_Event_Data->Event_Data.GATT_Read_Multiple_Response_Data)
            {
               /* Store the event data.                                 */
               ConnectionID   = GATT_Client_Event_Data->Event_Data.GATT_Read_Multiple_Response_Data->ConnectionID;
               RemoteDevice   = GATT_Client_Event_Data->Event_Data.GATT_Read_Multiple_Response_Data->RemoteDevice;
               TransactionID  = GATT_Client_Event_Data->Event_Data.GATT_Read_Multiple_Response_Data->TransactionID;
               ConnectionType = GATT_Client_Event_Data->Event_Data.GATT_Read_Multiple_Response_Data->ConnectionType;

               /* We will simply re-use the ValueLength and Value       */
               /* variables to refer to the AttributeValuesLength and   */
               /* AttributeValues.                                      */
               ValueLength    = GATT_Client_Event_Data->Event_Data.GATT_Read_Multiple_Response_Data->AttributeValuesLength;
               Value          = GATT_Client_Event_Data->Event_Data.GATT_Read_Multiple_Response_Data->AttributeValues;

               /* Print the event data.                                 */
               printf("   Connection ID:    %u.\r\n", ConnectionID);
               printf("   Transaction ID:   %u.\r\n", TransactionID);
               printf("   Connection Type:  %s.\r\n", (ConnectionType == gctLE)?"LE":"BR/EDR");
               BD_ADDRToStr(RemoteDevice, BoardStr);
               printf("   BD_ADDR:          %s.\r\n", BoardStr);
               printf("   Value Length:     %u.\r\n", ValueLength);

               printf("\r\nHandle List:\r\n");
               /* Let's print the handles we set previously before we   */
               /* issued the GATT Read Multiple Request.                */
               for(Index = 0; Index < (unsigned int)IPS_MAXIMUM_NUMBER_OF_CHARACTERISTICS; Index++)
               {
                  /* Print the hanlde only if it is valid.              */
                  /* * NOTE * These indexes correspond to the           */
                  /*          IPS_Characteristic_Type_t.                */
                  if(ReadMultipleRequestHandles[Index] != 0)
                  {
                     printf("%u. Handle:  0x%04X\r\n", Counter++, ReadMultipleRequestHandles[Index]);
                  }
               }

               /* Let's store the request data.                         */
               if(ReadMultipleRequestHandles[ictIndoorPositioningConfiguration] != 0)
               {
                  /* Call the internal function to decode and display   */
                  /* the characteristic.                                */
                  if(DecodeDisplayIPCResponse(Value, ValueLength) == 0)
                  {
                     /* Adjust the Value and ValueLength for the next   */
                     /* characteristic.                                 */
                     Value       += NON_ALIGNED_BYTE_SIZE;
                     ValueLength -= NON_ALIGNED_BYTE_SIZE;
                  }
                  else
                  {
                     /* Do not decode any more characteristics since we */
                     /* failed.                                         */
                     break;
                  }
               }

               /* Let's store the request data.                         */
               if(ReadMultipleRequestHandles[ictLatitude] != 0)
               {
                  /* Call the internal function to decode and display   */
                  /* the characteristic.                                */
                  if(DecodeDisplayLatitudeResponse(Value, ValueLength) == 0)
                  {
                     /* Adjust the Value and ValueLength for the next   */
                     /* characteristic.                                 */
                     Value       += NON_ALIGNED_SDWORD_SIZE;
                     ValueLength -= NON_ALIGNED_SDWORD_SIZE;
                  }
                  else
                  {
                     /* Do not decode any more characteristics since we */
                     /* failed.                                         */
                     break;
                  }
               }

               /* Let's store the request data.                         */
               if(ReadMultipleRequestHandles[ictLongitude] != 0)
               {
                  /* Call the internal function to decode and display   */
                  /* the characteristic.                                */
                  if(DecodeDisplayLongitudeResponse(Value, ValueLength) == 0)
                  {
                     /* Adjust the Value and ValueLength for the next   */
                     /* characteristic.                                 */
                     Value       += NON_ALIGNED_SDWORD_SIZE;
                     ValueLength -= NON_ALIGNED_SDWORD_SIZE;
                  }
                  else
                  {
                     /* Do not decode any more characteristics since we */
                     /* failed.                                         */
                     break;
                  }
               }

               /* Let's store the request data.                         */
               if(ReadMultipleRequestHandles[ictLocalNorthCoordinate] != 0)
               {
                  /* Call the internal function to decode and display   */
                  /* the characteristic.                                */
                  if(DecodeDisplayLocalNorthCoordinateResponse(Value, ValueLength) == 0)
                  {
                     /* Adjust the Value and ValueLength for the next   */
                     /* characteristic.                                 */
                     Value       += NON_ALIGNED_SWORD_SIZE;
                     ValueLength -= NON_ALIGNED_SWORD_SIZE;
                  }
                  else
                  {
                     /* Do not decode any more characteristics since we */
                     /* failed.                                         */
                     break;
                  }
               }

               /* Let's store the request data.                         */
               if(ReadMultipleRequestHandles[ictLocalEastCoordinate] != 0)
               {
                  /* Call the internal function to decode and display   */
                  /* the characteristic.                                */
                  if(DecodeDisplayLocalEastCoordinateResponse(Value, ValueLength) == 0)
                  {
                     /* Adjust the Value and ValueLength for the next   */
                     /* characteristic.                                 */
                     Value       += NON_ALIGNED_SWORD_SIZE;
                     ValueLength -= NON_ALIGNED_SWORD_SIZE;
                  }
                  else
                  {
                     /* Do not decode any more characteristics since we */
                     /* failed.                                         */
                     break;
                  }
               }

               /* Let's store the request data.                         */
               if(ReadMultipleRequestHandles[ictFloorNumber] != 0)
               {
                  /* Call the internal function to decode and display   */
                  /* the characteristic.                                */
                  if(DecodeDisplayFloorNumberResponse(Value, ValueLength) == 0)
                  {
                     /* Adjust the Value and ValueLength for the next   */
                     /* characteristic.                                 */
                     Value       += NON_ALIGNED_BYTE_SIZE;
                     ValueLength -= NON_ALIGNED_BYTE_SIZE;
                  }
                  else
                  {
                     /* Do not decode any more characteristics since we */
                     /* failed.                                         */
                     break;
                  }
               }

               /* Let's store the request data.                         */
               if(ReadMultipleRequestHandles[ictAltitude] != 0)
               {
                  /* Call the internal function to decode and display   */
                  /* the characteristic.                                */
                  if(DecodeDisplayAltitudeResponse(Value, ValueLength) == 0)
                  {
                     /* Adjust the Value and ValueLength for the next   */
                     /* characteristic.                                 */
                     Value       += NON_ALIGNED_WORD_SIZE;
                     ValueLength -= NON_ALIGNED_WORD_SIZE;
                  }
                  else
                  {
                     /* Do not decode any more characteristics since we */
                     /* failed.                                         */
                     break;
                  }
               }

               /* Let's store the request data.                         */
               if(ReadMultipleRequestHandles[ictUncertainty] != 0)
               {
                  /* Call the internal function to decode and display   */
                  /* the characteristic.                                */
                  if(DecodeDisplayUncertaintyResponse(Value, ValueLength) == 0)
                  {
                     /* Adjust the Value and ValueLength for the next   */
                     /* characteristic.                                 */
                     Value       += NON_ALIGNED_BYTE_SIZE;
                     ValueLength -= NON_ALIGNED_BYTE_SIZE;
                  }
                  else
                  {
                     /* Do not decode any more characteristics since we */
                     /* failed.                                         */
                     break;
                  }
               }

               /* Let's store the request data.                         */
               if(ReadMultipleRequestHandles[ictLocationName] != 0)
               {
                  printf("\r\nLocation Name:\r\n");
                  /* If the Location Name is attached it MUST be the    */
                  /* Last Characteristic in the read multiple response. */
                  /* Let's verify that it is at least a byte.           */
                  if(ValueLength >= NON_ALIGNED_BYTE_SIZE)
                  {
                     printf("   Value:   %s\r\n", (char *)Value);
                  }
                  else
                     printf("   Value:   Not Configured.\r\n");
               }
            }
            else
               printf("\r\nError - Null Read Multiple Response Data.\r\n");
            break;
         case etGATT_Client_Write_Response:
            printf("\r\netGATT_Client_Write_Response.\r\n");
            if(GATT_Client_Event_Data->Event_Data.GATT_Write_Response_Data)
            {
               /* Store the event data.                                 */
               ConnectionID   = GATT_Client_Event_Data->Event_Data.GATT_Write_Response_Data->ConnectionID;
               RemoteDevice   = GATT_Client_Event_Data->Event_Data.GATT_Write_Response_Data->RemoteDevice;
               TransactionID  = GATT_Client_Event_Data->Event_Data.GATT_Write_Response_Data->TransactionID;
               ConnectionType = GATT_Client_Event_Data->Event_Data.GATT_Write_Response_Data->ConnectionType;
               BytesWritten   = GATT_Client_Event_Data->Event_Data.GATT_Write_Response_Data->BytesWritten;

               /* Print the event data.                                 */
               printf("   Connection ID:    %u.\r\n", ConnectionID);
               printf("   Transaction ID:   %u.\r\n", TransactionID);
               printf("   Connection Type:  %s.\r\n", (ConnectionType == gctLE)?"LE":"BR/EDR");
               BD_ADDRToStr(RemoteDevice, BoardStr);
               printf("   BD_ADDR:          %s.\r\n", BoardStr);
               printf("   Bytes Written:    %u.\r\n", BytesWritten);
            }
            else
               printf("\r\nError - Null Write Response Data.\r\n");
            break;
         case etGATT_Client_Prepare_Write_Response:
            printf("\r\netGATT_Client_Prepare_Write_Response.\r\n");
            if(GATT_Client_Event_Data->Event_Data.GATT_Write_Response_Data)
            {
               /* Store the event data.                                 */
               ConnectionID    = GATT_Client_Event_Data->Event_Data.GATT_Prepare_Write_Response_Data->ConnectionID;
               RemoteDevice    = GATT_Client_Event_Data->Event_Data.GATT_Prepare_Write_Response_Data->RemoteDevice;
               TransactionID   = GATT_Client_Event_Data->Event_Data.GATT_Prepare_Write_Response_Data->TransactionID;
               ConnectionType  = GATT_Client_Event_Data->Event_Data.GATT_Prepare_Write_Response_Data->ConnectionType;
               BytesWritten    = GATT_Client_Event_Data->Event_Data.GATT_Prepare_Write_Response_Data->BytesWritten;
               AttributeHandle = GATT_Client_Event_Data->Event_Data.GATT_Prepare_Write_Response_Data->AttributeHandle;
               ValueOffset     = GATT_Client_Event_Data->Event_Data.GATT_Prepare_Write_Response_Data->AttributeValueOffset;
               ValueLength     = GATT_Client_Event_Data->Event_Data.GATT_Prepare_Write_Response_Data->AttributeValueLength;
               Value           = GATT_Client_Event_Data->Event_Data.GATT_Prepare_Write_Response_Data->AttributeValue;

               /* Print the event data.                                 */
               printf("   Connection ID:     %u.\r\n", ConnectionID);
               printf("   Transaction ID:    %u.\r\n", TransactionID);
               printf("   Connection Type:   %s.\r\n", (ConnectionType == gctLE)?"LE":"BR/EDR");
               BD_ADDRToStr(RemoteDevice, BoardStr);
               printf("   BD_ADDR:           %s.\r\n", BoardStr);
               printf("   Bytes Written:     %u.\r\n", BytesWritten);
               printf("   Handle:            0x%04X.\r\n", AttributeHandle);
               printf("   Value Offset:      %u.\r\n", ValueOffset);
               printf("   Value Length:      %u.\r\n", ValueLength);

               /* * NOTE * We could verify that data that was sent in   */
               /*          the request since it should be sent back to  */
               /*          us in this response, but we will not do that */
               /*          here.  We could check if the data is valid   */
               /*          and cancel the request here if it is not.    */

               /* Get the device information.                           */
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, RemoteDevice)) != NULL)
               {
                  /* Check if this is a write long response.            */
                  if(DeviceInfo->IPSClientInfo.IPS_Location_Name == AttributeHandle)
                  {
                     /* Increment the offset that we have written thus  */
                     /* far.                                            */
                     DeviceInfo->LocationNameOffset += ValueLength;
                     CurrentOffset                   = DeviceInfo->LocationNameOffset;

                     /* Determine the expected size of the attribute we */
                     /* are writing.                                    */
                     /* * NOTE * We add one to account for the NULL     */
                     /*          terminator.                            */
                     ExpectedLength = (Word_t)DeviceInfo->LocationNameLength;

                     /* Determine if we have more data to send.         */
                     if(DeviceInfo->LocationNameOffset >= ExpectedLength)
                     {
                        /* If have prepared all the data.               */
                        if(DeviceInfo->LocationNameOffset == ExpectedLength)
                        {
                           /* Inform the user to execute the write      */
                           /* request or cancel.                        */
                           printf("\r\nLocation Name has been prepared.\r\n");
                           printf("Use command: 'ExecuteWrite' to write the Location Name.\r\n");
                        }
                        else
                        {
                           printf("\r\nFailure - Location Name offset has exceeded the expected length.\r\n");
                           printf("Use command: 'ExecuteWrite' to cancel the request.\r\n");
                        }
                     }
                     else
                     {
                        /* Query the GATT Connection MTU to determine   */
                        /* how the amount of data we can send in the    */
                        /* prepare write.                               */
                        if((Result = GATT_Query_Connection_MTU(BluetoothStackID, ConnectionID, &MTU)) == 0)
                        {
                           /* Determine the length to send.             */
                           /* * NOTE * The expected length minus the    */
                           /*          offset is the length of attribute*/
                           /*          that we have sent thus far.  If  */
                           /*          that size is under the maximum   */
                           /*          size we can send for the prepare */
                           /*          write (MTU-5), then we will send */
                           /*          the remaining data, otherwise we */
                           /*          will send the max.               */
                           PreparedWriteLength = ((ExpectedLength-CurrentOffset) <= (MTU-5)) ? (ExpectedLength-CurrentOffset) : (MTU-5);

                           /* We need to send the next prepare write    */
                           /* request since we have not sent all the    */
                           /* data.                                     */
                           /* * NOTE * We will again try to send the    */
                           /*          maximimum amount of data based on*/
                           /*          MTU.                             */
                           /* * NOTE * We will not save the             */
                           /*          transactionID returned by this   */
                           /*          function, which we could use to  */
                           /*          cancel the request.              */
                           if((Result = GATT_Prepare_Write_Request(BluetoothStackID, ConnectionID, AttributeHandle, PreparedWriteLength, DeviceInfo->LocationNameOffset, (Byte_t *)(DeviceInfo->LocationName + DeviceInfo->LocationNameOffset), GATT_ClientEventCallback_IPS, AttributeHandle)) > 0)
                           {
                              printf("\r\nGATT Prepare Write Request sent:\r\n");
                              printf("   TransactionID:     %d\r\n",     Result);
                              printf("   Attribute Handle:  0x%04X\r\n", AttributeHandle);
                           }
                           else
                              DisplayFunctionError("GATT_Prepare_Write_Request", Result);
                        }
                        else
                           DisplayFunctionError("GATT_Query_Connection_MTU", Result);
                     }
                  }
               }
            }
            else
               printf("\r\nError - Null Write Response Data.\r\n");
            break;
         case etGATT_Client_Execute_Write_Response:
            printf("\r\netGATT_Client_Execute_Write_Response.\r\n");
            if(GATT_Client_Event_Data->Event_Data.GATT_Write_Response_Data)
            {
               /* Store the event data.                                 */
               ConnectionID    = GATT_Client_Event_Data->Event_Data.GATT_Execute_Write_Response_Data->ConnectionID;
               RemoteDevice    = GATT_Client_Event_Data->Event_Data.GATT_Execute_Write_Response_Data->RemoteDevice;
               TransactionID   = GATT_Client_Event_Data->Event_Data.GATT_Execute_Write_Response_Data->TransactionID;
               ConnectionType  = GATT_Client_Event_Data->Event_Data.GATT_Execute_Write_Response_Data->ConnectionType;

               /* Print the event data.                                 */
               printf("   Connection ID:     %u.\r\n", ConnectionID);
               printf("   Transaction ID:    %u.\r\n", TransactionID);
               printf("   Connection Type:   %s.\r\n", (ConnectionType == gctLE)?"LE":"BR/EDR");
               BD_ADDRToStr(RemoteDevice, BoardStr);
               printf("   BD_ADDR:           %s.\r\n", BoardStr);
            }
            else
               printf("\r\nError - Null Write Response Data.\r\n");
            break;
         case etGATT_Client_Exchange_MTU_Response:
            printf("\r\netGATT_Client_Exchange_MTU_Response.\r\n");
            if(GATT_Client_Event_Data->Event_Data.GATT_Exchange_MTU_Response_Data)
            {
               /* Store the event data.                                 */
               ConnectionID    = GATT_Client_Event_Data->Event_Data.GATT_Exchange_MTU_Response_Data->ConnectionID;
               RemoteDevice    = GATT_Client_Event_Data->Event_Data.GATT_Exchange_MTU_Response_Data->RemoteDevice;
               TransactionID   = GATT_Client_Event_Data->Event_Data.GATT_Exchange_MTU_Response_Data->TransactionID;
               ConnectionType  = GATT_Client_Event_Data->Event_Data.GATT_Exchange_MTU_Response_Data->ConnectionType;
               MTU             = GATT_Client_Event_Data->Event_Data.GATT_Exchange_MTU_Response_Data->ServerMTU;

               /* Print the event data.                                 */
               printf("   Connection ID:     %u.\r\n", ConnectionID);
               printf("   Transaction ID:    %u.\r\n", TransactionID);
               printf("   Connection Type:   %s.\r\n", (ConnectionType == gctLE)?"LE":"BR/EDR");
               BD_ADDRToStr(RemoteDevice, BoardStr);
               printf("   BD_ADDR:           %s.\r\n", BoardStr);
               printf("   MTU:               %u.\r\n", MTU);
            }
            else
               printf("\r\nError - Null Write Response Data.\r\n");
            break;
         default:
            break;
      }
   }
   else
   {
      /* There was an error with one or more of the input parameters.   */
      printf("\r\nIPS Callback Data: Event_Data = NULL.\r\n");
   }

   DisplayPrompt();
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
   BoardStr_t                 BoardStr;
   BD_ADDR_t                  RemoteDevice;
   DeviceInfo_t              *DeviceInfo;
   unsigned int               ConnectionID;
   unsigned int               TransactionID;
   GATT_Connection_Type_t     ConnectionType;
   GATT_Request_Error_Type_t  ErrorType;
   Word_t                     ValueLength;
   Byte_t                    *Value;

   /* Verify that all parameters to this callback are Semi-Valid.       */
   if((BluetoothStackID) && (GATT_Client_Event_Data))
   {
      /* Determine the event that occurred.                             */
      switch(GATT_Client_Event_Data->Event_Data_Type)
      {
         case etGATT_Client_Error_Response:
            printf("\r\netGATT_Client_Error_Response.\r\n");
            if(GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data)
            {
               /* Store the event data.                                 */
               ConnectionID   = GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ConnectionID;
               RemoteDevice   = GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->RemoteDevice;
               TransactionID  = GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->TransactionID;
               ConnectionType = GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ConnectionType;
               ErrorType      = GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorType;

               printf("   Connection ID:    %u.\r\n", ConnectionID);
               printf("   Transaction ID:   %u.\r\n", TransactionID);
               printf("   Connection Type:  %s.\r\n", (ConnectionType == gctLE)?"LE":"BR/EDR");
               BD_ADDRToStr(RemoteDevice, BoardStr);
               printf("   BD_ADDR:          %s.\r\n", BoardStr);
               printf("   Error Type:       %s.\r\n", (ErrorType == retErrorResponse)? "Response Error" : "Response Timeout");

               /* Only print out the rest if it is valid.               */
               if(GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorType == retErrorResponse)
               {
                  printf("   Request Opcode:   0x%02X.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->RequestOpCode);
                  printf("   Request Handle:   0x%04X.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->RequestHandle);
                  printf("   Error Code:       0x%02X.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorCode);

                  if(GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorCode < NUMBER_OF_ERROR_CODES)
                  {
                     printf("   Error Mesg:       %s.\r\n", ErrorCodeStr[GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorCode]);
                  }
                  else
                  {
                     printf("   Error Mesg:       Unknown.\r\n");
                  }
               }
            }
            else
               printf("\nError - Null Error Response Data.\r\n");
            break;
         case etGATT_Client_Read_Response:
            printf("\r\netGATT_Client_Read_Response.\r\n");
            if(GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data)
            {
               /* Store the event data.                                 */
               ConnectionID   = GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->ConnectionID;
               RemoteDevice   = GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->RemoteDevice;
               TransactionID  = GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->TransactionID;
               ConnectionType = GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->ConnectionType;
               ValueLength    = GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength;
               Value          = GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValue;

               /* Print the event data.                                 */
               printf("   Connection ID:    %u.\r\n", ConnectionID);
               printf("   Transaction ID:   %u.\r\n", TransactionID);
               printf("   Connection Type:  %s.\r\n", (ConnectionType == gctLE)?"LE":"BR/EDR");
               BD_ADDRToStr(RemoteDevice, BoardStr);
               printf("   BD_ADDR:          %s.\r\n", BoardStr);
               printf("   Handle:           0x%04X.\r\n", (Word_t)CallbackParameter);
               printf("   Value Length:     %u.\r\n", ValueLength);

               /* Get the device information.                           */
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
               {
                  /* Make sure the length is valid.                     */
                  if(ValueLength)
                  {
                     /* Copy the result data to a local buffer and      */
                     /* terminate the data with a NULL terminator.      */
                     BTPS_MemCopy(DeviceInfo->DISClientInfo.Buffer, Value, ValueLength);
                     DeviceInfo->DISClientInfo.Buffer[ValueLength] = '\0';

                     printf("\r\nDIS Information:\r\n");
                     switch(DeviceInfo->DISClientInfo.RequestType)
                     {
                        case dctManufacturerName:
                           printf("   Manufacturer Name: %s\r\n", DeviceInfo->DISClientInfo.Buffer);
                           break;
                        case dctModelNumber:
                           printf("   Model Number:      %s\r\n", DeviceInfo->DISClientInfo.Buffer);
                           break;
                        case dctSerialNumber:
                           printf("   Serial Number:     %s\r\n", DeviceInfo->DISClientInfo.Buffer);
                           break;
                        case dctHardwareRevision:
                           printf("   Hardware Revision: %s\r\n", DeviceInfo->DISClientInfo.Buffer);
                           break;
                        case dctFirmwareRevision:
                           printf("   Firmware Revision: %s\r\n", DeviceInfo->DISClientInfo.Buffer);
                           break;
                        case dctSoftwareRevision:
                           printf("   Software Revision: %s\r\n", DeviceInfo->DISClientInfo.Buffer);
                           break;
                        case dctSystemID:
                           printf("   System ID:\r\n");
                           printf("      ManID:          0x%02X%02X%02X%02X%02X\r\n", (Byte_t)DeviceInfo->DISClientInfo.Buffer[0], (Byte_t)DeviceInfo->DISClientInfo.Buffer[1], (Byte_t)DeviceInfo->DISClientInfo.Buffer[2], (Byte_t)DeviceInfo->DISClientInfo.Buffer[3], (Byte_t)DeviceInfo->DISClientInfo.Buffer[4]);
                           printf("      OUI:            0x%02X%02X%02X\r\n", (Byte_t)DeviceInfo->DISClientInfo.Buffer[5], (Byte_t)DeviceInfo->DISClientInfo.Buffer[6], (Byte_t)DeviceInfo->DISClientInfo.Buffer[7]);
                           break;
                        case dctIEEE11073Cert:
                           printf("   IEEE Cert dcta:    0x%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X\r\n",
                                  (Byte_t)DeviceInfo->DISClientInfo.Buffer[19], (Byte_t)DeviceInfo->DISClientInfo.Buffer[18], (Byte_t)DeviceInfo->DISClientInfo.Buffer[17], (Byte_t)DeviceInfo->DISClientInfo.Buffer[16],
                                  (Byte_t)DeviceInfo->DISClientInfo.Buffer[15], (Byte_t)DeviceInfo->DISClientInfo.Buffer[14], (Byte_t)DeviceInfo->DISClientInfo.Buffer[13], (Byte_t)DeviceInfo->DISClientInfo.Buffer[12],
                                  (Byte_t)DeviceInfo->DISClientInfo.Buffer[11], (Byte_t)DeviceInfo->DISClientInfo.Buffer[10], (Byte_t)DeviceInfo->DISClientInfo.Buffer[9],  (Byte_t)DeviceInfo->DISClientInfo.Buffer[8],
                                  (Byte_t)DeviceInfo->DISClientInfo.Buffer[7],  (Byte_t)DeviceInfo->DISClientInfo.Buffer[6],  (Byte_t)DeviceInfo->DISClientInfo.Buffer[5],  (Byte_t)DeviceInfo->DISClientInfo.Buffer[4],
                                  (Byte_t)DeviceInfo->DISClientInfo.Buffer[3],  (Byte_t)DeviceInfo->DISClientInfo.Buffer[2],  (Byte_t)DeviceInfo->DISClientInfo.Buffer[1],  (Byte_t)DeviceInfo->DISClientInfo.Buffer[0]);
                           break;
                        case dctPnPID:
                           printf("   PnP ID - Source:   %d\r\n", DeviceInfo->DISClientInfo.Buffer[0]);
                           printf("              VID:    0x%04X\r\n", READ_UNALIGNED_WORD_LITTLE_ENDIAN(&DeviceInfo->DISClientInfo.Buffer[1]));
                           printf("              PID:    0x%04X\r\n", READ_UNALIGNED_WORD_LITTLE_ENDIAN(&DeviceInfo->DISClientInfo.Buffer[3]));
                           printf("          Version:    0x%04X\r\n", READ_UNALIGNED_WORD_LITTLE_ENDIAN(&DeviceInfo->DISClientInfo.Buffer[5]));
                           break;
                        default:
                           printf("\r\nWarning - Unknown DIS attribute handle type.\r\n");
                           break;
                     }
                  }
                  else
                     printf("\r\nInvalid read response value length.\r\n");
               }
               else
                  printf("\r\nWarning - No device information.\r\n");
            }
            else
               printf("\nError - Null Read Response Data.\r\n");
            break;
         default:
            printf("\r\nWarning - Event: %u, not handled.\r\n", GATT_Client_Event_Data->Event_Data_Type);
            break;
      }
   }
   else
   {
      /* There was an error with one or more of the input parameters.   */
      printf("\r\nDIS Callback Data: Event_Data = NULL.\r\n");
   }

   DisplayPrompt();
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
               printf("   Connection ID:   %u.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ConnectionID);
               printf("   Transaction ID:  %u.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->TransactionID);
               printf("   Connection Type: %s.\r\n", (GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ConnectionType == gctLE)?"LE":"BR/EDR");
               printf("   BD_ADDR:         %s.\r\n", BoardStr);
               printf("   Error Type:      %s.\r\n", (GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorType == retErrorResponse)?"Response Error":"Response Timeout");

               /* Only print out the rest if it is valid.               */
               if(GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorType == retErrorResponse)
               {
                  printf("   Request Opcode:  0x%02X.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->RequestOpCode);
                  printf("   Request Handle:  0x%04X.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->RequestHandle);
                  printf("   Error Code:      0x%02X.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorCode);
                  printf("   Error Mesg:      %s.\r\n", ErrorCodeStr[GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorCode]);
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
   int                     Result;
   BoardStr_t              BoardStr;
   BD_ADDR_t               RemoteBDADDR;
   unsigned int            GATTConnectionID;
   GATT_Connection_Type_t  ConnectionType;
   Word_t                  MTU;
   DeviceInfo_t           *DeviceInfo;
   GAP_Encryption_Mode_t   EncryptionMode;

   /* Verify that all parameters to this callback are Semi-Valid.       */
   if((BluetoothStackID) && (GATT_Connection_Event_Data))
   {
      /* Determine the Connection Event that occurred.                  */
      switch(GATT_Connection_Event_Data->Event_Data_Type)
      {
         case etGATT_Connection_Device_Connection:
            printf("\r\netGATT_Connection_Device_Connection with size %u: \r\n", GATT_Connection_Event_Data->Event_Data_Size);
            if(GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data)
            {
               /* Save the connection address and ID for later use.     */
               ConnectionBD_ADDR = GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data->RemoteDevice;
               ConnectionID      = GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data->ConnectionID;

               /* Store the event data.                                 */
               ConnectionType = GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data->ConnectionType;
               MTU            = GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data->MTU;

               BD_ADDRToStr(ConnectionBD_ADDR, BoardStr);
               printf("   Connection ID:    %u.\r\n", ConnectionID);
               printf("   Connection Type:  %s.\r\n", ((ConnectionType == gctLE) ? "LE" : "BR/EDR"));
               printf("   Remote Device:    %s.\r\n", BoardStr);
               printf("   Connection MTU:   %u.\r\n", MTU);

               /* Since there is no connection event in GAP for BR/EDR  */
               /* we will handle any connection setup here when GATT    */
               /* connects for BR/EDR.                                  */
               /* * NOTE * This code should be simliar to the LE        */
               /*          Connection Event.                            */
               if(ConnectionType == gctBR_EDR)
               {
                  if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) == NULL)
                  {
                     /* No entry exists so create one.                  */
                     /* * NOTE * We will set the LE address type to     */
                     /*          public for the br/edr connection       */
                     /*          (required for this function).          */
                     if(CreateNewDeviceInfoEntry(&DeviceInfoList, latPublic, ConnectionBD_ADDR))
                     {
                        /* Find the device info entry that was just     */
                        /* created.                                     */
                        DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR);
                     }
                     else
                        printf("\r\nFailed to add device to Device Info List.\r\n");
                  }

                  if(DeviceInfo)
                  {
                     /* Flag that we are now connected over the BR/EDR  */
                     /* transport.                                      */
                     DeviceInfo->Flags |= DEVICE_INFO_FLAGS_BR_EDR_CONNECTED;

                     /* Let's query the encryption mode to determine if */
                     /* we need to flag that encryption has been        */
                     /* enabled.                                        */
                     /* * NOTE * We can do this since at this point we  */
                     /*          are connected to the remote device.    */
                     if((Result = GAP_Query_Encryption_Mode(BluetoothStackID, ConnectionBD_ADDR, &EncryptionMode)) == 0)
                     {
                        if(EncryptionMode != emDisabled)
                        {
                           /* If we do not have a stored link key then  */
                           /* we need to save the link key that was     */
                           /* created during pairing.                   */
                           if(!(DeviceInfo->Flags & DEVICE_INFO_FLAGS_LINK_KEY_VALID))
                           {
                              /* Store the link key.                    */
                              DeviceInfo->LinkKey = CreatedLinkKey;
                              DeviceInfo->Flags  |= DEVICE_INFO_FLAGS_LINK_KEY_VALID;
                           }

                           /* Flag that we are encrypted.               */
                           DeviceInfo->Flags |= DEVICE_INFO_FLAGS_LINK_ENCRYPTED;
                        }
                     }
                     else
                     {
                        DisplayFunctionError("GAP_Query_Encryption_Mode", Result);
                     }
                  }
                  else
                     printf("\r\nNo device Info.\r\n");
               }
               else
               {
                  /* Must be an LE connection.                          */
                  if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
                  {
                     if(LocalDeviceIsMaster)
                     {
                        /* Attempt to update the MTU to the maximum     */
                        /* supported.                                   */
                        GATT_Exchange_MTU_Request(BluetoothStackID, ConnectionID, BTPS_CONFIGURATION_GATT_DEFAULT_MAXIMUM_SUPPORTED_MTU_SIZE, GATT_ClientEventCallback_IPS, 0);
                     }
                  }
               }
            }
            else
               printf("\r\nError - Null Connection Data.\r\n");
            break;
         case etGATT_Connection_Device_Disconnection:
            printf("\r\netGATT_Connection_Device_Disconnection with size %u: \r\n", GATT_Connection_Event_Data->Event_Data_Size);
            if(GATT_Connection_Event_Data->Event_Data.GATT_Device_Disconnection_Data)
            {
               /* Store the event data.                                 */
               /* * NOTE * The global BD_ADDR and ID will be cleared at */
               /*          the end of this event.                       */
               ConnectionBD_ADDR = GATT_Connection_Event_Data->Event_Data.GATT_Device_Disconnection_Data->RemoteDevice;
               ConnectionID      = GATT_Connection_Event_Data->Event_Data.GATT_Device_Disconnection_Data->ConnectionID;
               ConnectionType    = GATT_Connection_Event_Data->Event_Data.GATT_Device_Disconnection_Data->ConnectionType;

               BD_ADDRToStr(ConnectionBD_ADDR, BoardStr);
               printf("   Connection ID:    %u.\r\n", ConnectionID);
               printf("   Connection Type:  %s.\r\n", ((ConnectionType == gctLE) ? "LE" : "BR/EDR"));
               printf("   Remote Device:    %s.\r\n", BoardStr);

               /* Clear the Connection ID.                              */
               ConnectionID = 0;

               /* Since there is no disconnection event in GAP for      */
               /* BR/EDR we will handle any cleanup here when GATT      */
               /* disconnects for BR/EDR.                               */
               /* * NOTE * This code should be identical to the LE      */
               /*          Disconnection Event.                         */
               if(GATT_Connection_Event_Data->Event_Data.GATT_Device_Disconnection_Data->ConnectionType == gctBR_EDR)
               {
                  if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
                  {
                     /* Flag that no service discovery operation is     */
                     /* outstanding for this device.                    */
                     DeviceInfo->Flags &= ~DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_OUTSTANDING;

                     /* Flag that there is no BR_EDR connection for the */
                     /* device.                                         */
                     DeviceInfo->Flags &= ~DEVICE_INFO_FLAGS_BR_EDR_CONNECTED;

                     /* If this device is not paired , then delete it.  */
                     /* The link will be encrypted if the device is     */
                     /* paired.                                         */
                     if((!(DeviceInfo->Flags & DEVICE_INFO_FLAGS_LINK_ENCRYPTED)) || (!(DeviceInfo->Flags & DEVICE_INFO_FLAGS_LINK_KEY_VALID)))
                     {
                        if((DeviceInfo = DeleteDeviceInfoEntry(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
                           FreeDeviceInfoEntryMemory(DeviceInfo);
                     }
                     else
                     {
                        /* Flag that the Link is no longer encrypted    */
                        /* since we have disconnected.                  */
                        DeviceInfo->Flags &= ~DEVICE_INFO_FLAGS_LINK_ENCRYPTED;
                     }
                  }

                  /* Clear the saved Connection BD_ADDR.                */
                  /* * NOTE * If this is an LE GATT disconnection event */
                  /*          then this will be cleared by the GAP LE   */
                  /*          Disconnection event.  If this is cleared  */
                  /*          first we will receive the event and not be*/
                  /*          able to find the device info.             */
                  ASSIGN_BD_ADDR(ConnectionBD_ADDR, 0, 0, 0, 0, 0, 0);
                  LocalDeviceIsMaster = FALSE;
               }
            }
            else
               printf("\r\nError - Null Disconnection Data.\r\n");
            break;
         case etGATT_Connection_Device_Connection_MTU_Update:
            printf("\r\netGATT_Connection_Device_Connection_MTU_Update with size %u: \r\n", GATT_Connection_Event_Data->Event_Data_Size);
            if(GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_MTU_Update_Data)
            {
               /* Store the event data.                                 */
               RemoteBDADDR     = GATT_Connection_Event_Data->Event_Data.GATT_Device_Disconnection_Data->RemoteDevice;
               GATTConnectionID = GATT_Connection_Event_Data->Event_Data.GATT_Device_Disconnection_Data->ConnectionID;
               ConnectionType   = GATT_Connection_Event_Data->Event_Data.GATT_Device_Disconnection_Data->ConnectionType;
               MTU              = GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_MTU_Update_Data->MTU;

               BD_ADDRToStr(RemoteBDADDR, BoardStr);
               printf("   Connection ID:    %u.\r\n", GATTConnectionID);
               printf("   Connection Type:  %s.\r\n", (ConnectionType == gctLE)?"LE":"BR/EDR");
               printf("   BD_ADDR:          %s.\r\n", BoardStr);
               printf("   MTU:              %u.\r\n", MTU);
            }
            else
               printf("\r\nError - Null Connection Update Data.");
            break;
         default:
            break;
      }
   }
   else
   {
      /* There was an error with one or more of the input parameters.   */
      printf("\r\nGATT Connection Callback Data: Event_Data = NULL.\r\n");
   }

   DisplayPrompt();
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
   unsigned int                 Index;
   DeviceInfo_t                *DeviceInfo;
   GATT_Service_Information_t  *IncludedServiceInfo;

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
                  printf("   Number of Characteristics:     %u\r\n", GATT_Service_Discovery_Event_Data->Event_Data.GATT_Service_Discovery_Indication_Data->NumberOfCharacteristics);

                  IncludedServiceInfo = &(GATT_Service_Discovery_Event_Data->Event_Data.GATT_Service_Discovery_Indication_Data->IncludedServiceList[0]);

                  /* If any included services exist, display those too. */
                  for(Index = 0; Index < GATT_Service_Discovery_Event_Data->Event_Data.GATT_Service_Discovery_Indication_Data->NumberOfIncludedService; Index++, IncludedServiceInfo++)
                  {
                     printf("Included Service Handle Range:\t0x%04X - 0x%04X\r\n", IncludedServiceInfo->Service_Handle, IncludedServiceInfo->End_Group_Handle);
                     printf("Included Service UUID: 0x");
                     DisplayUUID(&(IncludedServiceInfo->UUID));
                  }

                  /* Check the service discovery type.                  */
                  switch((Service_Discovery_Type_t)CallbackParameter)
                  {
                     case sdDIS:
                        /* Attempt to populate the handles for the DIS  */
                        /* Service.                                     */
                        DISPopulateHandles(&(DeviceInfo->DISClientInfo), GATT_Service_Discovery_Event_Data->Event_Data.GATT_Service_Discovery_Indication_Data);
                        break;
                     case sdGAPS:
                        /* Attempt to populate the handles for the GAP  */
                        /* Service.                                     */
                        GAPSPopulateHandles(&(DeviceInfo->GAPSClientInfo), GATT_Service_Discovery_Event_Data->Event_Data.GATT_Service_Discovery_Indication_Data);
                        break;
                     case sdIPS:
                        /* Attempt to populate the handles for the IPS  */
                        /* Service.                                     */
                        IPSPopulateHandles(DeviceInfo, GATT_Service_Discovery_Event_Data->Event_Data.GATT_Service_Discovery_Indication_Data);
                        break;
                     default:
                        printf("\r\nWarning - Unknown service discovery type.\r\n");
                        break;
                  }
               }
               break;
            case etGATT_Service_Discovery_Complete:
               /* Verify the event data.                                */
               if(GATT_Service_Discovery_Event_Data->Event_Data.GATT_Service_Discovery_Complete_Data)
               {
                  printf("\r\n");
                  printf("Service Discovery Operation Complete, Status 0x%02X.\r\n", GATT_Service_Discovery_Event_Data->Event_Data.GATT_Service_Discovery_Complete_Data->Status);

                  /* Flag that no service discovery operation is        */
                  /* outstanding for this device.                       */
                  DeviceInfo->Flags &= ~DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_OUTSTANDING;

                  /* Check the service discovery type.                  */
                  switch((Service_Discovery_Type_t)CallbackParameter)
                  {
                     case sdDIS:
                        /* Flag that service discovery has been         */
                        /* performed on for this connection.            */
                        DeviceInfo->Flags |= DEVICE_INFO_FLAGS_DIS_SERVICE_DISCOVERY_COMPLETE;

                        printf("\r\nDIS Service Discovery Summary\r\n\r\n");
                        printf("   Manufacturer Name:  %s\r\n", (DeviceInfo->DISClientInfo.ManufacturerNameHandle ? "Supported" : "Not Supported"));
                        printf("   Model Number:       %s\r\n", (DeviceInfo->DISClientInfo.ModelNumberHandle      ? "Supported" : "Not Supported"));
                        printf("   Serial Number:      %s\r\n", (DeviceInfo->DISClientInfo.SerialNumberHandle     ? "Supported" : "Not Supported"));
                        printf("   Hardware Revision:  %s\r\n", (DeviceInfo->DISClientInfo.HardwareRevisionHandle ? "Supported" : "Not Supported"));
                        printf("   Firmware Revision:  %s\r\n", (DeviceInfo->DISClientInfo.FirmwareRevisionHandle ? "Supported" : "Not Supported"));
                        printf("   Software Revision:  %s\r\n", (DeviceInfo->DISClientInfo.SoftwareRevisionHandle ? "Supported" : "Not Supported"));
                        printf("   SystemID:           %s\r\n", (DeviceInfo->DISClientInfo.SystemIDHandle         ? "Supported" : "Not Supported"));
                        printf("   IEEE 11073 Cert:    %s\r\n", (DeviceInfo->DISClientInfo.IEEE11073CertHandle    ? "Supported" : "Not Supported"));
                        printf("   PnP ID:             %s\r\n", (DeviceInfo->DISClientInfo.PnPIDHandle            ? "Supported" : "Not Supported"));
                        break;
                     case sdGAPS:
                        /* Flag that service discovery has been         */
                        /* performed on for this connection.            */
                        DeviceInfo->Flags |= DEVICE_INFO_FLAGS_GAPS_SERVICE_DISCOVERY_COMPLETE;

                        printf("\r\nGAPS Service Discovery Summary\r\n\r\n");
                        printf("   Device Name:       %s\r\n", (DeviceInfo->GAPSClientInfo.DeviceNameHandle       ? "Supported" : "Not Supported"));
                        printf("   Device Appearance: %s\r\n", (DeviceInfo->GAPSClientInfo.DeviceAppearanceHandle ? "Supported" : "Not Supported"));
                        break;
                     case sdIPS:
                        /* Flag that service discovery has been         */
                        /* performed on for this connection.            */
                        DeviceInfo->Flags |= DEVICE_INFO_FLAGS_IPS_SERVICE_DISCOVERY_COMPLETE;

                        /* Print a summary of what was discovered.      */
                        printf("\r\nIPS Service Discovery Summary\r\n\r\n");

                        printf("\r\nIndoor Positioning Configuration:  %s\r\n", (DeviceInfo->IPSClientInfo.IPS_Indoor_Positioning_Configuration                     ? "Supported" : "Not Supported"));
                        printf("   Descriptors:\r\n");
                        printf("      Extended Properties:         %s\r\n",     (DeviceInfo->IPSClientInfo.IPS_Indoor_Positioning_Configuration_Extended_Properties ? "Supported" : "Not Supported"));

                        printf("\r\nLatitude:                          %s\r\n", (DeviceInfo->IPSClientInfo.IPS_Latitude                                             ? "Supported" : "Not Supported"));
                        printf("   Descriptors:\r\n");
                        printf("      Extended Properties:         %s\r\n",     (DeviceInfo->IPSClientInfo.IPS_Latitude_Extended_Properties                         ? "Supported" : "Not Supported"));

                        printf("\r\nLongitude:                         %s\r\n", (DeviceInfo->IPSClientInfo.IPS_Longitude                                            ? "Supported" : "Not Supported"));
                        printf("   Descriptors:\r\n");
                        printf("      Extended Properties:         %s\r\n",     (DeviceInfo->IPSClientInfo.IPS_Longitude_Extended_Properties                        ? "Supported" : "Not Supported"));

                        printf("\r\nLocal North Coordinate:            %s\r\n", (DeviceInfo->IPSClientInfo.IPS_Local_North_Coordinate                               ? "Supported" : "Not Supported"));
                        printf("   Descriptors:\r\n");
                        printf("      Extended Properties:         %s\r\n",     (DeviceInfo->IPSClientInfo.IPS_Local_North_Coordinate_Extended_Properties           ? "Supported" : "Not Supported"));

                        printf("\r\nLocal East Coordinate:             %s\r\n", (DeviceInfo->IPSClientInfo.IPS_Local_East_Coordinate                                ? "Supported" : "Not Supported"));
                        printf("   Descriptors:\r\n");
                        printf("      Extended Properties:         %s\r\n",     (DeviceInfo->IPSClientInfo.IPS_Local_East_Coordinate_Extended_Properties            ? "Supported" : "Not Supported"));

                        printf("\r\nFloor Number:                      %s\r\n", (DeviceInfo->IPSClientInfo.IPS_Floor_Number                                         ? "Supported" : "Not Supported"));
                        printf("   Descriptors:\r\n");
                        printf("      Extended Properties:         %s\r\n",     (DeviceInfo->IPSClientInfo.IPS_Floor_Number_Extended_Properties                     ? "Supported" : "Not Supported"));

                        printf("\r\nAltitude:                          %s\r\n", (DeviceInfo->IPSClientInfo.IPS_Altitude                                             ? "Supported" : "Not Supported"));
                        printf("   Descriptors:\r\n");
                        printf("      Extended Properties:         %s\r\n",     (DeviceInfo->IPSClientInfo.IPS_Altitude_Extended_Properties                         ? "Supported" : "Not Supported"));

                        printf("\r\nUncertainty:                       %s\r\n", (DeviceInfo->IPSClientInfo.IPS_Uncertainty                                          ? "Supported" : "Not Supported"));
                        printf("   Descriptors:\r\n");
                        printf("      Extended Properties:         %s\r\n",     (DeviceInfo->IPSClientInfo.IPS_Uncertainty_Extended_Properties                      ? "Supported" : "Not Supported"));

                        printf("\r\nLocation Name:                     %s\r\n", (DeviceInfo->IPSClientInfo.IPS_Location_Name                                        ? "Supported" : "Not Supported"));
                        printf("   Descriptors:\r\n");
                        printf("      Extended Properties:         %s\r\n",     (DeviceInfo->IPSClientInfo.IPS_Location_Name_Extended_Properties                    ? "Supported" : "Not Supported"));
                        break;
                     default:
                        printf("\r\nWarning - Unknown service discovery type.\r\n");
                        break;
                  }
               }
               break;
         }
      }
   }
   else
      printf("\r\nGATT Callback Data: Event_Data = NULL.\r\n");

   DisplayPrompt();
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
