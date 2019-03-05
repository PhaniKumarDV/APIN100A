/*
* Copyright (c) 2016 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
/*****< linuxplxp.c >**********************************************************/
/*      Copyright 2016 Qualcomm Technologies, Inc.                            */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  LINUXPLXP - Linux Bluetooth Pulse Oximeter Profile using GATT (LE/BREDR)  */
/*             sample application.                                            */
/*                                                                            */
/*  Author:  Ryan McCord                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   03/18/16  R. McCord      Initial creation.                               */
/******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "LinuxPLXP.h"           /* Application Header.                       */

#include "SS1BTPS.h"             /* Main SS1 BT Stack Header.                 */
#include "SS1BTDBG.h"            /* BTPS Debug Header.                        */
#include "SS1BTGAT.h"            /* Main SS1 GATT Header.                     */
#include "SS1BTDIS.h"            /* Main SS1 DIS Service Header.              */
#include "SS1BTGAP.h"            /* Main SS1 GAP Service Header.              */
#include "SS1BTPLXS.h"           /* Main SS1 PLXS Service Header.             */

#include "time.h"                /* Included for time.                        */

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

#define PLXSP_PARAMETER_VALUE                        (2)  /* Denotes the value */
                                                         /* passed in on the  */
                                                         /* command line for  */
                                                         /* running with the  */
                                                         /* transport set to  */
                                                         /* PLXSP.             */

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

#define MAX_NUM_OF_PARAMETERS                       (6)  /* Denotes the max   */
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

#define PRINT_SERVICE_DISCOVERY_INFORMATION     (FALSE)  /* Enables printing  */
                                                         /* of information    */
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
                                                         /* create a PLXS     */
                                                         /* Server.           */

#define EXIT_MODE                                  (-10) /* Flags exit from   */
                                                         /* any Mode.         */

#define UNABLE_TO_ALLOCATE_MEMORY                  (-11) /* Denotes that we   */
                                                         /* failed because we */
                                                         /* couldn't allocate */
                                                         /* memory.           */

#define UUID_16_HEX_STRING_LENGTH                   (6)  /* Denotes the number*/
                                                         /* of characters     */
                                                         /* required to       */
                                                         /* represent a 16 bit*/
                                                         /* UUID when it is   */
                                                         /* type with a       */
                                                         /* leading 0x (in    */
                                                         /* ASCII form).      */

#define UUID_32_HEX_STRING_LENGTH                  (12)  /* Denotes the number*/
                                                         /* of characters     */
                                                         /* required to       */
                                                         /* represent a 32 bit*/
                                                         /* UUID when it is   */
                                                         /* type with a       */
                                                         /* leading 0x (in    */
                                                         /* ASCII form).      */

#define UUID_128_HEX_STRING_LENGTH                 (34)  /* Denotes the number*/
                                                         /* of characters     */
                                                         /* required to       */
                                                         /* represent a 128   */
                                                         /* bit UUID when it  */
                                                         /* is type with a    */
                                                         /* leading 0x (in    */
                                                         /* ASCII form).      */


   /* The following defines the maximum number of PLXS Continuous       */
   /* Measurements that may be stored by the PLXS Server.               */
#define PLXP_MAXIMUM_NUMBER_OF_SPOT_CHECK_MEASUREMENTS  (5)

   /* The following defines the default PLXS Features supported by the  */
   /* PLXS Server.                                                      */
#define PLXP_DEFAULT_PLXS_FEATURES                 (PLXS_FEATURES_MEASUREMENT_STATUS_SUPPORTED             | \
                                                    PLXS_FEATURES_DEVICE_AND_SENSOR_STATUS_SUPPORTED       | \
                                                    PLXS_FEATURES_SPOT_CHECK_MEASUREMENT_STORAGE_SUPPORTED | \
                                                    PLXS_FEATURES_TIME_STAMP_SUPPORTED                     | \
                                                    PLXS_FEATURES_SPO2PR_FAST_METRIC_SUPPORTED             | \
                                                    PLXS_FEATURES_SPO2PR_SLOW_METRIC_SUPPORTED             | \
                                                    PLXS_FEATURES_PULSE_AMPLITUDE_INDEX_SUPPORTED)

   /* The following defines the Measurement Status Supprt for the PLXS  */
   /* Server.                                                           */
#define PLXP_DEFAULT_MEASUREMENT_STATUS_SUPPORT    (PLXS_MSS_MEASUREMENT_ONGOING_SUPPORTED                | \
                                                    PLXS_MSS_EARLY_ESTIMATE_DATA_SUPPORTED                | \
                                                    PLXS_MSS_VALIDATED_DATA_SUPPORTED                     | \
                                                    PLXS_MSS_FULLY_QUALIFIED_DATA_SUPPORTED               | \
                                                    PLXS_MSS_MEASUREMENT_STORAGE_SUPPORTED                | \
                                                    PLXS_MSS_DATA_FOR_DEMONSTRATION_SUPPORTED             | \
                                                    PLXS_MSS_DATA_FOR_TESTING_SUPPORTED                   | \
                                                    PLXS_MSS_CALIBRATION_ONGOING_SUPPORTED                | \
                                                    PLXS_MSS_MEASUREMENT_UNAVAILABLE_SUPPORTED            | \
                                                    PLXS_MSS_QUESTIONABLE_MEASUREMENT_DETECTED_SUPPORTED  | \
                                                    PLXS_MSS_INVALID_MEASUREMENT_DETECTED_SUPPORTED)

   /* The following defines the Device and Sensort Status Supprt for the*/
   /* PLXS Server.                                                      */
#define PLXP_DEFAULT_DEVICE_AND_SENSOR_STATUS_SUPPORT  (PLXS_DSSS_EXTENDED_DISPLAY_UPDATE_ONGOING_SUPPORTED         | \
                                                        PLXS_DSSS_EQUIPMENT_MALFUNCTION_DETECTED_SUPPORTED          | \
                                                        PLXS_DSSS_SIGNAL_PROCESSING_IRREGULARITY_DETECTED_SUPPORTED | \
                                                        PLXS_DSSS_INADEQUITE_SIGNAL_DETECTED_SUPPORTED              | \
                                                        PLXS_DSSS_POOR_SIGNAL_DETECTED_SUPPORTED                    | \
                                                        PLXS_DSSS_LOW_PERFUSION_DETECTED_SUPPORTED                  | \
                                                        PLXS_DSSS_ERRATIC_SIGNAL_DETECTED_SUPPORTED                 | \
                                                        PLXS_DSSS_NON_PULSATILE_SIGNAL_DETECTED_SUPPORTED           | \
                                                        PLXS_DSSS_QUESTIONABLE_PULSE_DETECTED_SUPPORTED             | \
                                                        PLXS_DSSS_SIGNAL_ANALYSIS_ONGOING_SUPPORTED                 | \
                                                        PLXS_DSSS_SENSOR_INTERFACE_DETECTED_SUPPORTED               | \
                                                        PLXS_DSSS_SENSOR_UNCONNECTED_TO_USER_SUPPORTED              | \
                                                        PLXS_DSSS_UNKNOWN_SENSOR_CONNECTED_SUPPORTED                | \
                                                        PLXS_DSSS_SENSOR_DISPLACEMENT_SUPPORTED                     | \
                                                        PLXS_DSSS_SENSOR_MALFUNCTIONING_SUPPORTED                   | \
                                                        PLXS_DSSS_SENSOR_DISCONNECTED_SUPPORTED)

   /* The following constants represent the default log file names that */
   /* are used if no Log file name is specified when enabling debug.    */
#define DEFAULT_DEBUG_LOG_FILE_NAME                "LinuxPLXP_ASC.log"
#define DEFAULT_DEBUG_FTS_FILE_NAME                "LinuxPLXP_FTS.log"

   /* Determine the Name we will use for this compilation.              */
#define LE_DEMO_DEVICE_NAME                        "LinuxPLXP"

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
   sdPLXS,
   sdAll
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

   /* The following defines the bitmask values that may be set for the  */
   /* Flags field in the DeviceInfo_t structure.                        */
#define DEVICE_INFO_FLAGS_LTK_VALID                         0x0001
#define DEVICE_INFO_FLAGS_LINK_KEY_VALID                    0x0002
#define DEVICE_INFO_FLAGS_LINK_ENCRYPTED                    0x0004
#define DEVICE_INFO_FLAGS_BR_EDR_CONNECTED                  0x0008
#define DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_OUTSTANDING     0x0010
#define DEVICE_INFO_FLAGS_GATT_SERVICE_DISCOVERY_COMPLETE   0x0020
#define DEVICE_INFO_FLAGS_DIS_SERVICE_DISCOVERY_COMPLETE    0x0040
#define DEVICE_INFO_FLAGS_GAPS_SERVICE_DISCOVERY_COMPLETE   0x0080
#define DEVICE_INFO_FLAGS_PLXS_SERVICE_DISCOVERY_COMPLETE   0x0100
#define DEVICE_INFO_FLAGS_RACP_PROCEDURE_IN_PROGRESS        0x0200

   /* The following structure contains all of the GATT Client           */
   /* information that will need by a GATT Client connected to a GATT   */
   /* Server.                                                           */
   /* * NOTE * This information includes all the attribute handles for  */
   /*          GATT Characteristics found during service discovery.     */
typedef struct _tagGATT_Client_Info_t
{
   Word_t  ServiceChanged;
   Word_t  ServiceChangedCCCD;
} GATT_Client_Info_t;

   /* The following enumeration defines the possible DIS Characteristics*/
   /* that may be read on a remote DIS Server.                          */
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

   /* The following structure contains all of the DIS Client information*/
   /* that will need by a DIS Client connected to a DIS Server.         */
   /* * NOTE * This information includes all the attribute handles for  */
   /*          DIS Characteristics found during service discovery.      */
   /* * NOTE * The RequestType and Buffer fields are used by a DIS      */
   /*          Client for reading DIS Characteristics.                  */
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

   /* The following structure contains all of the GAPS Client           */
   /* information that will need by a GAPS Client connected to a GAPS   */
   /* Server.                                                           */
   /* * NOTE * This information includes all the attribute handles for  */
   /*          GAPS Characteristics found during service discovery.     */
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
   /* information on all paired devices.  For slave we will not use this*/
   /* structure.                                                        */
typedef struct _tagDeviceInfo_t
{
   Word_t                      Flags;
   Byte_t                      EncryptionKeySize;
   GAP_LE_Address_Type_t       ConnectionAddressType;
   BD_ADDR_t                   ConnectionBD_ADDR;
   Link_Key_t                  LinkKey;
   Long_Term_Key_t             LTK;
   Random_Number_t             Rand;
   Word_t                      EDIV;
   GATT_Client_Info_t          GATTClientInfo;
   DIS_Client_Info_t           DISClientInfo;
   GAPS_Client_Info_t          GAPSClientInfo;
   PLXS_Client_Information_t   PLXSClientInfo;
   PLXS_Server_Information_t   PLXSServerInfo;
   struct _tagDeviceInfo_t    *NextDeviceInfoPtr;
} DeviceInfo_t;

#define DEVICE_INFO_DATA_SIZE  (sizeof(DeviceInfo_t))

   /* The following enumeration defines the attribute handle types.     */
   /* This has been included to make sending requests and receiving     */
   /* responses easier to read.                                         */
typedef enum
{
   ahtPLX_Features,
   ahtSpot_Check_Measurement,
   ahtSpot_Check_Measurement_CCCD,
   ahtContinuous_Measurement,
   ahtContinuous_Measurement_CCCD,
   ahtRecordAccessControlPoint,
   ahtRecordAccessControlPoint_CCCD,
   ahtServiceChanged_CCCD
} PLXP_Attribute_Handle_Type_t;

   /* The following structure holds the data that is needed by the PLXP */
   /* Client for each GATT request/response.                            */
typedef struct _tagPLXP_Request_Data_t
{
   PLXP_Attribute_Handle_Type_t AttributeHandleType;
   Word_t                       AttributeHandle;
   union
   {
      Word_t                    Configuration;
      PLXS_RACP_Request_Data_t  RACPRequestData;
   } Data;
} PLXP_Request_Data_t;

   /* The following structure defines a Spot Check Measurement that may */
   /* be included in a list of Spot Check Measurements.                 */
typedef struct _tagPLXP_Spot_Check_Measurement_Data_t
{
   Boolean_t                          Valid;
   unsigned int                       ID;
   Boolean_t                          Indicated;
   PLXS_Spot_Check_Measurement_Data_t Measurement;
} PLXP_Spot_Check_Measurement_Data_t;

#define PLXP_SPOT_CHECK_MEASUREMENT_DATA_SIZE  (sizeof(PLXP_Spot_Check_Measurement_Data_t))

   /* The following structure holds the data that needs to be stored for*/
   /* Continuous Measurement notifications so that the timer callback   */
   /* can send more measurments.                                        */
   /* * NOTE * The Timeout field is in milli-seconds.                   */
typedef struct _tagPLXP_Notification_Data_t
{
   Boolean_t         NotificationsStarted;
   unsigned int      TimerID;
   unsigned long     Timeout;
   Byte_t            Flags;
   Word_t            Measurement_Status;
   PLXS_INT24_Data_t Device_And_Sensor_Status;
} PLXP_Notification_Data_t;

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
static unsigned int        PLXSInstanceID;
                                                    /* The following holds the PLXS    */
                                                    /* Instance IDs that are returned  */
                                                    /* from PLXS_Initialize_XXX().     */

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

   /* PLXP Server globals.                                              */

static PLXP_Spot_Check_Measurement_Data_t  SpotCheckMeasurementList[PLXP_MAXIMUM_NUMBER_OF_SPOT_CHECK_MEASUREMENTS];
                                                    /* The Spot Check Measurement List.*/

static unsigned int         MeasurementIDCtr;       /* Designates the measurement      */
                                                    /* counter.                        */

static PLXP_Notification_Data_t NotificationData;   /* The data that needs to be stored*/
                                                    /* during continuous measurement   */
                                                    /* notifications.                  */

   /* PLXP Client globals.                                              */

static PLXP_Request_Data_t   PLXSRequestData;       /* The PLXP Client request data.   */

static PLXS_Features_Data_t  PLX_Feature;           /* The supported PLXS Features of  */
                                                    /* The PLXP Server.                */

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

static void DisplayKnownServiceName(GATT_UUID_t *UUID);

   /* Generic Attribute Profile Functions and Commands.                 */
static void GATTPopulateHandles(GATT_Client_Info_t *ClientInfo, GATT_Service_Discovery_Indication_Data_t *ServiceInfo);
static void DisplayGATTDiscoverySummary(GATT_Client_Info_t *ClientInfo);
static int DiscoverAllServicesCommand(ParameterList_t *TempParam);

   /* Device Information Service Functions and Commands.                */
static int DiscoverDIS(ParameterList_t *TempParam);
static void DISPopulateHandles(DIS_Client_Info_t *ClientInfo, GATT_Service_Discovery_Indication_Data_t *ServiceInfo);
static void DisplayDISDiscoverySummary(DIS_Client_Info_t *ClientInfo);
static int ReadDISCharacteristic(ParameterList_t *TempParam);

   /* Generic Access Profile Service Functions and Commands.            */
static int DiscoverGAPS(ParameterList_t *TempParam);
static void GAPSPopulateHandles(GAPS_Client_Info_t *ClientInfo, GATT_Service_Discovery_Indication_Data_t *ServiceInfo);
static void DisplayGAPSDiscoverySummary(GAPS_Client_Info_t *ClientInfo);
static int ReadLocalName(ParameterList_t *TempParam);
static int SetLocalName(ParameterList_t *TempParam);
static int ReadRemoteName(ParameterList_t *TempParam);
static int ReadLocalAppearance(ParameterList_t *TempParam);
static int SetLocalAppearance(ParameterList_t *TempParam);
static int ReadRemoteAppearance(ParameterList_t *TempParam);

static void DumpAppearanceMappings(void);
static Boolean_t AppearanceToString(Word_t Appearance, char **String);
static Boolean_t AppearanceIndexToAppearance(unsigned int Index, Word_t *Appearance);

   /* Object Transfer Service Functions and Commands.                   */
static void PLXSPopulateHandles(PLXS_Client_Information_t *ClientInfo, GATT_Service_Discovery_Indication_Data_t *ServiceDiscoveryData);
static void StoreDescriptorHandles(PLXS_Client_Information_t *ClientInfo, GATT_Characteristic_Information_t *CharacteristicInfoPtr, Word_t *Handle);
static void DisplayPLXSDiscoverySummary(PLXS_Client_Information_t *ClientInfo);

static int RegisterPLXS(void);
static int UnregisterPLXS(void);

static int ReadPLXSCharacteristic(void);
static int WritePLXSCharacteristic(void);

static void CreateSpotCheckMeasurement(Byte_t Flags, Word_t Measurement_Status, unsigned int Device_And_Sensor_Status);
static void IndicateSpotCheckMeasurement(void);
static void DisplaySpotCheckMeasurementsList(void);

static void StartContinuousMeasurementNotifications(Byte_t Flags, Word_t Measurement_Status, unsigned int Device_And_Sensor_Status, unsigned long Timeout);
static void StopContinuousMeasurementNotifications(void);

static void ReportStoredRecordsProcedure(PLXS_RACP_Request_Data_t *RequestData, PLXS_RACP_Response_Data_t *ResponseData);
static void DeleteStoredRecordsProcedure(PLXS_RACP_Request_Data_t *RequestData, PLXS_RACP_Response_Data_t *ResponseData);
static void NumberOfStoredRecordsProcedure(PLXS_RACP_Request_Data_t *RequestData, PLXS_RACP_Response_Data_t *ResponseData);

static void DisplayCCCDType(PLXS_CCCD_Type_t Type);
static void DisplayRACPRequestData(PLXS_RACP_Request_Data_t *RequestData);
static void DisplayRACPResponseData(PLXS_RACP_Response_Data_t *ResponseData);

static void DisplayPLXFeatures(PLXS_Features_Data_t *Features);
static void DisplaySpotCheckMeasurement(PLXS_Spot_Check_Measurement_Data_t *Measurement);
static void DisplayContinuousMeasurement(PLXS_Continuous_Measurement_Data_t *Measurement);

static int RegisterPLXSCommand(ParameterList_t *TempParam);
static int UnregisterPLXSCommand(ParameterList_t *TempParam);
static int DiscoverPLXSCommand(ParameterList_t *TempParam);

static int StartNotificationsCommand(ParameterList_t *TempParam);
static int StopNotificationsCommand(ParameterList_t *TempParam);

static int CreateSpotCheckMeasurementCommand(ParameterList_t *TempParam);
static int IndicateSpotCheckMeasurementCommand(ParameterList_t *TempParam);
static int PrintSpotCheckMeasurementListCommand(ParameterList_t *TempParam);

static int ReadCharacteristicCommand(ParameterList_t *TempParam);
static int WriteCCCDCommand(ParameterList_t *TempParam);
static int WriteRACPCommand(ParameterList_t *TempParam);

   /* BTPS Callback function prototypes.                                */
static void BTPSAPI GAP_Event_Callback(unsigned int BluetoothStackID, GAP_Event_Data_t *GAP_Event_Data, unsigned long CallbackParameter);
static void BTPSAPI GAP_LE_Event_Callback(unsigned int BluetoothStackID, GAP_LE_Event_Data_t *GAP_LE_Event_Data, unsigned long CallbackParameter);
static void BTPSAPI PLXS_EventCallback(unsigned int BluetoothStackID, PLXS_Event_Data_t *PLXS_Event_Data, unsigned long CallbackParameter);
static void BTPSAPI GATT_ClientEventCallback_DIS(unsigned int BluetoothStackID, GATT_Client_Event_Data_t *GATT_Client_Event_Data, unsigned long CallbackParameter);
static void BTPSAPI GATT_ClientEventCallback_GAPS(unsigned int BluetoothStackID, GATT_Client_Event_Data_t *GATT_Client_Event_Data, unsigned long CallbackParameter);
static void BTPSAPI GATT_ClientEventCallback_PLXS(unsigned int BluetoothStackID, GATT_Client_Event_Data_t *GATT_Client_Event_Data, unsigned long CallbackParameter);
static void BTPSAPI GATT_Connection_Event_Callback(unsigned int BluetoothStackID, GATT_Connection_Event_Data_t *GATT_Connection_Event_Data, unsigned long CallbackParameter);
static void BTPSAPI GATT_Service_Discovery_Event_Callback(unsigned int BluetoothStackID, GATT_Service_Discovery_Event_Data_t *GATT_Service_Discovery_Event_Data, unsigned long CallbackParameter);
static void BTPSAPI PLXP_Continuous_Measurement_Timer_Callback(unsigned int BluetoothStackID, unsigned int TimerID, unsigned long CallbackParameter);

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
   /* element of the specified Key Info List.  Upon return of this      */
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
   AddCommand("GETMTU", GetGATTMTU);
   AddCommand("SETMTU", SetGATTMTU);
   AddCommand("REGISTERPLXS", RegisterPLXSCommand);
   AddCommand("UNREGISTERPLXS", UnregisterPLXSCommand);
   AddCommand("DISCOVERPLXS", DiscoverPLXSCommand);
   AddCommand("START", StartNotificationsCommand);
   AddCommand("STOP", StopNotificationsCommand);
   AddCommand("CREATE", CreateSpotCheckMeasurementCommand);
   AddCommand("INDICATE", IndicateSpotCheckMeasurementCommand);
   AddCommand("PRINT", PrintSpotCheckMeasurementListCommand);
   AddCommand("READ", ReadCharacteristicCommand);
   AddCommand("WRITECCCD", WriteCCCDCommand);
   AddCommand("WRITERACP", WriteRACPCommand);
   AddCommand("DISCOVERALL", DiscoverAllServicesCommand);
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
         /* Make sure the command has the same length or it is not the  */
         /* same command.                                               */
         if(BTPS_StringLength(Command) == BTPS_StringLength(CommandTable[Index].CommandName))
         {
            /* Compare the commands.                                    */
            /* * NOTE * The length of the command is the same so we will*/
            /*          simply use the length of the one in the table to*/
            /*          compare.                                        */
            if(BTPS_MemCompare(Command, CommandTable[Index].CommandName, BTPS_StringLength(CommandTable[Index].CommandName)) == 0)
               ret_val = CommandTable[Index].CommandFunction;
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
   unsigned int                     Index;
   GAP_LE_Advertising_Data_Entry_t *Advertising_Data_Entry;
   Byte_t                           Flags;
   GATT_UUID_t                      UUID;

   /* Verify that the input parameters seem semi-valid.                 */
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
               DisplayRawAdvertisingData(&(Advertising_Data->Data_Entries[Index]));

               /* We will check for the PLXS Service data.  Make sure   */
               /* the mandatory UUID is present.                        */
               if(Advertising_Data_Entry->AD_Data_Length >= NON_ALIGNED_WORD_SIZE)
               {
                  /* Store the type.                                    */
                  UUID.UUID_Type = guUUID_16;

                  /* Check if this is the PLXS Service data.            */
                  if(PLXS_COMPARE_PLXS_SERVICE_UUID_TO_UUID_16(*((UUID_16_t *)&(Advertising_Data_Entry->AD_Data_Buffer[0]))))
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
      if(Entry->AD_Data_Buffer)
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

   printf("\r\n");
}

   /* Displays the correct prompt depending on the Server/Client Mode.  */
static void DisplayPrompt(void)
{
   printf("\r\nLinuxPLXP>");

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

      /* Cleanup PLXS Service.                                          */
      if(PLXSInstanceID)
      {
         /* Un-registerPLXS.                                            */
         UnregisterPLXS();
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

      printf("\r\nStack Shutdown.\r\n");

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
   printf("* PLXP Sample Application                                        *\r\n");
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
   printf("* Command Options GATT:    DiscoverALL,                          *\r\n");
   printf("* Command Options DIS:     DiscoverDIS, ReadDIS,                 *\r\n");
   printf("* Command Options GAPS:    DiscoverGAPS, GetLocalName,           *\r\n");
   printf("*                          SetLocalName, GetRemoteName,          *\r\n");
   printf("*                          SetLocalAppearance,                   *\r\n");
   printf("*                          GetLocalAppearance,                   *\r\n");
   printf("*                          GetRemoteAppearance,                  *\r\n");
   printf("* Command Options PLXS:                                          *\r\n");
   printf("*                Server:   RegisterPLXS                          *\r\n");
   printf("*                          UnRegisterPLXS,                       *\r\n");
   printf("*          (Cont M.)       Start, Stop,                          *\r\n");
   printf("*          (Spot Check M.) Create, Indicate, Print,              *\r\n");
   printf("*                Client:   DiscoverPLXS,                         *\r\n");
   printf("*                          Read,                                 *\r\n");
   printf("*                          WriteCCCD,                            *\r\n");
   printf("*                          WriteRACP,                            *\r\n");
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
      if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].intParam >= 0) && (TempParam->Params[0].intParam <= 2))
      {
         /* Parameters appear to be valid, map the specified parameters */
         /* into the API specific parameters.                           */
         if(TempParam->Params[0].intParam == 0)
         {
            PairabilityMode = lpmNonPairableMode;
            Mode            = "lpmNonPairableMode";
         }
         else
         {
            if(TempParam->Params[0].intParam == 1)
            {
               PairabilityMode = lpmPairableMode;
               Mode            = "lpmPairableMode";
            }
            else
            {
               PairabilityMode = lpmPairableMode_EnableExtendedEvents;
               Mode            = "lpmPairableMode_EnableExtendedEvents";
            }
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
         DisplayUsage("SetPairabilityMode [Mode (0 = Non Pairable, 1 = Pairable, 2 = Pairable w/ Secure Connections]");

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
   GAP_LE_Advertising_Parameters_t     AdvertisingParameters;
   GAP_LE_Connectability_Parameters_t  ConnectabilityParameters;
   Word_t                              Appearance;
   union
   {
      Advertising_Data_t               AdvertisingData;
      Scan_Response_Data_t             ScanResponseData;
   } Advertisement_Data_Buffer;

   /* First, check that valid Bluetooth Stack ID exists. And that we are*/
   /* not already connected.                                            */
   if((BluetoothStackID) && (!ConnectionID))
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

            /* Configure the flags field plxsed on the Discoverability  */
            /* Mode.                                                    */
            if(LE_Parameters.DiscoverabilityMode == dmGeneralDiscoverableMode)
               Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[2] = HCI_LE_ADVERTISING_FLAGS_GENERAL_DISCOVERABLE_MODE_FLAGS_BIT_MASK;
            else
            {
               if(LE_Parameters.DiscoverabilityMode == dmLimitedDiscoverableMode)
                  Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[2] = HCI_LE_ADVERTISING_FLAGS_LIMITED_DISCOVERABLE_MODE_FLAGS_BIT_MASK;
            }

            if(PLXSInstanceID)
            {
               /* Advertise the service(1 byte type and 2 bytes UUID)   */
               Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[3] = 3;
               Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[4] = HCI_LE_ADVERTISING_REPORT_DATA_TYPE_16_BIT_SERVICE_UUID_COMPLETE;
               PLXS_ASSIGN_PLXS_SERVICE_UUID_16(&(Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[5]));

               /* Advertise the appearance.                             */
               Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[7] = 3;
               Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[8] = HCI_LE_ADVERTISING_REPORT_DATA_TYPE_APPEARANCE;
               Appearance                                                    = GAP_DEVICE_APPEARENCE_VALUE_GENERIC_COMPUTER;
               ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&(Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[9]), Appearance);
            }

            /* Write the advertising data to the chip.                  */
            ret_val = GAP_LE_Set_Advertising_Data(BluetoothStackID, (Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[0] + Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[3] + Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[7] + 3), &(Advertisement_Data_Buffer.AdvertisingData));
            if(!ret_val)
            {
               BTPS_MemInitialize(&(Advertisement_Data_Buffer.ScanResponseData), 0, sizeof(Scan_Response_Data_t));

               /* Set the Scan Response Data.                           */
               if(BTPS_StringLength(LE_DEMO_DEVICE_NAME) < (ADVERTISING_DATA_MAXIMUM_SIZE - 2))
               {
                  Advertisement_Data_Buffer.ScanResponseData.Scan_Response_Data[0] = (Byte_t)(1 + BTPS_StringLength(LE_DEMO_DEVICE_NAME));
                  Advertisement_Data_Buffer.ScanResponseData.Scan_Response_Data[1] = HCI_LE_ADVERTISING_REPORT_DATA_TYPE_LOCAL_NAME_COMPLETE;
               }
               else
               {
                  Advertisement_Data_Buffer.ScanResponseData.Scan_Response_Data[0] = (Byte_t)(ADVERTISING_DATA_MAXIMUM_SIZE - 2);
                  Advertisement_Data_Buffer.ScanResponseData.Scan_Response_Data[1] = HCI_LE_ADVERTISING_REPORT_DATA_TYPE_LOCAL_NAME_SHORTENED;
               }

               strncpy((char *)&(Advertisement_Data_Buffer.ScanResponseData.Scan_Response_Data[2]), LE_DEMO_DEVICE_NAME, (ADVERTISING_DATA_MAXIMUM_SIZE - 2));

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
                  /* * NOTE * Since we do not ever put ourselves to be  */
                  /*          direct connectable then we will set the   */
                  /*          DirectAddress to all 0s.                  */
                  ConnectabilityParameters.Connectability_Mode   = LE_Parameters.ConnectableMode;
                  ConnectabilityParameters.Own_Address_Type      = latPublic;
                  ConnectabilityParameters.Direct_Address_Type   = latPublic;
                  ASSIGN_BD_ADDR(ConnectabilityParameters.Direct_Address, 0, 0, 0, 0, 0, 0);

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
         DisplayUsage("AdvertiseLE [(0 = Disable, 1 = Enable)]");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      if(BluetoothStackID)
         printf("Connection already active.\r\n");

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

                     if((LogFileName) && (TempParam->Params[1].intParam != dtDebugTerminal))
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

   /* The following function is a helper function to display the service*/
   /* name for service discovery.                                       */
static void DisplayKnownServiceName(GATT_UUID_t *UUID)
{
   /* Print the service name if it is GATT.                             */
   if(GATT_COMPARE_GATT_SERVICE_UUID_TO_BLUETOOTH_UUID_16(UUID->UUID.UUID_16))
   {
      printf("   Name:             GATT\r\n");
   }

   /* Print the service name if it is DIS.                              */
   if(DIS_COMPARE_DIS_SERVICE_UUID_TO_UUID_16(UUID->UUID.UUID_16))
   {
      printf("   Name:             DIS\r\n");
   }

   /* Print the service name if it is GAPS.                             */
   if(GAP_COMPARE_GAP_SERVICE_UUID_TO_UUID_16(UUID->UUID.UUID_16))
   {
      printf("   Name:             GAPS\r\n");
   }

   /* Print the service name if it is PLXS.                             */
   if(PLXS_COMPARE_PLXS_SERVICE_UUID_TO_UUID_16(UUID->UUID.UUID_16))
   {
      printf("   Name:             PLXS\r\n");
   }
}

   /* The following function is a utility function that provides a      */
   /* mechanism of populating the GATT Service Changed attribute handle */
   /* with the information discovered from a GATT Discovery operation.  */
static void GATTPopulateHandles(GATT_Client_Info_t *ClientInfo, GATT_Service_Discovery_Indication_Data_t *ServiceInfo)
{
   unsigned int                                  Index;
   GATT_Characteristic_Information_t            *CharacteristicInfoPtr;
   GATT_Characteristic_Descriptor_Information_t *DescriptorInfoPtr;

   /* Verify that the input parameters are semi-valid.                  */
   if((ClientInfo) && (ServiceInfo) && (ServiceInfo->ServiceInformation.UUID.UUID_Type == guUUID_16) && (GATT_COMPARE_GATT_SERVICE_UUID_TO_BLUETOOTH_UUID_16(ServiceInfo->ServiceInformation.UUID.UUID.UUID_16)))
   {
      /* Loop through all characteristics discovered in the service     */
      /* and populate the correct entry.                                */
      CharacteristicInfoPtr = ServiceInfo->CharacteristicInformationList;
      if(CharacteristicInfoPtr)
      {
         for(Index = 0; Index < ServiceInfo->NumberOfCharacteristics; Index++, CharacteristicInfoPtr++)
         {
            /* Print the characteristic information discovered if       */
            /* needed.                                                  */
            if(PRINT_SERVICE_DISCOVERY_INFORMATION)
            {
               printf("\r\nGATT Characteristic:\r\n");
               printf("   Handle:       0x%04X\r\n", CharacteristicInfoPtr->Characteristic_Handle);
               printf("   Properties:   0x%02X\r\n", CharacteristicInfoPtr->Characteristic_Properties);
               printf("   UUID:         0x");
               DisplayUUID(&(CharacteristicInfoPtr->Characteristic_UUID));
               printf("   Descriptors:  %u\r\n", CharacteristicInfoPtr->NumberOfDescriptors);
            }

            /* All DIS UUIDs are defined to be 16 bit UUIDs.            */
            if(CharacteristicInfoPtr->Characteristic_UUID.UUID_Type == guUUID_16)
            {
               /* Determine which characteristic this is.               */
               /* * NOTE * We do not care about this handle, however we */
               /*          need to get the following CCCD handle.       */
               if(GATT_COMPARE_SERVICE_CHANGED_ATTRIBUTE_TYPE_TO_BLUETOOTH_UUID_16(CharacteristicInfoPtr->Characteristic_UUID.UUID.UUID_16))
               {
                  /* Loop through the Descriptor list since we are      */
                  /* expecting to find a descriptor for this            */
                  /* Characteristic.                                    */
                  for(Index = 0; Index < CharacteristicInfoPtr->NumberOfDescriptors; Index++)
                  {
                     /* Store the handle.                               */
                     ClientInfo->ServiceChanged = CharacteristicInfoPtr->Characteristic_Handle;

                     /* Store a pointer to the Characteristic           */
                     /* information.                                    */
                     DescriptorInfoPtr = &(CharacteristicInfoPtr->DescriptorList[Index]);

                     /* Print the descriptor information discovered if  */
                     /* needed.                                         */
                     if(PRINT_SERVICE_DISCOVERY_INFORMATION)
                     {
                        printf("\r\nDescriptor:\r\n");
                        printf("   Handle:  0x%04X\r\n", DescriptorInfoPtr->Characteristic_Descriptor_Handle);
                        printf("   UUID:    0x");
                        DisplayUUID(&(DescriptorInfoPtr->Characteristic_Descriptor_UUID));
                     }

                     /* Check that this is a 16 bit Characteristic      */
                     /* Descriptor UUID.                                */
                     if(DescriptorInfoPtr->Characteristic_Descriptor_UUID.UUID_Type == guUUID_16)
                     {
                        /* Check for the CCCD.                          */
                        if(GATT_COMPARE_CLIENT_CHARACTERISTIC_CONFIGURATION_ATTRIBUTE_TYPE_TO_BLUETOOTH_UUID_16(DescriptorInfoPtr->Characteristic_Descriptor_UUID.UUID.UUID_16))
                        {
                           /* Store the handle.                         */
                           ClientInfo->ServiceChangedCCCD = DescriptorInfoPtr->Characteristic_Descriptor_Handle;

                           /* Get the next descriptor.                  */
                           continue;
                        }

                        /* Always print warnings for unknown            */
                        /* Descriptors.                                 */
                        printf("\r\nWarning - Unknown Descriptor:\r\n");
                        printf("   Handle:  0x%04X\r\n", DescriptorInfoPtr->Characteristic_Descriptor_Handle);
                        printf("   UUID:    0x");
                        DisplayUUID(&(DescriptorInfoPtr->Characteristic_Descriptor_UUID));
                     }
                     else
                        printf("\r\nWarning - Characteristic Descriptor not a 16 bit UUID.\r\n");
                  }
                  continue;
               }

               /* Always print warnings about unknown DIS               */
               /* Characteristics .                                     */
               printf("\r\nWarning - Unknown GATT Characteristic.\r\n");
               printf("\r\nCharacteristic Handle: 0x%04X\r\n", CharacteristicInfoPtr->Characteristic_Handle);
               printf("   Properties:         0x%02X\r\n", CharacteristicInfoPtr->Characteristic_Properties);
               printf("   UUID:               0x");
               DisplayUUID(&(CharacteristicInfoPtr->Characteristic_UUID));
            }
         }
      }
   }
}

   /* The following function is responsible for displaying the GATT     */
   /* service discovery summary.                                        */
static void DisplayGATTDiscoverySummary(GATT_Client_Info_t *ClientInfo)
{
   printf("\r\nGATT Service Discovery Summary\r\n\r\n");
   printf("   Service Changed:       %s\r\n", (ClientInfo->ServiceChanged     ? "Supported" : "Not Supported"));
   printf("   Service Changed CCCD:  %s\r\n", (ClientInfo->ServiceChangedCCCD ? "Supported" : "Not Supported"));
}

   /* The following command is responsible for discovering all services.*/
static int DiscoverAllServicesCommand(ParameterList_t *TempParam)
{
   int           ret_val = 0;
   DeviceInfo_t *DeviceInfo;

   /* Verify that we are not configured as the PLXP Server              */
   if(!PLXSInstanceID)
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
               /* Start the service discovery process.                  */
               ret_val = GATT_Start_Service_Discovery(BluetoothStackID, ConnectionID, 0, NULL, GATT_Service_Discovery_Event_Callback, sdAll);
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
               printf("\r\nA service discovery operation is already in progress.\r\n");
         }
         else
            printf("\r\nUnknown PLXP Server.\r\n");
      }
      else
         printf("\r\nMust be connected to an PLXP Server.\r\n");
   }
   else
      printf("\r\nOnly an PLXP Client can discover PLXS.\r\n");

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

   /* The following function is responsible for displaying the DIS      */
   /* service discovery summary.                                        */
static void DisplayDISDiscoverySummary(DIS_Client_Info_t *ClientInfo)
{
   printf("\r\nDIS Service Discovery Summary\r\n\r\n");
   printf("   Manufacturer Name:  %s\r\n", (ClientInfo->ManufacturerNameHandle ? "Supported" : "Not Supported"));
   printf("   Model Number:       %s\r\n", (ClientInfo->ModelNumberHandle      ? "Supported" : "Not Supported"));
   printf("   Serial Number:      %s\r\n", (ClientInfo->SerialNumberHandle     ? "Supported" : "Not Supported"));
   printf("   Hardware Revision:  %s\r\n", (ClientInfo->HardwareRevisionHandle ? "Supported" : "Not Supported"));
   printf("   Firmware Revision:  %s\r\n", (ClientInfo->FirmwareRevisionHandle ? "Supported" : "Not Supported"));
   printf("   Software Revision:  %s\r\n", (ClientInfo->SoftwareRevisionHandle ? "Supported" : "Not Supported"));
   printf("   SystemID:           %s\r\n", (ClientInfo->SystemIDHandle         ? "Supported" : "Not Supported"));
   printf("   IEEE 11073 Cert:    %s\r\n", (ClientInfo->IEEE11073CertHandle    ? "Supported" : "Not Supported"));
   printf("   PnP ID:             %s\r\n", (ClientInfo->PnPIDHandle            ? "Supported" : "Not Supported"));
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
      /* Loop through all characteristics discovered in the service     */
      /* and populate the correct entry.                                */
      CurrentCharacteristic = ServiceInfo->CharacteristicInformationList;
      if(CurrentCharacteristic)
      {
         for(Index1 = 0; Index1 < ServiceInfo->NumberOfCharacteristics; Index1++, CurrentCharacteristic++)
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

   /* The following function is responsible for displaying the GAPS     */
   /* service discovery summary.                                        */
static void DisplayGAPSDiscoverySummary(GAPS_Client_Info_t *ClientInfo)
{
   printf("\r\nGAPS Service Discovery Summary\r\n\r\n");
   printf("   Device Name:        %s\r\n", (ClientInfo->DeviceNameHandle       ? "Supported" : "Not Supported"));
   printf("   Device Appearance:  %s\r\n", (ClientInfo->DeviceAppearanceHandle ? "Supported" : "Not Supported"));
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

   /* The following function is a utility function that provides a      */
   /* mechanism of populating an PLXP Client Information structure with */
   /* the information discovered from a GATT Service Discovery          */
   /* operation.                                                        */
static void PLXSPopulateHandles(PLXS_Client_Information_t *ClientInfo, GATT_Service_Discovery_Indication_Data_t *ServiceDiscoveryData)
{
   unsigned int                       Index;
   GATT_Characteristic_Information_t *CharacteristicInfoPtr;

   /* Verify that the input parameters are semi-valid.                  */
   if((ClientInfo) && (ServiceDiscoveryData) && (ServiceDiscoveryData->ServiceInformation.UUID.UUID_Type == guUUID_16) && (PLXS_COMPARE_PLXS_SERVICE_UUID_TO_UUID_16(ServiceDiscoveryData->ServiceInformation.UUID.UUID.UUID_16)))
   {
      /* Loop through all characteristics discovered in the service and */
      /* populate the correct entry.                                    */
      for(Index = 0; Index < ServiceDiscoveryData->NumberOfCharacteristics; Index++)
      {
         /* Store a pointer to the Characteristic information.          */
         CharacteristicInfoPtr = &(ServiceDiscoveryData->CharacteristicInformationList[Index]);

         /* Print the characteristic information discovered if needed.  */
         if(PRINT_SERVICE_DISCOVERY_INFORMATION)
         {
            printf("\r\nPLXS Characteristic:\r\n");
            printf("   Handle:       0x%04X\r\n", CharacteristicInfoPtr->Characteristic_Handle);
            printf("   Properties:   0x%02X\r\n", CharacteristicInfoPtr->Characteristic_Properties);
            printf("   UUID:         0x");
            DisplayUUID(&(CharacteristicInfoPtr->Characteristic_UUID));
            printf("   Descriptors:  %u\r\n", CharacteristicInfoPtr->NumberOfDescriptors);
         }

         /* All PLXS UUIDs are defined to be 16 bit UUIDs.              */
         if(CharacteristicInfoPtr->Characteristic_UUID.UUID_Type == guUUID_16)
         {
            /* PLXS Spot Check Measurement.                             */
            if(PLXS_COMPARE_SPOT_CHECK_MEASUREMENT_CHARACTERISTIC_UUID_TO_UUID_16(CharacteristicInfoPtr->Characteristic_UUID.UUID.UUID_16))
            {
               /* Verify the mandatory properties.                      */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_INDICATE))
                  printf("Warning - Mandatory indicate property not supported!\r\n");

               /* Store the handle.                                     */
               ClientInfo->Spot_Check_Measurement = CharacteristicInfoPtr->Characteristic_Handle;

               /* Store the CCCD descriptor handle.                     */
               StoreDescriptorHandles(ClientInfo, CharacteristicInfoPtr, &(ClientInfo->Spot_Check_Measurement_CCCD));

               /* Get the next Characteristic.                          */
               continue;
            }

            /* PLXS Continuous Measurement.                             */
            if(PLXS_COMPARE_CONTINUOUS_MEASUREMENT_CHARACTERISTIC_UUID_TO_UUID_16(CharacteristicInfoPtr->Characteristic_UUID.UUID.UUID_16))
            {
               /* Verify the mandatory properties.                      */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_NOTIFY))
                  printf("Warning - Mandatory notify property not supported!\r\n");

               /* Store the handle.                                     */
               ClientInfo->Continuous_Measurement = CharacteristicInfoPtr->Characteristic_Handle;

               /* Store the CCCD descriptor handle.                     */
               StoreDescriptorHandles(ClientInfo, CharacteristicInfoPtr, &(ClientInfo->Continuous_Measurement_CCCD));

               /* Get the next Characteristic.                          */
               continue;
            }

            /* PLXS Features.                                           */
            if(PLXS_COMPARE_PLX_FEATURES_CHARACTERISTIC_UUID_TO_UUID_16(CharacteristicInfoPtr->Characteristic_UUID.UUID.UUID_16))
            {
               /* Verify the mandatory properties.                      */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
                  printf("Warning - Mandatory read property not supported!\r\n");

               /* Store the handle.                                     */
               ClientInfo->PLX_Features = CharacteristicInfoPtr->Characteristic_Handle;

               /* Get the next Characteristic.                          */
               continue;
            }

            /* PLXS Record Access Control Point.                        */
            if(PLXS_COMPARE_RECORD_ACCESS_CONTROL_POINT_TYPE_UUID_TO_UUID_16(CharacteristicInfoPtr->Characteristic_UUID.UUID.UUID_16))
            {
               /* Verify the mandatory properties.                      */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_WRITE))
                  printf("Warning - Mandatory write property not supported!\r\n");

               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_INDICATE))
                  printf("Warning - Mandatory indicate property not supported!\r\n");

               /* Store the handle.                                     */
               ClientInfo->Record_Access_Control_Point = CharacteristicInfoPtr->Characteristic_Handle;

               /* Store the CCCD descriptor handle.                     */
               StoreDescriptorHandles(ClientInfo, CharacteristicInfoPtr, &(ClientInfo->Record_Access_Control_Point_CCCD));

               /* Get the next Characteristic.                          */
               continue;
            }

            /* Always print warnings for unknown Characteristics.       */
            printf("\r\nWarning - Unknown Characteristic:\r\n");
            printf("   Handle:       0x%04X\r\n", CharacteristicInfoPtr->Characteristic_Handle);
            printf("   Properties:   0x%02X\r\n", CharacteristicInfoPtr->Characteristic_Properties);
            printf("   UUID:         0x");
            DisplayUUID(&(CharacteristicInfoPtr->Characteristic_UUID));
            printf("   Descriptors:  %u\r\n", CharacteristicInfoPtr->NumberOfDescriptors);
         }
         else
            printf("\r\nWarning - Characteristic not a 16 bit UUID.\r\n");
      }
   }
}

   /* The following function is responsible for populating descriptor   */
   /* handles for an PLXS Characteristic.                               */
static void StoreDescriptorHandles(PLXS_Client_Information_t *ClientInfo, GATT_Characteristic_Information_t *CharacteristicInfoPtr, Word_t *Handle)
{
   unsigned int                                  Index;
   GATT_Characteristic_Descriptor_Information_t *DescriptorInfoPtr;

   /* Loop through the Descriptor list since we are expecting to find a */
   /* descriptor for this Characteristic.                               */
   for(Index = 0; Index < CharacteristicInfoPtr->NumberOfDescriptors; Index++)
   {
      /* Store a pointer to the Characteristic information.             */
      DescriptorInfoPtr = &(CharacteristicInfoPtr->DescriptorList[Index]);

      /* Print the descriptor information discovered if needed.         */
      if(PRINT_SERVICE_DISCOVERY_INFORMATION)
      {
         printf("\r\nDescriptor:\r\n");
         printf("   Handle:  0x%04X\r\n", DescriptorInfoPtr->Characteristic_Descriptor_Handle);
         printf("   UUID:    0x");
         DisplayUUID(&(DescriptorInfoPtr->Characteristic_Descriptor_UUID));
      }

      /* Check that this is a 16 bit Characteristic Descriptor UUID.    */
      if(DescriptorInfoPtr->Characteristic_Descriptor_UUID.UUID_Type == guUUID_16)
      {
         /* Check for the CCCD.                                         */
         if(GATT_COMPARE_CLIENT_CHARACTERISTIC_CONFIGURATION_ATTRIBUTE_TYPE_TO_BLUETOOTH_UUID_16(DescriptorInfoPtr->Characteristic_Descriptor_UUID.UUID.UUID_16))
         {
            /* Store the handle.                                        */
            *Handle = DescriptorInfoPtr->Characteristic_Descriptor_Handle;

            /* Get the next descriptor.                                 */
            continue;
         }

         /* Always print warnings for unknown Descriptors.              */
         printf("\r\nWarning - Unknown Descriptor:\r\n");
         printf("   Handle:  0x%04X\r\n", DescriptorInfoPtr->Characteristic_Descriptor_Handle);
         printf("   UUID:    0x");
         DisplayUUID(&(DescriptorInfoPtr->Characteristic_Descriptor_UUID));
      }
      else
         printf("\r\nWarning - Characteristic Descriptor not a 16 bit UUID.\r\n");
   }
}

   /* The following function is responsible for displaying the PLXS     */
   /* service discovery summary.                                        */
static void DisplayPLXSDiscoverySummary(PLXS_Client_Information_t *ClientInfo)
{
   printf("\r\nPLXS Service Discovery Summary\r\n\r\n");
   printf("   Continuous Measurement           : %s\r\n", (ClientInfo->Continuous_Measurement           ? "Supported" : "Not Supported"));
   printf("   Continuous Measurement CCCD      : %s\r\n", (ClientInfo->Continuous_Measurement_CCCD      ? "Supported" : "Not Supported"));
   printf("   Spot Check Measurement           : %s\r\n", (ClientInfo->Spot_Check_Measurement           ? "Supported" : "Not Supported"));
   printf("   Spot Check Measurement CCCD      : %s\r\n", (ClientInfo->Spot_Check_Measurement_CCCD      ? "Supported" : "Not Supported"));
   printf("   PLX Features                     : %s\r\n", (ClientInfo->PLX_Features                     ? "Supported" : "Not Supported"));
   printf("   Record Access Control Point      : %s\r\n", (ClientInfo->Record_Access_Control_Point      ? "Supported" : "Not Supported"));
   printf("   Record Access Control Point CCCD : %s\r\n", (ClientInfo->Record_Access_Control_Point_CCCD ? "Supported" : "Not Supported"));
}

   /* The following function is responsible for registering PLXS.       */
static int RegisterPLXS(void)
{
   int                    ret_val = 0;
   PLXS_Initialize_Data_t InitializeData;

   /* Verify that there is no active connection.                        */
   if(!ConnectionID)
   {
      /* Verify that the Service is not already registered.             */
      if(!PLXSInstanceID)
      {
         /* We will initialize all features for the Pulse Oximeter      */
         /* Service (PLXS).                                             */
         InitializeData.Continuous_Measurement = TRUE;
         InitializeData.Spot_Check_Measurement = TRUE;
         InitializeData.Measurement_Storage    = TRUE;

         /* Initialize the service.                                     */
         ret_val = PLXS_Initialize_Service(BluetoothStackID, (unsigned int)PLXS_SERVICE_FLAGS_DUAL_MODE, &InitializeData, PLXS_EventCallback, BluetoothStackID, &PLXSInstanceID);
         if((ret_val > 0) && (PLXSInstanceID > 0))
         {
            /* Display succplxs message.                                */
            printf("Successfully registered PLXS Service, PLXSInstanceID = %u.\r\n", ret_val);

            /* Save the ServiceID of the registered service.            */
            PLXSInstanceID = (unsigned int)ret_val;

            /* Store the default features of the PLXS Server.           */
            PLX_Feature.Support_Features                       = (Word_t)PLXP_DEFAULT_PLXS_FEATURES;
            PLX_Feature.Measurement_Status_Support             = (Word_t)PLXP_DEFAULT_MEASUREMENT_STATUS_SUPPORT;
            PLX_Feature.Device_And_Sensor_Status_Support.Lower = (Word_t)((DWord_t)PLXP_DEFAULT_DEVICE_AND_SENSOR_STATUS_SUPPORT & 0x0000FFFF);
            PLX_Feature.Device_And_Sensor_Status_Support.Upper = (Byte_t)(((DWord_t)PLXP_DEFAULT_DEVICE_AND_SENSOR_STATUS_SUPPORT & 0x00FF0000) >> 16);

            /* Simply return success to the caller.                     */
            ret_val = 0;
         }
         else
            DisplayFunctionError("PLXS_Initialize_Service", ret_val);
      }
      else
      {
         printf("\r\nPLXS is already registered.\r\n");
      }
   }
   else
   {
      printf("\r\nConnection currently active.\r\n");
   }

   return(ret_val);
}

   /* The following function is responsible for un-registering PLXS.    */
static int UnregisterPLXS(void)
{
   int           ret_val = 0;
   DeviceInfo_t *DeviceInfo;

   /* Verify that a service is registered.                              */
   if(PLXSInstanceID)
   {
      /* Get the device info for the connected device.                  */
      /* * NOTE * We will still un-register the service if the device   */
      /*          information cannot be found.  This means the link is  */
      /*          already disconnected.                                 */
      if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
      {
         /* If there is a connected device, then first disconnect it.   */
         if(!COMPARE_NULL_BD_ADDR(ConnectionBD_ADDR))
         {
            /* Determine the transport to disconnect.                   */
            if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_BR_EDR_CONNECTED)
            {
               /* Disconnect over BR/EDR                                */
               GATT_Disconnect(BluetoothStackID, ConnectionID);
            }
            else
            {
               /* Disconnect over LE.                                   */
               DisconnectLEDevice(BluetoothStackID, ConnectionBD_ADDR);
            }
         }
      }

      /* Unregister the PLXS Service with GATT.                         */
      ret_val = PLXS_Cleanup_Service(BluetoothStackID, PLXSInstanceID);
      if(ret_val == 0)
      {
         /* Display success message.                                    */
         printf("\r\nSuccessfully unregistered PLXS Service InstanceID %u.\r\n", PLXSInstanceID);

         /* Clear the InstanceID.                                       */
         PLXSInstanceID = 0;
      }
      else
         DisplayFunctionError("PLXS_Cleanup_Service", ret_val);
   }
   else
      printf("\r\nPLXS Service not registered.\r\n");

   return(ret_val);
}

   /* The following function will read a PLXS Characteristic.  This     */
   /* function may be called by the PLXS Client ONLY.                   */
   /* * NOTE * This function requires that the caller correctly set the */
   /*          PLXSRequestData.                                         */
static int ReadPLXSCharacteristic(void)
{
   int           Result;
   DeviceInfo_t *DeviceInfo;

   /* If we are the PLXS Client.                                        */
   if(!PLXSInstanceID)
   {
      /* Make sure that a connection exists.                            */
      if(ConnectionID)
      {
         /* Get the device info for the remote device.                  */
         /* * NOTE * ConnectionBD_ADDR should be valid if ConnectionID  */
         /*          is.                                                */
         if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
         {
            /* Make sure we have performed service discovery since we   */
            /* need the attribute handle to make the request.           */
            if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_PLXS_SERVICE_DISCOVERY_COMPLETE)
            {
               /* Make sure the attribute handle is set to zero in case */
               /* an error occurs.                                      */
               PLXSRequestData.AttributeHandle = 0;

               /* Determine the attribute handle based on the attribute */
               /* handle type.                                          */
               switch(PLXSRequestData.AttributeHandleType)
               {
                  case ahtPLX_Features:
                     PLXSRequestData.AttributeHandle = DeviceInfo->PLXSClientInfo.PLX_Features;
                     break;
                  case ahtSpot_Check_Measurement_CCCD:
                     PLXSRequestData.AttributeHandle = DeviceInfo->PLXSClientInfo.Spot_Check_Measurement_CCCD;
                     break;
                  case ahtContinuous_Measurement_CCCD:
                     PLXSRequestData.AttributeHandle = DeviceInfo->PLXSClientInfo.Continuous_Measurement_CCCD;
                     break;
                  case ahtRecordAccessControlPoint_CCCD:
                     PLXSRequestData.AttributeHandle = DeviceInfo->PLXSClientInfo.Record_Access_Control_Point_CCCD;
                     break;
                  default:
                     printf("\r\nCharacteristic specified CANNOT be read.\r\n");
                     break;
               }

               /* Make sure the attribute handle is valid.              */
               if(PLXSRequestData.AttributeHandle)
               {
                  /* Send the read request.                             */
                  /* * NOTE * We will not save the transactionID        */
                  /*          returned by this function, which we could */
                  /*          use to cancel the request.                */
                  if((Result = GATT_Read_Value_Request(BluetoothStackID, ConnectionID, PLXSRequestData.AttributeHandle, GATT_ClientEventCallback_PLXS, PLXSRequestData.AttributeHandle)) > 0)
                  {
                     printf("\r\nGATT Read Value Request sent:\r\n");
                     printf("   TransactionID:     %d\r\n", Result);
                     printf("   Attribute Handle:  0x%04X\r\n", PLXSRequestData.AttributeHandle);

                  }
                  else
                     DisplayFunctionError("GATT_Write_Request", Result);
               }
               else
                  printf("\r\nAttribute handle is invalid.\r\n");
            }
            else
               printf("\r\nPLXS Service discovery has not been performed.\r\n");
         }
         else
            printf("\r\nNo device information for the PLXS Server.\r\n");
      }
      else
         printf("\r\nNo connection to the PLXS Server.\r\n");
   }
   else
      printf("\r\nOnly the PLXS Client may read PLXS Characteristics.\r\n");

   return(0);
}

   /* The following function will write a PLXS Characteristic.  This    */
   /* function may be called by the PLXS Client ONLY.                   */
   /* * NOTE * This function requires that the caller correctly set the */
   /*          PLXSRequestData.                                         */
static int WritePLXSCharacteristic(void)
{
   int                 Result;
   DeviceInfo_t       *DeviceInfo;
   NonAlignedWord_t    Client_Configuration;
   PLXS_RACP_Request_t RACP_Request;

   /* If we are the PLXS Client.                                        */
   if(!PLXSInstanceID)
   {
      /* Make sure that a connection exists.                            */
      if(ConnectionID)
      {
         /* Get the device info for the remote device.                  */
         /* * NOTE * ConnectionBD_ADDR should be valid if ConnectionID  */
         /*          is.                                                */
         if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
         {
            /* Make sure we have performed service discovery since we   */
            /* need the attribute handle to make the request.           */
            if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_PLXS_SERVICE_DISCOVERY_COMPLETE)
            {
               /* Make sure the attribute handle is set to zero in case */
               /* an error occurs.                                      */
               PLXSRequestData.AttributeHandle = 0;

               /* Determine the attribute handle based on the attribute */
               /* handle type.                                          */
               switch(PLXSRequestData.AttributeHandleType)
               {
                  case ahtSpot_Check_Measurement_CCCD:
                     PLXSRequestData.AttributeHandle = DeviceInfo->PLXSClientInfo.Spot_Check_Measurement_CCCD;
                     break;
                  case ahtContinuous_Measurement_CCCD:
                     PLXSRequestData.AttributeHandle = DeviceInfo->PLXSClientInfo.Continuous_Measurement_CCCD;
                     break;
                  case ahtRecordAccessControlPoint_CCCD:
                     PLXSRequestData.AttributeHandle = DeviceInfo->PLXSClientInfo.Record_Access_Control_Point_CCCD;
                     break;
                  case ahtRecordAccessControlPoint:
                     PLXSRequestData.AttributeHandle = DeviceInfo->PLXSClientInfo.Record_Access_Control_Point;
                     break;
                  case ahtServiceChanged_CCCD:
                     PLXSRequestData.AttributeHandle = DeviceInfo->GATTClientInfo.ServiceChangedCCCD;
                     break;
                  default:
                     printf("\r\nCharacteristic specified CANNOT be written.\r\n");
                     break;
               }

               /* Format the request depending on the attribute handle  */
               /* type.                                                 */
               switch(PLXSRequestData.AttributeHandleType)
               {
                  case ahtSpot_Check_Measurement_CCCD:
                  case ahtContinuous_Measurement_CCCD:
                  case ahtRecordAccessControlPoint_CCCD:
                  case ahtServiceChanged_CCCD:
                     /* Intentional fall-through.                       */

                     /* Make sure the attribute handle is valid.        */
                     if(PLXSRequestData.AttributeHandle)
                     {
                        /* Format the configuration.                    */
                        ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&Client_Configuration, PLXSRequestData.Data.Configuration);

                        /* Send the write request.                      */
                        /* * NOTE * We will not save the transactionID  */
                        /*          returned by this function, which we */
                        /*          could use to cancel the request.    */
                        if((Result = GATT_Write_Request(BluetoothStackID, ConnectionID, PLXSRequestData.AttributeHandle, NON_ALIGNED_WORD_SIZE, (Byte_t *)&Client_Configuration, GATT_ClientEventCallback_PLXS, PLXSRequestData.AttributeHandle)) > 0)
                        {
                           printf("\r\nGATT Write Request sent:\r\n");
                           printf("   TransactionID:     %d\r\n", Result);
                           printf("   Attribute Handle:  0x%04X\r\n", PLXSRequestData.AttributeHandle);

                        }
                        else
                           DisplayFunctionError("GATT_Write_Request", Result);
                     }
                     else
                        printf("\r\nAttribute handle is invalid.\r\n");
                     break;
                  case ahtRecordAccessControlPoint:
                     /* Make sure the attribute handle is valid.        */
                     if(PLXSRequestData.AttributeHandle)
                     {
                        /* Format the RACP Request.                     */
                        ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&(RACP_Request.Op_Code), PLXSRequestData.Data.RACPRequestData.Op_Code);
                        ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&(RACP_Request.Operator), PLXSRequestData.Data.RACPRequestData.Operator);

                     /* Send the write request.                         */
                     /* * NOTE * We will not save the transactionID     */
                     /*          returned by this function, which we    */
                     /*          could use to cancel the request.       */
                        if((Result = GATT_Write_Request(BluetoothStackID, ConnectionID, PLXSRequestData.AttributeHandle, PLXS_RACP_REQUEST_SIZE, (Byte_t *)&RACP_Request, GATT_ClientEventCallback_PLXS, PLXSRequestData.AttributeHandle)) > 0)
                     {
                        printf("\r\nGATT Write Request sent:\r\n");
                        printf("   TransactionID:     %d\r\n", Result);
                        printf("   Attribute Handle:  0x%04X\r\n", PLXSRequestData.AttributeHandle);

                     }
                     else
                        DisplayFunctionError("GATT_Write_Request", Result);
                     }
                     else
                        printf("\r\nAttribute handle is invalid.\r\n");
                     break;
                  default:
                     /* Prevent compiler warnings.                      */
                     break;
               }
            }
            else
               printf("\r\nPLXS Service discovery has not been performed.\r\n");
         }
         else
            printf("\r\nNo device information for the PLXS Server.\r\n");

      }
      else
         printf("\r\nNo connection to the PLXS Server.\r\n");
   }
   else
      printf("\r\nOnly the PLXS Client may write PLXS Characteristics.\r\n");

   return(0);
}

   /* The following function is responsible for creating a new Spot     */
   /* Check Measurement.  This measurement will be stored until it is   */
   /* indicated.                                                        */
static void CreateSpotCheckMeasurement(Byte_t Flags, Word_t Measurement_Status, unsigned int Device_And_Sensor_Status)
{
   unsigned int                       Index;
   unsigned int                       Index2;
   PLXS_Spot_Check_Measurement_Data_t Measurement;
   time_t                             CurrentTime;
   struct tm                         *TimePtr;

   /* Make sure we are the server.                                      */
   if(PLXSInstanceID)
   {
      /* Initialize the new measurement.                                */
      BTPS_MemInitialize(&(Measurement), 0, PLXS_SPOT_CHECK_MEASUREMENT_DATA_SIZE);

      /* Assign the mandatory Measurement data.                         */
      Measurement.Flags = Flags;
      Measurement.SpO2  = (Word_t)(rand() % 100);
      Measurement.PR    = (Word_t)(rand() % 100);

      /* Check if we need to include the timestamp.                     */
      if(Measurement.Flags & PLXS_SPOT_CHECK_MEASUREMENT_FLAGS_TIME_STAMP_PRESENT)
      {
         /* Get the current system time to store for this generated     */
         /* measurement.                                                */
         CurrentTime = time(NULL);
         TimePtr     = localtime(&CurrentTime);

         if(TimePtr)
         {
            /* Convert the year and month into the proper format.       */
            Measurement.Timestamp.Year    = TimePtr->tm_year + 1900;
            Measurement.Timestamp.Month   = TimePtr->tm_mon + 1;
            Measurement.Timestamp.Day     = TimePtr->tm_mday;
            Measurement.Timestamp.Hours   = TimePtr->tm_hour;
            Measurement.Timestamp.Minutes = TimePtr->tm_min;
            Measurement.Timestamp.Seconds = TimePtr->tm_sec;
         }
      }

      /* Check if we need to include the Measurement Status.            */
      if(Measurement.Flags & PLXS_SPOT_CHECK_MEASUREMENT_FLAGS_MEASUREMENT_STATUS_PRESENT)
      {
         Measurement.Measurement_Status       = Measurement_Status;
      }

      /* Check if we need to include the Device and Sensor Status.      */
      if(Measurement.Flags & PLXS_SPOT_CHECK_MEASUREMENT_FLAGS_DEVICE_AND_SENSOR_STATUS_PRESENT)
      {
         Measurement.Device_And_Sensor_Status.Lower = (Word_t)(Device_And_Sensor_Status & 0x0000FFFF);
         Measurement.Device_And_Sensor_Status.Upper = (Byte_t)((Device_And_Sensor_Status & 0x00FF0000) >> 16);
      }

      /* Check if we need to include the Pulse Amplitude Index.         */
      if(Measurement.Flags & PLXS_SPOT_CHECK_MEASUREMENT_FLAGS_DEVICE_AND_SENSOR_STATUS_PRESENT)
      {
         Measurement.Pulse_Amplitude_Index    = (Word_t)(rand() % 100);
      }

      /* Loop through the Spot Check Measurement list and replace the   */
      /* oldest measurement if necessary.                               */
      for(Index = 0, Index2 = 0; Index < PLXP_MAXIMUM_NUMBER_OF_SPOT_CHECK_MEASUREMENTS; Index2++)
      {
         /* If we haven't used all the Measurement entries in the list  */
         /* yet we are guranteed to find an entry to store the          */
         /* measurement.                                                */
         if(MeasurementIDCtr < PLXP_MAXIMUM_NUMBER_OF_SPOT_CHECK_MEASUREMENTS)
         {
            /* Check if this entry has been used before.  The ID will be*/
            /* zero.                                                    */
            if(SpotCheckMeasurementList[Index].Valid == FALSE)
            {
               /* Set the required fields for this index.               */
               SpotCheckMeasurementList[Index].Measurement = Measurement;
               SpotCheckMeasurementList[Index].Valid       = TRUE;
               SpotCheckMeasurementList[Index].ID          = MeasurementIDCtr++;
               SpotCheckMeasurementList[Index].Indicated   = FALSE;

               /* We are done.                                          */
               break;
            }
         }
         else
         {
            /* We MUST replace the oldest measurement in the list.  The */
            /* oldest will be the first ID we match by Index2.          */
            if(SpotCheckMeasurementList[Index].ID == Index2)
            {
               /* Set the Spot Check Measurement data fields for this   */
               /* index.                                                */
               SpotCheckMeasurementList[Index].Measurement = Measurement;
               SpotCheckMeasurementList[Index].Valid       = TRUE;
               SpotCheckMeasurementList[Index].ID          = MeasurementIDCtr++;
               SpotCheckMeasurementList[Index].Indicated   = FALSE;

               /* We are done.                                          */
               break;
            }
         }

         /* If we are at the end of the Measurement list reset to the   */
         /* start of the list to keep checking ID's.                    */
         if(Index == (PLXP_MAXIMUM_NUMBER_OF_SPOT_CHECK_MEASUREMENTS - 1))
            Index = 0;
         else
            Index++;
      }
   }
   else
      printf("\r\nOnly the PLXS Server can create Spot Check Measurements.\r\n");
}

   /* The following function is responsible for indicating a Spot Check */
   /* Measurement.                                                      */
static void IndicateSpotCheckMeasurement(void)
{
   int                                 Result;
   unsigned int                        Index;
   unsigned int                        Index2;
   PLXS_Spot_Check_Measurement_Data_t *Measurement = NULL;
   unsigned int                        ID;

   /* Verify this is the server.                                        */
   if(PLXSInstanceID)
   {
      /* Verify we are connected.                                       */
      if(ConnectionID)
      {
         /* Find a measurement to indicate.  We will simply loop up to  */
         /* the maximum measurement ID that we have created.  This will */
         /* enforce we always indicate the oldest measurement first.    */
         /* This is inefficient but this sample shouldn't have a large  */
         /* number of measurements.                                     */
         /* * NOTE * Index will loop through the measurement list and   */
         /*          will need to be reset if we reach the end of the   */
         /*          list.                                              */
         for(Index = 0, Index2 = 0; (Index < PLXP_MAXIMUM_NUMBER_OF_SPOT_CHECK_MEASUREMENTS) && (Index2 < MeasurementIDCtr); Index2++)
         {
            /* If we find an ID that matches the counter is is already  */
            /* the oldest measurement.                                  */
            if(SpotCheckMeasurementList[Index].ID == Index2)
            {
               /* If we find a measurement that hasn't been indicated.  */
               /* Make sure the measurement is valid first.             */
               if((SpotCheckMeasurementList[Index].Valid) && (SpotCheckMeasurementList[Index].Indicated == FALSE))
               {
                  /* Flag that we indicated the measurement.            */
                  SpotCheckMeasurementList[Index].Indicated = TRUE;

                  /* Set a pointer to it.                               */
                  Measurement = &(SpotCheckMeasurementList[Index].Measurement);

                  /* Store the ID.                                      */
                  ID = SpotCheckMeasurementList[Index].ID;

                  /* We are done.                                       */
                  break;
               }
            }

            /* If we are at the end of the Measurement list reset to the*/
            /* start of the list to keep checking ID's.                 */
            if(Index == (PLXP_MAXIMUM_NUMBER_OF_SPOT_CHECK_MEASUREMENTS - 1))
               Index = 0;
            else
               Index++;
         }

         /* If we found a measurement to indicate.                      */
         if(Measurement)
         {
            /* Send the indication(s).                                  */
            if((Result = PLXS_Indicate_Spot_Check_Measurement(BluetoothStackID, PLXSInstanceID, ConnectionID, Measurement)) > 0)
            {
               printf("\r\nSpot Check Measurement indicated.\r\n");
               printf("   Measurement ID:  %u\r\n", ID);
               printf("   TransactionID:   %d\r\n", Result);
            }
            else
               DisplayFunctionError("PLXS_Indicate_Spot_Check_Measurement", Result);
         }
         else
            printf("\r\nNo Spot Check Measurements to indicate.\r\n");
      }
      else
         printf("\r\nNot connected to a remote PLXS Client.\r\n");
   }
   else
      printf("\r\nOnly the PLXS Server can send indications.\r\n");
}

   /* The following function is responsible for displaying the Spot     */
   /* Check Measurements List.                                          */
static void DisplaySpotCheckMeasurementsList(void)
{
   unsigned int Index;

   /* Make sure we are the PLXS Server.                                 */
   if(PLXSInstanceID)
   {
      /* Loop through the PLXS Measurement list and replace the oldest  */
      /* measurement if necessary.                                      */
      for(Index = 0; Index < (unsigned int)PLXP_MAXIMUM_NUMBER_OF_SPOT_CHECK_MEASUREMENTS; Index++)
      {
         /* Only print valid measurements.                              */
         if(SpotCheckMeasurementList[Index].Valid)
         {
            printf("\r\nSpot Check Measurement Data:\r\n");
            printf("   ID:                   %u\r\n", SpotCheckMeasurementList[Index].ID);
            printf("   Indicated:            %s\r\n", (SpotCheckMeasurementList[Index].Indicated) ? "Yes" : "No");
            DisplaySpotCheckMeasurement(&(SpotCheckMeasurementList[Index].Measurement));
         }
      }
   }
   else
      printf("\r\nOnly the PLXS Server can print measurements.\r\n");
}

   /* The following function is responsible for starting notifications  */
   /* of Continuous Measurements.  The measurements will be generated   */
   /* based on incoming parameters.                                     */
   /* * NOTE * The Timeout parameter is in milliseconds.                */
static void StartContinuousMeasurementNotifications(Byte_t Flags, Word_t Measurement_Status, unsigned int Device_And_Sensor_Status, unsigned long Timeout)
{
   DeviceInfo_t *DeviceInfo;

   /* Make sure we are the server.                                      */
   if(PLXSInstanceID)
   {
      /* Make sure we are connected.                                    */
      if(ConnectionID)
      {
         /* Get the device information for the remote device.           */
         if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
         {
            /* Make sure the PLXS Client has configured the Continuous  */
            /* Measurement CCCD for notifications.                      */
            if(DeviceInfo->PLXSServerInfo.Continuous_Measurement_CCCD & PLXS_CLIENT_CHARACTERISTIC_CONFIGURATION_NOTIFY_ENABLE)
            {
               /* Make sure notifications are not started.              */
               if(!(NotificationData.NotificationsStarted))
               {
                  /* Let's go ahead and store the data needed to create */
                  /* Continuous Measurements.                           */
                  NotificationData.Flags = Flags;

                  /* Check if we need to include the Measurement Status.*/
                  if(NotificationData.Flags & PLXS_CONTINUOUS_MEASUREMENT_FLAGS_MEASUREMENT_STATUS_PRESENT)
                  {
                     NotificationData.Measurement_Status       = Measurement_Status;
                  }

                  /* Check if we need to include the Device and Sensor  */
                  /* Status.                                            */
                  if(NotificationData.Flags & PLXS_CONTINUOUS_MEASUREMENT_FLAGS_DEVICE_AND_SENSOR_STATUS_PRESENT)
                  {
                     NotificationData.Device_And_Sensor_Status.Lower = (Word_t)(Device_And_Sensor_Status & 0x0000FFFF);
                     NotificationData.Device_And_Sensor_Status.Upper = (Byte_t)((Device_And_Sensor_Status & 0x00FF0000) >> 16);
                  }

                  /* Let's start sending notifications.                 */
                  NotificationData.NotificationsStarted = TRUE;
                  NotificationData.Timeout              = Timeout;

                  printf("\r\nStarting Continuous Measurement notifications.\r\n");

                  if((NotificationData.TimerID = BSC_StartTimer(BluetoothStackID, NotificationData.Timeout, PLXP_Continuous_Measurement_Timer_Callback, 0)) <= 0)
                  {
                     printf("\r\nCould not start the Continuous Measurement notification timer.\r\n");
                     NotificationData.NotificationsStarted = FALSE;
                     NotificationData.TimerID              = 0;
                  }
               }
               else
                  printf("\r\nContinuous Measurement notifications are already being sent.\r\n");
            }
            else
               printf("\r\nThe PLXS Client has NOT configured the CCCD for notifications.\r\n");
         }
         else
            printf("\r\nCould not get device information for the PLXS Client.\r\n");
      }
      else
         printf("\r\nMust be connected to a remote PLXS Client.\r\n");
   }
   else
      printf("\r\nOnly the PLXS Server can start Continuous Measurement notifications.\r\n");
}

   /* The following function is responsible for stopping Continuous     */
   /* Measurement notifications.                                        */
static void StopContinuousMeasurementNotifications(void)
{
   DeviceInfo_t *DeviceInfo;

   /* Make sure we are the server.                                      */
   if(PLXSInstanceID)
   {
      /* Make sure we are connected.                                    */
      if(ConnectionID)
      {
         /* Get the device information for the remote device.           */
         if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
         {
            /* Make sure the PLXS Client has configured the Continuous  */
            /* Measurement CCCD for notifications.                      */
            if(DeviceInfo->PLXSServerInfo.Continuous_Measurement_CCCD & PLXS_CLIENT_CHARACTERISTIC_CONFIGURATION_NOTIFY_ENABLE)
            {
               /* Make sure notifications are started.                  */
               if(NotificationData.NotificationsStarted)
               {
                  printf("\r\nStopping Continuous Measurement notifications.\r\n");

                  /* Stop the Continuous Measurement notification timer.*/
                  if(!(BSC_StopTimer(BluetoothStackID, NotificationData.TimerID)))
                  {
                     NotificationData.NotificationsStarted = FALSE;
                     NotificationData.TimerID              = 0;
                  }
                  else
                  {
                     printf("\r\nCould not stop the Continuous Measurement notification timer.\r\n");
                  }
               }
               else
                  printf("\r\nContinuous Measurement notifications are NOT being sent.\r\n");
            }
            else
               printf("\r\nThe PLXS Client has NOT configured the CCCD for notifications.\r\n");
         }
         else
            printf("\r\nCould not get device information for the PLXS Client.\r\n");
      }
      else
         printf("\r\nMust be connected to a remote PLXS Client.\r\n");
   }
   else
      printf("\r\nOnly the PLXS Server can start Continuous Measurement notifications.\r\n");
}

   /* The following function is responsible reporting the stored        */
   /* records.                                                          */
static void ReportStoredRecordsProcedure(PLXS_RACP_Request_Data_t *RequestData, PLXS_RACP_Response_Data_t *ResponseData)
{
   unsigned int Index;
   unsigned int NumberOfRecordsToReport = 0;

   if((RequestData) && (ResponseData))
   {
      /* Go ahead and set the Op Code and Operator for the RACP Response*/
      /* Data.                                                          */
      ResponseData->Response_Op_Code = rrotResponseOpCode;
      ResponseData->Operator         = rotNull;

      /* Only the All Records operator may be used for this procedure.  */
      if(RequestData->Operator == rotAllRecords)
      {
         /* Loop through the Spot Check Measurement List to determine   */
         /* the number of records that need to be reported.             */
         for(Index = 0; Index < PLXP_MAXIMUM_NUMBER_OF_SPOT_CHECK_MEASUREMENTS; Index++)
         {
            /* We will only include records that have not been          */
            /* indicated.                                               */
            if((SpotCheckMeasurementList[Index].Valid) && (SpotCheckMeasurementList[Index].Indicated == FALSE))
            {
               NumberOfRecordsToReport++;
            }
         }

         /* Make sure we found records to indicate.                     */
         if(NumberOfRecordsToReport)
         {
            printf("\r\nThe number of stored records to indicate: %u.\r\n", NumberOfRecordsToReport);

            /* Success.                                                 */
            ResponseData->Operand.Response_Code = rcvPLXSSuccess;
         }
         else
         {
            /* No records found.                                        */
            ResponseData->Operand.Response_Code = rcvPLXSNoRecordFound;
         }
      }
      else
      {
         /* Operator not supported.                                     */
         ResponseData->Operand.Response_Code = rcvPLXSInvalidOperator;
      }
   }
}

   /* The following function is responsible for deleting stored records.*/
static void DeleteStoredRecordsProcedure(PLXS_RACP_Request_Data_t *RequestData, PLXS_RACP_Response_Data_t *ResponseData)
{
   unsigned int Index;
   unsigned int NumberOfRecordsDeleted = 0;

   if((RequestData) && (ResponseData))
   {
      /* Go ahead and set the Op Code and Operator for the RACP Response*/
      /* Data.                                                          */
      ResponseData->Response_Op_Code = rrotResponseOpCode;
      ResponseData->Operator         = rotNull;

      /* Only the All Records operator may be used for this procedure.  */
      if(RequestData->Operator == rotAllRecords)
      {
         /* Loop through the Spot Check Measurement List to determine   */
         /* the number of records that need to be reported.             */
         for(Index = 0; Index < PLXP_MAXIMUM_NUMBER_OF_SPOT_CHECK_MEASUREMENTS; Index++)
         {
            /* We will only delete records that are valid.              */
            if(SpotCheckMeasurementList[Index].Valid)
            {
               /* Mark the measurement as deleted.                      */
               SpotCheckMeasurementList[Index].Valid = FALSE;

               NumberOfRecordsDeleted++;
            }
         }

         /* Make sure we found records to delete.                       */
         if(NumberOfRecordsDeleted)
         {
            printf("\r\nThe number of records deleted: %u.\r\n", NumberOfRecordsDeleted);

            /* Success.                                                 */
            ResponseData->Operand.Response_Code = rcvPLXSSuccess;
         }
         else
         {
            /* No records found.                                        */
            ResponseData->Operand.Response_Code = rcvPLXSNoRecordFound;
         }
      }
      else
      {
         /* Operator not supported.                                     */
         ResponseData->Operand.Response_Code = rcvPLXSOperatorNotSupported;
      }
   }
}

   /* The following function is responsible for                         */
static void NumberOfStoredRecordsProcedure(PLXS_RACP_Request_Data_t *RequestData, PLXS_RACP_Response_Data_t *ResponseData)
{
   unsigned int Index;
   unsigned int NumberOfRecords = 0;

   if((RequestData) && (ResponseData))
   {
      /* Go ahead and set the Operator for the RACP Response Data.      */
      ResponseData->Operator = rotNull;

      /* Only the All Records operator may be used for this procedure.  */
      if(RequestData->Operator == rotAllRecords)
      {
         /* Loop through the Spot Check Measurement List to determine   */
         /* the number of records that need to be reported.             */
         for(Index = 0; Index < PLXP_MAXIMUM_NUMBER_OF_SPOT_CHECK_MEASUREMENTS; Index++)
         {
            /* We will only include records that have not been          */
            /* indicated.                                               */
            if((SpotCheckMeasurementList[Index].Valid) && (SpotCheckMeasurementList[Index].Indicated == FALSE))
            {
               NumberOfRecords++;
            }
         }

         /* Success.                                                    */
         ResponseData->Response_Op_Code                 = rrotNumberOfStoredRecordsResponse;
         ResponseData->Operand.Number_Of_Stored_Records = (Word_t)NumberOfRecords;
      }
      else
      {
         /* Operator not supported.                                     */
         ResponseData->Response_Op_Code      = rrotResponseOpCode;
         ResponseData->Operand.Response_Code = rcvPLXSOperatorNotSupported;
      }
   }
}

   /* The following function is responsible for displaying the CCCD     */
   /* Type.                                                             */
static void DisplayCCCDType(PLXS_CCCD_Type_t Type)
{
   printf("\r\nPLXS CCCD Type:\r\n");
   printf("   Type: ");
   switch(Type)
   {
      case pcdSpotCheck:
         printf("Spot Check Measurement.\r\n");
         break;
      case pcdContinuous:
         printf("Continuous Measurement.\r\n");
         break;
      case pcdRACP:
         printf("Record Access Control Point.\r\n");
         break;
      default:
         printf("Invalid.\r\n");
         break;
   }
}

   /* The following function is responsible for displaying the RACP     */
   /* Request Data.                                                     */
static void DisplayRACPRequestData(PLXS_RACP_Request_Data_t *RequestData)
{
   printf("\r\nRACP Request Data:\r\n");

   if(RequestData)
   {
      /* Display the Op Code.                                           */
      printf("   Op Code:  ");
      switch(RequestData->Op_Code)
      {
         case rrtReportStoredRecordsRequest:
            printf("Report Stored Records Request.\r\n");
            break;
         case rrtDeleteStoredRecordsRequest:
            printf("Delete Stored Records Request.\r\n");
            break;
         case rrtAbortOperationRequest:
            printf("Abort Operation Request.\r\n");
            break;
         case rrtNumberOfStoredRecordsRequest:
            printf("Number of Stored Records Request.\r\n");
            break;
         default:
            printf("Invalid.\r\n");
            break;
      }

      /* Display the Operator.                                          */
      printf("   Operator: ");
      switch(RequestData->Operator)
      {
         case rotNull:
            printf("Null.\r\n");
            break;
         case rotAllRecords:
            printf("All Records.\r\n");
            break;
         default:
            printf("Invalid.\r\n");
            break;
      }
   }
}

   /* The following function is responsible for displaying the RACP     */
   /* Response Data.                                                    */
static void DisplayRACPResponseData(PLXS_RACP_Response_Data_t *ResponseData)
{
   printf("\r\nRACP Response Data:\r\n");

   if(ResponseData)
   {
      /* Display the Op Code.                                           */
      printf("   Response Op Code:  ");
      switch(ResponseData->Response_Op_Code)
      {
         case rrotNumberOfStoredRecordsResponse:
            printf("Number Of Stored Records Response.\r\n");
            break;
         case rrotResponseOpCode:
            printf("Response Op Code.\r\n");
            break;
         default:
            printf("Invalid.\r\n");
            break;
      }

      /* Display the Operator.                                          */
      printf("   Operator:          ");
      switch(ResponseData->Operator)
      {
         case rotNull:
            printf("Null.\r\n");
            break;
         case rotAllRecords:
            printf("All Records.\r\n");
            break;
         default:
            printf("Invalid.\r\n");
            break;
      }

      printf("   Request Op Code:   ");
      switch(ResponseData->Request_Op_Code)
      {
         case rrtReportStoredRecordsRequest:
            printf("Report Stored Records Request.\r\n");
            break;
         case rrtDeleteStoredRecordsRequest:
            printf("Delete Stored Records Request.\r\n");
            break;
         case rrtAbortOperationRequest:
            printf("Abort Operation Request.\r\n");
            break;
         case rrtNumberOfStoredRecordsRequest:
            printf("Number of Stored Records Request.\r\n");
            break;
         default:
            printf("Invalid.\r\n");
            break;
      }

      /* Display the Operand.                                           */
      printf("   Operand:           ");
      switch(ResponseData->Response_Op_Code)
      {
         case rrotNumberOfStoredRecordsResponse:
            printf("%u.\r\n", (unsigned int)(ResponseData->Operand.Number_Of_Stored_Records));
            break;
         case rrotResponseOpCode:
            /* This is a response code.                                 */
            switch(ResponseData->Operand.Response_Code)
            {
               case rcvPLXSSuccess:
                  printf("Success.\r\n");
                  break;
               case rcvPLXSOpCodeNotSupported:
                  printf("Op Code Not Supported.\r\n");
                  break;
               case rcvPLXSInvalidOperator:
                  printf("Invalid Operator.\r\n");
                  break;
               case rcvPLXSOperatorNotSupported:
                  printf("Operator Not Supported.\r\n");
                  break;
               case rcvPLXSInvalidOperand:
                  printf("Invalid Operand.\r\n");
                  break;
               case rcvPLXSNoRecordFound:
                  printf("No Records Found.\r\n");
                  break;
               case rcvPLXSAbortUnsuccessful:
                  printf("Abort Unsuccessful.\r\n");
                  break;
               case rcvPLXSProcedureNotCompleted:
                  printf("Procedure Not Completed.\r\n");
                  break;
               case rcvPLXSOperandNotSupported:
                  printf("Operand Not Supported.\r\n");
                  break;
               default:
                  printf("Invalid.\r\n");
                  break;
            }
            break;
         default:
            /* Prevent compiler warnings.                               */
            break;
      }
   }
}

   /* The following function is responsible for displaying the PLX      */
   /* Features of the PLXS Server.                                      */
static void DisplayPLXFeatures(PLXS_Features_Data_t *Features)
{
   unsigned int DeviceandSensorStatus = 0;

   if(Features)
   {
      printf("\r\nPLX Features:\r\n");

      printf("\r\nPLXS Features:  0x%04X\r\n", (unsigned int)(Features->Support_Features));
      printf("   Measurement Status:              %s\r\n", (Features->Support_Features & (unsigned int)PLXS_FEATURES_MEASUREMENT_STATUS_SUPPORTED) ? "Supported" : "Not supported");
      printf("   Device and Sensor Status:        %s\r\n", (Features->Support_Features & (unsigned int)PLXS_FEATURES_DEVICE_AND_SENSOR_STATUS_SUPPORTED) ? "Supported" : "Not supported");
      printf("   Spot Check Measurement Storage:  %s\r\n", (Features->Support_Features & (unsigned int)PLXS_FEATURES_SPOT_CHECK_MEASUREMENT_STORAGE_SUPPORTED) ? "Supported" : "Not supported");
      printf("   Time Stamp:                      %s\r\n", (Features->Support_Features & (unsigned int)PLXS_FEATURES_TIME_STAMP_SUPPORTED) ? "Supported" : "Not supported");
      printf("   PO2/PR Fast:                     %s\r\n", (Features->Support_Features & (unsigned int)PLXS_FEATURES_SPO2PR_FAST_METRIC_SUPPORTED) ? "Supported" : "Not supported");
      printf("   PO2/PR Slow:                     %s\r\n", (Features->Support_Features & (unsigned int)PLXS_FEATURES_SPO2PR_SLOW_METRIC_SUPPORTED) ? "Supported" : "Not supported");
      printf("   Pulse Amplitude Index:           %s\r\n", (Features->Support_Features & (unsigned int)PLXS_FEATURES_PULSE_AMPLITUDE_INDEX_SUPPORTED) ? "Supported" : "Not supported");
      printf("   Multiple Bonds:                  %s\r\n", (Features->Support_Features & (unsigned int)PLXS_FEATURES_MULTIPLE_BONDS_SUPPORTED) ? "Supported" : "Not supported");

      printf("\r\nMeasurement Status:  0x%04X\r\n", (unsigned int)(Features->Measurement_Status_Support));
      printf("   Measurement Ongoing:              %s\r\n", (Features->Measurement_Status_Support & (unsigned int)PLXS_MSS_MEASUREMENT_ONGOING_SUPPORTED) ? "Supported" : "Not supported");
      printf("   Early Estimate Data:              %s\r\n", (Features->Measurement_Status_Support & (unsigned int)PLXS_MSS_EARLY_ESTIMATE_DATA_SUPPORTED) ? "Supported" : "Not supported");
      printf("   Validated Data:                   %s\r\n", (Features->Measurement_Status_Support & (unsigned int)PLXS_MSS_VALIDATED_DATA_SUPPORTED) ? "Supported" : "Not supported");
      printf("   Fully Qualified Data:             %s\r\n", (Features->Measurement_Status_Support & (unsigned int)PLXS_MSS_FULLY_QUALIFIED_DATA_SUPPORTED) ? "Supported" : "Not supported");
      printf("   Measurement Storage:              %s\r\n", (Features->Measurement_Status_Support & (unsigned int)PLXS_MSS_MEASUREMENT_STORAGE_SUPPORTED) ? "Supported" : "Not supported");
      printf("   Data for Demonstration:           %s\r\n", (Features->Measurement_Status_Support & (unsigned int)PLXS_MSS_DATA_FOR_DEMONSTRATION_SUPPORTED) ? "Supported" : "Not supported");
      printf("   Data for Testing:                 %s\r\n", (Features->Measurement_Status_Support & (unsigned int)PLXS_MSS_DATA_FOR_TESTING_SUPPORTED) ? "Supported" : "Not supported");
      printf("   Calibration Ongoing:              %s\r\n", (Features->Measurement_Status_Support & (unsigned int)PLXS_MSS_CALIBRATION_ONGOING_SUPPORTED) ? "Supported" : "Not supported");
      printf("   Measurement Unavailable           %s\r\n", (Features->Measurement_Status_Support & (unsigned int)PLXS_MSS_CALIBRATION_ONGOING_SUPPORTED) ? "Supported" : "Not supported");
      printf("   Questional Measurement Detected:  %s\r\n", (Features->Measurement_Status_Support & (unsigned int)PLXS_MSS_CALIBRATION_ONGOING_SUPPORTED) ? "Supported" : "Not supported");
      printf("   Invalid Measurement Detected:     %s\r\n", (Features->Measurement_Status_Support & (unsigned int)PLXS_MSS_CALIBRATION_ONGOING_SUPPORTED) ? "Supported" : "Not supported");

      /* Store the UINT24 structure in DWord.                           */
      DeviceandSensorStatus |= ((unsigned int)(Features->Device_And_Sensor_Status_Support.Lower));
      DeviceandSensorStatus |= (((unsigned int)(Features->Device_And_Sensor_Status_Support.Upper)) << 16);

      printf("\r\nDevice and Sensor Status:  0x%04X\r\n", (unsigned int)(DeviceandSensorStatus));
      printf("   Extended Display Update Ongoing:          %s\r\n", (DeviceandSensorStatus & (unsigned int)PLXS_DSSS_EXTENDED_DISPLAY_UPDATE_ONGOING_SUPPORTED) ? "Supported" : "Not supported");
      printf("   Equipment Malfunction Detected:           %s\r\n", (DeviceandSensorStatus & (unsigned int)PLXS_DSSS_EQUIPMENT_MALFUNCTION_DETECTED_SUPPORTED) ? "Supported" : "Not supported");
      printf("   Signal Processing Irregularity Detected:  %s\r\n", (DeviceandSensorStatus & (unsigned int)PLXS_DSSS_SIGNAL_PROCESSING_IRREGULARITY_DETECTED_SUPPORTED) ? "Supported" : "Not supported");
      printf("   Inadequite Signal Detected:               %s\r\n", (DeviceandSensorStatus & (unsigned int)PLXS_DSSS_INADEQUITE_SIGNAL_DETECTED_SUPPORTED) ? "Supported" : "Not supported");
      printf("   Poor Signal Detected:                     %s\r\n", (DeviceandSensorStatus & (unsigned int)PLXS_DSSS_POOR_SIGNAL_DETECTED_SUPPORTED) ? "Supported" : "Not supported");
      printf("   Low Perfusion Detected:                   %s\r\n", (DeviceandSensorStatus & (unsigned int)PLXS_DSSS_LOW_PERFUSION_DETECTED_SUPPORTED) ? "Supported" : "Not supported");
      printf("   Erratic Signal Detected:                  %s\r\n", (DeviceandSensorStatus & (unsigned int)PLXS_DSSS_ERRATIC_SIGNAL_DETECTED_SUPPORTED) ? "Supported" : "Not supported");
      printf("   Non Pulsatile Signal Detected:            %s\r\n", (DeviceandSensorStatus & (unsigned int)PLXS_DSSS_NON_PULSATILE_SIGNAL_DETECTED_SUPPORTED) ? "Supported" : "Not supported");
      printf("   Questionable Pulse Detected               %s\r\n", (DeviceandSensorStatus & (unsigned int)PLXS_DSSS_QUESTIONABLE_PULSE_DETECTED_SUPPORTED) ? "Supported" : "Not supported");
      printf("   Signal Analysis Ongoing:                  %s\r\n", (DeviceandSensorStatus & (unsigned int)PLXS_DSSS_SIGNAL_ANALYSIS_ONGOING_SUPPORTED) ? "Supported" : "Not supported");
      printf("   Sensor Interface Detected:                %s\r\n", (DeviceandSensorStatus & (unsigned int)PLXS_DSSS_SENSOR_INTERFACE_DETECTED_SUPPORTED) ? "Supported" : "Not supported");
      printf("   Sesnor Unconnected to User:               %s\r\n", (DeviceandSensorStatus & (unsigned int)PLXS_DSSS_SENSOR_UNCONNECTED_TO_USER_SUPPORTED) ? "Supported" : "Not supported");
      printf("   Unknown Sensor Connected:                 %s\r\n", (DeviceandSensorStatus & (unsigned int)PLXS_DSSS_UNKNOWN_SENSOR_CONNECTED_SUPPORTED) ? "Supported" : "Not supported");
      printf("   Sensor Displacement:                      %s\r\n", (DeviceandSensorStatus & (unsigned int)PLXS_DSSS_SENSOR_DISPLACEMENT_SUPPORTED) ? "Supported" : "Not supported");
      printf("   Sensor Malfunction:                       %s\r\n", (DeviceandSensorStatus & (unsigned int)PLXS_DSSS_SENSOR_MALFUNCTIONING_SUPPORTED) ? "Supported" : "Not supported");
      printf("   Sensor Disconnected:                      %s\r\n", (DeviceandSensorStatus & (unsigned int)PLXS_DSSS_SENSOR_DISCONNECTED_SUPPORTED) ? "Supported" : "Not supported");
   }
}

   /* The following function is responsible for displaying a Spot Check */
   /* Measurement.                                                      */
static void DisplaySpotCheckMeasurement(PLXS_Spot_Check_Measurement_Data_t *Measurement)
{
   printf("\r\nSpot Check Measurement:\r\n");

   if(Measurement)
   {
      printf("   Flags:                  0x%02X\r\n", (unsigned int)(Measurement->Flags));
      printf("   SpO2:                   0x%04X\r\n", (unsigned int)(Measurement->SpO2));
      printf("   PR:                     0x%04X\r\n", (unsigned int)(Measurement->PR));
      printf("   Timestamp:\r\n");
      printf("      Year:                %u\r\n", (unsigned int)(Measurement->Timestamp.Year));
      printf("      Month:               %u\r\n", (unsigned int)(Measurement->Timestamp.Month));
      printf("      Day:                 %u\r\n", (unsigned int)(Measurement->Timestamp.Day));
      printf("      Hours:               %u\r\n", (unsigned int)(Measurement->Timestamp.Hours));
      printf("      Minutes:             %u\r\n", (unsigned int)(Measurement->Timestamp.Minutes));
      printf("      Seconds:             %u\r\n", (unsigned int)(Measurement->Timestamp.Seconds));
      printf("   Status:\r\n");
      printf("   Measurement:            0x%04X\r\n", (unsigned int)(Measurement->Measurement_Status));
      printf("   Device and Sensor:\r\n");
      printf("      Lower:               0x%04X\r\n", (unsigned int)(Measurement->Device_And_Sensor_Status.Lower));
      printf("      Upper:               0x%02X\r\n", (unsigned int)(Measurement->Device_And_Sensor_Status.Upper));
      printf("   Pulse Amplitude Index:  0x%04X\r\n", (unsigned int)(Measurement->Pulse_Amplitude_Index));
   }
}

   /* The following function is responsible for displaying a Spot Check */
   /* Measurement.                                                      */
static void DisplayContinuousMeasurement(PLXS_Continuous_Measurement_Data_t *Measurement)
{
   printf("\r\nContinuous Measurement:\r\n");

   if(Measurement)
   {
      printf("   Flags:                  0x%02X\r\n", (unsigned int)(Measurement->Flags));
      printf("   SpO2 Normal:            0x%04X\r\n", (unsigned int)(Measurement->SpO2_Normal));
      printf("   PR Normal:              0x%04X\r\n", (unsigned int)(Measurement->PR_Normal));
      printf("   SpO2 Fast:              0x%04X\r\n", (unsigned int)(Measurement->SpO2_Fast));
      printf("   PR Fast:                0x%04X\r\n", (unsigned int)(Measurement->PR_Fast));
      printf("   SpO2 Slow:              0x%04X\r\n", (unsigned int)(Measurement->SpO2_Slow));
      printf("   PR Slow:                0x%04X\r\n", (unsigned int)(Measurement->PR_Slow));
      printf("   Status:\r\n");
      printf("   Measurement:            0x%04X\r\n", (unsigned int)(Measurement->Measurement_Status));
      printf("   Device and Sensor:\r\n");
      printf("      Lower:               0x%04X\r\n", (unsigned int)(Measurement->Device_And_Sensor_Status.Lower));
      printf("      Upper:               0x%02X\r\n", (unsigned int)(Measurement->Device_And_Sensor_Status.Upper));
      printf("   Pulse Amplitude Index:  0x%04X\r\n", (unsigned int)(Measurement->Pulse_Amplitude_Index));
   }
}

   /* The following command is responsible for registering PLXS.        */
static int RegisterPLXSCommand(ParameterList_t *TempParam)
{
   /* Simply call and return the internal function.                     */
   return(RegisterPLXS());
}

   /* The following command is responsible for un-registering PLXS.     */
static int UnregisterPLXSCommand(ParameterList_t *TempParam)
{
   /* Simply call and return the internal function.                     */
   return(UnregisterPLXS());
}

   /* The following command is responsible for discovering attribute    */
   /* handles for PLXS on a remote PLXP Server.                         */
static int DiscoverPLXSCommand(ParameterList_t *TempParam)
{
   int           ret_val = 0;
   DeviceInfo_t *DeviceInfo;
   GATT_UUID_t   UUID;

   /* Verify that we are not configured as the PLXP Server              */
   if(!PLXSInstanceID)
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
               /* Configure the filter so that only the PLXS Service is */
               /* discovered.                                           */
               UUID.UUID_Type = guUUID_16;
               PLXS_ASSIGN_PLXS_SERVICE_UUID_16(&(UUID.UUID.UUID_16));

               /* Start the service discovery process.                  */
               ret_val = GATT_Start_Service_Discovery(BluetoothStackID, ConnectionID, (sizeof(UUID)/sizeof(GATT_UUID_t)), &UUID, GATT_Service_Discovery_Event_Callback, sdPLXS);
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
               printf("\r\nA service discovery operation is already in progress.\r\n");
         }
         else
            printf("\r\nUnknown PLXP Server.\r\n");
      }
      else
         printf("\r\nMust be connected to an PLXP Server.\r\n");
   }
   else
      printf("\r\nOnly a PLXP Client can discover PLXS.\r\n");

   return(ret_val);
}

   /* The following command is responsible for starting Continuous      */
   /* Measurement notifications.                                        */
static int StartNotificationsCommand(ParameterList_t *TempParam)
{
   Byte_t        Flags;
   Word_t        Measurement_Status;
   unsigned int  Device_And_Sensor_Status;
   unsigned long Timeout;

   /* Make sure the parameters are semi-valid.                          */
   if((TempParam) && (TempParam->NumberofParameters >= 4))
   {
      /* Store the parameters.                                          */
      Flags                    = (Byte_t)TempParam->Params[0].intParam;
      Measurement_Status       = (Word_t)TempParam->Params[1].intParam;
      Device_And_Sensor_Status = (unsigned int)TempParam->Params[2].intParam;
      Timeout                  = (unsigned long)TempParam->Params[3].intParam;

      /* Simply call the internal function to start sending             */
      /* notifications.                                                 */
      StartContinuousMeasurementNotifications(Flags, Measurement_Status, Device_And_Sensor_Status, Timeout);
   }
   else
   {
      /* Display the usage.                                             */
      printf("Usage: Start [Flags(UINT8)] [Measurement Status(UINT16)] [Device and Sensor Status(UINT16)] [Timeout (milli-seconds)].\r\n");

      printf("\r\nWhere Flags (Bitmask) is:\r\n");
      printf("   0x01 = Fast Field Present.\r\n");
      printf("   0x02 = Slow Field Present.\r\n");
      printf("   0x04 = Measurement Status Present.\r\n");
      printf("   0x08 = Device and Sensor Status Present.\r\n");
      printf("   0x10 = Pulse Amplitude Index Present.\r\n");

      printf("\r\nWhere Measurement Status (Bitmask) is:\r\n");
      printf("   0x0020 = Measurement Ongoing.\r\n");
      printf("   0x0040 = Early Estimate Data.\r\n");
      printf("   0x0080 = Validated Data.\r\n");
      printf("   0x0100 = Fully Qualified Data.\r\n");
      printf("   0x0200 = Data From Storage.\r\n");
      printf("   0x0400 = Data for Demonstration.\r\n");
      printf("   0x0800 = Data for Testing.\r\n");
      printf("   0x1000 = Calibration Ongoing.\r\n");
      printf("   0x2000 = Measurement Unavailabled.\r\n");
      printf("   0x4000 = Questionable Measurement detected.\r\n");
      printf("   0x8000 = Invalid Measurement detected.\r\n");

      printf("\r\nWhere Device and Sensor Status (Bitmask) is:\r\n");
      printf("   0x0001 = Extended Display Update Ongoing.\r\n");
      printf("   0x0002 = Equipment Malfunction detected.\r\n");
      printf("   0x0004 = Signal Process Irregulatory detected.\r\n");
      printf("   0x0008 = Inadequite Signal detected.\r\n");
      printf("   0x0010 = Poor Signal detected.\r\n");
      printf("   0x0020 = Low Perfusion detected.\r\n");
      printf("   0x0040 = Erratic Sign detected.\r\n");
      printf("   0x0080 = NonPulsatile Signal detected.\r\n");
      printf("   0x0100 = Questionable Signal detected.\r\n");
      printf("   0x0200 = Signal Analysis Ongoing.\r\n");
      printf("   0x0400 = Sensor Interface Detected.\r\n");
      printf("   0x0800 = Sensor Unconnected to User.\r\n");
      printf("   0x1000 = Unknown Sensor Connected.\r\n");
      printf("   0x2000 = Sensor Displaced.\r\n");
      printf("   0x4000 = Sensor Malfunctioning.\r\n");
      printf("   0x8000 = Sensor Disconnected.\r\n");
   }

   return(0);
}

   /* The following command is responsible for stopping Continuous      */
   /* Measurement notifications.                                        */
static int StopNotificationsCommand(ParameterList_t *TempParam)
{
   /* Simply call the internal function.                                */
   StopContinuousMeasurementNotifications();
   return(0);
}

   /* The following command is responsible for creating a new Spot Check*/
   /* Measurement.                                                      */
static int CreateSpotCheckMeasurementCommand(ParameterList_t *TempParam)
{
   Byte_t       Flags;
   Word_t       Measurement_Status;
   unsigned int Device_And_Sensor_Status;

   /* Make sure the parameters are semi-valid.                          */
   if((TempParam) && (TempParam->NumberofParameters >= 3))
   {
      /* Store the parameters.                                          */
      Flags                    = (Byte_t)TempParam->Params[0].intParam;
      Measurement_Status       = (Word_t)TempParam->Params[1].intParam;
      Device_And_Sensor_Status = (unsigned int)TempParam->Params[2].intParam;

      /* Simply call the internal function to start sending             */
      /* notifications.                                                 */
      CreateSpotCheckMeasurement(Flags, Measurement_Status, Device_And_Sensor_Status);
   }
   else
   {
      /* Display the usage.                                             */
      printf("Usage: Create [Flags(UINT8)] [Measurement Status(UINT16)] [Device and Sensor Status(UINT16)].\r\n");

      printf("\r\nWhere Flags (Bitmask) is:\r\n");
      printf("   0x01 = Time Stamp Present.\r\n");
      printf("   0x02 = Measurement Status Present.\r\n");
      printf("   0x04 = Device and Sensor Status Present.\r\n");
      printf("   0x08 = Pulse Amplitude Index Present.\r\n");
      printf("   0x10 = Device Clock not set.\r\n");

      printf("\r\nWhere Measurement Status (Bitmask) is:\r\n");
      printf("   0x0020 = Measurement Ongoing.\r\n");
      printf("   0x0040 = Early Estimate Data.\r\n");
      printf("   0x0080 = Validated Data.\r\n");
      printf("   0x0100 = Fully Qualified Data.\r\n");
      printf("   0x0200 = Data From Storage.\r\n");
      printf("   0x0400 = Data for Demonstration.\r\n");
      printf("   0x0800 = Data for Testing.\r\n");
      printf("   0x1000 = Calibration Ongoing.\r\n");
      printf("   0x2000 = Measurement Unavailabled.\r\n");
      printf("   0x4000 = Questionable Measurement detected.\r\n");
      printf("   0x8000 = Invalid Measurement detected.\r\n");

      printf("\r\nWhere Device and Sensor Status (Bitmask) is:\r\n");
      printf("   0x0001 = Extended Display Update Ongoing.\r\n");
      printf("   0x0002 = Equipment Malfunction detected.\r\n");
      printf("   0x0004 = Signal Process Irregulatory detected.\r\n");
      printf("   0x0008 = Inadequite Signal detected.\r\n");
      printf("   0x0010 = Poor Signal detected.\r\n");
      printf("   0x0020 = Low Perfusion detected.\r\n");
      printf("   0x0040 = Erratic Sign detected.\r\n");
      printf("   0x0080 = NonPulsatile Signal detected.\r\n");
      printf("   0x0100 = Questionable Signal detected.\r\n");
      printf("   0x0200 = Signal Analysis Ongoing.\r\n");
      printf("   0x0400 = Sensor Interface Detected.\r\n");
      printf("   0x0800 = Sensor Unconnected to User.\r\n");
      printf("   0x1000 = Unknown Sensor Connected.\r\n");
      printf("   0x2000 = Sensor Displaced.\r\n");
      printf("   0x4000 = Sensor Malfunctioning.\r\n");
      printf("   0x8000 = Sensor Disconnected.\r\n");
   }

   return(0);
}

   /* The following command is responsible for indicating Spot Check    */
   /* Measurements.                                                     */
static int IndicateSpotCheckMeasurementCommand(ParameterList_t *TempParam)
{
   /* Simply call the internal function.                                */
   IndicateSpotCheckMeasurement();
   return(0);
}

   /* The following command is responsible for printing the Spot Check  */
   /* Measurement List.                                                 */
static int PrintSpotCheckMeasurementListCommand(ParameterList_t *TempParam)
{
   /* Simply call the internal function.                                */
   DisplaySpotCheckMeasurementsList();
   return(0);
}

   /* The following command is responsible for reading a PLXS           */
   /* Characteristic.                                                   */
static int ReadCharacteristicCommand(ParameterList_t *TempParam)
{
   /* Make sure the parameters are semi-valid.                          */
   if((TempParam) && (TempParam->NumberofParameters >= 1))
   {
      PLXSRequestData.AttributeHandleType = (PLXP_Attribute_Handle_Type_t)TempParam->Params[0].intParam;

      /* Simply call the internal function.                             */
      ReadPLXSCharacteristic();
   }
   else
   {
      /* Display the usage.                                             */
      printf("\r\nUsage: Read [Attribute Handle Type (UINT8)]\r\n");
      printf("\r\nWhere Attribute Handle Type is:\r\n");
      printf("   0 = PLXS Features\r\n");
      printf("   1 = Spot Check Measurement (Not Readable)\r\n");
      printf("   2 = Spot Check Measurement CCCD\r\n");
      printf("   3 = Continuous Measurement (Not Readable)\r\n");
      printf("   4 = Continuous Measurement CCCD\r\n");
      printf("   5 = Record Access Control Point (Not Readable)\r\n");
      printf("   6 = Record Access Control Point CCCD\r\n");
      printf("   7 = Service Changed CCCD\r\n");
   }

   return(0);
}

   /* The following command is responsible for writing a PLXS           */
   /* Characteristic CCCD.                                              */
static int WriteCCCDCommand(ParameterList_t *TempParam)
{
   /* Make sure the parameters are semi-valid.                          */
   if((TempParam) && (TempParam->NumberofParameters >= 2))
   {
      /* Store the attribute handle type based on the PLXS CCCD         */
      /* Characteristic type.                                           */
      switch(TempParam->Params[0].intParam)
      {
         case pcdSpotCheck:
            /* Store the attribute handle type.                         */
            PLXSRequestData.AttributeHandleType = ahtSpot_Check_Measurement_CCCD;
            break;
         case pcdContinuous:
            /* Store the attribute handle type.                         */
            PLXSRequestData.AttributeHandleType = ahtContinuous_Measurement_CCCD;
            break;
         case pcdRACP:
            /* Store the attribute handle type.                         */
            PLXSRequestData.AttributeHandleType = ahtRecordAccessControlPoint_CCCD;
            break;
         case pcdServiceChanged:
            /* Store the attribute handle type.                         */
            PLXSRequestData.AttributeHandleType = ahtServiceChanged_CCCD;
            break;
         default:
            printf("\r\nInvalid PLXS CCCD Characteristic type.\r\n");
            break;
      }

      /* Store the CCCD.                                                */
      if(TempParam->Params[1].intParam == 0)
      {
         PLXSRequestData.Data.Configuration = 0;
      }
      else
      {
         if(TempParam->Params[1].intParam == 1)
         {
            PLXSRequestData.Data.Configuration = (Word_t)PLXS_CLIENT_CHARACTERISTIC_CONFIGURATION_INDICATE_ENABLE;
         }
         else
         {
            PLXSRequestData.Data.Configuration = (Word_t)PLXS_CLIENT_CHARACTERISTIC_CONFIGURATION_NOTIFY_ENABLE;
         }
      }

      /* Simply call the internal function to handle the write request. */
      WritePLXSCharacteristic();
   }
   else
   {
      /* Display the usage.                                             */
      printf("\r\nUsage: WriteCCCD [Type (UINT8)] [Opt. params (below)]\r\n");
      printf("\r\nWhere Type is:\r\n");
      printf("   0 = Spot Check Measurement CCCD [Configuration (0 = disable, 1 = indicate, 2 = notify)].\r\n");
      printf("   1 = Continuous Measurement CCCD [Configuration (0 = disable, 1 = indicate, 2 = notify)].\r\n");
      printf("   2 = Record Access Control Point CCCD [Configuration (0 = disable, 1 = indicate, 2 = notify)].\r\n");
   }

   return(0);
}

   /* The following command is responsible for writing the Record Access*/
   /* Control Point.                                                    */
static int WriteRACPCommand(ParameterList_t *TempParam)
{
   /* Make sure the parameters are semi-valid.                          */
   if((TempParam) && (TempParam->NumberofParameters >= 2))
   {
      /* Set the attribute handle type.                                 */
      PLXSRequestData.AttributeHandleType = ahtRecordAccessControlPoint;

      PLXSRequestData.Data.RACPRequestData.Op_Code  = (PLXS_RACP_Request_Type_t)TempParam->Params[0].intParam;
      PLXSRequestData.Data.RACPRequestData.Operator = (PLXS_RACP_Operator_Type_t)TempParam->Params[1].intParam;

      /* Simply call the internal function to handle the write request. */
      WritePLXSCharacteristic();
   }
   else
   {
      /* Display the usage.                                             */
      printf("\r\nUsage: WriteRACP [Op Code (UINT8)] [Operator (UINT8)]\r\n");

      printf("\r\nWhere Op Code is:\r\n");
      printf("   0x01 = Report Stored Records\r\n");
      printf("   0x02 = Delete Stored Records Request\r\n");
      printf("   0x03 = Abort Operation Request\r\n");
      printf("   0x04 = Number of Stored RecordsRequest.\r\n");

      printf("\r\nWhere Operator is:\r\n");
      printf("   0x00 = NULL\r\n");
      printf("   0x01 = All Records\r\n");
      printf("   0x02 = Less than or Equal to\r\n");
      printf("   0x03 = Greater than or Equal to\r\n");
      printf("   0x04 = Within Range of\r\n");
      printf("   0x05 = First Record\r\n");
      printf("   0x06 = Last Records\r\n");
   }

   return(0);
}

   /* ***************************************************************** */
   /*                         Event Callbacks                           */
   /* ***************************************************************** */

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
      printf("\r\nGAP Callback Data: Event_Data = NULL.\r\n");

      DisplayPrompt();
   }

   /* Make sure the output is displayed to the user.                    */
   fflush(stdout);
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

   /* The following is a PLXP Server Event Callback.  This function will*/
   /* be called whenever an PLXP Server event occurs that is associated */
   /* with the specified Bluetooth Stack ID.  This function passes to   */
   /* the caller the Bluetooth Stack ID, the PLXS Event Data that       */
   /* occurred and the PLXS Event Callback Parameter that was specified */
   /* when this Callback was installed.  The caller is free to use the  */
   /* contents of the PLXS Event Data ONLY in the context If the caller */
   /* requires the Data for a longer period of of this callback.  time, */
   /* then the callback function MUST copy the data into another Data   */
   /* Buffer This function is guaranteed NOT to be invoked more than    */
   /* once simultaneously for the specified this function DOES NOT have */
   /* be re-entrant).  It needs to be installed callback (i.e.  noted   */
   /* however, that if the same Callback is installed more than once,   */
   /* then the callbacks will Because of be called serially.  this, the */
   /* processing in this function should be as efficient as possible.   */
   /* It should also be noted that this function is called in the Thread*/
   /* Context of a Thread that the User does NOT own.  Therefore,       */
   /* processing in this function should be as efficient as possible    */
   /* (this argument holds anyway because another PLXS Event will not be*/
   /* processed while this function call is outstanding).               */
   /* ** NOTE ** This function MUST NOT Block and wait for events that  */
   /*            can only be satisfied by Receiving PLXS Event Packets. */
   /*            A Deadlock WILL occur because NO PLXS Event Callbacks  */
   /*            will be issued while this function is currently        */
   /*            outstanding.                                           */
static void BTPSAPI PLXS_EventCallback(unsigned int BluetoothStackID, PLXS_Event_Data_t *PLXS_Event_Data, unsigned long CallbackParameter)
{
   int                        Result;
   BoardStr_t                 BoardStr;
   DeviceInfo_t              *DeviceInfo;

   unsigned int               InstanceID;
   unsigned int               ConnectionID;
   unsigned int               TransactionID;
   BD_ADDR_t                  RemoteDevice;
   GATT_Connection_Type_t     ConnectionType;

   Word_t                     Configuration;
   Byte_t                     Status;
   Word_t                     BytesWritten;

   Byte_t                     ErrorCode;
   PLXS_CCCD_Type_t           CCCDType;
   PLXS_RACP_Request_Data_t   RACPRequestData;
   PLXS_RACP_Response_Data_t  RACPResponseData;

   unsigned int               Index;
   Boolean_t                  SendRACPResponse;

   /* Verify that all parameters to this callback are Semi-Valid.       */
   if((BluetoothStackID) && (PLXS_Event_Data))
   {
      /* Switch through the event type.                                 */
      switch(PLXS_Event_Data->Event_Data_Type)
      {
         case etPLXS_Server_Read_Features_Request:
            printf("\r\netPLXS_Server_Read_Features_Request with size %u.\r\n", PLXS_Event_Data->Event_Data_Size);
            if(PLXS_Event_Data->Event_Data.PLXS_Read_Features_Request_Data)
            {
               /* Store event information.                              */
               InstanceID     = PLXS_Event_Data->Event_Data.PLXS_Read_Features_Request_Data->InstanceID;
               ConnectionID   = PLXS_Event_Data->Event_Data.PLXS_Read_Features_Request_Data->ConnectionID;
               ConnectionType = PLXS_Event_Data->Event_Data.PLXS_Read_Features_Request_Data->ConnectionType;
               TransactionID  = PLXS_Event_Data->Event_Data.PLXS_Read_Features_Request_Data->TransactionID;
               RemoteDevice   = PLXS_Event_Data->Event_Data.PLXS_Read_Features_Request_Data->RemoteDevice;

               /* Print event information.                              */
               printf("   Instance ID:      %u.\r\n", InstanceID);
               printf("   Connection ID:    %u.\r\n", ConnectionID);
               printf("   Connection Type:  %s.\r\n", ((ConnectionType == gctLE) ? "LE" : "BR/EDR"));
               printf("   Transaction ID:   %u.\r\n", TransactionID);
               BD_ADDRToStr(RemoteDevice, BoardStr);
               printf("   Remote Device:    %s.\r\n", BoardStr);

               /* Send the response.                                    */
               if((Result = PLXS_Read_Features_Request_Response(BluetoothStackID, InstanceID, TransactionID, (Byte_t)PLXS_ERROR_CODE_SUCCESS, &PLX_Feature)) != 0)
               {
                  DisplayFunctionError("PLXS_Read_Features_Request_Response", Result);
               }
            }
            else
               printf("\r\nInvalid Event pointer.\r\n");
            break;
         case etPLXS_Server_Write_RACP_Request:
            printf("\r\netPLXS_Server_Write_RACP_Request with size %u.\r\n", PLXS_Event_Data->Event_Data_Size);
            if(PLXS_Event_Data->Event_Data.PLXS_Write_RACP_Request_Data)
            {
               /* Store event information.                              */
               InstanceID      = PLXS_Event_Data->Event_Data.PLXS_Write_RACP_Request_Data->InstanceID;
               ConnectionID    = PLXS_Event_Data->Event_Data.PLXS_Write_RACP_Request_Data->ConnectionID;
               ConnectionType  = PLXS_Event_Data->Event_Data.PLXS_Write_RACP_Request_Data->ConnectionType;
               TransactionID   = PLXS_Event_Data->Event_Data.PLXS_Write_RACP_Request_Data->TransactionID;
               RemoteDevice    = PLXS_Event_Data->Event_Data.PLXS_Write_RACP_Request_Data->RemoteDevice;
               RACPRequestData = PLXS_Event_Data->Event_Data.PLXS_Write_RACP_Request_Data->RequestData;

               /* Print event information.                              */
               printf("   Instance ID:      %u.\r\n", InstanceID);
               printf("   Connection ID:    %u.\r\n", ConnectionID);
               printf("   Connection Type:  %s.\r\n", ((ConnectionType == gctLE) ? "LE" : "BR/EDR"));
               printf("   Transaction ID:   %u.\r\n", TransactionID);
               BD_ADDRToStr(RemoteDevice, BoardStr);
               printf("   Remote Device:    %s.\r\n", BoardStr);

               /* Display the RACP Request Data.                        */
               DisplayRACPRequestData(&RACPRequestData);

               /* Get the device information for the remote device.     */
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, RemoteDevice)) != NULL)
               {
                  /* Make sure a procedure is not already in progress.  */
                  if(!(DeviceInfo->Flags & DEVICE_INFO_FLAGS_RACP_PROCEDURE_IN_PROGRESS))
                  {
                     /* Flag that a RACP Procedure is in progress.      */
                     DeviceInfo->Flags |= ((Word_t)DEVICE_INFO_FLAGS_RACP_PROCEDURE_IN_PROGRESS);

                     /* Make sure the RACP CCCD has been configured for */
                     /* indications.                                    */
                     if(DeviceInfo->PLXSServerInfo.Record_Access_Control_Point_CCCD == PLXS_CLIENT_CHARACTERISTIC_CONFIGURATION_INDICATE_ENABLE)
                     {
                        /* Send the GATT Write response to Acknowledge  */
                        /* that we have received the request.           */
                        if((Result = PLXS_RACP_Request_Response(BluetoothStackID, InstanceID, TransactionID, (Byte_t)PLXS_ERROR_CODE_SUCCESS)) != 0)
                        {
                           DisplayFunctionError("PLXS_RACP_Request_Response", Result);
                        }

                        /* We will go ahead and store the Response      */
                        /* Request Op Code in case it is needed.        */
                        RACPResponseData.Request_Op_Code = RACPRequestData.Op_Code;

                        /* Handle the request based on the Op Code.     */
                        switch(RACPRequestData.Op_Code)
                        {
                           case rrtReportStoredRecordsRequest:
                              /* * NOTE * We will assume that           */
                              /*          indications have been enabled */
                              /*          for the Spot Check            */
                              /*          Measurements.  However, we    */
                              /*          could check this here if      */
                              /*          necessary.                    */

                              /* Simply call the internal function.     */
                              ReportStoredRecordsProcedure(&RACPRequestData, &RACPResponseData);
                              break;
                           case rrtDeleteStoredRecordsRequest:
                              DeleteStoredRecordsProcedure(&RACPRequestData, &RACPResponseData);
                              break;
                           case rrtAbortOperationRequest:
                              printf("\r\nAbort Successful.\r\n");
                              RACPResponseData.Response_Op_Code      = rrotResponseOpCode;
                              RACPResponseData.Operator              = rotNull;
                              RACPResponseData.Operand.Response_Code = rcvPLXSSuccess;
                              break;
                           case rrtNumberOfStoredRecordsRequest:
                              NumberOfStoredRecordsProcedure(&RACPRequestData, &RACPResponseData);
                              break;
                           default:
                              /* We will simply indicate that an error  */
                              /* occured and that the Op Code is not    */
                              /* supported.                             */
                              /* * NOTE * We will return the Op Code and*/
                              /*          Operator received from the    */
                              /*          PLXP Client.                  */
                              RACPResponseData.Response_Op_Code      = rrotResponseOpCode;
                              RACPResponseData.Operator              = rotNull;
                              RACPResponseData.Operand.Response_Code = rcvPLXSOpCodeNotSupported;
                              break;
                        }

                        /* Display the RACP Response Data.              */
                        DisplayRACPResponseData(&RACPResponseData);

                        /* If the Report Stored Records Procedure is    */
                        /* successful, then we CANNOT indicate the RACP */
                        /* Response until all indications have been     */
                        /* sent.                                        */
                        if((RACPResponseData.Request_Op_Code != (PLXS_RACP_Request_Type_t)rrtReportStoredRecordsRequest) || ((RACPResponseData.Request_Op_Code == (PLXS_RACP_Request_Type_t)rrtReportStoredRecordsRequest) && (RACPResponseData.Operand.Response_Code != (PLXS_RACP_Response_Code_Value_t)rcvPLXSSuccess)))
                        {
                           /* Flag that a RACP Procedure is not         */
                           /* progress.                                 */
                           DeviceInfo->Flags &= ~((Word_t)DEVICE_INFO_FLAGS_RACP_PROCEDURE_IN_PROGRESS);

                           /* Send the indication for the result of the */
                           /* RACP Procedure.                           */
                           if((Result = PLXS_Indicate_RACP_Response(BluetoothStackID, InstanceID, ConnectionID, &RACPResponseData)) > 0)
                           {
                              printf("\r\nRACP Response indicated.\r\n");
                              printf("   TransactionID:   %d\r\n", Result);
                           }
                           else
                              DisplayFunctionError("PLXS_Indicate_RACP_Response", Result);
                        }
                        else
                        {
                           /* We MUST have at least one Spot Check      */
                           /* Measurement to indicate.                  */
                           for(Index = 0; Index < PLXP_MAXIMUM_NUMBER_OF_SPOT_CHECK_MEASUREMENTS; Index++)
                           {
                              /* Make sure the Spot Check Measurement is*/
                              /* valid and has not been indicated.      */
                              if((SpotCheckMeasurementList[Index].Valid) && (SpotCheckMeasurementList[Index].Indicated == FALSE))
                              {
                                 /* Flag that this Spot Check           */
                                 /* Measurement has been indicated.     */
                                 SpotCheckMeasurementList[Index].Indicated = TRUE;

                                 /* Send the indication.                */
                                 if((Result = PLXS_Indicate_Spot_Check_Measurement(BluetoothStackID, PLXSInstanceID, ConnectionID, &(SpotCheckMeasurementList[Index].Measurement))) > 0)
                                 {
                                    printf("\r\nSpot Check Measurement indicated.\r\n");
                                    printf("   Measurement ID:  %u\r\n", SpotCheckMeasurementList[Index].ID);
                                    printf("   TransactionID:   %d\r\n", Result);
                                 }
                                 else
                                    DisplayFunctionError("PLXS_Indicate_Spot_Check_Measurement", Result);

                                 /* We CANNOT indicate another Spot     */
                                 /* Check Measurement until we receive  */
                                 /* the confirmation for the first      */
                                 /* indication.                         */
                                 /* * NOTE * We will also check if we   */
                                 /*          are done when the          */
                                 /*          confirmation is received   */
                                 /*          and send the RACP Response */
                                 /*          indication.                */
                                 break;
                              }
                           }
                        }
                     }
                     else
                     {
                        /* Send the GATT Write response for the error.  */
                        if((Result = PLXS_RACP_Request_Response(BluetoothStackID, InstanceID, TransactionID, (Byte_t)PLXS_ERROR_CODE_CCCD_IMPROPERLY_CONFIGURED)) != 0)
                        {
                           DisplayFunctionError("PLXS_RACP_Request_Response", Result);
                        }
                     }
                  }
                  else
                  {
                     /* Send the GATT Write response for the error.     */
                     if((Result = PLXS_RACP_Request_Response(BluetoothStackID, InstanceID, TransactionID, (Byte_t)PLXS_ERROR_CODE_PROCEDURE_ALREADY_IN_PROGRESS)) != 0)
                     {
                        DisplayFunctionError("PLXS_RACP_Request_Response", Result);
                     }
                  }
               }
               else
                  printf("\r\nNo device information for remote device.\r\n");
            }
            else
               printf("\r\nInvalid Event pointer.\r\n");
            break;
         case etPLXS_Server_Read_CCCD_Request:
            printf("\r\netPLXS_Server_Read_CCCD_Request with size %u.\r\n", PLXS_Event_Data->Event_Data_Size);
            if(PLXS_Event_Data->Event_Data.PLXS_Read_CCCD_Request_Data)
            {
               /* Store event information.                              */
               InstanceID     = PLXS_Event_Data->Event_Data.PLXS_Read_CCCD_Request_Data->InstanceID;
               ConnectionID   = PLXS_Event_Data->Event_Data.PLXS_Read_CCCD_Request_Data->ConnectionID;
               ConnectionType = PLXS_Event_Data->Event_Data.PLXS_Read_CCCD_Request_Data->ConnectionType;
               TransactionID  = PLXS_Event_Data->Event_Data.PLXS_Read_CCCD_Request_Data->TransactionID;
               RemoteDevice   = PLXS_Event_Data->Event_Data.PLXS_Read_CCCD_Request_Data->RemoteDevice;
               CCCDType       = PLXS_Event_Data->Event_Data.PLXS_Read_CCCD_Request_Data->Type;

               /* Print event information.                              */
               printf("   Instance ID:      %u.\r\n", InstanceID);
               printf("   Connection ID:    %u.\r\n", ConnectionID);
               printf("   Connection Type:  %s.\r\n", ((ConnectionType == gctLE) ? "LE" : "BR/EDR"));
               printf("   Transaction ID:   %u.\r\n", TransactionID);
               BD_ADDRToStr(RemoteDevice, BoardStr);
               printf("   Remote Device:    %s.\r\n", BoardStr);
               DisplayCCCDType(CCCDType);

               /* Get the device information for the remote device.     */
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, RemoteDevice)) != NULL)
               {
                  switch(CCCDType)
                  {
                     case pcdSpotCheck:
                        /* Send the response.                           */
                        if((Result = PLXS_Read_CCCD_Request_Response(BluetoothStackID, InstanceID, TransactionID, (Byte_t)PLXS_ERROR_CODE_SUCCESS, CCCDType, DeviceInfo->PLXSServerInfo.Spot_Check_Measurement_CCCD)) != 0)
                        {
                           DisplayFunctionError("PLXS_Read_CCCD_Request_Response", Result);
                        }
                        break;
                     case pcdContinuous:
                        /* Send the response.                           */
                     if((Result = PLXS_Read_CCCD_Request_Response(BluetoothStackID, InstanceID, TransactionID, (Byte_t)PLXS_ERROR_CODE_SUCCESS, CCCDType, DeviceInfo->PLXSServerInfo.Continuous_Measurement_CCCD)) != 0)
                        {
                           DisplayFunctionError("PLXS_Read_CCCD_Request_Response", Result);
                        }
                        break;
                     case pcdRACP:
                        /* Send the response.                           */
                     if((Result = PLXS_Read_CCCD_Request_Response(BluetoothStackID, InstanceID, TransactionID, (Byte_t)PLXS_ERROR_CODE_SUCCESS, CCCDType, DeviceInfo->PLXSServerInfo.Record_Access_Control_Point_CCCD)) != 0)
                        {
                           DisplayFunctionError("PLXS_Read_CCCD_Request_Response", Result);
                        }
                        break;
                     default:
                        /* This should NEVER occur since this type is   */
                        /* assigned by the service.                     */
                        printf("\r\nInvalid CCCD Type.\r\n");
                        break;
                  }
               }
               else
                  printf("\r\nNo device information for remote device.\r\n");
            }
            else
               printf("\r\nInvalid Event pointer.\r\n");
            break;
         case etPLXS_Server_Write_CCCD_Request:
            printf("\r\netPLXS_Server_Write_CCCD_Request with size %u.\r\n", PLXS_Event_Data->Event_Data_Size);
            if(PLXS_Event_Data->Event_Data.PLXS_Write_CCCD_Request_Data)
            {
               /* Store event information.                              */
               InstanceID     = PLXS_Event_Data->Event_Data.PLXS_Write_CCCD_Request_Data->InstanceID;
               ConnectionID   = PLXS_Event_Data->Event_Data.PLXS_Write_CCCD_Request_Data->ConnectionID;
               ConnectionType = PLXS_Event_Data->Event_Data.PLXS_Write_CCCD_Request_Data->ConnectionType;
               TransactionID  = PLXS_Event_Data->Event_Data.PLXS_Write_CCCD_Request_Data->TransactionID;
               RemoteDevice   = PLXS_Event_Data->Event_Data.PLXS_Write_CCCD_Request_Data->RemoteDevice;
               CCCDType       = PLXS_Event_Data->Event_Data.PLXS_Write_CCCD_Request_Data->Type;
               Configuration  = PLXS_Event_Data->Event_Data.PLXS_Write_CCCD_Request_Data->Configuration;

               /* Print event information.                              */
               printf("   Instance ID:      %u.\r\n", InstanceID);
               printf("   Connection ID:    %u.\r\n", ConnectionID);
               printf("   Connection Type:  %s.\r\n", ((ConnectionType == gctLE) ? "LE" : "BR/EDR"));
               printf("   Transaction ID:   %u.\r\n", TransactionID);
               BD_ADDRToStr(RemoteDevice, BoardStr);
               printf("   Remote Device:    %s.\r\n", BoardStr);
               printf("   Configuration:    0x%04X.\r\n", (unsigned int)Configuration);

               DisplayCCCDType(CCCDType);

               /* Get the device information for the remote device.     */
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, RemoteDevice)) != NULL)
               {
                  /* We will assume that we are successful.             */
                  ErrorCode = (Byte_t)PLXS_ERROR_CODE_SUCCESS;

                  /* Determine how to handle the configuration request  */
                  /* based on the CCCD Type.                            */
                  switch(CCCDType)
                  {
                     case pcdSpotCheck:
                        /* Only a configuration that is disabled (0) or */
                        /* set to enabled indications will be accepted. */
                        if((Configuration == 0) || (Configuration == ((Word_t)PLXS_CLIENT_CHARACTERISTIC_CONFIGURATION_INDICATE_ENABLE)))
                        {
                           /* Store the configuration.                  */
                           DeviceInfo->PLXSServerInfo.Spot_Check_Measurement_CCCD = Configuration;
                        }
                        else
                           ErrorCode = (Byte_t)PLXS_ERROR_CODE_CCCD_IMPROPERLY_CONFIGURED;
                        break;
                     case pcdContinuous:
                        /* Only a configuration that is disabled (0) or */
                        /* set to enabled notifications will be         */
                        /* accepted.                                    */
                        if((Configuration == 0) || (Configuration == ((Word_t)PLXS_CLIENT_CHARACTERISTIC_CONFIGURATION_NOTIFY_ENABLE)))
                        {
                           /* Store the configuration.                  */
                           DeviceInfo->PLXSServerInfo.Continuous_Measurement_CCCD = Configuration;
                        }
                        else
                           ErrorCode = (Byte_t)PLXS_ERROR_CODE_CCCD_IMPROPERLY_CONFIGURED;
                        break;
                     case pcdRACP:
                        /* Only a configuration that is disabled (0) or */
                        /* set to enabled indications will be accepted. */
                        if((Configuration == 0) || (Configuration == ((Word_t)PLXS_CLIENT_CHARACTERISTIC_CONFIGURATION_INDICATE_ENABLE)))
                        {
                           /* Store the configuration.                  */
                           DeviceInfo->PLXSServerInfo.Record_Access_Control_Point_CCCD = Configuration;
                        }
                        else
                           ErrorCode = (Byte_t)PLXS_ERROR_CODE_CCCD_IMPROPERLY_CONFIGURED;
                        break;
                     default:
                        /* This should NEVER occur since this type is   */
                        /* assigned by the service.                     */
                        printf("\r\nInvalid CCCD Type.\r\n");
                        ErrorCode = (Byte_t)PLXS_ERROR_CODE_UNLIKELY_ERROR;
                        break;
                  }

                  /* Send the response.                                 */
                  if((Result = PLXS_Write_CCCD_Request_Response(BluetoothStackID, InstanceID, TransactionID, ErrorCode, CCCDType))!= 0)
                  {
                     DisplayFunctionError("PLXS_Write_CCCD_Request_Response", Result);
                  }
               }
               else
                  printf("\r\nNo device information for remote device.\r\n");
            }
            else
               printf("\r\nInvalid Event pointer.\r\n");
            break;
         case etPLXS_Server_Confirmation:
            printf("\r\netPLXS_Server_Confirmation with size %u.\r\n", PLXS_Event_Data->Event_Data_Size);
            if(PLXS_Event_Data->Event_Data.PLXS_Confirmation_Data)
            {
               /* Store event information.                              */
               InstanceID     = PLXS_Event_Data->Event_Data.PLXS_Confirmation_Data->InstanceID;
               ConnectionID   = PLXS_Event_Data->Event_Data.PLXS_Confirmation_Data->ConnectionID;
               TransactionID  = PLXS_Event_Data->Event_Data.PLXS_Confirmation_Data->TransactionID;
               ConnectionType = PLXS_Event_Data->Event_Data.PLXS_Confirmation_Data->ConnectionType;
               RemoteDevice   = PLXS_Event_Data->Event_Data.PLXS_Confirmation_Data->RemoteDevice;
               Status         = PLXS_Event_Data->Event_Data.PLXS_Confirmation_Data->Status;
               BytesWritten   = PLXS_Event_Data->Event_Data.PLXS_Confirmation_Data->BytesWritten;

               /* Print event information.                              */
               printf("   Instance ID:      %u.\r\n", InstanceID);
               printf("   Connection ID:    %u.\r\n", ConnectionID);
               printf("   Transaction ID:   %u.\r\n", TransactionID);
               printf("   Connection Type:  %s.\r\n", ((ConnectionType == gctLE) ? "LE" : "BR/EDR"));
               BD_ADDRToStr(RemoteDevice, BoardStr);
               printf("   Remote Device:    %s.\r\n", BoardStr);
               printf("   Status:           0x%02X.\r\n", Status);
               printf("   Bytes Written:    %u.\r\n", BytesWritten);

               /* Get the device information for the remote device.     */
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, RemoteDevice)) != NULL)
               {
                  /* Check if a Report Stored Records Procedure is in   */
                  /* progress.                                          */
                  if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_RACP_PROCEDURE_IN_PROGRESS)
                  {
                     /* Go ahead and assume we are finished.            */
                     SendRACPResponse = TRUE;

                     /* We MUST have at least one Spot Check Measurement*/
                     /* to indicate.                                    */
                     for(Index = 0; Index < PLXP_MAXIMUM_NUMBER_OF_SPOT_CHECK_MEASUREMENTS; Index++)
                     {
                        /* Make sure the Spot Check Measurement is valid*/
                        /* and has not been indicated.                  */
                        if((SpotCheckMeasurementList[Index].Valid) && (SpotCheckMeasurementList[Index].Indicated == FALSE))
                        {
                           /* We found a Spot-Check Measurement to      */
                           /* indicate so we are not done.              */
                           SendRACPResponse = FALSE;

                           /* Flag that this Spot Check Measurement has */
                           /* been indicated.                           */
                           SpotCheckMeasurementList[Index].Indicated = TRUE;

                           /* Send the indication.                      */
                           if((Result = PLXS_Indicate_Spot_Check_Measurement(BluetoothStackID, PLXSInstanceID, ConnectionID, &(SpotCheckMeasurementList[Index].Measurement))) > 0)
                           {
                              printf("\r\nSpot Check Measurement indicated.\r\n");
                              printf("   Measurement ID:  %u\r\n", SpotCheckMeasurementList[Index].ID);
                              printf("   TransactionID:   %d\r\n", Result);
                           }
                           else
                              DisplayFunctionError("PLXS_Indicate_Spot_Check_Measurement", Result);

                           /* We CANNOT indicate another Spot Check     */
                           /* Measurement until we receive the          */
                           /* confirmation for the first indication.    */
                           /* * NOTE * We will also check if we are done*/
                           /*          when the confirmation is received*/
                           /*          and send the RACP Response       */
                           /*          indication.                      */
                           break;
                        }
                     }

                     /* Check if we have no more indications to send.   */
                     if(SendRACPResponse)
                     {
                        /* Format the response.                         */
                        RACPResponseData.Response_Op_Code      = rrotResponseOpCode;
                        RACPResponseData.Operator              = rotNull;
                        RACPResponseData.Request_Op_Code       = rrtReportStoredRecordsRequest;
                        RACPResponseData.Operand.Response_Code = rcvPLXSSuccess;

                        /* Send the indication for the result of the    */
                        /* RACP Procedure.                              */
                        if((Result = PLXS_Indicate_RACP_Response(BluetoothStackID, InstanceID, ConnectionID, &RACPResponseData)) > 0)
                        {
                           printf("\r\nRACP Response indicated.\r\n");
                           printf("   TransactionID:   %d\r\n", Result);
                        }
                        else
                           DisplayFunctionError("PLXS_Indicate_RACP_Response", Result);

                        /* Flag that a RACP Procedure is not progress.  */
                        DeviceInfo->Flags &= ~((Word_t)DEVICE_INFO_FLAGS_RACP_PROCEDURE_IN_PROGRESS);
                     }
                  }
               }
               else
                  printf("\r\nNo device information for remote device.\r\n");
            }
            else
               printf("\r\nInvalid Event pointer.\r\n");
            break;
         default:
            printf("\r\nUnknown PLXS Event.\r\n");
            break;
      }
   }
   else
   {
      /* There was an error with one or more of the input parameters.   */
      printf("\r\nPLXS Callback Data: Event_Data = NULL.\r\n");
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
                        NameBuffer[strlen((char *)(GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValue))] = '\0';

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
static void BTPSAPI GATT_ClientEventCallback_PLXS(unsigned int BluetoothStackID, GATT_Client_Event_Data_t *GATT_Client_Event_Data, unsigned long CallbackParameter)
{
   int                        Result;
   DeviceInfo_t              *DeviceInfo;
   BoardStr_t                 BoardStr;
   unsigned int               ConnectionID;
   unsigned int               TransactionID;
   GATT_Connection_Type_t     ConnectionType;
   BD_ADDR_t                  RemoteDevice;
   GATT_Request_Error_Type_t  ErrorType;
   Byte_t                    *Value;
   Word_t                     ValueLength;
   Word_t                     AttributeHandle;
   Word_t                     BytesWritten;
   Word_t                     MTU;
   Word_t                     Configuration;
   PLXS_Features_Data_t       PLX_Features;

   /* Verify that all parameters to this callback are Semi-Valid.       */
   if((BluetoothStackID) && (GATT_Client_Event_Data))
   {
      /* Store the attribute handle for the request.                    */
      AttributeHandle = (Word_t)CallbackParameter;

      /* Determine the event that occurred.                             */
      switch(GATT_Client_Event_Data->Event_Data_Type)
      {
         case etGATT_Client_Error_Response:
            printf("\r\netGATT_Client_Error_Response.\r\n");
            if(GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data)
            {
               /* Store the event data.                                 */
               ConnectionID   = GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ConnectionID;
               ConnectionType = GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ConnectionType;
               TransactionID  = GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->TransactionID;
               RemoteDevice   = GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->RemoteDevice;
               ErrorType      = GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorType;

               printf("   Connection ID:    %u.\r\n", ConnectionID);
               printf("   Connection Type:  %s.\r\n", (ConnectionType == gctLE) ? "LE" : "BR/EDR");
               printf("   Transaction ID:   %u.\r\n", TransactionID);
               BD_ADDRToStr(RemoteDevice, BoardStr);
               printf("   BD_ADDR:          %s.\r\n", BoardStr);
               printf("   Error Type:       %s.\r\n", (ErrorType == retErrorResponse) ? "Response Error" : "Response Timeout");

               /* Only print out the rest if it is valid.               */
               if(GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorType == retErrorResponse)
               {
                  printf("   Request Opcode:   0x%02X.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->RequestOpCode);
                  printf("   Request Handle:   0x%04X.\r\n", AttributeHandle);
                  printf("   Error Code:       0x%02X.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorCode);

                  /* Print common error codes.                          */
                  switch(GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorCode)
                  {
                     case PLXS_ERROR_CODE_INSUFFICIENT_ENCRYPTION:
                        printf("   Error Mesg:       PLXS_ERROR_CODE_INSUFFICIENT_ENCRYPTION\r\n");
                        break;
                     case PLXS_ERROR_CODE_CCCD_IMPROPERLY_CONFIGURED:
                        printf("   Error Mesg:       PLXS_ERROR_CODE_CCCD_IMPROPERLY_CONFIGURED\r\n");
                        break;
                     case PLXS_ERROR_CODE_PROCEDURE_ALREADY_IN_PROGRESS:
                        printf("   Error Mesg:       PLXS_ERROR_CODE_PROCEDURE_ALREADY_IN_PROGRESS\r\n");
                        break;
                     case PLXS_ERROR_CODE_UNLIKELY_ERROR:
                        printf("   Error Mesg:       PLXS_ERROR_CODE_UNLIKELY_ERROR\r\n");
                        break;
                     default:
                        if(GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorCode < NUMBER_OF_ERROR_CODES)
                        {
                           printf("   Error Mesg:       %s.\r\n", ErrorCodeStr[GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorCode]);
                        }
                        else
                        {
                           printf("   Error Mesg:       Unknown.\r\n");
                        }
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
               ConnectionType = GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->ConnectionType;
               TransactionID  = GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->TransactionID;
               RemoteDevice   = GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->RemoteDevice;
               ValueLength    = GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength;
               Value          = GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValue;

               /* Print the event data.                                 */
               printf("   Connection ID:    %u.\r\n", ConnectionID);
               printf("   Connection Type:  %s.\r\n", (ConnectionType == gctLE) ? "LE" : "BR/EDR");
               printf("   Transaction ID:   %u.\r\n", TransactionID);
               BD_ADDRToStr(RemoteDevice, BoardStr);
               printf("   BD_ADDR:          %s.\r\n", BoardStr);
               printf("   Handle:           0x%04X.\r\n", AttributeHandle);
               printf("   Value Length:     %u.\r\n", ValueLength);

               /* Get the device information.                           */
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, RemoteDevice)) != NULL)
               {
                  /* Use the attribute handle type set when the request */
                  /* was sent to handle the response.                   */
                  switch(PLXSRequestData.AttributeHandleType)
                  {
                     case ahtPLX_Features:
                        /* Decode the PLX Features.                     */
                        if((Result = PLXS_Decode_Features(ValueLength, Value, &PLX_Features)) == 0)
                        {
                           /* Display the features.                     */
                           DisplayPLXFeatures(&PLX_Features);
                        }
                        else
                           DisplayFunctionError("PLXS_Decode_Features", Result);
                        break;
                     case ahtSpot_Check_Measurement_CCCD:
                     case ahtContinuous_Measurement_CCCD:
                     case ahtRecordAccessControlPoint_CCCD:
                        /* Intentional fall-through.                    */

                        /* Make sure the received value is semi-valid.  */
                        if(ValueLength >= NON_ALIGNED_WORD_SIZE)
                        {
                           /* Decode the value.                         */
                           Configuration = READ_UNALIGNED_WORD_LITTLE_ENDIAN(Value);

                           /* Display the configuration.                */
                           printf("\r\nCCCD:\r\n");
                           printf("   Value: 0x%04X\r\n", (unsigned int)Configuration);
                        }
                        break;
                     default:
                        printf("\r\nInvalid handle for GATT Read Response.\r\n");
                        break;
                  }
               }
               else
                  printf("\r\nUnknown PLXP Server.\r\n");
            }
            else
               printf("\r\nError - Null Read Response Data.\r\n");
            break;
         case etGATT_Client_Write_Response:
            printf("\r\netGATT_Client_Write_Response.\r\n");
            if(GATT_Client_Event_Data->Event_Data.GATT_Write_Response_Data)
            {
               /* Store the event data.                                 */
               ConnectionID   = GATT_Client_Event_Data->Event_Data.GATT_Write_Response_Data->ConnectionID;
               ConnectionType = GATT_Client_Event_Data->Event_Data.GATT_Write_Response_Data->ConnectionType;
               TransactionID  = GATT_Client_Event_Data->Event_Data.GATT_Write_Response_Data->TransactionID;
               RemoteDevice   = GATT_Client_Event_Data->Event_Data.GATT_Write_Response_Data->RemoteDevice;
               BytesWritten   = GATT_Client_Event_Data->Event_Data.GATT_Write_Response_Data->BytesWritten;

               /* Print the event data.                                 */
               printf("   Connection ID:    %u.\r\n", ConnectionID);
               printf("   Connection Type:  %s.\r\n", (ConnectionType == gctLE) ? "LE" : "BR/EDR");
               printf("   Transaction ID:   %u.\r\n", TransactionID);
               BD_ADDRToStr(RemoteDevice, BoardStr);
               printf("   BD_ADDR:          %s.\r\n", BoardStr);
               printf("   Handle:           0x%04X.\r\n", AttributeHandle);
               printf("   Bytes Written:    %u.\r\n", BytesWritten);
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
               ConnectionType  = GATT_Client_Event_Data->Event_Data.GATT_Exchange_MTU_Response_Data->ConnectionType;
               TransactionID   = GATT_Client_Event_Data->Event_Data.GATT_Exchange_MTU_Response_Data->TransactionID;
               RemoteDevice    = GATT_Client_Event_Data->Event_Data.GATT_Exchange_MTU_Response_Data->RemoteDevice;
               MTU             = GATT_Client_Event_Data->Event_Data.GATT_Exchange_MTU_Response_Data->ServerMTU;

               /* Print the event data.                                 */
               printf("   Connection ID:     %u.\r\n", ConnectionID);
               printf("   Connection Type:   %s.\r\n", (ConnectionType == gctLE) ? "LE" : "BR/EDR");
               printf("   Transaction ID:    %u.\r\n", TransactionID);
               BD_ADDRToStr(RemoteDevice, BoardStr);
               printf("   BD_ADDR:           %s.\r\n", BoardStr);
               printf("   MTU:               %u.\r\n", MTU);
            }
            else
               printf("\r\nError - Null Write Response Data.\r\n");
            break;
         default:
            /* Catch compiler warnings.                                 */
            break;
      }
   }
   else
   {
      /* There was an error with one or more of the input parameters.   */
      printf("\r\nGATT Callback Data: Event_Data = NULL.\r\n");
   }

   /* Print the command line prompt.                                    */
   DisplayPrompt();
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
   int                                 Result;
   DeviceInfo_t                       *DeviceInfo;
   GAP_Encryption_Mode_t               EncryptionMode;

   /* GATT Connection Event data types.                                 */
   BoardStr_t                          BoardStr;
   BD_ADDR_t                           RemoteBDADDR;
   unsigned int                        GATTConnectionID;
   unsigned int                        TransactionID;
   GATT_Connection_Type_t              ConnectionType;
   Word_t                              MTU;
   Word_t                              AttributeHandle;
   Word_t                              ValueLength;
   Byte_t                             *Value;
   PLXS_Spot_Check_Measurement_Data_t  SpotCheckMeasurement;
   PLXS_Continuous_Measurement_Data_t  ContinuousMeasurement;
   PLXS_RACP_Response_Data_t           RACPResponseData;

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
               /* Store the event data.                                 */
               ConnectionID      = GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data->ConnectionID;
               ConnectionType    = GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data->ConnectionType;
               ConnectionBD_ADDR = GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data->RemoteDevice;
               MTU               = GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data->MTU;

               printf("   Connection ID:    %u.\r\n", ConnectionID);
               printf("   Connection Type:  %s.\r\n", ((ConnectionType == gctLE) ? "LE" : "BR/EDR"));
               BD_ADDRToStr(ConnectionBD_ADDR, BoardStr);
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
                        GATT_Exchange_MTU_Request(BluetoothStackID, ConnectionID, BTPS_CONFIGURATION_GATT_DEFAULT_MAXIMUM_SUPPORTED_MTU_SIZE, GATT_ClientEventCallback_PLXS, 0);
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
               ConnectionID      = GATT_Connection_Event_Data->Event_Data.GATT_Device_Disconnection_Data->ConnectionID;
               ConnectionType    = GATT_Connection_Event_Data->Event_Data.GATT_Device_Disconnection_Data->ConnectionType;
               ConnectionBD_ADDR = GATT_Connection_Event_Data->Event_Data.GATT_Device_Disconnection_Data->RemoteDevice;

               printf("   Connection ID:    %u.\r\n", ConnectionID);
               printf("   Connection Type:  %s.\r\n", ((ConnectionType == gctLE) ? "LE" : "BR/EDR"));
               BD_ADDRToStr(ConnectionBD_ADDR, BoardStr);
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
               GATTConnectionID = GATT_Connection_Event_Data->Event_Data.GATT_Device_Disconnection_Data->ConnectionID;
               ConnectionType   = GATT_Connection_Event_Data->Event_Data.GATT_Device_Disconnection_Data->ConnectionType;
               RemoteBDADDR     = GATT_Connection_Event_Data->Event_Data.GATT_Device_Disconnection_Data->RemoteDevice;
               MTU              = GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_MTU_Update_Data->MTU;

               printf("   Connection ID:    %u.\r\n", GATTConnectionID);
               printf("   Connection Type:  %s.\r\n", (ConnectionType == gctLE)?"LE":"BR/EDR");
               BD_ADDRToStr(RemoteBDADDR, BoardStr);
               printf("   BD_ADDR:          %s.\r\n", BoardStr);
               printf("   MTU:              %u.\r\n", MTU);
            }
            else
               printf("\r\nError - Null Connection Update Data.");
            break;
         case etGATT_Connection_Server_Indication:
            printf("\r\netGATT_Connection_Server_Indication with size %u: \r\n", GATT_Connection_Event_Data->Event_Data_Size);
            if(GATT_Connection_Event_Data->Event_Data.GATT_Server_Indication_Data)
            {
               /* Store the event data.                                 */
               RemoteBDADDR     = GATT_Connection_Event_Data->Event_Data.GATT_Server_Indication_Data->RemoteDevice;
               GATTConnectionID = GATT_Connection_Event_Data->Event_Data.GATT_Server_Indication_Data->ConnectionID;
               TransactionID    = GATT_Connection_Event_Data->Event_Data.GATT_Server_Indication_Data->TransactionID;
               ConnectionType   = GATT_Connection_Event_Data->Event_Data.GATT_Server_Indication_Data->ConnectionType;
               AttributeHandle  = GATT_Connection_Event_Data->Event_Data.GATT_Server_Indication_Data->AttributeHandle;
               ValueLength      = GATT_Connection_Event_Data->Event_Data.GATT_Server_Indication_Data->AttributeValueLength;
               Value            = GATT_Connection_Event_Data->Event_Data.GATT_Server_Indication_Data->AttributeValue;

               BD_ADDRToStr(RemoteBDADDR, BoardStr);
               printf("   Connection ID:     %u.\r\n", GATTConnectionID);
               printf("   Transaction ID:    %u.\r\n", TransactionID);
               printf("   Connection Type:   %s.\r\n", ((ConnectionType == gctLE) ? "LE" : "BR/EDR"));
               printf("   Remote Device:     %s.\r\n", BoardStr);
               printf("   Attribute Handle:  0x%04X.\r\n", AttributeHandle);
               printf("   Attribute Length:  %d.\r\n", ValueLength);

               /* Send the confirmation.                                */
               if((Result = GATT_Handle_Value_Confirmation(BluetoothStackID, GATTConnectionID, TransactionID)) != 0)
                  DisplayFunctionError("GATT_Handle_Value_Confirmation", Result);

               /* Get the device information for the remote device.     */
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, RemoteBDADDR)) != NULL)
               {
                  /* Make sure service discovery has been performed so  */
                  /* we can verify the handle.                          */
                  /* * NOTE * The PLXP Client should have written the   */
                  /*          corresponding CCCD before an indication is*/
                  /*          received.  This means service discovery   */
                  /*          MUST have been performed.  Otherwise the  */
                  /*          PLXP Server sent an indication for an     */
                  /*          unconfigured CCCD.  This is an error.     */
                  if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_PLXS_SERVICE_DISCOVERY_COMPLETE)
                  {
                     /* Check if this is a Spot Check Measurement       */
                     /* indication.                                     */
                     if(AttributeHandle == DeviceInfo->PLXSClientInfo.Spot_Check_Measurement)
                     {
                        /* Initialize the Spot Check Measurement.       */
                        BTPS_MemInitialize(&SpotCheckMeasurement, 0, PLXS_SPOT_CHECK_MEASUREMENT_DATA_SIZE);

                        /* Decode the Spot Check Measurement.           */
                        if((Result = PLXS_Decode_Spot_Check_Measurement(ValueLength, Value, &SpotCheckMeasurement)) == 0)
                        {
                           DisplaySpotCheckMeasurement(&SpotCheckMeasurement);
                        }
                        else
                           DisplayFunctionError("PLXS_Decode_Spot_Check_Measurement", Result);

                        break;
                     }

                     /* Check if this is a Record Access Control Point  */
                     /* indication.                                     */
                     if(AttributeHandle == DeviceInfo->PLXSClientInfo.Record_Access_Control_Point)
                     {
                        /* Initialize the RACP Response Data.           */
                        BTPS_MemInitialize(&RACPResponseData, 0, PLXS_RACP_RESPONSE_DATA_SIZE);

                        /* Decode the RACP Response Data.               */
                        if((Result = PLXS_Decode_RACP_Response(ValueLength, Value, &RACPResponseData)) == 0)
                        {
                           /* Display the RACP Response Data.           */
                           DisplayRACPResponseData(&RACPResponseData);
                        }
                        else
                           DisplayFunctionError("PLXS_Decode_RACP_Response", Result);

                        break;
                     }

                     printf("\r\nInvalid attribute handle received with indication.\r\n");
                  }
                  else
                     printf("\r\nError - Service discovery has not been performed.\r\n");
               }
               else
                  printf("\r\nError - Unknown PLXP Server.\r\n");
            }
            break;
         case etGATT_Connection_Server_Notification:
            printf("\r\netGATT_Connection_Server_Notification with size %u: \r\n", GATT_Connection_Event_Data->Event_Data_Size);
            if(GATT_Connection_Event_Data->Event_Data.GATT_Server_Notification_Data)
            {
               /* Store the event data.                                 */
               RemoteBDADDR     = GATT_Connection_Event_Data->Event_Data.GATT_Server_Notification_Data->RemoteDevice;
               GATTConnectionID = GATT_Connection_Event_Data->Event_Data.GATT_Server_Notification_Data->ConnectionID;
               ConnectionType   = GATT_Connection_Event_Data->Event_Data.GATT_Server_Notification_Data->ConnectionType;
               AttributeHandle  = GATT_Connection_Event_Data->Event_Data.GATT_Server_Notification_Data->AttributeHandle;
               ValueLength      = GATT_Connection_Event_Data->Event_Data.GATT_Server_Notification_Data->AttributeValueLength;
               Value            = GATT_Connection_Event_Data->Event_Data.GATT_Server_Notification_Data->AttributeValue;

               BD_ADDRToStr(RemoteBDADDR, BoardStr);
               printf("   Connection ID:     %u.\r\n", GATTConnectionID);
               printf("   Connection Type:   %s.\r\n", ((ConnectionType == gctLE) ? "LE" : "BR/EDR"));
               printf("   Remote Device:     %s.\r\n", BoardStr);
               printf("   Attribute Handle:  0x%04X.\r\n", AttributeHandle);
               printf("   Attribute Length:  %d.\r\n", ValueLength);

               /* Get the device information for the remote device.     */
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, RemoteBDADDR)) != NULL)
               {
                  /* Make sure service discovery has been performed so  */
                  /* we can verify the handle.                          */
                  /* * NOTE * The PLXP Client should have written the   */
                  /*          corresponding CCCD before an indication is*/
                  /*          received.  This means service discovery   */
                  /*          MUST have been performed.  Otherwise the  */
                  /*          PLXP Server sent an indication for an     */
                  /*          unconfigured CCCD.  This is an error.     */
                  if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_PLXS_SERVICE_DISCOVERY_COMPLETE)
                  {
                     /* Check if this is a Continuous Measurement       */
                     /* indication.                                     */
                     if(AttributeHandle == DeviceInfo->PLXSClientInfo.Continuous_Measurement)
                     {
                        /* Initialize the Continuous Measurement.       */
                        BTPS_MemInitialize(&ContinuousMeasurement, 0, PLXS_CONTINUOUS_MEASUREMENT_DATA_SIZE);

                        /* Decode the Continuous Measurement.           */
                        if((Result = PLXS_Decode_Continuous_Measurement(ValueLength, Value, &ContinuousMeasurement)) == 0)
                        {
                           DisplayContinuousMeasurement(&ContinuousMeasurement);
                        }
                        else
                           DisplayFunctionError("PLXS_Decode_Continuous_Measurement", Result);

                        break;
                     }

                     printf("\r\nInvalid attribute handle received with notification.\r\n");
                  }
                  else
                     printf("\r\nError - Service discovery has not been performed.\r\n");
               }
               else
                  printf("\r\nError - Unknown PLXP Server.\r\n");
            }
            break;
         default:
            break;
      }
   }
   else
      printf("\r\nGATT Connection Callback Data: Event_Data = NULL.\r\n");

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
   unsigned int                              Index;
   DeviceInfo_t                             *DeviceInfo;
   GATT_Service_Information_t               *ServiceInfo;
   GATT_Service_Information_t               *IncludedServiceInfo;
   unsigned int                              NumberOfCharacteristics;
   unsigned int                              NumberOfIncludedServices;

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
                  /* Store the event data.                              */
                  ServiceInfo              = &(GATT_Service_Discovery_Event_Data->Event_Data.GATT_Service_Discovery_Indication_Data->ServiceInformation);
                  NumberOfCharacteristics  =   GATT_Service_Discovery_Event_Data->Event_Data.GATT_Service_Discovery_Indication_Data->NumberOfCharacteristics;
                  NumberOfIncludedServices =   GATT_Service_Discovery_Event_Data->Event_Data.GATT_Service_Discovery_Indication_Data->NumberOfIncludedService;

                  /* Print the event data.                              */
                  printf("\r\nService:\r\n");
                  DisplayKnownServiceName(&(ServiceInfo->UUID));
                  printf("   UUID:             0x");
                  DisplayUUID(&(ServiceInfo->UUID));
                  printf("   Handle Range:     0x%04X - 0x%04X\r\n", ServiceInfo->Service_Handle, ServiceInfo->End_Group_Handle);
                  printf("   Characteristics:  %u\r\n", NumberOfCharacteristics);

                  /* If any included services exist, display those too. */
                  if(NumberOfIncludedServices)
                  {
                     printf("\r\nIncluded Services:\r\n");
                     for(Index = 0; Index < NumberOfIncludedServices; Index++, IncludedServiceInfo++)
                     {
                        /* Store a pointer to the included service info.*/
                        IncludedServiceInfo = &(GATT_Service_Discovery_Event_Data->Event_Data.GATT_Service_Discovery_Indication_Data->IncludedServiceList[Index]);

                        /* Print the include service info.              */
                        printf("\r\nIncluded Service (%u):\r\n", Index);
                        DisplayKnownServiceName(&(IncludedServiceInfo->UUID));
                        printf("   UUID:             0x");
                        DisplayUUID(&(IncludedServiceInfo->UUID));
                        printf("   Handle Range:     0x%04X - 0x%04X\r\n", IncludedServiceInfo->Service_Handle, IncludedServiceInfo->End_Group_Handle);
                     }
                  }

                  /* Check the service discovery type.                  */
                  switch((Service_Discovery_Type_t)CallbackParameter)
                  {
                     case sdAll:
                        /* Attempt to populate the handles for all found*/
                        /* Services.                                    */
                        GATTPopulateHandles(&(DeviceInfo->GATTClientInfo), GATT_Service_Discovery_Event_Data->Event_Data.GATT_Service_Discovery_Indication_Data);
                        DISPopulateHandles(&(DeviceInfo->DISClientInfo), GATT_Service_Discovery_Event_Data->Event_Data.GATT_Service_Discovery_Indication_Data);
                        GAPSPopulateHandles(&(DeviceInfo->GAPSClientInfo), GATT_Service_Discovery_Event_Data->Event_Data.GATT_Service_Discovery_Indication_Data);
                        PLXSPopulateHandles(&(DeviceInfo->PLXSClientInfo), GATT_Service_Discovery_Event_Data->Event_Data.GATT_Service_Discovery_Indication_Data);
                        break;
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
                     case sdPLXS:
                        /* Attempt to populate the handles for the PLXS */
                        /* Service.                                     */
                        PLXSPopulateHandles(&(DeviceInfo->PLXSClientInfo), GATT_Service_Discovery_Event_Data->Event_Data.GATT_Service_Discovery_Indication_Data);
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
                     case sdAll:
                        /* Flag that service discovery has been         */
                        /* performed on for this connection.            */
                        DeviceInfo->Flags |= DEVICE_INFO_FLAGS_GATT_SERVICE_DISCOVERY_COMPLETE;
                        DeviceInfo->Flags |= DEVICE_INFO_FLAGS_DIS_SERVICE_DISCOVERY_COMPLETE;
                        DeviceInfo->Flags |= DEVICE_INFO_FLAGS_GAPS_SERVICE_DISCOVERY_COMPLETE;
                        DeviceInfo->Flags |= DEVICE_INFO_FLAGS_PLXS_SERVICE_DISCOVERY_COMPLETE;

                        /* Display the service discovery summaries.     */
                        DisplayGATTDiscoverySummary(&(DeviceInfo->GATTClientInfo));
                        DisplayDISDiscoverySummary(&(DeviceInfo->DISClientInfo));
                        DisplayGAPSDiscoverySummary(&(DeviceInfo->GAPSClientInfo));
                        DisplayPLXSDiscoverySummary(&(DeviceInfo->PLXSClientInfo));
                        break;
                     case sdDIS:
                        /* Flag that service discovery has been         */
                        /* performed on for this connection.            */
                        DeviceInfo->Flags |= DEVICE_INFO_FLAGS_DIS_SERVICE_DISCOVERY_COMPLETE;

                        /* Display the service discovery summary.       */
                        DisplayDISDiscoverySummary(&(DeviceInfo->DISClientInfo));
                        break;
                     case sdGAPS:
                        /* Flag that service discovery has been         */
                        /* performed on for this connection.            */
                        DeviceInfo->Flags |= DEVICE_INFO_FLAGS_GAPS_SERVICE_DISCOVERY_COMPLETE;

                        /* Display the service discovery summary.       */
                        DisplayGAPSDiscoverySummary(&(DeviceInfo->GAPSClientInfo));
                        break;
                     case sdPLXS:
                        /* Flag that service discovery has been         */
                        /* performed on for this connection.            */
                        DeviceInfo->Flags |= DEVICE_INFO_FLAGS_PLXS_SERVICE_DISCOVERY_COMPLETE;

                        /* Display the service discovery summary.       */
                        DisplayPLXSDiscoverySummary(&(DeviceInfo->PLXSClientInfo));
                        break;
                     default:
                        printf("\r\nWarning - Unknown service discovery type.\r\n");
                        break;
                  }
               }
               break;
            default:
               printf("\r\nInvalid GATT Service Discovery Event Type.\r\n");
               break;
         }
      }
   }
   else
      printf("\r\nFailure - GATT Service Discovery Event Data is NULL.\r\n");

   DisplayPrompt();
}

   /* The following function is the timer callback function for the     */
   /* minute timer.  This function will be called whenever a timer has  */
   /* been created, a Callback has been registered for the associated   */
   /* BluetoothStackID, and a minute has passed.  The purpose of this   */
   /* function is to periodically create and notify a continuous        */
   /* measurement..                                                     */
static void BTPSAPI PLXP_Continuous_Measurement_Timer_Callback(unsigned int BluetoothStackID, unsigned int TimerID, unsigned long CallbackParameter)
{
   int                                Result;
   PLXS_Continuous_Measurement_Data_t Measurement;

   /* Initialize the new measurement.                                   */
   BTPS_MemInitialize(&(Measurement), 0, PLXS_CONTINUOUS_MEASUREMENT_DATA_SIZE);

   /* Assign the mandatory Measurement data.                            */
   Measurement.Flags       = NotificationData.Flags;
   Measurement.SpO2_Normal = (Word_t)(rand() % 100);
   Measurement.PR_Normal   = (Word_t)(rand() % 100);

   /* Check if we need to include the Fast fields.                      */
   if(Measurement.Flags & PLXS_CONTINUOUS_MEASUREMENT_FLAGS_SPO2PR_FAST_FIELD_PRESENT)
   {
      Measurement.SpO2_Fast   = (Word_t)(rand() % 100);
      Measurement.PR_Fast     = (Word_t)(rand() % 100);
   }

   /* Check if we need to include the Slow fields.                      */
   if(Measurement.Flags & PLXS_CONTINUOUS_MEASUREMENT_FLAGS_SPO2PR_SLOW_FIELD_PRESENT)
   {
      Measurement.SpO2_Slow   = (Word_t)(rand() % 100);
      Measurement.PR_Slow     = (Word_t)(rand() % 100);
   }

   /* Check if we need to include the Measurement Status.               */
   if(Measurement.Flags & PLXS_CONTINUOUS_MEASUREMENT_FLAGS_MEASUREMENT_STATUS_PRESENT)
   {
      Measurement.Measurement_Status       = NotificationData.Measurement_Status;
   }

   /* Check if we need to include the Device and Sensor Status.         */
   if(Measurement.Flags & PLXS_CONTINUOUS_MEASUREMENT_FLAGS_DEVICE_AND_SENSOR_STATUS_PRESENT)
   {
      Measurement.Device_And_Sensor_Status = NotificationData.Device_And_Sensor_Status;
   }

   /* Check if we need to include the Pulse Amplitude Index.            */
   if(Measurement.Flags & PLXS_CONTINUOUS_MEASUREMENT_FLAGS_PULSE_AMPLITUDE_INDEX_PRESENT)
   {
      Measurement.Pulse_Amplitude_Index    = (Word_t)(rand() % 100);
   }

   /* Send the notification.                                            */
   /* * NOTE * If this fails we will stop sending notifications.        */
   if((Result = PLXS_Notify_Continuous_Measurement(BluetoothStackID, PLXSInstanceID, ConnectionID, &Measurement)) > 0)
   {
      printf("\r\nContinuous Measurement notified.\r\n");
      printf("   Measurement Length:  %d\r\n", Result);

      /* Restart the timer for the next measurement.                    */
      if((NotificationData.TimerID = BSC_StartTimer(BluetoothStackID, NotificationData.Timeout, PLXP_Continuous_Measurement_Timer_Callback, 0)) <= 0)
      {
         printf("\r\nCould not restart the timer.\r\n");
         NotificationData.NotificationsStarted = FALSE;
         NotificationData.TimerID              = 0;
      }
   }
   else
   {
      DisplayFunctionError("PLXS_Notify_Continuous_Measurement", Result);

      printf("\r\nStopping notifications.\r\n");
      NotificationData.NotificationsStarted = FALSE;
      NotificationData.TimerID              = 0;

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
         case PLXSP_PARAMETER_VALUE:
            /* The Transport selected was PLXSP, check to see if the    */
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
         printf("Parameter Error (Expecting = [USB = 0, UART = 1, PLXSP = 2 Flag] [IF !USB [Comm Port or Device File] [Baud Rate]])\r\n");
      }
   }
   else
   {
      /* An invalid number of parameters was entered on the Command     */
      /* Line.                                                          */
      printf("Parameter Error (Expecting = [USB = 0, UART = 1, PLXSP = 2 Flag] [IF !USB [Comm Port or Device File] [Baud Rate]])\r\n");
   }

   return 0;
}
